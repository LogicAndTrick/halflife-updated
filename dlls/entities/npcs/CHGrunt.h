/***
*
* Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*
* This product contains software technology licensed from Id
* Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
* All Rights Reserved.
*
* Use, distribution, and modification of this source code and/or resulting
* object code is restricted to non-commercial enhancements to products from
* Valve LLC.All other use, distribution, or modification is prohibited
* without written permission from Valve LLC.
*
****/
#pragma once

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "squadmonster.h"

#define HEAD_GROUP      1
#define HEAD_GRUNT      0
#define HEAD_COMMANDER  1
#define HEAD_SHOTGUN    2
#define HEAD_M203       3

#define GUN_GROUP       2
#define GUN_MP5         0
#define GUN_SHOTGUN     1
#define GUN_NONE        2

//=========================================================
// hgrunt
//=========================================================
class CHGrunt : public CSquadMonster
{
public:
    void Spawn(void) override;
    void Precache(void) override;
    void SetYawSpeed(void) override;
    int Classify(void) override;
    int ISoundMask(void) override;
    void HandleAnimEvent(MonsterEvent_t* pEvent) override;
    BOOL FCanCheckAttacks(void) override;
    BOOL CheckMeleeAttack1(float flDot, float flDist) override;
    BOOL CheckRangeAttack1(float flDot, float flDist) override;
    BOOL CheckRangeAttack2(float flDot, float flDist) override;
    void CheckAmmo(void) override;
    void SetActivity(Activity NewActivity) override;
    void StartTask(Task_t* pTask) override;
    void RunTask(Task_t* pTask) override;
    void DeathSound(void) override;
    void PainSound(void) override;
    void IdleSound(void) override;
    Vector GetGunPosition(void) override;
    void Shoot(void);
    void Shotgun(void);
    void PrescheduleThink(void) override;
    void GibMonster(void) override;
    void SpeakSentence(void);

    int Save(CSave& save) override;
    int Restore(CRestore& restore) override;

    CBaseEntity* Kick(void);
    Schedule_t* GetSchedule(void) override;
    Schedule_t* GetScheduleOfType(int Type) override;
    void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType) override;
    int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;

    int IRelationship(CBaseEntity* pTarget) override;

    BOOL FOkToSpeak(void);
    void JustSpoke(void);

    CUSTOM_SCHEDULES;
    static TYPEDESCRIPTION m_SaveData[];

    // checking the feasibility of a grenade toss is kind of costly, so we do it every couple of seconds,
    // not every server frame.
    float m_flNextGrenadeCheck;
    float m_flNextPainTime;
    float m_flLastEnemySightTime;

    Vector m_vecTossVelocity;

    BOOL m_fThrowGrenade;
    BOOL m_fStanding;
    BOOL m_fFirstEncounter; // only put on the handsign show in the squad's first encounter.
    int m_cClipSize;

    int m_voicePitch;

    int m_iBrassShell;
    int m_iShotgunShell;

    int m_iSentence;

    static const char* pGruntSentences[];
};
