#include "b_local.h"
#include "g_nav.h"
#include "anims.h"

extern void CG_DrawAlert( vec3_t origin, float rating );
extern void G_AddVoiceEvent( gentity_t *self, int event, int speakDebounceTime );

#define	MAX_SUSPECT_COUNT	5

#define	INVESTIGATE_RADIUS		128
#define INVESTIGATE_RADIUS_SQR	( INVESTIGATE_RADIUS * INVESTIGATE_RADIUS )

#define	MAX_LAST_SEEN_DIST		128
#define	MAX_LAST_SEEN_DIST_SQR	( MAX_LAST_SEEN_DIST * MAX_LAST_SEEN_DIST )

#define	MAX_VIEW_DIST		1024
#define MAX_VIEW_SPEED		250
#define	MAX_LIGHT_INTENSITY 255
#define	MIN_LIGHT_THRESHOLD	0.1

#define	DISTANCE_SCALE		0.25f
#define	DISTANCE_THRESHOLD	0.075f
#define	SPEED_SCALE			0.25f
#define	FOV_SCALE			0.5f
#define	LIGHT_SCALE			0.25f

#define	REALIZE_THRESHOLD	0.6f
#define CAUTIOUS_THRESHOLD	( REALIZE_THRESHOLD * 0.75 )

#define	PURSUE_DELAY		2000

qboolean NPC_CheckEnemyStealth( void );
extern qboolean NPC_CheckDisguise( gentity_t *ent );
extern void G_SoundOnEnt( gentity_t *ent, soundChannel_t channel, const char *soundPath );

//Local state enums
enum
{
	LSTATE_NONE = 0,
	LSTATE_UNDERFIRE,
	LSTATE_INVESTIGATE,
};

/*
-------------------------
Scav_Speak
-------------------------
*/

static void Scav_Speak( int ID, float chance )
{
	//Check to see if we pass
	if ( random() < chance )
		return;

	G_AddVoiceEvent( NPC, ID, 0 );
}

/*
-------------------------
Scav_Mark
-------------------------
*/

#if 0

static void Scav_Mark( void )
{
	vec3_t	end;

	VectorCopy( NPC->currentOrigin, end );
	end[2] += 64;

	CG_DrawNode( end, NODE_START );
}

#endif

/*
-------------------------
NPC_Scav_BattleChatter
-------------------------
*/

#if 1

static void NPC_Scav_BattleChatter( void )
{
	if ( TIMER_Done( NPC, "chatter" ) )
	{
		Scav_Speak( Q_irand(EV_CHATTER1, EV_CHATTER14), 0.8f );

		TIMER_Set( NPC, "chatter", Q_irand( 5000, 10000 ) );
	}
}

#endif

/*
-------------------------
NPC_Scav_Pain
-------------------------
*/

void NPC_Scav_Pain( gentity_t *self, gentity_t *other, int damage ) 
{
	if ( Q_stricmp( self->NPC_type, "hirogenalpha" ) != 0 )
	{
		self->NPC->localState = LSTATE_UNDERFIRE;
	
		TIMER_Set( self, "duck", 0 );
		TIMER_Set( self, "stand", 2000 );
	}

	NPC_Pain( self, other, damage );
}

/*
-------------------------
Scav_HoldPosition
-------------------------
*/

static void Scav_HoldPosition( void )
{
	NPCInfo->squadState = SQUAD_STAND_AND_SHOOT;
	NPCInfo->goalEntity = NULL;
	
	if ( TIMER_Done( NPC, "stand" ) )
	{
		TIMER_Set( NPC, "duck", Q_irand( 2000, 4000 ) );
	}
}

/*
-------------------------
Scav_Move
-------------------------
*/

static void Scav_Move( gentity_t *goal )
{
	NPCInfo->combatMove = qtrue;
	NPCInfo->goalEntity = goal;

	qboolean	moved = NPC_MoveToGoal();
	navInfo_t	info;
	
	//Get the move info
	NAV_GetLastMove( info );

	//If we hit our target, then stop and fire!
	if ( ( info.flags & NIF_COLLISION ) && ( info.blocker == NPC->enemy ) )
	{
		Scav_HoldPosition();
	}

	//If our move failed, then reset
	if ( moved == qfalse )
	{
		Scav_HoldPosition();
	}

	NPC_UpdateAngles( qtrue, qtrue );
}

