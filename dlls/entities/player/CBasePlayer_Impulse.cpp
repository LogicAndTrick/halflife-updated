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

#include "CBasePlayer.h"
#include "CBloodSplat.h"
#include "CSprayCan.h"
#include "game.h"
#include "classes/nodes/CGraph.h"
#include "pm_shared.h"
#include "UserMessages.h"
#include "entities/CGib.h"
#include "entities/CWorld.h"

void CBasePlayer::ImpulseCommands()
{
	TraceResult tr; // UNDONE: kill me! This is temporary for PreAlpha CDs

	// Handle use events
	PlayerUse();

	int iImpulse = (int)pev->impulse;
	switch (iImpulse)
	{
	case 99:
	{

		bool iOn;

		if (0 == gmsgLogo)
		{
			iOn = true;
			gmsgLogo = REG_USER_MSG("Logo", 1);
		}
		else
		{
			iOn = false;
		}

		ASSERT(gmsgLogo > 0);
		// send "health" update message
		MESSAGE_BEGIN(MSG_ONE, gmsgLogo, NULL, pev);
		WRITE_BYTE(static_cast<int>(iOn));
		MESSAGE_END();

		if (!iOn)
			gmsgLogo = 0;
		break;
	}
	case 100:
		// temporary flashlight for level designers
		if (FlashlightIsOn())
		{
			FlashlightTurnOff();
		}
		else
		{
			FlashlightTurnOn();
		}
		break;

	case 201: // paint decal

		if (gpGlobals->time < m_flNextDecalTime)
		{
			// too early!
			break;
		}

		UTIL_MakeVectors(pev->v_angle);
		UTIL_TraceLine(pev->origin + pev->view_ofs, pev->origin + pev->view_ofs + gpGlobals->v_forward * 128, ignore_monsters, ENT(pev), &tr);

		if (tr.flFraction != 1.0)
		{ // line hit something, so paint a decal
			m_flNextDecalTime = gpGlobals->time + decalfrequency.value;
			CSprayCan* pCan = GetClassPtr((CSprayCan*)NULL);
			pCan->Spawn(pev);
		}

		break;

	default:
		// check all of the cheat impulse commands now
		CheatImpulseCommands(iImpulse);
		break;
	}

	pev->impulse = 0;
}

