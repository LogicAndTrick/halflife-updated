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
#include "CSquadMonster.h"
#include "nodes.h"

//=========================================================
// SetState
//=========================================================
void CBaseMonster::SetState(MONSTERSTATE State)
{
	/*
	if ( State != m_MonsterState )
	{
		ALERT ( at_aiconsole, "State Changed to %d\n", State );
	}
*/

	switch (State)
	{

	// Drop enemy pointers when going to idle
	case MONSTERSTATE_IDLE:

		if (m_hEnemy != NULL)
		{
			m_hEnemy = NULL; // not allowed to have an enemy anymore.
			ALERT(at_aiconsole, "Stripped\n");
		}
		break;
	}

	m_MonsterState = State;
	m_IdealMonsterState = State;
}

//=========================================================
// RunAI
//=========================================================
void CBaseMonster::RunAI()
{
	// to test model's eye height
	// UTIL_ParticleEffect ( pev->origin + pev->view_ofs, g_vecZero, 255, 10 );

	// IDLE sound permitted in ALERT state is because monsters were silent in ALERT state. Only play IDLE sound in IDLE state
	// once we have sounds for that state.
	if ((m_MonsterState == MONSTERSTATE_IDLE || m_MonsterState == MONSTERSTATE_ALERT) && RANDOM_LONG(0, 99) == 0 && (pev->spawnflags & SF_MONSTER_GAG) == 0)
	{
		IdleSound();
	}

	if (m_MonsterState != MONSTERSTATE_NONE &&
		m_MonsterState != MONSTERSTATE_PRONE &&
		m_MonsterState != MONSTERSTATE_DEAD) // don't bother with this crap if monster is prone.
	{
		// collect some sensory Condition information.
		// don't let monsters outside of the player's PVS act up, or most of the interesting
		// things will happen before the player gets there!
		// UPDATE: We now let COMBAT state monsters think and act fully outside of player PVS. This allows the player to leave
		// an area where monsters are fighting, and the fight will continue.
		if (!FNullEnt(FIND_CLIENT_IN_PVS(edict())) || (m_MonsterState == MONSTERSTATE_COMBAT) || HaveCamerasInPVS(edict()))
		{
			Look(m_flDistLook);
			Listen(); // check for audible sounds.

			// now filter conditions.
			ClearConditions(IgnoreConditions());

			GetEnemy();
		}

		// do these calculations if monster has an enemy.
		if (m_hEnemy != NULL)
		{
			CheckEnemy(m_hEnemy);
		}

		CheckAmmo();
	}

	FCheckAITrigger();

	PrescheduleThink();

	MaintainSchedule();

	// if the monster didn't use these conditions during the above call to MaintainSchedule() or CheckAITrigger()
	// we throw them out cause we don't want them sitting around through the lifespan of a schedule
	// that doesn't use them.
	m_afConditions &= ~(bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE);
}

