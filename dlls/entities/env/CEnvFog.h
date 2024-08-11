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

#include "entities/CPointEntity.h"

#define SF_FOG_ACTIVE 1
#define SF_FOG_FADING 0x8000

// LRC - env_fog, extended a bit from the DMC version
class CEnvFog : public CPointEntity
{
public:
	void Spawn() override;
	void Precache() override;
	void EXPORT ResumeThink();
	void EXPORT Resume2Think();
	void EXPORT TurnOn();
	void EXPORT TurnOff();
	void EXPORT FadeInDone();
	void EXPORT FadeOutDone();
	void SendData(Vector col, int fFadeTime, int StartDist, int iEndDist);
	bool KeyValue(KeyValueData* pkvd) override;
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;

	STATE GetState() override;

	int m_iStartDist;
	int m_iEndDist;
	float m_iFadeIn;
	float m_iFadeOut;
	float m_fHoldTime;
	float m_fFadeStart; // if we're fading in/out, then when did the fade start?
};
