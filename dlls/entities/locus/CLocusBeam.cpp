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

#include "CLocusBeam.h"
#include "locus.h"
#include "entities/env/CBeam.h"

LINK_ENTITY_TO_CLASS(locus_beam, CLocusBeam);

TYPEDESCRIPTION CLocusBeam::m_SaveData[] =
	{
		DEFINE_FIELD(CLocusBeam, m_iszSprite, FIELD_STRING),
		DEFINE_FIELD(CLocusBeam, m_iszTargetName, FIELD_STRING),
		DEFINE_FIELD(CLocusBeam, m_iszStart, FIELD_STRING),
		DEFINE_FIELD(CLocusBeam, m_iszEnd, FIELD_STRING),
		DEFINE_FIELD(CLocusBeam, m_iWidth, FIELD_INTEGER),
		DEFINE_FIELD(CLocusBeam, m_iDistortion, FIELD_INTEGER),
		DEFINE_FIELD(CLocusBeam, m_fFrame, FIELD_FLOAT),
		DEFINE_FIELD(CLocusBeam, m_iScrollRate, FIELD_INTEGER),
		DEFINE_FIELD(CLocusBeam, m_fDuration, FIELD_FLOAT),
		DEFINE_FIELD(CLocusBeam, m_fDamage, FIELD_FLOAT),
		DEFINE_FIELD(CLocusBeam, m_iDamageType, FIELD_INTEGER),
		DEFINE_FIELD(CLocusBeam, m_iFlags, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CLocusBeam, CPointEntity);

bool CLocusBeam ::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "m_iszSprite"))
	{
		m_iszSprite = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszTargetName"))
	{
		m_iszTargetName = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszStart"))
	{
		m_iszStart = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszEnd"))
	{
		m_iszEnd = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iWidth"))
	{
		m_iWidth = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iDistortion"))
	{
		m_iDistortion = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_fFrame"))
	{
		m_fFrame = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iScrollRate"))
	{
		m_iScrollRate = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_fDuration"))
	{
		m_fDuration = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_fDamage"))
	{
		m_fDamage = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iDamageType"))
	{
		m_iDamageType = atoi(pkvd->szValue);
		return true;
	}
	return CBaseEntity::KeyValue(pkvd);
}

void CLocusBeam ::Precache()
{
	PRECACHE_MODEL((char*)STRING(m_iszSprite));
}

void CLocusBeam::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	CBaseEntity* pStartEnt;
	CBaseEntity* pEndEnt;
	Vector vecStartPos;
	Vector vecEndPos;
	CBeam* pBeam;

	switch (pev->impulse)
	{
	case 0: // ents
		pStartEnt = UTIL_FindEntityByTargetname(NULL, STRING(m_iszStart), pActivator);
		pEndEnt = UTIL_FindEntityByTargetname(NULL, STRING(m_iszEnd), pActivator);

		if (pStartEnt == NULL || pEndEnt == NULL)
			return;
		pBeam = CBeam::BeamCreate(STRING(m_iszSprite), m_iWidth);
		pBeam->EntsInit(pStartEnt->entindex(), pEndEnt->entindex());
		break;

	case 1: // pointent
		vecStartPos = CalcLocus_Position(this, pActivator, STRING(m_iszStart));
		pEndEnt = UTIL_FindEntityByTargetname(NULL, STRING(m_iszEnd), pActivator);

		if (pEndEnt == NULL)
			return;
		pBeam = CBeam::BeamCreate(STRING(m_iszSprite), m_iWidth);
		pBeam->PointEntInit(vecStartPos, pEndEnt->entindex());
		break;
	case 2: // points
		vecStartPos = CalcLocus_Position(this, pActivator, STRING(m_iszStart));
		vecEndPos = CalcLocus_Position(this, pActivator, STRING(m_iszEnd));

		pBeam = CBeam::BeamCreate(STRING(m_iszSprite), m_iWidth);
		pBeam->PointsInit(vecStartPos, vecEndPos);
		break;
	case 3: // point & offset
		vecStartPos = CalcLocus_Position(this, pActivator, STRING(m_iszStart));
		vecEndPos = CalcLocus_Velocity(this, pActivator, STRING(m_iszEnd));

		pBeam = CBeam::BeamCreate(STRING(m_iszSprite), m_iWidth);
		pBeam->PointsInit(vecStartPos, vecStartPos + vecEndPos);
		break;
	}
	pBeam->SetColor(pev->rendercolor.x, pev->rendercolor.y, pev->rendercolor.z);
	pBeam->SetBrightness(pev->renderamt);
	pBeam->SetNoise(m_iDistortion);
	pBeam->SetFrame(m_fFrame);
	pBeam->SetScrollRate(m_iScrollRate);
	pBeam->SetFlags(m_iFlags);
	pBeam->pev->dmg = m_fDamage;
	pBeam->pev->frags = m_iDamageType;
	pBeam->pev->spawnflags |= pev->spawnflags & (SF_BEAM_RING |
													SF_BEAM_SPARKSTART | SF_BEAM_SPARKEND | SF_BEAM_DECALS);
	if (m_fDuration)
	{
		pBeam->SetThink(&CBeam::SUB_Remove);
		pBeam->SetNextThink(m_fDuration);
	}
	pBeam->pev->targetname = m_iszTargetName;

	if (pev->target)
	{
		FireTargets(STRING(pev->target), pBeam, this, USE_TOGGLE, 0);
	}
}

void CLocusBeam::Spawn()
{
	Precache();
	m_iFlags = 0;
	if (pev->spawnflags & SF_LBEAM_SHADEIN)
		m_iFlags |= BEAM_FSHADEIN;
	if (pev->spawnflags & SF_LBEAM_SHADEOUT)
		m_iFlags |= BEAM_FSHADEOUT;
	if (pev->spawnflags & SF_LBEAM_SINE)
		m_iFlags |= BEAM_FSINE;
	if (pev->spawnflags & SF_LBEAM_SOLID)
		m_iFlags |= BEAM_FSOLID;
}
