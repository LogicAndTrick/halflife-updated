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

#pragma once

#include "entities/CBaseMonster.h"

// Monstermaker spawnflags
#define SF_MONSTERMAKER_START_ON 1		 // start active ( if has targetname )
#define SF_MONSTERMAKER_CYCLIC 4		 // drop one monster every time fired.
#define SF_MONSTERMAKER_MONSTERCLIP 8	 // Children are blocked by monsterclip
#define SF_MONSTERMAKER_LEAVECORPSE 16	 // Don't fade corpses.
#define SF_MONSTERMAKER_FORCESPAWN 32	 // AJH Force the monstermaker to spawn regardless of blocking enitites
#define SF_MONSTERMAKER_NO_WPN_DROP 1024 // Corpses don't drop weapons.

//=========================================================
// MonsterMaker - this ent creates monsters during the game.
//=========================================================
class CMonsterMaker : public CBaseMonster
{
public:
	void Spawn() override;
	void Precache() override;
	bool KeyValue(KeyValueData* pkvd) override;
	void EXPORT ToggleUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void EXPORT CyclicUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void EXPORT MakerThink();
	void EXPORT MakeMonsterThink();
	void DeathNotice(entvars_t* pevChild) override; // monster maker children use this to tell the monster maker that they have died.
	void TryMakeMonster();							// LRC - to allow for a spawndelay
	CBaseMonster* MakeMonster();					// LRC - actually make a monster (and return the new creation)

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];

	string_t m_iszMonsterClassname; // classname of the monster(s) that will be created.

	int m_cNumMonsters; // max number of monsters this ent can create


	int m_cLiveChildren;	// how many monsters made by this monster maker that are currently alive
	int m_iMaxLiveChildren; // max number of monsters that this maker may have out at one time.

	float m_flGround; // z coord of the ground under me, used to make sure no monsters are under the maker when it drops a new child

	bool m_fActive;
	bool m_fFadeChildren; // should we make the children fadeout?
	float m_fSpawnDelay;  // LRC- delay between triggering targets and making a child (for env_warpball, mainly)
};
