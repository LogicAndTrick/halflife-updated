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

#include "entities/CBaseEntity.h"

class CLaserSpot : public CBaseEntity
{
    void Spawn(void) override;
    void Precache(void) override;
    int ObjectCaps(void) override { return FCAP_DONT_SAVE; }

public:
    void Suspend(float flSuspendTime);
    void EXPORT Revive(void);

    static CLaserSpot* CreateSpot(void);
    static CLaserSpot* CreateSpot(const char* spritename);
};
