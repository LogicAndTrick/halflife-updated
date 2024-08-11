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
#include "CSprite.h"

// LRC - smoother lasers
#define SF_LASER_INTERPOLATE 0x0400

class CLaser : public CBeam
{
public:
	void Spawn() override;
	void PostSpawn() override;
	void Precache() override;
	bool KeyValue(KeyValueData* pkvd) override;

	void TurnOn();
	void TurnOff();
	STATE GetState() override { return (pev->effects & EF_NODRAW) ? STATE_OFF : STATE_ON; };

	void FireAtPoint(Vector startpos, TraceResult& point);

	void EXPORT StrikeThink();
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	EHANDLE m_hActivator; // AJH allow *locus start/end positions

	CSprite* m_pStartSprite;
	CSprite* m_pEndSprite;
	int m_iszStartSpriteName;
	int m_iszEndSpriteName;
	Vector m_firePosition;
	int m_iProjection;
	int m_iStoppedBy;
	int m_iszStartPosition;
	int m_iTowardsMode;
};
