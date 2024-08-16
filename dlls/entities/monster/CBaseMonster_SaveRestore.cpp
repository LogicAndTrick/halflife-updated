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

// Global Savedata for monster
// UNDONE: Save schedule data?  Can this be done?  We may
// lose our enemy pointer or other data (goal ent, target, etc)
// that make the current schedule invalid, perhaps it's best
// to just pick a new one when we start up again.
TYPEDESCRIPTION CBaseMonster::m_SaveData[] =
	{
		DEFINE_FIELD(CBaseMonster, m_hEnemy, FIELD_EHANDLE),
		DEFINE_FIELD(CBaseMonster, m_hTargetEnt, FIELD_EHANDLE),
		DEFINE_ARRAY(CBaseMonster, m_hOldEnemy, FIELD_EHANDLE, MAX_OLD_ENEMIES),
		DEFINE_ARRAY(CBaseMonster, m_vecOldEnemy, FIELD_POSITION_VECTOR, MAX_OLD_ENEMIES),

		DEFINE_FIELD(CBaseMonster, m_iClass, FIELD_INTEGER),
		DEFINE_FIELD(CBaseMonster, m_iPlayerReact, FIELD_INTEGER),

		DEFINE_FIELD(CBaseMonster, m_flFieldOfView, FIELD_FLOAT),
		DEFINE_FIELD(CBaseMonster, m_flWaitFinished, FIELD_TIME),
		DEFINE_FIELD(CBaseMonster, m_flMoveWaitFinished, FIELD_TIME),

		DEFINE_FIELD(CBaseMonster, m_Activity, FIELD_INTEGER),
		DEFINE_FIELD(CBaseMonster, m_IdealActivity, FIELD_INTEGER),
		DEFINE_FIELD(CBaseMonster, m_LastHitGroup, FIELD_INTEGER),
		DEFINE_FIELD(CBaseMonster, m_MonsterState, FIELD_INTEGER),
		DEFINE_FIELD(CBaseMonster, m_IdealMonsterState, FIELD_INTEGER),
		DEFINE_FIELD(CBaseMonster, m_iTaskStatus, FIELD_INTEGER),

		// Schedule_t			*m_pSchedule;

		DEFINE_FIELD(CBaseMonster, m_iScheduleIndex, FIELD_INTEGER),
		DEFINE_FIELD(CBaseMonster, m_afConditions, FIELD_INTEGER),
		// WayPoint_t			m_Route[ ROUTE_SIZE ];
		//	DEFINE_FIELD( CBaseMonster, m_movementGoal, FIELD_INTEGER ),
		//	DEFINE_FIELD( CBaseMonster, m_iRouteIndex, FIELD_INTEGER ),
		//	DEFINE_FIELD( CBaseMonster, m_moveWaitTime, FIELD_FLOAT ),

		DEFINE_FIELD(CBaseMonster, m_vecMoveGoal, FIELD_POSITION_VECTOR),
		DEFINE_FIELD(CBaseMonster, m_movementActivity, FIELD_INTEGER),

		//		int					m_iAudibleList; // first index of a linked list of sounds that the monster can hear.
		//	DEFINE_FIELD( CBaseMonster, m_afSoundTypes, FIELD_INTEGER ),
		DEFINE_FIELD(CBaseMonster, m_vecLastPosition, FIELD_POSITION_VECTOR),
		DEFINE_FIELD(CBaseMonster, m_iHintNode, FIELD_INTEGER),
		DEFINE_FIELD(CBaseMonster, m_afMemory, FIELD_INTEGER),
		DEFINE_FIELD(CBaseMonster, m_iMaxHealth, FIELD_INTEGER),

		DEFINE_FIELD(CBaseMonster, m_vecEnemyLKP, FIELD_POSITION_VECTOR),
		DEFINE_FIELD(CBaseMonster, m_cAmmoLoaded, FIELD_INTEGER),
		DEFINE_FIELD(CBaseMonster, m_afCapability, FIELD_INTEGER),

		DEFINE_FIELD(CBaseMonster, m_flNextAttack, FIELD_TIME),
		DEFINE_FIELD(CBaseMonster, m_bitsDamageType, FIELD_INTEGER),
		DEFINE_ARRAY(CBaseMonster, m_rgbTimeBasedDamage, FIELD_CHARACTER, CDMG_TIMEBASED),
		DEFINE_FIELD(CBaseMonster, m_bloodColor, FIELD_INTEGER),
		DEFINE_FIELD(CBaseMonster, m_failSchedule, FIELD_INTEGER),

		DEFINE_FIELD(CBaseMonster, m_flHungryTime, FIELD_TIME),
		DEFINE_FIELD(CBaseMonster, m_flDistTooFar, FIELD_FLOAT),
		DEFINE_FIELD(CBaseMonster, m_flDistLook, FIELD_FLOAT),
		DEFINE_FIELD(CBaseMonster, m_iTriggerCondition, FIELD_INTEGER),
		DEFINE_FIELD(CBaseMonster, m_iszTriggerTarget, FIELD_STRING),

		DEFINE_FIELD(CBaseMonster, m_HackedGunPos, FIELD_VECTOR),

		DEFINE_FIELD(CBaseMonster, m_scriptState, FIELD_INTEGER),
		DEFINE_FIELD(CBaseMonster, m_pCine, FIELD_CLASSPTR),
		DEFINE_FIELD(CBaseMonster, m_AllowItemDropping, FIELD_BOOLEAN),
};

// IMPLEMENT_SAVERESTORE( CBaseMonster, CBaseToggle );
bool CBaseMonster::Save(CSave& save)
{
	if (!CBaseToggle::Save(save))
		return false;
	if (pev->targetname)
		return save.WriteFields(STRING(pev->targetname), "CBaseMonster", this, m_SaveData, ARRAYSIZE(m_SaveData));
	else
		return save.WriteFields(STRING(pev->classname), "CBaseMonster", this, m_SaveData, ARRAYSIZE(m_SaveData));
}

bool CBaseMonster::Restore(CRestore& restore)
{
	if (!CBaseToggle::Restore(restore))
		return false;
	bool status = restore.ReadFields("CBaseMonster", this, m_SaveData, ARRAYSIZE(m_SaveData));

	// We don't save/restore routes yet
	RouteClear();

	// We don't save/restore schedules yet
	m_pSchedule = NULL;
	m_iTaskStatus = TASKSTATUS_NEW;

	// Reset animation
	m_Activity = ACT_RESET;

	// If we don't have an enemy, clear conditions like see enemy, etc.
	if (m_hEnemy == NULL)
		m_afConditions = 0;

	return status;
}
