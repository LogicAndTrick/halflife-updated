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

#define SF_BLOOD_RANDOM        0x0001
#define SF_BLOOD_STREAM        0x0002
#define SF_BLOOD_PLAYER        0x0004
#define SF_BLOOD_DECAL        0x0008

// Blood effects
class CBlood : public CPointEntity
{
public:
    void Spawn(void) override;
    void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
    void KeyValue(KeyValueData* pkvd) override;

    inline int Color(void) { return pev->impulse; }
    inline float BloodAmount(void) { return pev->dmg; }

    inline void SetColor(int color) { pev->impulse = color; }
    inline void SetBloodAmount(float amount) { pev->dmg = amount; }

    Vector Direction(CBaseEntity* pActivator); //LRC - added pActivator, for locus system
    Vector BloodPosition(CBaseEntity* pActivator);

private:
};
