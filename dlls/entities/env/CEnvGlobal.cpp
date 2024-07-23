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

#include "CEnvGlobal.h"

LINK_ENTITY_TO_CLASS(env_global, CEnvGlobal);

TYPEDESCRIPTION CEnvGlobal::m_SaveData[] =
	{
		DEFINE_FIELD(CEnvGlobal, m_globalstate, FIELD_STRING),
		DEFINE_FIELD(CEnvGlobal, m_triggermode, FIELD_INTEGER),
		DEFINE_FIELD(CEnvGlobal, m_initialstate, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CEnvGlobal, CBaseEntity);

bool CEnvGlobal::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "globalstate")) // State name
	{
		m_globalstate = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "triggermode"))
	{
		m_triggermode = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "initialstate"))
	{
		m_initialstate = atoi(pkvd->szValue);
		return true;
	}

	return CPointEntity::KeyValue(pkvd);
}

void CEnvGlobal::Spawn()
{
	if (FStringNull(m_globalstate))
	{
		REMOVE_ENTITY(ENT(pev));
		return;
	}
	if (FBitSet(pev->spawnflags, SF_GLOBAL_SET))
	{
		if (!gGlobalState.EntityInTable(m_globalstate))
			gGlobalState.EntityAdd(m_globalstate, gpGlobals->mapname, (GLOBALESTATE)m_initialstate);
	}
}

void CEnvGlobal::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	GLOBALESTATE oldState = gGlobalState.EntityGetState(m_globalstate);
	GLOBALESTATE newState;

	if (useType == USE_ON)
	{						  //
		newState = GLOBAL_ON; // AJH Allow env_global to use USE_TYPE
	}
	else if (useType == USE_OFF)
	{						   //
		newState = GLOBAL_OFF; //
	}
	else
	{ //

		switch (m_triggermode)
		{
		case 0:
			newState = GLOBAL_OFF;
			break;

		case 1:
			newState = GLOBAL_ON;
			break;

		case 2:
			newState = GLOBAL_DEAD;
			break;

		default:
		case 3:
			if (oldState == GLOBAL_ON)
				newState = GLOBAL_OFF;
			else if (oldState == GLOBAL_OFF)
				newState = GLOBAL_ON;
			else
				newState = oldState;
		}
	}

	if (gGlobalState.EntityInTable(m_globalstate))
		gGlobalState.EntitySetState(m_globalstate, newState);
	else
		gGlobalState.EntityAdd(m_globalstate, gpGlobals->mapname, newState);
}
