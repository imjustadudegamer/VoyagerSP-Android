//The Vorhsoth

#include "AI.h"
#include "anims.h"
#include "b_local.h"
#include "g_functions.h"

extern void FX_GroundTendrilSpawner( vec3_t start, vec3_t end );
extern void FX_ChestBeamSpawner( vec3_t origin, float height );
extern void FX_Vohrsoth_CreateGroundWarning( vec3_t origin );
extern void CG_DrawEdge( vec3_t start, vec3_t end, int type );
extern void PM_SetLegsAnimTimer( gentity_t *ent, int *legsAnimTimer, int time );
extern void G_SoundOnEnt( gentity_t *ent, soundChannel_t channel, const char *soundPath );

//Attack state enums
enum 
{
	VLS_QUAD_LOBBER = 1,
	VLS_TENDRIL,
	VLS_ROCKET_BURST,
	VLS_CHEST_BEAM,
};

//Attack times
const int QUAD_LOBBER_TIME	= 4100;
const int CHEST_BEAM_TIME	= 8000;
const int TENDRIL_TIME		= 8100;
const int ROCKET_BURST_TIME	= 4100;

/*
-------------------------
Vohrsoth_GetEnemyPosition
-------------------------
*/

static void Vohrsoth_GetEnemyPosition( vec3_t origin )
{
	vec3_t	end;

	VectorCopy( NPC->enemy->currentOrigin, end );

	end[2] -= 100;

	trace_t	tr;

	gi.trace( &tr, NPC->enemy->currentOrigin, NULL, NULL, end, NPC->enemy->s.number, MASK_SOLID );

	VectorCopy( tr.endpos, origin );
}

/*
-------------------------
tendril_think
-------------------------
*/

void tendril_think( gentity_t *self )
{
	//Explode with effect
	ExplodeDeath( self );
}

/*
-------------------------
Vohrsoth_CreateTendril
-------------------------
*/

#define	TENDRIL_DELAY	1000

static void Vohrsoth_CreateTendril( vec3_t origin )
{
	vec3_t	pos;

	//Create an effect to mark the position
	Vohrsoth_GetEnemyPosition( pos );
	FX_Vohrsoth_CreateGroundWarning( pos );

	//Spawn the entity
	gentity_t	*ent = G_Spawn();

	if ( ent == NULL )
	{
		gi.Printf("WARNING: Unable to spawn ground tendril\n");
		return;
	}

	//Make it invisible
	ent->s.solid = 0;
	ent->contents = 0;
	ent->clipmask = 0;
	ent->svFlags |= SVF_NOCLIENT;
	ent->s.eFlags |= EF_NODRAW;
	ent->count = 0;
	ent->classname = "tendril";
	ent->splashDamage = 100;
	ent->splashRadius = 64;
	ent->spawnflags |= 4;

	G_SetOrigin( ent, pos );
	VectorSet( ent->s.angles, 0, 90, 0 );

	ent->sounds = G_SoundIndex( va( "sound/weapons/explosions/explode%d.wav", Q_irand( 1, 4 ) ) );

	ent->e_ThinkFunc = thinkF_ExplodeDeath;
	ent->nextthink = level.time + TENDRIL_DELAY;

	G_SoundOnEnt( ent, CHAN_WEAPON, "sound/enemies/vorhsoth/beamhit.wav" );
}

/*
-------------------------
Vohrsoth_Tendril
-------------------------
*/

static void Vohrsoth_Tendril( void )
{
	//If our enemy is dead, don't do anything
	if ( NPC->enemy && NPC->enemy->health <= 0 )
		return;

	if ( TIMER_Done( NPC, "tendrilDelay" ) == qfalse )
		return;

	Vohrsoth_CreateTendril( NPC->enemy->currentOrigin );

	TIMER_Set( NPC, "tendrilDelay", 1000 );

	if ( TIMER_Done( NPC, "tendrilDrawDelay" ) == qfalse )
		return;

	vec3_t	start, end, forward;

	AngleVectors( NPC->currentAngles, forward, NULL, NULL );

	VectorMA( NPC->currentOrigin, 80, forward, start );

	VectorCopy( start, end );

	start[2] -= 24;
	end[2] += 48;

	FX_GroundTendrilSpawner( start, end );
	G_SoundOnEnt( NPC, CHAN_WEAPON, "sound/enemies/vorhsoth/beamfire.wav" );

	TIMER_Set( NPC, "tendrilDrawDelay", 99999999 );
}

/*
-------------------------
Vohrsoth_CreateLob
-------------------------
*/

