#include "g_local.h"
#include "g_functions.h"

extern void G_MoverTouchTeleportTriggers( gentity_t *ent, vec3_t oldOrg );
void G_StopObjectMoving( gentity_t *object );
/*
================
G_RunObject

================
*/
void G_RunObject( gentity_t *ent ) 
{
	vec3_t		origin, oldOrg;
	trace_t		tr;

	ent->nextthink = level.time + FRAMETIME;

	VectorCopy( ent->currentOrigin, oldOrg );
	// get current position
	EvaluateTrajectory( &ent->s.pos, level.time, origin );

	//FIXME: Get current angles?

	if ( VectorCompare( ent->currentOrigin, origin ) )
	{//error - didn't move at all!
		return;
	}
	// trace a line from the previous position to the current position,
	// ignoring interactions with the missile owner
	gi.trace( &tr, ent->currentOrigin, ent->mins, ent->maxs, origin, 
		ent->owner ? ent->owner->s.number : ent->s.number, ent->clipmask );

	if ( !tr.startsolid && !tr.allsolid && tr.fraction ) 
	{
		VectorCopy( tr.endpos, ent->currentOrigin );
		gi.linkentity( ent );
	}
	else
	//if ( tr.startsolid ) 
	{
		tr.fraction = 0;
	}

	if ( !(ent->s.eFlags & EF_TELEPORT_BIT) && !(ent->svFlags & SVF_NO_TELEPORT) )
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

	if ( ent->s.pos.trType == TR_GRAVITY )//tr.fraction < 1.0 && 
	{//FIXME: only do this if no trDelta
		//okay, we hit the floor, might as well stop or prediction will
		//make us go through the floor!
		//FIXME: this means we can't fall if something is pulled out from under us...
		G_StopObjectMoving( ent );
	}

	GEntity_TouchFunc( ent, &g_entities[tr.entityNum], &tr );
}

void G_StopObjectMoving( gentity_t *object )
{
	object->s.pos.trType = TR_STATIONARY;
	VectorCopy( object->currentOrigin, object->s.origin );
	VectorCopy( object->currentOrigin, object->s.pos.trBase );
	VectorClear( object->s.pos.trDelta );

	/*
	//Stop spinning
	VectorClear( self->s.apos.trDelta );
	vectoangles(trace->plane.normal, self->s.angles);
	VectorCopy(self->s.angles, self->currentAngles );
	VectorCopy(self->s.angles, self->s.apos.trBase);
	*/
}

void G_StartObjectMoving( gentity_t *object, vec3_t dir, float speed, trType_t trType )
{
	VectorNormalize (dir);

	//object->s.eType = ET_GENERAL;
	object->s.pos.trType = trType;
	VectorCopy( object->currentOrigin, object->s.pos.trBase );
	VectorScale(dir, speed, object->s.pos.trDelta );
	object->s.pos.trTime = level.time;

	/*
	//FIXME: incorporate spin?
	vectoangles(dir, object->s.angles);
	VectorCopy(object->s.angles, object->s.apos.trBase);
	VectorSet(object->s.apos.trDelta, 300, 0, 0 );
	object->s.apos.trTime = level.time;
	*/

	//FIXME: make these objects go through G_RunObject automatically, like missiles do
	if ( object->e_ThinkFunc == thinkF_NULL )
	{
		object->nextthink = level.time + FRAMETIME;
		object->e_ThinkFunc = thinkF_G_RunObject;
	}
	else
	{//You're responsible for calling RunObject
	}
}

gentity_t *G_CreateObject ( gentity_t *owner, vec3_t origin, vec3_t angles, int modelIndex, int frame, trType_t trType ) 
{
	gentity_t	*object;

	object = G_Spawn();

	if ( object == NULL )
	{
		return NULL;
	}

	object->classname = "object";//?
	object->nextthink = level.time + FRAMETIME;
	object->e_ThinkFunc = thinkF_G_RunObject;
	object->s.eType = ET_GENERAL;
	object->s.eFlags |= EF_AUTO_SIZE;//CG_Ents will create the mins & max itself based on model bounds
	object->s.modelindex = modelIndex;
	//FIXME: allow to set a targetname/script_targetname and animation info?
	object->s.frame = object->startFrame = object->endFrame = frame;
	object->owner = owner;
	//object->damage = 100;
	//object->splashDamage = 200;
	//object->splashRadius = 200;
	//object->methodOfDeath = MOD_GRENADE;
	//object->splashMethodOfDeath = MOD_GRENADE_SPLASH;
	object->clipmask = MASK_SOLID;//?
	//object->e_TouchFunc = touchF_charge_stick;

	//Give it SOME size for now
	VectorSet( object->mins, -4, -4, -4 );
	VectorSet( object->maxs, 4, 4, 4 );

	//Origin
	G_SetOrigin( object, origin );
	object->s.pos.trType = trType;
	VectorCopy( origin, object->s.pos.trBase );
	//Velocity
	VectorClear( object->s.pos.trDelta );
	object->s.pos.trTime = level.time;
	//VectorScale( dir, 300, object->s.pos.trDelta );
	//object->s.pos.trTime = level.time;

	//Angles
	VectorCopy( angles, object->s.angles );
	VectorCopy( object->s.angles, object->s.apos.trBase );
	//Angular Velocity
	VectorClear( object->s.apos.trDelta );
	object->s.apos.trTime = level.time;
	//VectorSet( object->s.apos.trDelta, 300, 0, 0 );
	//object->s.apos.trTime = level.time;

	gi.linkentity( object );

	return object;
}
