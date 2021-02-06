//========= Copyright � 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

// Triangle rendering, if any

#include "hud.h"
#include "cl_util.h"

// Triangle rendering apis are in gEngfuncs.pTriAPI

#include "const.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "triangleapi.h"
#include "effects/particlemgr.h"
#include "com_model.h"
#include "studio_util.h"

#include "Exports.h"
#include "particleman.h"
#include "tri.h"
#include "effects/CWeather.h"

#include "glInclude.h"

extern IParticleMan *g_pParticleMan;

extern int g_iWaterLevel;
extern vec3_t v_origin;

int UseTexture(HSPRITE &hsprSpr, char * str)
{
	if (hsprSpr == 0)
	{
		char sz[256];
		sprintf( sz, str );
		hsprSpr = SPR_Load( sz );
	}

	return gEngfuncs.pTriAPI->SpriteTexture( (struct model_s *)gEngfuncs.GetSpritePointer( hsprSpr ), 0 );
}

//
//-----------------------------------------------------
//

void SetPoint( float x, float y, float z, float (*matrix)[4])
{
	vec3_t point, result;
	point[0] = x;
	point[1] = y;
	point[2] = z;

	VectorTransform(point, matrix, result);

	gEngfuncs.pTriAPI->Vertex3f(result[0], result[1], result[2]);
}

//LRC - code for CShinySurface, declared in hud.h
CShinySurface::CShinySurface( float fScale, float fAlpha, float fMinX, float fMaxX, float fMinY, float fMaxY, float fZ, char *szSprite)
{
	m_fScale = fScale; m_fAlpha = fAlpha;
	m_fMinX = fMinX; m_fMinY = fMinY;
	m_fMaxX = fMaxX; m_fMaxY = fMaxY;
	m_fZ = fZ;
	m_hsprSprite = 0;
	sprintf( m_szSprite, szSprite );
	m_pNext = NULL;
}

CShinySurface::~CShinySurface()
{
	if (m_pNext)
		delete m_pNext;
}

void CShinySurface::DrawAll(const vec3_t &org)
{
	gEngfuncs.pTriAPI->RenderMode( kRenderTransAdd ); //kRenderTransTexture );
	gEngfuncs.pTriAPI->CullFace( TRI_NONE );

	for(CShinySurface *pCurrent = this; pCurrent; pCurrent = pCurrent->m_pNext)
	{
		pCurrent->Draw(org);
	}

	gEngfuncs.pTriAPI->RenderMode( kRenderNormal );
}

void CShinySurface::Draw(const vec3_t &org)
{
	// add 5 to the view height, so that we don't get an ugly repeating texture as it approaches 0.
	float fHeight = org.z - m_fZ + 5;

	// only visible from above
//	if (fHeight < 0) return;
	if (fHeight < 5) return;

	// fade out if we're really close to the surface, so they don't see an ugly repeating texture
//	if (fHeight < 15)
//		gEngfuncs.pTriAPI->Color4f( 1.0, 1.0, 1.0, (fHeight - 5)*0.1*m_fAlpha );
//	else
		gEngfuncs.pTriAPI->Color4f( 1.0, 1.0, 1.0, m_fAlpha );

	// check whether the texture is valid
	if (!UseTexture(m_hsprSprite, m_szSprite)) return;

//	gEngfuncs.Con_Printf("minx %f, maxx %f, miny %f, maxy %f\n", m_fMinX, m_fMaxX, m_fMinY, m_fMaxY);

	float fFactor = 1/(m_fScale*fHeight);
	float fMinTX = (org.x - m_fMinX)*fFactor;
	float fMaxTX = (org.x - m_fMaxX)*fFactor;
	float fMinTY = (org.y - m_fMinY)*fFactor;
	float fMaxTY = (org.y - m_fMaxY)*fFactor;
//	gEngfuncs.pTriAPI->Color4f( 1.0, 1.0, 1.0, m_fAlpha );
	gEngfuncs.pTriAPI->Begin( TRI_QUADS );
		gEngfuncs.pTriAPI->TexCoord2f(	fMinTX,		fMinTY					);
		gEngfuncs.pTriAPI->Vertex3f  (	m_fMinX,	m_fMinY,	m_fZ+0.02	); // add 0.02 to avoid z-buffer problems
		gEngfuncs.pTriAPI->TexCoord2f(	fMinTX,		fMaxTY					);
		gEngfuncs.pTriAPI->Vertex3f  (	m_fMinX,	m_fMaxY,	m_fZ+0.02	);
		gEngfuncs.pTriAPI->TexCoord2f(	fMaxTX,		fMaxTY					);
		gEngfuncs.pTriAPI->Vertex3f  (	m_fMaxX,	m_fMaxY,	m_fZ+0.02	);
		gEngfuncs.pTriAPI->TexCoord2f(	fMaxTX,		fMinTY					);
		gEngfuncs.pTriAPI->Vertex3f  (	m_fMaxX,	m_fMinY,	m_fZ+0.02	);
	gEngfuncs.pTriAPI->End();
}

//
//-----------------------------------------------------
//

//LRCT

void BlackFog ( void )
{
	//Not in water and we want fog.
	static float fColorBlack[3] = {0,0,0};
	bool bFog = g_iWaterLevel < 2 && g_fog.startDist > 0 && g_fog.endDist > 0;
	if (bFog)
		gEngfuncs.pTriAPI->Fog ( fColorBlack, g_fog.startDist, g_fog.endDist, bFog );
	else
		gEngfuncs.pTriAPI->Fog ( g_fog.fogColor, g_fog.startDist, g_fog.endDist, bFog );
}

void RenderFog ( void )
{
	//Not in water and we want fog.
	bool bFog = g_iWaterLevel < 2 && g_fog.startDist > 0 && g_fog.endDist > 0;
	if (bFog)
		gEngfuncs.pTriAPI->Fog ( g_fog.fogColor, g_fog.startDist, g_fog.endDist, bFog );
//	else
//		gEngfuncs.pTriAPI->Fog ( g_fFogColor, 10000, 10001, 0 );
}

void ClearToFogColor( void )
{
	if ( g_fog.startDist > 0 && g_fog.endDist > 0 )
	{
		glClearColor( g_fog.fogColor[0], g_fog.fogColor[1], g_fog.fogColor[2], 1.0f );
		glClear( GL_COLOR_BUFFER_BIT );
	}
}

/*
=================
HUD_DrawNormalTriangles

Non-transparent triangles-- add them here
=================
*/
void DLLEXPORT HUD_DrawNormalTriangles( void )
{
//	RecClDrawNormalTriangles();

	gHUD.m_Spectator.DrawOverview();
}

#if defined( _TFC )
void RunEventList( void );
#endif

/*
=================
HUD_DrawTransparentTriangles

Render any triangles with transparent rendermode needs here
=================
*/
extern ParticleSystemManager* g_pParticleSystems; // LRC

void DLLEXPORT HUD_DrawTransparentTriangles( void )
{
//	RecClDrawTransparentTriangles();

#if defined( _TFC )
	RunEventList();
#endif

	if (g_pParticleMan)
	{
		g_pParticleMan->Update();
		g_Weather.Update();
	}
	
	BlackFog();

   	//22/03/03 LRC: shiny surfaces
	if (gHUD.m_pShinySurface)
		gHUD.m_pShinySurface->DrawAll(v_origin);

	// LRC: find out the time elapsed since the last redraw
	static float fOldTime, fTime;
	fOldTime = fTime;
	fTime = gEngfuncs.GetClientTime();

	// LRC: draw and update particle systems
	g_pParticleSystems->UpdateSystems(fTime - fOldTime);
}
