//
// NPC.cpp - generic functions
//

#include "b_local.h"
#include "anims.h"
#include "g_functions.h"
#include "say.h"
#include "g_squad.h"

extern vec3_t playerMins;
extern vec3_t playerMaxs;
extern void PM_SetAnimFinal(int *torsoAnim,int *legsAnim,int type,int anim,int priority,int *torsoAnimTimer,int *legsAnimTimer,gentity_t *gent);
extern void PM_SetTorsoAnimTimer( gentity_t *ent, int *torsoAnimTimer, int time );
extern void PM_SetLegsAnimTimer( gentity_t *ent, int *legsAnimTimer, int time );
extern void NPC_BSNoClip ( void );
extern void G_AddVoiceEvent( gentity_t *self, int event, int speakDebounceTime );
extern void NPC_ApplyRoff (void);
extern void NPC_TempLookTarget ( gentity_t *self, int lookEntNum, int minLookTime, int maxLookTime );
extern void NPC_CheckPlayerAim ( void );
extern void NPC_CheckAllClear ( void );
extern void ResetTeamCounters( void );
extern void G_AddVoiceEvent( gentity_t *self, int event, int speakDebounceTime );
extern qboolean NPC_CheckLookTarget( gentity_t *self );
extern void NPC_SetLookTarget( gentity_t *self, int entNum, int clearTime );

//Local Variables
// ai debug cvars
cvar_t		*debugNPCAI;			// used to print out debug info about the bot AI
cvar_t		*debugNPCFreeze;		// set to disable bot ai and temporarily freeze them in place
cvar_t		*debugNPCAimingBeam;
cvar_t		*debugBreak;
cvar_t		*debugNoRoam;
cvar_t		*debugNPCName;

extern qboolean	stop_icarus;
extern int	teamLastEnemyTime[];

gentity_t		*NPC;
gNPC_t			*NPCInfo;
gclient_t		*client;
usercmd_t		ucmd;
qboolean		blocked;
visibility_t	enemyVisibility;

void NPC_SetAnim(gentity_t	*ent,int type,int anim,int priority);

/*
----------------------------------------
NPC_RemoveBody

Determines when it's ok to ditch the corpse
----------------------------------------
*/
#define REMOVE_DISTANCE		256
#define REMOVE_DISTANCE_SQR (REMOVE_DISTANCE * REMOVE_DISTANCE)

void NPC_RemoveBody( gentity_t *self )
{
	// Only do all of this nonsense for Scav boys ( and girls )
	if ( self->client->playerTeam == TEAM_SCAVENGERS || self->client->playerTeam == TEAM_KLINGON 
		|| self->client->playerTeam == TEAM_HIROGEN || self->client->playerTeam == TEAM_MALON )
	{
		self->nextthink = level.time + 1000; // try back in a second

		if ( DistanceSquared( g_entities[0].currentOrigin, self->currentOrigin ) <= REMOVE_DISTANCE_SQR )
		{
			return;
		}

		if ( (InFOV( self, &g_entities[0], 110, 90 )) ) // generous FOV check
		{
			if ( (NPC_ClearLOS( &g_entities[0], self->currentOrigin )) )
			{
				return;
			}
		}
	}

	G_FreeEntity( self );
}

/*
----------------------------------------
NPC_RemoveBody

Determines when it's ok to ditch the corpse
----------------------------------------
*/

int BodyRemovalPadTime( gentity_t *ent )
{
	int	time;

	if ( !ent || !ent->client )
		return 0;

	switch ( ent->client->playerTeam )
	{
	case TEAM_KLINGON:	// no effect, we just remove them when the player isn't looking
	case TEAM_SCAVENGERS:
	case TEAM_HIROGEN:
	case TEAM_MALON:
		time = 15000; // 15 secs.
		break;

	case TEAM_BORG:
		time = 2000;
		break;

	case TEAM_STASIS:
		return qtrue;
		break;

	case TEAM_FORGE:
		time = 1000;
		break;

	case TEAM_BOTS:
		time = 1000;
		break;

	case TEAM_8472:
		time = 2000;
		break;

	/*
	case TEAM_STARFLEET:
	*/
	default:
		// never go away
		time = Q3_INFINITE;
		break;
	}

	return time;
}


/*
----------------------------------------
NPC_RemoveBodyEffect

Effect to be applied when ditching the corpse
----------------------------------------
*/

static void NPC_RemoveBodyEffect(void)
{
	vec3_t		org;
	gentity_t	*tent;

	if ( !NPC || !NPC->client || (NPC->s.eFlags & EF_NODRAW) )
		return;

	switch(NPC->client->playerTeam)
	{
	case TEAM_BORG:
		// Don't do borg effects if disintegrating from Quantum, Alt-Phaser, & Alt-Compression...
		if ( NPC->client->ps.powerups[PW_REGEN] || NPC->client->ps.powerups[PW_DISINT_2] || NPC->client->ps.powerups[PW_DISINT_3] )
			return;

		VectorCopy( NPC->currentOrigin, org );
		org[2] += NPC->mins[2] + 1;
		G_TempEntity( org, EV_BORG_RECYCLE );
		break;

	case TEAM_STARFLEET:
		//FIXME: Starfleet beam out
		break;

	case TEAM_FORGE:
		NPC->s.eFlags |= EF_SCALE_DOWN;
		// Don't do forge effect if disintegrating from Quantum, Alt-Phaser, & Alt-Compression...
		if ( NPC->client->ps.powerups[PW_REGEN] || NPC->client->ps.powerups[PW_DISINT_2] || NPC->client->ps.powerups[PW_DISINT_3] )
			return;

		NPC->client->ps.eFlags |= EF_SCALE_DOWN;
		NPC->fx_time = level.time;
		G_AddEvent( NPC, EV_FORGE_FADE, 1 );
		break;

	case TEAM_BOTS:
		VectorCopy( NPC->currentOrigin, org );
		org[2] -= 16;
		tent = G_TempEntity( org, EV_BOT_EXPLODE );
		tent->owner = NPC;
		break;

	case TEAM_8472:
		// Don't do 8472 effects if disintegrating from Quantum, Alt-Phaser, & Alt-Compression...
		if ( NPC->client->ps.powerups[PW_REGEN] || NPC->client->ps.powerups[PW_DISINT_2] || NPC->client->ps.powerups[PW_DISINT_3] )
			return;

		// Scale them down while they die
		NPC->s.eFlags |= EF_SCALE_DOWN;
		NPC->client->ps.eFlags |= EF_SCALE_DOWN;
		NPC->fx_time = level.time;
		tent = G_TempEntity( NPC->client->ps.origin, EV_SPECIES_TELEPORT );
		tent->s.eventParm = 1;
		tent->owner = NPC;
		break;

	default:
		break;
	}
}


/*
----------------------------------------
DeadThink
----------------------------------------
*/

