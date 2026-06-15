// This file contains game side effects that the designers can place throughout the maps

#include "g_local.h"
#include "g_functions.h"

extern void G_SetEnemy( gentity_t *self, gentity_t *enemy );
/*QUAKED fx_spark (0 0 1) (-8 -8 -8) (8 8 8) STARTOFF
Emits sparks at the specified point in the specified direction

  "targetname" - toggles on/off whenever used
  "target" - ( optional ) direction to aim the sparks in, otherwise, uses the angles set in the editor.
  "delay(2000)"	- interal between events (randomly twice as long 
*/

//------------------------------------------
void spark_think( gentity_t *ent )
{
	G_AddEvent( ent, EV_FX_SPARK, 0 );
	ent->nextthink = level.time + (ent->delay + (random() * ent->delay));
}

//------------------------------------------
void spark_use( gentity_t *self, gentity_t *other, gentity_t *activator)
{
	if (self->behaviorSet[BSET_USE])
	{
		G_ActivateBehavior(self,BSET_USE);
	}

	if ( self->count )
	{
		self->e_ThinkFunc = thinkF_NULL;
		self->soundSet = NULL;
	}
	else
	{
		self->e_ThinkFunc = thinkF_spark_think;
		self->nextthink = level.time + 200;
	}
	
	self->count = !self->count;
}

//------------------------------------------
void spark_link( gentity_t *ent )
{
	ent->count = !(ent->spawnflags & 1);	//active flag

	if (!ent->targetname || ent->count )
	{
		ent->e_ThinkFunc = thinkF_spark_think;	
		ent->nextthink = level.time + ent->delay*random();	//don't all go off at once
	}
	else
	{
		ent->e_ThinkFunc = thinkF_NULL;
		ent->nextthink = -1;
	}

	if ( ent->target )
	{
		// try to use the target to orient me.
		gentity_t	*target = NULL;
		vec3_t		dir;

		target = G_Find (target, FOFS(targetname), ent->target);

		if (!target)
		{
			Com_Printf("spark_link: target specified but not found: %s\n", ent->target );

			ent->e_ThinkFunc = thinkF_NULL;
			ent->nextthink = -1;

			return;
		}
		
		VectorSubtract( target->s.origin, ent->s.origin, dir );
		VectorNormalize( dir );
		vectoangles( dir, ent->currentAngles );
		VectorCopy( ent->currentAngles, ent->s.angles );
		VectorCopy( ent->currentAngles, ent->s.apos.trBase );
	}
}

//------------------------------------------
void SP_fx_spark( gentity_t	*ent )
{
	if (ent->targetname)
	{
		ent->e_UseFunc = useF_spark_use;
	}
	
	if (!ent->delay)
	{
		ent->delay = 2000;
	}

	// Precaching sounds
	G_SoundIndex("sound/ambience/spark1.wav");
	G_SoundIndex("sound/ambience/spark2.wav");
	G_SoundIndex("sound/ambience/spark3.wav");
	G_SoundIndex("sound/ambience/spark4.wav");
	G_SoundIndex("sound/ambience/spark5.wav");
	G_SoundIndex("sound/ambience/spark6.wav");
	

	VectorCopy( ent->s.origin, ent->s.pos.trBase );

	// The thing that this is targetting may not be spawned in yet, so wait a bit to try and link to it
	ent->e_ThinkFunc = thinkF_spark_link; 
	ent->nextthink = level.time + 500;

	gi.linkentity( ent );
}

/*QUAKED fx_cooking_steam (0 0 1) (-8 -8 -8) (8 8 8) STARTOFF
Emits slowly moving steam puffs that rise up from the specified point

  "targetname" - toggles effect on/off whenver used
  "radius" - smoke puff size ( default 3.0 )
*/

//------------------------------------------
void cooking_steam_think( gentity_t *ent )
{
	G_AddEvent( ent, EV_FX_COOKING_STEAM, 0 );
	ent->nextthink = level.time + 100;
}

//------------------------------------------
void cooking_steam_use( gentity_t *self, gentity_t *other, gentity_t *activator)
{
	if (self->behaviorSet[BSET_USE])
	{
		G_ActivateBehavior(self,BSET_USE);
	}

	if ( self->count )
	{
		self->e_ThinkFunc = thinkF_NULL;
	}
	else
	{
		self->e_ThinkFunc = thinkF_cooking_steam_think;
		self->nextthink = level.time + 200;
	}
	
	self->count = !self->count;
}

//------------------------------------------
void SP_fx_cooking_steam( gentity_t	*ent )
{
	G_SpawnFloat( "radius", "3.0", &ent->radius );

	gi.linkentity( ent );

	if (ent->targetname)
	{
		ent->e_UseFunc = useF_cooking_steam_use;
	}

	ent->count = !(ent->spawnflags & 1);

	if (!ent->targetname || !(ent->spawnflags & 1) )
	{
		ent->e_ThinkFunc = thinkF_cooking_steam_think;
		ent->nextthink = level.time + 2000;
	}
	else
	{
		ent->e_ThinkFunc = thinkF_NULL;
		ent->nextthink = -1;
	}

	VectorCopy( ent->s.origin, ent->s.pos.trBase );
}

/*QUAKED fx_elecfire (0 0 1) (-8 -8 -8) (8 8 8)
Emits sparks at the specified point in the specified direction
Spawns smoke puffs.
*/

//------------------------------------------
void electric_fire_think( gentity_t *ent )
{
	G_AddEvent( ent, EV_FX_ELECFIRE, 0 );
	ent->nextthink = level.time + (750 + (random() * 300));
}

//------------------------------------------
void SP_fx_electricfire( gentity_t	*ent )
{
	ent->e_ThinkFunc = thinkF_electric_fire_think;
	ent->nextthink = level.time + 500;

	VectorCopy( ent->s.origin, ent->s.pos.trBase );
	
	// Precaching sounds
	G_SoundIndex("sound/ambience/spark1.wav");
	G_SoundIndex("sound/ambience/spark2.wav");
	G_SoundIndex("sound/ambience/spark3.wav");
	G_SoundIndex("sound/ambience/spark4.wav");
	G_SoundIndex("sound/ambience/spark5.wav");
	G_SoundIndex("sound/ambience/spark6.wav");

	gi.linkentity( ent );
}

/*QUAKED fx_drip (0 0 1) (-8 -8 -8) (8 8 8) STARTOFF OIL
Creates timed drips from specified point.

  STARTOFF - effect starts when used.
  OIL - specifying an RGB of 0 0 0 won't make the drip black so you'll have to use this

  "wait" - seconds between drips (default 1)
	0.5 = 2 drips per second
	1.0 = 1 drip per second

  "random" - drip randomness (default 0)
	0.5 = half second of drip variance

  "startRGBA" - drip color, Red Green Blue Alpha 
	(default 180 180 255 192)

  "radius" - drip radius
	(default 0.5)

  "targetname" - toggles on/off whenever used
*/

//------------------------------------------
void drip_think( gentity_t *ent )
{
	G_AddEvent( ent, EV_FX_DRIP, 0 );
	ent->nextthink = level.time + ( ent->wait + crandom() * ( ent->random * 0.5 )) * 1000;
}

//------------------------------------------
void drip_use( gentity_t *self, gentity_t *other, gentity_t *activator )
{
	if (self->behaviorSet[BSET_USE])
	{
		G_ActivateBehavior(self,BSET_USE);
	}

	if ( self->count )
	{
		self->e_ThinkFunc = thinkF_NULL;
	}
	else
	{
		self->e_ThinkFunc = thinkF_drip_think;
		self->nextthink = level.time + 200;
	}
	
	self->count = !self->count;
}

//------------------------------------------
void SP_fx_drip( gentity_t *ent )
{
	// Try to apply defaults if nothing was set
	G_SpawnFloat( "wait", "1.0", &ent->wait );
	G_SpawnFloat( "random", "0", &ent->random );
	G_SpawnVector4( "startRGBA", "180 180 255 192", (float *)&ent->startRGBA );
	G_SpawnFloat( "radius", "0.5", &ent->radius );
		
	// Convert from range of 0-255 to 0-1
	for ( int t=0; t < 4; t++)
	{
		ent->startRGBA[t] = ent->startRGBA[t] / 255;
	}

	if ( ent->targetname )
	{
		ent->e_UseFunc = useF_drip_use;
	}

	// This is used as the toggle switch
	ent->count = !(ent->spawnflags & 1);

	// Precaching sounds
	G_SoundIndex("sound/ambience/waterdrop1.wav");
	G_SoundIndex("sound/ambience/waterdrop2.wav");
	G_SoundIndex("sound/ambience/waterdrop3.wav");

	if (!ent->targetname || !(ent->spawnflags & 1) )
	{
		ent->e_ThinkFunc = thinkF_drip_think;
		ent->nextthink = level.time + 200;
	}
	else
	{
		ent->e_ThinkFunc = thinkF_NULL;
		ent->nextthink = -1;
	}

	VectorCopy( ent->s.origin, ent->s.pos.trBase );
	
	gi.linkentity( ent );
}


/*QUAKED fx_steam (0 0 1) (-8 -8 -8) (8 8 8) STARTOFF BURSTS EVIL_YELLOW
Emits steam at the specified point in the specified direction

EVIL_YELLOW - steam is a nasty yellow color

  "targetname" - toggles on/off whenever used
  "damage" - damage to apply when caught in steam vent, default - zero damage (no damage). Don't add this unless you really have to.
  "radius" - radius of the steam puffs ( 10 - default )
*/

