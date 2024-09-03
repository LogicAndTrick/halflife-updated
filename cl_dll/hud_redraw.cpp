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
// hud_redraw.cpp
//
#include "hud.h"
#include "cl_util.h"

#include "vgui_TeamFortressViewport.h"

#define MAX_LOGO_FRAMES 56

int grgLogoFrame[MAX_LOGO_FRAMES] =
	{
		1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 13, 13, 13, 13, 13, 12, 11, 10, 9, 8, 14, 15,
		16, 17, 18, 19, 20, 20, 20, 20, 20, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
		29, 29, 29, 29, 29, 28, 27, 26, 25, 24, 30, 31};

inline float UTIL_Lerp(float lerpfactor, float A, float B) { return A + lerpfactor * (B - A); }

extern bool g_iVisibleMouse;

float HUD_GetFOV();

extern float IN_GetMouseSensitivity();

// Think
void CHud::Think()
{
	m_scrinfo.iSize = sizeof(m_scrinfo);
	GetScreenInfo(&m_scrinfo);

	int newfov;
	HUDLIST* pList = m_pHudList;

	while (pList)
	{
		if ((pList->p->m_iFlags & HUD_ACTIVE) != 0)
			pList->p->Think();
		pList = pList->pNext;
	}

	newfov = HUD_GetFOV();
	if (newfov == 0)
	{
		m_iFOV = default_fov->value;
	}
	else
	{
		m_iFOV = newfov;
	}

	// the clients fov is actually set in the client data update section of the hud

	// Set a new sensitivity
	if (m_iFOV == default_fov->value)
	{
		// reset to saved sensitivity
		m_flMouseSensitivity = 0;
	}
	else
	{
		// set a new sensitivity that is proportional to the change from the FOV default
		m_flMouseSensitivity = IN_GetMouseSensitivity() * ((float)newfov / (float)V_max(default_fov->value, 90.0f)) * CVAR_GET_FLOAT("zoom_sensitivity_ratio");
	}

	// think about default fov
	if (m_iFOV == 0)
	{ // only let players adjust up in fov,  and only if they are not overriden by something else
		m_iFOV = V_max(default_fov->value, 90);
	}

	if (0 != gEngfuncs.IsSpectateOnly())
	{
		m_iFOV = gHUD.m_Spectator.GetFOV(); // default_fov->value;
	}
}

