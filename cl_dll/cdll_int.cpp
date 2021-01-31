/***
*
*	Copyright (c) 1999, Valve LLC. All rights reserved.
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
//
//  cdll_int.c
//
// this implementation handles the linking of the engine to the DLL
//

#include "hud.h"
#include "cl_util.h"
#include "netadr.h"
#undef INTERFACE_H
#include "../public/interface.h"
//#include "vgui_schememanager.h"
#include "mp3.h" //AJH - Killars MP3player

#include "pm_shared.h"

#include <string.h>
#include "vgui_int.h"
#include "interface.h"

#include "Platform.h"
#include "Exports.h"

#include "tri.h"
#include "vgui_TeamFortressViewport.h"
#include "../public/interface.h"

cl_enginefunc_t gEngfuncs;
CHud gHUD;
CMP3 gMP3; //AJH - Killars MP3player
TeamFortressViewport *gViewPort = NULL;


#include "particleman.h"
CSysModule *g_hParticleManModule = NULL;
IParticleMan *g_pParticleMan = NULL;

void CL_LoadParticleMan();
void CL_UnloadParticleMan();

void InitInput ();
void EV_HookEvents();
void IN_Commands();

// SCREEN GLOW FragBait0
extern void InitScreenGlow(); // FragBait0 - Glow Effect
extern void RenderScreenGlow(); // FragBait0 - Glow Effect

/*
================================
HUD_GetHullBounds

  Engine calls this to enumerate player collision hulls, for prediction.  Return 0 if the hullnumber doesn't exist.
================================
*/
int DLLEXPORT HUD_GetHullBounds( int hullnumber, float *mins, float *maxs )
{
//	RecClGetHullBounds(hullnumber, mins, maxs);

	int iret = 0;

	switch (hullnumber)
	{
	case 0:				// Normal player
		memcpy(mins, &VEC_HULL_MIN, sizeof(VEC_HULL_MIN));
		memcpy(maxs, &VEC_HULL_MAX, sizeof(VEC_HULL_MAX));
		iret = 1;
		break;
	case 1:				// Crouched player
		memcpy(mins, &VEC_DUCK_HULL_MIN, sizeof(VEC_DUCK_HULL_MIN));
		memcpy(maxs, &VEC_DUCK_HULL_MAX, sizeof(VEC_DUCK_HULL_MAX));
		iret = 1;
		break;
	case 2:				// Point based hull
		memcpy(mins, &g_vecZero, sizeof(g_vecZero));
		memcpy(maxs, &g_vecZero, sizeof(g_vecZero));
		iret = 1;
		break;
	}

	return iret;
}

/*
================================
HUD_ConnectionlessPacket

 Return 1 if the packet is valid.  Set response_buffer_size if you want to send a response packet.  Incoming, it holds the max
  size of the response_buffer, so you must zero it out if you choose not to respond.
================================
*/
int	DLLEXPORT HUD_ConnectionlessPacket( const struct netadr_s *net_from, const char *args, char *response_buffer, int *response_buffer_size )
{
//	RecClConnectionlessPacket(net_from, args, response_buffer, response_buffer_size);

	// Parse stuff from args
	int max_buffer_size = *response_buffer_size;

	// Zero it out since we aren't going to respond.
	// If we wanted to response, we'd write data into response_buffer
	*response_buffer_size = 0;

	// Since we don't listen for anything here, just respond that it's a bogus message
	// If we didn't reject the message, we'd return 1 for success instead.
	return 0;
}

void DLLEXPORT HUD_PlayerMoveInit( struct playermove_s *ppmove )
{
//	RecClClientMoveInit(ppmove);

	PM_Init( ppmove );
}

char DLLEXPORT HUD_PlayerMoveTexture( char *name )
{
//	RecClClientTextureType(name);

	return PM_FindTextureType( name );
}

void DLLEXPORT HUD_PlayerMove( struct playermove_s *ppmove, int server )
{
//	RecClClientMove(ppmove, server);

	PM_Move( ppmove, server );
}

