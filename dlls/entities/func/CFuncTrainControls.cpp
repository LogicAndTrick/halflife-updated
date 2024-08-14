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

#include "CFuncTrainControls.h"
#include "CFuncTrackTrain.h"

LINK_ENTITY_TO_CLASS(func_traincontrols, CFuncTrainControls);

void CFuncTrainControls::Find()
{
	CBaseEntity* pTarget = NULL;

	do
	{
		pTarget = UTIL_FindEntityByTargetname(pTarget, STRING(pev->target));
	} while (pTarget && !FClassnameIs(pTarget->pev, "func_tracktrain"));

	if (!pTarget)
	{
		ALERT(at_debug, "TrackTrainControls: No train %s\n", STRING(pev->target));
		return;
	}

	CFuncTrackTrain* ptrain = (CFuncTrackTrain*)pTarget;
	ptrain->SetControls(pev);
	UTIL_Remove(this);
}

void CFuncTrainControls::Spawn()
{
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	SET_MODEL(ENT(pev), STRING(pev->model));

	UTIL_SetSize(pev, pev->mins, pev->maxs);
	UTIL_SetOrigin(this, pev->origin);

	SetThink(&CFuncTrainControls::Find);
	SetNextThink(0);
}
