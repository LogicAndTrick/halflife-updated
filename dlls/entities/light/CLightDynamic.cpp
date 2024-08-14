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

#include "CLightDynamic.h"

LINK_ENTITY_TO_CLASS(light_glow, CLightDynamic);

void CLightDynamic::Spawn()
{
	Precache();

	SET_MODEL(ENT(pev), "sprites/null.spr");
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;

	if (!(pev->spawnflags & SF_LIGHTDYNAMIC_START_OFF))
	{
		pev->health = 1;
		SetEffects();
	}
}

void CLightDynamic::Precache()
{
	PRECACHE_MODEL("sprites/null.spr");
}

void CLightDynamic::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (ShouldToggle(useType, pev->health))
	{
		if (pev->health)
			pev->health = 0;
		else
			pev->health = 1;
		SetEffects();
	}
}

void CLightDynamic::SetEffects()
{
	if (pev->health)
	{
		if (pev->frags == 2)
			pev->effects |= EF_BRIGHTLIGHT;
		else if (pev->frags)
			pev->effects |= EF_DIMLIGHT;

		if (pev->spawnflags & SF_LIGHTDYNAMIC_FLARE)
			pev->effects |= EF_LIGHT;
	}
	else
	{
		pev->effects &= ~(EF_DIMLIGHT | EF_BRIGHTLIGHT | EF_LIGHT);
	}
}

STATE CLightDynamic::GetState()
{
	if (pev->health)
		return STATE_ON;
	else
		return STATE_OFF;
}
