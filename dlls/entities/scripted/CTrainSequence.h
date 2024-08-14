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

#define DIRECTION_NONE 0
#define DIRECTION_FORWARDS 1
#define DIRECTION_BACKWARDS 2
#define DIRECTION_STOP 3
#define DIRECTION_DESTINATION 4

#define SF_TRAINSEQ_REMOVE 2
#define SF_TRAINSEQ_DIRECT 4
#define SF_TRAINSEQ_DEBUG 8

class CFuncTrain;
class CFuncTrackTrain;

// LRC - scripted_trainsequence
class CTrainSequence : public CBaseEntity
{
public:
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	void EndThink();
	void TimeOutThink();
	bool KeyValue(KeyValueData* pkvd) override;
	STATE GetState() override { return (m_pTrain || m_pTrackTrain) ? STATE_ON : STATE_OFF; }
	int ObjectCaps() override;

	void StopSequence();
	void ArrivalNotify();

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	string_t m_iszEntity;
	string_t m_iszDestination;
	string_t m_iszTerminate;
	int m_iDirection;
	int m_iPostDirection;
	float m_fDuration;
	// at any given time, at most one of these pointers will be set.
	CFuncTrain* m_pTrain;
	CFuncTrackTrain* m_pTrackTrain;

	CBaseEntity* m_pDestination;
};
