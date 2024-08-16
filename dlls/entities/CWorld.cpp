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

#include "CWorld.h"
#include "CCorpse.h"
#include "entities/env/CMessage.h"
#include "classes/nodes/CGraph.h"
#include "entities/sound/CSoundEnt.h"
#include "client.h"
#include "decals.h"
#include "entities/player/CBasePlayer.h"
#include "gamerules.h"
#include "weapon/CBasePlayerItem.h"
#include "weapon/CBasePlayerWeapon.h"

bool g_startSuit; // LRC

bool g_allowGJump;

LINK_ENTITY_TO_CLASS(worldspawn, CWorld);

// This must match the list in util.h
DLL_DECALLIST gDecals[] = {
	{"{shot1", 0},	   // DECAL_GUNSHOT1
	{"{shot2", 0},	   // DECAL_GUNSHOT2
	{"{shot3", 0},	   // DECAL_GUNSHOT3
	{"{shot4", 0},	   // DECAL_GUNSHOT4
	{"{shot5", 0},	   // DECAL_GUNSHOT5
	{"{lambda01", 0},  // DECAL_LAMBDA1
	{"{lambda02", 0},  // DECAL_LAMBDA2
	{"{lambda03", 0},  // DECAL_LAMBDA3
	{"{lambda04", 0},  // DECAL_LAMBDA4
	{"{lambda05", 0},  // DECAL_LAMBDA5
	{"{lambda06", 0},  // DECAL_LAMBDA6
	{"{scorch1", 0},   // DECAL_SCORCH1
	{"{scorch2", 0},   // DECAL_SCORCH2
	{"{blood1", 0},	   // DECAL_BLOOD1
	{"{blood2", 0},	   // DECAL_BLOOD2
	{"{blood3", 0},	   // DECAL_BLOOD3
	{"{blood4", 0},	   // DECAL_BLOOD4
	{"{blood5", 0},	   // DECAL_BLOOD5
	{"{blood6", 0},	   // DECAL_BLOOD6
	{"{yblood1", 0},   // DECAL_YBLOOD1
	{"{yblood2", 0},   // DECAL_YBLOOD2
	{"{yblood3", 0},   // DECAL_YBLOOD3
	{"{yblood4", 0},   // DECAL_YBLOOD4
	{"{yblood5", 0},   // DECAL_YBLOOD5
	{"{yblood6", 0},   // DECAL_YBLOOD6
	{"{break1", 0},	   // DECAL_GLASSBREAK1
	{"{break2", 0},	   // DECAL_GLASSBREAK2
	{"{break3", 0},	   // DECAL_GLASSBREAK3
	{"{bigshot1", 0},  // DECAL_BIGSHOT1
	{"{bigshot2", 0},  // DECAL_BIGSHOT2
	{"{bigshot3", 0},  // DECAL_BIGSHOT3
	{"{bigshot4", 0},  // DECAL_BIGSHOT4
	{"{bigshot5", 0},  // DECAL_BIGSHOT5
	{"{spit1", 0},	   // DECAL_SPIT1
	{"{spit2", 0},	   // DECAL_SPIT2
	{"{bproof1", 0},   // DECAL_BPROOF1
	{"{gargstomp", 0}, // DECAL_GARGSTOMP1,	// Gargantua stomp crack
	{"{smscorch1", 0}, // DECAL_SMALLSCORCH1,	// Small scorch mark
	{"{smscorch2", 0}, // DECAL_SMALLSCORCH2,	// Small scorch mark
	{"{smscorch3", 0}, // DECAL_SMALLSCORCH3,	// Small scorch mark
	{"{mommablob", 0}, // DECAL_MOMMABIRTH		// BM Birth spray
	{"{mommablob", 0}, // DECAL_MOMMASPLAT		// BM Mortar spray?? need decal
};

