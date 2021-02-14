/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
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

#include "hud/hud.h"
#include "cl_util.h"
#include "const.h"
#include "entity_state.h"
#include "event_api.h"
#include "cl_entity.h"
#include "triangleapi.h"
#include "ParticleSystem.h"
#include "ParticleType.h"

#include "RandomRange.h"
#include "..\common\com_model.h"
#include "..\common\pmtrace.h" // for contents and traceline
#include "..\pm_shared\pm_defs.h"


float ParticleSystem::c_fCosTable[360 + 90];
bool ParticleSystem::c_bCosTableInit = false;

ParticleSystem::ParticleSystem(int iEntIndex, char* szFilename)
{
    int iParticles = 100; // default

    m_iEntIndex = iEntIndex;
    m_pNextSystem = NULL;
    m_pFirstType = NULL;
    if (!c_bCosTableInit)
    {
        for (int i = 0; i < 360 + 90; i++)
        {
            c_fCosTable[i] = cos(i * M_PI / 180.0);
        }
        c_bCosTableInit = true;
    }

    char* szFile = (char*)gEngfuncs.COM_LoadFile(szFilename, 5, NULL);
    char szToken[1024];

    if (!szFile)
    {
        gEngfuncs.Con_Printf("Couldn't open particle file %s. Using default particle settings.\n", szFilename);
        return;
    }
    else
    {
        szFile = gEngfuncs.COM_ParseFile(szFile, szToken);

        while (szFile)
        {
            if (!stricmp(szToken, "particles"))
            {
                szFile = gEngfuncs.COM_ParseFile(szFile, szToken);
                iParticles = atof(szToken);
            }
            else if (!stricmp(szToken, "maintype"))
            {
                szFile = gEngfuncs.COM_ParseFile(szFile, szToken);
                m_pMainType = AddPlaceholderType(szToken);
                //strncpy(m_szMainType, szToken, sizeof(m_szMainType) );
            }
            else if (!stricmp(szToken, "{"))
            {
                // parse new type
                this->ParseType(szFile); // parses the type, moves the file pointer
            }

            szFile = gEngfuncs.COM_ParseFile(szFile, szToken);
        }
    }

    gEngfuncs.COM_FreeFile(szFile);

    AllocateParticles(iParticles);
}

void ParticleSystem::AllocateParticles(int iParticles)
{
    m_pAllParticles = new particle[iParticles];
    m_pFreeParticle = m_pAllParticles;
    m_pActiveParticle = NULL;
    m_pMainParticle = NULL;

    // initialise the linked list
    particle* pLast = m_pAllParticles;
    particle* pParticle = pLast + 1;

    for (int i = 1; i < iParticles; i++)
    {
        pLast->nextpart = pParticle;

        pLast = pParticle;
        pParticle++;
    }
    pLast->nextpart = NULL;
}

ParticleSystem::~ParticleSystem(void)
{
    delete[] m_pAllParticles;

    ParticleType* pType = m_pFirstType;
    ParticleType* pNext;
    while (pType)
    {
        pNext = pType->m_pNext;
        delete pType;
        pType = pNext;
    }
}

// returns the ParticleType with the given name, if there is one
ParticleType* ParticleSystem::GetType(const char* szName)
{
    for (ParticleType* pType = m_pFirstType; pType; pType = pType->m_pNext)
    {
        if (!stricmp(pType->m_szName, szName))
            return pType;
    }
    return NULL;
}

ParticleType* ParticleSystem::AddPlaceholderType(const char* szName)
{
    m_pFirstType = new ParticleType(m_pFirstType);
    strncpy(m_pFirstType->m_szName, szName, sizeof(m_pFirstType->m_szName));
    return m_pFirstType;
}

