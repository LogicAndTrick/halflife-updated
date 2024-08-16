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

#include "CCorpse.h"

LINK_ENTITY_TO_CLASS(bodyque, CCorpse);

edict_t* g_pBodyQueueHead = nullptr;

void InitBodyQue()
{
	string_t istrClassname = MAKE_STRING("bodyque");

	g_pBodyQueueHead = CREATE_NAMED_ENTITY(istrClassname);
	entvars_t* pev = VARS(g_pBodyQueueHead);

	// Reserve 3 more slots for dead bodies
	for (int i = 0; i < 3; i++)
	{
		pev->owner = CREATE_NAMED_ENTITY(istrClassname);
		pev = VARS(pev->owner);
	}

	pev->owner = g_pBodyQueueHead;
}

//
// make a body que entry for the given ent so the ent can be respawned elsewhere
//
// GLOBALS ASSUMED SET:  g_eoBodyQueueHead
//
void CopyToBodyQue(entvars_t* pev)
{
	if ((pev->effects & EF_NODRAW) != 0)
		return;

	entvars_t* pevHead = VARS(g_pBodyQueueHead);

	pevHead->angles = pev->angles;
	pevHead->model = pev->model;
	pevHead->modelindex = pev->modelindex;
	pevHead->frame = pev->frame;
	pevHead->colormap = pev->colormap;
	pevHead->movetype = MOVETYPE_TOSS;
	pevHead->velocity = pev->velocity;
	pevHead->flags = 0;
	pevHead->deadflag = pev->deadflag;
	pevHead->renderfx = kRenderFxDeadPlayer;
	pevHead->renderamt = ENTINDEX(ENT(pev));

	pevHead->effects = pev->effects | EF_NOINTERP;
	// pevHead->goalstarttime = pev->goalstarttime;
	// pevHead->goalframe	= pev->goalframe;
	// pevHead->goalendtime = pev->goalendtime ;

	pevHead->sequence = pev->sequence;
	pevHead->animtime = pev->animtime;

	UTIL_SetEdictOrigin(g_pBodyQueueHead, pev->origin);
	UTIL_SetSize(pevHead, pev->mins, pev->maxs);
	g_pBodyQueueHead = pevHead->owner;
}