static void Vohrsoth_CreateLob( vec3_t start, vec3_t end )
{
	vec3_t	dir;

	//Setup the direction
	VectorSubtract( end, start, dir );
	VectorNormalize( dir );

	gentity_t	*bolt;

	bolt = G_Spawn();

	if ( bolt == NULL )
		return;

	bolt->classname = "vorh_lob";
	
	bolt->nextthink = level.time + 10000;
	bolt->e_ThinkFunc = thinkF_G_FreeEntity;

	bolt->s.eType = ET_MISSILE;
	bolt->svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_FORGE_PROJ;	//FIXME: For now
	bolt->owner = NPC;
	bolt->damage = 8 * 12;
	bolt->dflags = 0;
	bolt->splashDamage = 0;
	bolt->splashRadius = 0;
	bolt->methodOfDeath = MOD_SCAVENGER;
	bolt->clipmask = MASK_SHOT;

	// There are going to be a couple of different sized projectiles, so store 'em here
	bolt->count = 1;

	VectorSet( bolt->maxs, 2.0f, 2.0f, 2.0f );
	VectorScale( bolt->maxs, -8, bolt->mins );
	VectorScale( bolt->maxs, 8, bolt->maxs );

	bolt->s.pos.trType = TR_LINEAR;
	bolt->s.pos.trTime = level.time;
	VectorCopy( start, bolt->s.pos.trBase );
	
	VectorScale( dir, 700, bolt->s.pos.trDelta );
	
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->currentOrigin);
	// Used by trails
	VectorCopy (start, bolt->pos1 );
	VectorCopy (start, bolt->pos2 );

	G_SoundOnEnt( NPC, CHAN_WEAPON, "sound/weapons/explosions/fireball.wav" );
}

/*
-------------------------
Vohrsoth_QuadLobber
-------------------------
*/

const vec3_t lobAttackOffset[4] =
{
	{ 200, -90, 48 },
	{ 160, 70, 35 },
	{ 120, 40, 80 },
	{ 160, -75, 90 },
};

const int lobAttackDelay[4] =
{
	700,
	400,
	600,
	500
};

static void Vohrsoth_QuadLobber( void )
{
	//If our enemy is dead, don't do anything
	if ( NPC->enemy && NPC->enemy->health <= 0 )
		return;

	//See if we're ready to lob another
	if ( TIMER_Done( NPC, "lobDelay" ) == qfalse )
		return;

	if ( NPCInfo->squadState > 3 )
		return;

	vec3_t	start, forward, right;

	AngleVectors( NPC->currentAngles, forward, right, NULL );

	VectorMA( NPC->currentOrigin, lobAttackOffset[NPCInfo->squadState][0], forward, start );
	VectorMA( start, lobAttackOffset[NPCInfo->squadState][1], right, start );
	start[2] += lobAttackOffset[NPCInfo->squadState][2];

	Vohrsoth_CreateLob( start, NPC->enemy->currentOrigin );

	NPCInfo->squadState++;
	TIMER_Set( NPC, "lobDelay", lobAttackDelay[NPCInfo->squadState] );
}

/*
-------------------------
Vohrsoth_ChestBeam
-------------------------
*/

static void Vohrsoth_ChestBeam( void )
{
}

/*
-------------------------
Vohrsoth_Rocket
-------------------------
*/

static void Vohrsoth_Rocket( vec3_t start, vec3_t forward )
{
	gentity_t	*bolt;

	bolt = G_Spawn();

	VectorSet( bolt->mins, -4, -4, -4 );
	VectorSet( bolt->maxs, 4, 4, 4 );

	bolt->classname = "bot_rocket";
	bolt->nextthink = level.time + 200;
	bolt->e_ThinkFunc = thinkF_bot_rocket_think;
	bolt->s.eType = ET_MISSILE;
	bolt->svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_BOT_ROCKET;
	bolt->owner = NPC;
	bolt->damage = 12;
	bolt->dflags = 0;
	bolt->splashDamage = 8;
	bolt->splashRadius = 64;
	bolt->methodOfDeath = MOD_BOTROCKET;
	bolt->splashMethodOfDeath = MOD_BOTROCKET_SPLASH;
	bolt->clipmask = MASK_SHOT;
	bolt->contents = CONTENTS_SOLID;
	bolt->takedamage = qtrue;
	bolt->health = 10;
	bolt->e_DieFunc  = dieF_bot_rocket_die;

	//Mark that this shouldn't account for FOV
	bolt->spawnflags |= 2;

	bolt->s.pos.trType = TR_LINEAR;
	bolt->s.pos.trTime = level.time;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	VectorScale( forward, 400 + ( crandom() * 100 ), bolt->s.pos.trDelta );
	VectorCopy( forward, bolt->movedir );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy( start, bolt->currentOrigin );
	VectorCopy( start, bolt->pos1 );

	gi.linkentity( bolt );

	G_SoundOnEnt( NPC, CHAN_WEAPON, "sound/weapons/hunter_seeker/fire.wav" );
}