/*
-------------------------
Scav_Flee
-------------------------
*/

static qboolean Scav_Flee( void )
{
	return qfalse;
}

/*
-------------------------
Scav_TakeCover
-------------------------
*/

static qboolean Scav_TakeCover( void )
{
	int cp;

	//Find a cover point
	if ( ( cp = NPC_FindCombatPoint( NPC->currentOrigin, NPC->currentOrigin, NPC->enemy->currentOrigin, (CP_AVOID|CP_AVOID_ENEMY|CP_COVER) ) ) != -1 ) 
	{
		//Reserve it, and then move there
		if ( NPC_SetCombatPoint( cp ) )
		{
			//Setup our retreat information
			NPCInfo->squadState = SQUAD_RETREAT;
			ucmd.buttons |= BUTTON_CAREFUL;
			NPCInfo->combatPoint = cp;
			
			//Yell for cover
			Scav_Speak( EV_CHATTER1, 0.9f );
			
			TIMER_Set( NPC, "chatter", Q_irand( 5000, 10000 ) );
			
			if ( TIMER_Done( NPC, "stand" ) )
			{
				TIMER_Set( NPC, "duck", Q_irand( 3000, 5000 ) );
			}
						
			NPC_SetMoveGoal( NPC, level.combatPoints[cp].origin, 8, qtrue );
			Scav_Move( NPCInfo->goalEntity );

			return qtrue;
		}
	}

	return qfalse;
}

/*
-------------------------
Scav_TakePosition
-------------------------
*/

static qboolean Scav_TakePosition( void )
{
	int cp;

	//Find a cover point
	if ( ( cp = NPC_FindCombatPoint( NPC->currentOrigin, NPC->currentOrigin, NPC->enemy->currentOrigin, (CP_AVOID|CP_AVOID_ENEMY|CP_CLEAR) ) ) != -1 ) 
	{
		//Reserve it, and then move there
		if ( NPC_SetCombatPoint( cp ) )
		{
			//Setup our retreat information
			NPCInfo->squadState = SQUAD_TRANSITION;
			ucmd.buttons |= BUTTON_CAREFUL;
			NPCInfo->combatPoint = cp;
			
			TIMER_Set( NPC, "chatter", Q_irand( 5000, 10000 ) );
						
			NPC_SetMoveGoal( NPC, level.combatPoints[cp].origin, 8, qtrue );
			Scav_Move( NPCInfo->goalEntity );

			return qtrue;
		}
	}

	return qfalse;
}

/*
-------------------------
NPC_Scav_SleepShuffle
-------------------------
*/

static void NPC_Scav_SleepShuffle( void )
{
	//Play an awake script if we have one
	if ( VALIDSTRING( NPC->behaviorSet[BSET_AWAKE] ) )
	{
		G_ActivateBehavior( NPC, BSET_AWAKE);
		return;
	}

	//Automate some movement and noise
	if ( TIMER_Done( NPC, "shuffleTime" ) )
	{
		
		//TODO: Play sleeping shuffle animation

		//int	soundIndex = Q_irand( 0, 1 );

		/*
		switch ( soundIndex )
		{
		case 0:
			G_Sound( NPC, G_SoundIndex("sound/voice/imperialsleeper1/scav4/hunh.mp3") );
			break;

		case 1:
			G_Sound( NPC, G_SoundIndex("sound/voice/imperialsleeper3/scav4/tryingtosleep.wav") );
			break;
		}
		*/

		TIMER_Set( NPC, "shuffleTime", 4000 );
		TIMER_Set( NPC, "sleepTime", 2000 );
		return;
	}

	//They made another noise while we were stirring, see if we can see them
	if ( TIMER_Done( NPC, "sleepTime" ) )
	{
		NPC_CheckEnemyStealth();
		TIMER_Set( NPC, "sleepTime", 2000 );
	}
}

/*
-------------------------
NPC_Scav_Sleep
-------------------------
*/

void NPC_BSScav_Sleep( void )
{
	int alertEvent = NPC_CheckAlertEvents();

	//There is an event to look at
	if ( alertEvent >= 0 )
	{
		//See if it was enough to wake us up
		if ( level.alertEvents[alertEvent].level == AEL_DISCOVERED )
		{
			G_SetEnemy( NPC, &g_entities[0] );
			return;
		}

		//Otherwise just stir a bit
		NPC_Scav_SleepShuffle();
		return;
	}
}


