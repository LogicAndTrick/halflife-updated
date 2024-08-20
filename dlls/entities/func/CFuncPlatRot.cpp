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

#include "CFuncPlatRot.h"
#include "util/movewith.h"

LINK_ENTITY_TO_CLASS(func_platrot, CFuncPlatRot);

TYPEDESCRIPTION CFuncPlatRot::m_SaveData[] =
	{
		DEFINE_FIELD(CFuncPlatRot, m_end, FIELD_VECTOR),
		DEFINE_FIELD(CFuncPlatRot, m_start, FIELD_VECTOR),
};

IMPLEMENT_SAVERESTORE(CFuncPlatRot, CFuncPlat);

bool CFuncPlatRot::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "axes"))
	{
		UTIL_StringToVector((float*)(pev->movedir), pkvd->szValue);
		return true;
	}
	return CFuncPlat::KeyValue(pkvd);
}

void CFuncPlatRot::SetupRotation()
{
	if (m_vecFinalAngle.x != 0) // This plat rotates too!
	{
		CBaseToggle::AxisDir(pev);
		m_start = pev->angles;
		m_end = pev->angles + pev->movedir * m_vecFinalAngle.x;
	}
	else
	{
		m_start = g_vecZero;
		m_end = g_vecZero;
	}
	if (!FStringNull(pev->targetname)) // Start at top
	{
		UTIL_SetAngles(this, m_end);
		// pev->angles = m_end;
	}
}

void CFuncPlatRot::Spawn()
{
	CFuncPlat::Spawn();
	SetupRotation();
}

void CFuncPlatRot::GoDown()
{
	CFuncPlat::GoDown();
	//	RotMove( m_start, m_fNextThink - pev->ltime );   //G-Cont/ this fix for func_platrot
	// Ox, uj mne etot Laury!
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
}

//
// Platform has hit bottom.  Stops and waits forever.
//
void CFuncPlatRot::HitBottom()
{
	CFuncPlat::HitBottom();
	UTIL_SetAvelocity(this, g_vecZero);
	// pev->avelocity = g_vecZero;
	UTIL_SetAngles(this, m_start);
	// pev->angles = m_start;
}

//
// Platform is at bottom, now starts moving up
//
void CFuncPlatRot::GoUp()
{
	CFuncPlat::GoUp();
	//	RotMove( m_end, m_fNextThink - pev->ltime );

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
}

//
// Platform has hit top.  Pauses, then starts back down again.
//
void CFuncPlatRot::HitTop()
{
	CFuncPlat::HitTop();
	UTIL_SetAvelocity(this, g_vecZero);
	// pev->avelocity = g_vecZero;
	UTIL_SetAngles(this, m_end);
	// pev->angles = m_end;
}

void CFuncPlatRot::RotMove(Vector& destAngle, float time)
{
	// set destdelta to the vector needed to move
	Vector vecDestDelta = destAngle - pev->angles;

	// Travel time is so short, we're practically there already;  make it so.
	if (time >= 0.1)
	{
		UTIL_SetAvelocity(this, vecDestDelta / time);
		// pev->avelocity = vecDestDelta / time;
	}
	else
	{
		UTIL_SetAvelocity(this, vecDestDelta);
		// pev->avelocity = vecDestDelta;
		SetNextThink(1);
	}
}
