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

#include "CBaseMonster.h"
#include "CCineMonster.h"
#include "classes/nodes/CGraph.h"

//=========================================================
// GetTask - returns a pointer to the current
// scheduled task. NULL if there's a problem.
//=========================================================
Task_t* CBaseMonster::GetTask()
{
	if (m_iScheduleIndex < 0 || m_iScheduleIndex >= m_pSchedule->cTasks)
	{
		// m_iScheduleIndex is not within valid range for the monster's current schedule.
		return NULL;
	}
	else
	{
		return &m_pSchedule->pTasklist[m_iScheduleIndex];
	}
}

//=========================================================
// NextScheduledTask - increments the ScheduleIndex
//=========================================================
void CBaseMonster::NextScheduledTask()
{
	ASSERT(m_pSchedule != NULL);

	m_iTaskStatus = TASKSTATUS_NEW;
	m_iScheduleIndex++;

	if (FScheduleDone())
	{
		// just completed last task in schedule, so make it invalid by clearing it.
		SetConditions(bits_COND_SCHEDULE_DONE);
		// ClearSchedule();
	}
}

//=========================================================
// Start task - selects the correct activity and performs
// any necessary calculations to start the next task on the
// schedule.
//=========================================================
void CBaseMonster::StartTask(Task_t* pTask)
{
	switch (pTask->iTask)
	{
	case TASK_TURN_RIGHT:
	{
		float flCurrentYaw;

		flCurrentYaw = UTIL_AngleMod(pev->angles.y);
		pev->ideal_yaw = UTIL_AngleMod(flCurrentYaw - pTask->flData);
		SetTurnActivity();
		break;
	}
	case TASK_TURN_LEFT:
	{
		float flCurrentYaw;

		flCurrentYaw = UTIL_AngleMod(pev->angles.y);
		pev->ideal_yaw = UTIL_AngleMod(flCurrentYaw + pTask->flData);
		SetTurnActivity();
		break;
	}
	case TASK_REMEMBER:
	{
		Remember((int)pTask->flData);
		TaskComplete();
		break;
	}
	case TASK_FORGET:
	{
		Forget((int)pTask->flData);
		TaskComplete();
		break;
	}
	case TASK_FIND_HINTNODE:
	{
		m_iHintNode = FindHintNode();

		if (m_iHintNode != NO_NODE)
		{
			TaskComplete();
		}
		else
		{
			TaskFail();
		}
		break;
	}
	case TASK_STORE_LASTPOSITION:
	{
		m_vecLastPosition = pev->origin;
		TaskComplete();
		break;
	}
	case TASK_CLEAR_LASTPOSITION:
	{
		m_vecLastPosition = g_vecZero;
		TaskComplete();
		break;
	}
	case TASK_CLEAR_HINTNODE:
	{
		m_iHintNode = NO_NODE;
		TaskComplete();
		break;
	}
	case TASK_STOP_MOVING:
	{
		if (m_IdealActivity == m_movementActivity)
		{
			m_IdealActivity = GetStoppedActivity();
		}

		RouteClear();
		TaskComplete();
		break;
	}
	case TASK_PLAY_SEQUENCE_FACE_ENEMY:
	case TASK_PLAY_SEQUENCE_FACE_TARGET:
	case TASK_PLAY_SEQUENCE:
	{
		m_IdealActivity = (Activity)(int)pTask->flData;
		break;
	}
	case TASK_PLAY_ACTIVE_IDLE:
	{
		// monsters verify that they have a sequence for the node's activity BEFORE
		// moving towards the node, so it's ok to just set the activity without checking here.
		m_IdealActivity = (Activity)WorldGraph.m_pNodes[m_iHintNode].m_sHintActivity;
		break;
	}
	case TASK_SET_SCHEDULE:
	{
		Schedule_t* pNewSchedule;

		pNewSchedule = GetScheduleOfType((int)pTask->flData);

		if (pNewSchedule)
		{
			ChangeSchedule(pNewSchedule);
		}
		else
		{
			TaskFail();
		}

		break;
	}
	case TASK_FIND_NEAR_NODE_COVER_FROM_ENEMY:
	{
		if (m_hEnemy == NULL)
		{
			TaskFail();
			return;
		}

		if (FindCover(m_hEnemy->pev->origin, m_hEnemy->pev->view_ofs, 0, pTask->flData))
		{
			// try for cover farther than the FLData from the schedule.
			TaskComplete();
		}
		else
		{
			// no coverwhatsoever.
			TaskFail();
		}
		break;
	}
	case TASK_FIND_FAR_NODE_COVER_FROM_ENEMY:
	{
		if (m_hEnemy == NULL)
		{
			TaskFail();
			return;
		}

		if (FindCover(m_hEnemy->pev->origin, m_hEnemy->pev->view_ofs, pTask->flData, CoverRadius()))
		{
			// try for cover farther than the FLData from the schedule.
			TaskComplete();
		}
		else
		{
			// no coverwhatsoever.
			TaskFail();
		}
		break;
	}
	case TASK_FIND_NODE_COVER_FROM_ENEMY:
	{
		if (m_hEnemy == NULL)
		{
			TaskFail();
			return;
		}

		if (FindCover(m_hEnemy->pev->origin, m_hEnemy->pev->view_ofs, 0, CoverRadius()))
		{
			// try for cover farther than the FLData from the schedule.
			TaskComplete();
		}
		else
		{
			// no coverwhatsoever.
			TaskFail();
		}
		break;
	}
	case TASK_FIND_COVER_FROM_ENEMY:
	{
		entvars_t* pevCover;

		if (m_hEnemy == NULL)
		{
			// Find cover from self if no enemy available
			pevCover = pev;
			//				TaskFail();
			//				return;
		}
		else
			pevCover = m_hEnemy->pev;

		if (FindLateralCover(pevCover->origin, pevCover->view_ofs))
		{
			// try lateral first
			m_flMoveWaitFinished = gpGlobals->time + pTask->flData;
			TaskComplete();
		}
		else if (FindCover(pevCover->origin, pevCover->view_ofs, 0, CoverRadius()))
		{
			// then try for plain ole cover
			m_flMoveWaitFinished = gpGlobals->time + pTask->flData;
			TaskComplete();
		}
		else
		{
			// no coverwhatsoever.
			TaskFail();
		}
		break;
	}
	case TASK_FIND_COVER_FROM_ORIGIN:
	{
		if (FindCover(pev->origin, pev->view_ofs, 0, CoverRadius()))
		{
			// then try for plain ole cover
			m_flMoveWaitFinished = gpGlobals->time + pTask->flData;
			TaskComplete();
		}
		else
		{
			// no cover!
			TaskFail();
		}
	}
	break;
	case TASK_FIND_COVER_FROM_BEST_SOUND:
	{
		CSound* pBestSound;

		pBestSound = PBestSound();

		ASSERT(pBestSound != NULL);
		/*
			if ( pBestSound && FindLateralCover( pBestSound->m_vecOrigin, g_vecZero ) )
			{
				// try lateral first
				m_flMoveWaitFinished = gpGlobals->time + pTask->flData;
				TaskComplete();
			}
			*/

		if (pBestSound && FindCover(pBestSound->m_vecOrigin, g_vecZero, pBestSound->m_iVolume, CoverRadius()))
		{
			// then try for plain ole cover
			m_flMoveWaitFinished = gpGlobals->time + pTask->flData;
			TaskComplete();
		}
		else
		{
			// no coverwhatsoever. or no sound in list
			TaskFail();
		}
		break;
	}
	case TASK_FACE_HINTNODE:
	{
		pev->ideal_yaw = WorldGraph.m_pNodes[m_iHintNode].m_flHintYaw;
		SetTurnActivity();
		break;
	}

	case TASK_FACE_LASTPOSITION:
		MakeIdealYaw(m_vecLastPosition);
		SetTurnActivity();
		break;

	case TASK_FACE_TARGET:
		if (m_hTargetEnt != NULL)
		{
			MakeIdealYaw(m_hTargetEnt->pev->origin);
			SetTurnActivity();
		}
		else
			TaskFail();
		break;
	case TASK_FACE_ENEMY:
	{
		MakeIdealYaw(m_vecEnemyLKP);
		SetTurnActivity();
		break;
	}
	case TASK_FACE_IDEAL:
	{
		SetTurnActivity();
		break;
	}
	case TASK_FACE_ROUTE:
	{
		if (FRouteClear())
		{
			ALERT(at_aiconsole, "No route to face!\n");
			TaskFail();
		}
		else
		{
			MakeIdealYaw(m_Route[m_iRouteIndex].vecLocation);
			SetTurnActivity();
		}
		break;
	}
	case TASK_WAIT_PVS:
	case TASK_WAIT_INDEFINITE:
	{
		// don't do anything.
		break;
	}
	case TASK_WAIT:
	case TASK_WAIT_FACE_ENEMY:
	{ // set a future time that tells us when the wait is over.
		m_flWaitFinished = gpGlobals->time + pTask->flData;
		break;
	}
	case TASK_WAIT_RANDOM:
	{ // set a future time that tells us when the wait is over.
		m_flWaitFinished = gpGlobals->time + RANDOM_FLOAT(0.1, pTask->flData);
		break;
	}
	case TASK_MOVE_TO_TARGET_RANGE:
	{
		if ((m_hTargetEnt->pev->origin - pev->origin).Length() < 1)
			TaskComplete();
		else
		{
			m_vecMoveGoal = m_hTargetEnt->pev->origin;
			if (!MoveToTarget(ACT_WALK, 2))
				TaskFail();
		}
		break;
	}
	case TASK_RUN_TO_SCRIPT:
	case TASK_WALK_TO_SCRIPT:
	{
		Activity newActivity;

		if (!m_pGoalEnt || (m_pGoalEnt->pev->origin - pev->origin).Length() < 1)
			TaskComplete();
		else
		{
			if (pTask->iTask == TASK_WALK_TO_SCRIPT)
				newActivity = ACT_WALK;
			else
				newActivity = ACT_RUN;
			// This monster can't do this!
			if (LookupActivity(newActivity) == ACTIVITY_NOT_AVAILABLE)
				TaskComplete();
			else
			{
				if (m_pGoalEnt != NULL)
				{
					Vector vecDest;
					vecDest = m_pGoalEnt->pev->origin;

					if (!MoveToLocation(newActivity, 2, vecDest))
					{
						TaskFail();
						ALERT(at_aiconsole, "%s Failed to reach script!!!\n", STRING(pev->classname));
						RouteClear();
					}
				}
				else
				{
					TaskFail();
					ALERT(at_aiconsole, "%s: MoveTarget is missing!?!\n", STRING(pev->classname));
					RouteClear();
				}
			}
		}
		TaskComplete();
		break;
	}
	case TASK_CLEAR_MOVE_WAIT:
	{
		m_flMoveWaitFinished = gpGlobals->time;
		TaskComplete();
		break;
	}
	case TASK_MELEE_ATTACK1_NOTURN:
	case TASK_MELEE_ATTACK1:
	{
		m_IdealActivity = ACT_MELEE_ATTACK1;
		break;
	}
	case TASK_MELEE_ATTACK2_NOTURN:
	case TASK_MELEE_ATTACK2:
	{
		m_IdealActivity = ACT_MELEE_ATTACK2;
		break;
	}
	case TASK_RANGE_ATTACK1_NOTURN:
	case TASK_RANGE_ATTACK1:
	{
		m_IdealActivity = ACT_RANGE_ATTACK1;
		break;
	}
	case TASK_RANGE_ATTACK2_NOTURN:
	case TASK_RANGE_ATTACK2:
	{
		m_IdealActivity = ACT_RANGE_ATTACK2;
		break;
	}
	case TASK_RELOAD_NOTURN:
	case TASK_RELOAD:
	{
		m_IdealActivity = ACT_RELOAD;
		break;
	}
	case TASK_SPECIAL_ATTACK1:
	{
		m_IdealActivity = ACT_SPECIAL_ATTACK1;
		break;
	}
	case TASK_SPECIAL_ATTACK2:
	{
		m_IdealActivity = ACT_SPECIAL_ATTACK2;
		break;
	}
	case TASK_SET_ACTIVITY:
	{
		m_IdealActivity = (Activity)(int)pTask->flData;
		TaskComplete();
		break;
	}
	case TASK_GET_PATH_TO_ENEMY_LKP:
	{
		if (BuildRoute(m_vecEnemyLKP, bits_MF_TO_LOCATION, NULL))
		{
			TaskComplete();
		}
		else if (BuildNearestRoute(m_vecEnemyLKP, pev->view_ofs, 0, (m_vecEnemyLKP - pev->origin).Length()))
		{
			TaskComplete();
		}
		else
		{
			// no way to get there =(
			ALERT(at_aiconsole, "GetPathToEnemyLKP failed!!\n");
			TaskFail();
		}
		break;
	}
	case TASK_GET_PATH_TO_ENEMY:
	{
		CBaseEntity* pEnemy = m_hEnemy;

		if (pEnemy == NULL)
		{
			TaskFail();
			return;
		}

		if (BuildRoute(pEnemy->pev->origin, bits_MF_TO_ENEMY, pEnemy))
		{
			TaskComplete();
		}
		else if (BuildNearestRoute(pEnemy->pev->origin, pEnemy->pev->view_ofs, 0, (pEnemy->pev->origin - pev->origin).Length()))
		{
			TaskComplete();
		}
		else
		{
			// no way to get there =(
			ALERT(at_aiconsole, "GetPathToEnemy failed!!\n");
			TaskFail();
		}
		break;
	}
	case TASK_GET_PATH_TO_ENEMY_CORPSE:
	{
		UTIL_MakeVectors(pev->angles);
		if (BuildRoute(m_vecEnemyLKP - gpGlobals->v_forward * 64, bits_MF_TO_LOCATION, NULL))
		{
			TaskComplete();
		}
		else
		{
			ALERT(at_aiconsole, "GetPathToEnemyCorpse failed!!\n");
			TaskFail();
		}
	}
	break;
	case TASK_GET_PATH_TO_SPOT:
	{
		CBaseEntity* pPlayer = UTIL_FindEntityByClassname(NULL, "player");
		if (BuildRoute(m_vecMoveGoal, bits_MF_TO_LOCATION, pPlayer))
		{
			TaskComplete();
		}
		else
		{
			// no way to get there =(
			ALERT(at_aiconsole, "GetPathToSpot failed!!\n");
			TaskFail();
		}
		break;
	}

	case TASK_GET_PATH_TO_TARGET:
	{
		RouteClear();
		if (m_hTargetEnt != NULL && MoveToTarget(m_movementActivity, 1))
		{
			TaskComplete();
		}
		else
		{
			// no way to get there =(
			ALERT(at_aiconsole, "GetPathToSpot failed!!\n");
			TaskFail();
		}
		break;
	}
	case TASK_GET_PATH_TO_SCRIPT:
	{
		RouteClear();
		if (m_pCine != NULL && MoveToLocation(m_movementActivity, 1, m_pCine->pev->origin))
		{
			TaskComplete();
		}
		else
		{
			// no way to get there =(
			ALERT(at_aiconsole, "GetPathToSpot failed!!\n");
			TaskFail();
		}
		break;
	}
	case TASK_GET_PATH_TO_HINTNODE: // for active idles!
	{
		if (MoveToLocation(m_movementActivity, 2, WorldGraph.m_pNodes[m_iHintNode].m_vecOrigin))
		{
			TaskComplete();
		}
		else
		{
			// no way to get there =(
			ALERT(at_aiconsole, "GetPathToHintNode failed!!\n");
			TaskFail();
		}
		break;
	}
	case TASK_GET_PATH_TO_LASTPOSITION:
	{
		m_vecMoveGoal = m_vecLastPosition;

		if (MoveToLocation(m_movementActivity, 2, m_vecMoveGoal))
		{
			TaskComplete();
		}
		else
		{
			// no way to get there =(
			ALERT(at_aiconsole, "GetPathToLastPosition failed!!\n");
			TaskFail();
		}
		break;
	}
	case TASK_GET_PATH_TO_BESTSOUND:
	{
		CSound* pSound;

		pSound = PBestSound();

		if (pSound && MoveToLocation(m_movementActivity, 2, pSound->m_vecOrigin))
		{
			TaskComplete();
		}
		else
		{
			// no way to get there =(
			ALERT(at_aiconsole, "GetPathToBestSound failed!!\n");
			TaskFail();
		}
		break;
	}
	case TASK_GET_PATH_TO_BESTSCENT:
	{
		CSound* pScent;

		pScent = PBestScent();

		if (pScent && MoveToLocation(m_movementActivity, 2, pScent->m_vecOrigin))
		{
			TaskComplete();
		}
		else
		{
			// no way to get there =(
			ALERT(at_aiconsole, "GetPathToBestScent failed!!\n");

			TaskFail();
		}
		break;
	}
	case TASK_RUN_PATH:
	{
		// UNDONE: This is in some default AI and some monsters can't run? -- walk instead?
		if (LookupActivity(ACT_RUN) != ACTIVITY_NOT_AVAILABLE)
		{
			m_movementActivity = ACT_RUN;
		}
		else
		{
			m_movementActivity = ACT_WALK;
		}
		TaskComplete();
		break;
	}
	case TASK_WALK_PATH:
	{
		if (pev->movetype == MOVETYPE_FLY)
		{
			m_movementActivity = ACT_FLY;
		}
		if (LookupActivity(ACT_WALK) != ACTIVITY_NOT_AVAILABLE)
		{
			m_movementActivity = ACT_WALK;
		}
		else
		{
			m_movementActivity = ACT_RUN;
		}
		TaskComplete();
		break;
	}
	case TASK_STRAFE_PATH:
	{
		Vector2D vec2DirToPoint;
		Vector2D vec2RightSide;

		// to start strafing, we have to first figure out if the target is on the left side or right side
		UTIL_MakeVectors(pev->angles);

		vec2DirToPoint = (m_Route[0].vecLocation - pev->origin).Make2D().Normalize();
		vec2RightSide = gpGlobals->v_right.Make2D().Normalize();

		if (DotProduct(vec2DirToPoint, vec2RightSide) > 0)
		{
			// strafe right
			m_movementActivity = ACT_STRAFE_RIGHT;
		}
		else
		{
			// strafe left
			m_movementActivity = ACT_STRAFE_LEFT;
		}
		TaskComplete();
		break;
	}


	case TASK_WAIT_FOR_MOVEMENT:
	{
		if (FRouteClear())
		{
			TaskComplete();
		}
		break;
	}

	case TASK_EAT:
	{
		Eat(pTask->flData);
		TaskComplete();
		break;
	}
	case TASK_SMALL_FLINCH:
	{
		m_IdealActivity = GetSmallFlinchActivity();
		break;
	}
	case TASK_DIE:
	{
		RouteClear();

		m_IdealActivity = GetDeathActivity();

		pev->deadflag = DEAD_DYING;
		break;
	}
	case TASK_SOUND_WAKE:
	{
		AlertSound();
		TaskComplete();
		break;
	}
	case TASK_SOUND_DIE:
	{
		DeathSound();
		TaskComplete();
		break;
	}
	case TASK_SOUND_IDLE:
	{
		IdleSound();
		TaskComplete();
		break;
	}
	case TASK_SOUND_PAIN:
	{
		PainSound();
		TaskComplete();
		break;
	}
	case TASK_SOUND_DEATH:
	{
		DeathSound();
		TaskComplete();
		break;
	}
	case TASK_SOUND_ANGRY:
	{
		// sounds are complete as soon as we get here, cause we've already played them.
		ALERT(at_aiconsole, "SOUND\n");
		TaskComplete();
		break;
	}
	case TASK_WAIT_FOR_SCRIPT:
	{
		if (m_pCine->m_iDelay <= 0 && gpGlobals->time >= m_pCine->m_startTime)
		{
			TaskComplete(); // LRC - start playing immediately
		}
		else if (!m_pCine->IsAction() && !FStringNull(m_pCine->m_iszIdle))
		{
			m_pCine->StartSequence((CBaseMonster*)this, m_pCine->m_iszIdle, false);
			if (FStrEq(STRING(m_pCine->m_iszIdle), STRING(m_pCine->m_iszPlay)))
			{
				pev->framerate = 0;
			}
		}
		else
			m_IdealActivity = ACT_IDLE;

		break;
	}
	case TASK_PLAY_SCRIPT:
	{
		if (m_pCine->IsAction())
		{
			// ALERT(at_console,"PlayScript: setting idealactivity %d\n",m_pCine->m_fAction);
			switch (m_pCine->m_fAction)
			{
			case 0:
				m_IdealActivity = ACT_RANGE_ATTACK1;
				break;
			case 1:
				m_IdealActivity = ACT_RANGE_ATTACK2;
				break;
			case 2:
				m_IdealActivity = ACT_MELEE_ATTACK1;
				break;
			case 3:
				m_IdealActivity = ACT_MELEE_ATTACK2;
				break;
			case 4:
				m_IdealActivity = ACT_SPECIAL_ATTACK1;
				break;
			case 5:
				m_IdealActivity = ACT_SPECIAL_ATTACK2;
				break;
			case 6:
				m_IdealActivity = ACT_RELOAD;
				break;
			case 7:
				m_IdealActivity = ACT_HOP;
				break;
			}
			pev->framerate = 1.0; // shouldn't be needed, but just in case
			pev->movetype = MOVETYPE_FLY;
			ClearBits(pev->flags, FL_ONGROUND);
		}
		else
		{
			m_pCine->StartSequence((CBaseMonster*)this, m_pCine->m_iszPlay, true);
			if (m_fSequenceFinished)
				ClearSchedule();
			pev->framerate = 1.0;
			// ALERT( at_aiconsole, "Script %s has begun for %s\n", STRING( m_pCine->m_iszPlay ), STRING(pev->classname) );
		}
		m_scriptState = SCRIPT_PLAYING;
		break;
	}
	case TASK_ENABLE_SCRIPT:
	{
		m_pCine->DelayStart(false);
		TaskComplete();
		break;
	}
		// LRC
	case TASK_END_SCRIPT:
	{
		m_pCine->SequenceDone(this);
		TaskComplete();
		break;
	}
	case TASK_PLANT_ON_SCRIPT:
	{
		if (m_pCine != NULL)
		{
			// Plant on script
			// LRC - if it's a teleport script, do the turn too
			if (m_pCine->m_fMoveTo == 4 || m_pCine->m_fMoveTo == 6)
			{
				if (m_pCine->m_fTurnType == 0) // LRC
					pev->angles.y = m_hTargetEnt->pev->angles.y;
				else if (m_pCine->m_fTurnType == 1)
					pev->angles.y = UTIL_VecToYaw(m_hTargetEnt->pev->origin - pev->origin);
				pev->ideal_yaw = pev->angles.y;
				pev->avelocity = Vector(0, 0, 0);
				pev->velocity = Vector(0, 0, 0);
				pev->effects |= EF_NOINTERP;
			}

			if (m_pCine->m_fMoveTo != 6)
				pev->origin = m_pGoalEnt->pev->origin;
		}

		TaskComplete();
		break;
	}
	case TASK_FACE_SCRIPT:
	{
		if (m_pCine != NULL && m_pCine->m_fMoveTo != 0) // movetype "no move" makes us ignore turntype
		{
			switch (m_pCine->m_fTurnType)
			{
			case 0:
				pev->ideal_yaw = UTIL_AngleMod(m_pCine->pev->angles.y);
				break;
			case 1:
				// yes, this is inconsistent- turn to face uses the "target" and turn to angle uses the "cine".
				if (m_hTargetEnt)
					MakeIdealYaw(m_hTargetEnt->pev->origin);
				else
					MakeIdealYaw(m_pCine->pev->origin);
				break;
				// default: don't turn
			}
		}

		TaskComplete();
		m_IdealActivity = ACT_IDLE;
		RouteClear();
		break;
	}

	case TASK_SUGGEST_STATE:
	{
		m_IdealMonsterState = (MONSTERSTATE)(int)pTask->flData;
		TaskComplete();
		break;
	}

	case TASK_SET_FAIL_SCHEDULE:
		m_failSchedule = (int)pTask->flData;
		TaskComplete();
		break;

	case TASK_CLEAR_FAIL_SCHEDULE:
		m_failSchedule = SCHED_NONE;
		TaskComplete();
		break;

	default:
	{
		ALERT(at_aiconsole, "No StartTask entry for %d\n", (SHARED_TASKS)pTask->iTask);
		break;
	}
	}
}

