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

#include "entities/weapon/CGrenade.h"

#define SQUEEK_DETONATE_DELAY 15.0

enum w_squeak_e
{
	WSQUEAK_IDLE1 = 0,
	WSQUEAK_FIDGET,
	WSQUEAK_JUMP,
	WSQUEAK_RUN,
};

class CSqueakGrenade : public CGrenade
{
	void Spawn() override;
	void Precache() override;
	int Classify() override;
	void EXPORT SuperBounceTouch(CBaseEntity* pOther);
	void EXPORT HuntThink();
	int BloodColor() override { return BLOOD_COLOR_YELLOW; }
	void Killed(entvars_t* pevAttacker, int iGib) override;
	void GibMonster() override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];

	static float m_flNextBounceSoundTime;

	// CBaseEntity *m_pTarget;
	float m_flDie;
	Vector m_vecTarget;
	float m_flNextHunt;
	float m_flNextHit;
	Vector m_posPrev;
	EHANDLE m_hOwner;
	int m_iMyClass;
};
