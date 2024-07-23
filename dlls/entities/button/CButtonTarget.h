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

#include "entities/CBaseEntity.h"

#define SF_BTARGET_USE 0x0001
#define SF_BTARGET_ON 0x0002
#define SF_BTARGET_SOLIDNOT 0x0004 // AJH - Just testing this at the moment
#define SF_BTARGET_NOSHOT 0x0008   // AJH - So you can't trigger by shooting

// Button_target
class CButtonTarget : public CBaseEntity
{
public:
	bool KeyValue(KeyValueData* pkvd) override; // AJH
	void Spawn() override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	bool TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;
	int ObjectCaps() override;
	string_t m_sMaster; // AJH for lockable button_targets
};
