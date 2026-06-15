
#include "g_local.h"
#include "g_functions.h"
#include "objectives.h"
#include "g_infostrings.h"

#define MOVER_START_ON		1
#define MOVER_CRUSHER		4
#define MOVER_TOGGLE		8
#define MOVER_MUST_FACE		16
#define MOVER_SECURITY		32
#define MOVER_PLAYER_USE	128

int	BMS_START = 0;
int	BMS_MID = 1;
int	BMS_END = 2;

extern void G_SetEnemy( gentity_t *self, gentity_t *enemy );
/*
===============================================================================

PUSHMOVE

===============================================================================
*/

void MatchTeam( gentity_t *teamLeader, int moverState, int time );
extern qboolean G_BoundsOverlap(const vec3_t mins1, const vec3_t maxs1, const vec3_t mins2, const vec3_t maxs2);

typedef struct {
	gentity_t	*ent;
	vec3_t	origin;
	vec3_t	angles;
	float	deltayaw;
} pushed_t;
pushed_t	pushed[MAX_GENTITIES], *pushed_p;


/*
-------------------------
G_SetLoopSound
-------------------------
*/

void G_PlayDoorLoopSound( gentity_t *ent )
{
	if ( VALIDSTRING( ent->soundSet ) == false )
		return;

	sfxHandle_t	sfx = CAS_GetBModelSound( ent->soundSet, BMS_MID );

	if ( sfx == -1 )
	{
		ent->s.loopSound = 0;
		return;
	}

	ent->s.loopSound = sfx;
}

/*
-------------------------
G_PlayDoorSound
-------------------------
*/

void G_PlayDoorSound( gentity_t *ent, int type )
{
	if ( VALIDSTRING( ent->soundSet ) == false )
		return;

	sfxHandle_t	sfx = CAS_GetBModelSound( ent->soundSet, type );

	if ( sfx == -1 )
		return;

	G_AddEvent( ent, EV_BMODEL_SOUND, sfx );
}

/*
============
G_TestEntityPosition

============
*/
gentity_t	*G_TestEntityPosition( gentity_t *ent ) {
	trace_t	tr;
	int		mask;

	if ( ent->clipmask ) {
		mask = ent->clipmask;
	} else {
		mask = MASK_SOLID;
	}
	if ( ent->client ) {
		gi.trace( &tr, ent->client->ps.origin, ent->mins, ent->maxs, ent->client->ps.origin, ent->s.number, mask );
	} else {
		gi.trace( &tr, ent->s.pos.trBase, ent->mins, ent->maxs, ent->s.pos.trBase, ent->s.number, mask );
	}
	
	if (tr.startsolid)
		return &g_entities[ tr.entityNum ];
		
	return NULL;
}


/*
==================
G_TryPushingEntity

Returns qfalse if the move is blocked
==================
*/
qboolean	G_TryPushingEntity( gentity_t *check, gentity_t *pusher, vec3_t move, vec3_t amove ) {
	vec3_t		forward, right, up;
	vec3_t		org, org2, move2;
	gentity_t	*block;

	/*
	// EF_MOVER_STOP will just stop when contacting another entity
	// instead of pushing it, but entities can still ride on top of it
	if ( ( pusher->s.eFlags & EF_MOVER_STOP ) && 
		check->s.groundEntityNum != pusher->s.number ) {
		return qfalse;
	}
	*/

	// save off the old position
	if (pushed_p > &pushed[MAX_GENTITIES]) {
		G_Error( "pushed_p > &pushed[MAX_GENTITIES]" );
	}
	pushed_p->ent = check;
	VectorCopy (check->s.pos.trBase, pushed_p->origin);
	VectorCopy (check->s.apos.trBase, pushed_p->angles);
	if ( check->client ) {
		pushed_p->deltayaw = check->client->ps.delta_angles[YAW];
		VectorCopy (check->client->ps.origin, pushed_p->origin);
	}
	pushed_p++;

	// we need this for pushing things later
	VectorSubtract (vec3_origin, amove, org);
	AngleVectors (org, forward, right, up);

	// try moving the contacted entity 
	VectorAdd (check->s.pos.trBase, move, check->s.pos.trBase);
	if (check->client) {
		// make sure the client's view rotates when on a rotating mover
		check->client->ps.delta_angles[YAW] += ANGLE2SHORT(amove[YAW]);
	}

	// figure movement due to the pusher's amove
	VectorSubtract (check->s.pos.trBase, pusher->currentOrigin, org);
	org2[0] = DotProduct (org, forward);
	org2[1] = -DotProduct (org, right);
	org2[2] = DotProduct (org, up);
	VectorSubtract (org2, org, move2);
	VectorAdd (check->s.pos.trBase, move2, check->s.pos.trBase);
	if ( check->client ) {
		VectorAdd (check->client->ps.origin, move, check->client->ps.origin);
		VectorAdd (check->client->ps.origin, move2, check->client->ps.origin);
	}

	// may have pushed them off an edge
	if ( check->s.groundEntityNum != pusher->s.number ) {
		check->s.groundEntityNum = -1;
	}

	block = G_TestEntityPosition( check );
	if (!block) {
		// pushed ok
		if ( check->client ) {
			VectorCopy( check->client->ps.origin, check->currentOrigin );
		} else {
			VectorCopy( check->s.pos.trBase, check->currentOrigin );
		}
		gi.linkentity (check);
		return qtrue;
	}

	// if it is ok to leave in the old position, do it
	// this is only relevent for riding entities, not pushed
	// Sliding trapdoors can cause this.
	VectorCopy( (pushed_p-1)->origin, check->s.pos.trBase);
	if ( check->client ) {
		VectorCopy( (pushed_p-1)->origin, check->client->ps.origin);
	}
	VectorCopy( (pushed_p-1)->angles, check->s.apos.trBase );
	block = G_TestEntityPosition (check);
	if ( !block ) {
		check->s.groundEntityNum = -1;
		pushed_p--;
		return qtrue;
	}

	// blocked
	if ( pusher->damage )
	{//Do damage
		G_Damage(check, pusher, pusher->activator, move, check->currentOrigin, pusher->damage, 0, MOD_UNKNOWN );
	}

	return qfalse;
}


