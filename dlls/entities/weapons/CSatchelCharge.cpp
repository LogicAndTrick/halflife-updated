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

#include "CSatchelCharge.h"
#include "util/skill.h"

enum satchel_e
{
    SATCHEL_IDLE1 = 0,
    SATCHEL_FIDGET1,
    SATCHEL_DRAW,
    SATCHEL_DROP
};

enum satchel_radio_e
{
    SATCHEL_RADIO_IDLE1 = 0,
    SATCHEL_RADIO_FIDGET1,
    SATCHEL_RADIO_DRAW,
    SATCHEL_RADIO_FIRE,
    SATCHEL_RADIO_HOLSTER
};

LINK_ENTITY_TO_CLASS(monster_satchel, CSatchelCharge);

//=========================================================
// Deactivate - do whatever it is we do to an orphaned 
// satchel when we don't want it in the world anymore.
//=========================================================
void CSatchelCharge::Deactivate()
{
    pev->solid = SOLID_NOT;
    UTIL_Remove(this);
}


void CSatchelCharge::Spawn()
{
    Precache();
    // motor
    pev->movetype = MOVETYPE_BOUNCE;
    pev->solid = SOLID_BBOX;

    SET_MODEL(ENT(pev), "models/w_satchel.mdl");
    //UTIL_SetSize(pev, Vector( -16, -16, -4), Vector(16, 16, 32));    // Old box -- size of headcrab monsters/players get blocked by this
    UTIL_SetSize(pev, Vector(-4, -4, -4), Vector(4, 4, 4)); // Uses point-sized, and can be stepped over
    UTIL_SetOrigin(this, pev->origin);

    SetTouch(&CSatchelCharge::SatchelSlide);
    SetUse(&CSatchelCharge::DetonateUse);
    SetThink(&CSatchelCharge::SatchelThink);
    SetNextThink(0.1);

    pev->gravity = 0.5;
    pev->friction = 0.8;

    pev->dmg = gSkillData.plrDmgSatchel;
    // ResetSequenceInfo( );
    pev->sequence = 1;
}


void CSatchelCharge::SatchelSlide(CBaseEntity* pOther)
{
    entvars_t* pevOther = pOther->pev;

    // don't hit the guy that launched this grenade
    if (pOther->edict() == pev->owner)
        return;

    // pev->avelocity = Vector (300, 300, 300);
    pev->gravity = 1; // normal gravity now

    // HACKHACK - On ground isn't always set, so look for ground underneath
    TraceResult tr;
    UTIL_TraceLine(pev->origin, pev->origin - Vector(0, 0, 10), ignore_monsters, edict(), &tr);

    if (tr.flFraction < 1.0)
    {
        // add a bit of static friction
        pev->velocity = pev->velocity * 0.95;
        pev->avelocity = pev->avelocity * 0.9;
        // play sliding sound, volume based on velocity
    }
    if (!(pev->flags & FL_ONGROUND) && pev->velocity.Length2D() > 10)
    {
        BounceSound();
    }
    StudioFrameAdvance();
}


void CSatchelCharge::SatchelThink()
{
    StudioFrameAdvance();
    SetNextThink(0.1);

    if (!IsInWorld())
    {
        UTIL_Remove(this);
        return;
    }

    if (pev->waterlevel == 3 && pev->watertype != CONTENT_FOG)
    {
        pev->movetype = MOVETYPE_FLY;
        pev->velocity = pev->velocity * 0.8;
        pev->avelocity = pev->avelocity * 0.9;
        pev->velocity.z += 8;
    }
    else if (pev->waterlevel == 0 || pev->watertype == CONTENT_FOG)
    {
        pev->movetype = MOVETYPE_BOUNCE;
    }
    else
    {
        pev->velocity.z -= 8;
    }
}

void CSatchelCharge::Precache()
{
    PRECACHE_MODEL("models/grenade.mdl");
    PRECACHE_SOUND("weapons/g_bounce1.wav");
    PRECACHE_SOUND("weapons/g_bounce2.wav");
    PRECACHE_SOUND("weapons/g_bounce3.wav");
}

void CSatchelCharge::BounceSound()
{
    switch (RANDOM_LONG(0, 2))
    {
    case 0: EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/g_bounce1.wav", 1, ATTN_NORM);
        break;
    case 1: EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/g_bounce2.wav", 1, ATTN_NORM);
        break;
    case 2: EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/g_bounce3.wav", 1, ATTN_NORM);
        break;
    }
}
