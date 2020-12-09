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

#include "CFuncPlat.h"
#include "CPlatTrigger.h"
#include "util/movewith.h"
#include "trains.h"

//
//====================== PLAT code ====================================================
//

LINK_ENTITY_TO_CLASS(func_plat, CFuncPlat);

void CFuncPlat::CallGoUp() { GoUp(); }
void CFuncPlat::CallGoDown() { GoDown(); }
void CFuncPlat::CallHitTop() { HitTop(); }
void CFuncPlat::CallHitBottom() { HitBottom(); }

/*QUAKED func_plat (0 .5 .8) ? PLAT_LOW_TRIGGER
speed    default 150

Plats are always drawn in the extended position, so they will light correctly.

If the plat is the target of another trigger or button, it will start out disabled in
the extended position until it is trigger, when it will lower and become a normal plat.

If the "height" key is set, that will determine the amount the plat moves, instead of
being implicitly determined by the model's height.

Set "sounds" to one of the following:
1) base fast
2) chain slow
*/

void CFuncPlat::Setup(void)
{
    //pev->noiseMovement = MAKE_STRING("plats/platmove1.wav");
    //pev->noiseStopMoving = MAKE_STRING("plats/platstop1.wav");

    if (m_flTLength == 0)
        m_flTLength = 80;
    if (m_flTWidth == 0)
        m_flTWidth = 10;

    pev->angles = g_vecZero;

    pev->solid = SOLID_BSP;
    pev->movetype = MOVETYPE_PUSH;

    UTIL_SetOrigin(this, pev->origin); // set size and link into world
    UTIL_SetSize(pev, pev->mins, pev->maxs);
    SET_MODEL(ENT(pev), STRING(pev->model));

    // vecPosition1 is the top position, vecPosition2 is the bottom
    if (m_pMoveWith)
        m_vecPosition1 = pev->origin - m_pMoveWith->pev->origin;
    else
        m_vecPosition1 = pev->origin;
    m_vecPosition2 = m_vecPosition1;
    if (m_flHeight != 0)
        m_vecPosition2.z = m_vecPosition2.z - m_flHeight;
    else
        m_vecPosition2.z = m_vecPosition2.z - pev->size.z + 8;
    if (pev->speed == 0)
        pev->speed = 150;

    if (m_volume == 0)
        m_volume = 0.85;
}

static void PlatSpawnInsideTrigger(entvars_t* pevPlatform)
{
    GetClassPtr((CPlatTrigger*)NULL)->SpawnInsideTrigger(GetClassPtr((CFuncPlat*)pevPlatform));
}


void CFuncPlat::Precache()
{
    CBasePlatTrain::Precache();
    //PRECACHE_SOUND("plats/platmove1.wav");
    //PRECACHE_SOUND("plats/platstop1.wav");
    if (!IsTogglePlat())
        PlatSpawnInsideTrigger(pev); // the "start moving" trigger
}


void CFuncPlat::Spawn()
{
    Setup();

    Precache();

    // If this platform is the target of some button, it starts at the TOP position,
    // and is brought down by that button.  Otherwise, it starts at BOTTOM.
    if (!FStringNull(pev->targetname))
    {
        if (m_pMoveWith)
            UTIL_AssignOrigin(this, m_vecPosition1 + m_pMoveWith->pev->origin);
        else
            UTIL_AssignOrigin(this, m_vecPosition1);
        m_toggle_state = TS_AT_TOP;
        SetUse(&CFuncPlat::PlatUse);
    }
    else
    {
        if (m_pMoveWith)
            UTIL_AssignOrigin(this, m_vecPosition2 + m_pMoveWith->pev->origin);
        else
            UTIL_AssignOrigin(this, m_vecPosition2);
        m_toggle_state = TS_AT_BOTTOM;
    }
}


