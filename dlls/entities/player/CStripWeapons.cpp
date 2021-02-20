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

#include "CStripWeapons.h"
#include "CBasePlayer.h"
#include "gamerules/CGameRules.h"

LINK_ENTITY_TO_CLASS(player_weaponstrip, CStripWeapons);

TYPEDESCRIPTION CStripWeapons::m_SaveData[] =
{
    DEFINE_FIELD(CStripWeapons, m_i9mm, FIELD_INTEGER),
    DEFINE_FIELD(CStripWeapons, m_i357, FIELD_INTEGER),
    DEFINE_FIELD(CStripWeapons, m_iBuck, FIELD_INTEGER),
    DEFINE_FIELD(CStripWeapons, m_iBolt, FIELD_INTEGER),
    DEFINE_FIELD(CStripWeapons, m_iARGren, FIELD_INTEGER),
    DEFINE_FIELD(CStripWeapons, m_iRock, FIELD_INTEGER),
    DEFINE_FIELD(CStripWeapons, m_iUranium, FIELD_INTEGER),
    DEFINE_FIELD(CStripWeapons, m_iSatchel, FIELD_INTEGER),
    DEFINE_FIELD(CStripWeapons, m_iSnark, FIELD_INTEGER),
    DEFINE_FIELD(CStripWeapons, m_iTrip, FIELD_INTEGER),
    DEFINE_FIELD(CStripWeapons, m_iGren, FIELD_INTEGER),
    DEFINE_FIELD(CStripWeapons, m_iHornet, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CStripWeapons, CPointEntity);

void CStripWeapons::KeyValue(KeyValueData* pkvd)
{
    if (FStrEq(pkvd->szKeyName, "bullets"))
    {
        m_i9mm = atoi(pkvd->szValue);
        pkvd->fHandled = TRUE;
    }
    else if (FStrEq(pkvd->szKeyName, "magnum"))
    {
        m_i357 = atoi(pkvd->szValue);
        pkvd->fHandled = TRUE;
    }
    else if (FStrEq(pkvd->szKeyName, "shotgun"))
    {
        m_iBuck = atoi(pkvd->szValue);
        pkvd->fHandled = TRUE;
    }
    else if (FStrEq(pkvd->szKeyName, "crossbow"))
    {
        m_iBolt = atoi(pkvd->szValue);
        pkvd->fHandled = TRUE;
    }
    else if (FStrEq(pkvd->szKeyName, "argrenades"))
    {
        m_iARGren = atoi(pkvd->szValue);
        pkvd->fHandled = TRUE;
    }
    else if (FStrEq(pkvd->szKeyName, "rockets"))
    {
        m_iRock = atoi(pkvd->szValue);
        pkvd->fHandled = TRUE;
    }
    else if (FStrEq(pkvd->szKeyName, "uranium"))
    {
        m_iUranium = atoi(pkvd->szValue);
        pkvd->fHandled = TRUE;
    }
    else if (FStrEq(pkvd->szKeyName, "satchels"))
    {
        m_iSatchel = atoi(pkvd->szValue);
        pkvd->fHandled = TRUE;
    }
    else if (FStrEq(pkvd->szKeyName, "snarks"))
    {
        m_iSnark = atoi(pkvd->szValue);
        pkvd->fHandled = TRUE;
    }
    else if (FStrEq(pkvd->szKeyName, "tripmines"))
    {
        m_iTrip = atoi(pkvd->szValue);
        pkvd->fHandled = TRUE;
    }
    else if (FStrEq(pkvd->szKeyName, "handgrenades"))
    {
        m_iGren = atoi(pkvd->szValue);
        pkvd->fHandled = TRUE;
    }
    else if (FStrEq(pkvd->szKeyName, "hornetgun"))
    {
        m_iHornet = atoi(pkvd->szValue);
        pkvd->fHandled = TRUE;
    }
    else
    {
        CBaseEntity::KeyValue(pkvd);
    }
}

void SetAmmoStrip(WeaponStripInfo& info, const char* ammoName, const int amount)
{
    const auto index = CBasePlayer::GetAmmoIndex(ammoName);
    if (index < 0 || index > MAX_AMMO_SLOTS) return;
    info.ammoStrip[index] = amount;
}

void CStripWeapons::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
    CBasePlayer* pPlayer = nullptr;

    if (pActivator && pActivator->IsPlayer())
    {
        pPlayer = dynamic_cast<CBasePlayer*>(pActivator);
    }
    else if (!g_pGameRules->IsDeathmatch())
    {
        pPlayer = dynamic_cast<CBasePlayer*>(CBaseEntity::Instance(g_engfuncs.pfnPEntityOfEntIndex(1)));
    }

    if (!pPlayer) return;

    // Construct the weapon strip object
    auto strip = WeaponStripInfo();

    // Check spawnflags to see if the player keeps the weapons (or loses the suit)
    for (auto i = 0; i < MAX_WEAPONS; i++)
    {
        const auto flag = 1 << i;
        if (pev->spawnflags & flag)
        {
            if (i == 0) strip.removeSuit = true;
            else strip.weaponStrip[i] = false;
        }
    }

    // Set keyvalues for each ammo type
    SetAmmoStrip(strip, "9mm", m_i9mm);
    SetAmmoStrip(strip, "357", m_i357);
    SetAmmoStrip(strip, "buckshot", m_iBuck);
    SetAmmoStrip(strip, "bolts", m_iBolt);
    SetAmmoStrip(strip, "ARgrenades", m_iARGren);
    SetAmmoStrip(strip, "uranium", m_iUranium);
    SetAmmoStrip(strip, "rockets", m_iRock);
    SetAmmoStrip(strip, "Satchel Charge", m_iSatchel);
    SetAmmoStrip(strip, "Snarks", m_iSnark);
    SetAmmoStrip(strip, "Trip Mine", m_iTrip);
    SetAmmoStrip(strip, "Hand Grenade", m_iGren);
    SetAmmoStrip(strip, "Hornets", m_iHornet);

    // Remove the weapons/ammo from the player
    pPlayer->RemoveItems(strip);
}