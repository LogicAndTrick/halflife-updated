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

#include "CActAnimating.h"

#define TREE_AE_ATTACK 1

class CXenTreeTrigger;

class CXenTree : public CActAnimating
{
public:
	void Spawn() override;
	void Precache() override;
	void Touch(CBaseEntity* pOther) override;
	void Think() override;
	bool TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override
	{
		Attack();
		return false;
	}
	void HandleAnimEvent(MonsterEvent_t* pEvent) override;
	void Attack();
	int Classify() override { return CLASS_BARNACLE; }

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	static const char* pAttackHitSounds[];
	static const char* pAttackMissSounds[];

private:
	CXenTreeTrigger* m_pTrigger;
};