/*
-------------------------
NPC_CheckEnemyStealth
-------------------------
*/

qboolean NPC_CheckEnemyStealth( void )
{
	//NOTENOTE: For now, all stealh checks go against the player, since
	//			he is the main focus.  Squad members and rivals do not
	//			fall into this category and will be ignored.

	gentity_t	*target = &g_entities[0];	//Change this pointer to assess other entities

	//In case we aquired one some other way
	if ( NPC->enemy != NULL )
		return qtrue;

	//Ignore notarget
	if ( target->flags & FL_NOTARGET )
		return qfalse;

	//Don't pick up on a disguised player
	if ( NPC_CheckDisguise( target ) )
		return qfalse;

	//If the target is this close, then wake up regardless
	if ( DistanceSquared( target->currentOrigin, NPC->currentOrigin ) < (32*32) )
	{
		G_SetEnemy( NPC, target );
		return qtrue;
	}

	//Check FOV first
	if ( InFOV( target, NPC, NPCInfo->stats.hfov, NPCInfo->stats.vfov ) == qfalse )
		return qfalse;

	qboolean clearLOS = ( target->client->ps.leanofs ) ? NPC_ClearLOS( target->currentOrigin ) : NPC_ClearLOS( target );

	//Now check for clear line of vision
	if ( clearLOS )
	{
		float	hAngle_perc			= NPC_GetHFOVPercentage( target->currentOrigin, NPC->currentOrigin, NPC->currentAngles, NPCInfo->stats.hfov );
		float	vAngle_perc			= NPC_GetVFOVPercentage( target->currentOrigin, NPC->currentOrigin, NPC->currentAngles, NPCInfo->stats.vfov );

		//Cap them vertically pretty harshly
		vAngle_perc *= ( vAngle_perc * vAngle_perc );
		hAngle_perc *= ( hAngle_perc * hAngle_perc );

		//Cap our vertical vision severely
		if ( vAngle_perc <= 0.5f )
			return qfalse;

		//Assess the player's current status
		float	target_speed		= VectorLength( target->client->ps.velocity );
		float	target_dist			= Distance( target->currentOrigin, NPC->currentOrigin );
		int		target_crouching	= ( target->client->usercmd.upmove == -127 );
		float	dist_rating			= ( target_dist / MAX_VIEW_DIST );
		float	speed_rating		= ( target_speed / MAX_VIEW_SPEED );
		float	light_level			= ( target->lightLevel / MAX_LIGHT_INTENSITY );
		float	FOV_perc			= 1.0f - ( hAngle_perc + vAngle_perc ) * 0.5f;	//FIXME: Dunno about the average...
		
		//Too dark
		if ( light_level < MIN_LIGHT_THRESHOLD )
			return qfalse;

		//Too close?
		if ( dist_rating < DISTANCE_THRESHOLD )
		{
			//G_SetEnemy( NPC, target );
			//return qtrue;
		}

		//Out of range
		if ( dist_rating > 1.0f )
			return qfalse;
		
		//Cap our speed checks
		if ( speed_rating > 1.0f )
			speed_rating = 1.0f;
		
		//...Visibilty linearly wanes over distance
		//...As the percentage out of the FOV increases, straight perception suffers on an exponential scale

		//Calculate the distance and fov influences
		float dist_influence	= DISTANCE_SCALE * ( ( 1.0f - dist_rating ) );
		float fov_influence		= FOV_SCALE * ( 1.0f - FOV_perc );
		
		//Calculate our base rating
		float target_rating		= dist_influence + fov_influence;
		
		//...Lack of light hides, abundance of light exposes

		//Calculate the light influence
		float light_influence	= ( light_level - 0.5f ) * LIGHT_SCALE;

		//Modify our base value by the light's influence
		target_rating += light_influence;

		//...Motion draws the eye quickly
		target_rating += speed_rating * SPEED_SCALE;

		//...Smaller targets are harder to indentify

		//Now award any final bonuses to this number
		if ( target_crouching )
		{
			target_rating *= 0.9f;	//10% bonus
		}	
	
		//If he's violated the threshold, then realize him
		if ( target_rating > REALIZE_THRESHOLD )
		{
			G_SetEnemy( NPC, target );
			return qtrue;
		}

		//If he's above the caution threshold, then realize him in a few seconds unless he moves to cover
		if ( target_rating > CAUTIOUS_THRESHOLD )
		{
			//If we haven't already, start the counter
			if ( TIMER_Done( NPC, "enemyLastVisible" ) )
			{
				//NPCInfo->timeEnemyLastVisible = level.time + 2000;
				TIMER_Set( NPC, "enemyLastVisible", Q_irand( 4000, 8000 ) );
				//TODO: Play a sound along the lines of, "Huh?  What was that?"
			}
			else if ( TIMER_Get( NPC, "enemyLastVisible" ) == level.time )	//FIXME: Is this reliable?
			{
				G_SetEnemy( NPC, target );
				return qtrue;
			}

			return qfalse;
		}
	}

	return qfalse;
}