/*
-------------------------
Vohrsoth_RocketBurst
-------------------------
*/

const vec3_t rocketAttackOffset[4] =
{
	{ 75, 0, 80 },
	{ 90, 0, 85 },
	{ 110, 0, 75 },
	{ 120, 0, 70 },
};

static void Vohrsoth_RocketBurst( void )
{
	//If our enemy is dead, don't do anything
	if ( NPC->enemy && NPC->enemy->health <= 0 )
		return;

	if ( TIMER_Done( NPC, "rocketDelay" ) == qfalse )
		return;

	vec3_t	start, forward, right, dir;

	AngleVectors( NPC->currentAngles, forward, right, NULL );

	for ( int i = 0; i < 4; i++ )
	{
		VectorMA( NPC->currentOrigin, rocketAttackOffset[i][0], forward, start );
		VectorMA( start, rocketAttackOffset[i][1], right, start );
		start[2] += rocketAttackOffset[i][2];

		VectorSubtract( start, NPC->currentOrigin, dir );
		VectorNormalize( dir );

		Vohrsoth_Rocket( start, dir );
	}

	TIMER_Set( NPC, "rocketDelay", 500000 );
}


/*
-------------------------
Vohrsoth_CheckEnemy
-------------------------
*/

static void Vohrsoth_CheckEnemy( void )
{
	gentity_t	*target = &g_entities[0];	//FIXME: If you want something else, put it here

	NPC->enemy = target;
}

/*
-------------------------
Vohrsoth_StartAttack
-------------------------
*/

static void Vohrsoth_StartAttack( int attack )
{
	int	animID;
	int	timeOfs;

	switch ( attack )
	{
	case VLS_QUAD_LOBBER:
		animID	= BOTH_ATTACK1;
		timeOfs = QUAD_LOBBER_TIME;
		NPCInfo->squadState = 0;
		TIMER_Set( NPC, "lobDelay", lobAttackDelay[0] );
		break;

	case VLS_CHEST_BEAM:
		animID	= BOTH_ATTACK2;
		timeOfs = CHEST_BEAM_TIME;
		break;

	case VLS_TENDRIL:
		animID	= BOTH_ATTACK4;
		timeOfs = TENDRIL_TIME;
		TIMER_Set( NPC, "tendrilDelay", 2500 );
		TIMER_Set( NPC, "tendrilDrawDelay", 2500 );
		break;

	case VLS_ROCKET_BURST:
	default:
		animID	= BOTH_ATTACK3;
		timeOfs = ROCKET_BURST_TIME;
		TIMER_Set( NPC, "rocketDelay", 800 );
		break;
	}

	//Setup the information
	NPC_SetAnim( NPC, SETANIM_BOTH, animID, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD );
	TIMER_Set( NPC, "attackDelay", timeOfs );
	NPCInfo->localState = attack;
}

/*
-------------------------
NPC_BSVohrsoth_Attack
-------------------------
*/

void NPC_BSVohrsoth_Attack( void )
{
	//If we're dead, stop what we're doing
	if ( NPC->health <= 0 )
		return;

	//Check for enemies, or loss of them
	Vohrsoth_CheckEnemy();

	//If our enemy is dead, don't do anything
	if ( NPC->enemy && NPC->enemy->health <= 0 )
		return;

	//See if we're ready to attack again
	if ( TIMER_Done( NPC, "attackDelay" ) )
	{	
		Vohrsoth_StartAttack( Q_irand( VLS_QUAD_LOBBER, VLS_ROCKET_BURST ) );
	}

	//Decide which attack state we're in and update anything we need to
	switch ( NPCInfo->localState )
	{
	case VLS_QUAD_LOBBER:
		Vohrsoth_QuadLobber();
		break;

	case VLS_CHEST_BEAM:
		Vohrsoth_ChestBeam();
		break;

	case VLS_TENDRIL:
		Vohrsoth_Tendril();
		break;

	case VLS_ROCKET_BURST:
		Vohrsoth_RocketBurst();
		break;
	}
}
