//g_ambient.cpp
//Ambient creatures, effects, hazards, etc.
#include "g_local.h"
#include "g_functions.h"
#include "b_local.h"

extern void G_StartObjectMoving( gentity_t *object, vec3_t dir, float speed, trType_t trType );
extern void G_RunObject( gentity_t *ent );
extern void G_StopObjectMoving( gentity_t *object );
extern void CG_ElectricalExplosion( vec3_t start, vec3_t end, float radius );
extern void CG_Chunks( int owner, vec3_t origin, const vec3_t normal, float speed, int numChunks, material_t chunkType, int customChunk, float baseScale );
extern void CG_StasisFixitsThink ( centity_t *cent );
extern void CG_StasisFlierAttackThink ( centity_t *cent );
extern void CG_StasisFlierIdleThink ( centity_t *cent );
extern void CG_StasisFlierChildDeath ( vec3_t pos );
extern void CalcEntitySpot ( gentity_t *ent, spot_t spot, vec3_t point );
extern void G_SetEnemy( gentity_t *self, gentity_t *enemy );

void SP_ambient_etherian_fliers_child (gentity_t *self);

#define	FLIER_HOMELESS			1
#define FLIER_CHILD_MIN_SPEED	5
#define FLIER_CHILD_SPEED_RANGE	800
#define FLIER_CHILD_MIN_DIST	48
#define FLIER_CHILD_DIST_RANGE	128
#define FLIER_ATTACK_TIME		6000
#define FLIER_NOATTACK_TIME_MIN 1000
#define FLIER_NOATTACK_TIME_MAX 3000
#define FLIER_CHILD_DAMAGE		1
#define FLIER_REACHED_TARGET	48
#define FLIER_INSPECT_DAMAGE	4

/*QUAKED ambient_etherian_mine (1 0 0) (-8 -8 -30) (8 8 8)
Floats in position
Homes in on anything within it's radius (default is 160)
Impact with solid objects makes it explode with Etherian Stasis Energy effect
Damage affects other gords, does knockback
Can be shot, 1 hit one kill.

splashDamage and splashRadius - explosion size and damage
radius - how close someone has to be to go after them
speed - movement speed (default is 40)
target -fires them when explode

TODO:
	Allow targetting to explode when used
	Allow to start dormant and wake up?
	Allow setting of health?
*/
//---------------------------------------
void SP_ambient_etherian_mine (gentity_t *self)
{
	G_SetOrigin(self, self->s.origin);
	//pos1 is where to return to
	VectorCopy(self->s.origin, self->pos1);

	self->s.eType = ET_GENERAL;
	self->s.modelindex = G_ModelIndex("models/mapobjects/stasis/mine.md3");
	G_SoundIndex("sound/weapons/explosions/mine1.wav");	//precache
	G_SoundIndex("sound/weapons/explosions/mine2.wav");	//precache
	G_SoundIndex("sound/weapons/explosions/mine3.wav");	//precache

	if ( self->radius <= 0 )
		self->radius = 160;

	if ( self->speed <= 0 )
		self->speed = 40;

	VectorSet( self->mins, -8, -8, -30 );
	VectorSet( self->maxs, 8, 8, 8 );

	self->health = 1;
	self->takedamage = qtrue;
	self->contents = CONTENTS_SOLID;
	self->e_DieFunc  = dieF_mine_die;
	self->e_TouchFunc  = touchF_mine_touch;
	self->e_ThinkFunc  = thinkF_mine_think;
	self->nextthink = level.time + FRAMETIME;
	self->clipmask = MASK_NPCSOLID;
	self->noDamageTeam = TEAM_STASIS;

	if ( self->splashDamage <= 0 )
		self->splashDamage = 30;

	if ( self->splashRadius <= 0 )
		self->splashRadius = 64;

	self->s.eFlags |= EF_ANIM_ALLFAST;

	gi.linkentity(self);
}

