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

#include "CEnvDLight.h"
#include "util/locus.h"
#include "util/movewith.h"
#include "UserMessages.h"

LINK_ENTITY_TO_CLASS(env_dlight, CEnvDLight);

TYPEDESCRIPTION CEnvDLight::m_SaveData[] =
	{
		DEFINE_FIELD(CEnvDLight, m_vecPos, FIELD_VECTOR),
		DEFINE_FIELD(CEnvDLight, m_iKey, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CEnvDLight, CPointEntity);

int CEnvDLight::ms_iNextFreeKey = 1;

void CEnvDLight::PostSpawn()
{
	// each env_dlight uses its own key to reference the light on the client
	m_iKey = ms_iNextFreeKey;
	ms_iNextFreeKey++;

	if (FStringNull(pev->targetname) || pev->spawnflags & SF_DLIGHT_STARTON)
	{
		UTIL_DesiredAction(this);
	}
}

void CEnvDLight::DesiredAction()
{
	pev->spawnflags &= ~SF_DLIGHT_STARTON;
	Use(this, this, USE_ON, 0);
}

void CEnvDLight::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (!ShouldToggle(useType))
	{
		return;
	}
	if (GetState() == STATE_ON)
	{
		// turn off
		MakeLight(false);
		pev->spawnflags &= ~SF_DLIGHT_STARTON;
		DontThink();
		return;
	}

	if (pev->message)
	{
		m_vecPos = CalcLocus_Position(this, pActivator, STRING(pev->message));
	}
	else
	{
		m_vecPos = pev->origin;
	}

	// turn on
	MakeLight(true);
	pev->spawnflags |= SF_DLIGHT_STARTON;

	if (pev->health)
	{
		SetNextThink(pev->health);
	}
	else
	{
		if (pev->spawnflags & SF_DLIGHT_ONLYONCE)
		{
			SetThink(&CEnvDLight::SUB_Remove);
			SetNextThink(0);
		}
	}
}

void CEnvDLight::MakeLight(bool bActive)
{
	//	MESSAGE_BEGIN( MSG_ALL, gmsgKeyedDLight, NULL );
	MESSAGE_BEGIN(MSG_ALL, gmsgKeyedELight, NULL);
	WRITE_BYTE(m_iKey);
	WRITE_BYTE(bActive); // visible?
	if (bActive)
	{
		WRITE_COORD(m_vecPos.x);		// X
		WRITE_COORD(m_vecPos.y);		// Y
		WRITE_COORD(m_vecPos.z);		// Z
		WRITE_BYTE(pev->renderamt);		// radius * 0.1
		WRITE_BYTE(pev->rendercolor.x); // r
		WRITE_BYTE(pev->rendercolor.y); // g
		WRITE_BYTE(pev->rendercolor.z); // b
	}
	MESSAGE_END();
}

void CEnvDLight::Think()
{
	// turn off the light
	MakeLight(false);
	pev->spawnflags &= ~SF_DLIGHT_STARTON;

	if (pev->spawnflags & SF_DLIGHT_ONLYONCE)
	{
		SetThink(&CEnvDLight::SUB_Remove);
		SetNextThink(0);
	}
}
