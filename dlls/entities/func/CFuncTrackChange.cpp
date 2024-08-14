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

#include "CFuncTrackChange.h"
#include "CFuncTrackTrain.h"
#include "entities/path/CPathTrack.h"

LINK_ENTITY_TO_CLASS(func_trackchange, CFuncTrackChange);

TYPEDESCRIPTION CFuncTrackChange::m_SaveData[] =
	{
		DEFINE_GLOBAL_FIELD(CFuncTrackChange, m_trackTop, FIELD_CLASSPTR),
		DEFINE_GLOBAL_FIELD(CFuncTrackChange, m_trackBottom, FIELD_CLASSPTR),
		DEFINE_GLOBAL_FIELD(CFuncTrackChange, m_train, FIELD_CLASSPTR),
		DEFINE_GLOBAL_FIELD(CFuncTrackChange, m_trackTopName, FIELD_STRING),
		DEFINE_GLOBAL_FIELD(CFuncTrackChange, m_trackBottomName, FIELD_STRING),
		DEFINE_GLOBAL_FIELD(CFuncTrackChange, m_trainName, FIELD_STRING),
		DEFINE_FIELD(CFuncTrackChange, m_code, FIELD_INTEGER),
		DEFINE_FIELD(CFuncTrackChange, m_targetState, FIELD_INTEGER),
		DEFINE_FIELD(CFuncTrackChange, m_use, FIELD_BOOLEAN),
};

IMPLEMENT_SAVERESTORE(CFuncTrackChange, CFuncPlatRot);

void CFuncTrackChange::Spawn()
{
	Setup();
	if (FBitSet(pev->spawnflags, SF_TRACK_DONT_MOVE))
		m_vecPosition2.z = pev->origin.z;

	SetupRotation();

	if (FBitSet(pev->spawnflags, SF_TRACK_STARTBOTTOM))
	{
		UTIL_SetOrigin(this, m_vecPosition2);
		m_toggle_state = TS_AT_BOTTOM;
		pev->angles = m_start;
		m_targetState = TS_AT_TOP;
	}
	else
	{
		UTIL_SetOrigin(this, m_vecPosition1);
		m_toggle_state = TS_AT_TOP;
		pev->angles = m_end;
		m_targetState = TS_AT_BOTTOM;
	}

	EnableUse();
	pev->nextthink = pev->ltime + 2.0;
	SetThink(&CFuncTrackChange::Find);
	Precache();
}

void CFuncTrackChange::Precache()
{
	// Can't trigger sound
	PRECACHE_SOUND("buttons/button11.wav");

	CFuncPlatRot::Precache();
}

// UNDONE: Filter touches before re-evaluating the train.
void CFuncTrackChange::Touch(CBaseEntity* pOther)
{
#if 0
	TRAIN_CODE code;
	entvars_t *pevToucher = pOther->pev;
#endif
}

bool CFuncTrackChange::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "train"))
	{
		m_trainName = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "toptrack"))
	{
		m_trackTopName = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "bottomtrack"))
	{
		m_trackBottomName = ALLOC_STRING(pkvd->szValue);
		return true;
	}

	return CFuncPlatRot::KeyValue(pkvd); // Pass up to base class
}

void CFuncTrackChange::OverrideReset()
{
	pev->nextthink = pev->ltime + 1.0;
	SetThink(&CFuncTrackChange::Find);
}

void CFuncTrackChange::Find()
{
	// Find track entities
	CBaseEntity* pTarget;

	pTarget = UTIL_FindEntityByTargetname(NULL, STRING(m_trackTopName));
	if (pTarget && FClassnameIs(pTarget->pev, "path_track"))
	{
		m_trackTop = (CPathTrack*)pTarget;
		pTarget = UTIL_FindEntityByTargetname(NULL, STRING(m_trackBottomName));
		if (pTarget && FClassnameIs(pTarget->pev, "path_track"))
		{
			m_trackBottom = (CPathTrack*)pTarget;
			pTarget = UTIL_FindEntityByTargetname(NULL, STRING(m_trainName));
			if (pTarget && FClassnameIs(pTarget->pev, "func_tracktrain"))
			{
				m_train = (CFuncTrackTrain*)pTarget;
				Vector center = (pev->absmin + pev->absmax) * 0.5;
				m_trackBottom = m_trackBottom->Nearest(center);
				m_trackTop = m_trackTop->Nearest(center);
				UpdateAutoTargets(m_toggle_state);
				SetThink(NULL);
				return;
			}
			else
				ALERT(at_error, "Can't find train for track change! %s\n", STRING(m_trainName));
		}
		else
			ALERT(at_error, "Can't find bottom track for track change! %s\n", STRING(m_trackBottomName));
	}
	else
		ALERT(at_error, "Can't find top track for track change! %s\n", STRING(m_trackTopName));
}

