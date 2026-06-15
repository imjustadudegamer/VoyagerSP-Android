// g_misc.c

#include "g_local.h"
#include "g_functions.h"
#include "g_nav.h"
#include "g_items.h"
#include "g_infostrings.h"

extern ginfoitem_t	bg_infoItemList[];

extern void G_SetEnemy( gentity_t *self, gentity_t *enemy );
extern void G_SoundOnEnt (gentity_t *ent, soundChannel_t channel, const char *soundPath);

/*QUAKED func_group (0 0 0) ?
Used to group brushes together just for editor convenience.  They are turned into normal brushes by the utilities.
*/


/*QUAKED info_null (0 0.5 0) (-4 -4 -4) (4 4 4)
Used as a positional target for calculations in the utilities (spotlights, etc), but removed during gameplay.
*/
void SP_info_null( gentity_t *self ) {
	G_SetOrigin( self, self->s.origin );
	self->e_ThinkFunc = thinkF_G_FreeEntity;
	//Give other ents time to link
	self->nextthink = level.time + 1000;
}


/*QUAKED info_notnull (0 0.5 0) (-4 -4 -4) (4 4 4)
Used as a positional target for in-game calculation, like jumppad targets.
target_position does the same thing
*/
void SP_info_notnull( gentity_t *self ){
	G_SetOrigin( self, self->s.origin );
}


/*QUAKED light (0 1 0) (-8 -8 -8) (8 8 8) linear noIncidence
Non-displayed light.
"light" overrides the default 300 intensity. - affects size
a negative "light" will subtract the light's color
'Linear' checkbox gives linear falloff instead of inverse square
'noIncidence' checkbox makes lighting smoother
Lights pointed at a target will be spotlights.
"radius" overrides the default 64 unit radius of a spotlight at the target point.
"scale" multiplier for the light intensity - does not affect size
"color" sets the light's color
*/
void SP_light( gentity_t *self ) {
	G_FreeEntity( self );
}



/*
=================================================================================

TELEPORTERS

=================================================================================
*/

void TeleportPlayer( gentity_t *player, vec3_t origin, vec3_t angles, int teleport_fx ) {
	gentity_t	*tent;

	// use temp events at source and destination to prevent the effect
	// from getting dropped by a second player event
	switch ( teleport_fx )
	{
	case STASIS_TELEPORT_FX:
		tent = G_TempEntity( player->client->ps.origin, EV_STASIS_TELEPORT_OUT );
		tent->s.clientNum = player->s.clientNum;

		tent = G_TempEntity( origin, EV_STASIS_TELEPORT_IN );
		tent->s.clientNum = player->s.clientNum;
		break;

	case STARFLEET_TELEPORT_FX:
	default:
		tent = G_TempEntity( player->client->ps.origin, EV_PLAYER_TELEPORT_OUT );
		tent->s.clientNum = player->s.clientNum;

		tent = G_TempEntity( origin, EV_PLAYER_TELEPORT_IN );
		tent->s.clientNum = player->s.clientNum;
	}


	if ( player->NPC )
	{
		player->NPC->sPCurSegPoint1 = player->NPC->sPCurSegPoint2 = -1;
	}

	if ( player->NPC && ( player->NPC->aiFlags&NPCAI_FORM_TELE_NAV ) )
	{
		//My leader teleported, I was trying to catch up, take this off
		VectorClear( player->NPC->leaderTeleportSpot );
		player->NPC->aiFlags &= ~NPCAI_FORM_TELE_NAV;
		
	}
	else
	{ //Need to inform my followers I teleported
		gentity_t *follower = player->client->follower;
		while ( follower && follower->client && follower->NPC && follower->client->team_leader == player && follower->NPC->behaviorState == BS_FORMATION )
		{
			if ( !(follower->NPC->aiFlags&NPCAI_FORM_TELE_NAV) )
			{
				//Try to follow them for 10 seconds, then see if they're close, if not, try for another 10 seconds
				follower->NPC->navTime = level.time + 10000;
				follower->NPC->aiFlags |= NPCAI_FORM_TELE_NAV;
				follower->NPC->tempGoal->lastWaypoint = follower->NPC->tempGoal->waypoint = follower->NPC->tempGoal->lastValidWaypoint = WAYPOINT_NONE;
				VectorCopy( player->currentOrigin, follower->NPC->leaderTeleportSpot );
			}

			if ( follower->client )
			{
				follower = follower->client->follower;
			}
			else
			{
				follower = NULL;
			}
		}
	}

	// unlink to make sure it can't possibly interfere with G_KillBox
	gi.unlinkentity (player);

	VectorCopy ( origin, player->client->ps.origin );
	player->client->ps.origin[2] += 1;
	VectorCopy ( player->client->ps.origin, player->currentOrigin );

	// spit the player out
	AngleVectors( angles, player->client->ps.velocity, NULL, NULL );
	VectorScale( player->client->ps.velocity, 0, player->client->ps.velocity );
	//player->client->ps.pm_time = 160;		// hold time
	//player->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;

	// toggle the teleport bit so the client knows to not lerp
	player->client->ps.eFlags ^= EF_TELEPORT_BIT;

	// set angles
	SetClientViewAngle( player, angles );

	// kill anything at the destination
	G_KillBox (player);

	// save results of pmove
	PlayerStateToEntityState( &player->client->ps, &player->s );

	gi.linkentity (player);
}

void TeleportMover( gentity_t *mover, vec3_t origin, vec3_t diffAngles, qboolean snapAngle ) 
{//FIXME: need an effect
	vec3_t		oldAngle, newAngle;
	float		speed;

	// unlink to make sure it can't possibly interfere with G_KillBox
	gi.unlinkentity (mover);

	//reposition it
	VectorCopy( origin, mover->s.pos.trBase );
	VectorCopy( origin, mover->currentOrigin );

	//Maintain their previous speed, but adjusted for new direction
	if ( snapAngle )
	{//not a diffAngle, actually an absolute angle
		vec3_t	dir;

		VectorCopy( diffAngles, newAngle );
		AngleVectors( newAngle, dir, NULL, NULL );
		VectorNormalize( dir );//necessary?
		speed = VectorLength( mover->s.pos.trDelta );
		VectorScale( dir, speed, mover->s.pos.trDelta );
		mover->s.pos.trTime = level.time;

		VectorSubtract( newAngle, mover->s.apos.trBase, diffAngles );
		VectorCopy( newAngle, mover->s.apos.trBase );
	}
	else
	{
		speed = VectorNormalize( mover->s.pos.trDelta );

		vectoangles( mover->s.pos.trDelta, oldAngle );
		VectorAdd( oldAngle, diffAngles, newAngle );

		AngleVectors( newAngle, mover->s.pos.trDelta, NULL, NULL );
		VectorNormalize( mover->s.pos.trDelta );

		VectorScale( mover->s.pos.trDelta, speed, mover->s.pos.trDelta );
		mover->s.pos.trTime = level.time;

		//Maintain their previous angles, but adjusted to new orientation
		VectorAdd( mover->s.apos.trBase, diffAngles, mover->s.apos.trBase );
	}

	//Maintain their previous anglespeed, but adjusted to new orientation
	speed = VectorNormalize( mover->s.apos.trDelta );
	VectorAdd( mover->s.apos.trDelta, diffAngles, mover->s.apos.trDelta );
	VectorNormalize( mover->s.apos.trDelta );
	VectorScale( mover->s.apos.trDelta, speed, mover->s.apos.trDelta );

	mover->s.apos.trTime = level.time;
	
	//Tell them it was teleported this move
	mover->s.eFlags |= EF_TELEPORT_BIT;

	// kill anything at the destination
	//G_KillBox (mover);
	//FIXME: call touch func instead of killbox?

	gi.linkentity (mover);
}

