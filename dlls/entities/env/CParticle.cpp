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

#include "CParticle.h"
#include "movewith.h"
#include "UserMessages.h"

LINK_ENTITY_TO_CLASS(env_particle, CParticle);

void CParticle::Spawn(void)
{
	pev->solid = SOLID_NOT;

	//	pev->movetype		= MOVETYPE_NONE;	//AJH
	pev->movetype = MOVETYPE_NOCLIP; // AJH

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
	{ // AJH Spawnable env_particles!!

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
		pParticle->pev->body = 1; // turn children on automatically
		pParticle->Think();

		//	ALERT(at_debug,"Particle %s spawned new particle %s\n",STRING(pev->targetname),STRING(pParticle->pev->targetname));
	}
	else
	{ // AJH Standard non spawnuse USE function

		if (ShouldToggle(useType, pev->body))
		{
			pev->body = !pev->body;
			// ALERT(at_console, "Toggling Particle on/off %d\n", pev->body);
		}
	}
}
