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

#include "CTankSequence.h"
#include "entities/func/CFuncTank.h"

LINK_ENTITY_TO_CLASS(scripted_tanksequence, CTankSequence);

TYPEDESCRIPTION CTankSequence::m_SaveData[] =
	{
		DEFINE_FIELD(CTankSequence, m_iszEntity, FIELD_STRING),
		DEFINE_FIELD(CTankSequence, m_iszEnemy, FIELD_STRING),
		DEFINE_FIELD(CTankSequence, m_iUntil, FIELD_INTEGER),
		DEFINE_FIELD(CTankSequence, m_fDuration, FIELD_FLOAT),
		DEFINE_FIELD(CTankSequence, m_iTurn, FIELD_INTEGER),
		DEFINE_FIELD(CTankSequence, m_iShoot, FIELD_INTEGER),
		DEFINE_FIELD(CTankSequence, m_iActive, FIELD_INTEGER),
		DEFINE_FIELD(CTankSequence, m_iControllable, FIELD_INTEGER),
		DEFINE_FIELD(CTankSequence, m_iLaserSpot, FIELD_INTEGER),
		DEFINE_FIELD(CTankSequence, m_pTank, FIELD_CLASSPTR),
};

IMPLEMENT_SAVERESTORE(CTankSequence, CBaseEntity);

int CTankSequence::ObjectCaps()
{
	return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION);
}

