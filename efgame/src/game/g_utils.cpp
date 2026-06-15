// g_utils.c -- misc utility functions for game module

#include "g_local.h"
#include "g_functions.h"
#include "g_nav.h"

#define ACT_ACTIVE		qtrue
#define ACT_INACTIVE	qfalse
extern void NPC_UseResponse ( gentity_t *self, gentity_t *user, qboolean useWhenDone );
extern void G_InitBoltOnData ( gentity_t *ent );
extern int ffireLevel;
extern const int FFIRE_LEVEL_RETALIATION;
/*
=========================================================================

model / sound configstring indexes

=========================================================================
*/

/*
================
G_FindConfigstringIndex

================
*/
int G_FindConfigstringIndex( char *name, int start, int max, qboolean create ) {
	int		i;
	char	s[MAX_STRING_CHARS];

	if ( !name || !name[0] ) {
		return 0;
	}

	for ( i=1 ; i<max ; i++ ) {
		gi.GetConfigstring( start + i, s, sizeof( s ) );
		if ( !s[0] ) {
			break;
		}
		if ( !strcmp( s, name ) ) {
			return i;
		}
	}

	if ( !create ) {
		return 0;
	}

	if ( i == max ) {
		G_Error( "G_FindConfigstringIndex: overflow" );
	}

	gi.SetConfigstring( start + i, name );

	return i;
}


int G_ModelIndex( char *name ) {
	return G_FindConfigstringIndex (name, CS_MODELS, MAX_MODELS, qtrue);
}

int G_SoundIndex( char *name ) {
	return G_FindConfigstringIndex (name, CS_SOUNDS, MAX_SOUNDS, qtrue);
}

//===Bypass network for sounds on specific channels====================

extern void cgi_S_StartSound( vec3_t origin, int entityNum, int entchannel, sfxHandle_t sfx );
#include "../cgame/cg_media.h"	//access to cgs
extern void CG_TryPlayCustomSound( vec3_t origin, int entityNum, soundChannel_t channel, const char *soundName, int customSoundSet );
//NOTE: Do NOT Try to use this before the cgame DLL is valid, it will NOT work!
void G_SoundOnEnt (gentity_t *ent, soundChannel_t channel, const char *soundPath)
{
	int	index = G_SoundIndex( (char *)soundPath );

	if ( cgs.sound_precache[ index ] ) 
	{
		cgi_S_StartSound( NULL, ent->s.number, channel, cgs.sound_precache[ index ] );
	}
	else
	{
		CG_TryPlayCustomSound( NULL, ent->s.number, channel, soundPath, -1 );
	}
}
//=====================================================================


/*
================
G_TeamCommand

Broadcasts a command to only a specific team
================
*/
void G_TeamCommand( team_t team, char *cmd ) {
	int		i;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( level.clients[i].pers.connected == CON_CONNECTED ) {
			if ( level.clients[i].ps.persistant[PERS_TEAM] == team ) {
				gi.SendServerCommand( i, "%s", cmd );
			}
		}
	}
}


/*
=============
G_Find

Searches all active entities for the next one that holds
the matching string at fieldofs (use the FOFS() macro) in the structure.

Searches beginning at the entity after from, or the beginning if NULL
NULL will be returned if the end of the list is reached.

=============
*/
gentity_t *G_Find (gentity_t *from, int fieldofs, char *match)
{
	char	*s;

	if(!match || !match[0])
	{
		return NULL;
	}

	if (!from)
		from = g_entities;
	else
		from++;

	for ( ; from < &g_entities[globals.num_entities] ; from++)
	{
		if (!from->inuse)
			continue;
		s = *(char **) ((byte *)from + fieldofs);
		if (!s)
			continue;
		if (!Q_stricmp (s, match))
			return from;
	}

	return NULL;
}


