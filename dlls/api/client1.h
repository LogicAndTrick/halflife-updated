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

qboolean ClientConnect(edict_t* pEntity, const char* pszName, const char* pszAddress, char szRejectReason[128]);
void ClientDisconnect(edict_t* pEntity);
void ClientKill(edict_t* pEntity);
void ClientPutInServer(edict_t* pEntity);
void ClientCommand(edict_t* pEntity);
void ClientUserInfoChanged(edict_t* pEntity, char* infobuffer);
