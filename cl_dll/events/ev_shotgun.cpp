/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

#include "ev_common.h"

void EV_FireShotGunDouble(event_args_t* args)
{
    int idx;
    Vector origin;
    Vector angles;
    Vector velocity;

    int j;
    Vector ShellVelocity;
    Vector ShellOrigin;
    int shell;
    Vector vecSrc, vecAiming;
    Vector vecSpread;
    Vector up, right, forward;
    float flSpread = 0.01;

    idx = args->entindex;
    VectorCopy(args->origin, origin);
    VectorCopy(args->angles, angles);
    VectorCopy(args->velocity, velocity);

    AngleVectors(angles, forward, right, up);

    shell = gEngfuncs.pEventAPI->EV_FindModelIndex("models/shotgunshell.mdl"); // brass shell

    if (EV_IsLocal(idx))
    {
        // Add muzzle flash to current weapon model
        EV_MuzzleFlash();
        gEngfuncs.pEventAPI->EV_WeaponAnimation(SHOTGUN_FIRE2, 2);
        V_PunchAxis(0, -10.0);
    }

    for (j = 0; j < 2; j++)
    {
        EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 32, -12, 6);

        EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], shell, TE_BOUNCE_SHOTSHELL);
    }

    gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/dbarrel1.wav", gEngfuncs.pfnRandomFloat(0.98, 1.0), ATTN_NORM, 0,
                                      85 + gEngfuncs.pfnRandomLong(0, 0x1f));

    EV_GetGunPosition(args, vecSrc, origin);
    VectorCopy(forward, vecAiming);

    if (gEngfuncs.GetMaxClients() > 1)
    {
        EV_HLDM_FireBullets(idx, forward, right, up, 8, vecSrc, vecAiming, 2048, BULLET_PLAYER_BUCKSHOT, 0, &tracerCount[idx - 1], 0.17365, 0.04362);
    }
    else
    {
        EV_HLDM_FireBullets(idx, forward, right, up, 12, vecSrc, vecAiming, 2048, BULLET_PLAYER_BUCKSHOT, 0, &tracerCount[idx - 1], 0.08716, 0.08716);
    }
}

void EV_FireShotGunSingle(event_args_t* args)
{
    int idx;
    Vector origin;
    Vector angles;
    Vector velocity;

    Vector ShellVelocity;
    Vector ShellOrigin;
    int shell;
    Vector vecSrc, vecAiming;
    Vector vecSpread;
    Vector up, right, forward;
    float flSpread = 0.01;

    idx = args->entindex;
    VectorCopy(args->origin, origin);
    VectorCopy(args->angles, angles);
    VectorCopy(args->velocity, velocity);

    AngleVectors(angles, forward, right, up);

    shell = gEngfuncs.pEventAPI->EV_FindModelIndex("models/shotgunshell.mdl"); // brass shell

    if (EV_IsLocal(idx))
    {
        // Add muzzle flash to current weapon model
        EV_MuzzleFlash();
        gEngfuncs.pEventAPI->EV_WeaponAnimation(SHOTGUN_FIRE, 2);

        V_PunchAxis(0, -5.0);
    }

    EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 32, -12, 6);

    EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], shell, TE_BOUNCE_SHOTSHELL);

    gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/sbarrel1.wav", gEngfuncs.pfnRandomFloat(0.95, 1.0), ATTN_NORM, 0,
                                      93 + gEngfuncs.pfnRandomLong(0, 0x1f));

    EV_GetGunPosition(args, vecSrc, origin);
    VectorCopy(forward, vecAiming);

    if (gEngfuncs.GetMaxClients() > 1)
    {
        EV_HLDM_FireBullets(idx, forward, right, up, 4, vecSrc, vecAiming, 2048, BULLET_PLAYER_BUCKSHOT, 0, &tracerCount[idx - 1], 0.08716, 0.04362);
    }
    else
    {
        EV_HLDM_FireBullets(idx, forward, right, up, 6, vecSrc, vecAiming, 2048, BULLET_PLAYER_BUCKSHOT, 0, &tracerCount[idx - 1], 0.08716, 0.08716);
    }
}