/*
============
G_RadiusList - given an origin and a radius, return all entities that are in use that are within the list
============
*/
int G_RadiusList ( vec3_t origin, float radius,	gentity_t *ignore, qboolean takeDamage, gentity_t *ent_list[MAX_GENTITIES])					  
{
	float		dist;
	gentity_t	*ent;
	gentity_t	*entityList[MAX_GENTITIES];
	int			numListedEntities;
	vec3_t		mins, maxs;
	vec3_t		v;
	int			i, e;
	int			ent_count = 0;

	if ( radius < 1 ) 
	{
		radius = 1;
	}

	for ( i = 0 ; i < 3 ; i++ ) 
	{
		mins[i] = origin[i] - radius;
		maxs[i] = origin[i] + radius;
	}

	numListedEntities = gi.EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );

	for ( e = 0 ; e < numListedEntities ; e++ ) 
	{
		ent = entityList[ e ];

		if ((ent == ignore) || !(ent->inuse) || ent->takedamage != takeDamage)
			continue;

		// find the distance from the edge of the bounding box
		for ( i = 0 ; i < 3 ; i++ ) 
		{
			if ( origin[i] < ent->absmin[i] ) 
			{
				v[i] = ent->absmin[i] - origin[i];
			} else if ( origin[i] > ent->absmax[i] ) 
			{
				v[i] = origin[i] - ent->absmax[i];
			} else 
			{
				v[i] = 0;
			}
		}

		dist = VectorLength( v );
		if ( dist >= radius ) 
		{
			continue;
		}
		
		// ok, we are within the radius, add us to the incoming list
		ent_list[ent_count] = ent;
		ent_count++;

	}
	// we are done, return how many we found
	return(ent_count);
}


/*
=============
G_PickTarget

Selects a random entity from among the targets
=============
*/
#define MAXCHOICES	32

gentity_t *G_PickTarget (char *targetname)
{
	gentity_t	*ent = NULL;
	int		num_choices = 0;
	gentity_t	*choice[MAXCHOICES];

	if (!targetname)
	{
		gi.Printf("G_PickTarget called with NULL targetname\n");
		return NULL;
	}

	while(1)
	{
		ent = G_Find (ent, FOFS(targetname), targetname);
		if (!ent)
			break;
		choice[num_choices++] = ent;
		if (num_choices == MAXCHOICES)
			break;
	}

	if (!num_choices)
	{
		gi.Printf("G_PickTarget: target %s not found\n", targetname);
		return NULL;
	}

	return choice[rand() % num_choices];
}

void G_UseTargets2 (gentity_t *ent, gentity_t *activator, const char *string)
{
	gentity_t		*t;
	
//
// fire targets
//
	if (string)
	{
		t = NULL;
		while ( (t = G_Find (t, FOFS(targetname), (char *) string)) != NULL )
		{
			if (t == ent)
			{
//				gi.Printf ("WARNING: Entity used itself.\n");
			}
			else if ( !(t->svFlags & SVF_INACTIVE) )
			{
				if (t->e_UseFunc != useF_NULL)	// check can be omitted
				{
					GEntity_UseFunc(t, ent, activator);
				}
			}

			if (!ent->inuse)
			{
				gi.Printf("entity was removed while using targets\n");
				return;
			}
		}
	}
}

/*
==============================
G_UseTargets

"activator" should be set to the entity that initiated the firing.

Search for (string)targetname in all entities that
match (string)self.target and call their .use function

==============================
*/
void G_UseTargets (gentity_t *ent, gentity_t *activator)
{
//
// fire targets
//
	G_UseTargets2 (ent, activator, ent->target);
}


/*
=============
TempVector

This is just a convenience function
for making temporary vectors for function calls
=============
*/
float	*tv( float x, float y, float z ) {
	static	int		index;
	static	vec3_t	vecs[8];
	float	*v;

	// use an array so that multiple tempvectors won't collide
	// for a while
	v = vecs[index];
	index = (index + 1)&7;

	v[0] = x;
	v[1] = y;
	v[2] = z;

	return v;
}


/*
=============
VectorToString

This is just a convenience function
for printing vectors
=============
*/
char	*vtos( const vec3_t v ) {
	static	int		index;
	static	char	str[8][32];
	char	*s;

	// use an array so that multiple vtos won't collide
	s = str[index];
	index = (index + 1)&7;

	Com_sprintf (s, 32, "(%i %i %i)", (int)v[0], (int)v[1], (int)v[2]);

	return s;
}


