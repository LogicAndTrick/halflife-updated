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

#include "CBaseToggle.h"
#include "func/CBaseDoor.h"
#include "movewith.h"

#define ACCELTIMEINCREMENT 0.1 // AJH for acceleration/deceleration time steps

// Global Savedata for Toggle
TYPEDESCRIPTION CBaseToggle::m_SaveData[] =
	{
		DEFINE_FIELD(CBaseToggle, m_toggle_state, FIELD_INTEGER),
		DEFINE_FIELD(CBaseToggle, m_flActivateFinished, FIELD_TIME),
		DEFINE_FIELD(CBaseToggle, m_flMoveDistance, FIELD_FLOAT),
		DEFINE_FIELD(CBaseToggle, m_flWait, FIELD_FLOAT),
		DEFINE_FIELD(CBaseToggle, m_flLip, FIELD_FLOAT),
		DEFINE_FIELD(CBaseToggle, m_flTWidth, FIELD_FLOAT),
		DEFINE_FIELD(CBaseToggle, m_flTLength, FIELD_FLOAT),
		DEFINE_FIELD(CBaseToggle, m_vecPosition1, FIELD_POSITION_VECTOR),
		DEFINE_FIELD(CBaseToggle, m_vecPosition2, FIELD_POSITION_VECTOR),
		DEFINE_FIELD(CBaseToggle, m_vecAngle1, FIELD_VECTOR), // UNDONE: Position could go through transition, but also angle?
		DEFINE_FIELD(CBaseToggle, m_vecAngle2, FIELD_VECTOR), // UNDONE: Position could go through transition, but also angle?
		DEFINE_FIELD(CBaseToggle, m_cTriggersLeft, FIELD_INTEGER),
		DEFINE_FIELD(CBaseToggle, m_flHeight, FIELD_FLOAT),
		DEFINE_FIELD(CBaseToggle, m_pfnCallWhenMoveDone, FIELD_FUNCTION),
		DEFINE_FIELD(CBaseToggle, m_vecFinalDest, FIELD_POSITION_VECTOR),
		DEFINE_FIELD(CBaseToggle, m_flLinearMoveSpeed, FIELD_FLOAT),
		DEFINE_FIELD(CBaseToggle, m_flAngularMoveSpeed, FIELD_FLOAT), // LRC
		DEFINE_FIELD(CBaseToggle, m_vecFinalAngle, FIELD_VECTOR),
		DEFINE_FIELD(CBaseToggle, m_sMaster, FIELD_STRING),
		DEFINE_FIELD(CBaseToggle, m_bitsDamageInflict, FIELD_INTEGER), // damage type inflicted

		DEFINE_FIELD(CBaseToggle, m_flLinearAccel, FIELD_FLOAT), //
		DEFINE_FIELD(CBaseToggle, m_flLinearDecel, FIELD_FLOAT), //
		DEFINE_FIELD(CBaseToggle, m_flAccelTime, FIELD_FLOAT),	 // AJH Needed for acceleration/deceleration
		DEFINE_FIELD(CBaseToggle, m_flDecelTime, FIELD_FLOAT),	 //
		DEFINE_FIELD(CBaseToggle, m_flCurrentTime, FIELD_FLOAT), //
		DEFINE_FIELD(CBaseToggle, m_bDecelerate, FIELD_BOOLEAN), //

};
IMPLEMENT_SAVERESTORE(CBaseToggle, CBaseAnimating);


bool CBaseToggle::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "lip"))
	{
		m_flLip = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "wait"))
	{
		m_flWait = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "master"))
	{
		m_sMaster = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "distance"))
	{
		m_flMoveDistance = atof(pkvd->szValue);
		return true;
	}

	return CBaseDelay::KeyValue(pkvd);
}

// void CBaseToggle::LinearMove( Vector	vecInput, float flSpeed)
//{
//	LinearMove(vecInput, flSpeed);
// }