//------------------------------------------
void steam_think( gentity_t *ent )
{
	if ( ent->spawnflags & 2 )
	{
		ent->nextthink = level.time + 1000 + random() * 500;
	}
	else
	{
		ent->nextthink = level.time + 50;
	}

	G_AddEvent( ent, EV_FX_STEAM, ent->spawnflags );

	// FIXME: This may be a bit weird for steam bursts
	// If a fool gets in the bolt path, zap 'em
	if ( ent->damage ) 
	{
		vec3_t	start, temp;
		trace_t	trace;

		VectorSubtract( ent->s.origin2, ent->currentOrigin, temp );
		VectorNormalize( temp );
		VectorMA( ent->currentOrigin, 1, temp, start );

		gi.trace( &trace, start, NULL, NULL, ent->s.origin2, -1, MASK_SHOT );//ignore

		if ( trace.fraction < 1.0 )
		{
			if ( trace.entityNum < ENTITYNUM_WORLD )
			{
				gentity_t *victim = &g_entities[trace.entityNum];
				if ( victim && victim->takedamage )
				{
					G_Damage( victim, ent, ent->activator, temp, trace.endpos, ent->damage, 0, MOD_UNKNOWN );
				}
			}
		}
	}
}

//  If a steam ent got used before it linked, it would get messed up.  Don't allow a use until after
#define STEAM_NOT_LINKED 999

//------------------------------------------
void steam_use( gentity_t *self, gentity_t *other, gentity_t *activator )
{
	// Make sure that we have been initialized before using this, otherwise the
	//	steam angles will get really messed up.
	if ( self->count == STEAM_NOT_LINKED )
		return;

	if (self->behaviorSet[BSET_USE])
	{
		G_ActivateBehavior(self,BSET_USE);
	}

	if ( self->count )
	{
		self->e_ThinkFunc = thinkF_NULL;
		self->soundSet = NULL;
	}
	else
	{
		self->e_ThinkFunc = thinkF_steam_think;
		self->nextthink = level.time + 100;
		self->soundSet = "steamloop";
	}
	
	self->count = !self->count;
}

//------------------------------------------
void steam_link( gentity_t *ent )
{
	gentity_t	*target = NULL;
	vec3_t		dir;
	float		len;

	target = G_Find (target, FOFS(targetname), ent->target);

	if (!target)
	{
		Com_Printf("steam_link: unable to find target %s\n", ent->target );

		ent->e_ThinkFunc = thinkF_NULL;
		ent->nextthink = -1;

		return;
	}

	VectorSubtract( target->s.origin, ent->s.origin, dir );
	len = VectorNormalize( dir );
	vectoangles( dir, ent->s.angles );
	
	VectorCopy( target->s.origin, ent->s.origin2 );

	// This is used as the toggle switch
	ent->count = !(ent->spawnflags & 1);
	gi.linkentity( ent );

	if ( (ent->spawnflags & 1) )
	{
		ent->e_ThinkFunc = thinkF_NULL;
		ent->nextthink = -1;
		ent->soundSet = NULL;
		return;
	}

	ent->e_ThinkFunc = thinkF_steam_think;
	ent->nextthink = level.time + 1000;
}

//------------------------------------------
void SP_fx_steam( gentity_t	*ent )
{
	VectorCopy( ent->s.origin, ent->s.pos.trBase );
	gi.linkentity( ent );

	// Try to apply defaults if nothing was set
	G_SpawnInt( "damage", "0", &ent->damage );
	G_SpawnFloat( "radius", "10", &ent->radius );

	ent->count = STEAM_NOT_LINKED;

	if ( ent->targetname )
	{
		ent->e_UseFunc = useF_steam_use;
	}

	if ( VALIDSTRING( ent->soundSet) == false )
	{
		ent->soundSet = "steamloop";
	}

	ent->e_ThinkFunc = thinkF_steam_link;
	ent->nextthink = level.time + 500;
}

/*QUAKED fx_bolt (0 0 1) (-8 -8 -8) (8 8 8) STARTOFF DELAYED SPARKS BORG TAPER SMOOTH BW
Emits blue ( or borg green ) electric bolts from the specified point to the specified point

  STARTOFF - effect is initially off
  DELAYED - bolts are time delayed, otherwise effect continuously fires
  SPARKS - create impact sparks, probably best used for time delayed bolts
  BORG - Make the bolts green
  TAPER - Bolt will taper on one end
  SMOOTH - Bolt texture stretches across whole length, makes short bolts look much better.
  BW - Make the bolts black and white, will override BORG flag

  "wait" - seconds between bolts, only valid when DELAYED is checked (default 2)
  "damage" - damage per server frame (default 0)
  "targetname" - toggles effect on/off each time it's used
  "random" - bolt chaos (0.1 = too calm, 0.5 = default, 1.0 or higher = pretty wicked)
  "radius" - radius of the bolt (1.0 = default) 
*/

//------------------------------------------
void bolt_think( gentity_t *ent )
{
	vec3_t	start, temp;
	trace_t	trace;

	G_AddEvent( ent, EV_FX_BOLT, ent->spawnflags & 2 );
	ent->nextthink = level.time + (ent->wait + crandom() * ent->wait * 0.25) * 1000;

	// If a fool gets in the bolt path, zap 'em
	if ( ent->damage ) 
	{
		VectorSubtract( ent->s.origin2, ent->currentOrigin, temp );
		VectorNormalize( temp );
		VectorMA( ent->currentOrigin, 1, temp, start );

		gi.trace( &trace, start, NULL, NULL, ent->s.origin2, -1, MASK_SHOT );//ignore

		if ( trace.fraction < 1.0 )
		{
			if ( trace.entityNum < ENTITYNUM_WORLD )
			{
				gentity_t *victim = &g_entities[trace.entityNum];
				if ( victim && victim->takedamage )
				{
					G_Damage( victim, ent, ent->activator, temp, trace.endpos, ent->damage, 0, MOD_UNKNOWN );
				}
			}
		}
	}
}

//------------------------------------------
void bolt_use( gentity_t *self, gentity_t *other, gentity_t *activator )
{
	if (self->behaviorSet[BSET_USE])
	{
		G_ActivateBehavior(self,BSET_USE);
	}

	if ( self->count )
	{
		self->e_ThinkFunc = thinkF_NULL;
	}
	else
	{
		self->e_ThinkFunc = thinkF_bolt_think;
		self->nextthink = level.time + 200;
	}
	
	self->count = !self->count;
}

//------------------------------------------
void bolt_link( gentity_t *ent )
{
	gentity_t	*target = NULL;
	vec3_t		dir;
	float		len;

	target = G_Find (target, FOFS(targetname), ent->target);

	if (!target)
	{
		Com_Printf("bolt_link: unable to find target %s\n", ent->target );

		ent->e_ThinkFunc = thinkF_NULL;
		ent->nextthink = -1;

		return;
	}

	VectorSubtract( target->s.origin, ent->s.origin, dir );
	len = VectorNormalize( dir );
	vectoangles( dir, ent->s.angles );
	
	VectorCopy( target->s.origin, ent->s.origin2 );

	if ( ent->targetname )
	{
		ent->e_UseFunc = useF_bolt_use;
	}

	// This is used as the toggle switch
	ent->count = !(ent->spawnflags & 1);

	if (!ent->targetname || !(ent->spawnflags & 1) )
	{
		ent->e_ThinkFunc = thinkF_bolt_think;	
		ent->nextthink = level.time + 1000;
	}
	else
	{
		ent->e_ThinkFunc = thinkF_NULL;
		ent->nextthink = -1;
	}

	gi.linkentity( ent );
}

//------------------------------------------
void SP_fx_bolt( gentity_t *ent )
{
	G_SpawnInt( "damage", "0", &ent->damage );
	G_SpawnFloat( "random", "0.5", &ent->random );
	G_SpawnFloat( "radius", "1.0", &ent->radius );

	// See if effect is supposed to be delayed
	if ( ent->spawnflags & 2 )
	{
		G_SpawnFloat( "wait", "2.0", &ent->wait );
	}
	else
	{
		// Effect is continuous
		ent->wait = 0.1f;
	}

	VectorCopy( ent->s.origin, ent->s.pos.trBase );

	if (ent->target)
	{
		ent->e_ThinkFunc = thinkF_bolt_link;
		ent->nextthink = level.time + 100;
		return;
	}

	gi.linkentity( ent );
}

/*QUAKED fx_forge_bolt (0 0 1) (-8 -8 -8) (8 8 8) STARTOFF DELAYED SPARKS PULSE TAPER SMOOTH
Emits freaky orange bolts, sending pulses down the length of the beam if desired

  STARTOFF - effect is initially off
  DELAYED - bolts are time delayed, otherwise effect continuously fires
  SPARKS - create impact sparks, probably best used for time delayed bolts
  PULSE - sends a pulse down the length of the beam.
  TAPER - Bolt will taper on one end
  SMOOTH - Bolt texture stretches across whole length, makes short bolts look much better.

  "wait" - seconds between bolts, only valid when DELAYED is checked (default 2)
  "damage" - damage per server frame (default 0)
  "targetname" - toggles effect on/off each time it's used
  "random" - bolt chaos (0.1 = too calm, 0.4 = default, 1.0 or higher = pretty wicked)
  "radius" - radius of the bolt (3.0 = default) 
*/

