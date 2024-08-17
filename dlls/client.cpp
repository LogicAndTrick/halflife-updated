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
// Robin, 4-22-98: Moved set_suicide_frame() here from player.cpp to allow us to
//				   have one without a hardcoded player.mdl in tf_client.cpp

/*

===== client.cpp ========================================================

  client/server game specific stuff

*/

#include <algorithm>
#include <string>
#include <vector>

#include "extdll.h"
#include "util.h"
#include "filesystem_utils.h"
#include "cbase.h"
#include "com_model.h"
#include "saverestore.h"
#include "entities/player/CBasePlayer.h"
#include "client.h"
#include "classes/CSound.h"
#include "entities/sound/CSoundEnt.h"
#include "classes/gamerules/CGameRules.h"
#include "api/game.h"
#include "customentity.h"
#include "weapons.h"
#include "weaponinfo.h"
#include "usercmd.h"
#include "netadr.h"
#include "pm_shared.h"
#include "pm_defs.h"
#include "UserMessages.h"
#include "movewith.h"
#include "entities/CBaseSpectator.h"
#include "entities/CWorld.h"
#include "entities/CCorpse.h"
#include "entities/item/CItemAntidote.h"
#include "entities/item/CItemAntiRad.h"
#include "entities/item/CItemCamera.h"
#include "entities/item/CItemFlare.h"
#include "entities/item/CItemMedicalKit.h"
#include "entities/weapon/CBasePlayerItem.h"
#include "entities/weapon/CBasePlayerWeapon.h"
#include "entities/weapon/CRpg.h"

void LinkUserMessages();

/*
 * used by kill command and disconnect command
 * ROBIN: Moved here from player.cpp, to allow multiple player models
 */
void set_suicide_frame(entvars_t* pev)
{
	if (!FStrEq(STRING(pev->model), "models/player.mdl"))
		return; // allready gibbed

	//	pev->frame		= $deatha11;
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_TOSS;
	pev->deadflag = DEAD_DEAD;
	pev->nextthink = -1;
}



// called by ClientKill and DeadThink
void respawn(entvars_t* pev, bool fCopyCorpse)
{
	if (0 != gpGlobals->coop || 0 != gpGlobals->deathmatch)
	{
		if (fCopyCorpse)
		{
			// make a copy of the dead body for appearances sake
			CopyToBodyQue(pev);
		}

		// respawn player
		GetClassPtr((CBasePlayer*)pev)->Spawn();
	}
	else
	{ // restart the entire server
		SERVER_COMMAND("reload\n");
	}
}

#include "voice_gamemgr.h"
extern CVoiceGameMgr g_VoiceGameMgr;





/*
===========
ClientCommand
called each time a player uses a "cmd" command
============
*/
extern int gmsgPlayMP3; //AJH - Killars MP3player





