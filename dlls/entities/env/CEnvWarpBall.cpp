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

#include "CEnvWarpBall.h"
#include "CBeam.h"
#include "CSprite.h"
#include "util/trace.h"

LINK_ENTITY_TO_CLASS(env_warpball, CEnvWarpBall);

void CEnvWarpBall::Precache()
{
	PRECACHE_MODEL("sprites/lgtning.spr");
	PRECACHE_MODEL("sprites/Fexplo1.spr");
	PRECACHE_MODEL("sprites/XFlare1.spr");
	PRECACHE_SOUND("debris/beamstart2.wav");
	PRECACHE_SOUND("debris/beamstart7.wav");
}

void CEnvWarpBall::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	int iTimes = 0;
	int iDrawn = 0;
	TraceResult tr;
	Vector vecDest;
	CBeam* pBeam;
	while (iDrawn < pev->frags && iTimes < (pev->frags * 3)) // try to draw <frags> beams, but give up after 3x<frags> tries.
	{
		vecDest = pev->health * (Vector(RANDOM_FLOAT(-1, 1), RANDOM_FLOAT(-1, 1), RANDOM_FLOAT(-1, 1)).Normalize());
		UTIL_TraceLine(pev->origin, pev->origin + vecDest, ignore_monsters, NULL, &tr);
		if (tr.flFraction != 1.0)
		{
			// we hit something.
			iDrawn++;
			pBeam = CBeam::BeamCreate("sprites/lgtning.spr", 200);
			pBeam->PointsInit(pev->origin, tr.vecEndPos);
			pBeam->SetColor(197, 243, 169);
			pBeam->SetNoise(65);
			pBeam->SetBrightness(150);
			pBeam->SetWidth(18);
			pBeam->SetScrollRate(35);
			pBeam->SetThink(&CEnvWarpBall::SUB_Remove);
			pBeam->SetNextThink(1);
		}
		iTimes++;
	}
	EMIT_SOUND(edict(), CHAN_BODY, "debris/beamstart2.wav", 1, ATTN_NORM);

	CSprite* pSpr = CSprite::SpriteCreate("sprites/Fexplo1.spr", pev->origin, true);
	pSpr->AnimateAndDie(10);
	pSpr->SetTransparency(kRenderGlow, 77, 210, 130, 255, kRenderFxNoDissipation);

	pSpr = CSprite::SpriteCreate("sprites/XFlare1.spr", pev->origin, true);
	pSpr->AnimateAndDie(10);
	pSpr->SetTransparency(kRenderGlow, 184, 250, 214, 255, kRenderFxNoDissipation);

	SetNextThink(0.5);
}

void CEnvWarpBall::Think()
{
	EMIT_SOUND(edict(), CHAN_ITEM, "debris/beamstart7.wav", 1, ATTN_NORM);
	SUB_UseTargets(this, USE_TOGGLE, 0);
}
