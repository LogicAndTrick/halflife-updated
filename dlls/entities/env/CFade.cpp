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

#include "CFade.h"
#include "entities/player/CBasePlayer.h"
#include "shake.h"
#include "util/effects.h"

LINK_ENTITY_TO_CLASS(env_fade, CFade);

void CFade::Spawn()
{
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	pev->effects = 0;
	pev->frame = 0;

	m_iState = STATE_OFF; // LRC
}

bool CFade::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "duration"))
	{
		SetDuration(atof(pkvd->szValue));
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "holdtime"))
	{
		SetHoldTime(atof(pkvd->szValue));
		return true;
	}

	return CPointEntity::KeyValue(pkvd);
}


void CFade::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	int fadeFlags = 0;

	if (pev->spawnflags & SF_FADE_CAMERA)
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

	m_iState = STATE_TURN_ON; // LRC
	SetNextThink(Duration()); // LRC

	if ((pev->spawnflags & SF_FADE_IN) == 0)
		fadeFlags |= FFADE_OUT;

	if ((pev->spawnflags & SF_FADE_MODULATE) != 0)
		fadeFlags |= FFADE_MODULATE;

	if (pev->spawnflags & SF_FADE_PERMANENT) // LRC
		fadeFlags |= FFADE_STAYOUT;			 // LRC

	if ((pev->spawnflags & SF_FADE_ONLYONE) != 0)
	{
		if (pActivator->IsNetClient())
		{
			UTIL_ScreenFade(pActivator, pev->rendercolor, Duration(), HoldTime(), pev->renderamt, fadeFlags);
		}
	}
	else
	{
		UTIL_ScreenFadeAll(pev->rendercolor, Duration(), HoldTime(), pev->renderamt, fadeFlags);
	}
	SUB_UseTargets(this, USE_TOGGLE, 0);
}

// LRC: a bolt-on state!
void CFade::Think()
{
	if (m_iState == STATE_TURN_ON)
	{
		m_iState = STATE_ON;
		if (!(pev->spawnflags & SF_FADE_PERMANENT))
			SetNextThink(HoldTime());
	}
	else
		m_iState = STATE_OFF;
}
