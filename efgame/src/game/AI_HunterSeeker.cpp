#include "b_local.h"
#include "g_nav.h"

/*
-------------------------
Hunter_MaintainHeight
-------------------------
*/

#define VELOCITY_DECAY	0.85f

void Hunter_MaintainHeight( void )
{	
	float	dif;
	vec3_t	endPos;
	trace_t	trace;

	// Update our angles regardless
	NPC_UpdateAngles( qtrue, qtrue );

	// If we have an enemy, we should try to hover at about enemy eye level
	if ( NPC->enemy )
	{
		// Find the height difference
		dif = NPC->enemy->currentOrigin[2] - NPC->currentOrigin[2] + 46; // ??magic number

		// cap to prevent dramatic height shifts
		if ( fabs( dif ) > 16 )
		{
			dif = ( dif < 0 ? -16 : 16 );
		}

		ucmd.upmove = dif * 3;
	}
	else
	{
		// Stay at a given height until we take on an enemy
		VectorSet( endPos, NPC->currentOrigin[0], NPC->currentOrigin[1], NPC->currentOrigin[2] - 512 );
		gi.trace( &trace, NPC->currentOrigin, NULL, NULL, endPos, NPC->s.number, MASK_SOLID );

		if ( trace.fraction != 1.0f )
		{
			float	length = ( trace.fraction * 512 );

			if ( length < 80 )
			{
				ucmd.upmove = 32;
			}
			else if ( length > 120 )
			{
				ucmd.upmove = -32;
			}
			else
			{
				if ( NPC->client->ps.velocity[2] )
				{
					NPC->client->ps.velocity[2] *= VELOCITY_DECAY;

					if ( fabs( NPC->client->ps.velocity[2] ) < 1 )
					{
						NPC->client->ps.velocity[2] = 0;
					}
				}
			}
		}
	}

	// Apply friction
	if ( NPC->client->ps.velocity[0] )
	{
		NPC->client->ps.velocity[0] *= VELOCITY_DECAY;

		if ( fabs( NPC->client->ps.velocity[0] ) < 1 )
		{
			NPC->client->ps.velocity[0] = 0;
		}
	}

	if ( NPC->client->ps.velocity[1] )
	{
		NPC->client->ps.velocity[1] *= VELOCITY_DECAY;

		if ( fabs( NPC->client->ps.velocity[1] ) < 1 )
		{
			NPC->client->ps.velocity[1] = 0;
		}
	}
}

/*
-------------------------
HunterSeeker_Strafe
-------------------------
*/

#define HUNTER_STRAFE_VEL	256
#define HUNTER_STRAFE_DIS	200
#define HUNTER_UPWARD_PUSH	32

void HunterSeeker_Strafe( void )
{
	int		dir;
	vec3_t	end, right;
	trace_t	tr;

	AngleVectors( NPC->client->renderInfo.eyeAngles, NULL, right, NULL );

	// Pick a random strafe direction, then check to see if doing a strafe would be
	//	reasonable valid
	dir = ( rand() & 1 ) ? -1 : 1;
	VectorMA( NPC->currentOrigin, HUNTER_STRAFE_DIS * dir, right, end );

	gi.trace( &tr, NPC->currentOrigin, NULL, NULL, end, NPC->s.number, MASK_SOLID );

	// Close enough
	if ( tr.fraction > 0.9f )
	{
		VectorMA( NPC->client->ps.velocity, HUNTER_STRAFE_VEL * dir, right, NPC->client->ps.velocity );

		// Add a slight upward push
		NPC->client->ps.velocity[2] += HUNTER_UPWARD_PUSH;

		// Set the strafe start time so we can do a controlled roll
		NPC->fx_time = level.time;
		NPCInfo->standTime = level.time + 3000 + random() * 500;
	}
}

/*
-------------------------
HunterSeeker_Hunt
-------------------------
*/

#define HUNTER_FORWARD_BASE_SPEED	10
#define HUNTER_FORWARD_MULTIPLIER	5

void HunterSeeker_Hunt( qboolean visible, qboolean advance )
{
	float	distance, speed;
	vec3_t	forward;

	//If we're not supposed to stand still, pursue the player
	if ( NPCInfo->standTime < level.time )
	{
		// Only strafe when we can see the player
		if ( visible )
		{
			HunterSeeker_Strafe();
			return;
		}
	}

	//If we don't want to advance, stop here
	if ( advance == qfalse )
		return;

	//Only try and navigate if the player is visible
	if ( visible == qfalse )
	{
		// Move towards our goal
		NPCInfo->goalEntity = NPC->enemy;
		NPCInfo->goalRadius = 12;

		//Get our direction from the navigator if we can't see our target
		if ( NPC_GetMoveDirection( forward, &distance ) == qfalse )
			return;
	}
	else
	{
		VectorSubtract( NPC->enemy->currentOrigin, NPC->currentOrigin, forward );
		distance = VectorNormalize( forward );
	}

	speed = HUNTER_FORWARD_BASE_SPEED + HUNTER_FORWARD_MULTIPLIER * g_spskill->integer;
	VectorMA( NPC->client->ps.velocity, speed, forward, NPC->client->ps.velocity );
}

/*
-------------------------
HunterSeeker_Melee
-------------------------
*/

