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

#include "CBaseTurret.h"

class CTurret : public CBaseTurret
{
public:
    void Spawn() override;
    void Precache() override;
    // Think functions
    void SpinUpCall() override;
    void SpinDownCall() override;

    int Save(CSave& save) override;
    int Restore(CRestore& restore) override;

    static TYPEDESCRIPTION m_SaveData[];

    // other functions
    void Shoot(Vector& vecSrc, Vector& vecDirToEnemy) override;

private:
    int m_iStartSpin;
};