void teleporter_touch (gentity_t *self, gentity_t *other, trace_t *trace)
{
	gentity_t		*dest;

	if (!other->client)
		return;
	dest = 	G_PickTarget( self->target );
	if (!dest) {
		gi.Printf ("Couldn't find teleporter destination\n");
		return;
	}

	TeleportPlayer( other, dest->s.origin, dest->s.angles, STARFLEET_TELEPORT_FX );
}

/*QUAK-D misc_teleporter (1 0 0) (-32 -32 -24) (32 32 -16)
Stepping onto this disc will teleport players to the targeted misc_teleporter_dest object.
*/
void SP_misc_teleporter (gentity_t *ent)
{
	gentity_t		*trig;

	if (!ent->target)
	{
		gi.Printf ("teleporter without a target.\n");
		G_FreeEntity( ent );
		return;
	}

	ent->s.modelindex = G_ModelIndex( "models/objects/dmspot.md3" );
	ent->s.clientNum = 1;
	ent->s.loopSound = G_SoundIndex("sound/world/amb10.wav");
	ent->contents = CONTENTS_SOLID;

	G_SetOrigin( ent, ent->s.origin );

	VectorSet (ent->mins, -32, -32, -24);
	VectorSet (ent->maxs, 32, 32, -16);
	gi.linkentity (ent);

	trig = G_Spawn ();
	trig->e_TouchFunc = touchF_teleporter_touch;
	trig->contents = CONTENTS_TRIGGER;
	trig->target = ent->target;
	trig->owner = ent;
	G_SetOrigin( trig, ent->s.origin );
	VectorSet (trig->mins, -8, -8, 8);
	VectorSet (trig->maxs, 8, 8, 24);
	gi.linkentity (trig);
	
}

/*QUAK-D misc_teleporter_dest (1 0 0) (-32 -32 -24) (32 32 -16) - - NODRAW
Point teleporters at these.
*/
void SP_misc_teleporter_dest( gentity_t *ent ) {
	if ( ent->spawnflags & 4 ){
		return;
	}

	G_SetOrigin( ent, ent->s.origin );

	gi.linkentity (ent);
}


//===========================================================

/*QUAKED misc_model (1 0 0) (-16 -16 -16) (16 16 16)
"model"		arbitrary .md3 or .ase file to display
turns into map triangles - not solid
*/
void SP_misc_model( gentity_t *ent ) {

#if 0
	ent->s.modelindex = G_ModelIndex( ent->model );
	VectorSet (ent->mins, -16, -16, -16);
	VectorSet (ent->maxs, 16, 16, 16);

	G_SetOrigin( ent, ent->s.origin );
	VectorCopy( ent->s.angles, ent->s.apos.trBase );
	gi.linkentity (ent);
#else
	G_FreeEntity( ent );
#endif
}

//===========================================================

void setCamera ( gentity_t *ent )
{
	vec3_t		dir;
	gentity_t	*target = 0;

	// frame holds the rotate speed
	if ( ent->owner->spawnflags & 1 ) 
	{
		ent->s.frame = 25;
	} 
	else if ( ent->owner->spawnflags & 2 ) 
	{
		ent->s.frame = 75;
	}

	// clientNum holds the rotate offset
	ent->s.clientNum = ent->owner->s.clientNum;

	VectorCopy( ent->owner->s.origin, ent->s.origin2 );

	// see if the portal_camera has a target
	if (ent->owner->target) {
		target = G_PickTarget( ent->owner->target );
	}
	if ( target ) 
	{
		VectorSubtract( target->s.origin, ent->owner->s.origin, dir );
		VectorNormalize( dir );
	} 
	else 
	{
		G_SetMovedir( ent->owner->s.angles, dir );
	}

	ent->s.eventParm = DirToByte( dir );
}

void cycleCamera( gentity_t *self )
{
	self->owner = G_Find( self->owner, FOFS(targetname), self->target );
	if  ( self->owner == NULL )
	{
		//Uh oh! Not targeted at any ents!  Or reached end of list?  Which is it?
		//for now assume reached end of list and are cycling
		self->owner = G_Find( self->owner, FOFS(targetname), self->target );
		if  ( self->owner == NULL )
		{//still didn't find one
			gi.Printf( "Couldn't find target for misc_portal_surface\n" );
			G_FreeEntity( self );
			return;
		}
	}
	setCamera( self );

	if ( self->e_ThinkFunc == thinkF_cycleCamera )
	{
		if ( self->owner->wait > 0 )
		{
			self->nextthink = level.time + self->owner->wait;
		}
		else
		{
			self->nextthink = level.time + self->wait;
		}
	}
}

void misc_portal_use( gentity_t *self, gentity_t *other, gentity_t *activator )
{
	cycleCamera( self );
}

void locateCamera( gentity_t *ent ) 
{//FIXME: make this fadeout with distance from misc_camera_portal

	ent->owner = G_Find(NULL, FOFS(targetname), ent->target);
	if ( !ent->owner ) 
	{
		gi.Printf( "Couldn't find target for misc_portal_surface\n" );
		G_FreeEntity( ent );
		return;
	}

	setCamera( ent );

	if ( G_Find(ent->owner, FOFS(targetname), ent->target) != NULL  )
	{//targeted at more than one thing
		ent->e_ThinkFunc = thinkF_cycleCamera;
		if ( ent->owner->wait > 0 )
		{
			ent->nextthink = level.time + ent->owner->wait;
		}
		else
		{
			ent->nextthink = level.time + ent->wait;
		}
	}
}

