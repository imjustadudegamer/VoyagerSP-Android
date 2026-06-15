//NPC_reactions.cpp
#include "b_local.h"
#include "anims.h"
#include "g_functions.h"
#include "characters.h"

extern void G_AddVoiceEvent( gentity_t *self, int event, int speakDebounceTime );
float g_crosshairEntDist = Q3_INFINITE;
int g_crosshairSameEntTime = 0;
int g_crosshairEntNum = ENTITYNUM_NONE;
int g_crosshairEntTime = 0;
extern int	teamLastEnemyTime[];
extern	cvar_t	*g_spskill;
extern int PM_AnimLength( int index, animNumber_t anim );
extern characterName_t CharacterNames[];
extern void cgi_S_StartSound( vec3_t origin, int entityNum, int entchannel, sfxHandle_t sfx );
extern qboolean Q3_TaskIDPending( gentity_t *ent, taskID_t taskType );
extern int PM_PickAnim( gentity_t *self, int minAnim, int maxAnim );
extern qboolean NPC_CheckDisguise( gentity_t *ent );
extern qboolean NPC_CheckLookTarget( gentity_t *self );
extern void NPC_SetLookTarget( gentity_t *self, int entNum, int clearTime );

/*
-------------------------
NPC_CheckAttacker
-------------------------
*/

static void NPC_CheckAttacker( gentity_t *other )
{
	//FIXME: I don't see anything in here that would stop teammates from taking a teammate
	//			as an enemy.  Ideally, there would be code before this to prevent that from
	//			happening, but that is presumptuous.
	
	//valid ent - FIXME: a VALIDENT macro would be nice here
	if ( !other )
		return;

	if ( !other->inuse )
		return;

	//Don't take a target that doesn't want to be
	if ( other->flags & FL_NOTARGET ) 
		return;

	if ( NPC->svFlags & SVF_LOCKEDENEMY )
	{//IF LOCKED, CANNOT CHANGE ENEMY!!!!!
		return;
	}

	//If we haven't taken a target, just get mad
	if ( NPC->enemy == NULL )//was using "other", fixed to NPC
	{
		G_SetEnemy( NPC, other );
		return;
	}

	//Don't take the same enemy again
	if ( other == NPC->enemy )
		return;

	//Special case player interactions
	if ( other == &g_entities[0] )
	{
		//Account for the skill level to skew the results
		float	luckThreshold;

		switch ( g_spskill->integer )
		{
		//Easiest difficulty, mild chance of picking up the player
		case 0:
			luckThreshold = 0.9f;
			break;

		//Medium difficulty, half-half chance of picking up the player
		case 1:
			luckThreshold = 0.5f;
			break;

		//Hardest difficulty, always turn on attacking player
		case 2:
		default:
			luckThreshold = 0.0f;
			break;
		}

		//Randomly pick up the target
		if ( random() > luckThreshold )
		{
			G_ClearEnemy( other );
			other->enemy = NPC;
		}

		return;
	}
}

void NPC_SetPainEvent( gentity_t *self )
{
	if( self->client->playerTeam != TEAM_BORG )
	{
		if ( !Q3_TaskIDPending( self, TID_CHAN_VOICE ) )
		{
			G_AddEvent( self, EV_PAIN, self->health );
		}
	}
}

/*
-------------------------
NPC_GetPainChance
-------------------------
*/

#define	MIN_FLINCH_DAMAGE	50

float NPC_GetPainChance( gentity_t *self, int damage )
{
	if ( damage > MIN_FLINCH_DAMAGE )
		return 1.0f;

	switch ( g_spskill->integer )
	{
	case 0:	//easy
		return 0.75f;
		break;

	case 1://med
		return 0.35f;
		break;

	case 2://hard
	default:
		return 0.05f;
		break;
	}
}

/*
-------------------------
NPC_ChoosePainAnimation
-------------------------
*/

#define	MIN_PAIN_TIME	200

