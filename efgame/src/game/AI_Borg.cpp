#include "b_local.h"
#include "anims.h"
#include "AI.h"

//Projectile weapon
#define	MIN_WEAPON_ATTACK_RANGE			128
#define	MIN_WEAPON_ATTACK_RANGE_SQR		( MIN_WEAPON_ATTACK_RANGE * MIN_WEAPON_ATTACK_RANGE )

#define	MIN_WEAPON_DESIRED_RANGE		256
#define	MIN_WEAPON_DESIRED_RANGE_SQR	( MIN_WEAPON_DESIRED_RANGE * MIN_WEAPON_DESIRED_RANGE )

//Taser
#define	MIN_TASER_ATTACK_RANGE			128
#define	MIN_TASER_ATTACK_RANGE_SQR		( MIN_TASER_ATTACK_RANGE * MIN_TASER_ATTACK_RANGE )

#define	MIN_TASER_DESIRED_RANGE			256
#define	MIN_TASER_DESIRED_RANGE_SQR		( MIN_TASER_DESIRED_RANGE * MIN_TASER_DESIRED_RANGE )

//Assimilate
#define	MIN_ASSIMILATE_DIST				38
#define	MIN_ASSIMILATE_DIST_SQR			( MIN_ASSIMILATE_DIST * MIN_ASSIMILATE_DIST )
										
#define	MAX_ASSIMILATE_DIST				64
#define	MAX_ASSIMILATE_DIST_SQR			( MAX_ASSIMILATE_DIST * MAX_ASSIMILATE_DIST )

#define	BORG_PRE_ATTACK_DELAY			500

/*
-------------------------
Borg_CheckOverwhelmed
-------------------------
*/

#define	MAX_SWARM_EASY		1
#define	MAX_SWARM_NORMAL	2
#define	MAX_SWARM_HARD		5

qboolean Borg_CheckOverwhelmed( void )
{
	int	numSurrounding = AI_GetGroupSize( NPC->enemy->currentOrigin, 48, NPC->client->playerTeam, NPC );
	int	max;

	switch ( g_spskill->integer )
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
Borg_ClearShot
-------------------------
*/

qboolean Borg_ClearShot( gentity_t *ent )
{
	vec3_t	start, end, mins;

	VectorCopy( NPC->currentOrigin, start );
	VectorCopy( ent->currentOrigin, end );

	VectorCopy( NPC->mins, mins );
	mins[2] = 4;

	trace_t	tr;

	gi.trace( &tr, start, mins, NPC->maxs, end, NPC->s.number, MASK_SHOT );

	if ( tr.fraction == 1.0f )
		return qtrue;

	if ( tr.entityNum == ent->s.number )
		return qtrue;

	return qfalse;
}

/*
-------------------------
Borg_Hunt
-------------------------
*/

void Borg_Hunt( float distance )
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

	//Adjust our attack debounce
	/*
	if ( distance > (128*128) ) 
		NPC->attackDebounceTime = level.time + BORG_PRE_ATTACK_DELAY;
	*/

	//Update our angles regardless
	NPC_UpdateAngles( qtrue, qtrue );
}

/*
-------------------------
Borg_Weapon_Fire
-------------------------
*/

void Borg_Weapon_Fire( qboolean standStill )
{
	if ( NPCInfo->weaponTime < level.time && NPC_CheckCanAttackExt() )
	{
		ucmd.buttons |= BUTTON_ATTACK;
		
		NPC_ApplyWeaponFireDelay();

		//Set up delays
		if ( NPC->spawnflags & SFB_TASER )
		{
			NPCInfo->weaponTime = level.time + 2000;	//Time to do any attack again		
			
			if ( standStill )
				NPCInfo->standTime = level.time + 2000;	//Exact length
		}
		else
		{
			NPCInfo->weaponTime = level.time + 1700;	//Time to do any attack again		

			if ( standStill )
				NPCInfo->standTime = level.time + 1700;	//Exact length
		}
	}
}

/*
-------------------------
Borg_Backup
-------------------------
*/

void Borg_Backup( void )
{
	if ( NPCInfo->standTime < level.time )
	{
		NPCInfo->goalEntity = NPC->enemy;
		NPCInfo->goalRadius = 12;
		NPC_MoveToGoal();

		ucmd.forwardmove *= -1;
		ucmd.rightmove *= -1;
		//ucmd.buttons |= BUTTON_WALKING;
	}

	NPC_UpdateAngles( qtrue, qtrue );
}

/*
-------------------------
NPC_BSBorg_Weapon_Attack
-------------------------
*/

void NPC_BSBorg_Weapon_Attack( void )
{
	//If we don't have an enemy, just idle
	if ( NPC_CheckEnemyExt() == qfalse )
	{
		NPC_BSBorg_Idle();
		return;
	}

	//Rate our distance to the target, and our visibilty
	float		distance	= (int) DistanceSquared( NPC->currentOrigin, NPC->enemy->currentOrigin );	
	int			visRate		= NPC_ClearShot( NPC->enemy );

	//If we cannot see our target, move to see it
	if ( visRate == qfalse )
	{
		Borg_Hunt( distance );
		return;
	}

	//Player is too close, attack and back up
	if ( distance < MIN_WEAPON_ATTACK_RANGE_SQR )
	{
		Borg_Backup();

		if ( NPC_FaceEnemy() )
		{
			Borg_Weapon_Fire( qfalse );
		}

		return;
	}

	//We're too far, attack and move up
	if ( distance > MIN_WEAPON_DESIRED_RANGE_SQR )
	{
		Borg_Hunt( distance );	
		
		navInfo_t	info;

		NAV_GetLastMove( info );

		//Don't fire if we're following waypoints to our goal
		if ( ( info.flags & NIF_MACRO_NAV ) == 0 )
			Borg_Weapon_Fire( qfalse );

		return;
	}

	//Just right, stand and fire
	if ( NPC_FaceEnemy() )
	{
		Borg_Weapon_Fire( qfalse );
	}

	NPC_UpdateAngles( qtrue, qtrue );
}

