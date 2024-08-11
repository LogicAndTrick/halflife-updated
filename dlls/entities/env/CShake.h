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

#define SF_SHAKE_EVERYONE 0x0001 // Don't check radius
// UNDONE: These don't work yet
#define SF_SHAKE_DISRUPT 0x0002 // Disrupt controls
#define SF_SHAKE_INAIR 0x0004	// Shake players in air

// Screen shake
class CShake : public CPointEntity
{
public:
	void Spawn() override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	bool KeyValue(KeyValueData* pkvd) override;

	inline float Amplitude() { return pev->scale; }
	inline float Frequency() { return pev->dmg_save; }
	inline float Duration() { return pev->dmg_take; }
	inline float Radius() { return pev->dmg; }

	inline void SetAmplitude(float amplitude) { pev->scale = amplitude; }
	inline void SetFrequency(float frequency) { pev->dmg_save = frequency; }
	inline void SetDuration(float duration) { pev->dmg_take = duration; }
	inline void SetRadius(float radius) { pev->dmg = radius; }

	STATE m_iState;									 // LRC
	STATE GetState() override { return m_iState; };	 // LRC
	void Think() override { m_iState = STATE_OFF; }; // LRC
private:
	// pev->scale is amplitude
	// pev->dmg_save is frequency
	// pev->dmg_take is duration
	// pev->dmg is radius
	// radius of 0 means all players
	// NOTE: UTIL_ScreenShake() will only shake players who are on the ground
};
