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

#include "CFlockingFlyer.h"
#include "CFlockingFlyerFlock.h"
#include "util/trace.h"

LINK_ENTITY_TO_CLASS(monster_flyer, CFlockingFlyer);

TYPEDESCRIPTION CFlockingFlyer::m_SaveData[] =
	{
		DEFINE_FIELD(CFlockingFlyer, m_pSquadLeader, FIELD_CLASSPTR),
		DEFINE_FIELD(CFlockingFlyer, m_pSquadNext, FIELD_CLASSPTR),
		DEFINE_FIELD(CFlockingFlyer, m_fTurning, FIELD_BOOLEAN),
		DEFINE_FIELD(CFlockingFlyer, m_fCourseAdjust, FIELD_BOOLEAN),
		DEFINE_FIELD(CFlockingFlyer, m_fPathBlocked, FIELD_BOOLEAN),
		DEFINE_FIELD(CFlockingFlyer, m_vecReferencePoint, FIELD_POSITION_VECTOR),
		DEFINE_FIELD(CFlockingFlyer, m_vecAdjustedVelocity, FIELD_VECTOR),
		DEFINE_FIELD(CFlockingFlyer, m_flGoalSpeed, FIELD_FLOAT),
		DEFINE_FIELD(CFlockingFlyer, m_flLastBlockedTime, FIELD_TIME),
		DEFINE_FIELD(CFlockingFlyer, m_flFakeBlockedTime, FIELD_TIME),
		DEFINE_FIELD(CFlockingFlyer, m_flAlertTime, FIELD_TIME),
		//	DEFINE_FIELD( CFlockingFlyer, m_flFlockNextSoundTime, FIELD_TIME ),	// don't need to save
};

IMPLEMENT_SAVERESTORE(CFlockingFlyer, CBaseMonster);

void CFlockingFlyer::Spawn()
{
	Precache();
	SpawnCommonCode();

	pev->frame = 0;
	SetNextThink(0.1);
	SetThink(&CFlockingFlyer::IdleThink);
}

void CFlockingFlyer::Precache()
{
	// PRECACHE_MODEL("models/aflock.mdl");
	PRECACHE_MODEL("models/boid.mdl");
	CFlockingFlyerFlock::PrecacheFlockSounds();
}

void CFlockingFlyer::MakeSound()
{
	if (m_flAlertTime > gpGlobals->time)
	{
		// make agitated sounds
		switch (RANDOM_LONG(0, 1))
		{
		case 0:
			EMIT_SOUND(ENT(pev), CHAN_WEAPON, "boid/boid_alert1.wav", 1, ATTN_NORM);
			break;
		case 1:
			EMIT_SOUND(ENT(pev), CHAN_WEAPON, "boid/boid_alert2.wav", 1, ATTN_NORM);
			break;
		}

		return;
	}

	// make normal sound
	switch (RANDOM_LONG(0, 1))
	{
	case 0:
		EMIT_SOUND(ENT(pev), CHAN_WEAPON, "boid/boid_idle1.wav", 1, ATTN_NORM);
		break;
	case 1:
		EMIT_SOUND(ENT(pev), CHAN_WEAPON, "boid/boid_idle2.wav", 1, ATTN_NORM);
		break;
	}
}

void CFlockingFlyer::Killed(entvars_t* pevInflictor, entvars_t* pevAttacker, int iGib)
{
	CFlockingFlyer* pSquad;

	pSquad = (CFlockingFlyer*)m_pSquadLeader;

	while (pSquad)
	{
		pSquad->m_flAlertTime = gpGlobals->time + 15;
		pSquad = (CFlockingFlyer*)pSquad->m_pSquadNext;
	}

	if (m_pSquadLeader)
	{
		m_pSquadLeader->SquadRemove(this);
	}

	pev->deadflag = DEAD_DEAD;

	pev->framerate = 0;
	pev->effects = EF_NOINTERP;

	UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));
	pev->movetype = MOVETYPE_TOSS;

	SetThink(&CFlockingFlyer::FallHack);
	SetNextThink(0.1);
}

void CFlockingFlyer::FallHack()
{
	if ((pev->flags & FL_ONGROUND) != 0)
	{
		if (!FClassnameIs(pev->groundentity, "worldspawn"))
		{
			pev->flags &= ~FL_ONGROUND;
			SetNextThink(0.1);
		}
		else
		{
			pev->velocity = g_vecZero;
			SetThink(NULL);
		}
	}
}