/*QUAKED misc_portal_surface (0 0 1) (-8 -8 -8) (8 8 8)
The portal surface nearest this entity will show a view from the targeted misc_portal_camera, or a mirror view if untargeted.
This must be within 64 world units of the surface!

targetname - When used, cycles to the next misc_portal_camera it's targeted
wait - makes it auto-cycle between all cameras it's pointed at at intevervals of specified number of seconds.

  cameras will be cycled through in the order they were created on the map.
*/
void SP_misc_portal_surface(gentity_t *ent) 
{
	VectorClear( ent->mins );
	VectorClear( ent->maxs );
	gi.linkentity (ent);

	ent->svFlags = SVF_PORTAL;
	ent->s.eType = ET_PORTAL;
	ent->wait *= 1000;

	if ( !ent->target ) 
	{//mirror?
		VectorCopy( ent->s.origin, ent->s.origin2 );
	} 
	else 
	{
		ent->e_ThinkFunc = thinkF_locateCamera;
		ent->nextthink = level.time + 100;

		if ( ent->targetname )
		{
			ent->e_UseFunc = useF_misc_portal_use;
		}
	}
}

/*QUAKED misc_portal_camera (0 0 1) (-8 -8 -8) (8 8 8) slowrotate fastrotate
The target for a misc_portal_surface.  You can set either angles or target another entity (NOT an info_null) to determine the direction of view.
"roll" an angle modifier to orient the camera around the target vector;
*/
void SP_misc_portal_camera(gentity_t *ent) {
	float	roll;

	VectorClear( ent->mins );
	VectorClear( ent->maxs );
	gi.linkentity (ent);

	G_SpawnFloat( "roll", "0", &roll );

	ent->s.clientNum = roll/360.0 * 256;
	ent->wait *= 1000;
}

/*
======================================================================

  SHOOTERS

======================================================================
*/

void Use_Shooter( gentity_t *ent, gentity_t *other, gentity_t *activator ) 
{
/*	vec3_t		dir;
	float		deg;
	vec3_t		up, right;
*/
	if (ent->behaviorSet[BSET_USE])
	{
		G_ActivateBehavior(ent,BSET_USE);
	}
/*
	// see if we have a target
	if ( ent->enemy ) {
		VectorSubtract( ent->enemy->currentOrigin, ent->s.origin, dir );
		VectorNormalize( dir );
	} else {
		VectorCopy( ent->movedir, dir );
	}

	// randomize a bit
	PerpendicularVector( up, dir );
	CrossProduct( up, dir, right );

	deg = crandom() * ent->random;
	VectorMA( dir, deg, up, dir );

	deg = crandom() * ent->random;
	VectorMA( dir, deg, right, dir );

	VectorNormalize( dir );

	switch ( ent->s.weapon ) 
	{
	case WP_GRENADE_LAUNCHER:
		fire_grenade( ent, ent->s.origin, dir );
		break;
	case WP_ROCKET_LAUNCHER:
		fire_rocket( ent, ent->s.origin, dir );
		break;
	case WP_PLASMAGUN:
		fire_plasma( ent, ent->s.origin, dir );
		break;
	}

	G_AddEvent( ent, EV_FIRE_WEAPON, 0 );
*/
}

void InitShooter( gentity_t *ent, int weapon ) {
	ent->e_UseFunc = useF_Use_Shooter;
	ent->s.weapon = weapon;

	RegisterItem( FindItemForWeapon( (weapon_t) weapon ) );

	G_SetMovedir( ent->s.angles, ent->movedir );

	if ( !ent->random ) {
		ent->random = 1.0;
	}
	ent->random = sin( M_PI * ent->random / 180 );
	// target might be a moving object, so we can't set movedir for it
	if ( ent->target ) {
		G_SetEnemy(ent, G_PickTarget( ent->target ));
	}
	gi.linkentity( ent );
}

/*QUAKED shooter_rocket (1 0 0) (-16 -16 -16) (16 16 16)
Fires at either the target or the current direction.
"random" the number of degrees of deviance from the taget. (1.0 default)
*/
void SP_shooter_rocket( gentity_t *ent ) 
{
	InitShooter( ent, WP_TETRION_DISRUPTOR );
}

/*QUAKED shooter_plasma (1 0 0) (-16 -16 -16) (16 16 16)
Fires at either the target or the current direction.
"random" is the number of degrees of deviance from the taget. (1.0 default)
*/
void SP_shooter_plasma( gentity_t *ent ) 
{
	InitShooter( ent, WP_COMPRESSION_RIFLE);
}

/*QUAKED shooter_grenade (1 0 0) (-16 -16 -16) (16 16 16)
Fires at either the target or the current direction.
"random" is the number of degrees of deviance from the taget. (1.0 default)
*/
void SP_shooter_grenade( gentity_t *ent ) 
{
	InitShooter( ent, WP_GRENADE_LAUNCHER);
}


/*QUAKED object_cargo_barrel1 (1 0 0) (-16 -16 -16) (16 16 29) SMALLER KLINGON NO_SMOKE POWDERKEG
Cargo Barrel
if given a targetname, using it makes it explode

SMALLER - (-8, -8, -16) (8, 8, 8)
KLINGON - klingon style barrel
NO_SMOKE - will not leave lingering smoke cloud when killed
POWDERKEG - wooden explosive barrel

health		 default = 20 
splashDamage default = 100
splashRadius default = 200
*/
void SP_object_cargo_barrel1(gentity_t *ent)
{
	if(ent->spawnflags & 8)
	{
		//FIXME: make an index into an external string table for localization
		if (g_language && Q_stricmp("DEUTSCH",g_language->string)==0)
		{
			ent->fullName = "Pulver-Container";
		}
		else
		{
			ent->fullName = "Powderkeg Barrel";
		}
		ent->s.modelindex = G_ModelIndex( "/models/mapobjects/cargo/barrel_wood2.md3" );
		ent->sounds = G_SoundIndex("sound/weapons/explosions/explode3.wav");
	}
	else if(ent->spawnflags & 2)
	{
		//FIXME: make an index into an external string table for localization
		if (g_language && Q_stricmp("DEUTSCH",g_language->string)==0)
		{
			ent->fullName = "Klingonen-Frachtcontainer";
		}
		else
		{
			ent->fullName = "Klingon Cargo Barrel";
		}
		ent->s.modelindex = G_ModelIndex( "/models/mapobjects/scavenger/k_barrel.md3" );
		ent->sounds = G_SoundIndex("sound/weapons/explosions/explode4.wav");
	}
	else
	{
		//FIXME: make an index into an external string table for localization

		if (g_language && Q_stricmp("DEUTSCH",g_language->string)==0)
		{
			ent->fullName = "Föderations-Frachtcontainer";
		}
		else
		{
			ent->fullName = "Federation Cargo Barrel";
		}
		ent->s.modelindex = G_ModelIndex( va("/models/mapobjects/cargo/barrel%i.md3", Q_irand( 0, 2 )) );
		ent->sounds = G_SoundIndex("sound/weapons/explosions/explode1.wav");
	}

	ent->contents = CONTENTS_SOLID;
	
	if ( ent->spawnflags & 1 )
	{
		VectorSet (ent->mins, -8, -8, -16);
		VectorSet (ent->maxs, 8, 8, 8);
	}
	else
	{
		VectorSet (ent->mins, -16, -16, -16);
		VectorSet (ent->maxs, 16, 16, 29);
	}

	G_SetOrigin( ent, ent->s.origin );
	VectorCopy( ent->s.angles, ent->s.apos.trBase );

	if(!ent->health)
		ent->health = 20;
	
	if(!ent->splashDamage)
		ent->splashDamage = 100;

	if(!ent->splashRadius)
		ent->splashRadius = 200;

	ent->takedamage = qtrue;

	ent->e_DieFunc = dieF_ExplodeDeath_Wait;
	
	if(ent->targetname)
		ent->e_UseFunc = useF_GoExplodeDeath;

	gi.linkentity (ent);
}


