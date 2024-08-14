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

#include "CInfoMoveWith.h"
#include "movewith.h"

LINK_ENTITY_TO_CLASS(info_movewith, CInfoMoveWith);

void CInfoMoveWith::Spawn()
{
	if (pev->spawnflags & SF_IMW_INACTIVE)
		m_MoveWith = pev->netname;
	else
		m_MoveWith = pev->target;

	if (pev->spawnflags & SF_IMW_BLOCKABLE)
	{
		pev->solid = SOLID_SLIDEBOX;
		pev->movetype = MOVETYPE_FLY;
	}
	// and allow InitMoveWith to set things up as usual.
}

void CInfoMoveWith::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	CBaseEntity* pSibling;

	if (!ShouldToggle(useType))
		return;

	if (m_pMoveWith)
	{
		// remove this from the old parent's list of children
		pSibling = m_pMoveWith->m_pChildMoveWith;
		if (pSibling == this)
			m_pMoveWith->m_pChildMoveWith = this->m_pSiblingMoveWith;
		else
		{
			while (pSibling->m_pSiblingMoveWith && pSibling->m_pSiblingMoveWith != this)
			{
				pSibling = pSibling->m_pSiblingMoveWith;
			}

			if (pSibling->m_pSiblingMoveWith == this)
			{
				pSibling->m_pSiblingMoveWith = this->m_pSiblingMoveWith;
			}
			else
			{
				// failed to find myself in the list, complain
				ALERT(at_error, "info_movewith can't find itself\n");
				return;
			}
		}
		m_pMoveWith = NULL;
		m_pSiblingMoveWith = NULL;
	}

	if (pev->spawnflags & SF_IMW_INACTIVE)
	{
		pev->spawnflags &= ~SF_IMW_INACTIVE;
		m_MoveWith = pev->target;
	}
	else
	{
		pev->spawnflags |= SF_IMW_INACTIVE;
		m_MoveWith = pev->netname;
	}

	// set things up for the new m_MoveWith value
	if (!m_MoveWith)
	{
		UTIL_SetVelocity(this, g_vecZero); // come to a stop
		return;
	}

	m_pMoveWith = UTIL_FindEntityByTargetname(NULL, STRING(m_MoveWith));
	if (!m_pMoveWith)
	{
		ALERT(at_debug, "Missing movewith entity %s\n", STRING(m_MoveWith));
		return;
	}

	pSibling = m_pMoveWith->m_pChildMoveWith;
	while (pSibling) // check that this entity isn't already in the list of children
	{
		if (pSibling == this)
			return;
		pSibling = pSibling->m_pSiblingMoveWith;
	}

	// add this entity to the list of children
	m_pSiblingMoveWith = m_pMoveWith->m_pChildMoveWith; // may be null: that's fine by me.
	m_pMoveWith->m_pChildMoveWith = this;
	m_vecMoveWithOffset = pev->origin - m_pMoveWith->pev->origin;
	UTIL_SetVelocity(this, g_vecZero); // match speed with the new entity
}
