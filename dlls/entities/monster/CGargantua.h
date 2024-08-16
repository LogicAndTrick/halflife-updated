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
#include "entities/env/CBeam.h"
#include "entities/env/CSprite.h"

//=========================================================
// Gargantua Monster
//=========================================================
const float GARG_ATTACKDIST = 80.0;

// Garg animation events
#define GARG_AE_SLASH_LEFT 1
// #define GARG_AE_BEAM_ATTACK_RIGHT	2		// No longer used
#define GARG_AE_LEFT_FOOT 3
#define GARG_AE_RIGHT_FOOT 4
#define GARG_AE_STOMP 5
#define GARG_AE_BREATHE 6

// Gargantua is immune to any damage but this
#define GARG_DAMAGE (DMG_ENERGYBEAM | DMG_CRUSH | DMG_MORTAR | DMG_BLAST)
#define GARG_EYE_SPRITE_NAME "sprites/gargeye1.spr"
#define GARG_BEAM_SPRITE_NAME "sprites/xbeam3.spr"
#define GARG_BEAM_SPRITE2 "sprites/xbeam3.spr"
#define GARG_STOMP_SPRITE_NAME "sprites/gargeye1.spr"
#define GARG_STOMP_BUZZ_SOUND "weapons/mine_charge.wav"
#define GARG_FLAME_LENGTH 330
#define GARG_GIB_MODEL "models/metalplategibs.mdl"

#define ATTN_GARG (ATTN_NORM)

#define STOMP_SPRITE_COUNT 10

class CGargantua : public CBaseMonster
{
public:
	void Spawn() override;
	void Precache() override;
	void SetYawSpeed() override;
	int Classify() override;
	bool TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;
	void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType) override;
	void HandleAnimEvent(MonsterEvent_t* pEvent) override;

	bool CheckMeleeAttack1(float flDot, float flDist) override; // Swipe
	bool CheckMeleeAttack2(float flDot, float flDist) override; // Flames
	bool CheckRangeAttack1(float flDot, float flDist) override; // Stomp attack
	void SetObjectCollisionBox() override
	{
		pev->absmin = pev->origin + Vector(-80, -80, 0);
		pev->absmax = pev->origin + Vector(80, 80, 214);
	}

	Schedule_t* GetScheduleOfType(int Type) override;
	void StartTask(Task_t* pTask) override;
	void RunTask(Task_t* pTask) override;

	void PrescheduleThink() override;

	void Killed(entvars_t* pevInflictor, entvars_t* pevAttacker, int iGib) override;
	void DeathEffect();

	void EyeOff();
	void EyeOn(int level);
	void EyeUpdate();
	void Leap();
	void StompAttack();
	void FlameCreate();
	void FlameUpdate();
	void FlameControls(float angleX, float angleY);
	void FlameDestroy();
	inline bool FlameIsOn() { return m_pFlame[0] != NULL; }

	void FlameDamage(Vector vecStart, Vector vecEnd, entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType);

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	CUSTOM_SCHEDULES;

private:
	static const char* pAttackHitSounds[];
	static const char* pBeamAttackSounds[];
	static const char* pAttackMissSounds[];
	static const char* pRicSounds[];
	static const char* pFootSounds[];
	static const char* pIdleSounds[];
	static const char* pAlertSounds[];
	static const char* pPainSounds[];
	static const char* pAttackSounds[];
	static const char* pStompSounds[];
	static const char* pBreatheSounds[];

	CBaseEntity* GargantuaCheckTraceHullAttack(float flDist, int iDamage, int iDmgType);

	CSprite* m_pEyeGlow; // Glow around the eyes
	CBeam* m_pFlame[4];	 // Flame beams

	int m_eyeBrightness;   // Brightness target
	float m_seeTime;	   // Time to attack (when I see the enemy, I set this)
	float m_flameTime;	   // Time of next flame attack
	float m_painSoundTime; // Time of next pain sound
	float m_streakTime;	   // streak timer (don't send too many)
	float m_flameX;		   // Flame thrower aim
	float m_flameY;
};

class CSmoker : public CBaseEntity
{
public:
	void Spawn() override;
	void Think() override;
};

// Spiral Effect
class CSpiral : public CBaseEntity
{
public:
	void Spawn() override;
	void Think() override;
	int ObjectCaps() override { return FCAP_DONT_SAVE; }
	static CSpiral* Create(const Vector& origin, float height, float radius, float duration);
};

class CStomp : public CBaseEntity
{
public:
	void Spawn() override;
	void Think() override;
	static CStomp* StompCreate(const Vector& origin, const Vector& end, float speed);

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	float m_flLastThinkTime;

private:
	// UNDONE: re-use this sprite list instead of creating new ones all the time
	//	CSprite		*m_pSprites[ STOMP_SPRITE_COUNT ];
};
