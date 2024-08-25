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

#include "CBeam.h"
#include "customentity.h"
#include "weapons.h"
#include "decals.h"
#include "util/decal.h"
#include "util/effects.h"
#include "util/trace.h"

LINK_ENTITY_TO_CLASS(beam, CBeam);

void CBeam::Spawn()
{
	pev->solid = SOLID_NOT; // Remove model & collisions
	Precache();
}

void CBeam::Precache()
{
	if (pev->owner)
		SetStartEntity(ENTINDEX(pev->owner));
	if (pev->aiment)
		SetEndEntity(ENTINDEX(pev->aiment));
}

void CBeam::SetStartEntity(int entityIndex)
{
	pev->sequence = (entityIndex & 0x0FFF) | (pev->sequence & 0xF000);
	pev->owner = g_engfuncs.pfnPEntityOfEntIndex(entityIndex);
}

void CBeam::SetEndEntity(int entityIndex)
{
	pev->skin = (entityIndex & 0x0FFF) | (pev->skin & 0xF000);
	pev->aiment = g_engfuncs.pfnPEntityOfEntIndex(entityIndex);
}

// These don't take attachments into account
const Vector& CBeam::GetStartPos()
{
	if (GetType() == BEAM_ENTS)
	{
		edict_t* pent = g_engfuncs.pfnPEntityOfEntIndex(GetStartEntity());
		return pent->v.origin;
	}
	return pev->origin;
}

const Vector& CBeam::GetEndPos()
{
	int type = GetType();
	if (type == BEAM_POINTS || type == BEAM_HOSE)
	{
		return pev->angles;
	}

	edict_t* pent = g_engfuncs.pfnPEntityOfEntIndex(GetEndEntity());
	if (pent)
		return pent->v.origin;
	return pev->angles;
}

CBeam* CBeam::BeamCreate(const char* pSpriteName, int width)
{
	// Create a new entity with CBeam private data
	CBeam* pBeam = GetClassPtr((CBeam*)NULL);
	pBeam->pev->classname = MAKE_STRING("beam");

	pBeam->BeamInit(pSpriteName, width);

	return pBeam;
}

void CBeam::BeamInit(const char* pSpriteName, int width)
{
	pev->flags |= FL_CUSTOMENTITY;
	SetColor(255, 255, 255);
	SetBrightness(255);
	SetNoise(0);
	SetFrame(0);
	SetScrollRate(0);
	pev->model = MAKE_STRING(pSpriteName);
	SetTexture(PRECACHE_MODEL((char*)pSpriteName));
	SetWidth(width);
	pev->skin = 0;
	pev->sequence = 0;
	pev->rendermode = 0;
}

void CBeam::PointsInit(const Vector& start, const Vector& end)
{
	SetType(BEAM_POINTS);
	SetStartPos(start);
	SetEndPos(end);
	SetStartAttachment(0);
	SetEndAttachment(0);
	RelinkBeam();
}

void CBeam::HoseInit(const Vector& start, const Vector& direction)
{
	SetType(BEAM_HOSE);
	SetStartPos(start);
	SetEndPos(direction);
	SetStartAttachment(0);
	SetEndAttachment(0);
	RelinkBeam();
}

void CBeam::PointEntInit(const Vector& start, int endIndex)
{
	SetType(BEAM_ENTPOINT);
	SetStartPos(start);
	SetEndEntity(endIndex);
	SetStartAttachment(0);
	SetEndAttachment(0);
	RelinkBeam();
}

void CBeam::EntsInit(int startIndex, int endIndex)
{
	SetType(BEAM_ENTS);
	SetStartEntity(startIndex);
	SetEndEntity(endIndex);
	SetStartAttachment(0);
	SetEndAttachment(0);
	RelinkBeam();
}

