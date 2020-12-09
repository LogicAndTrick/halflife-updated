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

#include "CBasePlayerAmmo.h"
#include "gamerules/CGameRules.h"
#include "entities/player/CBasePlayer.h"

extern int gEvilImpulse101;

void CBasePlayerAmmo::Spawn(void)
{
    pev->movetype = MOVETYPE_TOSS;
    pev->solid = SOLID_TRIGGER;
    UTIL_SetSize(pev, Vector(-16, -16, 0), Vector(16, 16, 16));
    UTIL_SetOrigin(this, pev->origin);

    SetTouch(&CBasePlayerAmmo::DefaultTouch);
}

void CBasePlayerAmmo::DefaultTouch(CBaseEntity* pOther)
{
    if (!pOther->IsPlayer())
    {
        return;
    }

    if (!UTIL_IsMasterTriggered(m_sMaster, m_pPlayer))    //
        return;                                        // AJH allows for locked weapons

    if (AddAmmo(pOther))
    {
        if (g_pGameRules->AmmoShouldRespawn(this) == GR_AMMO_RESPAWN_YES)
        {
            Respawn();
        }
        else
        {
            SetTouch(NULL);
            SetThink(&CBasePlayerAmmo::SUB_Remove);
            SetNextThink(0.1);
        }
        SUB_UseTargets(pOther, USE_TOGGLE, 0);    //AJH now ammo can trigger stuff too
    }
    else if (gEvilImpulse101)
    {
        // evil impulse 101 hack, kill always
        SetTouch(NULL);
        SetThink(&CBasePlayerAmmo::SUB_Remove);
        SetNextThink(0.1);
    }
}

CBaseEntity* CBasePlayerAmmo::Respawn(void)
{
    pev->effects |= EF_NODRAW;
    SetTouch(NULL);

    UTIL_SetOrigin(this, g_pGameRules->VecAmmoRespawnSpot(this));// move to wherever I'm supposed to repawn.

    SetThink(&CBasePlayerAmmo::Materialize);
    AbsoluteNextThink(g_pGameRules->FlAmmoRespawnTime(this));

    return this;
}

void CBasePlayerAmmo::Materialize(void)
{
    if (pev->effects & EF_NODRAW)
    {
        // changing from invisible state to visible.
        EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "items/suitchargeok1.wav", 1, ATTN_NORM, 0, 150);
        pev->effects &= ~EF_NODRAW;
        pev->effects |= EF_MUZZLEFLASH;
    }

    SetTouch(&CBasePlayerAmmo::DefaultTouch);
}
