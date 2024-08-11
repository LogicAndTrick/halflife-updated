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

#include "CEnvMirror.h"

LINK_ENTITY_TO_CLASS(env_mirror, CEnvMirror);

TYPEDESCRIPTION CEnvMirror::m_SaveData[] =
	{
		DEFINE_FIELD(CEnvMirror, m_iInitialRenderMode, FIELD_INTEGER),
		DEFINE_FIELD(CEnvMirror, m_iActive, FIELD_INTEGER),
		DEFINE_FIELD(CEnvMirror, m_flRadius, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(CEnvMirror, CBaseEntity);

bool CEnvMirror ::KeyValue(KeyValueData* pkvd)
{

	if (FStrEq(pkvd->szKeyName, "radius"))
	{
		m_flRadius = atof(pkvd->szValue);
		return true;
	}

	return CBaseEntity::KeyValue(pkvd);
}

void CEnvMirror ::Spawn(void)
{
	// G-Cont. added this for emulate shiny floor (without this flag monsters can't walk on it)
	if (!m_pMoveWith)
		pev->flags |= FL_WORLDBRUSH;

	pev->angles = g_vecZero;
	pev->movetype = MOVETYPE_PUSH; // so it doesn't get pushed by anything

	pev->solid = SOLID_BSP;

	pev->impulse = 1;
	m_iActive = true;

	Precache();

	SET_MODEL(ENT(pev), STRING(pev->model));
	SetThink(&CEnvMirror ::MirrorThink);
	if (pev->spawnflags & SF_MIRROR_DRAWPLAYER)
		CBaseEntity::Create("player_marker", VecBModelOrigin(pev), pev->angles, NULL);
	m_iInitialRenderMode = pev->rendermode;
	if (!m_flRadius)
		m_flRadius = 330;
	if (!pev->frags) // Smart field system. g-cont
	{
		if (pev->size.y > pev->size.x && pev->size.z > pev->size.x)
			pev->frags = 0;
		if (pev->size.x > pev->size.y && pev->size.z > pev->size.y)
			pev->frags = 1;
		if (pev->size.y > pev->size.z && pev->size.x > pev->size.z)
			pev->frags = 2;
	}

	SetNextThink(0.1);
}

void CEnvMirror ::Precache(void)
{
	if (pev->spawnflags & SF_MIRROR_DRAWPLAYER)
		UTIL_PrecacheOther("player_marker");
	m_usMirror = PRECACHE_EVENT(1, "events/mirror.sc");
	bSent = false;
}

void CEnvMirror ::MirrorThink(void)
{
	if (bSent)
	{
		SetNextThink(0.01);
		return;
	}

	if (pev->impulse)
	{
		if (pev->rendermode != m_iInitialRenderMode)
			pev->rendermode = m_iInitialRenderMode;

		if (UTIL_PlayerByIndex(1))
		{
			PLAYBACK_EVENT_FULL(FEV_RELIABLE | FEV_GLOBAL, edict(), m_usMirror, 0.0, Center(), g_vecZero, 0.0, 0.0, m_flRadius, pev->frags, 1, 0);
			bSent = true;
		}
	}
	else
	{
		if (pev->rendermode != kRenderNormal)
		{
			pev->rendermode = kRenderNormal;
			PLAYBACK_EVENT_FULL(FEV_RELIABLE | FEV_GLOBAL, edict(), m_usMirror, 0.0, Center(), g_vecZero, 0.0, 0.0, m_flRadius, pev->frags, 0, 0);
		}

		bSent = true;
	}
	SetNextThink(0.01);
}

void CEnvMirror ::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (ShouldToggle(useType, m_iActive))
		m_iActive = !m_iActive;

	if (m_iActive)
	{
		m_iActive = true;
		pev->impulse = 1;
		bSent = false;
	}
	else
	{
		m_iActive = false;
		pev->impulse = 0;
		bSent = false;
	}
}
