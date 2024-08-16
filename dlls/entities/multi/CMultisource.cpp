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

#include "CMultisource.h"
#include "classes/saverestore/CGlobalState.h"

LINK_ENTITY_TO_CLASS(multisource, CMultiSource);

TYPEDESCRIPTION CMultiSource::m_SaveData[] =
	{
		//!!!BUGBUG FIX
		DEFINE_ARRAY(CMultiSource, m_rgEntities, FIELD_EHANDLE, MS_MAX_TARGETS),
		DEFINE_ARRAY(CMultiSource, m_rgTriggered, FIELD_INTEGER, MS_MAX_TARGETS),
		DEFINE_FIELD(CMultiSource, m_iTotal, FIELD_INTEGER),
		DEFINE_FIELD(CMultiSource, m_globalstate, FIELD_STRING),
};

IMPLEMENT_SAVERESTORE(CMultiSource, CBaseEntity);

//
// Cache user-entity-field values until spawn is called.
//
bool CMultiSource::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "style") ||
		FStrEq(pkvd->szKeyName, "height") ||
		FStrEq(pkvd->szKeyName, "killtarget") ||
		FStrEq(pkvd->szKeyName, "value1") ||
		FStrEq(pkvd->szKeyName, "value2") ||
		FStrEq(pkvd->szKeyName, "value3"))
		return true;
	else if (FStrEq(pkvd->szKeyName, "globalstate"))
	{
		m_globalstate = ALLOC_STRING(pkvd->szValue);
		return true;
	}

	return CPointEntity::KeyValue(pkvd);
}

void CMultiSource::Spawn()
{
	// set up think for later registration

	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	SetNextThink(0.1);
	pev->spawnflags |= SF_MULTI_INIT; // Until it's initialized
	SetThink(&CMultiSource::Register);
}

void CMultiSource::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	int i = 0;

	// Find the entity in our list
	while (i < m_iTotal)
		if (m_rgEntities[i++] == pCaller)
			break;

	// if we didn't find it, report error and leave
	if (i > m_iTotal)
	{
		if (pCaller->pev->targetname)
			ALERT(at_debug, "multisource \"%s\": Used by non-member %s \"%s\"\n", STRING(pev->targetname), STRING(pCaller->pev->classname), STRING(pCaller->pev->targetname));
		else
			ALERT(at_debug, "multisource \"%s\": Used by non-member %s\n", STRING(pev->targetname), STRING(pCaller->pev->classname));
		return;
	}

	// CONSIDER: a Use input to the multisource always toggles.  Could check useType for ON/OFF/TOGGLE
	// LRC- On could be meaningful. Off, sadly, can't work in the obvious manner.
	// LRC (09/06/01)- er... why not?
	// LRC (28/04/02)- that depends what the "obvious" manner is.

	// store the state before the change, so we can compare it to the new state
	STATE s = GetState();

	// do the change
	m_rgTriggered[i - 1] ^= 1;

	// did we change state?
	if (s == GetState())
		return;

	if (s == STATE_ON && pev->netname)
	{
		// the change disabled me and I have a "fire on disable" field
		ALERT(at_aiconsole, "Multisource %s deactivated (%d inputs)\n", STRING(pev->targetname), m_iTotal);
		if (m_globalstate)
			FireTargets(STRING(pev->netname), NULL, this, USE_OFF, 0);
		else
			FireTargets(STRING(pev->netname), NULL, this, USE_TOGGLE, 0);
	}
	else if (s == STATE_OFF)
	{
		// the change activated me
		ALERT(at_aiconsole, "Multisource %s enabled (%d inputs)\n", STRING(pev->targetname), m_iTotal);
		USE_TYPE useType = USE_TOGGLE;
		if (!FStringNull(m_globalstate))
			useType = USE_ON;
		SUB_UseTargets(NULL, useType, 0);
	}
}

// LRC- while we're in STATE_OFF, mastered entities can't do anything.
STATE CMultiSource::GetState()
{
	// Is everything triggered?
	int i = 0;

	// Still initializing?
	if ((pev->spawnflags & SF_MULTI_INIT) != 0)
		return STATE_OFF;

	while (i < m_iTotal)
	{
		if (m_rgTriggered[i] == 0)
			break;
		i++;
	}

	if (i == m_iTotal)
	{
		if (FStringNull(m_globalstate) || gGlobalState.EntityGetState(m_globalstate) == GLOBAL_ON)
			return STATE_ON;
	}

	return STATE_OFF;
}

void CMultiSource::Register()
{
	m_iTotal = 0;
	memset(m_rgEntities, 0, MS_MAX_TARGETS * sizeof(EHANDLE));

	SetThink(&CMultiSource::SUB_DoNothing);

	// search for all entities which target this multisource (pev->targetname)

	CBaseEntity* pTarget = UTIL_FindEntityByTarget(NULL, STRING(pev->targetname));
	while (pTarget && (m_iTotal < MS_MAX_TARGETS))
	{
		m_rgEntities[m_iTotal++] = pTarget;

		pTarget = UTIL_FindEntityByTarget(pTarget, STRING(pev->targetname));
	}

	pTarget = UTIL_FindEntityByClassname(NULL, "multi_manager");
	while (pTarget && (m_iTotal < MS_MAX_TARGETS))
	{
		if (pTarget->HasTarget(pev->targetname))
			m_rgEntities[m_iTotal++] = pTarget;

		pTarget = UTIL_FindEntityByClassname(pTarget, "multi_manager");
	}

	if (m_iTotal >= MS_MAX_TARGETS)
	{
		ALERT(at_debug, "WARNING: There are too many entities targetting multisource \"%s\". (limit is %d)\n", STRING(pev->targetname), MS_MAX_TARGETS);
	}

	pev->spawnflags &= ~SF_MULTI_INIT;
}