// creates a new particletype from the given file
// NB: this changes the value of szFile.
ParticleType* ParticleSystem::ParseType(char*& szFile)
{
    ParticleType* pType = new ParticleType();

    // parse the .aur file
    char szToken[1024];

    szFile = gEngfuncs.COM_ParseFile(szFile, szToken);
    while (stricmp(szToken, "}"))
    {
        if (!szFile)
            break;

        if (!stricmp(szToken, "name"))
        {
            szFile = gEngfuncs.COM_ParseFile(szFile, szToken);
            strncpy(pType->m_szName, szToken, sizeof(pType->m_szName));

            ParticleType* pTemp = GetType(szToken);
            if (pTemp)
            {
                // there's already a type with this name
                if (pTemp->m_bIsDefined)
                    gEngfuncs.Con_Printf("Warning: Particle type %s is defined more than once!\n", szToken);

                // copy all our data into the existing type, throw away the type we were making
                *pTemp = *pType;
                delete pType;
                pType = pTemp;
                pType->m_bIsDefined = true; // record the fact that it's defined, so we won't need to add it to the list
            }
        }
        else if (!stricmp(szToken, "gravity"))
        {
            szFile = gEngfuncs.COM_ParseFile(szFile, szToken);
            pType->m_Gravity = RandomRange(szToken);
        }
        else if (!stricmp(szToken, "windyaw"))
        {
            szFile = gEngfuncs.COM_ParseFile(szFile, szToken);
            pType->m_WindYaw = RandomRange(szToken);
        }
        else if (!stricmp(szToken, "windstrength"))
        {
            szFile = gEngfuncs.COM_ParseFile(szFile, szToken);
            pType->m_WindStrength = RandomRange(szToken);
        }
        else if (!stricmp(szToken, "sprite"))
        {
            szFile = gEngfuncs.COM_ParseFile(szFile, szToken);
            pType->m_hSprite = SPR_Load(szToken);
        }
        else if (!stricmp(szToken, "startalpha"))
        {
            szFile = gEngfuncs.COM_ParseFile(szFile, szToken);
            pType->m_StartAlpha = RandomRange(szToken);
        }
        else if (!stricmp(szToken, "endalpha"))
        {
            szFile = gEngfuncs.COM_ParseFile(szFile, szToken);
            pType->m_EndAlpha = RandomRange(szToken);
        }
        else if (!stricmp(szToken, "startred"))
        {
            szFile = gEngfuncs.COM_ParseFile(szFile, szToken);
            pType->m_StartRed = RandomRange(szToken);
        }
        else if (!stricmp(szToken, "endred"))
        {
            szFile = gEngfuncs.COM_ParseFile(szFile, szToken);
            pType->m_EndRed = RandomRange(szToken);
        }
        else if (!stricmp(szToken, "startgreen"))
        {
            szFile = gEngfuncs.COM_ParseFile(szFile, szToken);
            pType->m_StartGreen = RandomRange(szToken);
        }
        else if (!stricmp(szToken, "endgreen"))
        {
            szFile = gEngfuncs.COM_ParseFile(szFile, szToken);
            pType->m_EndGreen = RandomRange(szToken);
        }
        else if (!stricmp(szToken, "startblue"))
        {
            szFile = gEngfuncs.COM_ParseFile(szFile, szToken);
            pType->m_StartBlue = RandomRange(szToken);
        }
        else if (!stricmp(szToken, "endblue"))
        {
            szFile = gEngfuncs.COM_ParseFile(szFile, szToken);
            pType->m_EndBlue = RandomRange(szToken);
        }
        else if (!stricmp(szToken, "startsize"))
        {
            szFile = gEngfuncs.COM_ParseFile(szFile, szToken);
            pType->m_StartSize = RandomRange(szToken);
        }
        else if (!stricmp(szToken, "sizedelta"))
        {
            szFile = gEngfuncs.COM_ParseFile(szFile, szToken);
            pType->m_SizeDelta = RandomRange(szToken);
        }
        else if (!stricmp(szToken, "endsize"))
        {
            szFile = gEngfuncs.COM_ParseFile(szFile, szToken);
            pType->m_EndSize = RandomRange(szToken);
        }
        else if (!stricmp(szToken, "startangle"))
        {
            szFile = gEngfuncs.COM_ParseFile(szFile, szToken);
            pType->m_StartAngle = RandomRange(szToken);
        }
        else if (!stricmp(szToken, "angledelta"))
        {
            szFile = gEngfuncs.COM_ParseFile(szFile, szToken);
            pType->m_AngleDelta = RandomRange(szToken);
        }
        else if (!stricmp(szToken, "startframe"))
        {
            szFile = gEngfuncs.COM_ParseFile(szFile, szToken);
            pType->m_StartFrame = RandomRange(szToken);
        }
        else if (!stricmp(szToken, "endframe"))
        {
            szFile = gEngfuncs.COM_ParseFile(szFile, szToken);
            pType->m_EndFrame = RandomRange(szToken);
            pType->m_bEndFrame = true;
        }
        else if (!stricmp(szToken, "framerate"))
        {
            szFile = gEngfuncs.COM_ParseFile(szFile, szToken);
            pType->m_FrameRate = RandomRange(szToken);
        }
        else if (!stricmp(szToken, "lifetime"))
        {
            szFile = gEngfuncs.COM_ParseFile(szFile, szToken);
            pType->m_Life = RandomRange(szToken);
        }
        else if (!stricmp(szToken, "spraytype"))
        {
            szFile = gEngfuncs.COM_ParseFile(szFile, szToken);
            //gEngfuncs.Con_Printf("Read sprayname %s\n", szToken);
            ParticleType* pTemp = GetType(szToken);

            if (pTemp)
                pType->m_pSprayType = pTemp;
            else
                pType->m_pSprayType = AddPlaceholderType(szToken);
        }
        else if (!stricmp(szToken, "overlaytype"))
        {
            szFile = gEngfuncs.COM_ParseFile(szFile, szToken);
            ParticleType* pTemp = GetType(szToken);

            if (pTemp)
                pType->m_pOverlayType = pTemp;
            else
                pType->m_pOverlayType = AddPlaceholderType(szToken);
        }
        else if (!stricmp(szToken, "sprayrate"))
        {
            szFile = gEngfuncs.COM_ParseFile(szFile, szToken);
            pType->m_SprayRate = RandomRange(szToken);
        }
        else if (!stricmp(szToken, "sprayforce"))
        {
            szFile = gEngfuncs.COM_ParseFile(szFile, szToken);
            pType->m_SprayForce = RandomRange(szToken);
        }
        else if (!stricmp(szToken, "spraypitch"))
        {
            szFile = gEngfuncs.COM_ParseFile(szFile, szToken);
            pType->m_SprayPitch = RandomRange(szToken);
        }
        else if (!stricmp(szToken, "sprayyaw"))
        {
            szFile = gEngfuncs.COM_ParseFile(szFile, szToken);
            pType->m_SprayYaw = RandomRange(szToken);
        }
        else if (!stricmp(szToken, "drag"))
        {
            szFile = gEngfuncs.COM_ParseFile(szFile, szToken);
            pType->m_Drag = RandomRange(szToken);
        }
        else if (!stricmp(szToken, "bounce"))
        {
            szFile = gEngfuncs.COM_ParseFile(szFile, szToken);
            pType->m_Bounce = RandomRange(szToken);
            if (pType->m_Bounce.m_fMin != 0 || pType->m_Bounce.m_fMax != 0)
                pType->m_bBouncing = true;
        }
        else if (!stricmp(szToken, "bouncefriction"))
        {
            szFile = gEngfuncs.COM_ParseFile(szFile, szToken);
            pType->m_BounceFriction = RandomRange(szToken);
        }
        else if (!stricmp(szToken, "rendermode"))
        {
            szFile = gEngfuncs.COM_ParseFile(szFile, szToken);
            if (!stricmp(szToken, "additive"))
            {
                pType->m_iRenderMode = kRenderTransAdd;
            }
            else if (!stricmp(szToken, "solid"))
            {
                pType->m_iRenderMode = kRenderTransAlpha;
            }
            else if (!stricmp(szToken, "texture"))
            {
                pType->m_iRenderMode = kRenderTransTexture;
            }
            else if (!stricmp(szToken, "color"))
            {
                pType->m_iRenderMode = kRenderTransColor;
            }
        }
        else if (!stricmp(szToken, "drawcondition"))
        {
            szFile = gEngfuncs.COM_ParseFile(szFile, szToken);
            if (!stricmp(szToken, "empty"))
            {
                pType->m_iDrawCond = CONTENT_EMPTY;
            }
            else if (!stricmp(szToken, "water"))
            {
                pType->m_iDrawCond = CONTENT_WATER;
            }
            else if (!stricmp(szToken, "solid"))
            {
                pType->m_iDrawCond = CONTENT_SOLID;
            }
            else if (!stricmp(szToken, "special") || !stricmp(szToken, "special1"))
            {
                pType->m_iDrawCond = CONTENT_SPECIAL1;
            }
            else if (!stricmp(szToken, "special2"))
            {
                pType->m_iDrawCond = CONTENT_SPECIAL2;
            }
            else if (!stricmp(szToken, "special3"))
            {
                pType->m_iDrawCond = CONTENT_SPECIAL3;
            }
        }
        /*		else if ( !stricmp( szToken, "collision" ) )
                {
                    szFile = gEngfuncs.COM_ParseFile(szFile,szToken);
                    if ( !stricmp( szToken, "none" ) )
                    {
                        pType->m_iCollision = COLLISION_NONE;
                    }
                    else if ( !stricmp( szToken, "die" ) )
                    {
                        pType->m_iCollision = COLLISION_DIE;
                    }
                    else if ( !stricmp( szToken, "bounce" ) )
                    {
                        pType->m_iCollision = COLLISION_BOUNCE;
                    }
                }
        */
        // get the next token
        szFile = gEngfuncs.COM_ParseFile(szFile, szToken);
    }

    if (!pType->m_bIsDefined)
    {
        // if this is a newly-defined type, we need to add it to the list
        pType->m_pNext = m_pFirstType;
        m_pFirstType = pType;
        pType->m_bIsDefined = true;
    }

    return pType;
}

