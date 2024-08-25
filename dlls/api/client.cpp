/***
 *
 *	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
 *
 *	This product contains software technology licensed from Id
 *	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
 *	All Rights Reserved.
 *
 *   Use, distribution, and modification of this source code and/or resulting
 *   object code is restricted to non-commercial enhancements to products from
 *   Valve LLC.  All other use, distribution, or modification is prohibited
 *   without written permission from Valve LLC.
 *
 ****/

#include "api/client.h"
#include "api/game.h"
#include "UserMessages.h"
#include "classes/CSound.h"
#include "classes/gamerules/CGameRules.h"
#include "classes/gamerules/CHalfLifeMultiplay.h"
#include "entities/item/CItemAntidote.h"
#include "entities/item/CItemAntiRad.h"
#include "entities/item/CItemCamera.h"
#include "entities/item/CItemFlare.h"
#include "entities/item/CItemMedicalKit.h"
#include "entities/player/CBasePlayer.h"
#include "entities/sound/CSoundEnt.h"
#include "util/message.h"
#include "util/trace.h"

/*
===========
ClientConnect

called when a player connects to a server
============
*/
qboolean ClientConnect(edict_t* pEntity, const char* pszName, const char* pszAddress, char szRejectReason[128])
{
	return static_cast<qboolean>(g_pGameRules->ClientConnected(pEntity, pszName, pszAddress, szRejectReason));

	// a client connecting during an intermission can cause problems
	//	if (intermission_running)
	//		ExitIntermission ();
}

/*
===========
ClientDisconnect

called when a player disconnects from a server

GLOBALS ASSUMED SET:  g_fGameOver
============
*/
void ClientDisconnect(edict_t* pEntity)
{
	if (g_fGameOver)
		return;

	char text[256] = "";
	if (!FStringNull(pEntity->v.netname))
		snprintf(text, sizeof(text), "- %s has left the game\n", STRING(pEntity->v.netname));
	text[sizeof(text) - 1] = 0;
	MESSAGE_BEGIN(MSG_ALL, gmsgSayText, NULL);
	WRITE_BYTE(ENTINDEX(pEntity));
	WRITE_STRING(text);
	MESSAGE_END();

	CSound* pSound;
	pSound = CSoundEnt::SoundPointerForIndex(CSoundEnt::ClientSoundIndex(pEntity));
	{
		// since this client isn't around to think anymore, reset their sound.
		if (pSound)
		{
			pSound->Reset();
		}
	}

	// since the edict doesn't get deleted, fix it so it doesn't interfere.
	pEntity->v.takedamage = DAMAGE_NO; // don't attract autoaim
	pEntity->v.solid = SOLID_NOT;	   // nonsolid
	UTIL_SetEdictOrigin(pEntity, pEntity->v.origin);

	auto pPlayer = reinterpret_cast<CBasePlayer*>(GET_PRIVATE(pEntity));

	if (pPlayer)
	{
		if (pPlayer->m_pTank != NULL)
		{
			pPlayer->m_pTank->Use(pPlayer, pPlayer, USE_OFF, 0);
			pPlayer->m_pTank = NULL;
		}
	}

	g_pGameRules->ClientDisconnected(pEntity);
}

/*
============
ClientKill

Player entered the suicide command

GLOBALS ASSUMED SET:  g_ulModelIndexPlayer
============
*/
void ClientKill(edict_t* pEntity)
{
	entvars_t* pev = &pEntity->v;

	CBasePlayer* pl = (CBasePlayer*)CBasePlayer::Instance(pev);

	if (pl->m_fNextSuicideTime > gpGlobals->time)
		return; // prevent suiciding too ofter

	pl->m_fNextSuicideTime = gpGlobals->time + 1; // don't let them suicide for 5 seconds after suiciding

	// have the player kill themself
	pev->health = 0;
	pl->Killed(NULL, pev, GIB_NEVER);

	//	pev->modelindex = g_ulModelIndexPlayer;
	//	pev->frags -= 2;		// extra penalty
	//	respawn( pev );
}

/*
===========
ClientPutInServer

called each time a player is spawned
============
*/
void ClientPutInServer(edict_t* pEntity)
{
	CBasePlayer* pPlayer;

	entvars_t* pev = &pEntity->v;

	pPlayer = GetClassPtr((CBasePlayer*)pev);
	pPlayer->SetCustomDecalFrames(-1); // Assume none;

	// Allocate a CBasePlayer for pev, and call spawn
	pPlayer->Spawn();

	// Reset interpolation during first frame
	pPlayer->pev->effects |= EF_NOINTERP;

	pPlayer->pev->iuser1 = 0; // disable any spec modes
	pPlayer->pev->iuser2 = 0;
}

