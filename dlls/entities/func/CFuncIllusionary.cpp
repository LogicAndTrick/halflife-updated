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

#include "CFuncIllusionary.h"

LINK_ENTITY_TO_CLASS(func_illusionary, CFuncIllusionary);

bool CFuncIllusionary::KeyValue(KeyValueData* pkvd)
{
	// LRC- surely it just parses this automatically? pev values are handled by the engine.
	if (FStrEq(pkvd->szKeyName, "skin")) // skin is used for content type
	{
		pev->skin = atof(pkvd->szValue);
		return true;
	}

	return CBaseToggle::KeyValue(pkvd);
}

void CFuncIllusionary::Spawn()
{
	pev->angles = g_vecZero;
	pev->movetype = MOVETYPE_NONE;
	pev->solid = SOLID_NOT; // always solid_not
	SET_MODEL(ENT(pev), STRING(pev->model));

	// I'd rather eat the network bandwidth of this than figure out how to save/restore
	// these entities after they have been moved to the client, or respawn them ala Quake
	// Perhaps we can do this in deathmatch only.
	//	MAKE_STATIC(ENT(pev));
}
