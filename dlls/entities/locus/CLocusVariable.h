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

class CLocusVariable : public CPointEntity
{
public:
	void Spawn() override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	bool CalcVelocity(CBaseEntity* pLocus, Vector* OUTresult) override
	{
		*OUTresult = pev->movedir;
		return true;
	}
	//	Vector	CalcAngles( CBaseEntity *pLocus ) { return UTIL_VecToAngles( CalcVelocity(pLocus) ); }
	bool CalcNumber(CBaseEntity* pLocus, float* OUTresult) override
	{
		*OUTresult = pev->frags;
		return true;
	}

	bool KeyValue(KeyValueData* pkvd) override;
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];

	int m_iszPosition;
	int m_iszVelocity;
	int m_iszRatio;
	int m_iszTargetName;
	int m_iszFireOnSpawn;
	float m_fDuration;
};

// Position/number marker
class CMark : public CPointEntity
{
public:
	bool CalcVelocity(CBaseEntity* pLocus, Vector* OUTresult) override
	{
		*OUTresult = pev->movedir;
		return true;
	}
	//	bool	CalcAngles( CBaseEntity *pLocus, Vector* OUTresult ) { return UTIL_VecToAngles( CalcVelocity(pLocus) ); }
	bool CalcNumber(CBaseEntity* pLocus, float* OUTresult) override
	{
		*OUTresult = pev->frags;
		return true;
	}
	void Think() override { SUB_Remove(); }
};