//---------------------------------------
void mine_think (gentity_t *self)
{
	//FIXME: if not even in PVS of player, don't think at all, maybe should stop too?
	gentity_t *target;
	gentity_t *entity_list[MAX_GENTITIES];
	int	count;
	float	dist, speed = self->speed;
	float	bestDist = Q3_INFINITE;
	qboolean doMove = qfalse;
	qboolean changeDir = qfalse;
	vec3_t	 vec, targOrg;

	if ( !Q_irand(0, 3) )
	{
		count = G_RadiusList( self->currentOrigin, self->radius, self, qtrue, entity_list );
		for ( int i = 0; i < count; i++ )
		{
			target = entity_list[i];

			if ( ( target == self) || ( target == self->owner ) )
				continue;

			if ( target->client && target->client->playerTeam == TEAM_STARFLEET && !(target->flags & FL_NOTARGET) )
			{
				VectorCopy( target->currentOrigin, targOrg );
				targOrg[2] += target->client->ps.viewheight;

				VectorSubtract( targOrg, self->pos1, vec );
				dist = VectorLengthSquared( vec );
				if ( dist < self->radius*self->radius )
				{//Close to my start spot
					VectorSubtract( targOrg, self->currentOrigin, vec );
					dist = VectorLengthSquared( vec );
					if ( dist < bestDist )//all things equal, keep current
					{
						if ( !self->enemy )
						{//Aquired new enemy
							self->pushDebounceTime = 0;
						}
						G_SetEnemy(self, target);
						bestDist = dist;
					}
				}
			}
		}
	}

	if ( self->pushDebounceTime > level.time )
	{//Just drifting
		doMove = qtrue;
	}
	else
	{
		if ( self->enemy )
		{//31337 44!
			if ( self->aimDebounceTime <= level.time )
			{
				VectorCopy( self->enemy->currentOrigin, targOrg );
				targOrg[2] += self->enemy->client->ps.viewheight;

				VectorSubtract( targOrg, self->currentOrigin, self->movedir );
				dist = VectorNormalize( self->movedir );
				if ( dist < self->radius )
				{
					changeDir = qtrue;
					doMove = qtrue;
				}
				else
				{
					self->enemy = NULL;
				}
			}
		}

		if ( !self->enemy )
		{
			vec3_t	homeDir;

			changeDir = qtrue;
			doMove = qtrue;

			VectorSubtract( self->pos1, self->currentOrigin, homeDir );
			dist = VectorNormalize( homeDir );

			if ( dist > self->speed / 4 )
			{//Trying to get back to center
				VectorCopy( homeDir, self->movedir );
			}
			else
			{//hover about
				speed *= 0.25f;
				VectorSet( self->movedir, homeDir[0] + crandom(), homeDir[1] + crandom(), homeDir[2] + crandom() );
				self->pushDebounceTime = level.time + 500 + random() * 200;
			}
		}
	}
	
	//FIXME: accelerate to top speed?
	if ( doMove )
	{
		G_RunObject(self);
		if ( changeDir )
		{
			G_StartObjectMoving( self, self->movedir, speed, TR_LINEAR );
		}
	}
	else
	{
		G_StopObjectMoving( self );
	}

	self->nextthink = level.time + FRAMETIME;
}

//---------------------------------------
void mine_explode( gentity_t *self )
{
	if ( self->attackDebounceTime <= level.time )
	{
		if ( self->target )
		{
			G_UseTargets( self, self->enemy );
		}

		if ( self->splashDamage > 0 && self->splashRadius > 0 )
		{
			G_TempEntity( self->currentOrigin, EV_STASIS_MINE_EXPLODE );
			G_RadiusDamage( self->currentOrigin, self, self->splashDamage, self->splashRadius, self, MOD_UNKNOWN );
		}
		G_Sound ( self, G_SoundIndex ( va( "sound/weapons/explosions/mine%d.wav", Q_irand(1, 3) ) ) );

		G_FreeEntity( self );
	}
	else
	{
		self->s.eFlags |= EF_SCALE_UP;
		VectorSet( self->currentAngles, self->currentAngles[0]+crandom(), self->currentAngles[1]+crandom()*3, self->currentAngles[2]+crandom() );
		G_SetAngles( self, self->currentAngles );
		self->nextthink	= level.time + FRAMETIME;
	}
}

//---------------------------------------
void mine_die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod )
{
	//Do explosion
	self->health = 0;
	self->takedamage = qfalse;

	//Throw some chunks?
	self->e_PainFunc = painF_NULL;
	self->e_DieFunc  = dieF_NULL;
	self->e_TouchFunc  = touchF_NULL;
	self->e_ThinkFunc  = thinkF_mine_explode;
	self->nextthink	= level.time + FRAMETIME;
	self->attackDebounceTime = level.time + Q_irand(300, 500);

	G_SetEnemy(self, attacker);
}

//---------------------------------------
void mine_touch (gentity_t *self, gentity_t *other, trace_t *trace)
{
	//If valid enemy, explode, otherwise, if another mine, push it, otherwise bounce off
	if ( other->client && other->client->playerTeam && other->client->playerTeam != self->noDamageTeam )
	{//Hit something, go boom
		self->attackDebounceTime = 0;
		G_SetEnemy(self, other);
		mine_explode( self );
	}
	else if ( self->s.pos.trDelta && Q_stricmp( other->classname, self->classname ) == 0 )
	{//I'm moving and hit another dude
		//Drift for a second
		G_StartObjectMoving( self, self->movedir, self->speed, TR_LINEAR );
		self->pushDebounceTime = level.time + 1000;
	}
	else if ( other->s.number >= ENTITYNUM_WORLD )//Hit wall, bounce off
	{//FIXME: use normal of wall to determine bounce angle?
		G_StartObjectMoving( self, self->movedir, -self->speed, TR_LINEAR );
		self->pushDebounceTime = level.time + 1000;
	}
	else if ( !self->s.pos.trDelta )
	{//I'm not moving, push me
		vec3_t center;

		VectorSubtract(other->absmax, other->absmin, center);//size
		VectorMA(other->absmin, 0.5, center, center);//center
		VectorSubtract( self->currentOrigin, center, self->movedir );

		//FIXME: get speed from THEM?
		G_StartObjectMoving( self, self->movedir, self->speed, TR_LINEAR );
		self->pushDebounceTime = level.time + 1000;
	}
	else
	{
		G_StopObjectMoving( self );
	}
}


