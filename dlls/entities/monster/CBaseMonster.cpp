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

#include "CBaseMonster.h"
#include "entities/CGib.h"
#include "classes/gamerules/CGameRules.h"
#include "classes/nodes/CGraph.h"

//=========================================================
// KeyValue
//
// !!! netname entvar field is used in squadmonster for groupname!!!
//=========================================================
bool CBaseMonster::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "TriggerTarget"))
	{
		m_iszTriggerTarget = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "TriggerCondition"))
	{
		m_iTriggerCondition = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "allow_item_dropping"))
	{
		m_AllowItemDropping = atoi(pkvd->szValue) != 0;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iClass")) // LRC
	{
		m_iClass = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iPlayerReact")) // LRC
	{
		m_iPlayerReact = atoi(pkvd->szValue);
		return true;
	}

	return CBaseToggle::KeyValue(pkvd);
}

//=========================================================
// MonsterInit - after a monster is spawned, it needs to
// be dropped into the world, checked for mobility problems,
// and put on the proper path, if any. This function does
// all of those things after the monster spawns. Any
// initialization that should take place for all monsters
// goes here.
//=========================================================
void CBaseMonster::MonsterInit()
{
	if (!g_pGameRules->FAllowMonsters())
	{
		pev->flags |= FL_KILLME; // Post this because some monster code modifies class data after calling this function
								 //		REMOVE_ENTITY(ENT(pev));
		return;
	}

	// Set fields common to all monsters
	pev->effects = 0;
	pev->takedamage = DAMAGE_AIM;
	pev->ideal_yaw = pev->angles.y;
	pev->max_health = pev->health;
	pev->deadflag = DEAD_NO;
	m_IdealMonsterState = MONSTERSTATE_IDLE; // Assume monster will be idle, until proven otherwise

	m_IdealActivity = ACT_IDLE;

	SetBits(pev->flags, FL_MONSTER);
	if ((pev->spawnflags & SF_MONSTER_HITMONSTERCLIP) != 0)
		pev->flags |= FL_MONSTERCLIP;

	ClearSchedule();
	RouteClear();
	InitBoneControllers(); // FIX: should be done in Spawn

	m_iHintNode = NO_NODE;

	m_afMemory = MEMORY_CLEAR;

	m_hEnemy = NULL;

	m_flDistTooFar = 1024.0;
	m_flDistLook = 2048.0;

	// set eye position
	SetEyePosition();

	SetThink(&CBaseMonster::MonsterInitThink);
	SetNextThink(0.1);
	SetUse(&CBaseMonster::MonsterUse);
}

//=========================================================
// MonsterInitThink - Calls StartMonster. Startmonster is
// virtual, but this function cannot be
//=========================================================
void CBaseMonster::MonsterInitThink()
{
	StartMonster();
}

//=========================================================
// StartMonster - final bit of initization before a monster
// is turned over to the AI.
//=========================================================
void CBaseMonster::StartMonster()
{
	// update capabilities
	if (LookupActivity(ACT_RANGE_ATTACK1) != ACTIVITY_NOT_AVAILABLE)
	{
		m_afCapability |= bits_CAP_RANGE_ATTACK1;
	}
	if (LookupActivity(ACT_RANGE_ATTACK2) != ACTIVITY_NOT_AVAILABLE)
	{
		m_afCapability |= bits_CAP_RANGE_ATTACK2;
	}
	if (LookupActivity(ACT_MELEE_ATTACK1) != ACTIVITY_NOT_AVAILABLE)
	{
		m_afCapability |= bits_CAP_MELEE_ATTACK1;
	}
	if (LookupActivity(ACT_MELEE_ATTACK2) != ACTIVITY_NOT_AVAILABLE)
	{
		m_afCapability |= bits_CAP_MELEE_ATTACK2;
	}

	// Raise monster off the floor one unit, then drop to floor
	if (pev->movetype != MOVETYPE_FLY && !FBitSet(pev->spawnflags, SF_MONSTER_FALL_TO_GROUND))
	{
		pev->origin.z += 1;
		DROP_TO_FLOOR(ENT(pev));
		// Try to move the monster to make sure it's not stuck in a brush.
		// LRC- there are perfectly good reasons for making a monster stuck, so it shouldn't always be an error.
		if (!WALK_MOVE(ENT(pev), 0, 0, WALKMOVE_NORMAL) && !FBitSet(pev->spawnflags, SF_MONSTER_NO_YELLOW_BLOBS))
		{
			ALERT(at_debug, "%s \"%s\" stuck in wall--level design error", STRING(pev->classname), STRING(pev->targetname));
			pev->effects = EF_BRIGHTFIELD;
		}
	}
	else
	{
		pev->flags &= ~FL_ONGROUND;
	}

	if (!FStringNull(pev->target)) // this monster has a target
	{
		StartPatrol(UTIL_FindEntityByTargetname(NULL, STRING(pev->target)));
	}

	// SetState ( m_IdealMonsterState );
	// SetActivity ( m_IdealActivity );

	// Delay drop to floor to make sure each door in the level has had its chance to spawn
	// Spread think times so that they don't all happen at the same time (Carmack)
	SetThink(&CBaseMonster::CallMonsterThink);
	AbsoluteNextThink(m_fNextThink + RANDOM_FLOAT(0.1, 0.4)); // spread think times.

	if (!FStringNull(pev->targetname)) // wait until triggered
	{
		SetState(MONSTERSTATE_IDLE);
		// UNDONE: Some scripted sequence monsters don't have an idle?
		SetActivity(ACT_IDLE);
		ChangeSchedule(GetScheduleOfType(SCHED_WAIT_TRIGGER));
	}
}

//=========================================================
// CBaseMonster - USE - will make a monster angry at whomever
// activated it.
//=========================================================
void CBaseMonster::MonsterUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	// Don't do this because it can resurrect dying monsters
	// m_IdealMonsterState = MONSTERSTATE_ALERT;
}
