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

#include "CFuncMonsterClip.h"
#include "CFuncWall.h"

LINK_ENTITY_TO_CLASS(func_monsterclip, CFuncMonsterClip);

void CFuncMonsterClip::Spawn()
{
	CFuncWall::Spawn();
	if (CVAR_GET_FLOAT("showtriggers") == 0)
		pev->effects = EF_NODRAW;
	pev->flags |= FL_MONSTERCLIP;
}