/*QUAKED misc_dlight (0.2 0.8 0.2) (-4 -4 -4) (4 4 4) STARTOFF FADEON FADEOFF PULSE
Dynamic light, toggles on and off when used
  
STARTOFF - Starts off
FADEON - Fades from 0 Radius to start Radius
FADEOFF - Fades from current Radius to 0 Radius before turning off
PULSE - This flag must be checked if you want it to fade/switch between start and final RGBA, otherwise it will just sit at startRGBA

ownername - Will display the light at the origin of the entity with this targetname

startRGBA - Red Green Blue Radius to start with - This MUST be set or your light won't do anything

These next values are used only if you want to fade/switch between 2 values (PULSE flag on)
finalRGBA - Red Green Blue Radius to end with
speed - how long to take to fade from start to final and final to start.  Also how long to fade on and off if appropriate flags are checked (seconds)
finaltime - how long to hold at final (seconds)
starttime - how long to hold at start (seconds)

TODO: Add random to speed/radius?
*/
void SP_misc_dlight(gentity_t *ent)
{
	G_SetOrigin( ent, ent->s.origin );
	gi.linkentity( ent );

	ent->speed *= 1000;
	ent->wait *= 1000;
	ent->radius *= 1000;

	//FIXME: attach self to a train or something?
	ent->e_UseFunc = useF_misc_dlight_use;
	
	ent->misc_dlight_active = qfalse;
	ent->e_clThinkFunc = clThinkF_NULL;

	ent->s.eType = ET_GENERAL;
	//Delay first think so we can find owner
	if ( ent->ownername )
	{
		ent->e_ThinkFunc = thinkF_misc_dlight_think;
		ent->nextthink = level.time + FRAMETIME;
	}

	if ( !(ent->spawnflags & 1) )
	{//Turn myself on now
		GEntity_UseFunc( ent, ent, ent );
	}
}

void misc_dlight_use ( gentity_t *ent, gentity_t *other, gentity_t *activator )
{
	if (ent->behaviorSet[BSET_USE])
	{
		G_ActivateBehavior(ent,BSET_USE);
	}

	if ( ent->misc_dlight_active )
	{//We're on, turn off
		if ( ent->spawnflags & 4 )
		{//fade off
			ent->pushDebounceTime = 3;
		}
		else
		{
			ent->misc_dlight_active = qfalse;
			ent->e_clThinkFunc = clThinkF_NULL;

			ent->s.eType = ET_GENERAL;
			ent->svFlags &= ~SVF_BROADCAST;
		}
	}
	else
	{
		//Start at start regardless of when we were turned off
		if ( ent->spawnflags & 4 )
		{//fade on
			ent->pushDebounceTime = 2;
		}
		else
		{//Just start on
			ent->pushDebounceTime = 0;
		}
		ent->painDebounceTime = level.time;

		ent->misc_dlight_active = qtrue;

		ent->e_ThinkFunc = thinkF_misc_dlight_think;
		ent->nextthink = level.time + FRAMETIME;

		ent->e_clThinkFunc = clThinkF_CG_DLightThink;

		ent->s.eType = ET_THINKER;
		ent->svFlags |= SVF_BROADCAST;// Broadcast to all clients
	}
}

void misc_dlight_think ( gentity_t *ent )
{
	//Stay Attached to owner
	if ( ent->owner )
	{
		G_SetOrigin( ent, ent->owner->currentOrigin );
		gi.linkentity( ent );
	}
	else if ( ent->ownername )
	{
		ent->owner = G_Find( NULL, FOFS(targetname), ent->ownername );
		ent->ownername = NULL;
	}
	ent->nextthink = level.time + FRAMETIME;
}


void station_pain( gentity_t *self, gentity_t *other, int damage ) 
{
//	self->s.modelindex = G_ModelIndex("/models/mapobjects/stasis/plugin2_in.md3");
//	self->s.eFlags &= ~ EF_ANIM_ALLFAST;
//	self->s.eFlags |= EF_ANIM_ONCE;
//	gi.linkentity (self);
	self->s.modelindex = self->s.modelindex2;
	gi.linkentity (self);
}

// --------------------------------------------------------------------
//
//   HEALTH/ARMOR plugin functions
//
// --------------------------------------------------------------------

void health_use( gentity_t *self, gentity_t *other, gentity_t *activator);
int ITM_AddArmor (gentity_t *ent, int count);
int ITM_AddHealth (gentity_t *ent, int count);

void health_shutdown( gentity_t *self )
{
	if (!(self->s.eFlags & EF_ANIM_ONCE))
	{
		self->s.eFlags &= ~ EF_ANIM_ALLFAST;
		self->s.eFlags |= EF_ANIM_ONCE;

		G_SpawnString( "infostring", NULL, &self->infoString );

		// Switch to and animate its used up model.
		if (!Q_stricmp(self->model,"models/mapobjects/stasis/plugin2.md3"))	
		{
			self->s.modelindex = self->s.modelindex2;
		}
		else if (!Q_stricmp(self->model,"models/mapobjects/borg/plugin2.md3"))	
		{
			self->s.modelindex = self->s.modelindex2;
		}                                
		else if (!Q_stricmp(self->model,"models/mapobjects/stasis/plugin2_floor.md3"))	
		{
			self->s.modelindex = self->s.modelindex2;
			G_Sound(self, G_SoundIndex("sound/ambience/stasis/shrinkage1.wav") );
		}
		else if (!Q_stricmp(self->model,"models/mapobjects/forge/panels.md3"))
		{
			self->s.modelindex = self->s.modelindex2;
		}

		gi.linkentity (self);
	}
}

