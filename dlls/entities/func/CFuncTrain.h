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

#include "CBasePlatTrain.h"

#define SF_TRAIN_WAIT_RETRIGGER 1
#define SF_TRAIN_SETORIGIN 2
#define SF_TRAIN_START_ON 4 // Train is initially moving
#define SF_TRAIN_PASSABLE 8 // Train is not solid -- used to make water trains
#define SF_TRAIN_REVERSE 0x800000

class CTrainSequence;

class CFuncTrain : public CBasePlatTrain
{
public:
	void Spawn() override;
	void Precache() override;
	void PostSpawn() override;
	void OverrideReset() override;

	void Blocked(CBaseEntity* pOther) override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	bool KeyValue(KeyValueData* pkvd) override;

	// LRC
	void StartSequence(CTrainSequence* pSequence);
	void StopSequence();
	CTrainSequence* m_pSequence;

	void EXPORT Wait();
	void EXPORT Next();
	void EXPORT ThinkDoNext();
	void EXPORT SoundSetup();

	STATE GetState() override { return m_iState; }

	void ThinkCorrection() override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	entvars_t* m_pevCurrentTarget;
	int m_sounds;
	// LRC - now part of CBaseEntity:	bool		m_activated;
	STATE m_iState;
	float m_fStoredThink;
	Vector m_vecAvelocity;
};