/*QUAKED ambient_etherian_fliers (1 0 0) (-8 -8 -8) (8 8 8)
Fly around on paths, occaisionally swoop down and do a melee-range discharge and return to path

"target" them at a circular series of path_corners

"target2" - What they fire when they die
"splashDamage" - Damage their melee attack does per frame (default 3)
"splashRadius" - Size of their melee attack (default 64)
"radius" - How close they can be before I pick them up and attack (default 512)
"health" - (default 40)

TODO:
	Allow custom speed...?
*/

void flier_pain (gentity_t *self, gentity_t *attacker, int damage)
{//move? Anim? scream? Change dir?  Pitch/roll?
	//spin/drift?
	if ( !self->enemy && 
		(attacker->noDamageTeam != self->noDamageTeam || (attacker->client && attacker->client->playerTeam != self->noDamageTeam)) )
	{
		G_SetEnemy(self, attacker);
	}
}

//---------------------------------------
void flier_die ( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod )
{//drift, scream and disintegrate

	if ( self->target2 && self->target2[0] )
	{
		G_UseTargets2(self, attacker, self->target2);
	}
	self->health = 0;
	self->takedamage = qfalse;
	//Throw some chunks?
	self->e_PainFunc = painF_NULL;
	self->e_DieFunc  = dieF_NULL;
	self->e_ThinkFunc  = thinkF_mine_explode;
	self->nextthink	= level.time + FRAMETIME;
	//VectorCopy( self->currentOrigin, self->s.origin );

//	self->attackDebounceTime = level.time + Q_irand(300, 500);
}

//---------------------------------------
extern void CG_Line( vec3_t start, vec3_t end, vec3_t color, float alpha );
float flier_move( gentity_t *self, vec3_t destOrg, float turnRate, qboolean modTurnSpeeds )
{
	vec3_t	oldDir, newMoveDir, newAngles, dirDiff;
	float	moveDist=0;
	float	moveSpeed;

	EvaluateTrajectory( &self->s.apos, level.time, self->s.apos.trBase );

	if( self->s.pos.trType == TR_GRAVITY )
	{	// so this will inherit whatever the flier was doing when it died
		// dammit - do this manually because the gravity stuff isn't right.
		vec3_t velVect;

		VectorScale( self->movedir, self->speed, velVect );
		velVect[2] -= DEFAULT_GRAVITY * ( FRAMETIME / 1000.0 ) * .5;	// one quarter of gravity to look floaty
		VectorCopy( velVect, self->movedir );
		self->speed = VectorNormalize( self->movedir );

		G_StartObjectMoving( self, self->movedir, self->speed, TR_GRAVITY );
	}
	else
	{
		VectorSubtract( destOrg, self->currentOrigin, newMoveDir );
		vectoangles( newMoveDir, newAngles );
		moveDist = VectorNormalize( newMoveDir );

		VectorScale( self->movedir, 1.0 - turnRate, oldDir );
		VectorScale( newMoveDir, turnRate, newMoveDir );

		VectorAdd( oldDir, newMoveDir, self->movedir );
		VectorNormalize( self->movedir );
		//set movement
		//FIXME: mod speed so they slow down on turns
		//Speed slows as the change in moveDir increases
		VectorSubtract( self->movedir, oldDir, dirDiff );
		if( modTurnSpeeds )
		{
			moveSpeed = (2 - VectorLength( dirDiff )) * 0.5 * self->speed;
		}
		else
		{
			moveSpeed = self->speed;
		}

		G_StartObjectMoving( self, self->movedir, moveSpeed, TR_LINEAR );
	}


	// make the angles correct
	vec3_t destinationAngle;
	vectoangles( self->movedir, destinationAngle );

	AnglesSubtract( destinationAngle, self->s.apos.trBase, self->s.apos.trDelta );

	//since this is in terms of units per second but we need the amount for a frame
	VectorScale( self->s.apos.trDelta, 1000/FRAMETIME, self->s.apos.trDelta );
	self->s.apos.trTime = level.time;


	return moveDist;
}

//---------------------------------------
void flier_find_first_path_corner( gentity_t *self )
{
	if ( !self->target || !self->target[0] )
	{//Nowhere to start!
		if ( !self->etherian_fixit )
		{
			//gi.Error( "NO start path_corner for etherian flier\n" );
		}
	}
	self->activator = G_Find( NULL, FOFS(targetname), self->target );
	if ( !self->activator )
	{//Nowhere to start!
		if ( !self->etherian_fixit )
		{
			//gi.Error( "Can't find start path_corner %s for etherian flier\n", self->target );
		}
	}
}

//---------------------------------------
void flier_check_attack( gentity_t *self )
{
	if ( self->etherian_fixit )
	{//FIXME: heal
	}
	else
	{
		if ( self->attackDebounceTime > level.time )
		{
			//Do effect
			self->e_clThinkFunc = clThinkF_CG_StasisFlierAttackThink;
			//Do radius damage
			G_RadiusDamage( self->currentOrigin, self, self->splashDamage, self->splashRadius, self, MOD_STASIS );
		}
		else
		{
			self->e_clThinkFunc = clThinkF_NULL;
		}
	}
}

