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

#include "entities/CBaseDelay.h"

#define SF_FREEZE_LOCUS 1
#define SF_DONTFREEZE 2
#define SF_ACTIVE 0x800000

// Trigger to disable a player
class CPlayerFreeze : public CBaseDelay
{
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	void Think() override;
	STATE GetState() override { return pev->spawnflags & SF_ACTIVE ? STATE_ON : STATE_OFF; }
};