static void DeadThink ( void ) 
{
	if ( !NPCInfo->timeOfDeath && NPC->client != NULL && NPCInfo != NULL ) 
	{
		//haven't finished death anim yet and were NOT given a specific amount of time to wait before removal
		int				legsAnim	= (NPC->client->ps.legsAnim & ~ANIM_TOGGLEBIT);
		animation_t		*animations	= knownAnimFileSets[NPC->client->clientInfo.animFileIndex].animations;

		NPC->bounceCount = -1; // This is a cheap hack for optimizing the pointcontents check below

		if ( NPC->client->renderInfo.legsFrame == animations[legsAnim].firstFrame + (animations[legsAnim].numFrames - 1) )
		{
			//reached the end of the death anim
			NPCInfo->timeOfDeath = level.time + BodyRemovalPadTime( NPC );
		}
	}
	else
	{
		//death anim done (or were given a specific amount of time to wait before removal), wait the requisite amount of time them remove
		if ( level.time >= NPCInfo->timeOfDeath )
		{
			if ( NPC->client->ps.eFlags & EF_NODRAW )
			{
				NPC->e_ThinkFunc = thinkF_G_FreeEntity;
				NPC->nextthink = level.time + FRAMETIME;
			}
			else
			{
				// Start the body effect first, then delay 400ms before ditching the corpse
				NPC_RemoveBodyEffect();
				NPC->e_ThinkFunc = thinkF_NPC_RemoveBody;

				if ( NPC->client->playerTeam == TEAM_FORGE )
					NPC->nextthink = level.time + FRAMETIME * 8;
				else
					NPC->nextthink = level.time + FRAMETIME * 4;
			}
			return;
		}
	}

	// If the player is on the ground and the resting position contents haven't been set yet...(BounceCount tracks the contents)
	if ( NPC->bounceCount < 0 && NPC->s.groundEntityNum >= 0 )
	{
		// if client is in a nodrop area, make him/her nodraw
		int contents = NPC->bounceCount = gi.pointcontents( NPC->currentOrigin, -1 );

		if ( ( contents & CONTENTS_NODROP ) ) 
		{
			NPC->client->ps.eFlags |= EF_NODRAW;
		}
	}
	
	// run the bot through the server like it was a real client
	ClientThink(NPC->s.number, &ucmd);
	VectorCopy(NPC->s.origin, NPC->s.origin2 );
}


/*
===============
SetNPCGlobals

local function to set globals used throughout the AI code
===============
*/
void SetNPCGlobals( gentity_t *ent ) 
{
	NPC = ent;
	NPCInfo = ent->NPC;
	client = ent->client;
}

gentity_t	*_saved_NPC;
gNPC_t		*_saved_NPCInfo;
gclient_t	*_saved_client;

void SaveNPCGlobals() 
{
	_saved_NPC = NPC;
	_saved_NPCInfo = NPCInfo;
	_saved_client = client;
}

void RestoreNPCGlobals() 
{
	NPC = _saved_NPC;
	NPCInfo = _saved_NPCInfo;
	client = _saved_client;
}

//We MUST do this, other funcs were using NPC illegally when "self" wasn't the global NPC
void ClearNPCGlobals( void ) 
{
	NPC = NULL;
	NPCInfo = NULL;
	client = NULL;
}
//===============

extern	qboolean	showBBoxes;
static vec3_t RED = {1.0, 0.0, 0.0};
static vec3_t GREEN = {0.0, 1.0, 0.0};
static vec3_t BLUE = {0.0, 0.0, 1.0};
static vec3_t LIGHT_BLUE = {0.3f, 0.7f, 1.0};
char	showSPaths[MAX_QPATH];
extern void CG_Cube( vec3_t mins, vec3_t maxs, vec3_t color, float alpha );
extern void CG_Line( vec3_t start, vec3_t end, vec3_t color, float alpha );
extern void CG_Cylinder( vec3_t start, vec3_t end, float radius, vec3_t color );
void NPC_DrawSquadPath( squadPath_t *squadPath )
{
	int			i, j;
	vec3_t		mins, maxs;

	for(i = 0; i < squadPath->numWaypoints; i++)
	{
		if ( gi.inPVS(squadPath->waypoints[i].origin, g_entities[0].currentOrigin) )
		{
			VectorAdd( squadPath->waypoints[i].origin, playerMins, mins );
			VectorAdd( squadPath->waypoints[i].origin, playerMaxs, maxs );
			CG_Cube( mins, maxs, LIGHT_BLUE, 0.25 );

			for(j = 0; j < MAX_PATH_BRANCHES; j++)
			{
				if( squadPath->waypoints[i].nextWp[j] != -1 )
				{
					//don't draw 2-lines for 2-way connections...
					if ( i > squadPath->waypoints[i].nextWp[j] )
					{//Would have already drawn this one
						continue;
					}
					CG_Line( squadPath->waypoints[i].origin, squadPath->waypoints[squadPath->waypoints[i].nextWp[j]].origin, LIGHT_BLUE, 0.25 );
				}
			}
		}
	}
}

void NPC_ShowDebugInfo (void)
{
	if ( showBBoxes )
	{
		gentity_t	*found = NULL;
		vec3_t		mins, maxs;

		while( (found = G_Find( found, FOFS(classname), "NPC" ) ) != NULL )
		{
			if ( gi.inPVS( found->currentOrigin, g_entities[0].currentOrigin ) )
			{
				VectorAdd( found->currentOrigin, found->mins, mins );
				VectorAdd( found->currentOrigin, found->maxs, maxs );
				CG_Cube( mins, maxs, RED, 0.25 );
			}
		}
	}

	if ( showSPaths[0] )
	{
		if ( Q_stricmp( "all", showSPaths ) == 0 )
		{
			int			i;

			for(i = 0; i < num_squad_paths; i++)
			{
				NPC_DrawSquadPath( &squadPaths[i] );
			}
		}
		else
		{
			gentity_t	*found = NULL;

			if ( (found = G_Find(found, FOFS(targetname), showSPaths)) != NULL && found->NPC && found->NPC->iSquadPathIndex != -1 )
			{
				NPC_DrawSquadPath( &squadPaths[found->NPC->iSquadPathIndex] );
			}
		}
	}
}

void NPC_ApplyScriptFlags (void)
{
	if(NPCInfo->scriptFlags & SCF_CROUCHED)
	{
		ucmd.upmove = -127;
	}

	if(NPCInfo->scriptFlags & SCF_RUNNING)
	{
		ucmd.buttons &= ~BUTTON_WALKING;
	}
	else if(NPCInfo->scriptFlags & SCF_WALKING)
	{
		ucmd.buttons |= BUTTON_WALKING;
	}

	if(NPCInfo->scriptFlags & SCF_CAREFUL)
	{
		ucmd.buttons |= BUTTON_CAREFUL;
	}

	if(NPCInfo->scriptFlags & SCF_LEAN_RIGHT)
	{
		ucmd.buttons |= BUTTON_USE;
		ucmd.rightmove = 127;
		ucmd.forwardmove = 0;
		ucmd.upmove = 0;
	}
	else if(NPCInfo->scriptFlags & SCF_LEAN_LEFT)
	{
		ucmd.buttons |= BUTTON_USE;
		ucmd.rightmove = -127;
		ucmd.forwardmove = 0;
		ucmd.upmove = 0;
	}

	if ( (NPCInfo->scriptFlags & SCF_ALT_FIRE) && (ucmd.buttons & BUTTON_ATTACK) )
	{//Use altfire instead
		ucmd.buttons |= BUTTON_ALT_ATTACK;
	}
}