/*
============
G_MoverPush

Objects need to be moved back on a failed push,
otherwise riders would continue to slide.
If qfalse is returned, *obstacle will be the blocking entity
============
*/
qboolean G_MoverPush( gentity_t *pusher, vec3_t move, vec3_t amove, gentity_t **obstacle ) {
	int			i, e;
	gentity_t	*check;
	vec3_t		mins, maxs;
	pushed_t	*p;
	gentity_t	*entityList[MAX_GENTITIES];
	int			listedEntities;
	vec3_t		pusherMins, pusherMaxs, totalMins, totalMaxs;

	*obstacle = NULL;


	if ( !pusher->bmodel )
	{//misc_model_breakable
		VectorAdd( pusher->currentOrigin, pusher->mins, pusherMins );
		VectorAdd( pusher->currentOrigin, pusher->maxs, pusherMaxs );
	}

	// mins/maxs are the bounds at the destination
	// totalMins / totalMaxs are the bounds for the entire move
	if ( pusher->currentAngles[0] || pusher->currentAngles[1] || pusher->currentAngles[2]
		|| amove[0] || amove[1] || amove[2] ) 
	{
		float		radius;

		radius = RadiusFromBounds( pusher->mins, pusher->maxs );
		for ( i = 0 ; i < 3 ; i++ ) 
		{
			mins[i] = pusher->currentOrigin[i] + move[i] - radius;
			maxs[i] = pusher->currentOrigin[i] + move[i] + radius;
			totalMins[i] = mins[i] - move[i];
			totalMaxs[i] = maxs[i] - move[i];
		}
	} 
	else 
	{
		for (i=0 ; i<3 ; i++) 
		{
			mins[i] = pusher->absmin[i] + move[i];
			maxs[i] = pusher->absmax[i] + move[i];
		}

		VectorCopy( pusher->absmin, totalMins );
		VectorCopy( pusher->absmax, totalMaxs );
		for (i=0 ; i<3 ; i++) 
		{
			if ( move[i] > 0 ) 
			{
				totalMaxs[i] += move[i];
			} 
			else 
			{
				totalMins[i] += move[i];
			}
		}
	}

	// unlink the pusher so we don't get it in the entityList
	gi.unlinkentity( pusher );

	listedEntities = gi.EntitiesInBox( totalMins, totalMaxs, entityList, MAX_GENTITIES );

	// move the pusher to it's final position
	VectorAdd( pusher->currentOrigin, move, pusher->currentOrigin );
	VectorAdd( pusher->currentAngles, amove, pusher->currentAngles );
	gi.linkentity( pusher );

	// see if any solid entities are inside the final position
	for ( e = 0 ; e < listedEntities ; e++ ) {
		check = entityList[ e ];

		// only push items and players
		if ( check->s.eType != ET_ITEM ) 
		{
			if ( check->s.eType != ET_PLAYER ) 
			{
				continue;
			}
			/*
			else if ( check->health <= 0 )
			{//For now, don't push on dead players
				continue;
			}
			*/
			else if ( !pusher->bmodel )
			{
				vec3_t	checkMins, checkMaxs;

				VectorAdd( check->currentOrigin, check->mins, checkMins );
				VectorAdd( check->currentOrigin, check->maxs, checkMaxs );

				if ( G_BoundsOverlap( checkMins, checkMaxs, pusherMins, pusherMaxs ) )
				{//They're inside me already, no push - FIXME: we're testing a moves spot, aren't we, so we could have just moved inside them?
					continue;
				}
			}
		}


		if ( check->maxs[0] - check->mins[0] <= 0 &&
				check->maxs[1] - check->mins[1] <= 0 &&
				check->maxs[2] - check->mins[2] <= 0 )
		{//no size, don't push
			continue;
		}

		// if the entity is standing on the pusher, it will definitely be moved
		if ( check->s.groundEntityNum != pusher->s.number ) {
			// see if the ent needs to be tested
			if ( check->absmin[0] >= maxs[0]
			|| check->absmin[1] >= maxs[1]
			|| check->absmin[2] >= maxs[2]
			|| check->absmax[0] <= mins[0]
			|| check->absmax[1] <= mins[1]
			|| check->absmax[2] <= mins[2] ) {
				continue;
			}
			// see if the ent's bbox is inside the pusher's final position
			// this does allow a fast moving object to pass through a thin entity...
			if ( G_TestEntityPosition( check ) != pusher ) 
			{
				continue;
			}
		}

		// the entity needs to be pushed
		if ( G_TryPushingEntity( check, pusher, move, amove ) ) {
			continue;
		}

		// the move was blocked

		
		// save off the obstacle so we can call the block function (crush, etc)
		*obstacle = check;

		// move back any entities we already moved
		// go backwards, so if the same entity was pushed
		// twice, it goes back to the original position
		for ( p=pushed_p-1 ; p>=pushed ; p-- ) {
			VectorCopy (p->origin, p->ent->s.pos.trBase);
			VectorCopy (p->angles, p->ent->s.apos.trBase);
			if ( p->ent->client ) {
				p->ent->client->ps.delta_angles[YAW] = p->deltayaw;
				VectorCopy (p->origin, p->ent->client->ps.origin);
			}
			gi.linkentity (p->ent);
		}
		return qfalse;
	}

	return qtrue;
}


/*
=================
G_MoverTeam
=================
*/
void G_MoverTeam( gentity_t *ent ) {
	vec3_t		move, amove;
	gentity_t	*part, *obstacle;
	vec3_t		origin, angles;

	obstacle = NULL;

	// make sure all team slaves can move before commiting
	// any moves or calling any think functions
	// if the move is blocked, all moved objects will be backed out
	pushed_p = pushed;
	for (part = ent ; part ; part=part->teamchain) 
	{
		// get current position
		part->s.eFlags &= ~EF_BLOCKED_MOVER;
		EvaluateTrajectory( &part->s.pos, level.time, origin );
		EvaluateTrajectory( &part->s.apos, level.time, angles );
		VectorSubtract( origin, part->currentOrigin, move );
		VectorSubtract( angles, part->currentAngles, amove );
		if ( !G_MoverPush( part, move, amove, &obstacle ) ) 
		{
			break;	// move was blocked
		}
	}

	if (part) 
	{
		// if the pusher has a "blocked" function, call it
		// go back to the previous position
		for ( part = ent ; part ; part = part->teamchain ) 
		{
			//Push up time so it doesn't wiggle when blocked
			part->s.pos.trTime += level.time - level.previousTime;
			part->s.apos.trTime += level.time - level.previousTime;
			EvaluateTrajectory( &part->s.pos, level.time, part->currentOrigin );
			EvaluateTrajectory( &part->s.apos, level.time, part->currentAngles );
			gi.linkentity( part );
			part->s.eFlags |= EF_BLOCKED_MOVER;
		}

		if ( ent->e_BlockedFunc != blockedF_NULL ) 
		{// this check no longer necessary, done internally below, but it's here for reference
			GEntity_BlockedFunc( ent, obstacle );
		}
		return;
	}

	// the move succeeded
	for ( part = ent ; part ; part = part->teamchain ) 
	{
		// call the reached function if time is at or past end point
		if ( part->s.pos.trType == TR_LINEAR_STOP ) 
		{
			if ( level.time >= part->s.pos.trTime + part->s.pos.trDuration ) 
			{				
				GEntity_ReachedFunc( part );
			}
		}
	}
}

/*
================
G_RunMover

================
*/
void rebolt_turret( gentity_t *base );
void G_RunMover( gentity_t *ent ) {
	// if not a team captain, don't do anything, because
	// the captain will handle everything
	if ( ent->flags & FL_TEAMSLAVE ) {
		return;
	}

	// if stationary at one of the positions, don't move anything
	if ( ent->s.pos.trType != TR_STATIONARY || ent->s.apos.trType != TR_STATIONARY ) {
		G_MoverTeam( ent );
	}

	if ( ent->classname && Q_stricmp("misc_turret", ent->classname ) == 0 )
	{
		rebolt_turret( ent );
	}

	// check think function
	G_RunThink( ent );
}

/*
============================================================================

GENERAL MOVERS

Doors, plats, and buttons are all binary (two position) movers
Pos1 is "at rest", pos2 is "activated"
============================================================================
*/

/*
CalcTeamDoorCenter

Finds all the doors of a team and returns their center position
*/

void CalcTeamDoorCenter ( gentity_t *ent, vec3_t center ) 
{
	vec3_t		slavecenter;
	gentity_t	*slave;

	//Start with our center
	VectorAdd(ent->mins, ent->maxs, center);
	VectorScale(center, 0.5, center);
	for ( slave = ent->teamchain ; slave ; slave = slave->teamchain ) 
	{
		//Find slave's center
		VectorAdd(slave->mins, slave->maxs, slavecenter);
		VectorScale(slavecenter, 0.5, slavecenter);
		//Add that to our own, find middle
		VectorAdd(center, slavecenter, center);
		VectorScale(center, 0.5, center);
	}
}

/*
===============
SetMoverState
===============
*/
void SetMoverState( gentity_t *ent, moverState_t moverState, int time ) {
	vec3_t			delta;
	float			f;

	ent->moverState = moverState;

	ent->s.pos.trTime = time;

	if ( ent->s.pos.trDuration <= 0 )
	{//Don't allow divide by zero!
		ent->s.pos.trDuration = 1;
	}

	switch( moverState ) {
	case MOVER_POS1:
		VectorCopy( ent->pos1, ent->s.pos.trBase );
		ent->s.pos.trType = TR_STATIONARY;
		break;
	case MOVER_POS2:
		VectorCopy( ent->pos2, ent->s.pos.trBase );
		ent->s.pos.trType = TR_STATIONARY;
		break;
	case MOVER_1TO2:
		VectorCopy( ent->pos1, ent->s.pos.trBase );
		VectorSubtract( ent->pos2, ent->pos1, delta );
		f = 1000.0 / ent->s.pos.trDuration;
		VectorScale( delta, f, ent->s.pos.trDelta );
		ent->s.pos.trType = TR_LINEAR_STOP;
		ent->s.eFlags &= ~EF_BLOCKED_MOVER;
		break;
	case MOVER_2TO1:
		VectorCopy( ent->pos2, ent->s.pos.trBase );
		VectorSubtract( ent->pos1, ent->pos2, delta );
		f = 1000.0 / ent->s.pos.trDuration;
		VectorScale( delta, f, ent->s.pos.trDelta );
		ent->s.pos.trType = TR_LINEAR_STOP;
		ent->s.eFlags &= ~EF_BLOCKED_MOVER;
		break;
	}
	EvaluateTrajectory( &ent->s.pos, level.time, ent->currentOrigin );	
	gi.linkentity( ent );
}

