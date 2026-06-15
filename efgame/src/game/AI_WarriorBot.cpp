// Warrior Bot

#include "b_local.h"
#include "anims.h"

extern void CGCam_Shake( float intensity, int duration );

/*
-------------------------
NPC_BSWarrior_Idle
-------------------------
*/

void NPC_BSWarrior_Idle( void )
{
	NPC_BSIdle();
}

/*
-------------------------
Warrior_MeleeDamage
-------------------------
*/

#define	MELEE_DISTANCE	100

#define	WAR_DAMAGE_EASY		10
#define	WAR_DAMAGE_NORMAL	30
#define	WAR_DAMAGE_HARD		50

void Warrior_MeleeDamage( void )
{
	vec3_t	end, dir, mins, maxs;
	trace_t	trace;

	AngleVectors( NPC->currentAngles, dir, NULL, NULL );
	VectorMA( NPC->currentOrigin, MELEE_DISTANCE, dir, end );

	VectorSet( maxs,  4,  4,  4 );
	VectorSet( mins, -4, -4, -4 );

	gi.trace( &trace, NPC->currentOrigin, mins, maxs, end, NPC->s.number, MASK_SHOT );

	if ( trace.entityNum < ENTITYNUM_WORLD )
	{
		gentity_t	*traceEnt = &g_entities[trace.entityNum];

		int	damage;

		if ( g_spskill->integer == 0 )
			damage = WAR_DAMAGE_EASY;
		else if ( g_spskill->integer == 1 )
			damage = WAR_DAMAGE_NORMAL;
		else
			damage = WAR_DAMAGE_HARD;

		G_Damage( traceEnt, NPC, NPC, dir, trace.endpos, damage, 0, MOD_PHASER );
		G_Sound ( NPC, G_SoundIndex ("sound/enemies/warbot/stabhit.wav"));
	}
}

/*
-------------------------
Warrior_Melee
-------------------------
*/

void Warrior_Melee( void )
{
	if ( TIMER_Done( NPC, "preDelay" ) )
	{
		Warrior_MeleeDamage();

		//Set the timer ahead, we should never reach this again without first doing something else
		TIMER_Set( NPC, "preDelay", 50000 );
	}

	NPC_UpdateAngles( qtrue, qtrue );
}

/*
-------------------------
Warrior_Hunt
-------------------------
*/

extern animFileSet_t	knownAnimFileSets[MAX_ANIM_FILES];

static void Warrior_Hunt( void )
{
	NPCInfo->combatMove = qtrue;

	animation_t *animations = knownAnimFileSets[NPC->client->clientInfo.animFileIndex].animations;

	if ( ( NPC->client->renderInfo.legsFrame == (animations[BOTH_RUN1].firstFrame) ) || ( NPC->client->renderInfo.legsFrame == (animations[BOTH_RUN1].firstFrame)+3 ) )
	{
		vec3_t	dir;
		float	dist, intensityScale;
		float	realIntensity;

		VectorSubtract( NPC->currentOrigin, NPC->enemy->currentOrigin, dir );
		dist = VectorNormalize( dir );

		//Use the dir to add kick to the explosion

		if ( dist < 512 )
		{
			intensityScale = 1 - ( dist / 512.0f );
			realIntensity = 1.0f * intensityScale;

			CGCam_Shake( realIntensity, 150 );
		}
	}

	//If we're not supposed to stand still, pursue the player
	if ( NPCInfo->standTime < level.time )
	{
		if ( NPC->health < 50 )
		{
			ucmd.buttons |= (BUTTON_WALKING|BUTTON_CAREFUL);
		}

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

/*
-------------------------
NPC_BSWarrior_Attack
-------------------------
*/

#define	MIN_MELEE_RANGE		84
#define	MIN_MELEE_RANGE_SQR	( MIN_MELEE_RANGE * MIN_MELEE_RANGE )

void NPC_BSWarrior_Attack( void )
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
		NPC_BSWarrior_Idle();
		return;
	}

	//If we're in an attack, keep checking for the actual strike point
	if ( NPC->attackDebounceTime > level.time )
	{
		Warrior_Melee();
		return;
	}

	//Rate our distance to the target, and our visibilty
	float		distance	= (int) DistanceHorizontalSquared( NPC->currentOrigin, NPC->enemy->currentOrigin );	
	distance_e	distRate	= ( distance > MIN_MELEE_RANGE_SQR ) ? DIST_LONG : DIST_MELEE;
	int			visRate		= NPC_ClearLOS( NPC->enemy );

	//If we cannot see our target, move to see it
	if ( visRate == qfalse )
	{
		Warrior_Hunt();
		return;
	}

	//If we're too far away, then keep walking forward
	if ( distRate != DIST_MELEE )
	{
		Warrior_Hunt();
		return;
	}

	//Start an attack
	if ( NPC_FaceEnemy( qtrue ) )
	{
		TIMER_Set( NPC, "preDelay", 500 );
		NPC->attackDebounceTime = level.time + 1000;
		NPC_SetAnim( NPC, SETANIM_BOTH, Q_irand( BOTH_MELEE1, BOTH_MELEE5 ), SETANIM_FLAG_HOLD|SETANIM_FLAG_OVERRIDE );
		Warrior_Melee();
		return;
	}
	
	NPC_UpdateAngles( qtrue, qtrue );
}