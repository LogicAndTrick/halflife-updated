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

#include "entities/CBaseEntity.h"

void DoSpark(entvars_t* pev, const Vector& location);

class CEnvSpark : public CBaseEntity
{
public:
    void Spawn() override;
    void Precache() override;
    void DLLEXPORT SparkThink();
    void DLLEXPORT SparkWait();
    void DLLEXPORT SparkCyclic(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
    void DLLEXPORT SparkStart(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
    void DLLEXPORT SparkStop(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
    void KeyValue(KeyValueData* pkvd) override;

    int Save(CSave& save) override;
    int Restore(CRestore& restore) override;

    static TYPEDESCRIPTION m_SaveData[];

    float m_flDelay;
    STATE m_iState; //LRC
    STATE GetState() override;
};