//------------------------------------------
void forge_bolt_think( gentity_t *ent )
{
	vec3_t	start, temp;
	trace_t	trace;

	G_AddEvent( ent, EV_FX_FORGE_BOLT, ent->spawnflags & 2 );
	ent->nextthink = level.time + (ent->wait + crandom() * ent->wait * 0.25) * 1000;

	// If a fool gets in the bolt path, zap 'em
	if ( ent->damage ) 
	{
		VectorSubtract( ent->s.origin2, ent->currentOrigin, temp );
		VectorNormalize( temp );
		VectorMA( ent->currentOrigin, 1, temp, start );

		gi.trace( &trace, start, NULL, NULL, ent->s.origin2, -1, MASK_SHOT );//ignore

		if ( trace.fraction < 1.0 )
		{
			if ( trace.entityNum < ENTITYNUM_WORLD )
			{
				gentity_t *victim = &g_entities[trace.entityNum];
				if ( victim && victim->takedamage )
				{
					G_Damage( victim, ent, ent->activator, temp, trace.endpos, ent->damage, 0, MOD_UNKNOWN );
				}
			}
		}
	}
}

//------------------------------------------
void forge_bolt_use( gentity_t *self, gentity_t *other, gentity_t *activator )
{
	if (self->behaviorSet[BSET_USE])
	{
		G_ActivateBehavior(self,BSET_USE);
	}

	if ( self->count )
	{
		self->e_ThinkFunc = thinkF_NULL;
	}
	else
	{
		self->e_ThinkFunc = thinkF_forge_bolt_think;
		self->nextthink = level.time + 200;
	}
	
	self->count = !self->count;
}

//------------------------------------------
void forge_bolt_link( gentity_t *ent )
{
	gentity_t	*target = NULL;
	vec3_t		dir;
	float		len;

	target = G_Find (target, FOFS(targetname), ent->target);

	if (!target)
	{
		Com_Printf("forge_bolt_link: unable to find target %s\n", ent->target );

		ent->e_ThinkFunc = thinkF_NULL;
		ent->nextthink = -1;

		return;
	}

	VectorSubtract( target->s.origin, ent->s.origin, dir );
	len = VectorNormalize( dir );
	vectoangles( dir, ent->s.angles );
	
	VectorCopy( target->s.origin, ent->s.origin2 );

	if ( ent->targetname )
	{
		ent->e_UseFunc = useF_forge_bolt_use;
	}

	// This is used as the toggle switch
	ent->count = !(ent->spawnflags & 1);

	if (!ent->targetname || !(ent->spawnflags & 1) )
	{
		ent->e_ThinkFunc = thinkF_forge_bolt_think;	
		ent->nextthink = level.time + 1000;
	}
	else
	{
		ent->e_ThinkFunc = thinkF_NULL;
		ent->nextthink = -1;
	}

	gi.linkentity( ent );
}

//------------------------------------------
void SP_fx_forge_bolt( gentity_t *ent )
{
	G_SpawnInt( "damage", "0", &ent->damage );
	G_SpawnFloat( "random", "0.4", &ent->random );
	G_SpawnFloat( "radius", "3.0", &ent->radius );

	// See if effect is supposed to be delayed
	if ( ent->spawnflags & 2 )
	{
		G_SpawnFloat( "wait", "2.0", &ent->wait );
	}
	else
	{
		// Effect is continuous
		ent->wait = 0.1f;
	}

	VectorCopy( ent->s.origin, ent->s.pos.trBase );
	ent->delay = level.time + 1000;

	if (ent->target)
	{
		ent->e_ThinkFunc = thinkF_forge_bolt_link;
		ent->nextthink = level.time + 100;
		return;
	}

	gi.linkentity( ent );
}

/*QUAKED fx_plasma (0 0 1) (-8 -8 -8) (8 8 8) START_OFF
Emits plasma jet directed from the specified point to the specified point. Jet size scales based on length.  

  "target" (required)
  "targetname" - fires only when used
  "startRGBA" - starting cone color, Red Green Blue Alpha 
	(default  100 180 255 255) Light-Blue
  "finalRGBA" - final cone color, Red Green Blue Alpha 
	(default  0 0 180 0) Blue
  "damage" - damage PER FRAME, default zero

*/

//------------------------------------------
void plasma_think( gentity_t *ent )
{
	vec3_t	start, temp;
	trace_t	trace;

	G_AddEvent( ent, EV_FX_PLASMA, 0 );
	ent->nextthink = level.time + 100;

	// If a fool gets in the plasma cone, fry 'em
	if ( ent->damage )
	{
		VectorSubtract( ent->s.origin2, ent->currentOrigin, temp );
		VectorNormalize( temp );
		VectorMA( ent->currentOrigin, 1, temp, start );

		gi.trace( &trace, start, NULL, NULL, ent->s.origin2, -1, MASK_SHOT);//ignore

		if(trace.fraction < 1.0)
		{
			if(trace.entityNum < ENTITYNUM_WORLD)
			{
				gentity_t *victim = &g_entities[trace.entityNum];
				if(victim && victim->takedamage)
				{
					G_Damage( victim, ent, ent->activator, temp, trace.endpos, ent->damage, 0, MOD_UNKNOWN);
				}
			}
		}
	}
}

//------------------------------------------
void plasma_use( gentity_t *self, gentity_t *other, gentity_t *activator)
{

	if (self->behaviorSet[BSET_USE])
	{
		G_ActivateBehavior(self,BSET_USE);
	}

	if (self->e_ThinkFunc != thinkF_NULL)
	{
		self->e_ThinkFunc = thinkF_NULL;
	}
	else
	{
		self->e_ThinkFunc = thinkF_plasma_think;	
		self->nextthink = level.time + 200;
	}
}

//------------------------------------------
void plasma_link( gentity_t *ent )
{
	gentity_t	*target = NULL;

	target = G_Find (target, FOFS(targetname), ent->target);
	if (!target)
	{
		Com_Printf("plasma_link: unable to find target %s\n", ent->target );

		ent->e_ThinkFunc = thinkF_NULL;
		ent->nextthink = -1;

		return;
	}

	if (ent->targetname)
	{
		ent->e_UseFunc = useF_plasma_use;
	}

	if (!ent->targetname || !(ent->spawnflags & 1) )
	{
		ent->e_ThinkFunc = thinkF_plasma_think;	
		ent->nextthink = level.time + 200;
	}
	else
	{
		ent->e_ThinkFunc = thinkF_NULL;
	}

	gi.linkentity( ent );

	VectorCopy( target->s.origin, ent->s.origin2 );
}

//------------------------------------------
void SP_fx_plasma( gentity_t *ent )
{
	G_SpawnVector4( "startRGBA", "100 180 255 255", (float *)&ent->startRGBA );
	G_SpawnVector4( "finalRGBA", "0 0 180 0", (float *)&ent->finalRGBA );
	G_SpawnInt( "damage", "0", &ent->damage );

	// Convert from range of 0-255 to 0-1
	for (int t=0; t < 4; t++)
	{
		ent->startRGBA[t] = ent->startRGBA[t] / 255;
		ent->finalRGBA[t] = ent->finalRGBA[t] / 255;
	}
	
	VectorCopy( ent->s.origin, ent->s.pos.trBase );
	
	gi.linkentity( ent );

	ent->e_ThinkFunc = thinkF_plasma_link;
	ent->nextthink = level.time + 500;
}

/*QUAKED fx_surface_explosion (0 0 1) (-8 -8 -8) (8 8 8) NO_SMOKE LOUDER NODAMAGE
Creates a triggerable explosion aimed at a specific point.  Always oriented towards viewer.

  NO_SMOKE - Explosion doesn't trigger smoke
  LOUDER - Cheap hack to make the explosion sound louder.
  NODAMAGE - Does no damage

  "target" (optional) If no target is specified, the explosion is oriented up
  "damage" - Damage per blast, default is 50. Damage falls off based on proximity.
  "radius" - blast radius (default 20)
  "speed" - camera shake speed (default 12).  Set to zero to turn camera shakes off
  "targetname" - triggers explosion when used
*/

//------------------------------------------
void surface_explosion_use( gentity_t *self, gentity_t *other, gentity_t *activator)
{
	if (self->behaviorSet[BSET_USE])
	{
		G_ActivateBehavior(self,BSET_USE);
	}

	G_AddEvent( self, EV_FX_SURFACE_EXPLOSION, 0 );
	if ( self->splashDamage )
	{
		G_RadiusDamage( self->currentOrigin, self, self->splashDamage, self->splashRadius, NULL, MOD_UNKNOWN );
	}
}

//------------------------------------------
void surface_explosion_link( gentity_t *ent )
{
	gentity_t	*target = NULL;
	vec3_t		normal;

	target = G_Find (target, FOFS(targetname), ent->target);

	if ( target )
	{
		VectorSubtract( target->s.origin, ent->s.origin, normal );
		VectorNormalize( normal );
	}
	else
	{
		VectorSet(normal, 0, 0, 1);
	}

	VectorCopy( normal, ent->s.origin2 );

	ent->e_ThinkFunc = thinkF_NULL;
	ent->nextthink = -1;

	gi.linkentity( ent );
}

