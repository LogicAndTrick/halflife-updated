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

#include "CLightning.h"
#include "customentity.h"
#include "util/trace.h"

LINK_ENTITY_TO_CLASS(env_lightning, CLightning);
LINK_ENTITY_TO_CLASS(env_beam, CLightning);

TYPEDESCRIPTION CLightning::m_SaveData[] =
	{
		DEFINE_FIELD(CLightning, m_active, FIELD_BOOLEAN),
		DEFINE_FIELD(CLightning, m_iszStartEntity, FIELD_STRING),
		DEFINE_FIELD(CLightning, m_iStartAttachment, FIELD_INTEGER),
		DEFINE_FIELD(CLightning, m_iszEndEntity, FIELD_STRING),
		DEFINE_FIELD(CLightning, m_iEndAttachment, FIELD_INTEGER),
		DEFINE_FIELD(CLightning, m_life, FIELD_FLOAT),
		DEFINE_FIELD(CLightning, m_boltWidth, FIELD_INTEGER),
		DEFINE_FIELD(CLightning, m_noiseAmplitude, FIELD_INTEGER),
		DEFINE_FIELD(CLightning, m_brightness, FIELD_INTEGER),
		DEFINE_FIELD(CLightning, m_speed, FIELD_INTEGER),
		DEFINE_FIELD(CLightning, m_restrike, FIELD_FLOAT),
		DEFINE_FIELD(CLightning, m_spriteTexture, FIELD_INTEGER),
		DEFINE_FIELD(CLightning, m_iszSpriteName, FIELD_STRING),
		DEFINE_FIELD(CLightning, m_frameStart, FIELD_INTEGER),
		DEFINE_FIELD(CLightning, m_radius, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(CLightning, CBeam);

void CLightning::Spawn()
{
	if (FStringNull(m_iszSpriteName))
	{
		SetThink(&CLightning::SUB_Remove);
		return;
	}
	pev->solid = SOLID_NOT; // Remove model & collisions
	Precache();

	pev->dmgtime = gpGlobals->time;

	// LRC- a convenience for mappers. Will this mess anything up?
	if (pev->rendercolor == g_vecZero)
		pev->rendercolor = Vector(255, 255, 255);

	if (pev->frags == 0)
	{
		pev->frags = DMG_ENERGYBEAM;
	}

	if (ServerSide())
	{
		SetThink(NULL);
		if (pev->dmg != 0 || !FStringNull(pev->target))
		{
			SetThink(&CLightning::TripThink);
			SetNextThink(0.1);
		}
		if (!FStringNull(pev->targetname))
		{
			if ((pev->spawnflags & SF_BEAM_STARTON) == 0)
			{
				pev->effects = EF_NODRAW;
				m_active = false;
				DontThink();
			}
			else
				m_active = true;

			SetUse(&CLightning::ToggleUse);
		}
	}
	else
	{
		m_active = false;
		if (!FStringNull(pev->targetname))
		{
			SetUse(&CLightning::StrikeUse);
		}
		if (FStringNull(pev->targetname) || FBitSet(pev->spawnflags, SF_BEAM_STARTON))
		{
			SetThink(&CLightning::StrikeThink);
			SetNextThink(1.0);
		}
	}
}

void CLightning::Precache()
{
	m_spriteTexture = PRECACHE_MODEL((char*)STRING(m_iszSpriteName));
	CBeam::Precache();
}

void CLightning::Activate()
{
	if (ServerSide())
		BeamUpdateVars();

	CBeam::Activate();
}

bool CLightning::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "LightningStart"))
	{
		m_iszStartEntity = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "LightningStartAttachment"))
	{
		m_iStartAttachment = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "LightningEnd"))
	{
		m_iszEndEntity = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "LightningEndAttachment"))
	{
		m_iEndAttachment = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "life"))
	{
		m_life = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "BoltWidth"))
	{
		m_boltWidth = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "NoiseAmplitude"))
	{
		m_noiseAmplitude = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "TextureScroll"))
	{
		m_speed = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "StrikeTime"))
	{
		m_restrike = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "texture"))
	{
		m_iszSpriteName = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "framestart"))
	{
		m_frameStart = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "Radius"))
	{
		m_radius = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "damage"))
	{
		pev->dmg = atof(pkvd->szValue);
		return true;
	}

	return CBeam::KeyValue(pkvd);
}

