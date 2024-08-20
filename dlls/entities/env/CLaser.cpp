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

#include "CLaser.h"
#include "util/locus.h"
#include "util/movewith.h"

LINK_ENTITY_TO_CLASS(env_laser, CLaser);

TYPEDESCRIPTION CLaser::m_SaveData[] =
	{
		DEFINE_FIELD(CLaser, m_pStartSprite, FIELD_CLASSPTR),
		DEFINE_FIELD(CLaser, m_pEndSprite, FIELD_CLASSPTR),
		DEFINE_FIELD(CLaser, m_iszStartSpriteName, FIELD_STRING),
		DEFINE_FIELD(CLaser, m_iszEndSpriteName, FIELD_STRING),
		DEFINE_FIELD(CLaser, m_firePosition, FIELD_POSITION_VECTOR),
		DEFINE_FIELD(CLaser, m_iProjection, FIELD_INTEGER),
		DEFINE_FIELD(CLaser, m_iStoppedBy, FIELD_INTEGER),
		DEFINE_FIELD(CLaser, m_iszStartPosition, FIELD_STRING),
};

IMPLEMENT_SAVERESTORE(CLaser, CBeam);

void CLaser::Spawn()
{
	if (FStringNull(pev->model))
	{
		SetThink(&CLaser::SUB_Remove);
		return;
	}
	pev->solid = SOLID_NOT; // Remove model & collisions
	Precache();

	SetThink(&CLaser::StrikeThink);
	pev->flags |= FL_CUSTOMENTITY;
}

void CLaser::PostSpawn()
{
	if (m_iszStartSpriteName)
	{
		// LRC: allow the spritename to be the name of an env_sprite
		CBaseEntity* pTemp = UTIL_FindEntityByTargetname(NULL, STRING(m_iszStartSpriteName));
		if (pTemp == NULL)
		{
			m_pStartSprite = CSprite::SpriteCreate(STRING(m_iszStartSpriteName), pev->origin, true);
			if (m_pStartSprite)
				m_pStartSprite->SetTransparency(kRenderGlow, pev->rendercolor.x, pev->rendercolor.y, pev->rendercolor.z, pev->renderamt, pev->renderfx);
		}
		else if (!FClassnameIs(pTemp->pev, "env_sprite"))
		{
			ALERT(at_error, "env_laser \"%s\" found startsprite %s, but can't use: not an env_sprite\n", STRING(pev->targetname), STRING(m_iszStartSpriteName));
			m_pStartSprite = NULL;
		}
		else
		{
			// use an env_sprite defined by the mapper
			m_pStartSprite = (CSprite*)pTemp;
			m_pStartSprite->pev->movetype = MOVETYPE_NOCLIP;
		}
	}
	else if (pev->spawnflags & SF_LASER_INTERPOLATE) // interpolated lasers must have sprites at the start
	{
		m_pStartSprite = CSprite::SpriteCreate("sprites/null.spr", pev->origin, true);
	}
	else
		m_pStartSprite = NULL;


	if (m_iszEndSpriteName)
	{
		CBaseEntity* pTemp = UTIL_FindEntityByTargetname(NULL, STRING(m_iszEndSpriteName));
		if (pTemp == NULL)
		{
			m_pEndSprite = CSprite::SpriteCreate(STRING(m_iszEndSpriteName), pev->origin, true);
			if (m_pEndSprite)
				m_pEndSprite->SetTransparency(kRenderGlow, pev->rendercolor.x, pev->rendercolor.y, pev->rendercolor.z, pev->renderamt, pev->renderfx);
		}
		else if (!FClassnameIs(pTemp->pev, "env_sprite"))
		{
			ALERT(at_error, "env_laser \"%s\" found endsprite %s, but can't use: not an env_sprite\n", STRING(pev->targetname), STRING(m_iszEndSpriteName));
			m_pEndSprite = NULL;
		}
		else
		{
			// use an env_sprite defined by the mapper
			m_pEndSprite = (CSprite*)pTemp;
			m_pEndSprite->pev->movetype = MOVETYPE_NOCLIP;
		}
	}
	else if (pev->spawnflags & SF_LASER_INTERPOLATE) // interpolated lasers must have sprites at the end
	{
		m_pEndSprite = CSprite::SpriteCreate("sprites/null.spr", pev->origin, true);
	}
	else
		m_pEndSprite = NULL;

	// LRC
	if (m_pStartSprite && m_pEndSprite && pev->spawnflags & SF_LASER_INTERPOLATE)
		EntsInit(m_pStartSprite->entindex(), m_pEndSprite->entindex());
	else
		PointsInit(pev->origin, pev->origin);

	if (!FStringNull(pev->targetname) && (pev->spawnflags & SF_BEAM_STARTON) == 0)
		TurnOff();
	else
		TurnOn();
}

void CLaser::Precache()
{
	PRECACHE_MODEL("sprites/null.spr");
	pev->modelindex = PRECACHE_MODEL((char*)STRING(pev->model));
	if (m_iszStartSpriteName)
	{
		// UGLY HACK to check whether this is a filename: does it contain a dot?
		const char* c = STRING(m_iszStartSpriteName);
		while (*c)
		{
			if (*c == '.')
			{
				PRECACHE_MODEL((char*)STRING(m_iszStartSpriteName));
				break;
			}
			c++; // the magic word?
		}
	}

	if (m_iszEndSpriteName)
	{
		const char* c = STRING(m_iszEndSpriteName);
		while (*c)
		{
			if (*c == '.')
			{
				PRECACHE_MODEL((char*)STRING(m_iszEndSpriteName));
				break;
			}
			c++;
		}
	}
}