void Q3_DebugPrint( int level, const char *format, ... );
void NPC_HandleAIFlags (void)
{
	//FIXME: make these flags checks a function call like NPC_CheckAIFlagsAndTimers
	if ( NPCInfo->aiFlags & NPCAI_LOST )
	{//Print that you need help!
		//FIXME: shouldn't remove this just yet if cg_draw needs it
		NPCInfo->aiFlags &= ~NPCAI_LOST;
		
		/*
		if ( showWaypoints )
		{
			Q3_DebugPrint(WL_WARNING, "%s can't navigate to target %s (my wp: %d, goal wp: %d)\n", NPC->targetname, NPCInfo->goalEntity->targetname, NPC->waypoint, NPCInfo->goalEntity->waypoint );
		}
		*/

		if ( NPCInfo->goalEntity && NPCInfo->goalEntity == NPC->enemy )
		{//We can't nav to our enemy
			//Drop enemy and see if we should search for him
			NPC_LostEnemyDecideChase();
		}
	}

	if ( NPCInfo->aiFlags & NPCAI_AWAITING_COMM )
	{
		if(NPCInfo->commWaitTime < level.time)
		{
			//FIXME: we shouldn't assume team_leader, we should remember who sent this hail!
			NPC_SetSayState(NPC, NPC->client->team_leader, Q_irand(SAY_BADHAIL1, SAY_BADHAIL4));
			NPCInfo->aiFlags &= ~NPCAI_AWAITING_COMM;
		}
	}

	/*
	NPCInfo->canShove = qfalse;
	//flag never gets set in current nav implementation
	if (NPCInfo->aiFlags & NPCAI_BLOCKED)
	{
		NPCInfo->consecutiveBlockedMoves++;
		NPCInfo->blockedDebounceTime = level.time + 1000;//Remember you were blocked for a whole second
		//If totally blocked, should we see if we can jump the obstacle?
		if(NPCInfo->blockingEntNum == ENTITYNUM_WORLD)//WORLD
		{//Can't go anywhere
			G_ActivateBehavior( NPC, BSET_STUCK);
			//If you're in formation, what do we do here?
		}
		else
		{
			gentity_t *blocker = &g_entities[NPCInfo->blockingEntNum];

			if( NPCInfo->consecutiveBlockedMoves > 10 )
			{//Okay, shove them out of the way!
				if(NPCInfo->shoveCount > 3)
				{//Already tried shoving 4 times, just stand here
					NPCInfo->canShove = qfalse;
				}
				else
				{
					NPCInfo->canShove = qtrue;
				}
			}

			if(blocker->client && blocker->client->playerTeam == NPC->client->playerTeam)
			{//Should we ask it to get out of the way?
				//FIXME:  NPC_SetSayBState(NPC, blocker, Q_irand(SAY_MOVEIT1, SAY_MOVEIT4);// ?
				if(NPCInfo->blockedSpeechDebounceTime < level.time)
				{
					if ( NPC->behaviorSet[BSET_BLOCKED] )
					{
						G_ActivateBehavior( NPC, BSET_BLOCKED);
					}
					else
					{
						G_AddVoiceEvent( NPC, Q_irand(EV_BLOCKED1, EV_BLOCKED3), 0 );
					}
#ifdef _DEBUG
					//gi.Printf( "%s: 'Hey, %s, move it!'\n", NPC->targetname, blocker->targetname );
#endif
					//NPCInfo->blockedSpeechDebounceTime = level.time + 10000;//FIXME: make a define
					//Ok, need to make it get out of the way...
				}
			}
			else if((blocker->client || blocker->takedamage) && blocker->health > 0 && blocker->health < 200 )
			{//Attack it!?  Set enemy and temp behavior?  Hmm...
				//Careful, what if it's explosive?
				G_SetEnemy( NPC, blocker );
				if( NPCInfo->consecutiveBlockedMoves == 30 )
				{//Blocked for three seconds straight
					G_ActivateBehavior( NPC, BSET_BLOCKED);
				}
			}
		}
	}
	else if(NPCInfo->blockedDebounceTime < level.time)
	{//Only clear if haven't been blocked for a whole second
		NPCInfo->consecutiveBlockedMoves = 0;
		NPCInfo->shoveCount = 0;
	}

	if(NPCInfo->shoveDebounce < level.time)
	{//We have shoved for 1 second at least
		NPCInfo->lastShoveDir = 0.0f;
	}

	//NAV_ClearBlockedInfo(NPC);
	*/

	//MRJ Request:
	if ( NPCInfo->aiFlags & NPCAI_GREET_ALLIES && !NPC->enemy )//what if "enemy" is the greetEnt?
	{//If no enemy, look for teammates to greet
		//FIXME: don't say hi to the same guy over and over again.
		if ( NPCInfo->greetingDebounceTime < level.time )
		{//Has been at least 2 seconds since we greeted last
			if ( !NPCInfo->greetEnt )
			{//Find a teammate whom I'm facing and who is facing me and within 128
				NPCInfo->greetEnt = NPC_PickAlly( qtrue, 128, qtrue, qtrue );
			}

			if ( NPCInfo->greetEnt && !Q_irand(0, 5) )
			{//Start greeting someone
				qboolean	greeted = qfalse;

				//TODO:  If have a greetscript, run that instead?

				//FIXME: make them greet back?
				if( !Q_irand( 0, 2 ) )
				{//Play gesture anim (press gesture button?)
					greeted = qtrue;
					NPC_SetAnim( NPC, SETANIM_TORSO, Q_irand( BOTH_GESTURE1, BOTH_GESTURE3 ), SETANIM_FLAG_NORMAL|SETANIM_FLAG_HOLD );
					//NOTE: play full-body gesture if not moving?
				}

				if( !Q_irand( 0, 2 ) )
				{//Play random voice greeting sound
					greeted = qtrue;
					//FIXME: need NPC sound sets

					G_AddVoiceEvent( NPC, Q_irand(EV_GREET1, EV_GREET3), 2000 );
				}

				if( !Q_irand( 0, 1 ) )
				{//set looktarget to them for a second or two
					greeted = qtrue;
					NPC_TempLookTarget( NPC, NPCInfo->greetEnt->s.number, 1000, 3000 );
				}

				if ( greeted )
				{//Did at least one of the things above
					//Don't greet again for 2 - 4 seconds
					NPCInfo->greetingDebounceTime = level.time + Q_irand( 2000, 4000 );
					NPCInfo->greetEnt = NULL;
				}
			}
		}
	}
}

