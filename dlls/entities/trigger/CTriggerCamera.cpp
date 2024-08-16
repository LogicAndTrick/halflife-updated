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

#include "CTriggerCamera.h"
#include "entities/path/CPathCorner.h"
#include "entities/player/CBasePlayer.h"

LINK_ENTITY_TO_CLASS(trigger_camera, CTriggerCamera);

// Global Savedata for changelevel friction modifier
TYPEDESCRIPTION CTriggerCamera::m_SaveData[] =
	{
		DEFINE_FIELD(CTriggerCamera, m_hPlayer, FIELD_EHANDLE),
		DEFINE_FIELD(CTriggerCamera, m_hTarget, FIELD_EHANDLE),
		DEFINE_FIELD(CTriggerCamera, m_pentPath, FIELD_CLASSPTR),
		DEFINE_FIELD(CTriggerCamera, m_sPath, FIELD_STRING),
		DEFINE_FIELD(CTriggerCamera, m_flWait, FIELD_FLOAT),
		DEFINE_FIELD(CTriggerCamera, m_flReturnTime, FIELD_TIME),
		DEFINE_FIELD(CTriggerCamera, m_flStopTime, FIELD_TIME),
		DEFINE_FIELD(CTriggerCamera, m_moveDistance, FIELD_FLOAT),
		DEFINE_FIELD(CTriggerCamera, m_targetSpeed, FIELD_FLOAT),
		DEFINE_FIELD(CTriggerCamera, m_initialSpeed, FIELD_FLOAT),
		DEFINE_FIELD(CTriggerCamera, m_acceleration, FIELD_FLOAT),
		DEFINE_FIELD(CTriggerCamera, m_deceleration, FIELD_FLOAT),
		DEFINE_FIELD(CTriggerCamera, m_state, FIELD_BOOLEAN),
		DEFINE_FIELD(CTriggerCamera, m_iszViewEntity, FIELD_STRING),
};

IMPLEMENT_SAVERESTORE(CTriggerCamera, CBaseDelay);

void CTriggerCamera::Spawn()
{
	pev->movetype = MOVETYPE_NOCLIP;
	pev->solid = SOLID_NOT; // Remove model & collisions
	pev->renderamt = 0;		// The engine won't draw this model if this is set to 0 and blending is on
	pev->rendermode = kRenderTransTexture;

	m_initialSpeed = pev->speed;
	if (m_acceleration == 0)
		m_acceleration = 500;
	if (m_deceleration == 0)
		m_deceleration = 500;
}

bool CTriggerCamera::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "wait"))
	{
		m_flWait = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "moveto"))
	{
		m_sPath = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "acceleration"))
	{
		m_acceleration = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "deceleration"))
	{
		m_deceleration = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszViewEntity"))
	{
		m_iszViewEntity = ALLOC_STRING(pkvd->szValue);
		return true;
	}

	return CBaseDelay::KeyValue(pkvd);
}

void CTriggerCamera::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (!ShouldToggle(useType, m_state))
		return;

	// Toggle state
	m_state = !m_state;
	if (!m_state)
	{
		m_flReturnTime = gpGlobals->time;
		return;
	}
	if (!pActivator || !pActivator->IsPlayer())
	{
		pActivator = UTIL_GetLocalPlayer();

		if (!pActivator)
		{
			return;
		}
	}

	auto player = static_cast<CBasePlayer*>(pActivator);

	m_hPlayer = pActivator;

	if (m_flWait == -1) // G-Cont. if wait time = -1, set is 1E6 for retriggered only
		m_flReturnTime = gpGlobals->time + 1E6;
	else
		m_flReturnTime = gpGlobals->time + m_flWait;

	pev->speed = m_initialSpeed;
	m_targetSpeed = m_initialSpeed;

	if (FBitSet(pev->spawnflags, SF_CAMERA_PLAYER_TARGET))
	{
		m_hTarget = m_hPlayer;
	}
	else
	{
		m_hTarget = GetNextTarget();
	}

	// Nothing to look at!
	if (m_hTarget == NULL)
	{
		ALERT(at_debug, "Warning! Trigger Camera don't have target! Set target as player.");
		m_hTarget = m_hPlayer; // G-Cont. if cam target don't specified - target is player.
							   // return;
	}


	if (FBitSet(pev->spawnflags, SF_CAMERA_PLAYER_TAKECONTROL))
	{
		player->EnableControl(false);
	}

	if (!FStringNull(m_sPath))
	{
		m_pentPath = UTIL_FindEntityByTargetname(NULL, STRING(m_sPath));
	}
	else
	{
		m_pentPath = NULL;
	}

	m_flStopTime = gpGlobals->time;
	if (m_pentPath)
	{
		if (m_pentPath->pev->speed != 0)
			m_targetSpeed = m_pentPath->pev->speed;

		m_flStopTime += m_pentPath->GetDelay();
	}

	// copy over player information
	if (FBitSet(pev->spawnflags, SF_CAMERA_PLAYER_POSITION))
	{
		UTIL_SetOrigin(this, pActivator->pev->origin + pActivator->pev->view_ofs);
		pev->angles.x = -pActivator->pev->angles.x;
		pev->angles.y = pActivator->pev->angles.y;
		pev->angles.z = 0;
		pev->velocity = pActivator->pev->velocity;
	}
	else
	{
		pev->velocity = Vector(0, 0, 0);
	}

	// LRC
	if (m_iszViewEntity)
	{
		CBaseEntity* pEntity = UTIL_FindEntityByTargetname(NULL, STRING(m_iszViewEntity));
		if (pEntity)
		{
			int sendflags = 0;
			sendflags |= 1;
			if (pev->spawnflags & SF_CAMERA_DRAWHUD)
				sendflags |= 2;
			((CBasePlayer*)pActivator)->viewEntity = m_iszViewEntity;
			((CBasePlayer*)pActivator)->viewFlags = sendflags;
			((CBasePlayer*)pActivator)->viewNeedsUpdate = 1;
		}
	}
	else
	{
		int sendflags = 0;
		sendflags |= 1;
		if (pev->spawnflags & SF_CAMERA_DRAWHUD)
			sendflags |= 2;
		((CBasePlayer*)pActivator)->viewEntity = pev->targetname;
		((CBasePlayer*)pActivator)->viewFlags = sendflags;
		((CBasePlayer*)pActivator)->viewNeedsUpdate = 1;
	}

	player->m_hViewEntity = this;

	SET_MODEL(ENT(pev), STRING(pActivator->pev->model));

	// follow the player down
	SetThink(&CTriggerCamera::FollowTarget);
	SetNextThink(0);

	m_moveDistance = 0;
	Move();
}