//
// Just to ignore the "wad" field.
//
bool CWorld::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "skyname"))
	{
		// Sent over net now.
		CVAR_SET_STRING("sv_skyname", pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "sounds"))
	{
		gpGlobals->cdAudioTrack = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "WaveHeight"))
	{
		// Sent over net now.
		pev->scale = atof(pkvd->szValue) * (1.0 / 8.0);
		CVAR_SET_FLOAT("sv_wateramp", pev->scale);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "MaxRange"))
	{
		pev->speed = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "chaptertitle"))
	{
		pev->netname = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "startdark"))
	{
		// UNDONE: This is a gross hack!!! The CVAR is NOT sent over the client/sever link
		// but it will work for single player
		int flag = atoi(pkvd->szValue);
		if (0 != flag)
			pev->spawnflags |= SF_WORLD_DARK;
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "newunit"))
	{
		// Single player only.  Clear save directory if set
		if (0 != atoi(pkvd->szValue))
			CVAR_SET_FLOAT("sv_newunit", 1);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "gametitle"))
	{
		if (0 != atoi(pkvd->szValue))
			pev->spawnflags |= SF_WORLD_TITLE;

		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "mapteams"))
	{
		pev->team = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "defaultteam"))
	{
		if (0 != atoi(pkvd->szValue))
		{
			pev->spawnflags |= SF_WORLD_FORCETEAM;
		}
		return true;
	}
	// LRC- let map designers start the player with his suit already on
	else if (FStrEq(pkvd->szKeyName, "startsuit"))
	{
		g_startSuit = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "allowmonsters"))
	{
		CVAR_SET_FLOAT("mp_allowmonsters", atof(pkvd->szValue));
		return true;
	}
	// LRC- ends

	// AJH- Gauss Jump in single play
	else if (FStrEq(pkvd->szKeyName, "allow_sp_gjump"))
	{
		g_allowGJump = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "timed_damage"))
	{
		CVAR_SET_FLOAT("timed_damage", atof(pkvd->szValue));
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "max_medkit"))
	{
		CVAR_SET_FLOAT("max_medkit", atof(pkvd->szValue));
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "max_cameras"))
	{
		CVAR_SET_FLOAT("max_cameras", atof(pkvd->szValue));
		return true;
	}

	return CBaseEntity::KeyValue(pkvd);
}

CWorld::CWorld()
{
	if (World)
	{
		ALERT(at_error, "Do not create multiple instances of worldspawn\n");
		return;
	}

	World = this;
}

CWorld::~CWorld()
{
	if (World != this)
	{
		return;
	}

	World = nullptr;
}

void CWorld::Spawn()
{
	g_fGameOver = false;
	Precache();
}