//---------------------------------------
qboolean flier_reached_path_corner( gentity_t *self )
{
	vec3_t	vec;
	float	dist;

	if ( !self->activator )
	{
		return qfalse;
	}

	VectorSubtract(self->currentOrigin, self->activator->currentOrigin, vec);
	dist = VectorLengthSquared(vec);

	if(dist < 256)//16 squared
	{
		if ( !self->etherian_fixit )
		{
			if( !Q_irand(0, 3) )
			{
				//G_Sound( self, G_SoundIndex( "sound/enemies/eflyer/?.wav" ) );
			}
		}
		return qtrue;
	}

	return qfalse;
}

//---------------------------------------
qboolean flier_check_valid_enemy( gentity_t *self )
{
	if( !self->enemy )
	{//none
		return qfalse;
	}

	if ( self->etherian_fixit )
	{//FIXME: see if still exists and still damaged
		if( !(self->enemy->svFlags & SVF_BROKEN) )
		{//already fixed
			self->s.eFlags &= ~EF_EYEBEAM;
			self->enemy = NULL;
			return qfalse;
		}
	}
	else
	{
		if( self->enemy->flags & FL_NOTARGET )
		{//notarget
			self->enemy = NULL;
			return qfalse;
		}
		if( !self->enemy->health )
		{//Dead
			self->enemy = NULL;
			return qfalse;
		}
		else if ( !gi.inPVS( self->currentOrigin, self->enemy->currentOrigin ) )
		{//Not potentially visible
			self->enemy = NULL;
			return qfalse;
		}
	}

	return qtrue;
}

//---------------------------------------
void flier_find_enemy (gentity_t *self, qboolean alwaysAcquire )
{
	gentity_t *target;
	gentity_t *entity_list[MAX_GENTITIES];
	vec3_t vec;
	float	dist, bestDist = Q3_INFINITE;

	if ( Q_irand(0, 1) && !alwaysAcquire )
	{
		return;
	}

	if( !self->etherian_fixit )
	{	// we only bother with the player - we make not so much sense for the pals
		//g_entities[0]
		trace_t	tr;

		gi.trace( &tr, self->currentOrigin, vec3_origin, vec3_origin, g_entities[0].currentOrigin, self->s.number, MASK_SHOT );

		if ( tr.fraction == 1.0f || tr.entityNum == g_entities[0].s.number )
		{
			G_SetEnemy(self, &g_entities[0]);
		}
		else
		{
			return;
		}
	}
	else
	{
		int count = G_RadiusList( self->currentOrigin, 1024, self, !self->etherian_fixit, entity_list );
		for ( int i = 0; i < count; i++ )
		{
			target = entity_list[i];

			if ( target == self )
			{
				continue;
			}

			if ( target->svFlags & SVF_BROKEN )
			{
				VectorSubtract( target->currentOrigin, self->currentOrigin, vec );
				dist = VectorLengthSquared( vec );
				if ( dist < bestDist )//all things equal, keep current
				{
					if ( gi.inPVS( self->currentOrigin, target->currentOrigin ) )
					{//NAV_ClearPathToPoint changed to always add monsterclip, so have to do our own trace now
						trace_t	trace;

						gi.trace( &trace, self->currentOrigin, self->mins, self->maxs, target->currentOrigin, self->s.number, self->clipmask );

						if( ( ( trace.startsolid == qfalse ) && ( trace.allsolid == qfalse ) ) && ( trace.fraction == 1.0f ) )
						{
							G_SetEnemy(self, target);
							bestDist = dist;
						}
					}
				}
			}
		}
	}
}

//---------------------------------------
void fixit_touch ( gentity_t *self, gentity_t *other, trace_t *trace )
{
	if ( other->s.number == ENTITYNUM_WORLD )
	{
		if ( VectorLengthSquared( trace->plane.normal ) )
		{
			vec3_t	oldDir, newMoveDir;

			VectorScale( self->movedir, 0.3, oldDir );
			VectorScale( trace->plane.normal, 0.7, newMoveDir );

			VectorAdd( oldDir, newMoveDir, self->movedir );
			VectorNormalize( self->movedir );
		}
	}
}

//---------------------------------------
void flier_child_touch ( gentity_t *self, gentity_t *other, trace_t *trace )
{
	G_RadiusDamage( self->currentOrigin, self, FLIER_CHILD_DAMAGE, 36, self, MOD_STASIS );

	CG_StasisFlierChildDeath( self->currentOrigin );
	//CG_ElectricalExplosion( self->currentOrigin, self->currentOrigin, self->splashRadius );
	G_Sound ( self, G_SoundIndex ( va( "sound/weapons/explosions/mine%d.wav", Q_irand(1, 3) ) ) );

	G_FreeEntity( self );
}

//---------------------------------------
void flier_fire_children (gentity_t *attacker)
{
	int curChild = 0;

	gentity_t *curCheck = NULL;

	while( (curCheck = G_Find( curCheck, FOFS(classname), "flier_child" )) != NULL )
	{
		if( curCheck->owner == attacker )
		{
			VectorCopy( g_entities[0].currentOrigin, curCheck->pos1 );
			curCheck->attackDebounceTime = level.time + ( curChild * 200 );
			curCheck->e_clThinkFunc = clThinkF_CG_StasisFlierAttackThink;
			//curCheck->enemy = &g_entities[0]; // always target the player - this might be bad, though.  Who knows.

			if( Q_irand(0, 3) == 0 )
			{
				curChild++;
			}
		}
	}
}

