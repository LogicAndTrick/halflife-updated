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

// AJH Inventory items, some of which can be manually used, others automatically
// and are here just so the player knows they have it/them.
//This  has been moved from weapons.h
#define ITEM_HEALTHKIT   1
#define ITEM_ANTIDOTE    2
#define ITEM_ANTIRAD     3
#define ITEM_SECURITY    4
#define ITEM_LONGJUMP    5
#define ITEM_FLARE       6
#define ITEM_CAMERA      7
//AJH extended inventory (rename me when I'm implemented)
#define ITEM_SLOT8       8
#define ITEM_SLOT9       9
#define ITEM_SLOT10     10

class CItem : public CBaseEntity
{
public:
    void Spawn(void) override;
    CBaseEntity* Respawn(void) override;
    void EXPORT ItemTouch(CBaseEntity* pOther);
    void EXPORT Materialize(void);
    virtual BOOL MyTouch(CBasePlayer* pPlayer) { return FALSE; };
};
