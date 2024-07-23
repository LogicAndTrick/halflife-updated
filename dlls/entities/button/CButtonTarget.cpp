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

#include "CButtonTarget.h"

LINK_ENTITY_TO_CLASS(button_target, CButtonTarget);

bool CButtonTarget::KeyValue(KeyValueData* pkvd) // AJH
{
	if (FStrEq(pkvd->szKeyName, "master"))
	{
		m_sMaster = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	return CBaseEntity::KeyValue(pkvd);
}

void CButtonTarget::Spawn()
{
	pev->movetype = MOVETYPE_PUSH;
	if (pev->spawnflags & SF_BTARGET_SOLIDNOT)
	{							// AJH - non solid button targets
		pev->solid = SOLID_NOT; // note: setting non solid will stop
	}
	else
	{							//'trigger on shot' as no collision occurs
		pev->solid = SOLID_BSP; // Default behaviour is SOLID
	}							//

	SET_MODEL(ENT(pev), STRING(pev->model));

	if (pev->spawnflags & SF_BTARGET_NOSHOT)
	{								 // AJH - Don't allow triggering when shot
		pev->takedamage = DAMAGE_NO; // Default: allow triggering
	}
	else
	{								  //
		pev->takedamage = DAMAGE_YES; //
	}								  //

	if (FBitSet(pev->spawnflags, SF_BTARGET_ON))
		pev->frame = 1;
}

void CButtonTarget::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (!ShouldToggle(useType, 0 != pev->frame))
		return;

	if (!UTIL_IsMasterTriggered(m_sMaster, pActivator)) //
		return;											// AJH allows for locked button_targets

	pev->frame = 0 != pev->frame ? 0 : 1;
	if (0 != pev->frame)
		SUB_UseTargets(pActivator, USE_ON, 0);
	else
		SUB_UseTargets(pActivator, USE_OFF, 0);
}

int CButtonTarget::ObjectCaps()
{
	int caps = CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION;

	if (FBitSet(pev->spawnflags, SF_BTARGET_USE))
		return caps | FCAP_IMPULSE_USE;
	else
		return caps;
}

bool CButtonTarget::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	Use(Instance(pevAttacker), this, USE_TOGGLE, 0);

	return true;
}
