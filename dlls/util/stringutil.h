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
#include "util.h"

// Misc useful
inline bool FStrEq(const char* sz1, const char* sz2)
{
	return (strcmp(sz1, sz2) == 0);
}

inline bool FClassnameIs(edict_t* pent, const char* szClassname)
{
	return FStrEq(STRING(VARS(pent)->classname), szClassname);
}

inline bool FClassnameIs(entvars_t* pev, const char* szClassname)
{
	return FStrEq(STRING(pev->classname), szClassname);
}

void UTIL_StringToVector(float* pVector, const char* pString);
void UTIL_StringToRandomVector(float* pVector, const char* pString); // LRC
void UTIL_StringToIntArray(int* pVector, int count, const char* pString);

char* UTIL_VarArgs(const char* format, ...);

extern void UTIL_StripToken(const char* pKey, char* pDest); // for redundant keynames
