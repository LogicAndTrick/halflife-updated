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

#include "CXenSpore.h"

LINK_ENTITY_TO_CLASS(xen_spore_small, CXenSporeSmall);
LINK_ENTITY_TO_CLASS(xen_spore_medium, CXenSporeMed);
LINK_ENTITY_TO_CLASS(xen_spore_large, CXenSporeLarge);
LINK_ENTITY_TO_CLASS(xen_hull, CXenHull);

void CXenSpore::Spawn()
{
	Precache();

	SET_MODEL(ENT(pev), pModelNames[pev->skin]);
	pev->movetype = MOVETYPE_NONE;
	pev->solid = SOLID_BBOX;
	pev->takedamage = DAMAGE_YES;

	//	SetActivity( ACT_IDLE );
	pev->sequence = 0;
	pev->frame = RANDOM_FLOAT(0, 255);
	pev->framerate = RANDOM_FLOAT(0.7, 1.4);
	ResetSequenceInfo();
	SetNextThink(RANDOM_FLOAT(0.1, 0.4)); // Load balance these a bit
}

const char* CXenSpore::pModelNames[] =
	{
		"models/fungus(small).mdl",
		"models/fungus.mdl",
		"models/fungus(large).mdl",
};

void CXenSpore::Precache()
{
	PRECACHE_MODEL((char*)pModelNames[pev->skin]);
}

void CXenSpore::Touch(CBaseEntity* pOther)
{
}

void CXenSpore::Think()
{
	StudioFrameAdvance();
	SetNextThink(0.1);
}

// -----------------
// -----------------
// -----------------

void CXenSporeSmall::Spawn()
{
	pev->skin = 0;
	CXenSpore::Spawn();
	UTIL_SetSize(pev, Vector(-16, -16, 0), Vector(16, 16, 64));
}
void CXenSporeMed::Spawn()
{
	pev->skin = 1;
	CXenSpore::Spawn();
	UTIL_SetSize(pev, Vector(-40, -40, 0), Vector(40, 40, 120));
}

// I just eyeballed these -- fill in hulls for the legs
const Vector CXenSporeLarge::m_hullSizes[] =
	{
		Vector(90, -25, 0),
		Vector(25, 75, 0),
		Vector(-15, -100, 0),
		Vector(-90, -35, 0),
		Vector(-90, 60, 0),
};

void CXenSporeLarge::Spawn()
{
	pev->skin = 2;
	CXenSpore::Spawn();
	UTIL_SetSize(pev, Vector(-48, -48, 110), Vector(48, 48, 240));

	Vector forward, right;

	UTIL_MakeVectorsPrivate(pev->angles, forward, right, NULL);

	// Rotate the leg hulls into position
	for (int i = 0; i < ARRAYSIZE(m_hullSizes); i++)
		CXenHull::CreateHull(this, Vector(-12, -12, 0), Vector(12, 12, 120), (m_hullSizes[i].x * forward) + (m_hullSizes[i].y * right));
}

// -----------------
// -----------------
// -----------------

CXenHull* CXenHull::CreateHull(CBaseEntity* source, const Vector& mins, const Vector& maxs, const Vector& offset)
{
	CXenHull* pHull = GetClassPtr((CXenHull*)NULL);

	UTIL_SetOrigin(pHull, source->pev->origin + offset);
	SET_MODEL(pHull->edict(), STRING(source->pev->model));
	pHull->pev->solid = SOLID_BBOX;
	pHull->pev->classname = MAKE_STRING("xen_hull");
	pHull->pev->movetype = MOVETYPE_NONE;
	pHull->pev->owner = source->edict();
	UTIL_SetSize(pHull->pev, mins, maxs);
	pHull->pev->renderamt = 0;
	pHull->pev->rendermode = kRenderTransTexture;
	//	pHull->pev->effects = EF_NODRAW;

	return pHull;
}