/*
===============
G_SetMovedir

The editor only specifies a single value for angles (yaw),
but we have special constants to generate an up or down direction.
Angles will be cleared, because it is being used to represent a direction
instead of an orientation.
===============
*/
void G_SetMovedir( vec3_t angles, vec3_t movedir ) {
	static vec3_t VEC_UP		= {0, -1, 0};
	static vec3_t MOVEDIR_UP	= {0, 0, 1};
	static vec3_t VEC_DOWN		= {0, -2, 0};
	static vec3_t MOVEDIR_DOWN	= {0, 0, -1};

	if ( VectorCompare (angles, VEC_UP) ) {
		VectorCopy (MOVEDIR_UP, movedir);
	} else if ( VectorCompare (angles, VEC_DOWN) ) {
		VectorCopy (MOVEDIR_DOWN, movedir);
	} else {
		AngleVectors (angles, movedir, NULL, NULL);
	}
	VectorClear( angles );
}


float vectoyaw( const vec3_t vec ) {
	float	yaw;
	
	if (vec[YAW] == 0 && vec[PITCH] == 0) {
		yaw = 0;
	} else {
		if (vec[PITCH]) {
			yaw = ( atan2( vec[YAW], vec[PITCH]) * 180 / M_PI );
		} else if (vec[YAW] > 0) {
			yaw = 90;
		} else {
			yaw = 270;
		}
		if (yaw < 0) {
			yaw += 360;
		}
	}

	return yaw;
}


void G_InitGentity( gentity_t *e ) 
{
	e->inuse = qtrue;
	e->classname = "noclass";
	e->s.number = e - g_entities;

	ICARUS_FreeEnt( e );	//ICARUS information must be added after this point

	//Navigational setups
	e->waypoint				= WAYPOINT_NONE;
	e->lastWaypoint			= WAYPOINT_NONE;
	e->lastValidWaypoint	= WAYPOINT_NONE;

	G_InitBoltOnData( e );
}

/*
=================
G_Spawn

Either finds a free entity, or allocates a new one.

  The slots from 0 to MAX_CLIENTS-1 are always reserved for clients, and will
never be used by anything else.

Try to avoid reusing an entity that was recently freed, because it
can cause the client to think the entity morphed into something else
instead of being removed and recreated, which can cause interpolated
angles and bad trails.
=================
*/
gentity_t *G_Spawn( void ) 
{
	int			i, force;
	gentity_t	*e;

	e = NULL;	// shut up warning
	i = 0;		// shut up warning
	for ( force = 0 ; force < 2 ; force++ ) {
		// if we go through all entities and can't find one to free,
		// override the normal minimum times before use
		e = &g_entities[MAX_CLIENTS];
		for ( i = MAX_CLIENTS ; i<globals.num_entities ; i++, e++) {
			if ( e->inuse ) {
				continue;
			}

			// the first couple seconds of server time can involve a lot of
			// freeing and allocating, so relax the replacement policy
			if ( !force && e->freetime > 2000 && level.time - e->freetime < 1000 ) {
				continue;
			}

			// reuse this slot
			G_InitGentity( e );
			return e;
		}
		if ( i != ENTITYNUM_MAX_NORMAL ) {
			break;
		}
	}
	if ( i == ENTITYNUM_MAX_NORMAL ) {
		G_Error( "G_Spawn: no free entities" );
	}
	
	// open up a new slot
	globals.num_entities++;
	G_InitGentity( e );
	return e;
}


/*
=================
G_FreeEntity

Marks the entity as free
=================
*/
void G_FreeEntity( gentity_t *ed ) {
	gi.unlinkentity (ed);		// unlink from world

	ICARUS_FreeEnt( ed );

	/*if ( ed->neverFree ) {
		return;
	}*/

	memset (ed, 0, sizeof(*ed));
	//FIXME: This sets the entity number to zero, which is the player.
	//			This is really fucking stupid and has caused us no end of
	//			trouble!!!  Change it to set the s.number to the proper index
	//			(ed - g_entities) *OR* ENTITYNUM_NONE!!!
	ed->classname = "freed";
	ed->freetime = level.time;
	ed->inuse = qfalse;
}

