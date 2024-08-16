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

#include "CBaseMonster.h"

//=========================================================
// IRelationship - returns an integer that describes the
// relationship between two types of monster.
//=========================================================
int CBaseMonster::IRelationship(CBaseEntity* pTarget)
{
	static int iEnemy[17][17] =
		{//   NONE	MACH	PLYR	HPASS	HMIL	AMIL	APASS	AMONST	APREY	APRED	INSECT	PLRALY	PBWPN	ABWPN	FACT_A	FACT_B	FACT_C
			/*NONE*/ {R_NO, R_NO, R_NO, R_NO, R_NO, R_NO, R_NO, R_NO, R_NO, R_NO, R_NO, R_NO, R_NO, R_NO, R_NO, R_NO, R_NO},
			/*MACHINE*/ {R_NO, R_NO, R_DL, R_DL, R_NO, R_DL, R_DL, R_DL, R_DL, R_DL, R_NO, R_DL, R_DL, R_DL, R_DL, R_DL, R_DL},
			/*PLAYER*/ {R_NO, R_DL, R_NO, R_NO, R_DL, R_DL, R_DL, R_DL, R_DL, R_DL, R_NO, R_NO, R_DL, R_DL, R_DL, R_DL, R_DL},
			/*HUMANPASSIVE*/ {R_NO, R_NO, R_AL, R_AL, R_HT, R_HT, R_NO, R_HT, R_DL, R_DL, R_NO, R_AL, R_NO, R_NO, R_DL, R_DL, R_DL},
			/*HUMANMILITAR*/ {R_NO, R_NO, R_HT, R_DL, R_NO, R_HT, R_DL, R_DL, R_DL, R_DL, R_NO, R_HT, R_NO, R_NO, R_DL, R_DL, R_DL},
			/*ALIENMILITAR*/ {R_NO, R_DL, R_HT, R_DL, R_HT, R_NO, R_NO, R_NO, R_NO, R_NO, R_NO, R_DL, R_NO, R_NO, R_DL, R_DL, R_DL},
			/*ALIENPASSIVE*/ {R_NO, R_NO, R_NO, R_NO, R_NO, R_NO, R_NO, R_NO, R_NO, R_NO, R_NO, R_NO, R_NO, R_NO, R_DL, R_DL, R_DL},
			/*ALIENMONSTER*/ {R_NO, R_DL, R_DL, R_DL, R_DL, R_NO, R_NO, R_NO, R_NO, R_NO, R_NO, R_DL, R_NO, R_NO, R_DL, R_DL, R_DL},
			/*ALIENPREY   */ {R_NO, R_NO, R_DL, R_DL, R_DL, R_NO, R_NO, R_NO, R_NO, R_FR, R_NO, R_DL, R_NO, R_NO, R_DL, R_DL, R_DL},
			/*ALIENPREDATO*/ {R_NO, R_NO, R_DL, R_DL, R_DL, R_NO, R_NO, R_NO, R_HT, R_DL, R_NO, R_DL, R_NO, R_NO, R_DL, R_DL, R_DL},
			/*INSECT*/ {R_FR, R_FR, R_FR, R_FR, R_FR, R_NO, R_FR, R_FR, R_FR, R_FR, R_NO, R_FR, R_NO, R_NO, R_FR, R_FR, R_FR},
			/*PLAYERALLY*/ {R_NO, R_DL, R_AL, R_AL, R_DL, R_DL, R_DL, R_DL, R_DL, R_DL, R_NO, R_NO, R_NO, R_NO, R_DL, R_DL, R_DL},
			/*PBIOWEAPON*/ {R_NO, R_NO, R_DL, R_DL, R_DL, R_DL, R_DL, R_DL, R_DL, R_DL, R_NO, R_DL, R_NO, R_DL, R_DL, R_DL, R_DL},
			/*ABIOWEAPON*/ {R_NO, R_NO, R_DL, R_DL, R_DL, R_AL, R_NO, R_DL, R_DL, R_NO, R_NO, R_DL, R_DL, R_NO, R_DL, R_DL, R_DL},
			/*FACTION_A*/ {R_NO, R_DL, R_DL, R_DL, R_DL, R_DL, R_DL, R_DL, R_DL, R_DL, R_NO, R_DL, R_DL, R_DL, R_AL, R_DL, R_DL},
			/*FACTION_B*/ {R_NO, R_DL, R_DL, R_DL, R_DL, R_DL, R_DL, R_DL, R_DL, R_DL, R_NO, R_DL, R_DL, R_DL, R_DL, R_AL, R_DL},
			/*FACTION_C*/ {R_NO, R_DL, R_DL, R_DL, R_DL, R_DL, R_DL, R_DL, R_DL, R_DL, R_NO, R_DL, R_DL, R_DL, R_DL, R_DL, R_AL}};

	int iTargClass = pTarget->Classify();

	if (iTargClass == CLASS_PLAYER && m_iPlayerReact) // LRC
	{
		if (m_iPlayerReact == 1) // Ignore player
			return R_NO;
		else if (m_iPlayerReact == 4)
			return R_HT;
		else if (m_afMemory & bits_MEMORY_PROVOKED)
			return R_HT;
		else
			return R_NO;
	}

	return iEnemy[Classify()][iTargClass];
}