void CLightning::ToggleUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (!ShouldToggle(useType, m_active))
		return;
	if (m_active)
	{
		m_active = false;
		SUB_UseTargets(this, USE_OFF, 0); // LRC
		pev->effects |= EF_NODRAW;
		DontThink();
	}
	else
	{
		m_active = true;
		SUB_UseTargets(this, USE_ON, 0); // LRC
		BeamUpdatePoints();
		pev->effects &= ~EF_NODRAW;
		DoSparks(GetStartPos(), GetEndPos());
		if (pev->dmg > 0)
		{
			SetNextThink(0);
			pev->dmgtime = gpGlobals->time;
		}
	}
}

void CLightning::StrikeUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (!ShouldToggle(useType, m_active))
		return;

	if (m_active)
	{
		m_active = false;
		SetThink(NULL);
	}
	else
	{
		SetThink(&CLightning::StrikeThink);
		SetNextThink(0.1);
	}

	if (!FBitSet(pev->spawnflags, SF_BEAM_TOGGLE))
		SetUse(NULL);
}

bool IsPointEntity(CBaseEntity* pEnt)
{
	//	ALERT(at_console, "IsPE: %s, %d\n", STRING(pEnt->pev->classname), pEnt->pev->modelindex);
	if (pEnt->pev->modelindex && !(pEnt->pev->flags & FL_CUSTOMENTITY)) // LRC- follow (almost) any entity that has a model
		return false;
	else
		return true;
}

void CLightning::StrikeThink()
{
	if (m_life != 0 && m_restrike != -1) // LRC non-restriking beams! what an idea!
	{
		if ((pev->spawnflags & SF_BEAM_RANDOM) != 0)
			SetNextThink(m_life + RANDOM_FLOAT(0, m_restrike));
		else
			SetNextThink(m_life + m_restrike);
	}
	m_active = true;

	if (FStringNull(m_iszEndEntity))
	{
		if (FStringNull(m_iszStartEntity))
		{
			RandomArea();
		}
		else
		{
			CBaseEntity* pStart = RandomTargetname(STRING(m_iszStartEntity));
			if (pStart != NULL)
				RandomPoint(pStart->pev->origin);
			else
				ALERT(at_debug, "env_beam: unknown entity \"%s\"\n", STRING(m_iszStartEntity));
		}
		return;
	}

	CBaseEntity* pStart = RandomTargetname(STRING(m_iszStartEntity));
	CBaseEntity* pEnd = RandomTargetname(STRING(m_iszEndEntity));

	if (pStart != NULL && pEnd != NULL)
	{
		if (IsPointEntity(pStart) || IsPointEntity(pEnd))
		{
			if ((pev->spawnflags & SF_BEAM_RING) != 0)
			{
				// don't work
				// LRC- FIXME: tell the user there's a problem.
				return;
			}
		}

		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		if (IsPointEntity(pStart) || IsPointEntity(pEnd))
		{
			if (!IsPointEntity(pEnd)) // One point entity must be in pEnd
			{
				CBaseEntity* pTemp;
				pTemp = pStart;
				pStart = pEnd;
				pEnd = pTemp;
			}

			if (!IsPointEntity(pStart)) // One sided
			{
				WRITE_BYTE(TE_BEAMENTPOINT);
				WRITE_SHORT(pStart->entindex());
				WRITE_COORD(pEnd->pev->origin.x);
				WRITE_COORD(pEnd->pev->origin.y);
				WRITE_COORD(pEnd->pev->origin.z);
			}
			else
			{
				WRITE_BYTE(TE_BEAMPOINTS);
				WRITE_COORD(pStart->pev->origin.x);
				WRITE_COORD(pStart->pev->origin.y);
				WRITE_COORD(pStart->pev->origin.z);
				WRITE_COORD(pEnd->pev->origin.x);
				WRITE_COORD(pEnd->pev->origin.y);
				WRITE_COORD(pEnd->pev->origin.z);
			}
		}
		else
		{
			if ((pev->spawnflags & SF_BEAM_RING) != 0)
				WRITE_BYTE(TE_BEAMRING);
			else
				WRITE_BYTE(TE_BEAMENTS);
			WRITE_SHORT(pStart->entindex());
			WRITE_SHORT(pEnd->entindex());
		}

		WRITE_SHORT(m_spriteTexture);
		WRITE_BYTE(m_frameStart);			 // framestart
		WRITE_BYTE((int)pev->framerate);	 // framerate
		WRITE_BYTE((int)(m_life * 10.0));	 // life
		WRITE_BYTE(m_boltWidth);			 // width
		WRITE_BYTE(m_noiseAmplitude);		 // noise
		WRITE_BYTE((int)pev->rendercolor.x); // r, g, b
		WRITE_BYTE((int)pev->rendercolor.y); // r, g, b
		WRITE_BYTE((int)pev->rendercolor.z); // r, g, b
		WRITE_BYTE(pev->renderamt);			 // brightness
		WRITE_BYTE(m_speed);				 // speed
		MESSAGE_END();
		DoSparks(pStart->pev->origin, pEnd->pev->origin);
		if (pev->dmg || !FStringNull(pev->target))
		{
			TraceResult tr;
			UTIL_TraceLine(pStart->pev->origin, pEnd->pev->origin, dont_ignore_monsters, NULL, &tr);
			if (pev->dmg)
				BeamDamageInstant(&tr, pev->dmg);

			// LRC - tripbeams
			CBaseEntity* pTrip;
			if (!FStringNull(pev->target) && (pTrip = GetTripEntity(&tr)) != NULL)
				FireTargets(STRING(pev->target), pTrip, this, USE_TOGGLE, 0);
		}
	}
}

