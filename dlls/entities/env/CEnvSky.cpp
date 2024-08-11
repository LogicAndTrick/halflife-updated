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

#include "CEnvSky.h"
#include "movewith.h"

LINK_ENTITY_TO_CLASS(env_sky, CEnvSky);

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

	// AJH scale of the skybox 1/x (0=infinitly large/far away = no parallax)
	// No parallax is the default behaviour. FGD's can set a new default.
	WRITE_BYTE(pev->frags);
	// WRITE_BYTE(ENTINDEX(edict()));

	MESSAGE_END();
}
