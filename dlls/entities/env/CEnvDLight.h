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

#define SF_DLIGHT_ONLYONCE 1
#define SF_DLIGHT_STARTON 2

// LRC- env_dlight; Dynamic Entity Light creator
class CEnvDLight : public CPointEntity
{
public:
	void PostSpawn() override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	void Think() override;
	void DesiredAction() override;
	virtual void MakeLight(bool bActive);
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];
	STATE GetState() override
	{
		if (pev->spawnflags & SF_DLIGHT_STARTON)
			return STATE_ON;
		else
			return STATE_OFF;
	}

	Vector m_vecPos;
	int m_iKey;
	static int ms_iNextFreeKey;
};
