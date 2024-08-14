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

#include "CMotionManager.h"
#include "entities/trigger/CTriggerMotion.h"
#include "locus.h"
#include "movewith.h"

LINK_ENTITY_TO_CLASS(motion_manager, CMotionManager);

TYPEDESCRIPTION CMotionManager::m_SaveData[] =
	{
		DEFINE_FIELD(CMotionManager, m_iszPosition, FIELD_STRING),
		DEFINE_FIELD(CMotionManager, m_iPosMode, FIELD_INTEGER),
		DEFINE_FIELD(CMotionManager, m_iszFacing, FIELD_STRING),
		DEFINE_FIELD(CMotionManager, m_iFaceMode, FIELD_INTEGER),
		DEFINE_FIELD(CMotionManager, m_iPosAxis, FIELD_INTEGER),  // AJH
		DEFINE_FIELD(CMotionManager, m_iFaceAxis, FIELD_INTEGER), // AJH
		DEFINE_FIELD(CMotionManager, pThread, FIELD_CLASSPTR),	  // AJH
};

IMPLEMENT_SAVERESTORE(CMotionManager, CPointEntity);

bool CMotionManager::KeyValue(KeyValueData* pkvd)
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
	else if (FStrEq(pkvd->szKeyName, "m_iPosAxis")) // AJH
	{
		m_iPosAxis = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszFacing"))
	{
		m_iszFacing = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iFaceMode"))
	{
		m_iFaceMode = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iFaceAxis")) // AJH
	{
		m_iFaceAxis = atoi(pkvd->szValue);
		return true;
	}
	return CPointEntity::KeyValue(pkvd);
}

void CMotionManager::PostSpawn()
{
	pThread = GetClassPtr((CMotionThread*)NULL);
	if (FStringNull(pev->targetname))
		Use(this, this, USE_ON, 0);
}

void CMotionManager::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (useType == USE_OFF)
	{								  // AJH to allow motion_managers to turn off
		if (pThread != NULL)		  //
			pThread->m_hLocus = NULL; //
	}
	else
	{
		if (pThread == NULL) // AJH we shouldn't need this but for some reason the pointer is ALWAYS NULL after save/restore.
		{
			pThread = GetClassPtr((CMotionThread*)NULL);
			// ALERT(at_debug,"Motion_manager motion thread pointer is NULL, Creating new motionthread.\n");
		}
		CBaseEntity* pTarget = pActivator;
		if (pev->target)
		{
			pTarget = UTIL_FindEntityByTargetname(NULL, STRING(pev->target), pActivator);
			if (pTarget == NULL)
				ALERT(at_error, "motion_manager \"%s\" can't find entity \"%s\" to affect\n", STRING(pev->targetname), STRING(pev->target));
			else
			{
				do
				{
					Affect(pTarget, pActivator);
					pTarget = UTIL_FindEntityByTargetname(pTarget, STRING(pev->target), pActivator);
				} while (pTarget);
			}
		}
	}
}

void CMotionManager::Affect(CBaseEntity* pTarget, CBaseEntity* pActivator)
{
	if (pev->spawnflags & SF_MOTION_DEBUG)
		ALERT(at_debug, "DEBUG: Creating MotionThread for %s \"%s\"\n", STRING(pTarget->pev->classname), STRING(pTarget->pev->targetname));

	if (pThread == NULL)
	{
		ALERT(at_debug, "Motion_manager motion thread pointer is NULL!!\n");
		return; // error?
	}
	pThread->m_hLocus = pActivator;
	pThread->m_hTarget = pTarget;
	pThread->m_iszPosition = m_iszPosition;
	pThread->m_iPosMode = m_iPosMode;
	pThread->m_iPosAxis = m_iPosAxis; // AJH
	pThread->m_iszFacing = m_iszFacing;
	pThread->m_iFaceMode = m_iFaceMode;
	pThread->m_iFaceAxis = m_iFaceAxis; // AJH
	pThread->pev->spawnflags = pev->spawnflags;
	pThread->SetNextThink(0);
}

// ---------------
// ---------------
// ---------------

LINK_ENTITY_TO_CLASS(motion_thread, CPointEntity);

TYPEDESCRIPTION CMotionThread::m_SaveData[] =
	{
		DEFINE_FIELD(CMotionThread, m_iszPosition, FIELD_STRING),
		DEFINE_FIELD(CMotionThread, m_iPosMode, FIELD_INTEGER),
		DEFINE_FIELD(CMotionThread, m_iszFacing, FIELD_STRING),
		DEFINE_FIELD(CMotionThread, m_iFaceMode, FIELD_INTEGER),
		DEFINE_FIELD(CMotionThread, m_hLocus, FIELD_EHANDLE),
		DEFINE_FIELD(CMotionThread, m_hTarget, FIELD_EHANDLE),
		DEFINE_FIELD(CMotionThread, m_iPosAxis, FIELD_INTEGER),	 // AJH axis to affect
		DEFINE_FIELD(CMotionThread, m_iFaceAxis, FIELD_INTEGER), // AJH axis to affect
};

IMPLEMENT_SAVERESTORE(CMotionThread, CPointEntity);

void CMotionThread::Spawn() // AJH
{
	pev->classname = MAKE_STRING("motion_thread"); // We need this for save/restore to work
}

void CMotionThread::Think()
{
	// SetBits(pev->spawnflags, SF_MOTION_DEBUG);
	if (m_hLocus == NULL || m_hTarget == NULL)
	{
		if (pev->spawnflags & SF_MOTION_DEBUG)
			ALERT(at_debug, "motion_thread expires\n");
		SetThink(&CMotionThread::SUB_Remove);
		SetNextThink(0.1);
		return;
	}
	else if (pev->spawnflags & SF_MTHREAD_STEP)
	{ // MJB / AJH - stepped motion_management
		DontThink();
		if (pev->spawnflags & SF_MOTION_DEBUG)
			ALERT(at_console, "Stepped.\n");
	}
	else
	{
		SetNextThink(0); // think every frame
	}

	if (pev->spawnflags & SF_MOTION_DEBUG)
		ALERT(at_debug, "motion_thread affects %s \"%s\":\n", STRING(m_hTarget->pev->classname), STRING(m_hTarget->pev->targetname));

	Vector vecTemp;

	if (m_iszPosition)
	{
		switch (m_iPosMode)
		{
		case 0: // set position
			if (pev->spawnflags & SF_MOTION_DEBUG)
				ALERT(at_debug, "DEBUG: Set origin from %f %f %f ", m_hTarget->pev->origin.x, m_hTarget->pev->origin.y, m_hTarget->pev->origin.z);

			if (!m_iPosAxis)
			{ // AJH Default Behaviour (ORIGINAL)
				UTIL_AssignOrigin(m_hTarget, CalcLocus_Position(this, m_hLocus, STRING(m_iszPosition)));
			}
			else
			{ // AJH Affect this axis behaviour (NEW SOHL 1.4)
				Vector tmp = CalcLocus_Position(this, m_hLocus, STRING(m_iszPosition));
				Vector tmp2 = m_hTarget->pev->origin;
				if (m_iPosAxis & MT_AFFECT_X)
					tmp2.x = tmp.x;
				if (m_iPosAxis & MT_AFFECT_Y)
					tmp2.y = tmp.y;
				if (m_iPosAxis & MT_AFFECT_Z)
					tmp2.z = tmp.z;
				UTIL_AssignOrigin(m_hTarget, tmp2);
			}

			if (pev->spawnflags & SF_MOTION_DEBUG)
				ALERT(at_debug, "to %f %f %f\n", m_hTarget->pev->origin.x, m_hTarget->pev->origin.y, m_hTarget->pev->origin.z);
			m_hTarget->pev->flags &= ~FL_ONGROUND;
			break;

		case 1: // offset position (= fake velocity)
			if (pev->spawnflags & SF_MOTION_DEBUG)
				ALERT(at_debug, "DEBUG: Offset origin from %f %f %f ", m_hTarget->pev->origin.x, m_hTarget->pev->origin.y, m_hTarget->pev->origin.z);

			if (!m_iPosAxis)
			{ // AJH Default Behaviour (ORIGINAL)
				UTIL_AssignOrigin(m_hTarget, m_hTarget->pev->origin + gpGlobals->frametime * CalcLocus_Velocity(this, m_hLocus, STRING(m_iszPosition)));
			}
			else
			{ // AJH Affect this axis behaviour (NEW SOHL 1.4)
				Vector tmp = m_hTarget->pev->origin + gpGlobals->frametime * CalcLocus_Velocity(this, m_hLocus, STRING(m_iszPosition));
				Vector tmp2 = m_hTarget->pev->origin;
				if (m_iPosAxis & MT_AFFECT_X)
					tmp2.x = tmp.x;
				if (m_iPosAxis & MT_AFFECT_Y)
					tmp2.y = tmp.y;
				if (m_iPosAxis & MT_AFFECT_Z)
					tmp2.z = tmp.z;
				UTIL_AssignOrigin(m_hTarget, tmp2);
			}

			if (pev->spawnflags & SF_MOTION_DEBUG)
				ALERT(at_debug, "to %f %f %f\n", m_hTarget->pev->origin.x, m_hTarget->pev->origin.y, m_hTarget->pev->origin.z);
			m_hTarget->pev->flags &= ~FL_ONGROUND;
			break;

		case 2: // set velocity
			if (pev->spawnflags & SF_MOTION_DEBUG)
				ALERT(at_debug, "DEBUG: Set velocity from %f %f %f ", m_hTarget->pev->velocity.x, m_hTarget->pev->velocity.y, m_hTarget->pev->velocity.z);

			if (!m_iPosAxis)
			{ // AJH Default Behaviour (ORIGINAL)
				UTIL_SetVelocity(m_hTarget, CalcLocus_Velocity(this, m_hLocus, STRING(m_iszPosition)));
			}
			else
			{ // AJH Affect this axis behaviour (NEW SOHL 1.4)
				Vector tmp = CalcLocus_Velocity(this, m_hLocus, STRING(m_iszPosition));
				Vector tmp2 = m_hTarget->pev->velocity;
				if (m_iPosAxis & MT_AFFECT_X)
					tmp2.x = tmp.x;
				if (m_iPosAxis & MT_AFFECT_Y)
					tmp2.y = tmp.y;
				if (m_iPosAxis & MT_AFFECT_Z)
					tmp2.z = tmp.z;
				UTIL_SetVelocity(m_hTarget, tmp2);
			}

			if (pev->spawnflags & SF_MOTION_DEBUG)
				ALERT(at_debug, "to %f %f %f\n", m_hTarget->pev->velocity.x, m_hTarget->pev->velocity.y, m_hTarget->pev->velocity.z);
			break;

		case 3: // accelerate
			if (pev->spawnflags & SF_MOTION_DEBUG)
				ALERT(at_debug, "DEBUG: Accelerate from %f %f %f ", m_hTarget->pev->velocity.x, m_hTarget->pev->velocity.y, m_hTarget->pev->velocity.z);

			if (!m_iPosAxis)
			{ // AJH Default Behaviour (ORIGINAL)
				UTIL_SetVelocity(m_hTarget, m_hTarget->pev->velocity + gpGlobals->frametime * CalcLocus_Velocity(this, m_hLocus, STRING(m_iszPosition)));
			}
			else
			{ // AJH Affect this axis behaviour (NEW SOHL 1.4)
				Vector tmp = m_hTarget->pev->velocity + gpGlobals->frametime * CalcLocus_Velocity(this, m_hLocus, STRING(m_iszPosition));
				Vector tmp2 = m_hTarget->pev->velocity;
				if (m_iPosAxis & MT_AFFECT_X)
					tmp2.x = tmp.x;
				if (m_iPosAxis & MT_AFFECT_Y)
					tmp2.y = tmp.y;
				if (m_iPosAxis & MT_AFFECT_Z)
					tmp2.z = tmp.z;
				UTIL_SetVelocity(m_hTarget, tmp2);
			}

			if (pev->spawnflags & SF_MOTION_DEBUG)
				ALERT(at_debug, "to %f %f %f\n", m_hTarget->pev->velocity.x, m_hTarget->pev->velocity.y, m_hTarget->pev->velocity.z);
			break;

		case 4: // follow position
			if (pev->spawnflags & SF_MOTION_DEBUG)
				ALERT(at_debug, "DEBUG: Set velocity (path) from %f %f %f ", m_hTarget->pev->velocity.x, m_hTarget->pev->velocity.y, m_hTarget->pev->velocity.z);

			if (!m_iPosAxis)
			{ // AJH Default Behaviour (ORIGINAL)
				UTIL_SetVelocity(m_hTarget, CalcLocus_Position(this, m_hLocus, STRING(m_iszPosition)) - m_hTarget->pev->origin);
			}
			else
			{ // AJH Affect this axis behaviour (NEW SOHL 1.4)
				Vector tmp = CalcLocus_Position(this, m_hLocus, STRING(m_iszPosition)) - m_hTarget->pev->origin;
				Vector tmp2 = m_hTarget->pev->velocity;
				if (m_iPosAxis & MT_AFFECT_X)
					tmp2.x = tmp.x;
				if (m_iPosAxis & MT_AFFECT_Y)
					tmp2.y = tmp.y;
				if (m_iPosAxis & MT_AFFECT_Z)
					tmp2.z = tmp.z;
				UTIL_SetVelocity(m_hTarget, tmp2);
			}

			if (pev->spawnflags & SF_MOTION_DEBUG)
				ALERT(at_debug, "to %f %f %f\n", m_hTarget->pev->velocity.x, m_hTarget->pev->velocity.y, m_hTarget->pev->velocity.z);
			break;
		}
	}

	Vector vecVelAngles;

	if (m_iszFacing)
	{
		switch (m_iFaceMode)
		{
		case 0: // set angles
			vecTemp = CalcLocus_Velocity(this, m_hLocus, STRING(m_iszFacing));
			if (vecTemp != g_vecZero) // if the vector is 0 0 0, don't use it
			{
				if (pev->spawnflags & SF_MOTION_DEBUG)
					ALERT(at_debug, "DEBUG: Set angles from %f %f %f ", m_hTarget->pev->angles.x, m_hTarget->pev->angles.y, m_hTarget->pev->angles.z);

				if (!m_iFaceAxis)
				{ // AJH Default Behaviour (ORIGINAL)
					UTIL_SetAngles(m_hTarget, UTIL_VecToAngles(vecTemp));
				}
				else
				{ // AJH Affect this axis behaviour (NEW SOHL 1.4)
					Vector tmp = UTIL_VecToAngles(vecTemp);
					Vector tmp2 = m_hTarget->pev->angles;

					if (pev->spawnflags & SF_MOTION_SWAPXY)
					{ // MJB swap Pitch/Yaw axes
						ALERT(at_debug, "Swapping Pitch = %f, Yaw = %f\n", tmp.x, tmp.y);
						float swaptemp = tmp.x;
						tmp.x = tmp.y;
						tmp.y = swaptemp;
					}

					if (pev->spawnflags & SF_MOTION_SWAPYZ)
					{ // MJB swap Yaw/Roll axes
						ALERT(at_debug, "Swapping Yaw = %f, Roll = %f\n", tmp.y, tmp.z);
						float swaptemp = tmp.y;
						tmp.y = tmp.z;
						tmp.z = swaptemp;
					}

					if (pev->spawnflags & SF_MOTION_SWAPZX)
					{ // MJB swap Roll/Pitch axes
						ALERT(at_debug, "Swapping Roll = %f, Pitch = %f\n", tmp.z, tmp.x);
						float swaptemp = tmp.z;
						tmp.z = tmp.x;
						tmp.x = swaptemp;
					}

					if (m_iFaceAxis & MT_AFFECT_X)
					{
						tmp2.x = tmp.x;
					}
					if (m_iFaceAxis & MT_AFFECT_Y)
					{
						tmp2.y = tmp.y;
					}
					if (m_iFaceAxis & MT_AFFECT_Z)
					{
						tmp2.z = tmp.z;
					}
					UTIL_SetAngles(m_hTarget, tmp2);
				}

				if (pev->spawnflags & SF_MOTION_DEBUG)
					ALERT(at_debug, "to %f %f %f\n", m_hTarget->pev->angles.x, m_hTarget->pev->angles.y, m_hTarget->pev->angles.z);
			}
			else if (pev->spawnflags & SF_MOTION_DEBUG)
			{
				ALERT(at_debug, "Zero velocity, don't change angles\n");
			}
			break;

		case 1: // offset angles (= fake avelocity)
			vecTemp = CalcLocus_Velocity(this, m_hLocus, STRING(m_iszFacing));
			if (vecTemp != g_vecZero) // if the vector is 0 0 0, don't use it
			{
				if (pev->spawnflags & SF_MOTION_DEBUG)
					ALERT(at_debug, "DEBUG: Offset angles from %f %f %f ", m_hTarget->pev->angles.x, m_hTarget->pev->angles.y, m_hTarget->pev->angles.z);

				if (!m_iFaceAxis)
				{ // AJH Default Behaviour (ORIGINAL)
					UTIL_SetAngles(m_hTarget, m_hTarget->pev->angles + gpGlobals->frametime * UTIL_VecToAngles(vecTemp));
				}
				else
				{ // AJH Affect this axis behaviour (NEW SOHL 1.4)
					Vector tmp = m_hTarget->pev->angles + gpGlobals->frametime * UTIL_VecToAngles(vecTemp);
					Vector tmp2 = m_hTarget->pev->angles;

					if (pev->spawnflags & SF_MOTION_SWAPXY)
					{ // MJB swap Pitch/Yaw axes
						ALERT(at_debug, "Swapping Pitch = %f, Yaw = %f\n", tmp.x, tmp.y);
						float swaptemp = tmp.x;
						tmp.x = tmp.y;
						tmp.y = swaptemp;
					}

					if (pev->spawnflags & SF_MOTION_SWAPYZ)
					{ // MJB swap Yaw/Roll axes
						ALERT(at_debug, "Swapping Yaw = %f, Roll = %f\n", tmp.y, tmp.z);
						float swaptemp = tmp.y;
						tmp.y = tmp.z;
						tmp.z = swaptemp;
					}

					if (pev->spawnflags & SF_MOTION_SWAPZX)
					{ // MJB swap Roll/Pitch axes
						ALERT(at_debug, "Swapping Roll = %f, Pitch = %f\n", tmp.z, tmp.x);
						float swaptemp = tmp.z;
						tmp.z = tmp.x;
						tmp.x = swaptemp;
					}

					if (m_iFaceAxis & MT_AFFECT_X)
					{
						tmp2.x = tmp.x;
					}
					if (m_iFaceAxis & MT_AFFECT_Y)
					{
						tmp2.y = tmp.y;
					}
					if (m_iFaceAxis & MT_AFFECT_Z)
					{
						tmp2.z = tmp.z;
					}
					UTIL_SetAngles(m_hTarget, tmp2);
				}

				if (pev->spawnflags & SF_MOTION_DEBUG)
					ALERT(at_debug, "to %f %f %f\n", m_hTarget->pev->angles.x, m_hTarget->pev->angles.y, m_hTarget->pev->angles.z);
			}
			else if (pev->spawnflags & SF_MOTION_DEBUG)
			{
				ALERT(at_debug, "Zero velocity, don't change angles\n");
			}
			break;

		case 2: // offset angles (= fake avelocity)
			vecVelAngles = CalcLocus_PYR(this, m_hLocus, STRING(m_iszFacing));
			if (pev->spawnflags & SF_MOTION_DEBUG)
				ALERT(at_debug, "DEBUG: Rotate angles from %f %f %f ", m_hTarget->pev->angles.x, m_hTarget->pev->angles.y, m_hTarget->pev->angles.z);

			if (!m_iFaceAxis)
			{ // AJH Default Behaviour (ORIGINAL)
				UTIL_SetAngles(m_hTarget, m_hTarget->pev->angles + gpGlobals->frametime * vecVelAngles);
			}
			else
			{ // AJH Affect this axis behaviour (NEW SOHL 1.4)
				Vector tmp = m_hTarget->pev->angles + gpGlobals->frametime * vecVelAngles;
				Vector tmp2 = m_hTarget->pev->angles;

				if (pev->spawnflags & SF_MOTION_SWAPXY)
				{ // MJB swap Pitch/Yaw axes
					ALERT(at_debug, "Swapping Pitch = %f, Yaw = %f\n", tmp.x, tmp.y);
					float swaptemp = tmp.x;
					tmp.x = tmp.y;
					tmp.y = swaptemp;
				}

				if (pev->spawnflags & SF_MOTION_SWAPYZ)
				{ // MJB swap Yaw/Roll axes
					ALERT(at_debug, "Swapping Yaw = %f, Roll = %f\n", tmp.y, tmp.z);
					float swaptemp = tmp.y;
					tmp.y = tmp.z;
					tmp.z = swaptemp;
				}

				if (pev->spawnflags & SF_MOTION_SWAPZX)
				{ // MJB swap Roll/Pitch axes
					ALERT(at_debug, "Swapping Roll = %f, Pitch = %f\n", tmp.z, tmp.x);
					float swaptemp = tmp.z;
					tmp.z = tmp.x;
					tmp.x = swaptemp;
				}

				if (m_iFaceAxis & MT_AFFECT_X)
				{
					tmp2.x = tmp.x;
				}
				if (m_iFaceAxis & MT_AFFECT_Y)
				{
					tmp2.y = tmp.y;
				}
				if (m_iFaceAxis & MT_AFFECT_Z)
				{
					tmp2.z = tmp.z;
				}
				UTIL_SetAngles(m_hTarget, tmp2);
			}

			if (pev->spawnflags & SF_MOTION_DEBUG)
				ALERT(at_debug, "to %f %f %f\n", m_hTarget->pev->angles.x, m_hTarget->pev->angles.y, m_hTarget->pev->angles.z);
			break;

		case 3: // set avelocity
			vecVelAngles = CalcLocus_PYR(this, m_hLocus, STRING(m_iszFacing));
			if (pev->spawnflags & SF_MOTION_DEBUG)
				ALERT(at_debug, "DEBUG: Set avelocity from %f %f %f ", m_hTarget->pev->avelocity.x, m_hTarget->pev->avelocity.y, m_hTarget->pev->avelocity.z);

			if (!m_iFaceAxis)
			{ // AJH Default Behaviour (ORIGINAL)
				UTIL_SetAvelocity(m_hTarget, vecTemp);
			}
			else
			{ // AJH Affect this axis behaviour (NEW SOHL 1.4)
				Vector tmp2 = m_hTarget->pev->angles;

				if (pev->spawnflags & SF_MOTION_SWAPXY)
				{ // MJB swap Pitch/Yaw axes
					ALERT(at_debug, "Swapping Pitch = %f, Yaw = %f\n", tmp2.x, tmp2.y);
					float swaptemp = tmp2.x;
					tmp2.x = tmp2.y;
					tmp2.y = swaptemp;
				}

				if (pev->spawnflags & SF_MOTION_SWAPYZ)
				{ // MJB swap Yaw/Roll axes
					ALERT(at_debug, "Swapping Yaw = %f, Roll = %f\n", tmp2.y, tmp2.z);
					float swaptemp = tmp2.y;
					tmp2.y = tmp2.z;
					tmp2.z = swaptemp;
				}

				if (pev->spawnflags & SF_MOTION_SWAPZX)
				{ // MJB swap Roll/Pitch axes
					ALERT(at_debug, "Swapping Roll = %f, Pitch = %f\n", tmp2.z, tmp2.x);
					float swaptemp = tmp2.z;
					tmp2.z = tmp2.x;
					tmp2.x = swaptemp;
				}

				if (m_iFaceAxis & MT_AFFECT_X)
				{
					tmp2.x = vecTemp.x;
				}
				if (m_iFaceAxis & MT_AFFECT_Y)
				{
					tmp2.y = vecTemp.y;
				}
				if (m_iFaceAxis & MT_AFFECT_Z)
				{
					tmp2.z = vecTemp.z;
				}
				UTIL_SetAvelocity(m_hTarget, tmp2);
			}

			if (pev->spawnflags & SF_MOTION_DEBUG)
				ALERT(at_debug, "to %f %f %f\n", m_hTarget->pev->avelocity.x, m_hTarget->pev->avelocity.y, m_hTarget->pev->avelocity.z);
			break;
		}
	}
}
