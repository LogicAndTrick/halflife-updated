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

#include "CEnvShooter.h"
#include "entities/func/CBreakable.h"
#include "locus.h"
#include "monsters.h"

LINK_ENTITY_TO_CLASS(env_shooter, CEnvShooter);

TYPEDESCRIPTION CEnvShooter::m_SaveData[] =
	{
		DEFINE_FIELD(CEnvShooter, m_iszTouch, FIELD_STRING),
		DEFINE_FIELD(CEnvShooter, m_iszTouchOther, FIELD_STRING),
		DEFINE_FIELD(CEnvShooter, m_iPhysics, FIELD_INTEGER),
		DEFINE_FIELD(CEnvShooter, m_fFriction, FIELD_FLOAT),
		DEFINE_FIELD(CEnvShooter, m_vecSize, FIELD_VECTOR),
};

IMPLEMENT_SAVERESTORE(CEnvShooter, CGibShooter);

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
		pev->scale = CalcLocus_Number(this, STRING(pev->noise), 0); // AJH / MJB - allow locus_ratio for scale
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
			pGib->pev->scale = 1.0; // G-Cont. for fix with laarge gibs :) (MJB i know, but sometimes people deliberately want large models?)
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
		pShot->pev->movetype = MOVETYPE_BOUNCE; // G-Cont. fix for blank field m_iPhysics, e. g. - original HL
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

// -----------------------
// -----------------------
// -----------------------

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
