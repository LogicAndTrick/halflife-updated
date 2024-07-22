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

#include "CAutoTrigger.h"
#include "movewith.h"

LINK_ENTITY_TO_CLASS(trigger_auto, CAutoTrigger);

TYPEDESCRIPTION CAutoTrigger::m_SaveData[] =
	{
		DEFINE_FIELD(CAutoTrigger, m_globalstate, FIELD_STRING),
		DEFINE_FIELD(CAutoTrigger, triggerType, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CAutoTrigger, CBaseDelay);

bool CAutoTrigger::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "globalstate"))
	{
		m_globalstate = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "triggerstate"))
	{
		int type = atoi(pkvd->szValue);
		switch (type)
		{
		case 0:
			triggerType = USE_OFF;
			break;
		case 2:
			triggerType = USE_TOGGLE;
			break;
		default:
			triggerType = USE_ON;
			break;
		}
		return true;
	}

	return CBaseDelay::KeyValue(pkvd);
}

void CAutoTrigger::Activate()
{
	//	ALERT(at_console, "trigger_auto targetting \"%s\": activate\n", STRING(pev->target));
	UTIL_DesiredAction(this); // LRC - don't think until the player has spawned.

	CBaseDelay::Activate();
}

void CAutoTrigger::DesiredAction()
{
	//	ALERT(at_console, "trigger_auto targetting \"%s\": Fire at time %f\n", STRING(pev->target), gpGlobals->time);
	if (!m_globalstate || gGlobalState.EntityGetState(m_globalstate) == GLOBAL_ON)
	{
		if (pev->spawnflags & SF_AUTO_FROMPLAYER)
		{
			CBaseEntity* pPlayer = UTIL_FindEntityByClassname(NULL, "player");
			if (pPlayer)
				SUB_UseTargets(pPlayer, triggerType, 0);
			else
				ALERT(at_error, "trigger_auto: \"From Player\" is ticked, but no player found!\n");
		}
		else
		{
			SUB_UseTargets(this, triggerType, 0);
		}
		if (pev->spawnflags & SF_AUTO_FIREONCE)
			UTIL_Remove(this);
	}
}