//------------------------------------------
void SP_fx_surface_explosion( gentity_t *ent )
{
	if ( !(ent->spawnflags&4) )
	{
		G_SpawnInt( "damage", "50", &ent->splashDamage );
		G_SpawnFloat( "radius", "20", &ent->radius );
		ent->splashRadius = 160;
	}

	G_SpawnFloat( "speed", "12", &ent->speed );

	// Precaching sounds
/*	if ( ent->spawnflags & 2 )
	{
		G_SoundIndex( "sound/weapons/explosions/explode2.wav" );
	}
	else
	{
		G_SoundIndex( "sound/weapons/explosions/cargoexplode.wav" );
	}
*/
	VectorCopy( ent->s.origin, ent->s.pos.trBase );

	ent->e_UseFunc = useF_surface_explosion_use;

	ent->e_ThinkFunc = thinkF_surface_explosion_link;
	ent->nextthink = 1000;

	gi.linkentity( ent );
}

/*QUAKED fx_smoke (0 0 1) (-8 -8 -8) (8 8 8) STARTOFF
Emits cloud of thick black smoke from specified point.

  "target" (option) If no target is specified, the smoke drifts up
  "targetname" - fires only when used
  "radius" - size of the smoke puffs (default 16.0)

*/

//------------------------------------------
void smoke_think( gentity_t *ent )
{
	ent->nextthink = level.time + 100;
	G_AddEvent( ent, EV_FX_SMOKE, 0 );
}

//------------------------------------------
void smoke_use( gentity_t *self, gentity_t *other, gentity_t *activator)
{
	if (self->behaviorSet[BSET_USE])
	{
		G_ActivateBehavior(self,BSET_USE);
	}

	if (self->e_ThinkFunc != thinkF_NULL)
	{
		self->e_ThinkFunc = thinkF_NULL;
	}
	else
	{
		self->e_ThinkFunc = thinkF_smoke_think;
		self->nextthink = level.time + 100;
	}
}

//------------------------------------------
void smoke_link( gentity_t *ent )
{
	// this link func is used because the target ent may not have spawned in yet, this
	//	will give it a bit of extra time for that to happen.
	gentity_t	*target = NULL;
	vec3_t		dir;

	target = G_Find (target, FOFS(targetname), ent->target);
	if (target)
	{
		VectorCopy( target->s.origin, dir );		
	}
	else
	{
		VectorCopy( ent->s.origin, dir );
		dir[2] += 1;	// move up
	}

	if (ent->targetname)
	{
		ent->e_UseFunc = useF_smoke_use;
	}

	if (!ent->targetname || !(ent->spawnflags & 1) )
	{
		ent->e_ThinkFunc = thinkF_smoke_think;
		ent->nextthink = level.time + 200;
	}
	else
	{
		ent->e_ThinkFunc = thinkF_NULL;
		ent->nextthink = -1;
	}

	VectorCopy( dir, ent->s.origin2 );

	gi.linkentity( ent );
}

//------------------------------------------
void SP_fx_smoke( gentity_t *ent )
{
	G_SpawnFloat( "radius", "16.0", &ent->radius );

	VectorCopy( ent->s.origin, ent->s.pos.trBase );

	ent->e_ThinkFunc = thinkF_smoke_link;
	ent->nextthink = level.time + 1000;
	
	gi.linkentity( ent );
}

/*QUAKED fx_teleporter (0 0 1) (-8 -8 -8) (8 8 8) STARTOFF EXIT
Creates persistent teleporter pad effect for Stasis level.

  STARTOFF - Effect starts when used
  EXIT - Teleporter effect for destination teleport pad

  "targetname" - toggles on/off when used
*/

//------------------------------------------
void teleporter_think( gentity_t *ent )
{
	G_AddEvent( ent, EV_FX_TELEPORTER, 0 );
	ent->nextthink = level.time + 75;
}

//------------------------------------------
void teleporter_use( gentity_t *self, gentity_t *other, gentity_t *activator )
{
	if (self->behaviorSet[BSET_USE])
	{
		G_ActivateBehavior(self,BSET_USE);
	}

	if ( self->count )
	{
		self->e_ThinkFunc = thinkF_NULL;
	}
	else
	{
		self->e_ThinkFunc = thinkF_teleporter_think;
		self->nextthink = level.time + 200;
	}
	
	self->count = !self->count;
}

//------------------------------------------
void SP_fx_teleporter( gentity_t *ent )
{
	if (ent->targetname)
	{
		ent->e_UseFunc = useF_teleporter_use;
	}

	ent->count = !(ent->spawnflags & 1);

	if (!ent->targetname || !(ent->spawnflags & 1) )
	{
		ent->e_ThinkFunc = thinkF_teleporter_think;	
		ent->nextthink = level.time + 200;
	}
	else if ( ent->spawnflags & 1 )
	{
		ent->e_ThinkFunc = thinkF_NULL;
		ent->nextthink = -1;
	}

	VectorCopy( ent->s.origin, ent->s.pos.trBase );

	gi.linkentity( ent );
}

/*QUAKED fx_energy_stream (0 0 1) (-8 -8 -8) (8 8 8) STARTOFF
Creates streaming particles that travel between two points--for Stasis level. ONLY orients vertically.
	
 "damage" - amount of damage to player when standing in the stream (default 0)
 "target" (required) End point for particle stream.
 "targetname" - toggle effect on/off each time used.
*/

//------------------------------------------
void stream_think( gentity_t *ent )
{
	vec3_t	start, temp;
	trace_t	trace;

	G_AddEvent( ent, EV_FX_STREAM, 0 );
	ent->nextthink = level.time + 150;

	// If a fool gets in the bolt path, zap 'em
	if ( ent->damage ) 
	{
		VectorSubtract( ent->s.origin2, ent->currentOrigin, temp );
		VectorNormalize( temp );
		VectorMA( ent->currentOrigin, 1, temp, start );

		gi.trace( &trace, start, NULL, NULL, ent->s.origin2, -1, MASK_SHOT );//ignore

		if ( trace.fraction < 1.0 )
		{
			if ( trace.entityNum < ENTITYNUM_WORLD )
			{
				gentity_t *victim = &g_entities[trace.entityNum];
				if ( victim && victim->takedamage )
				{
					G_Damage( victim, ent, ent->activator, temp, trace.endpos, ent->damage, 0, MOD_UNKNOWN );
				}
			}
		}
	}
}

//------------------------------------------
void stream_use( gentity_t *self, gentity_t *other, gentity_t *activator)
{
	if (self->behaviorSet[BSET_USE])
	{
		G_ActivateBehavior(self,BSET_USE);
	}

	if ( self->count )
	{
		self->e_ThinkFunc = thinkF_stream_think;
		self->nextthink = level.time + 200;
	}
	else
	{
		self->e_ThinkFunc = thinkF_NULL;
	}
	
	self->count = !self->count;
}

//------------------------------------------
void stream_link( gentity_t *ent )
{
	gentity_t	*target = NULL;

	target = G_Find (target, FOFS(targetname), ent->target);
	if (!target)
	{
		Com_Printf("stream_link: unable to find target %s\n", ent->target );

		ent->e_ThinkFunc = thinkF_NULL;
		ent->nextthink = -1;
		return;
	}

	VectorCopy( target->s.origin, ent->s.origin2 );
	
	if (!ent->targetname || !(ent->spawnflags & 1) )
	{
		ent->e_ThinkFunc = thinkF_stream_think;
		ent->nextthink = level.time + 200;
	}
	else if ( ent->spawnflags & 1 )
	{
		ent->e_ThinkFunc = thinkF_NULL;
	}

	gi.linkentity( ent );
}

//------------------------------------------
void SP_fx_stream( gentity_t *ent )
{
	G_SpawnInt( "damage", "0", &ent->damage );

	if (ent->targetname)
	{
		ent->e_UseFunc = useF_stream_use;
	}

	ent->count = !(ent->spawnflags & 1);

	VectorCopy( ent->s.origin, ent->s.pos.trBase );

	ent->e_ThinkFunc = thinkF_stream_link;
	ent->nextthink = level.time + 1000;

	gi.linkentity( ent );
}

/*QUAKED fx_electrical_explosion (0 0 1) (-8 -8 -8) (8 8 8) x x NODAMAGE
Creates a triggerable explosion aimed at a specific point
NODAMAGE - does no damage

  "target" (optional) If no target is specified, the explosion is oriented up
  "damage" - Damage per blast, default is 20. Damage falls off based on proximity.
  "radius" - blast radius (default 50)
  "targetname" - explodes each time it's used

*/

//------------------------------------------
void electrical_explosion_use( gentity_t *self, gentity_t *other, gentity_t *activator)
{
	if (self->behaviorSet[BSET_USE])
	{
		G_ActivateBehavior(self,BSET_USE);
	}

	self->nextthink = level.time + 100;
	gi.linkentity( self );
	G_AddEvent( self, EV_FX_ELECTRICAL_EXPLOSION, 0 );

	if ( self->splashDamage )
	{
		G_RadiusDamage( self->currentOrigin, self, self->splashDamage, self->splashRadius, NULL, MOD_UNKNOWN );
	}
}