void ClientPrecache()
{
	// setup precaches always needed
	PRECACHE_SOUND("player/sprayer.wav"); // spray paint sound for PreAlpha

	// PRECACHE_SOUND("player/pl_jumpland2.wav");		// UNDONE: play 2x step sound

	PRECACHE_SOUND("player/pl_fallpain2.wav");
	PRECACHE_SOUND("player/pl_fallpain3.wav");

	PRECACHE_SOUND("player/pl_step1.wav"); // walk on concrete
	PRECACHE_SOUND("player/pl_step2.wav");
	PRECACHE_SOUND("player/pl_step3.wav");
	PRECACHE_SOUND("player/pl_step4.wav");

	PRECACHE_SOUND("common/npc_step1.wav"); // NPC walk on concrete
	PRECACHE_SOUND("common/npc_step2.wav");
	PRECACHE_SOUND("common/npc_step3.wav");
	PRECACHE_SOUND("common/npc_step4.wav");

	PRECACHE_SOUND("player/pl_metal1.wav"); // walk on metal
	PRECACHE_SOUND("player/pl_metal2.wav");
	PRECACHE_SOUND("player/pl_metal3.wav");
	PRECACHE_SOUND("player/pl_metal4.wav");

	PRECACHE_SOUND("player/pl_dirt1.wav"); // walk on dirt
	PRECACHE_SOUND("player/pl_dirt2.wav");
	PRECACHE_SOUND("player/pl_dirt3.wav");
	PRECACHE_SOUND("player/pl_dirt4.wav");

	PRECACHE_SOUND("player/pl_duct1.wav"); // walk in duct
	PRECACHE_SOUND("player/pl_duct2.wav");
	PRECACHE_SOUND("player/pl_duct3.wav");
	PRECACHE_SOUND("player/pl_duct4.wav");

	PRECACHE_SOUND("player/pl_grate1.wav"); // walk on grate
	PRECACHE_SOUND("player/pl_grate2.wav");
	PRECACHE_SOUND("player/pl_grate3.wav");
	PRECACHE_SOUND("player/pl_grate4.wav");

	PRECACHE_SOUND("player/pl_slosh1.wav"); // walk in shallow water
	PRECACHE_SOUND("player/pl_slosh2.wav");
	PRECACHE_SOUND("player/pl_slosh3.wav");
	PRECACHE_SOUND("player/pl_slosh4.wav");

	PRECACHE_SOUND("player/pl_tile1.wav"); // walk on tile
	PRECACHE_SOUND("player/pl_tile2.wav");
	PRECACHE_SOUND("player/pl_tile3.wav");
	PRECACHE_SOUND("player/pl_tile4.wav");
	PRECACHE_SOUND("player/pl_tile5.wav");

	PRECACHE_SOUND("player/pl_swim1.wav"); // breathe bubbles
	PRECACHE_SOUND("player/pl_swim2.wav");
	PRECACHE_SOUND("player/pl_swim3.wav");
	PRECACHE_SOUND("player/pl_swim4.wav");

	PRECACHE_SOUND("player/pl_ladder1.wav"); // climb ladder rung
	PRECACHE_SOUND("player/pl_ladder2.wav");
	PRECACHE_SOUND("player/pl_ladder3.wav");
	PRECACHE_SOUND("player/pl_ladder4.wav");

	PRECACHE_SOUND("player/pl_wade1.wav"); // wade in water
	PRECACHE_SOUND("player/pl_wade2.wav");
	PRECACHE_SOUND("player/pl_wade3.wav");
	PRECACHE_SOUND("player/pl_wade4.wav");

	PRECACHE_SOUND("debris/wood1.wav"); // hit wood texture
	PRECACHE_SOUND("debris/wood2.wav");
	PRECACHE_SOUND("debris/wood3.wav");

	PRECACHE_SOUND("plats/train_use1.wav"); // use a train

	PRECACHE_SOUND("buttons/spark5.wav"); // hit computer texture
	PRECACHE_SOUND("buttons/spark6.wav");
	PRECACHE_SOUND("debris/glass1.wav");
	PRECACHE_SOUND("debris/glass2.wav");
	PRECACHE_SOUND("debris/glass3.wav");

	PRECACHE_SOUND(SOUND_FLASHLIGHT_ON);
	PRECACHE_SOUND(SOUND_FLASHLIGHT_OFF);

	// player gib sounds
	PRECACHE_SOUND("common/bodysplat.wav");

	// player pain sounds
	PRECACHE_SOUND("player/pl_pain2.wav");
	PRECACHE_SOUND("player/pl_pain4.wav");
	PRECACHE_SOUND("player/pl_pain5.wav");
	PRECACHE_SOUND("player/pl_pain6.wav");
	PRECACHE_SOUND("player/pl_pain7.wav");

	PRECACHE_MODEL("models/player.mdl");

	// hud sounds

	PRECACHE_SOUND("common/wpn_hudoff.wav");
	PRECACHE_SOUND("common/wpn_hudon.wav");
	PRECACHE_SOUND("common/wpn_moveselect.wav");
	PRECACHE_SOUND("common/wpn_select.wav");
	PRECACHE_SOUND("common/wpn_denyselect.wav");

	// geiger sounds

	PRECACHE_SOUND("player/geiger6.wav");
	PRECACHE_SOUND("player/geiger5.wav");
	PRECACHE_SOUND("player/geiger4.wav");
	PRECACHE_SOUND("player/geiger3.wav");
	PRECACHE_SOUND("player/geiger2.wav");
	PRECACHE_SOUND("player/geiger1.wav");

	if (giPrecacheGrunt)
		UTIL_PrecacheOther("monster_human_grunt");
}

/*
===============
GetGameDescription

Returns the descriptive name of this .dll.  E.g., Half-Life, or Team Fortress 2
===============
*/
const char* GetGameDescription()
{
	if (g_pGameRules) // this function may be called before the world has spawned, and the game rules initialized
		return g_pGameRules->GetGameDescription();
	else
		return GAME_NAME;
}

/*
================
Sys_Error

Engine is going to shut down, allows setting a breakpoint in game .dll to catch that occasion
================
*/
void Sys_Error(const char* error_string)
{
	// Default case, do nothing.  MOD AUTHORS:  Add code ( e.g., _asm { int 3 }; here to cause a breakpoint for debugging your game .dlls
}

/*
================
PlayerCustomization

A new player customization has been registered on the server
UNDONE:  This only sets the # of frames of the spray can logo
animation right now.
================
*/
void PlayerCustomization(edict_t* pEntity, customization_t* pCust)
{
	entvars_t* pev = &pEntity->v;
	CBasePlayer* pPlayer = (CBasePlayer*)GET_PRIVATE(pEntity);

	if (!pPlayer)
	{
		ALERT(at_debug, "PlayerCustomization:  Couldn't get player!\n");
		return;
	}

	if (!pCust)
	{
		ALERT(at_debug, "PlayerCustomization:  NULL customization!\n");
		return;
	}

	switch (pCust->resource.type)
	{
	case t_decal:
		pPlayer->SetCustomDecalFrames(pCust->nUserData2); // Second int is max # of frames.
		break;
	case t_sound:
	case t_skin:
	case t_model:
		// Ignore for now.
		break;
	default:
		ALERT(at_debug, "PlayerCustomization:  Unknown customization type!\n");
		break;
	}
}

/*
================
SpectatorConnect

A spectator has joined the game
================
*/
void SpectatorConnect(edict_t* pEntity)
{
	entvars_t* pev = &pEntity->v;
	CBaseSpectator* pPlayer = (CBaseSpectator*)GET_PRIVATE(pEntity);

	if (pPlayer)
		pPlayer->SpectatorConnect();
}

/*
================
SpectatorConnect

A spectator has left the game
================
*/
void SpectatorDisconnect(edict_t* pEntity)
{
	entvars_t* pev = &pEntity->v;
	CBaseSpectator* pPlayer = (CBaseSpectator*)GET_PRIVATE(pEntity);

	if (pPlayer)
		pPlayer->SpectatorDisconnect();
}

/*
================
SpectatorConnect

A spectator has sent a usercmd
================
*/
void SpectatorThink(edict_t* pEntity)
{
	entvars_t* pev = &pEntity->v;
	CBaseSpectator* pPlayer = (CBaseSpectator*)GET_PRIVATE(pEntity);

	if (pPlayer)
		pPlayer->SpectatorThink();
}

