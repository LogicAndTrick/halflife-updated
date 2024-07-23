//=========================================
// NEW file for Spirit of Half-Life 0.7
// Created 14/01/02
//=========================================

// Spirit of Half-Life's particle system uses "locus triggers" to tell
// entities where to perform their actions.

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "locus.h"
#include "entities/locus/CLocusVariable.h"

// LRC 1.8
//  randomized vectors can be written in two ways:
//  '0 0 0 .. 1 1 1' or '0..1 0..1 0..1'.
//  the former is a lerp based on a single random choice (e.g. 0.42 0.42 0.42),
//  the latter is three random choices (e.g. 0.42 0.73 0.11).
bool TryParseVectorComponentwise(CBaseEntity* pLocus, const char* szText, Vector* OUTresult, Vector* swizzleBasis = NULL, bool isPYR = false)
{
	int nextComponentNameStart = 0;
	int nextVectorComponent = 0;
	float vecResult[6] = {0, 0, 0, 0, 0, 0};
	int inBrackets = 0;

	for (int i = 0; szText[i]; i++)
	{
		if (szText[i] == '(')
		{
			inBrackets++;
		}
		else if (szText[i] == ')')
		{
			inBrackets--;
		}
		else if (inBrackets == 0 && (szText[i] == ' ' || szText[i] == '\t' || szText[i] == ','))
		{
			// Ah, it's a vector.
			char szComponentName[128];

			strncpy(szComponentName, &szText[nextComponentNameStart], i);
			szComponentName[i - nextComponentNameStart] = 0;

			if (!strcmp(szComponentName, ".."))
			{
				if (nextVectorComponent > 3)
				{
					ALERT(at_error, "LV \"%s\" has an invalid '..' separator\n", szText);
				}
				else
				{
					nextVectorComponent = 3;
				}
			}
			else
			{
				bool Handled = false;

				if (nextVectorComponent >= 6)
				{
					ALERT(at_error, "LV \"%s\" has too many vector components\n", szText);
				}
				else
				{
					vecResult[nextVectorComponent] = CalcLocus_Number(pLocus, szComponentName, swizzleBasis, isPYR);
				}
				nextVectorComponent++;
			}

			// skip any further whitespace here
			while (szText[i + 1] == ' ' || szText[i + 1] == '\t')
				i++;

			nextComponentNameStart = i + 1;
		}
	}

	if (nextVectorComponent > 0)
	{
		if (nextVectorComponent >= 6)
		{
			ALERT(at_error, "LV \"%s\" has too many vector components\n", szText);
		}
		vecResult[nextVectorComponent] = CalcLocus_Number(pLocus, &szText[nextComponentNameStart], swizzleBasis, isPYR);

		if (nextVectorComponent >= 3)
		{
			// random lerp, but all three components use the same amount
			float lerpfactor = RANDOM_FLOAT(0, 1);
			OUTresult->x = UTIL_Lerp(lerpfactor, vecResult[0], vecResult[3]);
			OUTresult->y = UTIL_Lerp(lerpfactor, vecResult[1], vecResult[4]);
			OUTresult->z = UTIL_Lerp(lerpfactor, vecResult[2], vecResult[5]);
		}
		else
		{
			OUTresult->x = vecResult[0];
			OUTresult->y = vecResult[1];
			OUTresult->z = vecResult[2];
		}

		return true;
	}
	return false;
}

bool TryParseLocusBrackets(CBaseEntity* pLocus, const char* szText, char* OUTszPreBracket, char* OUTszPostBracket, Vector* swizzleBasis = NULL, bool isPYR = false)
{
	int numBrackets = 0;
	int bracketStartIdx;

	for (int i = 0; szText[i]; i++)
	{
		if (szText[i] == '(')
		{
			numBrackets++;
			if (numBrackets == 1)
			{
				strncpy(OUTszPreBracket, szText, i);
				OUTszPreBracket[i] = 0;

				bracketStartIdx = i + 1;
			}
		}
		else if (szText[i] == ')')
		{
			numBrackets--;
			if (numBrackets < 0)
			{
				break; // found ) with no preceeding (, obviously not valid
			}
			else if (numBrackets == 0)
			{
				strncpy(OUTszPostBracket, &szText[bracketStartIdx], i - bracketStartIdx);
				OUTszPostBracket[i - bracketStartIdx] = 0;
				return true;
			}
		}
	}
	return false;
}