/*
=============
LinearMove

calculate pev->velocity and pev->nextthink to reach vecDest from
pev->origin traveling at flSpeed
===============
*/
void CBaseToggle::LinearMove(Vector vecInput, float flSpeed) //, bool bNow )
{
	//	ALERT(at_console, "LMove %s: %f %f %f, speed %f\n", STRING(pev->targetname), vecInput.x, vecInput.y, vecInput.z, flSpeed);
	ASSERTSZ(flSpeed != 0, "LinearMove:  no speed is defined!");
	//	ASSERTSZ(m_pfnCallWhenMoveDone != NULL, "LinearMove: no post-move function defined");

	m_flLinearMoveSpeed = flSpeed;
	m_vecFinalDest = vecInput;
	m_flLinearAccel = -1.0; // AJH Not using Acceleration
	m_flLinearDecel = -1.0; //

	//	if ((m_pMoveWith || m_pChildMoveWith))// && !bNow)
	//	{
	//		ALERT(at_console,"Setting LinearMoveNow to happen after %f\n",gpGlobals->time);
	SetThink(&CBaseToggle::LinearMoveNow);
	UTIL_DesiredThink(this);
	// pev->nextthink = pev->ltime + 0.01;
	//	}
	//	else
	//	{
	//		LinearMoveNow(); // starring Martin Sheen and Marlon Brando
	//	}
}

void CBaseToggle ::LinearMove(Vector vecInput, float flSpeed, float flAccel, float flDecel) //, BOOL bNow )  // AJH Call this to use acceleration
{
	//	ALERT(at_console, "LMove %s: %f %f %f, speed %f, accel %f \n", STRING(pev->targetname), vecInput.x, vecInput.y, vecInput.z, flSpeed, flAccel);
	ASSERTSZ(flSpeed != 0, "LinearMove:  no speed is defined!");
	//	ASSERTSZ(m_pfnCallWhenMoveDone != NULL, "LinearMove: no post-move function defined");

	m_flLinearMoveSpeed = flSpeed;

	m_flLinearAccel = flAccel;
	m_flLinearDecel = flDecel;
	m_flCurrentTime = 0;

	if (m_flLinearAccel > 0)
	{
		m_flAccelTime = m_flLinearMoveSpeed / m_flLinearAccel;
	}
	else
	{
		m_flLinearAccel = -1;
		m_flAccelTime = 0;
	}
	if (m_flLinearDecel > 0)
	{
		m_flDecelTime = m_flLinearMoveSpeed / m_flLinearDecel;
	}
	else
	{
		m_flLinearDecel = -1;
		m_flDecelTime = 0;
	}

	m_vecFinalDest = vecInput;

	//	if ((m_pMoveWith || m_pChildMoveWith))// && !bNow)
	//	{
	//		ALERT(at_console,"Setting LinearMoveNow to happen after %f\n",gpGlobals->time);
	SetThink(&CBaseToggle::LinearMoveNow);
	UTIL_DesiredThink(this);
	// pev->nextthink = pev->ltime + 0.01;
	//	}
	//	else
	//	{
	//		LinearMoveNow(); // starring Martin Sheen and Marlon Brando
	//	}
}