/*
================
MatchTeam

All entities in a mover team will move from pos1 to pos2
in the same amount of time
================
*/
void MatchTeam( gentity_t *teamLeader, int moverState, int time ) {
	gentity_t		*slave;

	for ( slave = teamLeader ; slave ; slave = slave->teamchain ) {
		SetMoverState( slave, (moverState_t) moverState, time );
	}
}



/*
================
ReturnToPos1
================
*/
void ReturnToPos1( gentity_t *ent ) {
	ent->e_ThinkFunc = thinkF_NULL;
	ent->nextthink = 0;
	ent->s.time = level.time;

	MatchTeam( ent, MOVER_2TO1, level.time );

	// starting sound
	G_PlayDoorLoopSound( ent );
	G_PlayDoorSound( ent, BMS_START );	//??
}


/*
================
Reached_BinaryMover
================
*/

void Reached_BinaryMover( gentity_t *ent ) 
{
	// stop the looping sound
	ent->s.loopSound = 0;

	if ( ent->moverState == MOVER_1TO2 ) 
	{//reached open
		// reached pos2
		SetMoverState( ent, MOVER_POS2, level.time );

		// play sound
		G_PlayDoorSound( ent, BMS_END );

		if(ent->wait < 0)
		{//Done for good
			ent->e_ThinkFunc = thinkF_NULL;
			ent->nextthink = -1;
		}
		else
		{
			// return to pos1 after a delay
			ent->e_ThinkFunc = thinkF_ReturnToPos1;
			if(ent->spawnflags & 8)
			{//Toggle, keep think, wait for next use?
				ent->nextthink = -1;
			}
			else
			{
				ent->nextthink = level.time + ent->wait;
			}
		}

		// fire targets
		if ( !ent->activator ) 
		{
			ent->activator = ent;
		}
		G_UseTargets2( ent, ent->activator, ent->opentarget );
	} 
	else if ( ent->moverState == MOVER_2TO1 ) 
	{//closed
		// reached pos1
		SetMoverState( ent, MOVER_POS1, level.time );

		// play sound
		G_PlayDoorSound( ent, BMS_END );

		// close areaportals
		if ( ent->teammaster == ent || !ent->teammaster ) 
		{
			gi.AdjustAreaPortalState( ent, qfalse );
		}
		G_UseTargets2( ent, ent->activator, ent->closetarget );
	} 
	else 
	{
		G_Error( "Reached_BinaryMover: bad moverState" );
	}
}


/*
================
Use_BinaryMover_Go
================
*/
void Use_BinaryMover_Go( gentity_t *ent ) 
{
	int		total;
	int		partial;
//	gentity_t	*other = ent->enemy;
	gentity_t	*activator = ent->activator;

	ent->activator = activator;

	if ( ent->moverState == MOVER_POS1 ) 
	{
		// start moving 50 msec later, becase if this was player
		// triggered, level.time hasn't been advanced yet
		MatchTeam( ent, MOVER_1TO2, level.time + 50 );

		// starting sound
		G_PlayDoorLoopSound( ent );
		G_PlayDoorSound( ent, BMS_START );
		ent->s.time = level.time;

		// open areaportal
		if ( ent->teammaster == ent || !ent->teammaster ) {
			gi.AdjustAreaPortalState( ent, qtrue );
		}
		G_UseTargets( ent, ent->activator );
		return;
	}

	// if all the way up, just delay before coming down
	if ( ent->moverState == MOVER_POS2 ) {

		if ( ent->spawnflags & 8 )
		{//TOGGLE doors don't use wait!
			ent->nextthink = level.time + FRAMETIME;
		}
		else
		{
			ent->nextthink = level.time + ent->wait;
		}
		G_UseTargets2( ent, ent->activator, ent->target2 );
		return;
	}

	// only partway down before reversing
	if ( ent->moverState == MOVER_2TO1 ) 
	{
		total = ent->s.pos.trDuration;
		partial = level.time - ent->s.pos.trTime;//ent->s.time;
		ent->s.pos.trTime = level.time;//ent->s.time;

		if ( partial > total ) {
			partial = total;
		}

		MatchTeam( ent, MOVER_1TO2, level.time - ( total - partial ) );

		G_PlayDoorSound( ent, BMS_START );

		return;
	}

	// only partway up before reversing
	if ( ent->moverState == MOVER_1TO2 ) 
	{
		total = ent->s.pos.trDuration;
		partial = level.time - ent->s.pos.trTime;//ent->s.time;
		ent->s.pos.trTime = level.time;//ent->s.time;

		if ( partial > total ) {
			partial = total;
		}

		MatchTeam( ent, MOVER_2TO1, level.time - ( total - partial ) );

		G_PlayDoorSound( ent, BMS_START );

		return;
	}
}
/*
================
Use_BinaryMover
================
*/
void Use_BinaryMover( gentity_t *ent, gentity_t *other, gentity_t *activator ) 
{
	if ( ent->spawnflags & MOVER_SECURITY )
	{
		if ( ent->fly_sound_debounce_time > level.time )
		{
			return;
		}
		else if ( g_entities[0].client->tourSess.tour_objectives[OBJ_SECURITYCODE].status != OBJECTIVE_STAT_SUCCEEDED &&
			g_entities[0].client->tourSess.tour_objectives[OBJ_SECURITYCODE].status != SET_OBJ_SUCCEEDEDTOUR )
		{
			G_Sound( activator, G_SoundIndex( "sound/voice/computer/misc/secaccreq.wav" ) );
			ent->fly_sound_debounce_time = level.time + 5000;
			return;
		}
	}

	if (ent->behaviorSet[BSET_USE])
	{
		G_ActivateBehavior(ent,BSET_USE);
	}

	// only the master should be used
	if ( ent->flags & FL_TEAMSLAVE ) {
		Use_BinaryMover( ent->teammaster, other, activator );
		return;
	}

	G_SetEnemy( ent, other );
	ent->activator = activator;
	if(ent->delay)
	{
		ent->e_ThinkFunc = thinkF_Use_BinaryMover_Go;
		ent->nextthink = level.time + ent->delay;
	}
	else
	{
		Use_BinaryMover_Go(ent);
	}
}



/*
================
InitMover

"pos1", "pos2", and "speed" should be set before calling,
so the movement delta can be calculated
================
*/
void InitMoverTrData( gentity_t *ent )
{
	vec3_t		move;
	float		distance;

	ent->s.pos.trType = TR_STATIONARY;
	VectorCopy( ent->pos1, ent->s.pos.trBase );

	// calculate time to reach second position from speed
	VectorSubtract( ent->pos2, ent->pos1, move );
	distance = VectorLength( move );
	if ( ! ent->speed ) 
	{
		ent->speed = 100;
	}
	VectorScale( move, ent->speed, ent->s.pos.trDelta );
	ent->s.pos.trDuration = distance * 1000 / ent->speed;
	if ( ent->s.pos.trDuration <= 0 ) 
	{
		ent->s.pos.trDuration = 1;
	}
}

void InitMover( gentity_t *ent ) 
{
	float		light;
	vec3_t		color;
	qboolean	lightSet, colorSet;

	// if the "model2" key is set, use a seperate model
	// for drawing, but clip against the brushes
	if ( ent->model2 ) {
		ent->s.modelindex2 = G_ModelIndex( ent->model2 );
	}

	// if the "color" or "light" keys are set, setup constantLight
	lightSet = G_SpawnFloat( "light", "100", &light );
	colorSet = G_SpawnVector( "color", "1 1 1", color );
	if ( lightSet || colorSet ) {
		int		r, g, b, i;

		r = color[0] * 255;
		if ( r > 255 ) {
			r = 255;
		}
		g = color[1] * 255;
		if ( g > 255 ) {
			g = 255;
		}
		b = color[2] * 255;
		if ( b > 255 ) {
			b = 255;
		}
		i = light / 4;
		if ( i > 255 ) {
			i = 255;
		}
		ent->s.constantLight = r | ( g << 8 ) | ( b << 16 ) | ( i << 24 );
	}

	ent->e_UseFunc	   = useF_Use_BinaryMover;
	ent->e_ReachedFunc = reachedF_Reached_BinaryMover;

	ent->moverState = MOVER_POS1;
	ent->svFlags = SVF_USE_CURRENT_ORIGIN;
	if(ent->spawnflags & 128)
	{//Can be used by the player's BUTTON_USE
		ent->svFlags |= SVF_PLAYER_USABLE;
	}
	ent->s.eType = ET_MOVER;
	VectorCopy (ent->pos1, ent->currentOrigin);
	gi.linkentity (ent);

	InitMoverTrData( ent );
}