bool TryParseLocusNumber(const char* szText, float* OUTnumber, Vector* swizzleBasis, bool isPYR)
{
	float factor = 1;
	if (szText[0] == '-')
	{
		factor = -1;
		szText++;
	}

	if (swizzleBasis)
	{
		if (szText[1] == 0)
		{
			// if we're swizzling a vector, handle the special "x" "y" and "z" strings
			switch (szText[0])
			{
			case 'x':
			case 'X':
				if (isPYR)
					break;

				*OUTnumber = factor * swizzleBasis->x;
				return true;

			case 'y':
			case 'Y':
				*OUTnumber = factor * swizzleBasis->y;
				return true;

			case 'z':
			case 'Z':
				if (isPYR)
					break;

				*OUTnumber = factor * swizzleBasis->z;
				return true;

			case 'p':
			case 'P':
				if (!isPYR)
					break;

				*OUTnumber = factor * swizzleBasis->x;
				return true;

			case 'r':
			case 'R':
				if (!isPYR)
					break;

				*OUTnumber = factor * swizzleBasis->z;
				return true;
			}
		}
		// also allow these useful properties
		else if (!isPYR)
		{
			if (FStrEq(szText, "PITCH"))
			{
				Vector ang = UTIL_VecToAngles(*swizzleBasis);
				*OUTnumber = factor * ang.x;
				return true;
			}
			else if (FStrEq(szText, "YAW"))
			{
				Vector ang = UTIL_VecToAngles(*swizzleBasis);
				*OUTnumber = factor * ang.y;
				return true;
			}
			else if (FStrEq(szText, "LENGTH"))
			{
				*OUTnumber = factor * swizzleBasis->Length();
				return true;
			}
		}
	}

	if (*szText >= '0' && *szText <= '9')
	{ // assume it's a float
		*OUTnumber = factor * atof(szText);
		return true;
	}

	return false;
}

CBaseEntity* CalcLocusParameter(CBaseEntity* pLocus, const char* szParamName, Vector* swizzleBasis, bool isPYR)
{
	Vector tryVectorResult;
	float tryNumberResult;

	if (TryParseVectorComponentwise(pLocus, szParamName, &tryVectorResult, swizzleBasis, isPYR))
	{
		// passing a componentwise vector as a locus; make a temporary reference point
		CMark* pMark = GetClassPtr((CMark*)NULL);
		pMark->pev->classname = MAKE_STRING("mark");
		pMark->pev->origin = tryVectorResult;
		pMark->pev->movedir = tryVectorResult;
		pMark->pev->frags = 0;
		pMark->SetNextThink(0.1f);

		return pMark;
	}
	else if (TryParseLocusNumber(szParamName, &tryNumberResult, swizzleBasis, isPYR))
	{
		// passing a literal number as a locus; make a temporary reference point
		CMark* pMark = GetClassPtr((CMark*)NULL);
		pMark->pev->classname = MAKE_STRING("mark");
		pMark->pev->origin = g_vecZero;
		pMark->pev->movedir = g_vecZero;
		pMark->pev->frags = tryNumberResult;
		pMark->SetNextThink(0.1f);

		return pMark;
	}
	else
	{
		return UTIL_FindEntityByTargetname(NULL, szParamName, pLocus);
	}
}

bool TryCalcLocus_Position(CBaseEntity* pEntity, CBaseEntity* pLocus, const char* szText, Vector* OUTresult)
{
	// blank = 0 0 0
	if (szText[0] == 0)
	{
		*OUTresult = pEntity->pev->origin;
		return true;
	}

	// LRC 1.8
	if (TryParseVectorComponentwise(pLocus, szText, OUTresult))
	{
		return true;
	}

	char szPreBracket[128];
	char szPostBracket[128];
	if (TryParseLocusBrackets(pLocus, szText, szPreBracket, szPostBracket))
	{
		pLocus = CalcLocusParameter(pLocus, szPostBracket);
		szText = szPreBracket;
	}

	CBaseEntity* pCalc = UTIL_FindEntityByTargetname(NULL, szText, pLocus);

	if (pCalc != NULL)
	{
		return pCalc->CalcPosition(pLocus, OUTresult);
	}

	ALERT(at_debug, "%s \"%s\" has bad or missing calc_position value \"%s\"\n", STRING(pEntity->pev->classname), STRING(pEntity->pev->targetname), szText);
	return false;
}