void CFlockingFlyer::SpawnCommonCode()
{
	pev->deadflag = DEAD_NO;
	pev->classname = MAKE_STRING("monster_flyer");
	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_FLY;
	pev->takedamage = DAMAGE_YES;
	pev->health = 10;

	m_bloodColor = BLOOD_COLOR_GREEN;

	m_fPathBlocked = false; // obstacles will be detected
	m_flFieldOfView = 0.2;

	// SET_MODEL(ENT(pev), "models/aflock.mdl");
	SET_MODEL(ENT(pev), "models/boid.mdl");

	//	UTIL_SetSize(pev, Vector(0,0,0), Vector(0,0,0));
	UTIL_SetSize(pev, Vector(-5, -5, 0), Vector(5, 5, 2));
}

void CFlockingFlyer::BoidAdvanceFrame()
{
	float flapspeed = (pev->speed - pev->armorvalue) / AFLOCK_ACCELERATE;
	pev->armorvalue = pev->armorvalue * .8 + pev->speed * .2;

	if (flapspeed < 0)
		flapspeed = -flapspeed;
	if (flapspeed < 0.25)
		flapspeed = 0.25;
	if (flapspeed > 1.9)
		flapspeed = 1.9;

	pev->framerate = flapspeed;

	// lean
	pev->avelocity.x = -(pev->angles.x + flapspeed * 5);

	// bank
	pev->avelocity.z = -(pev->angles.z + pev->avelocity.y);

	// pev->framerate		= flapspeed;
	StudioFrameAdvance(0.1);
}

void CFlockingFlyer::IdleThink()
{
	SetNextThink(0.2);

	// see if there's a client in the same pvs as the monster
	//	if ( !FNullEnt(FIND_CLIENT_IN_PVS(edict())))
	if (!FNullEnt(FIND_CLIENT_IN_PVS(edict())) || HaveCamerasInPVS(edict()))
	{
		SetThink(&CFlockingFlyer::Start);
		SetNextThink(0.1);
	}
}

/**
 * Start - player enters the pvs, so get things going.
 */
void CFlockingFlyer::Start()
{
	SetNextThink(0.1);

	if (IsLeader())
	{
		SetThink(&CFlockingFlyer::FlockLeaderThink);
	}
	else
	{
		SetThink(&CFlockingFlyer::FlockFollowerThink);
	}

	/*
	Vector	vecTakeOff;
	vecTakeOff = Vector ( 0 , 0 , 0 );

	vecTakeOff.z = 50 + RANDOM_FLOAT ( 0, 100 );
	vecTakeOff.x = 20 - RANDOM_FLOAT ( 0, 40);
	vecTakeOff.y = 20 - RANDOM_FLOAT ( 0, 40);

	pev->velocity = vecTakeOff;


	pev->speed = pev->velocity.Length();
	pev->sequence = 0;
*/
	SetActivity(ACT_FLY);
	ResetSequenceInfo();
	BoidAdvanceFrame();

	pev->speed = AFLOCK_FLY_SPEED; // no delay!
}

/**
 * Leader boid calls this to form a flock from surrounding boids
 */
void CFlockingFlyer::FormFlock()
{
	if (!InSquad())
	{
		// I am my own leader
		m_pSquadLeader = this;
		m_pSquadNext = NULL;
		int squadCount = 1;

		CBaseEntity* pEntity = NULL;

		while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, AFLOCK_MAX_RECRUIT_RADIUS)) != NULL)
		{
			CBaseMonster* pRecruit = pEntity->MyMonsterPointer();

			if (pRecruit && pRecruit != this && pRecruit->IsAlive() && !pRecruit->m_pCine)
			{
				// Can we recruit this guy?
				if (FClassnameIs(pRecruit->pev, "monster_flyer"))
				{
					squadCount++;
					SquadAdd((CFlockingFlyer*)pRecruit);
				}
			}
		}
	}

	SetThink(&CFlockingFlyer::IdleThink); // now that flock is formed, go to idle and wait for a player to come along.
	SetNextThink(0);
}

/**
 * Searches for boids that are too close and pushes them away
 */
void CFlockingFlyer::SpreadFlock()
{
	Vector vecDir;
	float flSpeed; // holds vector magnitude while we fiddle with the direction

	CFlockingFlyer* pList = m_pSquadLeader;
	while (pList)
	{
		if (pList != this && (pev->origin - pList->pev->origin).Length() <= AFLOCK_TOO_CLOSE)
		{
			// push the other away
			vecDir = (pList->pev->origin - pev->origin);
			vecDir = vecDir.Normalize();

			// store the magnitude of the other boid's velocity, and normalize it so we
			// can average in a course that points away from the leader.
			flSpeed = pList->pev->velocity.Length();
			pList->pev->velocity = pList->pev->velocity.Normalize();
			pList->pev->velocity = (pList->pev->velocity + vecDir) * 0.5;
			pList->pev->velocity = pList->pev->velocity * flSpeed;
		}

		pList = pList->m_pSquadNext;
	}
}

