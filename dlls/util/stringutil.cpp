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

#include "util/stringutil.h"

void UTIL_StringToVector(float* pVector, const char* pString)
{
	char *pstr, *pfront, tempString[128];
	int j;

	strcpy(tempString, pString);
	pstr = pfront = tempString;

	for (j = 0; j < 3; j++) // lifted from pr_edict.c
	{
		pVector[j] = atof(pfront);

		while ('\0' != *pstr && *pstr != ' ')
			pstr++;
		if ('\0' == *pstr)
			break;
		pstr++;
		pfront = pstr;
	}
	if (j < 2)
	{
		/*
		ALERT( at_error, "Bad field in entity!! %s:%s == \"%s\"\n",
			pkvd->szClassName, pkvd->szKeyName, pkvd->szValue );
		*/
		for (j = j + 1; j < 3; j++)
			pVector[j] = 0;
	}
}

// LRC - randomized vectors of the form "0 0 0 .. 1 0 0"
void UTIL_StringToRandomVector(float* pVector, const char* pString)
{
	char *pstr, *pfront, tempString[128];
	int j;
	float pAltVec[3];

	strcpy(tempString, pString);
	pstr = pfront = tempString;

	for (j = 0; j < 3; j++) // lifted from pr_edict.c
	{
		pVector[j] = atof(pfront);

		while (*pstr && *pstr != ' ')
			pstr++;
		if (!*pstr)
			break;
		pstr++;
		pfront = pstr;
	}
	if (j < 2)
	{
		/*
		ALERT( at_error, "Bad field in entity!! %s:%s == \"%s\"\n",
			pkvd->szClassName, pkvd->szKeyName, pkvd->szValue );
		*/
		for (j = j + 1; j < 3; j++)
			pVector[j] = 0;
	}
	else if (*pstr == '.')
	{
		pstr++;
		if (*pstr != '.')
			return;
		pstr++;
		if (*pstr != ' ')
			return;

		UTIL_StringToVector(pAltVec, pstr);

		pVector[0] = RANDOM_FLOAT(pVector[0], pAltVec[0]);
		pVector[1] = RANDOM_FLOAT(pVector[1], pAltVec[1]);
		pVector[2] = RANDOM_FLOAT(pVector[2], pAltVec[2]);
	}
}

void UTIL_StringToIntArray(int* pVector, int count, const char* pString)
{
	char *pstr, *pfront, tempString[128];
	int j;

	strcpy(tempString, pString);
	pstr = pfront = tempString;

	for (j = 0; j < count; j++) // lifted from pr_edict.c
	{
		pVector[j] = atoi(pfront);

		while ('\0' != *pstr && *pstr != ' ')
			pstr++;
		if ('\0' == *pstr)
			break;
		pstr++;
		pfront = pstr;
	}

	for (j++; j < count; j++)
	{
		pVector[j] = 0;
	}
}

char* UTIL_VarArgs(const char* format, ...)
{
	va_list argptr;
	static char string[1024];

	va_start(argptr, format);
	vsprintf(string, format, argptr);
	va_end(argptr);

	return string;
}

//=========================================================
// UTIL_StripToken - for redundant keynames
//=========================================================
void UTIL_StripToken(const char* pKey, char* pDest)
{
	int i = 0;

	while ('\0' != pKey[i] && pKey[i] != '#')
	{
		pDest[i] = pKey[i];
		i++;
	}
	pDest[i] = 0;
}