/*
=================
G_TempEntity

Spawns an event entity that will be auto-removed
The origin will be snapped to save net bandwidth, so care
must be taken if the origin is right on a surface (snap towards start vector first)
=================
*/
gentity_t *G_TempEntity( vec3_t origin, int event ) {
	gentity_t		*e;
	vec3_t		snapped;

	e = G_Spawn();
	e->s.eType = ET_EVENTS + event;

	e->classname = "tempEntity";
	e->eventTime = level.time;
	e->freeAfterEvent = qtrue;

	VectorCopy( origin, snapped );
	SnapVector( snapped );		// save network bandwidth
	G_SetOrigin( e, snapped );

	// find cluster for PVS
	gi.linkentity( e );

	return e;
}



/*
==============================================================================

Kill box

==============================================================================
*/

/*
=================
G_KillBox

Kills all entities that would touch the proposed new positioning
of ent.  Ent should be unlinked before calling this!
=================
*/
void G_KillBox (gentity_t *ent) {
	int			i, num;
	gentity_t	*touch[MAX_GENTITIES], *hit;
	vec3_t		mins, maxs;

	VectorAdd( ent->client->ps.origin, ent->mins, mins );
	VectorAdd( ent->client->ps.origin, ent->maxs, maxs );
	num = gi.EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );

	for (i=0 ; i<num ; i++) {
		hit = touch[i];
		if ( !hit->client ) {
			continue;
		}
		if ( hit == ent ) {
			continue;
		}
		if ( !(hit->contents & ent->contents) ) {
			continue;
		}

		// nail it
		G_Damage ( hit, ent, ent, NULL, NULL,
			100000, DAMAGE_NO_PROTECTION, MOD_TELEFRAG);
	}

}

//==============================================================================


/*
===============
G_AddEvent

Adds an event+parm and twiddles the event counter
===============
*/
void G_AddEvent( gentity_t *ent, int event, int eventParm ) {
	int		bits;

	if ( !event ) {
		gi.Printf( "G_AddEvent: zero event added for entity %i\n", ent->s.number );
		return;
	}

#if 0 // FIXME: allow multiple events on an entity
	// if the entity has an event that hasn't expired yet, don't overwrite
	// it unless it is identical (repeated footsteps / muzzleflashes / etc )
	if ( ent->s.event && ent->s.event != event ) {
		gentity_t	*temp;

		// generate a temp entity that references the original entity
		gi.Printf( "eventPush\n" );

		temp = G_Spawn();
		temp->s.eType = ET_EVENT_ONLY;
		temp->s.otherEntityNum = ent->s.number;
		G_SetOrigin( temp, ent->s.origin );
		G_AddEvent( temp, event, eventParm );
		temp->freeAfterEvent = qtrue;
		gi.linkentity( temp );
		return;
	}
#endif

	// clients need to add the event in playerState_t instead of entityState_t
	if ( !ent->s.number ) //only one client
	{
#if 0
		bits = ent->client->ps.externalEvent & EV_EVENT_BITS;
		bits = ( bits + EV_EVENT_BIT1 ) & EV_EVENT_BITS;
		ent->client->ps.externalEvent = event | bits;
		ent->client->ps.externalEventParm = eventParm;
		ent->client->ps.externalEventTime = level.time;
#endif
		AddEventToPlayerstate( event, eventParm, &ent->client->ps );
	} else {
		bits = ent->s.event & EV_EVENT_BITS;
		bits = ( bits + EV_EVENT_BIT1 ) & EV_EVENT_BITS;
		ent->s.event = event | bits;
		ent->s.eventParm = eventParm;
	}
	ent->eventTime = level.time;
}


