#include "b_local.h"
#include "anims.h"
#include "g_nav.h"
#include "g_navigator.h"

extern CNavigator	navigator;
extern void G_SoundOnEnt( gentity_t *ent, soundChannel_t channel, const char *soundPath );

#define	NUM_ALPHA_SOUNDS	6

const char *alphaSpeech[NUM_ALPHA_SOUNDS] =
{
	"sound/voice/hirogen/scavboss/yourskull.mp3",
	"sound/voice/hirogen/scavboss/yourcomrade.mp3",
	"sound/voice/hirogen/scavboss/cleverprey.mp3",
	"sound/voice/hirogen/scavboss/iappreciate.mp3",
	"sound/voice/hirogen/scavboss/surrendernow.mp3",
	"sound/weapons/guncharge.wav",
};

/*
-------------------------
Hirogen_ActivateShield
-------------------------
*/

void Hirogen_ActivateShield( qboolean turnOn )
{
	if ( turnOn )
	{
		NPC->s.powerups |= ( 1 << PW_HIROGEN_SHIELD );
		NPC->client->ps.powerups[ PW_HIROGEN_SHIELD ] = level.time + 10000;
	}
	else
	{
		NPC->s.powerups &= ~( 1 << PW_HIROGEN_SHIELD );
		NPC->client->ps.powerups[ PW_HIROGEN_SHIELD ] = 0;
	}
}

/*
-------------------------
Hirogen_Hunt
-------------------------
*/

void Hirogen_Hunt( void )
{
	if ( NPCInfo->combatPoint == -1 )
	{
		NPCInfo->combatPoint = NPC_FindCombatPoint( CP_CLEAR );

		if ( NPCInfo->combatPoint == -1 )
		{
			assert(0);
			return;
		}

		//Move there
		NPC_SetMoveGoal( NPC, level.combatPoints[NPCInfo->combatPoint].origin, 4, qtrue );

		NPCInfo->combatMove = qtrue;

		NPC_MoveToGoal();
		NPC_UpdateAngles( qtrue, qtrue );

		return;
	}

	//See if we made it
	if ( NAV_HitNavGoal( NPC->currentOrigin, NPC->mins, NPC->maxs, level.combatPoints[NPCInfo->combatPoint].origin, 2 ) )
	{
		NPCInfo->combatPoint = -1;
		//Hirogen_Hunt();	//NOTENOTE: Remove the 10Hz latency that would be introduced otherwise, but be careful with this one!
		return;
	}

	//Move there
	NPC_SetMoveGoal( NPC, level.combatPoints[NPCInfo->combatPoint].origin, 2, qtrue );

	NPCInfo->combatMove = qtrue;

	NPC_MoveToGoal();
	NPC_UpdateAngles( qtrue, qtrue );
}

/*
-------------------------
Hirogen_PauseState
-------------------------
*/

void Hirogen_PauseState( void )
{
	//See if we're done pre-delaying
	if ( NPCInfo->pauseTime < level.time )
	{
		//See if we're still attacking
		if ( NPCInfo->weaponTime > level.time )
		{
			ucmd.buttons |= BUTTON_ATTACK;
			NPC_FaceEnemy();
			NPC_UpdateAngles( qtrue, qtrue );

			return;
		}

		//Otherwise we're reloading
		if ( NPCInfo->jumpTime < level.time )
		{
			NPC_SetAnim( NPC, SETANIM_BOTH, BOTH_ATTACK3, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD );
			NPCInfo->jumpTime = level.time + 5000;
			Hirogen_ActivateShield( qfalse );
			NPC_UpdateAngles( qtrue, qtrue );

			return;
		}

		NPC_FaceEnemy();
		NPC_UpdateAngles( qtrue, qtrue );
		
		return;
	}

	//Turn to face our enemy
	if ( NPC_FaceEnemy() == qfalse )
	{
		//If we aren't facing yet, then keep turning
		//FIXME: This could potentially be bad...
		NPCInfo->pauseTime += FRAMETIME;
		NPCInfo->weaponTime += FRAMETIME;
		NPCInfo->standTime += FRAMETIME;
	}

	NPC_UpdateAngles( qtrue, qtrue );
}

