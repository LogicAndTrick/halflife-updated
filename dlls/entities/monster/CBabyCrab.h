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

#include "CHeadCrab.h"

class CBabyCrab : public CHeadCrab
{
public:
	void Spawn() override;
	void Precache() override;
	void SetYawSpeed() override;
	float GetDamageAmount() override { return gSkillData.headcrabDmgBite * 0.3; }
	bool CheckRangeAttack1(float flDot, float flDist) override;
	Schedule_t* GetScheduleOfType(int Type) override;
	int GetVoicePitch() override { return PITCH_NORM + RANDOM_LONG(40, 50); }
	float GetSoundVolue() override { return 0.8; }
};
