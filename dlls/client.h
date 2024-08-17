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

#pragma once

void respawn(entvars_t* pev, bool fCopyCorpse);

extern void ClientPrecache();

extern const char* GetGameDescription();
extern void PlayerCustomization(edict_t* pEntity, customization_t* pCust);

extern void SpectatorConnect(edict_t* pEntity);
extern void SpectatorDisconnect(edict_t* pEntity);
extern void SpectatorThink(edict_t* pEntity);

extern void Sys_Error(const char* error_string);

extern void SetupVisibility(edict_t* pViewEntity, edict_t* pClient, unsigned char** pvs, unsigned char** pas);
extern void UpdateClientData(const struct edict_s* ent, int sendweapons, struct clientdata_s* cd);
extern int AddToFullPack(struct entity_state_s* state, int e, edict_t* ent, edict_t* host, int hostflags, int player, unsigned char* pSet);
extern void CreateBaseline(int player, int eindex, struct entity_state_s* baseline, struct edict_s* entity, int playermodelindex, Vector* player_mins, Vector* player_maxs);
extern void RegisterEncoders();

extern int GetWeaponData(struct edict_s* player, struct weapon_data_s* info);

extern void CmdStart(const edict_t* player, const struct usercmd_s* cmd, unsigned int random_seed);
extern void CmdEnd(const edict_t* player);

extern int ConnectionlessPacket(const struct netadr_s* net_from, const char* args, char* response_buffer, int* response_buffer_size);

extern int GetHullBounds(int hullnumber, float* mins, float* maxs);

extern void CreateInstancedBaselines();

extern int InconsistentFile(const edict_t* player, const char* filename, char* disconnect_message);

extern int AllowLagCompensation();
