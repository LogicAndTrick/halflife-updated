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

#include "CFuncWall.h"

// -------------------------------------------------------------------------------
//
// Monster only clip brush
//
// This brush will be solid for any entity who has the FL_MONSTERCLIP flag set
// in pev->flags
//
// otherwise it will be invisible and not solid.  This can be used to keep
// specific monsters out of certain areas
//
// -------------------------------------------------------------------------------
class CFuncMonsterClip : public CFuncWall
{
public:
	void Spawn() override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override {} // Clear out func_wall's use function
};
