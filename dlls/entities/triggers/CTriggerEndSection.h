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

#include "CBaseTrigger.h"

#define SF_ENDSECTION_USEONLY  0x0001

class CTriggerEndSection : public CBaseTrigger
{
public:
    void Spawn(void) override;
    void EXPORT EndSectionTouch(CBaseEntity* pOther);
    void KeyValue(KeyValueData* pkvd) override;
    void EXPORT EndSectionUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
};