//------------------------------------------
void electrical_explosion_link( gentity_t *ent )
{
	gentity_t	*target = NULL;
	vec3_t		normal;

	target = G_Find( target, FOFS(targetname), ent->target );

	if ( target )
	{
		VectorSubtract( target->s.pos.trBase, ent->s.origin, normal );
		VectorNormalize( normal );
	}
	else
	{
		// No target so just shoot up
		VectorSet( normal, 0, 0, 1 );
	}

	VectorCopy( normal, ent->s.origin2 );

	ent->e_ThinkFunc = thinkF_NULL;
	ent->nextthink = -1;

	gi.linkentity( ent );
}

//------------------------------------------
void SP_fx_electrical_explosion( gentity_t *ent )
{
	if ( !(ent->spawnflags&4) )
	{
		G_SpawnInt( "damage", "20", &ent->splashDamage );
		G_SpawnFloat( "radius", "50", &ent->radius );
		ent->splashRadius = 80;
	}

	// Precaching sounds
//	G_SoundIndex( "sound/weapons/explosions/cargoexplode.wav" );

	VectorCopy( ent->s.origin, ent->s.pos.trBase );

	ent->e_ThinkFunc = thinkF_electrical_explosion_link;
	ent->nextthink = level.time + 1000;

	ent->e_UseFunc = useF_electrical_explosion_use;
	gi.linkentity( ent );
}


//MCG
/*QUAKED fx_psycho_jism (1 0.5 0.5) (-8 -8 -8) (8 8 8) OPEN STARTOFF
Emits orange Forge psycho-jism bolts from the specified point to the specified point
OPEN - Ends do not taper, no sparks, no marks

"targetname" fires only when used
"duration" how many seconds each burst lasts, -1 will make it stay on forever
"wait" - If always on, how long to wait between blasts, in MILLISECONDS - default/min is 100 (1 frame at 10 fps), -1 means it will never fire again
"random" - random amount of seconds added to/subtracted from "wait" each firing
"damage" - How much damage to inflict PER FRAME (so probably want it kind of low), default is none

"target" - ent to point at- you MUST have this or the jism won't flow.  This can be anything you want, including a moving ent - for static beams, just use info_null
*/
void pj_bolt_think( gentity_t *ent );

//------------------------------------------
void pj_bolt_set_debounce( gentity_t *self )
{
	if ( self->wait >= FRAMETIME )
	{
		self->attackDebounceTime = level.time + self->wait + Q_irand( -self->random, self->random );
	}
	else if ( self->wait < 0 )
	{
		self->e_UseFunc = useF_NULL;
	}
	else 
	{
		self->attackDebounceTime = level.time + FRAMETIME + Q_irand( -self->random, self->random );
	}
}

extern void CG_PsychoJism( vec3_t start, vec3_t end, qboolean open );
//------------------------------------------
void pj_bolt_fire( gentity_t *ent )
{
	trace_t		trace;
	vec3_t		dir, org, end;
	int			ignore;
	qboolean	open;

	if ( !ent->enemy || !ent->enemy->inuse )
	{//info_null most likely
		ignore = ent->s.number;
		ent->enemy = NULL;
		VectorCopy( ent->s.origin2, org );
	}
	else
	{
		ignore = ent->enemy->s.number;
		VectorCopy( ent->enemy->currentOrigin, org );
	}

	VectorCopy( org, ent->s.origin2 );
	VectorSubtract( org, ent->s.origin, dir );
	VectorNormalize( dir );

	gi.trace( &trace, ent->s.origin, NULL, NULL, org, -1, MASK_SHOT );//ignore
	if ( ent->spawnflags & 1 )
	{
		open = qtrue;
		VectorCopy( org, end );
	}
	else
	{
		open = qfalse;
		VectorCopy( trace.endpos, end );
	}

	if ( trace.fraction < 1.0 )
	{
		if ( trace.entityNum < ENTITYNUM_WORLD )
		{
			gentity_t *victim = &g_entities[trace.entityNum];
			if ( victim && victim->takedamage )
			{
				G_Damage( victim, ent, ent->activator, dir, trace.endpos, ent->damage, 0, MOD_UNKNOWN );
			}
		}
	}

	CG_PsychoJism( ent->s.origin, end, open );

	ent->e_ThinkFunc = thinkF_pj_bolt_think;
	ent->nextthink = level.time + FRAMETIME;
}

//------------------------------------------
void pj_bolt_fire_start( gentity_t *self )
{
	pj_bolt_set_debounce( self );
	self->e_ThinkFunc		= thinkF_pj_bolt_think;
	self->nextthink			= level.time + FRAMETIME;
	self->painDebounceTime	= level.time + self->speed + Q_irand( -500, 500 );
	pj_bolt_fire( self );
}

//------------------------------------------
void pj_bolt_use( gentity_t *self, gentity_t *other, gentity_t *activator )
{
	if ( self->e_ThinkFunc == thinkF_NULL )
	{
		self->e_ThinkFunc	= thinkF_pj_bolt_think;
		self->nextthink		= level.time + 75;
	}
	else
	{
		self->e_ThinkFunc	= thinkF_NULL;
	}

	self->activator = activator;
}

//------------------------------------------
void pj_bolt_think( gentity_t *ent )
{
	if ( ent->attackDebounceTime > level.time )
	{
		ent->nextthink = level.time + FRAMETIME;
		return;
	}

	pj_bolt_fire_start( ent );
}

//------------------------------------------
void pj_bolt_link( gentity_t *ent )
{
	gentity_t	*target = NULL;
	vec3_t		dir;
	float		len;

	target = G_Find( target, FOFS(targetname), ent->target );

	if ( !target )
	{
		Com_Printf( "bolt_link: unable to find target %s\n", ent->target );
		return;
	}

	ent->attackDebounceTime = level.time;

	G_SetEnemy( ent, target );
	VectorSubtract( target->s.origin, ent->s.origin, dir );
	len = VectorNormalize( dir );
	vectoangles( dir, ent->s.angles );
	
	VectorCopy( target->s.origin, ent->s.origin2 );

	if ( (ent->spawnflags & 2) )
	{
		// Do nothing
	}
	else
	{//switch think functions to avoid doing the bolt_link every time
		ent->e_ThinkFunc = thinkF_pj_bolt_think;
		ent->nextthink	= level.time + FRAMETIME;
	}

	ent->e_UseFunc = useF_pj_bolt_use;
	gi.linkentity( ent );
}

//------------------------------------------
void SP_fx_psycho_jism( gentity_t *ent )
{
	VectorCopy( ent->s.origin, ent->s.pos.trBase );

	ent->speed	*= 1000;
	ent->wait	*= 1000;
	ent->random *= 1000;

	G_SpawnInt( "damage", "0", &ent->damage );

	if ( ent->speed < FRAMETIME )
	{
		ent->speed = FRAMETIME;
	}

	ent->activator = ent;
	ent->owner	= NULL;

	ent->e_ThinkFunc = thinkF_pj_bolt_link;
	ent->nextthink	= level.time + FRAMETIME;

	gi.linkentity( ent );
}

/*QUAKED fx_laser (0 0 1) (-8 -8 -8) (8 8 8) STARTOFF
Shoots laser beam from the specified point to the specified point.  Emits sparks and smoke, leaving a glowing trail.

  "target" (required) Can be attached to a func_train, etc.
  "targetname" - fires only when used
  "startRGBA" - laser color, Red Green Blue Alpha (default  255 222 32 255) Yellow-Orange
  "damage" - damage to inflict PER FRAME, default 0
*/

//------------------------------------------
void laser_think( gentity_t *ent )
{
	gentity_t *hapless_victim = NULL;
	vec3_t	start, end, temp;
	float	len;
	trace_t	trace;

	ent->nextthink = level.time + 100;

	// If a fool gets in the laser path, fry 'em
	VectorSubtract( ent->enemy->currentOrigin, ent->currentOrigin, temp );

	len = VectorNormalize( temp );
	VectorMA( ent->currentOrigin, 1, temp, start );	// Move out a bit so the trace doesn't start in a wall
	VectorMA( ent->currentOrigin, len * 3, temp, end );	// Shoot out to thrice the length of the beam just in case
	
	gi.trace( &trace, start, NULL, NULL, end, -1, MASK_SHOT );//ignore
	
	if ( trace.fraction < 1.0 )
	{
		if ( trace.entityNum < ENTITYNUM_WORLD )
		{
			hapless_victim = &g_entities[trace.entityNum];
			if ( hapless_victim && hapless_victim->takedamage && ent->damage )
			{
				G_Damage( hapless_victim, ent, ent->activator, temp, trace.endpos, ent->damage, 0, MOD_UNKNOWN );
			}
		}
	
		VectorCopy( trace.endpos, ent->pos1 );
		VectorCopy( trace.plane.normal, ent->pos2 );

		// Only add a beam if the trace hit something
		ent->activator = hapless_victim;
		G_AddEvent( ent, EV_FX_LASER, 0 );
	}
}

//------------------------------------------
void laser_use( gentity_t *self, gentity_t *other, gentity_t *activator)
{
	if (self->behaviorSet[BSET_USE])
	{
		G_ActivateBehavior(self,BSET_USE);
	}

	if (self->e_ThinkFunc != thinkF_NULL)
	{
		self->e_ThinkFunc = thinkF_NULL;
	}
	else
	{
		self->e_ThinkFunc = thinkF_laser_think;
		self->nextthink = level.time + 100;
	}
}

