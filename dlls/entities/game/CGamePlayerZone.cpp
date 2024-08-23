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

#include "CGamePlayerZone.h"

#include "util/trace.h"

LINK_ENTITY_TO_CLASS(game_zone_player, CGamePlayerZone);

TYPEDESCRIPTION CGamePlayerZone::m_SaveData[] =
	{
		DEFINE_FIELD(CGamePlayerZone, m_iszInTarget, FIELD_STRING),
		DEFINE_FIELD(CGamePlayerZone, m_iszOutTarget, FIELD_STRING),
		DEFINE_FIELD(CGamePlayerZone, m_iszInCount, FIELD_STRING),
		DEFINE_FIELD(CGamePlayerZone, m_iszOutCount, FIELD_STRING),
};

IMPLEMENT_SAVERESTORE(CGamePlayerZone, CRuleBrushEntity);

bool CGamePlayerZone::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "intarget"))
	{
		m_iszInTarget = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "outtarget"))
	{
		m_iszOutTarget = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "incount"))
	{
		m_iszInCount = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "outcount"))
	{
		m_iszOutCount = ALLOC_STRING(pkvd->szValue);
		return true;
	}

	return CRuleBrushEntity::KeyValue(pkvd);
}

void CGamePlayerZone::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	int playersInCount = 0;
	int playersOutCount = 0;

	if (!CanFireForActivator(pActivator))
		return;

	CBaseEntity* pPlayer = NULL;

	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		pPlayer = UTIL_PlayerByIndex(i);
		if (pPlayer)
		{
			TraceResult trace;
			HULL_NUMBER hullNumber;
			bool inside = false;

			hullNumber = human_hull;
			if ((pPlayer->pev->flags & FL_DUCKING) != 0)
			{
				hullNumber = human_hull;
				if ((pPlayer->pev->flags & FL_DUCKING) != 0)
				{
					hullNumber = head_hull;
				}
				UTIL_TraceModel(pPlayer->pev->origin, pPlayer->pev->origin, hullNumber, edict(), &trace);
				inside = 0 != trace.fStartSolid;
			}
			else
			{
				// LIMITATION: this doesn't allow for non-cuboid game_zone_player entities.
				//  (is that a problem?)
				inside = this->Intersects(pPlayer);
			}

			if (inside)
			{
				playersInCount++;
				if (!FStringNull(m_iszInTarget))
				{
					FireTargets(STRING(m_iszInTarget), pPlayer, pActivator, useType, value);
				}
			}
			else
			{
				playersOutCount++;
				if (!FStringNull(m_iszOutTarget))
				{
					FireTargets(STRING(m_iszOutTarget), pPlayer, pActivator, useType, value);
				}
			}
		}
	}

	if (!FStringNull(m_iszInCount))
	{
		FireTargets(STRING(m_iszInCount), pActivator, this, USE_SET, playersInCount);
	}

	if (!FStringNull(m_iszOutCount))
	{
		FireTargets(STRING(m_iszOutCount), pActivator, this, USE_SET, playersOutCount);
	}
}
