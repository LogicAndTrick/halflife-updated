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

#include "CSquadMonster.h"

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define CONTROLLER_AE_HEAD_OPEN 1
#define CONTROLLER_AE_BALL_SHOOT 2
#define CONTROLLER_AE_SMALL_SHOOT 3
#define CONTROLLER_AE_POWERUP_FULL 4
#define CONTROLLER_AE_POWERUP_HALF 5

#define CONTROLLER_FLINCH_DELAY 2 // at most one flinch every n secs

class CController : public CSquadMonster
{
public:
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	void Spawn() override;
	void Precache() override;
	void SetYawSpeed() override;
	int Classify() override;
	void HandleAnimEvent(MonsterEvent_t* pEvent) override;

	void RunAI() override;
	bool CheckRangeAttack1(float flDot, float flDist) override; // balls
	bool CheckRangeAttack2(float flDot, float flDist) override; // head
	bool CheckMeleeAttack1(float flDot, float flDist) override; // block, throw
	Schedule_t* GetSchedule() override;
	Schedule_t* GetScheduleOfType(int Type) override;
	void StartTask(Task_t* pTask) override;
	void RunTask(Task_t* pTask) override;
	CUSTOM_SCHEDULES;

	void Stop() override;
	void Move(float flInterval) override;
	int CheckLocalMove(const Vector& vecStart, const Vector& vecEnd, CBaseEntity* pTarget, float* pflDist) override;
	void MoveExecute(CBaseEntity* pTargetEnt, const Vector& vecDir, float flInterval) override;
	void SetActivity(Activity NewActivity) override;
	bool ShouldAdvanceRoute(float flWaypointDist) override;
	int LookupFloat();

	float m_flNextFlinch;

	float m_flShootTime;
	float m_flShootEnd;

	void PainSound() override;
	void AlertSound() override;
	void IdleSound() override;
	void AttackSound();
	void DeathSound() override;

	static const char* pAttackSounds[];
	static const char* pIdleSounds[];
	static const char* pAlertSounds[];
	static const char* pPainSounds[];
	static const char* pDeathSounds[];

	bool TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;
	void Killed(entvars_t* pevInflictor, entvars_t* pevAttacker, int iGib) override;
	void GibMonster() override;

	CSprite* m_pBall[2];   // hand balls
	int m_iBall[2];		   // how bright it should be
	float m_iBallTime[2];  // when it should be that color
	int m_iBallCurrent[2]; // current brightness

	Vector m_vecEstVelocity;

	Vector m_velocity;
	bool m_fInCombat;
};

//=========================================================
// Controller bouncy ball attack
//=========================================================
class CControllerHeadBall : public CBaseMonster
{
	void Spawn() override;
	void Precache() override;
	void EXPORT HuntThink();
	void EXPORT DieThink();
	void EXPORT BounceTouch(CBaseEntity* pOther);
	void MovetoTarget(Vector vecTarget);
	void Crawl();
	int m_iTrail;
	int m_flNextAttack;
	Vector m_vecIdeal;
	EHANDLE m_hOwner;
};

class CControllerZapBall : public CBaseMonster
{
	void Spawn() override;
	void Precache() override;
	void EXPORT AnimateThink();
	void EXPORT ExplodeTouch(CBaseEntity* pOther);

	EHANDLE m_hOwner;
};
