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

#include "CTriggerChangeCVar.h"

LINK_ENTITY_TO_CLASS(trigger_changecvar, CTriggerChangeCVar);

TYPEDESCRIPTION CTriggerChangeCVar::m_SaveData[] =
	{
		DEFINE_ARRAY(CTriggerChangeCVar, m_szStoredString, FIELD_CHARACTER, 256),
};

IMPLEMENT_SAVERESTORE(CTriggerChangeCVar, CBaseEntity);

void CTriggerChangeCVar::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	char szCommand[256];

	if (!(pev->netname))
		return;

	if (ShouldToggle(useType, pev->spawnflags & SF_CVAR_ACTIVE))
	{
		if (pev->spawnflags & SF_CVAR_ACTIVE)
		{
			sprintf(szCommand, "%s \"%s\"\n", STRING(pev->netname), m_szStoredString);
			pev->spawnflags &= ~SF_CVAR_ACTIVE;
		}
		else
		{
			strncpy(m_szStoredString, CVAR_GET_STRING(STRING(pev->netname)), 256);
			sprintf(szCommand, "%s \"%s\"\n", STRING(pev->netname), STRING(pev->message));
			pev->spawnflags |= SF_CVAR_ACTIVE;

			if (pev->armorvalue >= 0)
			{
				SetNextThink(pev->armorvalue);
			}
		}
		SERVER_COMMAND(szCommand);
	}
}

void CTriggerChangeCVar::Think()
{
	char szCommand[256];

	if (pev->spawnflags & SF_CVAR_ACTIVE)
	{
		sprintf(szCommand, "%s %s\n", STRING(pev->netname), m_szStoredString);
		SERVER_COMMAND(szCommand);
		pev->spawnflags &= ~SF_CVAR_ACTIVE;
	}
}
