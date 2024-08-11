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

typedef enum
{
	expRandom,
	expDirected
} Explosions;

typedef enum
{
	matGlass = 0,
	matWood,
	matMetal,
	matFlesh,
	matCinderBlock,
	matCeilingTile,
	matComputer,
	matUnbreakableGlass,
	matRocks,
	matNone,
	matLastMaterial
} Materials;

#define NUM_SHARDS 6 // this many shards spawned when breakable objects break;
#define SF_BREAKABLE_INVERT 16

// func_breakable - bmodel that breaks into pieces after taking damage
class CBreakable : public CBaseDelay
{
public:
	// basic functions
	void Spawn() override;
	void Precache() override;
	bool KeyValue(KeyValueData* pkvd) override;
	bool CalcNumber(CBaseEntity* pLocus, float* OUTresult) override;
	void EXPORT BreakTouch(CBaseEntity* pOther);
	void EXPORT BreakUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void EXPORT RespawnUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void EXPORT RespawnThink();
	void EXPORT RespawnFadeThink();
	void DamageSound();
	virtual void DoRespawn(); // AJH Fix for respawnable breakable pushables
	int Classify() override { return m_iClass; }

	// breakables use an overridden takedamage
	bool TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;
	// To spark when hit
	void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType) override;

	bool IsBreakable();
	bool SparkWhenHit();

	STATE GetState() override;

	int DamageDecal(int bitsDamageType) override;

	void EXPORT Die();
	int ObjectCaps() override { return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION); }
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	inline bool Explodable() { return ExplosionMagnitude() > 0; }
	inline int ExplosionMagnitude() { return pev->impulse; }
	inline void ExplosionSetMagnitude(int magnitude) { pev->impulse = magnitude; }

	static void MaterialSoundPrecache(Materials precacheMaterial);
	static void MaterialSoundRandom(edict_t* pEdict, Materials soundMaterial, float volume);
	static const char** MaterialSoundList(Materials precacheMaterial, int& soundCount);

	static const char* pSoundsWood[];
	static const char* pSoundsFlesh[];
	static const char* pSoundsGlass[];
	static const char* pSoundsMetal[];
	static const char* pSoundsConcrete[];
	static const char* pSpawnObjects[];

	static TYPEDESCRIPTION m_SaveData[];

	Materials m_Material;
	Explosions m_Explosion;
	int m_idShard;
	float m_angle;
	int m_iszGibModel;
	int m_iszSpawnObject;
	// LRC
	int m_iRespawnTime;
	int m_iInitialHealth;
	int m_iInitialRenderAmt;
	int m_iInitialRenderMode;
	int m_iClass;	  // so that monsters will attack it
	int m_iszWhenHit; // locus trigger
	CPointEntity* m_pHitProxy;
};
