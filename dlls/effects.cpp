/***
*
*	Copyright (c) 1999, 2000 Valve LLC. All rights reserved.
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
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "customentity.h"
#include "effects.h"
#include "weapons.h"
#include "decals.h"
#include "func_break.h"
#include "shake.h"
#include "player.h"	  //LRC - footstep stuff
#include "locus.h"	  //LRC - locus utilities
#include "movewith.h" //LRC - the DesiredThink system
#include "UserMessages.h"

#define SF_FUNNEL_REVERSE 1	   // funnel effect repels particles instead of attracting them.
#define SF_FUNNEL_REPEATABLE 2 // allows a funnel to be refired

#define SF_GIBSHOOTER_REPEATABLE 1 // allows a gibshooter to be refired


//LRC - make info_target an entity class in its own right
class CInfoTarget : public CPointEntity
{
public:
	void Spawn() override;
	void Precache() override;
};

LINK_ENTITY_TO_CLASS(info_target, CInfoTarget);

//LRC- force an info_target to use the sprite null.spr
#define SF_TARGET_HACK_VISIBLE 1

// Landmark class
void CInfoTarget::Spawn()
{
	//Precache();
	pev->solid = SOLID_NOT;
	if (pev->spawnflags & SF_TARGET_HACK_VISIBLE)
	{
		PRECACHE_MODEL("sprites/null.spr");
		SET_MODEL(ENT(pev), "sprites/null.spr");
		UTIL_SetSize(pev, g_vecZero, g_vecZero);
	}
}

void CInfoTarget::Precache()
{
	if (pev->spawnflags & SF_TARGET_HACK_VISIBLE)
		PRECACHE_MODEL("sprites/null.spr");
}


class CBubbling : public CBaseEntity
{
public:
	void Spawn() override;
	void Precache() override;
	bool KeyValue(KeyValueData* pkvd) override;

	void EXPORT FizzThink();
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	int ObjectCaps() override { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	static TYPEDESCRIPTION m_SaveData[];

	int m_density;
	int m_frequency;
	int m_bubbleModel;
	bool m_state;

	STATE GetState() override { return m_state ? STATE_ON : STATE_OFF; };
};

LINK_ENTITY_TO_CLASS(env_bubbles, CBubbling);

TYPEDESCRIPTION CBubbling::m_SaveData[] =
	{
		DEFINE_FIELD(CBubbling, m_density, FIELD_INTEGER),
		DEFINE_FIELD(CBubbling, m_frequency, FIELD_INTEGER),
		DEFINE_FIELD(CBubbling, m_state, FIELD_BOOLEAN),
		// Let spawn restore this!
		//	DEFINE_FIELD( CBubbling, m_bubbleModel, FIELD_INTEGER ),
};

IMPLEMENT_SAVERESTORE(CBubbling, CBaseEntity);


#define SF_BUBBLES_STARTOFF 0x0001

void CBubbling::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), STRING(pev->model)); // Set size

	pev->solid = SOLID_NOT; // Remove model & collisions
	pev->renderamt = 0;		// The engine won't draw this model if this is set to 0 and blending is on
	pev->rendermode = kRenderTransTexture;
	int speed = pev->speed > 0 ? pev->speed : -pev->speed;

	// HACKHACK!!! - Speed in rendercolor
	pev->rendercolor.x = speed >> 8;
	pev->rendercolor.y = speed & 255;
	pev->rendercolor.z = (pev->speed < 0) ? 1 : 0;


	if ((pev->spawnflags & SF_BUBBLES_STARTOFF) == 0)
	{
		SetThink(&CBubbling::FizzThink);
		SetNextThink(2.0);
		m_state = true;
	}
	else
		m_state = false;
}

void CBubbling::Precache()
{
	m_bubbleModel = PRECACHE_MODEL("sprites/bubble.spr"); // Precache bubble sprite
}

void CBubbling::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (ShouldToggle(useType, m_state))
		m_state = !m_state;

	if (m_state)
	{
		SetThink(&CBubbling::FizzThink);
		SetNextThink(0.1);
	}
	else
	{
		SetThink(NULL);
		DontThink();
	}
}


bool CBubbling::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "density"))
	{
		m_density = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "frequency"))
	{
		m_frequency = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "current"))
	{
		pev->speed = atoi(pkvd->szValue);
		return true;
	}

	return CBaseEntity::KeyValue(pkvd);
}


void CBubbling::FizzThink()
{
	MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, VecBModelOrigin(pev));
	WRITE_BYTE(TE_FIZZ);
	WRITE_SHORT((short)ENTINDEX(edict()));
	WRITE_SHORT((short)m_bubbleModel);
	WRITE_BYTE(m_density);
	MESSAGE_END();

	if (m_frequency > 19) // frequencies above 20 are treated as 20.
		SetNextThink(0.5);
	else
		SetNextThink(2.5 - (0.1 * m_frequency));
}

// --------------------------------------------------
//
// Beams
//
// --------------------------------------------------

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

#if 0
void CBeam::SetObjectCollisionBox()
{
	const Vector &startPos = GetStartPos(), &endPos = GetEndPos();

	pev->absmin.x = V_min( startPos.x, endPos.x );
	pev->absmin.y = V_min( startPos.y, endPos.y );
	pev->absmin.z = V_min( startPos.z, endPos.z );
	pev->absmax.x = V_max( startPos.x, endPos.x );
	pev->absmax.y = V_max( startPos.y, endPos.y );
	pev->absmax.z = V_max( startPos.z, endPos.z );
}
#endif


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

class CLightning : public CBeam
{
public:
	void Spawn() override;
	void Precache() override;
	bool KeyValue(KeyValueData* pkvd) override;
	void Activate() override;

	void EXPORT StrikeThink();
	void EXPORT TripThink();
	void RandomArea();
	void RandomPoint(Vector& vecSrc);
	void Zap(const Vector& vecSrc, const Vector& vecDest);
	void EXPORT StrikeUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void EXPORT ToggleUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

	inline bool ServerSide()
	{
		if (m_life == 0 && (pev->spawnflags & SF_BEAM_RING) == 0)
			return true;
		return false;
	}

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	void BeamUpdatePoints(); //LRC
	void BeamUpdateVars();

	STATE GetState() override { return m_active ? STATE_OFF : STATE_ON; };

	bool m_active;
	int m_iszStartEntity;
	int m_iszEndEntity;
	float m_life;
	int m_boltWidth;
	int m_noiseAmplitude;
	int m_brightness;
	int m_speed;
	float m_restrike;
	int m_spriteTexture;
	int m_iszSpriteName;
	int m_frameStart;
	int m_iStartAttachment;
	int m_iEndAttachment;

	float m_radius;
};

LINK_ENTITY_TO_CLASS(env_lightning, CLightning);
LINK_ENTITY_TO_CLASS(env_beam, CLightning);

// UNDONE: Jay -- CTripBeam is only a test
#if _DEBUG
class CTripBeam : public CLightning
{
	void Spawn() override;
};
LINK_ENTITY_TO_CLASS(trip_beam, CTripBeam);

void CTripBeam::Spawn()
{
	CLightning::Spawn();
	SetTouch(&CTripBeam::TriggerTouch);
	pev->solid = SOLID_TRIGGER;
	RelinkBeam();
}
#endif



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

	//LRC- a convenience for mappers. Will this mess anything up?
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
		SUB_UseTargets(this, USE_OFF, 0); //LRC
		pev->effects |= EF_NODRAW;
		DontThink();
	}
	else
	{
		m_active = true;
		SUB_UseTargets(this, USE_ON, 0); //LRC
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
	if (pEnt->pev->modelindex && !(pEnt->pev->flags & FL_CUSTOMENTITY)) //LRC- follow (almost) any entity that has a model
		return false;
	else
		return true;
}


void CLightning::StrikeThink()
{
	if (m_life != 0 && m_restrike != -1) //LRC non-restriking beams! what an idea!
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
				//LRC- FIXME: tell the user there's a problem.
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

			//LRC - tripbeams
			CBaseEntity* pTrip;
			if (!FStringNull(pev->target) && (pTrip = GetTripEntity(&tr)) != NULL)
				FireTargets(STRING(pev->target), pTrip, this, USE_TOGGLE, 0);
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
				//LRC - beams that heal people
				pHit->TakeHealth(-(pev->dmg * (gpGlobals->time - pev->dmgtime)), DMG_GENERIC);
			}
		}
	}
	pev->dmgtime = gpGlobals->time;
}

//LRC - used to be DamageThink, but now it's more general.
void CLightning::TripThink()
{
	SetNextThink(0.1);
	TraceResult tr;

	//ALERT(at_console,"TripThink\n");

	if (pev->dmg != 0)
	{
		UTIL_TraceLine(GetStartPos(), GetEndPos(), dont_ignore_monsters, NULL, &tr);
		BeamDamage(&tr);
	}

	//LRC - tripbeams
	if (!FStringNull(pev->target))
	{
		// nicked from monster_tripmine:
		//HACKHACK Set simple box using this really nice global!
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

	BeamUpdatePoints(); //LRC

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
		//LRC: allow the spritename to be the name of an env_sprite
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

	//LRC
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
		UTIL_SetVelocity(m_pStartSprite, g_vecZero); //LRC
	}
	if (m_pEndSprite)
	{
		m_pEndSprite->TurnOff();
		UTIL_SetVelocity(m_pEndSprite, g_vecZero); //LRC
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
		m_hActivator = pActivator; //AJH Storage variable to allow *locus start/end positions
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
		startpos = CalcLocus_Position(this, m_hActivator, STRING(m_iszStartPosition)); //AJH allow *locus start/end positions
	}

	if (m_iTowardsMode)
	{
		m_firePosition = startpos + CalcLocus_Velocity(this, m_hActivator, STRING(pev->message)); //AJH allow *locus start/end positions
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

	//LRC
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

	//LRC - tripbeams
	if (pev->target)
	{
		// nicked from monster_tripmine:
		//HACKHACK Set simple box using this really nice global!
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



class CGlow : public CPointEntity
{
public:
	void Spawn() override;
	void Think() override;
	void Animate(float frames);
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	float m_lastTime;
	float m_maxFrame;
};

LINK_ENTITY_TO_CLASS(env_glow, CGlow);

TYPEDESCRIPTION CGlow::m_SaveData[] =
	{
		DEFINE_FIELD(CGlow, m_lastTime, FIELD_TIME),
		DEFINE_FIELD(CGlow, m_maxFrame, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(CGlow, CPointEntity);

void CGlow::Spawn()
{
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	pev->effects = 0;
	pev->frame = 0;

	PRECACHE_MODEL((char*)STRING(pev->model));
	SET_MODEL(ENT(pev), STRING(pev->model));

	m_maxFrame = (float)MODEL_FRAMES(pev->modelindex) - 1;
	if (m_maxFrame > 1.0 && pev->framerate != 0)
		SetNextThink(0.1);

	m_lastTime = gpGlobals->time;
}


void CGlow::Think()
{
	Animate(pev->framerate * (gpGlobals->time - m_lastTime));

	SetNextThink(0.1);
	m_lastTime = gpGlobals->time;
}


void CGlow::Animate(float frames)
{
	if (m_maxFrame > 0)
		pev->frame = fmod(pev->frame + frames, m_maxFrame);
}


LINK_ENTITY_TO_CLASS(env_sprite, CSprite);

TYPEDESCRIPTION CSprite::m_SaveData[] =
	{
		DEFINE_FIELD(CSprite, m_lastTime, FIELD_TIME),
		DEFINE_FIELD(CSprite, m_maxFrame, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(CSprite, CPointEntity);

void CSprite::Spawn()
{
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	pev->effects = 0;
	pev->frame = 0;

	Precache();
	SET_MODEL(ENT(pev), STRING(pev->model));

	m_maxFrame = (float)MODEL_FRAMES(pev->modelindex) - 1;
	if (!FStringNull(pev->targetname) && (pev->spawnflags & SF_SPRITE_STARTON) == 0)
		TurnOff();
	else
		TurnOn();

	// Worldcraft only sets y rotation, copy to Z
	if (pev->angles.y != 0 && pev->angles.z == 0)
	{
		pev->angles.z = pev->angles.y;
		pev->angles.y = 0;
	}
}


void CSprite::Precache()
{
	PRECACHE_MODEL((char*)STRING(pev->model));

	// Reset attachment after save/restore
	if (pev->aiment)
		SetAttachment(pev->aiment, pev->body);
	else
	{
		// Clear attachment
		pev->skin = 0;
		pev->body = 0;
	}
}


void CSprite::SpriteInit(const char* pSpriteName, const Vector& origin)
{
	pev->model = MAKE_STRING(pSpriteName);
	pev->origin = origin;
	Spawn();
}

CSprite* CSprite::SpriteCreate(const char* pSpriteName, const Vector& origin, bool animate)
{
	CSprite* pSprite = GetClassPtr((CSprite*)NULL);
	pSprite->SpriteInit(pSpriteName, origin);
	pSprite->pev->classname = MAKE_STRING("env_sprite");
	pSprite->pev->solid = SOLID_NOT;
	pSprite->pev->movetype = MOVETYPE_NOCLIP;
	if (animate)
		pSprite->TurnOn();

	return pSprite;
}


void CSprite::AnimateThink()
{
	Animate(pev->framerate * (gpGlobals->time - m_lastTime));

	SetNextThink(0.1);
	m_lastTime = gpGlobals->time;
}

void CSprite::AnimateUntilDead()
{
	if (gpGlobals->time > pev->dmgtime)
		UTIL_Remove(this);
	else
	{
		AnimateThink();
		SetNextThink(0);
	}
}

void CSprite::Expand(float scaleSpeed, float fadeSpeed)
{
	pev->speed = scaleSpeed;
	pev->health = fadeSpeed;
	SetThink(&CSprite::ExpandThink);

	SetNextThink(0);
	m_lastTime = gpGlobals->time;
}


void CSprite::ExpandThink()
{
	float frametime = gpGlobals->time - m_lastTime;
	pev->scale += pev->speed * frametime;
	pev->renderamt -= pev->health * frametime;
	if (pev->renderamt <= 0)
	{
		pev->renderamt = 0;
		UTIL_Remove(this);
	}
	else
	{
		SetNextThink(0.1);
		m_lastTime = gpGlobals->time;
	}
}


void CSprite::Animate(float frames)
{
	pev->frame += frames;
	if (pev->frame > m_maxFrame)
	{
		if ((pev->spawnflags & SF_SPRITE_ONCE) != 0)
		{
			TurnOff();
		}
		else
		{
			if (m_maxFrame > 0)
				pev->frame = fmod(pev->frame, m_maxFrame);
		}
	}
}


void CSprite::TurnOff()
{
	pev->effects = EF_NODRAW;
	DontThink();
}


void CSprite::TurnOn()
{
	if (pev->message)
	{
		CBaseEntity* pTemp = UTIL_FindEntityByTargetname(NULL, STRING(pev->message));
		if (pTemp)
			SetAttachment(pTemp->edict(), pev->frags);
		else
			return;
	}
	pev->effects = 0;
	if ((0 != pev->framerate && m_maxFrame > 1.0) || (pev->spawnflags & SF_SPRITE_ONCE) != 0)
	{
		SetThink(&CSprite::AnimateThink);
		SetNextThink(0);
		m_lastTime = gpGlobals->time;
	}
	pev->frame = 0;
}


void CSprite::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	bool on = pev->effects != EF_NODRAW;
	if (ShouldToggle(useType, on))
	{
		if (on)
		{
			SUB_UseTargets(this, USE_OFF, 0); //LRC
			TurnOff();
		}
		else
		{
			SUB_UseTargets(this, USE_ON, 0); //LRC
			TurnOn();
		}
	}
}

//=================================================================
// env_model: like env_sprite, except you can specify a sequence.
//=================================================================
#define SF_ENVMODEL_OFF 1
#define SF_ENVMODEL_DROPTOFLOOR 2
#define SF_ENVMODEL_SOLID 4

class CEnvModel : public CBaseAnimating
{
	void Spawn() override;
	void Precache() override;
	void EXPORT Think() override;
	bool KeyValue(KeyValueData* pkvd) override;
	STATE GetState() override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	int ObjectCaps() override { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	void SetSequence();

	string_t m_iszSequence_On;
	string_t m_iszSequence_Off;
	int m_iAction_On;
	int m_iAction_Off;
};

TYPEDESCRIPTION CEnvModel::m_SaveData[] =
	{
		DEFINE_FIELD(CEnvModel, m_iszSequence_On, FIELD_STRING),
		DEFINE_FIELD(CEnvModel, m_iszSequence_Off, FIELD_STRING),
		DEFINE_FIELD(CEnvModel, m_iAction_On, FIELD_INTEGER),
		DEFINE_FIELD(CEnvModel, m_iAction_Off, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CEnvModel, CBaseAnimating);
LINK_ENTITY_TO_CLASS(env_model, CEnvModel);

bool CEnvModel::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "m_iszSequence_On"))
	{
		m_iszSequence_On = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszSequence_Off"))
	{
		m_iszSequence_Off = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iAction_On"))
	{
		m_iAction_On = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iAction_Off"))
	{
		m_iAction_Off = atoi(pkvd->szValue);
		return true;
	}
	return CBaseAnimating::KeyValue(pkvd);
}

void CEnvModel::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), STRING(pev->model));
	UTIL_SetOrigin(this, pev->origin);

	//	UTIL_AssignOrigin(this, pev->oldorigin); //AJH - WTF is this here for?

	if (pev->spawnflags & SF_ENVMODEL_SOLID)
	{
		pev->solid = SOLID_SLIDEBOX;
		UTIL_SetSize(pev, Vector(-10, -10, -10), Vector(10, 10, 10)); //LRCT
	}

	if (pev->spawnflags & SF_ENVMODEL_DROPTOFLOOR)
	{
		pev->origin.z += 1;
		DROP_TO_FLOOR(ENT(pev));
	}
	SetBoneController(0, 0);
	SetBoneController(1, 0);

	SetSequence();

	SetNextThink(0.1);
}

void CEnvModel::Precache()
{
	PRECACHE_MODEL((char*)STRING(pev->model));
}

STATE CEnvModel::GetState()
{
	if (pev->spawnflags & SF_ENVMODEL_OFF)
		return STATE_OFF;
	else
		return STATE_ON;
}

void CEnvModel::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (ShouldToggle(useType, !(pev->spawnflags & SF_ENVMODEL_OFF)))
	{
		if (pev->spawnflags & SF_ENVMODEL_OFF)
			pev->spawnflags &= ~SF_ENVMODEL_OFF;
		else
			pev->spawnflags |= SF_ENVMODEL_OFF;

		SetSequence();
		SetNextThink(0.1);
	}
}

void CEnvModel::Think()
{
	int iTemp;

	//	ALERT(at_console, "env_model Think fr=%f\n", pev->framerate);

	StudioFrameAdvance(); // set m_fSequenceFinished if necessary

	//	if (m_fSequenceLoops)
	//	{
	//		SetNextThink( 1E6 );
	//		return; // our work here is done.
	//	}
	if (m_fSequenceFinished && !m_fSequenceLoops)
	{
		if (pev->spawnflags & SF_ENVMODEL_OFF)
			iTemp = m_iAction_Off;
		else
			iTemp = m_iAction_On;

		switch (iTemp)
		{
			//		case 1: // loop
			//			pev->animtime = gpGlobals->time;
			//			m_fSequenceFinished = false;
			//			m_flLastEventCheck = gpGlobals->time;
			//			pev->frame = 0;
			//			break;
		case 2: // change state
			if (pev->spawnflags & SF_ENVMODEL_OFF)
				pev->spawnflags &= ~SF_ENVMODEL_OFF;
			else
				pev->spawnflags |= SF_ENVMODEL_OFF;
			SetSequence();
			break;
		default: //remain frozen
			return;
		}
	}
	SetNextThink(0.1);
}

void CEnvModel::SetSequence()
{
	int iszSeq;

	if (pev->spawnflags & SF_ENVMODEL_OFF)
		iszSeq = m_iszSequence_Off;
	else
		iszSeq = m_iszSequence_On;

	if (!iszSeq)
		return;
	pev->sequence = LookupSequence(STRING(iszSeq));

	if (pev->sequence == -1)
	{
		if (pev->targetname)
			ALERT(at_error, "env_model %s: unknown sequence \"%s\"\n", STRING(pev->targetname), STRING(iszSeq));
		else
			ALERT(at_error, "env_model: unknown sequence \"%s\"\n", STRING(pev->targetname), STRING(iszSeq));
		pev->sequence = 0;
	}

	pev->frame = 0;
	ResetSequenceInfo();

	if (pev->spawnflags & SF_ENVMODEL_OFF)
	{
		if (m_iAction_Off == 1)
			m_fSequenceLoops = 1;
		else
			m_fSequenceLoops = 0;
	}
	else
	{
		if (m_iAction_On == 1)
			m_fSequenceLoops = 1;
		else
			m_fSequenceLoops = 0;
	}
}


#define SF_GIBSHOOTER_REPEATABLE 1 // allows a gibshooter to be refired
#define SF_GIBSHOOTER_DEBUG 4	   //LRC

class CGibShooter : public CBaseDelay
{
public:
	void Spawn() override;
	void Precache() override;
	bool KeyValue(KeyValueData* pkvd) override;
	void EXPORT ShootThink();
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;

	virtual CBaseEntity* CreateGib(Vector vecPos, Vector vecVel);

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	int m_iGibs;
	int m_iGibCapacity;
	int m_iGibMaterial;
	int m_iGibModelIndex;
	//	float m_flGibVelocity;
	float m_flVariance;
	float m_flGibLife;
	int m_iszTargetname;
	int m_iszPosition;
	int m_iszVelocity;
	int m_iszVelFactor;
	int m_iszSpawnTarget;
	int m_iBloodColor;
};

TYPEDESCRIPTION CGibShooter::m_SaveData[] =
	{
		DEFINE_FIELD(CGibShooter, m_iGibs, FIELD_INTEGER),
		DEFINE_FIELD(CGibShooter, m_iGibCapacity, FIELD_INTEGER),
		DEFINE_FIELD(CGibShooter, m_iGibMaterial, FIELD_INTEGER),
		DEFINE_FIELD(CGibShooter, m_iGibModelIndex, FIELD_INTEGER),
		//	DEFINE_FIELD( CGibShooter, m_flGibVelocity, FIELD_FLOAT ),
		DEFINE_FIELD(CGibShooter, m_flVariance, FIELD_FLOAT),
		DEFINE_FIELD(CGibShooter, m_flGibLife, FIELD_FLOAT),
		DEFINE_FIELD(CGibShooter, m_iszTargetname, FIELD_STRING),
		DEFINE_FIELD(CGibShooter, m_iszPosition, FIELD_STRING),
		DEFINE_FIELD(CGibShooter, m_iszVelocity, FIELD_STRING),
		DEFINE_FIELD(CGibShooter, m_iszVelFactor, FIELD_STRING),
		DEFINE_FIELD(CGibShooter, m_iszSpawnTarget, FIELD_STRING),
		DEFINE_FIELD(CGibShooter, m_iBloodColor, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CGibShooter, CBaseDelay);
LINK_ENTITY_TO_CLASS(gibshooter, CGibShooter);


void CGibShooter::Precache()
{
	if (g_Language == LANGUAGE_GERMAN)
	{
		m_iGibModelIndex = PRECACHE_MODEL("models/germanygibs.mdl");
	}
	else if (m_iBloodColor == BLOOD_COLOR_YELLOW)
	{
		m_iGibModelIndex = PRECACHE_MODEL("models/agibs.mdl");
	}
	else
	{
		m_iGibModelIndex = PRECACHE_MODEL("models/hgibs.mdl");
	}
}


bool CGibShooter::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "m_iGibs"))
	{
		m_iGibs = m_iGibCapacity = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_flVelocity"))
	{
		m_iszVelFactor = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_flVariance"))
	{
		m_flVariance = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_flGibLife"))
	{
		m_flGibLife = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszTargetName"))
	{
		m_iszTargetname = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszPosition"))
	{
		m_iszPosition = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszVelocity"))
	{
		m_iszVelocity = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszVelFactor"))
	{
		m_iszVelFactor = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszSpawnTarget"))
	{
		m_iszSpawnTarget = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iBloodColor"))
	{
		m_iBloodColor = atoi(pkvd->szValue);
		return true;
	}

	return CBaseDelay::KeyValue(pkvd);
}

void CGibShooter::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	m_hActivator = pActivator;
	SetThink(&CGibShooter::ShootThink);
	SetNextThink(0);
}

void CGibShooter::Spawn()
{
	Precache();

	pev->solid = SOLID_NOT;
	pev->effects = EF_NODRAW;

	//	if ( m_flDelay == 0 )
	//	{
	//		m_flDelay = 0.1;
	//	}

	if (m_flGibLife == 0)
	{
		m_flGibLife = 25;
	}

	SetMovedir(pev);
	if (pev->body == 0)
		pev->body = MODEL_FRAMES(m_iGibModelIndex);
}


CBaseEntity* CGibShooter::CreateGib(Vector vecPos, Vector vecVel)
{
	if (CVAR_GET_FLOAT("violence_hgibs") == 0)
		return NULL;

	CGib* pGib = GetClassPtr((CGib*)NULL);

	//	if (pGib)
	//		ALERT(at_console, "Gib created ok\n");

	pGib->pev->origin = vecPos;
	pGib->pev->velocity = vecVel;

	if (m_iBloodColor == BLOOD_COLOR_YELLOW)
	{
		pGib->Spawn("models/agibs.mdl");
		pGib->m_bloodColor = BLOOD_COLOR_YELLOW;
	}
	else if (m_iBloodColor)
	{
		pGib->Spawn("models/hgibs.mdl");
		pGib->m_bloodColor = m_iBloodColor;
	}
	else
	{
		pGib->Spawn("models/hgibs.mdl");
		pGib->m_bloodColor = BLOOD_COLOR_RED;
	}

	if (pev->body <= 1)
	{
		ALERT(at_aiconsole, "GibShooter Body is <= 1!\n");
	}

	pGib->pev->body = RANDOM_LONG(1, pev->body - 1); // avoid throwing random amounts of the 0th gib. (skull).

	float thinkTime = pGib->m_fNextThink - gpGlobals->time;

	pGib->m_lifeTime = (m_flGibLife * RANDOM_FLOAT(0.95, 1.05)); // +/- 5%
	if (pGib->m_lifeTime < thinkTime)
	{
		pGib->SetNextThink(pGib->m_lifeTime);
		pGib->m_lifeTime = 0;
	}

	pGib->pev->avelocity.x = RANDOM_FLOAT(100, 200);
	pGib->pev->avelocity.y = RANDOM_FLOAT(100, 300);

	return pGib;
}


void CGibShooter::ShootThink()
{
	int i;
	if (m_flDelay == 0) // LRC - delay is 0, fire them all at once.
	{
		i = m_iGibs;
	}
	else
	{
		i = 1;
		SetNextThink(m_flDelay);
	}

	while (i > 0)
	{
		Vector vecShootDir;
		Vector vecPos;
		float flGibVelocity;
		if (!FStringNull(m_iszVelFactor))
			flGibVelocity = CalcLocus_Number(m_hActivator, STRING(m_iszVelFactor));
		else
			flGibVelocity = 1;

		if (!FStringNull(m_iszVelocity))
		{
			vecShootDir = CalcLocus_Velocity(this, m_hActivator, STRING(m_iszVelocity));
			flGibVelocity = flGibVelocity * vecShootDir.Length();
			vecShootDir = vecShootDir.Normalize();
		}
		else
			vecShootDir = pev->movedir;

		vecShootDir = vecShootDir + gpGlobals->v_right * RANDOM_FLOAT(-1, 1) * m_flVariance;
		vecShootDir = vecShootDir + gpGlobals->v_forward * RANDOM_FLOAT(-1, 1) * m_flVariance;
		vecShootDir = vecShootDir + gpGlobals->v_up * RANDOM_FLOAT(-1, 1) * m_flVariance;

		vecShootDir = vecShootDir.Normalize();

		if (!FStringNull(m_iszPosition))
			vecPos = CalcLocus_Position(this, m_hActivator, STRING(m_iszPosition));
		else
			vecPos = pev->origin;
		CBaseEntity* pGib = CreateGib(vecPos, vecShootDir * flGibVelocity);

		if (pGib)
		{
			pGib->pev->targetname = m_iszTargetname;
			//			pGib->pev->velocity = vecShootDir * flGibVelocity;

			if (pev->spawnflags & SF_GIBSHOOTER_DEBUG)
				ALERT(at_debug, "DEBUG: %s \"%s\" creates a shot at %f %f %f; vel %f %f %f; pos \"%s\"\n", STRING(pev->classname), STRING(pev->targetname), pGib->pev->origin.x, pGib->pev->origin.y, pGib->pev->origin.z, pGib->pev->velocity.x, pGib->pev->velocity.y, pGib->pev->velocity.z, STRING(m_iszPosition));

			if (m_iszSpawnTarget)
				FireTargets(STRING(m_iszSpawnTarget), pGib, this, USE_TOGGLE, 0);
		}

		i--;
		m_iGibs--;
	}

	if (m_iGibs <= 0)
	{
		if ((pev->spawnflags & SF_GIBSHOOTER_REPEATABLE) != 0)
		{
			m_iGibs = m_iGibCapacity;
			SetThink(NULL);
			DontThink();
		}
		else
		{
			SetThink(&CGibShooter::SUB_Remove);
			SetNextThink(0);
		}
	}
}


// Shooter particle
class CShot : public CSprite
{
public:
	void Touch(CBaseEntity* pOther) override;
};

void CShot::Touch(CBaseEntity* pOther)
{
	if (pev->teleport_time > gpGlobals->time)
		return;
	// don't fire too often in collisions!
	// teleport_time is the soonest this can be touched again.
	pev->teleport_time = gpGlobals->time + 0.1;

	if (pev->netname)
		FireTargets(STRING(pev->netname), this, this, USE_TOGGLE, 0);
	if (pev->message && pOther && pOther != g_pWorld)
		FireTargets(STRING(pev->message), pOther, this, USE_TOGGLE, 0);
}

class CEnvShooter : public CGibShooter
{
	void Precache() override;
	bool KeyValue(KeyValueData* pkvd) override;
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	void Spawn() override;

	static TYPEDESCRIPTION m_SaveData[];

	CBaseEntity* CreateGib(Vector vecPos, Vector vecVel) override;

	int m_iszTouch;
	int m_iszTouchOther;
	int m_iPhysics;
	float m_fFriction;
	Vector m_vecSize;
};

TYPEDESCRIPTION CEnvShooter::m_SaveData[] =
	{
		DEFINE_FIELD(CEnvShooter, m_iszTouch, FIELD_STRING),
		DEFINE_FIELD(CEnvShooter, m_iszTouchOther, FIELD_STRING),
		DEFINE_FIELD(CEnvShooter, m_iPhysics, FIELD_INTEGER),
		DEFINE_FIELD(CEnvShooter, m_fFriction, FIELD_FLOAT),
		DEFINE_FIELD(CEnvShooter, m_vecSize, FIELD_VECTOR),
};

IMPLEMENT_SAVERESTORE(CEnvShooter, CGibShooter);
LINK_ENTITY_TO_CLASS(env_shooter, CEnvShooter);

void CEnvShooter::Spawn()
{
	int iBody = pev->body;
	CGibShooter::Spawn();
	pev->body = iBody;
}

bool CEnvShooter::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "shootmodel"))
	{
		pev->model = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "shootsounds"))
	{
		int iNoise = atoi(pkvd->szValue);

		switch (iNoise)
		{
		case 0:
			m_iGibMaterial = matGlass;
			break;
		case 1:
			m_iGibMaterial = matWood;
			break;
		case 2:
			m_iGibMaterial = matMetal;
			break;
		case 3:
			m_iGibMaterial = matFlesh;
			break;
		case 4:
			m_iGibMaterial = matRocks;
			break;

		default:
		case -1:
			m_iGibMaterial = matNone;
			break;
		}

		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszTouch"))
	{
		m_iszTouch = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszTouchOther"))
	{
		m_iszTouchOther = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iPhysics"))
	{
		m_iPhysics = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_fFriction"))
	{
		m_fFriction = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_vecSize"))
	{
		UTIL_StringToVector((float*)m_vecSize, pkvd->szValue);
		m_vecSize = m_vecSize / 2;
		return true;
	}
	return CGibShooter::KeyValue(pkvd);
}


void CEnvShooter::Precache()
{
	if (pev->model)
		m_iGibModelIndex = PRECACHE_MODEL((char*)STRING(pev->model));
	CBreakable::MaterialSoundPrecache((Materials)m_iGibMaterial);
}


CBaseEntity* CEnvShooter::CreateGib(Vector vecPos, Vector vecVel)
{
	if (pev->noise)
		pev->scale = CalcLocus_Number(this, STRING(pev->noise), 0); //AJH / MJB - allow locus_ratio for scale
	if (m_iPhysics <= 1)											// normal gib or sticky gib
	{
		CGib* pGib = GetClassPtr((CGib*)NULL);

		pGib->pev->origin = vecPos;
		pGib->pev->velocity = vecVel;

		pGib->Spawn(STRING(pev->model));
		if (m_iPhysics) // sticky gib
		{
			pGib->pev->movetype = MOVETYPE_TOSS;
			pGib->pev->solid = SOLID_BBOX;
			UTIL_SetSize(pGib->pev, Vector(0, 0, 0), Vector(0, 0, 0));
			pGib->SetTouch(&CGib::StickyGibTouch);
		}
		if (pev->body > 0)
			pGib->pev->body = RANDOM_LONG(0, pev->body - 1);
		if (m_iBloodColor)
			pGib->m_bloodColor = m_iBloodColor;
		else
			pGib->m_bloodColor = DONT_BLEED;
		pGib->m_material = m_iGibMaterial;
		pGib->pev->rendermode = pev->rendermode;
		pGib->pev->renderamt = pev->renderamt;
		pGib->pev->rendercolor = pev->rendercolor;
		pGib->pev->renderfx = pev->renderfx;
		if (pev->scale >= 100)
			pGib->pev->scale = 1.0; //G-Cont. for fix with laarge gibs :) (MJB i know, but sometimes people deliberately want large models?)
		else
			pGib->pev->scale = pev->scale;
		pGib->pev->skin = pev->skin;

		float thinkTime = pGib->m_fNextThink - gpGlobals->time;

		pGib->m_lifeTime = (m_flGibLife * RANDOM_FLOAT(0.95, 1.05)); // +/- 5%
		if (pGib->m_lifeTime < thinkTime)
		{
			pGib->SetNextThink(pGib->m_lifeTime);
			pGib->m_lifeTime = 0;
		}

		pGib->pev->avelocity.x = RANDOM_FLOAT(100, 200);
		pGib->pev->avelocity.y = RANDOM_FLOAT(100, 300);

		return pGib;
	}

	// special shot
	CShot* pShot = GetClassPtr((CShot*)NULL);
	if (FStringNull(m_iPhysics))
		pShot->pev->movetype = MOVETYPE_BOUNCE; //G-Cont. fix for blank field m_iPhysics, e. g. - original HL
	pShot->pev->classname = MAKE_STRING("shot");
	pShot->pev->solid = SOLID_SLIDEBOX;
	pShot->pev->origin = vecPos;
	pShot->pev->velocity = vecVel;
	SET_MODEL(ENT(pShot->pev), STRING(pev->model));
	UTIL_SetSize(pShot->pev, -m_vecSize, m_vecSize);
	pShot->pev->renderamt = pev->renderamt;
	pShot->pev->rendermode = pev->rendermode;
	pShot->pev->rendercolor = pev->rendercolor;
	pShot->pev->renderfx = pev->renderfx;
	pShot->pev->netname = m_iszTouch;
	pShot->pev->message = m_iszTouchOther;
	pShot->pev->skin = pev->skin;
	pShot->pev->body = pev->body;
	pShot->pev->scale = pev->scale;
	pShot->pev->frame = pev->frame;
	pShot->pev->framerate = pev->framerate;
	pShot->pev->friction = m_fFriction;

	switch (m_iPhysics)
	{
	case 2:
		pShot->pev->movetype = MOVETYPE_NOCLIP;
		pShot->pev->solid = SOLID_NOT;
		break;
	case 3:
		pShot->pev->movetype = MOVETYPE_FLYMISSILE;
		break;
	case 4:
		pShot->pev->movetype = MOVETYPE_BOUNCEMISSILE;
		break;
	case 5:
		pShot->pev->movetype = MOVETYPE_TOSS;
		break;
	case 6:
		pShot->pev->movetype = MOVETYPE_BOUNCE;
		break;
	}

	if (pShot->pev->framerate)
	{
		pShot->m_maxFrame = (float)MODEL_FRAMES(pShot->pev->modelindex) - 1;
		if (pShot->m_maxFrame > 1.0)
		{
			if (m_flGibLife)
			{
				pShot->pev->dmgtime = gpGlobals->time + m_flGibLife;
				pShot->SetThink(&CShot::AnimateUntilDead);
			}
			else
			{
				pShot->SetThink(&CShot::AnimateThink);
			}
			pShot->SetNextThink(0);
			pShot->m_lastTime = gpGlobals->time;
			return pShot;
		}
	}

	// if it's not animating
	if (m_flGibLife)
	{
		pShot->SetThink(&CShot::SUB_Remove);
		pShot->SetNextThink(m_flGibLife);
	}
	return pShot;
}




class CTestEffect : public CBaseDelay
{
public:
	void Spawn() override;
	void Precache() override;
	// bool	KeyValue( KeyValueData *pkvd );
	void EXPORT TestThink();
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;

	int m_iLoop;
	int m_iBeam;
	CBeam* m_pBeam[24];
	float m_flBeamTime[24];
	float m_flStartTime;
};


LINK_ENTITY_TO_CLASS(test_effect, CTestEffect);

void CTestEffect::Spawn()
{
	Precache();
}

void CTestEffect::Precache()
{
	PRECACHE_MODEL("sprites/lgtning.spr");
}

void CTestEffect::TestThink()
{
	int i;
	float t = (gpGlobals->time - m_flStartTime);

	if (m_iBeam < 24)
	{
		CBeam* pbeam = CBeam::BeamCreate("sprites/lgtning.spr", 100);

		TraceResult tr;

		Vector vecSrc = pev->origin;
		Vector vecDir = Vector(RANDOM_FLOAT(-1.0, 1.0), RANDOM_FLOAT(-1.0, 1.0), RANDOM_FLOAT(-1.0, 1.0));
		vecDir = vecDir.Normalize();
		UTIL_TraceLine(vecSrc, vecSrc + vecDir * 128, ignore_monsters, ENT(pev), &tr);

		pbeam->PointsInit(vecSrc, tr.vecEndPos);
		// pbeam->SetColor( 80, 100, 255 );
		pbeam->SetColor(255, 180, 100);
		pbeam->SetWidth(100);
		pbeam->SetScrollRate(12);

		m_flBeamTime[m_iBeam] = gpGlobals->time;
		m_pBeam[m_iBeam] = pbeam;
		m_iBeam++;

#if 0
		Vector vecMid = (vecSrc + tr.vecEndPos) * 0.5;
		MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
			WRITE_BYTE(TE_DLIGHT);
			WRITE_COORD(vecMid.x);	// X
			WRITE_COORD(vecMid.y);	// Y
			WRITE_COORD(vecMid.z);	// Z
			WRITE_BYTE( 20 );		// radius * 0.1
			WRITE_BYTE( 255 );		// r
			WRITE_BYTE( 180 );		// g
			WRITE_BYTE( 100 );		// b
			WRITE_BYTE( 20 );		// time * 10
			WRITE_BYTE( 0 );		// decay * 0.1
		MESSAGE_END( );
#endif
	}

	if (t < 3.0)
	{
		for (i = 0; i < m_iBeam; i++)
		{
			t = (gpGlobals->time - m_flBeamTime[i]) / (3 + m_flStartTime - m_flBeamTime[i]);
			m_pBeam[i]->SetBrightness(255 * t);
			// m_pBeam[i]->SetScrollRate( 20 * t );
		}
		SetNextThink(0.1);
	}
	else
	{
		for (i = 0; i < m_iBeam; i++)
		{
			UTIL_Remove(m_pBeam[i]);
		}
		m_flStartTime = gpGlobals->time;
		m_iBeam = 0;
		SetThink(NULL);
	}
}


void CTestEffect::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	SetThink(&CTestEffect::TestThink);
	SetNextThink(0.1);
	m_flStartTime = gpGlobals->time;
}



// Blood effects
class CBlood : public CPointEntity
{
public:
	void Spawn() override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	bool KeyValue(KeyValueData* pkvd) override;

	inline int Color() { return pev->impulse; }
	inline float BloodAmount() { return pev->dmg; }

	inline void SetColor(int color) { pev->impulse = color; }
	inline void SetBloodAmount(float amount) { pev->dmg = amount; }

	Vector Direction(CBaseEntity* pActivator); //LRC - added pActivator, for locus system
	Vector BloodPosition(CBaseEntity* pActivator);

private:
};

LINK_ENTITY_TO_CLASS(env_blood, CBlood);



#define SF_BLOOD_RANDOM 0x0001
#define SF_BLOOD_STREAM 0x0002
#define SF_BLOOD_PLAYER 0x0004
#define SF_BLOOD_DECAL 0x0008

void CBlood::Spawn()
{
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	pev->effects = 0;
	pev->frame = 0;
	SetMovedir(pev);
	if (Color() == 0)
		SetColor(BLOOD_COLOR_RED);
}


bool CBlood::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "color"))
	{
		int color = atoi(pkvd->szValue);
		switch (color)
		{
		case 1:
			SetColor(BLOOD_COLOR_YELLOW);
			break;
		default:
			SetColor(BLOOD_COLOR_RED);
			break;
		}

		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "amount"))
	{
		SetBloodAmount(atof(pkvd->szValue));
		return true;
	}

	return CPointEntity::KeyValue(pkvd);
}


Vector CBlood::Direction(CBaseEntity* pActivator)
{
	if ((pev->spawnflags & SF_BLOOD_RANDOM) != 0)
		return UTIL_RandomBloodVector();
	else if (pev->netname)
		return CalcLocus_Velocity(this, pActivator, STRING(pev->netname));
	else
		return pev->movedir;
}


Vector CBlood::BloodPosition(CBaseEntity* pActivator)
{
	if ((pev->spawnflags & SF_BLOOD_PLAYER) != 0)
	{
		CBaseEntity* pPlayer;

		if (pActivator && pActivator->IsPlayer())
		{
			pPlayer = pActivator;
		}
		else
			pPlayer = UTIL_GetLocalPlayer();
		if (pPlayer)
			return (pPlayer->pev->origin + pPlayer->pev->view_ofs) + Vector(RANDOM_FLOAT(-10, 10), RANDOM_FLOAT(-10, 10), RANDOM_FLOAT(-10, 10));
		// if no player found, fall through
	}
	else if (pev->target)
	{
		return CalcLocus_Position(this, pActivator, STRING(pev->target));
	}

	return pev->origin;
}


void CBlood::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if ((pev->spawnflags & SF_BLOOD_STREAM) != 0)
		UTIL_BloodStream(BloodPosition(pActivator), Direction(pActivator), (Color() == BLOOD_COLOR_RED) ? 70 : Color(), BloodAmount());
	else
		UTIL_BloodDrips(BloodPosition(pActivator), Direction(pActivator), Color(), BloodAmount());

	if ((pev->spawnflags & SF_BLOOD_DECAL) != 0)
	{
		Vector forward = Direction(pActivator);
		Vector start = BloodPosition(pActivator);
		TraceResult tr;

		UTIL_TraceLine(start, start + forward * BloodAmount() * 2, ignore_monsters, NULL, &tr);
		if (tr.flFraction != 1.0)
			UTIL_BloodDecalTrace(&tr, Color());
	}
}



// Screen shake
class CShake : public CPointEntity
{
public:
	void Spawn() override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	bool KeyValue(KeyValueData* pkvd) override;

	inline float Amplitude() { return pev->scale; }
	inline float Frequency() { return pev->dmg_save; }
	inline float Duration() { return pev->dmg_take; }
	inline float Radius() { return pev->dmg; }

	inline void SetAmplitude(float amplitude) { pev->scale = amplitude; }
	inline void SetFrequency(float frequency) { pev->dmg_save = frequency; }
	inline void SetDuration(float duration) { pev->dmg_take = duration; }
	inline void SetRadius(float radius) { pev->dmg = radius; }

	STATE m_iState;									 //LRC
	STATE GetState() override { return m_iState; };	 //LRC
	void Think() override { m_iState = STATE_OFF; }; //LRC
private:
};

LINK_ENTITY_TO_CLASS(env_shake, CShake);

// pev->scale is amplitude
// pev->dmg_save is frequency
// pev->dmg_take is duration
// pev->dmg is radius
// radius of 0 means all players
// NOTE: UTIL_ScreenShake() will only shake players who are on the ground

#define SF_SHAKE_EVERYONE 0x0001 // Don't check radius
// UNDONE: These don't work yet
#define SF_SHAKE_DISRUPT 0x0002 // Disrupt controls
#define SF_SHAKE_INAIR 0x0004	// Shake players in air

void CShake::Spawn()
{
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	pev->effects = 0;
	pev->frame = 0;

	m_iState = STATE_OFF; //LRC

	if ((pev->spawnflags & SF_SHAKE_EVERYONE) != 0)
		pev->dmg = 0;
}


bool CShake::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "amplitude"))
	{
		SetAmplitude(atof(pkvd->szValue));
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "frequency"))
	{
		SetFrequency(atof(pkvd->szValue));
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "duration"))
	{
		SetDuration(atof(pkvd->szValue));
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "radius"))
	{
		SetRadius(atof(pkvd->szValue));
		return true;
	}

	return CPointEntity::KeyValue(pkvd);
}


void CShake::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	UTIL_ScreenShake(pev->origin, Amplitude(), Frequency(), Duration(), Radius());
	m_iState = STATE_ON;	  //LRC
	SetNextThink(Duration()); //LRC
}


class CFade : public CPointEntity
{
public:
	void Spawn() override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	bool KeyValue(KeyValueData* pkvd) override;

	STATE GetState() override { return m_iState; }; // LRC
	void Think() override;							//LRC

	inline float Duration() { return pev->dmg_take; }
	inline float HoldTime() { return pev->dmg_save; }

	inline void SetDuration(float duration) { pev->dmg_take = duration; }
	inline void SetHoldTime(float hold) { pev->dmg_save = hold; }

	STATE m_iState; // LRC. Don't saverestore this value, it's not worth it.
private:
};

LINK_ENTITY_TO_CLASS(env_fade, CFade);

// pev->dmg_take is duration
// pev->dmg_save is hold duration
#define SF_FADE_IN 0x0001		// Fade in, not out
#define SF_FADE_MODULATE 0x0002 // Modulate, don't blend
#define SF_FADE_ONLYONE 0x0004
#define SF_FADE_PERMANENT 0x0008 //LRC - hold permanently
#define SF_FADE_CAMERA 0x0010	 //fading only for camera

void CFade::Spawn()
{
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	pev->effects = 0;
	pev->frame = 0;

	m_iState = STATE_OFF; //LRC
}


bool CFade::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "duration"))
	{
		SetDuration(atof(pkvd->szValue));
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "holdtime"))
	{
		SetHoldTime(atof(pkvd->szValue));
		return true;
	}

	return CPointEntity::KeyValue(pkvd);
}


void CFade::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	int fadeFlags = 0;

	if (pev->spawnflags & SF_FADE_CAMERA)
	{
		if (!pActivator || !pActivator->IsPlayer())
		{
			pActivator = CBaseEntity::Instance(g_engfuncs.pfnPEntityOfEntIndex(1));
		}
		if (pActivator && pActivator->IsPlayer())
		{
			if (((CBasePlayer*)pActivator)->viewFlags == 0)
			{
				// 				ALERT(at_console, "player is not curently see in camera\n");
				return;
			}
		}
	}

	m_iState = STATE_TURN_ON; //LRC
	SetNextThink(Duration()); //LRC

	if ((pev->spawnflags & SF_FADE_IN) == 0)
		fadeFlags |= FFADE_OUT;

	if ((pev->spawnflags & SF_FADE_MODULATE) != 0)
		fadeFlags |= FFADE_MODULATE;

	if (pev->spawnflags & SF_FADE_PERMANENT) //LRC
		fadeFlags |= FFADE_STAYOUT;			 //LRC

	if ((pev->spawnflags & SF_FADE_ONLYONE) != 0)
	{
		if (pActivator->IsNetClient())
		{
			UTIL_ScreenFade(pActivator, pev->rendercolor, Duration(), HoldTime(), pev->renderamt, fadeFlags);
		}
	}
	else
	{
		UTIL_ScreenFadeAll(pev->rendercolor, Duration(), HoldTime(), pev->renderamt, fadeFlags);
	}
	SUB_UseTargets(this, USE_TOGGLE, 0);
}

//LRC: a bolt-on state!
void CFade::Think()
{
	if (m_iState == STATE_TURN_ON)
	{
		m_iState = STATE_ON;
		if (!(pev->spawnflags & SF_FADE_PERMANENT))
			SetNextThink(HoldTime());
	}
	else
		m_iState = STATE_OFF;
}


class CMessage : public CPointEntity
{
public:
	void Spawn() override;
	void Precache() override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	bool KeyValue(KeyValueData* pkvd) override;

private:
};

LINK_ENTITY_TO_CLASS(env_message, CMessage);


void CMessage::Spawn()
{
	Precache();

	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;

	switch (pev->impulse)
	{
	case 1: // Medium radius
		pev->speed = ATTN_STATIC;
		break;

	case 2: // Large radius
		pev->speed = ATTN_NORM;
		break;

	case 3: //EVERYWHERE
		pev->speed = ATTN_NONE;
		break;

	default:
	case 0: // Small radius
		pev->speed = ATTN_IDLE;
		break;
	}
	pev->impulse = 0;

	// No volume, use normal
	if (pev->scale <= 0)
		pev->scale = 1.0;
}


void CMessage::Precache()
{
	if (pev->noise)
		PRECACHE_SOUND((char*)STRING(pev->noise));
}

bool CMessage::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "messagesound"))
	{
		pev->noise = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "messagevolume"))
	{
		pev->scale = atof(pkvd->szValue) * 0.1;
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "messageattenuation"))
	{
		pev->impulse = atoi(pkvd->szValue);
		return true;
	}
	return CPointEntity::KeyValue(pkvd);
}


void CMessage::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	CBaseEntity* pPlayer = NULL;

	if (pev->spawnflags & SF_MESSAGE_ALL)
		UTIL_ShowMessageAll(STRING(pev->message));
	else
	{
		if (pActivator && pActivator->IsPlayer())
			pPlayer = pActivator;
		else
		{
			pPlayer = UTIL_GetLocalPlayer();
		}
		if (pPlayer)
			UTIL_ShowMessage(STRING(pev->message), pPlayer);
	}
	if (pev->noise)
	{
		EMIT_SOUND(edict(), CHAN_BODY, STRING(pev->noise), pev->scale, pev->speed);
	}
	if (pev->spawnflags & SF_MESSAGE_ONCE)
		UTIL_Remove(this);

	SUB_UseTargets(this, USE_TOGGLE, 0);
}



//=========================================================
// FunnelEffect
//=========================================================
class CEnvFunnel : public CBaseDelay
{
public:
	void Spawn() override;
	void Precache() override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;

	int m_iSprite; // Don't save, precache
};

void CEnvFunnel::Precache()
{
	//LRC
	if (pev->netname)
		m_iSprite = PRECACHE_MODEL((char*)STRING(pev->netname));
	else
		m_iSprite = PRECACHE_MODEL("sprites/flare6.spr");
}

LINK_ENTITY_TO_CLASS(env_funnel, CEnvFunnel);

void CEnvFunnel::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	//LRC
	Vector vecPos;
	if (pev->message)
		vecPos = CalcLocus_Position(this, pActivator, STRING(pev->message));
	else
		vecPos = pev->origin;

	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(TE_LARGEFUNNEL);
	WRITE_COORD(vecPos.x);
	WRITE_COORD(vecPos.y);
	WRITE_COORD(vecPos.z);
	WRITE_SHORT(m_iSprite);

	if (pev->spawnflags & SF_FUNNEL_REVERSE) // funnel flows in reverse
		WRITE_SHORT(1);
	else
		WRITE_SHORT(0);
	MESSAGE_END();

	if (!(pev->spawnflags & SF_FUNNEL_REPEATABLE))
	{
		SetThink(&CEnvFunnel::SUB_Remove);
		SetNextThink(0);
	}
}

void CEnvFunnel::Spawn()
{
	Precache();
	pev->solid = SOLID_NOT;
	pev->effects = EF_NODRAW;
}


//=========================================================
// LRC -  All the particle effects from Quake 1
//=========================================================
#define SF_QUAKEFX_REPEATABLE 1
class CEnvQuakeFx : public CPointEntity
{
public:
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
};

LINK_ENTITY_TO_CLASS(env_quakefx, CEnvQuakeFx);

void CEnvQuakeFx::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	Vector vecPos;
	if (pev->message)
		vecPos = CalcLocus_Position(this, pActivator, STRING(pev->message));
	else
		vecPos = pev->origin;

	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(pev->impulse);
	WRITE_COORD(vecPos.x);
	WRITE_COORD(vecPos.y);
	WRITE_COORD(vecPos.z);
	if (pev->impulse == TE_PARTICLEBURST)
	{
		WRITE_SHORT(pev->armortype);  // radius
		WRITE_BYTE(pev->frags);		  // particle colour
		WRITE_BYTE(pev->health * 10); // duration
	}
	else if (pev->impulse == TE_EXPLOSION2)
	{
		// these fields seem to have no effect - except that it
		// crashes when I send "0" for the number of colours..
		WRITE_BYTE(0); // colour
		WRITE_BYTE(1); // number of colours
	}
	MESSAGE_END();

	if (!(pev->spawnflags & SF_QUAKEFX_REPEATABLE))
	{
		SetThink(&CEnvQuakeFx::SUB_Remove);
		SetNextThink(0);
	}
}


//=========================================================
// LRC - Beam Trail effect
//=========================================================
#define SF_BEAMTRAIL_OFF 1
class CEnvBeamTrail : public CPointEntity
{
public:
	void Spawn() override;
	void Precache() override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	STATE GetState() override;
	void EXPORT StartTrailThink();
	void Affect(CBaseEntity* pTarget, USE_TYPE useType);

	int m_iSprite; // Don't save, precache
};

void CEnvBeamTrail::Precache()
{
	if (pev->target)
		PRECACHE_MODEL("sprites/null.spr");
	if (pev->netname)
		m_iSprite = PRECACHE_MODEL((char*)STRING(pev->netname));
}

LINK_ENTITY_TO_CLASS(env_beamtrail, CEnvBeamTrail);

STATE CEnvBeamTrail::GetState()
{
	if (pev->spawnflags & SF_BEAMTRAIL_OFF)
		return STATE_OFF;
	else
		return STATE_ON;
}

void CEnvBeamTrail::StartTrailThink()
{
	pev->spawnflags |= SF_BEAMTRAIL_OFF; // fake turning off, so the Use turns it on properly
	Use(this, this, USE_ON, 0);
}

void CEnvBeamTrail::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (pev->target)
	{
		CBaseEntity* pTarget = UTIL_FindEntityByTargetname(NULL, STRING(pev->target), pActivator);
		while (pTarget)
		{
			Affect(pTarget, useType);
			pTarget = UTIL_FindEntityByTargetname(pTarget, STRING(pev->target), pActivator);
		}
	}
	else
	{
		if (!ShouldToggle(useType))
			return;
		Affect(this, useType);
	}

	if (useType == USE_ON)
		pev->spawnflags &= ~SF_BEAMTRAIL_OFF;
	else if (useType == USE_OFF)
		pev->spawnflags |= SF_BEAMTRAIL_OFF;
	else if (useType == USE_TOGGLE)
	{
		if (pev->spawnflags & SF_BEAMTRAIL_OFF)
			pev->spawnflags &= ~SF_BEAMTRAIL_OFF;
		else
			pev->spawnflags |= SF_BEAMTRAIL_OFF;
	}
}

void CEnvBeamTrail::Affect(CBaseEntity* pTarget, USE_TYPE useType)
{
	if (useType == USE_ON || pev->spawnflags & SF_BEAMTRAIL_OFF)
	{
		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_BEAMFOLLOW);
		WRITE_SHORT(pTarget->entindex()); // entity
		WRITE_SHORT(m_iSprite);			  // model
		WRITE_BYTE(pev->health * 10);	  // life
		WRITE_BYTE(pev->armorvalue);	  // width
		WRITE_BYTE(pev->rendercolor.x);	  // r, g, b
		WRITE_BYTE(pev->rendercolor.y);	  // r, g, b
		WRITE_BYTE(pev->rendercolor.z);	  // r, g, b
		WRITE_BYTE(pev->renderamt);		  // brightness
		MESSAGE_END();
	}
	else
	{
		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_KILLBEAM);
		WRITE_SHORT(pTarget->entindex());
		MESSAGE_END();
	}
}

void CEnvBeamTrail::Spawn()
{
	Precache();

	SET_MODEL(ENT(pev), "sprites/null.spr");
	UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));

	if (!(pev->spawnflags & SF_BEAMTRAIL_OFF))
	{
		SetThink(&CEnvBeamTrail::StartTrailThink);
		UTIL_DesiredThink(this);
	}
}


//=========================================================
// LRC -  custom footstep sounds
//=========================================================
#define SF_FOOTSTEPS_SET 1
#define SF_FOOTSTEPS_ONCE 2

class CEnvFootsteps : public CBaseEntity
{
public:
	void Spawn() override;
	void Precache() override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	int ObjectCaps() override { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	STATE GetState() override;
	STATE GetState(CBaseEntity* pEnt) override;
	void PrecacheNoise(const char* szNoise);
};

LINK_ENTITY_TO_CLASS(env_footsteps, CEnvFootsteps);

void CEnvFootsteps::Spawn()
{
	Precache();
}

void CEnvFootsteps::PrecacheNoise(const char* szNoise)
{
	static char szBuf[128];
	int i = 0, j = 0;
	for (i = 0; szNoise[i]; i++)
	{
		if (szNoise[i] == '?')
		{
			strcpy(szBuf, szNoise);
			for (j = 0; j < 4; j++)
			{
				szBuf[i] = j + '1';
				PRECACHE_SOUND(szBuf);
			}
		}
	}
	if (!j)
		PRECACHE_SOUND((char*)szNoise);
}

void CEnvFootsteps::Precache()
{
	if (pev->noise)
		PrecacheNoise(STRING(pev->noise));
	if (pev->noise1)
		PrecacheNoise(STRING(pev->noise1));
	if (pev->noise2)
		PrecacheNoise(STRING(pev->noise2));
	if (pev->noise3)
		PrecacheNoise(STRING(pev->noise3));
}

STATE CEnvFootsteps::GetState()
{
	if (pev->spawnflags & SF_FOOTSTEPS_SET)
		return STATE_OFF;
	return pev->impulse ? STATE_ON : STATE_OFF;
}

STATE CEnvFootsteps::GetState(CBaseEntity* pEnt)
{
	if (pev->spawnflags & SF_FOOTSTEPS_SET)
		return STATE_OFF;
	if (pEnt->IsPlayer())
	{
		// based on trigger_hurt code
		int playerMask = 1 << (pEnt->entindex() - 1);

		if (pev->impulse & playerMask)
			return STATE_ON;
		else
			return STATE_OFF;
	}
	else
		return GetState();
}

void CEnvFootsteps::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	//	union floatToString ftsTemp;

	//CONSIDER: add an "all players" spawnflag, like game_text?
	if (pActivator && pActivator->IsPlayer())
	{
		int playerMask = 1 << (pActivator->entindex() - 1);

		if (pev->spawnflags & SF_FOOTSTEPS_SET || (!(pev->impulse & playerMask) && (useType == USE_ON || useType == USE_TOGGLE)))
		{
			pev->impulse |= playerMask;
			if (pev->frags)
			{
				char sTemp[4];
				sprintf(sTemp, "%d", (int)pev->frags);
				g_engfuncs.pfnSetPhysicsKeyValue(pActivator->edict(), "stype", sTemp);
				//pActivator->pev->iFootstepType = pev->frags;
			}
			else if (pev->noise)
			{
				g_engfuncs.pfnSetPhysicsKeyValue(pActivator->edict(), "ssnd", STRING(pev->noise));
			}
			if (pev->noise1)
			{
				g_engfuncs.pfnSetPhysicsKeyValue(pActivator->edict(), "lsnd", STRING(pev->noise1));
			}
			if (pev->noise2)
			{
				g_engfuncs.pfnSetPhysicsKeyValue(pActivator->edict(), "wsnd", STRING(pev->noise2));
			}
			if (pev->noise3)
			{
				g_engfuncs.pfnSetPhysicsKeyValue(pActivator->edict(), "psnd", STRING(pev->noise3));
			}
			// workaround for physinfo string bug: force the engine to null-terminate it
			g_engfuncs.pfnSetPhysicsKeyValue(pActivator->edict(), "x", "0");
			//ALERT(at_console, "ON, InfoString = %s\n", g_engfuncs.pfnGetPhysicsInfoString(pActivator->edict()));
			if (pev->spawnflags & SF_FOOTSTEPS_SET && pev->spawnflags & SF_FOOTSTEPS_ONCE)
			{
				UTIL_Remove(this);
			}
		}
		else if ((pev->impulse & playerMask) && (useType == USE_OFF || useType == USE_TOGGLE))
		{
			pev->impulse &= ~playerMask;
			if (pev->frags)
			{
				g_engfuncs.pfnSetPhysicsKeyValue(pActivator->edict(), "stype", "0");
			}
			else if (pev->noise)
			{
				g_engfuncs.pfnSetPhysicsKeyValue(pActivator->edict(), "ssnd", "0");
			}
			if (pev->noise1)
			{
				g_engfuncs.pfnSetPhysicsKeyValue(pActivator->edict(), "lsnd", "0");
			}
			if (pev->noise2)
			{
				g_engfuncs.pfnSetPhysicsKeyValue(pActivator->edict(), "wsnd", "0");
			}
			if (pev->noise3)
			{
				g_engfuncs.pfnSetPhysicsKeyValue(pActivator->edict(), "psnd", "0");
			}
			// workaround for physinfo string bug: force the engine to null-terminate it
			g_engfuncs.pfnSetPhysicsKeyValue(pActivator->edict(), "x", "0");
			//ALERT(at_console, "OFF, InfoString = %s\n", g_engfuncs.pfnGetPhysicsInfoString(pActivator->edict()));
			if (pev->spawnflags & SF_FOOTSTEPS_ONCE)
			{
				UTIL_Remove(this);
			}
		}
		else
		{
			//ALERT(at_console, "NO EFFECT\n");
		}
	}
}

//=========================================================
//LRC- the long-awaited effect. (Rain, in the desert? :)
//
//FIXME: give designers a _lot_ more control.
//=========================================================
#define MAX_RAIN_BEAMS 32

#define AXIS_X 1
#define AXIS_Y 2
#define AXIS_Z 0

#define EXTENT_OBSTRUCTED 1
#define EXTENT_ARCING 2
#define EXTENT_OBSTRUCTED_REVERSE 3
#define EXTENT_ARCING_REVERSE 4
#define EXTENT_ARCING_THROUGH 5 //AJH

class CEnvRain : public CBaseEntity
{
public:
	void Spawn() override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	void Think() override;
	void Precache() override;
	bool KeyValue(KeyValueData* pkvd) override;
	int ObjectCaps() override { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	STATE m_iState;
	int m_spriteTexture;
	int m_iszSpriteName; // have to saverestore this, the beams keep a link to it
	int m_dripSize;
	int m_minDripSpeed;
	int m_maxDripSpeed;
	int m_burstSize;
	int m_brightness;
	int m_pitch; // don't saverestore this
	float m_flUpdateTime;
	float m_flMaxUpdateTime;
	//	CBeam*	m_pBeams[MAX_RAIN_BEAMS];
	int m_axis;
	int m_iExtent;
	float m_fLifeTime;
	int m_iNoise;

	STATE GetState() override { return m_iState; };
};

LINK_ENTITY_TO_CLASS(env_rain, CEnvRain);

TYPEDESCRIPTION CEnvRain::m_SaveData[] =
	{
		DEFINE_FIELD(CEnvRain, m_iState, FIELD_INTEGER),
		DEFINE_FIELD(CEnvRain, m_spriteTexture, FIELD_INTEGER),
		DEFINE_FIELD(CEnvRain, m_dripSize, FIELD_INTEGER),
		DEFINE_FIELD(CEnvRain, m_minDripSpeed, FIELD_INTEGER),
		DEFINE_FIELD(CEnvRain, m_maxDripSpeed, FIELD_INTEGER),
		DEFINE_FIELD(CEnvRain, m_burstSize, FIELD_INTEGER),
		DEFINE_FIELD(CEnvRain, m_brightness, FIELD_INTEGER),
		DEFINE_FIELD(CEnvRain, m_flUpdateTime, FIELD_FLOAT),
		DEFINE_FIELD(CEnvRain, m_flMaxUpdateTime, FIELD_FLOAT),
		DEFINE_FIELD(CEnvRain, m_iszSpriteName, FIELD_STRING),
		DEFINE_FIELD(CEnvRain, m_axis, FIELD_INTEGER),
		DEFINE_FIELD(CEnvRain, m_iExtent, FIELD_INTEGER),
		DEFINE_FIELD(CEnvRain, m_fLifeTime, FIELD_FLOAT),
		DEFINE_FIELD(CEnvRain, m_iNoise, FIELD_INTEGER),
		//	DEFINE_FIELD( CEnvRain, m_pBeams, FIELD_CLASSPTR, MAX_RAIN_BEAMS ),
};

IMPLEMENT_SAVERESTORE(CEnvRain, CBaseEntity);

void CEnvRain::Precache()
{
	m_spriteTexture = PRECACHE_MODEL((char*)STRING(m_iszSpriteName));
}

bool CEnvRain::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "m_dripSize"))
	{
		m_dripSize = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_burstSize"))
	{
		m_burstSize = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_dripSpeed"))
	{
		int temp = atoi(pkvd->szValue);
		m_maxDripSpeed = temp + (temp / 4);
		m_minDripSpeed = temp - (temp / 4);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_brightness"))
	{
		m_brightness = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_flUpdateTime"))
	{
		m_flUpdateTime = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_flMaxUpdateTime"))
	{
		m_flMaxUpdateTime = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "pitch"))
	{
		m_pitch = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "texture"))
	{
		m_iszSpriteName = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_axis"))
	{
		m_axis = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iExtent"))
	{
		m_iExtent = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_fLifeTime"))
	{
		m_fLifeTime = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iNoise"))
	{
		m_iNoise = atoi(pkvd->szValue);
		return true;
	}
	return CBaseEntity::KeyValue(pkvd);
}

void CEnvRain::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (!ShouldToggle(useType))
		return;

	if (m_iState == STATE_ON)
	{
		m_iState = STATE_OFF;
		DontThink();
	}
	else
	{
		m_iState = STATE_ON;
		SetNextThink(0.1);
	}
}

#define SF_RAIN_START_OFF 1

void CEnvRain::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), STRING(pev->model)); // Set size
	pev->solid = SOLID_NOT;
	pev->effects = EF_NODRAW;

	if (pev->rendercolor == g_vecZero)
		pev->rendercolor = Vector(255, 255, 255);

	if (m_pitch)
		pev->angles.x = m_pitch;
	//	else if (pev->angles.x == 0) // don't allow horizontal rain.  //AJH -Why not?
	//	pev->angles.x = 90;

	if (m_burstSize == 0) // in case the level designer forgot to set it.
		m_burstSize = 2;

	if (pev->spawnflags & SF_RAIN_START_OFF)
		m_iState = STATE_OFF;
	else
	{
		m_iState = STATE_ON;
		SetNextThink(0.1);
	}
}

void CEnvRain::Think()
{
	//	ALERT(at_console,"RainThink %d %d %d %s\n",m_spriteTexture,m_dripSize,m_brightness,STRING(m_iszSpriteName));
	Vector vecSrc;
	Vector vecDest;

	UTIL_MakeVectors(pev->angles);
	Vector vecOffs = gpGlobals->v_forward;
	switch (m_axis)
	{
	case AXIS_X:
		vecOffs = vecOffs * (pev->size.x / vecOffs.x);
		break;
	case AXIS_Y:
		vecOffs = vecOffs * (pev->size.y / vecOffs.y);
		break;
	case AXIS_Z:
		vecOffs = vecOffs * (pev->size.z / vecOffs.z);
		break;
	}

	//	ALERT(at_console,"RainThink offs.z = %f, size.z = %f\n",vecOffs.z,pev->size.z);

	int repeats;
	if (!m_fLifeTime && !m_flUpdateTime && !m_flMaxUpdateTime)
		repeats = m_burstSize * 3;
	else
		repeats = m_burstSize;

	int drawn = 0;
	int tries = 0;
	TraceResult tr;
	bool bDraw;

	while (drawn < repeats && tries < (repeats * 3))
	{
		tries++;
		if (m_axis == AXIS_X)
			vecSrc.x = pev->maxs.x;
		else
			vecSrc.x = pev->mins.x + RANDOM_LONG(0, pev->size.x);
		if (m_axis == AXIS_Y)
			vecSrc.y = pev->maxs.y;
		else
			vecSrc.y = pev->mins.y + RANDOM_LONG(0, pev->size.y);
		if (m_axis == AXIS_Z)
			vecSrc.z = pev->maxs.z;
		else
			vecSrc.z = pev->mins.z + RANDOM_LONG(0, pev->size.z);
		vecDest = vecSrc - vecOffs;
		bDraw = true;

		switch (m_iExtent)
		{
		case EXTENT_OBSTRUCTED:
			UTIL_TraceLine(vecSrc, vecDest, ignore_monsters, NULL, &tr);
			vecDest = tr.vecEndPos;
			break;
		case EXTENT_OBSTRUCTED_REVERSE:
			UTIL_TraceLine(vecDest, vecSrc, ignore_monsters, NULL, &tr);
			vecSrc = tr.vecEndPos;
			break;
		case EXTENT_ARCING:
			UTIL_TraceLine(vecSrc, vecDest, ignore_monsters, NULL, &tr);
			if (tr.flFraction == 1.0)
				bDraw = false;
			vecDest = tr.vecEndPos;
			break;
		case EXTENT_ARCING_THROUGH: //AJH - Arcs full length of brush only when blocked
			UTIL_TraceLine(vecDest, vecSrc, dont_ignore_monsters, NULL, &tr);
			if (tr.flFraction == 1.0)
				bDraw = false;
			break;
		case EXTENT_ARCING_REVERSE:
			UTIL_TraceLine(vecDest, vecSrc, ignore_monsters, NULL, &tr);
			if (tr.flFraction == 1.0)
				bDraw = false;
			vecSrc = tr.vecEndPos;
			break;
		}
		//		vecDest.z = pev->mins.z;
		if (!bDraw)
			continue;

		drawn++;

		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_BEAMPOINTS);
		WRITE_COORD(vecDest.x);
		WRITE_COORD(vecDest.y);
		WRITE_COORD(vecDest.z);
		WRITE_COORD(vecSrc.x);
		WRITE_COORD(vecSrc.y);
		WRITE_COORD(vecSrc.z);
		WRITE_SHORT(m_spriteTexture);
		WRITE_BYTE((int)0); // framestart
		WRITE_BYTE((int)0); // framerate
		if (m_fLifeTime)	// life
			WRITE_BYTE((int)(m_fLifeTime * 10));
		else if (m_flMaxUpdateTime)
			WRITE_BYTE((int)(RANDOM_FLOAT(m_flUpdateTime, m_flMaxUpdateTime) * 30));
		else
			WRITE_BYTE((int)(m_flUpdateTime * 30));					  // life
		WRITE_BYTE(m_dripSize);										  // width
		WRITE_BYTE(m_iNoise);										  // noise
		WRITE_BYTE((int)pev->rendercolor.x);						  // r,
		WRITE_BYTE((int)pev->rendercolor.y);						  //    g,
		WRITE_BYTE((int)pev->rendercolor.z);						  //       b
		WRITE_BYTE(m_brightness);									  // brightness
		WRITE_BYTE((int)RANDOM_LONG(m_minDripSpeed, m_maxDripSpeed)); // speed
		MESSAGE_END();
	}

	// drawn will be false if we didn't draw anything.
	if (pev->target && drawn)
		FireTargets(STRING(pev->target), this, this, USE_TOGGLE, 0);

	if (m_flMaxUpdateTime)
		SetNextThink(RANDOM_FLOAT(m_flMaxUpdateTime, m_flUpdateTime));
	else if (m_flUpdateTime)
		SetNextThink(m_flUpdateTime);
}

//==================================================================
//LRC- Xen monsters' warp-in effect, for those too lazy to build it. :)
//==================================================================
class CEnvWarpBall : public CBaseEntity
{
public:
	void Precache() override;
	void Spawn() override { Precache(); }
	void Think() override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	int ObjectCaps() override { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
};

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

//==================================================================
//LRC- Shockwave effect, like when a Houndeye attacks.
//==================================================================
#define SF_SHOCKWAVE_CENTERED 1
#define SF_SHOCKWAVE_REPEATABLE 2

class CEnvShockwave : public CPointEntity
{
public:
	void Precache() override;
	void Spawn() override { Precache(); }
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	bool KeyValue(KeyValueData* pkvd) override;
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	void DoEffect(Vector vecPos);

	int m_iTime;
	int m_iRadius;
	int m_iHeight;
	int m_iScrollRate;
	int m_iNoise;
	int m_iFrameRate;
	int m_iStartFrame;
	int m_iSpriteTexture;
	char m_cType;
	int m_iszPosition;
};

LINK_ENTITY_TO_CLASS(env_shockwave, CEnvShockwave);

TYPEDESCRIPTION CEnvShockwave::m_SaveData[] =
	{
		DEFINE_FIELD(CEnvShockwave, m_iHeight, FIELD_INTEGER),
		DEFINE_FIELD(CEnvShockwave, m_iTime, FIELD_INTEGER),
		DEFINE_FIELD(CEnvShockwave, m_iRadius, FIELD_INTEGER),
		DEFINE_FIELD(CEnvShockwave, m_iScrollRate, FIELD_INTEGER),
		DEFINE_FIELD(CEnvShockwave, m_iNoise, FIELD_INTEGER),
		DEFINE_FIELD(CEnvShockwave, m_iFrameRate, FIELD_INTEGER),
		DEFINE_FIELD(CEnvShockwave, m_iStartFrame, FIELD_INTEGER),
		DEFINE_FIELD(CEnvShockwave, m_iSpriteTexture, FIELD_INTEGER),
		DEFINE_FIELD(CEnvShockwave, m_cType, FIELD_CHARACTER),
		DEFINE_FIELD(CEnvShockwave, m_iszPosition, FIELD_STRING),
};

IMPLEMENT_SAVERESTORE(CEnvShockwave, CBaseEntity);

void CEnvShockwave::Precache()
{
	m_iSpriteTexture = PRECACHE_MODEL((char*)STRING(pev->netname));
}

bool CEnvShockwave::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "m_iTime"))
	{
		m_iTime = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iRadius"))
	{
		m_iRadius = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iHeight"))
	{
		m_iHeight = atoi(pkvd->szValue) / 2; //LRC- the actual height is doubled when drawn
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iScrollRate"))
	{
		m_iScrollRate = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iNoise"))
	{
		m_iNoise = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iFrameRate"))
	{
		m_iFrameRate = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iStartFrame"))
	{
		m_iStartFrame = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszPosition"))
	{
		m_iszPosition = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_cType"))
	{
		m_cType = atoi(pkvd->szValue);
		return true;
	}
	return CBaseEntity::KeyValue(pkvd);
}

void CEnvShockwave::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	Vector vecPos;
	if (m_iszPosition)
		vecPos = CalcLocus_Position(this, pActivator, STRING(m_iszPosition));
	else
		vecPos = pev->origin;

	if (!(pev->spawnflags & SF_SHOCKWAVE_CENTERED))
		vecPos.z += m_iHeight;

	if (pev->target)
		FireTargets(STRING(pev->target), pActivator, pCaller, useType, value); //AJH

	// blast circle
	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
	if (m_cType)
		WRITE_BYTE(m_cType);
	else
		WRITE_BYTE(TE_BEAMCYLINDER);
	WRITE_COORD(vecPos.x); // coord coord coord (center position)
	WRITE_COORD(vecPos.y);
	WRITE_COORD(vecPos.z);
	WRITE_COORD(vecPos.x); // coord coord coord (axis and radius)
	WRITE_COORD(vecPos.y);
	WRITE_COORD(vecPos.z + m_iRadius);
	WRITE_SHORT(m_iSpriteTexture);	// short (sprite index)
	WRITE_BYTE(m_iStartFrame);		// byte (starting frame)
	WRITE_BYTE(m_iFrameRate);		// byte (frame rate in 0.1's)
	WRITE_BYTE(m_iTime);			// byte (life in 0.1's)
	WRITE_BYTE(m_iHeight);			// byte (line width in 0.1's)
	WRITE_BYTE(m_iNoise);			// byte (noise amplitude in 0.01's)
	WRITE_BYTE(pev->rendercolor.x); // byte,byte,byte (color)
	WRITE_BYTE(pev->rendercolor.y);
	WRITE_BYTE(pev->rendercolor.z);
	WRITE_BYTE(pev->renderamt); // byte (brightness)
	WRITE_BYTE(m_iScrollRate);	// byte (scroll speed in 0.1's)
	MESSAGE_END();

	if (!(pev->spawnflags & SF_SHOCKWAVE_REPEATABLE))
	{
		SetThink(&CEnvShockwave::SUB_Remove);
		SetNextThink(0);
	}
}

//==================================================================
//LRC- env_dlight; Dynamic Entity Light creator
//==================================================================
#define SF_DLIGHT_ONLYONCE 1
#define SF_DLIGHT_STARTON 2
class CEnvDLight : public CPointEntity
{
public:
	void PostSpawn() override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	void Think() override;
	void DesiredAction() override;
	virtual void MakeLight(bool bActive);
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];
	STATE GetState() override
	{
		if (pev->spawnflags & SF_DLIGHT_STARTON)
			return STATE_ON;
		else
			return STATE_OFF;
	}

	Vector m_vecPos;
	int m_iKey;
	static int ms_iNextFreeKey;
};

LINK_ENTITY_TO_CLASS(env_dlight, CEnvDLight);

TYPEDESCRIPTION CEnvDLight::m_SaveData[] =
	{
		DEFINE_FIELD(CEnvDLight, m_vecPos, FIELD_VECTOR),
		DEFINE_FIELD(CEnvDLight, m_iKey, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CEnvDLight, CPointEntity);

int CEnvDLight::ms_iNextFreeKey = 1;

void CEnvDLight::PostSpawn()
{
	// each env_dlight uses its own key to reference the light on the client
	m_iKey = ms_iNextFreeKey;
	ms_iNextFreeKey++;

	if (FStringNull(pev->targetname) || pev->spawnflags & SF_DLIGHT_STARTON)
	{
		UTIL_DesiredAction(this);
	}
}

void CEnvDLight::DesiredAction()
{
	pev->spawnflags &= ~SF_DLIGHT_STARTON;
	Use(this, this, USE_ON, 0);
}

void CEnvDLight::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (!ShouldToggle(useType))
	{
		return;
	}
	if (GetState() == STATE_ON)
	{
		// turn off
		MakeLight(false);
		pev->spawnflags &= ~SF_DLIGHT_STARTON;
		DontThink();
		return;
	}

	if (pev->message)
	{
		m_vecPos = CalcLocus_Position(this, pActivator, STRING(pev->message));
	}
	else
	{
		m_vecPos = pev->origin;
	}

	// turn on
	MakeLight(true);
	pev->spawnflags |= SF_DLIGHT_STARTON;

	if (pev->health)
	{
		SetNextThink(pev->health);
	}
	else
	{
		if (pev->spawnflags & SF_DLIGHT_ONLYONCE)
		{
			SetThink(&CEnvDLight::SUB_Remove);
			SetNextThink(0);
		}
	}
}

void CEnvDLight::MakeLight(bool bActive)
{
	//	MESSAGE_BEGIN( MSG_ALL, gmsgKeyedDLight, NULL );
	MESSAGE_BEGIN(MSG_ALL, gmsgKeyedELight, NULL);
	WRITE_BYTE(m_iKey);
	WRITE_BYTE(bActive); // visible?
	if (bActive)
	{
		WRITE_COORD(m_vecPos.x);		// X
		WRITE_COORD(m_vecPos.y);		// Y
		WRITE_COORD(m_vecPos.z);		// Z
		WRITE_BYTE(pev->renderamt);		// radius * 0.1
		WRITE_BYTE(pev->rendercolor.x); // r
		WRITE_BYTE(pev->rendercolor.y); // g
		WRITE_BYTE(pev->rendercolor.z); // b
	}
	MESSAGE_END();
}

void CEnvDLight::Think()
{
	// turn off the light
	MakeLight(false);
	pev->spawnflags &= ~SF_DLIGHT_STARTON;

	if (pev->spawnflags & SF_DLIGHT_ONLYONCE)
	{
		SetThink(&CEnvDLight::SUB_Remove);
		SetNextThink(0);
	}
}


//==================================================================
//LRC- env_elight; Dynamic Entity Light creator
//==================================================================
class CEnvELight : public CEnvDLight
{
public:
	void PostSpawn() override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	void MakeLight(bool bActive) override;
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	EHANDLE m_hAttach;
	int m_iKey;
	static int ms_iNextFreeKey;
};

LINK_ENTITY_TO_CLASS(env_elight, CEnvELight);

int CEnvELight::ms_iNextFreeKey = 1;

TYPEDESCRIPTION CEnvELight::m_SaveData[] =
	{
		DEFINE_FIELD(CEnvELight, m_hAttach, FIELD_EHANDLE),
		DEFINE_FIELD(CEnvELight, m_iKey, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CEnvELight, CEnvDLight);

void CEnvELight::PostSpawn()
{
	// each env_elight uses its own key to reference the light on the client
	m_iKey = ms_iNextFreeKey;
	ms_iNextFreeKey++;
}

void CEnvELight::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (pev->target)
	{
		m_hAttach = UTIL_FindEntityByTargetname(NULL, STRING(pev->target), pActivator);
		if (m_hAttach == NULL)
		{
			ALERT(at_console, "env_elight \"%s\" can't find target %s\n", STRING(pev->targetname), STRING(pev->target));
			return; // error?
		}
	}
	else
	{
		m_hAttach = this;
	}

	CEnvDLight::Use(pActivator, pCaller, useType, value);
}

void CEnvELight::MakeLight(bool bActive)
{
	if (m_hAttach == NULL)
	{
		DontThink();
		pev->takedamage = 0;
		return;
	}

	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
	WRITE_BYTE(TE_ELIGHT);
	WRITE_SHORT(m_hAttach->entindex() + 0x1000 * pev->impulse); // entity, attachment
	WRITE_COORD(m_vecPos.x);									// X
	WRITE_COORD(m_vecPos.y);									// Y
	WRITE_COORD(m_vecPos.z);									// Z
	WRITE_COORD(pev->renderamt);								// radius * 0.1
	WRITE_BYTE(pev->rendercolor.x);								// r
	WRITE_BYTE(pev->rendercolor.y);								// g
	WRITE_BYTE(pev->rendercolor.z);								// b
	//WRITE_BYTE( iTime );				// time * 10
	WRITE_BYTE(pev->health); // time * 10
	WRITE_COORD(pev->frags); // decay * 0.1
	MESSAGE_END();
}


//=========================================================
// LRC - Decal effect
//=========================================================
class CEnvDecal : public CPointEntity
{
public:
	void Spawn() override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
};

LINK_ENTITY_TO_CLASS(env_decal, CEnvDecal);

void CEnvDecal::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	int iTexture = 0;

	switch (pev->impulse)
	{
	case 1:
		iTexture = DECAL_GUNSHOT1 + RANDOM_LONG(0, 4);
		break;
	case 2:
		iTexture = DECAL_BLOOD1 + RANDOM_LONG(0, 5);
		break;
	case 3:
		iTexture = DECAL_YBLOOD1 + RANDOM_LONG(0, 5);
		break;
	case 4:
		iTexture = DECAL_GLASSBREAK1 + RANDOM_LONG(0, 2);
		break;
	case 5:
		iTexture = DECAL_BIGSHOT1 + RANDOM_LONG(0, 4);
		break;
	case 6:
		iTexture = DECAL_SCORCH1 + RANDOM_LONG(0, 1);
		break;
	case 7:
		iTexture = DECAL_SPIT1 + RANDOM_LONG(0, 1);
		break;
	}

	if (pev->impulse)
		iTexture = gDecals[iTexture].index;
	else
		iTexture = pev->skin; // custom texture

	Vector vecPos;
	if (!FStringNull(pev->target))
		vecPos = CalcLocus_Position(this, pActivator, STRING(pev->target));
	else
		vecPos = pev->origin;

	Vector vecOffs;
	if (!FStringNull(pev->netname))
		vecOffs = CalcLocus_Velocity(this, pActivator, STRING(pev->netname));
	else
	{
		UTIL_MakeVectors(pev->angles);
		vecOffs = gpGlobals->v_forward;
	}

	if (pev->message)
		vecOffs = vecOffs * CalcLocus_Number(pActivator, STRING(pev->message));
	else
		vecOffs = vecOffs.Normalize() * 4000;


	TraceResult trace;
	int entityIndex;

	UTIL_TraceLine(vecPos, vecPos + vecOffs, ignore_monsters, NULL, &trace);

	if (trace.flFraction == 1.0)
		return; // didn't hit anything, oh well

	entityIndex = (short)ENTINDEX(trace.pHit);

	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(TE_BSPDECAL);
	WRITE_COORD(trace.vecEndPos.x);
	WRITE_COORD(trace.vecEndPos.y);
	WRITE_COORD(trace.vecEndPos.z);
	WRITE_SHORT(iTexture);
	WRITE_SHORT(entityIndex);
	if (entityIndex)
		WRITE_SHORT((int)VARS(trace.pHit)->modelindex);
	MESSAGE_END();
}

void CEnvDecal::Spawn()
{
	if (pev->impulse == 0)
	{
		pev->skin = DECAL_INDEX(STRING(pev->noise));

		if (pev->skin == 0)
			ALERT(at_debug, "locus_decal \"%s\" can't find decal \"%s\"\n", STRING(pev->noise));
	}
}


//=========================================================
// Beverage Dispenser
// overloaded pev->frags, is now a flag for whether or not a can is stuck in the dispenser.
// overloaded pev->health, is now how many cans remain in the machine.
//=========================================================
class CEnvBeverage : public CBaseDelay
{
public:
	void Spawn() override;
	void Precache() override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;

	// it's 'on' while there are cans left
	STATE GetState() override { return (pev->health > 0) ? STATE_ON : STATE_OFF; };
};

void CEnvBeverage::Precache()
{
	PRECACHE_MODEL("models/can.mdl");
	PRECACHE_SOUND("weapons/g_bounce3.wav");
}

LINK_ENTITY_TO_CLASS(env_beverage, CEnvBeverage);

void CEnvBeverage::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (pev->frags != 0 || pev->health <= 0)
	{
		// no more cans while one is waiting in the dispenser, or if I'm out of cans.
		return;
	}

	Vector vecPos;
	if (pev->target)
		vecPos = CalcLocus_Position(this, pActivator, STRING(pev->target));
	else
		vecPos = pev->origin;

	CBaseEntity* pCan = CBaseEntity::Create("item_sodacan", vecPos, pev->angles, edict());

	if (pev->skin == 6)
	{
		// random
		pCan->pev->skin = RANDOM_LONG(0, 5);
	}
	else
	{
		pCan->pev->skin = pev->skin;
	}

	pev->frags = 1;
	pev->health--;
}

void CEnvBeverage::Spawn()
{
	Precache();
	pev->solid = SOLID_NOT;
	pev->effects = EF_NODRAW;
	pev->frags = 0;

	if (pev->health == 0)
	{
		pev->health = 10;
	}
}

//=========================================================
// Soda can
//=========================================================
class CItemSoda : public CBaseEntity
{
public:
	void Spawn() override;
	void Precache() override;
	void EXPORT CanThink();
	void EXPORT CanTouch(CBaseEntity* pOther);
};

void CItemSoda::Precache()
{
	// added for Nemo1024  --LRC
	PRECACHE_MODEL("models/can.mdl");
	PRECACHE_SOUND("weapons/g_bounce3.wav");
}

LINK_ENTITY_TO_CLASS(item_sodacan, CItemSoda);

void CItemSoda::Spawn()
{
	Precache();
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_TOSS;

	SET_MODEL(ENT(pev), "models/can.mdl");
	UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));

	SetThink(&CItemSoda::CanThink);
	SetNextThink(0.5);
}

void CItemSoda::CanThink()
{
	EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/g_bounce3.wav", 1, ATTN_NORM);

	pev->solid = SOLID_TRIGGER;
	UTIL_SetSize(pev, Vector(-8, -8, 0), Vector(8, 8, 8));
	SetThink(NULL);
	SetTouch(&CItemSoda::CanTouch);
}

void CItemSoda::CanTouch(CBaseEntity* pOther)
{
	if (!pOther->IsPlayer())
	{
		return;
	}

	// spoit sound here

	pOther->TakeHealth(1, DMG_GENERIC); // a bit of health.

	if (!FNullEnt(pev->owner))
	{
		// tell the machine the can was taken
		pev->owner->v.frags = 0;
	}

	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	pev->effects = EF_NODRAW;
	SetTouch(NULL);
	SetThink(&CItemSoda::SUB_Remove);
	SetNextThink(0);
}

//=========================================================
// LRC - env_fog, extended a bit from the DMC version
//=========================================================
#define SF_FOG_ACTIVE 1
#define SF_FOG_FADING 0x8000

class CEnvFog : public CPointEntity
{
public:
	void Spawn() override;
	void Precache() override;
	void EXPORT ResumeThink();
	void EXPORT Resume2Think();
	void EXPORT TurnOn();
	void EXPORT TurnOff();
	void EXPORT FadeInDone();
	void EXPORT FadeOutDone();
	void SendData(Vector col, int fFadeTime, int StartDist, int iEndDist);
	bool KeyValue(KeyValueData* pkvd) override;
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;

	STATE GetState() override;

	int m_iStartDist;
	int m_iEndDist;
	float m_iFadeIn;
	float m_iFadeOut;
	float m_fHoldTime;
	float m_fFadeStart; // if we're fading in/out, then when did the fade start?
};

TYPEDESCRIPTION CEnvFog::m_SaveData[] =
	{
		DEFINE_FIELD(CEnvFog, m_iStartDist, FIELD_INTEGER),
		DEFINE_FIELD(CEnvFog, m_iEndDist, FIELD_INTEGER),
		DEFINE_FIELD(CEnvFog, m_iFadeIn, FIELD_INTEGER),
		DEFINE_FIELD(CEnvFog, m_iFadeOut, FIELD_INTEGER),
		DEFINE_FIELD(CEnvFog, m_fHoldTime, FIELD_FLOAT),
		DEFINE_FIELD(CEnvFog, m_fFadeStart, FIELD_TIME),
};

IMPLEMENT_SAVERESTORE(CEnvFog, CBaseEntity);

bool CEnvFog::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "startdist"))
	{
		m_iStartDist = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "enddist"))
	{
		m_iEndDist = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "fadein"))
	{
		m_iFadeIn = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "fadeout"))
	{
		m_iFadeOut = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "holdtime"))
	{
		m_fHoldTime = atof(pkvd->szValue);
		return true;
	}
	return CBaseEntity::KeyValue(pkvd);
}

STATE CEnvFog::GetState()
{
	if (pev->spawnflags & SF_FOG_ACTIVE)
	{
		if (pev->spawnflags & SF_FOG_FADING)
			return STATE_TURN_ON;
		else
			return STATE_ON;
	}
	else
	{
		if (pev->spawnflags & SF_FOG_FADING)
			return STATE_TURN_OFF;
		else
			return STATE_OFF;
	}
}

void CEnvFog::Spawn()
{
	pev->effects |= EF_NODRAW;

	if (pev->targetname == 0)
		pev->spawnflags |= SF_FOG_ACTIVE;

	if (pev->spawnflags & SF_FOG_ACTIVE)
	{
		SetThink(&CEnvFog::TurnOn);
		UTIL_DesiredThink(this);
	}

	// Precache is now used only to continue after a game has loaded.
	//	Precache();

	// things get messed up if we try to draw fog with a startdist
	// or an enddist of 0, so we don't allow it.
	if (m_iStartDist == 0)
		m_iStartDist = 1;
	if (m_iEndDist == 0)
		m_iEndDist = 1;
}

void CEnvFog::Precache()
{
	if (pev->spawnflags & SF_FOG_ACTIVE)
	{
		SetThink(&CEnvFog::ResumeThink);
		SetNextThink(0.1);
	}
}

extern int gmsgSetFog;

void CEnvFog::TurnOn()
{
	//	ALERT(at_console, "Fog turnon %f\n", gpGlobals->time);

	pev->spawnflags |= SF_FOG_ACTIVE;

	if (m_iFadeIn)
	{
		pev->spawnflags |= SF_FOG_FADING;
		SendData(pev->rendercolor, m_iFadeIn, m_iStartDist, m_iEndDist);
		SetNextThink(m_iFadeIn);
		SetThink(&CEnvFog::FadeInDone);
	}
	else
	{
		pev->spawnflags &= ~SF_FOG_FADING;
		SendData(pev->rendercolor, 0, m_iStartDist, m_iEndDist);
		if (m_fHoldTime)
		{
			SetNextThink(m_fHoldTime);
			SetThink(&CEnvFog::TurnOff);
		}
	}
}

void CEnvFog::TurnOff()
{
	//	ALERT(at_console, "Fog turnoff\n");

	pev->spawnflags &= ~SF_FOG_ACTIVE;

	if (m_iFadeOut)
	{
		pev->spawnflags |= SF_FOG_FADING;
		SendData(pev->rendercolor, -m_iFadeOut, m_iStartDist, m_iEndDist);
		SetNextThink(m_iFadeOut);
		SetThink(&CEnvFog::FadeOutDone);
	}
	else
	{
		pev->spawnflags &= ~SF_FOG_FADING;
		SendData(g_vecZero, 0, 0, 0);
		DontThink();
	}
}

//yes, this intermediate think function is necessary.
// the engine seems to ignore the nextthink time when starting up.
// So this function gets called immediately after the precache finishes,
// regardless of what nextthink time is specified.
void CEnvFog::ResumeThink()
{
	//	ALERT(at_console, "Fog resume %f\n", gpGlobals->time);
	SetThink(&CEnvFog ::Resume2Think);
	SetNextThink(0.1);
}

void CEnvFog ::Resume2Think()
{
	SendData(pev->rendercolor, m_iFadeIn, m_iStartDist, m_iEndDist);
}

void CEnvFog::FadeInDone()
{
	pev->spawnflags &= ~SF_FOG_FADING;
	//LRC 1.8 we don't need to resend...	SendData( pev->rendercolor, 0, m_iStartDist, m_iEndDist);

	if (m_fHoldTime)
	{
		SetNextThink(m_fHoldTime);
		SetThink(&CEnvFog::TurnOff);
	}
}

void CEnvFog::FadeOutDone()
{
	pev->spawnflags &= ~SF_FOG_FADING;
	SendData(g_vecZero, 0, 0, 0);
}

void CEnvFog::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	//	ALERT(at_console, "Fog use %s %s\n", GetStringForUseType(useType), GetStringForState(GetState()));
	if (ShouldToggle(useType))
	{
		if (pev->spawnflags & SF_FOG_ACTIVE)
			TurnOff();
		else
			TurnOn();
	}
}

void CEnvFog::SendData(Vector col, int iFadeTime, int iStartDist, int iEndDist)
{
	//	ALERT(at_console, "Fog send (%d %d %d), %d - %d\n", col.x, col.y, col.z, iStartDist, iEndDist);
	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CBasePlayer* pPlayer = (CBasePlayer*)UTIL_PlayerByIndex(i);
		if (pPlayer)
		{
			MESSAGE_BEGIN(MSG_ONE, gmsgSetFog, NULL, pPlayer->pev);
			WRITE_BYTE(col.x);
			WRITE_BYTE(col.y);
			WRITE_BYTE(col.z);
			WRITE_SHORT(iFadeTime);
			WRITE_SHORT(iStartDist);
			WRITE_SHORT(iEndDist);
			MESSAGE_END();

			//			pPlayer->m_iFogStartDist = iStartDist;
			//			pPlayer->m_iFogEndDist = iEndDist;
			//			pPlayer->m_vecFogColor = col;
			//			pPlayer->m_bClientFogRefresh = false;
		}
	}
}

LINK_ENTITY_TO_CLASS(env_fog, CEnvFog);

//=========================================================
// LRC - env_sky, an unreal tournament-style sky effect
//=========================================================
class CEnvSky : public CBaseEntity
{
public:
	void Activate() override;
	void DesiredAction() override;
};

void CEnvSky ::Activate()
{
	UTIL_DesiredAction(this);
	pev->effects |= EF_NODRAW;
	pev->nextthink = gpGlobals->time + 1.0;
}

extern int gmsgSetSky;

void CEnvSky ::DesiredAction()
{
	MESSAGE_BEGIN(MSG_BROADCAST, gmsgSetSky, NULL);
	WRITE_BYTE(1);				// mode
	WRITE_COORD(pev->origin.x); // view position
	WRITE_COORD(pev->origin.y);
	WRITE_COORD(pev->origin.z);

	//AJH scale of the skybox 1/x (0=infinitly large/far away = no parallax)
	//No parallax is the default behaviour. FGD's can set a new default.
	WRITE_BYTE(pev->frags);
	//WRITE_BYTE(ENTINDEX(edict()));

	MESSAGE_END();
}

LINK_ENTITY_TO_CLASS(env_sky, CEnvSky);



//=========================================================
// LRC - env_particle, uses the aurora particle system
//=========================================================
//extern int gmsgParticle = 0;
#define SF_PARTICLE_ON 1
#define SF_PARTICLE_SPAWNUSE 2 //AJH for spawnable env_particles

class CParticle : public CPointEntity
{
public:
	void Spawn() override;
	void Activate() override;
	void Precache() override;
	void DesiredAction() override;
	void EXPORT Think() override;

	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
};

LINK_ENTITY_TO_CLASS(env_particle, CParticle);

void CParticle::Spawn(void)
{
	pev->solid = SOLID_NOT;

	//	pev->movetype		= MOVETYPE_NONE;	//AJH
	pev->movetype = MOVETYPE_NOCLIP; //AJH

	pev->renderamt = 128;
	pev->rendermode = kRenderTransTexture;

	// 'body' determines whether the effect is active or not //AJH only if we have a targetname
	pev->body = pev->targetname ? (pev->spawnflags & SF_PARTICLE_ON) != 0 : 1;

	Precache();

	UTIL_SetOrigin(this, pev->origin);
	SET_MODEL(edict(), "sprites/null.spr");
}


void CParticle::Precache(void)
{
	PRECACHE_MODEL("sprites/null.spr");
}

void CParticle::Activate(void)
{
	CPointEntity::Activate();
	UTIL_DesiredAction(this);
}

void CParticle::DesiredAction()
{
	pev->nextthink = gpGlobals->time + 1;
}

void CParticle::Think()
{
	MESSAGE_BEGIN(MSG_ALL, gmsgParticle);
	WRITE_SHORT(entindex());
	WRITE_STRING(STRING(pev->message));
	MESSAGE_END();
}

void CParticle::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (pev->spawnflags & SF_PARTICLE_SPAWNUSE || useType == USE_SPAWN)
	{ //AJH Spawnable env_particles!!

		// Create a new entity with Cparticle private data
		CParticle* pParticle = GetClassPtr((CParticle*)NULL);
		pParticle->pev->classname = MAKE_STRING("particle");

		if (pev->netname != NULL)
		{
			pParticle->pev->targetname = pev->netname; // set childrens name (targetname) from netname
		}

		pParticle->pev->message = pev->message;
		pParticle->pev->origin = pActivator->pev->origin;
		pParticle->pev->angles = pActivator->pev->angles;
		pParticle->Spawn();
		pParticle->pev->body = 1; //turn children on automatically
		pParticle->Think();

		//	ALERT(at_debug,"Particle %s spawned new particle %s\n",STRING(pev->targetname),STRING(pParticle->pev->targetname));
	}
	else
	{ //AJH Standard non spawnuse USE function

		if (ShouldToggle(useType, pev->body))
		{
			pev->body = !pev->body;
			//ALERT(at_console, "Toggling Particle on/off %d\n", pev->body);
		}
	}
}

//=========================================================
// G-Cont - env_mirror, mirroring only models
//=========================================================

#define SF_MIRROR_DRAWPLAYER 0x01

class CEnvMirror : public CBaseEntity
{
public:
	void Spawn(void) override;
	void Precache(void) override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	bool KeyValue(KeyValueData* pkvd) override;
	int ObjectCaps(void) override { return CBaseEntity ::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	void EXPORT MirrorThink(void);
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	STATE GetState(void) override { return m_iActive ? STATE_ON : STATE_OFF; } //Support this stuff for watcher
	bool m_iActive;
	float m_flRadius;
	static TYPEDESCRIPTION m_SaveData[];

private:
	int m_iInitialRenderMode;
	bool bSent;
	unsigned short m_usMirror; //Moved from weapons.cpp for more struct code. G-Cont
};

TYPEDESCRIPTION CEnvMirror::m_SaveData[] =
	{
		DEFINE_FIELD(CEnvMirror, m_iInitialRenderMode, FIELD_INTEGER),
		DEFINE_FIELD(CEnvMirror, m_iActive, FIELD_INTEGER),
		DEFINE_FIELD(CEnvMirror, m_flRadius, FIELD_FLOAT),
};
IMPLEMENT_SAVERESTORE(CEnvMirror, CBaseEntity);

bool CEnvMirror ::KeyValue(KeyValueData* pkvd)
{

	if (FStrEq(pkvd->szKeyName, "radius"))
	{
		m_flRadius = atof(pkvd->szValue);
		return true;
	}

	return CBaseEntity::KeyValue(pkvd);
}

LINK_ENTITY_TO_CLASS(env_mirror, CEnvMirror);

void CEnvMirror ::Spawn(void)
{
	// G-Cont. added this for emulate shiny floor (without this flag monsters can't walk on it)
	if (!m_pMoveWith)
		pev->flags |= FL_WORLDBRUSH;

	pev->angles = g_vecZero;
	pev->movetype = MOVETYPE_PUSH; // so it doesn't get pushed by anything

	pev->solid = SOLID_BSP;

	pev->impulse = 1;
	m_iActive = true;

	Precache();

	SET_MODEL(ENT(pev), STRING(pev->model));
	SetThink(&CEnvMirror ::MirrorThink);
	if (pev->spawnflags & SF_MIRROR_DRAWPLAYER)
		CBaseEntity::Create("player_marker", VecBModelOrigin(pev), pev->angles, NULL);
	m_iInitialRenderMode = pev->rendermode;
	if (!m_flRadius)
		m_flRadius = 330;
	if (!pev->frags) //Smart field system. g-cont
	{
		if (pev->size.y > pev->size.x && pev->size.z > pev->size.x)
			pev->frags = 0;
		if (pev->size.x > pev->size.y && pev->size.z > pev->size.y)
			pev->frags = 1;
		if (pev->size.y > pev->size.z && pev->size.x > pev->size.z)
			pev->frags = 2;
	}

	SetNextThink(0.1);
}

void CEnvMirror ::Precache(void)
{
	if (pev->spawnflags & SF_MIRROR_DRAWPLAYER)
		UTIL_PrecacheOther("player_marker");
	m_usMirror = PRECACHE_EVENT(1, "events/mirror.sc");
	bSent = false;
}

void CEnvMirror ::MirrorThink(void)
{
	if (bSent)
	{
		SetNextThink(0.01);
		return;
	}

	if (pev->impulse)
	{
		if (pev->rendermode != m_iInitialRenderMode)
			pev->rendermode = m_iInitialRenderMode;

		if (UTIL_PlayerByIndex(1))
		{
			PLAYBACK_EVENT_FULL(FEV_RELIABLE | FEV_GLOBAL, edict(), m_usMirror, 0.0, (float*)&Center(), (float*)&g_vecZero, 0.0, 0.0, m_flRadius, pev->frags, 1, 0);
			bSent = true;
		}
	}
	else
	{
		if (pev->rendermode != kRenderNormal)
		{
			pev->rendermode = kRenderNormal;
			PLAYBACK_EVENT_FULL(FEV_RELIABLE | FEV_GLOBAL, edict(), m_usMirror, 0.0, (float*)&Center(), (float*)&g_vecZero, 0.0, 0.0, m_flRadius, pev->frags, 0, 0);
		}

		bSent = true;
	}
	SetNextThink(0.01);
}


void CEnvMirror ::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (ShouldToggle(useType, m_iActive))
		m_iActive = !m_iActive;

	if (m_iActive)
	{
		m_iActive = true;
		pev->impulse = 1;
		bSent = false;
	}
	else
	{
		m_iActive = false;
		pev->impulse = 0;
		bSent = false;
	}
}

//=========================================================
// G-Cont - env_rain, use triAPI
//=========================================================

void CRainSettings::Spawn()
{
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	pev->effects |= EF_NODRAW;
}

bool CRainSettings::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "m_flDistance"))
	{
		Rain_Distance = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iMode"))
	{
		Rain_Mode = atoi(pkvd->szValue);
		return true;
	}

	return CBaseEntity::KeyValue(pkvd);
}

LINK_ENTITY_TO_CLASS(rain_settings, CRainSettings);

TYPEDESCRIPTION CRainSettings::m_SaveData[] =
	{
		DEFINE_FIELD(CRainSettings, Rain_Distance, FIELD_FLOAT),
		DEFINE_FIELD(CRainSettings, Rain_Mode, FIELD_INTEGER),
};
IMPLEMENT_SAVERESTORE(CRainSettings, CBaseEntity);



void CRainModify::Spawn()
{
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	pev->effects |= EF_NODRAW;

	if (FStringNull(pev->targetname))
		pev->spawnflags |= 1;
}

bool CRainModify::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "m_iDripsPerSecond"))
	{
		Rain_Drips = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_flWindX"))
	{
		Rain_windX = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_flWindY"))
	{
		Rain_windY = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_flRandX"))
	{
		Rain_randX = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_flRandY"))
	{
		Rain_randY = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_flTime"))
	{
		fadeTime = atof(pkvd->szValue);
		return true;
	}

	return CBaseEntity::KeyValue(pkvd);
}

void CRainModify::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (pev->spawnflags & 1)
		return; // constant

	if (gpGlobals->deathmatch)
	{
		ALERT(at_console, "Rain error: only static rain in multiplayer\n");
		return; // not in multiplayer
	}

	CBasePlayer* pPlayer;
	pPlayer = (CBasePlayer*)CBaseEntity::Instance(g_engfuncs.pfnPEntityOfEntIndex(1));

	if (fadeTime)
	{ // write to 'ideal' settings
		pPlayer->Rain_ideal_dripsPerSecond = Rain_Drips;
		pPlayer->Rain_ideal_randX = Rain_randX;
		pPlayer->Rain_ideal_randY = Rain_randY;
		pPlayer->Rain_ideal_windX = Rain_windX;
		pPlayer->Rain_ideal_windY = Rain_windY;

		pPlayer->Rain_endFade = gpGlobals->time + fadeTime;
		pPlayer->Rain_nextFadeUpdate = gpGlobals->time + 1;
	}
	else
	{
		pPlayer->Rain_dripsPerSecond = Rain_Drips;
		pPlayer->Rain_randX = Rain_randX;
		pPlayer->Rain_randY = Rain_randY;
		pPlayer->Rain_windX = Rain_windX;
		pPlayer->Rain_windY = Rain_windY;

		pPlayer->Rain_needsUpdate = 1;
	}
}

LINK_ENTITY_TO_CLASS(rain_modify, CRainModify);

TYPEDESCRIPTION CRainModify::m_SaveData[] =
	{
		DEFINE_FIELD(CRainModify, fadeTime, FIELD_FLOAT),
		DEFINE_FIELD(CRainModify, Rain_Drips, FIELD_INTEGER),
		DEFINE_FIELD(CRainModify, Rain_randX, FIELD_FLOAT),
		DEFINE_FIELD(CRainModify, Rain_randY, FIELD_FLOAT),
		DEFINE_FIELD(CRainModify, Rain_windX, FIELD_FLOAT),
		DEFINE_FIELD(CRainModify, Rain_windY, FIELD_FLOAT),
};
IMPLEMENT_SAVERESTORE(CRainModify, CBaseEntity);