void health_think( gentity_t *ent )
{
	int dif;

	// He's dead, Jim. Don't give him health
	if (ent->enemy->health<1)
	{
		ent->count = 0;
		ent->e_ThinkFunc = thinkF_NULL;
	}

	// Still has power to give
	if (ent->count > 0)
	{
		// For every 3 points of health, you get 1 point of armor
		// BUT!!! after health is filled up, you get the full energy going to armor

		dif = ent->enemy->client->ps.stats[STAT_MAX_HEALTH] - ent->enemy->health;

		if (dif > 3 )
		{
			dif= 3;
		}
		else if (dif < 0) 
		{
			dif= 0;	
		}

		if (dif > ent->count)	// Can't give more than count
		{
			dif = ent->count;
		}

		if ((ITM_AddHealth (ent->enemy,dif)) && (dif>0))		
		{
			ITM_AddArmor (ent->enemy,1);	// 1 armor for every 3 health

			ent->count-=dif;
			ent->nextthink = level.time + 10;
		}
		else	// User has taken all health he can hold, see about giving it all to armor
		{
			dif = ent->enemy->client->ps.stats[STAT_MAX_HEALTH] - 
				ent->enemy->client->ps.stats[STAT_ARMOR];

			if (dif > 3)
			{
				dif = 3;
			}
			else if (dif < 0) 
			{
				dif= 0;	
			}

			if (ent->count < dif)	// Can't give more than count
			{
				dif = ent->count;
			}

			if ((!ITM_AddArmor(ent->enemy,dif)) || (dif<=0))
			{
				ent->e_UseFunc = useF_health_use;	
				ent->e_ThinkFunc = thinkF_NULL;
			}
			else
			{
				ent->count-=dif;
				ent->nextthink = level.time + 10;
			}
		}
	}

	if (ent->count < 1)
	{
		health_shutdown(ent);
	}
}

void misc_model_useup( gentity_t *self, gentity_t *other, gentity_t *activator)
{
	if (self->behaviorSet[BSET_USE])
	{
		G_ActivateBehavior(self,BSET_USE);
	}

	self->s.eFlags &= ~ EF_ANIM_ALLFAST;
	self->s.eFlags |= EF_ANIM_ONCE;

	if ( VALIDSTRING( self->infoString ))
		self->infoString = NULL;

	// Switch to and animate its used up model.
	self->s.modelindex = self->s.modelindex2;

	gi.linkentity (self);

	// Use target when used
	if (self->spawnflags & 8)
	{
		G_UseTargets( self, activator );	
	}

	self->e_UseFunc = useF_NULL;	
	self->e_ThinkFunc = thinkF_NULL;
	self->nextthink = -1;
}

void health_use( gentity_t *self, gentity_t *other, gentity_t *activator)
{//FIXME: Heal entire team?  Or only those that are undying...?
	int dif;
	int dif2;
	int hold;

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

		if (other->client)
		{
			// He's dead, Jim. Don't give him health
			if (other->client->ps.stats[STAT_HEALTH]<1)
			{
				dif = 1;
				self->count = 0;
			}

			// Health
			dif = other->client->ps.stats[STAT_MAX_HEALTH] - other->client->ps.stats[STAT_HEALTH];
			// Armor
			dif2 = other->client->ps.stats[STAT_MAX_HEALTH] - other->client->ps.stats[STAT_ARMOR];
			hold = (dif2 - dif);
			// For every 3 points of health, you get 1 point of armor
			// BUT!!! after health is filled up, you get the full energy going to armor
			if (hold>0)	// Need more armor than health
			{
				// Calculate total amount of station energy needed.

				hold = dif / 3;	//	For every 3 points of health, you get 1 point of armor
				dif2 -= hold;
				dif2 += dif;	

				dif = dif2;
			}
		}
		else
		{	// Being triggered to be used up
			dif = 1;
			self->count = 0;
		}
		
		// Does player already have full health and full armor?
		if (dif > 0)
		{
			G_Sound(self, G_SoundIndex("sound/player/suithealth.wav") );

			if ((dif >= self->count) || (self->count<1)) // use it all up?
			{
				health_shutdown(self);
			}
			// Use target when used
			if (self->spawnflags & 8)
			{
				G_UseTargets( self, activator );	
			}

			self->e_UseFunc = useF_NULL;	
			self->enemy = other;
			self->e_ThinkFunc = thinkF_health_think;
			self->nextthink = level.time + 50;
		}
		else
		{
			G_Sound(self, G_SoundIndex("sound/weapons/noammo.wav") );
		}
	}	
}

// --------------------------------------------------------------------
//
//   AMMO plugin functions
//
// --------------------------------------------------------------------
void ammo_use( gentity_t *self, gentity_t *other, gentity_t *activator);
int Add_Ammo2 (gentity_t *ent, int ammoType, int count);

void ammo_shutdown( gentity_t *self )
{
	if (!(self->s.eFlags & EF_ANIM_ONCE))
	{
		self->s.eFlags &= ~ EF_ANIM_ALLFAST;
		self->s.eFlags |= EF_ANIM_ONCE;
		G_SpawnString( "infostring", NULL, &self->infoString );
		if (!Q_stricmp(self->model,"models/mapobjects/stasis/plugin.md3"))	
		{
			self->s.modelindex = self->s.modelindex2;
		}
		else if (!Q_stricmp(self->model,"models/mapobjects/borg/plugin.md3"))	
		{
			self->s.modelindex = self->s.modelindex2;
		}
		else if (!Q_stricmp(self->model,"models/mapobjects/stasis/plugin_floor.md3"))	
		{
			G_Sound(self, G_SoundIndex("sound/ambience/stasis/shrinkage.wav") );
			self->s.modelindex = self->s.modelindex2;
		}

		gi.linkentity (self);
	}
}
void ammo_think( gentity_t *ent )
{
	int dif;

	// Still has ammo to give
	if (ent->count > 0 && ent->enemy )
	{
		dif = ammoData[AMMO_STARFLEET].max  - ent->enemy->client->ps.ammo[AMMO_STARFLEET];

		if (dif > 2 )
		{
			dif= 2;
		}
		else if (dif < 0) 
		{
			dif= 0;	
		}

		if (ent->count < dif)	// Can't give more than count
		{
			dif = ent->count;
		}

		// Give player ammo 
		if (Add_Ammo2(ent->enemy,AMMO_STARFLEET,dif) && (dif!=0))	
		{
			ent->count-=dif;
			ent->nextthink = level.time + 10;
		}
		else	// User has taken all ammo he can hold
		{
			ent->e_UseFunc = useF_ammo_use;	
			ent->e_ThinkFunc = thinkF_NULL;
		}
	}

	if (ent->count < 1)
	{
		ammo_shutdown(ent);
	}
}