/*
-------------------------
HirogenAlpha_Precache
-------------------------
*/

void HirogenAlpha_Precache( void )
{
	for ( int i = 0; i < NUM_ALPHA_SOUNDS; i++ )
	{
		G_SoundIndex( (char *) alphaSpeech[i] );
	}	
}

/*
-------------------------
Hirogen_Speak
-------------------------
*/

static qboolean Hirogen_Speak( void )
{
	//Make sure we haven't already tried to say the line
	if ( NPCInfo->squadState )
		return qfalse;

	switch ( NPCInfo->localState )
	{
	case 0:
		G_SoundOnEnt( NPC, CHAN_VOICE, alphaSpeech[0] );
		TIMER_Set( NPC, "speech", 2500 );
		break;

	case 1:
		G_SoundOnEnt( NPC, CHAN_VOICE, alphaSpeech[1] );
		TIMER_Set( NPC, "speech", 2500 );
		break;

	case 3:
		G_SoundOnEnt( NPC, CHAN_VOICE, alphaSpeech[2] );
		TIMER_Set( NPC, "speech", 2500 );
		break;

	case 5:
		G_SoundOnEnt( NPC, CHAN_VOICE, alphaSpeech[3] );
		TIMER_Set( NPC, "speech", 2500 );
		break;

	case 7:
		G_SoundOnEnt( NPC, CHAN_VOICE, alphaSpeech[4] );
		TIMER_Set( NPC, "speech", 2500 );
		break;

	default:
		return qfalse;
		break;
	}

	NPCInfo->squadState = 1;
	return qtrue;
}

//NOTENOTE: standTime	= time standing still while weapon is being fired
//NOTENOTE: weaponTime	= time to hold firing
	
/*
-------------------------
NPC_BSHirogenAlpha_Attack
-------------------------
*/

#define HIROGEN_PREFIRE_DELAY	1000
#define HIROGEN_FIRE_DURATION	3000
#define HIROGEN_RELOAD_PAUSE	3800

void NPC_BSHirogenAlpha_Attack( void )
{
	//FIXME: Move to spawn location
	NPC->flags |= FL_NO_KNOCKBACK;

	if ( TIMER_Done( NPC, "speech" ) == qfalse )
	{
		NPC_FaceEnemy( qtrue );
		return;
	}

	//Do any taunts if need-be
	if ( Hirogen_Speak() )
		return;

	//Check for the enemy (formality)
	if ( NPC_CheckEnemyExt() == qfalse )
	{
		Hirogen_ActivateShield( qtrue );
		NPC_BSHirogenAlpha_Idle();
		return;
	}

	//See if we're currently attacking the player
	if ( NPCInfo->standTime > level.time )
	{
		Hirogen_PauseState();
		return;
	}

	//Turn our shielding on
	Hirogen_ActivateShield( qtrue );

	//See if the target is visible
	if ( NPC_ClearLOS( NPC->enemy ) == qfalse )
	{
		Hirogen_Hunt();

		//This is done to keep the enemy from stopping too soon
		if ( DistanceSquared( NPC->currentOrigin, NPCInfo->tempGoal->currentOrigin ) > 96 )
		{
			NPC_UpdateAngles( qtrue, qtrue );
			return;
		}
	}

	//Must be at our goal
	if ( DistanceSquared( NPC->currentOrigin, NPCInfo->tempGoal->currentOrigin ) > 96 )
	{
		Hirogen_Hunt();
		return;
	}

	//Setup our timing information
	NPCInfo->pauseTime	= level.time + HIROGEN_PREFIRE_DELAY;
	NPCInfo->weaponTime = NPCInfo->pauseTime + HIROGEN_FIRE_DURATION;
	NPCInfo->standTime	= NPCInfo->weaponTime + HIROGEN_RELOAD_PAUSE;
	
	G_SoundOnEnt( NPC, CHAN_WEAPON, alphaSpeech[5] );

	NPC_UpdateAngles( qtrue, qtrue );
}

/*
-------------------------
NPC_BSHirogenAlpha_Idle
-------------------------
*/

void NPC_BSHirogenAlpha_Idle( void )
{
	NPC_BSIdle();
}