void NPC_AvoidWallsAndCliffs (void)
{
/*
	vec3_t	forward, right, testPos, angles, mins;
	trace_t	trace;
	float	fwdDist, rtDist;
	//FIXME: set things like this forward dir once at the beginning
	//of a frame instead of over and over again
	if ( NPCInfo->aiFlags & NPCAI_NO_COLL_AVOID )
	{
		return;
	}

	if ( ucmd.upmove > 0 || NPC->client->ps.groundEntityNum == ENTITYNUM_NONE )
	{//Going to jump or in the air
		return;
	}

	if ( !ucmd.forwardmove && !ucmd.rightmove )
	{
		return;
	}

	if ( fabs( AngleDelta( NPC->currentAngles[YAW], NPCInfo->desiredYaw ) ) < 5.0 )//!ucmd.angles[YAW] )
	{//Not turning much, don't do this
		//NOTE: Should this not happen only if you're not turning AT ALL?
		//	You could be turning slowly but moving fast, so that would
		//	still let you walk right off a cliff...
		//NOTE: Or maybe it is a good idea to ALWAYS do this, regardless
		//	of whether ot not we're turning?  But why would we be walking
		//  straight into a wall or off	a cliff unless we really wanted to?
		return;
	}

	VectorCopy( NPC->mins, mins );
	mins[2] += STEPSIZE;
	angles[YAW] = NPC->client->ps.viewangles[YAW];//Add ucmd.angles[YAW]?
	AngleVectors( angles, forward, right, NULL );
	fwdDist = ((float)ucmd.forwardmove)/16.0f;
	rtDist = ((float)ucmd.rightmove)/16.0f;
	VectorMA( NPC->currentOrigin, fwdDist, forward, testPos );
	VectorMA( testPos, rtDist, right, testPos );
	gi.trace( &trace, NPC->currentOrigin, mins, NPC->maxs, testPos, NPC->s.number, NPC->clipmask );
	if ( trace.allsolid || trace.startsolid || trace.fraction < 1.0 )
	{//Going to bump into something, don't move, just turn
		ucmd.forwardmove = 0;
		ucmd.rightmove = 0;
		return;
	}

	VectorCopy(trace.endpos, testPos);
	testPos[2] -= 128;

	gi.trace( &trace, trace.endpos, mins, NPC->maxs, testPos, NPC->s.number, NPC->clipmask );
	if ( trace.allsolid || trace.startsolid || trace.fraction < 1.0 )
	{//Not going off a cliff
		return;
	}

	//going to fall at least 128, don't move, just turn... is this bad, though?  What if we want them to drop off?
	ucmd.forwardmove = 0;
	ucmd.rightmove = 0;
	return;
*/
}

void NPC_CheckAttackScript(void)
{
	if(!(ucmd.buttons & BUTTON_ATTACK))
	{
		return;
	}

	if(NPC->behaviorSet[BSET_ATTACK])
	{
		G_ActivateBehavior(NPC, BSET_ATTACK);
	}
}

float NPC_MaxDistSquaredForWeapon (void);
void NPC_CheckAttackHold(void)
{
	vec3_t		vec;

	// If they don't have an enemy they shouldn't hold their attack anim.
	if ( !NPC->enemy )
	{
		NPCInfo->attackHoldTime = 0;
		return;
	}

	if ( ( NPC->client->ps.weapon == WP_BORG_ASSIMILATOR ) || ( NPC->client->ps.weapon == WP_BORG_DRILL ) )
	{//FIXME: don't keep holding this if can't hit enemy?

		// If they don't have shields ( been disabled) they shouldn't hold their attack anim.
		if ( !(NPC->NPC->aiFlags & NPCAI_SHIELDS) )
		{
			NPCInfo->attackHoldTime = 0;
			return;
		}

		VectorSubtract(NPC->enemy->currentOrigin, NPC->currentOrigin, vec);
		if( VectorLengthSquared(vec) > NPC_MaxDistSquaredForWeapon() )
		{
			NPCInfo->attackHoldTime = 0;
			PM_SetTorsoAnimTimer(NPC, &NPC->client->ps.torsoAnimTimer, 0);
		}
		else if( NPCInfo->attackHoldTime && NPCInfo->attackHoldTime > level.time )
		{
			ucmd.buttons |= BUTTON_ATTACK;
		}
		else if ( ( NPCInfo->attackHold ) && ( ucmd.buttons & BUTTON_ATTACK ) )
		{
			NPCInfo->attackHoldTime = level.time + NPCInfo->attackHold;
			PM_SetTorsoAnimTimer(NPC, &NPC->client->ps.torsoAnimTimer, NPCInfo->attackHold);
		}
		else
		{
			NPCInfo->attackHoldTime = 0;
			PM_SetTorsoAnimTimer(NPC, &NPC->client->ps.torsoAnimTimer, 0);
		}
	}
	else
	{//everyone else...?  FIXME: need to tie this into AI somehow?
		VectorSubtract(NPC->enemy->currentOrigin, NPC->currentOrigin, vec);
		if( VectorLengthSquared(vec) > NPC_MaxDistSquaredForWeapon() )
		{
			NPCInfo->attackHoldTime = 0;
		}
		else if( NPCInfo->attackHoldTime && NPCInfo->attackHoldTime > level.time )
		{
			ucmd.buttons |= BUTTON_ATTACK;
		}
		else if ( ( NPCInfo->attackHold ) && ( ucmd.buttons & BUTTON_ATTACK ) )
		{
			NPCInfo->attackHoldTime = level.time + NPCInfo->attackHold;
		}
		else
		{
			NPCInfo->attackHoldTime = 0;
		}
	}
}

/*
void NPC_KeepCurrentFacing(void)

Fills in a default ucmd to keep current angles facing
*/
void NPC_KeepCurrentFacing(void)
{
	if(!ucmd.angles[YAW])
	{
		ucmd.angles[YAW] = ANGLE2SHORT( client->ps.viewangles[YAW] ) - client->ps.delta_angles[YAW];
	}

	if(!ucmd.angles[PITCH])
	{
		ucmd.angles[PITCH] = ANGLE2SHORT( client->ps.viewangles[PITCH] ) - client->ps.delta_angles[PITCH];
	}
}

/*
-------------------------
NPC_BehaviorSet_Default
-------------------------
*/

