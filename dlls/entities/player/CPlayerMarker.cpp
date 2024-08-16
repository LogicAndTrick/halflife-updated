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

#include "CPlayerMarker.h"

LINK_ENTITY_TO_CLASS(player_marker, CPlayerMarker);

void CPlayerMarker ::Spawn(void)
{
	Precache();
	SET_MODEL(ENT(pev), "models/null.mdl");
	ALERT(at_aiconsole, "DEBUG: Player_marker coordinates is %f %f %f \n", pev->origin.x, pev->origin.y, pev->origin.z);
}

void CPlayerMarker ::Precache(void)
{
	PRECACHE_MODEL("models/null.mdl");
}