//=========================================================
// GetIdealState - surveys the Conditions information available
// and finds the best new state for a monster.
//=========================================================
MONSTERSTATE CBaseMonster::GetIdealState()
{
	int iConditions;

	iConditions = IScheduleFlags();

	// If no schedule conditions, the new ideal state is probably the reason we're in here.
	switch (m_MonsterState)
	{
	case MONSTERSTATE_IDLE:

		/*
		IDLE goes to ALERT upon hearing a sound
		-IDLE goes to ALERT upon being injured
		IDLE goes to ALERT upon seeing food
		-IDLE goes to COMBAT upon sighting an enemy
		IDLE goes to HUNT upon smelling food
		*/
		{
			if ((iConditions & bits_COND_NEW_ENEMY) != 0)
			{
				// new enemy! This means an idle monster has seen someone it dislikes, or
				// that a monster in combat has found a more suitable target to attack
				m_IdealMonsterState = MONSTERSTATE_COMBAT;
			}
			else if ((iConditions & bits_COND_LIGHT_DAMAGE) != 0)
			{
				MakeIdealYaw(m_vecEnemyLKP);
				m_IdealMonsterState = MONSTERSTATE_ALERT;
			}
			else if ((iConditions & bits_COND_HEAVY_DAMAGE) != 0)
			{
				MakeIdealYaw(m_vecEnemyLKP);
				m_IdealMonsterState = MONSTERSTATE_ALERT;
			}
			else if ((iConditions & bits_COND_HEAR_SOUND) != 0)
			{
				CSound* pSound;

				pSound = PBestSound();
				ASSERT(pSound != NULL);
				if (pSound)
				{
					MakeIdealYaw(pSound->m_vecOrigin);
					if ((pSound->m_iType & (bits_SOUND_COMBAT | bits_SOUND_DANGER)) != 0)
						m_IdealMonsterState = MONSTERSTATE_ALERT;
				}
			}
			else if ((iConditions & (bits_COND_SMELL | bits_COND_SMELL_FOOD)) != 0)
			{
				m_IdealMonsterState = MONSTERSTATE_ALERT;
			}

			break;
		}
	case MONSTERSTATE_ALERT:
		/*
		ALERT goes to IDLE upon becoming bored
		-ALERT goes to COMBAT upon sighting an enemy
		ALERT goes to HUNT upon hearing a noise
		*/
		{
			if ((iConditions & (bits_COND_NEW_ENEMY | bits_COND_SEE_ENEMY)) != 0)
			{
				// see an enemy we MUST attack
				m_IdealMonsterState = MONSTERSTATE_COMBAT;
			}
			else if ((iConditions & bits_COND_HEAR_SOUND) != 0)
			{
				m_IdealMonsterState = MONSTERSTATE_ALERT;
				CSound* pSound = PBestSound();
				ASSERT(pSound != NULL);
				if (pSound)
					MakeIdealYaw(pSound->m_vecOrigin);
			}
			break;
		}
	case MONSTERSTATE_COMBAT:
		/*
		COMBAT goes to HUNT upon losing sight of enemy
		COMBAT goes to ALERT upon death of enemy
		*/
		{
			if (m_hEnemy == NULL)
			{
				m_IdealMonsterState = MONSTERSTATE_ALERT;
				// pev->effects = EF_BRIGHTFIELD;
				ALERT(at_aiconsole, "***Combat state with no enemy!\n");
			}
			break;
		}
	case MONSTERSTATE_HUNT:
		/*
		HUNT goes to ALERT upon seeing food
		HUNT goes to ALERT upon being injured
		HUNT goes to IDLE if goal touched
		HUNT goes to COMBAT upon seeing enemy
		*/
		{
			break;
		}
	case MONSTERSTATE_SCRIPT:
		if ((iConditions & (bits_COND_TASK_FAILED | bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE)) != 0)
		{
			ExitScriptedSequence(); // This will set the ideal state
		}
		break;

	case MONSTERSTATE_DEAD:
		m_IdealMonsterState = MONSTERSTATE_DEAD;
		break;
	}

	return m_IdealMonsterState;
}

