//Species 8472 Niner Charlie Tango Zebra 23.3435

#include "b_local.h"
#include "anims.h"
#include "g_nav.h"

extern void PM_SetLegsAnimTimer( gentity_t *ent, int *legsAnimTimer, int time );
extern void PM_SetTorsoAnimTimer( gentity_t *ent, int *torsoAnimTimer, int time );

void Species_Attack( void );
qboolean Species_CheckAttack( void );

#define	IDLE_RANGE		192
#define	IDLE_RANGE_SQR	( IDLE_RANGE * IDLE_RANGE )

/*
-------------------------
Species_ClearLOS
-------------------------
*/

#define	Species_MELEE_OFS	28

qboolean Species_ClearLOS( gentity_t *attacker, gentity_t *ent )
{
	trace_t		tr;
	
	if ( ( attacker == NULL ) || ( ent == NULL ) )
		return qfalse;

	vec3_t	start, end;

	VectorCopy( attacker->currentOrigin, start );
	start[2] += Species_MELEE_OFS;

	VectorCopy( ent->currentOrigin, end );
	end[2] += 8;

	gi.trace ( &tr, start, NULL, NULL, end, attacker->s.number, CONTENTS_SOLID|CONTENTS_BODY );

	if ( &g_entities[tr.entityNum] == ent )
		return qtrue;

	return qfalse;
}

/*
-------------------------
Species_CheckOverwhelmed
-------------------------
*/

#define	MAX_SWARM_EASY		1
#define	MAX_SWARM_NORMAL	3
#define	MAX_SWARM_HARD		5

qboolean Species_CheckOverwhelmed( void )
{
	int	numSurrounding = AI_GetGroupSize( NPC->enemy->currentOrigin, 48, NPC->client->playerTeam, NPC );
	int	max;

	switch( g_spskill->integer )
	{
	case 0:
		max = MAX_SWARM_EASY;
		break;

	case 1:
		max = MAX_SWARM_NORMAL;
		break;

	default:
	case 2:
		max = MAX_SWARM_HARD;
		break;
	}

	if ( numSurrounding >= max )
		return qtrue;

	return qfalse;
}

/*
-------------------------
Species_Hunt
-------------------------
*/

void Species_Hunt( void )
{
	if ( NPCInfo->weaponTime > level.time )
		return;

	if ( ( Species_ClearLOS( NPC, NPC->enemy ) == qfalse ) || ( Distance( NPC->currentOrigin, NPC->enemy->currentOrigin ) > 72 ) )
	{
		//If we're not supposed to stand still, pursue the player
		if ( NPCInfo->standTime < level.time )
		{
			PM_SetTorsoAnimTimer( NPC, &NPC->client->ps.torsoAnimTimer, 0 );

			//Move towards our goal
			NPCInfo->combatMove = qtrue;
			NPCInfo->goalEntity = NPC->enemy;
			NPCInfo->goalRadius = 12;
			NPC_MoveToGoal();
			AI_CheckEnemyCollision( NPC );
			NPC_UpdateAngles( qtrue, qtrue );
		}

		return;
	}

	//Attack if we can
	if ( NPCInfo->weaponTime < level.time )
	{
		if ( Species_CheckAttack() == qtrue )
		{
			//Get um!
			Species_Attack();
			NPCInfo->weaponTime = level.time + 1000;
		}
	}
}


/*
-------------------------
NPC_BSSpecies_Idle
-------------------------
*/

void NPC_BSSpecies_Idle( void )
{
	int alertEvent = NPC_CheckAlertEvents();

	//There is an event to look at
	if ( alertEvent >= 0 )
	{
		//Do something
		return;
	}
}

/*
-------------------------
Species_Strike
-------------------------
*/

#define	STRIKE_DIST		64
#define STRIKE_DAMAGE	7

void Species_Strike( gentity_t *target )
{
	if ( NPC->client->fireDelay == 0 || NPC->client->fireDelay >= 200 )
	{
		// Don't damage yet
		return;
	}

	trace_t	trace;
	vec3_t	endpos, forward;

	//Get our forward direction
	AngleVectors( NPC->currentAngles, forward, NULL, NULL );

	//Get the extent of our reach
	VectorMA( NPC->currentOrigin, STRIKE_DIST, forward, endpos );

	vec3_t	mins = { -8, -8, -8 }, maxs = { 8, 8, 8 };

	//Trace to hit
	gi.trace( &trace, NPC->currentOrigin, mins, maxs, endpos, NPC->s.number, MASK_SHOT );

	gentity_t *traceEnt = &g_entities[ trace.entityNum ];

	//If we hit, damage the victim
	if ( traceEnt->takedamage )
	{
		//FIXME: Pass in proper weapon information
		G_Damage( traceEnt, NPC, NPC, forward, trace.endpos, STRIKE_DAMAGE + ( g_spskill->integer * STRIKE_DAMAGE * 0.5f ), 0, MOD_PHASER );
		G_Sound( NPC, G_SoundIndex ("sound/enemies/species8472/hit5.wav") );
	}

	NPC->client->fireDelay = 0;
}

/*
-------------------------
8Species_Attack
-------------------------
*/

void Species_Attack( void )
{
	NPC_SetAnim( NPC, SETANIM_BOTH, BOTH_ATTACK1, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD );
	PM_SetTorsoAnimTimer( NPC, &NPC->client->ps.torsoAnimTimer, -1 );
//	ucmd.buttons |= BUTTON_ATTACK;

	NPC->attackDebounceTime = level.time + 1000;
	NPC->client->fireDelay = 400;
	NPCInfo->standTime = level.time + 1200;
}

/*
-------------------------
Species_CheckAttack
-------------------------
*/

qboolean Species_CheckAttack( void )
{
	return ( NPC->attackDebounceTime < level.time );
}

/*
-------------------------
NPC_BSSpecies_Attack
-------------------------
*/

#define	MIN_ATTACK_RANGE		100
#define	MIN_ATTACK_RANGE_SQR	( MIN_ATTACK_RANGE * MIN_ATTACK_RANGE )

void NPC_BSSpecies_Attack( void )
{
	NPC_UpdateAngles( qtrue, qtrue );

	//If we're attacking, then try to hit
	if ( ( NPC->enemy ) && ( NPC->client->fireDelay ) )
	{
		//Only hit if we're facing the enemy
		if ( NPC_FaceEnemy( qtrue ) )
			Species_Strike( NPC->enemy );

		return;
	}

	//Don't do anything if we're hurt
	if ( NPC->painDebounceTime > level.time )
		return;

	//If we don't have an enemy, just idle
	if ( NPC_CheckEnemyExt() == qfalse )
	{
		NPC_BSSpecies_Idle();
		return;
	}

	//See if there are already two or more enemies surrounding the target
	if ( Species_CheckOverwhelmed() )
	{
		gentity_t *ent = NPC_PickEnemy( NPC, TEAM_STARFLEET, qfalse, qfalse, qfalse );

		if ( ent == NPC->enemy )
		{
			//If so, just wait
			if ( DistanceSquared( NPC->currentOrigin, NPC->enemy->currentOrigin ) < IDLE_RANGE_SQR )
			{
				return;
			}
		}

		// Take the new enemy
		if ( ent )
		{
			G_SetEnemy( NPC, ent );
		}
	}

	//Otherwise charge in and attack
	Species_Hunt();
}