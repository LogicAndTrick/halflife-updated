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

#include "light.h"
#include "extdll.h"
#include "util.h"

// LRC
int GetStdLightStyle(int iStyle)
{
	switch (iStyle)
	{
	// 0 normal
	case 0:
		return MAKE_STRING("m");

	// 1 FLICKER (first variety)
	case 1:
		return MAKE_STRING("mmnmmommommnonmmonqnmmo");

	// 2 SLOW STRONG PULSE
	case 2:
		return MAKE_STRING("abcdefghijklmnopqrstuvwxyzyxwvutsrqponmlkjihgfedcba");

	// 3 CANDLE (first variety)
	case 3:
		return MAKE_STRING("mmmmmaaaaammmmmaaaaaabcdefgabcdefg");

	// 4 FAST STROBE
	case 4:
		return MAKE_STRING("mamamamamama");

	// 5 GENTLE PULSE 1
	case 5:
		return MAKE_STRING("jklmnopqrstuvwxyzyxwvutsrqponmlkj");

	// 6 FLICKER (second variety)
	case 6:
		return MAKE_STRING("nmonqnmomnmomomno");

	// 7 CANDLE (second variety)
	case 7:
		return MAKE_STRING("mmmaaaabcdefgmmmmaaaammmaamm");

	// 8 CANDLE (third variety)
	case 8:
		return MAKE_STRING("mmmaaammmaaammmabcdefaaaammmmabcdefmmmaaaa");

	// 9 SLOW STROBE (fourth variety)
	case 9:
		return MAKE_STRING("aaaaaaaazzzzzzzz");

	// 10 FLUORESCENT FLICKER
	case 10:
		return MAKE_STRING("mmamammmmammamamaaamammma");

	// 11 SLOW PULSE NOT FADE TO BLACK
	case 11:
		return MAKE_STRING("abcdefghijklmnopqrrqponmlkjihgfedcba");

	// 12 UNDERWATER LIGHT MUTATION
	// this light only distorts the lightmap - no contribution
	// is made to the brightness of affected surfaces
	case 12:
		return MAKE_STRING("mmnnmmnnnmmnn");

	// 13 OFF (LRC)
	case 13:
		return MAKE_STRING("a");

	// 14 SLOW FADE IN (LRC)
	case 14:
		return MAKE_STRING("aabbccddeeffgghhiijjkkllmmmmmmmmmmmmmm");

	// 15 MED FADE IN (LRC)
	case 15:
		return MAKE_STRING("abcdefghijklmmmmmmmmmmmmmmmmmmmmmmmmmm");

	// 16 FAST FADE IN (LRC)
	case 16:
		return MAKE_STRING("acegikmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm");

	// 17 SLOW FADE OUT (LRC)
	case 17:
		return MAKE_STRING("llkkjjiihhggffeeddccbbaaaaaaaaaaaaaaaa");

	// 18 MED FADE OUT (LRC)
	case 18:
		return MAKE_STRING("lkjihgfedcbaaaaaaaaaaaaaaaaaaaaaaaaaaa");

	// 19 FAST FADE OUT (LRC)
	case 19:
		return MAKE_STRING("kigecaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");

	default:
		return MAKE_STRING("m");
	}
}
