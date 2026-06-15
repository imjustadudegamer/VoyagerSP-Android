// define GAME_INCLUDE so that g_public.h does not define the
// short, server-visible gclient_t and gentity_t structures,
// because we define the full size ones in this file
#define GAME_INCLUDE
#include "q_shared.h"
#include "g_shared.h"
#include "bg_local.h"
#include "anims.h"
#include "Q3_Interface.h"

extern pmove_t		*pm;
extern pml_t		pml;
extern	cvar_t	*g_ICARUSDebug;

extern qboolean ValidAnimFileIndex ( int index );
animFileSet_t	knownAnimFileSets[MAX_ANIM_FILES];

/*
-------------------------
PM_TorsoAnimForFrame
Returns animNumber for current frame
-------------------------
*/
int PM_TorsoAnimForFrame( gentity_t *ent, int torsoFrame )
{
	//Must be a valid client
	if ( ent->client == NULL )
		return -1;

	//Must have a file index entry
	if( ValidAnimFileIndex( ent->client->clientInfo.animFileIndex ) == qfalse )
		return -1;

	animation_t *animations = knownAnimFileSets[ent->client->clientInfo.animFileIndex].animations;

	for ( int animation = 0; animation < LEGS_WALKBACK1; animation++ )
	{
		if ( animations[animation].firstFrame > torsoFrame )
		{//This anim starts after this frame
			continue;
		}

		if ( animations[animation].firstFrame + animations[animation].numFrames < torsoFrame )
		{//This anim ends before this frame
			continue;
		}
		//else, must be in this anim!
		return animation;
	}

	//Not in ANY torsoAnim?  SHOULD NEVER HAPPEN
	assert(0);
	return -1;
}
/*
-------------------------
PM_HasAnimation
-------------------------
*/

qboolean PM_HasAnimation( gentity_t *ent, int animation )
{
	//Must be a valid client
	if ( ent->client == NULL )
		return qfalse;

	//Must have a file index entry
	if( ValidAnimFileIndex( ent->client->clientInfo.animFileIndex ) == qfalse )
		return qfalse;

	animation_t *animations = knownAnimFileSets[ent->client->clientInfo.animFileIndex].animations;

	//No frames, no anim
	if ( animations[animation].numFrames == 0 )
		return qfalse;

	//Has the sequence
	return qtrue;
}

int PM_PickAnim( gentity_t *self, int minAnim, int maxAnim )
{
	int anim;
	int count = 0;

	do
	{
		anim = Q_irand(minAnim, maxAnim);
		count++;
	}
	while ( !PM_HasAnimation( self, anim ) && count < 1000 );

	return anim;
}

/*
-------------------------
PM_AnimLength
-------------------------
*/

int PM_AnimLength( int index, animNumber_t anim )
{
	if ( ValidAnimFileIndex( index ) == false )
		return 0;

	return knownAnimFileSets[index].animations[anim].numFrames * fabs(knownAnimFileSets[index].animations[anim].frameLerp);
}

/*
-------------------------
PM_SetLegsAnimTimer
-------------------------
*/

void PM_SetLegsAnimTimer( gentity_t *ent, int *legsAnimTimer, int time )
{
	*legsAnimTimer = time;

	if ( *legsAnimTimer < 0 && time != -1 )
	{//Cap timer to 0 if was counting down, but let it be -1 if that was intentional
		*legsAnimTimer = 0;
	}

	if ( !*legsAnimTimer && ent && Q3_TaskIDPending( ent, TID_ANIM_LOWER ) )
	{//Waiting for legsAnimTimer to complete, and it just got set to zero
		if ( !Q3_TaskIDPending( ent, TID_ANIM_BOTH) )
		{//Not waiting for top
			Q3_TaskIDComplete( ent, TID_ANIM_LOWER );
		}
		else 
		{//Waiting for both to finish before complete 
			Q3_TaskIDClear( &ent->taskID[TID_ANIM_LOWER] );//Bottom is done, regardless
			if ( !Q3_TaskIDPending( ent, TID_ANIM_UPPER) )
			{//top is done and we're done
				Q3_TaskIDComplete( ent, TID_ANIM_BOTH );
			}
		}
	}
}

/*
-------------------------
PM_SetTorsoAnimTimer
-------------------------
*/