void CWorld::Precache()
{
	// Flag this entity for removal if it's not the actual world entity.
	if (World != this)
	{
		UTIL_Remove(this);
		return;
	}

	// LRC - set up the world lists
	g_pWorld = this;
	m_pAssistLink = NULL;
	m_pFirstAlias = NULL;
	//	ALERT(at_console, "Clearing AssistList\n");

	g_pLastSpawn = NULL;

#if 1
	CVAR_SET_STRING("sv_gravity", "800"); // 67ft/sec
	CVAR_SET_STRING("sv_stepsize", "18");
#else
	CVAR_SET_STRING("sv_gravity", "384"); // 32ft/sec
	CVAR_SET_STRING("sv_stepsize", "24");
#endif

	CVAR_SET_STRING("room_type", "0"); // clear DSP

	// Set up game rules
	delete g_pGameRules;

	g_pGameRules = InstallGameRules();

	//!!!UNDONE why is there so much Spawn code in the Precache function? I'll just keep it here

	/*	if ( WorldGraph.m_fGraphPresent && !WorldGraph.m_fGraphPointersSet )
	{
		if ( !WorldGraph.FSetGraphPointers() )
		{
			ALERT ( at_debug, "**Graph pointers were not set!\n");
		}
		else
		{
			ALERT ( at_debug, "**Graph Pointers Set!\n" );
		}
	}*/

	///!!!LATER - do we want a sound ent in deathmatch? (sjb)
	// pSoundEnt = CBaseEntity::Create( "soundent", g_vecZero, g_vecZero, edict() );
	pSoundEnt = GetClassPtr((CSoundEnt*)NULL);
	pSoundEnt->Spawn();

	if (!pSoundEnt)
	{
		ALERT(at_debug, "**COULD NOT CREATE SOUNDENT**\n");
	}

	InitBodyQue();

	// init sentence group playback stuff from sentences.txt.
	// ok to call this multiple times, calls after first are ignored.

	SENTENCEG_Init();

	// init texture type array from materials.txt

	TEXTURETYPE_Init();


	// the area based ambient sounds MUST be the first precache_sounds

	// player precaches
	Weapon_Precache(); // get weapon precaches
	Item_Precache(); // get Inventory Item precaches

	ClientPrecache();

	// sounds used from C physics code
	PRECACHE_SOUND("common/null.wav"); // clears sound channels

	PRECACHE_SOUND("items/suitchargeok1.wav"); //!!! temporary sound for respawning weapons.
	PRECACHE_SOUND("items/gunpickup2.wav");	   // player picks up a gun.

	PRECACHE_SOUND("common/bodydrop3.wav"); // dead bodies hitting the ground (animation events)
	PRECACHE_SOUND("common/bodydrop4.wav");

	g_Language = (int)CVAR_GET_FLOAT("sv_language");
	if (g_Language == LANGUAGE_GERMAN)
	{
		PRECACHE_MODEL("models/germangibs.mdl");
	}
	else
	{
		PRECACHE_MODEL("models/hgibs.mdl");
		PRECACHE_MODEL("models/agibs.mdl");
	}

	PRECACHE_SOUND("weapons/ric1.wav");
	PRECACHE_SOUND("weapons/ric2.wav");
	PRECACHE_SOUND("weapons/ric3.wav");
	PRECACHE_SOUND("weapons/ric4.wav");
	PRECACHE_SOUND("weapons/ric5.wav");

	PRECACHE_MODEL("sprites/null.spr"); // LRC

	//
	// Setup light animation tables. 'a' is total darkness, 'z' is maxbright.
	//
	int i;

	// 0 normal
	for (i = 0; i <= 13; i++)
	{
		LIGHT_STYLE(i, (char*)STRING(GetStdLightStyle(i)));
	}

	// styles 32-62 are assigned by the light program for switchable lights

	// 63 testing
	LIGHT_STYLE(63, "a");

	for (i = 0; i < ARRAYSIZE(gDecals); i++)
		gDecals[i].index = DECAL_INDEX(gDecals[i].name);

	// init the WorldGraph.
	WorldGraph.InitGraph();

	// make sure the .NOD file is newer than the .BSP file.
	if (!WorldGraph.CheckNODFile((char*)STRING(gpGlobals->mapname)))
	{ // NOD file is not present, or is older than the BSP file.
		WorldGraph.AllocNodes();
	}
	else
	{ // Load the node graph for this level
		if (!WorldGraph.FLoadGraph((char*)STRING(gpGlobals->mapname)))
		{ // couldn't load, so alloc and prepare to build a graph.
			ALERT(at_debug, "*Error opening .NOD file\n");
			WorldGraph.AllocNodes();
		}
		else
		{
			ALERT(at_debug, "\n*Graph Loaded!\n");
		}
	}

	if (pev->speed > 0)
		CVAR_SET_FLOAT("sv_zmax", pev->speed);
	else
		CVAR_SET_FLOAT("sv_zmax", 4096);

	if (!FStringNull(pev->netname))
	{
		ALERT(at_aiconsole, "Chapter title: %s\n", STRING(pev->netname));
		CBaseEntity* pEntity = CBaseEntity::Create("env_message", g_vecZero, g_vecZero, NULL);
		if (pEntity)
		{
			pEntity->SetThink(&CBaseEntity::SUB_CallUseToggle);
			pEntity->pev->message = pev->netname;
			pev->netname = 0;
			pEntity->SetNextThink(0.3);
			pEntity->pev->spawnflags = SF_MESSAGE_ONCE;
		}
	}

	if ((pev->spawnflags & SF_WORLD_DARK) != 0)
		CVAR_SET_FLOAT("v_dark", 1.0);
	else
		CVAR_SET_FLOAT("v_dark", 0.0);

	gDisplayTitle = (pev->spawnflags & SF_WORLD_TITLE) != 0;

	if ((pev->spawnflags & SF_WORLD_FORCETEAM) != 0)
	{
		CVAR_SET_FLOAT("mp_defaultteam", 1);
	}
	else
	{
		CVAR_SET_FLOAT("mp_defaultteam", 0);
	}
}
