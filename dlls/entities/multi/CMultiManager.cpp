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

#include "CMultiManager.h"
#include "util/movewith.h"

LINK_ENTITY_TO_CLASS(multi_manager, CMultiManager);

// Global Savedata for multi_manager
TYPEDESCRIPTION CMultiManager::m_SaveData[] =
	{
		DEFINE_FIELD(CMultiManager, m_cTargets, FIELD_INTEGER),
		DEFINE_FIELD(CMultiManager, m_index, FIELD_INTEGER),
		DEFINE_FIELD(CMultiManager, m_iState, FIELD_INTEGER), // LRC
		DEFINE_FIELD(CMultiManager, m_iMode, FIELD_INTEGER),  // LRC
		DEFINE_FIELD(CMultiManager, m_startTime, FIELD_TIME),
		DEFINE_FIELD(CMultiManager, m_triggerType, FIELD_INTEGER), // LRC
		DEFINE_ARRAY(CMultiManager, m_iTargetName, FIELD_STRING, MAX_MULTI_TARGETS),
		DEFINE_ARRAY(CMultiManager, m_flTargetDelay, FIELD_FLOAT, MAX_MULTI_TARGETS),
		DEFINE_FIELD(CMultiManager, m_sMaster, FIELD_STRING), // LRC
		DEFINE_FIELD(CMultiManager, m_hActivator, FIELD_EHANDLE),
		DEFINE_FIELD(CMultiManager, m_flWait, FIELD_FLOAT),			 // LRC
		DEFINE_FIELD(CMultiManager, m_flMaxWait, FIELD_FLOAT),		 // LRC
		DEFINE_FIELD(CMultiManager, m_iszThreadName, FIELD_STRING),	 // LRC
		DEFINE_FIELD(CMultiManager, m_iszLocusThread, FIELD_STRING), // LRC
};

IMPLEMENT_SAVERESTORE(CMultiManager, CBaseEntity);

bool CMultiManager::KeyValue(KeyValueData* pkvd)
{

	// UNDONE: Maybe this should do something like this:
	// CBaseToggle::KeyValue( pkvd );
	// if ( !pkvd->fHandled )
	// ... etc.
	//
	// LRC- that would support Delay, Killtarget, Lip, Distance, Wait and Master.
	// Wait is already supported. I've added master here. To hell with the others.

	if (FStrEq(pkvd->szKeyName, "wait"))
	{
		m_flWait = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "maxwait"))
	{
		m_flMaxWait = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "master")) // LRC
	{
		m_sMaster = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszThreadName")) // LRC
	{
		m_iszThreadName = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszLocusThread")) // LRC
	{
		m_iszLocusThread = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "mode")) // LRC
	{
		m_iMode = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "triggerstate")) // LRC
	{
		switch (atoi(pkvd->szValue))
		{
		case 4:
			pev->spawnflags |= SF_MULTIMAN_SAMETRIG;
			break;
		case 1:
			m_triggerType = USE_ON;
			break; // LRC- yes, this algorithm is different
		case 2:
			m_triggerType = USE_OFF;
			break; // from the trigger_relay equivalent-
		case 3:
			m_triggerType = USE_KILL;
			break; // trigger_relay's got to stay backwards
		default:
			m_triggerType = USE_TOGGLE;
			break; // compatible.
		}
		pev->spawnflags |= SF_MULTIMAN_TRIGCHOSEN;
		return true;
	}
	else // add this field to the target list
	{
		// this assumes that additional fields are targetnames and their values are delay values.
		if (m_cTargets < MAX_MULTI_TARGETS)
		{
			char tmp[128];

			UTIL_StripToken(pkvd->szKeyName, tmp);
			m_iTargetName[m_cTargets] = ALLOC_STRING(tmp);
			m_flTargetDelay[m_cTargets] = atof(pkvd->szValue);
			m_cTargets++;
			return true;
		}
		else // LRC - keep a count of how many targets, for the error message
		{
			m_cTargets++;
		}
	}

	return false;
}

