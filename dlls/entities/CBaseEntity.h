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
#include "util.h"
#include "cbase.h"
#include "util/sound.h"
#include "util/subs.h"
#include "util/mathutil.h"
#include "util/random.h"
#include "classes/saverestore/CSave.h"
#include "classes/saverestore/CRestore.h"

extern Vector VecBModelOrigin(entvars_t* pevBModel);

// This is the glue that hooks .MAP entity class names to our CPP classes
// The _declspec forces them to be exported by name so we can do a lookup with GetProcAddress()
// The function is used to intialize / allocate the object for the entity
#define LINK_ENTITY_TO_CLASS(mapClassName, DLLClassName)    \
	extern "C" DLLEXPORT void mapClassName(entvars_t* pev); \
	void mapClassName(entvars_t* pev) { GetClassPtr((DLLClassName*)pev); }