//---------------------------------------
void flier_follow_path (gentity_t *self)
{//FIXME: don't think when enemy out of PVS?
	if ( !self->etherian_fixit )
	{
		if ( self->speed > 140 )
		{
			self->speed -= 10;
		}
	}
	self->nextthink = level.time + FRAMETIME;
	G_RunObject( self );

	flier_check_valid_enemy( self );
	flier_check_attack( self );

	if ( !self->enemy )
	{//Look for enemies
		flier_find_enemy( self, qfalse );
	}

	if ( self->enemy && self->aimDebounceTime < level.time)
	{
		if( self->etherian_fixit )
		{
			//FIXME: Decide whether or not to swoop
			if ( !self->etherian_fixit )
			{
				//G_Sound( self, G_SoundIndex( "sound/enemies/eflyer/dssklatk.wav" ) );
			}
			self->e_ThinkFunc = thinkF_flier_swoop_to_enemy;
		}
		else
		{
			self->enemy = NULL;
			//	G_SetEnemy(self, NULL);//?
			flier_find_enemy ( self, qtrue );
			if( self->enemy )
			{
				// assault the player with my guys
				flier_fire_children( self );
				self->aimDebounceTime = level.time + FLIER_ATTACK_TIME;
			}
			else
			{
				self->aimDebounceTime = level.time + Q_irand(FLIER_NOATTACK_TIME_MIN, FLIER_NOATTACK_TIME_MAX);
			}
		}
	}
	else
	{//Follow path, with some variance
		qboolean goToNext = qfalse;

		if ( self->activator )
		{//We're already heading to a path_corner
			if ( flier_reached_path_corner( self ) )
			{
				goToNext = qtrue;
			}
		}
		else
		{//First time, let's grab one
			flier_find_first_path_corner( self );
			goToNext = qtrue;
		}
		
		if ( goToNext && self->activator )
		{//Need to find a new path_corner
			//Should always find one, else: ERROR!  Must loop!
			if ( self->activator->target && self->activator->target[0] )
			{//Find our next path_corner
				self->activator = G_Find( NULL, FOFS(targetname), self->activator->target );
			}
		}

		if ( self->activator )
		{
			flier_move( self, self->activator->currentOrigin, 0.3f, qtrue );
		}
	}
}

void misc_model_breakable_init( gentity_t *ent );
void flier_swoop_to_enemy (gentity_t *self)
{
	if ( !self->etherian_fixit )
	{
		if ( self->speed < 300 )
		{
			self->speed += 10;
		}
	}
	self->wait = 0;// will need to reacquire path
	self->nextthink = level.time + FRAMETIME;
	G_RunObject( self );
	//Home on on enemy
	flier_check_valid_enemy( self );
	flier_check_attack( self );
	if ( self->enemy )
	{
		vec3_t targOrg, vec;
		float dist;

		VectorCopy( self->enemy->currentOrigin, targOrg );
		if ( !self->etherian_fixit )
		{//flier
			targOrg[2] += self->enemy->client->ps.viewheight;
		}
		VectorSubtract( targOrg, self->currentOrigin, vec );
		dist = VectorLengthSquared( vec );

		if ( dist < (self->splashRadius*self->splashRadius) )
		{//attack!
			if ( !self->etherian_fixit )
			{//Zap it
				//G_Sound( self, G_SoundIndex( "sound/enemies/etherians/attack.wav" ) );
				G_Sound( self, G_SoundIndex( "sound/weapons/stasis_alien/fire.wav" ) );
				//G_Sound( self, G_SoundIndex( "sound/enemies/borg/borgtaser.wav" ) );
				self->aimDebounceTime = level.time + 99999999;
				self->attackDebounceTime = level.time + 2000;
				self->e_ThinkFunc = thinkF_flier_return_to_path;
			}
			else
			{//Heal it
				if ( self->enemy->health >= self->enemy->max_health )
				{
					if ( self->enemy->health == self->enemy->max_health )
					{
						//Do this only the first time
						misc_model_breakable_init( self->enemy );

						// Make sure that the animation restarts when it gets fixed
						self->enemy->s.frame = 0;
						self->enemy->s.eFlags &= ~ EF_ANIM_ALLFAST;
						self->enemy->s.eFlags |= EF_ANIM_ONCE;

						if ( self->enemy->target2 && self->enemy->target2[0] )
						{//Fixing a breakable uses its target2
							G_UseTargets2( self->enemy, self, self->enemy->target2 );
						}
					}
					
					if ( self->spawnflags & FLIER_HOMELESS )
					{//Stay here until we find something else to fix
						self->activator = NULL;
						self->target = NULL;
						VectorCopy( self->currentOrigin, self->fixit_start_position );
					}

					self->enemy->svFlags &= ~SVF_BROKEN;

					self->s.eFlags &= ~EF_EYEBEAM;
					self->aimDebounceTime = level.time + 300;
					self->attackDebounceTime = level.time + 100;
					self->e_ThinkFunc = thinkF_flier_return_to_path;
				}
				else
				{
					int	saveSpeed = self->speed;

					if ( ! (self->enemy->s.eFlags & EF_FIXING ) )
					{
						// Fun fixing has now begun
						self->enemy->s.eFlags |= EF_FIXING;
						self->enemy->fx_time = level.time;
						
						G_AddEvent( self, EV_GENERAL_SOUND, G_SoundIndex( "sound/ambience/stasis/fireflyfixed.wav" ));
					}

					// Fixing is based on time because of the shader effect used, it animates at a set rate, so
					//	we should sync up to that if we have any hopes of the effect looking like we want.
					if ( self->enemy->fx_time + 1500 < level.time )
					{
						//Yeah! Fixing is done!
						self->enemy->health = self->enemy->max_health;
					}
					self->s.eFlags |= EF_EYEBEAM;
					self->speed = 20;
					flier_move( self, targOrg, 0.3f, qtrue );
					self->speed = saveSpeed;
				}
			}
		}
		else
		{
			flier_move( self, targOrg, 0.3f, qtrue );
		}
	}
	else
	{
		self->e_ThinkFunc = thinkF_flier_return_to_path;
	}
	//When in range, start discharge timer
	//Once started discharge, begin return_to_path thinking
}

