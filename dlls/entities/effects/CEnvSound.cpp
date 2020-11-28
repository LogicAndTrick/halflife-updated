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

#include "CEnvSound.h"
#include "player.h"

LINK_ENTITY_TO_CLASS(env_sound, CEnvSound);

TYPEDESCRIPTION CEnvSound::m_SaveData[] =
{
    DEFINE_FIELD(CEnvSound, m_flRadius, FIELD_FLOAT),
    DEFINE_FIELD(CEnvSound, m_flRoomtype, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(CEnvSound, CBaseEntity);


void CEnvSound::KeyValue(KeyValueData* pkvd)
{
    if (FStrEq(pkvd->szKeyName, "radius"))
    {
        m_flRadius = atof(pkvd->szValue);
        pkvd->fHandled = TRUE;
    }
    if (FStrEq(pkvd->szKeyName, "roomtype"))
    {
        m_flRoomtype = atof(pkvd->szValue);
        pkvd->fHandled = TRUE;
    }
}

// returns TRUE if the given sound entity (pev) is in range 
// and can see the given player entity (pevTarget)

BOOL FEnvSoundInRange(entvars_t* pev, entvars_t* pevTarget, float* pflRange)
{
    CEnvSound* pSound = GetClassPtr((CEnvSound*)pev);
    Vector vecSpot1 = pev->origin + pev->view_ofs;
    Vector vecSpot2 = pevTarget->origin + pevTarget->view_ofs;
    Vector vecRange;
    float flRange;
    TraceResult tr;

    UTIL_TraceLine(vecSpot1, vecSpot2, ignore_monsters, ENT(pev), &tr);

    // check if line of sight crosses water boundary, or is blocked

    if ((tr.fInOpen && tr.fInWater) || tr.flFraction != 1)
        return FALSE;

    // calc range from sound entity to player

    vecRange = tr.vecEndPos - vecSpot1;
    flRange = vecRange.Length();

    if (pSound->m_flRadius < flRange)
        return FALSE;

    if (pflRange)
        *pflRange = flRange;

    return TRUE;
}

//
// A client that is visible and in range of a sound entity will
// have its room_type set by that sound entity.  If two or more
// sound entities are contending for a client, then the nearest
// sound entity to the client will set the client's room_type.
// A client's room_type will remain set to its prior value until
// a new in-range, visible sound entity resets a new room_type.
//

// CONSIDER: if player in water state, autoset roomtype to 14,15 or 16. 

void CEnvSound::Think(void)
{
    // get pointer to client if visible; FIND_CLIENT_IN_PVS will
    // cycle through visible clients on consecutive calls.

    edict_t* pentPlayer = FIND_CLIENT_IN_PVS(edict());
    CBasePlayer* pPlayer = NULL;

    if (FNullEnt(pentPlayer))
        goto env_sound_Think_slow; // no player in pvs of sound entity, slow it down

    pPlayer = GetClassPtr((CBasePlayer*)VARS(pentPlayer));
    float flRange;

    // check to see if this is the sound entity that is 
    // currently affecting this player

    if (!FNullEnt(pPlayer->m_pentSndLast) && (pPlayer->m_pentSndLast == ENT(pev)))
    {
        // this is the entity currently affecting player, check
        // for validity

        if (pPlayer->m_flSndRoomtype != 0 && pPlayer->m_flSndRange != 0)
        {
            // we're looking at a valid sound entity affecting
            // player, make sure it's still valid, update range

            if (FEnvSoundInRange(pev, VARS(pentPlayer), &flRange))
            {
                pPlayer->m_flSndRange = flRange;
                goto env_sound_Think_fast;
            }
            else
            {
                // current sound entity affecting player is no longer valid,
                // flag this state by clearing room_type and range.
                // NOTE: we do not actually change the player's room_type
                // NOTE: until we have a new valid room_type to change it to.

                pPlayer->m_flSndRange = 0;
                pPlayer->m_flSndRoomtype = 0;
                goto env_sound_Think_slow;
            }
        }
        else
        {
            // entity is affecting player but is out of range,
            // wait passively for another entity to usurp it...
            goto env_sound_Think_slow;
        }
    }

    // if we got this far, we're looking at an entity that is contending
    // for current player sound. the closest entity to player wins.

    if (FEnvSoundInRange(pev, VARS(pentPlayer), &flRange))
    {
        if (flRange < pPlayer->m_flSndRange || pPlayer->m_flSndRange == 0)
        {
            // new entity is closer to player, so it wins.
            pPlayer->m_pentSndLast = ENT(pev);
            pPlayer->m_flSndRoomtype = m_flRoomtype;
            pPlayer->m_flSndRange = flRange;

            // send room_type command to player's server.
            // this should be a rare event - once per change of room_type
            // only!

            //CLIENT_COMMAND(pentPlayer, "room_type %f", m_flRoomtype);

            MESSAGE_BEGIN(MSG_ONE, SVC_ROOMTYPE, NULL, pentPlayer); // use the magic #1 for "one client"
            WRITE_SHORT((short)m_flRoomtype); // sequence number
            MESSAGE_END();

            // crank up nextthink rate for new active sound entity
            // by falling through to think_fast...
        }
        // player is not closer to the contending sound entity,
        // just fall through to think_fast. this effectively
        // cranks up the think_rate of entities near the player.
    }

    // player is in pvs of sound entity, but either not visible or
    // not in range. do nothing, fall through to think_fast...

env_sound_Think_fast:
    SetNextThink(0.25);
    return;

env_sound_Think_slow:
    SetNextThink(0.75);
    return;
}

//
// env_sound - spawn a sound entity that will set player roomtype
// when player moves in range and sight.
//
//
void CEnvSound::Spawn()
{
    // spread think times
    SetNextThink(RANDOM_FLOAT(0.0, 0.5));
}