//------------------------------------------
void laser_link( gentity_t *ent )
{
	gentity_t	*target = NULL;

	target = G_Find (target, FOFS(targetname), ent->target);

	if (!target)
	{
		Com_Printf("laser_link: unable to find target %s\n", ent->target );

		ent->e_ThinkFunc = thinkF_NULL;
		ent->nextthink = -1;
		return;
	}

	G_SetEnemy(ent, target);

	if (ent->targetname)
	{
		ent->e_UseFunc = useF_laser_use;
	}

	if (!ent->targetname || !(ent->spawnflags & 1) )
	{
		ent->e_ThinkFunc = thinkF_laser_think;
		ent->nextthink = level.time + 1000;
	}
	else
	{
		ent->e_ThinkFunc = thinkF_NULL;
		ent->nextthink = -1;
	}
	
	VectorCopy( target->s.origin, ent->s.origin2 );

	gi.linkentity( ent );
}

//------------------------------------------
void SP_fx_laser( gentity_t *ent )
{
	G_SpawnVector4( "startRGBA", "255 222 32 255", (float *)&ent->startRGBA );
	G_SpawnInt( "damage", "0", &ent->damage );

	// Convert from range of 0-255 to 0-1
	for (int t=0; t < 4; t++)
	{
		ent->startRGBA[t] = ent->startRGBA[t] / 255;
	}

	VectorCopy( ent->s.origin, ent->s.pos.trBase );

	ent->e_ThinkFunc = thinkF_laser_link;
	ent->nextthink = 1000;
	
	gi.linkentity( ent );
}

/*QUAKED fx_transporter_stream (0 0 1) (-8 -8 -8) (8 8 8) STARTOFF
Creates streaming particles that travel between two points--for forge level.
	
"target" (required) End point for particle stream.
"targetname" - fires only when used

*/

//------------------------------------------
void transporter_stream_think( gentity_t *ent )
{
	G_AddEvent( ent, EV_FX_TRANSPORTER_STREAM, 0 );
	ent->nextthink = level.time + 150;
}

//------------------------------------------
void transporter_use( gentity_t *self, gentity_t *other, gentity_t *activator)
{
	if (self->behaviorSet[BSET_USE])
	{
		G_ActivateBehavior(self,BSET_USE);
	}

	if (self->e_ThinkFunc != thinkF_NULL)
	{
		self->e_ThinkFunc = thinkF_NULL;
	}
	else
	{
		self->e_ThinkFunc = thinkF_transporter_stream_think;
		self->nextthink = level.time + 150;
	}
}

//------------------------------------------
void transporter_stream_link( gentity_t *ent )
{
	gentity_t	*target = NULL;

	target = G_Find (target, FOFS(targetname), ent->target);

	if (!target)
	{
		Com_Printf( "transporter_stream_link: unable to find target %s\n", ent->target );

		ent->e_ThinkFunc = thinkF_NULL;
		ent->nextthink = -1;

		return;
	}

	if (ent->targetname)
	{
		ent->e_UseFunc = useF_transporter_use;
	}

	if (!ent->targetname || !(ent->spawnflags & 1) )
	{
		ent->e_ThinkFunc = thinkF_transporter_stream_think;
		ent->nextthink = level.time + 200;
	}
	else
	{
		ent->e_ThinkFunc = thinkF_NULL;
		ent->nextthink = -1;
	}

	VectorCopy( target->s.origin, ent->s.origin2 );

	gi.linkentity( ent );

}

//------------------------------------------
void SP_fx_transporter_stream( gentity_t *ent )
{
	VectorCopy( ent->s.origin, ent->s.pos.trBase );

	ent->e_ThinkFunc = thinkF_transporter_stream_link;
	ent->nextthink = 1000;

	gi.linkentity( ent );
}

/*QUAKED fx_explosion_trail (0 0 1) (-8 -8 -8) (8 8 8)
Creates a triggerable explosion aimed at a specific point.  Always oriented towards viewer.

  "target" (required) - end point for the explosion
  "damage" - Damage per blast, default is 150. Damage falls off based on proximity.
  "radius" - blast radius/explosion size (default 80)
  "targetname" - triggers explosion when used
*/

//------------------------------------------
void explosion_trail_use( gentity_t *self, gentity_t *other, gentity_t *activator)
{
	if (self->behaviorSet[BSET_USE])
	{
		G_ActivateBehavior(self,BSET_USE);
	}

	G_AddEvent( self, EV_FX_EXPLOSION_TRAIL, 0 );
}

//------------------------------------------
void explosion_trail_link( gentity_t *ent )
{
	gentity_t	*target = NULL;

	ent->e_ThinkFunc = thinkF_NULL;
	ent->nextthink = -1;

	target = G_Find (target, FOFS(targetname), ent->target);

	if ( !target )
	{
		Com_Printf("explosion_trail_link: unable to find target %s\n", ent->target );
		return;
	}

	VectorCopy( target->s.origin, ent->s.origin2 );

	gi.linkentity( ent );
}

//------------------------------------------
void SP_fx_explosion_trail( gentity_t *ent )
{
	G_SpawnInt( "damage", "150", &ent->splashDamage );
	G_SpawnFloat( "radius", "80", &ent->radius );

	VectorCopy( ent->s.origin, ent->s.pos.trBase );

	ent->e_UseFunc = useF_explosion_trail_use;

	ent->e_ThinkFunc = thinkF_explosion_trail_link;
	ent->nextthink = 1000;

	ent->splashRadius = 160;

	ent->svFlags |= SVF_BROADCAST;

	gi.linkentity( ent );
}

/*QUAKED fx_blow_chunks (0 0 1) (-8 -8 -8) (8 8 8)
Creates a triggerable chunk spewer that can be aimed at a specific point.

  "target" - (required) Target to spew chunks at
  "targetname" - triggers chunks when used 
  "count" - Number of chunks to spew (default 5)
  "speed" - How fast a chunk will move when it get's spewed (default 175)
  "radius" - Average size of a chunk (default 10)

"material" - default is "metal" - choose from this list:
0 = MAT_METAL		(default)
1 = MAT_GLASS
2 = MAT_ELECTRICAL	(sparks)
3 = MAT_ORGANIC		(not implemented)
4 =	MAT_BORG		(borg chunks)
5 =	MAT_STASIS		(stasis chunks)
6 =	MAT_GLASS_METAL (mixed chunk type)
(there will be more eventually)

*/

//------------------------------------------
void blow_chunks_use( gentity_t *self, gentity_t *other, gentity_t *activator)
{
	if (self->behaviorSet[BSET_USE])
	{
		G_ActivateBehavior(self,BSET_USE);
	}

	self->svFlags |= SVF_BROADCAST;
	G_AddEvent( self, EV_FX_BLOW_CHUNKS, 0 );
}

//------------------------------------------
void blow_chunks_link( gentity_t *ent )
{
	gentity_t	*target = NULL;

	ent->e_ThinkFunc = thinkF_NULL;
	ent->nextthink = -1;

	target = G_Find (target, FOFS(targetname), ent->target);

	if ( !target )
	{
		Com_Printf("blow_chunks_link: unable to find target %s\n", ent->target );
		return;
	}

	VectorCopy( target->s.origin, ent->s.origin2 );

	gi.linkentity( ent );
}

//------------------------------------------
void SP_fx_blow_chunks( gentity_t *ent )
{
	G_SpawnInt( "count", "5", &ent->count );
	G_SpawnFloat( "radius", "10", &ent->radius );
	G_SpawnFloat( "speed", "175", &ent->speed );

	VectorCopy( ent->s.origin, ent->s.pos.trBase );

	ent->e_UseFunc = useF_blow_chunks_use;

	ent->e_ThinkFunc = thinkF_blow_chunks_link;
	ent->nextthink = 1000;

	gi.linkentity( ent );
}

/*QUAKED fx_borg_energy_beam (0 0 1) (-8 -8 -8) (8 8 8) STARTOFF CONE
A borg tracing beam that either carves out a cone or swings like a pendulum, sweeping across an area. 
	
STARTOFF - The trace beam will start when used.
CONE - Beam traces a cone, default trace shape is a pendulum, sweeping across an area.

"radius" - Radius of the area to trace (default 30)
"speed" - How fast the tracer beam moves (default 100)
"startRGBA" - Effect color specified in RED GREEN BLUE ALPHA (default 0 255 0 128)
"target" (required) End point for trace beam, should be placed at the very center of the trace area.
"targetname" - fires only when used

*/

//------------------------------------------
void borg_energy_beam_think( gentity_t *ent )
{
	G_AddEvent( ent, EV_FX_BORG_ENERGY_BEAM, 0 );
	ent->nextthink = level.time + 100;
}

//------------------------------------------
void borg_energy_beam_use( gentity_t *self, gentity_t *other, gentity_t *activator)
{
	if (self->behaviorSet[BSET_USE])
	{
		G_ActivateBehavior(self,BSET_USE);
	}

	if (self->e_ThinkFunc != thinkF_NULL)
	{
		self->e_ThinkFunc = thinkF_NULL;
	}
	else
	{
		self->e_ThinkFunc = thinkF_borg_energy_beam_think;
		self->nextthink = level.time + 150;
	}
}

