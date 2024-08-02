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

#define SF_PENDULUM_SWING 2 // spawnflag that makes a pendulum a rope swing.

class CPendulum : public CBaseEntity
{
public:
	void Spawn() override;
	bool KeyValue(KeyValueData* pkvd) override;
	void EXPORT SwingThink();
	void EXPORT PendulumUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void EXPORT StopThink();
	void Touch(CBaseEntity* pOther) override;
	void EXPORT RopeTouch(CBaseEntity* pOther); // this touch func makes the pendulum a rope
	int ObjectCaps() override { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	void Blocked(CBaseEntity* pOther) override;
	STATE GetState() override { return (pev->speed) ? STATE_ON : STATE_OFF; }

	static TYPEDESCRIPTION m_SaveData[];

	float m_accel;	  // Acceleration
	float m_distance; //
	float m_time;
	float m_damp;
	float m_maxSpeed;
	float m_dampSpeed;
	Vector m_center;
	Vector m_start;

	EHANDLE m_hActivator; // AJH (give frags to this entity)
};