/*
-------------------------
Scav_NoiseAlert
-------------------------
*/

/*
#define NUM_ALERT_NOISES	8

static const char *noiseAlerts[NUM_ALERT_NOISES] =
{
	"sound/voice/impchessman2/scav4/whatwasthat.mp3",
	"sound/voice/imperialsleeper1/hunh.mp3",
	"sound/voice/impguard4/scav5/heywhat.mp3",
	"sound/voice/klingon1/scav3/what.mp3",
	"sound/voice/klingoneng3/scav2/whatwasthat.mp3",
	"sound/voice/klingoneng4/scav3/what.mp3",
	"sound/voice/klingonguard/scav2/whatsthat.mp3",
	"sound/voice/klingonguard/scav2/whosthere.mp3",
};

static void Scav_NoiseAlert( void )
{
	//G_SoundOnEnt( NPC, CHAN_VOICE_ATTEN, noiseAlerts[Q_irand(0,NUM_ALERT_NOISES-1)] );
	Scav_Speak( EV_CHATTER11, 0.0f );
}

/*
-------------------------
NPC_Scav_InvestigateEvent
-------------------------
*/

#define	MAX_CHECK_THRESHOLD	1

static void NPC_Scav_InvestigateEvent( int eventID, bool extraSuspicious )
{
	//If they've given themselves away, just take them as an enemy
	if ( level.alertEvents[eventID].level == AEL_DISCOVERED )
	{
		G_SetEnemy( NPC, &g_entities[0] );
		return;
	}

/*
	//Must be ready to take another sound event
	if ( NPCInfo->investigateSoundDebounceTime > level.time )
		return;

	//Save the position for movement (if necessary)
	VectorCopy( level.alertEvents[eventID].position, NPCInfo->investigateGoal );

	//Say something
	Scav_NoiseAlert();

	//First awareness of it
	NPCInfo->investigateCount += ( extraSuspicious ) ? 2 : 1;

	//Clamp the value
	if ( NPCInfo->investigateCount > 4 )
		NPCInfo->investigateCount = 4;

	//See if we should walk over and investigate
	/*
	if ( NPCInfo->investigateCount > MAX_CHECK_THRESHOLD )
	{
		int id = NPC_FindCombatPoint( NPCInfo->investigateGoal, NPCInfo->investigateGoal, NPCInfo->investigateGoal, CPF_INVESTIGATE );

		if ( id != -1 )
		{
			NPC_SetMoveGoal( NPC, level.combatPoints[id].origin, 16, qtrue );
			NPCInfo->localState = LSTATE_INVESTIGATE;
		}
	}
	*/

	//Setup the debounce info
/*
	NPCInfo->investigateDebounceTime		= NPCInfo->investigateCount * 5000;
	NPCInfo->investigateSoundDebounceTime	= level.time + 2000;
	NPCInfo->pauseTime						= level.time;
*/

	//Start investigating
	//NPCInfo->tempBehavior = BS_INVESTIGATE;
}

/*
-------------------------
Scav_OffsetLook
-------------------------
*/

static void Scav_OffsetLook( float offset, vec3_t out )
{
	vec3_t	angles, forward, temp;

	GetAnglesForDirection( NPC->currentOrigin, NPCInfo->investigateGoal, angles );
	angles[YAW] += offset;
	AngleVectors( angles, forward, NULL, NULL );
	VectorMA( NPC->currentOrigin, 64, forward, out );
	
	CalcEntitySpot( NPC, SPOT_HEAD, temp );
	out[2] = temp[2];
}

/*
-------------------------
Scav_LookAround
-------------------------
*/