////////////////////////////////////////////////////////
// PAS and PVS routines for client messaging
//

/*
================
SetupVisibility

A client can have a separate "view entity" indicating that his/her view should depend on the origin of that
view entity.  If that's the case, then pViewEntity will be non-NULL and will be used.  Otherwise, the current
entity's origin is used.  Either is offset by the view_ofs to get the eye position.

From the eye position, we set up the PAS and PVS to use for filtering network messages to the client.  At this point, we could
 override the actual PAS or PVS values, or use a different origin.

NOTE:  Do not cache the values of pas and pvs, as they depend on reusable memory in the engine, they are only good for this one frame
================
*/
void SetupVisibility(edict_t* pViewEntity, edict_t* pClient, unsigned char** pvs, unsigned char** pas)
{
	Vector org;
	edict_t* pView = pClient;

	// Find the client's PVS
	if (pViewEntity)
	{
		pView = pViewEntity;
	}
	// for trigger_viewset
	CBasePlayer* pPlayer = (CBasePlayer*)CBaseEntity::Instance((struct edict_s*)pClient);
	if (pPlayer->viewFlags & 1) // custom view active
	{
		CBaseEntity* pViewEnt = UTIL_FindEntityByTargetname(NULL, STRING(pPlayer->viewEntity));
		if (!FNullEnt(pViewEnt))
		{
			//	ALERT(at_console, "setting PAS/PVS to entity %s\n", STRING(pPlayer->viewEntity));
			pView = pViewEnt->edict();
		}
		else
			pPlayer->viewFlags = 0;
	}
	if ((pClient->v.flags & FL_PROXY) != 0)
	{
		*pvs = NULL; // the spectator proxy sees
		*pas = NULL; // and hears everything
		return;
	}

	org = pView->v.origin + pView->v.view_ofs;
	if ((pView->v.flags & FL_DUCKING) != 0)
	{
		org = org + (VEC_HULL_MIN - VEC_DUCK_HULL_MIN);
	}

	*pvs = ENGINE_SET_PVS((float*)&org);
	*pas = ENGINE_SET_PAS((float*)&org);
}

#include "entity_state.h"

