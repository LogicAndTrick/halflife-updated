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

class CBaseMutableAlias : public CPointEntity
{
public:
	bool IsMutableAlias() override { return true; }
	virtual CBaseEntity* FollowAlias(CBaseEntity* pFrom) { return NULL; };
	virtual void ChangeValue(int iszValue) { ALERT(at_error, "%s entities cannot change value!", STRING(pev->classname)); }
	virtual void ChangeValue(CBaseEntity* pValue) { ChangeValue(pValue->pev->targetname); }
	virtual void FlushChanges(){};

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	CBaseMutableAlias* m_pNextAlias;
};
