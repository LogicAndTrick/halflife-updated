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

#include "CBaseTrigger.h"

#define SF_HEVCHARGE_START_OFF 0x02
#define SF_HEVCHARGE_NOANNOUNCE 0x04

//=====================================
//
// trigger_hevcharge
// charges/discharges the player's suit. if the trigger has a targetname, firing it will toggle state
// LRC
class CTriggerHevCharge : public CBaseTrigger
{
public:
	void Spawn() override;
	void EXPORT ChargeTouch(CBaseEntity* pOther);
	void EXPORT AnnounceThink();
};