//------------------------------------------------------------
void ammo_use( gentity_t *self, gentity_t *other, gentity_t *activator)
{
	int dif;

	if (self->behaviorSet[BSET_USE])
	{
		G_ActivateBehavior(self,BSET_USE);
	}

	if (self->e_ThinkFunc != thinkF_NULL)
	{
		if (self->e_UseFunc != useF_NULL)
		{
			self->e_ThinkFunc = thinkF_NULL;
		}
	}
	else
	{
		if (other->client)
		{
			dif = ammoData[AMMO_STARFLEET].max - other->client->ps.ammo[AMMO_STARFLEET];
		}
		else
		{	// Being triggered to be used up
			dif = 1;
			self->count = 0;
		}

		// Does player already have full ammo?
		if (dif > 0)
		{
			G_Sound(self, G_SoundIndex("sound/player/suitenergy.wav") );

			if ((dif >= self->count) || (self->count<1)) // use it all up?
			{
				ammo_shutdown(self);
			}
		}	
		else
		{
			G_Sound(self, G_SoundIndex("sound/weapons/noammo.wav") );
		}
		// Use target when used
		if (self->spawnflags & 8)
		{
			G_UseTargets( self, activator );	
		}

		self->e_UseFunc = useF_NULL;	
		G_SetEnemy( self, other );
		self->e_ThinkFunc = thinkF_ammo_think;
		self->nextthink = level.time + 50;
	}	
}

//------------------------------------------------------------
void mega_ammo_use( gentity_t *self, gentity_t *other, gentity_t *activator )
{
	if ( self->behaviorSet[BSET_USE] )
		G_ActivateBehavior( self, BSET_USE );

	// Use target when used
	G_UseTargets( self, activator );

	// first use, adjust the max ammo a person can hold for each type of ammo
	ammoData[AMMO_STARFLEET].max	= 999;
	ammoData[AMMO_ALIEN].max		= 999;

	// Set up our count with whatever the max difference will be
	if ( other->client->ps.ammo[AMMO_ALIEN] > other->client->ps.ammo[AMMO_STARFLEET] )
		self->count = ammoData[AMMO_STARFLEET].max - other->client->ps.ammo[AMMO_STARFLEET];
	else
		self->count = ammoData[AMMO_ALIEN].max - other->client->ps.ammo[AMMO_ALIEN];

	G_Sound( self, G_SoundIndex("sound/player/superenergy.wav") );
	G_SpawnString( "infostring", NULL, &self->infoString );

	// Clear our usefunc, then think until our ammo is full
	self->e_UseFunc = useF_NULL;	
	G_SetEnemy( self, other );
	self->e_ThinkFunc = thinkF_mega_ammo_think;
	self->nextthink = level.time + 50;

	self->s.frame = 0;
	self->s.eFlags |= EF_ANIM_ONCE;
}

//------------------------------------------------------------
void mega_ammo_think( gentity_t *self )
{
	int	ammo_add = 5;

	// If the middle model is done animating, and we haven't switched to the last model yet...
	//		chuck up the last model.

	if (!Q_stricmp(self->model,"models/mapobjects/forge/power_up_boss.md3"))	// Because the normal forge_ammo model can use this too
	{
		if ( self->s.frame > 16 && self->s.modelindex != self->s.modelindex2 )
			self->s.modelindex = self->s.modelindex2;
	}
	
	if ( self->enemy && self->count > 0 )
	{
		// Add an equal ammount of ammo to each type
		self->enemy->client->ps.ammo[AMMO_STARFLEET]	+= ammo_add;
		self->enemy->client->ps.ammo[AMMO_ALIEN]		+= ammo_add;

		// Now cap to prevent overflows
		if ( self->enemy->client->ps.ammo[AMMO_STARFLEET] > ammoData[AMMO_STARFLEET].max )
			self->enemy->client->ps.ammo[AMMO_STARFLEET] = ammoData[AMMO_STARFLEET].max;

		if ( self->enemy->client->ps.ammo[AMMO_ALIEN] > ammoData[AMMO_ALIEN].max )
			self->enemy->client->ps.ammo[AMMO_ALIEN] = ammoData[AMMO_ALIEN].max;

		// Decrement the count given counter
		self->count -= ammo_add;

		// If we've given all we should, prevent giving any more, even if they player is no longer full
		if ( self->count <= 0 )
		{
			self->count = 0;
			self->e_ThinkFunc = thinkF_NULL;
			self->nextthink = -1;
		}
		else
			self->nextthink = 20;
	}
}

//------------------------------------------------------------
void spawn_stasis_control_trigger( gentity_t *ent )
{
	gentity_t	*other;
	vec3_t		mins, maxs;

	// Set the base bounds
	VectorCopy( ent->s.origin, mins);
	VectorCopy( ent->s.origin, maxs);

	// Now add an area of influence around the thing
	for ( int i = 0; i < 3; i++ )
	{
		maxs[i] += 64;
		mins[i] -= 64;
	}

	// create a trigger with this size
	other = G_Spawn();
	VectorCopy (mins, other->mins);
	VectorCopy (maxs, other->maxs);

	// set up the other bits that the engine needs to know
	other->owner = ent;
	other->contents = CONTENTS_TRIGGER;
	other->e_TouchFunc = touchF_touch_stasis_control_trigger;

	gi.linkentity (other);
}

//------------------------------------------------------------
void touch_stasis_control_trigger( gentity_t *self, gentity_t *other, trace_t *trace )
{
	if ( self->owner )
	{
		self->owner->s.eFlags |= EF_ANIM_ONCE;
		G_Sound( self->owner, G_SoundIndex("sound/movers/switches/stasisopen.wav") );
	}

	// Trigger once only
	self->e_TouchFunc = touchF_NULL;
}

//------------------------------------------------------------
void stasis_control_switch_use( gentity_t *self, gentity_t *other, gentity_t *activator)
{
	if ( self->spawnflags & 1 ) // NO_PLAYER
	{
		if (other)
		{
			if (other->client)
			{
				return;
			}
		}
	}

	if (self->behaviorSet[BSET_USE])
	{
		G_ActivateBehavior(self,BSET_USE);
	}

	if (self->e_ThinkFunc != thinkF_NULL)
	{
		self->e_ThinkFunc = thinkF_NULL;
	}

	if ( self->spawnflags & 2 ) // NO_TRIGGER
	{
		// There wasn't a trigger, so start the animation and the sound when the NPC uses us in a script
		self->s.eFlags |= EF_ANIM_ONCE;
		G_Sound(self, G_SoundIndex("sound/movers/switches/stasisopen.wav") );
	}
	else
	{
		G_Sound(self, G_SoundIndex("sound/movers/switches/stasispos.wav") );
	}

	G_UseTargets( self, activator );	

	self->e_UseFunc = useF_NULL;	
	G_SetEnemy( self, other );
}

//------------------------------------------------------------
void switch_models( gentity_t *self, gentity_t *other, gentity_t *activator )
{
	// FIXME: need a sound here!!
	if ( self->s.modelindex2 )
		self->s.modelindex = self->s.modelindex2;
}