void flier_return_to_path (gentity_t *self)
{
	if ( !self->etherian_fixit )
	{
		if ( self->speed > 140 )
		{
			self->speed -= 10;
		}
	}
	self->nextthink = level.time + FRAMETIME;
	G_RunObject( self );

	flier_check_valid_enemy( self );
	//If discharge timer on, keep effect going
	flier_check_attack( self );

	if ( self->etherian_fixit && !self->enemy && self->aimDebounceTime > level.time )
	{//FIXITs can look for enemies on the way back
		flier_find_enemy( self, qfalse );
	}

	if ( self->enemy && self->aimDebounceTime < level.time )
	{
		//FIXME: Decide whether or not to swoop
		if ( !self->etherian_fixit )
		{
			//need sounds!
			//G_Sound( self, G_SoundIndex( "sound/enemies/eflyer/dssklatk.wav" ) );
		}
		self->e_ThinkFunc = thinkF_flier_swoop_to_enemy;
	}
	else
	{
		gentity_t	*pCorner = NULL;
		vec3_t		vec;
		float		dist, bestDist = Q3_INFINITE;

		//FIXME: if no path, return to spawn spot?
		if ( !self->activator )
		{
			if( !self->target || !self->target[0] )
			{
				VectorSubtract( self->fixit_start_position, self->currentOrigin, vec );
				if ( VectorLengthSquared( vec ) < 256 )
				{
					if ( !self->etherian_fixit )
					{
						self->aimDebounceTime = level.time + 3000;
					}
					G_StopObjectMoving( self );
					self->e_ThinkFunc = thinkF_flier_follow_path;
				}
				else
				{
					flier_move( self, self->fixit_start_position, 0.3f, qtrue );
				}
				return;
			}
			//If don't have a path corner, pick one - what if we followed the player into an area without one?
			flier_find_first_path_corner( self );
			if ( !self->activator )
			{
				return;
			}
		}

		if ( flier_reached_path_corner( self ) )
		{
			//gi.Printf( "%s got on path\n", self->classname );
			//When reach path corner, start follow_path behavior
			if ( !self->etherian_fixit )
			{
				self->aimDebounceTime = level.time + 3000;
			}
			self->e_ThinkFunc = thinkF_flier_follow_path;
			return;
		}
		//Find our closest path_corner in PVS
		if ( !self->wait )
		{//Off path
			while ( (pCorner = G_Find( pCorner, FOFS(targetname), self->activator->target )) != NULL )
			{
				VectorSubtract( pCorner->currentOrigin, self->pos1, vec );
				dist = VectorLengthSquared( vec );
				if ( dist < bestDist )
				{//closest so far
					//NAV_ClearPathToPoint changed to always add monsterclip, so have to do our own trace now
					if ( gi.inPVS( self->currentOrigin, pCorner->currentOrigin ) )
					{
						trace_t	trace;

						gi.trace( &trace, self->currentOrigin, self->mins, self->maxs, pCorner->currentOrigin, self->s.number, self->clipmask );

						if( ( ( trace.startsolid == qfalse ) && ( trace.allsolid == qfalse ) ) && ( trace.fraction == 1.0f ) )
						{
							self->activator = pCorner;
							self->wait = 1;//on path now
							bestDist = dist;
						}
					}
				}
			}
		}
	}

	if ( self->activator )
	{//Head back to the path corner
		flier_move( self, self->activator->currentOrigin, 0.3f, qtrue );
	}
}

void adjust_flier_child_dest (vec3_t inVect, int randomKey)
{
	// this is pretty gruesome
	
	// the first number scales the period of the cosine wave,
	// the second number intensifies the randomKey,
	// and the third number is the height of the cosine wave (basically distance of variation)
	inVect[0] += cos( level.time * .0013 + (randomKey*26) ) * 15;
	inVect[1] += cos( level.time * .0017 + (randomKey*17)) * 25;
	inVect[2] += cos( level.time * .002 + (randomKey*31)) * 25;
}

