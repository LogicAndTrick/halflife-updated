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

#include "CMessage.h"

#include "util/message.h"

LINK_ENTITY_TO_CLASS(env_message, CMessage);

void CMessage::Spawn()
{
	Precache();

	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;

	switch (pev->impulse)
	{
	case 1: // Medium radius
		pev->speed = ATTN_STATIC;
		break;

	case 2: // Large radius
		pev->speed = ATTN_NORM;
		break;

	case 3: // EVERYWHERE
		pev->speed = ATTN_NONE;
		break;

	default:
	case 0: // Small radius
		pev->speed = ATTN_IDLE;
		break;
	}
	pev->impulse = 0;

	// No volume, use normal
	if (pev->scale <= 0)
		pev->scale = 1.0;
}

void CMessage::Precache()
{
	if (pev->noise)
		PRECACHE_SOUND((char*)STRING(pev->noise));
}

bool CMessage::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "messagesound"))
	{
		pev->noise = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "messagevolume"))
	{
		pev->scale = atof(pkvd->szValue) * 0.1;
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "messageattenuation"))
	{
		pev->impulse = atoi(pkvd->szValue);
		return true;
	}
	return CPointEntity::KeyValue(pkvd);
}

void CMessage::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	CBaseEntity* pPlayer = NULL;

	if (pev->spawnflags & SF_MESSAGE_ALL)
		UTIL_ShowMessageAll(STRING(pev->message));
	else
	{
		if (pActivator && pActivator->IsPlayer())
			pPlayer = pActivator;
		else
		{
			pPlayer = UTIL_GetLocalPlayer();
		}
		if (pPlayer)
			UTIL_ShowMessage(STRING(pev->message), pPlayer);
	}
	if (pev->noise)
	{
		EMIT_SOUND(edict(), CHAN_BODY, STRING(pev->noise), pev->scale, pev->speed);
	}
	if (pev->spawnflags & SF_MESSAGE_ONCE)
		UTIL_Remove(this);

	SUB_UseTargets(this, USE_TOGGLE, 0);
}
