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

#define SF_MIRROR_DRAWPLAYER 0x01

// G-Cont - env_mirror, mirroring only models
class CEnvMirror : public CBaseEntity
{
public:
	void Spawn(void) override;
	void Precache(void) override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	bool KeyValue(KeyValueData* pkvd) override;
	int ObjectCaps(void) override { return CBaseEntity ::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	void EXPORT MirrorThink(void);
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	STATE GetState(void) override { return m_iActive ? STATE_ON : STATE_OFF; } // Support this stuff for watcher
	bool m_iActive;
	float m_flRadius;
	static TYPEDESCRIPTION m_SaveData[];

private:
	int m_iInitialRenderMode;
	bool bSent;
	unsigned short m_usMirror; // Moved from weapons.cpp for more struct code. G-Cont
};
