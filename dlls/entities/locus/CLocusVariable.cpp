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

#include "CLocusVariable.h"
#include "util/locus.h"

LINK_ENTITY_TO_CLASS(locus_variable, CLocusVariable);

TYPEDESCRIPTION CLocusVariable::m_SaveData[] =
	{
		DEFINE_FIELD(CLocusVariable, m_iszPosition, FIELD_STRING),
		DEFINE_FIELD(CLocusVariable, m_iszVelocity, FIELD_STRING),
		DEFINE_FIELD(CLocusVariable, m_iszRatio, FIELD_STRING),
		DEFINE_FIELD(CLocusVariable, m_iszTargetName, FIELD_STRING),
		DEFINE_FIELD(CLocusVariable, m_iszFireOnSpawn, FIELD_STRING),
		DEFINE_FIELD(CLocusVariable, m_fDuration, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(CLocusVariable, CPointEntity)

bool CLocusVariable ::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "m_iszPosition"))
	{
		m_iszPosition = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszVelocity"))
	{
		m_iszVelocity = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszRatio"))
	{
		m_iszRatio = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszTargetName"))
	{
		m_iszTargetName = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszFireOnSpawn"))
	{
		m_iszFireOnSpawn = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_fDuration"))
	{
		m_fDuration = atof(pkvd->szValue);
		return true;
	}
	return CPointEntity::KeyValue(pkvd);
}

void CLocusVariable::Spawn()
{
	SetMovedir(pev);
}

void CLocusVariable::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	Vector vecPos = g_vecZero;
	Vector vecDir = g_vecZero;
	float fRatio = 0;
	if (m_iszPosition)
		vecPos = CalcLocus_Position(this, pActivator, STRING(m_iszPosition));
	if (m_iszVelocity)
		vecDir = CalcLocus_Velocity(this, pActivator, STRING(m_iszVelocity));
	if (m_iszRatio)
		fRatio = CalcLocus_Number(pActivator, STRING(m_iszRatio));

	if (m_iszTargetName)
	{
		CMark* pMark = GetClassPtr((CMark*)NULL);
		pMark->pev->classname = MAKE_STRING("mark");
		pMark->pev->origin = vecPos;
		pMark->pev->movedir = vecDir;
		pMark->pev->frags = fRatio;
		pMark->pev->targetname = m_iszTargetName;
		pMark->SetNextThink(m_fDuration);

		FireTargets(STRING(m_iszFireOnSpawn), pMark, this, USE_TOGGLE, 0);
	}
	else
	{
		pev->origin = vecPos;
		pev->movedir = vecDir;
		pev->frags = fRatio;

		FireTargets(STRING(m_iszFireOnSpawn), this, this, USE_TOGGLE, 0);
	}
}