//=========================================================
// CheckEnemy - part of the Condition collection process,
// gets and stores data and conditions pertaining to a monster's
// enemy. Returns true if Enemy LKP was updated.
//=========================================================
bool CBaseMonster::CheckEnemy(CBaseEntity* pEnemy)
{
	float flDistToEnemy;
	bool iUpdatedLKP; // set this to true if you update the EnemyLKP in this function.

	iUpdatedLKP = false;
	ClearConditions(bits_COND_ENEMY_FACING_ME);

	if (!FVisible(pEnemy))
	{
		ASSERT(!HasConditions(bits_COND_SEE_ENEMY));
		SetConditions(bits_COND_ENEMY_OCCLUDED);
	}
	else
		ClearConditions(bits_COND_ENEMY_OCCLUDED);

	if (!pEnemy->IsAlive())
	{
		SetConditions(bits_COND_ENEMY_DEAD);
		ClearConditions(bits_COND_SEE_ENEMY | bits_COND_ENEMY_OCCLUDED);
		return false;
	}

	Vector vecEnemyPos = pEnemy->pev->origin;
	// distance to enemy's origin
	flDistToEnemy = (vecEnemyPos - pev->origin).Length();
	vecEnemyPos.z += pEnemy->pev->size.z * 0.5;
	// distance to enemy's head
	float flDistToEnemy2 = (vecEnemyPos - pev->origin).Length();
	if (flDistToEnemy2 < flDistToEnemy)
		flDistToEnemy = flDistToEnemy2;
	else
	{
		// distance to enemy's feet
		vecEnemyPos.z -= pEnemy->pev->size.z;
		float flDistToEnemy2 = (vecEnemyPos - pev->origin).Length();
		if (flDistToEnemy2 < flDistToEnemy)
			flDistToEnemy = flDistToEnemy2;
	}

	if (HasConditions(bits_COND_SEE_ENEMY))
	{
		CBaseMonster* pEnemyMonster;

		iUpdatedLKP = true;
		m_vecEnemyLKP = pEnemy->pev->origin;

		pEnemyMonster = pEnemy->MyMonsterPointer();

		if (pEnemyMonster)
		{
			if (pEnemyMonster->FInViewCone(this))
			{
				SetConditions(bits_COND_ENEMY_FACING_ME);
			}
			else
				ClearConditions(bits_COND_ENEMY_FACING_ME);
		}

		if (pEnemy->pev->velocity != Vector(0, 0, 0))
		{
			// trail the enemy a bit
			m_vecEnemyLKP = m_vecEnemyLKP - pEnemy->pev->velocity * RANDOM_FLOAT(-0.05, 0);
		}
		else
		{
			// UNDONE: use pev->oldorigin?
		}
	}
	else if (!HasConditions(bits_COND_ENEMY_OCCLUDED | bits_COND_SEE_ENEMY) && (flDistToEnemy <= 256))
	{
		// if the enemy is not occluded, and unseen, that means it is behind or beside the monster.
		// if the enemy is near enough the monster, we go ahead and let the monster know where the
		// enemy is.
		iUpdatedLKP = true;
		m_vecEnemyLKP = pEnemy->pev->origin;
	}

	if (flDistToEnemy >= m_flDistTooFar)
	{
		// enemy is very far away from monster
		SetConditions(bits_COND_ENEMY_TOOFAR);
	}
	else
		ClearConditions(bits_COND_ENEMY_TOOFAR);

	if (FCanCheckAttacks())
	{
		CheckAttacks(m_hEnemy, flDistToEnemy);
	}

	if (m_movementGoal == MOVEGOAL_ENEMY)
	{
		for (int i = m_iRouteIndex; i < ROUTE_SIZE; i++)
		{
			if (m_Route[i].iType == (bits_MF_IS_GOAL | bits_MF_TO_ENEMY))
			{
				// UNDONE: Should we allow monsters to override this distance (80?)
				if ((m_Route[i].vecLocation - m_vecEnemyLKP).Length() > 80)
				{
					// Refresh
					FRefreshRoute();
					return iUpdatedLKP;
				}
			}
		}
	}

	return iUpdatedLKP;
}

//=========================================================
// PushEnemy - remember the last few enemies, always remember the player
//=========================================================
void CBaseMonster::PushEnemy(CBaseEntity* pEnemy, Vector& vecLastKnownPos)
{
	int i;

	if (pEnemy == NULL)
		return;

	// UNDONE: blah, this is bad, we should use a stack but I'm too lazy to code one.
	for (i = 0; i < MAX_OLD_ENEMIES; i++)
	{
		if (m_hOldEnemy[i] == pEnemy)
			return;
		if (m_hOldEnemy[i] == NULL) // someone died, reuse their slot
			break;
	}
	if (i >= MAX_OLD_ENEMIES)
		return;

	m_hOldEnemy[i] = pEnemy;
	m_vecOldEnemy[i] = vecLastKnownPos;
}

