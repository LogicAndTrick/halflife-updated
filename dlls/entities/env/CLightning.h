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

#include "CBeam.h"

class CLightning : public CBeam
{
public:
	void Spawn() override;
	void Precache() override;
	bool KeyValue(KeyValueData* pkvd) override;
	void Activate() override;

	void EXPORT StrikeThink();
	void EXPORT TripThink();
	void RandomArea();
	void RandomPoint(Vector& vecSrc);
	void Zap(const Vector& vecSrc, const Vector& vecDest);
	void EXPORT StrikeUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void EXPORT ToggleUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

	inline bool ServerSide()
	{
		if (m_life == 0 && (pev->spawnflags & SF_BEAM_RING) == 0)
			return true;
		return false;
	}

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	void BeamUpdatePoints(); // LRC
	void BeamUpdateVars();

	STATE GetState() override { return m_active ? STATE_OFF : STATE_ON; };

	bool m_active;
	int m_iszStartEntity;
	int m_iszEndEntity;
	float m_life;
	int m_boltWidth;
	int m_noiseAmplitude;
	int m_brightness;
	int m_speed;
	float m_restrike;
	int m_spriteTexture;
	int m_iszSpriteName;
	int m_frameStart;
	int m_iStartAttachment;
	int m_iEndAttachment;

	float m_radius;
};