/*
AddToFullPack

Return 1 if the entity state has been filled in for the ent and the entity will be propagated to the client, 0 otherwise

state is the server maintained copy of the state info that is transmitted to the client
a MOD could alter values copied into state to send the "host" a different look for a particular entity update, etc.
e and ent are the entity that is being added to the update, if 1 is returned
host is the player's edict of the player whom we are sending the update to
player is 1 if the ent/e is a player and 0 otherwise
pSet is either the PAS or PVS that we previous set up.  We can use it to ask the engine to filter the entity against the PAS or PVS.
we could also use the pas/ pvs that we set in SetupVisibility, if we wanted to.  Caching the value is valid in that case, but still only for the current frame
*/
int AddToFullPack(struct entity_state_s* state, int e, edict_t* ent, edict_t* host, int hostflags, int player, unsigned char* pSet)
{
	// Entities with an index greater than this will corrupt the client's heap because 
	// the index is sent with only 11 bits of precision (2^11 == 2048).
	// So we don't send them, just like having too many entities would result
	// in the entity not being sent.
	if (e >= MAX_EDICTS)
	{
		return 0;
	}

	int i;

	auto entity = reinterpret_cast<CBaseEntity*>(GET_PRIVATE(ent));

	// don't send if flagged for NODRAW and it's not the host getting the message
	if ((ent->v.effects & EF_NODRAW) != 0 &&
		(ent != host))
		return 0;

	// Ignore ents without valid / visible models
	if (0 == ent->v.modelindex || !STRING(ent->v.model))
		return 0;

	// Don't send spectators to other players
	if ((ent->v.flags & FL_SPECTATOR) != 0 && (ent != host))
	{
		return 0;
	}

	// Ignore if not the host and not touching a PVS/PAS leaf
	// If pSet is NULL, then the test will always succeed and the entity will be added to the update
	if (ent != host)
	{
		if (!ENGINE_CHECK_VISIBILITY((const struct edict_s*)ent, pSet))
		{
			if (ent->v.renderfx != kRenderFxEntInPVS)
				return 0;
		}
	}


	// Don't send entity to local client if the client says it's predicting the entity itself.
	if ((ent->v.flags & FL_SKIPLOCALHOST) != 0)
	{
		if ((hostflags & 1) != 0 && (ent->v.owner == host))
			return 0;
	}

	if (0 != host->v.groupinfo)
	{
		UTIL_SetGroupTrace(host->v.groupinfo, GROUP_OP_AND);

		// Should always be set, of course
		if (0 != ent->v.groupinfo)
		{
			if (g_groupop == GROUP_OP_AND)
			{
				if ((ent->v.groupinfo & host->v.groupinfo) == 0)
					return 0;
			}
			else if (g_groupop == GROUP_OP_NAND)
			{
				if ((ent->v.groupinfo & host->v.groupinfo) != 0)
					return 0;
			}
		}

		UTIL_UnsetGroupTrace();
	}

	memset(state, 0, sizeof(*state));

	// Assign index so we can track this entity from frame to frame and
	//  delta from it.
	state->number = e;
	state->entityType = ENTITY_NORMAL;

	// Flag custom entities.
	if ((ent->v.flags & FL_CUSTOMENTITY) != 0)
	{
		state->entityType = ENTITY_BEAM;
	}

	//
	// Copy state data
	//

	// Round animtime to nearest millisecond
	state->animtime = (int)(1000.0 * ent->v.animtime) / 1000.0;

	memcpy(state->origin, ent->v.origin, 3 * sizeof(float));
	memcpy(state->angles, ent->v.angles, 3 * sizeof(float));
	memcpy(state->mins, ent->v.mins, 3 * sizeof(float));
	memcpy(state->maxs, ent->v.maxs, 3 * sizeof(float));

	memcpy(state->startpos, ent->v.startpos, 3 * sizeof(float));
	memcpy(state->endpos, ent->v.endpos, 3 * sizeof(float));

	state->impacttime = ent->v.impacttime;
	state->starttime = ent->v.starttime;

	state->modelindex = ent->v.modelindex;

	state->frame = ent->v.frame;

	state->skin = ent->v.skin;
	state->effects = ent->v.effects;

	// This non-player entity is being moved by the game .dll and not the physics simulation system
	//  make sure that we interpolate it's position on the client if it moves
	/*
	if (0 == player &&
		0 != ent->v.animtime &&
		ent->v.velocity[0] == 0 &&
		ent->v.velocity[1] == 0 &&
		ent->v.velocity[2] == 0)
	{
		state->eflags |= EFLAG_SLERP;
	}
	*/

	if ((ent->v.flags & FL_FLY) != 0)
	{
		state->eflags |= EFLAG_SLERP;
	}
	else
	{
		state->eflags &= ~EFLAG_SLERP;
	}

	state->eflags |= entity->m_EFlags;

	state->scale = ent->v.scale;
	state->solid = ent->v.solid;
	state->colormap = ent->v.colormap;

	state->movetype = ent->v.movetype;
	state->sequence = ent->v.sequence;
	state->framerate = ent->v.framerate;
	state->body = ent->v.body;

	for (i = 0; i < 4; i++)
	{
		state->controller[i] = ent->v.controller[i];
	}

	for (i = 0; i < 2; i++)
	{
		state->blending[i] = ent->v.blending[i];
	}

	state->rendermode = ent->v.rendermode;
	state->renderamt = ent->v.renderamt;
	state->renderfx = ent->v.renderfx;
	state->rendercolor.r = ent->v.rendercolor.x;
	state->rendercolor.g = ent->v.rendercolor.y;
	state->rendercolor.b = ent->v.rendercolor.z;

	state->aiment = 0;
	if (ent->v.aiment)
	{
		state->aiment = ENTINDEX(ent->v.aiment);
	}

	state->owner = 0;
	if (ent->v.owner)
	{
		int owner = ENTINDEX(ent->v.owner);

		// Only care if owned by a player
		if (owner >= 1 && owner <= gpGlobals->maxClients)
		{
			state->owner = owner;
		}
	}

	// HACK:  Somewhat...
	// Class is overridden for non-players to signify a breakable glass object ( sort of a class? )
	// that's 'class' in the sense medic, engineer, etc... !! --LRC
	if (0 == player)
	{
		state->playerclass = ent->v.playerclass;
	}

	// Special stuff for players only
	if (0 != player)
	{
		memcpy(state->basevelocity, ent->v.basevelocity, 3 * sizeof(float));

		state->weaponmodel = MODEL_INDEX(STRING(ent->v.weaponmodel));
		state->gaitsequence = ent->v.gaitsequence;
		state->spectator = ent->v.flags & FL_SPECTATOR;
		state->friction = ent->v.friction;

		state->gravity = ent->v.gravity;
		//		state->team			= ent->v.team;
		//
		state->usehull = (ent->v.flags & FL_DUCKING) != 0 ? 1 : 0;
		state->health = ent->v.health;
	}

	return 1;
}

/*
===================
CreateBaseline

Creates baselines used for network encoding, especially for player data since players are not spawned until connect time.
===================
*/
void CreateBaseline(int player, int eindex, struct entity_state_s* baseline, struct edict_s* entity, int playermodelindex, Vector* player_mins, Vector* player_maxs)
{
	baseline->origin = entity->v.origin;
	baseline->angles = entity->v.angles;
	baseline->frame = entity->v.frame;
	baseline->skin = (short)entity->v.skin;

	// render information
	baseline->rendermode = (byte)entity->v.rendermode;
	baseline->renderamt = (byte)entity->v.renderamt;
	baseline->rendercolor.r = (byte)entity->v.rendercolor.x;
	baseline->rendercolor.g = (byte)entity->v.rendercolor.y;
	baseline->rendercolor.b = (byte)entity->v.rendercolor.z;
	baseline->renderfx = (byte)entity->v.renderfx;

	if (0 != player)
	{
		baseline->mins = *player_mins;
		baseline->maxs = *player_maxs;

		baseline->colormap = eindex;
		baseline->modelindex = playermodelindex;
		baseline->friction = 1.0;
		baseline->movetype = MOVETYPE_WALK;

		baseline->scale = entity->v.scale;
		baseline->solid = SOLID_SLIDEBOX;
		baseline->framerate = 1.0;
		baseline->gravity = 1.0;
	}
	else
	{
		baseline->mins = entity->v.mins;
		baseline->maxs = entity->v.maxs;

		baseline->colormap = 0;
		baseline->modelindex = entity->v.modelindex; //SV_ModelIndex(pr_strings + entity->v.model);
		baseline->movetype = entity->v.movetype;

		baseline->scale = entity->v.scale;
		baseline->solid = entity->v.solid;
		baseline->framerate = entity->v.framerate;
		baseline->gravity = entity->v.gravity;
	}
}

typedef struct
{
	char name[32];
	int field;
} entity_field_alias_t;