/**
 * Alters the caller's course if he's too close to others
 * This function should **ONLY** be called when Caller's velocity is normalized!!
 */
void CFlockingFlyer::SpreadFlock2()
{
	Vector vecDir;

	CFlockingFlyer* pList = m_pSquadLeader;
	while (pList)
	{
		if (pList != this && (pev->origin - pList->pev->origin).Length() <= AFLOCK_TOO_CLOSE)
		{
			vecDir = (pev->origin - pList->pev->origin);
			vecDir = vecDir.Normalize();

			pev->velocity = (pev->velocity + vecDir);
		}

		pList = pList->m_pSquadNext;
	}
}

/**
 * FBoidPathBlocked - returns true if there is an obstacle ahead
 */
bool CFlockingFlyer::FPathBlocked()
{
	TraceResult tr;
	Vector vecDist; // used for general measurements
	Vector vecDir;	// used for general measurements
	bool fBlocked;

	if (m_flFakeBlockedTime > gpGlobals->time)
	{
		m_flLastBlockedTime = gpGlobals->time;
		return true;
	}

	// use VELOCITY, not angles, not all boids point the direction they are flying
	// vecDir = UTIL_VecToAngles( pevBoid->velocity );
	UTIL_MakeVectors(pev->angles);

	fBlocked = false; // assume the way ahead is clear

	// check for obstacle ahead
	UTIL_TraceLine(pev->origin, pev->origin + gpGlobals->v_forward * AFLOCK_CHECK_DIST, ignore_monsters, ENT(pev), &tr);
	if (tr.flFraction != 1.0)
	{
		m_flLastBlockedTime = gpGlobals->time;
		fBlocked = true;
	}

	// extra wide checks
	UTIL_TraceLine(pev->origin + gpGlobals->v_right * 12, pev->origin + gpGlobals->v_right * 12 + gpGlobals->v_forward * AFLOCK_CHECK_DIST, ignore_monsters, ENT(pev), &tr);
	if (tr.flFraction != 1.0)
	{
		m_flLastBlockedTime = gpGlobals->time;
		fBlocked = true;
	}

	UTIL_TraceLine(pev->origin - gpGlobals->v_right * 12, pev->origin - gpGlobals->v_right * 12 + gpGlobals->v_forward * AFLOCK_CHECK_DIST, ignore_monsters, ENT(pev), &tr);
	if (tr.flFraction != 1.0)
	{
		m_flLastBlockedTime = gpGlobals->time;
		fBlocked = true;
	}

	if (!fBlocked && gpGlobals->time - m_flLastBlockedTime > 6)
	{
		// not blocked, and it's been a few seconds since we've actually been blocked.
		m_flFakeBlockedTime = gpGlobals->time + RANDOM_LONG(1, 3);
	}

	return fBlocked;
}

/**
 * Leader boids use this think every tenth
 */
