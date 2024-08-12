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

#include "entities/monster/CTalkMonster.h"
#include "entities/env/CBeam.h"

// For holograms, make them not solid so the player can walk through them
// LRC- this seems to interfere with SF_MONSTER_CLIP
#define SF_GENERICMONSTER_NOTSOLID 4
#define SF_HEAD_CONTROLLER 8
#define SF_GENERICMONSTER_INVULNERABLE 32
#define SF_GENERICMONSTER_PLAYERMODEL 64

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
// G-Cont. This code - support for htorch model from Op4 ;)
#define HTORCH_AE_SHOWGUN (17)
#define HTORCH_AE_SHOWTORCH (18)
#define HTORCH_AE_HIDETORCH (19)
#define HTORCH_AE_ONGAS (20)
#define HTORCH_AE_OFFGAS (21)
#define GUN_DEAGLE 0
#define GUN_TORCH 1
#define GUN_NONE 2

class CGenericMonster : public CTalkMonster
{
public:
	void Spawn() override;
	void Precache() override;
	void SetYawSpeed() override;
	int Classify() override;
	void HandleAnimEvent(MonsterEvent_t* pEvent) override;
	int ISoundMask() override;
	bool KeyValue(KeyValueData* pkvd) override;
	void Torch();
	void MakeGas();
	void UpdateGas();
	void KillGas();

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	bool HasCustomGibs() override { return m_iszGibModel; }

	CBeam* m_pBeam;
	int m_iszGibModel;
};

class CDeadGenericMonster : public CBaseMonster
{
public:
	void Spawn() override;
	void Precache() override;
	int Classify() override { return CLASS_PLAYER_ALLY; }
	bool KeyValue(KeyValueData* pkvd) override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	bool HasCustomGibs() override { return m_iszGibModel; }

	int m_iszGibModel;
};
