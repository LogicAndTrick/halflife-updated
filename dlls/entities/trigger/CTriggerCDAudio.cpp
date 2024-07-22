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

#include "CTriggerCDAudio.h"

LINK_ENTITY_TO_CLASS(trigger_cdaudio, CTriggerCDAudio);

//
// Changes tracks or stops CD when player touches
//
// !!!HACK - overloaded HEALTH to avoid adding new field
void CTriggerCDAudio::Touch(CBaseEntity* pOther)
{
	if (!pOther->IsPlayer())
	{ // only clients may trigger these events
		return;
	}

	PlayTrack();
}

void CTriggerCDAudio::Spawn()
{
	InitTrigger();
}

void CTriggerCDAudio::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	PlayTrack();
}

void PlayCDTrack(int iTrack, int iSong)
{
	// manually find the single player.
	CBaseEntity* pClient = UTIL_GetLocalPlayer();

	// Can't play if the client is not connected!
	if (!pClient)
		return;

	if (iTrack < -1 || iTrack > 30)
	{
		ALERT(at_debug, "TriggerCDAudio - Track %d out of range\n");
		return;
	}

	if (iTrack == -1)
	{
		CLIENT_COMMAND(pClient->edict(), "stopaudio\n");
		CLIENT_COMMAND(pClient->edict(), "cd stop\n");
	}
	else
	{
		char string[64];

		if (iSong)
			sprintf(string, "playaudio %s\n", STRING(iSong));
		else
			sprintf(string, "cd play %3d\n", iTrack);
		CLIENT_COMMAND(pClient->edict(), string);
	}
}

// only plays for ONE client, so only use in single play!
void CTriggerCDAudio::PlayTrack()
{
	PlayCDTrack((int)pev->health, (int)pev->message);

	SetTouch(NULL);
	UTIL_Remove(this);
}