/*
===============================================================================

DOOR

A use can be triggered either by a touch function, by being shot, or by being
targeted by another entity.

===============================================================================
*/

/*
================
Blocked_Door
================
*/
void Blocked_Door( gentity_t *ent, gentity_t *other ) {
	// remove anything other than a client
	if ( !other->client ) {
		G_FreeEntity( other );
		return;
	}

	if ( ent->damage ) {
		G_Damage( other, ent, ent, NULL, NULL, ent->damage, 0, MOD_CRUSH );
	}
	if ( ent->spawnflags & 4 ) {
		return;		// crushers don't reverse
	}

	// reverse direction
	Use_BinaryMover( ent, ent, other );
}


/*
================
Touch_DoorTrigger
================
*/

void Touch_DoorTrigger( gentity_t *ent, gentity_t *other, trace_t *trace ) 
{
	if ( ent->owner->moverState != MOVER_1TO2 ) 
	{//Door is not already opening
		//if ( ent->owner->moverState == MOVER_POS1 || ent->owner->moverState == MOVER_2TO1 )
		//{//only check these if closed or closing

		//If door is closed, opening or open, check this
		if ( other->client && ent->owner->spawnflags & MOVER_MUST_FACE )
		{//check to see if we're facing the door
			vec3_t	vec, doorcenter, movedir;
			float	dot, dist, speed;

			//Dir from activator to door center
			CalcTeamDoorCenter( ent->owner, doorcenter );
			VectorSubtract( doorcenter, other->currentOrigin, vec );
			dist = VectorNormalize ( vec );
			//FIXME: see if they're moving very fast and mod dist by this?
			if ( dist > 72 )
			{// dist > 72 never opens door
				return;
			}
			else if ( dist > 32 )
			{// <= 32 opens door always
				//Activator's facing dir
				VectorCopy( other->client->ps.velocity, movedir );
				speed = VectorNormalize( movedir );
				dot = DotProduct( vec, movedir );
				if ( dot < 0.8 )
				{
					return;
				}
			}
			
			if ( ent->owner->e_ThinkFunc == thinkF_ReturnToPos1 && ent->owner->nextthink > level.time && ent->owner->nextthink < level.time + 500 )
			{//Trek door is waiting to close, but still being activated
				//so put off closing for half a second
				ent->owner->nextthink = level.time + 500;
				return;
			}

			if ( ent->owner->moverState == MOVER_POS2 )
			{//already open, stay open
				//QUESTION: what if it's a toggle door?  Should we ever not do this?
				return;
			}
		}
		Use_BinaryMover( ent->owner, ent, other );
	}

	/*
	//Old style
	if ( ent->owner->moverState != MOVER_1TO2 ) {
		Use_BinaryMover( ent->owner, ent, other );
	}
	*/
}

/*
======================
Think_SpawnNewDoorTrigger

All of the parts of a door have been spawned, so create
a trigger that encloses all of them
======================
*/
void Think_SpawnNewDoorTrigger( gentity_t *ent ) {
	gentity_t		*other;
	vec3_t		mins, maxs;
	int			i, best;

	// set all of the slaves as shootable
	if ( ent->takedamage ) {
		for ( other = ent ; other ; other = other->teamchain ) {
			other->takedamage = qtrue;
		}
	}

	// find the bounds of everything on the team
	VectorCopy (ent->absmin, mins);
	VectorCopy (ent->absmax, maxs);

	for (other = ent->teamchain ; other ; other=other->teamchain) {
		AddPointToBounds (other->absmin, mins, maxs);
		AddPointToBounds (other->absmax, mins, maxs);
	}

	// find the thinnest axis, which will be the one we expand
	best = 0;
	for ( i = 1 ; i < 3 ; i++ ) {
		if ( maxs[i] - mins[i] < maxs[best] - mins[best] ) {
			best = i;
		}
	}
	maxs[best] += 120;
	mins[best] -= 120;

	// create a trigger with this size
	other = G_Spawn ();
	VectorCopy (mins, other->mins);
	VectorCopy (maxs, other->maxs);
	other->owner = ent;
	other->contents = CONTENTS_TRIGGER;
	other->e_TouchFunc = touchF_Touch_DoorTrigger;
	gi.linkentity (other);

	MatchTeam( ent, ent->moverState, level.time );
}

void Think_MatchTeam( gentity_t *ent ) 
{
	MatchTeam( ent, ent->moverState, level.time );
}


/*QUAKED func_door (0 .5 .8) ? START_OPEN x CRUSHER TOGGLE TREK_DOOR SECURITY x PLAYER_USE
START_OPEN	the door to moves to its destination when spawned, and operate in reverse.  It is used to temporarily or permanently close off an area when triggered (not useful for touch or takedamage doors).
CRUSHER		?
TOGGLE		wait in both the start and end states for a trigger event - does NOT work on Trek doors.
TREK_DOOR	Must be moving toward this door and within 72 to open, within 32 will always open - Star Trek Doors.  Cannot be toggled
SECURITY	Door will not work until objective OBJ_SECURITYCODE is maked as succeeded
PLAYER_USE	Player can use it with the use button

"target"     Door fires this when it starts moving from it's closed position to its open position.
"opentarget" Door fires this after reaching its "open" position
"target2"    Door fires this when it starts moving from it's open position to its closed position.
"closetarget" Door fires this after reaching its "closed" position
"model2"	.md3 model to also draw
"angle"		determines the opening direction
"targetname" if set, no touch field will be spawned and a remote button or trigger field activates the door.
"speed"		movement speed (100 default)
"wait"		wait before returning (3 default, -1 = never return)
"delay"		when used, how many seconds to wait before moving - default is none
"lip"		lip remaining at end of move (8 default)
"dmg"		damage to inflict when blocked (2 default, set to negative for no damage)
"color"		constantLight color
"light"		constantLight radius
"health"	if set, the door must be shot open
"sounds" - sound door makes when opening/closing
0 - no sound (default)
*/
void SP_func_door (gentity_t *ent) 
{
	vec3_t	abs_movedir;
	float	distance;
	vec3_t	size;
	float	lip;

	ent->e_BlockedFunc = blockedF_Blocked_Door;

	if ( ent->spawnflags & MOVER_SECURITY )
	{
		G_SoundIndex( "sound/voice/computer/misc/secaccreq.wav" );
	}

	if ( !ent->infoString )
	{//default infoString for doors is "door", defaults to show up on active scanning only
		ent->infoString = bg_infoItemList[II_DOOR].infoString;
		ent->s.eFlags |= EF_NO_TED;//only shows up if scanned
	}

	//Trek doors cannot toggle!!!
	if ( ent->targetname && ent->targetname[0] )
	{
		if ( ent->spawnflags & 8 )//Toggle
		{//door is toggle-able and has a targetname, so must not have meant it to be a trek_door
			ent->spawnflags &= ~16;//trek_door
		}
	}
	else if ( ent->spawnflags & 16 )//trek_door
	{//door is a trek_door and has a targetname, so must not have meant it to be a trek_door
		ent->spawnflags &= ~8;//Toggle
	}

	// default speed of 400
	if (!ent->speed)
		ent->speed = 400;

	// default wait of 2 seconds
	if (!ent->wait)
		ent->wait = 2;
	ent->wait *= 1000;

	ent->delay *= 1000;

	// default lip of 8 units
	G_SpawnFloat( "lip", "8", &lip );

	// default damage of 2 points
	G_SpawnInt( "dmg", "2", &ent->damage );
	if ( ent->damage < 0 )
	{
		ent->damage = 0;
	}

	// first position at start
	VectorCopy( ent->s.origin, ent->pos1 );

	// calculate second position
	gi.SetBrushModel( ent, ent->model );
	G_SetMovedir (ent->s.angles, ent->movedir);
	abs_movedir[0] = fabs(ent->movedir[0]);
	abs_movedir[1] = fabs(ent->movedir[1]);
	abs_movedir[2] = fabs(ent->movedir[2]);
	VectorSubtract( ent->maxs, ent->mins, size );
	distance = DotProduct( abs_movedir, size ) - lip;
	VectorMA( ent->pos1, distance, ent->movedir, ent->pos2 );

	// if "start_open", reverse position 1 and 2
	if ( ent->spawnflags & 1 ) {
		vec3_t	temp;

		VectorCopy( ent->pos2, temp );
		VectorCopy( ent->s.origin, ent->pos2 );
		VectorCopy( temp, ent->pos1 );
	}

	InitMover( ent );

	ent->nextthink = level.time + FRAMETIME;

	if ( ! (ent->flags & FL_TEAMSLAVE ) ) 
	{
		int health;

		G_SpawnInt( "health", "0", &health );
		
		 if ( health ) 
		{
			ent->takedamage = qtrue;
		}
		
		if ( ent->targetname || health || ent->spawnflags & 128) //PLAYER_USE
		{
			// non touch/shoot doors
			ent->e_ThinkFunc = thinkF_Think_MatchTeam;
		} 
		else 
		{
			ent->e_ThinkFunc = thinkF_Think_SpawnNewDoorTrigger;
		}
	}
}

