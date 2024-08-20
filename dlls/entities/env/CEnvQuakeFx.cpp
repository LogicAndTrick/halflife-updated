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

#include "CEnvQuakeFx.h"
#include "util/locus.h"

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