void CMultiManager::Spawn()
{
	CBaseEntity* pTarget;
	pTarget = UTIL_FindEntityByTargetname(NULL, STRING(pev->targetname));
	if (pTarget = UTIL_FindEntityByTargetname(NULL, "gen_lightsmm2"))
	{
		ALERT(at_debug, "DEBUG: multi_manager \"%s\": killed.\n", STRING(pev->targetname));
		UTIL_Remove(this);
		// G-Cont. this code is fixed BUG for blowout map c1a0a
	}

	pev->solid = SOLID_NOT;
	SetUse(&CMultiManager::ManagerUse);
	SetThink(&CMultiManager::ManagerThink);

	m_iState = STATE_OFF;
	m_startTime = 0;

	// LRC
	if (m_cTargets > MAX_MULTI_TARGETS)
	{
		ALERT(at_debug, "WARNING: multi_manager \"%s\" has too many targets (limit is %d, it has %d)\n", STRING(pev->targetname), MAX_MULTI_TARGETS, m_cTargets);
		m_cTargets = MAX_MULTI_TARGETS;
	}

	if (!FBitSet(pev->spawnflags, SF_MULTIMAN_TRIGCHOSEN))
		m_triggerType = USE_TOGGLE;

	// Sort targets
	// Quick and dirty bubble sort
	bool swapped = true;

	while (swapped)
	{
		swapped = false;
		for (int i = 1; i < m_cTargets; i++)
		{
			if (m_flTargetDelay[i] < m_flTargetDelay[i - 1])
			{
				// Swap out of order elements
				int name = m_iTargetName[i];
				float delay = m_flTargetDelay[i];
				m_iTargetName[i] = m_iTargetName[i - 1];
				m_flTargetDelay[i] = m_flTargetDelay[i - 1];
				m_iTargetName[i - 1] = name;
				m_flTargetDelay[i - 1] = delay;
				swapped = true;
			}
		}
	}

	if (pev->spawnflags & SF_MULTIMAN_SPAWNFIRE)
	{
		SetThink(&CMultiManager::UseThink);
		SetUse(NULL);
		UTIL_DesiredThink(this);
	}
}

bool CMultiManager::HasTarget(string_t targetname)
{
	for (int i = 0; i < m_cTargets; i++)
		if (FStrEq(STRING(targetname), STRING(m_iTargetName[i])))
			return true;

	return false;
}

void CMultiManager::UseThink()
{
	SetThink(&CMultiManager::ManagerThink);
	SetUse(&CMultiManager::ManagerUse);
	Use(this, this, USE_TOGGLE, 0);
}