particle* ParticleSystem::ActivateParticle()
{
    particle* pActivated = m_pFreeParticle;
    if (pActivated)
    {
        m_pFreeParticle = pActivated->nextpart;
        pActivated->nextpart = m_pActiveParticle;
        m_pActiveParticle = pActivated;
    }
    return pActivated;
}

extern vec3_t v_origin;

void ParticleSystem::CalculateDistance()
{
    if (!m_pActiveParticle)
        return;

    vec3_t offset = v_origin - m_pActiveParticle->origin; // just pick one
    m_fViewerDist = offset[0] * offset[0] + offset[1] * offset[1] + offset[2] * offset[2];
}


bool ParticleSystem::UpdateSystem(float frametime, /*vec3_t &right, vec3_t &up,*/ int messagenum)
{
    // the entity emitting this system
    cl_entity_t* source = gEngfuncs.GetEntityByIndex(m_iEntIndex);

    // Don't update if the system is outside the player's PVS.
    if (!source || source->curstate.messagenum < messagenum)
        return true;

    if (m_pMainParticle == NULL)
    {
        //	gEngfuncs.Con_Printf("body %d\n", source->curstate.body);
        if (source->curstate.body)
        {
            ParticleType* pType = m_pMainType; //GetMainType();
            if (pType)
            {
                m_pMainParticle = pType->CreateParticle(this); //m_pMainParticle);
                //				m_pMainParticle = ActivateParticle();
                if (m_pMainParticle)
                {
                    m_pMainParticle->m_iEntIndex = m_iEntIndex;
                    //m_pMainParticle->origin = source->curstate.origin;
                    m_pMainParticle->age_death = -1; // never die
                    //pPart->origin[0] = x; pPart->origin[1] = y; pPart->origin[2] = z;
                }
            }
        }
    }
    else if (source && source->curstate.body == 0)
    {
        m_pMainParticle->age_death = 0; // die now
        m_pMainParticle = NULL;
    }

    particle* pParticle = m_pActiveParticle;
    particle* pLast = NULL;

    //	gEngfuncs.GetViewAngles((float*)normal);
    //	AngleVectors(normal,forward,right,up);

    while (pParticle)
    {
        //		if( TestParticle(part) )
        //		{
        if (UpdateParticle(pParticle, frametime))
        {
            // draw it, move onto the next one
            //DrawParticle( pParticle, right, up );

            pLast = pParticle;
            pParticle = pParticle->nextpart;
        }
        else // deactivate it
        {
            if (pLast)
            {
                pLast->nextpart = pParticle->nextpart;
                pParticle->nextpart = m_pFreeParticle;
                m_pFreeParticle = pParticle;
                pParticle = pLast->nextpart;
            }
            else // deactivate the first particle in the list
            {
                m_pActiveParticle = pParticle->nextpart;
                pParticle->nextpart = m_pFreeParticle;
                m_pFreeParticle = pParticle;
                pParticle = m_pActiveParticle;
            }
        }
    }


    return true;
}