void CTriggerCamera::FollowTarget()
{
	if (m_hPlayer == NULL)
		return;

	if (m_hTarget == NULL || m_flReturnTime < gpGlobals->time)
	{
		auto player = static_cast<CBasePlayer*>(static_cast<CBaseEntity*>(m_hPlayer));

		if (player->IsAlive())
		{
			player->viewEntity = 0;
			player->viewFlags = 0;
			player->viewNeedsUpdate = 1;
			player->EnableControl(true);
		}

		player->m_hViewEntity = nullptr;
		player->m_bResetViewEntity = false;

		SUB_UseTargets(this, USE_TOGGLE, 0);
		pev->avelocity = Vector(0, 0, 0);
		m_state = false;
		return;
	}

	Vector vecGoal = UTIL_VecToAngles(m_hTarget->pev->origin - pev->origin);
	vecGoal.x = -vecGoal.x;

	if (pev->angles.y > 360)
		pev->angles.y -= 360;

	if (pev->angles.y < 0)
		pev->angles.y += 360;

	float dx = vecGoal.x - pev->angles.x;
	float dy = vecGoal.y - pev->angles.y;

	if (dx < -180)
		dx += 360;
	if (dx > 180)
		dx = dx - 360;

	if (dy < -180)
		dy += 360;
	if (dy > 180)
		dy = dy - 360;

	pev->avelocity.x = dx * 40 * 0.01;
	pev->avelocity.y = dy * 40 * 0.01;


	if (!(FBitSet(pev->spawnflags, SF_CAMERA_PLAYER_TAKECONTROL)))
	{
		pev->velocity = pev->velocity * 0.8;
		if (pev->velocity.Length() < 10.0) // LRC- whyyyyyy???
			pev->velocity = g_vecZero;
	}

	SetNextThink(0);

	Move();
}

void CTriggerCamera::Move()
{
	// Not moving on a path, return
	if (!m_pentPath)
		return;

	// Subtract movement from the previous frame
	m_moveDistance -= pev->speed * gpGlobals->frametime;

	// Have we moved enough to reach the target?
	if (m_moveDistance <= 0)
	{
		// Fire the passtarget if there is one
		if (!FStringNull(m_pentPath->pev->message))
		{
			FireTargets(STRING(m_pentPath->pev->message), this, this, USE_TOGGLE, 0);
			if (FBitSet(m_pentPath->pev->spawnflags, SF_CORNER_FIREONCE))
				m_pentPath->pev->message = 0;
		}
		// Time to go to the next target
		m_pentPath = m_pentPath->GetNextTarget();

		// Set up next corner
		if (!m_pentPath)
		{
			pev->velocity = g_vecZero;
		}
		else
		{
			if (m_pentPath->pev->speed != 0)
				m_targetSpeed = m_pentPath->pev->speed;

			Vector delta = m_pentPath->pev->origin - pev->origin;
			m_moveDistance = delta.Length();
			pev->movedir = delta.Normalize();
			m_flStopTime = gpGlobals->time + m_pentPath->GetDelay();
		}
	}

	if (m_flStopTime > gpGlobals->time)
		pev->speed = UTIL_Approach(0, pev->speed, m_deceleration * gpGlobals->frametime);
	else
		pev->speed = UTIL_Approach(m_targetSpeed, pev->speed, m_acceleration * gpGlobals->frametime);

	float fraction = 2 * gpGlobals->frametime;
	pev->velocity = ((pev->movedir * pev->speed) * fraction) + (pev->velocity * (1 - fraction));
}
