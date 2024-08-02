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

#include "CBaseMutableAlias.h"

/*********************
 * Worldcraft entity: multi_alias
 *
 * targetname- name
 * other values are handled in a multi_manager-like way.
 **********************/
class CMultiAlias : public CBaseMutableAlias
{
public:
	bool KeyValue(KeyValueData* pkvd) override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];

	CBaseEntity* FollowAlias(CBaseEntity* pFrom) override;

	int m_cTargets;
	int m_iszTargets[MAX_MULTI_TARGETS];
	int m_iTotalValue;
	int m_iValues[MAX_MULTI_TARGETS];
	int m_iMode;
};