/*
=============
G_Sound
=============
*/
void G_Sound( gentity_t *ent, int soundIndex ) 
{
	gentity_t	*te;

	te = G_TempEntity( ent->currentOrigin, EV_GENERAL_SOUND );
	te->s.eventParm = soundIndex;
}

/*
=============
G_SoundBroadcast

  Plays sound that can permeate PVS blockage
=============
*/
void G_SoundBroadcast( gentity_t *ent, int soundIndex ) 
{
	gentity_t	*te;

	te = G_TempEntity( ent->currentOrigin, EV_GLOBAL_SOUND );	//full volume
	te->s.eventParm = soundIndex;
	te->svFlags |= SVF_BROADCAST;
}

//==============================================================================


/*
================
G_SetOrigin

Sets the pos trajectory for a fixed position
================
*/
void G_SetOrigin( gentity_t *ent, vec3_t origin ) 
{
	VectorCopy( origin, ent->s.pos.trBase );
	if(ent->client)
	{
		VectorCopy( origin, ent->client->ps.origin );
		VectorCopy( origin, ent->s.origin );
	}
	else
	{
		ent->s.pos.trType = TR_STATIONARY;
	}
	ent->s.pos.trTime = 0;
	ent->s.pos.trDuration = 0;
	VectorClear( ent->s.pos.trDelta );

	VectorCopy( origin, ent->currentOrigin );
}

//===============================================================================
qboolean G_CheckInSolid (gentity_t *self, qboolean fix)
{
	trace_t	trace;
	vec3_t	end, mins;

	VectorCopy(self->currentOrigin, end);
	end[2] += self->mins[2];
	VectorCopy(self->mins, mins);
	mins[2] = 0;

	gi.trace(&trace, self->currentOrigin, mins, self->maxs, end, self->s.number, self->clipmask);
	if(trace.allsolid || trace.startsolid)
	{
		return qtrue;
	}
	
	if(trace.fraction < 1.0)
	{
		if(fix)
		{//Put them at end of trace and check again
			vec3_t	neworg;

			VectorCopy(trace.endpos, neworg);
			neworg[2] -= self->mins[2];
			G_SetOrigin(self, neworg);
			gi.linkentity(self);

			return G_CheckInSolid(self, qfalse);
		}
		else
		{
			return qtrue;
		}
	}
		
	return qfalse;
}

qboolean infront(gentity_t *from, gentity_t *to)
{
	vec3_t	angles, dir, forward;
	float	dot;

	angles[PITCH] = angles[ROLL] = 0;
	angles[YAW] = from->s.angles[YAW];
	AngleVectors(angles, forward, NULL, NULL);

	VectorSubtract(to->s.origin, from->s.origin, dir);
	VectorNormalize(dir);

	dot = DotProduct(forward, dir);
	if(dot < 0.0f)
	{
		return qfalse;
	}

	return qtrue;
}

void Svcmd_Use_f( void )
{
	char	*cmd1 = gi.argv(1);

	if ( !cmd1 || !cmd1[0] )
	{
		//FIXME: warning message
		gi.Printf( "'use' takes targetname of ent or 'list' (lists all usable ents)\n" );
		return;
	}
	else if ( !Q_stricmp("list", cmd1) )
	{
		gentity_t	*ent;

		gi.Printf("Listing all usable entities:\n");

		for ( int i = 1; i < ENTITYNUM_WORLD; i++ )
		{
			 ent = &g_entities[i];
			 if ( ent )
			 {
				 if ( ent->targetname && ent->targetname[0] )
				 {
					 if ( ent->e_UseFunc != useF_NULL )
					 {
						 if ( ent->NPC )
						 {
							gi.Printf( "%s (NPC)\n", ent->targetname );
						 }
						 else
						 {
							gi.Printf( "%s\n", ent->targetname );
						 }
					 }
				 }
			 }
		}

		gi.Printf("End of list.\n");
	}
	else
	{
		G_UseTargets2( &g_entities[0], &g_entities[0], cmd1 );
	}
}

//======================================================

