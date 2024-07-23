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

#define SF_ENVSTATE_START_ON 1
#define SF_ENVSTATE_DEBUG 2

//==================================================
// LRC- a simple entity, just maintains a state
//==================================================
class CEnvState : public CPointEntity
{
public:
	void Spawn() override;
	void Think() override;
	bool KeyValue(KeyValueData* pkvd) override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;

	bool IsLockedByMaster() { return !UTIL_IsMasterTriggered(m_sMaster, NULL); };

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	STATE GetState() override { return m_iState; }

	static TYPEDESCRIPTION m_SaveData[];

	STATE m_iState;
	float m_fTurnOnTime;
	float m_fTurnOffTime;
	int m_sMaster;
};
