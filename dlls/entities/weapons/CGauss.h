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

#define GAUSS_PRIMARY_CHARGE_VOLUME  256 // how loud gauss is while charging
#define GAUSS_PRIMARY_FIRE_VOLUME    450 // how loud gauss is when discharged

class CGauss : public CBasePlayerWeapon
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

    void PrimaryAttack(void) override;
    void SecondaryAttack(void) override;
    void WeaponIdle(void) override;

    void StartFire(void);
    void Fire(Vector vecOrigSrc, Vector vecDirShooting, float flDamage);
    float GetFullChargeTime(void);
    int m_iBalls;
    int m_iGlow;
    int m_iBeam;
    int m_iSoundState; // don't save this

    // was this weapon just fired primary or secondary?
    // we need to know so we can pick the right set of effects. 
    BOOL m_fPrimaryFire;

    BOOL UseDecrement(void) override
    {
#if defined( CLIENT_WEAPONS )
        return TRUE;
#else
        return FALSE;
#endif
    }

private:
    unsigned short m_usGaussFire;
    unsigned short m_usGaussSpin;
};
