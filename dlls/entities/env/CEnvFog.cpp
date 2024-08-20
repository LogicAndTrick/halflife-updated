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

#include "CEnvFog.h"
#include "util/movewith.h"
#include "entities/player/CBasePlayer.h"

LINK_ENTITY_TO_CLASS(env_fog, CEnvFog);

TYPEDESCRIPTION CEnvFog::m_SaveData[] =
	{
		DEFINE_FIELD(CEnvFog, m_iStartDist, FIELD_INTEGER),
		DEFINE_FIELD(CEnvFog, m_iEndDist, FIELD_INTEGER),
		DEFINE_FIELD(CEnvFog, m_iFadeIn, FIELD_INTEGER),
		DEFINE_FIELD(CEnvFog, m_iFadeOut, FIELD_INTEGER),
		DEFINE_FIELD(CEnvFog, m_fHoldTime, FIELD_FLOAT),
		DEFINE_FIELD(CEnvFog, m_fFadeStart, FIELD_TIME),
};

IMPLEMENT_SAVERESTORE(CEnvFog, CBaseEntity);

bool CEnvFog::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "startdist"))
	{
		m_iStartDist = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "enddist"))
	{
		m_iEndDist = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "fadein"))
	{
		m_iFadeIn = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "fadeout"))
	{
		m_iFadeOut = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "holdtime"))
	{
		m_fHoldTime = atof(pkvd->szValue);
		return true;
	}
	return CBaseEntity::KeyValue(pkvd);
}

STATE CEnvFog::GetState()
{
	if (pev->spawnflags & SF_FOG_ACTIVE)
	{
		if (pev->spawnflags & SF_FOG_FADING)
			return STATE_TURN_ON;
		else
			return STATE_ON;
	}
	else
	{
		if (pev->spawnflags & SF_FOG_FADING)
			return STATE_TURN_OFF;
		else
			return STATE_OFF;
	}
}

void CEnvFog::Spawn()
{
	pev->effects |= EF_NODRAW;

	if (pev->targetname == 0)
		pev->spawnflags |= SF_FOG_ACTIVE;

	if (pev->spawnflags & SF_FOG_ACTIVE)
	{
		SetThink(&CEnvFog::TurnOn);
		UTIL_DesiredThink(this);
	}

	// Precache is now used only to continue after a game has loaded.
	//	Precache();

	// things get messed up if we try to draw fog with a startdist
	// or an enddist of 0, so we don't allow it.
	if (m_iStartDist == 0)
		m_iStartDist = 1;
	if (m_iEndDist == 0)
		m_iEndDist = 1;
}

void CEnvFog::Precache()
{
	if (pev->spawnflags & SF_FOG_ACTIVE)
	{
		SetThink(&CEnvFog::ResumeThink);
		SetNextThink(0.1);
	}
}

extern int gmsgSetFog;

void CEnvFog::TurnOn()
{
	//	ALERT(at_console, "Fog turnon %f\n", gpGlobals->time);

	pev->spawnflags |= SF_FOG_ACTIVE;

	if (m_iFadeIn)
	{
		pev->spawnflags |= SF_FOG_FADING;
		SendData(pev->rendercolor, m_iFadeIn, m_iStartDist, m_iEndDist);
		SetNextThink(m_iFadeIn);
		SetThink(&CEnvFog::FadeInDone);
	}
	else
	{
		pev->spawnflags &= ~SF_FOG_FADING;
		SendData(pev->rendercolor, 0, m_iStartDist, m_iEndDist);
		if (m_fHoldTime)
		{
			SetNextThink(m_fHoldTime);
			SetThink(&CEnvFog::TurnOff);
		}
	}
}

void CEnvFog::TurnOff()
{
	//	ALERT(at_console, "Fog turnoff\n");

	pev->spawnflags &= ~SF_FOG_ACTIVE;

	if (m_iFadeOut)
	{
		pev->spawnflags |= SF_FOG_FADING;
		SendData(pev->rendercolor, -m_iFadeOut, m_iStartDist, m_iEndDist);
		SetNextThink(m_iFadeOut);
		SetThink(&CEnvFog::FadeOutDone);
	}
	else
	{
		pev->spawnflags &= ~SF_FOG_FADING;
		SendData(g_vecZero, 0, 0, 0);
		DontThink();
	}
}

// yes, this intermediate think function is necessary.
//  the engine seems to ignore the nextthink time when starting up.
//  So this function gets called immediately after the precache finishes,
//  regardless of what nextthink time is specified.
void CEnvFog::ResumeThink()
{
	//	ALERT(at_console, "Fog resume %f\n", gpGlobals->time);
	SetThink(&CEnvFog ::Resume2Think);
	SetNextThink(0.1);
}

void CEnvFog ::Resume2Think()
{
	SendData(pev->rendercolor, m_iFadeIn, m_iStartDist, m_iEndDist);
}

void CEnvFog::FadeInDone()
{
	pev->spawnflags &= ~SF_FOG_FADING;
	// LRC 1.8 we don't need to resend...	SendData( pev->rendercolor, 0, m_iStartDist, m_iEndDist);

	if (m_fHoldTime)
	{
		SetNextThink(m_fHoldTime);
		SetThink(&CEnvFog::TurnOff);
	}
}

void CEnvFog::FadeOutDone()
{
	pev->spawnflags &= ~SF_FOG_FADING;
	SendData(g_vecZero, 0, 0, 0);
}

void CEnvFog::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	//	ALERT(at_console, "Fog use %s %s\n", GetStringForUseType(useType), GetStringForState(GetState()));
	if (ShouldToggle(useType))
	{
		if (pev->spawnflags & SF_FOG_ACTIVE)
			TurnOff();
		else
			TurnOn();
	}
}

void CEnvFog::SendData(Vector col, int iFadeTime, int iStartDist, int iEndDist)
{
	//	ALERT(at_console, "Fog send (%d %d %d), %d - %d\n", col.x, col.y, col.z, iStartDist, iEndDist);
	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CBasePlayer* pPlayer = (CBasePlayer*)UTIL_PlayerByIndex(i);
		if (pPlayer)
		{
			MESSAGE_BEGIN(MSG_ONE, gmsgSetFog, NULL, pPlayer->pev);
			WRITE_BYTE(col.x);
			WRITE_BYTE(col.y);
			WRITE_BYTE(col.z);
			WRITE_SHORT(iFadeTime);
			WRITE_SHORT(iStartDist);
			WRITE_SHORT(iEndDist);
			MESSAGE_END();

			//			pPlayer->m_iFogStartDist = iStartDist;
			//			pPlayer->m_iFogEndDist = iEndDist;
			//			pPlayer->m_vecFogColor = col;
			//			pPlayer->m_bClientFogRefresh = false;
		}
	}
}
