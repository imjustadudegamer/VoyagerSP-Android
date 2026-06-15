//Reaver

#include "b_local.h"
#include "anims.h"
#include "g_nav.h"

void Reaver_Hunt( void );
void Reaver_Strike( gentity_t *target );
void Reaver_Attack( void );
qboolean Reaver_CheckAttack( attack_e type );
void Reaver_Melee( void );
void Reaver_Ranged( float dist );

/*
-------------------------
NPC_BSReaver_Idle
-------------------------
*/

void NPC_BSReaver_Idle( void )
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
Reaver_Strike
-------------------------
*/

#define	STRIKE_DIST		80
#define STRIKE_DAMAGE	10

void Reaver_Strike( gentity_t *target )
{
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
		G_Damage( traceEnt, NPC, NPC, forward, trace.endpos, STRIKE_DAMAGE, DAMAGE_NO_KNOCKBACK, MOD_PHASER );
	}
}

/*
-------------------------
Reaver_Attack
-------------------------
*/

void Reaver_Attack( void )
{
	// The slash trail "formation" hasn't been started yet, so don't try and use the old points.
	NPC->trigger_formation = qfalse;

	if ( rand() & 1 )
	{
		NPC_SetAnim( NPC, SETANIM_BOTH, BOTH_MELEE1, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD );
		NPCInfo->weaponTime = level.time + 1000;
		NPCInfo->standTime = level.time + 2000;
	}
	else
	{
		NPC_SetAnim( NPC, SETANIM_BOTH, BOTH_MELEE2, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD );
		NPCInfo->weaponTime = level.time + 1000;
		NPCInfo->standTime = level.time + 2000;
	}

	//Inflict damage
	Reaver_Strike( NPC->enemy );
}

/*
-------------------------
Reaver_CheckAttack
-------------------------
*/

qboolean Reaver_CheckAttack( attack_e type )
{
	switch( type )
	{
	case ATTACK_MELEE:
		break;

	case ATTACK_RANGE:
		break;
	}

	return ( NPCInfo->weaponTime < level.time );
}

/*
-------------------------
Reaver_Melee
-------------------------
*/

void Reaver_Melee( void )
{
	//Check to hit
	if ( Reaver_CheckAttack( ATTACK_MELEE ) == qtrue )
	{
		//Get um!
		Reaver_Attack();
	}

	NPC_FaceEnemy();
	NPC_UpdateAngles( qtrue, qtrue );
}

/*
-------------------------
Reaver_Pounce
-------------------------
*/

#define	REAVER_JUMP_VELOCITY	400
#define	REAVER_JUMP_HEIGHT		325

void Reaver_Start_Pounce( void )
{
	// Set up the jump animations, but don't jump just yet
	NPC_SetAnim( NPC, SETANIM_BOTH, BOTH_JUMP1, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD );

	client->ps.pm_flags |= PMF_JUMP_HELD;
	client->ps.groundEntityNum = ENTITYNUM_NONE;

	NPCInfo->pauseTime	= level.time + 2000;	//Time to fire a ranged shot again
	NPCInfo->standTime	= level.time + 1500;	//Time to stand still
	NPCInfo->weaponTime = level.time + 2000;	//Time to do any attack again
	NPCInfo->jumpTime	= level.time + 3000;	//Time to jump again
}

/*
-------------------------
Reaver_Jump
-------------------------
*/
void Reaver_Jump( void )
{
	if ( NPC->enemy )
		NPC_FaceEnemy();

	// This seems a bit risky, but here I'm trying to actually delay the jump process so it's timed up better with its anims
	if ( NPCInfo->jumpTime - 2500 > level.time && level.time + 2700 > NPCInfo->jumpTime )
	{
		vec3_t	dir;

		AngleVectors( NPC->currentAngles, dir, NULL, NULL );
		VectorScale( dir, REAVER_JUMP_VELOCITY, dir );
		dir[2] += REAVER_JUMP_HEIGHT;

		VectorCopy( dir, NPC->client->ps.velocity );
	}
}

/*
-------------------------
Reaver_JumpOK
-------------------------
*/