void ParticleSystem::DrawSystem() //vec3_t &right, vec3_t &up)
{
    vec3_t normal, forward, right, up;

    gEngfuncs.GetViewAngles((float*)normal);
    AngleVectors(normal, forward, right, up);

    particle* pParticle = m_pActiveParticle;
    for (pParticle = m_pActiveParticle; pParticle; pParticle = pParticle->nextpart)
    {
        DrawParticle(pParticle, right, up);
    }
}

/*bool ParticleSystem::ParticleIsVisible( particle* part )
{
	return true;

	vec3_t normal, forward, right, up;
	gEngfuncs.GetViewAngles((float*)normal);
	AngleVectors( normal, forward, right, up );

	Vector vec = ( part->origin - (gEngfuncs.GetLocalPlayer())->origin );
	Vector vecDir = vec.Normalize( );
	float distance = vec.Length();
	
	

	if ( DotProduct ( vecDir, forward ) < 0 )
		return false;
//  
//  float dot = fabs( DotProduct ( vecDir, right ) ) + fabs( DotProduct ( vecDir, up ) ) * 0.5;
//  // tweak for distance
//  dot *= 1.0 + 0.2 * ( distance / 8192 );
//  
//  // try to use a smaller arc when zooming (smooth sniping)
//  float arc = .75;
//  
//  if ( dot > arc )
//  	return false;
//  
	return true;
}*/

