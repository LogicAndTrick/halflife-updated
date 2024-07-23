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

#include "CRotButton.h"
#include "entities/func/CBaseDoor.h"

LINK_ENTITY_TO_CLASS(func_rot_button, CRotButton);

bool CRotButton::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "axes"))
	{
		UTIL_StringToVector((float*)(pev->movedir), pkvd->szValue);
		return true;
	}
	return CBaseButton::KeyValue(pkvd);
}

void CRotButton::Spawn()
{
	const char* pszSound;
	//----------------------------------------------------
	// determine sounds for buttons
	// a sound of 0 should not make a sound
	//----------------------------------------------------
	pszSound = ButtonSound(m_sounds);
	PRECACHE_SOUND(pszSound);
	pev->noise = ALLOC_STRING(pszSound);

	// set the axis of rotation
	CBaseToggle::AxisDir(pev);

	// check for clockwise rotation
	if (FBitSet(pev->spawnflags, SF_DOOR_ROTATE_BACKWARDS))
		pev->movedir = pev->movedir * -1;

	pev->movetype = MOVETYPE_PUSH;

	if ((pev->spawnflags & SF_ROTBUTTON_NOTSOLID) != 0)
		pev->solid = SOLID_NOT;
	else
		pev->solid = SOLID_BSP;

	SET_MODEL(ENT(pev), STRING(pev->model));

	if (pev->speed == 0)
		pev->speed = 40;

	if (m_flWait == 0)
		m_flWait = 1;

	if (pev->health > 0)
	{
		pev->takedamage = DAMAGE_YES;
	}

	m_toggle_state = TS_AT_BOTTOM;
	m_vecAngle1 = pev->angles;
	m_vecAngle2 = pev->angles + pev->movedir * m_flMoveDistance;
	ASSERTSZ(m_vecAngle1 != m_vecAngle2, "rotating button start/end positions are equal");

	m_fStayPushed = (m_flWait == -1 ? true : false);
	m_fRotating = true;

	// if the button is flagged for USE button activation only, take away it's touch function and add a use function
	if (!FBitSet(pev->spawnflags, SF_BUTTON_TOUCH_ONLY))
	{
		SetTouch(NULL);
		if (FBitSet(pev->spawnflags, SF_BUTTON_USEKEY))
			SetUse(&CRotButton::ButtonUse_IgnorePlayer);
		else
			SetUse(&CRotButton::ButtonUse);
	}
	else // touchable button
	{
		SetTouch(&CRotButton::ButtonTouch);
		if (!FBitSet(pev->spawnflags, SF_BUTTON_USEKEY))
			SetUse(&CRotButton::ButtonUse_IgnorePlayer);
		else
			SetUse(&CRotButton::ButtonUse);
	}

	// SetTouch( ButtonTouch );
}