void CBasePlayer::CheatImpulseCommands(int iImpulse)
{
	if (0 == g_psv_cheats->value)
	{
		return;
	}

	CBaseEntity* pEntity;
	TraceResult tr;

	switch (iImpulse)
	{
	case 76:
	{
		if (!giPrecacheGrunt)
		{
			giPrecacheGrunt = true;
			ALERT(at_debug, "You must now restart to use Grunt-o-matic.\n");
		}
		else
		{
			UTIL_MakeVectors(Vector(0, pev->v_angle.y, 0));
			Create("monster_human_grunt", pev->origin + gpGlobals->v_forward * 128, pev->angles);
		}
		break;
	}

	case 90: // LRC - send USE_TOGGLE
	{
		char* impulsetarget = (char*)CVAR_GET_STRING("sohl_impulsetarget");
		if (impulsetarget)
			FireTargets(impulsetarget, this, this, USE_TOGGLE, 0);
		break;
	}
	case 91: // LRC - send USE_ON
	{
		char* impulsetarget = (char*)CVAR_GET_STRING("sohl_impulsetarget");
		if (impulsetarget)
			FireTargets(impulsetarget, this, this, USE_ON, 0);
		break;
	}
	case 92: // LRC - send USE_OFF
	{
		char* impulsetarget = (char*)CVAR_GET_STRING("sohl_impulsetarget");
		if (impulsetarget)
			FireTargets(impulsetarget, this, this, USE_OFF, 0);
		break;
	}
	case 93: // AJH - send USE_TOGGLE
	{
		pEntity = UTIL_FindEntityForward(this);
		if (pEntity)
			pEntity->Use(this, this, USE_TOGGLE, 0);
		break;
	}
	case 94: // AJH - send USE_ON
	{
		pEntity = UTIL_FindEntityForward(this);
		if (pEntity)
			pEntity->Use(this, this, USE_ON, 0);
		break;
	}
	case 95: // AJH - send USE_OFF
	{
		pEntity = UTIL_FindEntityForward(this);
		if (pEntity)
			pEntity->Use(this, this, USE_OFF, 0);
		break;
	}
	/*	case 96: //AJH - send USE_KILL				//AJH this doesn't work due to directly calling
		{											//the target entities use function instead of
			pEntity = FindEntityForward( this );	//calling FireTargets.
			if (pEntity)
				pEntity->Use( this, this, USE_KILL, 0);
			break;
		}
	*/
	case 97: // AJH - send USE_SPAWN
	{
		pEntity = UTIL_FindEntityForward(this);
		if (pEntity)
			pEntity->Use(this, this, USE_SPAWN, 0);
		break;
	}


	case 101:
		gEvilImpulse101 = true;
		GiveNamedItem("item_suit");
		GiveNamedItem("item_battery");
		GiveNamedItem("weapon_crowbar");
		GiveNamedItem("weapon_9mmhandgun");
		GiveNamedItem("ammo_9mmclip");
		GiveNamedItem("weapon_shotgun");
		GiveNamedItem("ammo_buckshot");
		GiveNamedItem("weapon_9mmAR");
		GiveNamedItem("ammo_9mmAR");
		GiveNamedItem("ammo_ARgrenades");
		GiveNamedItem("weapon_handgrenade");
		GiveNamedItem("weapon_tripmine");
		GiveNamedItem("weapon_357");
		GiveNamedItem("ammo_357");
		GiveNamedItem("weapon_crossbow");
		GiveNamedItem("ammo_crossbow");
		GiveNamedItem("weapon_egon");
		GiveNamedItem("weapon_gauss");
		GiveNamedItem("ammo_gaussclip");
		GiveNamedItem("weapon_rpg");
		GiveNamedItem("ammo_rpgclip");
		GiveNamedItem("weapon_satchel");
		GiveNamedItem("weapon_snark");
		GiveNamedItem("weapon_hornetgun");
		GiveNamedItem("item_longjump");
		gEvilImpulse101 = false;
		break;

	case 102:
		// Gibbage!!!
		CGib::SpawnRandomGibs(pev, 1, true);
		break;

	case 103:
		// What the hell are you doing?
		pEntity = UTIL_FindEntityForward(this);
		if (pEntity)
		{
			CBaseMonster* pMonster = pEntity->MyMonsterPointer();
			if (pMonster)
				pMonster->ReportAIState();
		}
		break;

	case 104:
		// Dump all of the global state varaibles (and global entity names)
		gGlobalState.DumpGlobals();
		break;

	case 105: // player makes no sound for monsters to hear.
	{
		if (m_fNoPlayerSound)
		{
			ALERT(at_debug, "Player is audible\n");
			m_fNoPlayerSound = false;
		}
		else
		{
			ALERT(at_debug, "Player is silent\n");
			m_fNoPlayerSound = true;
		}
		break;
	}

	case 106:
		// Give me the classname and targetname of this entity.
		pEntity = UTIL_FindEntityForward(this);
		if (pEntity)
		{
			ALERT(at_debug, "Classname: %s", STRING(pEntity->pev->classname));

			if (!FStringNull(pEntity->pev->targetname))
			{
				ALERT(at_debug, " - Targetname: %s\n", STRING(pEntity->pev->targetname));
			}
			else
			{
				ALERT(at_debug, " - TargetName: No Targetname\n");
			}

			ALERT(at_debug, "Model: %s\n", STRING(pEntity->pev->model));
			if (!FStringNull(pEntity->pev->globalname))
				ALERT(at_debug, "Globalname: %s\n", STRING(pEntity->pev->globalname));
			ALERT(at_debug, "State: %s\n", GetStringForState(pEntity->GetState())); // LRC
			ALERT(at_debug, "pev->model: %s, pev->rendermode: %d\n", STRING(pEntity->pev->model), pEntity->pev->rendermode);
			ALERT(at_debug, "pev->renderfx: %d, pev->rendercolor: %3f %3f %3f\n", pEntity->pev->renderfx, pEntity->pev->rendercolor.x, pEntity->pev->rendercolor.y, pEntity->pev->rendercolor.z);
			ALERT(at_debug, "pev->renderamt: %3f, pev->health: %3f\n", pEntity->pev->renderamt, pEntity->pev->health);
		}
		break;

	case 107:
	{
		TraceResult tr;

		edict_t* pWorld = CWorld::World->edict();

		Vector start = pev->origin + pev->view_ofs;
		Vector end = start + gpGlobals->v_forward * 1024;
		UTIL_TraceLine(start, end, ignore_monsters, edict(), &tr);
		if (tr.pHit)
			pWorld = tr.pHit;
		const char* pTextureName = TRACE_TEXTURE(pWorld, start, end);
		if (pTextureName)
			ALERT(at_debug, "Texture: %s (%c)\n", pTextureName, PM_FindTextureType(pTextureName));
	}
	break;
	case 195: // show shortest paths for entire level to nearest node
	{
		Create("node_viewer_fly", pev->origin, pev->angles);
	}
	break;
	case 196: // show shortest paths for entire level to nearest node
	{
		Create("node_viewer_large", pev->origin, pev->angles);
	}
	break;
	case 197: // show shortest paths for entire level to nearest node
	{
		Create("node_viewer_human", pev->origin, pev->angles);
	}
	break;
	case 199: // show nearest node and all connections
	{
		ALERT(at_debug, "%d\n", WorldGraph.FindNearestNode(pev->origin, bits_NODE_GROUP_REALM));
		WorldGraph.ShowNodeConnections(WorldGraph.FindNearestNode(pev->origin, bits_NODE_GROUP_REALM));
	}
	break;
	case 202: // Random blood splatter
		UTIL_MakeVectors(pev->v_angle);
		UTIL_TraceLine(pev->origin + pev->view_ofs, pev->origin + pev->view_ofs + gpGlobals->v_forward * 128, ignore_monsters, ENT(pev), &tr);

		if (tr.flFraction != 1.0)
		{ // line hit something, so paint a decal
			CBloodSplat* pBlood = GetClassPtr((CBloodSplat*)NULL);
			pBlood->Spawn(pev);
		}
		break;
	case 203: // remove creature.
		pEntity = UTIL_FindEntityForward(this);
		if (pEntity)
		{
			if (0 != pEntity->pev->takedamage)
				pEntity->SetThink(&CBaseEntity::SUB_Remove);
		}
		break;
	}
}
