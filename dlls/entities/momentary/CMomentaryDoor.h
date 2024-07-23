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

#include "entities/CBaseToggle.h"

#define noiseMoving noise1
#define noiseArrived noise2

class CMomentaryDoor : public CBaseToggle
{
public:
	void Spawn() override;
	void Precache() override;
	void EXPORT MomentaryMoveDone();

	bool KeyValue(KeyValueData* pkvd) override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	int ObjectCaps() override { return CBaseToggle::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	void EXPORT DoorMoveDone();
	void EXPORT StopMoveSound();

	byte m_bMoveSnd; // sound a door makes while moving
	byte m_bStopSnd; // sound a door makes while moving

	STATE m_iState;
	float m_fLastPos;

	STATE GetState() override { return m_iState; }
	bool CalcNumber(CBaseEntity* pLocus, float* OUTresult) override
	{
		*OUTresult = m_fLastPos;
		return true;
	}
};
