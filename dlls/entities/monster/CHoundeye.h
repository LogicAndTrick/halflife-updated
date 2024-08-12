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

// houndeye does 20 points of damage spread over a sphere 384 units in diameter, and each additional
// squad member increases the BASE damage by 110%, per the spec.
#define HOUNDEYE_MAX_SQUAD_SIZE 4
#define HOUNDEYE_MAX_ATTACK_RADIUS 384
#define HOUNDEYE_SQUAD_BONUS (float)1.1

// Marphy Fact Files Fix - Fix various instances of Houndeye not correctly blinking/closing eyes
#define HOUNDEYE_EYE_FRAMES 3 // how many different switchable maps for the eye

#define HOUNDEYE_SOUND_STARTLE_VOLUME 128 // how loud a sound has to be to badly scare a sleeping houndeye

//=========================================================
// monster-specific tasks
//=========================================================
enum
{
	TASK_HOUND_CLOSE_EYE = LAST_COMMON_TASK + 1,
	TASK_HOUND_OPEN_EYE,
	TASK_HOUND_THREAT_DISPLAY,
	TASK_HOUND_FALL_ASLEEP,
	TASK_HOUND_WAKE_UP,
	TASK_HOUND_HOP_BACK
};

//=========================================================
// monster-specific schedule types
//=========================================================
enum
{
	SCHED_HOUND_AGITATED = LAST_COMMON_SCHEDULE + 1,
	SCHED_HOUND_HOP_RETREAT,
	SCHED_HOUND_FAIL,
};

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define HOUND_AE_WARN 1
#define HOUND_AE_STARTATTACK 2
#define HOUND_AE_THUMP 3
#define HOUND_AE_ANGERSOUND1 4
#define HOUND_AE_ANGERSOUND2 5
#define HOUND_AE_HOPBACK 6
#define HOUND_AE_CLOSE_EYE 7

//=========================================================
// Houndeye - spooky sonic dog.
//=========================================================
class CHoundeye : public CSquadMonster
{
public:
	void Spawn() override;
	void Precache() override;
	int Classify() override;
	void HandleAnimEvent(MonsterEvent_t* pEvent) override;
	void SetYawSpeed() override;
	void WarmUpSound();
	void AlertSound() override;
	void DeathSound() override;
	void WarnSound();
	void PainSound() override;
	void IdleSound() override;
	void StartTask(Task_t* pTask) override;
	void RunTask(Task_t* pTask) override;
	void SonicAttack();
	void PrescheduleThink() override;
	void SetActivity(Activity NewActivity) override;
	void WriteBeamColor();
	bool CheckRangeAttack1(float flDot, float flDist) override;
	bool FValidateHintType(short sHint) override;
	bool FCanActiveIdle() override;
	Schedule_t* GetScheduleOfType(int Type) override;
	Schedule_t* GetSchedule() override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	CUSTOM_SCHEDULES;
	static TYPEDESCRIPTION m_SaveData[];

	int m_iSpriteTexture;
	bool m_fAsleep;			// some houndeyes sleep in idle mode if this is set, the houndeye is lying down
	bool m_fDontBlink;		// don't try to open/close eye if this bit is set!
	Vector m_vecPackCenter; // the center of the pack. The leader maintains this by averaging the origins of all pack members.
};
