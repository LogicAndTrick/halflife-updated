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

#include "CPlayerFreeze.h"

#include "locus.h"
#include "entities/player/CBasePlayer.h"
#include "UserMessages.h"

LINK_ENTITY_TO_CLASS(player_freeze, CPlayerFreeze);

void CPlayerFreeze::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (!(pev->spawnflags & SF_FREEZE_LOCUS))
	{
		pActivator = UTIL_FindEntityByClassname(NULL, "player");
	}

	float turnSpeed = 1E6;

	if (pActivator && pActivator->pev->flags & FL_CLIENT)
	{
		//		if (!ShouldToggle(useType, pActivator->pev->flags & FL_FROZEN))
		//			return;

		if ((pev->spawnflags & SF_ACTIVE && useType == USE_TOGGLE) || useType == USE_OFF)
		{
			// unfreeze him
			if (!(pev->spawnflags & SF_DONTFREEZE))
			{
				((CBasePlayer*)((CBaseEntity*)pActivator))->EnableControl(true);
				m_hActivator = NULL;
				DontThink();
			}

			// unclamp his view
			MESSAGE_BEGIN(MSG_ONE, gmsgClampView, NULL, pActivator->pev);
			WRITE_SHORT(0);
			WRITE_SHORT(360);
			WRITE_BYTE(0);
			WRITE_BYTE(255);
			WRITE_LONG(*(long*)&turnSpeed);
			MESSAGE_END();

			pev->spawnflags &= ~SF_ACTIVE;
		}
		else if ((!(pev->spawnflags & SF_ACTIVE) && useType == USE_TOGGLE) || useType == USE_ON)
		{
			// freeze him
			if (!(pev->spawnflags & SF_DONTFREEZE))
			{
				((CBasePlayer*)((CBaseEntity*)pActivator))->EnableControl(false);
			}

			float yawRange = CalcLocus_Number(pActivator, STRING(pev->noise));
			float pitchUpRange = CalcLocus_Number(pActivator, STRING(pev->noise1));
			float pitchDownRange;
			if (FStringNull(pev->noise2))
				pitchDownRange = pitchUpRange;
			else
				pitchDownRange = CalcLocus_Number(pActivator, STRING(pev->noise2));

			if (yawRange < 360 || pitchUpRange < 90 || pitchDownRange < 90)
			{
				Vector clampTargetDir = CalcLocus_Velocity(this, pActivator, STRING(pev->netname));
				Vector clampTargetAngle = UTIL_VecToAngles(clampTargetDir);

				if (!FStringNull(pev->noise3))
					turnSpeed = CalcLocus_Number(pActivator, STRING(pev->noise3));

				// clamp max values
				if (yawRange > 360)
					yawRange = 360;
				if (pitchUpRange > 90)
					pitchUpRange = 90;
				if (pitchDownRange > 90)
					pitchDownRange = 90;

				if (clampTargetAngle.x > 90)
					clampTargetAngle.x -= 360;

				float minYaw = clampTargetAngle.y - yawRange / 2;
				float maxYaw = clampTargetAngle.y + yawRange / 2;
				// this is viewangle pitch, so up is negative
				float minPitch = -clampTargetAngle.x + pitchDownRange;
				float maxPitch = -clampTargetAngle.x - pitchUpRange;

				while (minYaw < 0)
				{
					minYaw += 360;
					maxYaw += 360;
				}

				// clamp the view
				MESSAGE_BEGIN(MSG_ONE, gmsgClampView, NULL, pActivator->pev);
				WRITE_SHORT(minYaw);
				WRITE_SHORT(maxYaw);
				WRITE_BYTE(minPitch + 128);
				WRITE_BYTE(maxPitch + 128);
				WRITE_LONG(*(long*)&turnSpeed);
				MESSAGE_END();
			}
			else
			{
				// unclamp the view
				MESSAGE_BEGIN(MSG_ONE, gmsgClampView, NULL, pActivator->pev);
				WRITE_SHORT(0);
				WRITE_SHORT(360);
				WRITE_BYTE(0);
				WRITE_BYTE(255);
				WRITE_LONG(*(long*)&turnSpeed);
				MESSAGE_END();
			}

			pev->spawnflags |= SF_ACTIVE;

			if (m_flDelay)
			{
				m_hActivator = pActivator;
				SetNextThink(m_flDelay);
			}
		}
	}
}

void CPlayerFreeze::Think()
{
	Use(m_hActivator, this, USE_ON, 0);
}