//------------------------------------------------------------
void touch_ammo_crystal_tigger( gentity_t *self, gentity_t *other, trace_t *trace )
{
	if ( !other->client )
		return;

	// dead people can't pick things up
	if ( other->health < 1 )
		return;

	// Only player can pick it up
	if ( !other->s.number == 0 )
	{
		return;
	}

	if ( other->client->ps.ammo[ AMMO_ALIEN ] >= ammoData[AMMO_ALIEN].max )
	{
		return;		// can't hold any more
	}

	// Add the ammo
	other->client->ps.ammo[AMMO_ALIEN] += self->owner->count;

	if ( other->client->ps.ammo[AMMO_ALIEN] > ammoData[AMMO_ALIEN].max ) 
	{
		other->client->ps.ammo[AMMO_ALIEN] = ammoData[AMMO_ALIEN].max;
	}

	// Trigger once only
	self->e_TouchFunc = touchF_NULL;

	// swap the model to the version without the crystal and ditch the infostring
	self->owner->s.modelindex = self->owner->s.modelindex2;
	self->owner->infoString = NULL;

	// play the normal pickup sound
	G_AddEvent( other, EV_ITEM_PICKUP, ITM_AMMO_CRYSTAL_BORG );

	// fire item targets
	G_UseTargets( self->owner, other );
}

//------------------------------------------------------------
void spawn_ammo_crystal_trigger( gentity_t *ent )
{
	gentity_t	*other;
	vec3_t		mins, maxs;

	// Set the base bounds
	VectorCopy( ent->s.origin, mins );
	VectorCopy( ent->s.origin, maxs );

	// Now add an area of influence around the thing
	for ( int i = 0; i < 3; i++ )
	{
		maxs[i] += 48;
		mins[i] -= 48;
	}

	// create a trigger with this size
	other = G_Spawn( );

	VectorCopy( mins, other->mins );
	VectorCopy( maxs, other->maxs );

	// set up the other bits that the engine needs to know
	other->owner = ent;
	other->contents = CONTENTS_TRIGGER;
	other->e_TouchFunc = touchF_touch_ammo_crystal_tigger;

	gi.linkentity( other );
}

//REPLICATOR ITEMS
infoStringItem_t G_InfoStringForModel( const char *model )
{
	infoStringItem_t infoString = II_NONE;
	//NOTE THIS ASSUMES COMMON INFOSTRINGS ARE IN CONSECUTIVE ORDER
	if ( Q_stricmp( "models/mapobjects/tour_mode/roastbeef.md3", model ) == 0 )
	{
		infoString = (infoStringItem_t)Q_irand( II_ROASTBEEF, II_STEAK );
	}
	else if ( Q_stricmp( "models/mapobjects/cargo/square_cup.md3", model ) == 0 )
	{
		infoString = (infoStringItem_t)Q_irand( II_WINE, II_BERGUNDY );
	}
	else if ( Q_stricmp( "models/mapobjects/tour_mode/cup.md3", model ) == 0 )
	{
		infoString = (infoStringItem_t)Q_irand( II_COFFEE, II_LANDRASCOFFEE );
	}
	else if ( Q_stricmp( "models/mapobjects/cargo/tray1.md3", model ) == 0 )
	{
		infoString = (infoStringItem_t)Q_irand( II_PASTA, II_CRISPS );
	}
	else if ( Q_stricmp( "models/mapobjects/cargo/tray2.md3", model ) == 0 )
	{
		infoString = (infoStringItem_t)Q_irand( II_STEAK_SALAD, II_SOUP_FRITTERS );
	}
	else
	{//nothing!
	}

	return infoString;
}

char *G_SoundForInfoString( infoStringItem_t infoString )
{
	char *playSound = NULL;

	if ( infoString <= II_NONE || infoString >= II_NUM_ITEMS )
	{
		return NULL;
	}
	switch ( infoString )
	{
	case II_ROASTBEEF:
		playSound = "sound/voice/computer/tour/beef1.wav";
		break;
	case II_PORKCHOPS:
		playSound = "sound/voice/computer/tour/beef2.wav";
		break;
	case II_RUMPROAST:
		playSound = "sound/voice/computer/tour/beef3.wav";
		break;
	case II_STEAK:
		playSound = "sound/voice/computer/tour/beef4.wav";
		break;
	case II_WINE:
		playSound = "sound/voice/computer/tour/cup1.wav";
		break;
	case II_BRANDY:
		playSound = "sound/voice/computer/tour/cup2.wav";
		break;
	case II_LATOUR:
		playSound = "sound/voice/computer/tour/cup3.wav";
		break;
	case II_BERGUNDY:
		playSound = "sound/voice/computer/tour/cup4.wav";
		break;
	case II_COFFEE:
		playSound = "sound/voice/computer/tour/cup5.wav";
		break;
	case II_JUICE:
		playSound = "sound/voice/computer/tour/cup6.wav";
		break;
	case II_TEA:
		playSound = "sound/voice/computer/tour/cup62.wav";
		break;
	case II_LANDRASCOFFEE:
		playSound = "sound/voice/computer/tour/cup7.wav";
		break;
	case II_PASTA:
		playSound = "sound/voice/computer/tour/plate.wav";
		break;
	case II_BANTAN:
		playSound = "sound/voice/computer/tour/plate1.wav";
		break;
	case II_CARDAWAY:
		playSound = "sound/voice/computer/tour/plate2.wav";
		break;
	case II_CRISPS:
		playSound = "sound/voice/computer/tour/plate3.wav";
		break;
	case II_STEAK_SALAD:
		playSound = "sound/voice/computer/tour/bowl.wav";
		break;
	case II_STEW_VEGG:
		playSound = "sound/voice/computer/tour/bowl2.wav";
		break;
	case II_SOUP_BISCUITS:
		playSound = "sound/voice/computer/tour/bowl3.wav";
		break;
	case II_SOUP_FRITTERS:
		playSound = "sound/voice/computer/tour/bowl4.wav";
		break;
	//NOTE: No Romulan Ale
	}

	return playSound;
}

