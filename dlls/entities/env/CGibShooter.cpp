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

#include "CGibShooter.h"
#include "entities/CGib.h"
#include "locus.h"

LINK_ENTITY_TO_CLASS(gibshooter, CGibShooter);

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
