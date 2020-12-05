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

#include "CBaseEntity.h"

//
// generic Delay entity.
//
class CBaseDelay : public CBaseEntity
{
public:
    float m_flDelay;
    int m_iszKillTarget;
    EHANDLE m_hActivator; //LRC - moved here from CBaseToggle

    void KeyValue(KeyValueData* pkvd) override;

    static TYPEDESCRIPTION m_SaveData[];
    int Save(CSave& save) override;
    int Restore(CRestore& restore) override;

    // common member functions
    void SUB_UseTargets(CBaseEntity* pActivator, USE_TYPE useType, float value);
    void DLLEXPORT DelayThink(void);
};