#define FIELD_ORIGIN0 0
#define FIELD_ORIGIN1 1
#define FIELD_ORIGIN2 2
#define FIELD_ANGLES0 3
#define FIELD_ANGLES1 4
#define FIELD_ANGLES2 5

static entity_field_alias_t entity_field_alias[] =
	{
		{"origin[0]", 0},
		{"origin[1]", 0},
		{"origin[2]", 0},
		{"angles[0]", 0},
		{"angles[1]", 0},
		{"angles[2]", 0},
};

void Entity_FieldInit(struct delta_s* pFields)
{
	entity_field_alias[FIELD_ORIGIN0].field = DELTA_FINDFIELD(pFields, entity_field_alias[FIELD_ORIGIN0].name);
	entity_field_alias[FIELD_ORIGIN1].field = DELTA_FINDFIELD(pFields, entity_field_alias[FIELD_ORIGIN1].name);
	entity_field_alias[FIELD_ORIGIN2].field = DELTA_FINDFIELD(pFields, entity_field_alias[FIELD_ORIGIN2].name);
	entity_field_alias[FIELD_ANGLES0].field = DELTA_FINDFIELD(pFields, entity_field_alias[FIELD_ANGLES0].name);
	entity_field_alias[FIELD_ANGLES1].field = DELTA_FINDFIELD(pFields, entity_field_alias[FIELD_ANGLES1].name);
	entity_field_alias[FIELD_ANGLES2].field = DELTA_FINDFIELD(pFields, entity_field_alias[FIELD_ANGLES2].name);
}

/*
==================
Entity_Encode

Callback for sending entity_state_t info over network. 
FIXME:  Move to script
==================
*/
void Entity_Encode(struct delta_s* pFields, const unsigned char* from, const unsigned char* to)
{
	entity_state_t *f, *t;
	static bool initialized = false;

	if (!initialized)
	{
		Entity_FieldInit(pFields);
		initialized = true;
	}

	f = (entity_state_t*)from;
	t = (entity_state_t*)to;

	// Never send origin to local player, it's sent with more resolution in clientdata_t structure
	const bool localplayer = (t->number - 1) == ENGINE_CURRENT_PLAYER();
	if (localplayer)
	{
		DELTA_UNSETBYINDEX(pFields, entity_field_alias[FIELD_ORIGIN0].field);
		DELTA_UNSETBYINDEX(pFields, entity_field_alias[FIELD_ORIGIN1].field);
		DELTA_UNSETBYINDEX(pFields, entity_field_alias[FIELD_ORIGIN2].field);
	}

	if ((t->impacttime != 0) && (t->starttime != 0))
	{
		DELTA_UNSETBYINDEX(pFields, entity_field_alias[FIELD_ORIGIN0].field);
		DELTA_UNSETBYINDEX(pFields, entity_field_alias[FIELD_ORIGIN1].field);
		DELTA_UNSETBYINDEX(pFields, entity_field_alias[FIELD_ORIGIN2].field);

		DELTA_UNSETBYINDEX(pFields, entity_field_alias[FIELD_ANGLES0].field);
		DELTA_UNSETBYINDEX(pFields, entity_field_alias[FIELD_ANGLES1].field);
		DELTA_UNSETBYINDEX(pFields, entity_field_alias[FIELD_ANGLES2].field);
	}

	if ((t->movetype == MOVETYPE_FOLLOW) &&
		(t->aiment != 0))
	{
		DELTA_UNSETBYINDEX(pFields, entity_field_alias[FIELD_ORIGIN0].field);
		DELTA_UNSETBYINDEX(pFields, entity_field_alias[FIELD_ORIGIN1].field);
		DELTA_UNSETBYINDEX(pFields, entity_field_alias[FIELD_ORIGIN2].field);
	}
	else if (t->aiment != f->aiment)
	{
		DELTA_SETBYINDEX(pFields, entity_field_alias[FIELD_ORIGIN0].field);
		DELTA_SETBYINDEX(pFields, entity_field_alias[FIELD_ORIGIN1].field);
		DELTA_SETBYINDEX(pFields, entity_field_alias[FIELD_ORIGIN2].field);
	}
}

static entity_field_alias_t player_field_alias[] =
	{
		{"origin[0]", 0},
		{"origin[1]", 0},
		{"origin[2]", 0},
};

void Player_FieldInit(struct delta_s* pFields)
{
	player_field_alias[FIELD_ORIGIN0].field = DELTA_FINDFIELD(pFields, player_field_alias[FIELD_ORIGIN0].name);
	player_field_alias[FIELD_ORIGIN1].field = DELTA_FINDFIELD(pFields, player_field_alias[FIELD_ORIGIN1].name);
	player_field_alias[FIELD_ORIGIN2].field = DELTA_FINDFIELD(pFields, player_field_alias[FIELD_ORIGIN2].name);
}

