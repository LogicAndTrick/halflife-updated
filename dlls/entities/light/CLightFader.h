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
#include "CLight.h"

class CLightFader : public CPointEntity
{
public:
	void EXPORT FadeThink();
	void EXPORT WaitThink();
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];

	CLight* m_pLight;
	char m_cFrom;
	char m_cTo;
	char m_szCurStyle[2];
	float m_fEndTime;
	int m_iszPattern;
	float m_fStep;
	int m_iWait;
};