#if defined(_MSC_VER) || defined(WIN32)
typedef wchar_t uchar16;
typedef unsigned int uchar32;
#else
typedef unsigned short uchar16;
typedef wchar_t uchar32;
#endif

//-----------------------------------------------------------------------------
// Purpose: determine if a uchar32 represents a valid Unicode code point
//-----------------------------------------------------------------------------
bool Q_IsValidUChar32(uchar32 uVal)
{
	// Values > 0x10FFFF are explicitly invalid; ditto for UTF-16 surrogate halves,
	// values ending in FFFE or FFFF, or values in the 0x00FDD0-0x00FDEF reserved range
	return (uVal < 0x110000u) && ((uVal - 0x00D800u) > 0x7FFu) && ((uVal & 0xFFFFu) < 0xFFFEu) && ((uVal - 0x00FDD0u) > 0x1Fu);
}

// Decode one character from a UTF-8 encoded string. Treats 6-byte CESU-8 sequences
// as a single character, as if they were a correctly-encoded 4-byte UTF-8 sequence.
int Q_UTF8ToUChar32(const char* pUTF8_, uchar32& uValueOut, bool& bErrorOut)
{
	const uint8* pUTF8 = (const uint8*)pUTF8_;

	int nBytes = 1;
	uint32 uValue = pUTF8[0];
	uint32 uMinValue = 0;

	// 0....... single byte
	if (uValue < 0x80)
		goto decodeFinishedNoCheck;

	// Expecting at least a two-byte sequence with 0xC0 <= first <= 0xF7 (110...... and 11110...)
	if ((uValue - 0xC0u) > 0x37u || (pUTF8[1] & 0xC0) != 0x80)
		goto decodeError;

	uValue = (uValue << 6) - (0xC0 << 6) + pUTF8[1] - 0x80;
	nBytes = 2;
	uMinValue = 0x80;

	// 110..... two-byte lead byte
	if ((uValue & (0x20 << 6)) == 0)
		goto decodeFinished;

	// Expecting at least a three-byte sequence
	if ((pUTF8[2] & 0xC0) != 0x80)
		goto decodeError;

	uValue = (uValue << 6) - (0x20 << 12) + pUTF8[2] - 0x80;
	nBytes = 3;
	uMinValue = 0x800;

	// 1110.... three-byte lead byte
	if ((uValue & (0x10 << 12)) == 0)
		goto decodeFinishedMaybeCESU8;

	// Expecting a four-byte sequence, longest permissible in UTF-8
	if ((pUTF8[3] & 0xC0) != 0x80)
		goto decodeError;

	uValue = (uValue << 6) - (0x10 << 18) + pUTF8[3] - 0x80;
	nBytes = 4;
	uMinValue = 0x10000;

	// 11110... four-byte lead byte. fall through to finished.

decodeFinished:
	if (uValue >= uMinValue && Q_IsValidUChar32(uValue))
	{
	decodeFinishedNoCheck:
		uValueOut = uValue;
		bErrorOut = false;
		return nBytes;
	}
decodeError:
	uValueOut = '?';
	bErrorOut = true;
	return nBytes;

decodeFinishedMaybeCESU8:
	// Do we have a full UTF-16 surrogate pair that's been UTF-8 encoded afterwards?
	// That is, do we have 0xD800-0xDBFF followed by 0xDC00-0xDFFF? If so, decode it all.
	if ((uValue - 0xD800u) < 0x400u && pUTF8[3] == 0xED && (uint8)(pUTF8[4] - 0xB0) < 0x10 && (pUTF8[5] & 0xC0) == 0x80)
	{
		uValue = 0x10000 + ((uValue - 0xD800u) << 10) + ((uint8)(pUTF8[4] - 0xB0) << 6) + pUTF8[5] - 0x80;
		nBytes = 6;
		uMinValue = 0x10000;
	}
	goto decodeFinished;
}

//-----------------------------------------------------------------------------
// Purpose: Returns true if UTF-8 string contains invalid sequences.
//-----------------------------------------------------------------------------
bool Q_UnicodeValidate(const char* pUTF8)
{
	bool bError = false;
	while ('\0' != *pUTF8)
	{
		uchar32 uVal;
		// Our UTF-8 decoder silently fixes up 6-byte CESU-8 (improperly re-encoded UTF-16) sequences.
		// However, these are technically not valid UTF-8. So if we eat 6 bytes at once, it's an error.
		int nCharSize = Q_UTF8ToUChar32(pUTF8, uVal, bError);
		if (bError || nCharSize == 6)
			return false;
		pUTF8 += nCharSize;
	}
	return true;
}

