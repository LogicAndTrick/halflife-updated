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

#include "CTriggerMotion.h"
#include "locus.h"

LINK_ENTITY_TO_CLASS(trigger_motion, CTriggerMotion);

TYPEDESCRIPTION CTriggerMotion::m_SaveData[] =
	{
		DEFINE_FIELD(CTriggerMotion, m_iszPosition, FIELD_STRING),
		DEFINE_FIELD(CTriggerMotion, m_iPosMode, FIELD_INTEGER),
		DEFINE_FIELD(CTriggerMotion, m_iszAngles, FIELD_STRING),
		DEFINE_FIELD(CTriggerMotion, m_iAngMode, FIELD_INTEGER),
		DEFINE_FIELD(CTriggerMotion, m_iszVelocity, FIELD_STRING),
		DEFINE_FIELD(CTriggerMotion, m_iVelMode, FIELD_INTEGER),
		DEFINE_FIELD(CTriggerMotion, m_iszAVelocity, FIELD_STRING),
		DEFINE_FIELD(CTriggerMotion, m_iAVelMode, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CTriggerMotion, CPointEntity);

bool CTriggerMotion::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "m_iszPosition"))
	{
		m_iszPosition = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iPosMode"))
	{
		m_iPosMode = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszAngles"))
	{
		m_iszAngles = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iAngMode"))
	{
		m_iAngMode = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszVelocity"))
	{
		m_iszVelocity = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iVelMode"))
	{
		m_iVelMode = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszAVelocity"))
	{
		m_iszAVelocity = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iAVelMode"))
	{
		m_iAVelMode = atoi(pkvd->szValue);
		return true;
	}
	return CPointEntity::KeyValue(pkvd);
}

void CTriggerMotion::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	CBaseEntity* pTarget = UTIL_FindEntityByTargetname(NULL, STRING(pev->target), pActivator);
	if (pTarget == NULL || pActivator == NULL)
		return;

	if (pev->spawnflags & SF_MOTION_DEBUG)
		ALERT(at_debug, "DEBUG: trigger_motion affects %s \"%s\":\n", STRING(pTarget->pev->classname), STRING(pTarget->pev->targetname));

	if (m_iszPosition)
	{
		switch (m_iPosMode)
		{
		case 0:
			if (pev->spawnflags & SF_MOTION_DEBUG)
				ALERT(at_debug, "DEBUG: Set origin from %f %f %f ", pTarget->pev->origin.x, pTarget->pev->origin.y, pTarget->pev->origin.z);
			pTarget->pev->origin = CalcLocus_Position(this, pActivator, STRING(m_iszPosition));
			if (pev->spawnflags & SF_MOTION_DEBUG)
				ALERT(at_debug, "to %f %f %f\n", pTarget->pev->origin.x, pTarget->pev->origin.y, pTarget->pev->origin.z);
			pTarget->pev->flags &= ~FL_ONGROUND;
			break;
		case 1:
			if (pev->spawnflags & SF_MOTION_DEBUG)
				ALERT(at_debug, "DEBUG: Set origin from %f %f %f ", pTarget->pev->origin.x, pTarget->pev->origin.y, pTarget->pev->origin.z);
			pTarget->pev->origin = pTarget->pev->origin + CalcLocus_Velocity(this, pActivator, STRING(m_iszPosition));
			if (pev->spawnflags & SF_MOTION_DEBUG)
				ALERT(at_debug, "to %f %f %f\n", pTarget->pev->origin.x, pTarget->pev->origin.y, pTarget->pev->origin.z);
			pTarget->pev->flags &= ~FL_ONGROUND;
			break;
		}
	}

	Vector vecTemp;
	Vector vecVelAngles;
	if (m_iszAngles)
	{
		switch (m_iAngMode)
		{
		case 0:
			vecTemp = CalcLocus_Velocity(this, pActivator, STRING(m_iszAngles));
			if (pev->spawnflags & SF_MOTION_DEBUG)
				ALERT(at_debug, "DEBUG: Set angles from %f %f %f ", pTarget->pev->angles.x, pTarget->pev->angles.y, pTarget->pev->angles.z);
			pTarget->pev->angles = UTIL_VecToAngles(vecTemp);
			if (pev->spawnflags & SF_MOTION_DEBUG)
				ALERT(at_debug, "to %f %f %f\n", pTarget->pev->angles.x, pTarget->pev->angles.y, pTarget->pev->angles.z);
			break;
		case 1:
			vecTemp = CalcLocus_Velocity(this, pActivator, STRING(m_iszAngles)); // LRC 1.8 - was using m_iszVelocity by mistake
			if (pev->spawnflags & SF_MOTION_DEBUG)
				ALERT(at_debug, "DEBUG: Rotate angles from %f %f %f ", pTarget->pev->angles.x, pTarget->pev->angles.y, pTarget->pev->angles.z);
			pTarget->pev->angles = pTarget->pev->angles + UTIL_VecToAngles(vecTemp);
			if (pev->spawnflags & SF_MOTION_DEBUG)
				ALERT(at_debug, "to %f %f %f\n", pTarget->pev->angles.x, pTarget->pev->angles.y, pTarget->pev->angles.z);
			break;
		case 2:
			vecTemp = CalcLocus_PYR(this, pActivator, STRING(m_iszAngles)); // LRC 1.8
			if (pev->spawnflags & SF_MOTION_DEBUG)
				ALERT(at_debug, "DEBUG: Rotate angles from %f %f %f ", pTarget->pev->angles.x, pTarget->pev->angles.y, pTarget->pev->angles.z);
			pTarget->pev->angles = pTarget->pev->angles + vecTemp;
			if (pev->spawnflags & SF_MOTION_DEBUG)
				ALERT(at_debug, "to %f %f %f\n", pTarget->pev->angles.x, pTarget->pev->angles.y, pTarget->pev->angles.z);
			break;
		}
	}

	if (m_iszVelocity)
	{
		switch (m_iVelMode)
		{
		case 0:
			if (pev->spawnflags & SF_MOTION_DEBUG)
				ALERT(at_debug, "DEBUG: Set velocity from %f %f %f ", pTarget->pev->velocity.x, pTarget->pev->velocity.y, pTarget->pev->velocity.z);
			pTarget->pev->velocity = CalcLocus_Velocity(this, pActivator, STRING(m_iszVelocity));
			if (pev->spawnflags & SF_MOTION_DEBUG)
				ALERT(at_debug, "to %f %f %f\n", pTarget->pev->velocity.x, pTarget->pev->velocity.y, pTarget->pev->velocity.z);
			break;
		case 1:
			if (pev->spawnflags & SF_MOTION_DEBUG)
				ALERT(at_debug, "DEBUG: Set velocity from %f %f %f ", pTarget->pev->velocity.x, pTarget->pev->velocity.y, pTarget->pev->velocity.z);
			pTarget->pev->velocity = pTarget->pev->velocity + CalcLocus_Velocity(this, pActivator, STRING(m_iszVelocity));
			if (pev->spawnflags & SF_MOTION_DEBUG)
				ALERT(at_debug, "to %f %f %f\n", pTarget->pev->velocity.x, pTarget->pev->velocity.y, pTarget->pev->velocity.z);
			break;
		case 2:
			vecTemp = CalcLocus_Velocity(this, pActivator, STRING(m_iszVelocity));
			vecVelAngles = UTIL_VecToAngles(vecTemp) + UTIL_VecToAngles(pTarget->pev->velocity);
			UTIL_MakeVectors(vecVelAngles);
			if (pev->spawnflags & SF_MOTION_DEBUG)
				ALERT(at_debug, "DEBUG: Rotate velocity from %f %f %f ", pTarget->pev->velocity.x, pTarget->pev->velocity.y, pTarget->pev->velocity.z);
			pTarget->pev->velocity = pTarget->pev->velocity.Length() * gpGlobals->v_forward;
			pTarget->pev->velocity.z = -pTarget->pev->velocity.z; // vecToAngles reverses the z angle
			if (pev->spawnflags & SF_MOTION_DEBUG)
				ALERT(at_debug, "to %f %f %f\n", pTarget->pev->velocity.x, pTarget->pev->velocity.y, pTarget->pev->velocity.z);
			break;
		case 3:
			vecTemp = CalcLocus_PYR(this, pActivator, STRING(m_iszVelocity));
			vecVelAngles = vecTemp + UTIL_VecToAngles(pTarget->pev->velocity);
			UTIL_MakeVectors(vecVelAngles);
			if (pev->spawnflags & SF_MOTION_DEBUG)
				ALERT(at_debug, "DEBUG: Rotate velocity from %f %f %f ", pTarget->pev->velocity.x, pTarget->pev->velocity.y, pTarget->pev->velocity.z);
			pTarget->pev->velocity = pTarget->pev->velocity.Length() * gpGlobals->v_forward;
			pTarget->pev->velocity.z = -pTarget->pev->velocity.z; // vecToAngles reverses the z angle
			if (pev->spawnflags & SF_MOTION_DEBUG)
				ALERT(at_debug, "to %f %f %f\n", pTarget->pev->velocity.x, pTarget->pev->velocity.y, pTarget->pev->velocity.z);
			break;
		}
	}

	if (m_iszAVelocity)
	{ // AJH (you forgot to add this Laurie!!)
		switch (m_iAVelMode)
		{
		case 0:
			vecTemp = CalcLocus_PYR(this, pActivator, STRING(m_iszAVelocity));
			if (pev->spawnflags & SF_MOTION_DEBUG)
				ALERT(at_debug, "DEBUG: Set avelocity from %f %f %f ", pTarget->pev->avelocity.x, pTarget->pev->avelocity.y, pTarget->pev->avelocity.z);
			pTarget->pev->avelocity = vecTemp;
			if (pev->spawnflags & SF_MOTION_DEBUG)
				ALERT(at_debug, "to %f %f %f\n", pTarget->pev->avelocity.x, pTarget->pev->avelocity.y, pTarget->pev->avelocity.z);
			break;
		case 1:
			vecTemp = CalcLocus_PYR(this, pActivator, STRING(m_iszAVelocity));
			if (pev->spawnflags & SF_MOTION_DEBUG)
				ALERT(at_debug, "DEBUG: Set avelocity from %f %f %f ", pTarget->pev->avelocity.x, pTarget->pev->avelocity.y, pTarget->pev->avelocity.z);
			pTarget->pev->avelocity = pTarget->pev->avelocity + vecTemp;
			if (pev->spawnflags & SF_MOTION_DEBUG)
				ALERT(at_debug, "to %f %f %f\n", pTarget->pev->avelocity.x, pTarget->pev->avelocity.y, pTarget->pev->avelocity.z);
			break;
		}
	} // AJH
}
