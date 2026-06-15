#include "g_local.h"
#include "g_functions.h"

#define	MISSILE_PRESTEP_TIME	50
extern void CG_SurfaceExplosion( vec3_t origin, vec3_t normal, float radius, float shake_speed, qboolean smoke );
extern void AddSoundEvent( gentity_t *owner, vec3_t position, float radius, alertEventLevel_e alertLevel );
extern void AddSightEvent( gentity_t *owner, vec3_t position, float radius, alertEventLevel_e alertLevel );
qboolean LogAccuracyHit( gentity_t *target, gentity_t *attacker );

qboolean W_CheckBorgAdapt( gentity_t *attacker, int weapon, gentity_t *traceEnt, vec3_t endpos );

/*
================
G_BounceMissile

================
*/
void G_BounceMissile( gentity_t *ent, trace_t *trace ) {
	vec3_t	velocity;
	float	dot;
	int		hitTime;

	// reflect the velocity on the trace plane
	hitTime = level.previousTime + ( level.time - level.previousTime ) * trace->fraction;
	EvaluateTrajectoryDelta( &ent->s.pos, hitTime, velocity );
	dot = DotProduct( velocity, trace->plane.normal );
	VectorMA( velocity, -2*dot, trace->plane.normal, ent->s.pos.trDelta );

	if ( ent->s.eFlags & EF_BOUNCE_HALF ) 
	{
		VectorScale( ent->s.pos.trDelta, 0.5, ent->s.pos.trDelta );

		// check for stop
		if ( trace->plane.normal[2] > 0 && ent->s.pos.trDelta[2] < 40 ) 
		{
			G_SetOrigin( ent, trace->endpos );
			ent->nextthink = level.time + 500;
			return;
		}
	}

	VectorAdd( ent->currentOrigin, trace->plane.normal, ent->currentOrigin);
	VectorCopy( ent->currentOrigin, ent->s.pos.trBase );
	VectorCopy( trace->plane.normal, ent->pos1 );
	ent->s.pos.trTime = level.time;
}
 
/*
================
G_MissileStick

================
*/
void G_MissileStick( gentity_t *ent, trace_t *trace )
{
	vec3_t	org;

	// Back away from the wall
	VectorMA( trace->endpos, 1.5, trace->plane.normal, org );
	G_SetOrigin( ent, org );
	VectorCopy( trace->plane.normal, ent->pos1 );

	VectorClear( ent->s.apos.trDelta );
//	vectoangles( trace->plane.normal, ent->s.angles);
	// This will orient the object to face in the direction of the normal
	VectorScale( trace->plane.normal, -1, ent->s.pos.trDelta );
	ent->s.pos.trTime = level.time;
//	VectorCopy( ent->s.angles, ent->currentAngles );
//	VectorCopy( ent->s.angles, ent->s.apos.trBase);

	ent->nextthink = level.time + 1700 + random() * 300;
	ent->e_ThinkFunc = thinkF_grenadeSpewShrapnel;
}

