//Avatar

#include "b_local.h"
#include "anims.h"
#include "g_nav.h"

void		Avatar_Hunt( void );
qboolean	Avatar_Backup( void );
void		Avatar_Strike( gentity_t *target );
void		Avatar_Attack( void );
qboolean	Avatar_CheckAttack( attack_e type );
void		Avatar_Melee( void );
void		Avatar_Ranged( float dist );
extern void NPC_ApplyWeaponFireDelay( void );

/*
-------------------------
NPC_BSAvatar_Idle
-------------------------
*/

void NPC_BSAvatar_Idle( void )
{
	//See if we have a goal to run to
	if ( UpdateGoal() )
	{
		NPC_MoveToGoal();
	}

	//Update our angles regardless
	NPC_UpdateAngles( qtrue, qtrue );
}

/*
-------------------------
Avatar_Strike
-------------------------
*/

#define	STRIKE_DIST		64
#define STRIKE_DAMAGE	10

void Avatar_Strike( gentity_t *target )
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
		G_Damage( traceEnt, NPC, NPC, forward, trace.endpos, STRIKE_DAMAGE, 0, MOD_PHASER );
	}
}

/*
-------------------------
Avatar_Attack
-------------------------
*/

void Avatar_Attack( void )
{	
	NPC->client->ps.weapon = WP_MELEE;
	ucmd.buttons |= BUTTON_ALT_ATTACK;

	NPC->attackDebounceTime = level.time + 1100;

	//Inflict damage
	Avatar_Strike( NPC->enemy );
}

/*
-------------------------
Avatar_CheckAttack
-------------------------
*/

qboolean Avatar_CheckAttack( attack_e type )
{
	//TODO: Implement or remove
	switch( type )
	{
	case ATTACK_MELEE:
		break;

	case ATTACK_RANGE:
		break;
	}

	return ( NPC->attackDebounceTime < level.time );
}

/*
-------------------------
Avatar_Melee
-------------------------
*/

void Avatar_Melee( void )
{
	//Check to hit
	if ( Avatar_CheckAttack( ATTACK_MELEE ) == qtrue )
	{
		//Get um!
		Avatar_Attack();
	}
}

/*
-------------------------
Avatar_Ranged
-------------------------
*/

#define MIN_CRITICAL_DIST		256
#define MIN_CRITICAL_DIST_SQR	( MIN_CRITICAL_DIST * MIN_CRITICAL_DIST )

#define	MIN_DESIRED_DIST		512
#define MIN_DESIRED_DIST_SQR	( MIN_DESIRED_DIST * MIN_DESIRED_DIST )

void Avatar_Ranged( float dist )
{
	//Attempt to fire off a shot at the player
	if ( Avatar_CheckAttack( ATTACK_RANGE ) == qtrue )
	{
		if ( ( Q_irand( 0, 10 ) > 8 ) && ( NPCInfo->pauseTime < level.time ) )
		{
			//Fire
			NPC->client->ps.weapon = WP_FORGE_PSYCH;
			ucmd.buttons |= BUTTON_ATTACK;
			
			NPC_ApplyWeaponFireDelay();

			//Setup delays
			NPCInfo->pauseTime = level.time + 2000;		//Time to fire a ranged shot again
			NPCInfo->standTime = level.time + 1400;		//Time to stand still
			NPCInfo->weaponTime = level.time + 2000;	//Time to do any attack again
			NPC->attackDebounceTime = level.time + 1100;
			return;
		}
	}

	if ( dist > MIN_DESIRED_DIST_SQR )
	{
		// We want to get closer
		Avatar_Hunt();
	}
	else if ( dist < MIN_CRITICAL_DIST_SQR )
	{
		// We're not happy when the player gets too close
		Avatar_Backup();
	}
}

/*
-------------------------
Avatar_Hunt
-------------------------
*/

void Avatar_Hunt( void )
{
	//If we're not supposed to stand still, pursue the player
	if ( NPCInfo->standTime < level.time )
	{
		//Move towards our goal
		NPCInfo->goalEntity = NPC->enemy;
		NPCInfo->goalRadius = 12;
		NPC_MoveToGoal();

		//Add a delay time so they don't attack immediately out of a run
		NPCInfo->weaponTime = level.time + 1000;
	}
}

/*
-------------------------
Avatar_Backup
-------------------------
*/

#define BACKUP_CHECK_DIST	-64
#define DROP_DIST			72

qboolean Avatar_Backup( void )
{
	trace_t	trace;
	vec3_t	endpos, forward;

	// Get our forward direction
	AngleVectors( NPC->currentAngles, forward, NULL, NULL );

	// Check behind us a bit
	VectorMA( NPC->currentOrigin, BACKUP_CHECK_DIST, forward, endpos );
	gi.trace( &trace, NPC->currentOrigin, NULL, NULL, endpos, NPC->s.number, MASK_SHOT );

	if ( trace.fraction < 0.8f )
	{
		// There is a wall behind us
		return false;
	}

	// Check down so we don't run off a cliff
	VectorSet( forward, 0, 0, -1 );

	VectorMA( endpos, DROP_DIST, forward, forward );
	gi.trace( &trace, endpos, NULL, NULL, forward, NPC->s.number, MASK_SHOT );

	if ( trace.fraction == 1.0f )
	{
		// there is a drop so don't backup
		return false;
	}

	if ( NPCInfo->standTime < level.time )
	{
		NPCInfo->goalEntity = NPC->enemy;
		NPCInfo->goalRadius = 12;
		NPC_MoveToGoal();

		ucmd.forwardmove *= -1;
		ucmd.rightmove *= -1;
	}

	return qtrue;
}

/*
-------------------------
NPC_BSAvatar_Attack
-------------------------
*/

#define	MIN_MELEE_RANGE		82
#define	MIN_MELEE_RANGE_SQR	( MIN_MELEE_RANGE * MIN_MELEE_RANGE )

void NPC_BSAvatar_Attack( void )
{
	//Update our angles regardless
	NPC_UpdateAngles( qtrue, qtrue );

	//If we don't have an enemy, just idle
	if ( NPC_CheckEnemyExt() == qfalse )
	{
		NPC_BSAvatar_Idle();
		return;
	}

	//Rate our distance to the target, and our visibilty
	float		distance	= (int) DistanceHorizontalSquared( NPC->currentOrigin, NPC->enemy->currentOrigin );	
	distance_e	distRate	= ( distance > MIN_MELEE_RANGE_SQR ) ? DIST_LONG : DIST_MELEE;
	int			visRate		= NPC_ClearLOS( NPC->enemy );

	//If we cannot see our target, move to see it
	if ( visRate == qfalse )
	{
		Avatar_Hunt();		
		return;
	}

	if ( distance < MIN_CRITICAL_DIST )
	{
		// We're not happy when the player gets too close
		if ( Avatar_Backup() )
		{
			// we could backup so don't do anything else for now
			return;
		}
	}

	if ( NPC->enemy )
		NPC_FaceEnemy();

	//Decide what to do next
	switch ( distRate )
	{
	case DIST_MELEE:
		Avatar_Melee();
		break;

	case DIST_LONG:
		Avatar_Ranged( distance );
		break;
	}
}
