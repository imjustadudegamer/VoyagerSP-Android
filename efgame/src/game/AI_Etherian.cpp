//Etherians

#include "AI.h"
#include "b_local.h"
#include "g_nav.h"
#include "anims.h"

extern void FX_StasisCharge( vec3_t origin, vec3_t dir, float perc_done );
extern void PM_SetLegsAnimTimer( gentity_t *ent, int *legsAnimTimer, int time );

static void Etherian_Melee( void );

#define	IDLE_RANGE		128
#define	IDLE_RANGE_SQR	( IDLE_RANGE * IDLE_RANGE )

/*
-------------------------
Etherian_Stand
-------------------------
*/

static void Etherian_Stand( void )
{
	PM_SetLegsAnimTimer( NPC, &NPC->client->ps.legsAnimTimer, 0 );
	NPC_FaceEnemy();
	NPC_UpdateAngles( qtrue, qtrue );
}

/*
-------------------------
Etherian_ClearLOS
-------------------------
*/

#define	ETHERIAN_MELEE_OFS	28

static qboolean Etherian_ClearLOS( gentity_t *attacker, gentity_t *ent )
{
	trace_t		tr;
	
	if ( ( attacker == NULL ) || ( ent == NULL ) )
		return qfalse;

	vec3_t	start, end;

	VectorCopy( attacker->currentOrigin, start );
	start[2] += ETHERIAN_MELEE_OFS;

	VectorCopy( ent->currentOrigin, end );
	//end[2] += 8;

	gi.trace ( &tr, start, NULL, NULL, end, attacker->s.number, CONTENTS_SOLID|CONTENTS_BODY );

	if ( &g_entities[tr.entityNum] == ent )
		return qtrue;

	return qfalse;
}

/*
-------------------------
NPC_BSEtherian_Idle
-------------------------
*/

void NPC_BSEtherian_Idle( void )
{
	//Idle here
	NPC_BSIdle();
}

/*
-------------------------
Etherian_Hunt
-------------------------
*/

static void Etherian_Hunt( qboolean doMelee )
{
	if ( NPC->enemy == NULL )
	{
		assert( 0 );
		return;
	}

	if ( ( Etherian_ClearLOS( NPC, NPC->enemy ) == qfalse ) || ( Distance( NPC->currentOrigin, NPC->enemy->currentOrigin ) > 64 ) )
	{
		PM_SetLegsAnimTimer( NPC, &NPC->client->ps.legsAnimTimer, 0 );

		//If we're not supposed to stand still, pursue the player
		if ( NPCInfo->standTime < level.time )
		{
			//Move towards our goal
			NPCInfo->combatMove = qtrue;
			NPCInfo->goalEntity = NPC->enemy;
			NPCInfo->goalRadius = 12;
			NPC_MoveToGoal();
		}

		NPC_FaceEnemy();
		NPC_UpdateAngles( qtrue, qtrue );
		return;
	}

	//See if we want melee
	if ( doMelee == qfalse )
		return;

	//Update our angles regardless
	if ( NPC_FaceEnemy( qfalse ) )
	{
		NPC_UpdateAngles( qtrue, qtrue );

		if ( NPCInfo->weaponTime < level.time )
		{
			NPCInfo->weaponTime = level.time + 400;
			Etherian_Melee();
		}

		//Keep swinging
		NPC_SetAnim( NPC, SETANIM_BOTH, BOTH_MELEE1, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD );
		PM_SetLegsAnimTimer( NPC, &NPC->client->ps.legsAnimTimer, -1 );
	}
}

/*
-------------------------
Etherian_Charge
-------------------------
*/

#define CHARGE_FORWARD	14.0f
#define CHARGE_UP		24.0f

static void Etherian_Charge( void )
{
	vec3_t	org, forward, up = { 0.0f, 0.0f, 1.0f };
	
	NPC_FaceEnemy();
	NPC_UpdateAngles( qtrue, qtrue );

	// Move out in front of the alien, try to position roughly where hands will meet 
	AngleVectors( NPC->currentAngles, forward, NULL, NULL );
	VectorMA( NPC->currentOrigin, CHARGE_FORWARD, forward, org );
	VectorMA( org, CHARGE_UP, up, org );

	FX_StasisCharge( org, forward, ( 0.75f + ( random() * 0.25f ) ) );
}

