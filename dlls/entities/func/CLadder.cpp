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

#include "CLadder.h"

LINK_ENTITY_TO_CLASS(func_ladder, CLadder);

bool CLadder::KeyValue(KeyValueData* pkvd)
{
	return CBaseTrigger::KeyValue(pkvd);
}

//=========================================================
// func_ladder - makes an area vertically negotiable
//=========================================================
void CLadder::Precache()
{
	// Do all of this in here because we need to 'convert' old saved games
	pev->solid = SOLID_NOT;
	pev->skin = CONTENTS_LADDER;
	if (CVAR_GET_FLOAT("showtriggers") == 0 && !(pev->spawnflags & SF_LADDER_VISIBLE))
	{
		pev->effects |= EF_NODRAW;
		// LRC- NODRAW is a better-performance way to stop things being drawn.
		//  (unless... would it prevent client-side movement algorithms from working?)
		//		pev->rendermode = kRenderTransTexture;
		//		pev->renderamt = 0;
	}
	else
		pev->effects &= ~EF_NODRAW;
}

void CLadder::Spawn()
{
	Precache();

	SET_MODEL(ENT(pev), STRING(pev->model)); // set size and link into world
	pev->movetype = MOVETYPE_PUSH;
}
