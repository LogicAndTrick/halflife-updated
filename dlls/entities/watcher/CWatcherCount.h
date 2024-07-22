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

#include "entities/CBaseEntity.h"
#include "CStateWatcher.h"

#define SF_WRCOUNT_FIRESTART 0x0001
#define SF_WRCOUNT_STARTED 0x8000

class CWatcherCount : public CBaseToggle // AJH Heavily rewritten as it didn't work
{
public:
	int m_iMode;

	bool KeyValue(KeyValueData* pkvd) override;
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	void Spawn() override;
	void EXPORT Think() override;
	STATE GetState() override { return (pev->spawnflags & SF_SWATCHER_VALID) ? STATE_ON : STATE_OFF; };
	int ObjectCaps() override { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	bool CalcNumber(CBaseEntity* pLocus, float* OUTresult) override
	{
		*OUTresult = pev->iuser1;
		return true;
	}
};
