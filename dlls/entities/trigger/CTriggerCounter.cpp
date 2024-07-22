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

#include "CTriggerCounter.h"

LINK_ENTITY_TO_CLASS(trigger_counter, CTriggerCounter);

bool CTriggerCounter::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "count"))
	{
		m_cTriggersLeft = (int)atof(pkvd->szValue);
		return true;
	}
	return CTriggerMultiple::KeyValue(pkvd);
}

void CTriggerCounter::Spawn()
{
	// By making the flWait be -1, this counter-trigger will disappear after it's activated
	// (but of course it needs cTriggersLeft "uses" before that happens).
	m_flWait = -1;

	if (m_cTriggersLeft == 0)
		m_cTriggersLeft = 2;
	SetUse(&CTriggerCounter::CounterUse);
}

void CTriggerCounter::CounterUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	m_cTriggersLeft--;
	m_hActivator = pActivator;

	if (m_cTriggersLeft < 0)
		return;

	bool fTellActivator =
		(m_hActivator != 0) &&
		FClassnameIs(m_hActivator->pev, "player") &&
		!FBitSet(pev->spawnflags, SPAWNFLAG_NOMESSAGE);
	if (m_cTriggersLeft != 0)
	{
		if (fTellActivator)
		{
			// UNDONE: I don't think we want these Quakesque messages
			switch (m_cTriggersLeft)
			{
			case 1:
				ALERT(at_debug, "Only 1 more to go...");
				break;
			case 2:
				ALERT(at_debug, "Only 2 more to go...");
				break;
			case 3:
				ALERT(at_debug, "Only 3 more to go...");
				break;
			default:
				ALERT(at_debug, "There are more to go...");
				break;
			}
		}
		return;
	}

	// !!!UNDONE: I don't think we want these Quakesque messages
	if (fTellActivator)
		ALERT(at_debug, "Sequence completed!");

	ActivateMultiTrigger(m_hActivator);
}

bool CTriggerCounter::CalcNumber(CBaseEntity* pLocus, float* OUTresult)
{
	*OUTresult = float(m_cTriggersLeft);
	return true;
}