//------------------------------------------
void borg_energy_beam_link( gentity_t *ent )
{
	gentity_t	*target = NULL;

	target = G_Find (target, FOFS(targetname), ent->target);

	if (!target)
	{
		Com_Printf( "borg_energy_beam_link: unable to find target %s\n", ent->target );

		ent->e_ThinkFunc = thinkF_NULL;
		ent->nextthink = -1;

		return;
	}

	if (ent->targetname)
	{
		ent->e_UseFunc = useF_borg_energy_beam_use;
	}

	if (!ent->targetname || !(ent->spawnflags & 1) )
	{
		ent->e_ThinkFunc = thinkF_borg_energy_beam_think;
		ent->nextthink = level.time + 200;
	}
	else
	{
		ent->e_ThinkFunc = thinkF_NULL;
		ent->nextthink = -1;
	}

	VectorCopy( target->s.origin, ent->s.origin2 );
	VectorCopy( target->s.origin, ent->pos1 );


	gi.linkentity( ent );

}

//------------------------------------------
void SP_fx_borg_energy_beam( gentity_t *ent )
{
	G_SpawnFloat( "radius", "30", &ent->radius );
	G_SpawnFloat( "speed", "100", &ent->speed );
	G_SpawnVector4( "startRGBA", "0 255 0 128", (float *)&ent->startRGBA );

	// Convert from range of 0-255 to 0-1
	for (int t=0; t < 4; t++)
	{
		ent->startRGBA[t] = ent->startRGBA[t] / 255;
	}

//	ent->svFlags |= SVF_BROADCAST;

	VectorCopy( ent->s.origin, ent->s.pos.trBase );

	ent->e_ThinkFunc = thinkF_borg_energy_beam_link;
	ent->nextthink = 1000;

	gi.linkentity( ent );
}

/*QUAKED fx_shimmery_thing (0 0 1) (-8 -8 -8) (8 8 8) STARTOFF TAPER NO_AUTO_SHUTOFF
Creates a shimmering cone or cylinder of colored light that stretches between two points.  Looks like a teleporter type thing.  Will also autoshut off

  STARTOFF - Effect turns on when used.
  TAPER - Cylinder tapers toward the top, creating a conical effect
  NO_AUTO_SHUTOFF - Tells the effect that it should never try to shut itself off.

  "radius" - radius of the cylinder or of the base of the cone. (default 10)
  "target" (required) End point for stream.
  "targetname" - fires only when used
  "delay" - how long to stay on before turning itself off ( default 2000 - 2 seconds )

*/

//------------------------------------------
void shimmery_thing_think( gentity_t *ent )
{
	G_AddEvent( ent, EV_FX_SHIMMERY_THING, 0 );
	ent->nextthink = level.time + FRAMETIME;
	ent->fx_time -= FRAMETIME;

	if ( ent->fx_time <= 0 && !(ent->spawnflags & 4 )) // NO_AUTO_SHUTOFF
	{
		// shut me off
		shimmery_thing_use( ent, NULL, NULL );
	}
}

//------------------------------------------
void shimmery_thing_use( gentity_t *self, gentity_t *other, gentity_t *activator)
{
	if (self->behaviorSet[BSET_USE])
	{
		G_ActivateBehavior(self,BSET_USE);
	}

	if (self->e_ThinkFunc != thinkF_NULL)
	{
		self->e_ThinkFunc = thinkF_NULL;
	}
	else
	{
		self->e_ThinkFunc = thinkF_shimmery_thing_think;
		self->nextthink = level.time + 150;
		self->fx_time = self->delay;
	}
}

//------------------------------------------
void shimmery_thing_link( gentity_t *ent )
{
	gentity_t	*target = NULL;

	target = G_Find (target, FOFS(targetname), ent->target);

	if (!target)
	{
		Com_Printf( "shimmery_thing_link: unable to find target %s\n", ent->target );

		ent->e_ThinkFunc = thinkF_NULL;
		ent->nextthink = -1;

		return;
	}

	if (ent->targetname)
	{
		ent->e_UseFunc = useF_shimmery_thing_use;
	}

	if (!ent->targetname || !(ent->spawnflags & 1) )
	{
		ent->e_ThinkFunc = thinkF_shimmery_thing_think;
		ent->nextthink = level.time + 200;
		ent->fx_time = ent->delay + 200;
	}
	else
	{
		ent->e_ThinkFunc = thinkF_NULL;
		ent->nextthink = -1;
	}

	VectorCopy( target->s.origin, ent->s.origin2 );

	gi.linkentity( ent );

}

//------------------------------------------
void SP_fx_shimmery_thing( gentity_t *ent )
{
	G_SpawnFloat( "radius", "10", &ent->radius );
	G_SpawnInt( "delay", "2000", &ent->delay );

//	ent->svFlags |= SVF_BROADCAST;

	VectorCopy( ent->s.origin, ent->s.pos.trBase );

	ent->e_ThinkFunc = thinkF_shimmery_thing_link;
	ent->nextthink = 1000;

	gi.linkentity( ent );
}

/*QUAKED fx_borg_bolt (0 0 1) (-8 -8 -8) (8 8 8) STARTOFF
Emits yellow electric bolts from the specified point to the specified point.
Emits showers of sparks if the endpoints are sufficiently close.

  STARTOFF - effect is initially off

  "target" (required) end point of the beam.  Can be a func_train, info_notnull, etc.
  "target2" (optional) starting point of the beam if the start point is moving,
			otherwise, the start point is the origin of the fx_borg_bolt entity.
  "targetname" - toggles effect on/off each time it's used
*/

//------------------------------------------
void borg_bolt_think( gentity_t *ent )
{
	G_AddEvent( ent, EV_FX_BORG_BOLT, 0 );
	ent->nextthink = level.time + 100 + random() * 25;
}

//------------------------------------------
void borg_bolt_use( gentity_t *self, gentity_t *other, gentity_t *activator )
{
	if (self->behaviorSet[BSET_USE])
	{
		G_ActivateBehavior(self,BSET_USE);
	}

	if ( self->count )
	{
		self->e_ThinkFunc = thinkF_NULL;
	}
	else
	{
		self->e_ThinkFunc = thinkF_borg_bolt_think;
		self->nextthink = level.time + 200;
	}
	
	self->count = !self->count;
}

//------------------------------------------
void borg_bolt_link( gentity_t *ent )
{
	gentity_t	*target = NULL, *target2 = NULL;

	target = G_Find (target, FOFS(targetname), ent->target);
	target2 = G_Find (target2, FOFS(targetname), ent->target2);

	if (!target)
	{
		Com_Printf("borg_bolt_link: unable to find target %s\n", ent->target );

		ent->e_ThinkFunc = thinkF_NULL;
		ent->nextthink = -1;

		return;
	}

	if ( !target2 )
	{
		// Since there isn't a second target, stash my origin
		VectorCopy( ent->s.origin, ent->s.pos.trBase );
	}

//	ent->svFlags |= SVF_BROADCAST;// Broadcast to all clients

	// Stash the targets in case the end points are moving
	G_SetEnemy(ent, target);
	ent->chain = target2;

	VectorCopy( target->s.origin, ent->s.origin2 );

	if ( ent->targetname )
	{
		ent->e_UseFunc = useF_borg_bolt_use;
	}

	// This is used as the toggle switch
	ent->count = !(ent->spawnflags & 1);

	if (!ent->targetname || !(ent->spawnflags & 1) )
	{
		ent->e_ThinkFunc = thinkF_borg_bolt_think;	
		ent->nextthink = level.time + 1000;
	}
	else
	{
		ent->e_ThinkFunc = thinkF_NULL;
		ent->nextthink = -1;
	}

	G_SoundIndex( "sound/enemies/borg/borgtaser.wav" );
	gi.linkentity( ent );
}

//------------------------------------------
void SP_fx_borg_bolt( gentity_t *ent )
{
	ent->e_ThinkFunc = thinkF_borg_bolt_link;
	ent->nextthink = level.time + 1000;

	gi.linkentity( ent );
}


/*QUAKED fx_crew_beam_in (0 0 1) (-16 -16 -4) (16 16 4) TELEPORT_OUT
Transporter effect, same as beam-in effect for NPC

TELEPORT_OUT -- play the teleport out effect, default effect is teleport in

"targetname" - (required)  start effect when used.
"target" - (optional)  If target is used, effect spawns at the origin of target.  If not used, effect spawns at the origin of itself.
"type" - (optional) Select one from the list below, default will be Starfleet beam-in effect
			1 - Starfleet (default)
			2 - Borg
			3 - Stasis 
*/

//------------------------------------------
void crew_beam_in_use( gentity_t *self, gentity_t *other, gentity_t *activator )
{
	gentity_t	*target = NULL;
	gentity_t	*tent;

	if (self->behaviorSet[BSET_USE])
	{
		G_ActivateBehavior(self,BSET_USE);
	}

	if ( self->target )
	{
		target = G_Find (target, FOFS(targetname), self->target);

		if (!target)
		{
			Com_Printf( "crew_beam_in_use: unable to find target %s\n", self->target );
			return;
		}

		VectorCopy( target->currentOrigin, self->s.pos.trBase );
	}

	switch ( self->count )
	{
	case BORG_TELEPORT_FX:
		G_AddEvent( self, EV_BORG_TELEPORT, 0 );
		break;

	case STASIS_TELEPORT_FX:
		tent = G_TempEntity( self->currentOrigin, EV_STASIS_TELEPORT_IN );
		tent->owner = self;
		tent->count = 0;
		break;

	default:
		if ( self->spawnflags & 1 )	// teleport out
		{
			tent = G_TempEntity( self->s.pos.trBase, EV_PLAYER_TELEPORT_OUT );
		}
		else
		{
			tent = G_TempEntity( self->s.pos.trBase, EV_PLAYER_TELEPORT_IN );
		}

		tent->owner = target;
		break;
	}
}

