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

#include "entities/CBaseEntity.h"

#define SF_BUTTON_DONTMOVE 1
#define SF_ROTBUTTON_NOTSOLID 1
#define SF_BUTTON_ONLYDIRECT 16	  // LRC - button can't be used through walls.
#define SF_BUTTON_TOGGLE 32		  // button stays pushed until reactivated
#define SF_BUTTON_SPARK_IF_OFF 64 // button sparks in OFF state
#define SF_BUTTON_NOT_SOLID 128	  // button isn't solid
#define SF_BUTTON_TOUCH_ONLY 256  // button must be touched to be used.
#define SF_BUTTON_USEKEY 512	  // change the reaction of the button to the USE key. \
								  // (i.e. if it's meant to be ignored, don't ignore it; otherwise ignore it.)

//===================================
// func_button (= CBaseButton)
//===================================