static void Scav_LookAround( void )
{
	vec3_t	lookPos;
	float	perc = (float) ( level.time - NPCInfo->pauseTime ) / (float) NPCInfo->investigateDebounceTime;

	//Keep looking at the spot
	if ( perc < 0.25 )
	{
		VectorCopy( NPCInfo->investigateGoal, lookPos );
	}
	else if ( perc < 0.5f )		//Look up but straight ahead
	{
		Scav_OffsetLook( 0.0f, lookPos );
	}
	else if ( perc < 0.75f )	//Look right
	{
		Scav_OffsetLook( 45.0f, lookPos );
	}
	else	//Look left
	{
		Scav_OffsetLook( -45.0f, lookPos );
	}

	NPC_FacePosition( lookPos );
}

/*
-------------------------
NPC_BSScav_Investigate
-------------------------
*/

void NPC_BSScav_Investigate( void )
{
	//If we're done looking, then just return to what we were doing
	if ( ( NPCInfo->investigateDebounceTime + NPCInfo->pauseTime ) < level.time )
	{
		NPCInfo->tempBehavior = BS_DEFAULT;
		NPCInfo->goalEntity = UpdateGoal();
		
		NPC_UpdateAngles( qtrue, qtrue );
		return;
	}

	//See if we're searching for the noise's origin
	if ( NPCInfo->localState == LSTATE_INVESTIGATE )
	{
		//See if we're there
		if ( NAV_HitNavGoal( NPC->currentOrigin, NPC->mins, NPC->maxs, NPCInfo->goalEntity->currentOrigin, 32 ) == qfalse )
		{
			ucmd.buttons |= (BUTTON_CAREFUL|BUTTON_WALKING);

			//Try and move there
			if ( NPC_MoveToGoal()  )
			{
				//Bump our times
				NPCInfo->investigateDebounceTime	= NPCInfo->investigateCount * 5000;
				NPCInfo->pauseTime					= level.time;

				NPC_UpdateAngles( qtrue, qtrue );
				return;
			}
		}

		//Otherwise we're done or have given up
		NPCInfo->localState = LSTATE_NONE;
	}

	//Look around
	Scav_LookAround();

	//Look for an enemy
	if ( NPC_CheckEnemyStealth() )
	{
		NPCInfo->behaviorState	= BS_RUN_AND_SHOOT;
		NPCInfo->tempBehavior	= BS_DEFAULT;
		return;
	}
}

/*
-------------------------
NPC_BSScav_Patrol
-------------------------
*/

void NPC_BSScav_Patrol( void )
{
	//Look for any enemies
	if ( NPC_CheckEnemyStealth() )
	{
		NPCInfo->behaviorState = BS_RUN_AND_SHOOT;
		NPC_AngerSound();
		return;
	}

	int alertEvent = NPC_CheckAlertEvents();

	//There is an event to look at
	if ( alertEvent >= 0 )
	{
		NPC_Scav_InvestigateEvent( alertEvent, qfalse );
		return;
	}

	//If we have somewhere to go, then do that
	if ( UpdateGoal() )
	{
		ucmd.buttons |= BUTTON_WALKING;
		//Scav_Move( NPCInfo->goalEntity );
		NPC_MoveToGoal();
	}

	NPC_UpdateAngles( qtrue, qtrue );
}

/*
-------------------------
NPC_BSScav_Idle
-------------------------
*/

void NPC_BSScav_Idle( void )
{
	int alertEvent = NPC_CheckAlertEvents();

	//There is an event to look at
	if ( alertEvent >= 0 )
	{
		NPC_Scav_InvestigateEvent( alertEvent, qfalse );
		return;
	}

	TIMER_Set( NPC, "roamTime", 2000 + Q_irand( 1000, 2000 ) );
}

/*
-------------------------
Scav_CheckMoveState
-------------------------
*/

