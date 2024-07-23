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

#define SF_CALCFROMCVAR_PARSEASLR 1

// LRC 1.8 - entity that reads values from cvars, mostly intended for testing (so you can change values at the console)
class CCalcFromCVar : public CPointEntity
{
public:
	bool CalcPosition(CBaseEntity* pLocus, Vector* OUTresult) override;
	bool CalcVelocity(CBaseEntity* pLocus, Vector* OUTresult) override;
	bool CalcNumber(CBaseEntity* pLocus, float* OUTresult) override;
};
