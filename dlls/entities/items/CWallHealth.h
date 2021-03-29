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

#include "entities/CBaseToggle.h"

//-------------------------------------------------------------
// Wall mounted health kit
//-------------------------------------------------------------
class CWallHealth : public CBaseToggle
{
public:
    void Spawn() override;
    void Precache() override;
    void DLLEXPORT Off();
    void DLLEXPORT Recharge();
    void KeyValue(KeyValueData* pkvd) override;
    void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
    int ObjectCaps() override { return (CBaseToggle::ObjectCaps() | FCAP_CONTINUOUS_USE) & ~FCAP_ACROSS_TRANSITION; }
    int Save(CSave& save) override;
    int Restore(CRestore& restore) override;
    STATE GetState() override;

    static TYPEDESCRIPTION m_SaveData[];

    float m_flNextCharge;
    int m_iReactivate; // DeathMatch Delay until reactivated
    int m_iJuice;
    int m_iOn; // 0 = off, 1 = startup, 2 = going
    float m_flSoundTime;
};
