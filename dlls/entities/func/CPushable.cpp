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

#include "CPushable.h"

LINK_ENTITY_TO_CLASS(func_pushable, CPushable);

TYPEDESCRIPTION CPushable::m_SaveData[] =
	{
		DEFINE_FIELD(CPushable, m_maxSpeed, FIELD_FLOAT),
		DEFINE_FIELD(CPushable, m_soundTime, FIELD_TIME),
};

IMPLEMENT_SAVERESTORE(CPushable, CBreakable);

const char* CPushable::m_soundNames[3] = {"debris/pushbox1.wav", "debris/pushbox2.wav", "debris/pushbox3.wav"};

void CPushable::Spawn()
{
	Vector vecMins = pev->mins;
	Vector vecMaxs = pev->maxs;

	if ((pev->spawnflags & SF_PUSH_BREAKABLE) != 0)
		CBreakable::Spawn();
	else
		Precache();

	pev->movetype = MOVETYPE_PUSHSTEP;
	pev->solid = SOLID_BBOX;
	SET_MODEL(ENT(pev), STRING(pev->model));

	//	UTIL_SetSize( pev, vecMins, vecMaxs );

	if (pev->friction > 399)
		pev->friction = 399;

	m_maxSpeed = 400 - pev->friction;
	SetBits(pev->flags, FL_FLOAT);
	pev->friction = 0;

	pev->origin.z += 1; // Pick up off of the floor
	UTIL_SetOrigin(this, pev->origin);

	// Multiply by area of the box's cross-section (assume 1000 units^3 standard volume)
	pev->skin = (pev->skin * (pev->maxs.x - pev->mins.x) * (pev->maxs.y - pev->mins.y)) * 0.0005;
	m_soundTime = 0;
}

void CPushable::Precache()
{
	for (int i = 0; i < 3; i++)
		PRECACHE_SOUND(m_soundNames[i]);

	if ((pev->spawnflags & SF_PUSH_BREAKABLE) != 0)
		CBreakable::Precache();
}

bool CPushable::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "size"))
	{
		int bbox = atoi(pkvd->szValue);

		switch (bbox)
		{
		case 0: // Point
			UTIL_SetSize(pev, Vector(-8, -8, -8), Vector(8, 8, 8));
			break;

		case 2: // Big Hull!?!?	!!!BUGBUG Figure out what this hull really is
			UTIL_SetSize(pev, VEC_DUCK_HULL_MIN * 2, VEC_DUCK_HULL_MAX * 2);
			break;

		case 3: // Player duck
			UTIL_SetSize(pev, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX);
			break;

		default:
		case 1: // Player
			UTIL_SetSize(pev, VEC_HULL_MIN, VEC_HULL_MAX);
			break;
		}

		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "buoyancy"))
	{
		pev->skin = atof(pkvd->szValue);
		return true;
	}

	return CBreakable::KeyValue(pkvd);
}

// Pull the func_pushable
void CPushable::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (!pActivator || !pActivator->IsPlayer())
	{
		if ((pev->spawnflags & SF_PUSH_BREAKABLE) != 0)
			this->CBreakable::Use(pActivator, pCaller, useType, value);
		return;
	}

	if (pev->spawnflags & SF_PUSH_NOPULL)
		return; // LRC: a non-pullable pushable.

	if (pActivator->pev->velocity != g_vecZero)
		Move(pActivator, false);
}

void CPushable::Touch(CBaseEntity* pOther)
{
	if (FClassnameIs(pOther->pev, "worldspawn"))
		return;

	Move(pOther, true);
}

void CPushable::Move(CBaseEntity* pOther, bool push)
{
	entvars_t* pevToucher = pOther->pev;
	bool playerTouch = false;

	// Is entity standing on this pushable ?
	if (FBitSet(pevToucher->flags, FL_ONGROUND) && pevToucher->groundentity && VARS(pevToucher->groundentity) == pev)
	{
		// Only push if floating
		if (pev->waterlevel > 0 && pev->watertype > CONTENT_FLYFIELD)
			pev->velocity.z += pevToucher->velocity.z * 0.1;

		return;
	}


	if (pOther->IsPlayer())
	{
		if (push && (pevToucher->button & (IN_FORWARD | IN_USE)) == 0) // Don't push unless the player is pushing forward and NOT use (pull)
			return;
		playerTouch = true;
	}

	float factor;

	if (playerTouch)
	{
		if ((pevToucher->flags & FL_ONGROUND) == 0) // Don't push away from jumping/falling players unless in water
		{
			if (pev->waterlevel < 1 || pev->watertype <= CONTENT_FLYFIELD)
				return;
			else
				factor = 0.1;
		}
		else
			factor = 1;
	}
	else
		factor = 0.25;

	if (!push)
		factor = factor * 0.5;

	Vector oldVelocity = pev->velocity; // LRC 1.8
	pev->velocity.x += pevToucher->velocity.x * factor;
	pev->velocity.y += pevToucher->velocity.y * factor;

	float length = sqrt(pev->velocity.x * pev->velocity.x + pev->velocity.y * pev->velocity.y);
	if (push && (length > MaxSpeed()))
	{
		pev->velocity.x = (pev->velocity.x * MaxSpeed() / length);
		pev->velocity.y = (pev->velocity.y * MaxSpeed() / length);
	}

	if (playerTouch)
	{
		// LRC 1.8
		if (pev->spawnflags & SF_PUSH_NOSUPERPUSH)
		{
			// don't accelerate the pushable to be faster than the person pushing it
			float playerSpeed = pevToucher->velocity.Length2D();
			Vector playerPushDir = pevToucher->velocity / playerSpeed;
			playerPushDir.z = 0;
			float newdot = DotProduct(playerPushDir, pev->velocity); // how fast we're going with respect to the playerPushDir
			float olddot = DotProduct(playerPushDir, oldVelocity);	 // how fast we used to be going
			if (/*olddot <= playerSpeed+0.1f &&*/ newdot > playerSpeed)
			{
				// if it wasn't going too fast before, and now it is, adjust to the pusher's actual velocity
				pev->velocity.x -= playerPushDir.x * newdot;
				pev->velocity.y -= playerPushDir.y * newdot;
				pev->velocity.x += pevToucher->velocity.x;
				pev->velocity.y += pevToucher->velocity.y;
			}
		}

		pevToucher->velocity.x = pev->velocity.x;
		pevToucher->velocity.y = pev->velocity.y;
		if ((gpGlobals->time - m_soundTime) > 0.7)
		{
			m_soundTime = gpGlobals->time;
			if (length > 0 && FBitSet(pev->flags, FL_ONGROUND))
			{
				m_lastSound = RANDOM_LONG(0, 2);
				EMIT_SOUND(ENT(pev), CHAN_WEAPON, m_soundNames[m_lastSound], 0.5, ATTN_NORM);
				//			SetThink( StopSound );
				//			SetNextThink( 0.1 );
			}
			else
				STOP_SOUND(ENT(pev), CHAN_WEAPON, m_soundNames[m_lastSound]);
		}
	}
}

bool CPushable::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	if ((pev->spawnflags & SF_PUSH_BREAKABLE) != 0)
		return CBreakable::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);

	return true;
}

void CPushable::DoRespawn(void)
{ // AJH Fix for respawnable breakable pushables (BY HAWK777)
	pev->solid = SOLID_BBOX;
	pev->origin.z += 1;
	UTIL_SetOrigin(this, pev->origin);
}
