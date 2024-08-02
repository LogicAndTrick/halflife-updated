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

#include "CInfoAlias.h"

LINK_ENTITY_TO_CLASS(info_alias, CInfoAlias);

TYPEDESCRIPTION CInfoAlias::m_SaveData[] = // AJH
	{
		DEFINE_FIELD(CInfoAlias, m_cTargets, FIELD_INTEGER),
		DEFINE_FIELD(CInfoAlias, m_iMode, FIELD_INTEGER),
		DEFINE_FIELD(CInfoAlias, m_iCurrentTarget, FIELD_INTEGER),
		DEFINE_ARRAY(CInfoAlias, m_iTargetName, FIELD_STRING, MAX_ALIAS_TARGETS),
		//	DEFINE_FIELD( CInfoAlias, m_pNextAlias, FIELD_CLASSPTR ),
};

IMPLEMENT_SAVERESTORE(CInfoAlias, CBaseMutableAlias);

bool CInfoAlias ::KeyValue(KeyValueData* pkvd) // AJH
{
	if (FStrEq(pkvd->szKeyName, "mode"))
	{
		m_iMode = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "targetname"))
	{
		pev->targetname = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "target"))
	{
		pev->target = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "netname"))
	{
		pev->netname = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else // add this field to the target list
	{
		// this assumes that additional fields are targetnames, and there values are the order.
		if (m_cTargets < MAX_ALIAS_TARGETS)
		{
			char tmp[128];

			UTIL_StripToken(pkvd->szKeyName, tmp);
			int iValue = atoi(pkvd->szValue);
			if (iValue <= MAX_ALIAS_TARGETS && iValue > 0)
			{
				if (m_iTargetName[iValue - 1] == NULL)
				{
					m_iTargetName[iValue - 1] = ALLOC_STRING(tmp);
					m_cTargets++;
				}
				else
				{
					ALERT(at_debug, "ERROR: INFO_ALIAS target \"%s\" has a value \"%i\" that has already been used by target \"%s\"\n", ALLOC_STRING(tmp), iValue, STRING(m_iTargetName[iValue - 1]));
				}
			}
			else
			{
				ALERT(at_debug, "ERROR: INFO_ALIAS target \"%s\" has an illegal value \"%i\".\nIt must be within the range 1-%i.\n", ALLOC_STRING(tmp), iValue, MAX_ALIAS_TARGETS);
			}
			return true;
		}
		// We can't actually have this or we will get array out of bounds exceptions when +using the alias at MAX_ALIAS_TARGETS
		/*else // keep a count of how many targets, for the error message
		{
			m_cTargets++;
		}*/
	}
	return CBaseMutableAlias::KeyValue(pkvd);
}

void CInfoAlias::Spawn(void)
{
	if (m_iMode == 0)
	{
		if (pev->spawnflags & SF_ALIAS_OFF)
			pev->message = pev->netname;
		else
			pev->message = pev->target;
	}
	else
	{
		if (pev->spawnflags & SF_ALIAS_DEBUG) // Don't really need this much debug info
			ALERT(at_debug, "DEBUG: info_alias %s contains %d targets\n", STRING(pev->targetname), m_cTargets);
		if (m_cTargets > MAX_ALIAS_TARGETS)
		{
			ALERT(at_debug, "WARNING: info_alias \"%s\" has too many targets (limit is %d)\n", STRING(pev->targetname), MAX_ALIAS_TARGETS);
			m_cTargets = MAX_ALIAS_TARGETS;
		}
		else
		{
			m_iCurrentTarget = 0;
			pev->message = m_iTargetName[m_iCurrentTarget];
			pev->noise = m_iTargetName[m_iCurrentTarget];
		}
	}
}

void CInfoAlias::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (m_iMode == 0)
	{ // Old On/Off Code

		if (pev->spawnflags & SF_ALIAS_OFF)
		{
			if (pev->spawnflags & SF_ALIAS_DEBUG)
				ALERT(at_debug, "DEBUG: info_alias %s turns on\n", STRING(pev->targetname));
			pev->spawnflags &= ~SF_ALIAS_OFF;
			pev->noise = pev->target;
		}
		else
		{
			if (pev->spawnflags & SF_ALIAS_DEBUG)
				ALERT(at_debug, "DEBUG: info_alias %s turns off\n", STRING(pev->targetname));
			pev->spawnflags |= SF_ALIAS_OFF;
			pev->noise = pev->netname;
		}
		UTIL_AddToAliasList(this);
	}

	else
	{ // AJH - new list mode for info_alias
		pev->noise = m_iTargetName[m_iCurrentTarget];
		//	pev->message = m_iTargetName[m_iCurrentTarget];
		if (useType == USE_OFF)
		{
			m_iCurrentTarget--;
			if (m_iCurrentTarget <= -1)
				m_iCurrentTarget = m_cTargets;
		}
		else
		{
			m_iCurrentTarget++;
			if (m_iCurrentTarget >= m_cTargets)
				m_iCurrentTarget = 0;
		}


		if (pev->spawnflags & SF_ALIAS_DEBUG)
		{
			ALERT(at_debug, "DEBUG: info_alias %s  refers to target entity number %d \n", STRING(pev->targetname), m_iTargetName[m_iCurrentTarget]);
			ALERT(at_debug, "DEBUG: info_alias %s  steps to target %d \n", STRING(pev->targetname), m_iCurrentTarget);
		}
		UTIL_AddToAliasList(this);
	}
}

CBaseEntity* CInfoAlias::FollowAlias(CBaseEntity* pFrom)
{
	CBaseEntity* pFound = UTIL_FindEntityByTargetname(pFrom, STRING(pev->message));

	if (pev->spawnflags & SF_ALIAS_DEBUG)
	{ // More excessive debug info
		ALERT(at_debug, "DEBUG: info_alias %s  refers to target %d \n", STRING(pev->targetname), m_iCurrentTarget);
		ALERT(at_debug, "DEBUG: info_alias %s  refers to target entity %s \n", STRING(pev->targetname), STRING(pev->message));

		if (pFound)
			ALERT(at_debug, "DEBUG: info_alias %s  refers to target entity %s \n", STRING(pev->targetname), STRING(pFound->pev->targetname));
	}
	return pFound;
}

void CInfoAlias::ChangeValue(int iszValue)
{
	pev->noise = iszValue;
	UTIL_AddToAliasList(this);
}

void CInfoAlias::FlushChanges()
{
	pev->message = pev->noise;
	if (pev->spawnflags & SF_ALIAS_DEBUG)
		ALERT(at_debug, "DEBUG: info_alias %s now refers to \"%s\"\n", STRING(pev->targetname), STRING(pev->message));
}