int DLLEXPORT Initialize( cl_enginefunc_t *pEnginefuncs, int iVersion )
{
	gEngfuncs = *pEnginefuncs;

//	RecClInitialize(pEnginefuncs, iVersion);

	if (iVersion != CLDLL_INTERFACE_VERSION)
		return 0;

	memcpy(&gEngfuncs, pEnginefuncs, sizeof(cl_enginefunc_t));

	EV_HookEvents();
	CL_LoadParticleMan();

	// get tracker interface, if any
	return 1;
}


/*
==========================
	HUD_VidInit

Called when the game initializes
and whenever the vid_mode is changed
so the HUD can reinitialize itself.
==========================
*/

int DLLEXPORT HUD_VidInit()
{
//	RecClHudVidInit();
	gHUD.VidInit();

	VGui_Startup();
	
//LRCTEMP 1.8	if (CVAR_GET_FLOAT("r_glow") != 0)	 //check the cvar for the glow is on.//AJH Modified to include glow mode (1&2)
//LRCTEMP 1.8		InitScreenGlow(); // glow effect --FragBait0

	return 1;
}

/*
==========================
	HUD_Init

Called whenever the client connects
to a server.  Reinitializes all 
the hud variables.
==========================
*/

void DLLEXPORT HUD_Init()
{
//	RecClHudInit();
	InitInput();
	gHUD.Init();
	Scheme_Init();
}


/*
==========================
	HUD_Redraw

called every screen frame to
redraw the HUD.
===========================
*/

int DLLEXPORT HUD_Redraw( float time, int intermission )
{
//	RecClHudRedraw(time, intermission);
	
//LRCTEMP 1.8	if (CVAR_GET_FLOAT("r_glow") != 0)	 //check the cvar for the glow is on.//AJH Modified to include glow mode (1&2)
//LRCTEMP 1.8		RenderScreenGlow(); // glow effect --FragBait0

	gHUD.Redraw( time, intermission );

	return 1;
}


/*
==========================
	HUD_UpdateClientData

called every time shared client
dll/engine data gets changed,
and gives the cdll a chance
to modify the data.

returns 1 if anything has been changed, 0 otherwise.
==========================
*/

int DLLEXPORT HUD_UpdateClientData(client_data_t *pcldata, float flTime )
{
//	RecClHudUpdateClientData(pcldata, flTime);

	IN_Commands();

	return gHUD.UpdateClientData(pcldata, flTime );
}

/*
==========================
	HUD_Reset

Called at start and end of demos to restore to "non"HUD state.
==========================
*/

void DLLEXPORT HUD_Reset()
{
//	RecClHudReset();

	gHUD.VidInit();
}

/*
==========================
HUD_Frame

Called by engine every frame that client .dll is loaded
==========================
*/

void DLLEXPORT HUD_Frame( double time )
{
//	RecClHudFrame(time);

	GetClientVoiceMgr()->Frame(time);
}


/*
==========================
HUD_VoiceStatus

Called when a player starts or stops talking.
==========================
*/

void DLLEXPORT HUD_VoiceStatus(int entindex, qboolean bTalking)
{
////	RecClVoiceStatus(entindex, bTalking);

	GetClientVoiceMgr()->UpdateSpeakerStatus(entindex, bTalking);
}

/*
==========================
HUD_DirectorMessage

Called when a director event message was received
==========================
*/

void DLLEXPORT HUD_DirectorMessage( int iSize, void *pbuf )
{
//	RecClDirectorMessage(iSize, pbuf);

	gHUD.m_Spectator.DirectorMessage( iSize, pbuf );
}

void CL_UnloadParticleMan()
{
	Sys_UnloadModule( g_hParticleManModule );

	g_pParticleMan = NULL;
	g_hParticleManModule = NULL;
}