TRAIN_CODE CFuncTrackChange::EvaluateTrain(CPathTrack* pcurrent)
{
	// Go ahead and work, we don't have anything to switch, so just be an elevator
	if (!pcurrent || !m_train)
		return TRAIN_SAFE;

	if (m_train->m_ppath == pcurrent || (pcurrent->m_pprevious && m_train->m_ppath == pcurrent->m_pprevious) ||
		(pcurrent->m_pnext && m_train->m_ppath == pcurrent->m_pnext))
	{
		if (m_train->pev->speed != 0)
			return TRAIN_BLOCKING;

		Vector dist = pev->origin - m_train->pev->origin;
		float length = dist.Length2D();
		if (length < m_train->m_length) // Empirically determined close distance
			return TRAIN_FOLLOWING;
		else if (length > (150 + m_train->m_length))
			return TRAIN_SAFE;

		return TRAIN_BLOCKING;
	}

	return TRAIN_SAFE;
}

void CFuncTrackChange::UpdateTrain(Vector& dest)
{
	float time;
	Vector vel = pev->velocity;
	if (m_pfnThink == &CFuncTrackChange::LinearMoveNow)
	{
		// we're going to do a LinearMoveNow: calculate the velocity it'll have
		Vector vecDest;
		if (m_pMoveWith)
			vecDest = m_vecFinalDest + m_pMoveWith->pev->origin;
		else
			vecDest = m_vecFinalDest;
		Vector vecDestDelta = vecDest - pev->origin;
		time = vecDestDelta.Length() / m_flLinearMoveSpeed;
		vel = vecDestDelta / time;
	}
	else
	{
		time = (pev->nextthink - pev->ltime);
	}

	m_train->pev->velocity = vel;
	m_train->pev->avelocity = pev->avelocity;
	m_train->NextThink(m_train->pev->ltime + time, false);

	// Attempt at getting the train to rotate properly around the origin of the trackchange
	if (time <= 0)
	{
		//		ALERT(at_console, "no time, set trainvel %f %f %f\n", m_train->pev->velocity.x, m_train->pev->velocity.y, m_train->pev->velocity.z);
		return;
	}

	Vector offset = m_train->pev->origin - pev->origin;
	Vector delta = dest - pev->angles;
	// Transform offset into local coordinates
	UTIL_MakeInvVectors(delta, gpGlobals);
	Vector local;
	local.x = DotProduct(offset, gpGlobals->v_forward);
	local.y = DotProduct(offset, gpGlobals->v_right);
	local.z = DotProduct(offset, gpGlobals->v_up);

	local = local - offset;
	m_train->pev->velocity = vel + (local * (1.0 / time));

	//	ALERT(at_console, "set trainvel %f %f %f\n", m_train->pev->velocity.x, m_train->pev->velocity.y, m_train->pev->velocity.z);
}

void CFuncTrackChange::GoDown()
{
	if (m_code == TRAIN_BLOCKING)
		return;

	// HitBottom may get called during CFuncPlat::GoDown(), so set up for that
	// before you call GoDown()

	UpdateAutoTargets(TS_GOING_DOWN);
	// If ROTMOVE, move & rotate
	if (FBitSet(pev->spawnflags, SF_TRACK_DONT_MOVE))
	{
		SetMoveDone(&CFuncTrackChange::CallHitBottom);
		m_toggle_state = TS_GOING_DOWN;
		AngularMove(m_start, pev->speed);
	}
	else
	{
		CFuncPlat::GoDown();
		SetMoveDone(&CFuncTrackChange::CallHitBottom);

		Vector vecDest;
		if (m_pMoveWith)
		{
			vecDest = m_vecFinalDest + m_pMoveWith->pev->origin;
		}
		else
			vecDest = m_vecFinalDest;
		Vector vecDestDelta = vecDest - pev->origin;
		float flTravelTime = vecDestDelta.Length() / m_flLinearMoveSpeed;

		RotMove(m_start, flTravelTime);
		//		RotMove( m_start, pev->nextthink - pev->ltime );
	}
	// Otherwise, rotate first, move second

	// If the train is moving with the platform, update it
	if (m_code == TRAIN_FOLLOWING)
	{
		UpdateTrain(m_start);
		m_train->m_ppath = NULL;
	}
}

