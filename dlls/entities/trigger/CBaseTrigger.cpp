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

#include "CBaseTrigger.h"

LINK_ENTITY_TO_CLASS(trigger, CBaseTrigger);

bool CBaseTrigger::CanTouch(entvars_t* pevToucher)
{
	if (!pev->netname)
	{
		// Only touch clients, monsters, or pushables (depending on flags)
		if (pevToucher->flags & FL_CLIENT)
			return !(pev->spawnflags & SF_TRIGGER_NOCLIENTS);
		else if (pevToucher->flags & FL_MONSTER)
			return pev->spawnflags & SF_TRIGGER_ALLOWMONSTERS;
		else if (FClassnameIs(pevToucher, "func_pushable"))
			return pev->spawnflags & SF_TRIGGER_PUSHABLES;
		else
			return pev->spawnflags & SF_TRIGGER_EVERYTHING;
	}
	else
	{
		// If netname is set, it's an entity-specific trigger; we ignore the spawnflags.
		if (!FClassnameIs(pevToucher, STRING(pev->netname)) &&
			(!pevToucher->targetname || !FStrEq(STRING(pevToucher->targetname), STRING(pev->netname))))
			return false;
	}
	return true;
}

//
// ToggleUse - If this is the USE function for a trigger, its state will toggle every time it's fired
//
void CBaseTrigger::ToggleUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (pev->solid == SOLID_NOT)
	{ // if the trigger is off, turn it on
		pev->solid = SOLID_TRIGGER;
		m_hActivator = pActivator; // AJH players can get frags for world kills

		// Force retouch
		gpGlobals->force_retouch++;
	}
	else
	{ // turn the trigger off
		pev->solid = SOLID_NOT;
		m_hActivator = NULL; // AJH players can get frags for world kills
	}
	UTIL_SetOrigin(this, pev->origin);
}

/*
================
InitTrigger
================
*/
void CBaseTrigger::InitTrigger()
{
	// trigger angles are used for one-way touches.  An angle of 0 is assumed
	// to mean no restrictions, so use a yaw of 360 instead.
	if (pev->angles != g_vecZero)
		SetMovedir(pev);
	pev->solid = SOLID_TRIGGER;
	pev->movetype = MOVETYPE_NONE;
	SET_MODEL(ENT(pev), STRING(pev->model)); // set size and link into world
	if (CVAR_GET_FLOAT("showtriggers") == 0)
		SetBits(pev->effects, EF_NODRAW);
}
