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

#include "CFuncShine.h"
#include "util/movewith.h"

LINK_ENTITY_TO_CLASS(func_shine, CFuncShine);

extern int gmsgAddShine;

void CFuncShine ::Spawn()
{
	pev->solid = SOLID_NOT; // always solid_not
	SET_MODEL(ENT(pev), STRING(pev->model));
	pev->effects |= EF_NODRAW;

	// not that we actually need to precache it here, but we do need to make sure it exists
	PRECACHE_MODEL((char*)STRING(pev->message));
}

void CFuncShine ::Activate()
{
	//	ALERT(at_console, "Activate shine\n");

	CBaseEntity::Activate();
	UTIL_DesiredAction(this);
}

void CFuncShine ::DesiredAction(void)
{
	if (pev->message && pev->renderamt)
	{
		//		ALERT(at_console, "Prepare think\n");
		pev->nextthink = gpGlobals->time + 1.5;
	}
}

void CFuncShine ::Think()
{
	//	ALERT(at_console, "Think shine\n");
	MESSAGE_BEGIN(MSG_BROADCAST, gmsgAddShine, NULL);
	WRITE_BYTE(pev->scale);
	WRITE_BYTE(pev->renderamt);
	WRITE_COORD(pev->absmin.x + 2); // take off 2: mins values are padded, but we just want to hug the surface
	WRITE_COORD(pev->absmax.x - 2);
	WRITE_COORD(pev->absmin.y + 2);
	WRITE_COORD(pev->absmax.y - 2);
	WRITE_COORD(pev->absmin.z + 2);
	WRITE_STRING(STRING(pev->message));
	MESSAGE_END();
}