void CL_LoadParticleMan()
{
	char szPDir[512];

	if ( gEngfuncs.COM_ExpandFilename( PARTICLEMAN_DLLNAME, szPDir, sizeof( szPDir ) ) == FALSE )
	{
		g_pParticleMan = NULL;
		g_hParticleManModule = NULL;
		return;
	}

	g_hParticleManModule = Sys_LoadModule( szPDir );
	CreateInterfaceFn particleManFactory = Sys_GetFactory( g_hParticleManModule );

	if ( particleManFactory == NULL )
	{
		g_pParticleMan = NULL;
		g_hParticleManModule = NULL;
		return;
	}

	g_pParticleMan = (IParticleMan *)particleManFactory( PARTICLEMAN_INTERFACE, NULL);

	if ( g_pParticleMan )
	{
		 g_pParticleMan->SetUp( &gEngfuncs );

		 // Add custom particle classes here BEFORE calling anything else or you will die.
		 g_pParticleMan->AddCustomParticleClassSize ( sizeof ( CBaseParticle ) );
	}
}

cldll_func_dst_t *g_pcldstAddrs;

extern "C" void DLLEXPORT F(void *pv)
{
	cldll_func_t *pcldll_func = (cldll_func_t *)pv;

	// Hack!
	g_pcldstAddrs = ((cldll_func_dst_t *)pcldll_func->pHudVidInitFunc);

	cldll_func_t cldll_func = 
	{
	Initialize,
	HUD_Init,
	HUD_VidInit,
	HUD_Redraw,
	HUD_UpdateClientData,
	HUD_Reset,
	HUD_PlayerMove,
	HUD_PlayerMoveInit,
	HUD_PlayerMoveTexture,
	IN_ActivateMouse,
	IN_DeactivateMouse,
	IN_MouseEvent,
	IN_ClearStates,
	IN_Accumulate,
	CL_CreateMove,
	CL_IsThirdPerson,
	CL_CameraOffset,
	KB_Find,
	CAM_Think,
	V_CalcRefdef,
	HUD_AddEntity,
	HUD_CreateEntities,
	HUD_DrawNormalTriangles,
	HUD_DrawTransparentTriangles,
	HUD_StudioEvent,
	HUD_PostRunCmd,
	HUD_Shutdown,
	HUD_TxferLocalOverrides,
	HUD_ProcessPlayerState,
	HUD_TxferPredictionData,
	Demo_ReadBuffer,
	HUD_ConnectionlessPacket,
	HUD_GetHullBounds,
	HUD_Frame,
	HUD_Key_Event,
	HUD_TempEntUpdate,
	HUD_GetUserEntity,
	HUD_VoiceStatus,
	HUD_DirectorMessage,
	HUD_GetStudioModelInterface,
	HUD_ChatInputPosition,
	};

	*pcldll_func = cldll_func;
}

#include "cl_dll/IGameClientExports.h"

//-----------------------------------------------------------------------------
// Purpose: Exports functions that are used by the gameUI for UI dialogs
//-----------------------------------------------------------------------------
class CClientExports : public IGameClientExports
{
public:
	// returns the name of the server the user is connected to, if any
    const char *GetServerHostName() override
    {
		/*if (gViewPortInterface)
		{
			return gViewPortInterface->GetServerName();
		}*/
		return "";
	}

	// ingame voice manipulation
    bool IsPlayerGameVoiceMuted(int playerIndex) override
    {
		if (GetClientVoiceMgr())
			return GetClientVoiceMgr()->IsPlayerBlocked(playerIndex);
		return false;
	}

    void MutePlayerGameVoice(int playerIndex) override
    {
		if (GetClientVoiceMgr())
		{
			GetClientVoiceMgr()->SetPlayerBlockedState(playerIndex, true);
		}
	}

    void UnmutePlayerGameVoice(int playerIndex) override
    {
		if (GetClientVoiceMgr())
		{
			GetClientVoiceMgr()->SetPlayerBlockedState(playerIndex, false);
		}
	}
};

EXPOSE_SINGLE_INTERFACE(CClientExports, IGameClientExports, GAMECLIENTEXPORTS_INTERFACE_VERSION);