// This could be a bit expensive, but fortunately, it doesn't get called very often...
//  ...and it helps to keep them from doing jumps that would make them look stupid
qboolean Reaver_JumpOK( void )
{
	vec3_t	dir, org, end, mins, maxs;
	trace_t	tr;

	// build a trace box
	VectorSet( maxs, 8, 8, 8 );
	VectorScale( maxs, -1, mins );

	AngleVectors( NPC->currentAngles, dir, NULL, NULL );

	// trace diagonally up
	VectorMA( NPC->currentOrigin, 160, dir, org );
	org[2] += 160;

	gi.trace( &tr, NPC->currentOrigin, mins, maxs, org, NPC->s.number, MASK_NPCSOLID ); // NOTE: box trace

	if ( tr.fraction != 1.0f )
		return qfalse; // hit something

	// trace forward
	VectorMA( NPC->currentOrigin, 256, dir, org );
	gi.trace( &tr, NPC->currentOrigin, NULL, NULL, org, NPC->s.number, MASK_NPCSOLID );

	if ( tr.fraction != 1.0f )
		return qfalse; // hit something

	// trace down from the forward point to see if we might fall to our deaths from this jump
	VectorSet( dir, 0, 0, -1 );
	VectorMA( org, 200, dir, end );

	gi.trace( &tr, org, NULL, NULL, end, NPC->s.number, MASK_NPCSOLID );

	if ( tr.fraction == 1.0f )
		return qfalse; // would fall too far

	return qtrue; // jump, baby, jump!!
}

/*
-------------------------
Reaver_Ranged
-------------------------
*/

#define	MIN_JUMP_DIST		380
#define MIN_JUMP_DIST_SQR	( MIN_JUMP_DIST * MIN_JUMP_DIST )

void Reaver_Ranged( float dist )
{
	if ( NPCInfo->weaponTime < level.time && NPC_CheckCanAttackExt() )
	{
		//Attempt to fire off a shot at the player
		if ( NPCInfo->pauseTime < level.time )
		{
			//Fire
			NPC->client->ps.weapon = WP_FORGE_PROJ;
			ucmd.buttons |= BUTTON_ATTACK;

			NPC_ApplyWeaponFireDelay();

			//Setup delays
			NPCInfo->pauseTime = level.time + 2000;		//Time to fire a ranged shot again
			NPCInfo->standTime = level.time + 1500;		//Time to stand still
			NPCInfo->weaponTime = level.time + 2500;	//Time to do any attack again

			return;
		}
	}
	
	//See if we should leap at the player
	if ( dist > MIN_JUMP_DIST_SQR )
	{
		if ( ( Q_irand( 0, 50 ) > 48 ) && ( NPCInfo->jumpTime < level.time ) )
		{
			if ( Reaver_JumpOK() )
			{
				Reaver_Start_Pounce();
				return;
			}
		}
	}

	Reaver_Hunt();
}

/*
-------------------------
Reaver_Hunt
-------------------------
*/

void Reaver_Hunt( void )
{
	NPCInfo->combatMove = qtrue;

	//If we're not supposed to stand still, pursue the player
	if ( NPCInfo->standTime < level.time )
	{
		//Move towards our goal
		NPCInfo->goalEntity = NPC->enemy;
		NPCInfo->goalRadius = 12;
		NPC_MoveToGoal();
	}

	//Update our angles regardless
	NPC_UpdateAngles( qtrue, qtrue );
}

/*
-------------------------
Reaver_Backup
-------------------------
*/

void Reaver_Backup()
{
	vec3_t	dir;

	// Cheesy push
	AngleVectors( NPC->currentAngles, dir, NULL, NULL );
	VectorMA( NPC->client->ps.velocity, -50, dir, NPC->client->ps.velocity );
}

/*
-------------------------
NPC_BSReaver_Attack
-------------------------
*/

#define MIN_CRITICAL_DIST		64
#define MIN_CRITICAL_DIST_SQR	( MIN_CRITICAL_DIST * MIN_CRITICAL_DIST )
#define	MIN_MELEE_RANGE			80
#define	MIN_MELEE_RANGE_SQR		( MIN_MELEE_RANGE * MIN_MELEE_RANGE )

void NPC_BSReaver_Attack( void )
{
	// We may have a pounce animation started and are waiting to actually start the jump movement...
	Reaver_Jump();

	//If we don't have an enemy, just idle
	if ( NPC_CheckEnemyExt() == qfalse )
	{
		NPC_BSReaver_Idle();
		return;
	}

	//Rate our distance to the target, and our visibilty
	float		distance	= (int) DistanceHorizontalSquared( NPC->currentOrigin, NPC->enemy->currentOrigin );	
	distance_e	distRate	= ( distance > MIN_MELEE_RANGE_SQR ) ? DIST_LONG : DIST_MELEE;
	int			visRate		= NPC_ClearLOS( NPC->enemy );

	//If we cannot see our target, move to see it
	if ( visRate == qfalse )
	{
		Reaver_Hunt();
		return;
	}

	if ( distance < MIN_CRITICAL_DIST_SQR )
	{
		// We're not happy when the player gets too close
		Reaver_Backup();
	}

	//Decide what to do next
	switch ( distRate )
	{
	case DIST_MELEE:
		Reaver_Melee();
		break;

	case DIST_LONG:
		Reaver_Ranged( distance );
		break;
	}
}
