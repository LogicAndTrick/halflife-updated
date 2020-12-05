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

#include "entities/CBaseAnimating.h"

#define SF_ENVMODEL_OFF          1
#define SF_ENVMODEL_DROPTOFLOOR  2
#define SF_ENVMODEL_SOLID        4

//=================================================================
// env_model: like env_sprite, except you can specify a sequence.
//=================================================================
class CEnvModel : public CBaseAnimating
{
    void Spawn(void) override;
    void Precache(void) override;
    void DLLEXPORT Think(void) override;
    void KeyValue(KeyValueData* pkvd) override;
    STATE GetState(void) override;
    void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
    int ObjectCaps(void) override { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

    int Save(CSave& save) override;
    int Restore(CRestore& restore) override;
    static TYPEDESCRIPTION m_SaveData[];

    void SetSequence(void);

    string_t m_iszSequence_On;
    string_t m_iszSequence_Off;
    int m_iAction_On;
    int m_iAction_Off;
};
