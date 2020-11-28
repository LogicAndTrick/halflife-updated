/***
*
*    Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*
*    This product contains software technology licensed from Id
*    Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*    All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
#pragma once

#include "CBreakable.h"

class CPushable : public CBreakable
{
public:
    void Spawn() override;
    void Precache() override;
    void Touch(CBaseEntity* pOther) override;
    void Move(CBaseEntity* pMover, int push);
    void KeyValue(KeyValueData* pkvd) override;
    void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
    void DoRespawn() override; //AJH Fix for respawnable breakable pushables (BY HAWK777)
    void EXPORT StopSound();

    int ObjectCaps() override;

    int Save(CSave& save) override;
    int Restore(CRestore& restore) override;

    inline float MaxSpeed();

    // breakables use an overridden takedamage
    int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;

    static TYPEDESCRIPTION m_SaveData[];

    static const char* m_soundNames[3];
    int m_lastSound; // no need to save/restore, just keeps the same sound from playing twice in a row
    float m_maxSpeed;
    float m_soundTime;
};