void G_PrecacheModelSounds( const char *model )
{
	if ( Q_stricmp( "models/mapobjects/tour_mode/roastbeef.md3", model ) == 0 )
	{
		G_SoundIndex( G_SoundForInfoString( II_ROASTBEEF ) );
		G_SoundIndex( G_SoundForInfoString( II_PORKCHOPS ) );
		G_SoundIndex( G_SoundForInfoString( II_RUMPROAST ) );
		G_SoundIndex( G_SoundForInfoString( II_STEAK ) );
	}
	else if ( Q_stricmp( "models/mapobjects/cargo/square_cup.md3", model ) == 0 )
	{
		G_SoundIndex( G_SoundForInfoString( II_WINE ) );
		G_SoundIndex( G_SoundForInfoString( II_BRANDY ) );
		G_SoundIndex( G_SoundForInfoString( II_LATOUR ) );
		G_SoundIndex( G_SoundForInfoString( II_BERGUNDY ) );
	}
	else if ( Q_stricmp( "models/mapobjects/tour_mode/cup.md3", model ) == 0 )
	{
		G_SoundIndex( G_SoundForInfoString( II_COFFEE ) );
		G_SoundIndex( G_SoundForInfoString( II_JUICE ) );
		G_SoundIndex( G_SoundForInfoString( II_TEA ) );
		G_SoundIndex( G_SoundForInfoString( II_LANDRASCOFFEE ) );
	}
	else if ( Q_stricmp( "models/mapobjects/cargo/tray1.md3", model ) == 0 )
	{
		G_SoundIndex( G_SoundForInfoString( II_PASTA ) );
		G_SoundIndex( G_SoundForInfoString( II_BANTAN ) );
		G_SoundIndex( G_SoundForInfoString( II_CARDAWAY ) );
		G_SoundIndex( G_SoundForInfoString( II_CRISPS ) );
	}
	else if ( Q_stricmp( "models/mapobjects/cargo/tray2.md3", model ) == 0 )
	{
		G_SoundIndex( G_SoundForInfoString( II_STEAK_SALAD ) );
		G_SoundIndex( G_SoundForInfoString( II_STEW_VEGG ) );
		G_SoundIndex( G_SoundForInfoString( II_SOUP_BISCUITS ) );
		G_SoundIndex( G_SoundForInfoString( II_SOUP_FRITTERS ) );
	}
	else
	{//nothing!
	}
}

void PlaySoundAndSetInfostring( gentity_t *self, gentity_t *activator, const char *model )
{
	infoStringItem_t	infoString = II_NONE;
	char				*playSound = NULL;

	if ( !self || !model )
	{
		return;
	}
	infoString = G_InfoStringForModel( model );
	
	if ( infoString > II_NONE && infoString < II_NUM_ITEMS )
	{
		self->infoString = bg_infoItemList[infoString].infoString;
		self->contents |= CONTENTS_CORPSE;
		playSound = G_SoundForInfoString( infoString );
		if ( playSound != NULL )
		{
			G_SoundOnEnt( activator, CHAN_ANNOUNCER, playSound );
		}
	}
}

void misc_replicator_item_remove ( gentity_t *self, gentity_t *other, gentity_t *activator )
{
	self->s.eFlags |= EF_NODRAW;
	self->contents = 0;
	self->s.modelindex = 0;
	self->e_UseFunc = useF_misc_replicator_item_spawn;
	//FIXME: pickup sound?
	if ( activator->client )
	{
		activator->health += 5;
		if ( activator->health > activator->client->ps.stats[STAT_MAX_HEALTH] )	// Past max health
		{
			activator->health = activator->client->ps.stats[STAT_MAX_HEALTH];
		}
	}
}

void misc_replicator_item_finish_spawn( gentity_t *self )
{
	//self->contents = CONTENTS_ITEM;
	//FIXME: blinks out for a couple frames when transporter effect is done?
	self->e_UseFunc = useF_misc_replicator_item_remove;
}

void misc_replicator_item_spawn ( gentity_t *self, gentity_t *other, gentity_t *activator )
{
	char *model = NULL;
	switch ( Q_irand( 1, self->count ) )
	{
	case 1:
		self->s.modelindex = self->bounceCount;
		model = self->model;
		break;
	case 2:
		self->s.modelindex = self->fly_sound_debounce_time;
		model = self->model2;
		break;
	case 3:
		self->s.modelindex = self->painDebounceTime;
		model = self->target;
		break;
	case 4:
		self->s.modelindex = self->disconnectDebounceTime;
		model = self->target2;
		break;
	case 5:
		self->s.modelindex = self->attackDebounceTime;
		model = self->target3;
		break;
	case 6://max
		self->s.modelindex = self->pushDebounceTime;
		model = self->target3;
		break;
	}
	self->s.eFlags &= ~EF_NODRAW;
	self->e_ThinkFunc = thinkF_misc_replicator_item_finish_spawn;
	self->nextthink = level.time + 4000;//shorter?
	self->e_UseFunc = useF_NULL;

	PlaySoundAndSetInfostring( self, activator, model );

	gentity_t *tent = G_TempEntity( self->currentOrigin, EV_REPLICATOR );
	tent->owner = self;
}

/*QUAKED misc_replicator_item (0.2 0.8 0.2) (-4 -4 0) (4 4 8) 
When used. this will "spawn" an entity with a random model from the ones provided below...

Using it again removes the item as if it were picked up.

model  - first random model key
model2  - second random model key
model3  - third random model key
model4  - fourth random model key
model5  - fifth random model key
model6  - sixth random model key

NOTE: do not skip one of these model names, start with the lowest and fill in each next highest one with a value.  A gap will cause the item to not work correctly.

NOTE: if you use an invalid model, it will still try to use it and show the NULL axis model (or nothing at all)

targetname - how you refer to it for using it

TODO: Some way to assign an infoString to each model?
*/
void SP_misc_replicator_item ( gentity_t *self )
{
	//precache
	if ( self->model )
	{
		self->bounceCount = G_ModelIndex( self->model );
		G_PrecacheModelSounds( self->model );
		self->count++;
		if ( self->model2 )
		{
			self->fly_sound_debounce_time = G_ModelIndex( self->model2 );
			G_PrecacheModelSounds( self->model2 );
			self->count++;
			if ( self->target )
			{
				self->painDebounceTime = G_ModelIndex( self->target );
				G_PrecacheModelSounds( self->target );
				self->count++;
				if ( self->target2 )
				{
					self->disconnectDebounceTime = G_ModelIndex( self->target2 );
					G_PrecacheModelSounds( self->target2 );
					self->count++;
					if ( self->target3 )
					{
						self->attackDebounceTime = G_ModelIndex( self->target3 );
						G_PrecacheModelSounds( self->target3 );
						self->count++;
						if ( self->target4 )
						{
							self->pushDebounceTime = G_ModelIndex( self->target4 );
							G_PrecacheModelSounds( self->target4 );
							self->count++;
						}
					}
				}
			}
		}
	}
	G_SoundIndex( "sound/movers/switches/replicator.wav" );
	self->e_UseFunc = useF_misc_replicator_item_spawn;

	self->s.eFlags |= EF_NODRAW;
	//self->contents = 0;

	VectorSet( self->mins, -4, -4, 0 );
	VectorSet( self->maxs, 4, 4, 8 );
	G_SetOrigin( self, self->s.origin );
	G_SetAngles( self, self->s.angles );

	gi.linkentity( self );
}