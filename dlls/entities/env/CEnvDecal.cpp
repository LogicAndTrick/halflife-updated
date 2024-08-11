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

#include "CEnvDecal.h"

#include "decals.h"
#include "locus.h"

LINK_ENTITY_TO_CLASS(env_decal, CEnvDecal);

void CEnvDecal::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	int iTexture = 0;

	switch (pev->impulse)
	{
	case 1:
		iTexture = DECAL_GUNSHOT1 + RANDOM_LONG(0, 4);
		break;
	case 2:
		iTexture = DECAL_BLOOD1 + RANDOM_LONG(0, 5);
		break;
	case 3:
		iTexture = DECAL_YBLOOD1 + RANDOM_LONG(0, 5);
		break;
	case 4:
		iTexture = DECAL_GLASSBREAK1 + RANDOM_LONG(0, 2);
		break;
	case 5:
		iTexture = DECAL_BIGSHOT1 + RANDOM_LONG(0, 4);
		break;
	case 6:
		iTexture = DECAL_SCORCH1 + RANDOM_LONG(0, 1);
		break;
	case 7:
		iTexture = DECAL_SPIT1 + RANDOM_LONG(0, 1);
		break;
	}

	if (pev->impulse)
		iTexture = gDecals[iTexture].index;
	else
		iTexture = pev->skin; // custom texture

	Vector vecPos;
	if (!FStringNull(pev->target))
		vecPos = CalcLocus_Position(this, pActivator, STRING(pev->target));
	else
		vecPos = pev->origin;

	Vector vecOffs;
	if (!FStringNull(pev->netname))
		vecOffs = CalcLocus_Velocity(this, pActivator, STRING(pev->netname));
	else
	{
		UTIL_MakeVectors(pev->angles);
		vecOffs = gpGlobals->v_forward;
	}

	if (pev->message)
		vecOffs = vecOffs * CalcLocus_Number(pActivator, STRING(pev->message));
	else
		vecOffs = vecOffs.Normalize() * 4000;


	TraceResult trace;
	int entityIndex;

	UTIL_TraceLine(vecPos, vecPos + vecOffs, ignore_monsters, NULL, &trace);

	if (trace.flFraction == 1.0)
		return; // didn't hit anything, oh well

	entityIndex = (short)ENTINDEX(trace.pHit);

	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(TE_BSPDECAL);
	WRITE_COORD(trace.vecEndPos.x);
	WRITE_COORD(trace.vecEndPos.y);
	WRITE_COORD(trace.vecEndPos.z);
	WRITE_SHORT(iTexture);
	WRITE_SHORT(entityIndex);
	if (entityIndex)
		WRITE_SHORT((int)VARS(trace.pHit)->modelindex);
	MESSAGE_END();
}

void CEnvDecal::Spawn()
{
	if (pev->impulse == 0)
	{
		pev->skin = DECAL_INDEX(STRING(pev->noise));

		if (pev->skin == 0)
			ALERT(at_debug, "locus_decal \"%s\" can't find decal \"%s\"\n", STRING(pev->noise));
	}
}