/*
-------------------------
Etherian_Melee
-------------------------
*/

#define	ETHERIAN_MELEE_DAMAGE		4
#define	ETHERIAN_MELEE_RANGE		64
#define	ETHERIAN_MELEE_RANGE_SQR	( ETHERIAN_MELEE_RANGE * ETHERIAN_MELEE_RANGE )


static void Etherian_Melee( void )
{
	if ( NPC->enemy == NULL )
		return;

	//Must first be within a valid range
	if ( DistanceSquared( NPC->currentOrigin, NPC->enemy->currentOrigin ) <= ETHERIAN_MELEE_RANGE_SQR )
	{
		//FIXME: Could be redundant
		if ( Etherian_ClearLOS( NPC, NPC->enemy ) )
		{
			vec3_t	dir;
			VectorSubtract( NPC->enemy->currentOrigin, NPC->currentOrigin, dir );
			VectorNormalize( dir );

			G_Damage( NPC->enemy, NPC, NPC, dir, NPC->enemy->currentOrigin, ETHERIAN_MELEE_DAMAGE, DAMAGE_NO_KNOCKBACK, MOD_STASIS );
			Etherian_Charge();
		}
	}
}

/*
-------------------------
Etherian_AttemptFire
-------------------------
*/

static qboolean Etherian_AttemptFire( void )
{
	if ( NPCInfo->weaponTime > level.time )
		return qfalse;

	int	attackChance;
	int attackLimit  = 10;

	switch ( g_spskill->integer )
	{
	case 0:
		attackChance = 9;
		break;

	case 1:
		attackChance = 7;
		break;

	default:
	case 2:
		attackChance = 4;
		break;
	}

	return ( Q_irand( 0, attackLimit ) > attackChance );
}

/*
-------------------------
Etherian_Strafe
-------------------------
*/

void Etherian_Strafe( void )
{
	vec3_t	testpos, forward, right;
	int		dist = 64;
	trace_t	trace;

	//Test left for clear movement and LOS
	AngleVectors( NPC->currentAngles, forward, right, NULL );

	if ( Q_irand( 0, 1 ) )
		dist *= -1;

	VectorMA( NPC->currentOrigin, dist, right, testpos );

	NPCInfo->squadState = SQUAD_TRANSITION;

	//Check that move
	if ( NAV_CheckAhead( NPC, testpos, trace, NPC->clipmask ) == qfalse )
	{
		if ( trace.fraction > 0.5f )
		{
			NPC_SetMoveGoal( NPC, trace.endpos, 8, qtrue );
			return;
		}
		else
		{
			VectorMA( right, -dist, right, testpos );

			if ( NAV_CheckAhead( NPC, testpos, trace, NPC->clipmask ) == qfalse )
			{
				if ( trace.fraction > 0.5f )
				{
					NPC_SetMoveGoal( NPC, trace.endpos, 8, qtrue );
					return;
				}
			}

			NPC_SetMoveGoal( NPC, testpos, 8, qtrue );
			return;
		}
	}

	NPC_SetMoveGoal( NPC, testpos, 8, qtrue );
}

/*
-------------------------
Etherian_CheckMoveState
-------------------------
*/

qboolean Etherian_CheckMoveState( void )
{
	//If we're moving, continue to do so
	if ( ( NPCInfo->goalEntity != NPC->enemy ) && ( NPCInfo->goalEntity != NULL ) )
	{
		//Did we make it?
		if ( NAV_HitNavGoal( NPC->currentOrigin, NPC->mins, NPC->maxs, NPCInfo->goalEntity->currentOrigin, 16 ) )
		{
			TIMER_Set( NPC, "attackDelay", Q_irand( 750, 1250 ) );	//FIXME: Slant for difficulty levels
			TIMER_Set( NPC, "strafe", Q_irand( 1000, 2000 ) );
			NPCInfo->squadState = SQUAD_STAND_AND_SHOOT;
			NPCInfo->goalEntity = NULL;
			return qfalse;
		}

		//Keep running
		NPCInfo->squadState = SQUAD_TRANSITION;
		NPC_SlideMoveToGoal();
		NPC_FaceEnemy( qtrue );
		NPC_UpdateAngles( qtrue, qtrue );
		return qtrue;
	}

	return qfalse;
}

