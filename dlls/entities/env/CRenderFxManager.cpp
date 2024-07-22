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

#include "CRenderFxManager.h"
#include "locus.h"

LINK_ENTITY_TO_CLASS(env_render, CRenderFxManager);

bool CRenderFxManager::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "m_fScale"))
	{
		pev->noise = ALLOC_STRING(pkvd->szValue); // AJH Allows LR scale (Thanks Mike)
		return true;
	}
	return CPointEntity::KeyValue(pkvd);
}

void CRenderFxManager::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (!FStringNull(pev->target))
	{
		CBaseEntity* pTarget = UTIL_FindEntityByTargetname(NULL, STRING(pev->target), pActivator);
		bool first = true;
		while (pTarget != NULL)
		{
			Affect(pTarget, first, pActivator);
			first = false;
			pTarget = UTIL_FindEntityByTargetname(pTarget, STRING(pev->target), pActivator);
		}
	}

	if (pev->spawnflags & SF_RENDER_ONLYONCE)
	{
		SetThink(&CRenderFxManager::SUB_Remove);
		SetNextThink(0.1);
	}
}

void CRenderFxManager::Affect(CBaseEntity* pTarget, bool bIsFirst, CBaseEntity* pActivator)
{
	entvars_t* pevTarget = pTarget->pev;

	float fAmtFactor = 1;
	if (pev->message && !FBitSet(pev->spawnflags, SF_RENDER_MASKAMT))
		fAmtFactor = CalcLocus_Number(pActivator, STRING(pev->message));

	if (!FBitSet(pev->spawnflags, SF_RENDER_MASKFX))
		pevTarget->renderfx = pev->renderfx;
	if (!FBitSet(pev->spawnflags, SF_RENDER_MASKMODE))
	{
		// LRC - amt is often 0 when mode is normal. Set it to be fully visible, for fade purposes.
		if (pev->frags && pevTarget->renderamt == 0 && pevTarget->rendermode == kRenderNormal)
			pevTarget->renderamt = 255;
		pevTarget->rendermode = pev->rendermode;
	}
	if (pev->frags == 0) // not fading?
	{
		if (!FBitSet(pev->spawnflags, SF_RENDER_MASKAMT))
			pevTarget->renderamt = pev->renderamt * fAmtFactor;
		if (!FBitSet(pev->spawnflags, SF_RENDER_MASKCOLOR))
			pevTarget->rendercolor = pev->rendercolor;
		if (pev->noise)
		{
			pevTarget->scale = CalcLocus_Number(pActivator, STRING(pev->noise)); // AJH Allows LR scale
			ALERT(at_debug, "Setting scale from %s, to %f\n", STRING(pev->noise), pevTarget->scale);
		}

		if (bIsFirst)
			FireTargets(STRING(pev->netname), pTarget, this, USE_TOGGLE, 0);
	}
	else
	{
		// LRC - fade the entity in/out!
		//  (We create seperate fader entities to do this, one for each entity that needs fading.)
		CRenderFxFader* pFader = GetClassPtr((CRenderFxFader*)NULL);
		pFader->m_hTarget = pTarget;
		pFader->m_iStartAmt = pevTarget->renderamt;
		pFader->m_vecStartColor = pevTarget->rendercolor;
		pFader->m_fStartScale = pevTarget->scale;
		if (pFader->m_fStartScale == 0)
			pFader->m_fStartScale = 1; // When we're scaling, 0 is treated as 1. Use 1 as the number to fade from.
		pFader->pev->spawnflags = pev->spawnflags;

		if (bIsFirst)
			pFader->pev->target = pev->netname;

		if (!FBitSet(pev->spawnflags, SF_RENDER_MASKAMT))
			pFader->m_iOffsetAmt = (pev->renderamt * fAmtFactor) - pevTarget->renderamt;
		else
			pFader->m_iOffsetAmt = 0;

		if (!FBitSet(pev->spawnflags, SF_RENDER_MASKCOLOR))
		{
			pFader->m_vecOffsetColor.x = pev->rendercolor.x - pevTarget->rendercolor.x;
			pFader->m_vecOffsetColor.y = pev->rendercolor.y - pevTarget->rendercolor.y;
			pFader->m_vecOffsetColor.z = pev->rendercolor.z - pevTarget->rendercolor.z;
		}
		else
		{
			pFader->m_vecOffsetColor = g_vecZero;
		}

		if (pev->noise)
			pFader->m_fOffsetScale = CalcLocus_Number(pActivator, STRING(pev->noise)) - pevTarget->scale; // AJH Allows LR scale
		else
			pFader->m_fOffsetScale = 0;

		pFader->m_flStartTime = gpGlobals->time;
		pFader->m_flDuration = pev->frags;
		pFader->m_flCoarseness = pev->armorvalue;
		pFader->SetNextThink(0);
		pFader->Spawn();
	}
}