void NPC_BehaviorSet_Default( int bState )
{
	switch( bState )
	{
	case BS_WAIT://Do abolutely nothing
		NPC_BSWait ();
		break;

	case BS_IDLE://Stand around, move if have a goal, update angles
		NPC_BSIdle ();
		break;
	
	case BS_ROAM://Roam around, collect stuff
		NPC_BSRoam ();
		break;
	
	case BS_WALK://Walk toward their goals
		NPC_BSWalk ();
		break;

	case BS_CROUCH://Crouch-Walk toward their goals
		NPC_BSCrouch ();
		break;

	case BS_RUN://Run toward their goals
		NPC_BSRun ();
		break;

	case BS_STAND_AND_SHOOT://Stay in one spot and shoot- duck when neccesary
		NPC_BSStandAndShoot ();
		break;

	case BS_STAND_GUARD://Wait around for an enemy
		NPC_BSStandGuard ();
		break;
	
	case BS_PATROL://Follow a path, looking for enemies
		NPC_BSPatrol ();
		break;
	
	case BS_HUNT_AND_KILL://Track down enemies and kill them
		NPC_BSHuntAndKill ();
		break;
	
	case BS_EVADE://Run from enemies
		NPC_BSEvade ();
		break;
	
	case BS_EVADE_AND_SHOOT://Run from enemies, shoot them if they hit you
		NPC_BSEvadeAndShoot ();
		break;
	
	case BS_RUN_AND_SHOOT://Run to your goal and shoot enemy when possible
		NPC_BSRunAndShoot ();
		break;
	
	case BS_DEFEND://Defend an entity or spot?
		NPC_BSDefend ();
		break;
	
	case BS_COMBAT://Attack, evade, use cover, move about, etc.  Full combat AI - id Bot code
		NPC_BSCombat ();
		break;

	case BS_MEDIC://Go for lowest health buddy, hide and heal him.
		NPC_BSMedic ();
		break;

	case BS_MEDIC_COMBAT:	//27: Run to and heal people to a certain level then go back to hiding spot
		NPC_BSMedicCombat ();
		break;
	
	case BS_MEDIC_HIDE:		//28: Stay in hiding spot and wait for others to come to you
		NPC_BSMedicHide ();
		break;

	case BS_TAKECOVER://Find nearest cover from enemies
		NPC_BSTakeCover ();
		break;

	case BS_GET_AMMO://Go get some ammo
		NPC_BSGetAmmo ();
		break;

	case BS_ADVANCE_FIGHT://head toward captureGoal, shoot anything that gets in the way
		NPC_BSAdvanceFight ();
		break;

	case BS_FACE://Face a direction
		NPC_BSFace ();
		break;

	case BS_FORMATION://Maintain a formation
		NPC_BSFormation ();
		break;

	case BS_MOVE:
		NPC_BSMove();
		break;

	case BS_WAITHEAL:		//24: Do nothing until health > 75
		NPC_BSWaitHeal();
		break;

	case BS_SHOOT:			//25: shoot straight ahead
		NPC_BSShoot();
		break;

	case BS_SNIPER:			//26: wait for a good shot
		NPC_BSSniper();
		break;

	case BS_POINT_AND_SHOOT:
		NPC_BSPointShoot(qtrue);
		break;

	case BS_FACE_ENEMY:
		NPC_BSPointShoot(qfalse);
		break;

	case BS_INVESTIGATE:	//29:
		NPC_BSInvestigate();
		break;

	case BS_SLEEP://Follow a path, looking for enemies
		NPC_BSSleep ();
		break;

	case BS_SAY://36: Turn to sayTarg, use talk anim, say your sayString (look up string in your sounds table), exit tempBState when sound finished (anim of mouth should be timed to length of sound as well)
		NPC_BSSay ();
		break;

	case BS_AIM://37: Turn head and torso to facing, keep feet in place if you can
		NPC_BSFace ();
		break;

	case BS_LOOK://38: Turn head only to facing, keep torso and head in place if you can
		NPC_BSFace ();
		break;

	case BS_POINT_COMBAT://39: Head toward closest empty point_combat and shoot from there
		NPC_BSPointCombat ();
		break;

	case BS_FOLLOW_LEADER://# 40: Follow your leader and shoot any enemies you come across
		NPC_BSFollowLeader();
		break;

	case BS_JUMP:			//41: Face navgoal and jump to it.
		NPC_BSJump();
		break;

	case BS_REMOVE:
		NPC_BSRemove();
		break;

	case BS_SEARCH:			//# 43: Using current waypoint as a base, search the immediate branches of waypoints for enemies
		NPC_BSSearch();
		break;

	case BS_FLY:			//# 44: Moves around without gravity
		NPC_BSFly();
		break;

	case BS_NOCLIP:
		NPC_BSNoClip();
		break;

	case BS_WANDER:			//# 46: Wander down random waypoint paths
		NPC_BSWander();
		break;

	case BS_FACE_LEADER://# 47: Keep facing the leader
		NPC_BSFaceLeader();
		break;

	default:
	case BS_DEFAULT://whatever
		NPC_BSIdle ();
		break;
	}
}

/*
-------------------------
NPC_BehaviorSet_Vohrsoth
-------------------------
*/

void NPC_BehaviorSet_Vohrsoth( int bState )
{
	switch( bState )
	{
	case BS_RUN_AND_SHOOT:
	case BS_HUNT_AND_KILL:
		NPC_BSVohrsoth_Attack();
		break;

	default:
		NPC_BehaviorSet_Default( bState );
		break;
	}
}

/*
-------------------------
NPC_BehaviorSet_Borg
-------------------------
*/

void NPC_BehaviorSet_Borg( int bState )
{
	switch( bState )
	{
	case BS_RUN_AND_SHOOT:
	case BS_HUNT_AND_KILL:
		NPC_BSBorg_Attack();
		break;
	
	case BS_IDLE:
		NPC_BSBorg_Idle();
		break;

	case BS_STAND_AND_SHOOT:
		NPC_BSBorg_StandAndShoot();
		break;

	default:
		NPC_BehaviorSet_Default( bState );
		break;
	}
}

/*
-------------------------
NPC_BehaviorSet_Hirogen_Alpha
-------------------------
*/

void NPC_BehaviorSet_Hirogen_Alpha( int bState )
{
	switch( bState )
	{
	case BS_RUN_AND_SHOOT:
	case BS_HUNT_AND_KILL:
		NPC_BSHirogenAlpha_Attack();
		break;
	
	case BS_IDLE:
		NPC_BSHirogenAlpha_Idle();
		break;

	default:
		NPC_BehaviorSet_Default( bState );
		break;
	}
}

/*
-------------------------
NPC_BehaviorSet_Parasite
-------------------------
*/

void NPC_BehaviorSet_Parasite( int bState )
{
	switch( bState )
	{
	case BS_RUN_AND_SHOOT:
	case BS_HUNT_AND_KILL:
	case BS_PATROL:
		NPC_BSParasite_Attack();
		break;
	
	case BS_IDLE:
		NPC_BSParasite_Idle();
		break;

	default:
		NPC_BehaviorSet_Default( bState );
		break;
	}
}

/*
-------------------------
NPC_BehaviorSet_Etherian
-------------------------
*/

void NPC_BehaviorSet_Etherian( int bState )
{
	switch( bState )
	{
	case BS_RUN_AND_SHOOT:
	case BS_HUNT_AND_KILL:
	case BS_PATROL:
		NPC_BSEtherian_Attack();
		break;
	
	case BS_IDLE:
		NPC_BSEtherian_Idle();
		break;

	default:
		NPC_BehaviorSet_Default( bState );
		break;
	}
}

/*
-------------------------
NPC_BehaviorSet_Species
-------------------------
*/

void NPC_BehaviorSet_Species( int bState )
{
	switch( bState )
	{
	case BS_RUN_AND_SHOOT:
	case BS_HUNT_AND_KILL:
	case BS_PATROL:
		NPC_BSSpecies_Attack();
		break;
	
	case BS_IDLE:
		NPC_BSSpecies_Idle();
		break;

	default:
		NPC_BehaviorSet_Default( bState );
		break;
	}
}

/*
-------------------------
NPC_BehaviorSet_HeadBot
-------------------------
*/

void NPC_BehaviorSet_HeadBot( int bState )
{
	switch( bState )
	{
	case BS_RUN:
	case BS_NOCLIP:
		NPC_BSHeadBot_Run();
		break;

	default:
		NPC_BehaviorSet_Default( bState );
		break;
	}
}

/*
-------------------------
NPC_BehaviorSet_HunterSeeker
-------------------------
*/