//=========================================================
// FCheckAITrigger - checks the monster's AI Trigger Conditions,
// if there is a condition, then checks to see if condition is
// met. If yes, the monster's TriggerTarget is fired.
//
// Returns true if the target is fired.
//=========================================================
bool CBaseMonster::FCheckAITrigger()
{
	bool fFireTarget;

	if (m_iTriggerCondition == AITRIGGER_NONE)
	{
		// no conditions, so this trigger is never fired.
		return false;
	}

	fFireTarget = false;

	switch (m_iTriggerCondition)
	{
	case AITRIGGER_SEEPLAYER_ANGRY_AT_PLAYER:
		if (m_hEnemy != NULL && m_hEnemy->IsPlayer() && HasConditions(bits_COND_SEE_ENEMY))
		{
			fFireTarget = true;
		}
		break;
	case AITRIGGER_SEEPLAYER_UNCONDITIONAL:
		if (HasConditions(bits_COND_SEE_CLIENT))
		{
			fFireTarget = true;
		}
		break;
	case AITRIGGER_SEEPLAYER_NOT_IN_COMBAT:
		if (HasConditions(bits_COND_SEE_CLIENT) &&
			m_MonsterState != MONSTERSTATE_COMBAT &&
			m_MonsterState != MONSTERSTATE_PRONE &&
			m_MonsterState != MONSTERSTATE_SCRIPT)
		{
			fFireTarget = true;
		}
		break;
	case AITRIGGER_TAKEDAMAGE:
		if ((m_afConditions & (bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE)) != 0)
		{
			fFireTarget = true;
		}
		break;
	case AITRIGGER_DEATH:
		if (pev->deadflag != DEAD_NO)
		{
			fFireTarget = true;
		}
		break;
	case AITRIGGER_HALFHEALTH:
		if (IsAlive() && pev->health <= (pev->max_health / 2))
		{
			fFireTarget = true;
		}
		break;
		/*

  // !!!UNDONE - no persistant game state that allows us to track these two.

	case AITRIGGER_SQUADMEMBERDIE:
		break;
	case AITRIGGER_SQUADLEADERDIE:
		break;
*/
	case AITRIGGER_HEARWORLD:
		if ((m_afConditions & bits_COND_HEAR_SOUND) != 0 && (m_afSoundTypes & bits_SOUND_WORLD) != 0)
		{
			fFireTarget = true;
		}
		break;
	case AITRIGGER_HEARPLAYER:
		if ((m_afConditions & bits_COND_HEAR_SOUND) != 0 && (m_afSoundTypes & bits_SOUND_PLAYER) != 0)
		{
			fFireTarget = true;
		}
		break;
	case AITRIGGER_HEARCOMBAT:
		if ((m_afConditions & bits_COND_HEAR_SOUND) != 0 && (m_afSoundTypes & bits_SOUND_COMBAT) != 0)
		{
			fFireTarget = true;
		}
		break;
	}

	if (fFireTarget)
	{
		// fire the target, then set the trigger conditions to NONE so we don't fire again
		ALERT(at_aiconsole, "AI Trigger Fire Target\n");
		FireTargets(STRING(m_iszTriggerTarget), this, this, USE_TOGGLE, 0);
		m_iTriggerCondition = AITRIGGER_NONE;
		return true;
	}

	return false;
}

void CBaseMonster::ReportAIState()
{
	ALERT_TYPE level = at_console;

	static const char* pStateNames[] = {"None", "Idle", "Combat", "Alert", "Hunt", "Prone", "Scripted", "PlayDead", "Dead"};

	ALERT(level, "%s: ", STRING(pev->classname));
	if ((int)m_MonsterState < ARRAYSIZE(pStateNames))
		ALERT(level, "State: %s, ", pStateNames[m_MonsterState]);
	int i = 0;
	while (activity_map[i].type != 0)
	{
		if (activity_map[i].type == (int)m_Activity)
		{
			ALERT(level, "Activity %s, ", activity_map[i].name);
			break;
		}
		i++;
	}

	if (m_pSchedule)
	{
		const char* pName = NULL;
		pName = m_pSchedule->pName;
		if (!pName)
			pName = "Unknown";
		ALERT(level, "Schedule %s, ", pName);
		Task_t* pTask = GetTask();
		if (pTask)
			ALERT(level, "Task %d (#%d), ", pTask->iTask, m_iScheduleIndex);
	}
	else
		ALERT(level, "No Schedule, ");

	if (m_hEnemy != NULL)
		ALERT(level, "\nEnemy is %s", STRING(m_hEnemy->pev->classname));
	else
		ALERT(level, "No enemy");

	if (IsMoving())
	{
		ALERT(level, " Moving ");
		if (m_flMoveWaitFinished > gpGlobals->time)
			ALERT(level, ": Stopped for %.2f. ", m_flMoveWaitFinished - gpGlobals->time);
		else if (m_IdealActivity == GetStoppedActivity())
			ALERT(level, ": In stopped anim. ");
	}

	CSquadMonster* pSquadMonster = MySquadMonsterPointer();

	if (pSquadMonster)
	{
		if (!pSquadMonster->InSquad())
		{
			ALERT(level, "not ");
		}

		ALERT(level, "In Squad, ");

		if (!pSquadMonster->IsLeader())
		{
			ALERT(level, "not ");
		}

		ALERT(level, "Leader.");
	}

	ALERT(level, "\n");
	ALERT(level, "Yaw speed:%3.1f,Health: %3.1f\n", pev->yaw_speed, pev->health);
	if ((pev->spawnflags & SF_MONSTER_PRISONER) != 0)
		ALERT(level, " PRISONER! ");
	if ((pev->spawnflags & SF_MONSTER_PREDISASTER) != 0)
		ALERT(level, " Pre-Disaster! ");
	ALERT(level, "\n");
}

