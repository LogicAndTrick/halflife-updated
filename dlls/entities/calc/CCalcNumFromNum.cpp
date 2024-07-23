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

#include "CCalcNumFromNum.h"
#include "locus.h"

LINK_ENTITY_TO_CLASS(calc_numfromnum, CCalcNumFromNum); // LRC 1.8: new name for this entity
LINK_ENTITY_TO_CLASS(calc_ratio, CCalcNumFromNum);		// old name, still legal

bool CCalcNumFromNum::CalcNumber(CBaseEntity* pLocus, float* OUTresult)
{
	// ALERT(at_debug,"Calc_ratio 'ratio_to_return' is %i\n",pev->skin);
	float fBasis = CalcLocus_Number(pLocus, STRING(pev->target));
	switch (pev->impulse)
	{
	case 1: fBasis = 1 - fBasis; break;						 // reversed
	case 2: fBasis = -fBasis; break;						 // negative
	case 3: fBasis = 1 / fBasis; break;						 // reciprocal
	case 4: fBasis = fBasis * fBasis; break;				 // square
	case 5: fBasis = 1 / (fBasis * fBasis); break;			 // reciprocal
	case 6: fBasis = sqrt(fBasis); break;					 // reciprocal
	case 7: fBasis = cos(fBasis * (M_PI / 180.0f)); break;	 // cosine
	case 8: fBasis = sin(fBasis * (M_PI / 180.0f)); break;	 // sine
	case 9: fBasis = tan(fBasis * (M_PI / 180.0f)); break;	 // tangent
	case 10: fBasis = acos(fBasis) * (180.0f / M_PI); break; // inv cosine
	case 11: fBasis = asin(fBasis) * (180.0f / M_PI); break; // inv sine
	case 12: fBasis = atan(fBasis) * (180.0f / M_PI); break; // inv tan
	}

	if (!FStringNull(pev->netname))
	{
		float fOffset;
		if (!TryCalcLocus_Number(pLocus, STRING(pev->netname), &fOffset))
			return false;
		fBasis += fOffset;
	}
	if (!FStringNull(pev->message))
	{
		float fScale;
		if (!TryCalcLocus_Number(pLocus, STRING(pev->message), &fScale))
			return false;
		fBasis = fBasis * fScale;
	}

	if (!FStringNull(pev->noise))
	{
		float fMin;
		if (!TryCalcLocus_Number(pLocus, STRING(pev->noise), &fMin))
			return false;

		if (!FStringNull(pev->noise1))
		{
			float fMax;
			if (!TryCalcLocus_Number(pLocus, STRING(pev->noise1), &fMax))
				return false;

			if (fBasis >= fMin && fBasis <= fMax)
			{
				*OUTresult = fBasis;
				return true;
			}

			switch ((int)pev->frags)
			{
			case 0:
				if (fBasis < fMin)
					*OUTresult = fMin;
				else
					*OUTresult = fMax;
				return true;
			case 1:
				while (fBasis < fMin)
					fBasis += fMax - fMin;
				while (fBasis > fMax)
					fBasis -= fMax - fMin;
				*OUTresult = fBasis;
				return true;
			case 2:
				while (fBasis < fMin || fBasis > fMax)
				{
					if (fBasis < fMin)
						fBasis = fMin + fMax - fBasis;
					else
						fBasis = fMax + fMax - fBasis;
				}
				*OUTresult = fBasis;
				return true;
			case 3:
				return false;
			}
		}

		if (fBasis > fMin)
			*OUTresult = fBasis;
		else if (pev->frags != 3)
			*OUTresult = fMin; // crop to nearest value
		else
			return false;
	}
	else if (!FStringNull(pev->noise1))
	{
		float fMax;
		if (!TryCalcLocus_Number(pLocus, STRING(pev->noise1), &fMax))
			return false;

		if (fBasis < fMax)
			*OUTresult = fBasis;
		else if (pev->frags != 3)
			*OUTresult = fMax; // crop to nearest value
		else
			return false;
	}
	else
		*OUTresult = fBasis;

	return true;
}
