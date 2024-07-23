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

// LRC 1.8 - entity that deals with our new concept of "failure" in a calc entity
class CCalcFallback : public CPointEntity
{
public:
	EHANDLE m_hActivator; // don't need to save this, we only keep it for one frame

	void Think() override;

	bool CalcPosition(CBaseEntity* pLocus, Vector* OUTresult) override;
	bool CalcVelocity(CBaseEntity* pLocus, Vector* OUTresult) override;
	bool CalcNumber(CBaseEntity* pLocus, float* OUTresult) override;

	template <class R, const R* DEFAULTR>
	bool CalcWithFallback(CBaseEntity* pLocus, R* OUTresult, bool (*CalcFunction)(CBaseEntity* pEntity, CBaseEntity* pLocus, const char* szText, R* OUTresult));
};
