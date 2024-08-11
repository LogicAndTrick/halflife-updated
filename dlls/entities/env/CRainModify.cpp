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

#include "CRainModify.h"
#include "player.h"

LINK_ENTITY_TO_CLASS(rain_modify, CRainModify);

TYPEDESCRIPTION CRainModify::m_SaveData[] =
	{
		DEFINE_FIELD(CRainModify, fadeTime, FIELD_FLOAT),
		DEFINE_FIELD(CRainModify, Rain_Drips, FIELD_INTEGER),
		DEFINE_FIELD(CRainModify, Rain_randX, FIELD_FLOAT),
		DEFINE_FIELD(CRainModify, Rain_randY, FIELD_FLOAT),
		DEFINE_FIELD(CRainModify, Rain_windX, FIELD_FLOAT),
		DEFINE_FIELD(CRainModify, Rain_windY, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(CRainModify, CBaseEntity);

void CRainModify::Spawn()
{
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	pev->effects |= EF_NODRAW;

	if (FStringNull(pev->targetname))
		pev->spawnflags |= 1;
}

bool CRainModify::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "m_iDripsPerSecond"))
	{
		Rain_Drips = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_flWindX"))
	{
		Rain_windX = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_flWindY"))
	{
		Rain_windY = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_flRandX"))
	{
		Rain_randX = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_flRandY"))
	{
		Rain_randY = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_flTime"))
	{
		fadeTime = atof(pkvd->szValue);
		return true;
	}

	return CBaseEntity::KeyValue(pkvd);
}

void CRainModify::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (pev->spawnflags & 1)
		return; // constant

	if (gpGlobals->deathmatch)
	{
		ALERT(at_console, "Rain error: only static rain in multiplayer\n");
		return; // not in multiplayer
	}

	CBasePlayer* pPlayer;
	pPlayer = (CBasePlayer*)CBaseEntity::Instance(g_engfuncs.pfnPEntityOfEntIndex(1));

	if (fadeTime)
	{ // write to 'ideal' settings
		pPlayer->Rain_ideal_dripsPerSecond = Rain_Drips;
		pPlayer->Rain_ideal_randX = Rain_randX;
		pPlayer->Rain_ideal_randY = Rain_randY;
		pPlayer->Rain_ideal_windX = Rain_windX;
		pPlayer->Rain_ideal_windY = Rain_windY;

		pPlayer->Rain_endFade = gpGlobals->time + fadeTime;
		pPlayer->Rain_nextFadeUpdate = gpGlobals->time + 1;
	}
	else
	{
		pPlayer->Rain_dripsPerSecond = Rain_Drips;
		pPlayer->Rain_randX = Rain_randX;
		pPlayer->Rain_randY = Rain_randY;
		pPlayer->Rain_windX = Rain_windX;
		pPlayer->Rain_windY = Rain_windY;

		pPlayer->Rain_needsUpdate = 1;
	}
}