static qboolean Scav_CheckMoveState( void )
{
	//See if we're a scout
	if ( NPCInfo->squadState == SQUAD_SCOUT )
	{
		//If we're supposed to stay put, then crouch and fire
		if ( TIMER_Done( NPC, "stick" ) == qfalse )
		{
			NPC_SetAnim( NPC, SETANIM_TORSO, BOTH_STAND2, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD );
			ucmd.upmove = -128;

			//Check to fire
			if ( NPC_CheckCanAttackExt() )
			{
				WeaponThink( qtrue );
			}

			return qtrue;
		}

		//Otherwise, if we can see our target, just shoot
		if ( ( NPC_ClearLOS( NPC->enemy ) ) && ( NPC_ClearShot( NPC->enemy ) ) )
		{
			NPCInfo->squadState = SQUAD_STAND_AND_SHOOT;
			return qfalse;
		}

		//Move to find our target
		Scav_Move( NPC->enemy );
		
		return qtrue;
	}

	//See if we're moving towards a goal
	if ( ( NPCInfo->goalEntity != NPC->enemy ) && ( NPCInfo->goalEntity != NULL ) )
	{
		//Did we make it?
		if ( NAV_HitNavGoal( NPC->currentOrigin, NPC->mins, NPC->maxs, NPCInfo->goalEntity->currentOrigin, 16 ) )
		{
			TIMER_Set( NPC, "attackDelay", Q_irand( 250, 500 ) );	//FIXME: Slant for difficulty levels
			NPCInfo->squadState = ( NPCInfo->squadState == SQUAD_RETREAT ) ? SQUAD_COVER : SQUAD_STAND_AND_SHOOT;
			NPCInfo->goalEntity = NULL;
			return qfalse;
		}

		//Keep running
		NPCInfo->squadState = SQUAD_RETREAT;
		ucmd.buttons |= BUTTON_CAREFUL;
		
		Scav_Move( NPCInfo->goalEntity );
		TIMER_Set( NPC, "roamTime", Q_irand( 4000, 8000 ) );

		return qtrue;
	}

	//See if we should be ducking
	if ( TIMER_Done( NPC, "duck" ) == qfalse )
	{
		NPC_SetAnim( NPC, SETANIM_TORSO, BOTH_STAND2, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD );
		ucmd.upmove = -128;

		//Check to fire
		if ( NPC_CheckCanAttackExt() )
		{
			WeaponThink( qtrue );
		}
		
		return qtrue;
	}

	return qfalse;
}

/*
-------------------------
Scav_CheckLocalState
-------------------------
*/

static qboolean Scav_CheckLocalState( AIGroupInfo_t	&group )
{
	int flee = 0;

	switch ( NPCInfo->localState )
	{
	case LSTATE_UNDERFIRE:	//Under attack
		
		flee = ( ( group.numGroup ) && ( group.numFront == 0 ) && ( group.numState[SQUAD_POINT] == 0 ) ) ? qtrue : ( Q_irand( 0, 16 ) == 0 );

		if ( flee )
		{
			//Make sure we're the only one doing this, or else we can have big navigational problems
			if ( ( group.numGroup > 1 ) && ( group.numState[ SQUAD_TRANSITION ] > 2 ) && ( group.numState[ SQUAD_RETREAT ] > 2 ) )
				return qfalse;

			//Try and flee
			if ( Scav_Flee() == qfalse )
			{
				//Try to at least find cover
				if ( Scav_TakeCover() == qfalse )
					return qfalse;
			}
		}

		NPCInfo->localState = LSTATE_NONE;
		NPC_UpdateAngles( qtrue, qtrue );
		return qtrue;
		break;

	default:
		break;
	}

	return qfalse;
}

/*
-------------------------
Scav_CheckSquadState
-------------------------
*/

static qboolean Scav_CheckSquadState( AIGroupInfo_t	&group )
{
	//If we're at point, fire away
	if ( NPCInfo->squadState == SQUAD_POINT )
	{
		if ( TIMER_Done( NPC, "stuck" ) )
		{
			NPCInfo->squadState = SQUAD_STAND_AND_SHOOT;
			return qfalse;
		}

		//Crouch and fire
		NPC_SetAnim( NPC, SETANIM_TORSO, BOTH_STAND2, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD );
		ucmd.upmove = -128;

		//Check to fire
		if ( NPC_CheckCanAttackExt() )
		{
			WeaponThink( qtrue );
		}
		
		return qtrue;
	}

	//If we're point, then get down
	if ( ( group.numGroup ) && ( group.numFront == 0 ) && ( group.numState[SQUAD_POINT] == 0 ) )
	{
		NPCInfo->squadState = SQUAD_POINT;
		ucmd.upmove = -128;
		NPC_SetAnim( NPC, SETANIM_TORSO, BOTH_STAND2, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD );
		TIMER_Set( NPC, "duck", Q_irand( 3000, 4000 ) );

		//Check to fire
		if ( NPC_CheckCanAttackExt() )
		{
			WeaponThink( qtrue );
		}

		return qtrue;

	}

	return qfalse;
}

