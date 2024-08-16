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

#include "CTargetFMODAudio.h"

LINK_ENTITY_TO_CLASS(ambient_fmodstream, CTargetFMODAudio);
LINK_ENTITY_TO_CLASS(trigger_mp3audio, CTargetFMODAudio);

TYPEDESCRIPTION CTargetFMODAudio::m_SaveData[] =
	{
		DEFINE_FIELD(CTargetFMODAudio, m_bPlaying, FIELD_BOOLEAN),
};
IMPLEMENT_SAVERESTORE(CTargetFMODAudio, CPointEntity);

void CTargetFMODAudio ::Spawn(void)
{
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;

	m_bPlaying = false; // start out not playing
}

void CTargetFMODAudio::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	char command[64];

	if (!pActivator->IsPlayer()) // activator should be a player
		return;

	if (!m_bPlaying) // if we're not playing, start playing!
		m_bPlaying = true;
	else
	{ // if we're already playing, stop the mp3
		m_bPlaying = false;
		CLIENT_COMMAND(pActivator->edict(), "stopaudio\n");
		return;
	}

	// issue the play/loop command
	sprintf(command, "playaudio %s\n", STRING(pev->message));

	CLIENT_COMMAND(pActivator->edict(), command);

	// remove if set
	if (FBitSet(pev->spawnflags, SF_REMOVE_ON_FIRE))
		UTIL_Remove(this);
}
