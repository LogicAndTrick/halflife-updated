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

#include "CBaseButton.h"

// Rotating button (aka "lever")
class CRotButton : public CBaseButton
{
public:
	void Spawn() override;
	void PostSpawn() override {} // don't use the moveWith fix from CBaseButton
	bool KeyValue(KeyValueData* pkvd) override;
};
