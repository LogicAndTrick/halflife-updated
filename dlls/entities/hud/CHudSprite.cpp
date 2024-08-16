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

#include "CHudSprite.h"
#include "UserMessages.h"

LINK_ENTITY_TO_CLASS(hud_sprite, CHudSprite);

void CHudSprite::Spawn()
{
	// LRC 1.8
	//  We can't keep a hud.txt spritename in pev->model, because on loading a saved game, it
	//  tries to precache it as though it was a model file. (And crashes.)
	//  So now we keep them in pev->message... but for backwards compatibility,
	//  transfer the "model" field into "message", and clear model.
	if (FStringNull(pev->message))
	{
		pev->message = pev->model;
		pev->model = 0;
	}

	if (FStringNull(pev->targetname))
	{
		pev->spawnflags |= SF_HUDSPR_ACTIVE;
	}

	if (pev->spawnflags & SF_HUDSPR_ACTIVE)
	{
		SetNextThink(2);
	}
}

void CHudSprite::Think()
{
	Use(this, this, USE_ON, 0);
}

void CHudSprite::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (!pActivator || !pActivator->IsPlayer())
	{
		pActivator = CBaseEntity::Instance(g_engfuncs.pfnPEntityOfEntIndex(1));
	}

	if (ShouldToggle(useType))
	{
		if (pev->spawnflags & SF_HUDSPR_ACTIVE)
			pev->spawnflags &= ~SF_HUDSPR_ACTIVE;
		else
			pev->spawnflags |= SF_HUDSPR_ACTIVE;
	}

	//		byte   : true = ENABLE icon, false = DISABLE icon
	//		string : the sprite name to display
	//		byte   : red
	//		byte   : green
	//		byte   : blue
	MESSAGE_BEGIN(MSG_ONE, gmsgStatusIcon, NULL, pActivator->pev);
	WRITE_BYTE(pev->spawnflags & SF_HUDSPR_ACTIVE);
	WRITE_STRING(STRING(pev->message));
	WRITE_BYTE(pev->rendercolor.x);
	WRITE_BYTE(pev->rendercolor.y);
	WRITE_BYTE(pev->rendercolor.z);
	MESSAGE_END();
}