//=========================================================
// FindHintNode
//=========================================================
int CBaseMonster::FindHintNode()
{
	int i;
	TraceResult tr;

	if (0 == WorldGraph.m_fGraphPresent)
	{
		ALERT(at_aiconsole, "find_hintnode: graph not ready!\n");
		return NO_NODE;
	}

	if (WorldGraph.m_iLastActiveIdleSearch >= WorldGraph.m_cNodes)
	{
		WorldGraph.m_iLastActiveIdleSearch = 0;
	}

	for (i = 0; i < WorldGraph.m_cNodes; i++)
	{
		int nodeNumber = (i + WorldGraph.m_iLastActiveIdleSearch) % WorldGraph.m_cNodes;
		CNode& node = WorldGraph.Node(nodeNumber);

		if (0 != node.m_sHintType)
		{
			// this node has a hint. Take it if it is visible, the monster likes it, and the monster has an animation to match the hint's activity.
			if (FValidateHintType(node.m_sHintType))
			{
				if (0 == node.m_sHintActivity || LookupActivity(node.m_sHintActivity) != ACTIVITY_NOT_AVAILABLE)
				{
					UTIL_TraceLine(pev->origin + pev->view_ofs, node.m_vecOrigin + pev->view_ofs, ignore_monsters, ENT(pev), &tr);

					if (tr.flFraction == 1.0)
					{
						WorldGraph.m_iLastActiveIdleSearch = nodeNumber + 1; // next monster that searches for hint nodes will start where we left off.
						return nodeNumber;									 // take it!
					}
				}
			}
		}
	}

	WorldGraph.m_iLastActiveIdleSearch = 0; // start at the top of the list for the next search.

	return NO_NODE;
}

//=========================================================
// FValidateHintType - tells use whether or not the monster cares
// about the type of Hint Node given
//=========================================================
bool CBaseMonster::FValidateHintType(short sHint)
{
	return false;
}

//=========================================================
// Ignore conditions - before a set of conditions is allowed
// to interrupt a monster's schedule, this function removes
// conditions that we have flagged to interrupt the current
// schedule, but may not want to interrupt the schedule every
// time. (Pain, for instance)
//=========================================================
int CBaseMonster::IgnoreConditions()
{
	int iIgnoreConditions = 0;

	if (!FShouldEat())
	{
		// not hungry? Ignore food smell.
		iIgnoreConditions |= bits_COND_SMELL_FOOD;
	}

	if (m_MonsterState == MONSTERSTATE_SCRIPT && m_pCine)
		iIgnoreConditions |= m_pCine->IgnoreConditions();

	return iIgnoreConditions;
}

void CBaseMonster::StartPatrol(CBaseEntity* path)
{
	m_pGoalEnt = path;

	if (!m_pGoalEnt)
	{
		ALERT(at_error, "ReadyMonster()--%s couldn't find target \"%s\"\n", STRING(pev->classname), STRING(pev->target));
	}
	else
	{
		// Monster will start turning towards his destination
		//		MakeIdealYaw ( m_pGoalEnt->pev->origin );

		// set the monster up to walk a path corner path.
		// !!!BUGBUG - this is a minor bit of a hack.
		// JAYJAY
		m_movementGoal = MOVEGOAL_PATHCORNER;

		if (pev->movetype == MOVETYPE_FLY)
			m_movementActivity = ACT_FLY;
		else
			m_movementActivity = ACT_WALK;

		if (!FRefreshRoute())
		{
			ALERT(at_aiconsole, "Can't Create Route!\n");
		}
		SetState(MONSTERSTATE_IDLE);
		ChangeSchedule(GetScheduleOfType(SCHED_IDLE_WALK));
	}
}

//=========================================================
// FCanActiveIdle
//=========================================================
bool CBaseMonster::FCanActiveIdle()
{
	/*
	if ( m_MonsterState == MONSTERSTATE_IDLE && m_IdealMonsterState == MONSTERSTATE_IDLE && !IsMoving() )
	{
		return true;
	}
	*/
	return false;
}
