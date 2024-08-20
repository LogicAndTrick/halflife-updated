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

#include "CTriggerRelay.h"
#include "util/locus.h"
#include "entities/player/CBasePlayer.h"

LINK_ENTITY_TO_CLASS(trigger_relay, CTriggerRelay);

TYPEDESCRIPTION CTriggerRelay::m_SaveData[] =
	{
		DEFINE_FIELD(CTriggerRelay, m_triggerType, FIELD_INTEGER),
		DEFINE_FIELD(CTriggerRelay, m_sMaster, FIELD_STRING),
		DEFINE_FIELD(CTriggerRelay, m_iszAltTarget, FIELD_STRING), // G-Cont. in garagedoordemo code without this stuff, demo don't work with save\load ;)
};

IMPLEMENT_SAVERESTORE(CTriggerRelay, CBaseDelay);

bool CTriggerRelay::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "master"))
	{
		m_sMaster = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszAltTarget"))
	{
		m_iszAltTarget = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "triggerstate"))
	{
		int type = atoi(pkvd->szValue);
		switch (type)
		{
		case 0:
			m_triggerType = USE_ON;
			break;
		case 2:
			m_triggerType = USE_TOGGLE;
			break;
		case 4:
			m_triggerType = USE_KILL;
			break;
		case 5:
			m_triggerType = USE_SAME;
			break;
		case 7:
			m_triggerType = USE_SET;
			break;
		default:
			m_triggerType = USE_OFF;
			break;
		}
		return true;
	}

	return CBaseDelay::KeyValue(pkvd);
}

void CTriggerRelay::Spawn()
{
	if (FStringNull(m_triggerType)) // G-Cont. Hmm... In original Half-life, all nice works without this stuff
		m_triggerType = USE_ON;
}

void CTriggerRelay::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (!UTIL_IsMasterTriggered(m_sMaster, pActivator))
	{
		if (m_iszAltTarget)
		{
			// FIXME: the alternate target should really use m_flDelay.
			// if (pev->spawnflags & SF_RELAY_USESAME)
			if (m_triggerType == USE_SAME)
				FireTargets(STRING(m_iszAltTarget), pActivator, this, useType, 0);
			else
				FireTargets(STRING(m_iszAltTarget), pActivator, this, m_triggerType, 0);
			if (pev->spawnflags & SF_RELAY_DEBUG)
				ALERT(at_debug, "DEBUG: trigger_relay \"%s\" locked by master %s - fired alternate target %s\n", STRING(pev->targetname), STRING(m_sMaster), STRING(m_iszAltTarget));
			if (pev->spawnflags & SF_RELAY_FIREONCE)
			{
				if (pev->spawnflags & SF_RELAY_DEBUG)
					ALERT(at_debug, "trigger_relay \"%s\" removes itself.\n");
				UTIL_Remove(this);
			}
		}
		else if (pev->spawnflags & SF_RELAY_DEBUG)
			ALERT(at_debug, "DEBUG: trigger_relay \"%s\" wasn't activated: locked by master %s\n", STRING(pev->targetname), STRING(m_sMaster));
		return;
	}
	if (pev->spawnflags & SF_RELAY_DEBUG)
	{
		ALERT(at_debug, "DEBUG: trigger_relay \"%s\" was sent %s", STRING(pev->targetname), GetStringForUseType(useType));
		if (pActivator)
		{
			if (FStringNull(pActivator->pev->targetname))
				ALERT(at_debug, " from \"%s\"", STRING(pActivator->pev->classname));
			else
				ALERT(at_debug, " from \"%s\"", STRING(pActivator->pev->targetname));
		}
		else
			ALERT(at_debug, " (no locus)");
	}

	if (FStringNull(pev->target) && !m_iszKillTarget)
	{
		if (pev->spawnflags & SF_RELAY_DEBUG)
			ALERT(at_debug, ".\n");
		return;
	}

	if (pev->spawnflags & SF_FIRE_CAMERA) // For triggering once if player see in camera
	{
		if (!pActivator || !pActivator->IsPlayer())
		{
			pActivator = CBaseEntity::Instance(g_engfuncs.pfnPEntityOfEntIndex(1));
		}
		if (pActivator && pActivator->IsPlayer())
		{
			if (((CBasePlayer*)pActivator)->viewFlags == 0)
			{
				// 				ALERT(at_console, "player is not curently see in camera\n");
				return;
			}
		}
	}
	if (!pev->skin)
		pev->skin = 0; // AJH set skin (ratio to return) to default

	if (pev->message)
		value = CalcLocus_Number(pActivator, STRING(pev->message) /*LRC 1.8 removed ,pev->skin*/); // AJH skin (ratio to return)

	if (m_triggerType == USE_SAME)
	{
		if (pev->spawnflags & SF_RELAY_DEBUG)
		{
			if (m_flDelay)
				ALERT(at_debug, ": will send %s(same) in %f seconds.\n", GetStringForUseType(useType), m_flDelay);
			else
				ALERT(at_debug, ": sending %s(same) now.\n", GetStringForUseType(useType));
		}
		SUB_UseTargets(pActivator, useType, value);
	}
	else if (m_triggerType == USE_SET)
	{
		if (pev->spawnflags & SF_RELAY_DEBUG)
		{
			if (m_flDelay)
				ALERT(at_debug, ": will send ratio %f in %f seconds.\n", value, m_flDelay);
			else
				ALERT(at_debug, ": sending ratio %f now.\n", value);
		}
		SUB_UseTargets(pActivator, m_triggerType, value);
	}
	else
	{
		if (pev->spawnflags & SF_RELAY_DEBUG)
		{
			if (m_flDelay)
				ALERT(at_debug, ": will send %s in %f seconds.\n", GetStringForUseType(m_triggerType), m_flDelay);
			else
				ALERT(at_debug, ": sending %s now.\n", GetStringForUseType(m_triggerType));
		}
		SUB_UseTargets(pActivator, m_triggerType, 0);
	}
	if (pev->spawnflags & SF_RELAY_FIREONCE)
	{
		if (pev->spawnflags & SF_RELAY_DEBUG)
			ALERT(at_debug, "trigger_relay \"%s\" removes itself.\n");
		UTIL_Remove(this);
	}
}
