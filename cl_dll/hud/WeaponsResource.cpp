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
* Valve LLC.  All other use, distribution, or modification is prohibited
* without written permission from Valve LLC.
*
****/

#include "CHud.h"
#include "WeaponsResource.h"
#include "HistoryResource.h"

client_sprite_t* GetSpriteList(client_sprite_t* pList, const char* psz, int iRes, int iCount);

WeaponsResource gWR;

void WeaponsResource::LoadAllWeaponSprites(void)
{
    for (int i = 0; i < MAX_WEAPONS; i++)
    {
        if (rgWeapons[i].iId)
            LoadWeaponSprites(&rgWeapons[i]);
    }
}

int WeaponsResource::CountAmmo(int iId)
{
    if (iId < 0)
        return 0;

    return riAmmo[iId];
}

int WeaponsResource::HasAmmo(WEAPON* p)
{
    if (!p)
        return FALSE;

    // weapons with no max ammo can always be selected
    if (p->iMax1 == -1)
        return TRUE;

    return (p->iAmmoType == -1) || p->iClip > 0 || CountAmmo(p->iAmmoType)
        || CountAmmo(p->iAmmo2Type) || (p->iFlags & WEAPON_FLAGS_SELECTONEMPTY);
}


void WeaponsResource::LoadWeaponSprites(WEAPON* pWeapon)
{
    int i, iRes;

    if (ScreenWidth < 640)
        iRes = 320;
    else
        iRes = 640;

    char sz[256];

    if (!pWeapon)
        return;

    memset(&pWeapon->rcActive, 0, sizeof(wrect_t));
    memset(&pWeapon->rcInactive, 0, sizeof(wrect_t));
    memset(&pWeapon->rcAmmo, 0, sizeof(wrect_t));
    memset(&pWeapon->rcAmmo2, 0, sizeof(wrect_t));
    pWeapon->hInactive = 0;
    pWeapon->hActive = 0;
    pWeapon->hAmmo = 0;
    pWeapon->hAmmo2 = 0;

    sprintf(sz, "sprites/%s.txt", pWeapon->szName);
    client_sprite_t* pList = SPR_GetList(sz, &i);

    if (!pList)
        return;

    client_sprite_t* p;

    p = GetSpriteList(pList, "crosshair", iRes, i);
    if (p)
    {
        sprintf(sz, "sprites/%s.spr", p->szSprite);
        pWeapon->hCrosshair = SPR_Load(sz);
        pWeapon->rcCrosshair = p->rc;
    }
    else
        pWeapon->hCrosshair = 0;

    p = GetSpriteList(pList, "autoaim", iRes, i);
    if (p)
    {
        sprintf(sz, "sprites/%s.spr", p->szSprite);
        pWeapon->hAutoaim = SPR_Load(sz);
        pWeapon->rcAutoaim = p->rc;
    }
    else
        pWeapon->hAutoaim = 0;

    p = GetSpriteList(pList, "zoom", iRes, i);
    if (p)
    {
        sprintf(sz, "sprites/%s.spr", p->szSprite);
        pWeapon->hZoomedCrosshair = SPR_Load(sz);
        pWeapon->rcZoomedCrosshair = p->rc;
    }
    else
    {
        pWeapon->hZoomedCrosshair = pWeapon->hCrosshair; //default to non-zoomed crosshair
        pWeapon->rcZoomedCrosshair = pWeapon->rcCrosshair;
    }

    p = GetSpriteList(pList, "zoom_autoaim", iRes, i);
    if (p)
    {
        sprintf(sz, "sprites/%s.spr", p->szSprite);
        pWeapon->hZoomedAutoaim = SPR_Load(sz);
        pWeapon->rcZoomedAutoaim = p->rc;
    }
    else
    {
        pWeapon->hZoomedAutoaim = pWeapon->hZoomedCrosshair; //default to zoomed crosshair
        pWeapon->rcZoomedAutoaim = pWeapon->rcZoomedCrosshair;
    }

    p = GetSpriteList(pList, "weapon", iRes, i);
    if (p)
    {
        sprintf(sz, "sprites/%s.spr", p->szSprite);
        pWeapon->hInactive = SPR_Load(sz);
        pWeapon->rcInactive = p->rc;

        gHR.iHistoryGap = V_max(gHR.iHistoryGap, pWeapon->rcActive.bottom - pWeapon->rcActive.top);
    }
    else
        pWeapon->hInactive = 0;

    p = GetSpriteList(pList, "weapon_s", iRes, i);
    if (p)
    {
        sprintf(sz, "sprites/%s.spr", p->szSprite);
        pWeapon->hActive = SPR_Load(sz);
        pWeapon->rcActive = p->rc;
    }
    else
        pWeapon->hActive = 0;

    p = GetSpriteList(pList, "ammo", iRes, i);
    if (p)
    {
        sprintf(sz, "sprites/%s.spr", p->szSprite);
        pWeapon->hAmmo = SPR_Load(sz);
        pWeapon->rcAmmo = p->rc;

        gHR.iHistoryGap = V_max(gHR.iHistoryGap, pWeapon->rcActive.bottom - pWeapon->rcActive.top);
    }
    else
        pWeapon->hAmmo = 0;

    p = GetSpriteList(pList, "ammo2", iRes, i);
    if (p)
    {
        sprintf(sz, "sprites/%s.spr", p->szSprite);
        pWeapon->hAmmo2 = SPR_Load(sz);
        pWeapon->rcAmmo2 = p->rc;

        gHR.iHistoryGap = V_max(gHR.iHistoryGap, pWeapon->rcActive.bottom - pWeapon->rcActive.top);
    }
    else
        pWeapon->hAmmo2 = 0;
}

// Returns the first weapon for a given slot.
WEAPON* WeaponsResource::GetFirstPos(int iSlot)
{
    WEAPON* pret = NULL;

    for (int i = 0; i < MAX_WEAPON_POSITIONS; i++)
    {
        if (rgSlots[iSlot][i] && HasAmmo(rgSlots[iSlot][i]))
        {
            pret = rgSlots[iSlot][i];
            break;
        }
    }

    return pret;
}


WEAPON* WeaponsResource::GetNextActivePos(int iSlot, int iSlotPos)
{
    if (iSlotPos >= MAX_WEAPON_POSITIONS || iSlot >= MAX_WEAPON_SLOTS)
        return NULL;

    WEAPON* p = gWR.rgSlots[iSlot][iSlotPos + 1];

    if (!p || !gWR.HasAmmo(p))
        return GetNextActivePos(iSlot, iSlotPos + 1);

    return p;
}


/* =================================
    GetSpriteList

Finds and returns the matching
sprite name 'psz' and resolution 'iRes'
in the given sprite list 'pList'
iCount is the number of items in the pList
================================= */
client_sprite_t* GetSpriteList(client_sprite_t* pList, const char* psz, int iRes, int iCount)
{
    if (!pList)
        return NULL;

    int i = iCount;
    client_sprite_t* p = pList;

    while (i--)
    {
        if ((!strcmp(psz, p->szName)) && (p->iRes == iRes))
            return p;
        p++;
    }

    return NULL;
}
