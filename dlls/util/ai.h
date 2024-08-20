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

bool FBoxVisible(entvars_t* pevLooker, entvars_t* pevTarget, Vector& vecTargetOrigin, float flSize = 0.0);
Vector VecCheckToss(entvars_t* pev, const Vector& vecSpot1, Vector vecSpot2, float flGravityAdj = 1.0);
Vector VecCheckThrow(entvars_t* pev, const Vector& vecSpot1, Vector vecSpot2, float flSpeed, float flGravityAdj = 1.0);
bool IsFacing(entvars_t* pevTest, const Vector& reference);
