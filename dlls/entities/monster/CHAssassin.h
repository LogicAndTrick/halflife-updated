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

//=========================================================
// monster-specific schedule types
//=========================================================
enum
{
	SCHED_ASSASSIN_EXPOSED = LAST_COMMON_SCHEDULE + 1, // cover was blown.
	SCHED_ASSASSIN_JUMP,							   // fly through the air
	SCHED_ASSASSIN_JUMP_ATTACK,						   // fly through the air and shoot
	SCHED_ASSASSIN_JUMP_LAND,						   // hit and run away
};

//=========================================================
// monster-specific tasks
//=========================================================
enum
{
	TASK_ASSASSIN_FALL_TO_GROUND = LAST_COMMON_TASK + 1, // falling and waiting to hit ground
};

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define ASSASSIN_AE_SHOOT1 1
#define ASSASSIN_AE_TOSS1 2
#define ASSASSIN_AE_JUMP 3

#define bits_MEMORY_BADJUMP (bits_MEMORY_CUSTOM1)

//=========================================================
// hassassin - Human assassin, fast and stealthy
//=========================================================
class CHAssassin : public CBaseMonster
{
public:
	void Spawn() override;
	void Precache() override;
	void SetYawSpeed() override;
	int Classify() override;
	int ISoundMask() override;
	void Shoot();
	void HandleAnimEvent(MonsterEvent_t* pEvent) override;
	Schedule_t* GetSchedule() override;
	Schedule_t* GetScheduleOfType(int Type) override;
	bool CheckMeleeAttack1(float flDot, float flDist) override; // jump
	// bool CheckMeleeAttack2 ( float flDot, float flDist );
	bool CheckRangeAttack1(float flDot, float flDist) override; // shoot
	bool CheckRangeAttack2(float flDot, float flDist) override; // throw grenade
	void StartTask(Task_t* pTask) override;
	void RunAI() override;
	void RunTask(Task_t* pTask) override;
	void DeathSound() override;
	void IdleSound() override;
	CUSTOM_SCHEDULES;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	float m_flLastShot;
	float m_flDiviation;

	float m_flNextJump;
	Vector m_vecJumpVelocity;

	float m_flNextGrenadeCheck;
	Vector m_vecTossVelocity;
	bool m_fThrowGrenade;

	int m_iTargetRanderamt;

	int m_iFrustration;

	int m_iShell;
};
