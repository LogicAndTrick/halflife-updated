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

#include "CBlood.h"
#include "util/locus.h"

LINK_ENTITY_TO_CLASS(env_blood, CBlood);


void CBlood::Spawn(void)
{
    pev->solid = SOLID_NOT;
    pev->movetype = MOVETYPE_NONE;
    pev->effects = 0;
    pev->frame = 0;
    SetMovedir(pev);
    if (Color() == 0) SetColor(BLOOD_COLOR_RED);
}


void CBlood::KeyValue(KeyValueData* pkvd)
{
    if (FStrEq(pkvd->szKeyName, "color"))
    {
        int color = atoi(pkvd->szValue);
        switch (color)
        {
        case 1:
            SetColor(BLOOD_COLOR_YELLOW);
            break;
        default:
            SetColor(BLOOD_COLOR_RED);
            break;
        }

        pkvd->fHandled = TRUE;
    }
    else if (FStrEq(pkvd->szKeyName, "amount"))
    {
        SetBloodAmount(atof(pkvd->szValue));
        pkvd->fHandled = TRUE;
    }
    else
        CPointEntity::KeyValue(pkvd);
}


Vector CBlood::Direction(CBaseEntity* pActivator)
{
    if (pev->spawnflags & SF_BLOOD_RANDOM)
        return UTIL_RandomBloodVector();
    else if (pev->netname)
        return CalcLocus_Velocity(this, pActivator, STRING(pev->netname));
    else
        return pev->movedir;
}


Vector CBlood::BloodPosition(CBaseEntity* pActivator)
{
    if (pev->spawnflags & SF_BLOOD_PLAYER)
    {
        edict_t* pPlayer;

        if (pActivator && pActivator->IsPlayer())
        {
            pPlayer = pActivator->edict();
        }
        else
            pPlayer = g_engfuncs.pfnPEntityOfEntIndex(1);
        if (pPlayer)
            return (pPlayer->v.origin + pPlayer->v.view_ofs) + Vector(RANDOM_FLOAT(-10, 10), RANDOM_FLOAT(-10, 10), RANDOM_FLOAT(-10, 10));
        // if no player found, fall through
    }
    else if (pev->target)
    {
        return CalcLocus_Position(this, pActivator, STRING(pev->target));
    }

    return pev->origin;
}


void CBlood::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
    if (pev->spawnflags & SF_BLOOD_STREAM)
        UTIL_BloodStream(BloodPosition(pActivator), Direction(pActivator), (Color() == BLOOD_COLOR_RED) ? 70 : Color(), BloodAmount());
    else
        UTIL_BloodDrips(BloodPosition(pActivator), Direction(pActivator), Color(), BloodAmount());

    if (pev->spawnflags & SF_BLOOD_DECAL)
    {
        Vector forward = Direction(pActivator);
        Vector start = BloodPosition(pActivator);
        TraceResult tr;

        UTIL_TraceLine(start, start + forward * BloodAmount() * 2, ignore_monsters, NULL, &tr);
        if (tr.flFraction != 1.0)
            UTIL_BloodDecalTrace(&tr, Color());
    }
}
