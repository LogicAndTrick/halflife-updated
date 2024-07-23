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

#include "CEnvState.h"

void CEnvState::Spawn()
{
	if (pev->spawnflags & SF_ENVSTATE_START_ON)
		m_iState = STATE_ON;
	else
		m_iState = STATE_OFF;
}

TYPEDESCRIPTION CEnvState::m_SaveData[] =
	{
		DEFINE_FIELD(CEnvState, m_iState, FIELD_INTEGER),
		DEFINE_FIELD(CEnvState, m_fTurnOnTime, FIELD_INTEGER),
		DEFINE_FIELD(CEnvState, m_fTurnOffTime, FIELD_INTEGER),
		DEFINE_FIELD(CEnvState, m_sMaster, FIELD_STRING),
};

IMPLEMENT_SAVERESTORE(CEnvState, CPointEntity);

LINK_ENTITY_TO_CLASS(env_state, CEnvState);

bool CEnvState::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "turnontime"))
	{
		m_fTurnOnTime = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "turnofftime"))
	{
		m_fTurnOffTime = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "master"))
	{
		m_sMaster = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	return CPointEntity::KeyValue(pkvd);
}

void CEnvState::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (!ShouldToggle(useType) || IsLockedByMaster())
	{
		if (pev->spawnflags & SF_ENVSTATE_DEBUG)
		{
			ALERT(at_debug, "DEBUG: env_state \"%s\" ", STRING(pev->targetname));
			if (IsLockedByMaster())
				ALERT(at_debug, "ignored trigger %s; locked by master \"%s\".\n", GetStringForUseType(useType), STRING(m_sMaster));
			else if (useType == USE_ON)
				ALERT(at_debug, "ignored trigger USE_ON; already on\n");
			else if (useType == USE_OFF)
				ALERT(at_debug, "ignored trigger USE_OFF; already off\n");
			else
				ALERT(at_debug, "ignored trigger %s.\n", GetStringForUseType(useType));
		}
		return;
	}

	switch (GetState())
	{
	case STATE_ON:
	case STATE_TURN_ON:
		if (m_fTurnOffTime)
		{
			m_iState = STATE_TURN_OFF;
			if (pev->spawnflags & SF_ENVSTATE_DEBUG)
			{
				ALERT(at_debug, "DEBUG: env_state \"%s\" triggered; will turn off in %f seconds.\n", STRING(pev->targetname), m_fTurnOffTime);
			}
			SetNextThink(m_fTurnOffTime);
		}
		else
		{
			m_iState = STATE_OFF;
			if (pev->spawnflags & SF_ENVSTATE_DEBUG)
			{
				ALERT(at_debug, "DEBUG: env_state \"%s\" triggered, turned off", STRING(pev->targetname));
				if (pev->target)
				{
					ALERT(at_debug, ": firing \"%s\"", STRING(pev->target));
					if (pev->noise2)
						ALERT(at_debug, " and \"%s\"", STRING(pev->noise2));
				}
				else if (pev->noise2)
					ALERT(at_debug, ": firing \"%s\"", STRING(pev->noise2));
				ALERT(at_debug, ".\n");
			}
			FireTargets(STRING(pev->target), pActivator, this, USE_OFF, 0);
			FireTargets(STRING(pev->noise2), pActivator, this, USE_TOGGLE, 0);
			DontThink();
		}
		break;
	case STATE_OFF:
	case STATE_TURN_OFF:
		if (m_fTurnOnTime)
		{
			m_iState = STATE_TURN_ON;
			if (pev->spawnflags & SF_ENVSTATE_DEBUG)
			{
				ALERT(at_debug, "DEBUG: env_state \"%s\" triggered; will turn on in %f seconds.\n", STRING(pev->targetname), m_fTurnOnTime);
			}
			SetNextThink(m_fTurnOnTime);
		}
		else
		{
			m_iState = STATE_ON;
			if (pev->spawnflags & SF_ENVSTATE_DEBUG)
			{
				ALERT(at_debug, "DEBUG: env_state \"%s\" triggered, turned on", STRING(pev->targetname));
				if (pev->target)
				{
					ALERT(at_debug, ": firing \"%s\"", STRING(pev->target));
					if (pev->noise1)
						ALERT(at_debug, " and \"%s\"", STRING(pev->noise1));
				}
				else if (pev->noise1)
					ALERT(at_debug, ": firing \"%s\"", STRING(pev->noise1));
				ALERT(at_debug, ".\n");
			}
			FireTargets(STRING(pev->target), pActivator, this, USE_ON, 0);
			FireTargets(STRING(pev->noise1), pActivator, this, USE_TOGGLE, 0);
			DontThink();
		}
		break;
	}
}

void CEnvState::Think()
{
	if (m_iState == STATE_TURN_ON)
	{
		m_iState = STATE_ON;
		if (pev->spawnflags & SF_ENVSTATE_DEBUG)
		{
			ALERT(at_debug, "DEBUG: env_state \"%s\" turned itself on", STRING(pev->targetname));
			if (pev->target)
			{
				ALERT(at_debug, ": firing %s", STRING(pev->target));
				if (pev->noise1)
					ALERT(at_debug, " and %s", STRING(pev->noise1));
			}
			else if (pev->noise1)
				ALERT(at_debug, ": firing %s", STRING(pev->noise1));
			ALERT(at_debug, ".\n");
		}
		FireTargets(STRING(pev->target), this, this, USE_ON, 0);
		FireTargets(STRING(pev->noise1), this, this, USE_TOGGLE, 0);
	}
	else if (m_iState == STATE_TURN_OFF)
	{
		m_iState = STATE_OFF;
		if (pev->spawnflags & SF_ENVSTATE_DEBUG)
		{
			ALERT(at_debug, "DEBUG: env_state \"%s\" turned itself off", STRING(pev->targetname));
			if (pev->target)
				ALERT(at_debug, ": firing %s", STRING(pev->target));
			if (pev->noise2)
				ALERT(at_debug, " and %s", STRING(pev->noise2));
			else if (pev->noise2)
				ALERT(at_debug, ": firing %s", STRING(pev->noise2));
			ALERT(at_debug, ".\n");
		}
		FireTargets(STRING(pev->target), this, this, USE_OFF, 0);
		FireTargets(STRING(pev->noise2), this, this, USE_TOGGLE, 0);
	}
}
