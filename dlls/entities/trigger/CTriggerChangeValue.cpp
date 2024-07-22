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

#include "CTriggerChangeValue.h"

LINK_ENTITY_TO_CLASS(trigger_changevalue, CTriggerChangeValue);

TYPEDESCRIPTION CTriggerChangeValue::m_SaveData[] =
	{
		DEFINE_FIELD(CTriggerChangeValue, m_iszNewValue, FIELD_STRING),
};

IMPLEMENT_SAVERESTORE(CTriggerChangeValue, CBaseDelay);

bool CTriggerChangeValue::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "m_iszNewValue"))
	{
		m_iszNewValue = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	return CBaseDelay::KeyValue(pkvd);
}

void CTriggerChangeValue::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	CBaseEntity* pTarget = UTIL_FindEntityByTargetname(NULL, STRING(pev->target), pActivator);

	if (pTarget)
	{
		KeyValueData mypkvd;
		mypkvd.szKeyName = (char*)STRING(pev->netname);
		mypkvd.szValue = (char*)STRING(m_iszNewValue);
		mypkvd.fHandled = false;
		pTarget->KeyValue(&mypkvd);
		// Error if not handled?
	}
}