// Designers were using this to fire targets that may or may not exist --
// so I changed it to use the standard target fire code, made it a little simpler.
void CMultiManager::ManagerThink()
{
	// LRC- different manager modes
	if (m_iMode)
	{
		// special triggers have no time delay, so we can clean up before firing
		if (pev->spawnflags & SF_MULTIMAN_LOOP)
		{
			//			ALERT(at_console,"Manager loops back\n");
			// if it's a loop, start again!
			if (m_flMaxWait) // LRC- random time to wait?
				m_startTime = RANDOM_FLOAT(m_flWait, m_flMaxWait);
			else if (m_flWait) // LRC- constant time to wait?
				m_startTime = m_flWait;
			else // LRC- just start immediately.
				m_startTime = 0;
			if (pev->spawnflags & SF_MULTIMAN_DEBUG)
				ALERT(at_debug, "DEBUG: multi_manager \"%s\": restarting loop.\n", STRING(pev->targetname));
			SetNextThink(m_startTime);
			m_startTime = m_fNextThink;
			m_iState = STATE_TURN_ON;
			//			ALERT(at_console, "MM loops, nextthink %f\n", m_fNextThink);
		}
		else if (IsClone() || pev->spawnflags & SF_MULTIMAN_ONLYONCE)
		{
			if (pev->spawnflags & SF_MULTIMAN_DEBUG)
				ALERT(at_debug, "DEBUG: multi_manager \"%s\": killed.\n", STRING(pev->targetname));
			SetThink(&CMultiManager::SUB_Remove);
			SetNextThink(0.1);
			SetUse(NULL);
		}
		else
		{
			if (pev->spawnflags & SF_MULTIMAN_DEBUG)
				ALERT(at_debug, "DEBUG: multi_manager \"%s\": last burst.\n", STRING(pev->targetname));
			m_iState = STATE_OFF;
			SetThink(NULL);
			SetUse(&CMultiManager::ManagerUse); // allow manager re-use
		}

		int i = 0;
		if (m_iMode == MM_MODE_CHOOSE) // choose one of the members, and fire it
		{
			float total = 0;
			for (i = 0; i < m_cTargets; i++)
			{
				total += m_flTargetDelay[i];
			}

			// no weightings given, so just pick one.
			if (total == 0)
			{
				const char* sTarg = STRING(m_iTargetName[RANDOM_LONG(0, m_cTargets - 1)]);
				if (pev->spawnflags & SF_MULTIMAN_DEBUG)
					ALERT(at_debug, "DEBUG: multi_manager \"%s\": firing \"%s\" (random choice).\n", STRING(pev->targetname), sTarg);
				FireTargets(sTarg, m_hActivator, this, m_triggerType, 0);
			}
			else // pick one by weighting
			{
				float chosen = RANDOM_FLOAT(0, total);
				float curpos = 0;
				for (i = 0; i < m_cTargets; i++)
				{
					curpos += m_flTargetDelay[i];
					if (curpos >= chosen)
					{
						if (pev->spawnflags & SF_MULTIMAN_DEBUG)
							ALERT(at_debug, "DEBUG: multi_manager \"%s\": firing \"%s\" (weighted random choice).\n", STRING(pev->targetname), STRING(m_iTargetName[i]));
						FireTargets(STRING(m_iTargetName[i]), m_hActivator, this, m_triggerType, 0);
						break;
					}
				}
			}
		}
		else if (m_iMode == MM_MODE_PERCENT) // try to call each member
		{
			for (i = 0; i < m_cTargets; i++)
			{
				if (RANDOM_LONG(0, 100) <= m_flTargetDelay[i])
				{
					if (pev->spawnflags & SF_MULTIMAN_DEBUG)
						ALERT(at_debug, "DEBUG: multi_manager \"%s\": firing \"%s\" (%f%% chance).\n", STRING(pev->targetname), STRING(m_iTargetName[i]), m_flTargetDelay[i]);
					FireTargets(STRING(m_iTargetName[i]), m_hActivator, this, m_triggerType, 0);
				}
			}
		}
		else if (m_iMode == MM_MODE_SIMULTANEOUS)
		{
			for (i = 0; i < m_cTargets; i++)
			{
				if (pev->spawnflags & SF_MULTIMAN_DEBUG)
					ALERT(at_debug, "DEBUG: multi_manager \"%s\": firing \"%s\" (simultaneous).\n", STRING(pev->targetname), STRING(m_iTargetName[i]));
				FireTargets(STRING(m_iTargetName[i]), m_hActivator, this, m_triggerType, 0);
			}
		}

		return;
	}

	// ok, so m_iMode is 0; we're doing normal time-based stuff.

	float time;
	int finalidx;
	int index = m_index; // store the current index

	time = gpGlobals->time - m_startTime;

	//	ALERT(at_console,"Manager think for time %f\n",time);

	// find the last index we're going to fire this time
	finalidx = m_index;
	while (finalidx < m_cTargets && m_flTargetDelay[finalidx] <= time)
		finalidx++;

	if (finalidx >= m_cTargets) // will we finish firing targets this time?
	{
		if (pev->spawnflags & SF_MULTIMAN_LOOP)
		{
			//			ALERT(at_console,"Manager loops back\n");
			// if it's a loop, start again!
			m_index = 0;
			if (m_flMaxWait) // LRC- random time to wait?
			{
				m_startTime = RANDOM_FLOAT(m_flWait, m_flMaxWait);
				m_iState = STATE_TURN_ON; // while we're waiting, we're in state TURN_ON
			}
			else if (m_flWait) // LRC- constant time to wait?
			{
				m_startTime = m_flWait;
				m_iState = STATE_TURN_ON;
			}
			else // LRC- just start immediately.
			{
				m_startTime = 0;
				m_iState = STATE_ON;
			}
			if (pev->spawnflags & SF_MULTIMAN_DEBUG)
				ALERT(at_debug, "DEBUG: multi_manager \"%s\": restarting loop.\n", STRING(pev->targetname));
			SetNextThink(m_startTime);
			m_startTime += gpGlobals->time;
		}
		else
		{
			m_iState = STATE_OFF; // LRC- STATE_OFF means "yes, we've finished".
			if (IsClone() || pev->spawnflags & SF_MULTIMAN_ONLYONCE)
			{
				SetThink(&CMultiManager::SUB_Remove);
				SetNextThink(0.1);
				SetUse(NULL);
				if (pev->spawnflags & SF_MULTIMAN_DEBUG)
					ALERT(at_debug, "DEBUG: multi_manager \"%s\": killed.\n", STRING(pev->targetname));
			}
			else
			{
				SetThink(NULL);
				SetUse(&CMultiManager::ManagerUse); // allow manager re-use
				if (pev->spawnflags & SF_MULTIMAN_DEBUG)
					ALERT(at_debug, "DEBUG: multi_manager \"%s\": last burst.\n", STRING(pev->targetname));
			}
		}
	}
	else
	{
		m_index = finalidx;
		m_iState = STATE_ON; // LRC- while we're in STATE_ON we're firing targets, and haven't finished yet.
		AbsoluteNextThink(m_startTime + m_flTargetDelay[m_index]);
	}

	while (index < m_cTargets && m_flTargetDelay[index] <= time)
	{
		//		ALERT(at_console,"Manager sends %d to %s\n",m_triggerType,STRING(m_iTargetName[m_index]));
		if (pev->spawnflags & SF_MULTIMAN_DEBUG)
			ALERT(at_debug, "DEBUG: multi_manager \"%s\": firing \"%s\".\n", STRING(pev->targetname), STRING(m_iTargetName[index]));
		FireTargets(STRING(m_iTargetName[index]), m_hActivator, this, m_triggerType, 0);
		index++;
	}
}