void CFlockingFlyer::FlockLeaderThink()
{
	TraceResult tr;
	Vector vecDist;		// used for general measurements
	Vector vecDir;		// used for general measurements
	int cProcessed = 0; // keep track of how many other boids we've processed
	float flLeftSide;
	float flRightSide;


	SetNextThink(0.1);

	UTIL_MakeVectors(pev->angles);

	// is the way ahead clear?
	if (!FPathBlocked())
	{
		// if the boid is turning, stop the trend.
		if (m_fTurning)
		{
			m_fTurning = false;
			pev->avelocity.y = 0;
		}

		m_fPathBlocked = false;

		if (pev->speed <= AFLOCK_FLY_SPEED)
			pev->speed += 5;

		pev->velocity = gpGlobals->v_forward * pev->speed;

		BoidAdvanceFrame();

		return;
	}

	// IF we get this far in the function, the leader's path is blocked!
	m_fPathBlocked = true;

	if (!m_fTurning) // something in the way and boid is not already turning to avoid
	{
		// measure clearance on left and right to pick the best dir to turn
		UTIL_TraceLine(pev->origin, pev->origin + gpGlobals->v_right * AFLOCK_CHECK_DIST, ignore_monsters, ENT(pev), &tr);
		vecDist = (tr.vecEndPos - pev->origin);
		flRightSide = vecDist.Length();

		UTIL_TraceLine(pev->origin, pev->origin - gpGlobals->v_right * AFLOCK_CHECK_DIST, ignore_monsters, ENT(pev), &tr);
		vecDist = (tr.vecEndPos - pev->origin);
		flLeftSide = vecDist.Length();

		// turn right if more clearance on right side
		if (flRightSide > flLeftSide)
		{
			pev->avelocity.y = -AFLOCK_TURN_RATE;
			m_fTurning = true;
		}
		// default to left turn :)
		else if (flLeftSide > flRightSide)
		{
			pev->avelocity.y = AFLOCK_TURN_RATE;
			m_fTurning = true;
		}
		else
		{
			// equidistant. Pick randomly between left and right.
			m_fTurning = true;

			if (RANDOM_LONG(0, 1) == 0)
			{
				pev->avelocity.y = AFLOCK_TURN_RATE;
			}
			else
			{
				pev->avelocity.y = -AFLOCK_TURN_RATE;
			}
		}
	}
	SpreadFlock();

	pev->velocity = gpGlobals->v_forward * pev->speed;

	// check and make sure we aren't about to plow into the ground, don't let it happen
	UTIL_TraceLine(pev->origin, pev->origin - gpGlobals->v_up * 16, ignore_monsters, ENT(pev), &tr);
	if (tr.flFraction != 1.0 && pev->velocity.z < 0)
		pev->velocity.z = 0;

	// maybe it did, though.
	if (FBitSet(pev->flags, FL_ONGROUND))
	{
		UTIL_SetOrigin(this, pev->origin + Vector(0, 0, 1));
		pev->velocity.z = 0;
	}

	if (m_flFlockNextSoundTime < gpGlobals->time)
	{
		MakeSound();
		m_flFlockNextSoundTime = gpGlobals->time + RANDOM_FLOAT(1, 3);
	}

	BoidAdvanceFrame();

	return;
}

/**
 * follower boids execute this code when flocking
 */
void CFlockingFlyer::FlockFollowerThink()
{
	TraceResult tr;
	Vector vecDist;
	Vector vecDir;
	Vector vecDirToLeader;
	float flDistToLeader;

	SetNextThink(0.1);

	if (IsLeader() || !InSquad())
	{
		// the leader has been killed and this flyer suddenly finds himself the leader.
		SetThink(&CFlockingFlyer::FlockLeaderThink);
		return;
	}

	vecDirToLeader = (m_pSquadLeader->pev->origin - pev->origin);
	flDistToLeader = vecDirToLeader.Length();

	// match heading with leader
	pev->angles = m_pSquadLeader->pev->angles;

	//
	// We can see the leader, so try to catch up to it
	//
	if (FInViewCone(m_pSquadLeader))
	{
		// if we're too far away, speed up
		if (flDistToLeader > AFLOCK_TOO_FAR)
		{
			m_flGoalSpeed = m_pSquadLeader->pev->velocity.Length() * 1.5;
		}

		// if we're too close, slow down
		else if (flDistToLeader < AFLOCK_TOO_CLOSE)
		{
			m_flGoalSpeed = m_pSquadLeader->pev->velocity.Length() * 0.5;
		}
	}
	else
	{
		// wait up! the leader isn't out in front, so we slow down to let him pass
		m_flGoalSpeed = m_pSquadLeader->pev->velocity.Length() * 0.5;
	}

	SpreadFlock2();

	pev->speed = pev->velocity.Length();
	pev->velocity = pev->velocity.Normalize();

	// if we are too far from leader, average a vector towards it into our current velocity
	if (flDistToLeader > AFLOCK_TOO_FAR)
	{
		vecDirToLeader = vecDirToLeader.Normalize();
		pev->velocity = (pev->velocity + vecDirToLeader) * 0.5;
	}

	// clamp speeds and handle acceleration
	if (m_flGoalSpeed > AFLOCK_FLY_SPEED * 2)
	{
		m_flGoalSpeed = AFLOCK_FLY_SPEED * 2;
	}

	if (pev->speed < m_flGoalSpeed)
	{
		pev->speed += AFLOCK_ACCELERATE;
	}
	else if (pev->speed > m_flGoalSpeed)
	{
		pev->speed -= AFLOCK_ACCELERATE;
	}

	pev->velocity = pev->velocity * pev->speed;

	BoidAdvanceFrame();
}