void CBaseToggle ::LinearMoveNow() // AJH Now supports acceleration
{
	//	ALERT(at_console, "LMNow %s\n", STRING(pev->targetname));

	Vector vecDest;
	//	if (m_pMoveWith || m_pChildMoveWith )
	//		ALERT(at_console,"THINK: LinearMoveNow happens at %f, speed %f\n",gpGlobals->time, m_flLinearMoveSpeed);

	if (m_pMoveWith)
	{
		vecDest = m_vecFinalDest + m_pMoveWith->pev->origin;
	}
	else
		vecDest = m_vecFinalDest;

	// Already there?
	if (vecDest == pev->origin)
	{
		ALERT(at_console, "%s Already There!!\n", STRING(pev->targetname));
		LinearMoveDone();
		return;
	}

	// set destdelta to the vector needed to move
	Vector vecDestDelta = vecDest - pev->origin;

	// divide vector length by speed to get time to reach dest
	float flTravelTime = vecDestDelta.Length() / m_flLinearMoveSpeed;
	if (m_flLinearAccel > 0 || m_flLinearDecel > 0)
	{ // AJH Are we using acceleration/deceleration?

		if (m_bDecelerate == true)
		{ // Are we slowing down?
			m_flCurrentTime -= ACCELTIMEINCREMENT;
			if (m_flCurrentTime <= 0)
			{
				//	ALERT(at_debug, "%s has finished moving\n", STRING(pev->targetname));
				LinearMoveDone(); // Finished slowing.

				m_flCurrentTime = 0;   //
				m_bDecelerate = false; // reset
			}
			else
			{


				UTIL_SetVelocity(this, vecDestDelta.Normalize() * (m_flLinearDecel * m_flCurrentTime)); // Slow down
				//	ALERT(at_debug, "%s is decelerating, time: %f speed: %f vector: %f %f %f\n", STRING(pev->targetname),m_flCurrentTime,(m_flLinearDecel*m_flCurrentTime),vecDestDelta.Normalize().x,vecDestDelta.Normalize().y,vecDestDelta.Normalize().z);

				// Continually calls LinearMoveNow every ACCELTIMEINCREMENT (seconds?) till stopped
				if (m_flCurrentTime < ACCELTIMEINCREMENT)
				{
					SetNextThink(m_flCurrentTime, true);
					m_flCurrentTime = 0;
				}
				else
				{
					SetNextThink(ACCELTIMEINCREMENT, true);
				}
				SetThink(&CBaseToggle ::LinearMoveNow);
			}
		}
		else
		{

			if (m_flCurrentTime < m_flAccelTime)
			{ // We are Accelerating.

				//	ALERT(at_console, "%s is accelerating\n", STRING(pev->targetname));
				UTIL_SetVelocity(this, vecDestDelta.Normalize() * (m_flLinearAccel * m_flCurrentTime));

				// Continually calls LinearMoveNow every 0.1 (seconds?) till up to speed
				SetNextThink(ACCELTIMEINCREMENT, true);
				SetThink(&CBaseToggle ::LinearMoveNow);
				m_flCurrentTime += ACCELTIMEINCREMENT;

				// BUGBUG this will mean that we will be going faster than maxspeed on the last call to 'accelerate'
			}
			else
			{ // We are now at full speed.

				//	m_flAccelTime = m_flCurrentTime;
				//	ALERT(at_console, "%s is traveling at constant speed\n", STRING(pev->targetname));

				UTIL_SetVelocity(this, vecDestDelta.Normalize() * (m_flLinearMoveSpeed)); // we are probably going slightly faster.

				// set nextthink to trigger a recall to LinearMoveNow when we need to slow down.
				SetNextThink((vecDestDelta.Length() - (m_flLinearMoveSpeed * m_flDecelTime / 2)) / (m_flLinearMoveSpeed), true);
				SetThink(&CBaseToggle ::LinearMoveNow);
				//	m_flCurrentTime = (flTravelTime);
				m_bDecelerate = true;								// Set boolean so next call we know we are decelerating.
				m_flDecelTime += (m_flCurrentTime - m_flAccelTime); // Hack to fix time increment bug
				m_flCurrentTime = m_flDecelTime;
			}
		}
	}
	else
	{ // We are not using acceleration.

		// set nextthink to trigger a call to LinearMoveDone when dest is reached
		SetNextThink(flTravelTime, true);
		SetThink(&CBaseToggle::LinearMoveDone);

		// scale the destdelta vector by the time spent traveling to get velocity
		//	pev->velocity = vecDestDelta / flTravelTime;
		UTIL_SetVelocity(this, vecDestDelta / flTravelTime);

		//	ALERT(at_console, "LMNow \"%s\": Vel %f %f %f, think %f\n", STRING(pev->targetname), pev->velocity.x, pev->velocity.y, pev->velocity.z, pev->nextthink);
	}
}