/*
================
G_MissileImpact

================
*/
void G_MissileImpact( gentity_t *ent, trace_t *trace ) 
{
	qboolean		do_damage = qtrue;
	gentity_t		*other;

	other = &g_entities[trace->entityNum];

	// check for bounce
	if ( !other->takedamage && ( ent->s.eFlags & ( EF_BOUNCE | EF_BOUNCE_HALF ) ) ) 
	{
		// Check to see if there is a bounce count
		if ( ent->bounceCount ) {
			// decrement number of bounces and then see if it should be done bouncing
			if ( !(--ent->bounceCount) ) {
				// He (or she) will bounce no more (after this current bounce, that is).
				ent->s.eFlags &= !( EF_BOUNCE | EF_BOUNCE_HALF );
			}
		}

		if ( ent->owner && ent->owner->s.number == 0 ) 
		{
			//Add the event
			AddSoundEvent( ent->owner, ent->currentOrigin, 128, AEL_DISCOVERED );
			AddSightEvent( ent->owner, ent->currentOrigin, 256, AEL_DISCOVERED );
		}
		G_BounceMissile( ent, trace );
		G_AddEvent( ent, EV_GRENADE_BOUNCE, 0 );
		return;
	}

	// check for sticking
	if ( !other->takedamage && ( ent->s.eFlags & EF_MISSILE_STICK ) ) 
	{
		if ( ent->owner && ent->owner->s.number == 0 ) 
		{
			//Add the event
			AddSoundEvent( ent->owner, ent->currentOrigin, 128, AEL_DISCOVERED );
			AddSightEvent( ent->owner, ent->currentOrigin, 256, AEL_DISCOVERED );
		}
		G_MissileStick( ent, trace );
		G_AddEvent( ent, EV_MISSILE_STICK, 0 );
		return;
	}

	// impact damage
	if (other->takedamage) 
	{
		// FIXME: wrong damage direction?
		if ( ent->damage ) 
		{
			vec3_t	velocity;

			EvaluateTrajectoryDelta( &ent->s.pos, level.time, velocity );
			if ( VectorLength( velocity ) == 0 ) 
			{
				velocity[2] = 1;	// stepped on a grenade
			}

			if ( ent->owner )
			{
				if( LogAccuracyHit( other, ent->owner ) ) 
				{
					ent->owner->client->ps.persistant[PERS_ACCURACY_HITS]++;
				}
			}

			do_damage = W_CheckBorgAdapt( ent, ent->s.weapon, other, ent->currentOrigin );

			if ( do_damage )
			{
				G_Damage( other, ent, ent->owner, velocity,
						ent->s.origin, ent->damage, 
						ent->dflags, ent->methodOfDeath);
			}
		}
	}

	// is it cheaper in bandwidth to just remove this ent and create a new
	// one, rather than changing the missile into the explosion?
	//G_FreeEntity(ent);
	
	if ( other->takedamage && other->client && do_damage ) 
	{
		G_AddEvent( ent, EV_MISSILE_HIT, DirToByte( trace->plane.normal ) );
		ent->s.otherEntityNum = other->s.number;
	} 
	else 
	{
		G_AddEvent( ent, EV_MISSILE_MISS, DirToByte( trace->plane.normal ) );
		ent->s.otherEntityNum = other->s.number;
	}

	if ( ent->owner && ent->owner->s.number == 0 )
	{
		//Add the event
		AddSoundEvent( ent->owner, ent->currentOrigin, 256, AEL_DISCOVERED );
		AddSightEvent( ent->owner, ent->currentOrigin, 512, AEL_DISCOVERED );
	}

	ent->freeAfterEvent = qtrue;

	// change over to a normal entity right at the point of impact
	ent->s.eType = ET_GENERAL;

	//SnapVectorTowards( trace->endpos, ent->s.pos.trBase );	// save net bandwidth
	VectorCopy( trace->endpos, ent->s.pos.trBase );

	G_SetOrigin( ent, trace->endpos );

	// splash damage (doesn't apply to person directly hit)
	if ( ent->splashDamage ) 
	{
		G_RadiusDamage( trace->endpos, ent->owner, ent->splashDamage, ent->splashRadius, 
			other, ent->splashMethodOfDeath );
	}

	gi.linkentity( ent );
}

/*
================
G_ExplodeMissile

Explode a missile without an impact
================
*/
void G_ExplodeMissile( gentity_t *ent ) 
{
	vec3_t		dir;
	vec3_t		origin;

	EvaluateTrajectory( &ent->s.pos, level.time, origin );
	SnapVector( origin );
	G_SetOrigin( ent, origin );

	// we don't have a valid direction, so just point straight up
	dir[0] = dir[1] = 0;
	dir[2] = 1;

	if ( ent->owner && ent->owner->s.number == 0 )
	{
		//Add the event
		AddSoundEvent( ent->owner, ent->currentOrigin, 256, AEL_DISCOVERED );
		AddSightEvent( ent->owner, ent->currentOrigin, 512, AEL_DISCOVERED );
	}
/*	ent->s.eType = ET_GENERAL;
	G_AddEvent( ent, EV_MISSILE_MISS, DirToByte( dir ) );

	ent->freeAfterEvent = qtrue;*/

	// splash damage
	if ( ent->splashDamage ) 
	{
		G_RadiusDamage( ent->currentOrigin, ent->owner, ent->splashDamage, ent->splashRadius, NULL
			, ent->splashMethodOfDeath );
	}

	G_FreeEntity(ent);
	//gi.linkentity( ent );
}


