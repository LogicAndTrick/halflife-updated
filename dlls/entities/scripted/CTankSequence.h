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

#define SF_TSEQ_DUMPPLAYER 1
#define SF_TSEQ_REPEATABLE 2

#define TSEQ_UNTIL_NONE 0
#define TSEQ_UNTIL_FACING 1
#define TSEQ_UNTIL_DEATH 2

#define TSEQ_TURN_NO 0
#define TSEQ_TURN_ANGLE 1
#define TSEQ_TURN_FACE 2
#define TSEQ_TURN_ENEMY 3

#define TSEQ_SHOOT_NO 0
#define TSEQ_SHOOT_ONCE 1
#define TSEQ_SHOOT_ALWAYS 2
#define TSEQ_SHOOT_FACING 3

#define TSEQ_FLAG_NOCHANGE 0
#define TSEQ_FLAG_ON 1
#define TSEQ_FLAG_OFF 2
#define TSEQ_FLAG_TOGGLE 3

class CFuncTank;

// LRC - Scripted Tank Sequence
class CTankSequence : public CBaseEntity
{
public:
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	void EndThink();
	void TimeOutThink();
	bool KeyValue(KeyValueData* pkvd) override;
	STATE GetState() override { return m_pTank ? STATE_ON : STATE_OFF; }
	int ObjectCaps() override;

	void StopSequence();
	void FacingNotify();
	void DeadEnemyNotify();

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	string_t m_iszEntity;
	string_t m_iszEnemy;
	int m_iUntil;
	float m_fDuration;
	int m_iTurn;
	int m_iShoot;
	int m_iActive;
	int m_iControllable;
	int m_iLaserSpot;
	CFuncTank* m_pTank; // the sequence can only control one tank at a time, for the moment
};
