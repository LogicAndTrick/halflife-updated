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
#include "entities/effects/CBeam.h"

class CApache : public CBaseMonster
{
    int Save(CSave& save) override;
    int Restore(CRestore& restore) override;
    static TYPEDESCRIPTION m_SaveData[];

    void Spawn(void) override;
    void Precache(void) override;
    int Classify(void) override { return CLASS_HUMAN_MILITARY; };
    int BloodColor(void) override { return DONT_BLEED; }
    void Killed(entvars_t* pevAttacker, int iGib) override;
    void GibMonster(void) override;

    void SetObjectCollisionBox(void) override
    {
        pev->absmin = pev->origin + Vector(-300, -300, -172);
        pev->absmax = pev->origin + Vector(300, 300, 8);
    }

    void EXPORT HuntThink(void);
    void EXPORT FlyTouch(CBaseEntity* pOther);
    void EXPORT CrashTouch(CBaseEntity* pOther);
    void EXPORT DyingThink(void);
    void EXPORT StartupUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
    void EXPORT NullThink(void);

    void ShowDamage(void);
    void Flight(void);
    void FireRocket(void);
    BOOL FireGun(void);

    int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;
    void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType) override;

    int m_iRockets;
    float m_flForce;
    float m_flNextRocket;

    Vector m_vecTarget;
    Vector m_posTarget;

    Vector m_vecDesired;
    Vector m_posDesired;

    Vector m_vecGoal;

    Vector m_angGun;
    float m_flLastSeen;
    float m_flPrevSeen;

    int m_iSoundState; // don't save this

    int m_iSpriteTexture;
    int m_iExplode;
    int m_iBodyGibs;

    float m_flGoalSpeed;

    int m_iDoSmokePuff;
    CBeam* m_pBeam;
};
