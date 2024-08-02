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

#define SF_WALL_START_OFF 0x0001

class CFuncWallToggle : public CFuncWall
{
public:
	void Spawn() override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	void TurnOff();
	void TurnOn();
	bool IsOn();
	STATE GetState() override { return (pev->solid == SOLID_NOT) ? STATE_OFF : STATE_ON; };
};
