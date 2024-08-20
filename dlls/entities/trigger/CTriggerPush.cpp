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

#include "CTriggerPush.h"
#include "util/locus.h"

LINK_ENTITY_TO_CLASS(trigger_push, CTriggerPush);

TYPEDESCRIPTION CTriggerPush::m_SaveData[] =
	{
		DEFINE_FIELD(CTriggerPush, m_iszPushVel, FIELD_STRING),
		DEFINE_FIELD(CTriggerPush, m_iszPushSpeed, FIELD_STRING),
};

IMPLEMENT_SAVERESTORE(CTriggerPush, CBaseTrigger);

bool CTriggerPush::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "m_iszPushSpeed"))
	{
		m_iszPushSpeed = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszPushVel"))
	{
		m_iszPushVel = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	return CBaseTrigger::KeyValue(pkvd);
}

/*QUAKED trigger_push (.5 .5 .5) ? TRIG_PUSH_ONCE
Pushes the player
*/
void CTriggerPush::Spawn()
{
	if (pev->angles == g_vecZero)
		pev->angles.y = 360;
	InitTrigger();

	if (FBitSet(pev->spawnflags, SF_TRIGGER_PUSH_START_OFF)) // if flagged to Start Turned Off, make trigger nonsolid.
		pev->solid = SOLID_NOT;

	SetUse(&CTriggerPush::ToggleUse);

	UTIL_SetOrigin(this, pev->origin); // Link into the list
}

void CTriggerPush::Touch(CBaseEntity* pOther)
{
	entvars_t* pevToucher = pOther->pev;

	// UNDONE: Is there a better way than health to detect things that have physics? (clients/monsters)
	switch (pevToucher->movetype)
	{
	case MOVETYPE_NONE:
	case MOVETYPE_PUSH:
	case MOVETYPE_NOCLIP:
	case MOVETYPE_FOLLOW:
		return;
	}

	Vector vecPush;
	if (!FStringNull(m_iszPushVel))
		vecPush = CalcLocus_Velocity(this, pOther, STRING(m_iszPushVel));
	else
		vecPush = pev->movedir;

	if (!FStringNull(m_iszPushSpeed))
		vecPush = vecPush * CalcLocus_Number(pOther, STRING(m_iszPushSpeed));

	if (pev->speed)
		vecPush = vecPush * pev->speed;
	else
		vecPush = vecPush * 100;

	if (pevToucher->solid != SOLID_NOT && pevToucher->solid != SOLID_BSP)
	{
		// Instant trigger, just transfer velocity and remove
		if (FBitSet(pev->spawnflags, SF_TRIG_PUSH_ONCE))
		{
			pevToucher->velocity = pevToucher->velocity + vecPush;
			if (pevToucher->velocity.z > 0)
				pevToucher->flags &= ~FL_ONGROUND;
			UTIL_Remove(this);
		}
		else
		{ // Push field, transfer to base velocity
			if (pevToucher->flags & FL_BASEVELOCITY)
				vecPush = vecPush + pevToucher->basevelocity;

			pevToucher->basevelocity = vecPush;

			pevToucher->flags |= FL_BASEVELOCITY;
			//			ALERT( at_console, "Vel %f, base %f\n", pevToucher->velocity.z, pevToucher->basevelocity.z );
		}
	}
}