//// HOST_SAY
// String comes in as
// say blah blah blah
// or as
// blah blah blah
//
void Host_Say(edict_t* pEntity, bool teamonly)
{
	CBasePlayer* client;
	int j;
	char* p;
	char text[128];
	char szTemp[256];
	const char* cpSay = "say";
	const char* cpSayTeam = "say_team";
	const char* pcmd = CMD_ARGV(0);

	// We can get a raw string now, without the "say " prepended
	if (CMD_ARGC() == 0)
		return;

	entvars_t* pev = &pEntity->v;
	CBasePlayer* player = GetClassPtr((CBasePlayer*)pev);

	// Not yet.
	if (player->m_flNextChatTime > gpGlobals->time)
		return;

	if (!stricmp(pcmd, cpSay) || !stricmp(pcmd, cpSayTeam))
	{
		if (CMD_ARGC() >= 2)
		{
			p = (char*)CMD_ARGS();
		}
		else
		{
			// say with a blank message, nothing to do
			return;
		}
	}
	else // Raw text, need to prepend argv[0]
	{
		if (CMD_ARGC() >= 2)
		{
			sprintf(szTemp, "%s %s", (char*)pcmd, (char*)CMD_ARGS());
		}
		else
		{
			// Just a one word command, use the first word...sigh
			sprintf(szTemp, "%s", (char*)pcmd);
		}
		p = szTemp;
	}

	// remove quotes if present
	if (*p == '"')
	{
		p++;
		p[strlen(p) - 1] = 0;
	}

	// make sure the text has content

	if (!p || '\0' == p[0] || !Q_UnicodeValidate(p))
		return; // no character found, so say nothing

	// turn on color set 2  (color on,  no sound)
	// turn on color set 2  (color on,  no sound)
	if (player->IsObserver() && (teamonly))
		sprintf(text, "%c(SPEC) %s: ", 2, STRING(pEntity->v.netname));
	else if (teamonly)
		sprintf(text, "%c(TEAM) %s: ", 2, STRING(pEntity->v.netname));
	else
		sprintf(text, "%c%s: ", 2, STRING(pEntity->v.netname));

	j = sizeof(text) - 2 - strlen(text); // -2 for /n and null terminator
	if ((int)strlen(p) > j)
		p[j] = 0;

	strcat(text, p);
	strcat(text, "\n");


	player->m_flNextChatTime = gpGlobals->time + CHAT_INTERVAL;

	// loop through all players
	// Start with the first player.
	// This may return the world in single player if the client types something between levels or during spawn
	// so check it, or it will infinite loop

	client = NULL;
	while (((client = (CBasePlayer*)UTIL_FindEntityByClassname(client, "player")) != NULL) && (!FNullEnt(client->edict())))
	{
		if (!client->pev)
			continue;

		if (client->edict() == pEntity)
			continue;

		if (!(client->IsNetClient())) // Not a client ? (should never be true)
			continue;

		// can the receiver hear the sender? or has he muted him?
		if (g_VoiceGameMgr.PlayerHasBlockedPlayer(client, player))
			continue;

		if (!player->IsObserver() && teamonly && g_pGameRules->PlayerRelationship(client, CBaseEntity::Instance(pEntity)) != GR_TEAMMATE)
			continue;

		// Spectators can only talk to other specs
		if (player->IsObserver() && teamonly)
			if (!client->IsObserver())
				continue;

		MESSAGE_BEGIN(MSG_ONE, gmsgSayText, NULL, client->pev);
		WRITE_BYTE(ENTINDEX(pEntity));
		WRITE_STRING(text);
		MESSAGE_END();
	}

	// print to the sending client
	MESSAGE_BEGIN(MSG_ONE, gmsgSayText, NULL, &pEntity->v);
	WRITE_BYTE(ENTINDEX(pEntity));
	WRITE_STRING(text);
	MESSAGE_END();

	// echo to server console
	g_engfuncs.pfnServerPrint(text);

	const char* temp;
	if (teamonly)
		temp = "say_team";
	else
		temp = "say";

	// team match?
	if (g_teamplay)
	{
		UTIL_LogPrintf("\"%s<%i><%s><%s>\" %s \"%s\"\n",
			STRING(pEntity->v.netname),
			GETPLAYERUSERID(pEntity),
			GETPLAYERAUTHID(pEntity),
			g_engfuncs.pfnInfoKeyValue(g_engfuncs.pfnGetInfoKeyBuffer(pEntity), "model"),
			temp,
			p);
	}
	else
	{
		UTIL_LogPrintf("\"%s<%i><%s><%i>\" %s \"%s\"\n",
			STRING(pEntity->v.netname),
			GETPLAYERUSERID(pEntity),
			GETPLAYERAUTHID(pEntity),
			GETPLAYERUSERID(pEntity),
			temp,
			p);
	}
}

