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

#include "entities/CBaseMonster.h"

class CFlockingFlyerFlock : public CBaseMonster
{
public:
    void Spawn(void) override;
    void Precache(void) override;
    void KeyValue(KeyValueData* pkvd) override;
    void SpawnFlock(void);

    int Save(CSave& save) override;
    int Restore(CRestore& restore) override;
    static TYPEDESCRIPTION m_SaveData[];

    // Sounds are shared by the flock
    static void PrecacheFlockSounds(void);

    int m_cFlockSize;
    float m_flFlockRadius;
};