void flier_child (gentity_t *self)
{
	vec3_t	destDist;
	float	destLen;
	vec3_t	destPos;

	self->nextthink = level.time + FRAMETIME;
	G_RunObject( self );

	if( self->inuse && self->owner )
	{	// only adjust this stuff if we're still following the parent
		if( self->owner->inuse == qfalse )
		{	// if our owner is dead, fall to the ground lifeless
			self->s.pos.trType = TR_GRAVITY;
			self->e_TouchFunc  = touchF_flier_child_touch;
			self->owner = NULL;
			VectorCopy( self->currentOrigin, destPos ); //?
			self->clipmask = MASK_PLAYERSOLID;
			//self->speed *= .2; // slow down as we fall out of the air
		}
		else if( self->owner )
		{
			float targetSpeed;

			// locate my destination
			if( self->attackDebounceTime > 0 && self->attackDebounceTime <= level.time )
			{	// I am on the attack... So go there until I hit it.
				VectorCopy( self->pos1, destPos );

				// See if we got there yet
				vec3_t diff;

				VectorSubtract( self->pos1, self->currentOrigin, diff );
				if( VectorLength( diff ) < FLIER_REACHED_TARGET )
				{
					self->attackDebounceTime = 0;
					self->e_clThinkFunc = clThinkF_CG_StasisFlierIdleThink;
				}
			}
			else
			{
				VectorCopy( self->owner->currentOrigin, destPos );
			}

			if( self->attackDebounceTime > 0 && self->attackDebounceTime <= level.time )
			{
				G_RadiusDamage( self->currentOrigin, self, FLIER_INSPECT_DAMAGE, 36, self, MOD_STASIS );
			}

			adjust_flier_child_dest( destPos, (int)(intptr_t)self );	// randomKey = per-entity wobble seed; low-32 of ptr is fine (LP64-safe cast)

			VectorSubtract( destPos, self->currentOrigin, destDist);
			destLen = VectorLength( destDist );

			if( destLen > FLIER_CHILD_DIST_RANGE + FLIER_CHILD_MIN_DIST )
			{
				// out of range - go maximum speed
				targetSpeed = FLIER_CHILD_SPEED_RANGE + FLIER_CHILD_MIN_SPEED;
			}
			else if( destLen < FLIER_CHILD_MIN_DIST )
			{
				// too close - go minimum speed
				targetSpeed = FLIER_CHILD_MIN_SPEED;
			}
			else
			{
				float ratio;

				ratio = ( destLen - FLIER_CHILD_MIN_DIST ) / FLIER_CHILD_DIST_RANGE;
				//ratio *= ratio;	// since this will always be less than one, it will be a lower curve (this mult makes it smaller)
				targetSpeed = ratio * FLIER_CHILD_SPEED_RANGE + FLIER_CHILD_MIN_SPEED;
			}

			if( targetSpeed > self->speed + 90 )
			{
				self->speed += 90;
			}
			else
			{
				self->speed = targetSpeed;
			}
		}
	}

	if( self->inuse)
	{
		flier_move( self, destPos, 0.65f, qfalse );
	}
}

void SP_ambient_etherian_fliers_child (gentity_t *self)
{
	//how do I change this fellas size?

	self->s.scale = .05;

	G_SetOrigin(self, self->s.origin);
	VectorCopy( self->s.origin, self->fixit_start_position );

	self->s.eType = ET_THINKER;
	//self->s.modelindex = G_ModelIndex("models/mapobjects/stasis/mine.md3");
	//self->s.modelindex = G_ModelIndex("models/mapobjects/cargo/hypo.md3");
	self->s.modelindex = 0; // Is this correct?

	self->speed = 100;
	self->health = 40;

	//Set contents type and clipmask to contents_shotclip
	VectorSet( self->mins, -8, -8, -8 );
	VectorSet( self->maxs, 8, 8, 8 );
	self->takedamage = qfalse;
	self->contents = CONTENTS_NONE;
	self->clipmask = CONTENTS_NONE;//?
	self->noDamageTeam = TEAM_STASIS;

	//Put on first path corner
	self->e_ThinkFunc = thinkF_flier_child;
	self->nextthink = level.time + FRAMETIME;
	self->e_PainFunc = painF_flier_pain;
	self->e_DieFunc = dieF_flier_die;
	//Need a use func?  touch func?

	self->s.apos.trType = TR_LINEAR;

	self->etherian_fixit = qfalse;

	self->noDamageTeam = TEAM_STASIS;

	self->svFlags |= SVF_NO_TELEPORT;

	self->classname = "flier_child";

	// when these guys have an enemy, they will charge it
	// otherwise, they will just swarm after their parent
	self->enemy = NULL;

	self->e_clThinkFunc = clThinkF_CG_StasisFlierIdleThink;

	// give myself a creation timestamp
	//self->attackDebounceTime = level.time;

	//rather, this indicates that I should go to pos1 rather than my owner, 'till I hit it
	self->attackDebounceTime = 0;

	gi.linkentity(self);
}