void NPC_ChoosePainAnimation( gentity_t *self, int damage )
{
	//If we've already taken pain, then don't take it again
	if ( level.time < self->painDebounceTime )
		return;

	int		pain_anim;
	float	pain_chance = NPC_GetPainChance( self, damage );

	//See what we want to do
	switch( (int) self->client->playerTeam )
	{
	
	//Crewmembers shouldn't base their pain on skill level
	case TEAM_STARFLEET:
		
		//Don't always take pain
		pain_chance = 0.25f;	//25%

		break;

	case TEAM_BOTS:

		//Never take pain
		if ( ( Q_stricmp( self->NPC_type, "warriorbot" ) == 0 ) || ( Q_stricmp( self->NPC_type, "warriorbot_boss" ) == 0 ) )
		{
			//Have to hit them hard to make them flinch
			if ( damage < 50 )
				return;

			//Take it less often
			pain_chance *= 0.5f;
		}

		break;

	case TEAM_FORGE:
		
		//Never take pain
		if ( Q_stricmp( self->NPC_type, "Vohrsoth" ) == 0 )
			return;

		break;

	//Hirogen Alpha does special shield maintenance
	case TEAM_HIROGEN:

		if ( Q_stricmp( self->NPC_type, "hirogenalpha" ) == 0 )
		{
			if ( Q_irand( 0, 1 ) )
			{
				//Set our pain animation
				self->painDebounceTime = level.time + 1000;
				NPC_SetAnim(self,SETANIM_BOTH,BOTH_PAIN1,SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
				
			}
			else
			{
				//Enraged
				self->painDebounceTime = level.time + 1000;
				NPC_SetAnim(self,SETANIM_BOTH,BOTH_POWERUP1,SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
			}
			
			//Turn the shield back on immediately, only allow one hit per shield drop
			//FIXME: Is this reliable?
			if ( self->client->ps.powerups[ PW_HIROGEN_SHIELD ] != -1 && !self->NPC->ignorePain )
			{
				self->s.powerups |= ( 1 << PW_HIROGEN_SHIELD );
				self->client->ps.powerups[ PW_HIROGEN_SHIELD ] = level.time + 10000;
				NPC_SetPainEvent( self );
			}

			return;
		}

		break;
		
	//All other NPC pain reactions
	default:
		break;
	}

	//See if we're going to flinch
	if ( random() < pain_chance )
	{
		//Pick and play our animation
		pain_anim = PM_PickAnim( self, BOTH_PAIN1, BOTH_PAIN3 );	//initialize to good data
		NPC_SetAnim( self, SETANIM_BOTH, pain_anim, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD );
		
		//Setup the timing for it
		self->painDebounceTime = level.time + PM_AnimLength( NPC->client->clientInfo.animFileIndex, (animNumber_t) pain_anim );
		self->client->fireDelay = 0;
		NPC_SetPainEvent( self );
	}

}

/*
===============
NPC_Pain
===============
*/
void NPC_Pain( gentity_t *self, gentity_t *other, int damage ) 
{
	team_t otherTeam = TEAM_FREE;

	if ( self->NPC == NULL ) 
		return;

	if ( other == NULL ) 
		return;

	//or just remove ->pain in player_die?
	if ( self->client->ps.pm_type == PM_DEAD )
		return;

	if ( other == self ) 
		return;

	//MCG: Ignore damage from your own team for now
	if ( other->client )
	{
		otherTeam = other->client->playerTeam;
		if ( otherTeam == TEAM_DISGUISE )
		{
			otherTeam = TEAM_STARFLEET;
		}
	}

	if ( other != self->enemy && self->client->playerTeam && other->client && otherTeam == self->client->playerTeam ) 
	{//Still run pain and flee scripts
		if ( self->client && self->NPC )
		{//Run any pain instructions
			if ( self->health <= (self->max_health/3) && ( VALIDSTRING( self->behaviorSet[BSET_FLEE] ) ) )
			{
				G_ActivateBehavior(self, BSET_FLEE);
			}
			else if( VALIDSTRING( self->behaviorSet[BSET_PAIN] ) )
			{
				G_ActivateBehavior(self, BSET_PAIN);
			}
		}
		return;
	}

	//Hirogen boss with shield
	if ( ( self->client->playerTeam == TEAM_HIROGEN ) ) 
	{
		if ( ( Q_stricmp( self->NPC_type, "hirogenalpha" ) == 0 ) && ( self->s.powerups & ( 1 << PW_HIROGEN_SHIELD ) ) )
			return;
	}

	SaveNPCGlobals();
	SetNPCGlobals( self );

	//Do extra bits
	if ( NPCInfo->ignorePain == qfalse )
	{
		//Check to take a new enemy
		NPC_CheckAttacker( other );

		if ( damage != -1 )
		{//don't play pain anim
			//Set our proper pain animation
			NPC_ChoosePainAnimation( self, damage );
		}
	}

	//Attempt to run any pain instructions
	if(self->client && self->NPC)
	{
		//FIXME: This needs better heuristics perhaps
		if(self->health <= (self->max_health/3) && ( VALIDSTRING( self->behaviorSet[BSET_FLEE] ) ) )
		{
			G_ActivateBehavior(self, BSET_FLEE);
		}
		else if( VALIDSTRING( self->behaviorSet[BSET_PAIN] ) )
		{
			G_ActivateBehavior(self, BSET_PAIN);
		}
	}

	//Attempt to fire any paintargets we might have
	if( self->paintarget && self->paintarget[0] )
	{
		G_UseTargets2(self, other, self->paintarget);
	}

	RestoreNPCGlobals();
}

/*
-------------------------
NPC_Touch
-------------------------
*/

void NPC_Touch(gentity_t *self, gentity_t *other, trace_t *trace) 
{
	if(!self->NPC)
		return;

	SaveNPCGlobals();
	SetNPCGlobals( self );

	if ( other->client ) 
	{//FIXME:  if pushing against another bot, both ucmd.rightmove = 127???
		//Except if not facing one another...
		if ( other->health > 0 ) 
		{
			NPCInfo->touchedByPlayer = other;
		}

		if ( other == NPCInfo->goalEntity ) 
		{
			NPCInfo->aiFlags |= NPCAI_TOUCHED_GOAL;
		}

		if( !(self->svFlags&SVF_LOCKEDENEMY) && !(self->svFlags&SVF_IGNORE_ENEMIES) && !(other->flags & FL_NOTARGET) )
		{
			if ( self->client->enemyTeam )
			{//See if we bumped into an enemy
				if ( other->client->playerTeam == self->client->enemyTeam )
				{//bumped into an enemy
					//FIXME: should we care about disguise here?
					if( NPCInfo->behaviorState != BS_HUNT_AND_KILL && !NPCInfo->tempBehavior )
					{//MCG - Begin: checking specific BS mode here, this is bad, a HACK
						//FIXME: not medics?
						G_SetEnemy( NPC, other );
		//				NPCInfo->tempBehavior = BS_HUNT_AND_KILL;
					}
				}
			}
		}

		//FIXME: do this if player is moving toward me and with a certain dist?
		/*
		if ( other->s.number == 0 && self->client->playerTeam == other->client->playerTeam )
		{
			VectorAdd( self->s.pushVec, other->client->ps.velocity, self->s.pushVec );
		}
		*/
	}
	else 
	{//FIXME: check for SVF_NONNPC_ENEMY flag here?
		if ( other == NPCInfo->goalEntity ) 
		{
			NPCInfo->aiFlags |= NPCAI_TOUCHED_GOAL;
		}
	}

	RestoreNPCGlobals();
}

/*
-------------------------
NPC_TempLookTarget
-------------------------
*/

void NPC_TempLookTarget( gentity_t *self, int lookEntNum, int minLookTime, int maxLookTime )
{
	if ( !self->client )
	{
		return;
	}

	if ( !minLookTime )
	{
		minLookTime = 1000;
	}

	if ( !maxLookTime )
	{
		maxLookTime = 1000;
	}

	if ( !NPC_CheckLookTarget( self ) )
	{//Not already looking at something else
		//Look at him for 1 to 3 seconds
		NPC_SetLookTarget( self, lookEntNum, level.time + Q_irand( minLookTime, maxLookTime ) );
	}
}

void NPC_Respond( gentity_t *self, int userNum )
{
	int event;

	if ( self->NPC->behaviorState == BS_FORMATION )
	{
		event = Q_irand(EV_MISSION1, EV_MISSION3);
	}
	else
	{
		if ( Q_irand( 0, 1 ) )
		{
			event = Q_irand(EV_RESPOND1, EV_RESPOND3);
		}
		else
		{
			event = Q_irand(EV_BUSY1, EV_BUSY3);
		}

		if( !Q_irand( 0, 1 ) )
		{//set looktarget to them for a second or two
			NPC_TempLookTarget( self, userNum, 1000, 3000 );
		}
	}

	G_AddVoiceEvent( self, event, 3000 );
}

void WaitNPCRespond ( gentity_t *self )
{
	//make sure the responding ent is still valid
	if ( !self->enemy || !self->enemy->client || !self->enemy->NPC )
	{
		G_FreeEntity( self );
		return;
	}

	if ( gi.S_Override[0] )
	{//player is still talking
		self->nextthink = level.time + 500;
		//set enemy to not respond for a bit longer
		self->enemy->NPC->blockedSpeechDebounceTime = level.time + 1000;
		return;
	}

	if ( self->enemy->health <= 0 || (!self->alt_fire && (self->enemy->NPC->scriptFlags&SCF_NO_RESPONSE)) )
	{
		G_FreeEntity( self );
		return;
	}

	//set enemy to be ready to respond
	self->enemy->NPC->blockedSpeechDebounceTime = 0;

	if ( self->alt_fire )
	{//Run the NPC's usescript
		G_ActivateBehavior( self->enemy, BSET_USE );
	}
	else
	{//make them respond generically
		NPC_Respond( self->enemy, 0 );
	}
}

/*
void G_PlayerGreet( gentity_t *self, thinkFunc_t thinkFunc )

  Makes the player say a greeting and waits until he's done to make the NPC respond
*/
qboolean G_PlayerGreet( gentity_t *self, qboolean useWhenDone )
{
	if ( !gi.S_Override[0] )
	{//used by the player and the player isn't talking
		sfxHandle_t	greeting = NULL;
		//If it's the player doing this, see who they're talking to and
		//have Munro play the right greeting sound (character name if a known
		//character else just lt., ensign or crewman).
		if ( Q_irand( 0, 3 ) )
		{
			for ( int character = 0; character < CHARACTER_CREWMAN; character++ )
			{
				if ( Q_stricmp( self->NPC_type, CharacterNames[character].name ) == 0 )
				{
					greeting = CharacterNames[character].soundIndex;
					break;
				}
			}

			//if didn't find one, use rank.
			if ( greeting == NULL && Q_irand( 0, 1 ) )
			{
				switch( self->NPC->rank )
				{
				case RANK_CREWMAN:
				case RANK_ENSIGN:
					greeting = CharacterNames[CHARACTER_CREWMAN].soundIndex;
					break;
				/*
				case RANK_ENSIGN:
					greeting = CharacterNames[CHARACTER_ENSIGN].soundIndex;
					break;
				*/
				case RANK_LT_JG:
				case RANK_LT:
					greeting = CharacterNames[CHARACTER_LT].soundIndex;
					break;
				case RANK_LT_COMM:
				case RANK_COMMANDER:
					greeting = CharacterNames[CHARACTER_COMM].soundIndex;
					break;
				case RANK_CAPTAIN:
					greeting = CharacterNames[CHARACTER_CAPT].soundIndex;
					break;
				case RANK_CIVILIAN:
				default:
					break;
				}
			}
		}//else qwe'll use a generic greeting

		if ( greeting == NULL )
		{
			greeting = CharacterNames[Q_irand(CHARACTER_GENERIC1, CHARACTER_GENERIC4)].soundIndex;
		}

		if ( greeting != NULL )
		{
			//G_SoundOnEnt( user, CHAN_VOICE, greeting );
			cgi_S_StartSound (NULL, 0, CHAN_VOICE, greeting );

			//Responder waits until Munro is done talking then makes the NPC respond.
			gentity_t *responder = G_Spawn();
			responder->enemy = self;
			responder->alt_fire = useWhenDone;
			responder->e_ThinkFunc = thinkF_WaitNPCRespond;
			responder->nextthink = level.time + 500;
			//set self to not respond for a bit longer
			self->NPC->blockedSpeechDebounceTime = level.time + 1000;
			return qtrue;
		}
	}

	return qfalse;
}
/*
-------------------------
NPC_UseResponse
-------------------------
*/

void NPC_UseResponse( gentity_t *self, gentity_t *user, qboolean useWhenDone )
{
	qboolean	noGreet = qfalse;

	if ( !self->NPC || !self->client )
	{
		return;
	}

	if ( user->s.number != 0 )
	{//not used by the player
		if ( useWhenDone )
		{
			G_ActivateBehavior( self, BSET_USE );
		}
		return;
	}

	if ( user->client && self->client->playerTeam != user->client->playerTeam )
	{//only those on the same team react
		if ( useWhenDone )
		{
			G_ActivateBehavior( self, BSET_USE );
		}
		return;
	}

	if ( self->NPC->blockedSpeechDebounceTime > level.time )
	{//I'm not responding right now
		return;
	}

	if ( gi.S_Override[self->s.number] )
	{//I'm talking already
		if ( !useWhenDone )
		{//you're not trying to use me
			return;
		}
		else
		{//I'm talking, so don't greet me
			noGreet = qtrue;
		}
	}

	if ( !noGreet && user->s.number == 0 && G_PlayerGreet( self, useWhenDone ) )
	{
		return;
	}

	if ( useWhenDone )
	{
		G_ActivateBehavior( self, BSET_USE );
	}
	else
	{
		NPC_Respond( self, user->s.number );
	}
}

/*
-------------------------
NPC_Use
-------------------------
*/

void NPC_Use( gentity_t *self, gentity_t *other, gentity_t *activator ) 
{
	if (self->client->ps.pm_type == PM_DEAD)
	{//or just remove ->pain in player_die?
		return;
	}

	SaveNPCGlobals();
	SetNPCGlobals( self );

	if(self->client && self->NPC)
	{//Run any use instructions
		if ( self->NPC->behaviorState == BS_MEDIC_HIDE && activator->client )
		{//Heal me NOW, dammit!
			if ( activator->health < activator->max_health )
			{//person needs help
				if ( self->NPC->eventualGoal != activator )
				{//not my current patient already
					NPC_TakePatient( activator );
					if ( self->behaviorSet[BSET_USE] )
					{
						G_ActivateBehavior( self, BSET_USE );
					}
				}
			}
			else if ( !self->enemy && activator->s.number == 0 && !gi.S_Override[self->s.number] && !(self->NPC->scriptFlags&SCF_NO_RESPONSE) )
			{//I don't have an enemy and I'm not talking and I was used by the player
				NPC_UseResponse( self, other, qfalse );
			}
		}
		else if ( self->behaviorSet[BSET_USE] )
		{
			NPC_UseResponse( self, other, qtrue );
		}
		else if ( isMedic( self ) )
		{//Heal me NOW, dammit!
			NPC_TakePatient( activator );
		}
		else if ( !self->enemy && activator->s.number == 0 && !gi.S_Override[self->s.number] && !(self->NPC->scriptFlags&SCF_NO_RESPONSE) )
		{//I don't have an enemy and I'm not talking and I was used by the player
			NPC_UseResponse( self, other, qfalse );
		}
	}

	RestoreNPCGlobals();
}

void NPC_CheckPlayerAim( void )
{
	//FIXME: need appropriate dialogue
	/*
	gentity_t *player = &g_entities[0];

	if ( player && player->client && player->client->ps.weapon > (int)(WP_NONE) && player->client->ps.weapon < (int)(WP_TRICORDER) )
	{//player has a weapon ready
		if ( g_crosshairEntNum == NPC->s.number && level.time - g_crosshairEntTime < 200 
			&& g_crosshairSameEntTime >= 3000 && g_crosshairEntDist < 256 )
		{//if the player holds the crosshair on you for a few seconds
			//ask them what the fuck they're doing
			G_AddVoiceEvent( NPC, Q_irand( EV_FF_1A, EV_FF_1C ), 0 );
		}
	}
	*/
}

void NPC_CheckAllClear( void )
{
	//FIXME: need to make this happen only once after losing enemies, not over and over again
	/*
	if ( NPC->client && !NPC->enemy && level.time - teamLastEnemyTime[NPC->client->playerTeam] > 10000 )
	{//Team hasn't seen an enemy in 10 seconds
		if ( !Q_irand( 0, 2 ) )
		{
			G_AddVoiceEvent( NPC, Q_irand(EV_SETTLE1, EV_SETTLE3), 3000 );
		}
	}
	*/
}