// Use CMD_ARGV,  CMD_ARGV, and CMD_ARGC to get pointers the character string command.
void ClientCommand(edict_t* pEntity)
{
	const char* pcmd = CMD_ARGV(0);
	const char* pstr;

	// Is the client spawned yet?
	if (!pEntity->pvPrivateData)
		return;

	entvars_t* pev = &pEntity->v;

	auto player = GetClassPtr<CBasePlayer>(reinterpret_cast<CBasePlayer*>(&pEntity->v));

	if (FStrEq(pcmd, "VModEnable")) // LRC - shut up about VModEnable...
	{
		return;
	}
	else if (FStrEq(pcmd, "hud_color")) // LRC
	{
		if (CMD_ARGC() == 4)
		{
			int col = (atoi(CMD_ARGV(1)) & 255) << 16;
			col += (atoi(CMD_ARGV(2)) & 255) << 8;
			col += (atoi(CMD_ARGV(3)) & 255);
			MESSAGE_BEGIN(MSG_ONE, gmsgHUDColor, NULL, &pEntity->v);
			WRITE_LONG(col);
			MESSAGE_END();
		}
		else
		{
			ALERT(at_console, "Syntax: hud_color RRR GGG BBB\n");
		}
	}
	else if (FStrEq(pcmd, "fire")) // LRC - trigger entities manually
	{
		if (g_psv_cheats->value)
		{
			CBaseEntity* pPlayer = CBaseEntity::Instance(pEntity);
			if (CMD_ARGC() > 1)
			{
				FireTargets(CMD_ARGV(1), pPlayer, pPlayer, USE_TOGGLE, 0);
			}
			else
			{
				TraceResult tr;
				UTIL_MakeVectors(pev->v_angle);
				UTIL_TraceLine(
					pev->origin + pev->view_ofs,
					pev->origin + pev->view_ofs + gpGlobals->v_forward * 1000,
					dont_ignore_monsters, pEntity, &tr);

				if (tr.pHit)
				{
					CBaseEntity* pHitEnt = CBaseEntity::Instance(tr.pHit);
					if (pHitEnt)
					{
						pHitEnt->Use(pPlayer, pPlayer, USE_TOGGLE, 0);
						ClientPrint(&pEntity->v, HUD_PRINTCONSOLE, UTIL_VarArgs("Fired %s \"%s\"\n", STRING(pHitEnt->pev->classname), STRING(pHitEnt->pev->targetname)));
					}
				}
			}
		}
	}
	else if (FStrEq(pcmd, "say"))
	{
		Host_Say(pEntity, false);
	}
	else if (FStrEq(pcmd, "say_team"))
	{
		Host_Say(pEntity, true);
	}
	else if (FStrEq(pcmd, "fullupdate"))
	{
		player->ForceClientDllUpdate();
	}
	else if (FStrEq(pcmd, "playaudio")) // AJH - MP3/OGG player (based on killars MP3)
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgPlayMP3, NULL, ENT(pev));
		WRITE_STRING((char*)CMD_ARGV(1));
		MESSAGE_END();
	}
	else if (FStrEq(pcmd, "inventory")) // AJH - Inventory system
	{
		CBasePlayer* pPlayer = (CBasePlayer*)CBaseEntity::Instance(pEntity);
		if (CMD_ARGC() > 1)
		{
			if (FStrEq(CMD_ARGV(1), "1"))
			{
				//	ALERT(at_debug,"DEBUG: calling medkit::use()\n");
				GetClassPtr((CItemMedicalKit*)NULL)->Use(pPlayer, pPlayer, USE_TOGGLE, 0);
			}
			else if (FStrEq(CMD_ARGV(1), "2"))
			{
				//	ALERT(at_debug,"DEBUG: calling antitox::use()\n");
				GetClassPtr((CItemAntidote*)NULL)->Use(pPlayer, pPlayer, USE_TOGGLE, 0);
			}
			else if (FStrEq(CMD_ARGV(1), "3"))
			{
				//	ALERT(at_debug,"DEBUG: calling antirad::use()\n");
				GetClassPtr((CItemAntiRad*)NULL)->Use(pPlayer, pPlayer, USE_TOGGLE, 0);
			}
			else if (FStrEq(CMD_ARGV(1), "6"))
			{
				//	ALERT(at_debug,"DEBUG: calling flare::use()\n");
				GetClassPtr((CItemFlare*)NULL)->Use(pPlayer, pPlayer, USE_TOGGLE, 0);
			}
			else if (FStrEq(CMD_ARGV(1), "7"))
			{
				if (pPlayer->m_pItemCamera != NULL)
				{

					if (CMD_ARGC() > 2)				  // If we have a specific usetype command
					{								  // (possibly consider letting the player jump between active cameras using a third parameter)
						if (FStrEq(CMD_ARGV(2), "2")) // View from Camera
						{
							pPlayer->m_pItemCamera->Use(pPlayer, pPlayer, USE_SET, 2);
						}
						else if (FStrEq(CMD_ARGV(2), "3")) // Back to normal view (don't delete camera)
						{
							pPlayer->m_pItemCamera->Use(pPlayer, pPlayer, USE_SET, 3);
						}
						else if (FStrEq(CMD_ARGV(2), "0")) // Back to normal view (delete camera)
						{
							pPlayer->m_pItemCamera->Use(pPlayer, pPlayer, USE_SET, 0);
						}
						//	else if (FStrEq(CMD_ARGV(2),"1")) //Move camera to current position (uncomment if you want to use this)
						//	{
						//		pPlayer->m_pItemCamera->Use(pPlayer,pPlayer,USE_SET,1);
						//	}
					}
					else
						pPlayer->m_pItemCamera->Use(pPlayer, pPlayer, USE_TOGGLE, 0);
				}
				else
					ALERT(at_console, "You must have a camera in your inventory before you can use one!\n");
			}
			else if (FStrEq(CMD_ARGV(1), "8"))
			{
				ALERT(at_console, "Note: This item (adrenaline syringe) is still to be implemented \n");
			}
			else if (FStrEq(CMD_ARGV(1), "9"))
			{
				ALERT(at_console, "Note: This item (site to site transporter) is still to be implemented \n");
			}
			else if (FStrEq(CMD_ARGV(1), "10"))
			{
				ALERT(at_console, "Note: This item (Lazarus stealth shield) is still to be implemented \n");
			}
			else
			{
				ALERT(at_debug, "DEBUG: Inventory item %s cannot be manually used.\n", CMD_ARGV(1));
			}
		}
		else
			ALERT(at_console, "Usage: inventory <itemnumber>\nItems are:\n\t1: Portable Medkit (Manual)\n2: AntiTox syringe (Automatic)\n3: AntiRad syringe (Automatic)\n7: Remote camera\n");
	}
	else if (FStrEq(pcmd, "give"))
	{
		if (0 != g_psv_cheats->value)
		{
			int iszItem = ALLOC_STRING(CMD_ARGV(1)); // Make a copy of the classname
			player->GiveNamedItem(STRING(iszItem));
		}
	}

	else if (FStrEq(pcmd, "drop"))
	{
		// player is dropping an item.
		player->DropPlayerItem((char*)CMD_ARGV(1));
	}
	else if (FStrEq(pcmd, "fov"))
	{
		if (0 != g_psv_cheats->value && CMD_ARGC() > 1)
		{
			player->m_iFOV = atoi(CMD_ARGV(1));
		}
		else
		{
			CLIENT_PRINTF(pEntity, print_console, UTIL_VarArgs("\"fov\" is \"%d\"\n", (int)player->m_iFOV));
		}
	}
	else if (FStrEq(pcmd, "use"))
	{
		player->SelectItem((char*)CMD_ARGV(1));
	}
	else if (((pstr = strstr(pcmd, "weapon_")) != NULL) && (pstr == pcmd))
	{
		player->SelectItem(pcmd);
	}
	else if (FStrEq(pcmd, "lastinv"))
	{
		player->SelectLastItem();
	}
	else if (FStrEq(pcmd, "spectate")) // clients wants to become a spectator
	{
		// always allow proxies to become a spectator
		if ((pev->flags & FL_PROXY) != 0 || 0 != allow_spectators.value)
		{
			edict_t* pentSpawnSpot = g_pGameRules->GetPlayerSpawnSpot(player);
			player->StartObserver(pev->origin, VARS(pentSpawnSpot)->angles);

			// notify other clients of player switching to spectator mode
			UTIL_ClientPrintAll(HUD_PRINTNOTIFY, UTIL_VarArgs("%s switched to spectator mode\n",
													 (!FStringNull(pev->netname) && STRING(pev->netname)[0] != 0) ? STRING(pev->netname) : "unconnected"));
		}
		else
			ClientPrint(pev, HUD_PRINTCONSOLE, "Spectator mode is disabled.\n");
	}
	else if (FStrEq(pcmd, "specmode")) // new spectator mode
	{
		if (player->IsObserver())
			player->Observer_SetMode(atoi(CMD_ARGV(1)));
	}
	else if (FStrEq(pcmd, "closemenus"))
	{
		// just ignore it
	}
	else if (FStrEq(pcmd, "follownext")) // follow next player
	{
		if (player->IsObserver())
			player->Observer_FindNextPlayer(atoi(CMD_ARGV(1)) != 0);
	}
	else if (g_pGameRules->ClientCommand(player, pcmd))
	{
		// MenuSelect returns true only if the command is properly handled,  so don't print a warning
	}
	else
	{
		// tell the user they entered an unknown command
		char command[128];

		// check the length of the command (prevents crash)
		// max total length is 192 ...and we're adding a string below ("Unknown command: %s\n")
		strncpy(command, pcmd, 127);
		command[127] = '\0';

		// tell the user they entered an unknown command
		ClientPrint(&pEntity->v, HUD_PRINTCONSOLE, UTIL_VarArgs("Unknown command: %s\n", command));
	}
}