/*
==================
Player_Encode

Callback for sending entity_state_t for players info over network. 
==================
*/
void Player_Encode(struct delta_s* pFields, const unsigned char* from, const unsigned char* to)
{
	entity_state_t *f, *t;
	static bool initialized = false;

	if (!initialized)
	{
		Player_FieldInit(pFields);
		initialized = true;
	}

	f = (entity_state_t*)from;
	t = (entity_state_t*)to;

	// Never send origin to local player, it's sent with more resolution in clientdata_t structure
	const bool localplayer = (t->number - 1) == ENGINE_CURRENT_PLAYER();
	if (localplayer)
	{
		DELTA_UNSETBYINDEX(pFields, entity_field_alias[FIELD_ORIGIN0].field);
		DELTA_UNSETBYINDEX(pFields, entity_field_alias[FIELD_ORIGIN1].field);
		DELTA_UNSETBYINDEX(pFields, entity_field_alias[FIELD_ORIGIN2].field);
	}

	if ((t->movetype == MOVETYPE_FOLLOW) &&
		(t->aiment != 0))
	{
		DELTA_UNSETBYINDEX(pFields, entity_field_alias[FIELD_ORIGIN0].field);
		DELTA_UNSETBYINDEX(pFields, entity_field_alias[FIELD_ORIGIN1].field);
		DELTA_UNSETBYINDEX(pFields, entity_field_alias[FIELD_ORIGIN2].field);
	}
	else if (t->aiment != f->aiment)
	{
		DELTA_SETBYINDEX(pFields, entity_field_alias[FIELD_ORIGIN0].field);
		DELTA_SETBYINDEX(pFields, entity_field_alias[FIELD_ORIGIN1].field);
		DELTA_SETBYINDEX(pFields, entity_field_alias[FIELD_ORIGIN2].field);
	}
}

#define CUSTOMFIELD_ORIGIN0 0
#define CUSTOMFIELD_ORIGIN1 1
#define CUSTOMFIELD_ORIGIN2 2
#define CUSTOMFIELD_ANGLES0 3
#define CUSTOMFIELD_ANGLES1 4
#define CUSTOMFIELD_ANGLES2 5
#define CUSTOMFIELD_SKIN 6
#define CUSTOMFIELD_SEQUENCE 7
#define CUSTOMFIELD_ANIMTIME 8

entity_field_alias_t custom_entity_field_alias[] =
	{
		{"origin[0]", 0},
		{"origin[1]", 0},
		{"origin[2]", 0},
		{"angles[0]", 0},
		{"angles[1]", 0},
		{"angles[2]", 0},
		{"skin", 0},
		{"sequence", 0},
		{"animtime", 0},
};

void Custom_Entity_FieldInit(struct delta_s* pFields)
{
	custom_entity_field_alias[CUSTOMFIELD_ORIGIN0].field = DELTA_FINDFIELD(pFields, custom_entity_field_alias[CUSTOMFIELD_ORIGIN0].name);
	custom_entity_field_alias[CUSTOMFIELD_ORIGIN1].field = DELTA_FINDFIELD(pFields, custom_entity_field_alias[CUSTOMFIELD_ORIGIN1].name);
	custom_entity_field_alias[CUSTOMFIELD_ORIGIN2].field = DELTA_FINDFIELD(pFields, custom_entity_field_alias[CUSTOMFIELD_ORIGIN2].name);
	custom_entity_field_alias[CUSTOMFIELD_ANGLES0].field = DELTA_FINDFIELD(pFields, custom_entity_field_alias[CUSTOMFIELD_ANGLES0].name);
	custom_entity_field_alias[CUSTOMFIELD_ANGLES1].field = DELTA_FINDFIELD(pFields, custom_entity_field_alias[CUSTOMFIELD_ANGLES1].name);
	custom_entity_field_alias[CUSTOMFIELD_ANGLES2].field = DELTA_FINDFIELD(pFields, custom_entity_field_alias[CUSTOMFIELD_ANGLES2].name);
	custom_entity_field_alias[CUSTOMFIELD_SKIN].field = DELTA_FINDFIELD(pFields, custom_entity_field_alias[CUSTOMFIELD_SKIN].name);
	custom_entity_field_alias[CUSTOMFIELD_SEQUENCE].field = DELTA_FINDFIELD(pFields, custom_entity_field_alias[CUSTOMFIELD_SEQUENCE].name);
	custom_entity_field_alias[CUSTOMFIELD_ANIMTIME].field = DELTA_FINDFIELD(pFields, custom_entity_field_alias[CUSTOMFIELD_ANIMTIME].name);
}

/*
==================
Custom_Encode

Callback for sending entity_state_t info ( for custom entities ) over network. 
FIXME:  Move to script
==================
*/
void Custom_Encode(struct delta_s* pFields, const unsigned char* from, const unsigned char* to)
{
	entity_state_t *f, *t;
	int beamType;
	static bool initialized = false;

	if (!initialized)
	{
		Custom_Entity_FieldInit(pFields);
		initialized = true;
	}

	f = (entity_state_t*)from;
	t = (entity_state_t*)to;

	beamType = t->rendermode & 0x0f;

	if (beamType != BEAM_POINTS && beamType != BEAM_ENTPOINT)
	{
		DELTA_UNSETBYINDEX(pFields, custom_entity_field_alias[CUSTOMFIELD_ORIGIN0].field);
		DELTA_UNSETBYINDEX(pFields, custom_entity_field_alias[CUSTOMFIELD_ORIGIN1].field);
		DELTA_UNSETBYINDEX(pFields, custom_entity_field_alias[CUSTOMFIELD_ORIGIN2].field);
	}

	if (beamType != BEAM_POINTS)
	{
		DELTA_UNSETBYINDEX(pFields, custom_entity_field_alias[CUSTOMFIELD_ANGLES0].field);
		DELTA_UNSETBYINDEX(pFields, custom_entity_field_alias[CUSTOMFIELD_ANGLES1].field);
		DELTA_UNSETBYINDEX(pFields, custom_entity_field_alias[CUSTOMFIELD_ANGLES2].field);
	}

	if (beamType != BEAM_ENTS && beamType != BEAM_ENTPOINT)
	{
		DELTA_UNSETBYINDEX(pFields, custom_entity_field_alias[CUSTOMFIELD_SKIN].field);
		DELTA_UNSETBYINDEX(pFields, custom_entity_field_alias[CUSTOMFIELD_SEQUENCE].field);
	}

	// animtime is compared by rounding first
	// see if we really shouldn't actually send it
	if ((int)f->animtime == (int)t->animtime)
	{
		DELTA_UNSETBYINDEX(pFields, custom_entity_field_alias[CUSTOMFIELD_ANIMTIME].field);
	}
}