bool TryCalcLocus_Velocity(CBaseEntity* pEntity, CBaseEntity* pLocus, const char* szText, Vector* OUTresult, Vector* swizzleBasis)
{
	// blank = 0 0 0
	if (szText[0] == 0)
	{
		*OUTresult = g_vecZero;
		return true;
	}

	if (swizzleBasis && FStrEq(szText, "X Y Z"))
	{
		// optimization: parse the default swizzle nice and fast
		*OUTresult = *swizzleBasis;
		return true;
	}

	if (TryParseVectorComponentwise(pLocus, szText, OUTresult, swizzleBasis))
	{
		return true;
	}

	char szPreBracket[128];
	char szPostBracket[128];
	if (TryParseLocusBrackets(pLocus, szText, szPreBracket, szPostBracket, swizzleBasis))
	{
		pLocus = CalcLocusParameter(pLocus, szPostBracket, swizzleBasis);
		szText = szPreBracket;
	}

	CBaseEntity* pCalc = UTIL_FindEntityByTargetname(NULL, szText, pLocus);

	if (pCalc != NULL)
		return pCalc->CalcVelocity(pLocus, OUTresult);

	ALERT(at_debug, "%s \"%s\" has bad or missing LV value \"%s\"\n", STRING(pEntity->pev->classname), STRING(pEntity->pev->targetname), szText);
	return false;
}

// LRC 1.8 - for parsing the new [PYR] fields
bool TryCalcLocus_PYR(CBaseEntity* pEntity, CBaseEntity* pLocus, const char* szText, Vector* OUTresult, Vector* swizzleBasis)
{
	if (swizzleBasis && FStrEq(szText, "P Y R"))
	{
		// optimization: parse the default swizzle nice and fast
		*OUTresult = *swizzleBasis;
		return true;
	}

	if (TryParseVectorComponentwise(pLocus, szText, OUTresult, swizzleBasis, true))
	{
		return true;
	}

	CBaseEntity* pCalc = UTIL_FindEntityByTargetname(NULL, szText, pLocus);

	if (pCalc != NULL)
		return pCalc->CalcPYR(pLocus, OUTresult);

	ALERT(at_error, "%s \"%s\" has bad or missing PYR value \"%s\"\n", STRING(pEntity->pev->classname), STRING(pEntity->pev->targetname), szText);
	return false;
}

bool TryCalcLocus_Number(CBaseEntity* pLocus, const char* szText, float* OUTresult, Vector* swizzleBasis, bool isPYR)
{
	// blank = 0
	if (szText[0] == 0)
	{
		*OUTresult = 0;
		return true;
	}

	// LRC 1.8 - randomized ratios
	for (int i = 0; szText[i]; i++)
	{
		if (szText[i] == '.' && szText[i + 1] == '.')
		{
			// found a '..': it's a random value from a range
			char szComponentName[128];
			strncpy(szComponentName, szText, i);
			szComponentName[i] = 0;

			float A, B;
			bool bA = TryCalcLocus_NumberNonRandom(pLocus, szComponentName, &A, swizzleBasis, isPYR);
			bool bB = TryCalcLocus_NumberNonRandom(pLocus, &szText[i + 2], &B, swizzleBasis, isPYR);

			if (bA && bB)
				*OUTresult = RANDOM_FLOAT(A, B);
			else if (bA)
				*OUTresult = A;
			else if (bB)
				*OUTresult = B;
			else
				return false;

			return true;
		}
	}

	return TryCalcLocus_NumberNonRandom(pLocus, szText, OUTresult, swizzleBasis, isPYR);
}

bool TryCalcLocus_NumberNonRandom(CBaseEntity* pLocus, const char* szText, float* OUTresult, Vector* swizzleBasis, bool isPYR)
{
	if (TryParseLocusNumber(szText, OUTresult, swizzleBasis, isPYR))
	{
		return true;
	}

	char szPreBracket[128];
	char szPostBracket[128];
	if (TryParseLocusBrackets(pLocus, szText, szPreBracket, szPostBracket, swizzleBasis, isPYR))
	{
		pLocus = CalcLocusParameter(pLocus, szPostBracket, swizzleBasis, isPYR);
		szText = szPreBracket;
	}

	CBaseEntity* pCalc = UTIL_FindEntityByTargetname(NULL, szText, pLocus);

	if (pCalc != NULL)
		return pCalc->CalcNumber(pLocus, OUTresult);

	ALERT(at_debug, "Bad or missing [LR] value \"%s\"\n", szText);
	return false;
	//	return 0; // we need some signal for "fail". NaN, maybe?
}