//=========================================================
// PopEnemy - try remembering the last few enemies
//=========================================================
bool CBaseMonster::PopEnemy()
{
	// UNDONE: blah, this is bad, we should use a stack but I'm too lazy to code one.
	for (int i = MAX_OLD_ENEMIES - 1; i >= 0; i--)
	{
		if (m_hOldEnemy[i] != NULL)
		{
			if (m_hOldEnemy[i]->IsAlive()) // cheat and know when they die
			{
				m_hEnemy = m_hOldEnemy[i];
				m_vecEnemyLKP = m_vecOldEnemy[i];
				// ALERT( at_console, "remembering\n");
				return true;
			}
			else
			{
				m_hOldEnemy[i] = NULL;
			}
		}
	}
	return false;
}

//=========================================================
// Get Enemy - tries to find the best suitable enemy for the monster.
//=========================================================
bool CBaseMonster::GetEnemy()
{
	CBaseEntity* pNewEnemy;

	if (HasConditions(bits_COND_SEE_HATE | bits_COND_SEE_DISLIKE | bits_COND_SEE_NEMESIS))
	{
		pNewEnemy = BestVisibleEnemy();

		if (pNewEnemy != m_hEnemy && pNewEnemy != NULL)
		{
			// DO NOT mess with the monster's m_hEnemy pointer unless the schedule the monster is currently running will be interrupted
			// by COND_NEW_ENEMY. This will eliminate the problem of monsters getting a new enemy while they are in a schedule that doesn't care,
			// and then not realizing it by the time they get to a schedule that does. I don't feel this is a good permanent fix.

			if (m_pSchedule)
			{
				if ((m_pSchedule->iInterruptMask & bits_COND_NEW_ENEMY) != 0)
				{
					PushEnemy(m_hEnemy, m_vecEnemyLKP);
					SetConditions(bits_COND_NEW_ENEMY);
					m_hEnemy = pNewEnemy;
					m_vecEnemyLKP = m_hEnemy->pev->origin;
				}
				// if the new enemy has an owner, take that one as well
				if (pNewEnemy->pev->owner != NULL)
				{
					CBaseEntity* pOwner = GetMonsterPointer(pNewEnemy->pev->owner);
					if (pOwner && (pOwner->pev->flags & FL_MONSTER) != 0 && IRelationship(pOwner) != R_NO)
						PushEnemy(pOwner, m_vecEnemyLKP);
				}
			}
		}
	}

	// remember old enemies
	if (m_hEnemy == NULL && PopEnemy())
	{
		if (m_pSchedule)
		{
			if ((m_pSchedule->iInterruptMask & bits_COND_NEW_ENEMY) != 0)
			{
				SetConditions(bits_COND_NEW_ENEMY);
			}
		}
	}

	if (m_hEnemy != NULL)
	{
		// monster has an enemy.
		return true;
	}

	return false; // monster has no enemy
}

//=========================================================
// BestVisibleEnemy - this functions searches the link
// list whose head is the caller's m_pLink field, and returns
// a pointer to the enemy entity in that list that is nearest the
// caller.
//
// !!!UNDONE - currently, this only returns the closest enemy.
// we'll want to consider distance, relationship, attack types, back turned, etc.
//=========================================================
CBaseEntity* CBaseMonster::BestVisibleEnemy()
{
	CBaseEntity* pReturn;
	CBaseEntity* pNextEnt;
	int iNearest;
	int iDist;
	int iBestRelationship;

	iNearest = 8192; // so first visible entity will become the closest.
	pNextEnt = m_pLink;
	pReturn = NULL;
	iBestRelationship = R_NO;

	while (pNextEnt != NULL)
	{
		if (pNextEnt->IsAlive())
		{
			if (IRelationship(pNextEnt) > iBestRelationship)
			{
				// this entity is disliked MORE than the entity that we
				// currently think is the best visible enemy. No need to do
				// a distance check, just get mad at this one for now.
				iBestRelationship = IRelationship(pNextEnt);
				iNearest = (pNextEnt->pev->origin - pev->origin).Length();
				pReturn = pNextEnt;
			}
			else if (IRelationship(pNextEnt) == iBestRelationship)
			{
				// this entity is disliked just as much as the entity that
				// we currently think is the best visible enemy, so we only
				// get mad at it if it is closer.
				iDist = (pNextEnt->pev->origin - pev->origin).Length();

				if (iDist <= iNearest)
				{
					iNearest = iDist;
					iBestRelationship = IRelationship(pNextEnt);
					pReturn = pNextEnt;
				}
			}
		}

		pNextEnt = pNextEnt->m_pLink;
	}

	return pReturn;
}
