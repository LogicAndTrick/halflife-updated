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

#include "entities/CBaseDelay.h"

#define SF_CAMERA_PLAYER_POSITION    1
#define SF_CAMERA_PLAYER_TARGET        2
#define SF_CAMERA_PLAYER_TAKECONTROL 4
#define SF_CAMERA_DRAWHUD            16

class CTriggerCamera : public CBaseDelay
{
public:
    void Spawn() override;
    void KeyValue(KeyValueData* pkvd) override;
    void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
    void DLLEXPORT FollowTarget();
    void Move();

    int Save(CSave& save) override;
    int Restore(CRestore& restore) override;
    int ObjectCaps() override { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
    static TYPEDESCRIPTION m_SaveData[];

    EHANDLE m_hPlayer;
    EHANDLE m_hTarget;
    CBaseEntity* m_pentPath;
    int m_sPath;
    float m_flWait;
    float m_flReturnTime;
    float m_flStopTime;
    float m_moveDistance;
    float m_targetSpeed;
    float m_initialSpeed;
    float m_acceleration;
    float m_deceleration;
    int m_state;
    int m_iszViewEntity;
};