bool CTankSequence::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "m_iUntil"))
	{
		m_iUntil = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iTurn"))
	{
		m_iTurn = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iShoot"))
	{
		m_iShoot = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iActive"))
	{
		m_iActive = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iControllable"))
	{
		m_iControllable = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iLaserSpot"))
	{
		m_iLaserSpot = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszEntity"))
	{
		m_iszEntity = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszEnemy"))
	{
		m_iszEnemy = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	return CBaseEntity::KeyValue(pkvd);
}

void CTankSequence::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (!ShouldToggle(useType))
		return;

	if (GetState() == STATE_OFF)
	{
		// take control of the tank, start the sequence

		CBaseEntity* pEnt = UTIL_FindEntityByTargetname(NULL, STRING(m_iszEntity));
		if (!pEnt || !FStrEq(STRING(pEnt->pev->classname), "func_tank"))
		{
			ALERT(at_error, "Invalid or missing tank \"%s\" for scripted_tanksequence!\n", STRING(m_iszEntity));
			return;
		}
		CFuncTank* pTank = (CFuncTank*)pEnt;

		// check whether it's being controlled by another sequence
		if (pTank->m_pSequence)
			return;

		// check whether it's being controlled by the player
		if (pTank->m_pControls)
		{
			if (pev->spawnflags & SF_TSEQ_DUMPPLAYER)
			{
				pTank->StopControl(pTank->m_pControls);
			}
			else if (!FBitSet(pev->spawnflags, SF_TSEQ_DUMPPLAYER))
			{
				// ALERT(at_debug, "scripted_tanksequence can't take tank away from player\n");
				return;
			}
		}

		// passed all the tests, so we can now take control of it.
		if (m_iTurn == TSEQ_TURN_ENEMY)
		{
			CBaseEntity* pEnemy;
			if (m_iszEnemy)
				pEnemy = UTIL_FindEntityGeneric(STRING(m_iszEnemy), pTank->pev->origin, pTank->m_maxRange);
			else
				pEnemy = pTank->BestVisibleEnemy();

			if (pEnemy)
			{
				pTank->m_pSequenceEnemy = pEnemy;
				pTank->StartSequence(this);
			}
		}
		else
		{
			pTank->StartSequence(this);
		}

		if (m_iShoot == TSEQ_SHOOT_ALWAYS)
			pTank->pev->spawnflags |= SF_TANK_SEQFIRE;
		else
			pTank->pev->spawnflags &= ~SF_TANK_SEQFIRE;

		m_pTank = pTank;
		if (m_fDuration)
		{
			SetThink(&CTankSequence::TimeOutThink);
			SetNextThink(m_fDuration);
		}
	}
	else // don't check UNTIL_TRIGGER - any UNTIL value can be prematurely ended.
	{
		// disable any other end conditions
		DontThink();

		// release control of the tank
		StopSequence();
	}
}

void CTankSequence::FacingNotify()
{
	if (m_iUntil == TSEQ_UNTIL_FACING)
	{
		SetThink(&CTankSequence::EndThink);
		SetNextThink(0);
	}
	else if (m_iShoot == TSEQ_SHOOT_FACING)
		m_pTank->pev->spawnflags |= SF_TANK_SEQFIRE;
}

void CTankSequence::DeadEnemyNotify()
{
	if (m_iUntil == TSEQ_UNTIL_DEATH)
	{
		SetThink(&CTankSequence::EndThink);
		SetNextThink(0);
	}
	//	else
	//		m_pTank->pev->spawnflags &= ~SF_TANK_SEQFIRE; // if the enemy's dead, stop firing
}

void CTankSequence::EndThink()
{
	// the sequence has expired. Release control of the tank.
	StopSequence();
	if (!FStringNull(pev->target))
		FireTargets(STRING(pev->target), this, this, USE_TOGGLE, 0);
}

void CTankSequence::TimeOutThink()
{
	// the sequence has timed out. Release control of the tank.
	StopSequence();
	if (!FStringNull(pev->netname))
		FireTargets(STRING(pev->netname), this, this, USE_TOGGLE, 0);
}

void CTankSequence::StopSequence()
{
	if (!m_pTank)
	{
		ALERT(at_error, "TankSeq: StopSequence with no tank!\n");
		return; // this shouldn't happen. Just insurance...
	}

	// if we're doing "shoot at end", fire that shot now.
	if (m_iShoot == TSEQ_SHOOT_ONCE)
	{
		m_pTank->m_fireLast = gpGlobals->time - 1 / m_pTank->m_fireRate; // exactly one shot.
		Vector forward;
		UTIL_MakeVectorsPrivate(m_pTank->pev->angles, forward, NULL, NULL);
		m_pTank->TryFire(m_pTank->BarrelPosition(), forward, m_pTank->pev);
	}

	if (m_iLaserSpot)
	{
		if (m_pTank->pev->spawnflags & SF_TANK_LASERSPOT && m_iLaserSpot != TSEQ_FLAG_ON)
		{
			m_pTank->pev->spawnflags &= ~SF_TANK_LASERSPOT;
		}
		else if (!FBitSet(m_pTank->pev->spawnflags, SF_TANK_LASERSPOT) && m_iLaserSpot != TSEQ_FLAG_OFF)
		{
			m_pTank->pev->spawnflags |= SF_TANK_LASERSPOT;
		}
	}

	if (m_iControllable)
	{
		if (m_pTank->pev->spawnflags & SF_TANK_CANCONTROL && m_iControllable != TSEQ_FLAG_ON)
		{
			m_pTank->pev->spawnflags &= ~SF_TANK_CANCONTROL;
		}
		else if (!(m_pTank->pev->spawnflags & SF_TANK_CANCONTROL) && m_iControllable != TSEQ_FLAG_OFF)
		{
			m_pTank->pev->spawnflags |= SF_TANK_CANCONTROL;
		}
	}

	m_pTank->StopSequence();

	if (!FBitSet(pev->spawnflags, SF_TSEQ_REPEATABLE))
		UTIL_Remove(this);

	if (m_pTank->IsActive() && (m_iActive == TSEQ_FLAG_OFF || m_iActive == TSEQ_FLAG_TOGGLE))
	{
		m_pTank->TankDeactivate();
		if (m_pTank->m_pSpot)
			m_pTank->m_pSpot->Suspend(-1);
	}
	else if (!m_pTank->IsActive() && (m_iActive == TSEQ_FLAG_ON || m_iActive == TSEQ_FLAG_TOGGLE))
	{
		m_pTank->TankActivate();
		if (m_pTank->m_pSpot)
			m_pTank->m_pSpot->Revive();
	}

	m_pTank = NULL;
}