//=========================================================
// RunTask
//=========================================================
void CBaseMonster::RunTask(Task_t* pTask)
{
	switch (pTask->iTask)
	{
	case TASK_TURN_RIGHT:
	case TASK_TURN_LEFT:
	{
		ChangeYaw(pev->yaw_speed);

		if (FacingIdeal())
		{
			TaskComplete();
		}
		break;
	}

	case TASK_PLAY_SEQUENCE_FACE_ENEMY:
	case TASK_PLAY_SEQUENCE_FACE_TARGET:
	{
		CBaseEntity* pTarget;

		if (pTask->iTask == TASK_PLAY_SEQUENCE_FACE_TARGET)
			pTarget = m_hTargetEnt;
		else
			pTarget = m_hEnemy;
		if (pTarget)
		{
			pev->ideal_yaw = UTIL_VecToYaw(pTarget->pev->origin - pev->origin);
			ChangeYaw(pev->yaw_speed);
		}
		if (m_fSequenceFinished)
			TaskComplete();
	}
	break;

	case TASK_PLAY_SEQUENCE:
	case TASK_PLAY_ACTIVE_IDLE:
	{
		if (m_fSequenceFinished)
		{
			TaskComplete();
		}
		break;
	}


	case TASK_FACE_ENEMY:
	{
		MakeIdealYaw(m_vecEnemyLKP);

		ChangeYaw(pev->yaw_speed);

		if (FacingIdeal())
		{
			TaskComplete();
		}
		break;
	}
	case TASK_FACE_HINTNODE:
	case TASK_FACE_LASTPOSITION:
	case TASK_FACE_TARGET:
	case TASK_FACE_IDEAL:
	case TASK_FACE_ROUTE:
	{
		ChangeYaw(pev->yaw_speed);

		if (FacingIdeal())
		{
			TaskComplete();
		}
		break;
	}
	case TASK_WAIT_PVS:
	{
		if (!FNullEnt(FIND_CLIENT_IN_PVS(edict())) || HaveCamerasInPVS(edict()))
		{
			TaskComplete();
		}
		break;
	}
	case TASK_WAIT_INDEFINITE:
	{
		// don't do anything.
		break;
	}
	case TASK_WAIT:
	case TASK_WAIT_RANDOM:
	{
		if (gpGlobals->time >= m_flWaitFinished)
		{
			TaskComplete();
		}
		break;
	}
	case TASK_WAIT_FACE_ENEMY:
	{
		MakeIdealYaw(m_vecEnemyLKP);
		ChangeYaw(pev->yaw_speed);

		if (gpGlobals->time >= m_flWaitFinished)
		{
			TaskComplete();
		}
		break;
	}
	case TASK_MOVE_TO_TARGET_RANGE:
	{
		float distance;

		if (m_hTargetEnt == NULL)
			TaskFail();
		else
		{
			distance = (m_vecMoveGoal - pev->origin).Length2D();
			// Re-evaluate when you think your finished, or the target has moved too far
			if ((distance < pTask->flData) || (m_vecMoveGoal - m_hTargetEnt->pev->origin).Length() > pTask->flData * 0.5)
			{
				m_vecMoveGoal = m_hTargetEnt->pev->origin;
				distance = (m_vecMoveGoal - pev->origin).Length2D();
				FRefreshRoute();
			}

			// Set the appropriate activity based on an overlapping range
			// overlap the range to prevent oscillation
			if (distance < pTask->flData)
			{
				TaskComplete();
				RouteClear(); // Stop moving
			}
			else if (distance < 190 && m_movementActivity != ACT_WALK)
				m_movementActivity = ACT_WALK;
			else if (distance >= 270 && m_movementActivity != ACT_RUN)
				m_movementActivity = ACT_RUN;
		}

		break;
	}
	case TASK_WAIT_FOR_MOVEMENT:
	{
		if (MovementIsComplete())
		{
			TaskComplete();
			RouteClear(); // Stop moving
		}
		break;
	}
	case TASK_DIE:
	{
		if (m_fSequenceFinished && pev->frame >= 255)
		{
			pev->deadflag = DEAD_DEAD;

			SetThink(NULL);
			StopAnimation();

			if (!BBoxFlat())
			{
				// a bit of a hack. If a corpses' bbox is positioned such that being left solid so that it can be attacked will
				// block the player on a slope or stairs, the corpse is made nonsolid.
				//					pev->solid = SOLID_NOT;
				UTIL_SetSize(pev, Vector(-4, -4, 0), Vector(4, 4, 1));
			}
			else // !!!HACKHACK - put monster in a thin, wide bounding box until we fix the solid type/bounding volume problem
				UTIL_SetSize(pev, Vector(pev->mins.x, pev->mins.y, pev->mins.z), Vector(pev->maxs.x, pev->maxs.y, pev->mins.z + 1));

			if (ShouldFadeOnDeath())
			{
				// this monster was created by a monstermaker... fade the corpse out.
				SUB_StartFadeOut();
			}
			else
			{
				// body is gonna be around for a while, so have it stink for a bit.
				CSoundEnt::InsertSound(bits_SOUND_CARCASS, pev->origin, 384, 30);
			}
		}
		break;
	}
	case TASK_RANGE_ATTACK1_NOTURN:
	case TASK_MELEE_ATTACK1_NOTURN:
	case TASK_MELEE_ATTACK2_NOTURN:
	case TASK_RANGE_ATTACK2_NOTURN:
	case TASK_RELOAD_NOTURN:
	{
		if (m_fSequenceFinished)
		{
			m_Activity = ACT_RESET;
			TaskComplete();
		}
		break;
	}
	case TASK_RANGE_ATTACK1:
	case TASK_MELEE_ATTACK1:
	case TASK_MELEE_ATTACK2:
	case TASK_RANGE_ATTACK2:
	case TASK_SPECIAL_ATTACK1:
	case TASK_SPECIAL_ATTACK2:
	case TASK_RELOAD:
	{
		MakeIdealYaw(m_vecEnemyLKP);
		ChangeYaw(pev->yaw_speed);

		if (m_fSequenceFinished)
		{
			m_Activity = ACT_RESET;
			TaskComplete();
		}
		break;
	}
	case TASK_SMALL_FLINCH:
	{
		if (m_fSequenceFinished)
		{
			TaskComplete();
		}
	}
	break;
	case TASK_WAIT_FOR_SCRIPT:
	{
		if (m_pCine->m_iDelay <= 0 && gpGlobals->time >= m_pCine->m_startTime)
		{
			TaskComplete();
		}
		break;
	}
	case TASK_PLAY_SCRIPT:
	{
		//			ALERT(at_console, "Play Script\n");
		if (m_fSequenceFinished)
		{
			//				ALERT(at_console, "Anim Finished\n");
			if (m_pCine->m_iRepeatsLeft > 0)
			{
				//					ALERT(at_console, "Frame %f; Repeat %d from %f\n", pev->frame, m_pCine->m_iRepeatsLeft, m_pCine->m_fRepeatFrame);
				m_pCine->m_iRepeatsLeft--;
				pev->frame = m_pCine->m_fRepeatFrame;
				ResetSequenceInfo();
			}
			else
			{
				TaskComplete();
			}
		}
		break;
	}
	}
}
