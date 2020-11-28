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

#define SF_AUTO_FIREONCE    0x0001
#define SF_AUTO_FROMPLAYER  0x0002

// This trigger will fire when the level spawns (or respawns if not fire once)
// It will check a global state before firing.  It supports delay and killtargets
class CAutoTrigger : public CBaseDelay
{
public:
    void KeyValue(KeyValueData* pkvd) override;
    void Activate(void) override;
    void DesiredAction(void) override;

    int ObjectCaps(void) override { return CBaseDelay::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
    int Save(CSave& save) override;
    int Restore(CRestore& restore) override;

    static TYPEDESCRIPTION m_SaveData[];

private:
    int m_globalstate;
    USE_TYPE triggerType;
};
