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

#include "entities/CBaseDelay.h"

//=====================
// LRC - trigger_sound
//=====================
class CTriggerSound : public CBaseDelay
{
public:
	bool KeyValue(KeyValueData* pkvd) override;
	void Spawn() override;
	void Touch(CBaseEntity* pOther) override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];
	int ObjectCaps() override { return CBaseDelay::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	float m_flRoomtype;
	string_t m_iszMaster;
};