void SP_ambient_etherian_fliers (gentity_t *self)
{
	G_SetOrigin(self, self->s.origin);
	VectorCopy( self->s.origin, self->fixit_start_position );

	self->s.eType = ET_THINKER;
	self->s.modelindex = G_ModelIndex("models/players/skull/flyingskull.md3");
	//G_SoundIndex( "sound/enemies/etherians/attack.wav" );
	G_SoundIndex( "sound/weapons/stasis_alien/fire.wav" );
	
	self->speed = 140;

	if ( self->splashDamage <= 0 )
	{
		self->splashDamage = 3;
	}

	if ( self->splashRadius <= 0 )
	{
		self->splashRadius = 64;
	}

	if ( self->radius <= 0 )
	{//How close they can be before I pick them up and attack
		self->radius = 512;
	}
	
	if ( self->health <= 0 )
	{
		self->health = 40;
	}

	//Set contents type and clipmask to contents_shotclip
	VectorSet( self->mins, -20, -20, -20 );
	VectorSet( self->maxs, 20, 20, 20 );
	self->takedamage = qtrue;
	self->contents = CONTENTS_BODY;
	self->clipmask = CONTENTS_SHOTCLIP;
	self->noDamageTeam = TEAM_STASIS;

	//Put on first path corner
	self->e_ThinkFunc = thinkF_flier_return_to_path;
	self->nextthink = level.time + FRAMETIME;
	self->e_PainFunc = painF_flier_pain;
	self->e_DieFunc = dieF_flier_die;
	//Need a use func?  touch func?

	self->s.apos.trType = TR_LINEAR;

	self->etherian_fixit = qfalse;

	self->noDamageTeam = TEAM_STASIS;

	self->svFlags |= SVF_NO_TELEPORT;

	// let it animate automatically
	self->s.eFlags |= EF_ANIM_ALLFAST;

	gi.linkentity(self);

	// set up the children for this thing
	for( int i = 0; i < 6; i++ ){
		gentity_t *child;
		child = G_Spawn();
		VectorCopy(self->s.origin, child->s.origin);
		SP_ambient_etherian_fliers_child(child);//probably missing a bunch of important fields this way
		child->owner = self;
	}

}

/*QUAKED ambient_etherian_fixits (1 0 0) (-8 -8 -8) (8 8 8) HOMELESS
Creates a group of fixit sprites - they will fly to
broken units in their view and fix them.

HOMELESS - Tells them to not try to return to their start position/path once they've fixed something... they'll just stay where they were.

"radius" - max radius for fixits (default 20)
"speed"  - how fast they swarm (default 100)
"count"  - count of fixits (max of 10, default 6)
"random" - how much variance in the radius 0-100 (default 40)
			0 = no variance; 100 = max variance

TODO:
	Give a path?
*/

void SP_ambient_etherian_fixits (gentity_t *self)
{//FIXME: implement
	G_SetOrigin( self, self->s.origin );
	VectorCopy( self->s.origin, self->fixit_start_position );
#ifdef _DEBUG
	self->s.modelindex = 0;
#endif	
	// Apply the defaults
	G_SpawnFloat( "radius", "20", &self->radius );
	G_SpawnFloat( "speed", "100", &self->speed );
	G_SpawnInt( "count", "6", &self->count );
	G_SpawnFloat( "random", "40", &self->random );

	if ( self->count > 10 )
	{
		Com_Printf( "ambient_etherian_fixits:  count was exceeded!" );
		self->count = 10;
	}

	self->splashRadius = 32;

	// pre-cache sounds
	self->s.loopSound = G_SoundIndex( "sound/ambience/stasis/fireflywhisper.wav" );
	G_SoundIndex( "sound/ambience/stasis/fireflyfixed.wav" );

	// Set a random start (pos1) and movement angle/speed(pos2) for the leader..the rest of the swarm will
	//	be generated by use of an angular offset table.
	VectorSet( self->pos1, crandom() * 360, crandom() * 360, 0 );
	VectorSet( self->pos2, crandom() * 0.4 * self->speed, crandom() * 0.4 * self->speed, 0 );

	VectorCopy( self->currentOrigin, self->s.pos.trBase );
	VectorClear( self->s.pos.trDelta );
	self->clipmask = CONTENTS_SHOTCLIP|CONTENTS_SOLID;
	self->s.pos.trType = TR_LINEAR;
	self->s.pos.trTime = level.time;

	VectorCopy( self->s.angles, self->s.apos.trBase );
	VectorClear( self->s.apos.trDelta );
	self->s.apos.trType = TR_LINEAR;
	self->s.apos.trTime = level.time;

	//Start thinking
	self->s.eType = ET_THINKER;
	self->e_clThinkFunc = clThinkF_CG_StasisFixitsThink;

	self->e_TouchFunc = touchF_fixit_touch;
	self->e_ThinkFunc = thinkF_flier_return_to_path;
	self->nextthink = level.time + FRAMETIME;

	self->etherian_fixit = qtrue;

	self->noDamageTeam = TEAM_STASIS;
	
	self->svFlags |= SVF_NO_TELEPORT|SVF_NONNPC_ENEMY;

	gi.linkentity(self);
}