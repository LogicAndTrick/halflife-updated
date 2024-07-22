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

#define SF_MULTIMAN_CLONE 0x80000000
#define SF_MULTIMAN_SAMETRIG 0x40000000
#define SF_MULTIMAN_TRIGCHOSEN 0x20000000

#define SF_MULTIMAN_THREAD 0x00000001
#define SF_MULTIMAN_LOOP 0x00000004
#define SF_MULTIMAN_ONLYONCE 0x00000008
#define SF_MULTIMAN_SPAWNFIRE 0x00000010
#define SF_MULTIMAN_DEBUG 0x00000020

#define MM_MODE_CHOOSE 1
#define MM_MODE_PERCENT 2
#define MM_MODE_SIMULTANEOUS 3

//**********************************************************
// The Multimanager Entity - when fired, will fire up to 16 targets
// at specified times.
// FLAG:		THREAD (create clones when triggered)
// FLAG:		CLONE (this is a clone for a threaded execution)
class CMultiManager : public CBaseEntity // Toggle
{
public:
	bool KeyValue(KeyValueData* pkvd) override;
	void Spawn() override;
	void EXPORT UseThink();
	void EXPORT ManagerThink();
	void EXPORT ManagerUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

#if _DEBUG
	void EXPORT ManagerReport();
#endif

	bool HasTarget(string_t targetname) override;

	int ObjectCaps() override { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];

	STATE m_iState;
	STATE GetState() override { return m_iState; };

	int m_cTargets;							  // the total number of targets in this manager's fire list.
	int m_index;							  // Current target
	float m_startTime;						  // Time we started firing
	int m_iTargetName[MAX_MULTI_TARGETS];	  // list if indexes into global string array
	float m_flTargetDelay[MAX_MULTI_TARGETS]; // delay (in seconds) from time of manager fire to target fire

	float m_flWait;		  // LRC- minimum length of time to wait
	float m_flMaxWait;	  // LRC- random, maximum length of time to wait
	string_t m_sMaster;	  // LRC- master
	int m_iMode;		  // LRC- 0 = timed, 1 = pick random, 2 = each random
	int m_iszThreadName;  // LRC
	int m_iszLocusThread; // LRC

	bool CalcNumber(CBaseEntity* pLocus, float* OUTresult) override
	{
		if (m_startTime > 0)
		{
			*OUTresult = gpGlobals->time - m_startTime;
			return true;
		}
		else
		{
			return false;
		}
	}

	EHANDLE m_hActivator;

private:
	USE_TYPE m_triggerType; // LRC
	inline bool IsClone() { return (pev->spawnflags & SF_MULTIMAN_CLONE) != 0; }
	inline bool ShouldClone()
	{
		if (IsClone())
			return false;

		return (pev->spawnflags & SF_MULTIMAN_THREAD) != 0;
	}

	CMultiManager* Clone();
};
