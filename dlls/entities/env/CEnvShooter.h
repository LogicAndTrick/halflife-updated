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

#include "CGibShooter.h"
#include "CSprite.h"


class CEnvShooter : public CGibShooter
{
	void Precache() override;
	bool KeyValue(KeyValueData* pkvd) override;
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	void Spawn() override;

	static TYPEDESCRIPTION m_SaveData[];

	CBaseEntity* CreateGib(Vector vecPos, Vector vecVel) override;

	int m_iszTouch;
	int m_iszTouchOther;
	int m_iPhysics;
	float m_fFriction;
	Vector m_vecSize;
};

// Shooter particle
class CShot : public CSprite
{
public:
	void Touch(CBaseEntity* pOther) override;
};