// Redraw
// step through the local data,  placing the appropriate graphics & text as appropriate
// returns 1 if they've changed, 0 otherwise
bool CHud::Redraw(float flTime, bool intermission)
{
	m_fOldTime = m_flTime; // save time of previous redraw
	m_flTime = flTime;
	m_flTimeDelta = (double)m_flTime - m_fOldTime;
	static float m_flShotTime = 0;

	//LRC - handle fog fading effects. (is this the right place for it?)
	if (g_fFogFadeDuration)
	{
		// Nicer might be to use some kind of logarithmic fade-in?
		double fFraction = m_flTimeDelta / g_fFogFadeDuration;
		if (fFraction > 0)
		{
			g_fFogFadeFraction += fFraction;

			//		CONPRINT("FogFading: %f - %f, frac %f, time %f, final %d\n", g_fStartDist, g_fEndDist, fFraction, flTime, g_iFinalEndDist);

			if (g_fFogFadeFraction >= 1.0f)
			{
				// fading complete
				g_fFogFadeFraction = 1.0f;
				g_fFogFadeDuration = 0.0f;
			}

			// set the new fog values
			g_fog.endDist = UTIL_Lerp(g_fFogFadeFraction, g_fogPreFade.endDist, g_fogPostFade.endDist);
			g_fog.startDist = UTIL_Lerp(g_fFogFadeFraction, g_fogPreFade.startDist, g_fogPostFade.startDist);
			g_fog.fogColor[0] = UTIL_Lerp(g_fFogFadeFraction, g_fogPreFade.fogColor[0], g_fogPostFade.fogColor[0]);
			g_fog.fogColor[1] = UTIL_Lerp(g_fFogFadeFraction, g_fogPreFade.fogColor[1], g_fogPostFade.fogColor[1]);
			g_fog.fogColor[2] = UTIL_Lerp(g_fFogFadeFraction, g_fogPreFade.fogColor[2], g_fogPostFade.fogColor[2]);
		}
	}

	// Clock was reset, reset delta
	if (m_flTimeDelta < 0)
		m_flTimeDelta = 0;

	// Bring up the scoreboard during intermission
	if (gViewPort)
	{
		if (m_iIntermission && !intermission)
		{
			// Have to do this here so the scoreboard goes away
			m_iIntermission = intermission;
			gViewPort->HideCommandMenu();
			gViewPort->HideScoreBoard();
			gViewPort->UpdateSpectatorPanel();
		}
		else if (!m_iIntermission && intermission)
		{
			m_iIntermission = intermission;
			gViewPort->HideCommandMenu();
			gViewPort->HideVGUIMenu();
			gViewPort->ShowScoreBoard();
			gViewPort->UpdateSpectatorPanel();

			// Take a screenshot if the client's got the cvar set
			if (CVAR_GET_FLOAT("hud_takesshots") != 0)
				m_flShotTime = flTime + 1.0; // Take a screenshot in a second
		}
	}

	if (0 != m_flShotTime && m_flShotTime < flTime)
	{
		gEngfuncs.pfnClientCmd("snapshot\n");
		m_flShotTime = 0;
	}

	m_iIntermission = intermission;

	// if no redrawing is necessary
	// return 0;

	// trigger_viewset stuff
	if ((viewFlags & 1) && (viewFlags & 4)) //AJH Draw the camera hud
	{

		int r, g, b, x, y, a;
		//wrect_t rc;
		HSPRITE m_hCam1;
		int HUD_camera_active;
		int HUD_camera_rect;

		a = 225;

		UnpackRGB(r, g, b, gHUD.m_iHUDColor);
		ScaleColors(r, g, b, a);

		//Draw the flashing camera active logo
		HUD_camera_active = gHUD.GetSpriteIndex("camera_active");
		m_hCam1 = gHUD.GetSprite(HUD_camera_active);
		SPR_Set(m_hCam1, r, g, b);
		x = SPR_Width(m_hCam1, 0);
		x = ScreenWidth - x;
		y = SPR_Height(m_hCam1, 0) / 2;

		// Draw the camera sprite at 1 fps
		int i = (int)(flTime) % 2;
		i = grgLogoFrame[i] - 1;

		SPR_DrawAdditive(i, x, y, NULL);

		//Draw the camera reticle (top left)
		HUD_camera_rect = gHUD.GetSpriteIndex("camera_rect_tl");
		m_hCam1 = gHUD.GetSprite(HUD_camera_rect);
		SPR_Set(m_hCam1, r, g, b);
		x = ScreenWidth / 4;
		y = ScreenHeight / 4;

		SPR_DrawAdditive(0, x, y, &gHUD.GetSpriteRect(HUD_camera_rect));

		//Draw the camera reticle (top right)
		HUD_camera_rect = gHUD.GetSpriteIndex("camera_rect_tr");
		m_hCam1 = gHUD.GetSprite(HUD_camera_rect);
		SPR_Set(m_hCam1, r, g, b);

		int w, h;
		w = SPR_Width(m_hCam1, 0) / 2;
		h = SPR_Height(m_hCam1, 0) / 2;

		x = ScreenWidth - ScreenWidth / 4 - w;
		y = ScreenHeight / 4;

		SPR_DrawAdditive(0, x, y, &gHUD.GetSpriteRect(HUD_camera_rect));

		//Draw the camera reticle (bottom left)
		HUD_camera_rect = gHUD.GetSpriteIndex("camera_rect_bl");
		m_hCam1 = gHUD.GetSprite(HUD_camera_rect);
		SPR_Set(m_hCam1, r, g, b);
		x = ScreenWidth / 4;
		y = ScreenHeight - ScreenHeight / 4 - h;

		SPR_DrawAdditive(0, x, y, &gHUD.GetSpriteRect(HUD_camera_rect));

		//Draw the camera reticle (bottom right)
		HUD_camera_rect = gHUD.GetSpriteIndex("camera_rect_br");
		m_hCam1 = gHUD.GetSprite(HUD_camera_rect);
		SPR_Set(m_hCam1, r, g, b);
		x = ScreenWidth - ScreenWidth / 4 - w;
		y = ScreenHeight - ScreenHeight / 4 - h;

		SPR_DrawAdditive(0, x, y, &gHUD.GetSpriteRect(HUD_camera_rect));
	}

	if ((viewFlags & 1) && !(viewFlags & 2)) // custom view active, and flag "draw hud" isnt set
		return true;

	// draw all registered HUD elements
	if (0 != m_pCvarDraw->value)
	{
		HUDLIST* pList = m_pHudList;

		while (pList)
		{
			if (!intermission)
			{
				if ((pList->p->m_iFlags & HUD_ACTIVE) != 0 && (m_iHideHUDDisplay & HIDEHUD_ALL) == 0)
					pList->p->Draw(flTime);
			}
			else
			{ // it's an intermission,  so only draw hud elements that are set to draw during intermissions
				if ((pList->p->m_iFlags & HUD_INTERMISSION) != 0)
					pList->p->Draw(flTime);
			}

			pList = pList->pNext;
		}
	}

	// are we in demo mode? do we need to draw the logo in the top corner?
	if (0 != m_iLogo)
	{
		int x, y, i;

		if (m_hsprLogo == 0)
			m_hsprLogo = LoadSprite("sprites/%d_logo.spr");

		SPR_Set(m_hsprLogo, 250, 250, 250);

		x = SPR_Width(m_hsprLogo, 0);
		x = ScreenWidth - x;
		y = SPR_Height(m_hsprLogo, 0) / 2;

		// Draw the logo at 20 fps
		int iFrame = (int)(flTime * 20) % MAX_LOGO_FRAMES;
		i = grgLogoFrame[iFrame] - 1;

		SPR_DrawAdditive(i, x, y, NULL);
	}

	/*
	if ( g_iVisibleMouse )
	{
		void IN_GetMousePos( int *mx, int *my );
		int mx, my;

		IN_GetMousePos( &mx, &my );
		
		if (m_hsprCursor == 0)
		{
			char sz[256];
			sprintf( sz, "sprites/cursor.spr" );
			m_hsprCursor = SPR_Load( sz );
		}

		SPR_Set(m_hsprCursor, 250, 250, 250 );
		
		// Draw the logo at 20 fps
		SPR_DrawAdditive( 0, mx, my, NULL );
	}
	*/

	return true;
}

