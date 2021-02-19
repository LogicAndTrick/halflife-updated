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

#include "CBasePlayerWeapon.h"

#define EGON_PRIMARY_VOLUME      450
#define EGON_BEAM_SPRITE         "sprites/xbeam1.spr"
#define EGON_FLARE_SPRITE        "sprites/XSpark1.spr"
#define EGON_SOUND_OFF           "weapons/egon_off1.wav"
#define EGON_SOUND_RUN           "weapons/egon_run3.wav"
#define EGON_SOUND_STARTUP       "weapons/egon_windup2.wav"

#define EGON_SWITCH_NARROW_TIME  0.75   // Time it takes to switch fire modes
#define EGON_SWITCH_WIDE_TIME    1.5

class CEgon : public CBasePlayerWeapon
{
public:
#ifndef CLIENT_DLL
    int Save(CSave& save) override;
    int Restore(CRestore& restore) override;
    static TYPEDESCRIPTION m_SaveData[];
#endif

    void Spawn(void) override;
    void Precache(void) override;
    int GetItemInfo(ItemInfo* p) override;
    int AddToPlayer(CBasePlayer* pPlayer) override;

    BOOL Deploy(void) override;
    void Holster(int skiplocal = 0) override;

    void UpdateEffect(const Vector& startPoint, const Vector& endPoint, float timeBlend);

    void CreateEffect(void);
    void DestroyEffect(void);

    void EndAttack(void);
    void Attack(void);
    void PrimaryAttack(void) override;
    BOOL ShouldWeaponIdle() override { return TRUE; }
    void WeaponIdle(void) override;

    float m_flAmmoUseTime; // since we use < 1 point of ammo per update, we subtract ammo on a timer.

    float GetPulseInterval(void);
    float GetDischargeInterval(void);

    void Fire(const Vector& vecOrigSrc, const Vector& vecDir);

    BOOL HasAmmo(void);

    void UseAmmo(int count);

    enum EGON_FIREMODE { FIRE_NARROW, FIRE_WIDE };

    CBeam* m_pBeam;
    CBeam* m_pNoise;
    CSprite* m_pSprite;

    BOOL UseDecrement(void) override
    {
#if defined( CLIENT_WEAPONS )
        return TRUE;
#else
        return FALSE;
#endif
    }

    unsigned short m_usEgonStop;

private:
    float m_shootTime;
    EGON_FIREMODE m_fireMode;
    float m_shakeTime;
    BOOL m_deployed;

    unsigned short m_usEgonFire;
};