/*
=================
RegisterEncoders

Allows game .dll to override network encoding of certain types of entities and tweak values, etc.
=================
*/
void RegisterEncoders()
{
	DELTA_ADDENCODER("Entity_Encode", Entity_Encode);
	DELTA_ADDENCODER("Custom_Encode", Custom_Encode);
	DELTA_ADDENCODER("Player_Encode", Player_Encode);
}

int GetWeaponData(struct edict_s* player, struct weapon_data_s* info)
{
	memset(info, 0, MAX_WEAPONS * sizeof(weapon_data_t));

#if defined(CLIENT_WEAPONS)
	int i;
	weapon_data_t* item;
	entvars_t* pev = &player->v;
	CBasePlayer* pl = dynamic_cast<CBasePlayer*>(CBasePlayer::Instance(pev));
	CBasePlayerWeapon* gun;

	ItemInfo II;

	if (!pl)
		return 1;

	// go through all of the weapons and make a list of the ones to pack
	for (i = 0; i < MAX_ITEM_TYPES; i++)
	{
		if (pl->m_rgpPlayerItems[i])
		{
			// there's a weapon here. Should I pack it?
			CBasePlayerItem* pPlayerItem = pl->m_rgpPlayerItems[i];

			while (pPlayerItem)
			{
				gun = pPlayerItem->GetWeaponPtr();
				if (gun && gun->UseDecrement())
				{
					// Get The ID.
					memset(&II, 0, sizeof(II));
					gun->GetItemInfo(&II);

					if (II.iId >= 0 && II.iId < MAX_WEAPONS)
					{
						item = &info[II.iId];

						item->m_iId = II.iId;
						item->m_iClip = gun->m_iClip;

						item->m_flTimeWeaponIdle = V_max(gun->m_flTimeWeaponIdle, -0.001);
						item->m_flNextPrimaryAttack = V_max(gun->m_flNextPrimaryAttack, -0.001);
						item->m_flNextSecondaryAttack = V_max(gun->m_flNextSecondaryAttack, -0.001);
						item->m_fInReload = static_cast<int>(gun->m_fInReload);
						item->m_fInSpecialReload = gun->m_fInSpecialReload;
						item->fuser1 = V_max(gun->pev->fuser1, -0.001);
						item->fuser2 = gun->m_flStartThrow;
						item->fuser3 = gun->m_flReleaseThrow;
						item->iuser1 = gun->m_chargeReady;
						item->iuser2 = gun->m_fInAttack;
						item->iuser3 = gun->m_fireState;

						gun->GetWeaponData(*item);

						//						item->m_flPumpTime				= V_max( gun->m_flPumpTime, -0.001 );
					}
				}
				pPlayerItem = pPlayerItem->m_pNext;
			}
		}
	}
#endif
	return 1;
}

/*
=================
UpdateClientData

Data sent to current client only
engine sets cd to 0 before calling.
=================
*/
void UpdateClientData(const edict_t* ent, int sendweapons, struct clientdata_s* cd)
{
	if (!ent || !ent->pvPrivateData)
		return;
	entvars_t* pev = (entvars_t*)&ent->v;
	CBasePlayer* pl = dynamic_cast<CBasePlayer*>(CBasePlayer::Instance(pev));
	entvars_t* pevOrg = NULL;

	// if user is spectating different player in First person, override some vars
	if (pl && pl->pev->iuser1 == OBS_IN_EYE)
	{
		if (pl->m_hObserverTarget)
		{
			pevOrg = pev;
			pev = pl->m_hObserverTarget->pev;
			pl = dynamic_cast<CBasePlayer*>(CBasePlayer::Instance(pev));
		}
	}

	cd->flags = pev->flags;
	cd->health = pev->health;

	cd->viewmodel = MODEL_INDEX(STRING(pev->viewmodel));

	cd->waterlevel = pev->waterlevel;
	cd->watertype = pev->watertype;
	cd->weapons = pev->weapons;

	// Vectors
	cd->origin = pev->origin;
	cd->velocity = pev->velocity;
	cd->view_ofs = pev->view_ofs;
	cd->punchangle = pev->punchangle;

	cd->bInDuck = pev->bInDuck;
	cd->flTimeStepSound = pev->flTimeStepSound;
	cd->flDuckTime = pev->flDuckTime;
	cd->flSwimTime = pev->flSwimTime;
	cd->waterjumptime = pev->teleport_time;

	strcpy(cd->physinfo, ENGINE_GETPHYSINFO(ent));

	cd->maxspeed = pev->maxspeed;
	cd->fov = pl->m_iFOV;
	cd->weaponanim = pev->weaponanim;

	cd->pushmsec = pev->pushmsec;

	//Spectator mode
	if (pevOrg != NULL)
	{
		// don't use spec vars from chased player
		cd->iuser1 = pevOrg->iuser1;
		cd->iuser2 = pevOrg->iuser2;
	}
	else
	{
		cd->iuser1 = pev->iuser1;
		cd->iuser2 = pev->iuser2;
	}



#if defined(CLIENT_WEAPONS)
	if (0 != sendweapons)
	{
		if (pl)
		{
			cd->m_flNextAttack = pl->m_flNextAttack;
			cd->fuser2 = pl->m_flNextAmmoBurn;
			cd->fuser3 = pl->m_flAmmoStartCharge;
			cd->vuser1.x = pl->ammo_9mm;
			cd->vuser1.y = pl->ammo_357;
			cd->vuser1.z = pl->ammo_argrens;
			cd->ammo_nails = pl->ammo_bolts;
			cd->ammo_shells = pl->ammo_buckshot;
			cd->ammo_rockets = pl->ammo_rockets;
			cd->ammo_cells = pl->ammo_uranium;
			cd->vuser2.x = pl->ammo_hornets;


			if (pl->m_pActiveItem)
			{
				CBasePlayerWeapon* gun = pl->m_pActiveItem->GetWeaponPtr();
				if (gun && gun->UseDecrement())
				{
					ItemInfo II;
					memset(&II, 0, sizeof(II));
					gun->GetItemInfo(&II);

					cd->m_iId = II.iId;

					cd->vuser3.z = gun->m_iSecondaryAmmoType;
					cd->vuser4.x = gun->m_iPrimaryAmmoType;
					cd->vuser4.y = pl->m_rgAmmo[gun->m_iPrimaryAmmoType];
					cd->vuser4.z = pl->m_rgAmmo[gun->m_iSecondaryAmmoType];

					if (pl->m_pActiveItem->m_iId == WEAPON_RPG)
					{
						cd->vuser2.y = static_cast<vec_t>(((CRpg*)pl->m_pActiveItem)->m_fSpotActive);
						cd->vuser2.z = ((CRpg*)pl->m_pActiveItem)->m_cActiveRockets;
					}
				}
			}
		}
	}
#endif
}

