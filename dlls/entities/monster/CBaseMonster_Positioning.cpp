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

#include "CBaseMonster.h"
#include "util/trace.h"

//=========================================================
// MakeIdealYaw - gets a yaw value for the caller that would
// face the supplied vector. Value is stuffed into the monster's
// ideal_yaw
//=========================================================
void CBaseMonster::MakeIdealYaw(Vector vecTarget)
{
	Vector vecProjection;

	// strafing monster needs to face 90 degrees away from its goal
	if (m_movementActivity == ACT_STRAFE_LEFT)
	{
		vecProjection.x = -vecTarget.y;
		vecProjection.y = vecTarget.x;

		pev->ideal_yaw = UTIL_VecToYaw(vecProjection - pev->origin);
	}
	else if (m_movementActivity == ACT_STRAFE_RIGHT)
	{
		vecProjection.x = vecTarget.y;
		vecProjection.y = vecTarget.x;

		pev->ideal_yaw = UTIL_VecToYaw(vecProjection - pev->origin);
	}
	else
	{
		pev->ideal_yaw = UTIL_VecToYaw(vecTarget - pev->origin);
	}
}

//=========================================================
// FlYawDiff - returns the difference ( in degrees ) between
// monster's current yaw and ideal_yaw
//
// Positive result is left turn, negative is right turn
//=========================================================
float CBaseMonster::FlYawDiff()
{
	float flCurrentYaw;

	flCurrentYaw = UTIL_AngleMod(pev->angles.y);

	if (flCurrentYaw == pev->ideal_yaw)
	{
		return 0;
	}


	return UTIL_AngleDiff(pev->ideal_yaw, flCurrentYaw);
}

//=========================================================
// Changeyaw - turns a monster towards its ideal_yaw
//=========================================================
float CBaseMonster::ChangeYaw(int yawSpeed)
{
	float ideal, current, move, speed;

	current = UTIL_AngleMod(pev->angles.y);
	ideal = pev->ideal_yaw;
	if (current != ideal)
	{
		float delta = gpGlobals->time - m_flLastYawTime;

		m_flLastYawTime = gpGlobals->time;

		if (delta > 0.25)
		{
			delta = 0.25;
		}

		speed = yawSpeed * delta * 2;
		move = ideal - current;

		if (ideal > current)
		{
			if (move >= 180)
				move = move - 360;
		}
		else
		{
			if (move <= -180)
				move = move + 360;
		}

		if (move > 0)
		{ // turning to the monster's left
			if (move > speed)
				move = speed;
		}
		else
		{ // turning to the monster's right
			if (move < -speed)
				move = -speed;
		}

		pev->angles.y = UTIL_AngleMod(current + move);

		// turn head in desired direction only if they have a turnable head
		if ((m_afCapability & bits_CAP_TURN_HEAD) != 0)
		{
			float yaw = pev->ideal_yaw - pev->angles.y;
			if (yaw > 180)
				yaw -= 360;
			if (yaw < -180)
				yaw += 360;
			// yaw *= 0.8;
			SetBoneController(0, yaw);
		}
	}
	else
		move = 0;

	return move;
}

//=========================================================
// VecToYaw - turns a directional vector into a yaw value
// that points down that vector.
//=========================================================
float CBaseMonster::VecToYaw(Vector vecDir)
{
	if (vecDir.x == 0 && vecDir.y == 0 && vecDir.z == 0)
		return pev->angles.y;

	return UTIL_VecToYaw(vecDir);
}

//=========================================================
// SetEyePosition
//
// queries the monster's model for $eyeposition and copies
// that vector to the monster's view_ofs
//
//=========================================================
void CBaseMonster::SetEyePosition()
{
	Vector vecEyePosition;
	void* pmodel = GET_MODEL_PTR(ENT(pev));

	GetEyePosition(pmodel, vecEyePosition);

	pev->view_ofs = vecEyePosition;

	if (pev->view_ofs == g_vecZero)
	{
		ALERT(at_aiconsole, "%s has no view_ofs!\n", STRING(pev->classname));
	}
}

