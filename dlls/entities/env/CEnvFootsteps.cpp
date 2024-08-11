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

#include "CEnvFootsteps.h"

LINK_ENTITY_TO_CLASS(env_footsteps, CEnvFootsteps);

void CEnvFootsteps::Spawn()
{
	Precache();
}

void CEnvFootsteps::PrecacheNoise(const char* szNoise)
{
	static char szBuf[128];
	int i = 0, j = 0;
	for (i = 0; szNoise[i]; i++)
	{
		if (szNoise[i] == '?')
		{
			strcpy(szBuf, szNoise);
			for (j = 0; j < 4; j++)
			{
				szBuf[i] = j + '1';
				PRECACHE_SOUND(szBuf);
			}
		}
	}
	if (!j)
		PRECACHE_SOUND((char*)szNoise);
}

void CEnvFootsteps::Precache()
{
	if (pev->noise)
		PrecacheNoise(STRING(pev->noise));
	if (pev->noise1)
		PrecacheNoise(STRING(pev->noise1));
	if (pev->noise2)
		PrecacheNoise(STRING(pev->noise2));
	if (pev->noise3)
		PrecacheNoise(STRING(pev->noise3));
}

STATE CEnvFootsteps::GetState()
{
	if (pev->spawnflags & SF_FOOTSTEPS_SET)
		return STATE_OFF;
	return pev->impulse ? STATE_ON : STATE_OFF;
}

STATE CEnvFootsteps::GetState(CBaseEntity* pEnt)
{
	if (pev->spawnflags & SF_FOOTSTEPS_SET)
		return STATE_OFF;
	if (pEnt->IsPlayer())
	{
		// based on trigger_hurt code
		int playerMask = 1 << (pEnt->entindex() - 1);

		if (pev->impulse & playerMask)
			return STATE_ON;
		else
			return STATE_OFF;
	}
	else
		return GetState();
}

void CEnvFootsteps::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	//	union floatToString ftsTemp;

	// CONSIDER: add an "all players" spawnflag, like game_text?
	if (pActivator && pActivator->IsPlayer())
	{
		int playerMask = 1 << (pActivator->entindex() - 1);

		if (pev->spawnflags & SF_FOOTSTEPS_SET || (!(pev->impulse & playerMask) && (useType == USE_ON || useType == USE_TOGGLE)))
		{
			pev->impulse |= playerMask;
			if (pev->frags)
			{
				char sTemp[4];
				sprintf(sTemp, "%d", (int)pev->frags);
				g_engfuncs.pfnSetPhysicsKeyValue(pActivator->edict(), "stype", sTemp);
				// pActivator->pev->iFootstepType = pev->frags;
			}
			else if (pev->noise)
			{
				g_engfuncs.pfnSetPhysicsKeyValue(pActivator->edict(), "ssnd", STRING(pev->noise));
			}
			if (pev->noise1)
			{
				g_engfuncs.pfnSetPhysicsKeyValue(pActivator->edict(), "lsnd", STRING(pev->noise1));
			}
			if (pev->noise2)
			{
				g_engfuncs.pfnSetPhysicsKeyValue(pActivator->edict(), "wsnd", STRING(pev->noise2));
			}
			if (pev->noise3)
			{
				g_engfuncs.pfnSetPhysicsKeyValue(pActivator->edict(), "psnd", STRING(pev->noise3));
			}
			// workaround for physinfo string bug: force the engine to null-terminate it
			g_engfuncs.pfnSetPhysicsKeyValue(pActivator->edict(), "x", "0");
			// ALERT(at_console, "ON, InfoString = %s\n", g_engfuncs.pfnGetPhysicsInfoString(pActivator->edict()));
			if (pev->spawnflags & SF_FOOTSTEPS_SET && pev->spawnflags & SF_FOOTSTEPS_ONCE)
			{
				UTIL_Remove(this);
			}
		}
		else if ((pev->impulse & playerMask) && (useType == USE_OFF || useType == USE_TOGGLE))
		{
			pev->impulse &= ~playerMask;
			if (pev->frags)
			{
				g_engfuncs.pfnSetPhysicsKeyValue(pActivator->edict(), "stype", "0");
			}
			else if (pev->noise)
			{
				g_engfuncs.pfnSetPhysicsKeyValue(pActivator->edict(), "ssnd", "0");
			}
			if (pev->noise1)
			{
				g_engfuncs.pfnSetPhysicsKeyValue(pActivator->edict(), "lsnd", "0");
			}
			if (pev->noise2)
			{
				g_engfuncs.pfnSetPhysicsKeyValue(pActivator->edict(), "wsnd", "0");
			}
			if (pev->noise3)
			{
				g_engfuncs.pfnSetPhysicsKeyValue(pActivator->edict(), "psnd", "0");
			}
			// workaround for physinfo string bug: force the engine to null-terminate it
			g_engfuncs.pfnSetPhysicsKeyValue(pActivator->edict(), "x", "0");
			// ALERT(at_console, "OFF, InfoString = %s\n", g_engfuncs.pfnGetPhysicsInfoString(pActivator->edict()));
			if (pev->spawnflags & SF_FOOTSTEPS_ONCE)
			{
				UTIL_Remove(this);
			}
		}
		else
		{
			// ALERT(at_console, "NO EFFECT\n");
		}
	}
}
