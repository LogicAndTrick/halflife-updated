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

void PlayerPostThink(edict_t* pEntity);
void PlayerPreThink(edict_t* pEntity);

void PlayerCustomization(edict_t* pEntity, customization_t* pCust);

void SetupVisibility(edict_t* pViewEntity, edict_t* pClient, unsigned char** pvs, unsigned char** pas);
int GetWeaponData(struct edict_s* player, struct weapon_data_s* info);
int GetHullBounds(int hullnumber, float* mins, float* maxs);