/*
=================
CmdStart

We're about to run this usercmd for the specified player.  We can set up groupinfo and masking here, etc.
This is the time to examine the usercmd for anything extra.  This call happens even if think does not.
=================
*/
void CmdStart(const edict_t* player, const struct usercmd_s* cmd, unsigned int random_seed)
{
	entvars_t* pev = (entvars_t*)&player->v;
	CBasePlayer* pl = dynamic_cast<CBasePlayer*>(CBasePlayer::Instance(pev));

	if (!pl)
		return;

	if (pl->pev->groupinfo != 0)
	{
		UTIL_SetGroupTrace(pl->pev->groupinfo, GROUP_OP_AND);
	}

	pl->random_seed = random_seed;
}

/*
=================
CmdEnd

Each cmdstart is exactly matched with a cmd end, clean up any group trace flags, etc. here
=================
*/
void CmdEnd(const edict_t* player)
{
	entvars_t* pev = (entvars_t*)&player->v;
	CBasePlayer* pl = dynamic_cast<CBasePlayer*>(CBasePlayer::Instance(pev));

	if (!pl)
		return;
	if (pl->pev->groupinfo != 0)
	{
		UTIL_UnsetGroupTrace();
	}
}

/*
================================
ConnectionlessPacket

 Return 1 if the packet is valid.  Set response_buffer_size if you want to send a response packet.  Incoming, it holds the max
  size of the response_buffer, so you must zero it out if you choose not to respond.
================================
*/
int ConnectionlessPacket(const struct netadr_s* net_from, const char* args, char* response_buffer, int* response_buffer_size)
{
	// Parse stuff from args
	int max_buffer_size = *response_buffer_size;

	// Zero it out since we aren't going to respond.
	// If we wanted to response, we'd write data into response_buffer
	*response_buffer_size = 0;

	// Since we don't listen for anything here, just respond that it's a bogus message
	// If we didn't reject the message, we'd return 1 for success instead.
	return 0;
}

/*
================================
GetHullBounds

  Engine calls this to enumerate player collision hulls, for prediction.  Return 0 if the hullnumber doesn't exist.
================================
*/
int GetHullBounds(int hullnumber, float* mins, float* maxs)
{
	return static_cast<int>(PM_GetHullBounds(hullnumber, mins, maxs));
}

/*
================================
CreateInstancedBaselines

Create pseudo-baselines for items that aren't placed in the map at spawn time, but which are likely
to be created during play ( e.g., grenades, ammo packs, projectiles, corpses, etc. )
================================
*/
void CreateInstancedBaselines()
{
	int iret = 0;
	entity_state_t state;

	memset(&state, 0, sizeof(state));

	// Create any additional baselines here for things like grendates, etc.
	// iret = ENGINE_INSTANCE_BASELINE( pc->pev->classname, &state );

	// Destroy objects.
	//UTIL_Remove( pc );
}

/*
================================
InconsistentFile

One of the ENGINE_FORCE_UNMODIFIED files failed the consistency check for the specified player
 Return 0 to allow the client to continue, 1 to force immediate disconnection ( with an optional disconnect message of up to 256 characters )
================================
*/
int InconsistentFile(const edict_t* player, const char* filename, char* disconnect_message)
{
	// Server doesn't care?
	if (CVAR_GET_FLOAT("mp_consistency") != 1)
		return 0;

	// Default behavior is to kick the player
	sprintf(disconnect_message, "Server is enforcing file consistency for %s\n", filename);

	// Kick now with specified disconnect message.
	return 1;
}

/*
================================
AllowLagCompensation

 The game .dll should return 1 if lag compensation should be allowed ( could also just set
  the sv_unlag cvar.
 Most games right now should return 0, until client-side weapon prediction code is written
  and tested for them ( note you can predict weapons, but not do lag compensation, too, 
  if you want.
================================
*/
int AllowLagCompensation()
{
	return 1;
}
