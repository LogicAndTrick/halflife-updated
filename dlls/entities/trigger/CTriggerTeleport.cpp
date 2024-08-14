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

#include "CTriggerTeleport.h"

LINK_ENTITY_TO_CLASS(trigger_teleport, CTriggerTeleport);

void CTriggerTeleport::Spawn()
{
	InitTrigger();

	SetTouch(&CTriggerTeleport::TeleportTouch);
}

void CTriggerTeleport::TeleportTouch(CBaseEntity* pOther)
{
	entvars_t* pevToucher = pOther->pev;
	CBaseEntity* pTarget = NULL;

	// Only teleport monsters or clients
	if (!FBitSet(pevToucher->flags, FL_CLIENT | FL_MONSTER))
		return;

	if (!UTIL_IsMasterTriggered(m_sMaster, pOther))
		return;

	if (!CanTouch(pevToucher))
		return;

	pTarget = UTIL_FindEntityByTargetname(pTarget, STRING(pev->target));
	if (!pTarget)
		return;

	// LRC - landmark based teleports
	CBaseEntity* pLandmark = UTIL_FindEntityByTargetname(NULL, STRING(pev->message));
	if (pLandmark)
	{
		Vector vecOriginOffs = pTarget->pev->origin - pLandmark->pev->origin;
		// ALERT(at_console, "Offs initially: %f %f %f\n", vecOriginOffs.x, vecOriginOffs.y, vecOriginOffs.z);

		// do we need to rotate the entity?
		if (pLandmark->pev->angles != pTarget->pev->angles)
		{
			Vector vecVA;
			float ydiff = pTarget->pev->angles.y - pLandmark->pev->angles.y;

			// set new angle to face
			//			ALERT(at_console, "angles = %f %f %f\n", pOther->pev->angles.x, pOther->pev->angles.y, pOther->pev->angles.z);
			pOther->pev->angles.y += ydiff;
			if (pOther->IsPlayer())
			{
				//				ALERT(at_console, "v_angle = %f %f %f\n", pOther->pev->v_angle.x, pOther->pev->v_angle.y, pOther->pev->v_angle.z);
				pOther->pev->angles.x = pOther->pev->v_angle.x;
				//				pOther->pev->v_angle.y += ydiff;
				pOther->pev->fixangle = true;
			}

			// set new velocity
			vecVA = UTIL_VecToAngles(pOther->pev->velocity);
			vecVA.y += ydiff;
			UTIL_MakeVectors(vecVA);
			pOther->pev->velocity = gpGlobals->v_forward * pOther->pev->velocity.Length();
			// fix the ugly "angle to vector" behaviour - a legacy from Quake
			pOther->pev->velocity.z = -pOther->pev->velocity.z;

			// set new origin
			Vector vecPlayerOffs = pOther->pev->origin - pLandmark->pev->origin;
			// ALERT(at_console, "PlayerOffs: %f %f %f\n", vecPlayerOffs.x, vecPlayerOffs.y, vecPlayerOffs.z);
			vecVA = UTIL_VecToAngles(vecPlayerOffs);
			UTIL_MakeVectors(vecVA);
			vecVA.y += ydiff;
			UTIL_MakeVectors(vecVA);
			Vector vecPlayerOffsNew = gpGlobals->v_forward * vecPlayerOffs.Length();
			vecPlayerOffsNew.z = -vecPlayerOffsNew.z;
			// ALERT(at_console, "PlayerOffsNew: %f %f %f\n", vecPlayerOffsNew.x, vecPlayerOffsNew.y, vecPlayerOffsNew.z);

			vecOriginOffs = vecOriginOffs + vecPlayerOffsNew - vecPlayerOffs;
			// ALERT(at_console, "vecOriginOffs: %f %f %f\n", vecOriginOffs.x, vecOriginOffs.y, vecOriginOffs.z);
			//			vecOriginOffs.y++;
		}

		UTIL_SetOrigin(pOther, pOther->pev->origin + vecOriginOffs);
	}
	else
	{
		Vector tmp = pTarget->pev->origin;

		if (pOther->IsPlayer())
		{
			tmp.z -= pOther->pev->mins.z; // make origin adjustments in case the teleportee is a player. (origin in center, not at feet)
		}
		tmp.z++;
		UTIL_SetOrigin(pOther, tmp);

		pOther->pev->angles = pTarget->pev->angles;
		pOther->pev->velocity = pOther->pev->basevelocity = g_vecZero;
		if (pOther->IsPlayer())
		{
			pOther->pev->v_angle = pTarget->pev->angles; // LRC
			pevToucher->fixangle = true;
		}
	}

	pevToucher->flags &= ~FL_ONGROUND;
	pevToucher->fixangle = true;

	FireTargets(STRING(pev->noise), pOther, this, USE_TOGGLE, 0);
}
