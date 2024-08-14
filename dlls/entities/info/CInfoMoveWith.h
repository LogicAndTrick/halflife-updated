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

#include "entities/CBaseEntity.h"

#define SF_IMW_INACTIVE 1
#define SF_IMW_BLOCKABLE 2

// LRC - info_movewith, the first entity I've made which
//       truly doesn't fit ANY preexisting category.
class CInfoMoveWith : public CBaseEntity
{
public:
	void Spawn() override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	int ObjectCaps() override { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	STATE GetState() override { return (pev->spawnflags & SF_IMW_INACTIVE) ? STATE_OFF : STATE_ON; }
};