void PM_SetTorsoAnimTimer( gentity_t *ent, int *torsoAnimTimer, int time )
{
	*torsoAnimTimer = time;

	if ( *torsoAnimTimer < 0 && time != -1 )
	{//Cap timer to 0 if was counting down, but let it be -1 if that was intentional
		*torsoAnimTimer = 0;
	}

	if ( !*torsoAnimTimer && ent && Q3_TaskIDPending( ent, TID_ANIM_UPPER ) )
	{//Waiting for torsoAnimTimer to complete, and it just got set to zero
		if ( !Q3_TaskIDPending( ent, TID_ANIM_BOTH) )
		{//Not waiting for bottom
			Q3_TaskIDComplete( ent, TID_ANIM_UPPER );
		}
		else 
		{//Waiting for both to finish before complete 
			Q3_TaskIDClear( &ent->taskID[TID_ANIM_UPPER] );//Top is done, regardless
			if ( !Q3_TaskIDPending( ent, TID_ANIM_LOWER) )
			{//lower is done and we're done
				Q3_TaskIDComplete( ent, TID_ANIM_BOTH );
			}
		}
	}
}

/*
-------------------------
PM_SetAnimFinal
-------------------------
*/

void PM_SetAnimFinal(int *torsoAnim,int *legsAnim,int setAnimParts,int anim,int setAnimFlags,int *torsoAnimTimer,int *legsAnimTimer,gentity_t *gent)
{
	if(!ValidAnimFileIndex(gent->client->clientInfo.animFileIndex))
	{
		return;
	}
	animation_t *animations = knownAnimFileSets[gent->client->clientInfo.animFileIndex].animations;

	// Set torso anim
	if (setAnimParts & SETANIM_TORSO)
	{
		// Don't reset if it's already running the anim
		if( !(setAnimFlags & SETANIM_FLAG_RESTART) && (*torsoAnim & ~ANIM_TOGGLEBIT ) == anim )
		{
			goto setAnimLegs;
		}
		// or if a more important anim is running
		if( !(setAnimFlags & SETANIM_FLAG_OVERRIDE) && ((*torsoAnimTimer > 0)||(*torsoAnimTimer == -1)) )
		{	
			goto setAnimLegs;
		}

		if ( !PM_HasAnimation( gent, anim ) )
		{
			if ( g_ICARUSDebug->integer >= 3  )
			{
				gi.Printf(S_COLOR_RED"SET_ANIM_UPPER ERROR: anim %s does not exist in this model (%s)!\n", animTable[anim].name, ((gent!=NULL&&gent->client!=NULL) ? gent->client->renderInfo.torsoModelName : "unknown") );
			}
			goto setAnimLegs;
		}

		*torsoAnim = ( ( *torsoAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT ) | anim;
/*
#ifdef _DEBUG
		gi.Printf(S_COLOR_GREEN"SET_ANIM_UPPER: %s (%s)\n", animTable[anim].name, gent->targetname );
#endif
*/
		if ((gent->client) && (setAnimFlags & SETANIM_FLAG_HOLD))
		{//FIXME: allow to set a specific time?
			PM_SetTorsoAnimTimer( gent, torsoAnimTimer, animations[anim].numFrames * fabs(animations[anim].frameLerp) );
		}
	}

setAnimLegs:
	// Set legs anim
	if (setAnimParts & SETANIM_LEGS)
	{
		// Don't reset if it's already running the anim
		if( !(setAnimFlags & SETANIM_FLAG_RESTART) && (*legsAnim & ~ANIM_TOGGLEBIT ) == anim )
		{
			goto setAnimDone;
		}
		// or if a more important anim is running
		if( !(setAnimFlags & SETANIM_FLAG_OVERRIDE) && ((*legsAnimTimer > 0)||(*legsAnimTimer == -1)) )
		{	
			goto setAnimDone;
		}

		if ( !PM_HasAnimation( gent, anim ) )
		{
			if ( g_ICARUSDebug->integer >= 3 )
			{
				gi.Printf(S_COLOR_RED"SET_ANIM_LOWER ERROR: anim %s does not exist in this model (%s)!\n", animTable[anim].name, ((gent!=NULL&&gent->client!=NULL) ? gent->client->renderInfo.legsModelName : "unknown") );
			}
			goto setAnimDone;
		}

		*legsAnim = ( ( *legsAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT ) | anim;
/*
#ifdef _DEBUG		
		gi.Printf(S_COLOR_GREEN"SET_ANIM_LOWER: %s (%s)\n", animTable[anim].name, gent->targetname );
#endif
*/
		if ((gent->client) && (setAnimFlags & SETANIM_FLAG_HOLD))
		{//FIXME: allow to set a specific time?
			PM_SetLegsAnimTimer( gent, legsAnimTimer, animations[anim].numFrames * fabs(animations[anim].frameLerp) );
		}
	}

setAnimDone:
	return;
}

void PM_SetAnim(pmove_t	*pm,int setAnimParts,int anim,int setAnimFlags)
{	// FIXME : once torsoAnim and legsAnim are in the same structure for NPC and Players
	// rename PM_SetAnimFinal to PM_SetAnim and have both NPC and Players call PM_SetAnim

	if ( pm->ps->pm_type >= PM_DEAD ) 
	{//FIXME: sometimes we'll want to set anims when your dead... twitches, impacts, etc.
		return;
	}

	if ( pm->gent == NULL ) 
	{
		return;
	}

	if (setAnimFlags&SETANIM_FLAG_OVERRIDE)
	{
//		pm->ps->animationTimer = 0;
		
		if (setAnimParts & SETANIM_TORSO)
		{
			if( (setAnimFlags & SETANIM_FLAG_RESTART) || (pm->ps->torsoAnim & ~ANIM_TOGGLEBIT ) != anim )
			{
				PM_SetTorsoAnimTimer( pm->gent, &pm->ps->torsoAnimTimer, 0 );
			}
		}
		if (setAnimParts & SETANIM_LEGS)
		{
			if( (setAnimFlags & SETANIM_FLAG_RESTART) || (pm->ps->legsAnim & ~ANIM_TOGGLEBIT ) != anim )
			{
				PM_SetLegsAnimTimer( pm->gent, &pm->ps->legsAnimTimer, 0 );
			}
		}
	}

	PM_SetAnimFinal(&pm->ps->torsoAnim,&pm->ps->legsAnim,setAnimParts,anim,setAnimFlags,&pm->ps->torsoAnimTimer,&pm->ps->legsAnimTimer,pm->gent);
}

/*
-------------------------
PM_TorsoAnimBorg
-------------------------
*/

void PM_TorsoAnimBorg( void )
{
	if((pm->ps->legsAnim & ~ANIM_TOGGLEBIT) == BOTH_RUN1 )
	{
		PM_SetAnim(pm,SETANIM_TORSO,BOTH_RUN1,SETANIM_FLAG_NORMAL);
	}
/*	else if((pm->ps->legsAnim & ~ANIM_TOGGLEBIT) == BOTH_RUN2 )
	{
		PM_SetAnim(pm,SETANIM_TORSO,BOTH_RUN2,SETANIM_FLAG_NORMAL);
	}
*/
	else if((pm->ps->legsAnim & ~ANIM_TOGGLEBIT) == BOTH_WALK1 )
	{
		PM_SetAnim(pm,SETANIM_TORSO,BOTH_WALK1,SETANIM_FLAG_NORMAL);
	}
/*
	else if((pm->ps->legsAnim & ~ANIM_TOGGLEBIT) == BOTH_WALK2 )
	{
		PM_SetAnim(pm,SETANIM_TORSO,BOTH_WALK2,SETANIM_FLAG_NORMAL);
	}
*/
	else if((pm->ps->legsAnim & ~ANIM_TOGGLEBIT) == BOTH_STAND1 )
	{
		PM_SetAnim(pm,SETANIM_TORSO,BOTH_STAND1,SETANIM_FLAG_NORMAL);
	}
/*
	else if((pm->ps->legsAnim & ~ANIM_TOGGLEBIT) == BOTH_STAND2 )
	{
		PM_SetAnim(pm,SETANIM_TORSO,BOTH_STAND2,SETANIM_FLAG_NORMAL);
	}
*/
}

/*
-------------------------
PM_TorsoAnimation
-------------------------
*/

void PM_TorsoAnimation( void ) 
{//FIXME: Write a much smarter and more appropriate anim picking routine logic...
//	int	oldAnim;
	if(pm->gent != NULL && pm->gent->client)
	{
		pm->gent->client->renderInfo.torsoFpsMod = 1.0f;
	}

	if ( pm->ps->weapon == WP_NONE || pm->ps->weapon == WP_TRICORDER || pm->ps->weapon == WP_RED_HYPO || pm->ps->weapon == WP_BLUE_HYPO || pm->ps->weapon == WP_IMPERIAL_BLADE || pm->ps->weapon == WP_KLINGON_BLADE || 
		pm->ps->weaponstate == WEAPON_READY || pm->ps->weaponstate == WEAPON_CHARGING)
	{
		if((pm->ps->legsAnim & ~ANIM_TOGGLEBIT) == BOTH_RUN1 )
		{
			PM_SetAnim(pm,SETANIM_TORSO,BOTH_RUN1,SETANIM_FLAG_NORMAL);
		}
		else if((pm->ps->legsAnim & ~ANIM_TOGGLEBIT) == BOTH_RUN2 )
		{
			PM_SetAnim(pm,SETANIM_TORSO,BOTH_RUN2,SETANIM_FLAG_NORMAL);
		}
		else if((pm->ps->legsAnim & ~ANIM_TOGGLEBIT) == BOTH_WALK1 )
		{
			PM_SetAnim(pm,SETANIM_TORSO,BOTH_WALK1,SETANIM_FLAG_NORMAL);
		}
		else if((pm->ps->legsAnim & ~ANIM_TOGGLEBIT) == BOTH_WALK2 )
		{
			PM_SetAnim(pm,SETANIM_TORSO,BOTH_WALK2,SETANIM_FLAG_NORMAL);
		}
		else if((pm->ps->legsAnim & ~ANIM_TOGGLEBIT) == BOTH_CROUCH1IDLE && pm->ps->clientNum != 0 &&//player falls through
			pm->ps->weapon != WP_PALADIN )//crossbow guy falls through
		{
			//??? Why nothing?  What if you were running???
			//PM_SetAnim(pm,SETANIM_TORSO,BOTH_CROUCH1IDLE,SETANIM_FLAG_NORMAL);
		}
		else if((pm->ps->legsAnim & ~ANIM_TOGGLEBIT) == BOTH_JUMP1 )
		{
			PM_SetAnim(pm,SETANIM_TORSO,BOTH_JUMP1,SETANIM_FLAG_NORMAL);
		}
		else
		{//Used to default to both_stand1 which is an arms-down anim
			if(pm->gent != NULL && pm->gent->s.number == 0)
			{//PLayer- temp hack for weapon frame
				if ( pm->ps->weapon == WP_TRICORDER )
				{
					PM_SetAnim(pm,SETANIM_TORSO,TORSO_TRICORDER1,SETANIM_FLAG_NORMAL);
				}
				else
				{
					PM_SetAnim(pm,SETANIM_TORSO,BOTH_STAND1,SETANIM_FLAG_NORMAL);
				}
			}
			else
			{
				switch(pm->ps->weapon)
				{
				case WP_PHASER:
					PM_SetAnim(pm,SETANIM_TORSO,TORSO_WEAPONREADY1,SETANIM_FLAG_NORMAL);
					break;
				case WP_COMPRESSION_RIFLE:
					PM_SetAnim(pm,SETANIM_TORSO,TORSO_WEAPONREADY2,SETANIM_FLAG_NORMAL);
					break;
				case WP_BORG_ASSIMILATOR:
				case WP_BORG_DRILL:
				case WP_BORG_WEAPON:
				case WP_BORG_TASER:
					PM_SetAnim(pm,SETANIM_TORSO,BOTH_STAND1,SETANIM_FLAG_NORMAL);
					break;
				case WP_TRICORDER:
				case WP_RED_HYPO:
				case WP_BLUE_HYPO:
				case WP_VOYAGER_HYPO:
					PM_SetAnim(pm,SETANIM_TORSO,BOTH_STAND1,SETANIM_FLAG_NORMAL);
					break;
				case WP_NONE:
					PM_SetAnim(pm,SETANIM_TORSO,BOTH_STAND1,SETANIM_FLAG_NORMAL);
					break;
				case WP_MELEE:
					PM_SetAnim(pm,SETANIM_TORSO,BOTH_STAND1,SETANIM_FLAG_NORMAL);
					break;
				case WP_STASIS_ATTACK:
					PM_SetAnim(pm,SETANIM_BOTH,BOTH_STAND1,SETANIM_FLAG_NORMAL);
					break;
				case WP_FORGE_PROJ:
					PM_SetAnim(pm,SETANIM_TORSO,BOTH_STAND2,SETANIM_FLAG_NORMAL);
					break;
				case WP_FORGE_PSYCH:
					PM_SetAnim(pm,SETANIM_TORSO,BOTH_STAND2,SETANIM_FLAG_NORMAL);
					break;
				case WP_SCAVENGER_RIFLE:
				case WP_CHAOTICA_GUARD_GUN:
					PM_SetAnim(pm,SETANIM_TORSO,TORSO_WEAPONREADY2,SETANIM_FLAG_NORMAL);
					//PM_SetAnim(pm,SETANIM_LEGS,BOTH_ATTACK2,SETANIM_FLAG_NORMAL);
					break;
				case WP_KLINGON_BLADE:
				case WP_IMPERIAL_BLADE:
					PM_SetAnim(pm,SETANIM_TORSO,BOTH_STAND3,SETANIM_FLAG_NORMAL);
					break;
				case WP_DESPERADO:
					PM_SetAnim(pm,SETANIM_TORSO,TORSO_WEAPONREADY2,SETANIM_FLAG_NORMAL);
					break;
				case WP_PALADIN:
					PM_SetAnim(pm,SETANIM_TORSO,TORSO_WEAPONREADY3,SETANIM_FLAG_NORMAL);
					break;
				case WP_BOT_WELDER:
					PM_SetAnim(pm,SETANIM_TORSO,TORSO_WEAPONIDLE2,SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_RESTART|SETANIM_FLAG_HOLD);
					break;
				default:
					PM_SetAnim(pm,SETANIM_TORSO,TORSO_WEAPONREADY3,SETANIM_FLAG_NORMAL);
					break;
				}
			}
		}
	}
	else if ( pm->ps->weaponstate == WEAPON_IDLE )
	{
		if(pm->gent && pm->gent->client->race == RACE_BORG)
		{
			PM_TorsoAnimBorg();
		}
		else if((pm->ps->legsAnim & ~ANIM_TOGGLEBIT) == BOTH_GUARD_LOOKAROUND1 )
		{
			PM_SetAnim(pm,SETANIM_TORSO,BOTH_GUARD_LOOKAROUND1,SETANIM_FLAG_NORMAL);
		}
		else if((pm->ps->legsAnim & ~ANIM_TOGGLEBIT) == BOTH_GUARD_IDLE1 )
		{
			PM_SetAnim(pm,SETANIM_TORSO,BOTH_GUARD_IDLE1,SETANIM_FLAG_NORMAL);
		}
		else if((pm->ps->legsAnim & ~ANIM_TOGGLEBIT) == BOTH_STAND2_RANDOM1 )
		{
			PM_SetAnim(pm,SETANIM_TORSO,BOTH_STAND2_RANDOM1,SETANIM_FLAG_NORMAL);
		}
		else if((pm->ps->legsAnim & ~ANIM_TOGGLEBIT) == BOTH_STAND2_RANDOM2 )
		{
			PM_SetAnim(pm,SETANIM_TORSO,BOTH_STAND2_RANDOM2,SETANIM_FLAG_NORMAL);
		}
		else if((pm->ps->legsAnim & ~ANIM_TOGGLEBIT) == BOTH_STAND2_RANDOM3 )
		{
			PM_SetAnim(pm,SETANIM_TORSO,BOTH_STAND2_RANDOM3,SETANIM_FLAG_NORMAL);
		}
		else if((pm->ps->legsAnim & ~ANIM_TOGGLEBIT) == BOTH_STAND2_RANDOM4 )
		{
			PM_SetAnim(pm,SETANIM_TORSO,BOTH_STAND2_RANDOM4,SETANIM_FLAG_NORMAL);
		}
		else if((pm->ps->legsAnim & ~ANIM_TOGGLEBIT) == BOTH_STAND2TO4 )
		{
			PM_SetAnim(pm,SETANIM_TORSO,BOTH_STAND2TO4,SETANIM_FLAG_NORMAL);
		}
		else if((pm->ps->legsAnim & ~ANIM_TOGGLEBIT) == BOTH_STAND4TO2 )
		{
			PM_SetAnim(pm,SETANIM_TORSO,BOTH_STAND4TO2,SETANIM_FLAG_NORMAL);
		}
		else if((pm->ps->legsAnim & ~ANIM_TOGGLEBIT) == BOTH_STAND4 )
		{
			PM_SetAnim(pm,SETANIM_TORSO,BOTH_STAND4,SETANIM_FLAG_NORMAL);
		}
		else
		{
			switch(pm->ps->weapon)
			{
			case WP_PHASER:
				if((pm->ps->legsAnim & ~ANIM_TOGGLEBIT) == BOTH_RUN1 )
				{
					PM_SetAnim(pm,SETANIM_TORSO,BOTH_RUN1,SETANIM_FLAG_NORMAL);
				}
				else
				{
					PM_SetAnim(pm,SETANIM_TORSO,TORSO_WEAPONIDLE1,SETANIM_FLAG_NORMAL);
				}
				break;

			case WP_COMPRESSION_RIFLE:
				PM_SetAnim(pm,SETANIM_TORSO,TORSO_WEAPONIDLE2,SETANIM_FLAG_NORMAL);
				break;

			case WP_BORG_ASSIMILATOR:
			case WP_BORG_DRILL:
			case WP_BORG_WEAPON:
			case WP_BORG_TASER:
				PM_SetAnim(pm,SETANIM_TORSO,BOTH_STAND1,SETANIM_FLAG_NORMAL);
				break;

			case WP_TRICORDER://should never get here, but just in case
			case WP_RED_HYPO:
			case WP_BLUE_HYPO:
			case WP_VOYAGER_HYPO:
				PM_SetAnim(pm,SETANIM_TORSO,BOTH_STAND1,SETANIM_FLAG_NORMAL);
				break;

			case WP_NONE:
				PM_SetAnim(pm,SETANIM_TORSO,BOTH_STAND1,SETANIM_FLAG_NORMAL);
				break;

			case WP_MELEE:
				PM_SetAnim(pm,SETANIM_TORSO,BOTH_STAND1,SETANIM_FLAG_NORMAL);
				break;

			case WP_STASIS_ATTACK:
				PM_SetAnim(pm,SETANIM_BOTH,BOTH_STAND1,SETANIM_FLAG_NORMAL);
				break;

			case WP_SCAVENGER_RIFLE:
			case WP_CHAOTICA_GUARD_GUN:
				PM_SetAnim(pm,SETANIM_TORSO,TORSO_WEAPONIDLE2,SETANIM_FLAG_NORMAL);
				break;

			case WP_KLINGON_BLADE:
			case WP_IMPERIAL_BLADE:
				PM_SetAnim(pm,SETANIM_TORSO,BOTH_STAND3,SETANIM_FLAG_NORMAL);
				break;

			case WP_DESPERADO:
				PM_SetAnim(pm,SETANIM_TORSO,TORSO_WEAPONIDLE2,SETANIM_FLAG_NORMAL);
				break;

			case WP_PALADIN:
				PM_SetAnim(pm,SETANIM_TORSO,TORSO_WEAPONIDLE2,SETANIM_FLAG_NORMAL);
				break;

			case WP_BOT_WELDER:
				PM_SetAnim(pm,SETANIM_TORSO,TORSO_WEAPONIDLE2,SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_RESTART|SETANIM_FLAG_HOLD);
				break;

			default:
				PM_SetAnim(pm,SETANIM_TORSO,TORSO_WEAPONIDLE3,SETANIM_FLAG_NORMAL);
				break;
			}
		}
	}
}

//=========================================================================
// Anim checking utils
//=========================================================================

int PM_GetTurnAnim( gentity_t *gent, int anim )
{
	if ( !gent )
	{
		return -1;
	}

	anim &= ~ANIM_TOGGLEBIT;

	switch( anim )
	{
	case BOTH_STAND1:			//# Standing idle: no weapon: hands down
	case BOTH_STAND1_RANDOM1:	//# Random standing idle
	case BOTH_STAND1_RANDOM2:	//# Random standing idle
	case BOTH_STAND1_RANDOM3:	//# Random standing idle
	case BOTH_STAND1_RANDOM4:	//# Random standing idle
	case BOTH_STAND1_RANDOM5:	//# Random standing idle
	case BOTH_STAND1_RANDOM6:	//# Random standing idle
	case BOTH_STAND1_RANDOM7:	//# Random standing idle
	case BOTH_STAND1_RANDOM8:	//# Random standing idle
	case BOTH_STAND1_RANDOM9:	//# Random standing idle
	case BOTH_STAND1_RANDOM10:	//# Random standing idle
	case BOTH_STAND1_RANDOM11:	//# Random standing idle
	case BOTH_STAND1_RANDOM12:	//# Random standing idle
	case BOTH_STAND1_RANDOM13:	//# Random standing idle
	case BOTH_STAND1_RANDOM14:	//# Random standing idle
	case BOTH_STAND2:			//# Standing idle with a weapon
	case BOTH_STAND2_RANDOM1:	//# Random standing idle
	case BOTH_STAND2_RANDOM2:	//# Random standing idle
	case BOTH_STAND2_RANDOM3:	//# Random standing idle
	case BOTH_STAND2_RANDOM4:	//# Random standing idle
	case BOTH_STAND3:			//# Standing hands behind back: at ease: etc.
	case BOTH_STAND4:			//# two handed: gun down: relaxed stand
	case BOTH_STAND5:			//# two handed: gun up: relaxed stand
	case BOTH_STAND6:			//# one handed: gun at side: relaxed stand
	case BOTH_STAND7:			//# (Chell) timid stance while looking around slightly and breathing
	case BOTH_STAND8:			//# breathing after exherting oneself one handed
	case BOTH_STAND9:			//# breathing after exherting oneself two handed
	case BOTH_STAND1TO3:			//# Transition from stand1 to stand3
	case BOTH_STAND3TO1:			//# Transition from stand3 to stand1
	case BOTH_STAND2TO4:			//# Transition from stand2 to stand4
	case BOTH_STAND4TO2:			//# Transition from stand4 to stand2
	case BOTH_STANDTOCONSOLE1:	//# a transition from stand animations to console animations
	case BOTH_STANDUP1:			//# standing up and stumbling
	case BOTH_TALKGESTURE1:		//# standing up and talking
	case BOTH_TALKGESTURE2:		//# standing up and talking
	case BOTH_TALKGESTURE3:		//# standing up and talking
	case BOTH_CONSOLE1:			//# Using a waist-high console with both hands
	case BOTH_CONSOLE1IDLE:		//# Idle of CONSOLE1
	case BOTH_CONSOLE1RIGHT:		//# Reach right from CONSOLE1
	case BOTH_CONSOLE1LEFT:		//# Reach left from CONSOLE1
	case BOTH_CONSOLE2:			//# Using a head-high wall console with the right hand
	case BOTH_CONSOLE3:			//# arms parallel to ground and typing similar to con.1
	case BOTH_CONSOLE3IDLE:		//# arms parallel to ground and typing similar to con.1
	case BOTH_CONSOLE3RIGHT:		//# arms parallel to ground and typing similar to con.1
	case BOTH_CONSOLE3LEFT:		//# arms parallel to ground and typing similar to con.1
	case BOTH_CONSOLETOSTAND1:	//# a transition from console animations to stand animations
	case BOTH_GESTURE1:			//# Generic gesture: non-specific
	case BOTH_GESTURE2:			//# Generic gesture: non-specific
	case BOTH_GESTURE3:			//# Generic gesture: non-specific
	case BOTH_CROWDLOOK1:		//# Person staring out into space 1
	case BOTH_CROWDLOOK2:		//# Person staring out into space 2
	case BOTH_CROWDLOOK3:		//# Person staring out into space 3
	case BOTH_CROWDLOOK4:		//# Person staring out into space 4
	case BOTH_GRAB1:				//# Grabbing something from table
	case BOTH_GRAB2:				//# Grabbing something from table
	case BOTH_GRAB3:				//# Grabbing something from table
	case BOTH_GRABBED1:			//# cin9.3 chell being grabbed 180 from munro: 28 pixels away
	case BOTH_GRABBED2:			//# cin9.3 idle grabbed 180 from munro: 28 pixels away
	case BOTH_SURPRISED1:		//# Surprised reaction 1
	case BOTH_SURPRISED2:		//# Surprised reaction 2
	case BOTH_SURPRISED3:		//# Surprised reaction 3
	case BOTH_SURPRISED4:		//# Surprised reaction 4
	case BOTH_SURPRISED5:		//# Surprised reaction 5
	case BOTH_POSSESSED1:		//# 7 of 9 possessed
	case BOTH_POSSESSED2:		//# 7 of 9 possessed with hand out 
	case BOTH_SNAPTO1:			//# cin.23: 7o9 coming to from borg possession
	case BOTH_SNAPTO2:			//# cin.23: 7o9 coming to from borg possession2
	case BOTH_LAUGH2:			//# standing laugh of mocking Biessman
	case BOTH_ACTIVATEBELT1:		//# activating transport buffer on belt
	case BOTH_TALK1:				//# Generic talk anim
		if ( PM_HasAnimation( gent, LEGS_TURN1 ) )
		{
			return LEGS_TURN1;
		}
		else
		{
			return -1;
		}
		break;
	case BOTH_ATTACK1:			//# Attack with generic 1-handed weapon
	case BOTH_ATTACK2:			//# Attack with generic 2-handed weapon
	case BOTH_ATTACK3:			//# Attack with heavy 2-handed weapon
	case BOTH_ATTACK4:			//# Attack with ???
	case BOTH_ATTACK5:			//# Attack with rocket launcher
	case BOTH_MELEE1:			//# First melee attack
	case BOTH_MELEE2:			//# Second melee attack
	case BOTH_MELEE3:			//# Third melee attack
	case BOTH_MELEE4:			//# Fourth melee attack
	case BOTH_MELEE5:			//# Fifth melee attack
	case BOTH_MELEE6:			//# Sixth melee attack
	case BOTH_HELP1:				//# helping hold injured4 man.
	case BOTH_DROPANGERWEAP2:		//# cin.23: Nelson lowering weapon in anger
	case BOTH_ASSIMILATED1:		//# Cin.18: Foster being assimilated by borg
	case BOTH_SHIELD1:			//# cin.6: munro's initial reaction to explosion
	case BOTH_SHIELD2:			//# cin.6: munro in shielding position looping
	case BOTH_COVERUP1_LOOP:		//# animation of getting in line of friendly fire
	case BOTH_HEROSTANCE1:		//# Biessman in the final shootout
	case BOTH_SCARED1:			//# Scared reaction 1
	case BOTH_SCARED2:			//# Scared reaction 2
	case BOTH_GUARD_LOOKAROUND1:	//# Cradling weapon and looking around
	case BOTH_GUARD_IDLE1:		//# Cradling weapon and standing
	case BOTH_GUARD_LKRT1:		//# cin17: quick glance right to sound of door slamming
	case BOTH_ALERT1:			//# Startled by something while on guard
	case BOTH_LEAN1:				//# leaning on a railing
	case BOTH_LEAN1TODROPHELM:	//# transition from LEAN1 to DROPHELM
		if ( PM_HasAnimation( gent, LEGS_TURN2 ) )
		{
			return LEGS_TURN2;
		}
		else
		{
			return -1;
		}
		break;
	default:
		return -1;
		break;
	}
}

qboolean PM_InOnGroundAnim (gentity_t *self)
{
	switch(self->s.legsAnim&~ANIM_TOGGLEBIT)
	{
	case BOTH_DEAD1:
	case BOTH_DEAD2:
	case BOTH_DEAD3:
	case BOTH_DEAD4:
	case BOTH_DEAD5:
	case BOTH_DEADFORWARD1:
	case BOTH_DEADBACKWARD1:
	case BOTH_DEADFORWARD2:
	case BOTH_DEADBACKWARD2:
	case BOTH_LYINGDEATH1:
	case BOTH_LYINGDEAD1:
	case BOTH_PAIN2WRITHE1:		//# Transition from upright position to writhing on ground anim
	case BOTH_WRITHING1:			//# Lying on ground writhing in pain
	case BOTH_WRITHING1RLEG:		//# Lying on ground writhing in pain: holding right leg
	case BOTH_WRITHING1LLEG:		//# Lying on ground writhing in pain: holding left leg
	case BOTH_WRITHING2:			//# Lying on stomache writhing in pain
	case BOTH_INJURED1:			//# Lying down: against wall - can also be sleeping
	case BOTH_CRAWLBACK1:			//# Lying on back: crawling backwards with elbows
	case BOTH_INJURED2:			//# Injured pose 2
	case BOTH_INJURED3:			//# Injured pose 3
	case BOTH_INJURED6:			//# Injured pose 6
	case BOTH_INJURED6ATTACKSTART:	//# Start attack while in injured 6 pose 
	case BOTH_INJURED6ATTACKSTOP:	//# End attack while in injured 6 pose
	case BOTH_INJURED6COMBADGE:	//# Hit combadge while in injured 6 pose
	case BOTH_INJURED6POINT:		//# Chang points to door while in injured state
	case BOTH_SLEEP1:			//# laying on back-rknee up-rhand on torso
	case BOTH_SLEEP2:			//# on floor-back against wall-arms crossed
	case BOTH_SLEEP5:			//# Laying on side sleeping on flat sufrace
	case BOTH_SLEEP_IDLE1:		//# rub face and nose while asleep from sleep pose 1
	case BOTH_SLEEP_IDLE2:		//# shift position while asleep - stays in sleep2
	case BOTH_SLEEP1_NOSE:		//# Scratch nose from SLEEP1 pose
	case BOTH_SLEEP2_SHIFT:		//# Shift in sleep from SLEEP2 pose
		return qtrue;
		break;
	}

	return qfalse;
}

qboolean PM_InDeathAnim ( void )
{//Purposely does not cover stumbledeath and falldeath...
	switch((pm->ps->legsAnim&~ANIM_TOGGLEBIT))
	{
	case BOTH_DIVE1:
	case BOTH_DEATHBACKWARD1:
	case BOTH_DEATHBACKWARD2:
	case BOTH_DEATHFORWARD1:
	case BOTH_DEATHFORWARD2:
	case BOTH_DEATH1:
	case BOTH_DEATH2:
	case BOTH_DEATH3:
	case BOTH_DEATH4:
	case BOTH_DEATH5:
	case BOTH_DEATH6:
	case BOTH_DEATH7:

	case BOTH_DEATH1IDLE:
	case BOTH_LYINGDEATH1:
	case BOTH_STUMBLEDEATH1:
		return qtrue;
		break;
	default:
		return qfalse;
		break;
	}
}
