//========= Copyright � 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================
#pragma once

#include "hud/hud.h"
#include "hud/CHud.h"
#include "pm_defs.h"
#include "cl_util.h"
#include "extdll.h"
#include "util.h"
#include "weapons.h"
#include "event_api.h"
#include "r_studioint.h"
#include "util/sound.h"

extern engine_studio_api_t IEngineStudio;
extern cvar_t* cl_lw;
extern int tracerCount[32];

// Some of these are HL/TFC specific?
struct cl_entity_s* GetEntity(int idx);
struct cl_entity_s* GetViewEntity();

void EV_CreateTracer(float* start, float* end);
qboolean EV_IsPlayer(int idx);
qboolean EV_IsLocal(int idx);
void EV_GetGunPosition(event_args_t* args, float* pos, float* origin);
void EV_EjectBrass(float* origin, float* velocity, float rotation, int model, int soundtype);
void EV_GetDefaultShellInfo(event_args_t* args, float* origin, float* velocity, float* ShellVelocity, float* ShellOrigin, float* forward, float* right, 
                            float* up, float forwardScale, float upScale, float rightScale);
void EV_MuzzleFlash();

void EV_HLDM_GunshotDecalTrace(pmtrace_t* pTrace, char* decalName);
void EV_HLDM_DecalGunshot(pmtrace_t* pTrace, int iBulletType);
int EV_HLDM_CheckTracer(int idx, float* vecSrc, float* end, float* forward, float* right, int iBulletType, int iTracerFreq, int* tracerCount);
void EV_HLDM_FireBullets(int idx, float* forward, float* right, float* up, int cShots, float* vecSrc, float* vecDirShooting, float flDistance, int iBulletType, int iTracerFreq, int* tracerCount, float flSpreadX, float flSpreadY);

void V_PunchAxis(int axis, float punch);
void VectorAngles(const float* forward, float* angles);

extern "C" {
    // HLDM
    void EV_FireGlock1(event_args_t* args);
    void EV_FireGlock2(event_args_t* args);
    void EV_FireShotGunSingle(event_args_t* args);
    void EV_FireShotGunDouble(event_args_t* args);
    void EV_FireMP5(event_args_t* args);
    void EV_FireMP52(event_args_t* args);
    void EV_FirePython(event_args_t* args);
    void EV_FireGauss(event_args_t* args);
    void EV_SpinGauss(event_args_t* args);
    void EV_FireCrossbow(event_args_t* args);
    void EV_FireCrossbow2(event_args_t* args);
    void EV_FireRpg(event_args_t* args);
    void EV_EgonFire(event_args_t* args);
    void EV_EgonStop(event_args_t* args);
    void EV_HornetGunFire(event_args_t* args);
    void EV_TripmineFire(event_args_t* args);
    void EV_SnarkFire(event_args_t* args);

    void EV_TrainPitchAdjust(event_args_t* args);
}