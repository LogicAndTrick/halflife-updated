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

#include "entities/CPointEntity.h"

// LRC- force an info_target to use the sprite null.spr
#define SF_TARGET_HACK_VISIBLE 1

// LRC - make info_target an entity class in its own right
class CInfoTarget : public CPointEntity
{
public:
	void Spawn() override;
	void Precache() override;
};