void G_SetActiveState(char *targetstring, qboolean actState)
{
	gentity_t	*target = NULL;
	while( NULL != (target = G_Find(target, FOFS(targetname), targetstring)) )
	{
		target->svFlags = actState ? (target->svFlags&~SVF_INACTIVE) : (target->svFlags|SVF_INACTIVE);
	}
}

void target_activate_use(gentity_t *self, gentity_t *other, gentity_t *activator)
{

	if (self->behaviorSet[BSET_USE])
	{
		G_ActivateBehavior(self,BSET_USE);
	}

	G_SetActiveState(self->target, ACT_ACTIVE);
}

void target_deactivate_use(gentity_t *self, gentity_t *other, gentity_t *activator)
{
	if (self->behaviorSet[BSET_USE])
	{
		G_ActivateBehavior(self,BSET_USE);
	}

	G_SetActiveState(self->target, ACT_INACTIVE);
}

//FIXME: make these apply to doors, etc too?
/*QUAKED target_activate (1 0 0) (-4 -4 -4) (4 4 4)
Will set the target(s) to be usable/triggerable
*/
void SP_target_activate( gentity_t *self )
{
	G_SetOrigin( self, self->s.origin );
	self->e_UseFunc = useF_target_activate_use;
}

/*QUAKED target_deactivate (1 0 0) (-4 -4 -4) (4 4 4)
Will set the target(s) to be non-usable/triggerable
*/
void SP_target_deactivate( gentity_t *self )
{
	G_SetOrigin( self, self->s.origin );
	self->e_UseFunc = useF_target_deactivate_use;
}


//======================================================

/*
==============
ValidUseTarget

Returns whether or not the targeted entity is useable
==============
*/
qboolean ValidUseTarget( gentity_t *ent )
{
	if ( ent->e_UseFunc == useF_NULL )
	{
		return qfalse;
	}

	if ( ent->svFlags & SVF_INACTIVE )
	{//set by target_deactivate
		return qfalse;
	}
	
	if ( !(ent->svFlags & SVF_PLAYER_USABLE) )
	{//Check for flag that denotes BUTTON_USE useability
		return qfalse;
	}
	
	//FIXME: This is only a temp fix..
	if ( !strncmp( ent->classname, "trigger", 7) )
	{
		return qfalse;
	}

	return qtrue;
}

/*
==============
TryUse

Try and use an entity in the world, directly ahead of us
==============
*/

#define USE_DISTANCE	64.0f

void TryUse( gentity_t *ent )
{
	gentity_t	*target;
	trace_t		trace;
	vec3_t		src, dest, vf;

	//cg.refdef.vieworg, basically
	VectorCopy( ent->client->renderInfo.eyePoint, src );
	//cg.refdef.viewangles, basically
	AngleVectors( ent->client->renderInfo.eyeAngles, vf, NULL, NULL );
	//extend to find end of use trace
	VectorMA( src, USE_DISTANCE, vf, dest );

	//Trace ahead to find a valid target
	gi.trace( &trace, src, vec3_origin, vec3_origin, dest, ent->s.number, MASK_OPAQUE|CONTENTS_BODY|CONTENTS_ITEM|CONTENTS_CORPSE );
	
	if ( trace.fraction == 1.0f || trace.entityNum < 1 )
	{
		//TODO: Play a failure sound
		return;
	}

	target = &g_entities[trace.entityNum];

	//Check for a use command
	if ( ValidUseTarget( target ) )
	{
		GEntity_UseFunc( target, ent, ent );
	}
	else if ( target->svFlags & SVF_STASIS_DOOR && target->trigger_formation )
	{
		// If the door is a stasis door and it's locked....and finally, if the player can't use it...well..then play the NEG sound
		G_AddEvent( ent, EV_GENERAL_SOUND, G_SoundIndex( "sound/movers/switches/stasisneg.wav" ));
	}
	else if ( target->client && target->client->ps.pm_type < PM_DEAD && target->NPC!=NULL && target->client->playerTeam && target->client->playerTeam == ent->client->playerTeam && !(target->NPC->scriptFlags&SCF_NO_RESPONSE) )
	{
		NPC_UseResponse ( target, ent, qfalse );
	}
}

