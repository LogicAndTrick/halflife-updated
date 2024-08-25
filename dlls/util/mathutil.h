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
#include "enginecallback.h"

// Misc. Prototypes
float UTIL_VecToYaw(const Vector& vec);
Vector UTIL_VecToAngles(const Vector& vec);
float UTIL_AngleMod(float a);
float UTIL_AngleDiff(float destAngle, float srcAngle);

Vector UTIL_AxisRotationToAngles(const Vector& vec, float angle); // LRC
Vector UTIL_AxisRotationToVec(const Vector& vec, float angle);	 // LRC

void UTIL_MakeVectors(const Vector& vecAngles);

inline void UTIL_MakeVectorsPrivate(const Vector& vecAngles, float* p_vForward, float* p_vRight, float* p_vUp)
{
	g_engfuncs.pfnAngleVectors(vecAngles, p_vForward, p_vRight, p_vUp);
}

void UTIL_MakeAimVectors(const Vector& vecAngles); // like MakeVectors, but assumes pitch isn't inverted
void UTIL_MakeInvVectors(const Vector& vec, globalvars_t* pgv);

Vector UTIL_ClampVectorToBox(const Vector& input, const Vector& clampSize);

float UTIL_Approach(float target, float value, float speed);
float UTIL_ApproachAngle(float target, float value, float speed);
float UTIL_AngleDistance(float next, float cur);

inline float UTIL_Lerp(float lerpfactor, float A, float B) { return A + lerpfactor * (B - A); } // LRC 1.8 - long-missing convenience!
bool UTIL_IsFacing(entvars_t* pevTest, const Vector& reference);							// LRC

// Use for ease-in, ease-out style interpolation (accel/decel)
float UTIL_SplineFraction(float value, float scale);

// Sorta like FInViewCone, but for nonmonsters.
float UTIL_DotPoints(const Vector& vecSrc, const Vector& vecCheck, const Vector& vecDir);

unsigned short FixedUnsigned16(float value, float scale);
short FixedSigned16(float value, float scale);
