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

#define ROACH_IDLE 0
#define ROACH_BORED 1
#define ROACH_SCARED_BY_ENT 2
#define ROACH_SCARED_BY_LIGHT 3
#define ROACH_SMELL_FOOD 4
#define ROACH_EAT 5

//=========================================================
// Monster's Anim Events Go Here
//=========================================================

/**
 * cockroach
 */
class CRoach : public CBaseMonster
{
public:
	void Spawn() override;
	void Precache() override;
	void SetYawSpeed() override;
	void EXPORT MonsterThink() override;
	void Move(float flInterval) override;
	void PickNewDest(int iCondition);
	void EXPORT Touch(CBaseEntity* pOther) override;
	void Killed(entvars_t* pevInflictor, entvars_t* pevAttacker, int iGib) override;

	float m_flLastLightLevel;
	float m_flNextSmellTime;
	int Classify() override;
	void Look(int iDistance) override;
	int ISoundMask() override;

	// UNDONE: These don't necessarily need to be save/restored, but if we add more data, it may
	bool m_fLightHacked;
	int m_iMode;
	// -----------------------------
};
