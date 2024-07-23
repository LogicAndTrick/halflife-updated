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

#include "CCalcFromCVar.h"
#include "locus.h"

LINK_ENTITY_TO_CLASS(calc_fromcvar, CCalcFromCVar);

bool CCalcFromCVar::CalcPosition(CBaseEntity* pLocus, Vector* OUTresult)
{
	return TryCalcLocus_Position(this, pLocus, CVAR_GET_STRING(STRING(pev->target)), OUTresult);
}

bool CCalcFromCVar::CalcVelocity(CBaseEntity* pLocus, Vector* OUTresult)
{
	return TryCalcLocus_Velocity(this, pLocus, CVAR_GET_STRING(STRING(pev->target)), OUTresult);
}

bool CCalcFromCVar::CalcNumber(CBaseEntity* pLocus, float* OUTresult)
{
	if (pev->spawnflags & SF_CALCFROMCVAR_PARSEASLR)
	{
		return TryCalcLocus_Number(pLocus, CVAR_GET_STRING(STRING(pev->target)), OUTresult);
	}
	else
	{
		*OUTresult = CVAR_GET_FLOAT(STRING(pev->target));
		return true;
	}
}