/*
============
After moving, set origin to exact final destination, call "move done" function
============
*/
/*void CBaseToggle::LinearMoveDone()
{
	Vector vecDiff;
	if (m_pMoveWith)
		vecDiff = (m_vecFinalDest + m_pMoveWith->pev->origin) - pev->origin;
	else
		vecDiff = m_vecFinalDest - pev->origin;
	if (vecDiff.Length() > 0.05) //pev->velocity.Length())
	{
		// HACK: not there yet, try waiting one more frame.
		ALERT(at_console,"Rejecting difference %f\n",vecDiff.Length());
		SetThink(LinearMoveFinalDone);
		pev->nextthink = gpGlobals->time + 0.01;
	}
	else
	{
		LinearMoveFinalDone();
	}
}*/

void CBaseToggle::LinearMoveDone()
{
	SetThink(&CBaseToggle::LinearMoveDoneNow);
	//	ALERT(at_console, "LMD: desiredThink %s\n", STRING(pev->targetname));
	UTIL_DesiredThink(this);
}

void CBaseToggle::LinearMoveDoneNow()
{
	Vector vecDest;

	//	ALERT(at_console, "LMDone %s\n", STRING(pev->targetname));

	UTIL_SetVelocity(this, g_vecZero); //, true);
									   //	pev->velocity = g_vecZero;
	if (m_pMoveWith)
	{
		vecDest = m_vecFinalDest + m_pMoveWith->pev->origin;
		//		ALERT(at_console, "LMDone %s: p.origin = %f %f %f, origin = %f %f %f. Set it to %f %f %f\n", STRING(pev->targetname), m_pMoveWith->pev->origin.x,  m_pMoveWith->pev->origin.y,  m_pMoveWith->pev->origin.z, pev->origin.x, pev->origin.y, pev->origin.z, vecDest.x, vecDest.y, vecDest.z);
	}
	else
	{
		vecDest = m_vecFinalDest;
		//		ALERT(at_console, "LMDone %s: origin = %f %f %f. Set it to %f %f %f\n", STRING(pev->targetname), pev->origin.x, pev->origin.y, pev->origin.z, vecDest.x, vecDest.y, vecDest.z);
	}
	UTIL_AssignOrigin(this, vecDest);
	DontThink(); // LRC
	// pev->nextthink = -1;
	if (m_pfnCallWhenMoveDone)
		(this->*m_pfnCallWhenMoveDone)();
}

bool CBaseToggle::IsLockedByMaster()
{
	return !FStringNull(m_sMaster) && !UTIL_IsMasterTriggered(m_sMaster, m_hActivator);
}

// LRC- mapping toggle-states to global states
STATE CBaseToggle::GetState()
{
	switch (m_toggle_state)
	{
	case TS_AT_TOP:
		return STATE_ON;
	case TS_AT_BOTTOM:
		return STATE_OFF;
	case TS_GOING_UP:
		return STATE_TURN_ON;
	case TS_GOING_DOWN:
		return STATE_TURN_OFF;
	default:
		return STATE_OFF; // This should never happen.
	}
};

