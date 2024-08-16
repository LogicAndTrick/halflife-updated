/***
 *
 *	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
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

#include "CEnvSound.h"
#include "entities/player/CBasePlayer.h"

LINK_ENTITY_TO_CLASS(env_sound, CEnvSound);

TYPEDESCRIPTION CEnvSound::m_SaveData[] =
	{
		DEFINE_FIELD(CEnvSound, m_flRadius, FIELD_FLOAT),
		DEFINE_FIELD(CEnvSound, m_Roomtype, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CEnvSound, CBaseEntity);

bool CEnvSound::KeyValue(KeyValueData* pkvd)
{

	if (FStrEq(pkvd->szKeyName, "radius"))
	{
		m_flRadius = atof(pkvd->szValue);
		return true;
	}
	if (FStrEq(pkvd->szKeyName, "roomtype"))
	{
		m_Roomtype = atoi(pkvd->szValue);
		return true;
	}

	return false;
}

// returns true if the given sound entity (pev) is in range
// and can see the given player entity (pevTarget)
bool FEnvSoundInRange(CEnvSound* pSound, entvars_t* pevTarget, float& flRange)
{
	const Vector vecSpot1 = pSound->pev->origin + pSound->pev->view_ofs;
	const Vector vecSpot2 = pevTarget->origin + pevTarget->view_ofs;
	TraceResult tr;

	UTIL_TraceLine(vecSpot1, vecSpot2, ignore_monsters, pSound->edict(), &tr);

	// check if line of sight crosses water boundary, or is blocked

	if ((0 != tr.fInOpen && 0 != tr.fInWater) || tr.flFraction != 1)
		return false;

	// calc range from sound entity to player

	const Vector vecRange = tr.vecEndPos - vecSpot1;
	flRange = vecRange.Length();

	return pSound->m_flRadius >= flRange;
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
void CEnvSound::Think()
{
	const bool shouldThinkFast = [this]()
	{
		// get pointer to client if visible; FIND_CLIENT_IN_PVS will
		// cycle through visible clients on consecutive calls.
		edict_t* pentPlayer = FIND_CLIENT_IN_PVS(edict());

		if (FNullEnt(pentPlayer))
			return false; // no player in pvs of sound entity, slow it down

		// check to see if this is the sound entity that is currently affecting this player
		auto pPlayer = GetClassPtr((CBasePlayer*)VARS(pentPlayer));
		float flRange;

		if (pPlayer->m_SndLast && pPlayer->m_SndLast == this)
		{
			// this is the entity currently affecting player, check for validity
			if (pPlayer->m_SndRoomtype != 0 && pPlayer->m_flSndRange != 0)
			{
				// we're looking at a valid sound entity affecting
				// player, make sure it's still valid, update range
				if (FEnvSoundInRange(this, VARS(pentPlayer), flRange))
				{
					pPlayer->m_flSndRange = flRange;
					return true;
				}
				else
				{
					// current sound entity affecting player is no longer valid,
					// flag this state by clearing sound handle and range.
					// NOTE: we do not actually change the player's room_type
					// NOTE: until we have a new valid room_type to change it to.

					pPlayer->m_SndLast = nullptr;
					pPlayer->m_flSndRange = 0;
				}
			}

			// entity is affecting player but is out of range,
			// wait passively for another entity to usurp it...
			return false;
		}

		// if we got this far, we're looking at an entity that is contending
		// for current player sound. the closest entity to player wins.
		if (FEnvSoundInRange(this, VARS(pentPlayer), flRange))
		{
			if (flRange < pPlayer->m_flSndRange || pPlayer->m_flSndRange == 0)
			{
				// new entity is closer to player, so it wins.
				pPlayer->m_SndLast = this;
				pPlayer->m_SndRoomtype = m_Roomtype;
				pPlayer->m_flSndRange = flRange;

				// New room type is sent to player in CBasePlayer::UpdateClientData.

				// crank up nextthink rate for new active sound entity
			}
			// player is not closer to the contending sound entity.
			// this effectively cranks up the think rate of env_sound entities near the player.
		}

		// player is in pvs of sound entity, but either not visible or not in range. do nothing.

		return true;
	}();

	SetNextThink(shouldThinkFast ? 0.25 : 0.75);
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
