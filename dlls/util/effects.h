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

void UTIL_ParticleEffect(const Vector& vecOrigin, const Vector& vecDirection, unsigned int ulColor, unsigned int ulCount);

void UTIL_ScreenShake(const Vector& center, float amplitude, float frequency, float duration, float radius);
void UTIL_ScreenShakeAll(const Vector& center, float amplitude, float frequency, float duration);

void UTIL_ScreenFadeAll(const Vector& color, float fadeTime, float holdTime, int alpha, int flags);
void UTIL_ScreenFade(CBaseEntity* pEntity, const Vector& color, float fadeTime, float fadeHold, int alpha, int flags);

void UTIL_BloodStream(const Vector& origin, const Vector& direction, int color, int amount);
void UTIL_BloodDrips(const Vector& origin, const Vector& direction, int color, int amount);
Vector UTIL_RandomBloodVector();
bool UTIL_ShouldShowBlood(int bloodColor);

void UTIL_Bubbles(Vector mins, Vector maxs, int count);
void UTIL_BubbleTrail(Vector from, Vector to, int count);

void UTIL_Sparks(const Vector& position);
void UTIL_Ricochet(const Vector& position, float scale);
