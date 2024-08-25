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

#include "util/effects.h"
#include "util/sound.h"
#include "util/mathutil.h"
#include "shake.h"
#include "enginecallback.h"
#include "util/trace.h"
#include "UserMessages.h"
#include "weapons.h"
#include "classes/gamerules/CGameRules.h"
#include "entities/CBaseEntity.h"

void UTIL_ParticleEffect(const Vector& vecOrigin, const Vector& vecDirection, unsigned int ulColor, unsigned int ulCount)
{
	PARTICLE_EFFECT(vecOrigin, vecDirection, (float)ulColor, (float)ulCount);
}

// Shake the screen of all clients within radius
// radius == 0, shake all clients
// UNDONE: Allow caller to shake clients not ONGROUND?
// UNDONE: Fix falloff model (disabled)?
// UNDONE: Affect user controls?
// LRC UNDONE: Work during trigger_camera?
void UTIL_ScreenShake(const Vector& center, float amplitude, float frequency, float duration, float radius)
{
	int i;
	float localAmplitude;
	ScreenShake shake;

	shake.duration = FixedUnsigned16(duration, 1 << 12);  // 4.12 fixed
	shake.frequency = FixedUnsigned16(frequency, 1 << 8); // 8.8 fixed

	for (i = 1; i <= gpGlobals->maxClients; i++)
	{
		CBaseEntity* pPlayer = UTIL_PlayerByIndex(i);

		if (!pPlayer || (pPlayer->pev->flags & FL_ONGROUND) == 0) // Don't shake if not onground
			continue;

		localAmplitude = 0;

		if (radius <= 0)
			localAmplitude = amplitude;
		else
		{
			Vector delta = center - pPlayer->pev->origin;
			float distance = delta.Length();

			// Had to get rid of this falloff - it didn't work well
			if (distance < radius)
				localAmplitude = amplitude; // radius - distance;
		}
		if (0 != localAmplitude)
		{
			shake.amplitude = FixedUnsigned16(localAmplitude, 1 << 12); // 4.12 fixed

			MESSAGE_BEGIN(MSG_ONE, gmsgShake, NULL, pPlayer->edict()); // use the magic #1 for "one client"

			WRITE_SHORT(shake.amplitude); // shake amount
			WRITE_SHORT(shake.duration);  // shake lasts this long
			WRITE_SHORT(shake.frequency); // shake noise frequency

			MESSAGE_END();
		}
	}
}

void UTIL_ScreenShakeAll(const Vector& center, float amplitude, float frequency, float duration)
{
	UTIL_ScreenShake(center, amplitude, frequency, duration, 0);
}

void UTIL_ScreenFadeBuild(ScreenFade& fade, const Vector& color, float fadeTime, float fadeHold, int alpha, int flags)
{
	fade.duration = FixedUnsigned16(fadeTime, 1 << 12); // 4.12 fixed
	fade.holdTime = FixedUnsigned16(fadeHold, 1 << 12); // 4.12 fixed
	fade.r = (int)color.x;
	fade.g = (int)color.y;
	fade.b = (int)color.z;
	fade.a = alpha;
	fade.fadeFlags = flags;
}

void UTIL_ScreenFadeWrite(const ScreenFade& fade, CBaseEntity* pEntity)
{
	if (!pEntity || !pEntity->IsNetClient())
		return;

	MESSAGE_BEGIN(MSG_ONE, gmsgFade, NULL, pEntity->edict()); // use the magic #1 for "one client"

	WRITE_SHORT(fade.duration);	 // fade lasts this long
	WRITE_SHORT(fade.holdTime);	 // fade lasts this long
	WRITE_SHORT(fade.fadeFlags); // fade type (in / out)
	WRITE_BYTE(fade.r);			 // fade red
	WRITE_BYTE(fade.g);			 // fade green
	WRITE_BYTE(fade.b);			 // fade blue
	WRITE_BYTE(fade.a);			 // fade blue

	MESSAGE_END();
}

void UTIL_ScreenFadeAll(const Vector& color, float fadeTime, float holdTime, int alpha, int flags)
{
	int i;
	ScreenFade fade;

	UTIL_ScreenFadeBuild(fade, color, fadeTime, holdTime, alpha, flags);

	for (i = 1; i <= gpGlobals->maxClients; i++)
	{
		CBaseEntity* pPlayer = UTIL_PlayerByIndex(i);
		UTIL_ScreenFadeWrite(fade, pPlayer);
	}
}

void UTIL_ScreenFade(CBaseEntity* pEntity, const Vector& color, float fadeTime, float fadeHold, int alpha, int flags)
{
	ScreenFade fade;

	UTIL_ScreenFadeBuild(fade, color, fadeTime, fadeHold, alpha, flags);
	UTIL_ScreenFadeWrite(fade, pEntity);
}