void ScaleColors(int& r, int& g, int& b, int a)
{
	float x = (float)a / 255;
	r = (int)(r * x);
	g = (int)(g * x);
	b = (int)(b * x);
}

int CHud::DrawHudString(int xpos, int ypos, int iMaxX, const char* szIt, int r, int g, int b)
{
	return xpos + gEngfuncs.pfnDrawString(xpos, ypos, szIt, r, g, b);
}

int CHud::DrawHudNumberString(int xpos, int ypos, int iMinX, int iNumber, int r, int g, int b)
{
	char szString[32];
	sprintf(szString, "%d", iNumber);
	return DrawHudStringReverse(xpos, ypos, iMinX, szString, r, g, b);
}

// draws a string from right to left (right-aligned)
int CHud::DrawHudStringReverse(int xpos, int ypos, int iMinX, const char* szString, int r, int g, int b)
{
	return xpos - gEngfuncs.pfnDrawStringReverse(xpos, ypos, szString, r, g, b);
}

int CHud::DrawHudNumber(int x, int y, int iFlags, int iNumber, int r, int g, int b)
{
	int iWidth = GetSpriteRect(m_HUD_number_0).right - GetSpriteRect(m_HUD_number_0).left;
	int k;

	if (iNumber > 0)
	{
		// SPR_Draw 100's
		if (iNumber >= 100)
		{
			k = iNumber / 100;
			SPR_Set(GetSprite(m_HUD_number_0 + k), r, g, b);
			SPR_DrawAdditive(0, x, y, &GetSpriteRect(m_HUD_number_0 + k));
			x += iWidth;
		}
		else if ((iFlags & DHN_3DIGITS) != 0)
		{
			//SPR_DrawAdditive( 0, x, y, &rc );
			x += iWidth;
		}

		// SPR_Draw 10's
		if (iNumber >= 10)
		{
			k = (iNumber % 100) / 10;
			SPR_Set(GetSprite(m_HUD_number_0 + k), r, g, b);
			SPR_DrawAdditive(0, x, y, &GetSpriteRect(m_HUD_number_0 + k));
			x += iWidth;
		}
		else if ((iFlags & (DHN_3DIGITS | DHN_2DIGITS)) != 0)
		{
			//SPR_DrawAdditive( 0, x, y, &rc );
			x += iWidth;
		}

		// SPR_Draw ones
		k = iNumber % 10;
		SPR_Set(GetSprite(m_HUD_number_0 + k), r, g, b);
		SPR_DrawAdditive(0, x, y, &GetSpriteRect(m_HUD_number_0 + k));
		x += iWidth;
	}
	else if ((iFlags & DHN_DRAWZERO) != 0)
	{
		SPR_Set(GetSprite(m_HUD_number_0), r, g, b);

		// SPR_Draw 100's
		if ((iFlags & DHN_3DIGITS) != 0)
		{
			//SPR_DrawAdditive( 0, x, y, &rc );
			x += iWidth;
		}

		if ((iFlags & (DHN_3DIGITS | DHN_2DIGITS)) != 0)
		{
			//SPR_DrawAdditive( 0, x, y, &rc );
			x += iWidth;
		}

		// SPR_Draw ones

		SPR_DrawAdditive(0, x, y, &GetSpriteRect(m_HUD_number_0));
		x += iWidth;
	}

	return x;
}