/*
================
G_RunMissile

================
*/
void G_MoverTouchTeleportTriggers( gentity_t *ent, vec3_t oldOrg );
void G_RunMissile( gentity_t *ent ) 
{
	vec3_t		origin, oldOrg;
	trace_t		tr;

	VectorCopy( ent->currentOrigin, oldOrg );
	// get current position
	EvaluateTrajectory( &ent->s.pos, level.time, origin );
	// get current angles
	VectorMA( ent->s.apos.trBase, (level.time - ent->s.apos.trTime) * 0.001, ent->s.apos.trDelta, ent->s.apos.trBase );

	// trace a line from the previous position to the current position,
	// ignoring interactions with the missile owner
	gi.trace( &tr, ent->currentOrigin, ent->mins, ent->maxs, origin, 
		ent->owner ? ent->owner->s.number : ENTITYNUM_NONE, ent->clipmask );

	VectorCopy( tr.endpos, ent->currentOrigin );

	if ( tr.startsolid ) 
	{
		tr.fraction = 0;
	}

	gi.linkentity( ent );

	// check think function
	G_RunThink( ent );

	if ( ent->s.eType != ET_MISSILE ) 
	{
		return;		// exploded
	}

	if ( !(ent->s.eFlags & EF_TELEPORT_BIT) )
	{
		G_MoverTouchTeleportTriggers( ent, oldOrg );
		if ( ent->s.eFlags & EF_TELEPORT_BIT )
		{//was teleported
			return;
		}
	}
	else
	{
		ent->s.eFlags &= ~EF_TELEPORT_BIT;
	}

	if ( tr.fraction == 1 ) 
	{
		return;
	}

	// never explode or bounce on sky
	if ( tr.surfaceFlags & SURF_NOIMPACT ) 
	{
		G_FreeEntity( ent );
		return;
	}

	G_MissileImpact( ent, &tr );
}

void charge_stick (gentity_t *self, gentity_t *other, trace_t *trace)
{
	gentity_t	*tent;
	//self->s.eType = ET_GENERAL;
	//FIXME: once on ground, shouldn't explode if touched by someone?
	//FIXME: if owner touches it again, pick it up?  Or if he "uses" it?
	self->e_TouchFunc = touchF_NULL;
	self->e_ThinkFunc = thinkF_NULL;
	self->nextthink = -1;
	/*
	if(other->client)
	{//FIXME: doesn't do effect if hits someone, just hurts them
		if(self->s.pos.trType == TR_STATIONARY)
		{
			VectorCopy( self->currentOrigin, self->s.origin );
			self->e_ThinkFunc = thinkF_ExplodeDeath;
			self->nextthink = level.time + 500;
			G_Sound(self, G_SoundIndex("sound/weapons/detpackfire.wav"));
		}
		else
		{
			VectorCopy( self->currentOrigin, self->s.origin );
			self->e_ThinkFunc = thinkF_ExplodeDeath;
			self->nextthink = level.time + 100;
		}

		if(self->owner && self->owner->client)
		{
			self->owner->client->ps.eFlags &= ~EF_PLANTED_CHARGE;
		}
	}
	else
	*/
	{
		self->s.pos.trType = TR_STATIONARY;
		VectorCopy( self->currentOrigin, self->s.origin );
		VectorCopy( self->currentOrigin, self->s.pos.trBase );
		VectorClear( self->s.pos.trDelta );

		VectorClear( self->s.apos.trDelta );
		vectoangles(trace->plane.normal, self->s.angles);
		VectorCopy(self->s.angles, self->currentAngles );
		VectorCopy(self->s.angles, self->s.apos.trBase);

		G_Sound(self, G_SoundIndex("sound/weapons/detpacklatch.wav"));
		
		tent = G_TempEntity( self->currentOrigin, EV_MISSILE_MISS );
		tent->s.weapon = 0;
		tent->owner = self;
	}
}

void drop_charge (gentity_t *self, vec3_t start, vec3_t dir) 
{
	gentity_t	*bolt;

	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->classname = "charge";
	bolt->nextthink = level.time + FRAMETIME;
	bolt->e_ThinkFunc = thinkF_G_RunObject;
	bolt->s.eType = ET_GENERAL;
	bolt->s.modelindex = G_ModelIndex("models/boltOns/detpack.md3");
	//bolt->playerTeam = self->client->playerTeam;
	bolt->owner = self;
	bolt->damage = 100;
	bolt->splashDamage = 200;
	bolt->splashRadius = 200;
	bolt->methodOfDeath = MOD_GRENADE;
	bolt->splashMethodOfDeath = MOD_GRENADE_SPLASH;
	bolt->clipmask = MASK_SHOT;
	bolt->e_TouchFunc = touchF_charge_stick;
	bolt->sounds = G_SoundIndex("sound/weapons/explosions/explode5.wav");

	G_SetOrigin(bolt, start);
	bolt->s.pos.trType = TR_GRAVITY;
	VectorCopy( start, bolt->s.pos.trBase );
	VectorScale(dir, 300, bolt->s.pos.trDelta );
	bolt->s.pos.trTime = level.time;

	vectoangles(dir, bolt->s.angles);
	VectorCopy(bolt->s.angles, bolt->s.apos.trBase);
	VectorSet(bolt->s.apos.trDelta, 300, 0, 0 );
	bolt->s.apos.trTime = level.time;

	gi.linkentity(bolt);
}