/*
-------------------------
NPC_BSBorg_Taser_Attack
-------------------------
*/

void NPC_BSBorg_Taser_Attack( void )
{
	//If we don't have an enemy, just idle
	if ( NPC_CheckEnemyExt() == qfalse )
	{
		NPC_BSBorg_Idle();
		return;
	}

	//Rate our distance to the target, and our visibilty
	float		distance	= (int) DistanceSquared( NPC->currentOrigin, NPC->enemy->currentOrigin );	
	int			visRate		= NPC_ClearShot( NPC->enemy );

	//If we cannot see our target, move to see it
	if ( visRate == qfalse )
	{
		Borg_Hunt( distance );
		return;
	}

	//Player is too close, attack and back up
	if ( distance < MIN_TASER_ATTACK_RANGE_SQR )
	{
		Borg_Backup();
		Borg_Weapon_Fire( qfalse );
		return;
	}

	//We're too far, attack and move up
	if ( distance > MIN_TASER_DESIRED_RANGE_SQR )
	{
		navInfo_t	info;

		Borg_Hunt( distance );
		NAV_GetLastMove( info );

		//Don't fire if we're following waypoints to our goal
		if ( ( info.flags & NIF_MACRO_NAV ) == 0 )
			Borg_Weapon_Fire( qfalse );

		return;
	}

	//Just right, stand and fire
	if ( NPC_FaceEnemy() )
	{
		Borg_Weapon_Fire( qtrue );
	}

	NPC_UpdateAngles( qtrue, qtrue );
}

/*
-------------------------
NPC_BSBorg_Assimilate
-------------------------
*/

void NPC_BSBorg_Assimilate( void )
{
	//If we don't have an enemy, just idle
	if ( NPC_CheckEnemyExt() == qfalse )
	{
		NPC_BSBorg_Idle();
		return;
	}

	//Rate our distance to the target, and our visibilty
	float		distance	= (int) DistanceSquared( NPC->currentOrigin, NPC->enemy->currentOrigin );	
	int			visRate		= Borg_ClearShot( NPC->enemy );

	//Make sure we don't flood the player
	if ( Borg_CheckOverwhelmed() )
	{
		if ( distance > (64*64) )
		{
			if ( distance > (128*128) )
			{
				Borg_Hunt( distance );
			}

			NPC_UpdateAngles( qtrue, qtrue );
			return;
		}
	}

	//If we cannot see our target, move to see it
	if ( visRate == qfalse )
	{
		Borg_Hunt( distance );
		AI_CheckEnemyCollision( NPC );

		return;
	}

	//Must be within attack range
	if ( distance > MIN_ASSIMILATE_DIST_SQR )
	{
		Borg_Hunt( distance );
		
		if ( distance > MAX_ASSIMILATE_DIST_SQR )
		{
			NPC_UpdateAngles( qtrue, qtrue );
			return;
		}
	}

	//Start assimilating!
	if ( NPC_FaceEnemy() )
	{
		//FIXME: sounds play too soon, need to use fireDelay
		ucmd.buttons |= BUTTON_ATTACK;
	}

	NPC_UpdateAngles( qtrue, qtrue );
}

/*
-------------------------
NPC_BSBorg_Attack
-------------------------
*/

void NPC_BSBorg_Attack( void )
{
	if ( NPC->painDebounceTime > level.time )
	{
		NPC_UpdateAngles( qtrue, qtrue );
		return;
	}

	if ( NPC->spawnflags & SFB_GUN )
	{
		NPC_BSBorg_Weapon_Attack();
		return;
	}
	
	if ( NPC->spawnflags & SFB_TASER )
	{
		NPC_BSBorg_Taser_Attack();
		return;
	}

	NPC_BSBorg_Assimilate();
	return;		
}

/*
-------------------------
NPC_BSBorg_Idle
-------------------------
*/

void NPC_BSBorg_Idle( void )
{
	NPC_BSIdle();
}

/*
-------------------------
NPC_BSBorg_StandAndShoot
-------------------------
*/

void NPC_BSBorg_StandAndShoot( void )
{
	if ( NPC->painDebounceTime > level.time )
	{
		NPC_UpdateAngles( qtrue, qtrue );
		return;
	}

	//If we don't have an enemy, just idle
	if ( NPC_CheckEnemyExt() == qfalse )
	{
		NPC_BSBorg_Idle();
		return;
	}

	//Just face and fire
	if ( NPC_FaceEnemy() )
	{		
		Borg_Weapon_Fire( qfalse );
	}

	NPC_UpdateAngles( qtrue, qtrue );
}