void HunterSeeker_Melee( qboolean visible, qboolean advance )
{
	// Make sure that if we are switching weapon states that the burstCount gets cleared.
	//	Otherwise, it could easily be using the burstCount from the other weapon attack
	if ( !NPC->trigger_formation )
	{
		NPCInfo->burstCount = 0;
		NPC->trigger_formation = qtrue;
	}

	// Attempt to fire off a shot at the player
	if ( NPCInfo->weaponTime < level.time )
	{
		// Burst count is based off of skill setting
		NPCInfo->burstCount = Q_irand( 2, 4 ) * ( g_spskill->integer + 1);
		NPCInfo->weaponTime = level.time + 5000;	// Time to do any attack again
	}
	else if ( NPCInfo->burstCount && ( NPCInfo->pauseTime < level.time ) )
	{
		// Fire
		ucmd.buttons |= BUTTON_ALT_ATTACK;

		// Setup delays
		NPCInfo->pauseTime = level.time + ( 3 - g_spskill->integer ) * 100;		// Time to fire a ranged shot again
		NPCInfo->burstCount--;
		NPC->count = !NPC->count;					// Alternate the muzzle
	}

	HunterSeeker_Hunt( visible, advance );
}

/*
-------------------------
HunterSeeker_Ranged
-------------------------
*/

void HunterSeeker_Ranged( qboolean visible, qboolean advance )
{
	// Make sure that if we are switching weapon states that the burstCount gets cleared.
	//	Otherwise, it could easily be using the burstCount from the other weapon attack
	if ( NPC->trigger_formation )
	{
		NPCInfo->burstCount = 0;
		NPC->trigger_formation = qfalse;
	}

	// Attempt to fire off a burst of rockets at the player
	if ( ( Q_irand( 0, 10 ) > 8 ) && ( NPCInfo->weaponTime < level.time ) )
	{
		// Burst count is based off of skill setting
		NPCInfo->burstCount = (g_spskill->integer%3) + 1; 
		NPCInfo->weaponTime = level.time + 5000;	// Time to do any attack again
	}
	else if ( NPCInfo->burstCount && ( NPCInfo->pauseTime < level.time ) )
	{
		// Fire
		ucmd.buttons |= BUTTON_ATTACK;

		// Setup delays
		NPCInfo->pauseTime = level.time + 200 + random() * 100;		// Time to fire the next rocket in the burst
		NPCInfo->burstCount--;
	}

	HunterSeeker_Hunt( visible, advance );
}

/*
-------------------------
NPC_BSHunterSeeker_Attack
-------------------------
*/

#define	MIN_MELEE_RANGE		320
#define	MIN_MELEE_RANGE_SQR	( MIN_MELEE_RANGE * MIN_MELEE_RANGE )

#define MIN_DISTANCE		128
#define MIN_DISTANCE_SQR	( MIN_DISTANCE * MIN_DISTANCE )

void NPC_BSHunterSeeker_Attack( void )
{
	// Always keep a good height off the ground
	Hunter_MaintainHeight();

	// If we don't have an enemy, just idle
	if ( NPC_CheckEnemyExt() == qfalse )
	{
		NPC_BSHunterSeeker_Idle();
		return;
	}

	// Rate our distance to the target, and our visibilty
	float		distance	= (int) DistanceHorizontalSquared( NPC->currentOrigin, NPC->enemy->currentOrigin );	
	distance_e	distRate	= ( distance > MIN_MELEE_RANGE_SQR ) ? DIST_LONG : DIST_MELEE;
	qboolean	visible		= NPC_ClearLOS( NPC->enemy );
	qboolean	advance		= (qboolean)(distance > MIN_DISTANCE_SQR);

	// If we cannot see our target, move to see it
	if ( visible == qfalse )
	{
		HunterSeeker_Hunt( visible, advance );
		return;
	}

	// Sometimes I have problems with facing the enemy I'm attacking, so force the issue so I don't look dumb
	NPC_FaceEnemy( qtrue );

	// Decide what type of attack to do
	switch ( distRate )
	{
	case DIST_MELEE:
		HunterSeeker_Melee( visible, advance );
		break;

	case DIST_LONG:
		HunterSeeker_Ranged( visible, advance );
		break;
	}
}

/*
-------------------------
NPC_BSHunterSeeker_Idle
-------------------------
*/

void NPC_BSHunterSeeker_Idle( void )
{
	Hunter_MaintainHeight();

	NPC_BSIdle();
}

/*QUAKED NPC_HunterSeeker(1 0 0) (-10 -10 0) (10 10 20) x x x x DROPTOFLOOR SILENTSPAWN NOTSOLID STARTINSOLID SHY
*/

extern void SP_NPC_spawner( gentity_t *self );

void SP_NPC_HunterSeeker( gentity_t *self )
{
	//FIXME: make an index into an external string table for localization
	if (g_language && Q_stricmp("DEUTSCH",g_language->string)==0)
	{
		self->fullName = "Fremdartige Lebensform";
	}
	else
	{
		self->fullName = "Alien Lifeform";
	}

	if(!self->NPC_type)
		self->NPC_type = "hunterseeker";

	// For some reason, they don't always have their weapon precached unless you do this right away?
	RegisterItem( FindItemForWeapon( WP_BOT_ROCKET ) );

	SP_NPC_spawner( self );
}