/*
===============================================================================

PLAT

===============================================================================
*/

/*
==============
Touch_Plat

Don't allow decent if a living player is on it
===============
*/
void Touch_Plat( gentity_t *ent, gentity_t *other, trace_t *trace ) {
	if ( !other->client || other->client->ps.stats[STAT_HEALTH] <= 0 ) {
		return;
	}

	// delay return-to-pos1 by one second
	if ( ent->moverState == MOVER_POS2 ) {
		ent->nextthink = level.time + 1000;
	}
}

/*
==============
Touch_PlatCenterTrigger

If the plat is at the bottom position, start it going up
===============
*/
void Touch_PlatCenterTrigger(gentity_t *ent, gentity_t *other, trace_t *trace ) {
	if ( !other->client ) {
		return;
	}

	if ( ent->owner->moverState == MOVER_POS1 ) {
		Use_BinaryMover( ent->owner, ent, other );
	}
}


/*
================
SpawnPlatTrigger

Spawn a trigger in the middle of the plat's low position
Elevator cars require that the trigger extend through the entire low position,
not just sit on top of it.
================
*/
void SpawnPlatTrigger( gentity_t *ent ) {
	gentity_t	*trigger;
	vec3_t	tmin, tmax;

	// the middle trigger will be a thin trigger just
	// above the starting position
	trigger = G_Spawn();
	trigger->e_TouchFunc = touchF_Touch_PlatCenterTrigger;
	trigger->contents = CONTENTS_TRIGGER;
	trigger->owner = ent;
	
	tmin[0] = ent->pos1[0] + ent->mins[0] + 33;
	tmin[1] = ent->pos1[1] + ent->mins[1] + 33;
	tmin[2] = ent->pos1[2] + ent->mins[2];

	tmax[0] = ent->pos1[0] + ent->maxs[0] - 33;
	tmax[1] = ent->pos1[1] + ent->maxs[1] - 33;
	tmax[2] = ent->pos1[2] + ent->maxs[2] + 8;

	if ( tmax[0] <= tmin[0] ) {
		tmin[0] = ent->pos1[0] + (ent->mins[0] + ent->maxs[0]) *0.5;
		tmax[0] = tmin[0] + 1;
	}
	if ( tmax[1] <= tmin[1] ) {
		tmin[1] = ent->pos1[1] + (ent->mins[1] + ent->maxs[1]) *0.5;
		tmax[1] = tmin[1] + 1;
	}
	
	VectorCopy (tmin, trigger->mins);
	VectorCopy (tmax, trigger->maxs);

	gi.linkentity (trigger);
}


/*QUAKED func_plat (0 .5 .8) ? x x x x x x x PLAYER_USE
PLAYER_USE	Player can use it with the use button

Plats are always drawn in the extended position so they will light correctly.

"lip"		default 8, protrusion above rest position
"height"	total height of movement, defaults to model height
"speed"		overrides default 200.
"dmg"		overrides default 2
"model2"	.md3 model to also draw
"color"		constantLight color
"light"		constantLight radius
*/
void SP_func_plat (gentity_t *ent) {
	float		lip, height;

//	ent->sound1to2 = ent->sound2to1 = G_SoundIndex("sound/movers/plats/pt1_strt.wav");
//	ent->soundPos1 = ent->soundPos2 = G_SoundIndex("sound/movers/plats/pt1_end.wav");

	VectorClear (ent->s.angles);

	G_SpawnFloat( "speed", "200", &ent->speed );
	G_SpawnInt( "dmg", "2", &ent->damage );
	G_SpawnFloat( "wait", "1", &ent->wait );
	G_SpawnFloat( "lip", "8", &lip );

	ent->wait = 1000;

	// create second position
	gi.SetBrushModel( ent, ent->model );

	if ( !G_SpawnFloat( "height", "0", &height ) ) {
		height = (ent->maxs[2] - ent->mins[2]) - lip;
	}

	// pos1 is the rest (bottom) position, pos2 is the top
	VectorCopy( ent->s.origin, ent->pos2 );
	VectorCopy( ent->pos2, ent->pos1 );
	ent->pos1[2] -= height;

	InitMover( ent );

	// touch function keeps the plat from returning while
	// a live player is standing on it
	ent->e_TouchFunc = touchF_Touch_Plat;

	ent->e_BlockedFunc = blockedF_Blocked_Door;

	ent->owner = ent;	// so it can be treated as a door

	// spawn the trigger if one hasn't been custom made
	if ( !ent->targetname ) {
		SpawnPlatTrigger(ent);
	}
}


/*
===============================================================================

BUTTON

===============================================================================
*/

/*
==============
Touch_Button

===============
*/
void Touch_Button(gentity_t *ent, gentity_t *other, trace_t *trace ) {
	if ( !other->client ) {
		return;
	}

	if ( ent->moverState == MOVER_POS1 ) {
		Use_BinaryMover( ent, other, other );
	}
}


/*QUAKED func_button (0 .5 .8) ? x x x x x x x PLAYER_USE
PLAYER_USE	Player can use it with the use button

When a button is touched, it moves some distance in the direction of it's angle, triggers all of it's targets, waits some time, then returns to it's original position where it can be triggered again.

"model2"	.md3 model to also draw
"angle"		determines the opening direction
"target"	all entities with a matching targetname will be used
"speed"		override the default 40 speed
"wait"		override the default 1 second wait (-1 = never return)
"lip"		override the default 4 pixel lip remaining at end of move
"health"	if set, the button must be killed instead of touched
"color"		constantLight color
"light"		constantLight radius
*/
void SP_func_button( gentity_t *ent ) {
	vec3_t		abs_movedir;
	float		distance;
	vec3_t		size;
	float		lip;

	ent->sound1to2 = G_SoundIndex("sound/movers/switches/butn2.wav");
	
	if ( !ent->speed ) {
		ent->speed = 40;
	}

	if ( !ent->wait ) {
		ent->wait = 1;
	}
	ent->wait *= 1000;

	// first position
	VectorCopy( ent->s.origin, ent->pos1 );

	// calculate second position
	gi.SetBrushModel( ent, ent->model );

	G_SpawnFloat( "lip", "4", &lip );

	G_SetMovedir( ent->s.angles, ent->movedir );
	abs_movedir[0] = fabs(ent->movedir[0]);
	abs_movedir[1] = fabs(ent->movedir[1]);
	abs_movedir[2] = fabs(ent->movedir[2]);
	VectorSubtract( ent->maxs, ent->mins, size );
	distance = abs_movedir[0] * size[0] + abs_movedir[1] * size[1] + abs_movedir[2] * size[2] - lip;
	VectorMA (ent->pos1, distance, ent->movedir, ent->pos2);

	if (ent->health) {
		// shootable button
		ent->takedamage = qtrue;
	} else {
		// touchable button
		ent->e_TouchFunc = touchF_Touch_Button;
	}

	InitMover( ent );
}



/*
===============================================================================

TRAIN

===============================================================================
*/


#define TRAIN_START_ON		1
#define TRAIN_TOGGLE		2
#define TRAIN_BLOCK_STOPS	4

/*
===============
Think_BeginMoving

The wait time at a corner has completed, so start moving again
===============
*/
void Think_BeginMoving( gentity_t *ent ) {
	ent->s.pos.trTime = level.time;
	ent->s.pos.trType = TR_LINEAR_STOP;
}

