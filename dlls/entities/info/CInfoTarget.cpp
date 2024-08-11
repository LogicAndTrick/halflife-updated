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

#include "CInfoTarget.h"

LINK_ENTITY_TO_CLASS(info_target, CInfoTarget);

// Landmark class
void CInfoTarget::Spawn()
{
	// Precache();
	pev->solid = SOLID_NOT;
	if (pev->spawnflags & SF_TARGET_HACK_VISIBLE)
	{
		PRECACHE_MODEL("sprites/null.spr");
		SET_MODEL(ENT(pev), "sprites/null.spr");
		UTIL_SetSize(pev, g_vecZero, g_vecZero);
	}
}

void CInfoTarget::Precache()
{
	if (pev->spawnflags & SF_TARGET_HACK_VISIBLE)
		PRECACHE_MODEL("sprites/null.spr");
}
