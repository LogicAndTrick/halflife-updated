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

#include "CBaseMonster.h"

// Animation events
#define LEECH_AE_ATTACK 1
#define LEECH_AE_FLOP 2

// Movement constants

#define LEECH_ACCELERATE 10
#define LEECH_CHECK_DIST 45
#define LEECH_SWIM_SPEED 50
#define LEECH_SWIM_ACCEL 80
#define LEECH_SWIM_DECEL 10
#define LEECH_TURN_RATE 90
#define LEECH_SIZEX 10
#define LEECH_FRAMETIME 0.1

#define DEBUG_BEAMS 0

#if DEBUG_BEAMS
#include "entities/env/CBeam.h"
#endif

//=========================================================
// leech - basic little swimming monster
//=========================================================
class CLeech : public CBaseMonster
{
public:
	void Spawn() override;
	void Precache() override;

	void EXPORT SwimThink();
	void EXPORT DeadThink();
	void Touch(CBaseEntity* pOther) override
	{
		if (pOther->IsPlayer())
		{
			// If the client is pushing me, give me some base velocity
			if (gpGlobals->trace_ent && gpGlobals->trace_ent == edict())
			{
				pev->basevelocity = pOther->pev->velocity;
				pev->flags |= FL_BASEVELOCITY;
			}
		}
	}

	void SetObjectCollisionBox() override
	{
		pev->absmin = pev->origin + Vector(-8, -8, 0);
		pev->absmax = pev->origin + Vector(8, 8, 2);
	}

	void AttackSound();
	void AlertSound() override;
	void UpdateMotion();
	float ObstacleDistance(CBaseEntity* pTarget);
	void MakeVectors();
	void RecalculateWaterlevel();
	void SwitchLeechState();

	// Base entity functions
	void HandleAnimEvent(MonsterEvent_t* pEvent) override;
	int BloodColor() override { return DONT_BLEED; }
	void Killed(entvars_t* pevInflictor, entvars_t* pevAttacker, int iGib) override;
	void Activate() override;
	bool TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;
	int Classify() override { return CLASS_INSECT; }
	int IRelationship(CBaseEntity* pTarget) override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	static const char* pAttackSounds[];
	static const char* pAlertSounds[];

private:
	// UNDONE: Remove unused boid vars, do group behavior
	float m_flTurning;	 // is this boid turning?
	bool m_fPathBlocked; // true if there is an obstacle ahead
	float m_flAccelerate;
	float m_obstacle;
	float m_top;
	float m_bottom;
	float m_height;
	float m_waterTime;
	float m_sideTime; // Timer to randomly check clearance on sides
	float m_zTime;
	float m_stateTime;
	float m_attackSoundTime;

#if DEBUG_BEAMS
	CBeam* m_pb;
	CBeam* m_pt;
#endif
};
