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

#define N_SCALE 15
#define N_SPHERES 20

//=========================================================
// Nihilanth, final Boss monster
//=========================================================
class CNihilanth : public CBaseMonster
{
public:
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	void Spawn() override;
	void Precache() override;
	int Classify() override { return CLASS_ALIEN_MILITARY; }
	int BloodColor() override { return BLOOD_COLOR_YELLOW; }
	void Killed(entvars_t* pevAttacker, int iGib) override;
	void GibMonster() override;

	void SetObjectCollisionBox() override
	{
		pev->absmin = pev->origin + Vector(-16 * N_SCALE, -16 * N_SCALE, -48 * N_SCALE);
		pev->absmax = pev->origin + Vector(16 * N_SCALE, 16 * N_SCALE, 28 * N_SCALE);
	}

	void HandleAnimEvent(MonsterEvent_t* pEvent) override;

	void EXPORT StartupThink();
	void EXPORT HuntThink();
	void EXPORT CrashTouch(CBaseEntity* pOther);
	void EXPORT DyingThink();
	void EXPORT StartupUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void EXPORT NullThink();
	void EXPORT CommandUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

	void FloatSequence();
	void NextActivity();

	void Flight();

	bool AbsorbSphere();
	bool EmitSphere();
	void TargetSphere(USE_TYPE useType, float value);
	CBaseEntity* RandomTargetname(const char* szName);
	void ShootBalls();
	void MakeFriend(Vector vecPos);

	bool TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;
	void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType) override;

	void PainSound() override;
	void DeathSound() override;

	static const char* pAttackSounds[];	  // vocalization: play sometimes when he launches an attack
	static const char* pBallSounds[];	  // the sound of the lightening ball launch
	static const char* pShootSounds[];	  // grunting vocalization: play sometimes when he launches an attack
	static const char* pRechargeSounds[]; // vocalization: play when he recharges
	static const char* pLaughSounds[];	  // vocalization: play sometimes when hit and still has lots of health
	static const char* pPainSounds[];	  // vocalization: play sometimes when hit and has much less health and no more chargers
	static const char* pDeathSounds[];	  // vocalization: play as he dies

	// x_teleattack1.wav	the looping sound of the teleport attack ball.

	float m_flForce;

	float m_flNextPainSound;

	Vector m_velocity;
	Vector m_avelocity;

	Vector m_vecTarget;
	Vector m_posTarget;

	Vector m_vecDesired;
	Vector m_posDesired;

	float m_flMinZ;
	float m_flMaxZ;

	Vector m_vecGoal;

	float m_flLastSeen;
	float m_flPrevSeen;

	int m_irritation;

	int m_iLevel;
	int m_iTeleport;

	EHANDLE m_hRecharger;

	EHANDLE m_hSphere[N_SPHERES];
	int m_iActiveSpheres;

	float m_flAdj;

	CSprite* m_pBall;

	char m_szRechargerTarget[64];
	char m_szDrawUse[64];
	char m_szTeleportUse[64];
	char m_szTeleportTouch[64];
	char m_szDeadUse[64];
	char m_szDeadTouch[64];

	float m_flShootEnd;
	float m_flShootTime;

	EHANDLE m_hFriend[3];
};

//=========================================================
// Controller bouncy ball attack
//=========================================================
class CNihilanthHVR : public CBaseMonster
{
public:
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	void Spawn() override;
	void Precache() override;

	void CircleInit(CBaseEntity* pTarget);
	void AbsorbInit();
	void TeleportInit(CNihilanth* pOwner, CBaseEntity* pEnemy, CBaseEntity* pTarget, CBaseEntity* pTouch);
	void GreenBallInit();
	void ZapInit(CBaseEntity* pEnemy);

	void EXPORT HoverThink();
	bool CircleTarget(Vector vecTarget);
	void EXPORT DissipateThink();

	void EXPORT ZapThink();
	void EXPORT TeleportThink();
	void EXPORT TeleportTouch(CBaseEntity* pOther);

	void EXPORT RemoveTouch(CBaseEntity* pOther);
	void EXPORT BounceTouch(CBaseEntity* pOther);
	void EXPORT ZapTouch(CBaseEntity* pOther);

	CBaseEntity* RandomClassname(const char* szName);

	// void EXPORT SphereUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	void MovetoTarget(Vector vecTarget);
	virtual void Crawl();

	void Zap();
	void Teleport();

	float m_flIdealVel;
	Vector m_vecIdeal;
	CNihilanth* m_pNihilanth;
	EHANDLE m_hTouch;
	int m_nFrames;
};
