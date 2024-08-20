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

#include "extdll.h"
class CBaseEntity;

bool TryCalcLocus_Position(CBaseEntity* pEntity, CBaseEntity* pLocus, const char* szText, Vector* OUTresult);
bool TryCalcLocus_Velocity(CBaseEntity* pEntity, CBaseEntity* pLocus, const char* szText, Vector* OUTresult, Vector* swizzleBasis = 0);
bool TryCalcLocus_PYR(CBaseEntity* pEntity, CBaseEntity* pLocus, const char* szText, Vector* OUTresult, Vector* swizzleBasis = 0); // LRC 1.8
bool TryCalcLocus_Number(CBaseEntity* pLocus, const char* szText, float* OUTresult, Vector* swizzleBasis = 0, bool isPYR = false);

inline Vector CalcLocus_Position(CBaseEntity* pEntity, CBaseEntity* pLocus, const char* szText)
{
	Vector result;
	if (TryCalcLocus_Position(pEntity, pLocus, szText, &result))
		return result;
	else
		return g_vecZero;
}
inline Vector CalcLocus_Velocity(CBaseEntity* pEntity, CBaseEntity* pLocus, const char* szText, Vector* swizzleBasis = 0)
{
	Vector result;
	if (TryCalcLocus_Velocity(pEntity, pLocus, szText, &result, swizzleBasis))
		return result;
	else
		return g_vecZero;
}
inline Vector CalcLocus_PYR(CBaseEntity* pEntity, CBaseEntity* pLocus, const char* szText, Vector* swizzleBasis = 0)
{
	Vector result;
	if (TryCalcLocus_PYR(pEntity, pLocus, szText, &result, swizzleBasis))
		return result;
	else
		return g_vecZero;
}
inline float CalcLocus_Number(CBaseEntity* pLocus, const char* szText, Vector* swizzleBasis = 0, bool isPYR = false)
{
	float result;
	if (TryCalcLocus_Number(pLocus, szText, &result, swizzleBasis, isPYR))
		return result;
	else
		return 0;
}

bool TryCalcLocus_NumberNonRandom(CBaseEntity* pLocus, const char* szText, float* OUTresult, Vector* swizzleBasis = 0, bool isPYR = false); // LRC 1.8
bool TryCalcLocus_NumberSimple(CBaseEntity* pLocus, const char* szText, float* OUTresult, Vector* swizzleBasis = 0, bool isPYR = false);	// LRC 1.8

CBaseEntity* CalcLocusParameter(CBaseEntity* pLocus, const char* szParam, Vector* swizzleBasis = 0, bool isPYR = false); // LRC 1.8
