#include "g_local.h"
#include "g_functions.h"

extern void	InitMover( gentity_t *ent );

extern gentity_t	*G_TestEntityPosition( gentity_t *ent );
void func_wait_return_solid( gentity_t *self )
{
	//once a frame, see if it's clear.
	self->clipmask = CONTENTS_BODY;
	if ( !(self->spawnflags&16) || G_TestEntityPosition( self ) == NULL )
	{
		gi.SetBrushModel( self, self->model );
		InitMover( self );
		VectorCopy( self->s.origin, self->s.pos.trBase );
		VectorCopy( self->s.origin, self->currentOrigin );
		self->svFlags &= ~SVF_NOCLIENT;
		self->s.eFlags &= ~EF_NODRAW;
		self->e_UseFunc = useF_func_usable_use;
		self->clipmask = 0;
		if ( self->s.eFlags & EF_ANIM_ONCE )
		{//Start our anim
			self->s.frame = 0;
		}
		if ( !(self->spawnflags&1) )
		{//START_OFF doesn't effect area portals
			gi.AdjustAreaPortalState( self, qfalse );
		}
	}
	else
	{
		self->clipmask = 0;
		self->e_ThinkFunc = thinkF_func_wait_return_solid;
		self->nextthink = level.time + FRAMETIME;
	}
}

void func_usable_think( gentity_t *self )
{
	if ( self->spawnflags & 8 )
	{
		self->svFlags |= SVF_PLAYER_USABLE;	//Replace the usable flag
		self->e_UseFunc = useF_func_usable_use;
		self->e_ThinkFunc = thinkF_NULL;
	}
}

void func_usable_use (gentity_t *self, gentity_t *other, gentity_t *activator)
{//Toggle on and off
	G_ActivateBehavior( self, BSET_USE );
	if ( self->spawnflags & 8 )		// fixme: uurrgh!!!!  that's disgusting
	{//ALWAYS_ON
		//Remove the ability to use the entity directly
		self->svFlags &= ~SVF_PLAYER_USABLE;
		//also remove ability to call any use func at all!
		self->e_UseFunc = useF_NULL;
		
		if(self->target && self->target[0])
		{
			G_UseTargets(self, activator);
		}
		
		if ( self->wait )
		{
			self->e_ThinkFunc = thinkF_func_usable_think;
			self->nextthink = level.time + ( self->wait * 1000 );
		}

		return;
	}
	else if ( !self->count )
	{//become solid again
		self->count = 1;
		func_wait_return_solid( self );
	}
	else
	{
		self->s.solid = 0;
		self->contents = 0;
		self->clipmask = 0;
		self->svFlags |= SVF_NOCLIENT;
		self->s.eFlags |= EF_NODRAW;
		self->count = 0;

		if(self->target && self->target[0])
		{
			G_UseTargets(self, activator);
		}
		self->e_ThinkFunc = thinkF_NULL;
		self->nextthink = -1;
		if ( !(self->spawnflags&1) )
		{//START_OFF doesn't effect area portals
			gi.AdjustAreaPortalState( self, qtrue );
		}
	}
}

void func_usable_pain(gentity_t *self, gentity_t *attacker, int damage)
{
	GEntity_UseFunc( self, attacker, attacker );
}

void func_usable_die(gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod)
{
	self->takedamage = qfalse;
	GEntity_UseFunc( self, inflictor, attacker );
}

/*QUAKED func_usable (0 .5 .8) ? STARTOFF AUTOANIMATE ANIM_ONCE ALWAYS_ON BLOCKCHECK x x PLAYER_USE
START_OFF - the wall will not be there
AUTOANIMATE - if a model is used it will animate				
ANIM_ONCE - When turned on, goes through anim once
ALWAYS_ON - Doesn't toggle on and off when used, just runs usescript and fires target
PLAYER_USE - Player can use it with the use button
BLOCKCHECK - Will not turn on while something is inside it

A bmodel that just sits there, doing nothing.  Can be used for conditional walls and models.
"targetname" - When used, will toggle on and off
"target"	Will fire this target every time it is toggled OFF
"model2"	.md3 model to also draw
"color"		constantLight color
"light"		constantLight radius
"usescript" script to run when turned on
"deathscript"  script to run when turned off
"wait"		amount of time before the object is usable again (only valid with ALWAYS_ON flag)
"health"	if it has health, it will be used whenever shot at/killed - if you want it to only be used once this way, set health to 1
*/

void SP_func_usable( gentity_t *self ) 
{
	gi.SetBrushModel( self, self->model );
	InitMover( self );
	VectorCopy( self->s.origin, self->s.pos.trBase );
	VectorCopy( self->s.origin, self->currentOrigin );

	self->count = 1;
	if (self->spawnflags & 1)
	{
		self->s.solid = 0;
		self->contents = 0;
		self->clipmask = 0;
		self->svFlags |= SVF_NOCLIENT;
		self->s.eFlags |= EF_NODRAW;
		self->count = 0;
	}

	if (self->spawnflags & 2)
	{
		self->s.eFlags |= EF_ANIM_ALLFAST;
	}

	if (self->spawnflags & 4)
	{//FIXME: need to be able to do change to something when it's done?  Or not be usable until it's done?
		self->s.eFlags |= EF_ANIM_ONCE;
	}

	self->e_UseFunc = useF_func_usable_use;

	if ( self->health )
	{
		self->takedamage = qtrue;
		self->e_DieFunc = dieF_func_usable_die;
		self->e_PainFunc = painF_func_usable_pain;
	}

	gi.linkentity (self);
}