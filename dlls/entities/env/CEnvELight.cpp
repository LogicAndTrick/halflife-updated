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

#include "CEnvELight.h"


LINK_ENTITY_TO_CLASS(env_elight, CEnvELight);

int CEnvELight::ms_iNextFreeKey = 1;

TYPEDESCRIPTION CEnvELight::m_SaveData[] =
	{
		DEFINE_FIELD(CEnvELight, m_hAttach, FIELD_EHANDLE),
		DEFINE_FIELD(CEnvELight, m_iKey, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CEnvELight, CEnvDLight);

void CEnvELight::PostSpawn()
{
	// each env_elight uses its own key to reference the light on the client
	m_iKey = ms_iNextFreeKey;
	ms_iNextFreeKey++;
}

void CEnvELight::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (pev->target)
	{
		m_hAttach = UTIL_FindEntityByTargetname(NULL, STRING(pev->target), pActivator);
		if (m_hAttach == NULL)
		{
			ALERT(at_console, "env_elight \"%s\" can't find target %s\n", STRING(pev->targetname), STRING(pev->target));
			return; // error?
		}
	}
	else
	{
		m_hAttach = this;
	}

	CEnvDLight::Use(pActivator, pCaller, useType, value);
}

void CEnvELight::MakeLight(bool bActive)
{
	if (m_hAttach == NULL)
	{
		DontThink();
		pev->takedamage = 0;
		return;
	}

	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
	WRITE_BYTE(TE_ELIGHT);
	WRITE_SHORT(m_hAttach->entindex() + 0x1000 * pev->impulse); // entity, attachment
	WRITE_COORD(m_vecPos.x);									// X
	WRITE_COORD(m_vecPos.y);									// Y
	WRITE_COORD(m_vecPos.z);									// Z
	WRITE_COORD(pev->renderamt);								// radius * 0.1
	WRITE_BYTE(pev->rendercolor.x);								// r
	WRITE_BYTE(pev->rendercolor.y);								// g
	WRITE_BYTE(pev->rendercolor.z);								// b
	// WRITE_BYTE( iTime );				// time * 10
	WRITE_BYTE(pev->health); // time * 10
	WRITE_COORD(pev->frags); // decay * 0.1
	MESSAGE_END();
}