// LRC - used to be DamageThink, but now it's more general.
void CLightning::TripThink()
{
	SetNextThink(0.1);
	TraceResult tr;

	// ALERT(at_console,"TripThink\n");

	if (pev->dmg != 0)
	{
		UTIL_TraceLine(GetStartPos(), GetEndPos(), dont_ignore_monsters, NULL, &tr);
		BeamDamage(&tr);
	}

	// LRC - tripbeams
	if (!FStringNull(pev->target))
	{
		// nicked from monster_tripmine:
		// HACKHACK Set simple box using this really nice global!
		gpGlobals->trace_flags = FTRACE_SIMPLEBOX;
		UTIL_TraceLine(GetStartPos(), GetEndPos(), dont_ignore_monsters, NULL, &tr);
		CBaseEntity* pTrip = GetTripEntity(&tr);
		if (pTrip)
		{
			if (!FBitSet(pev->spawnflags, SF_BEAM_TRIPPED))
			{
				FireTargets(STRING(pev->target), pTrip, this, USE_TOGGLE, 0);
				pev->spawnflags |= SF_BEAM_TRIPPED;
			}
		}
		else
		{
			pev->spawnflags &= ~SF_BEAM_TRIPPED;
		}
	}
}

void CLightning::Zap(const Vector& vecSrc, const Vector& vecDest)
{
#if 1
	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(TE_BEAMPOINTS);
	WRITE_COORD(vecSrc.x);
	WRITE_COORD(vecSrc.y);
	WRITE_COORD(vecSrc.z);
	WRITE_COORD(vecDest.x);
	WRITE_COORD(vecDest.y);
	WRITE_COORD(vecDest.z);
	WRITE_SHORT(m_spriteTexture);
	WRITE_BYTE(m_frameStart);			 // framestart
	WRITE_BYTE((int)pev->framerate);	 // framerate
	WRITE_BYTE((int)(m_life * 10.0));	 // life
	WRITE_BYTE(m_boltWidth);			 // width
	WRITE_BYTE(m_noiseAmplitude);		 // noise
	WRITE_BYTE((int)pev->rendercolor.x); // r, g, b
	WRITE_BYTE((int)pev->rendercolor.y); // r, g, b
	WRITE_BYTE((int)pev->rendercolor.z); // r, g, b
	WRITE_BYTE(pev->renderamt);			 // brightness
	WRITE_BYTE(m_speed);				 // speed
	MESSAGE_END();
#else
	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(TE_LIGHTNING);
	WRITE_COORD(vecSrc.x);
	WRITE_COORD(vecSrc.y);
	WRITE_COORD(vecSrc.z);
	WRITE_COORD(vecDest.x);
	WRITE_COORD(vecDest.y);
	WRITE_COORD(vecDest.z);
	WRITE_BYTE(10);
	WRITE_BYTE(50);
	WRITE_BYTE(40);
	WRITE_SHORT(m_spriteTexture);
	MESSAGE_END();
#endif
	DoSparks(vecSrc, vecDest);
}