int CHud::GetNumWidth(int iNumber, int iFlags)
{
	if ((iFlags & DHN_3DIGITS) != 0)
		return 3;

	if ((iFlags & DHN_2DIGITS) != 0)
		return 2;

	if (iNumber <= 0)
	{
		if ((iFlags & DHN_DRAWZERO) != 0)
			return 1;
		else
			return 0;
	}

	if (iNumber < 10)
		return 1;

	if (iNumber < 100)
		return 2;

	return 3;
}

int CHud::GetHudNumberWidth(int number, int width, int flags)
{
	const int digitWidth = GetSpriteRect(m_HUD_number_0).right - GetSpriteRect(m_HUD_number_0).left;

	int totalDigits = 0;

	if (number > 0)
	{
		totalDigits = static_cast<int>(log10(number)) + 1;
	}
	else if ((flags & DHN_DRAWZERO) != 0)
	{
		totalDigits = 1;
	}

	totalDigits = V_max(totalDigits, width);

	return totalDigits * digitWidth;
}

int CHud::DrawHudNumberReverse(int x, int y, int number, int flags, int r, int g, int b)
{
	if (number > 0 || (flags & DHN_DRAWZERO) != 0)
	{
		const int digitWidth = GetSpriteRect(m_HUD_number_0).right - GetSpriteRect(m_HUD_number_0).left;

		int remainder = number;

		do
		{
			const int digit = remainder % 10;
			const int digitSpriteIndex = m_HUD_number_0 + digit;

			//This has to happen *before* drawing because we're drawing in reverse
			x -= digitWidth;

			SPR_Set(GetSprite(digitSpriteIndex), r, g, b);
			SPR_DrawAdditive(0, x, y, &GetSpriteRect(digitSpriteIndex));

			remainder /= 10;
		} while (remainder > 0);
	}

	return x;
}