/*
===============
Reached_Train
===============
*/
void Reached_Train( gentity_t *ent ) {
	gentity_t		*next;
	float			speed;
	vec3_t			move;
	float			length;

	// copy the apropriate values
	next = ent->nextTrain;
	if ( !next || !next->nextTrain ) {
		return;		// just stop
	}

	// fire all other targets
	G_UseTargets( next, NULL );

	// set the new trajectory
	ent->nextTrain = next->nextTrain;
	VectorCopy( next->s.origin, ent->pos1 );
	VectorCopy( next->nextTrain->s.origin, ent->pos2 );

	// if the path_corner has a speed, use that
	if ( next->speed ) {
		speed = next->speed;
	} else {
		// otherwise use the train's speed
		speed = ent->speed;
	}
	if ( speed < 1 ) {
		speed = 1;
	}

	// calculate duration
	VectorSubtract( ent->pos2, ent->pos1, move );
	length = VectorLength( move );

	ent->s.pos.trDuration = length * 1000 / speed;

	// looping sound
/*
	if ( VALIDSTRING( next->soundSet ) )
	{
		ent->s.loopSound = CAS_GetBModelSound( next->soundSet, BMS_MID );//ent->soundLoop;
	}
*/
	G_PlayDoorLoopSound( ent );

	// start it going
	SetMoverState( ent, MOVER_1TO2, level.time );

	// if there is a "wait" value on the target, don't start moving yet
	if ( next->wait ) {
		ent->nextthink = level.time + next->wait * 1000;
		ent->e_ThinkFunc = thinkF_Think_BeginMoving;
		ent->s.pos.trType = TR_STATIONARY;
	}
}


/*
===============
Think_SetupTrainTargets

Link all the corners together
===============
*/
void Think_SetupTrainTargets( gentity_t *ent ) {
	gentity_t		*path, *next, *start;

	ent->nextTrain = G_Find( NULL, FOFS(targetname), ent->target );
	if ( !ent->nextTrain ) {
		gi.Printf( "func_train at %s with an unfound target\n",
			vtos(ent->absmin) );
		return;
	}

	//FIXME: this can go into an infinite loop if last path_corner doesn't link to first
	//path_corner, like so:
	// t1---->t2---->t3
	//         ^      |
	//          \_____|
	start = NULL;
	for ( path = ent->nextTrain ; path != start ; path = next ) {
		if ( !start ) {
			start = path;
		}

		if ( !path->target ) {
//			gi.Printf( "Train corner at %s without a target\n",
//				vtos(path->s.origin) );
			return;
		}

		// find a path_corner among the targets
		// there may also be other targets that get fired when the corner
		// is reached
		next = NULL;
		do {
			next = G_Find( next, FOFS(targetname), path->target );
			if ( !next ) {
				gi.Printf( "Train corner at %s without a target path_corner\n",
					vtos(path->s.origin) );
				return;
			}
		} while ( strcmp( next->classname, "path_corner" ) );

		path->nextTrain = next;
	}

	// start the train moving from the first corner
	Reached_Train( ent );
}



/*QUAKED path_corner (.5 .3 0) (-8 -8 -8) (8 8 8)
Train path corners.
Target: next path corner and other targets to fire
"speed" speed to move to the next corner
"wait" seconds to wait before behining move to next corner
*/
void SP_path_corner( gentity_t *self ) {
	if ( !self->targetname ) {
		gi.Printf ("path_corner with no targetname at %s\n", vtos(self->s.origin));
		G_FreeEntity( self );
		return;
	}
	// path corners don't need to be linked in
	VectorCopy(self->s.origin, self->currentOrigin);
}



/*QUAKED func_train (0 .5 .8) ? START_ON TOGGLE BLOCK_STOPS x x x x PLAYER_USE
PLAYER_USE	Player can use it with the use button

A train is a mover that moves between path_corner target points.
Trains MUST HAVE AN ORIGIN BRUSH.
The train spawns at the first target it is pointing at.
"model2"	.md3 model to also draw
"speed"		default 100
"dmg"		default	2
"noise"		looping sound to play when the train is in motion
"target"	next path corner
"color"		constantLight color
"light"		constantLight radius
*/
void SP_func_train (gentity_t *self) {
	VectorClear (self->s.angles);

	if (self->spawnflags & TRAIN_BLOCK_STOPS) {
		self->damage = 0;
	} else {
		if (!self->damage) {
			self->damage = 2;
		}
	}

	if ( !self->speed ) {
		self->speed = 100;
	}

	if ( !self->target ) {
		gi.Printf ("func_train without a target at %s\n", vtos(self->absmin));
		G_FreeEntity( self );
		return;
	}

	char *noise;

	if ( G_SpawnString( "noise", "", &noise ) )
	{
		if ( VALIDSTRING( noise ) )
		{
			self->s.loopSound = G_SoundIndex( noise );
		}
	}

	gi.SetBrushModel( self, self->model );
	InitMover( self );

	self->e_ReachedFunc = reachedF_Reached_Train;

	// start trains on the second frame, to make sure their targets have had
	// a chance to spawn
	self->nextthink = level.time + FRAMETIME;
	self->e_ThinkFunc = thinkF_Think_SetupTrainTargets;
}

/*
===============================================================================

STATIC

===============================================================================
*/

/*QUAKED func_static (0 .5 .8) ? x x x x x x x PLAYER_USE
PLAYER_USE	Player can use it with the use button

A bmodel that just sits there, doing nothing.  Can be used for conditional walls and models.
"model2"	.md3 model to also draw
"color"		constantLight color
"light"		constantLight radius

*/
void SP_func_static( gentity_t *ent ) 
{
	gi.SetBrushModel( ent, ent->model );

	VectorCopy( ent->s.origin, ent->pos1 );
	VectorCopy( ent->s.origin, ent->pos2 );

	InitMover( ent );

	ent->e_UseFunc = useF_func_static_use;
	ent->e_ReachedFunc = reachedF_NULL;

	G_SetOrigin( ent, ent->s.origin );

	gi.linkentity( ent );
}

void func_static_use ( gentity_t *self, gentity_t *other, gentity_t *activator )
{
	if ( self->behaviorSet[BSET_USE] )
	{
		G_ActivateBehavior( self, BSET_USE );
	}

	G_UseTargets( self, activator );
}

/*
===============================================================================

ROTATING

===============================================================================
*/


/*QUAKED func_rotating (0 .5 .8) ? START_ON - X_AXIS Y_AXIS x x x PLAYER_USE
PLAYER_USE	Player can use it with the use button

You need to have an origin brush as part of this entity.  The center of that brush will be
the point around which it is rotated. It will rotate around the Z axis by default.  You can
check either the X_AXIS or Y_AXIS box to change that.

"model2"	.md3 model to also draw
"speed"		determines how fast it moves; default value is 100.
"dmg"		damage to inflict when blocked (2 default)
"color"		constantLight color
"light"		constantLight radius
*/
void SP_func_rotating (gentity_t *ent) {
	if ( !ent->speed ) {
		ent->speed = 100;
	}

	// set the axis of rotation
	ent->s.apos.trType = TR_LINEAR;
	if ( ent->spawnflags & 4 ) {
		ent->s.apos.trDelta[2] = ent->speed;
	} else if ( ent->spawnflags & 8 ) {
		ent->s.apos.trDelta[0] = ent->speed;
	} else {
		ent->s.apos.trDelta[1] = ent->speed;
	}

	if (!ent->damage) {
		ent->damage = 2;
	}

	gi.SetBrushModel( ent, ent->model );
	InitMover( ent );

	VectorCopy( ent->s.origin, ent->s.pos.trBase );
	VectorCopy( ent->s.pos.trBase, ent->currentOrigin );
	VectorCopy( ent->s.apos.trBase, ent->currentAngles );

	gi.linkentity( ent );
}


/*
===============================================================================

BOBBING

===============================================================================
*/

void func_bobbing_use( gentity_t *self, gentity_t *other, gentity_t *activator )
{
	// Toggle our move state
	if ( self->s.pos.trType == TR_SINE )
	{
		self->s.pos.trType = TR_INTERPOLATE;

		// Save off roughly where we were
		VectorCopy( self->currentOrigin, self->s.pos.trBase );
		// Store the current phase value so we know how to start up where we left off.
		self->radius = ( level.time - self->s.pos.trTime ) / (float)self->s.pos.trDuration;
	}
	else
	{
		self->s.pos.trType = TR_SINE;

		// Set the time based on the saved phase value
		self->s.pos.trTime = level.time - self->s.pos.trDuration * self->radius;
		// Always make sure we are starting with a fresh base
		VectorCopy( self->s.origin, self->s.pos.trBase );
	}
}

