//Scout Bot

#include "b_local.h"
#include "anims.h"

/*
-------------------------
NPC_BSScout_Idle
-------------------------
*/

void NPC_BSScout_Idle( void )
{
	//TODO: Implement
	NPC_BSIdle();
}

/*
-------------------------
Scout_Hunt
-------------------------
*/

static void Scout_Hunt( void )
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

	//Update our angles regardless
	NPC_UpdateAngles( qtrue, qtrue );
}

//Don't get too far away
#define	MAX_SCOUTBOT_RANGE		512
#define	MAX_SCOUTBOT_RANGE_SQR	( MAX_SCOUTBOT_RANGE * MAX_SCOUTBOT_RANGE )

#define	MIN_SCOUTBOT_RANGE		128
#define	MIN_SCOUTBOT_RANGE_SQR	( MIN_SCOUTBOT_RANGE * MIN_SCOUTBOT_RANGE )

/*
-------------------------
Scout_Ranged
-------------------------
*/

static void Scout_Ranged( float distance )
{
	//See if we're too far
	if ( distance > MAX_SCOUTBOT_RANGE_SQR )
	{
		Scout_Hunt();
	}

	//See if we're too close
	if ( distance < MIN_SCOUTBOT_RANGE_SQR )
	{
		if ( NPCInfo->standTime < level.time )
		{
			NPCInfo->goalEntity = NPC->enemy;
			NPCInfo->goalRadius = 12;
			NPC_MoveToGoal();

			ucmd.forwardmove *= -1;
			ucmd.rightmove *= -1;

			//Don't allow firing when backing up... sort of a strategy for these guys
			NPCInfo->weaponTime = level.time + 200;
		}

		NPC_UpdateAngles( qtrue, qtrue );
	}

	//See if we can fire again
	if ( NPCInfo->weaponTime < level.time )
	{
		//Try and hit the player
		if ( NPC_CheckCanAttackExt() )
		{
			WeaponThink( qtrue );
			
			if ( g_spskill->integer == 0 )
				NPCInfo->weaponTime = level.time + 1000;
			else if ( g_spskill->integer == 1 )
				NPCInfo->weaponTime = level.time + 750;
			else
				NPCInfo->weaponTime = level.time + 500;
		}
	}

	NPC_FaceEnemy();
	NPC_UpdateAngles( qtrue, qtrue );
}

/*
-------------------------
NPC_BSScout_Attack
-------------------------
*/

#define	MIN_MELEE_RANGE		64
#define	MIN_MELEE_RANGE_SQR	( MIN_MELEE_RANGE * MIN_MELEE_RANGE )

void NPC_BSScout_Attack( void )
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
		NPC_BSReaver_Idle();
		return;
	}

	//Rate our distance to the target, and our visibilty
	float		distance	= (int) DistanceHorizontalSquared( NPC->currentOrigin, NPC->enemy->currentOrigin );	
	int			visRate		= NPC_ClearShot( NPC->enemy );

	//If we cannot see our target, move to see it
	if ( visRate == qfalse )
	{
		Scout_Hunt();
		return;
	}

	Scout_Ranged( distance );
}