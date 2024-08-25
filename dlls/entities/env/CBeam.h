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

#include "entities/CBaseEntity.h"
#include "customentity.h"

#define SF_BEAM_STARTON 0x0001
#define SF_BEAM_TOGGLE 0x0002
#define SF_BEAM_RANDOM 0x0004
#define SF_BEAM_RING 0x0008
#define SF_BEAM_SPARKSTART 0x0010
#define SF_BEAM_SPARKEND 0x0020
#define SF_BEAM_DECALS 0x0040
#define SF_BEAM_SHADEIN 0x0080
#define SF_BEAM_SHADEOUT 0x0100
#define SF_BEAM_SOLID 0x0200
#define SF_BEAM_TEMPORARY 0x8000
// LRC - tripbeams
#define SF_BEAM_TRIPPED 0x80000

class CBeam : public CBaseEntity
{
public:
	void Spawn() override;
	void Precache() override;
	int ObjectCaps() override
	{
		int flags = 0;
		if (pev->spawnflags & SF_BEAM_TEMPORARY)
			flags = FCAP_DONT_SAVE;
		return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | flags;
	}

	void EXPORT TriggerTouch(CBaseEntity* pOther);

	// These functions are here to show the way beams are encoded as entities.
	// Encoding beams as entities simplifies their management in the client/server architecture
	inline void SetType(int type) { pev->rendermode = (pev->rendermode & 0xF0) | (type & 0x0F); }
	inline void SetFlags(int flags) { pev->rendermode = (pev->rendermode & 0x0F) | (flags & 0xF0); }
	inline void SetStartPos(const Vector& pos) { pev->origin = pos; }
	inline void SetEndPos(const Vector& pos) { pev->angles = pos; }
	void SetStartEntity(int entityIndex);
	void SetEndEntity(int entityIndex);

	inline void SetStartAttachment(int attachment) { pev->sequence = (pev->sequence & 0x0FFF) | ((attachment & 0xF) << 12); }
	inline void SetEndAttachment(int attachment) { pev->skin = (pev->skin & 0x0FFF) | ((attachment & 0xF) << 12); }

	inline void SetTexture(int spriteIndex) { pev->modelindex = spriteIndex; }
	inline void SetWidth(int width) { pev->scale = width; }
	inline void SetNoise(int amplitude) { pev->body = amplitude; }
	inline void SetColor(int r, int g, int b)
	{
		pev->rendercolor.x = r;
		pev->rendercolor.y = g;
		pev->rendercolor.z = b;
	}
	inline void SetBrightness(int brightness) { pev->renderamt = brightness; }
	inline void SetFrame(float frame) { pev->frame = frame; }
	inline void SetScrollRate(int speed) { pev->animtime = speed; }

	inline int GetType() { return pev->rendermode & 0x0F; }
	inline int GetFlags() { return pev->rendermode & 0xF0; }
	inline int GetStartEntity() { return pev->sequence & 0xFFF; }
	inline int GetEndEntity() { return pev->skin & 0xFFF; }

	const Vector& GetStartPos();
	const Vector& GetEndPos();

	Vector Center() override { return (GetStartPos() + GetEndPos()) * 0.5; } // center point of beam

	inline int GetTexture() { return pev->modelindex; }
	inline int GetWidth() { return pev->scale; }
	inline int GetNoise() { return pev->body; }
	// inline void GetColor( int r, int g, int b ) { pev->rendercolor.x = r; pev->rendercolor.y = g; pev->rendercolor.z = b; }
	inline int GetBrightness() { return pev->renderamt; }
	inline int GetFrame() { return pev->frame; }
	inline int GetScrollRate() { return pev->animtime; }

	CBaseEntity* GetTripEntity(TraceResult* ptr); // LRC

	// Call after you change start/end positions
	void RelinkBeam();
	//	void		SetObjectCollisionBox();

	void DoSparks(const Vector& start, const Vector& end);
	CBaseEntity* RandomTargetname(const char* szName);
	void BeamDamage(TraceResult* ptr);
	// Init after BeamCreate()
	void BeamInit(const char* pSpriteName, int width);
	void PointsInit(const Vector& start, const Vector& end);
	void PointEntInit(const Vector& start, int endIndex);
	void EntsInit(int startIndex, int endIndex);
	void HoseInit(const Vector& start, const Vector& direction);

	static CBeam* BeamCreate(const char* pSpriteName, int width);

	inline void LiveForTime(float time)
	{
		SetThink(&CBeam::SUB_Remove);
		SetNextThink(time);
	}
	inline void BeamDamageInstant(TraceResult* ptr, float damage)
	{
		pev->dmg = damage;
		pev->dmgtime = gpGlobals->time - 1;
		BeamDamage(ptr);
	}
};
