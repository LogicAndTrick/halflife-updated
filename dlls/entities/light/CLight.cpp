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

#include "CLight.h"
#include "util/light.h"

LINK_ENTITY_TO_CLASS(light, CLight);
LINK_ENTITY_TO_CLASS(light_spot, CLight);

TYPEDESCRIPTION CLight::m_SaveData[] =
	{
		DEFINE_FIELD(CLight, m_iState, FIELD_INTEGER),
		DEFINE_FIELD(CLight, m_iszPattern, FIELD_STRING),
		DEFINE_FIELD(CLight, m_iszCurrentStyle, FIELD_STRING),
		DEFINE_FIELD(CLight, m_iOnStyle, FIELD_INTEGER),
		DEFINE_FIELD(CLight, m_iOffStyle, FIELD_INTEGER),
		DEFINE_FIELD(CLight, m_iTurnOnStyle, FIELD_INTEGER),
		DEFINE_FIELD(CLight, m_iTurnOffStyle, FIELD_INTEGER),
		DEFINE_FIELD(CLight, m_iTurnOnTime, FIELD_INTEGER),
		DEFINE_FIELD(CLight, m_iTurnOffTime, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CLight, CPointEntity);

//
// Cache user-entity-field values until spawn is called.
//
bool CLight::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "m_iOnStyle"))
	{
		m_iOnStyle = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iOffStyle"))
	{
		m_iOffStyle = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iTurnOnStyle"))
	{
		m_iTurnOnStyle = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iTurnOffStyle"))
	{
		m_iTurnOffStyle = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iTurnOnTime"))
	{
		m_iTurnOnTime = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iTurnOffTime"))
	{
		m_iTurnOffTime = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "pitch"))
	{
		pev->angles.x = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "pattern"))
	{
		m_iszPattern = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "firetarget"))
	{
		pev->target = ALLOC_STRING(pkvd->szValue);
		return true;
	}

	return CPointEntity::KeyValue(pkvd);
}

void CLight::SetStyle(int iszPattern)
{
	if (m_iStyle < 32) // if it's using a global style, don't change it
		return;
	m_iszCurrentStyle = iszPattern;
	//	ALERT(at_console, "SetStyle %d \"%s\"\n", m_iStyle, (char *)STRING( iszPattern ));
	LIGHT_STYLE(m_iStyle, (char*)STRING(iszPattern));
}

// regardless of what's been set by trigger_lightstyle ents, set the style I think I need
void CLight::SetCorrectStyle()
{
	if (m_iStyle >= 32)
	{
		switch (m_iState)
		{
		case STATE_ON:
			if (m_iszPattern) // custom styles have priority over standard ones
				SetStyle(m_iszPattern);
			else if (m_iOnStyle)
				SetStyle(GetStdLightStyle(m_iOnStyle));
			else
				SetStyle(MAKE_STRING("m"));
			break;
		case STATE_OFF:
			if (m_iOffStyle)
				SetStyle(GetStdLightStyle(m_iOffStyle));
			else
				SetStyle(MAKE_STRING("a"));
			break;
		case STATE_TURN_ON:
			if (m_iTurnOnStyle)
				SetStyle(GetStdLightStyle(m_iTurnOnStyle));
			else
				SetStyle(MAKE_STRING("a"));
			break;
		case STATE_TURN_OFF:
			if (m_iTurnOffStyle)
				SetStyle(GetStdLightStyle(m_iTurnOffStyle));
			else
				SetStyle(MAKE_STRING("m"));
			break;
		}
	}
	else
	{
		m_iszCurrentStyle = GetStdLightStyle(m_iStyle);
	}
}

void CLight::Think()
{
	switch (GetState())
	{
	case STATE_TURN_ON:
		m_iState = STATE_ON;
		FireTargets(STRING(pev->target), this, this, USE_ON, 0);
		break;
	case STATE_TURN_OFF:
		m_iState = STATE_OFF;
		FireTargets(STRING(pev->target), this, this, USE_OFF, 0);
		break;
	}
	SetCorrectStyle();
}

/*QUAKED light (0 1 0) (-8 -8 -8) (8 8 8) LIGHT_START_OFF
Non-displayed light.
Default light value is 300
Default style is 0
If targeted, it will toggle between on or off.
*/

void CLight::Spawn()
{
	if (FStringNull(pev->targetname))
	{ // inert light
		REMOVE_ENTITY(ENT(pev));
		return;
	}

	if (FBitSet(pev->spawnflags, SF_LIGHT_START_OFF))
		m_iState = STATE_OFF;
	else
		m_iState = STATE_ON;
	SetCorrectStyle();
}

void CLight::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (m_iStyle >= 32)
	{
		if (!ShouldToggle(useType))
			return;

		switch (GetState())
		{
		case STATE_ON:
		case STATE_TURN_ON:
			if (m_iTurnOffTime)
			{
				m_iState = STATE_TURN_OFF;
				SetNextThink(m_iTurnOffTime);
			}
			else
				m_iState = STATE_OFF;
			break;
		case STATE_OFF:
		case STATE_TURN_OFF:
			if (m_iTurnOnTime)
			{
				m_iState = STATE_TURN_ON;
				SetNextThink(m_iTurnOnTime);
			}
			else
				m_iState = STATE_ON;
			break;
		}
		SetCorrectStyle();
	}
}
