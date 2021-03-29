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

#include "CTriggerOnSight.h"
#include "entities/CBaseMonster.h"
#include "util/findentity.h"

LINK_ENTITY_TO_CLASS(trigger_onsight, CTriggerOnSight);

void CTriggerOnSight::Spawn()
{
    if (pev->target || pev->noise)
        // if we're going to have to trigger stuff, start thinking
        SetNextThink(1);
    else
        // otherwise, just check whenever someone asks about our state.
        pev->spawnflags |= SF_ONSIGHT_DEMAND;

    if (pev->max_health > 0)
    {
        pev->health = cos(pev->max_health / 2 * M_PI / 180.0);
        //        ALERT(at_debug, "Cosine is %f\n", pev->health);
    }
}

STATE CTriggerOnSight::GetState()
{
    if (pev->spawnflags & SF_ONSIGHT_DEMAND)
        return VisionCheck() ? STATE_ON : STATE_OFF;
    else
        return (pev->spawnflags & SF_ONSIGHT_ACTIVE) ? STATE_ON : STATE_OFF;
}

void CTriggerOnSight::Think()
{
    // is this a sensible rate?
    SetNextThink(0.1);

    //    if (!UTIL_IsMasterTriggered(m_sMaster, NULL))
    //    {
    //        pev->spawnflags &= ~SF_ONSIGHT_ACTIVE;
    //        return;
    //    }

    if (VisionCheck())
    {
        if (!FBitSet(pev->spawnflags, SF_ONSIGHT_ACTIVE))
        {
            FireTargets(STRING(pev->target), this, this, USE_TOGGLE, 0);
            FireTargets(STRING(pev->noise1), this, this, USE_ON, 0);
            pev->spawnflags |= SF_ONSIGHT_ACTIVE;
        }
    }
    else
    {
        if (pev->spawnflags & SF_ONSIGHT_ACTIVE)
        {
            FireTargets(STRING(pev->noise), this, this, USE_TOGGLE, 0);
            FireTargets(STRING(pev->noise1), this, this, USE_OFF, 0);
            pev->spawnflags &= ~SF_ONSIGHT_ACTIVE;
        }
    }
}

BOOL CTriggerOnSight::VisionCheck() //AJH modifed to check if multiple entities can see
{
    // and GetState check (stops dead monsters seeing)
    CBaseEntity* pLooker;
    if (pev->netname)
    {
        pLooker = UTIL_FindEntityByTargetname(NULL, STRING(pev->netname));
        while (pLooker != NULL)
        {
            if (!(pev->spawnflags & SF_ONSIGHT_STATECHECK) || (pev->spawnflags & SF_ONSIGHT_STATECHECK && pLooker->GetState() != STATE_OFF))
            {
                CBaseEntity* pSeen;
                if (pev->message)
                {
                    pSeen = UTIL_FindEntityByTargetname(NULL, STRING(pev->message));
                    if (!pSeen)
                    {
                        // must be a classname.
                        pSeen = UTIL_FindEntityByClassname(pSeen, STRING(pev->message));

                        while (pSeen != NULL)
                        {
                            if (CanSee(pLooker, pSeen))
                                return TRUE;
                            pSeen = UTIL_FindEntityByClassname(pSeen, STRING(pev->message));
                        }
                    }
                    else
                    {
                        while (pSeen != NULL)
                        {
                            if (CanSee(pLooker, pSeen))
                                return TRUE;
                            pSeen = UTIL_FindEntityByTargetname(pSeen, STRING(pev->message));
                        }
                    }
                }
                else
                {
                    if (CanSee(pLooker, this))
                        return TRUE;
                }
            }
            pLooker = UTIL_FindEntityByTargetname(pLooker, STRING(pev->netname));
        }
        return FALSE;
    }
    else
    {
        pLooker = UTIL_FindEntityByClassname(NULL, "player");
        if (!pLooker)
        {
            ALERT(at_error, "trigger_onsight can't find player!?\n");
            return FALSE;
        }
        CBaseEntity* pSeen;
        if (pev->message)
            pSeen = UTIL_FindEntityByTargetname(NULL, STRING(pev->message));
        else
            return CanSee(pLooker, this);

        if (!pSeen)
        {
            // must be a classname.
            pSeen = UTIL_FindEntityByClassname(pSeen, STRING(pev->message));
            while (pSeen != NULL)
            {
                if (CanSee(pLooker, pSeen))
                    return TRUE;
                pSeen = UTIL_FindEntityByClassname(pSeen, STRING(pev->message));
            }
            return FALSE;
        }
        else
        {
            while (pSeen != NULL)
            {
                if (CanSee(pLooker, pSeen))
                    return TRUE;
                pSeen = UTIL_FindEntityByTargetname(pSeen, STRING(pev->message));
            }
            return FALSE;
        }
    }
}