/*
-------------------------
Scav_Hunt
-------------------------
*/

static void Scav_Hunt( AIGroupInfo_t &group )
{
	//If we can see our enemy and hit them...
	if ( ( NPC_ClearLOS( NPC->enemy ) ) && ( NPC_ClearShot( NPC->enemy ) ) )
	{
		//See if we should run to a new spot
		if ( TIMER_Done( NPC, "roamTime" ) )
		{
			TIMER_Set( NPC, "roamTime", Q_irand( 4000, 8000 ) );

			if ( ( Q_irand( 0, 5 ) == 0 ) && ( group.numState[ SQUAD_COVER ] <= 1 ) )
			{
				if ( Scav_TakeCover() )
					return;
			}
			else
			{
				if ( Scav_TakePosition() )
					return;
			}
		}

		//See if we're done waiting to fire
		if ( TIMER_Done( NPC, "attackDelay" ) )
		{
			//See if we're able to attack
			if ( NPC_CheckCanAttackExt() )
			{
				//Do random battle chatter
				NPC_Scav_BattleChatter();

				WeaponThink( qtrue );
			}
		}

		//Update our seen enemy position
		VectorCopy( NPC->enemy->currentOrigin, NPCInfo->enemyLastSeenLocation );
		NPCInfo->enemyLastSeenTime = level.time;

		NPC_FaceEnemy( qtrue );
		TIMER_Set( NPC, "stick", Q_irand( 2000, 4000 ) );
		return;
	}

	//See if we should continue to fire on their last position
	if ( ( TIMER_Done( NPC, "stick" ) == qfalse ) && ( group.numState[ SQUAD_SCOUT ] ) )
	{
		//Fire on the last known position
		vec3_t	dir, angles;

		VectorSubtract( NPCInfo->enemyLastSeenLocation, NPC->currentOrigin, dir );

		VectorNormalize( dir );

		vectoangles( dir, angles );

		NPCInfo->desiredYaw		= angles[YAW];
		NPCInfo->desiredPitch	= angles[PITCH];
		NPC_UpdateAngles( qtrue, qtrue );

		WeaponThink( qtrue );
			
		NPCInfo->squadState = SQUAD_STAND_AND_SHOOT;

		return;
	}

	//Run after the player
	if ( group.numState[ SQUAD_SCOUT ] == 0 )
	{
		NPCInfo->squadState = SQUAD_SCOUT;

		Scav_Move( NPC->enemy );
		
		TIMER_Set( NPC, "roamTime", Q_irand( 2000, 4000 ) );
		TIMER_Set( NPC, "attackDelay", Q_irand( 250, 500 ) );
		TIMER_Set( NPC, "duck", 0 );
		TIMER_Set( NPC, "stick", Q_irand( 2000, 4000 ) );
		
		NPC_FreeCombatPoint( NPCInfo->combatPoint );

		return;
	}

	//Face the enemy
	NPC_FaceEnemy( qtrue );

	//Check to fire
	if ( NPC_CheckCanAttackExt() )
	{
		WeaponThink( qtrue );
	}

	//FIXME: Temp stuff
	NPC_UpdateAngles( qtrue, qtrue );
}
/*
-------------------------
NPC_BSScav_Attack
-------------------------
*/

void NPC_BSScav_Attack( void )
{
	//Don't do anything if we're hurt
	if ( NPC->painDebounceTime > level.time )
	{
		NPC_UpdateAngles( qtrue, qtrue );
		return;
	}

	//If we don't have an enemy, just idle
	if ( NPC_CheckEnemyExt() == qfalse )
	{
		NPC_BSScav_Idle();
		return;
	}

	NPCInfo->combatMove = qtrue;

	//Get our group info
	AIGroupInfo_t	group;
	AI_GetGroup( group, NPC->currentOrigin, NPC->currentAngles, 45, 512, NPC->client->playerTeam, NPC, NPC->enemy );

	//Check for local events to take care of
	if ( Scav_CheckLocalState( group ) )
		return;

	//Check for movement to take care of
	if ( Scav_CheckMoveState() )
		return;

	//Check our squad's status
	if ( Scav_CheckSquadState( group ) )
		return;

	//Track the player and kill them if possible
	Scav_Hunt( group );
}
