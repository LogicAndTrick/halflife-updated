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

#pragma once

#include "entities/CPointEntity.h"

#define SF_GROUP_DEBUG 2

/*********************
 * Worldcraft entity: info_group
 *
 * targetname- name
 * target-     alias entity to affect
 * other values are handled in a multi_manager-like way.
 **********************/
class CInfoGroup : public CPointEntity
{
public:
	bool KeyValue(KeyValueData* pkvd) override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	int GetMember(const char* szMemberName);

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];

	int m_cMembers;
	int m_iszMemberName[MAX_MULTI_TARGETS];
	int m_iszMemberValue[MAX_MULTI_TARGETS];
	int m_iszDefaultMember;
};