CMultiManager* CMultiManager::Clone()
{
	CMultiManager* pMulti = GetClassPtr((CMultiManager*)NULL);

	edict_t* pEdict = pMulti->pev->pContainingEntity;
	memcpy(pMulti->pev, pev, sizeof(*pev));
	pMulti->pev->pContainingEntity = pEdict;

	pMulti->pev->spawnflags |= SF_MULTIMAN_CLONE;
	pMulti->m_cTargets = m_cTargets;
	if (m_iszThreadName)
		pMulti->pev->targetname = m_iszThreadName; // LRC
	pMulti->m_triggerType = m_triggerType;		   // LRC
	pMulti->m_iMode = m_iMode;					   // LRC
	pMulti->m_flWait = m_flWait;				   // LRC
	pMulti->m_flMaxWait = m_flMaxWait;			   // LRC
	memcpy(pMulti->m_iTargetName, m_iTargetName, sizeof(m_iTargetName));
	memcpy(pMulti->m_flTargetDelay, m_flTargetDelay, sizeof(m_flTargetDelay));

	return pMulti;
}

// The USE function builds the time table and starts the entity thinking.
void CMultiManager::ManagerUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (pev->spawnflags & SF_MULTIMAN_LOOP)
	{
		if (m_iState != STATE_OFF) // if we're on, or turning on...
		{
			if (useType != USE_ON) // ...then turn it off if we're asked to.
			{
				if (pev->spawnflags & SF_MULTIMAN_DEBUG)
					ALERT(at_debug, "DEBUG: multi_manager \"%s\": Loop halted on request.\n", STRING(pev->targetname));
				m_iState = STATE_OFF;
				if (IsClone() || pev->spawnflags & SF_MULTIMAN_ONLYONCE)
				{
					SetThink(&CMultiManager::SUB_Remove);
					SetNextThink(0.1);
					SetUse(NULL);
					if (pev->spawnflags & SF_MULTIMAN_DEBUG)
						ALERT(at_debug, "DEBUG: multi_manager \"%s\": loop halted (removing).\n", STRING(pev->targetname));
				}
				else
				{
					SetThink(NULL);
					if (pev->spawnflags & SF_MULTIMAN_DEBUG)
						ALERT(at_debug, "DEBUG: multi_manager \"%s\": loop halted.\n", STRING(pev->targetname));
				}
			}
			// else we're already on and being told to turn on, so do nothing.
			else if (pev->spawnflags & SF_MULTIMAN_DEBUG)
				ALERT(at_debug, "DEBUG: multi_manager \"%s\": Loop already active.\n", STRING(pev->targetname));
			return;
		}
		else if (useType == USE_OFF) // it's already off
		{
			if (pev->spawnflags & SF_MULTIMAN_DEBUG)
				ALERT(at_debug, "DEBUG: multi_manager \"%s\": Loop already inactive.\n", STRING(pev->targetname));
			return;
		}
		// otherwise, start firing targets as normal.
	}
	//	ALERT(at_console,"Manager used, targetting [");
	//	for (int i = 0; i < m_cTargets; i++)
	//	{
	//		ALERT(at_console," %s(%f)",STRING(m_iTargetName[i]),m_flTargetDelay[i]);
	//	}
	//	ALERT(at_console," ]\n");

	// LRC- "master" support
	if (!UTIL_IsMasterTriggered(m_sMaster, pActivator))
	{
		if (pev->spawnflags & SF_MULTIMAN_DEBUG)
			ALERT(at_debug, "DEBUG: multi_manager \"%s\": Can't trigger, locked by master \"%s\".\n", STRING(pev->targetname), STRING(m_sMaster));
		return;
	}

	// In multiplayer games, clone the MM and execute in the clone (like a thread)
	// to allow multiple players to trigger the same multimanager
	if (ShouldClone())
	{
		CMultiManager* pClone = Clone();
		if (pev->spawnflags & SF_MULTIMAN_DEBUG)
			ALERT(at_debug, "DEBUG: multi_manager \"%s\": Creating clone.\n", STRING(pev->targetname));
		pClone->ManagerUse(pActivator, pCaller, useType, value);
		if (m_iszLocusThread)
			FireTargets(STRING(m_iszLocusThread), pClone, this, USE_TOGGLE, 0);
		return;
	}

	m_hActivator = pActivator;
	m_index = 0;
	float timeOffset;

	if (m_flMaxWait) // LRC- random time to wait?
	{
		timeOffset = RANDOM_FLOAT(m_flWait, m_flMaxWait);
		m_iState = STATE_TURN_ON; // while we're waiting, we're in state TURN_ON
	}
	else if (m_flWait) // LRC- constant time to wait?
	{
		timeOffset = m_flWait;
		m_iState = STATE_TURN_ON;
	}
	else // LRC- just start immediately.
	{
		timeOffset = 0;
		m_iState = STATE_ON;
	}

	m_startTime = timeOffset + gpGlobals->time;

	// startTime should not be affected by this next bit
	if (m_cTargets > 0 && !m_iMode && m_flTargetDelay[0] < 0)
	{
		// negative wait on the first target?
		timeOffset += m_flTargetDelay[0];
	}

	if (pev->spawnflags & SF_MULTIMAN_SAMETRIG) // LRC
		m_triggerType = useType;

	if (pev->spawnflags & SF_MULTIMAN_LOOP)
		SetUse(&CMultiManager::ManagerUse); // clones won't already have this set
	else
		SetUse(NULL); // disable use until all targets have fired

	if (timeOffset > 0)
	{
		if (pev->spawnflags & SF_MULTIMAN_DEBUG)
			ALERT(at_debug, "DEBUG: multi_manager \"%s\": Begin in %f seconds.\n", STRING(pev->targetname), timeOffset);
		SetThink(&CMultiManager::ManagerThink);
		SetNextThink(timeOffset);
	}
	else
	{
		SetThink(&CMultiManager::ManagerThink);
		ManagerThink();
	}
}

#if _DEBUG
void CMultiManager::ManagerReport()
{
	int cIndex;

	for (cIndex = 0; cIndex < m_cTargets; cIndex++)
	{
		ALERT(at_debug, "%s %f\n", STRING(m_iTargetName[cIndex]), m_flTargetDelay[cIndex]);
	}
}
#endif
