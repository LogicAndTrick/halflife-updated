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
#include "entities/env/CSprite.h"

extern Vector VecBModelOrigin(entvars_t* pevBModel);

#define TURRET_SHOTS 2
#define TURRET_RANGE (100 * 12)
#define TURRET_SPREAD Vector(0, 0, 0)
#define TURRET_TURNRATE 30 // angles per 0.1 second
#define TURRET_MAXWAIT 15  // seconds turret will stay active w/o a target
#define TURRET_MAXSPIN 5   // seconds turret barrel will spin w/o a target
#define TURRET_MACHINE_VOLUME 0.5

typedef enum
{
	TURRET_ANIM_NONE = 0,
	TURRET_ANIM_FIRE,
	TURRET_ANIM_SPIN,
	TURRET_ANIM_DEPLOY,
	TURRET_ANIM_RETIRE,
	TURRET_ANIM_DIE,
} TURRET_ANIM;

class CBaseTurret : public CBaseMonster
{
public:
	void Spawn() override;
	void Precache() override;
	bool KeyValue(KeyValueData* pkvd) override;
	void EXPORT TurretUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

	void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType) override;
	bool TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;
	int Classify() override;

	int BloodColor() override { return DONT_BLEED; }
	void GibMonster() override {} // UNDONE: Throw turret gibs?

	// Think functions

	void EXPORT ActiveThink();
	void EXPORT SearchThink();
	void EXPORT AutoSearchThink();
	void EXPORT TurretDeath();

	virtual void EXPORT SpinDownCall() { m_iSpin = false; }
	virtual void EXPORT SpinUpCall() { m_iSpin = true; }

	// void SpinDown();
	// float EXPORT SpinDownCall() { return SpinDown(); }

	// virtual float SpinDown() { return 0;}
	// virtual float Retire() { return 0;}

	void EXPORT Deploy();
	void EXPORT Retire();

	void EXPORT Initialize();

	virtual void Ping();
	virtual void EyeOn();
	virtual void EyeOff();

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];

	// other functions
	void SetTurretAnim(TURRET_ANIM anim);
	bool MoveTurret();
	virtual void Shoot(Vector& vecSrc, Vector& vecDirToEnemy) {}

	float m_flMaxSpin; // Max time to spin the barrel w/o a target
	bool m_iSpin;

	CSprite* m_pEyeGlow;
	int m_eyeBrightness;

	int m_iDeployHeight;
	int m_iRetractHeight;
	int m_iMinPitch;

	int m_iBaseTurnRate; // angles per second
	float m_fTurnRate;	 // actual turn rate
	int m_iOrientation;	 // 0 = floor, 1 = Ceiling
	bool m_iOn;
	virtual STATE getState()
	{
		if (m_iOn)
		{
			return STATE_ON;
		}
		else
		{
			return STATE_OFF;
		}
	}

	bool m_fBeserk;	   // Sometimes this bitch will just freak out
	bool m_iAutoStart; // true if the turret auto deploys when a target
					   // enters its range

	Vector m_vecLastSight;
	float m_flLastSight; // Last time we saw a target
	float m_flMaxWait;	 // Max time to seach w/o a target
	int m_iSearchSpeed;	 // Not Used!

	// movement
	float m_flStartYaw;
	Vector m_vecCurAngles;
	Vector m_vecGoalAngles;


	float m_flPingTime;	  // Time until the next ping, used when searching
	float m_flSpinUpTime; // Amount of time until the barrel should spin down when searching
};
