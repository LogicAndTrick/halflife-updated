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

#pragma once

#include "CBaseEntity.h"

#define GERMAN_GIB_COUNT 4
#define HUMAN_GIB_COUNT 6
#define ALIEN_GIB_COUNT 4

//
// A gib is a chunk of a body, or a piece of wood/metal/rocks/etc.
//
class CGib : public CBaseEntity
{
public:
	void Spawn(const char* szGibModel);
	void EXPORT BounceGibTouch(CBaseEntity* pOther);
	void EXPORT StickyGibTouch(CBaseEntity* pOther);
	void EXPORT WaitTillLand();
	void LimitVelocity();

	int ObjectCaps() override { return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | FCAP_DONT_SAVE; }
	static void SpawnHeadGib(entvars_t* pevVictim);
	static void SpawnHeadGib(entvars_t* pevVictim, const char* szGibModel);
	static void SpawnRandomGibs(entvars_t* pevVictim, int cGibs, bool human);
	static void SpawnRandomGibs(entvars_t* pevVictim, int cGibs, bool notfirst, const char* szGibModel); // LRC
	static void SpawnStickyGibs(entvars_t* pevVictim, Vector vecOrigin, int cGibs);

	int m_bloodColor;
	int m_cBloodDecals;
	int m_material;
	float m_lifeTime;
};
