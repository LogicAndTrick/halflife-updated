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

#include "entities/base/CPointEntity.h"

#define SF_FOG_ACTIVE 1
#define SF_FOG_FADING 0x8000

//=========================================================
// LRC - env_fog, extended a bit from the DMC version
//=========================================================
class CEnvFog : public CPointEntity
{
public:
    void Spawn(void) override;
    void Precache(void) override;
    void DLLEXPORT ResumeThink(void);
    void DLLEXPORT Resume2Think(void);
    void DLLEXPORT TurnOn(void);
    void DLLEXPORT TurnOff(void);
    void DLLEXPORT FadeInDone(void);
    void DLLEXPORT FadeOutDone(void);
    void SendData(Vector col, int fFadeTime, int StartDist, int iEndDist);
    void KeyValue(KeyValueData* pkvd) override;
    int Save(CSave& save) override;
    int Restore(CRestore& restore) override;
    static TYPEDESCRIPTION m_SaveData[];
    void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;

    STATE GetState(void) override;

    int m_iStartDist;
    int m_iEndDist;
    float m_iFadeIn;
    float m_iFadeOut;
    float m_fHoldTime;
    float m_fFadeStart; // if we're fading in/out, then when did the fade start?
};
