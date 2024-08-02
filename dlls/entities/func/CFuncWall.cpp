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

#include "CFuncWall.h"

LINK_ENTITY_TO_CLASS(func_wall, CFuncWall);

void CFuncWall::Spawn()
{
	// If it can't move/go away, it's really part of the world
	if (!m_pMoveWith) // LRC
		pev->flags |= FL_WORLDBRUSH;

	// AJH This allows rotating of func_walls on spawn.
	// It would be easier to just use pev->angles directly but some old maps might have 'angles' specified already
	// and we don't want to stuff them up. Therefore we'll make people use the key 'message' in VHE to specify the angle.
	// pev->angles		= g_vecZero;

	UTIL_StringToVector((float*)pev->angles, STRING(pev->message));

	if (pev->angles != g_vecZero)
		ALERT(at_debug, "Rotating brush %s to %i,%i,%i\n", STRING(pev->model), pev->angles.x, pev->angles.y, pev->angles.z);

	pev->movetype = MOVETYPE_PUSH; // so it doesn't get pushed by anything
	pev->solid = SOLID_BSP;
	SET_MODEL(ENT(pev), STRING(pev->model));

	// LRC
	if (m_iStyle >= 32)
		LIGHT_STYLE(m_iStyle, "a");
	else if (m_iStyle <= -32)
		LIGHT_STYLE(-m_iStyle, "z");
}

void CFuncWall::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (ShouldToggle(useType, pev->frame != 0))
	{
		pev->frame = 1 - pev->frame;
		if (m_iStyle >= 32)
		{
			if (pev->frame)
				LIGHT_STYLE(m_iStyle, "z");
			else
				LIGHT_STYLE(m_iStyle, "a");
		}
		else if (m_iStyle <= -32)
		{
			if (pev->frame)
				LIGHT_STYLE(-m_iStyle, "a");
			else
				LIGHT_STYLE(-m_iStyle, "z");
		}
	}
}
