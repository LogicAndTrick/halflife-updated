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

#define SF_CHANGEALIAS_RESOLVE 1
#define SF_CHANGEALIAS_DEBUG   2

/*********************
* Worldcraft entity: trigger_changealias
*
* target-     alias entity to affect
* netname-    value to change the alias to
**********************/
class CTriggerChangeAlias : public CBaseEntity
{
public:
    void Spawn(void) override;
    void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;

    int ObjectCaps(void) override { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
};