void CLightning::RandomArea()
{
	int iLoops = 0;

	for (iLoops = 0; iLoops < 10; iLoops++)
	{
		Vector vecSrc = pev->origin;

		Vector vecDir1 = Vector(RANDOM_FLOAT(-1.0, 1.0), RANDOM_FLOAT(-1.0, 1.0), RANDOM_FLOAT(-1.0, 1.0));
		vecDir1 = vecDir1.Normalize();
		TraceResult tr1;
		UTIL_TraceLine(vecSrc, vecSrc + vecDir1 * m_radius, ignore_monsters, ENT(pev), &tr1);

		if (tr1.flFraction == 1.0)
			continue;

		Vector vecDir2;
		do
		{
			vecDir2 = Vector(RANDOM_FLOAT(-1.0, 1.0), RANDOM_FLOAT(-1.0, 1.0), RANDOM_FLOAT(-1.0, 1.0));
		} while (DotProduct(vecDir1, vecDir2) > 0);
		vecDir2 = vecDir2.Normalize();
		TraceResult tr2;
		UTIL_TraceLine(vecSrc, vecSrc + vecDir2 * m_radius, ignore_monsters, ENT(pev), &tr2);

		if (tr2.flFraction == 1.0)
			continue;

		if ((tr1.vecEndPos - tr2.vecEndPos).Length() < m_radius * 0.1)
			continue;

		UTIL_TraceLine(tr1.vecEndPos, tr2.vecEndPos, ignore_monsters, ENT(pev), &tr2);

		if (tr2.flFraction != 1.0)
			continue;

		Zap(tr1.vecEndPos, tr2.vecEndPos);

		break;
	}
}

void CLightning::RandomPoint(Vector& vecSrc)
{
	int iLoops = 0;

	for (iLoops = 0; iLoops < 10; iLoops++)
	{
		Vector vecDir1 = Vector(RANDOM_FLOAT(-1.0, 1.0), RANDOM_FLOAT(-1.0, 1.0), RANDOM_FLOAT(-1.0, 1.0));
		vecDir1 = vecDir1.Normalize();
		TraceResult tr1;
		UTIL_TraceLine(vecSrc, vecSrc + vecDir1 * m_radius, ignore_monsters, ENT(pev), &tr1);

		if ((tr1.vecEndPos - vecSrc).Length() < m_radius * 0.1)
			continue;

		if (tr1.flFraction == 1.0)
			continue;

		Zap(vecSrc, tr1.vecEndPos);
		break;
	}
}

// LRC: Called whenever the beam gets turned on, in case an alias changed or one of the points has moved.
void CLightning::BeamUpdatePoints()
{
	int beamType;
	bool pointStart, pointEnd;

	CBaseEntity* pStart = UTIL_FindEntityByTargetname(NULL, STRING(m_iszStartEntity));
	CBaseEntity* pEnd = UTIL_FindEntityByTargetname(NULL, STRING(m_iszEndEntity));
	if (!pStart || !pEnd)
		return;
	pointStart = IsPointEntity(pStart);
	pointEnd = IsPointEntity(pEnd);

	beamType = BEAM_ENTS;
	if (pointStart || pointEnd)
	{
		if (!pointStart) // One point entity must be in pStart
		{
			CBaseEntity* pTemp;
			// Swap start & end
			pTemp = pStart;
			pStart = pEnd;
			pEnd = pTemp;
			bool swap = pointStart;
			pointStart = pointEnd;
			pointEnd = swap;
		}
		if (!pointEnd)
			beamType = BEAM_ENTPOINT;
		else
			beamType = BEAM_POINTS;
	}

	SetType(beamType);
	if (beamType == BEAM_POINTS || beamType == BEAM_ENTPOINT || beamType == BEAM_HOSE)
	{
		SetStartPos(pStart->pev->origin);
		if (beamType == BEAM_POINTS || beamType == BEAM_HOSE)
			SetEndPos(pEnd->pev->origin);
		else
		{
			SetEndEntity(ENTINDEX(ENT(pEnd->pev)));
			SetEndAttachment(m_iEndAttachment);
		}
	}
	else
	{
		SetStartEntity(ENTINDEX(ENT(pStart->pev)));
		SetStartAttachment(m_iStartAttachment);
		SetEndEntity(ENTINDEX(ENT(pEnd->pev)));
		SetEndAttachment(m_iEndAttachment);
	}

	RelinkBeam();
}

void CLightning::BeamUpdateVars()
{
	pev->skin = 0;
	pev->sequence = 0;
	pev->rendermode = 0;
	pev->flags |= FL_CUSTOMENTITY;
	pev->model = m_iszSpriteName;
	SetTexture(m_spriteTexture);

	BeamUpdatePoints(); // LRC

	SetWidth(m_boltWidth);
	SetNoise(m_noiseAmplitude);
	SetFrame(m_frameStart);
	SetScrollRate(m_speed);
	if ((pev->spawnflags & SF_BEAM_SHADEIN) != 0)
		SetFlags(BEAM_FSHADEIN);
	else if ((pev->spawnflags & SF_BEAM_SHADEOUT) != 0)
		SetFlags(BEAM_FSHADEOUT);
	else if (pev->spawnflags & SF_BEAM_SOLID)
		SetFlags(BEAM_FSOLID);
}
