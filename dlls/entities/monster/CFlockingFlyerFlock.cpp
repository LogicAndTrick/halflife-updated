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

#include "CFlockingFlyerFlock.h"
#include "CFlockingFlyer.h"

LINK_ENTITY_TO_CLASS(monster_flyer_flock, CFlockingFlyerFlock);

TYPEDESCRIPTION CFlockingFlyerFlock::m_SaveData[] =
	{
		DEFINE_FIELD(CFlockingFlyerFlock, m_cFlockSize, FIELD_INTEGER),
		DEFINE_FIELD(CFlockingFlyerFlock, m_flFlockRadius, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(CFlockingFlyerFlock, CBaseMonster);

bool CFlockingFlyerFlock::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "iFlockSize"))
	{
		m_cFlockSize = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "flFlockRadius"))
	{
		m_flFlockRadius = atof(pkvd->szValue);
		return true;
	}

	return false;
}

void CFlockingFlyerFlock::Spawn()
{
	Precache();
	SpawnFlock();

	REMOVE_ENTITY(ENT(pev)); // dump the spawn ent
}

void CFlockingFlyerFlock::Precache()
{
	// PRECACHE_MODEL("models/aflock.mdl");
	PRECACHE_MODEL("models/boid.mdl");

	PrecacheFlockSounds();
}


void CFlockingFlyerFlock::PrecacheFlockSounds()
{
	PRECACHE_SOUND("boid/boid_alert1.wav");
	PRECACHE_SOUND("boid/boid_alert2.wav");

	PRECACHE_SOUND("boid/boid_idle1.wav");
	PRECACHE_SOUND("boid/boid_idle2.wav");
}

void CFlockingFlyerFlock::SpawnFlock()
{
	float R = m_flFlockRadius;
	int iCount;
	Vector vecSpot;
	CFlockingFlyer *pBoid, *pLeader;

	pLeader = pBoid = NULL;

	for (iCount = 0; iCount < m_cFlockSize; iCount++)
	{
		pBoid = GetClassPtr((CFlockingFlyer*)NULL);

		if (!pLeader)
		{
			// make this guy the leader.
			pLeader = pBoid;

			pLeader->m_pSquadLeader = pLeader;
			pLeader->m_pSquadNext = NULL;
		}

		vecSpot.x = RANDOM_FLOAT(-R, R);
		vecSpot.y = RANDOM_FLOAT(-R, R);
		vecSpot.z = RANDOM_FLOAT(0, 16);
		vecSpot = pev->origin + vecSpot;

		UTIL_SetOrigin(pBoid, vecSpot);
		pBoid->pev->movetype = MOVETYPE_FLY;
		pBoid->SpawnCommonCode();
		pBoid->pev->flags &= ~FL_ONGROUND;
		pBoid->pev->velocity = g_vecZero;
		pBoid->pev->angles = pev->angles;

		pBoid->pev->frame = 0;
		pBoid->SetNextThink(0.2);
		pBoid->SetThink(&CFlockingFlyer::IdleThink);

		if (pBoid != pLeader)
		{
			pLeader->SquadAdd(pBoid);
		}
	}
}