bool ParticleSystem::UpdateParticle(particle* part, float frametime)
{
    //	gEngfuncs.Con_Printf("UpdParticle %f: age %f, life %f\n", frametime, part->age, part->age_death);
    if (frametime == 0)
        return true;

    part->age += frametime;

    cl_entity_t* source = gEngfuncs.GetEntityByIndex(m_iEntIndex); //AJH moved here

    // is this particle bound to an entity?
    if (part->m_iEntIndex)
    {
        //cl_entity_t *source = gEngfuncs.GetEntityByIndex( m_iEntIndex ); //AJH
        if (source && source->curstate.body)
        {
            part->velocity = (source->curstate.origin - part->origin) / frametime;
            part->origin = source->curstate.origin;
            //			part->velocity = source->curstate.velocity;
            //			gEngfuncs.Con_Printf("using velocity %f %f %f\n", part->velocity.x, part->velocity.y, part->velocity.z);
        }
        else
        {
            // entity is switched off, die
            return false;
        }
    }
    else
    {
        // not tied to an entity, check whether it's time to die
        if (part->age_death >= 0 && part->age > part->age_death)
            return false;

        // apply acceleration and velocity
        vec3_t vecOldPos = part->origin;
        if (part->m_fDrag)
            VectorMA(part->velocity, -part->m_fDrag * frametime, part->velocity - part->m_vecWind, part->velocity);
        VectorMA(part->velocity, frametime, part->accel, part->velocity);
        VectorMA(part->origin, frametime, part->velocity, part->origin);

        if (part->pType->m_bBouncing)
        {
            vec3_t vecTarget;
            VectorMA(part->origin, frametime, part->velocity, vecTarget);
            pmtrace_t* tr = gEngfuncs.PM_TraceLine(part->origin, vecTarget, PM_TRACELINE_PHYSENTSONLY, 2 /*point hull*/, -1);
            if (tr->fraction < 1)
            {
                part->origin = tr->endpos;
                float bounceforce = Vector::DotProduct(tr->plane.normal, part->velocity);
                float newspeed = (1 - part->pType->m_BounceFriction.GetInstance());
                part->velocity = part->velocity * newspeed;
                VectorMA(part->velocity, -bounceforce * (newspeed + part->pType->m_Bounce.GetInstance()), tr->plane.normal, part->velocity);
            }
        }
    }


    // spray children
    if (part->age_spray && part->age > part->age_spray)
    {
        part->age_spray = part->age + 1 / part->pType->m_SprayRate.GetInstance();

        //particle *pChild = ActivateParticle();
        if (part->pType->m_pSprayType)
        {
            particle* pChild = part->pType->m_pSprayType->CreateParticle(this); //pChild);
            if (pChild)
            {
                pChild->origin = part->origin;
                float fSprayForce = part->pType->m_SprayForce.GetInstance();
                pChild->velocity = part->velocity;
                if (fSprayForce)
                {
                    float fSprayPitch = part->pType->m_SprayPitch.GetInstance()/*;*/ - source->curstate.angles.x; //AJH For rotating paticles.
                    float fSprayYaw = part->pType->m_SprayYaw.GetInstance()/*;*/ - source->curstate.angles.y; //AJH
                    float fForceCosPitch = fSprayForce * CosLookup(fSprayPitch); //- source->curstate.angles.z;		//AJH
                    //	vec3_t vecSprayVel;
                    pChild->velocity.x += CosLookup(fSprayYaw) * fForceCosPitch;
                    pChild->velocity.y += SinLookup(fSprayYaw) * fForceCosPitch;
                    pChild->velocity.z -= SinLookup(fSprayPitch) * fSprayForce;
                }
            }
        }
    }

    part->m_fSize += part->m_fSizeStep * frametime;
    part->m_fAlpha += part->m_fAlphaStep * frametime;
    part->m_fRed += part->m_fRedStep * frametime;
    part->m_fGreen += part->m_fGreenStep * frametime;
    part->m_fBlue += part->m_fBlueStep * frametime;
    part->frame += part->m_fFrameStep * frametime;
    if (part->m_fAngleStep)
    {
        part->m_fAngle += part->m_fAngleStep * frametime;
        while (part->m_fAngle < 0) part->m_fAngle += 360;
        while (part->m_fAngle > 360) part->m_fAngle -= 360;

        //		gEngfuncs.Con_Printf("Rotating to %f, width %f\n", part->m_fAngle, part->m_fWidth);
    }
    return true;
}