void G_ChangeMap (const char *mapname, const char *spawntarget, qboolean hub)
{
//	gi.Printf("Loading...");
	//ignore if player is dead
	if (g_entities[0].client->ps.pm_type == PM_DEAD)
		return;

	// Record the player loadout into the engine carry-cvars BEFORE we leave this level, so the next level's
	// Player_RestoreFromPrevLevel carries weapons/ammo/health/etc across the transition. (Retail did this in
	// the engine; this port's bridge spawns the new map fresh + carries the player via these cvars.)
	extern void Player_CacheForNextLevel(void);
	Player_CacheForNextLevel();

	if ( spawntarget == NULL ) {
		spawntarget = "";	//prevent it from becoming "(null)"
	}
	if ( ffireLevel >= FFIRE_LEVEL_RETALIATION )
	{
		gi.SendConsoleCommand( "maptransition _brig\n" );
	}
	else if ( hub == qtrue )
	{
		gi.SendConsoleCommand( va("loadtransition %s %s\n", mapname, spawntarget) );
	}
	else
	{
		gi.SendConsoleCommand( va("maptransition %s %s\n", mapname, spawntarget) );
	}
}

qboolean G_PointInBounds( vec3_t point, vec3_t mins, vec3_t maxs )
{
	for(int i = 0; i < 3; i++ )
	{
		if ( point[i] < mins[i] )
		{
			return qfalse;
		}
		if ( point[i] > maxs[i] )
		{
			return qfalse;
		}
	}

	return qtrue;
}

qboolean G_BoxInBounds( vec3_t point, vec3_t mins, vec3_t maxs, vec3_t boundsMins, vec3_t boundsMaxs )
{
	vec3_t boxMins;
	vec3_t boxMaxs;

	VectorAdd( point, mins, boxMins );
	VectorAdd( point, maxs, boxMaxs );

	if(boxMaxs[0]>boundsMaxs[0])
		return qfalse;

	if(boxMaxs[1]>boundsMaxs[1])
		return qfalse;

	if(boxMaxs[2]>boundsMaxs[2])
		return qfalse;

	if(boxMins[0]<boundsMins[0])
		return qfalse;

	if(boxMins[1]<boundsMins[1])
		return qfalse;

	if(boxMins[2]<boundsMins[2])
		return qfalse;

	//box is completely contained within bounds
	return qtrue;
}


void G_SetAngles( gentity_t *ent, vec3_t angles )
{
	VectorCopy( angles, ent->currentAngles );
	VectorCopy( angles, ent->s.angles );
	VectorCopy( angles, ent->s.apos.trBase );
}

qboolean G_ClearTrace( vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, int ignore, int clipmask )
{
	static	trace_t	tr;

	gi.trace( &tr, start, mins, maxs, end, ignore, clipmask );

	if ( tr.allsolid || tr.startsolid || tr.fraction < 1.0 )
	{
		return qfalse;
	}

	return qtrue;
}

/*
G_LanguageFilename - create a filename with an extension based on the value in g_language
*/
void G_LanguageFilename(char *baseName,char *baseExtension,char *finalName)
{
	char	language[32];
	fileHandle_t	file;
	
	Q_strncpyz(language,g_language?g_language->string:"",sizeof(language));

	// If it's English then no extension
	if (language[0]=='\0' || Q_stricmp("ENGLISH",language)==0)
	{
		Com_sprintf(finalName,MAX_QPATH,"%s.%s",baseName,baseExtension);
	}
	else
	{
		Com_sprintf(finalName,MAX_QPATH,"%s_%s.%s",baseName,language,baseExtension);

		//Attempt to load the file
		gi.FS_FOpenFile( finalName, &file, FS_READ );

		if ( file == 0 )	//	This extension doesn't exist, go English.
		{
			Com_sprintf(finalName,MAX_QPATH,"%s.%s",baseName,baseExtension);
		}
		else
		{
			gi.FS_FCloseFile( file );
		}
	}
}
