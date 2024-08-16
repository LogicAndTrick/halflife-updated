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

#include "CBaseMonster.h"

#define AFLOCK_MAX_RECRUIT_RADIUS 1024
#define AFLOCK_FLY_SPEED 125
#define AFLOCK_TURN_RATE 75
#define AFLOCK_ACCELERATE 10
#define AFLOCK_CHECK_DIST 192
#define AFLOCK_TOO_CLOSE 100
#define AFLOCK_TOO_FAR 256

class CFlockingFlyer : public CBaseMonster
{
public:
	void Spawn() override;
	void Precache() override;
	void SpawnCommonCode();
	void EXPORT IdleThink();
	void BoidAdvanceFrame();
	void EXPORT FormFlock();
	void EXPORT Start();
	void EXPORT FlockLeaderThink();
	void EXPORT FlockFollowerThink();
	void EXPORT FallHack();
	void MakeSound();
	void AlertFlock();
	void SpreadFlock();
	void SpreadFlock2();
	void Killed(entvars_t* pevInflictor, entvars_t* pevAttacker, int iGib) override;
	void Poop();
	bool FPathBlocked();
	// bool KeyValue( KeyValueData *pkvd );

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	bool IsLeader() { return m_pSquadLeader == this; }
	bool InSquad() { return m_pSquadLeader != NULL; }
	int SquadCount();
	void SquadRemove(CFlockingFlyer* pRemove);
	void SquadUnlink();
	void SquadAdd(CFlockingFlyer* pAdd);
	void SquadDisband();

	CFlockingFlyer* m_pSquadLeader;
	CFlockingFlyer* m_pSquadNext;
	bool m_fTurning;			  // is this boid turning?
	bool m_fCourseAdjust;		  // followers set this flag true to override flocking while they avoid something
	bool m_fPathBlocked;		  // true if there is an obstacle ahead
	Vector m_vecReferencePoint;	  // last place we saw leader
	Vector m_vecAdjustedVelocity; // adjusted velocity (used when fCourseAdjust is true)
	float m_flGoalSpeed;
	float m_flLastBlockedTime;
	float m_flFakeBlockedTime;
	float m_flAlertTime;
	float m_flFlockNextSoundTime;
};