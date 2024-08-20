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

#include "extdll.h"

void UpdateClientData(const struct edict_s* ent, int sendweapons, struct clientdata_s* cd);
int AddToFullPack(struct entity_state_s* state, int e, edict_t* ent, edict_t* host, int hostflags, int player, unsigned char* pSet);
void CreateBaseline(int player, int eindex, struct entity_state_s* baseline, struct edict_s* entity, int playermodelindex, Vector* player_mins, Vector* player_maxs);
void RegisterEncoders();

int ConnectionlessPacket(const struct netadr_s* net_from, const char* args, char* response_buffer, int* response_buffer_size);
void CreateInstancedBaselines();
int InconsistentFile(const edict_t* player, const char* filename, char* disconnect_message);
int AllowLagCompensation();
