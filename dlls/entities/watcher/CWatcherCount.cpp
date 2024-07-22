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

#include "CWatcherCount.h"

TYPEDESCRIPTION CWatcherCount::m_SaveData[] =
	{
		DEFINE_FIELD(CWatcherCount, m_iMode, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CWatcherCount, CBaseToggle);
LINK_ENTITY_TO_CLASS(watcher_count, CWatcherCount);

bool CWatcherCount::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "m_iMode"))
	{
		m_iMode = atof(pkvd->szValue);
		return true;
	}
	return CBaseToggle::KeyValue(pkvd);
}

void CWatcherCount::Spawn()
{
	pev->solid = SOLID_NOT;
	SetNextThink(0.5);
}

void CWatcherCount::Think()
{
	SetNextThink(0.1);
	int iCount = 0;
	CBaseEntity* pCurrent = NULL;

	pCurrent = UTIL_FindEntityByTargetname(NULL, STRING(pev->noise));
	while (pCurrent != NULL)
	{
		if (pCurrent->GetState() != STATE_OFF)
			iCount++;
		pCurrent = UTIL_FindEntityByTargetname(pCurrent, STRING(pev->noise));
	}
	/*	if (pev->spawnflags & SF_WRCOUNT_STARTED)
	{
		if (iCount > pev->frags)
		{
			if (iCount > pev->impulse && pev->frags <= pev->impulse)
				FireTargets(STRING(pev->netname), this, this, USE_TOGGLE, 0);
			FireTargets(STRING(pev->noise1), this, this, USE_TOGGLE, 0);
		}
		else if (iCount < pev->frags)
		{
			if (iCount <= pev->impulse && pev->frags > pev->impulse)
				FireTargets(STRING(pev->message), this, this, USE_TOGGLE, 0);
			FireTargets(STRING(pev->noise2), this, this, USE_TOGGLE, 0);
		}
	}
	else
	{
		pev->spawnflags |= SF_WRCOUNT_STARTED;
		if (pev->spawnflags & SF_WRCOUNT_FIRESTART)
		{
			if (iCount < pev->impulse)
				FireTargets(STRING(pev->netname), this, this, USE_TOGGLE, 0);
			else
				FireTargets(STRING(pev->message), this, this, USE_TOGGLE, 0);
		}
	}*/

	if (pev->spawnflags & SF_WRCOUNT_STARTED) // AJH old code was totally buggered
	{										  // this should actually work!!
		if (iCount > pev->iuser1)
		{
			FireTargets(STRING(pev->noise1), this, this, USE_TOGGLE, 0);
		}
		else if (iCount < pev->iuser1)
		{
			FireTargets(STRING(pev->noise2), this, this, USE_TOGGLE, 0);
		}

		switch (m_iMode)
		{
		case 0:
		{
			if (iCount == pev->impulse && pev->iuser1 != pev->impulse)
			{
				FireTargets(STRING(pev->netname), this, this, USE_TOGGLE, 0);
				FireTargets(STRING(pev->target), this, this, USE_TOGGLE, 0);
			}

			else if (iCount != pev->impulse && pev->iuser1 == pev->impulse)
			{
				FireTargets(STRING(pev->message), this, this, USE_TOGGLE, 0);
				FireTargets(STRING(pev->target), this, this, USE_TOGGLE, 0);
			}
			break;
		}

		case 1:
		{
			if (iCount > pev->impulse && pev->iuser1 <= pev->impulse)
			{
				FireTargets(STRING(pev->netname), this, this, USE_TOGGLE, 0);
				FireTargets(STRING(pev->target), this, this, USE_TOGGLE, 0);
			}
			else if (iCount <= pev->impulse && pev->iuser1 > pev->impulse)
			{
				FireTargets(STRING(pev->message), this, this, USE_TOGGLE, 0);
				FireTargets(STRING(pev->target), this, this, USE_TOGGLE, 0);
			}
			break;
		}

		case 2:
		{
			if (iCount < pev->impulse && pev->iuser1 >= pev->impulse)
			{
				FireTargets(STRING(pev->netname), this, this, USE_TOGGLE, 0);
				FireTargets(STRING(pev->target), this, this, USE_TOGGLE, 0);
			}
			else if (iCount >= pev->impulse && pev->iuser1 < pev->impulse)
			{
				FireTargets(STRING(pev->message), this, this, USE_TOGGLE, 0);
				FireTargets(STRING(pev->target), this, this, USE_TOGGLE, 0);
			}
			break;
		}

		case 3:
		{
			if (iCount != pev->impulse && pev->iuser1 == pev->impulse)
			{
				FireTargets(STRING(pev->netname), this, this, USE_TOGGLE, 0);
				FireTargets(STRING(pev->target), this, this, USE_TOGGLE, 0);
			}
			else if (iCount == pev->impulse && pev->iuser1 != pev->impulse)
			{
				FireTargets(STRING(pev->message), this, this, USE_TOGGLE, 0);
				FireTargets(STRING(pev->target), this, this, USE_TOGGLE, 0);
			}
			break;
		}
		case 4:
		{
			if (iCount <= pev->impulse && pev->iuser1 > pev->impulse)
			{
				FireTargets(STRING(pev->netname), this, this, USE_TOGGLE, 0);
				FireTargets(STRING(pev->target), this, this, USE_TOGGLE, 0);
			}
			else if (iCount > pev->impulse && pev->iuser1 <= pev->impulse)
			{
				FireTargets(STRING(pev->message), this, this, USE_TOGGLE, 0);
				FireTargets(STRING(pev->target), this, this, USE_TOGGLE, 0);
			}
			break;
		}
		case 5:
		{
			if (iCount >= pev->impulse && pev->iuser1 < pev->impulse)
			{
				FireTargets(STRING(pev->netname), this, this, USE_TOGGLE, 0);
				FireTargets(STRING(pev->target), this, this, USE_TOGGLE, 0);
			}
			else if (iCount < pev->impulse && pev->iuser1 >= pev->impulse)
			{
				FireTargets(STRING(pev->message), this, this, USE_TOGGLE, 0);
				FireTargets(STRING(pev->target), this, this, USE_TOGGLE, 0);
			}
			break;
		}
		default:
		{
			break;
		}
		}
	}
	else
	{
		pev->spawnflags |= SF_WRCOUNT_STARTED;
		if (pev->spawnflags & SF_WRCOUNT_FIRESTART)
		{
			if (iCount < pev->impulse)
				FireTargets(STRING(pev->netname), this, this, USE_TOGGLE, 0);
			else
				FireTargets(STRING(pev->message), this, this, USE_TOGGLE, 0);
		}
	}
	pev->iuser1 = iCount;
}
