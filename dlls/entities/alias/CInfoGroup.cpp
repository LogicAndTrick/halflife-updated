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

#include "CInfoGroup.h"
#include "CBaseMutableAlias.h"

LINK_ENTITY_TO_CLASS(info_group, CInfoGroup);

TYPEDESCRIPTION CInfoGroup::m_SaveData[] =
	{
		DEFINE_FIELD(CInfoGroup, m_cMembers, FIELD_INTEGER),
		DEFINE_ARRAY(CInfoGroup, m_iszMemberName, FIELD_STRING, MAX_MULTI_TARGETS),
		DEFINE_ARRAY(CInfoGroup, m_iszMemberValue, FIELD_STRING, MAX_MULTI_TARGETS),
		DEFINE_FIELD(CInfoGroup, m_iszDefaultMember, FIELD_STRING),
};

IMPLEMENT_SAVERESTORE(CInfoGroup, CBaseEntity);

bool CInfoGroup::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "defaultmember"))
	{
		m_iszDefaultMember = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	// this assumes that additional fields are targetnames and their values are delay values.
	else if (m_cMembers < MAX_MULTI_TARGETS)
	{
		char tmp[128];
		UTIL_StripToken(pkvd->szKeyName, tmp);
		m_iszMemberName[m_cMembers] = ALLOC_STRING(tmp);
		m_iszMemberValue[m_cMembers] = ALLOC_STRING(pkvd->szValue);
		m_cMembers++;
		return true;
	}
	else
	{
		ALERT(at_error, "Too many members for info_group %s (limit is %d)\n", STRING(pev->targetname), MAX_MULTI_TARGETS);
		return false;
	}
}

void CInfoGroup::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	CBaseEntity* pTarget = UTIL_FindEntityByTargetname(NULL, STRING(pev->target));

	if (pTarget && pTarget->IsMutableAlias())
	{
		if (pev->spawnflags & SF_GROUP_DEBUG)
			ALERT(at_debug, "DEBUG: info_group %s changes the contents of %s \"%s\"\n", STRING(pev->targetname), STRING(pTarget->pev->classname), STRING(pTarget->pev->targetname));
		((CBaseMutableAlias*)pTarget)->ChangeValue(this);
	}
	else if (pev->target)
	{
		ALERT(at_debug, "info_group \"%s\": alias \"%s\" was not found or not an alias!", STRING(pev->targetname), STRING(pev->target));
	}
}

int CInfoGroup::GetMember(const char* szMemberName)
{
	if (!szMemberName)
	{
		ALERT(at_debug, "info_group: GetMember called with null szMemberName!?\n");
		return NULL;
	}
	for (int i = 0; i < m_cMembers; i++)
	{
		if (FStrEq(szMemberName, STRING(m_iszMemberName[i])))
		{
			//			ALERT(at_console,"getMember: found member\n");
			return m_iszMemberValue[i];
		}
	}

	if (m_iszDefaultMember)
	{
		static char szBuffer[128];
		strcpy(szBuffer, STRING(m_iszDefaultMember));
		strcat(szBuffer, szMemberName);
		return MAKE_STRING(szBuffer);
		// this is a messy way to do it... but currently, only one
		// GetMember gets performed at a time, so it works.
	}

	ALERT(at_debug, "info_group \"%s\" has no member called \"%s\".\n", STRING(pev->targetname), szMemberName);
	//	ALERT(at_console,"getMember: fail\n");
	return NULL;
}