bool CLaser::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "LaserStart"))
	{
		m_iszStartPosition = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "LaserTarget"))
	{
		pev->message = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iTowardsMode"))
	{
		m_iTowardsMode = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "width"))
	{
		SetWidth((int)atof(pkvd->szValue));
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "NoiseAmplitude"))
	{
		SetNoise(atoi(pkvd->szValue));
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "TextureScroll"))
	{
		SetScrollRate(atoi(pkvd->szValue));
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "texture"))
	{
		pev->model = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "StartSprite"))
	{
		m_iszStartSpriteName = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "EndSprite"))
	{
		m_iszEndSpriteName = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "framestart"))
	{
		pev->frame = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "damage"))
	{
		pev->dmg = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iProjection"))
	{
		m_iProjection = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iStoppedBy"))
	{
		m_iStoppedBy = atoi(pkvd->szValue);
		return true;
	}
	return CBeam::KeyValue(pkvd);
}

void CLaser::TurnOff()
{
	pev->effects |= EF_NODRAW;
	DontThink();
	if (m_pStartSprite)
	{
		m_pStartSprite->TurnOff();
		UTIL_SetVelocity(m_pStartSprite, g_vecZero); // LRC
	}
	if (m_pEndSprite)
	{
		m_pEndSprite->TurnOff();
		UTIL_SetVelocity(m_pEndSprite, g_vecZero); // LRC
	}
}

void CLaser::TurnOn()
{
	pev->effects &= ~EF_NODRAW;

	if (m_pStartSprite)
		m_pStartSprite->TurnOn();

	if (m_pEndSprite)
		m_pEndSprite->TurnOn();

	pev->dmgtime = gpGlobals->time;

	if (pev->spawnflags & SF_BEAM_SHADEIN)
		SetFlags(BEAM_FSHADEIN);
	else if (pev->spawnflags & SF_BEAM_SHADEOUT)
		SetFlags(BEAM_FSHADEOUT);
	else if (pev->spawnflags & SF_BEAM_SOLID)
		SetFlags(BEAM_FSOLID);

	SetNextThink(0);
}

void CLaser::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	bool active = (GetState() == STATE_ON);

	if (!ShouldToggle(useType, active))
		return;
	if (active)
		TurnOff();
	else
	{
		m_hActivator = pActivator; // AJH Storage variable to allow *locus start/end positions
		TurnOn();
	}
}

void CLaser::FireAtPoint(Vector startpos, TraceResult& tr)
{
	if (pev->spawnflags & SF_LASER_INTERPOLATE && m_pStartSprite && m_pEndSprite)
	{
		UTIL_SetVelocity(m_pStartSprite, (startpos - m_pStartSprite->pev->origin) * 10);
		UTIL_SetVelocity(m_pEndSprite, (tr.vecEndPos - m_pEndSprite->pev->origin) * 10);
	}
	else
	{
		if (m_pStartSprite)
			UTIL_AssignOrigin(m_pStartSprite, startpos);

		if (m_pEndSprite)
			UTIL_AssignOrigin(m_pEndSprite, tr.vecEndPos);

		SetStartPos(startpos);
		SetEndPos(tr.vecEndPos);
	}

	BeamDamage(&tr);
	DoSparks(startpos, tr.vecEndPos);
}

void CLaser::StrikeThink()
{
	Vector startpos = pev->origin;
	bool success = true;

	if (m_iszStartPosition)
	{
		startpos = CalcLocus_Position(this, m_hActivator, STRING(m_iszStartPosition)); // AJH allow *locus start/end positions
	}

	if (m_iTowardsMode)
	{
		m_firePosition = startpos + CalcLocus_Velocity(this, m_hActivator, STRING(pev->message)); // AJH allow *locus start/end positions
	}
	else
	{
		CBaseEntity* pEnd = RandomTargetname(STRING(pev->message));

		if (pEnd)
		{
			pEnd->CalcPosition(m_hActivator, &m_firePosition);
		}
		else
		{
			m_firePosition = CalcLocus_Position(this, m_hActivator, STRING(pev->message));
		}
	}

	TraceResult tr;

	// LRC
	//	UTIL_TraceLine( pev->origin, m_firePosition, dont_ignore_monsters, NULL, &tr );
	IGNORE_GLASS iIgnoreGlass;
	if (m_iStoppedBy % 2) // if it's an odd number
		iIgnoreGlass = ignore_glass;
	else
		iIgnoreGlass = dont_ignore_glass;

	IGNORE_MONSTERS iIgnoreMonsters;
	if (m_iStoppedBy <= 1)
		iIgnoreMonsters = dont_ignore_monsters;
	else if (m_iStoppedBy <= 3)
		iIgnoreMonsters = missile;
	else
		iIgnoreMonsters = ignore_monsters;

	if (m_iProjection)
	{
		Vector vecProject = startpos + 4096 * ((m_firePosition - startpos).Normalize());
		UTIL_TraceLine(startpos, vecProject, iIgnoreMonsters, iIgnoreGlass, NULL, &tr);
	}
	else
	{
		UTIL_TraceLine(startpos, m_firePosition, iIgnoreMonsters, iIgnoreGlass, NULL, &tr);
	}

	FireAtPoint(startpos, tr);

	// LRC - tripbeams
	if (pev->target)
	{
		// nicked from monster_tripmine:
		// HACKHACK Set simple box using this really nice global!
		gpGlobals->trace_flags = FTRACE_SIMPLEBOX;
		UTIL_TraceLine(startpos, m_firePosition, dont_ignore_monsters, NULL, &tr);
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
	SetNextThink(0.1);
}