//=========================================================
// BBoxIsFlat - check to see if the monster's bounding box
// is lying flat on a surface (traces from all four corners
// are same length.)
//=========================================================
bool CBaseMonster::BBoxFlat()
{
	TraceResult tr;
	Vector vecPoint;
	float flXSize, flYSize;
	float flLength;
	float flLength2;

	flXSize = pev->size.x / 2;
	flYSize = pev->size.y / 2;

	vecPoint.x = pev->origin.x + flXSize;
	vecPoint.y = pev->origin.y + flYSize;
	vecPoint.z = pev->origin.z;

	UTIL_TraceLine(vecPoint, vecPoint - Vector(0, 0, 100), ignore_monsters, ENT(pev), &tr);
	flLength = (vecPoint - tr.vecEndPos).Length();

	vecPoint.x = pev->origin.x - flXSize;
	vecPoint.y = pev->origin.y - flYSize;

	UTIL_TraceLine(vecPoint, vecPoint - Vector(0, 0, 100), ignore_monsters, ENT(pev), &tr);
	flLength2 = (vecPoint - tr.vecEndPos).Length();
	if (flLength2 > flLength)
	{
		return false;
	}
	flLength = flLength2;

	vecPoint.x = pev->origin.x - flXSize;
	vecPoint.y = pev->origin.y + flYSize;
	UTIL_TraceLine(vecPoint, vecPoint - Vector(0, 0, 100), ignore_monsters, ENT(pev), &tr);
	flLength2 = (vecPoint - tr.vecEndPos).Length();
	if (flLength2 > flLength)
	{
		return false;
	}
	flLength = flLength2;

	vecPoint.x = pev->origin.x + flXSize;
	vecPoint.y = pev->origin.y - flYSize;
	UTIL_TraceLine(vecPoint, vecPoint - Vector(0, 0, 100), ignore_monsters, ENT(pev), &tr);
	flLength2 = (vecPoint - tr.vecEndPos).Length();
	if (flLength2 > flLength)
	{
		return false;
	}
	flLength = flLength2;

	return true;
}

//=========================================================
// FacingIdeal - tells us if a monster is facing its ideal
// yaw. Created this function because many spots in the
// code were checking the yawdiff against this magic
// number. Nicer to have it in one place if we're gonna
// be stuck with it.
//=========================================================
bool CBaseMonster::FacingIdeal()
{
	if (fabs(FlYawDiff()) <= 0.006) //!!!BUGBUG - no magic numbers!!!
	{
		return true;
	}

	return false;
}

//=========================================================
// FInViewCone - returns true is the passed ent is in
// the caller's forward view cone. The dot product is performed
// in 2d, making the view cone infinitely tall.
//=========================================================
bool CBaseMonster::FInViewCone(CBaseEntity* pEntity)
{
	Vector2D vec2LOS;
	float flDot;

	UTIL_MakeVectors(pev->angles);

	vec2LOS = (pEntity->pev->origin - pev->origin).Make2D();
	vec2LOS = vec2LOS.Normalize();

	flDot = DotProduct(vec2LOS, gpGlobals->v_forward.Make2D());

	if (flDot > m_flFieldOfView)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//=========================================================
// FInViewCone - returns true is the passed vector is in
// the caller's forward view cone. The dot product is performed
// in 2d, making the view cone infinitely tall.
//=========================================================
bool CBaseMonster::FInViewCone(Vector* pOrigin)
{
	Vector2D vec2LOS;
	float flDot;

	UTIL_MakeVectors(pev->angles);

	vec2LOS = (*pOrigin - pev->origin).Make2D();
	vec2LOS = vec2LOS.Normalize();

	flDot = DotProduct(vec2LOS, gpGlobals->v_forward.Make2D());

	if (flDot > m_flFieldOfView)
	{
		return true;
	}
	else
	{
		return false;
	}
}