void CBeam::RelinkBeam()
{
	const Vector &startPos = GetStartPos(), &endPos = GetEndPos();

	pev->mins.x = V_min(startPos.x, endPos.x);
	pev->mins.y = V_min(startPos.y, endPos.y);
	pev->mins.z = V_min(startPos.z, endPos.z);
	pev->maxs.x = V_max(startPos.x, endPos.x);
	pev->maxs.y = V_max(startPos.y, endPos.y);
	pev->maxs.z = V_max(startPos.z, endPos.z);
	pev->mins = pev->mins - pev->origin;
	pev->maxs = pev->maxs - pev->origin;

	UTIL_SetSize(pev, pev->mins, pev->maxs);
	UTIL_SetOrigin(this, pev->origin);
}

void CBeam::TriggerTouch(CBaseEntity* pOther)
{
	if ((pOther->pev->flags & (FL_CLIENT | FL_MONSTER)) != 0)
	{
		if (pev->owner)
		{
			CBaseEntity* pOwner = CBaseEntity::Instance(pev->owner);
			pOwner->Use(pOther, this, USE_TOGGLE, 0);
		}
		ALERT(at_debug, "Firing targets!!!\n");
	}
}

CBaseEntity* CBeam::RandomTargetname(const char* szName)
{
	int total = 0;

	CBaseEntity* pEntity = NULL;
	CBaseEntity* pNewEntity = NULL;
	while ((pNewEntity = UTIL_FindEntityByTargetname(pNewEntity, szName)) != NULL)
	{
		total++;
		if (RANDOM_LONG(0, total - 1) < 1)
			pEntity = pNewEntity;
	}
	return pEntity;
}

void CBeam::DoSparks(const Vector& start, const Vector& end)
{
	if ((pev->spawnflags & (SF_BEAM_SPARKSTART | SF_BEAM_SPARKEND)) != 0)
	{
		if ((pev->spawnflags & SF_BEAM_SPARKSTART) != 0)
		{
			UTIL_Sparks(start);
		}
		if ((pev->spawnflags & SF_BEAM_SPARKEND) != 0)
		{
			UTIL_Sparks(end);
		}
	}
}

CBaseEntity* CBeam::GetTripEntity(TraceResult* ptr)
{
	CBaseEntity* pTrip;

	if (ptr->flFraction == 1.0 || ptr->pHit == NULL)
		return NULL;

	pTrip = CBaseEntity::Instance(ptr->pHit);
	if (pTrip == NULL)
		return NULL;

	if (FStringNull(pev->netname))
	{
		if (pTrip->pev->flags & (FL_CLIENT | FL_MONSTER))
			return pTrip;
		else
			return NULL;
	}
	else if (FClassnameIs(pTrip->pev, STRING(pev->netname)))
		return pTrip;
	else if (FStrEq(STRING(pTrip->pev->targetname), STRING(pev->netname)))
		return pTrip;
	else
		return NULL;
}

void CBeam::BeamDamage(TraceResult* ptr)
{
	RelinkBeam();
	if (ptr->flFraction != 1.0 && ptr->pHit != NULL)
	{
		CBaseEntity* pHit = CBaseEntity::Instance(ptr->pHit);
		if (pHit)
		{
			if (pev->dmg > 0)
			{
				ClearMultiDamage();
				pHit->TraceAttack(pev, pev->dmg * (gpGlobals->time - pev->dmgtime), (ptr->vecEndPos - pev->origin).Normalize(), ptr, pev->frags);
				ApplyMultiDamage(pev, pev);
				if ((pev->spawnflags & SF_BEAM_DECALS) != 0)
				{
					if (pHit->IsBSPModel())
						UTIL_DecalTrace(ptr, DECAL_BIGSHOT1 + RANDOM_LONG(0, 4));
				}
			}
			else
			{
				// LRC - beams that heal people
				pHit->TakeHealth(-(pev->dmg * (gpGlobals->time - pev->dmgtime)), DMG_GENERIC);
			}
		}
	}
	pev->dmgtime = gpGlobals->time;
}