void NPC_BehaviorSet_HunterSeeker( int bState )
{
	switch( bState )
	{
	case BS_RUN_AND_SHOOT:
	case BS_HUNT_AND_KILL:
	case BS_PATROL:
		NPC_BSHunterSeeker_Attack();
		break;
	
	case BS_IDLE:
		NPC_BSHunterSeeker_Idle();
		break;

	default:
		NPC_BehaviorSet_Default( bState );
		break;
	}
}

/*
-------------------------
NPC_BehaviorSet_Avatar
-------------------------
*/

void NPC_BehaviorSet_Avatar( int bState )
{
	switch( bState )
	{
	case BS_RUN_AND_SHOOT:
	case BS_HUNT_AND_KILL:
	case BS_PATROL:
		NPC_BSAvatar_Attack();
		break;
	
	case BS_IDLE:
		NPC_BSAvatar_Idle();
		break;

	default:
		NPC_BehaviorSet_Default( bState );
		break;
	}
}

/*
-------------------------
NPC_BehaviorSet_Reaver
-------------------------
*/

void NPC_BehaviorSet_Reaver( int bState )
{
	switch( bState )
	{
	case BS_RUN_AND_SHOOT:
	case BS_HUNT_AND_KILL:
	case BS_PATROL:
		NPC_BSReaver_Attack();
		break;
	
	case BS_IDLE:
		NPC_BSReaver_Idle();
		break;

	default:
		NPC_BehaviorSet_Default( bState );
		break;
	}
}

/*
-------------------------
NPC_BehaviorSet_Harvester
-------------------------
*/

void NPC_BehaviorSet_Harvester( int bState )
{
	switch( bState )
	{
	case BS_RUN_AND_SHOOT:
	case BS_HUNT_AND_KILL:
	case BS_PATROL:
		NPC_BSHarvester_Attack();
		break;
	
	case BS_IDLE:
		NPC_BSHarvester_Idle();
		break;

	default:
		NPC_BehaviorSet_Default( bState );
		break;
	}
}

/*
-------------------------
NPC_BehaviorSet_WarriorBot
-------------------------
*/

void NPC_BehaviorSet_WarriorBot( int bState )
{
	switch( bState )
	{
	case BS_RUN_AND_SHOOT:
	case BS_HUNT_AND_KILL:
	case BS_PATROL:
		NPC_BSWarrior_Attack();
		break;
	
	case BS_IDLE:
		NPC_BSWarrior_Idle();
		break;

	default:
		NPC_BehaviorSet_Default( bState );
		break;
	}
}

/*
-------------------------
NPC_BehaviorSet_ScoutBot
-------------------------
*/

void NPC_BehaviorSet_ScoutBot( int bState )
{
	switch( bState )
	{
	case BS_RUN_AND_SHOOT:
	case BS_HUNT_AND_KILL:
	case BS_PATROL:
		NPC_BSScout_Attack();
		break;
	
	case BS_IDLE:
		NPC_BSScout_Idle();
		break;

	default:
		NPC_BehaviorSet_Default( bState );
		break;
	}
}

/*
-------------------------
NPC_BehaviorSet_Scavenger
-------------------------
*/

void NPC_BehaviorSet_Scavenger( int bState )
{
	switch( bState )
	{
	case BS_RUN_AND_SHOOT:
	case BS_HUNT_AND_KILL:
		NPC_BSScav_Attack();
		break;

	case BS_IDLE:
		NPC_BSScav_Idle();
		break;

	case BS_INVESTIGATE:
		NPC_BSScav_Investigate();
		break;

	case BS_PATROL:
		NPC_BSScav_Patrol();
		break;

	case BS_SLEEP:
		NPC_BSScav_Sleep();
		break;

	default:
		NPC_BehaviorSet_Default( bState );
		break;
	}
}

/*
-------------------------
NPC_RunBehavior
-------------------------
*/

void NPC_RunBehavior( int team, int bState )
{
	qboolean dontSetAim = qfalse;

	switch( team )
	{
	case TEAM_SCAVENGERS:
	case TEAM_IMPERIAL:
	case TEAM_KLINGON:
	case TEAM_HIROGEN:
	case TEAM_MALON:
		
		if ( !Q_stricmp( NPC->NPC_type, "hirogenalpha" ) )
		{
			NPC_BehaviorSet_Hirogen_Alpha( bState );
			return;
		}

		if ( NPC->client->ps.weapon == WP_KLINGON_BLADE ||
			NPC->client->ps.weapon == WP_IMPERIAL_BLADE )
		{//special melee exception
			NPC_BehaviorSet_Default( bState );
			return;
		}

		if ( Q_stricmp( "satan", NPC->NPC_type ) == 0 )
		{//very special case
			NPC_BehaviorSet_Default( bState );
			return;
		}

		NPC_BehaviorSet_Scavenger( bState );
		break;

	case TEAM_BOTS:
		
		if ( ( !Q_stricmp( NPC->NPC_type, "warriorbot" ) ) || ( !Q_stricmp( NPC->NPC_type, "warriorbot_boss" ) ) )
		{
			NPC_BehaviorSet_WarriorBot( bState );
			return;
		}

		if ( !Q_stricmp( NPC->NPC_type, "scoutbot" ) )
		{
			NPC_BehaviorSet_ScoutBot( bState );
			return;
		}

		if ( !Q_stricmp( NPC->NPC_type, "hunterseeker" ) )
		{
			NPC_BehaviorSet_HunterSeeker( bState );
			return;
		}

		if ( !Q_stricmp( NPC->NPC_type, "headbot" ) )
		{
			NPC_BehaviorSet_HeadBot( bState );
			return;
		}

		break;

	case TEAM_FORGE:

		if ( !Q_stricmp( NPC->NPC_type, "harvester" ) || !Q_stricmp( NPC->NPC_type, "biohulk" ) )
		{
			NPC_BehaviorSet_Harvester( bState );
			return;
		}

		if ( !Q_stricmp( NPC->NPC_type, "reaver" ) )
		{
			NPC_BehaviorSet_Reaver( bState );
			return;
		}

		if ( !Q_stricmp( NPC->NPC_type, "avatar" ) )
		{
			NPC_BehaviorSet_Avatar( bState );
			return;
		}

		if ( !Q_stricmp( NPC->NPC_type, "vohrsoth" ) )
		{
			NPC_BehaviorSet_Vohrsoth( bState );
			return;
		}

		//FIXME: call forge boss AI
		NPC_BehaviorSet_Default( bState );
		return;
		break;

	case TEAM_8472:

		NPC_BehaviorSet_Species( bState );
		return;
		break;

	case TEAM_STASIS:

		NPC_BehaviorSet_Etherian( bState );
		return;
		break;

	case TEAM_PARASITE:

		NPC_BehaviorSet_Parasite( bState );
		return;
		break;

	case TEAM_BORG:
		
		NPC_BehaviorSet_Borg( bState );
		return;
		break;

	default:
		NPC_BehaviorSet_Default( bState );
		dontSetAim = qtrue;
		break;
	}

	if ( !dontSetAim )
	{
		if ( NPC->enemy )
		{
			NPCInfo->lookMode = LT_AIMSWING;
		}
		else
		{
			if ( NPCInfo->lookMode == LT_AIMSWING )
			{
				NPCInfo->lookMode = LT_NONE;
			}
		}
	}

}

