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

#include "CLocusAlias.h"

LINK_ENTITY_TO_CLASS(locus_alias, CLocusAlias);

TYPEDESCRIPTION CLocusAlias::m_SaveData[] =
	{
		DEFINE_FIELD(CLocusAlias, m_hValue, FIELD_EHANDLE),
		DEFINE_FIELD(CLocusAlias, m_hChangeTo, FIELD_EHANDLE),
};

IMPLEMENT_SAVERESTORE(CLocusAlias, CBaseMutableAlias);

void CLocusAlias::PostSpawn()
{
	m_hValue = UTIL_FindEntityByTargetname(NULL, STRING(pev->netname));
}

void CLocusAlias::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	m_hChangeTo = pActivator;
	UTIL_AddToAliasList(this);
}

void CLocusAlias::FlushChanges()
{
	m_hValue = m_hChangeTo;
	m_hChangeTo = NULL;
}

CBaseEntity* CLocusAlias::FollowAlias(CBaseEntity* pFrom)
{
	if (m_hValue == NULL)
		return NULL;
	else if (pFrom == NULL || (OFFSET(m_hValue->pev) > OFFSET(pFrom->pev)))
	{
		//		ALERT(at_console, "LocusAlias returns %s:  %f %f %f\n", STRING(m_pValue->pev->targetname), m_pValue->pev->origin.x, m_pValue->pev->origin.y, m_pValue->pev->origin.z);
		return m_hValue;
	}
	else
		return NULL;
}
