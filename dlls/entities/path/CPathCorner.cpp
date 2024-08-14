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

#include "CPathCorner.h"

LINK_ENTITY_TO_CLASS(path_corner, CPathCorner);

// Global Savedata for Delay
TYPEDESCRIPTION CPathCorner::m_SaveData[] =
	{
		DEFINE_FIELD(CPathCorner, m_flWait, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(CPathCorner, CPointEntity);

//
// Cache user-entity-field values until spawn is called.
//
bool CPathCorner::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "wait"))
	{
		m_flWait = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "turnspeed")) // LRC
	{
		if (pkvd->szValue[0]) // if the field is blank, don't set the spawnflag.
		{
			pev->spawnflags |= SF_CORNER_AVELOCITY;
			UTIL_StringToVector((float*)pev->avelocity, pkvd->szValue);
		}
		return true;
	}

	return CPointEntity::KeyValue(pkvd);
}

void CPathCorner::Spawn()
{
	ASSERTSZ(!FStringNull(pev->targetname), "path_corner without a targetname");
}

#if 0
void CPathCorner::Touch( CBaseEntity *pOther )
{
	entvars_t*		pevToucher = pOther->pev;
		
	if ( FBitSet ( pevToucher->flags, FL_MONSTER ) )
	{// monsters don't navigate path corners based on touch anymore
		return;
	}

	// If OTHER isn't explicitly looking for this path_corner, bail out
	if ( pOther->m_pGoalEnt != this )
	{
		return;
	}

	// If OTHER has an enemy, this touch is incidental, ignore
	if ( !FNullEnt(pevToucher->enemy) )
	{
		return;		// fighting, not following a path
	}
	
	// UNDONE: support non-zero flWait
	/*
	if (m_flWait != 0)
		ALERT(at_warning, "Non-zero path-cornder waits NYI");
	*/

	// Find the next "stop" on the path, make it the goal of the "toucher".
	if (FStringNull(pev->target))
	{
		ALERT(at_warning, "PathCornerTouch: no next stop specified");
	}
	
	pOther->m_pGoalEnt = UTIL_FindEntityByTargetname ( NULL, STRING(pev->target) );

	// If "next spot" was not found (does not exist - level design error)
	if ( !pOther->m_pGoalEnt )
	{
		ALERT(at_debug, "PathCornerTouch--%s couldn't find next stop in path: %s", STRING(pev->classname), STRING(pev->target));
		return;
	}

	// Turn towards the next stop in the path.
	pevToucher->ideal_yaw = UTIL_VecToYaw ( pOther->m_pGoalEnt->pev->origin - pevToucher->origin );
}
#endif