/*
===============
NPC_ExecuteBState

  MCG

NPC Behavior state thinking

===============
*/
void NPC_ExecuteBState ( gentity_t *self)//, int msec ) 
{
	bState_t	bState;

	NPC_HandleAIFlags();

	//FIXME: these next three bits could be a function call, some sort of setup/cleanup func
	//Lookmode must be reset every think cycle
	if(NPC->aimDebounceTime < level.time)
	{
		NPCInfo->lookMode = LT_NONE;
	}

	if(NPC->delayScriptTime && NPC->delayScriptTime <= level.time)
	{
		G_ActivateBehavior( NPC, BSET_DELAYED);
		NPC->delayScriptTime = 0;
	}

	//Clear this and let bState set it itself, so it automatically handles changing bStates... but we need a set bState wrapper func
	NPCInfo->combatMove = qfalse;

	//Execute our bState
	if(NPCInfo->tempBehavior)
	{//Overrides normal behavior until cleared
		bState = NPCInfo->tempBehavior;
	}
	else
	{
		if(!NPCInfo->behaviorState)
			NPCInfo->behaviorState = NPCInfo->defaultBehavior;

		bState = NPCInfo->behaviorState;
	}

	//Pick the proper bstate for us and run it
	NPC_RunBehavior( self->client->playerTeam, bState );
	
	//FIXME: Make these a func call
	if(bState != BS_FORMATION)
	{//So we know to re-acquire our closest squadpath point
		self->NPC->lastSquadPoint = -1;
//		NPCInfo->aiFlags |= NPCAI_OFF_PATH;
	}

	if(bState != BS_POINT_COMBAT && NPCInfo->combatPoint != -1)
	{
		//level.combatPoints[NPCInfo->combatPoint].occupied = qfalse;
		//NPCInfo->combatPoint = -1;
	}

	//Here we need to see what the scripted stuff told us to do
//Only process snapshot if independant and in combat mode- this would pick enemies and go after needed items
//	ProcessSnapshot();

//Ignore my needs if I'm under script control- this would set needs for items
//	CheckSelf();

	//Back to normal?  All decisions made?
	
	//FIXME: don't walk off ledges unless we can get to our goal faster that way, or that's our goal's surface
	//NPCPredict();

	if ( NPC->enemy )
	{
		if ( !NPC->enemy->inuse )
		{//just in case bState doesn't catch this
			G_ClearEnemy( NPC );
		}
	}

	if ( !NPC_CheckLookTarget( NPC ) )
	{
		if ( NPC->enemy )
		{
			if ( NPC->client->ps.weapon != WP_IMPERIAL_BLADE && NPC->client->ps.weapon != WP_KLINGON_BLADE )
			{//looking right at enemy during melee looks odd
				NPC_SetLookTarget( NPC, NPC->enemy->s.number, 0 );
			}
		}
	}

	if ( NPC->enemy )
	{
		if(NPC->enemy->flags & FL_DONT_SHOOT)
		{
			ucmd.buttons &= ~BUTTON_ATTACK;
		}

		if(client->ps.weaponstate == WEAPON_IDLE)
		{
			client->ps.weaponstate = WEAPON_READY;
		}
	}
	else 
	{
		if(client->ps.weaponstate == WEAPON_READY)
		{
			client->ps.weaponstate = WEAPON_IDLE;
		}
	}

	if(!(ucmd.buttons & BUTTON_ATTACK) && NPC->attackDebounceTime > level.time)
	{//We just shot but aren't still shooting, so hold the gun up for a while
		if(client->ps.weapon == WP_PHASER )
		{//One-handed
			NPC_SetAnim(NPC,SETANIM_TORSO,TORSO_WEAPONREADY1,SETANIM_FLAG_NORMAL);
		}
		else if(client->ps.weapon == WP_COMPRESSION_RIFLE)
		{//Sniper pose
			NPC_SetAnim(NPC,SETANIM_TORSO,TORSO_WEAPONREADY2,SETANIM_FLAG_NORMAL);
		}
		/*//FIXME: What's the proper solution here?
		else
		{//heavy weapon
			NPC_SetAnim(NPC,SETANIM_TORSO,TORSO_WEAPONREADY3,SETANIM_FLAG_NORMAL);
		}
		*/
	}
	else if (!NPC->enemy && bState != BS_FORMATION)//HACK!
	{
		if(client->ps.weapon != WP_TRICORDER)
		{
			if((NPC->s.torsoAnim&~ANIM_TOGGLEBIT) == TORSO_WEAPONREADY1 || (NPC->s.torsoAnim&~ANIM_TOGGLEBIT) == TORSO_WEAPONREADY2)
			{//we look ready for action, using one of the first 2 weapon, let's rest our weapon on our shoulder
				NPC_SetAnim(NPC,SETANIM_TORSO,TORSO_WEAPONIDLE1,SETANIM_FLAG_NORMAL);
			}
		}
	}

	NPC_CheckAttackHold();
	NPC_ApplyScriptFlags();
	
	//cliff and wall avoidance
	NPC_AvoidWallsAndCliffs();

	// run the bot through the server like it was a real client
//=== Save the ucmd for the second no-think Pmove ============================
	ucmd.serverTime = level.time - 50;
	NPCInfo->last_ucmd = ucmd;
	if ( !NPCInfo->attackHoldTime )
	{
		NPCInfo->last_ucmd.buttons &= ~BUTTON_ATTACK;//so we don't fire twice in one think
	}
//============================================================================
	NPC_CheckAttackScript();
	NPC_KeepCurrentFacing();

	if ( !NPC->next_roff_time || NPC->next_roff_time < level.time )
	{//If we were following a roff, we don't do normal pmoves.
		ClientThink( NPC->s.number, &ucmd );
	}
	else
	{
		NPC_ApplyRoff();
	}
	//Had to leave this in, some legacy code must still be using s.angles
	//Shouldn't interfere with interpolation of angles, should it?
	VectorCopy( client->ps.viewangles, NPC->currentAngles );

	// end of thinking cleanup
	NPCInfo->touchedByPlayer = NULL;

	NPC_CheckPlayerAim();
	NPC_CheckAllClear();
	
	/*if( ucmd.forwardmove || ucmd.rightmove )
	{
		int	i, la = -1, ta = -1;

		for(i = 0; i < MAX_ANIMATIONS; i++)
		{
			if((NPC->client->ps.legsAnim&~ANIM_TOGGLEBIT) == i)
			{
				la = i;
			}

			if((NPC->client->ps.torsoAnim&~ANIM_TOGGLEBIT) == i)
			{
				ta = i;
			}
			
			if(la != -1 && ta != -1)
			{
				break;
			}
		}

		if(la != -1 && ta != -1)
		{//FIXME: should never play same frame twice or restart an anim before finishing it
			gi.Printf("LegsAnim: %s(%d) TorsoAnim: %s(%d)\n", animTable[la].name, NPC->renderInfo.legsFrame, animTable[ta].name, NPC->client->renderInfo.torsoFrame);
		}
	}*/
}

