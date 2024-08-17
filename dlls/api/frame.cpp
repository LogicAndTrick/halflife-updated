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

#include <algorithm>

#include "frame.h"
#include "entities/CBaseEntity.h"
#include "filesystem_utils.h"
#include "game.h"
#include "movewith.h"
#include "pm_defs.h"
#include "pm_shared.h"
#include "classes/gamerules/CGameRules.h"

static bool g_LastAllowBunnyHoppingState = false;
static unsigned int g_ulFrameCount = 0;
static std::vector<std::string> g_MapsToLoad;

static void LoadNextMap();

void StartFrame()
{
	if (g_pGameRules)
		g_pGameRules->Think();

	if (g_fGameOver)
		return;

	gpGlobals->teamplay = teamplay.value;
	g_ulFrameCount++;

	const bool allowBunnyHopping = sv_allowbunnyhopping.value != 0;

	if (allowBunnyHopping != g_LastAllowBunnyHoppingState)
	{
		g_LastAllowBunnyHoppingState = allowBunnyHopping;

		for (int i = 1; i <= gpGlobals->maxClients; ++i)
		{
			auto player = UTIL_PlayerByIndex(i);

			if (!player)
			{
				continue;
			}

			g_engfuncs.pfnSetPhysicsKeyValue(player->edict(), "bj", UTIL_dtos1(allowBunnyHopping ? 1 : 0));
		}
	}

	// If we're loading all maps then change maps after 3 seconds (time starts at 1)
	// to give the game time to generate files.
	if (!g_MapsToLoad.empty() && gpGlobals->time > 4)
	{
		LoadNextMap();
	}

	//	CheckDesiredList(); //LRC
	CheckAssistList(); // LRC
}

static void LoadNextMap()
{
	const std::string mapName = std::move(g_MapsToLoad.back());
	g_MapsToLoad.pop_back();

	pmove->Con_Printf("Loading map \"%s\" automatically (%d left)\n",
		mapName.c_str(), static_cast<int>(g_MapsToLoad.size() + 1));

	if (g_MapsToLoad.empty())
	{
		pmove->Con_Printf("Loading last map\n");
		g_MapsToLoad.shrink_to_fit();
	}

	SERVER_COMMAND(UTIL_VarArgs("map \"%s\"\n", mapName.c_str()));
}

static void LoadAllMaps()
{
	if (!g_MapsToLoad.empty())
	{
		pmove->Con_Printf("Already loading all maps (%d remaining)\nUse sv_stop_loading_all_maps to stop\n",
			static_cast<int>(g_MapsToLoad.size()));
		return;
	}

	FileFindHandle_t handle = FILESYSTEM_INVALID_FIND_HANDLE;

	const char* fileName = g_pFileSystem->FindFirst("maps/*.bsp", &handle);

	if (fileName != nullptr)
	{
		do
		{
			std::string mapName = fileName;
			mapName.resize(mapName.size() - 4);

			if (std::find_if(g_MapsToLoad.begin(), g_MapsToLoad.end(), [=](const auto& candidate)
					{ return 0 == stricmp(candidate.c_str(), mapName.c_str()); }) == g_MapsToLoad.end())
			{
				g_MapsToLoad.push_back(std::move(mapName));
			}
		} while ((fileName = g_pFileSystem->FindNext(handle)) != nullptr);

		g_pFileSystem->FindClose(handle);

		// Sort in reverse order so the first map in alphabetic order is loaded first.
		std::sort(g_MapsToLoad.begin(), g_MapsToLoad.end(), [](const auto& lhs, const auto& rhs)
			{ return rhs < lhs; });
	}

	if (!g_MapsToLoad.empty())
	{
		if (CMD_ARGC() == 2)
		{
			const char* firstMapToLoad = CMD_ARGV(1);

			// Clear out all maps that would have been loaded before this one.
			if (auto it = std::find(g_MapsToLoad.begin(), g_MapsToLoad.end(), firstMapToLoad);
				it != g_MapsToLoad.end())
			{
				const std::size_t numberOfMapsToSkip = g_MapsToLoad.size() - (it - g_MapsToLoad.begin());

				g_MapsToLoad.erase(it + 1, g_MapsToLoad.end());

				pmove->Con_Printf("Skipping %d maps to start with \"%s\"\n",
					static_cast<int>(numberOfMapsToSkip), g_MapsToLoad.back().c_str());
			}
			else
			{
				pmove->Con_Printf("Unknown map \"%s\", starting from beginning\n", firstMapToLoad);
			}
		}

		pmove->Con_Printf("Loading %d maps one at a time to generate files\n", static_cast<int>(g_MapsToLoad.size()));

		// Load the first map right now.
		LoadNextMap();
	}
	else
	{
		pmove->Con_Printf("No maps to load\n");
	}
}

void InitMapLoadingUtils()
{
	g_engfuncs.pfnAddServerCommand("sv_load_all_maps", &LoadAllMaps);
	// Escape hatch in case the command is executed in error.
	g_engfuncs.pfnAddServerCommand("sv_stop_loading_all_maps", []()
		{ g_MapsToLoad.clear(); });
}