//
// Used by SUB_UseTargets, when a platform is the target of a button.
// Start bringing platform down.
//
void CFuncPlat::PlatUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
    m_hActivator = pActivator; //AJH

    if (IsTogglePlat())
    {
        // Top is off, bottom is on
        BOOL on = (m_toggle_state == TS_AT_BOTTOM) ? TRUE : FALSE;

        if (!ShouldToggle(useType, on))
            return;

        if (m_toggle_state == TS_AT_TOP)
        {
            SetNextThink(0.01);
            SetThink(&CFuncPlat::CallGoDown);
        }
        else if (m_toggle_state == TS_AT_BOTTOM)
        {
            SetNextThink(0.01);
            SetThink(&CFuncPlat::CallGoUp);
        }
    }
    else
    {
        SetUse(NULL);

        if (m_toggle_state == TS_AT_TOP)
        {
            SetNextThink(0.01);
            SetThink(&CFuncPlat::CallGoDown);
        }
    }
}


//
// Platform is at top, now starts moving down.
//
void CFuncPlat::GoDown(void)
{
    if (pev->noiseMovement)
        EMIT_SOUND(ENT(pev), CHAN_STATIC, (char*)STRING(pev->noiseMovement), m_volume, ATTN_NORM);

    ASSERT(m_toggle_state == TS_AT_TOP || m_toggle_state == TS_GOING_UP);
    m_toggle_state = TS_GOING_DOWN;
    SetMoveDone(&CFuncPlat::CallHitBottom);
    LinearMove(m_vecPosition2, pev->speed);
}


//
// Platform has hit bottom.  Stops and waits forever.
//
void CFuncPlat::HitBottom(void)
{
    if (pev->noiseMovement)
        STOP_SOUND(ENT(pev), CHAN_STATIC, (char*)STRING(pev->noiseMovement));

    if (pev->noiseStopMoving)
        EMIT_SOUND(ENT(pev), CHAN_WEAPON, (char*)STRING(pev->noiseStopMoving), m_volume, ATTN_NORM);

    ASSERT(m_toggle_state == TS_GOING_DOWN);
    m_toggle_state = TS_AT_BOTTOM;
}


//
// Platform is at bottom, now starts moving up
//
void CFuncPlat::GoUp(void)
{
    if (pev->noiseMovement)
        EMIT_SOUND(ENT(pev), CHAN_STATIC, (char*)STRING(pev->noiseMovement), m_volume, ATTN_NORM);

    ASSERT(m_toggle_state == TS_AT_BOTTOM || m_toggle_state == TS_GOING_DOWN);
    m_toggle_state = TS_GOING_UP;
    SetMoveDone(&CFuncPlat::CallHitTop);
    LinearMove(m_vecPosition1, pev->speed);
}


//
// Platform has hit top.  Pauses, then starts back down again.
//
void CFuncPlat::HitTop(void)
{
    if (pev->noiseMovement)
        STOP_SOUND(ENT(pev), CHAN_STATIC, (char*)STRING(pev->noiseMovement));

    if (pev->noiseStopMoving)
        EMIT_SOUND(ENT(pev), CHAN_WEAPON, (char*)STRING(pev->noiseStopMoving), m_volume, ATTN_NORM);

    ASSERT(m_toggle_state == TS_GOING_UP);
    m_toggle_state = TS_AT_TOP;

    if (!IsTogglePlat())
    {
        // After a delay, the platform will automatically start going down again.
        SetThink(&CFuncPlat::CallGoDown);
        SetNextThink(3);
    }
}


void CFuncPlat::Blocked(CBaseEntity* pOther)
{
    ALERT(at_aiconsole, "%s Blocked by %s\n", STRING(pev->classname), STRING(pOther->pev->classname));
    // Hurt the blocker a little
    if (m_hActivator)
        pOther->TakeDamage(pev, m_hActivator->pev, 1, DMG_CRUSH); //AJH Attribute damage to he who switched me.
    else
        pOther->TakeDamage(pev, pev, 1, DMG_CRUSH);
    if (pev->noiseMovement)
        STOP_SOUND(ENT(pev), CHAN_STATIC, (char*)STRING(pev->noiseMovement));

    // Send the platform back where it came from
    ASSERT(m_toggle_state == TS_GOING_UP || m_toggle_state == TS_GOING_DOWN);
    if (m_toggle_state == TS_GOING_UP)
    {
        SetNextThink(0);
        SetThink(&CFuncPlat::GoDown);
    }
    else if (m_toggle_state == TS_GOING_DOWN)
    {
        SetNextThink(0);
        SetThink(&CFuncPlat::GoUp);
    }
}