/*
=============
AngularMove

calculate pev->velocity and pev->nextthink to reach vecDest from
pev->origin traveling at flSpeed
Just like LinearMove, but rotational.
===============
*/
void CBaseToggle::AngularMove(Vector vecDestAngle, float flSpeed)
{
	ASSERTSZ(flSpeed != 0, "AngularMove:  no speed is defined!");
	//	ASSERTSZ(m_pfnCallWhenMoveDone != NULL, "AngularMove: no post-move function defined");

	m_vecFinalAngle = vecDestAngle;
	m_flAngularMoveSpeed = flSpeed;

	//	if ((m_pMoveWith || m_pChildMoveWith))// && !bNow)
	//	{
	//		ALERT(at_console,"Setting AngularMoveNow to happen after %f\n",gpGlobals->time);
	SetThink(&CBaseToggle::AngularMoveNow);
	UTIL_DesiredThink(this);
	//	ExternalThink( 0.01 );
	//		pev->nextthink = pev->ltime + 0.01;
	//	}
	//	else
	//	{
	//		AngularMoveNow(); // starring Martin Sheen and Marlon Brando
	//	}
}

void CBaseToggle::AngularMoveNow()
{
	//	ALERT(at_console, "AngularMoveNow %f\n", pev->ltime);
	Vector vecDestAngle;

	if (m_pMoveWith)
		vecDestAngle = m_vecFinalAngle + m_pMoveWith->pev->angles;
	else
		vecDestAngle = m_vecFinalAngle;

	// Already there?
	if (vecDestAngle == pev->angles)
	{
		AngularMoveDone();
		return;
	}

	// set destdelta to the vector needed to move
	Vector vecDestDelta = vecDestAngle - pev->angles;

	// divide by speed to get time to reach dest
	float flTravelTime = vecDestDelta.Length() / m_flAngularMoveSpeed;

	// set nextthink to trigger a call to AngularMoveDone when dest is reached
	SetNextThink(flTravelTime, true);
	SetThink(&CBaseToggle::AngularMoveDone);

	// scale the destdelta vector by the time spent traveling to get velocity
	UTIL_SetAvelocity(this, vecDestDelta / flTravelTime);
}

void CBaseToggle::AngularMoveDone()
{
	SetThink(&CBaseToggle::AngularMoveDoneNow);
	//	ALERT(at_console, "LMD: desiredThink %s\n", STRING(pev->targetname));
	UTIL_DesiredThink(this);
}

/*
============
After rotating, set angle to exact final angle, call "move done" function
============
*/
void CBaseToggle::AngularMoveDoneNow()
{
	//	ALERT(at_console, "AngularMoveDone %f\n", pev->ltime);
	UTIL_SetAvelocity(this, g_vecZero);
	if (m_pMoveWith)
	{
		UTIL_SetAngles(this, m_vecFinalAngle + m_pMoveWith->pev->angles);
	}
	else
	{
		UTIL_SetAngles(this, m_vecFinalAngle);
	}
	DontThink();
	if (m_pfnCallWhenMoveDone)
		(this->*m_pfnCallWhenMoveDone)();
}

// this isn't currently used. Otherwise I'd fix it to use movedir the way it should...
float CBaseToggle::AxisValue(int flags, const Vector& angles)
{
	if (FBitSet(flags, SF_DOOR_ROTATE_Z))
		return angles.z;
	if (FBitSet(flags, SF_DOOR_ROTATE_X))
		return angles.x;

	return angles.y;
}

void CBaseToggle::AxisDir(entvars_t* pev)
{
	if (pev->movedir != g_vecZero) // LRC
		return;

	if (FBitSet(pev->spawnflags, SF_DOOR_ROTATE_Z))
		pev->movedir = Vector(0, 0, 1); // around z-axis
	else if (FBitSet(pev->spawnflags, SF_DOOR_ROTATE_X))
		pev->movedir = Vector(1, 0, 0); // around x-axis
	else
		pev->movedir = Vector(0, 1, 0); // around y-axis
}

float CBaseToggle::AxisDelta(int flags, const Vector& angle1, const Vector& angle2)
{
	if (FBitSet(flags, SF_DOOR_ROTATE_Z))
		return angle1.z - angle2.z;

	if (FBitSet(flags, SF_DOOR_ROTATE_X))
		return angle1.x - angle2.x;

	return angle1.y - angle2.y;
}
