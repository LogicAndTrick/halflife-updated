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

#include "entities/CBaseMonster.h"

// Grenades flagged with this will be triggered when the owner calls detonateSatchelCharges
#define SF_DETONATE 0x0001

// Contact Grenade / Timed grenade / Satchel Charge
class CGrenade : public CBaseMonster
{
public:
	void Spawn() override;

	typedef enum
	{
		SATCHEL_DETONATE = 0,
		SATCHEL_RELEASE
	} SATCHELCODE;

	static CGrenade* ShootTimed(entvars_t* pevOwner, Vector vecStart, Vector vecVelocity, float time);
	static CGrenade* ShootContact(entvars_t* pevOwner, Vector vecStart, Vector vecVelocity);
	static CGrenade* ShootSatchelCharge(entvars_t* pevOwner, Vector vecStart, Vector vecVelocity);
	static void UseSatchelCharges(entvars_t* pevOwner, SATCHELCODE code);

	void Explode(Vector vecSrc, Vector vecAim);
	void Explode(TraceResult* pTrace, int bitsDamageType);
	void EXPORT Smoke();

	void EXPORT BounceTouch(CBaseEntity* pOther);
	void EXPORT SlideTouch(CBaseEntity* pOther);
	void EXPORT ExplodeTouch(CBaseEntity* pOther);
	void EXPORT DangerSoundThink();
	void EXPORT PreDetonate();
	void EXPORT Detonate();
	void EXPORT DetonateUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void EXPORT TumbleThink();

	virtual void BounceSound();
	int BloodColor() override { return DONT_BLEED; }
	void Killed(entvars_t* pevInflictor, entvars_t* pevAttacker, int iGib) override;

	bool m_fRegisteredSound; // whether or not this grenade has issued its DANGER sound to the world sound list yet.
};
