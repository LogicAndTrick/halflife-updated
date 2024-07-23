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

#include "CCalcFallback.h"
#include "locus.h"
#include "movewith.h"

LINK_ENTITY_TO_CLASS(calc_fallback, CCalcFallback);

inline bool TryCalcLocus_VelocityNoSwizzle(CBaseEntity* pEntity, CBaseEntity* pLocus, const char* szText, Vector* OUTresult) { return TryCalcLocus_Velocity(pEntity, pLocus, szText, OUTresult); }
inline bool TryCalcLocus_NumberNoSwizzle(CBaseEntity* pEntity, CBaseEntity* pLocus, const char* szText, float* OUTresult) { return TryCalcLocus_Number(pLocus, szText, OUTresult); }
float s_Zero = 0;

template <class R, const R* DEFAULTR>
bool CCalcFallback::CalcWithFallback(CBaseEntity* pLocus, R* OUTresult, bool (*CalcFunction)(CBaseEntity* pEntity, CBaseEntity* pLocus, const char* szText, R* OUTresult))
{
	if (CalcFunction(this, pLocus, STRING(pev->target), OUTresult))
	{
		return true;
	}
	else
	{
		if (FStringNull(pev->netname))
		{
			// return the default fallback (0 or 0,0,0).
			*OUTresult = *DEFAULTR;
		}
		else
		{
			// return the fallback value
			if (!CalcFunction(this, pLocus, STRING(pev->netname), OUTresult))
			{
				// uh-oh, the fallback itself has failed!?
				// return a failure, and don't trigger the fallback target.
				// (thus if you have fallbacks for fallbacks for fallbacks, you only get a trigger from the fallback that worked.)
				return false;
			}
		}

		if (!FStringNull(pev->message))
		{
			m_hActivator = pLocus;
			UTIL_DesiredThink(this); // do the fallback trigger at the end of this frame
		}
		return true;
	}
}

bool CCalcFallback::CalcPosition(CBaseEntity* pLocus, Vector* OUTresult)
{
	return CalcWithFallback<Vector, &g_vecZero>(pLocus, OUTresult, TryCalcLocus_Position);
}

bool CCalcFallback::CalcVelocity(CBaseEntity* pLocus, Vector* OUTresult)
{
	return CalcWithFallback<Vector, &g_vecZero>(pLocus, OUTresult, TryCalcLocus_VelocityNoSwizzle);
}

bool CCalcFallback::CalcNumber(CBaseEntity* pLocus, float* OUTresult)
{
	return CalcWithFallback<float, &s_Zero>(pLocus, OUTresult, TryCalcLocus_NumberNoSwizzle);
}

void CCalcFallback::Think()
{
	FireTargets(STRING(pev->message), m_hActivator, this, USE_TOGGLE, 0);
}
