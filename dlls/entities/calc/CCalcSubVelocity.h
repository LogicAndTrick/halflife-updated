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

#define SF_CALCVELOCITY_NORMALIZE 1
#define SF_CALCVELOCITY_SWAPZ 2		// MJB this should more correctly be called 'invertZ', but never mind.
#define SF_CALCVELOCITY_DISCARDX 4	// MJB Set to 0 / ignore X-value (good for 'planar-normalised' vectors)
#define SF_CALCVELOCITY_DISCARDY 8	// MJB You see, the line will never change pitch - its (MATHEMATICAL) locus is a plane (look up locus in 3-unit maths context ;).
#define SF_CALCVELOCITY_DISCARDZ 16 // MJB Set Z value to 0 / ignore.
/*#define SF_CALCVELOCITY_SWAPXY 4 // MJB axis swapping (pitch and yaw)
#define SF_CALCVELOCITY_SWAPYZ 8 // MJB axis swapping (yaw and roll)
#define SF_CALCVELOCITY_SWAPXZ 16 // MJB axis swapping (pitch and roll)
#define SF_CALCVELOCITY_DEBUGSWAP 32 // So what the hell is the swapping DOING?*/


class CCalcSubVelocity : public CPointEntity
{
	bool Convert(CBaseEntity* pLocus, Vector vecVel, Vector* OUTresult);
	bool ConvertAngles(CBaseEntity* pLocus, Vector vecAngles, Vector* OUTresult);

public:
	bool CalcVelocity(CBaseEntity* pLocus, Vector* OUTresult) override;
};
