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

// Beam maker
#define BEAM_FSINE        0x10
#define BEAM_FSOLID       0x20
#define BEAM_FSHADEIN     0x40
#define BEAM_FSHADEOUT    0x80

#define SF_LBEAM_SHADEIN  128
#define SF_LBEAM_SHADEOUT 256
#define SF_LBEAM_SOLID    512
#define SF_LBEAM_SINE     1024

class CLocusBeam : public CPointEntity
{
public:
    void Spawn(void) override;
    void Precache(void) override;
    void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;

    void KeyValue(KeyValueData* pkvd) override;
    int Save(CSave& save) override;
    int Restore(CRestore& restore) override;

    static TYPEDESCRIPTION m_SaveData[];

    int m_iszSprite;
    int m_iszTargetName;
    int m_iszStart;
    int m_iszEnd;
    int m_iWidth;
    int m_iDistortion;
    float m_fFrame;
    int m_iScrollRate;
    float m_fDuration;
    float m_fDamage;
    int m_iDamageType;
    int m_iFlags;
};