/*QUAKED func_bobbing (0 .5 .8) ? X_AXIS Y_AXIS START_OFF x x x x PLAYER_USE
PLAYER_USE	Player can use it with the use button

Normally bobs on the Z axis
"model2"	.md3 model to also draw
"height"	amplitude of bob (32 default)
"speed"		seconds to complete a bob cycle (4 default)
"phase"		the 0.0 to 1.0 offset in the cycle to start at
"dmg"		damage to inflict when blocked (2 default)
"color"		constantLight color
"light"		constantLight radius
"targetname" turns on/off when used
*/
void SP_func_bobbing (gentity_t *ent) {
	float		height;
	float		phase;

	G_SpawnFloat( "speed", "4", &ent->speed );
	G_SpawnFloat( "height", "32", &height );
	G_SpawnInt( "dmg", "2", &ent->damage );
	G_SpawnFloat( "phase", "0", &phase );

	gi.SetBrushModel( ent, ent->model );
	InitMover( ent );

	VectorCopy( ent->s.origin, ent->s.pos.trBase );
	VectorCopy( ent->s.origin, ent->currentOrigin );

	// set the axis of bobbing
	if ( ent->spawnflags & 1 ) {
		ent->s.pos.trDelta[0] = height;
	} else if ( ent->spawnflags & 2 ) {
		ent->s.pos.trDelta[1] = height;
	} else {
		ent->s.pos.trDelta[2] = height;
	}

	ent->s.pos.trDuration = ent->speed * 1000;
	ent->s.pos.trTime = ent->s.pos.trDuration * phase;

	if ( ent->spawnflags & 4 ) // start_off
	{
		ent->s.pos.trType = TR_INTERPOLATE;

		// Now use the phase to calculate where it should be at the start.
		ent->radius = phase;
		phase = (float)sin( phase * M_PI * 2 );
		VectorMA( ent->s.pos.trBase, phase, ent->s.pos.trDelta, ent->s.pos.trBase );

		if ( ent->targetname )
		{
			ent->e_UseFunc = useF_func_bobbing_use;
		}
	}
	else
	{
		ent->s.pos.trType = TR_SINE;
	}
}

/*
===============================================================================

PENDULUM

===============================================================================
*/


/*QUAKED func_pendulum (0 .5 .8) ? x x x x x x x PLAYER_USE
PLAYER_USE	Player can use it with the use button

You need to have an origin brush as part of this entity.
Pendulums always swing north / south on unrotated models.  Add an angles field to the model to allow rotation in other directions.
Pendulum frequency is a physical constant based on the length of the beam and gravity.
"model2"	.md3 model to also draw
"speed"		the number of degrees each way the pendulum swings, (30 default)
"phase"		the 0.0 to 1.0 offset in the cycle to start at
"dmg"		damage to inflict when blocked (2 default)
"color"		constantLight color
"light"		constantLight radius
*/
void SP_func_pendulum(gentity_t *ent) {
	float		freq;
	float		length;
	float		phase;
	float		speed;

	G_SpawnFloat( "speed", "30", &speed );
	G_SpawnInt( "dmg", "2", &ent->damage );
	G_SpawnFloat( "phase", "0", &phase );

	gi.SetBrushModel( ent, ent->model );

	// find pendulum length
	length = fabs( ent->mins[2] );
	if ( length < 8 ) {
		length = 8;
	}

	freq = 1 / ( M_PI * 2 ) * sqrt( g_gravity->value / ( 3 * length ) );

	ent->s.pos.trDuration = ( 1000 / freq );

	InitMover( ent );

	VectorCopy( ent->s.origin, ent->s.pos.trBase );
	VectorCopy( ent->s.origin, ent->currentOrigin );

	VectorCopy( ent->s.angles, ent->s.apos.trBase );

	ent->s.apos.trDuration = 1000 / freq;
	ent->s.apos.trTime = ent->s.apos.trDuration * phase;
	ent->s.apos.trType = TR_SINE;

	ent->s.apos.trDelta[2] = speed;
}

/*
===============================================================================

WALL

===============================================================================
*/

//static -slc
void use_wall( gentity_t *ent, gentity_t *other, gentity_t *activator )
{
	if (ent->behaviorSet[BSET_USE])
	{
		G_ActivateBehavior(ent,BSET_USE);
	}

	// Not there so make it there
	if (!(ent->contents & CONTENTS_SOLID))
	{
		ent->svFlags &= ~SVF_NOCLIENT;
		ent->s.eFlags &= ~EF_NODRAW;
		ent->contents = CONTENTS_SOLID;
		if ( !(ent->spawnflags&1) )
		{//START_OFF doesn't effect area portals
			gi.AdjustAreaPortalState( ent, qfalse );
		}
	}
	// Make it go away
	else
	{
		ent->contents = 0;
		ent->svFlags |= SVF_NOCLIENT;
		ent->s.eFlags |= EF_NODRAW;
		if ( !(ent->spawnflags&1) )
		{//START_OFF doesn't effect area portals
			gi.AdjustAreaPortalState( ent, qtrue );
		}
	}
}

#define FUNC_WALL_OFF	1
#define FUNC_WALL_ANIM	2


/*QUAKED func_wall (0 .5 .8) ? START_OFF AUTOANIMATE x x x x x PLAYER_USE
PLAYER_USE	Player can use it with the use button

A bmodel that just sits there, doing nothing.  Can be used for conditional walls and models.
"model2"	.md3 model to also draw
"color"		constantLight color
"light"		constantLight radius

START_OFF - the wall will not be there
AUTOANIMATE - if a model is used it will animate				
*/
void SP_func_wall( gentity_t *ent ) 
{
	gi.SetBrushModel( ent, ent->model );

	VectorCopy( ent->s.origin, ent->pos1 );
	VectorCopy( ent->s.origin, ent->pos2 );

	InitMover( ent );
	VectorCopy( ent->s.origin, ent->s.pos.trBase );
	VectorCopy( ent->s.origin, ent->currentOrigin );

	// it must be START_OFF
	if (ent->spawnflags & FUNC_WALL_OFF)
	{
		ent->contents = 0;
		ent->svFlags |= SVF_NOCLIENT;
		ent->s.eFlags |= EF_NODRAW;
	}

	if (!(ent->spawnflags & FUNC_WALL_ANIM))
	{
		ent->s.eFlags |= EF_ANIM_ALLFAST;
	}
	ent->e_UseFunc = useF_use_wall;

	gi.linkentity (ent);

}

/*
-------------------------------------------

func_stasis_door

-------------------------------------------
*/

/*
--------------------------------------
stasis_door_blocked
--------------------------------------
*/

static qboolean stasis_door_blocked( gentity_t *ent ) 
{
	int			ct = 0;
	gentity_t	*entity_list[MAX_GENTITIES];

	// Do a quick check to see if someone is close to the door...pos1 is actually the door origin
	ct = G_RadiusList( ent->pos1, 128, ent, qtrue, entity_list );

	if ( ct )
	{
		for ( int i = 0; i < ct; i++ )
		{
			if ( entity_list[i]->client )
			{
				return true;
			}
		}
	}

	// no one is there
	return false;
}

#define DOOR_OPENING	1
#define DOOR_CLOSING	2
#define DOOR_OPEN		3
#define DOOR_CLOSED		4

/*
--------------------------------------
check_stasis_door_state
--------------------------------------
*/

void toggle_stasis_door( gentity_t *ent )
{
	if ( ent->behaviorSet[BSET_USE] )
	{
		G_ActivateBehavior( ent, BSET_USE );
	}

	if ( ent->count == DOOR_CLOSED )
	{
		// make it go away
		ent->count = DOOR_OPENING;
		ent->fx_time = level.time + 1000;

		// Now we add the model back in since we need to be able to fade something out...and bmodels can't do that.
		ent->s.modelindex2 = G_ModelIndex( ent->model2 );
		ent->s.eFlags |= EF_NODRAW;

		G_AddEvent( ent, EV_GENERAL_SOUND, G_SoundIndex( "sound/movers/doors/stasisdoor.wav" ));

		// Now would be a good time to open up the area portal..heh heh.
		gi.AdjustAreaPortalState( ent, qtrue );
	}
	else if ( ent->count == DOOR_OPEN )
	{
		// Door isn't there, so make it come back
		ent->count = DOOR_CLOSING;
		ent->svFlags &= ~SVF_NOCLIENT;
		ent->contents = CONTENTS_SOLID;
		ent->fx_time = level.time + 1000;

		G_AddEvent( ent, EV_GENERAL_SOUND, G_SoundIndex( "sound/movers/doors/stasisdoor.wav" ));
	}
	else if ( ent->count == DOOR_CLOSING )
	{
		// door is in the process of closing
		ent->count = DOOR_OPENING;
		ent->fx_time = level.time + ( ent->fx_time - level.time );
	}

	// Door should be done in 1 second
	ent->e_ThinkFunc = thinkF_check_stasis_door_state;
	ent->nextthink	 = level.time + 1000;
}


