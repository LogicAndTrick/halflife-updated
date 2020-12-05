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

#include "entities/CBaseEntity.h"

// UNDONE: Save/restore this?  Don't forget to set classname and LINK_ENTITY_TO_CLASS()
// 
// OVERLOADS SOME ENTVARS:
//
// speed - the ideal magnitude of my velocity
class CCrossbowBolt : public CBaseEntity
{
    void Spawn(void) override;
    void Precache(void) override;
    int Classify(void) override;
    void DLLEXPORT BubbleThink(void);
    void DLLEXPORT BoltTouch(CBaseEntity* pOther);
    void DLLEXPORT ExplodeThink(void);

    int m_iTrail;

public:
    static CCrossbowBolt* BoltCreate(void);
};
