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

#include "CBaseDelay.h"

LINK_ENTITY_TO_CLASS(DelayedUse, CBaseDelay);

// Global Savedata for Delay
TYPEDESCRIPTION CBaseDelay::m_SaveData[] =
	{
		DEFINE_FIELD(CBaseDelay, m_flDelay, FIELD_FLOAT),
		DEFINE_FIELD(CBaseDelay, m_iszKillTarget, FIELD_STRING),
		DEFINE_FIELD(CBaseDelay, m_hActivator, FIELD_EHANDLE), // LRC
};

IMPLEMENT_SAVERESTORE(CBaseDelay, CBaseEntity);

bool CBaseDelay::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "delay"))
	{
		m_flDelay = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "killtarget"))
	{
		m_iszKillTarget = ALLOC_STRING(pkvd->szValue);
		return true;
	}

	return CBaseEntity::KeyValue(pkvd);
}

void CBaseDelay::SUB_UseTargets(CBaseEntity* pActivator, USE_TYPE useType, float value)
{
	//
	// exit immediatly if we don't have a target or kill target
	//
	if (FStringNull(pev->target) && FStringNull(m_iszKillTarget))
		return;

	//
	// check for a delay
	//
	if (m_flDelay != 0)
	{
		// create a temp object to fire at a later time
		CBaseDelay* pTemp = GetClassPtr((CBaseDelay*)NULL);
		pTemp->pev->classname = MAKE_STRING("DelayedUse");

		pTemp->SetNextThink(m_flDelay);

		pTemp->SetThink(&CBaseDelay::DelayThink);

		// Save the useType
		pTemp->pev->button = (int)useType;
		pTemp->m_iszKillTarget = m_iszKillTarget;
		pTemp->m_flDelay = 0; // prevent "recursion"
		pTemp->pev->target = pev->target;

		// LRC - Valve had a hacked thing here to avoid breaking
		//  save/restore. In Spirit that's not a problem.
		//  I've moved m_hActivator into this class, for the "elegant" fix.
		pTemp->m_hActivator = pActivator;

		return;
	}

	//
	// kill the killtargets
	//

	if (!FStringNull(m_iszKillTarget))
	{
		edict_t* pentKillTarget = NULL;

		ALERT(at_aiconsole, "KillTarget: %s\n", STRING(m_iszKillTarget));
		// LRC- now just USE_KILLs its killtarget, for consistency.
		FireTargets(STRING(m_iszKillTarget), pActivator, this, USE_KILL, 0);
	}

	//
	// fire targets
	//
	if (!FStringNull(pev->target))
	{
		FireTargets(STRING(pev->target), pActivator, this, useType, value);
	}
}

void CBaseDelay::DelayThink()
{
	CBaseEntity* pActivator = NULL;

	// The use type is cached (and stashed) in pev->button
	// LRC - now using m_hActivator.
	SUB_UseTargets(m_hActivator, (USE_TYPE)pev->button, 0);
	REMOVE_ENTITY(ENT(pev));
}
