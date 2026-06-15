// Parasites

#include "b_local.h"
#include "g_nav.h" 
#include "anims.h" 

#define	MIN_MELEE_RANGE			60
#define	MIN_MELEE_RANGE_SQR		( MIN_MELEE_RANGE * MIN_MELEE_RANGE )

#define	MIN_AWAKE_RANGE			96
#define	MIN_AWAKE_RANGE_SQR		( MIN_AWAKE_RANGE * MIN_AWAKE_RANGE )

#define ATTACK_DISTANCE			36

#define AGGRESSIVE_SPAWNFLAG	8
#define FLAG_SPIT				4

#define SPIT_DIST_MULTIPLIER	10

#define	PARASITE_MELEE_OFS		28
#define	PARASITE_MELEE_DAMAGE	3

extern void PM_SetLegsAnimTimer( gentity_t *ent, int *legsAnimTimer, int time );
qboolean	NPC_BSParasite_Feast( float distance );
void		NPC_BSParasite_Idle( void );
void		NPC_BSParasite_Attack( void );
void		NPC_BSParasite_Hunt( float distance, qboolean advance );
void		NPC_ParasiteAttackPlayer( float distance );

//------------------------------------------------
void NPC_BSParasite_Idle( void )
{
	NPC_UpdateAngles( qfalse, qtrue );

	// See if we have a goal to run to
	if ( UpdateGoal() )
	{
		NPC_MoveToGoal();
	}
}


//------------------------------------------------
void NPC_BSParasite_Attack( void )
{
	NPC_UpdateAngles( qtrue, qtrue );

	// If we don't have an enemy or we should be feasting on a wall, just idle
	if ( NPC_CheckEnemyExt() == qfalse )
	{
		NPC_BSParasite_Idle();
		return;
	}

	// Rate our distance to the target, and our visibilty
	float		distance	= (int)DistanceHorizontalSquared( NPC->currentOrigin, NPC->enemy->currentOrigin );	
	qboolean	visible		= NPC_ClearLOS( NPC->enemy );
	float		distToCheck = MIN_MELEE_RANGE_SQR;
	qboolean	advance		= qfalse;

	if ( NPC_BSParasite_Feast( distance ) )
	{
		NPC_BSParasite_Idle();
		return;
	}

	if( NPC->spawnflags & FLAG_SPIT )
	{	
		// spitters attack at longer range
		distToCheck *= SPIT_DIST_MULTIPLIER;
	}

	if ( !visible || distance > distToCheck )
		advance = qtrue;

	NPC_BSParasite_Hunt( distance, advance );
}

// Hunt down the player
//------------------------------------------------
void NPC_BSParasite_Hunt( float distance, qboolean advance )
{
	NPCInfo->combatMove = qtrue;

	// If we're not supposed to stand still, pursue the player
	if ( NPCInfo->standTime < level.time )
	{
		if ( advance )
		{
			PM_SetLegsAnimTimer( NPC, &NPC->client->ps.legsAnimTimer, 0 );

			// Move towards our goal
			NPCInfo->goalEntity = NPC->enemy;
			NPCInfo->goalRadius = 12;
			NPC_MoveToGoal();

			// Don't attack right out of a run
			NPCInfo->weaponTime = level.time + 300;
		}
		else
		{
			// Try and attack
			NPC_ParasiteAttackPlayer( distance );
		}
	}

	NPC_UpdateAngles( qtrue, qtrue );
}

// Check if we should be feasting on a wall
//------------------------------------------------
qboolean NPC_BSParasite_Feast( float distance )
{
	if ( NPC->spawnflags & AGGRESSIVE_SPAWNFLAG || NPC->enemy)
		return qfalse;

	if ( distance < MIN_AWAKE_RANGE_SQR )
	{
		// Make me agressive
		NPC->spawnflags |= AGGRESSIVE_SPAWNFLAG;
		return qfalse;
	}

	return qtrue;
}

//------------------------------------------------
void NPC_ParasiteAttackPlayer( float distance )
{
	NPC_FaceEnemy();

	if ( distance > MIN_MELEE_RANGE_SQR )
	{
		PM_SetLegsAnimTimer( NPC, &NPC->client->ps.legsAnimTimer, 0 );

		if ( NPCInfo->weaponTime < level.time && ( NPC->spawnflags & FLAG_SPIT ) )
		{
			NPC_SetAnim( NPC, SETANIM_LEGS, BOTH_ATTACK1, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD );

			// shoot
			ucmd.buttons |= BUTTON_ATTACK;
			NPC_ApplyWeaponFireDelay();

			// Set up delays
			NPCInfo->weaponTime = level.time + 1300 + ( random() * 800 ) + ( 2 - g_spskill->integer) * 1200;
			NPCInfo->standTime = level.time + 1200;	// Exact length
		}
	}
	else
	{	
		// Slash with face
		NPC_SetAnim( NPC, SETANIM_LEGS, BOTH_ATTACK2, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD );
		PM_SetLegsAnimTimer( NPC, &NPC->client->ps.legsAnimTimer, -1 );
		
		if ( NPCInfo->standTime < level.time )
		{
			vec3_t	endpos, forward;
			trace_t	trace;

			//Get our forward direction
			AngleVectors( NPC->currentAngles, forward, NULL, NULL );

			//Get the extent of our reach
			VectorMA( NPC->currentOrigin, ATTACK_DISTANCE, forward, endpos );

			vec3_t	mins = { -4, -4, -4 }, maxs = { 4, 4, 4 };

			//Trace to hit
			gi.trace( &trace, NPC->currentOrigin, mins, maxs, endpos, NPC->s.number, MASK_SHOT );

			gentity_t *traceEnt = &g_entities[ trace.entityNum ];

			//If we hit, damage the victim
			if ( traceEnt->takedamage )
			{
				//FIXME: Pass in proper weapon information
				G_Damage( traceEnt, NPC, NPC, forward, trace.endpos, PARASITE_MELEE_DAMAGE, DAMAGE_NO_KNOCKBACK, MOD_STASIS );
			}

			NPCInfo->weaponTime = level.time + 200 + ( 2 - g_spskill->integer) * 400;
			NPCInfo->standTime = level.time + 200 + ( 2 - g_spskill->integer) * 400;
		}
	}
}