void ParticleSystem::DrawParticle(particle* part, vec3_t& right, vec3_t& up)
{
    //	gEngfuncs.Con_Printf("DrawParticle: size %f, pos %f %f %f\n", part->size, part->origin[0], part->origin[1], part->origin[2]);
    float fSize = part->m_fSize;
    vec3_t point1, point2, point3, point4;
    vec3_t origin = part->origin;

    // nothing to draw?
    if (fSize == 0)
        return;

    float fCosSize = CosLookup(part->m_fAngle) * fSize;
    float fSinSize = SinLookup(part->m_fAngle) * fSize;

    // calculate the four corners of the sprite
    VectorMA(origin, fSinSize, up, point1);
    VectorMA(point1, -fCosSize, right, point1);

    VectorMA(origin, fCosSize, up, point2);
    VectorMA(point2, fSinSize, right, point2);

    VectorMA(origin, -fSinSize, up, point3);
    VectorMA(point3, fCosSize, right, point3);

    VectorMA(origin, -fCosSize, up, point4);
    VectorMA(point4, -fSinSize, right, point4);

    struct model_s* pModel;
    int iContents = 0;

    for (particle* pDraw = part; pDraw; pDraw = pDraw->m_pOverlay)
    {
        if (pDraw->pType->m_hSprite == 0)
            continue;

        if (pDraw->pType->m_iDrawCond)
        {
            if (iContents == 0)
                iContents = gEngfuncs.PM_PointContents(origin, NULL);

            if (iContents != pDraw->pType->m_iDrawCond)
                continue;
        }

        pModel = (struct model_s*)gEngfuncs.GetSpritePointer(pDraw->pType->m_hSprite);

        // if we've reached the end of the sprite's frames, loop back
        while (pDraw->frame > pModel->numframes)
            pDraw->frame -= pModel->numframes;

        while (pDraw->frame < 0)
            pDraw->frame += pModel->numframes;

        if (!gEngfuncs.pTriAPI->SpriteTexture(pModel, int(pDraw->frame)))
            continue;

        gEngfuncs.pTriAPI->RenderMode(pDraw->pType->m_iRenderMode);
        gEngfuncs.pTriAPI->Color4f(pDraw->m_fRed, pDraw->m_fGreen, pDraw->m_fBlue, pDraw->m_fAlpha);
        gEngfuncs.pTriAPI->Begin(TRI_QUADS);
        gEngfuncs.pTriAPI->TexCoord2f(0, 0);
        gEngfuncs.pTriAPI->Vertex3fv(point1);

        gEngfuncs.pTriAPI->TexCoord2f(1, 0);
        gEngfuncs.pTriAPI->Vertex3fv(point2);

        gEngfuncs.pTriAPI->TexCoord2f(1, 1);
        gEngfuncs.pTriAPI->Vertex3fv(point3);

        gEngfuncs.pTriAPI->TexCoord2f(0, 1);
        gEngfuncs.pTriAPI->Vertex3fv(point4);
        gEngfuncs.pTriAPI->End();
    }
}
