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

#include "CMonsterTarget.h"

LINK_ENTITY_TO_CLASS(monster_target, CMonsterTarget);

void CMonsterTarget::Spawn()
{
	if (pev->spawnflags & SF_MONSTERTARGET_OFF)
		pev->health = 0;
	else
		pev->health = 1; // Don't ignore me, I'm not dead. I'm quite well really. I think I'll go for a walk...
	SetBits(pev->flags, FL_MONSTER);
}

void CMonsterTarget::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (ShouldToggle(useType))
	{
		if (pev->health)
			pev->health = 0;
		else
			pev->health = 1;
	}
}