/*
	// Is this boid's course blocked?
	if ( FBoidPathBlocked (pev) )
	{
		// course is still blocked from last time. Just keep flying along adjusted
		// velocity
		if ( m_fCourseAdjust )
		{
			pev->velocity = m_vecAdjustedVelocity * pev->speed;
			return;
		}
		else // set course adjust flag and calculate adjusted velocity
		{
			m_fCourseAdjust = true;

			// use VELOCITY, not angles, not all boids point the direction they are flying
			//vecDir = UTIL_VecToAngles( pev->velocity );
			//UTIL_MakeVectors ( vecDir );

			UTIL_MakeVectors ( pev->angles );

			// measure clearance on left and right to pick the best dir to turn
			UTIL_TraceLine(pev->origin, pev->origin + gpGlobals->v_right * AFLOCK_CHECK_DIST, ignore_monsters, ENT(pev), &tr);
			vecDist = (tr.vecEndPos - pev->origin);
			flRightSide = vecDist.Length();

			UTIL_TraceLine(pev->origin, pev->origin - gpGlobals->v_right * AFLOCK_CHECK_DIST, ignore_monsters, ENT(pev), &tr);
			vecDist = (tr.vecEndPos - pev->origin);
			flLeftSide = vecDist.Length();

			// slide right if more clearance on right side
			if ( flRightSide > flLeftSide )
			{
				m_vecAdjustedVelocity = gpGlobals->v_right;
			}
			// else slide left
			else
			{
				m_vecAdjustedVelocity = gpGlobals->v_right * -1;
			}
		}
		return;
	}

	// if we make it this far, boids path is CLEAR!
	m_fCourseAdjust = false;
*/


/**
 * SquadUnlink(), Unlink the squad pointers.
 */
void CFlockingFlyer::SquadUnlink()
{
	m_pSquadLeader = NULL;
	m_pSquadNext = NULL;
}

/**
 * SquadAdd(), add pAdd to my squad
 */
void CFlockingFlyer::SquadAdd(CFlockingFlyer* pAdd)
{
	ASSERT(pAdd != NULL);
	ASSERT(!pAdd->InSquad());
	ASSERT(this->IsLeader());

	pAdd->m_pSquadNext = m_pSquadNext;
	m_pSquadNext = pAdd;
	pAdd->m_pSquadLeader = this;
}

/**
 * SquadRemove(), remove pRemove from my squad.
 * If I am pRemove, promote m_pSquadNext to leader
 */
void CFlockingFlyer::SquadRemove(CFlockingFlyer* pRemove)
{
	ASSERT(pRemove != NULL);
	ASSERT(this->IsLeader());
	ASSERT(pRemove->m_pSquadLeader == this);

	if (SquadCount() > 2)
	{
		// Removing the leader, promote m_pSquadNext to leader
		if (pRemove == this)
		{
			CFlockingFlyer* pLeader = m_pSquadNext;

			// copy the enemy LKP to the new leader
			pLeader->m_vecEnemyLKP = m_vecEnemyLKP;

			if (pLeader)
			{
				CFlockingFlyer* pList = pLeader;

				while (pList)
				{
					pList->m_pSquadLeader = pLeader;
					pList = pList->m_pSquadNext;
				}
			}
			SquadUnlink();
		}
		else // removing a node
		{
			CFlockingFlyer* pList = this;

			// Find the node before pRemove
			while (pList->m_pSquadNext != pRemove)
			{
				// assert to test valid list construction
				ASSERT(pList->m_pSquadNext != NULL);
				pList = pList->m_pSquadNext;
			}
			// List validity
			ASSERT(pList->m_pSquadNext == pRemove);

			// Relink without pRemove
			pList->m_pSquadNext = pRemove->m_pSquadNext;

			// Unlink pRemove
			pRemove->SquadUnlink();
		}
	}
	else
		SquadDisband();
}

/**
 * SquadCount(), return the number of members of this squad
 * callable from leaders & followers
 */
int CFlockingFlyer::SquadCount()
{
	CFlockingFlyer* pList = m_pSquadLeader;
	int squadCount = 0;
	while (pList)
	{
		squadCount++;
		pList = pList->m_pSquadNext;
	}

	return squadCount;
}

/**
 * SquadDisband(), Unlink all squad members
 */
void CFlockingFlyer::SquadDisband()
{
	CFlockingFlyer* pList = m_pSquadLeader;
	CFlockingFlyer* pNext;

	while (pList)
	{
		pNext = pList->m_pSquadNext;
		pList->SquadUnlink();
		pList = pNext;
	}
}
