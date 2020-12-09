/***
*
*    Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*
*    This product contains software technology licensed from Id
*    Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*    All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
#pragma once

#include "entities/CBaseEntity.h"

#define SF_BRUSH_ROTATE_Y_AXIS             0 //!?! (LRC)
#define SF_BRUSH_ROTATE_INSTANT            1
#define SF_BRUSH_ROTATE_BACKWARDS          2
#define SF_BRUSH_ROTATE_Z_AXIS             4
#define SF_BRUSH_ROTATE_X_AXIS             8

#define SF_BRUSH_ACCDCC        16  // brush should accelerate and decelerate when toggled
#define SF_BRUSH_HURT          32  // rotating brush that inflicts pain based on rotation speed
#define SF_ROTATING_NOT_SOLID  64  // some special rotating objects are not solid.

#define SF_BRUSH_ROTATE_SMALLRADIUS      128
#define SF_BRUSH_ROTATE_MEDIUMRADIUS     256
#define SF_BRUSH_ROTATE_LARGERADIUS      512

// covering cheesy noise1, noise2, & noise3 fields so they make more sense (for rotating fans)
#define noiseStart   noise1
#define noiseStop    noise2
#define noiseRunning noise3

class CFuncRotating : public CBaseEntity
{
public:
    // basic functions
    void Spawn() override;
    void Precache() override;
    void DLLEXPORT SpinUp();
    void DLLEXPORT SpinDown();
    void KeyValue(KeyValueData* pkvd) override;
    void DLLEXPORT HurtTouch(CBaseEntity* pOther);
    void DLLEXPORT RotatingUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
    void DLLEXPORT WaitForStart(); //LRC - get round 1.1.0.8's bizarre behaviour on startup
    void DLLEXPORT Rotate();
    void RampPitchVol(int fUp);
    void Blocked(CBaseEntity* pOther) override;
    int ObjectCaps() override;
    int Save(CSave& save) override;
    int Restore(CRestore& restore) override;

    static TYPEDESCRIPTION m_SaveData[];

    float m_flFanFriction;
    float m_flAttenuation;
    float m_flVolume;
    float m_pitch;
    int m_sounds;

    EHANDLE m_hActivator; //AJH

    float m_fCurSpeed; //LRC - during spin-up and spin-down, this is
                       // the current speed factor (between 0 and 1).
                       // storing this here lets us avoid the hassle of deriving it
                       // from pev->avelocity.

    STATE m_iState; //LRC
    STATE GetState() override; //LRC
};
