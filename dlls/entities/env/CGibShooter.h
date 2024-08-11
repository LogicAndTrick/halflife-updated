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

#define SF_GIBSHOOTER_REPEATABLE 1 // allows a gibshooter to be refired
#define SF_GIBSHOOTER_DEBUG 4	   // LRC

class CGibShooter : public CBaseDelay
{
public:
	void Spawn() override;
	void Precache() override;
	bool KeyValue(KeyValueData* pkvd) override;
	void EXPORT ShootThink();
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;

	virtual CBaseEntity* CreateGib(Vector vecPos, Vector vecVel);

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	int m_iGibs;
	int m_iGibCapacity;
	int m_iGibMaterial;
	int m_iGibModelIndex;
	//	float m_flGibVelocity;
	float m_flVariance;
	float m_flGibLife;
	int m_iszTargetname;
	int m_iszPosition;
	int m_iszVelocity;
	int m_iszVelFactor;
	int m_iszSpawnTarget;
	int m_iBloodColor;
};
