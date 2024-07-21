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

#include "CBabyCrab.h"

LINK_ENTITY_TO_CLASS(monster_babycrab, CBabyCrab);

void CBabyCrab::Spawn()
{
	CHeadCrab::Spawn();
	if (pev->model)
		SET_MODEL(ENT(pev), STRING(pev->model)); // LRC
	else
		SET_MODEL(ENT(pev), "models/baby_headcrab.mdl");
	pev->rendermode = kRenderTransTexture;
	pev->renderamt = 192;
	UTIL_SetSize(pev, Vector(-12, -12, 0), Vector(12, 12, 24));

	pev->health = gSkillData.headcrabHealth * 0.25; // less health than full grown
}

void CBabyCrab::Precache()
{
	if (pev->model)
		PRECACHE_MODEL((char*)STRING(pev->model)); // LRC
	else
		PRECACHE_MODEL("models/baby_headcrab.mdl");
	CHeadCrab::Precache();
}


void CBabyCrab::SetYawSpeed()
{
	pev->yaw_speed = 120;
}


bool CBabyCrab::CheckRangeAttack1(float flDot, float flDist)
{
	if ((pev->flags & FL_ONGROUND) != 0)
	{
		if (pev->groundentity && (pev->groundentity->v.flags & (FL_CLIENT | FL_MONSTER)) != 0)
			return true;

		// A little less accurate, but jump from closer
		if (flDist <= 180 && flDot >= 0.55)
			return true;
	}

	return false;
}


Schedule_t* CBabyCrab::GetScheduleOfType(int Type)
{
	switch (Type)
	{
	case SCHED_FAIL: // If you fail, try to jump!
		if (m_hEnemy != NULL)
			return slHCRangeAttack1Fast;
		break;

	case SCHED_RANGE_ATTACK1:
	{
		return slHCRangeAttack1Fast;
	}
	break;
	}

	return CHeadCrab::GetScheduleOfType(Type);
}
