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

#include "util/mathutil.h"

float UTIL_VecToYaw(const Vector& vec)
{
	return VEC_TO_YAW(vec);
}

Vector UTIL_VecToAngles(const Vector& vec)
{
	float rgflVecOut[3];
	VEC_TO_ANGLES(vec, rgflVecOut);
	return Vector(rgflVecOut);
}

// ripped this out of the engine
float UTIL_AngleMod(float a)
{
	if (a < 0)
	{
		a = a + 360 * ((int)(a / 360) + 1);
	}
	else if (a >= 360)
	{
		a = a - 360 * ((int)(a / 360));
	}
	// a = (360.0/65536) * ((int)(a*(65536/360.0)) & 65535);
	return a;
}

float UTIL_AngleDiff(float destAngle, float srcAngle)
{
	float delta;

	delta = destAngle - srcAngle;
	if (destAngle > srcAngle)
	{
		if (delta >= 180)
			delta -= 360;
	}
	else
	{
		if (delta <= -180)
			delta += 360;
	}
	return delta;
}

// LRC - pass in a normalised axis vector and a number of degrees, and this returns the corresponding
//  angles value for an entity.
Vector UTIL_AxisRotationToAngles(const Vector& vec, float angle)
{
	Vector vecTemp = UTIL_AxisRotationToVec(vec, angle);
	float rgflVecOut[3];
	// ugh, mathsy.
	rgflVecOut[0] = asin(vecTemp.z) * (-180.0 / M_PI);
	rgflVecOut[1] = acos(vecTemp.x) * (180.0 / M_PI);
	if (vecTemp.y < 0)
		rgflVecOut[1] = -rgflVecOut[1];
	rgflVecOut[2] = 0; // for now
	return Vector(rgflVecOut);
}

// LRC - as above, but returns the position of point 1 0 0 under the given rotation
Vector UTIL_AxisRotationToVec(const Vector& vec, float angle)
{
	float rgflVecOut[3];
	float flRads = angle * (M_PI / 180.0);
	float c = cos(flRads);
	float s = sin(flRads);
	float v = vec.x * (1 - c);
	// ugh, more maths. Thank goodness for internet geometry sites...
	rgflVecOut[0] = vec.x * v + c;
	rgflVecOut[1] = vec.y * v + vec.z * s;
	rgflVecOut[2] = vec.z * v - vec.y * s;
	return Vector(rgflVecOut);
}

void UTIL_MakeVectors(const Vector& vecAngles)
{
	MAKE_VECTORS(vecAngles);
}

void UTIL_MakeAimVectors(const Vector& vecAngles)
{
	float rgflVec[3];
	vecAngles.CopyToArray(rgflVec);
	rgflVec[0] = -rgflVec[0];
	MAKE_VECTORS(rgflVec);
}

#define SWAP(a, b, temp) ((temp) = (a), (a) = (b), (b) = (temp))

void UTIL_MakeInvVectors(const Vector& vec, globalvars_t* pgv)
{
	MAKE_VECTORS(vec);

	float tmp;
	pgv->v_right = pgv->v_right * -1;

	SWAP(pgv->v_forward.y, pgv->v_right.x, tmp);
	SWAP(pgv->v_forward.z, pgv->v_up.x, tmp);
	SWAP(pgv->v_right.z, pgv->v_up.y, tmp);
}

Vector UTIL_ClampVectorToBox(const Vector& input, const Vector& clampSize)
{
	Vector sourceVector = input;

	if (sourceVector.x > clampSize.x)
		sourceVector.x -= clampSize.x;
	else if (sourceVector.x < -clampSize.x)
		sourceVector.x += clampSize.x;
	else
		sourceVector.x = 0;

	if (sourceVector.y > clampSize.y)
		sourceVector.y -= clampSize.y;
	else if (sourceVector.y < -clampSize.y)
		sourceVector.y += clampSize.y;
	else
		sourceVector.y = 0;

	if (sourceVector.z > clampSize.z)
		sourceVector.z -= clampSize.z;
	else if (sourceVector.z < -clampSize.z)
		sourceVector.z += clampSize.z;
	else
		sourceVector.z = 0;

	return sourceVector.Normalize();
}

float UTIL_Approach(float target, float value, float speed)
{
	float delta = target - value;

	if (delta > speed)
		value += speed;
	else if (delta < -speed)
		value -= speed;
	else
		value = target;

	return value;
}

float UTIL_ApproachAngle(float target, float value, float speed)
{
	target = UTIL_AngleMod(target);
	value = UTIL_AngleMod(target);

	float delta = target - value;

	// Speed is assumed to be positive
	if (speed < 0)
		speed = -speed;

	if (delta < -180)
		delta += 360;
	else if (delta > 180)
		delta -= 360;

	if (delta > speed)
		value += speed;
	else if (delta < -speed)
		value -= speed;
	else
		value = target;

	return value;
}

float UTIL_AngleDistance(float next, float cur)
{
	float delta = next - cur;

	// LRC- correct for deltas > 360
	while (delta < -180)
		delta += 360;
	while (delta > 180)
		delta -= 360;

	return delta;
}

// LRC - moved here from barney.cpp
bool UTIL_IsFacing(entvars_t* pevTest, const Vector& reference)
{
	Vector vecDir = (reference - pevTest->origin);
	vecDir.z = 0;
	vecDir = vecDir.Normalize();
	Vector forward, angle;
	angle = pevTest->v_angle;
	angle.x = 0;
	UTIL_MakeVectorsPrivate(angle, forward, NULL, NULL);
	// He's facing me, he meant it
	if (DotProduct(forward, vecDir) > 0.96) // +/- 15 degrees or so
	{
		return true;
	}
	return false;
}

float UTIL_SplineFraction(float value, float scale)
{
	value = scale * value;
	float valueSquared = value * value;

	// Nice little ease-in, ease-out spline-like curve
	return 3 * valueSquared - 2 * valueSquared * value;
}

//=========================================================
// UTIL_DotPoints - returns the dot product of a line from
// src to check and vecdir.
//=========================================================
float UTIL_DotPoints(const Vector& vecSrc, const Vector& vecCheck, const Vector& vecDir)
{
	Vector2D vec2LOS;

	vec2LOS = (vecCheck - vecSrc).Make2D();
	vec2LOS = vec2LOS.Normalize();

	return DotProduct(vec2LOS, (vecDir.Make2D()));
}

unsigned short FixedUnsigned16(float value, float scale)
{
	int output;

	output = value * scale;
	if (output < 0)
		output = 0;
	if (output > 0xFFFF)
		output = 0xFFFF;

	return (unsigned short)output;
}

short FixedSigned16(float value, float scale)
{
	int output;

	output = value * scale;

	if (output > 32767)
		output = 32767;

	if (output < -32768)
		output = -32768;

	return (short)output;
}
