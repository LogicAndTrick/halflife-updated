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

class CBaseEntity;

void UTIL_ShowMessage(const char* pString, CBaseEntity* pPlayer);
void UTIL_ShowMessageAll(const char* pString);

// prints messages through the HUD
void ClientPrint(entvars_t* client, int msg_dest, const char* msg_name, const char* param1 = NULL, const char* param2 = NULL, const char* param3 = NULL, const char* param4 = NULL);

// prints a message to each client
void UTIL_ClientPrintAll(int msg_dest, const char* msg_name, const char* param1 = NULL, const char* param2 = NULL, const char* param3 = NULL, const char* param4 = NULL);
inline void UTIL_CenterPrintAll(const char* msg_name, const char* param1 = NULL, const char* param2 = NULL, const char* param3 = NULL, const char* param4 = NULL)
{
	UTIL_ClientPrintAll(HUD_PRINTCENTER, msg_name, param1, param2, param3, param4);
}

// prints a message to the HUD say (chat)
void UTIL_SayText(const char* pText, CBaseEntity* pEntity);
void UTIL_SayTextAll(const char* pText, CBaseEntity* pEntity);

typedef struct hudtextparms_s
{
	float x;
	float y;
	int effect;
	byte r1, g1, b1, a1;
	byte r2, g2, b2, a2;
	float fadeinTime;
	float fadeoutTime;
	float holdTime;
	float fxTime;
	int channel;
} hudtextparms_t;

// prints as transparent 'title' to the HUD
void UTIL_HudMessage(CBaseEntity* pEntity, const hudtextparms_t& textparms, const char* pMessage);
void UTIL_HudMessageAll(const hudtextparms_t& textparms, const char* pMessage);

// for handy use with ClientPrint params
char* UTIL_dtos1(int d);

// Writes message to console with timestamp and FragLog header.
void UTIL_LogPrintf(const char* fmt, ...);
