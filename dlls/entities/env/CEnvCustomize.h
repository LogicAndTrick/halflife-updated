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

#define SF_CUSTOM_AFFECTDEAD 1
#define SF_CUSTOM_ONCE 2
#define SF_CUSTOM_DEBUG 4

#define CUSTOM_FLAG_NOCHANGE 0
#define CUSTOM_FLAG_ON 1
#define CUSTOM_FLAG_OFF 2
#define CUSTOM_FLAG_TOGGLE 3
#define CUSTOM_FLAG_USETYPE 4
#define CUSTOM_FLAG_INVUSETYPE 5

// Changes various properties of an entity (some properties only apply to monsters.)
class CEnvCustomize : public CBaseEntity
{
public:
	void Spawn() override;
	void Precache() override;
	void PostSpawn() override;
	void DesiredAction() override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;

	void Affect(CBaseEntity* pTarget, USE_TYPE useType);
	int GetActionFor(int iField, int iActive, USE_TYPE useType, char* szDebug);
	void SetBoneController(float fController, int cnum, CBaseEntity* pTarget);

	int ObjectCaps() override { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	bool KeyValue(KeyValueData* pkvd) override;
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];

	float m_flRadius;
	int m_iszModel;
	int m_iClass;
	int m_iPlayerReact;
	int m_iPrisoner;
	int m_iMonsterClip;
	int m_iVisible;
	int m_iSolid;
	int m_iProvoked;
	int m_voicePitch;
	int m_iBloodColor;
	float m_fFramerate;
	float m_fController0;
	float m_fController1;
	float m_fController2;
	float m_fController3;
};