/*
========================
ClientUserInfoChanged

called after the player changes
userinfo - gives dll a chance to modify it before
it gets sent into the rest of the engine.
========================
*/
void ClientUserInfoChanged(edict_t* pEntity, char* infobuffer)
{
	// Is the client spawned yet?
	if (!pEntity->pvPrivateData)
		return;

	// msg everyone if someone changes their name,  and it isn't the first time (changing no name to current name)
	if (!FStringNull(pEntity->v.netname) && STRING(pEntity->v.netname)[0] != 0 && !FStrEq(STRING(pEntity->v.netname), g_engfuncs.pfnInfoKeyValue(infobuffer, "name")))
	{
		char sName[256];
		char* pName = g_engfuncs.pfnInfoKeyValue(infobuffer, "name");
		strncpy(sName, pName, sizeof(sName) - 1);
		sName[sizeof(sName) - 1] = '\0';

		// First parse the name and remove any %'s
		for (char* pApersand = sName; pApersand != NULL && *pApersand != 0; pApersand++)
		{
			// Replace it with a space
			if (*pApersand == '%')
				*pApersand = ' ';
		}

		// Set the name
		g_engfuncs.pfnSetClientKeyValue(ENTINDEX(pEntity), infobuffer, "name", sName);

		if (gpGlobals->maxClients > 1)
		{
			char text[256];
			sprintf(text, "* %s changed name to %s\n", STRING(pEntity->v.netname), g_engfuncs.pfnInfoKeyValue(infobuffer, "name"));
			MESSAGE_BEGIN(MSG_ALL, gmsgSayText, NULL);
			WRITE_BYTE(ENTINDEX(pEntity));
			WRITE_STRING(text);
			MESSAGE_END();
		}

		// team match?
		if (g_teamplay)
		{
			UTIL_LogPrintf("\"%s<%i><%s><%s>\" changed name to \"%s\"\n",
				STRING(pEntity->v.netname),
				GETPLAYERUSERID(pEntity),
				GETPLAYERAUTHID(pEntity),
				g_engfuncs.pfnInfoKeyValue(infobuffer, "model"),
				g_engfuncs.pfnInfoKeyValue(infobuffer, "name"));
		}
		else
		{
			UTIL_LogPrintf("\"%s<%i><%s><%i>\" changed name to \"%s\"\n",
				STRING(pEntity->v.netname),
				GETPLAYERUSERID(pEntity),
				GETPLAYERAUTHID(pEntity),
				GETPLAYERUSERID(pEntity),
				g_engfuncs.pfnInfoKeyValue(infobuffer, "name"));
		}
	}

	g_pGameRules->ClientUserInfoChanged(GetClassPtr((CBasePlayer*)&pEntity->v), infobuffer);
}
