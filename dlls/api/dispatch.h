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

int DispatchSpawn(edict_t* pent);
void DispatchKeyValue(edict_t* pentKeyvalue, KeyValueData* pkvd);
void DispatchTouch(edict_t* pentTouched, edict_t* pentOther);
void DispatchUse(edict_t* pentUsed, edict_t* pentOther);
void DispatchThink(edict_t* pent);
void DispatchBlocked(edict_t* pentBlocked, edict_t* pentOther);
void DispatchSave(edict_t* pent, SAVERESTOREDATA* pSaveData);
int DispatchRestore(edict_t* pent, SAVERESTOREDATA* pSaveData, int globalEntity);
void DispatchObjectCollsionBox(edict_t* pent);
void OnFreeEntPrivateData(edict_s* pEdict);