//-------------------------------------------
void check_stasis_door_state( gentity_t *ent )
{
	if ( ent->count == DOOR_OPENING )
	{
		// See if this is one of those crazy locked doors
		if ( ent->teamchain )
		{
			// it is, so we should tell the trigger (teammaster) to control the related door
			if ( !ent->teammaster )
				// FIXME: Error message here....NOT Cool!
				return;

			// Set the trigger owner to the new door so the trigger will now toggle the new door
			ent->teammaster->owner = ent->teamchain; 

			// Swap doors and make sure the new one is set up properly
			ent->teamchain->count = DOOR_OPEN;
			ent->teamchain->svFlags |= SVF_NOCLIENT;
			ent->teamchain->contents = 0;
			ent->teamchain->s.eFlags |= EF_NODRAW;

			// We'll also need to switch the model we are using
			ent->teamchain->model2 = ent->team;
			ent->teamchain->s.modelindex2 = G_ModelIndex( ent->teamchain->model2 );

			ent->teamchain->e_ThinkFunc = thinkF_check_stasis_door_state;
			ent->teamchain->nextthink = level.time + 2000;
		}
		else // regular door
		{
			ent->nextthink = level.time + 2000;
		}

		ent->count = DOOR_OPEN;
		ent->svFlags |= SVF_NOCLIENT;
		ent->contents = 0;
		ent->s.eFlags |= EF_NODRAW;
		return;
	}
	else if ( ent->count == DOOR_CLOSING )
	{
		ent->count = DOOR_CLOSED;

		// The door is fully closed (and faded) so don't draw the door model anymore
		ent->s.modelindex2 = 0;
		ent->s.eFlags &= ~EF_NODRAW; // let the bmodel draw
		ent->svFlags &= ~SVF_NOCLIENT;

		// Now would be a good time to close the area portal.
		gi.AdjustAreaPortalState( ent, qfalse );
		return;
	}

	// If someone is close to the door, we don't want to try and close it
	if ( !stasis_door_blocked( ent ) )
	{
		toggle_stasis_door( ent );
	}
	else
	{
		// Couldn't be opened yet so try back in a second or so.
		ent->nextthink = level.time + 2000;
	}
}

//-------------------------------------------
void touch_stasis_door_trig( gentity_t *ent, gentity_t *other, trace_t *trace )
{
	// The door is solid so it's ok to open it, otherwise,
	//	the door is already open and we don't need to bother with the state change
	if ( ent->owner->count == DOOR_CLOSED )
	{
		toggle_stasis_door( ent->owner );
	}
}

//-------------------------------------------
void use_stasis_door( gentity_t *ent, gentity_t *other, gentity_t *activator )
{
	// If there isn't already a door trigger there, now might be a good time to add one.
	if ( ent->trigger_formation )
	{
		spawn_stasis_door_trig( ent );
		ent->trigger_formation = qfalse;
	}

	// The door is solid so it's ok to open it, otherwise,
	//	the door is already open and we don't need to bother with the state change
	if ( ent->count == DOOR_CLOSED )
	{
		toggle_stasis_door( ent );
	}
}

/*
-------------------------------------------
spawn_stasis_door_trig

Create trigger
-------------------------------------------
*/

void spawn_stasis_door_trig( gentity_t *ent ) 
{
	gentity_t	*other;
	vec3_t		mins, maxs;
	int			i, best;

	// find the bounds of everything on the team
	VectorCopy (ent->absmin, mins);
	VectorCopy (ent->absmax, maxs);

	// find the thinnest axis, which will be the one we expand
	best = 0;
	for ( i = 1 ; i < 3 ; i++ ) {
		if ( maxs[i] - mins[i] < maxs[best] - mins[best] ) {
			best = i;
		}
	}
	maxs[best] += 96;
	mins[best] -= 96;

	// create a trigger with this size
	other = G_Spawn ();

	VectorCopy (mins, other->mins);
	VectorCopy (maxs, other->maxs);
	other->owner = ent;
	other->contents = CONTENTS_TRIGGER;
	other->e_TouchFunc = touchF_touch_stasis_door_trig;

	// The trigger "controls" this stasis door, so store a reference to it ( for locked doors )
	ent->teammaster = other;

	gi.linkentity (other);
}

/*
---------------------------------------------
disable_stasis_door

When a stasis door has a target2, that door
will make the target2 door invisible and not
solid.
---------------------------------------------
*/
void disable_stasis_door( gentity_t *ent )
{
	gentity_t *target;

	target = G_Find( NULL, FOFS(targetname), ent->target2 );

	// Stash the related door, even if there wasn't one, so we don't have to bother with looking it up later
	ent->teamchain = target;

	if ( target == NULL )
		// FIXME: It may be more appropriate to dump out an error message here and do any other things necessary
		return;

	// Cool, we found the other door, so make the thing go away for now.
	target->count = DOOR_OPEN;
	target->svFlags |= SVF_NOCLIENT;
	target->contents = 0;
	target->s.eFlags |= EF_NODRAW;

	// Store this other model as part of this door team.
	ent->team = ent->model2;

	// Now get the real model we want to use in this case
	ent->model2 = "models/mapobjects/stasis/door2.md3";
}

//-------------------------------------------
/*QUAKED func_stasis_door (0 .5 .8) ? x x x TOGGLE x x x PLAYER_USE
A bmodel that just sits there and opens when a player gets close to it.

PLAYER_USE	Player can use it with the use button
TOGGLE		wait in both the start and end states for a trigger event

"color"		constantLight color
"light"		constantLight radius
"target2"	targetting another func_stasis_door makes it have locked door behaviour

*/
void SP_func_stasis_door( gentity_t *ent ) 
{
	gi.SetBrushModel( ent, ent->model );

	G_SoundIndex( "sound/movers/doors/stasisdoor.wav" );
	G_SoundIndex( "sound/movers/switches/stasisneg.wav" );

	// Stasis doors have a model2, so precache me now
	G_ModelIndex( "models/mapobjects/stasis/door2.md3" );
	ent->model2 = "models/mapobjects/stasis/door.md3";
	InitMover( ent );
	
	// Now that we have the model precached, clear this out so it doesn't draw the model
	//	until we are ready to do the actual fade.
	ent->s.modelindex2 = 0;

	// sigh...ent->s.origin seems to be some kind of translational offset for the brush....so don't try and set 
	//	the "correct" door origin because it should actually be <0 0 0>..hence the stashing of the origin in pos1.
	VectorAdd( ent->absmax, ent->absmin, ent->pos1 );
	VectorScale( ent->pos1, 0.5f, ent->pos1 );
	VectorCopy( ent->pos1, ent->pos2 );
	
	G_SetOrigin( ent, ent->s.origin );

	if ( ent->targetname || ent->spawnflags & 128 )
	{
		// door must be used in order to work
		ent->trigger_formation = qtrue;

		// This is one of those crazy locked doors that need to do crazy things
		if ( ent->target2 )
		{
			// We'll have to change the state of the target2 door so that it isn't visible yet
			ent->e_ThinkFunc = thinkF_disable_stasis_door;
			ent->nextthink = level.time + FRAMETIME * 5; // give the target a chance to spawn in
		}
	}
	else
	{
		// Auto create a door trigger so the designers don't have to
		ent->e_ThinkFunc = thinkF_spawn_stasis_door_trig;
		ent->nextthink = level.time + FRAMETIME * 5; // give the target a chance to spawn in
		ent->trigger_formation = qfalse;
	}

	ent->e_UseFunc = useF_use_stasis_door;
	ent->svFlags |= SVF_STASIS_DOOR;
	ent->count = DOOR_CLOSED;

	gi.linkentity (ent);
	// evil name hacking
	ent->classname = "func_door";
}