void NPC_CheckInSolid(void)
{
	trace_t	trace;
	vec3_t	point;
	VectorCopy(NPC->currentOrigin, point);
	point[2] -= 0.25;

	gi.trace(&trace, NPC->currentOrigin, NPC->mins, NPC->maxs, point, NPC->s.number, NPC->clipmask);
	if(!trace.startsolid && !trace.allsolid)
	{
		VectorCopy(NPC->currentOrigin, NPCInfo->lastClearOrigin);
	}
	else
	{
		if(VectorLengthSquared(NPCInfo->lastClearOrigin))
		{
//			gi.Printf("%s stuck in solid at %s: fixing...\n", NPC->script_targetname, vtos(NPC->currentOrigin));
			G_SetOrigin(NPC, NPCInfo->lastClearOrigin);
			gi.linkentity(NPC);
		}
	}
}

/*
===============
NPC_Think

Main NPC AI - called once per frame
===============
*/
void NPC_Think ( gentity_t *self)//, int msec ) 
{
	self->nextthink = level.time + FRAMETIME;

	SetNPCGlobals( self );

	memset( &ucmd, 0, sizeof( ucmd ) );

	// see if NPC ai is frozen
	if ( debugNPCFreeze->value ) 
	{
		NPC_UpdateAngles( qtrue, qtrue );
		ClientThink(self->s.number, &ucmd);
		VectorCopy(self->s.origin, self->s.origin2 );
		return;
	}

	if(!self || !self->NPC || !self->client)
	{
		return;
	}

	// dead NPCs have a special think, don't run scripts (for now)
	//FIXME: this breaks deathscripts
	if (self->health <= 0) 
	{
		DeadThink();
		if(NPCInfo->nextBStateThink <= level.time)
		{
			if( self->taskManager && !stop_icarus )
			{
				self->taskManager->Update( );
			}
		}
		return;
	}

	self->nextthink = level.time + FRAMETIME/2;

	if(NPCInfo->nextBStateThink <= level.time)
	{
		/*
		if( self->taskManager && !stop_icarus )
		{
			self->taskManager->Update( );
		}
		*/

		if(NPC->s.eType != ET_PLAYER)
		{//Something drastic happened in our script
			return;
		}

		NPC_ExecuteBState( self );

		//Maybe even 200 ms?
		NPCInfo->nextBStateThink = level.time + FRAMETIME;

		if( self->taskManager && !stop_icarus )
		{
			self->taskManager->Update( );
		}
	}
	else
	{
		//or use client->pers.lastCommand?
		NPCInfo->last_ucmd.serverTime = level.time - 50;
		if ( !NPC->next_roff_time || NPC->next_roff_time < level.time )
		{//If we were following a roff, we don't do normal pmoves.
			//FIXME: firing angles (no aim offset) or regular angles?
			NPC_UpdateAngles(qtrue, qtrue);
			ClientThink(NPC->s.number, &NPCInfo->last_ucmd);
		}
		else
		{
			NPC_ApplyRoff();
		}
		VectorCopy(self->s.origin, self->s.origin2 );
	}
}

void NPC_InitAI ( void ) 
{
	debugBreak = gi.cvar ( "d_break", "0", 0  );
	debugNPCAI = gi.cvar ( "d_npcai", "0", 0  );
	debugNPCFreeze = gi.cvar ( "d_npcfreeze", "0", 0 );
	debugNoRoam = gi.cvar ( "d_noroam", "0", 0 );
	debugNPCAimingBeam = gi.cvar ( "d_npcaiming", "0", 0 );
}

/*
==================================
void NPC_InitAnimTable( void )

  Need to initialize this table.
  If someone tried to play an anim
  before table is filled in with
  values, causes tasks that wait for
  anim completion to never finish.
  (frameLerp of 0 * numFrames of 0 = 0)
==================================
*/
void NPC_InitAnimTable( void )
{
	for ( int i = 0; i < MAX_ANIM_FILES; i++ )
	{
		for ( int j = 0; j < MAX_ANIMATIONS; j++ )
		{
			knownAnimFileSets[i].animations[j].firstFrame = 0;
			knownAnimFileSets[i].animations[j].frameLerp = 100;
			knownAnimFileSets[i].animations[j].initialLerp = 100;
			knownAnimFileSets[i].animations[j].numFrames = 0;
		}
	}
}

void NPC_InitGame( void ) 
{
//	globals.NPCs = (gNPC_t *) gi.TagMalloc(game.maxclients * sizeof(game.bots[0]), TAG_GAME);
	debugNPCName = gi.cvar ( "d_npc", "", 0  );
	NPC_LoadParms();
	NPC_InitAI();
	NPC_InitAnimTable();
	ResetTeamCounters();
	for ( int team = TEAM_FREE; team < TEAM_NUM_TEAMS; team++ )
	{
		teamLastEnemyTime[team] = -10000;
	}
}

void NPC_SetAnim(gentity_t	*ent,int setAnimParts,int anim,int setAnimFlags)
{	// FIXME : once torsoAnim and legsAnim are in the same structure for NCP and Players
	// rename PM_SETAnimFinal to PM_SetAnim and have both NCP and Players call PM_SetAnim

	if(ent->client)
	{//Players, NPCs
		if (setAnimFlags&SETANIM_FLAG_OVERRIDE)
		{		
			if (setAnimParts & SETANIM_TORSO)
			{
				if( (setAnimFlags & SETANIM_FLAG_RESTART) || (ent->client->ps.torsoAnim & ~ANIM_TOGGLEBIT ) != anim )
				{
					PM_SetTorsoAnimTimer( ent, &ent->client->ps.torsoAnimTimer, 0 );
				}
			}
			if (setAnimParts & SETANIM_LEGS)
			{
				if( (setAnimFlags & SETANIM_FLAG_RESTART) || (ent->client->ps.legsAnim & ~ANIM_TOGGLEBIT ) != anim )
				{
					PM_SetLegsAnimTimer( ent, &ent->client->ps.legsAnimTimer, 0 );
				}
			}
		}

		PM_SetAnimFinal(&ent->client->ps.torsoAnim,&ent->client->ps.legsAnim,setAnimParts,anim,setAnimFlags,
			&ent->client->ps.torsoAnimTimer,&ent->client->ps.legsAnimTimer,ent);
	}
	else
	{//bodies, etc.
		if (setAnimFlags&SETANIM_FLAG_OVERRIDE)
		{		
			if (setAnimParts & SETANIM_TORSO)
			{
				if( (setAnimFlags & SETANIM_FLAG_RESTART) || (ent->s.torsoAnim & ~ANIM_TOGGLEBIT ) != anim )
				{
					PM_SetTorsoAnimTimer( ent, &ent->s.torsoAnimTimer, 0 );
				}
			}
			if (setAnimParts & SETANIM_LEGS)
			{
				if( (setAnimFlags & SETANIM_FLAG_RESTART) || (ent->s.legsAnim & ~ANIM_TOGGLEBIT ) != anim )
				{
					PM_SetLegsAnimTimer( ent, &ent->s.legsAnimTimer, 0 );
				}
			}
		}

		PM_SetAnimFinal(&ent->s.torsoAnim,&ent->s.legsAnim,setAnimParts,anim,setAnimFlags,
			&ent->s.torsoAnimTimer,&ent->s.legsAnimTimer,ent);
	}
}
