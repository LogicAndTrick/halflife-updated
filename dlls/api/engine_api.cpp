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

#include "entities/CBaseEntity.h"
#include "api/cmd.h"
#include "api/frame.h"
#include "api/network.h"
#include "api/player.h"
#include "api/game.h"
#include "api/client.h"
#include "api/dispatch.h"
#include "api/saverestore.h"
#include "api/server.h"
#include "api/spectator.h"

#include "pm_shared.h"

static DLL_FUNCTIONS gFunctionTable =
	{
		GameDLLInit,			   // pfnGameInit

		DispatchSpawn,			   // pfnSpawn
		DispatchThink,			   // pfnThink
		DispatchUse,			   // pfnUse
		DispatchTouch,			   // pfnTouch
		DispatchBlocked,		   // pfnBlocked
		DispatchKeyValue,		   // pfnKeyValue
		DispatchSave,			   // pfnSave
		DispatchRestore,		   // pfnRestore
		DispatchObjectCollsionBox, // pfnAbsBox

		SaveWriteFields, // pfnSaveWriteFields
		SaveReadFields,	 // pfnSaveReadFields

		SaveGlobalState,	// pfnSaveGlobalState
		RestoreGlobalState, // pfnRestoreGlobalState
		ResetGlobalState,	// pfnResetGlobalState

		ClientConnect,		   // pfnClientConnect
		ClientDisconnect,	   // pfnClientDisconnect
		ClientKill,			   // pfnClientKill
		ClientPutInServer,	   // pfnClientPutInServer
		ClientCommand,		   // pfnClientCommand
		ClientUserInfoChanged, // pfnClientUserInfoChanged
		ServerActivate,		   // pfnServerActivate
		ServerDeactivate,	   // pfnServerDeactivate

		PlayerPreThink,	 // pfnPlayerPreThink
		PlayerPostThink, // pfnPlayerPostThink

		StartFrame,		  // pfnStartFrame
		ParmsNewLevel,	  // pfnParmsNewLevel
		ParmsChangeLevel, // pfnParmsChangeLevel

		GetGameDescription,	 // pfnGetGameDescription    Returns string describing current .dll game.
		PlayerCustomization, // pfnPlayerCustomization   Notifies .dll of new customization for player.

		SpectatorConnect,	 // pfnSpectatorConnect      Called when spectator joins server
		SpectatorDisconnect, // pfnSpectatorDisconnect   Called when spectator leaves the server
		SpectatorThink,		 // pfnSpectatorThink        Called when spectator sends a command packet (usercmd_t)

		Sys_Error, // pfnSys_Error				Called when engine has encountered an error

		PM_Move,			// pfnPM_Move
		PM_Init,			// pfnPM_Init				Server version of player movement initialization
		PM_FindTextureType, // pfnPM_FindTextureType

		SetupVisibility,		  // pfnSetupVisibility        Set up PVS and PAS for networking for this client
		UpdateClientData,		  // pfnUpdateClientData       Set up data sent only to specific client
		AddToFullPack,			  // pfnAddToFullPack
		CreateBaseline,			  // pfnCreateBaseline			Tweak entity baseline for network encoding, allows setup of player baselines, too.
		RegisterEncoders,		  // pfnRegisterEncoders		Callbacks for network encoding
		GetWeaponData,			  // pfnGetWeaponData
		CmdStart,				  // pfnCmdStart
		CmdEnd,					  // pfnCmdEnd
		ConnectionlessPacket,	  // pfnConnectionlessPacket
		GetHullBounds,			  // pfnGetHullBounds
		CreateInstancedBaselines, // pfnCreateInstancedBaselines
		InconsistentFile,		  // pfnInconsistentFile
		AllowLagCompensation,	  // pfnAllowLagCompensation
};

NEW_DLL_FUNCTIONS gNewDLLFunctions =
	{
		OnFreeEntPrivateData, // pfnOnFreeEntPrivateData
		GameDLLShutdown,
};

extern "C" {

int GetEntityAPI(DLL_FUNCTIONS* pFunctionTable, int interfaceVersion)
{
	if (!pFunctionTable || interfaceVersion != INTERFACE_VERSION)
	{
		return 0;
	}

	memcpy(pFunctionTable, &gFunctionTable, sizeof(DLL_FUNCTIONS));
	return 1;
}

int GetEntityAPI2(DLL_FUNCTIONS* pFunctionTable, int* interfaceVersion)
{
	if (!pFunctionTable || *interfaceVersion != INTERFACE_VERSION)
	{
		// Tell engine what version we had, so it can figure out who is out of date.
		*interfaceVersion = INTERFACE_VERSION;
		return 0;
	}

	memcpy(pFunctionTable, &gFunctionTable, sizeof(DLL_FUNCTIONS));
	return 1;
}

int GetNewDLLFunctions(NEW_DLL_FUNCTIONS* pFunctionTable, int* interfaceVersion)
{
	if (!pFunctionTable || *interfaceVersion != NEW_DLL_FUNCTIONS_VERSION)
	{
		*interfaceVersion = NEW_DLL_FUNCTIONS_VERSION;
		return 0;
	}

	memcpy(pFunctionTable, &gNewDLLFunctions, sizeof(gNewDLLFunctions));
	return 1;
}
}
