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

#define SF_ALIAS_OFF 1
#define SF_ALIAS_DEBUG 2

#define MAX_ALIAS_TARGETS 16 // AJH

/*********************
 * Worldcraft entity: info_alias
 *
 * targetname- alias name
 * target-     alias destination while ON
 * netname-    alias destination while OFF
 * mode-		  0= On/Off mode, 1= 'list' mode
 **********************/
class CInfoAlias : public CBaseMutableAlias // AJH Now includes 'listmode' aliasing
{
public:
	int m_cTargets;						  // AJH the total number of targets in this alias's fire list.
	int m_iTargetName[MAX_ALIAS_TARGETS]; // AJH list of indexes into global string array
	int m_iMode;						  // AJH 0 = On/Off mode, 1 = list mode
	int m_iCurrentTarget;				  // AJH the current target that is being aliased

	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	void Spawn() override;
	STATE GetState() override { return (pev->spawnflags & SF_ALIAS_OFF) ? STATE_OFF : STATE_ON; }

	CBaseEntity* FollowAlias(CBaseEntity* pFrom) override;
	void ChangeValue(int iszValue) override;
	void FlushChanges() override;
	bool KeyValue(struct KeyValueData_s*) override; // AJH

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];
};