/*
-------------------------
Etherian_Ranged
-------------------------
*/

#define	MIN_MISSLE_DIST		300
#define	MIN_MISSLE_DIST_SQR ( MIN_MISSLE_DIST * MIN_MISSLE_DIST )

static void Etherian_Ranged( void )
{
	//If we're not ready to move again, just stay put
	if ( NPCInfo->standTime > level.time )
	{
		NPC_FaceEnemy( qtrue );
		NPC_UpdateAngles( qtrue, qtrue );
		return;
	}

	//If we can't see the target, move to do so
	if ( NPC_ClearShot( NPC->enemy ) == qfalse )
	{
		Etherian_Hunt( qfalse );
		return;
	}

	//See if we should close up the gap a little
	if ( DistanceSquared( NPC->currentOrigin, NPC->enemy->currentOrigin ) > MIN_MISSLE_DIST_SQR )
	{
		Etherian_Hunt( qfalse );
		return;
	}

	//See if we should fire at the player
	if ( Etherian_AttemptFire() )
	{
		ucmd.buttons |= BUTTON_ATTACK;
		NPC_ApplyWeaponFireDelay();

		NPCInfo->standTime = level.time + 1000;
		NPCInfo->weaponTime = level.time + Q_irand( 1000, 3000 );
	}

	NPC_FaceEnemy( qtrue );
	NPC_UpdateAngles( qtrue, qtrue );
}


/*
-------------------------
Etherian_GetAttackerThreshold
-------------------------
*/

#define	MAX_SWARM_EASY		1
#define	MAX_SWARM_NORMAL	3
#define	MAX_SWARM_HARD		5

static inline int Etherian_GetAttackerThreshold( void )
{
	if ( g_spskill->integer == 0 )
		return MAX_SWARM_EASY;

	if ( g_spskill->integer == 1 )
		return MAX_SWARM_NORMAL;

	return MAX_SWARM_HARD;
}

/*
-------------------------
Etherian_DistributeAttack
-------------------------
*/

static qboolean Etherian_DistributeAttack( void )
{
	int			threshold	= Etherian_GetAttackerThreshold();
	gentity_t	*enemy		= AI_DistributeAttack( NPC, NPC->enemy, NPC->client->playerTeam, threshold );	

	//If we have no new target, then we're waiting
	if ( enemy == NULL )
		return qfalse;

	//If the enemy is new, take it
	if ( enemy != NPC->enemy )
	{
		G_SetEnemy( NPC, enemy );
		return qtrue;
	}

	return qtrue;
}

/* 
-------------------------
NPC_BSEtherian_Attack
-------------------------
*/

#define	MIN_MELEE_RANGE		256
#define	MIN_MELEE_RANGE_SQR	( MIN_MELEE_RANGE * MIN_MELEE_RANGE )

void NPC_BSEtherian_Attack( void )
{
	//Don't do anything if we're hurt
	if ( NPC->painDebounceTime > level.time )
	{
		NPC_UpdateAngles( qtrue, qtrue );
		return;
	}

	//See if we were going anywhere
	if ( Etherian_CheckMoveState() )
		return;

	//If we don't have an enemy, just idle
	if ( NPC_CheckEnemyExt() == qfalse )
	{
		NPC_BSEtherian_Idle();
		return;
	}

	//Distribute our attack across the enemy team
	if ( Etherian_DistributeAttack() == qfalse )
	{
		NPC_FaceEnemy( qtrue );
		NPC_UpdateAngles( qtrue, qtrue );
		return;
	}

	//Different behavior for projectile lobbers
	if ( NPC->client->ps.weapon == WP_STASIS_ATTACK )
	{
		Etherian_Ranged();
		return;
	}

	//Otherwise check to see how many group members are in front of you
	AIGroupInfo_t	group;
	AI_GetGroup( group, NPC, 512 );

	//If 2 or more, then continue to wait
	if ( group.numFront >= 2 )
	{
		if ( DistanceSquared( NPC->currentOrigin, NPC->enemy->currentOrigin ) > IDLE_RANGE_SQR )
		{
			Etherian_Hunt( qtrue );
			return;
		}

		Etherian_Stand();
		return;
	}

	//Otherwise charge in and attack
	Etherian_Hunt( qtrue );
}