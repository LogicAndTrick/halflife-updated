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
#include "CFuncTankControls.h"
#include "weapons.h"

enum TANKBULLET
{
	TANK_BULLET_NONE = 0,
	TANK_BULLET_9MM = 1,
	TANK_BULLET_MP5 = 2,
	TANK_BULLET_12MM = 3,
};

#define SF_TANK_ACTIVE 0x0001
// #define SF_TANK_PLAYER			0x0002	// for internal camera function. G-Cont
// #define SF_TANK_HUMANS			0x0004	// for external camera (override internal camera). G-Cont
// #define SF_TANK_ALIENS			0x0008
#define SF_TANK_LINEOFSIGHT 0x0010
#define SF_TANK_CANCONTROL 0x0020
#define SF_TANK_LASERSPOT 0x0040   // LRC
#define SF_TANK_MATCHTARGET 0x0080 // LRC
#define SF_TANK_SOUNDON 0x8000
#define SF_TANK_SEQFIRE 0x10000 // LRC - a TankSequence is telling me to fire

constexpr Vector gTankSpread[] =
	{
		Vector(0, 0, 0),			 // perfect
		Vector(0.025, 0.025, 0.025), // small cone
		Vector(0.05, 0.05, 0.05),	 // medium cone
		Vector(0.1, 0.1, 0.1),		 // large cone
		Vector(0.25, 0.25, 0.25),	 // extra-large cone
};

#define MAX_FIRING_SPREADS ARRAYSIZE(gTankSpread)

class CTankSequence;

class CFuncTank : public CBaseEntity
{
public:
	void Spawn() override;
	void PostSpawn() override;
	void Precache() override;
	bool KeyValue(KeyValueData* pkvd) override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	void Think() override;
	void TrackTarget();
	CBaseEntity* BestVisibleEnemy();
	int IRelationship(CBaseEntity* pTarget);

	int Classify() override { return m_iTankClass; }

	void TryFire(const Vector& barrelEnd, const Vector& forward, entvars_t* pevAttacker);
	virtual void Fire(const Vector& barrelEnd, const Vector& forward, entvars_t* pevAttacker);
	virtual Vector UpdateTargetPosition(CBaseEntity* pTarget)
	{
		return pTarget->BodyTarget(pev->origin);
	}

	void StartRotSound();
	void StopRotSound();
	STATE GetState() override { return m_iActive ? STATE_ON : STATE_OFF; } // Support this stuff for watcher
	int m_iActive;

	// Bmodels don't go across transitions
	int ObjectCaps() override { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	inline bool IsActive() { return (pev->spawnflags & SF_TANK_ACTIVE) != 0; }
	inline void TankActivate()
	{
		pev->spawnflags |= SF_TANK_ACTIVE;
		SetNextThink(0.1);
		m_fireLast = 0;
	}
	inline void TankDeactivate()
	{
		pev->spawnflags &= ~SF_TANK_ACTIVE;
		m_fireLast = 0;
		StopRotSound();
	}
	inline bool CanFire() { return (gpGlobals->time - m_lastSightTime) < m_persist; }
	bool InRange(float range);

	// Acquire a target.  pPlayer is a player in the PVS
	edict_t* FindTarget(edict_t* pPlayer);

	void TankTrace(const Vector& vecStart, const Vector& vecForward, const Vector& vecSpread, TraceResult& tr);

	Vector BarrelPosition()
	{
		Vector forward, right, up;
		UTIL_MakeVectorsPrivate(pev->angles, forward, right, up);
		return pev->origin + (forward * m_barrelPos.x) + (right * m_barrelPos.y) + (up * m_barrelPos.z);
	}

	void AdjustAnglesForBarrel(Vector& angles, float distance);

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	//	bool OnControls( entvars_t *pevTest );
	bool StartControl(CBasePlayer* pController, CFuncTankControls* pControls);
	void StopControl(CFuncTankControls* pControls);
	//	void ControllerPostFrame();

	CFuncTankControls* m_pControls; // LRC - tankcontrols is used as a go-between.

	void StartSequence(CTankSequence* pSequence);
	void StopSequence();

	CTankSequence* m_pSequence;	   // LRC - if set, then this is the sequence the tank is currently performing
	CBaseEntity* m_pSequenceEnemy; // LRC - the entity that our sequence wants us to attack
	CLaserSpot* m_pSpot;		   // Laser spot entity

	// LRC - unprotected these, so that TankSequence can look at them
	float m_maxRange; // Max range to aim/track
	float m_fireLast; // Last time I fired
	float m_fireRate; // How many rounds/second

protected:
	//	CBasePlayer* m_pController;
	float m_flNextAttack;
	// LRC	Vector		m_vecControllerUsePos;

	float m_yawCenter;	  // "Center" yaw
	float m_yawRate;	  // Max turn rate to track targets
	float m_yawRange;	  // Range of turning motion (one-sided: 30 is +/- 30 degress from center)
						  // Zero is full rotation
	float m_yawTolerance; // Tolerance angle

	float m_pitchCenter;	// "Center" pitch
	float m_pitchRate;		// Max turn rate on pitch
	float m_pitchRange;		// Range of pitch motion as above
	float m_pitchTolerance; // Tolerance angle

	float m_lastSightTime; // Last time I saw target
	float m_persist;	   // Persistence of firing (how long do I shoot when I can't see)
	float m_minRange;	   // Minimum range to aim/track

	Vector m_barrelPos;	 // Length of the freakin barrel
	float m_spriteScale; // Scale of any sprites we shoot
	int m_iszSpriteSmoke;
	int m_iszSpriteFlash;
	TANKBULLET m_bulletType; // Bullet type
	int m_iBulletDamage;	 // 0 means use Bullet type's default damage

	Vector m_sightOrigin; // Last sight of target
	int m_spread;		  // firing spread
	int m_iszMaster;	  // Master entity
	int m_iszFireMaster;  // LRC - Fire-Master entity (prevents firing when inactive)

	int m_iTankClass; // Behave As

	void UpdateSpot();
	//	CLaserSpot*  m_pViewTarg;	// Player view indicator

	CPointEntity* m_pFireProxy; // LRC - locus position for custom shots
	int m_iszLocusFire;
};
