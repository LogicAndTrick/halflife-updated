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

#include "CStateWatcher.h"

LINK_ENTITY_TO_CLASS(multi_watcher, CStateWatcher);
LINK_ENTITY_TO_CLASS(watcher, CStateWatcher);

TYPEDESCRIPTION CStateWatcher::m_SaveData[] =
	{
		DEFINE_FIELD(CStateWatcher, m_fLogic, FIELD_INTEGER),
		DEFINE_FIELD(CStateWatcher, m_cTargets, FIELD_INTEGER),
		DEFINE_ARRAY(CStateWatcher, m_iTargetName, FIELD_STRING, MAX_MULTI_TARGETS),
		//	DEFINE_ARRAY( CStateWatcher, m_pTargetEnt, FIELD_CLASSPTR, MAX_MULTI_TARGETS ),
};

IMPLEMENT_SAVERESTORE(CStateWatcher, CBaseToggle);

bool CStateWatcher::KeyValue(KeyValueData* pkvd)
{
	char tmp[128];
	if (FStrEq(pkvd->szKeyName, "m_fLogic"))
	{
		m_fLogic = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszWatch"))
	{
		if (m_cTargets < MAX_MULTI_TARGETS)
		{
			m_iTargetName[m_cTargets] = ALLOC_STRING(pkvd->szValue);
			m_cTargets++;
			return true;
		}
		else
		{
			ALERT(at_debug, "%s: Too many targets for %s \"%s\" (limit is %d)\n", pkvd->szKeyName, STRING(pev->classname), STRING(pev->targetname), MAX_MULTI_TARGETS);
			return false;
		}
	}
	else // add this field to the target list
	{
		// this assumes that additional fields are targetnames and their values are delay values.
		if (m_cTargets < MAX_MULTI_TARGETS)
		{
			UTIL_StripToken(pkvd->szKeyName, tmp);
			m_iTargetName[m_cTargets] = ALLOC_STRING(tmp);
			m_cTargets++;
			return true;
		}
		else
		{
			ALERT(at_debug, "WARNING - %s: Too many targets for %s \"%s\" (limit is %d)\n", pkvd->szKeyName, STRING(pev->classname), STRING(pev->targetname), MAX_MULTI_TARGETS);
			return false;
		}
	}
}

void CStateWatcher::Spawn()
{
	pev->solid = SOLID_NOT;
	if (pev->target)
		SetNextThink(0.5);
}

STATE CStateWatcher::GetState()
{
	if (EvalLogic(NULL))
		return STATE_ON;
	else
		return STATE_OFF;
}

STATE CStateWatcher::GetState(CBaseEntity* pActivator)
{
	//	if (pActivator)
	//		ALERT(at_console, "GetState( %s \"%s\" )\n", STRING(pActivator->pev->classname), STRING(pActivator->pev->targetname));
	//	else
	//		ALERT(at_console, "GetState( NULL )\n");
	if (EvalLogic(pActivator))
		return STATE_ON;
	else
		return STATE_OFF;
}

void CStateWatcher::Think()
{
	SetNextThink(0.1);
	int oldflag = pev->spawnflags & SF_SWATCHER_VALID;

	if (EvalLogic(NULL))
		pev->spawnflags |= SF_SWATCHER_VALID;
	else
		pev->spawnflags &= ~SF_SWATCHER_VALID;

	if ((pev->spawnflags & SF_SWATCHER_VALID) != oldflag)
	{
		// the update changed my state...

		if (oldflag)
		{
			// ...to off. Send "off".
			//			ALERT(at_console,"%s turns off\n",STRING(pev->classname));
			if (!FBitSet(pev->spawnflags, SF_SWATCHER_DONTSEND_OFF))
			{
				if (pev->spawnflags & SF_SWATCHER_SENDTOGGLE)
					SUB_UseTargets(this, USE_TOGGLE, 0);
				else
					SUB_UseTargets(this, USE_OFF, 0);
			}
		}
		else
		{
			// ...to on. Send "on".
			//			ALERT(at_console,"%s turns on\n",STRING(pev->classname));
			if (!FBitSet(pev->spawnflags, SF_SWATCHER_DONTSEND_ON))
			{
				if (pev->spawnflags & SF_SWATCHER_SENDTOGGLE)
					SUB_UseTargets(this, USE_TOGGLE, 0);
				else
					SUB_UseTargets(this, USE_ON, 0);
			}
		}
	}
}

bool CStateWatcher::EvalLogic(CBaseEntity* pActivator)
{
	int i;
	bool b;
	bool xorgot = false;

	CBaseEntity* pEntity;

	for (i = 0; i < m_cTargets; i++)
	{
		//		if (m_pTargetEnt[i] == NULL)
		//		{
		//			pEntity = m_pTargetEnt[i];
		//		}
		//		else
		//		{
		pEntity = UTIL_FindEntityByTargetname(NULL, STRING(m_iTargetName[i]), pActivator);
		if (pEntity != NULL)
		{
			//				if ((STRING(m_iTargetName[i]))[0] != '*') // don't cache alias values
			//				{
			//					//ALERT(at_console,"Watcher: entity %s cached\n",STRING(m_iTargetName[i]));
			//					m_pTargetEnt[i] = pEntity;
			//				}
			// else
			// ALERT(at_console,"Watcher: aliased entity %s not cached\n",STRING(m_iTargetName[i]));
		}
		else
		{
			// ALERT(at_console,"Watcher: missing entity %s\n",STRING(m_iTargetName[i]));
			continue; // couldn't find this entity; don't do the test.
		}
		//		}

		b = false;
		switch (pEntity->GetState())
		{
		case STATE_ON:
			if (!(pev->spawnflags & SF_SWATCHER_NOTON))
				b = true;
			break;
		case STATE_OFF:
			if (pev->spawnflags & SF_SWATCHER_OFF)
				b = true;
			break;
		case STATE_TURN_ON:
			if (pev->spawnflags & SF_SWATCHER_TURN_ON)
				b = true;
			break;
		case STATE_TURN_OFF:
			if (pev->spawnflags & SF_SWATCHER_TURN_ON)
				b = true;
			break;
		case STATE_IN_USE:
			if (pev->spawnflags & SF_SWATCHER_IN_USE)
				b = true;
			break;
		}
		// handle the states for this logic mode
		if (b)
		{
			switch (m_fLogic)
			{
			case SWATCHER_LOGIC_OR:
				//				ALERT(at_console,"b is true, OR returns true\n");
				return true;
			case SWATCHER_LOGIC_NOR:
				//				ALERT(at_console,"b is true, NOR returns false\n");
				return false;
			case SWATCHER_LOGIC_XOR:
				//				ALERT(at_console,"b is true, XOR\n");
				if (xorgot)
					return false;
				xorgot = true;
				break;
			case SWATCHER_LOGIC_XNOR:
				//				ALERT(at_console,"b is true, XNOR\n");
				if (xorgot)
					return true;
				xorgot = true;
				break;
			}
		}
		else // b is false
		{
			switch (m_fLogic)
			{
			case SWATCHER_LOGIC_AND:
				//				ALERT(at_console,"b is false, AND returns false\n");
				return false;
			case SWATCHER_LOGIC_NAND:
				//				ALERT(at_console,"b is false, NAND returns true\n");
				return true;
			}
		}
	}
	// handle the default cases for each logic mode
	switch (m_fLogic)
	{
	case SWATCHER_LOGIC_AND:
	case SWATCHER_LOGIC_NOR:
		//		ALERT(at_console,"final, AND/NOR returns true\n");
		return true;
	case SWATCHER_LOGIC_XOR:
		//		ALERT(at_console,"final, XOR\n");
		return xorgot;
	case SWATCHER_LOGIC_XNOR:
		//		ALERT(at_console,"final, XNOR\n");
		return !xorgot;
	default: // NAND, OR
			 //		ALERT(at_console,"final, NAND/OR returns false\n");
		return false;
	}
}