void UTIL_BloodStream(const Vector& origin, const Vector& direction, int color, int amount)
{
	if (!UTIL_ShouldShowBlood(color))
		return;

	if (g_Language == LANGUAGE_GERMAN && color == BLOOD_COLOR_RED)
		color = 0;


	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, origin);
	WRITE_BYTE(TE_BLOODSTREAM);
	WRITE_COORD(origin.x);
	WRITE_COORD(origin.y);
	WRITE_COORD(origin.z);
	WRITE_COORD(direction.x);
	WRITE_COORD(direction.y);
	WRITE_COORD(direction.z);
	WRITE_BYTE(color);
	WRITE_BYTE(V_min(amount, 255));
	MESSAGE_END();
}

void UTIL_BloodDrips(const Vector& origin, const Vector& direction, int color, int amount)
{
	if (!UTIL_ShouldShowBlood(color))
		return;

	if (color == DONT_BLEED || amount == 0)
		return;

	if (g_Language == LANGUAGE_GERMAN && color == BLOOD_COLOR_RED)
		color = 0;

	if (g_pGameRules->IsMultiplayer())
	{
		// scale up blood effect in multiplayer for better visibility
		amount *= 2;
	}

	if (amount > 255)
		amount = 255;

	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, origin);
	WRITE_BYTE(TE_BLOODSPRITE);
	WRITE_COORD(origin.x); // pos
	WRITE_COORD(origin.y);
	WRITE_COORD(origin.z);
	WRITE_SHORT(g_sModelIndexBloodSpray);		  // initial sprite model
	WRITE_SHORT(g_sModelIndexBloodDrop);		  // droplet sprite models
	WRITE_BYTE(color);							  // color index into host_basepal
	WRITE_BYTE(V_min(V_max(3, amount / 10), 16)); // size
	MESSAGE_END();
}

Vector UTIL_RandomBloodVector()
{
	Vector direction;

	direction.x = RANDOM_FLOAT(-1, 1);
	direction.y = RANDOM_FLOAT(-1, 1);
	direction.z = RANDOM_FLOAT(0, 1);

	return direction;
}

bool UTIL_ShouldShowBlood(int color)
{
	if (color != DONT_BLEED)
	{
		if (color == BLOOD_COLOR_RED)
		{
			if (CVAR_GET_FLOAT("violence_hblood") != 0)
				return true;
		}
		else
		{
			if (CVAR_GET_FLOAT("violence_ablood") != 0)
				return true;
		}
	}
	return false;
}

void UTIL_Bubbles(Vector mins, Vector maxs, int count)
{
	Vector mid = (mins + maxs) * 0.5;

	float flHeight = UTIL_WaterLevel(mid, mid.z, mid.z + 1024);
	flHeight = flHeight - mins.z;

	MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, mid);
	WRITE_BYTE(TE_BUBBLES);
	WRITE_COORD(mins.x); // mins
	WRITE_COORD(mins.y);
	WRITE_COORD(mins.z);
	WRITE_COORD(maxs.x); // maxz
	WRITE_COORD(maxs.y);
	WRITE_COORD(maxs.z);
	WRITE_COORD(flHeight); // height
	WRITE_SHORT(g_sModelIndexBubbles);
	WRITE_BYTE(count); // count
	WRITE_COORD(8);	   // speed
	MESSAGE_END();
}

void UTIL_BubbleTrail(Vector from, Vector to, int count)
{
	float flHeight = UTIL_WaterLevel(from, from.z, from.z + 256);
	flHeight = flHeight - from.z;

	if (flHeight < 8)
	{
		flHeight = UTIL_WaterLevel(to, to.z, to.z + 256);
		flHeight = flHeight - to.z;
		if (flHeight < 8)
			return;

		// UNDONE: do a ploink sound
		flHeight = flHeight + to.z - from.z;
	}

	if (count > 255)
		count = 255;

	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(TE_BUBBLETRAIL);
	WRITE_COORD(from.x); // mins
	WRITE_COORD(from.y);
	WRITE_COORD(from.z);
	WRITE_COORD(to.x); // maxz
	WRITE_COORD(to.y);
	WRITE_COORD(to.z);
	WRITE_COORD(flHeight); // height
	WRITE_SHORT(g_sModelIndexBubbles);
	WRITE_BYTE(count); // count
	WRITE_COORD(8);	   // speed
	MESSAGE_END();
}

void UTIL_Sparks(const Vector& position)
{
	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, position);
	WRITE_BYTE(TE_SPARKS);
	WRITE_COORD(position.x);
	WRITE_COORD(position.y);
	WRITE_COORD(position.z);
	MESSAGE_END();
}

void UTIL_Ricochet(const Vector& position, float scale)
{
	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, position);
	WRITE_BYTE(TE_ARMOR_RICOCHET);
	WRITE_COORD(position.x);
	WRITE_COORD(position.y);
	WRITE_COORD(position.z);
	WRITE_BYTE((int)(scale * 10));
	MESSAGE_END();
}