// ---------------------
// ---------------------
// ---------------------

TYPEDESCRIPTION CRenderFxFader::m_SaveData[] =
	{
		DEFINE_FIELD(CRenderFxFader, m_flStartTime, FIELD_FLOAT),
		DEFINE_FIELD(CRenderFxFader, m_flDuration, FIELD_FLOAT),
		DEFINE_FIELD(CRenderFxFader, m_flCoarseness, FIELD_FLOAT),
		DEFINE_FIELD(CRenderFxFader, m_iStartAmt, FIELD_INTEGER),
		DEFINE_FIELD(CRenderFxFader, m_iOffsetAmt, FIELD_INTEGER),
		DEFINE_FIELD(CRenderFxFader, m_vecStartColor, FIELD_VECTOR),
		DEFINE_FIELD(CRenderFxFader, m_vecOffsetColor, FIELD_VECTOR),
		DEFINE_FIELD(CRenderFxFader, m_fStartScale, FIELD_FLOAT),
		DEFINE_FIELD(CRenderFxFader, m_fOffsetScale, FIELD_FLOAT),
		DEFINE_FIELD(CRenderFxFader, m_hTarget, FIELD_EHANDLE),
};

IMPLEMENT_SAVERESTORE(CRenderFxFader, CBaseEntity);

void CRenderFxFader::Spawn()
{
	SetThink(&CRenderFxFader::FadeThink);
}

void CRenderFxFader::FadeThink()
{
	if (((CBaseEntity*)m_hTarget) == NULL)
	{
		//		ALERT(at_console, "render_fader removed\n");
		SUB_Remove();
		return;
	}

	float flDegree = (gpGlobals->time - m_flStartTime) / m_flDuration;

	if (flDegree >= 1)
	{
		//		ALERT(at_console, "render_fader removes self\n");

		m_hTarget->pev->renderamt = m_iStartAmt + m_iOffsetAmt;
		m_hTarget->pev->rendercolor = m_vecStartColor + m_vecOffsetColor;
		m_hTarget->pev->scale = m_fStartScale + m_fOffsetScale;

		SUB_UseTargets(m_hTarget, USE_TOGGLE, 0);

		if (pev->spawnflags & SF_RENDER_KILLTARGET)
		{
			m_hTarget->SetThink(&CBaseEntity::SUB_Remove);
			m_hTarget->SetNextThink(0.1);
		}

		m_hTarget = NULL;

		SetNextThink(0.1);
		SetThink(&CRenderFxFader::SUB_Remove);
	}
	else
	{
		m_hTarget->pev->renderamt = m_iStartAmt + m_iOffsetAmt * flDegree;

		m_hTarget->pev->rendercolor.x = m_vecStartColor.x + m_vecOffsetColor.x * flDegree;
		m_hTarget->pev->rendercolor.y = m_vecStartColor.y + m_vecOffsetColor.y * flDegree;
		m_hTarget->pev->rendercolor.z = m_vecStartColor.z + m_vecOffsetColor.z * flDegree;

		m_hTarget->pev->scale = m_fStartScale + m_fOffsetScale * flDegree;

		SetNextThink(m_flCoarseness); //?
	}
}
