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
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "client.h"
#include "classes/gamerules/CGameRules.h"
#include "pm_shared.h"
#include "movewith.h"
#include "skill.h"
#include "classes/saverestore/CGlobalState.h"
#include "classes/saverestore/CRestore.h"
#include "classes/saverestore/CSave.h"
#include "classes/saverestore/CSaveRestoreBuffer.h"

void OnFreeEntPrivateData(edict_s* pEdict)
{
	if (pEdict && pEdict->pvPrivateData)
	{
		auto entity = reinterpret_cast<CBaseEntity*>(pEdict->pvPrivateData);

		delete entity;

		//Zero this out so the engine doesn't try to free it again.
		pEdict->pvPrivateData = nullptr;
	}
}