//
// Platform is at bottom, now starts moving up
//
void CFuncTrackChange::GoUp()
{
	if (m_code == TRAIN_BLOCKING)
		return;

	// HitTop may get called during CFuncPlat::GoUp(), so set up for that
	// before you call GoUp();

	UpdateAutoTargets(TS_GOING_UP);
	if (FBitSet(pev->spawnflags, SF_TRACK_DONT_MOVE))
	{
		SetMoveDone(&CFuncTrackChange::CallHitTop);
		m_toggle_state = TS_GOING_UP;
		AngularMove(m_end, pev->speed);
	}
	else
	{
		// If ROTMOVE, move & rotate
		CFuncPlat::GoUp();
		SetMoveDone(&CFuncTrackChange::CallHitTop);

		Vector vecDest;
		if (m_pMoveWith)
		{
			vecDest = m_vecFinalDest + m_pMoveWith->pev->origin;
		}
		else
			vecDest = m_vecFinalDest;
		Vector vecDestDelta = vecDest - pev->origin;
		float flTravelTime = vecDestDelta.Length() / m_flLinearMoveSpeed;

		RotMove(m_end, flTravelTime);
		//		RotMove( m_end, pev->nextthink - pev->ltime );
	}

	// Otherwise, move first, rotate second

	// If the train is moving with the platform, update it
	if (m_code == TRAIN_FOLLOWING)
	{
		UpdateTrain(m_end);
		m_train->m_ppath = NULL;
	}
}

// Normal track change
void CFuncTrackChange::UpdateAutoTargets(int toggleState)
{
	if (!m_trackTop || !m_trackBottom)
		return;

	if (toggleState == TS_AT_TOP)
		ClearBits(m_trackTop->pev->spawnflags, SF_PATH_DISABLED);
	else
		SetBits(m_trackTop->pev->spawnflags, SF_PATH_DISABLED);

	if (toggleState == TS_AT_BOTTOM)
		ClearBits(m_trackBottom->pev->spawnflags, SF_PATH_DISABLED);
	else
		SetBits(m_trackBottom->pev->spawnflags, SF_PATH_DISABLED);

	UpdateTrain(pev->origin); // fix now is func_trackchange BUG. G-Cont
}

void CFuncTrackChange::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (m_toggle_state != TS_AT_TOP && m_toggle_state != TS_AT_BOTTOM)
		return;

	// If train is in "safe" area, but not on the elevator, play alarm sound
	if (m_toggle_state == TS_AT_TOP)
		m_code = EvaluateTrain(m_trackTop);
	else if (m_toggle_state == TS_AT_BOTTOM)
		m_code = EvaluateTrain(m_trackBottom);
	else
		m_code = TRAIN_BLOCKING;
	if (m_code == TRAIN_BLOCKING)
	{
		// Play alarm and return
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "buttons/button11.wav", 1, ATTN_NORM);
		return;
	}

	// Otherwise, it's safe to move
	// If at top, go down
	// at bottom, go up

	DisableUse();
	if (m_toggle_state == TS_AT_TOP)
		GoDown();
	else
		GoUp();
}

//
// Platform has hit bottom.  Stops and waits forever.
//
void CFuncTrackChange::HitBottom()
{
	CFuncPlatRot::HitBottom();
	if (m_code == TRAIN_FOLLOWING)
	{
		//		UpdateTrain();
		m_train->SetTrack(m_trackBottom);
	}
	SetThink(NULL);
	pev->nextthink = -1;

	UpdateAutoTargets(m_toggle_state);

	EnableUse();
}

//
// Platform has hit bottom.  Stops and waits forever.
//
void CFuncTrackChange::HitTop()
{
	CFuncPlatRot::HitTop();
	if (m_code == TRAIN_FOLLOWING)
	{
		//		UpdateTrain();
		m_train->SetTrack(m_trackTop);
	}

	// Don't let the plat go back down
	SetThink(NULL);
	pev->nextthink = -1;
	UpdateAutoTargets(m_toggle_state);
	EnableUse();
}