//------------------------------------------
void SP_fx_crew_beam_in( gentity_t *ent )
{
	G_SpawnInt( "type", "1", &ent->count );

	if (ent->targetname)
	{
		ent->e_UseFunc = useF_crew_beam_in_use;
	}
	else
	{
		Com_Printf( "fx_crew_beam_in: no targetname was found!" );
	}

//	ent->svFlags |= SVF_BROADCAST;
	VectorCopy( ent->s.origin, ent->s.pos.trBase );
	gi.linkentity( ent );
}

/*QUAKED fx_stasis_teleporter (0 0 1) (-16 -16 -4) (16 16 4) STARTOFF
New stasis teleporter effect.

STARTOFF - Effect doesn't start until it's used.

"targetname" - (optional)  toggles effect on/off when used.

*/

//------------------------------------------
void stasis_teleporter_think( gentity_t *ent )
{
	gentity_t	*entity_list[MAX_GENTITIES];
	int			count;
	float		dist = 256.0f * 256.0f, len;
	vec3_t		diff;

	if ( !(ent->spawnflags & 2) ) // EXIT
	{
		// Effect should display proximity behaviour
		count = G_RadiusList( ent->currentOrigin, 256, ent, qtrue, entity_list );

		for ( int i = 0; i < count; i++ )
		{
			if ( entity_list[i]->client )
			{
				VectorSubtract( ent->currentOrigin, entity_list[i]->currentOrigin, diff );
				len = VectorLengthSquared( diff );

				if ( len < dist )
				{
					dist = len;
				}
			}
		}

		ent->radius = sqrt( dist );
	}

	G_AddEvent( ent, EV_FX_STASIS_TELEPORTER, 0 );
	ent->nextthink = level.time + 100;

	// This is all special case stuff--in stasis 1 there is a teleporter that gets turned into a mover.
	if ( ent->s.eType == ET_MOVER )
	{
		if ( VectorCompare( ent->currentOrigin, ent->pos2 ) )
		{
			// We are done moving, so turn me back into what I really should be, otherwise my sounds won't play like they should.
			ent->soundSet = "stasistransporter";
			ent->s.eType = ET_GENERAL;
		}
	}
}

//------------------------------------------
void stasis_teleporter_use( gentity_t *self, gentity_t *other, gentity_t *activator )
{
	if (self->behaviorSet[BSET_USE])
	{
		G_ActivateBehavior(self,BSET_USE);
	}

	if ( self->count )
	{
		// It was already on, so turn it off and ditch the soundset
		self->e_ThinkFunc = thinkF_NULL;
		self->soundSet = NULL;
	}
	else
	{
		// It wasn't on, so turn the thing on now
		self->e_ThinkFunc = thinkF_stasis_teleporter_think;
		self->nextthink = level.time + 100;
		self->soundSet = "stasistransporter";
	}

	// Toggle the state
	self->count = !self->count;
}

//------------------------------------------
void SP_fx_stasis_teleporter( gentity_t *ent )
{
//	ent->svFlags |= SVF_BROADCAST;
	VectorCopy( ent->s.origin, ent->s.pos.trBase );
	gi.linkentity( ent );

	if (ent->targetname)
	{
		ent->e_UseFunc = useF_stasis_teleporter_use;
	}

	if ( VALIDSTRING( ent->soundSet) == false && !(ent->spawnflags & 1) ) // START_OFF
	{
		ent->soundSet = "stasistransporter";
	}

	ent->count = !(ent->spawnflags & 1);	//active flag

	if (!ent->targetname || ent->count )
	{
		ent->e_ThinkFunc = thinkF_stasis_teleporter_think;	
		ent->nextthink = level.time + 1000;

		if ( VALIDSTRING( ent->soundSet) == false )
		{
			ent->soundSet = "stasistransporter";
		}
	}
	else
	{
		// We'll fire this thing up when it gets used
		ent->e_ThinkFunc = thinkF_NULL;
		ent->nextthink = -1;
	}
}

/*QUAKED fx_stasis_mushrooms (0 0 1) (-16 -16 -4) (16 16 4)
Ambient effect for when the stasis mushrooms get used.
*/

//------------------------------------------
void stasis_mushroom_think( gentity_t *ent )
{
	G_AddEvent( ent, EV_FX_STASIS_MUSHROOM, 0 );
	ent->nextthink = level.time + 75;

	// This thing will fire count times before shutting off.
	if ( --ent->count < 1 ) 
	{
		ent->e_ThinkFunc = thinkF_NULL;
	}
}

//------------------------------------------
void stasis_mushroom_use( gentity_t *self, gentity_t *other, gentity_t *activator )
{
	if (self->behaviorSet[BSET_USE])
	{
		G_ActivateBehavior(self,BSET_USE);
	}

	// Turn the thing on for a bit
	self->e_ThinkFunc = thinkF_stasis_mushroom_think;
	self->nextthink = level.time + 75;

	// This is the number of particles that get spewed per usage
	self->count = 12;
}

//------------------------------------------
void SP_fx_stasis_mushrooms( gentity_t *ent )
{
//	ent->svFlags |= SVF_BROADCAST;
	VectorCopy( ent->s.origin, ent->s.pos.trBase );
	gi.linkentity( ent );

	ent->e_UseFunc = useF_stasis_mushroom_use;
}

/*QUAKED fx_dn_beam_glow (0 0 1) (-16 -16 -4) (16 16 4) STARTOFF
Dreadnought focusing beam glow

  STARTOFF - effect starts off

"targetname" - (optional)  toggles effect on/off when used.
"target" - (required) thing to apply effect to
"radius" - beam impact radius ( default 64 )
*/

//------------------------------------------
void dn_beam_glow( gentity_t *self )
{
	// Check to see if this is the first time being called
	if ( !self->owner )
	{
		self->owner = G_Find( NULL, FOFS(targetname), self->target );

		if ( !self->owner )
		{
			// Not so good..so think no more
			Com_Printf( "dn_beam_glow->think: target not found!" );
			self->e_ThinkFunc = thinkF_NULL;
			return;
		}
	}

	vec3_t	end, dir;
	trace_t	trace;

	AngleVectors( self->owner->currentAngles, dir, NULL, NULL );
	VectorMA( self->currentOrigin, 1024, dir, end );

	gi.trace( &trace, self->currentOrigin, NULL, NULL, end, -1, CONTENTS_SOLID );//ignore

	if ( trace.fraction < 1.0 )
	{
		// we impacted, so render the effect
		VectorCopy( trace.endpos, self->pos1 );
		VectorCopy( trace.plane.normal, self->pos2 );

		G_AddEvent( self, EV_FX_DN_BEAM_GLOW, 0 );
	}

	self->nextthink = level.time + 100;	
}

//------------------------------------------
void dn_beam_use( gentity_t *self, gentity_t *other, gentity_t *activator )
{
	if ( self->e_ThinkFunc == thinkF_NULL )
	{
		// I was off, so turn me on.
		self->e_ThinkFunc = thinkF_dn_beam_glow;
		self->nextthink = level.time + 100;
	}
	else
	{
		// shut me off
		self->e_ThinkFunc = thinkF_NULL;
	}
}

//------------------------------------------
void SP_fx_dn_beam_glow( gentity_t *ent )
{
	G_SpawnFloat( "radius", "64", &ent->radius );

	G_SetOrigin( ent, ent->s.origin );//, ent->s.pos.trBase );
	gi.linkentity( ent );

	if ( ent->target )
	{
		ent->e_UseFunc = useF_dn_beam_use;

		if ( !ent->spawnflags & 1 )
		{
			ent->e_ThinkFunc = thinkF_dn_beam_glow;
			ent->nextthink = level.time + 100;
		}
	}
	else
	{
		Com_Printf( "fx_dn_beam_glow: no target found!" );
	}
}

/*QUAKED fx_garden_fountain (0 0 1) (-16 -16 -4) (16 16 4) STARTOFF
Simple fountain effect

  STARTOFF - effect starts off

"targetname" - (optional)  toggles effect on/off when used.
*/

//------------------------------------------
void garden_fountain_spurt( gentity_t *self )
{
	G_AddEvent( self, EV_FX_GARDEN_FOUNTAIN_SPURT, 0 );
	self->nextthink = level.time + 100;	
}

//------------------------------------------
void garden_foutain_use( gentity_t *self, gentity_t *other, gentity_t *activator )
{
	if ( self->e_ThinkFunc == thinkF_NULL )
	{
		// I was off, so turn me on.
		self->e_ThinkFunc = thinkF_garden_fountain_spurt;
		self->nextthink = level.time + 100;
	}
	else
	{
		// shut me off
		self->e_ThinkFunc = thinkF_NULL;
	}
}

//------------------------------------------
void SP_fx_garden_fountain( gentity_t *ent )
{
	VectorCopy( ent->s.origin, ent->s.pos.trBase );
	gi.linkentity( ent );

	ent->e_UseFunc = useF_garden_fountain_use;

	if ( !ent->spawnflags & 1 )
	{
		ent->e_ThinkFunc = thinkF_garden_fountain_spurt;
		ent->nextthink = level.time + 100;

		if ( VALIDSTRING( ent->soundSet) == false )
		{
			ent->soundSet = "fountain";
		}
	}
}