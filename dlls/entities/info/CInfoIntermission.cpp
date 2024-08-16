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

#include "CInfoIntermission.h"

LINK_ENTITY_TO_CLASS(info_intermission, CInfoIntermission);

void CInfoIntermission::Spawn()
{
	UTIL_SetOrigin(this, pev->origin);
	pev->solid = SOLID_NOT;
	pev->effects = EF_NODRAW;
	pev->v_angle = g_vecZero;

	SetNextThink(2); // let targets spawn!
}

void CInfoIntermission::Think()
{
	CBaseEntity* pTarget;

	// find my target
	pTarget = UTIL_FindEntityByTargetname(NULL, STRING(pev->target));

	if (pTarget)
	{
		pev->v_angle = UTIL_VecToAngles((pTarget->pev->origin - pev->origin).Normalize());
		pev->v_angle.x = -pev->v_angle.x;
	}
}
