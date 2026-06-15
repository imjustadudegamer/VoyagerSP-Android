// Harvester

#include "b_local.h"
#include "anims.h"
#include "g_nav.h"

/*
-------------------------
NPC_BSHarvester_Idle
-------------------------
*/

void NPC_BSHarvester_Idle( void )
{
	//See if we have a goal to run to
	if ( UpdateGoal() )
	{
		NPC_MoveToGoal();
	}

	NPC_UpdateAngles( qfalse, qtrue );
}

/*
-------------------------
Harvester_Strike
-------------------------
*/

#define	STRIKE_DIST		100
#define STRIKE_DAMAGE	6

void Harvester_Strike( void )
{
	trace_t	trace;
	vec3_t	endpos, forward;

	// Check to see if our damage should be delayed
	// NOTE: Since PM_Weapon handles decrementing fireDelay, I was afraid fireDelay would get to zero before I caught it, hence the generous range check
	if ( NPC->client->fireDelay == 0 || NPC->client->fireDelay >= 200 )
	{
		// Don't damage yet
		return;
	}

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
		G_Damage( traceEnt, NPC, NPC, forward, trace.endpos, STRIKE_DAMAGE, DAMAGE_NO_KNOCKBACK, MOD_PHASER );
		G_Sound( NPC, G_SoundIndex ("sound/enemies/harvester/stab3.wav") );
	}

	NPC->client->fireDelay = 0;
}

/*
-------------------------
Harvester_Attack
-------------------------
*/

#define HARVESTER_MELEE1_TIME	700
#define HARVESTER_MELEE2_TIME	700

void Harvester_Attack( void )
{
	NPC->trigger_formation = qfalse;

	// Pick an attack animation
	if ( rand() & 1 )
	{
		NPC_SetAnim( NPC, SETANIM_BOTH, BOTH_MELEE1, SETANIM_FLAG_OVERRIDE | SETANIM_FLAG_HOLD );

		NPCInfo->weaponTime = level.time + HARVESTER_MELEE1_TIME;
		NPC->attackDebounceTime = level.time + HARVESTER_MELEE1_TIME;

		// Damage should be delayed to correspond with the meat of the attack.
		NPC->client->fireDelay = 300;
	}
	else
	{
		NPC_SetAnim( NPC, SETANIM_BOTH, BOTH_MELEE2, SETANIM_FLAG_OVERRIDE | SETANIM_FLAG_HOLD );

		NPCInfo->weaponTime = level.time + HARVESTER_MELEE2_TIME;
		NPC->attackDebounceTime = level.time + HARVESTER_MELEE2_TIME;

		// Damage should be delayed to correspond with the meat of the attack.
		NPC->client->fireDelay = 400;
	}
}

/*
-------------------------
Harvester_CheckAttack
-------------------------
*/

qboolean Harvester_CheckAttack( void )
{
	return ( NPC->attackDebounceTime < level.time );
}

/*
-------------------------
NPC_BSHarvester_Attack
-------------------------
*/

#define	MIN_ATTACK_RANGE		100
#define	MIN_ATTACK_RANGE_SQR	( MIN_ATTACK_RANGE * MIN_ATTACK_RANGE )

void NPC_BSHarvester_Attack( void )
{
	// We may have delayed damage queued up, so check to see if this is the case
	Harvester_Strike();

	// If we don't have an enemy, just idle for now
	if ( NPC_CheckEnemyExt() == qfalse )
	{
		NPC_BSHarvester_Idle();
		return;
	}

	// Close in on the player if I should, but make sure that I've committed to any attacks I was currently doing.
	if ( DistanceHorizontalSquared( NPC->currentOrigin, NPC->enemy->currentOrigin ) > MIN_ATTACK_RANGE_SQR 
		&& NPCInfo->weaponTime < level.time )
	{
		// dunno, could this just be really bad?  seems to work though
		NPC->client->ps.legsAnimTimer = 0;
		NPC->client->ps.torsoAnimTimer = 0;

		// I always run
		ucmd.buttons &= ~BUTTON_WALKING;

		NPCInfo->combatMove = qtrue;
		NPCInfo->goalEntity = NPC->enemy;
		NPCInfo->goalRadius = 12;
		NPC_MoveToGoal();
		NPC_UpdateAngles( qtrue, qtrue );

		// Add a small delay time so I don't attack immediately out of a run
		NPC->attackDebounceTime = level.time + 400;
	}
	else
	{
		// Check to see if I should hit
		if ( Harvester_CheckAttack() == qtrue )
		{
			// Sometimes I have problems with facing the enemy I'm attacking, so force the issue so I don't look dumb
			if ( NPC->enemy )
				NPC_FaceEntity( NPC->enemy );

			// Get um!
			Harvester_Attack();
		}

		NPC_UpdateAngles( qtrue, qtrue );
	}
}
