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

#include "saverestore.h"
#include "classes/saverestore/CRestore.h"
#include "classes/saverestore/CSave.h"
#include "pm_shared.h"
#include "classes/saverestore/CGlobalState.h"
#include "entities/player/CBasePlayer.h"

void SaveWriteFields(SAVERESTOREDATA* pSaveData, const char* pname, void* pBaseData, TYPEDESCRIPTION* pFields, int fieldCount)
{
	if (!CSaveRestoreBuffer::IsValidSaveRestoreData(pSaveData))
	{
		return;
	}

	CSave saveHelper(*pSaveData);
	saveHelper.WriteFields("SWF", pname, pBaseData, pFields, fieldCount);
}

void SaveReadFields(SAVERESTOREDATA* pSaveData, const char* pname, void* pBaseData, TYPEDESCRIPTION* pFields, int fieldCount)
{
	if (!CSaveRestoreBuffer::IsValidSaveRestoreData(pSaveData))
	{
		return;
	}

	// Always check if the player is stuck when loading a save game.
	g_CheckForPlayerStuck = true;

	CRestore restoreHelper(*pSaveData);
	restoreHelper.ReadFields(pname, pBaseData, pFields, fieldCount);
}

void SaveGlobalState(SAVERESTOREDATA* pSaveData)
{
	if (!CSaveRestoreBuffer::IsValidSaveRestoreData(pSaveData))
	{
		return;
	}

	CSave saveHelper(*pSaveData);
	gGlobalState.Save(saveHelper);
}

void RestoreGlobalState(SAVERESTOREDATA* pSaveData)
{
	if (!CSaveRestoreBuffer::IsValidSaveRestoreData(pSaveData))
	{
		return;
	}

	CRestore restoreHelper(*pSaveData);
	gGlobalState.Restore(restoreHelper);
}

void ResetGlobalState()
{
	gGlobalState.ClearStates();
	gInitHUD = true; // Init the HUD on a new game / load game
}