/*BOOL CTriggerOnSight :: VisionCheck()
{
    CBaseEntity *pLooker;
    if (pev->netname)
    {
        pLooker = UTIL_FindEntityByTargetname(NULL, STRING(pev->netname));
        if (!pLooker)
            return FALSE; // if we can't find the eye entity, give up
    }
    else
    {
        pLooker = UTIL_FindEntityByClassname(NULL, "player");
        if (!pLooker)
        {
            ALERT(at_error, "trigger_onsight can't find player!?\n");
            return FALSE;
        }
    }

    CBaseEntity *pSeen;
    if (pev->message)
        pSeen = UTIL_FindEntityByTargetname(NULL, STRING(pev->message));
    else
        return CanSee(pLooker, this);

    if (!pSeen)
    {
        // must be a classname.
        pSeen = UTIL_FindEntityByClassname(pSeen, STRING(pev->message));
        while (pSeen != NULL)
        {
            if (CanSee(pLooker, pSeen))
                return TRUE;
            pSeen = UTIL_FindEntityByClassname(pSeen, STRING(pev->message));
        }
        return FALSE;
    }
    else
    {
        while (pSeen != NULL)
        {
            if (CanSee(pLooker, pSeen))
                return TRUE;
            pSeen = UTIL_FindEntityByTargetname(pSeen, STRING(pev->message));
        }
        return FALSE;
    }
}*/

// by the criteria we're using, can the Looker see the Seen entity?
BOOL CTriggerOnSight::CanSee(CBaseEntity* pLooker, CBaseEntity* pSeen)
{
    // out of range?
    if (pev->frags && (pLooker->pev->origin - pSeen->pev->origin).Length() > pev->frags)
        return FALSE;

    // check FOV if appropriate
    if (pev->max_health < 360)
    {
        // copied from CBaseMonster's FInViewCone function
        Vector2D vec2LOS;
        float flDot;
        float flComp = pev->health;
        UTIL_MakeVectors(pLooker->pev->angles);
        vec2LOS = (pSeen->pev->origin - pLooker->pev->origin).Make2D();
        vec2LOS = vec2LOS.Normalize();
        flDot = Vector2D::DotProduct(vec2LOS, gpGlobals->v_forward.Make2D());

        //        ALERT(at_debug, "flDot is %f\n", flDot);

        if (pev->max_health == -1)
        {
            CBaseMonster* pMonst = pLooker->MyMonsterPointer();
            if (pMonst)
                flComp = pMonst->m_flFieldOfView;
            else
                return FALSE; // not a monster, can't use M-M-M-MonsterVision
        }

        // outside field of view
        if (flDot <= flComp)
            return FALSE;
    }

    // check LOS if appropriate
    if (!FBitSet(pev->spawnflags, SF_ONSIGHT_NOLOS))
    {
        TraceResult tr;
        if (SF_ONSIGHT_NOGLASS)
            UTIL_TraceLine(pLooker->EyePosition(), pSeen->pev->origin, ignore_monsters, ignore_glass, pLooker->edict(), &tr);
        else
            UTIL_TraceLine(pLooker->EyePosition(), pSeen->pev->origin, ignore_monsters, dont_ignore_glass, pLooker->edict(), &tr);
        if (tr.flFraction < 1.0 && tr.pHit != pSeen->edict())
            return FALSE;
    }

    return TRUE;
}
