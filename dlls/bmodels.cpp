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

//
// BModelOrigin - calculates origin of a bmodel from absmin/size because all bmodel origins are 0 0 0
//
Vector VecBModelOrigin(entvars_t* pevBModel)
{
	return (pevBModel->absmin + pevBModel->absmax) * 0.5; // LRC - bug fix for rotating ents
														  //	return pevBModel->absmin + ( pevBModel->size * 0.5 );
}
