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
#include "entities/CBaseToggle.h"

#define SF_WATCHERRATIO_MANUALUPDATES  0x0001
#define SF_WATCHERRATIO_FIREONSTART    0x0002
#define SF_WATCHERRATIO_DEBUGMODE      0x0004

#define SF_WATCHERRATIO_FIRSTUPDATE    0x8000
#define SF_WATCHERRATIO_ON             0x10000

enum WatcherRatioComparison
{
    WRC_Equal,
    WRC_NotEqual,
    WRC_Greater,
    WRC_GreaterOrEqual,
    WRC_Less,
    WRC_LessOrEqual,
};

class CWatcherRatio : public CBaseToggle
{
public:
    void Spawn() override;
    void DLLEXPORT Think() override;
    void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
    STATE GetState() override { return (pev->spawnflags & SF_WATCHERRATIO_ON) ? STATE_ON : STATE_OFF; }
    int ObjectCaps() override { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
    bool CalcNumber(CBaseEntity* pLocus, float* OUTresult) override;

    void UpdateState(CBaseEntity* pLocus, bool mustTrigger);
};
