#include "g_local.h"
#include "g_functions.h"

extern team_t TranslateTeamName( const char *name );
extern void G_SetAngles( gentity_t *ent, vec3_t angles );

//client side shortcut hacks from cg_local.h
extern void CG_SurfaceExplosion( vec3_t origin, vec3_t normal, float radius, float shake_speed, qboolean smoke );
extern void CG_MiscModelExplosion( vec3_t origin, vec3_t normal );
extern void CG_Chunks( int owner, vec3_t origin, const vec3_t normal, float speed, int numChunks, material_t chunkType, int customChunk, float baseScale );
extern void G_SetEnemy( gentity_t *self, gentity_t *enemy );
extern void SP_NPC_Reaver( gentity_t *self);

extern	qboolean	player_locked;

void funcBBrushDieGo (gentity_t *self)
{
	vec3_t		org, dir, up;
	gentity_t	*attacker = self->enemy;
	float		scale;
	int			numChunks;
	material_t	chunkType = self->material;
		
	//So chunks don't get stuck inside me
	self->s.solid = 0;
	self->contents = 0;
	self->clipmask = 0;
	gi.linkentity(self); 

	VectorSet(up, 0, 0, 1);

	if ( self->target && attacker != NULL )
	{
		G_UseTargets(self, attacker);
	}

	VectorSubtract(self->absmax, self->absmin, org);//size

	scale = VectorLength(org)/64.0f;
	numChunks = ceil(scale*2) + Q_irand(0, 3);
	// Cap the scale so it doesn't get too out of control
	if ( scale > 1.0f )
		scale = 1.0f;
	
	if ( scale < 0.2f )
		scale = 0.2f;

	VectorMA(self->absmin, 0.5, org, org);

	if ( attacker != NULL && attacker->client )
	{
		VectorSubtract( org, attacker->currentOrigin, dir );
		VectorNormalize( dir );
	}
	else
	{
		VectorCopy(up, dir);
	}

	if(self->splashDamage > 0 && self->splashRadius > 0)
	{
		G_RadiusDamage( org, self, self->splashDamage, self->splashRadius, self, MOD_UNKNOWN );

		CG_MiscModelExplosion( org, dir );
		G_Sound(self, G_SoundIndex("sound/weapons/explosions/cargoexplode.wav"));
	}

	//FIXME: base numChunks off size?
	CG_Chunks( self->s.number, org, dir, Q_flrand(400, 600), numChunks, chunkType, 0, scale );

	gi.AdjustAreaPortalState( self, qtrue );
	G_FreeEntity( self );
}

void funcBBrushDie (gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod)
{
	self->takedamage = qfalse;//stop chain reaction runaway loops

	G_SetEnemy(self, attacker);

	if(self->delay)
	{
		self->e_ThinkFunc = thinkF_funcBBrushDieGo;
		self->nextthink = level.time + floor(self->delay * 1000.0f);
		return;
	}
	
	funcBBrushDieGo(self);
}

void funcBBrushUse (gentity_t *self, gentity_t *other, gentity_t *activator)
{
	G_ActivateBehavior( self, BSET_USE );
	if(self->spawnflags & 64)
	{//Using it doesn't break it, makes it use it's targets
		if(self->target && self->target[0])
		{
			G_UseTargets(self, activator);
		}
	}
	else
	{
		funcBBrushDie(self, other, activator, self->health, MOD_UNKNOWN);
	}
}

void funcBBrushPain(gentity_t *self, gentity_t *attacker, int damage)
{
	if(self->painDebounceTime > level.time)
	{
		return;
	}

	if(self->paintarget)
	{
		G_UseTargets2 (self, self->activator, self->paintarget);
	}

	G_ActivateBehavior( self, BSET_PAIN );

	if(self->wait == -1)
	{
		self->e_PainFunc = painF_NULL;
		return;
	}

	self->painDebounceTime = level.time + self->wait;
}

static void InitBBrush ( gentity_t *ent ) 
{
	float		light;
	vec3_t		color;
	qboolean	lightSet, colorSet;

	VectorCopy( ent->s.origin, ent->pos1 );
	
	gi.SetBrushModel( ent, ent->model );

	ent->e_DieFunc = dieF_funcBBrushDie;
	
	ent->svFlags |= SVF_BBRUSH;

	// if the "model2" key is set, use a seperate model
	// for drawing, but clip against the brushes
	if ( ent->model2 ) 
	{
		ent->s.modelindex2 = G_ModelIndex( ent->model2 );
	}

	// if the "color" or "light" keys are set, setup constantLight
	lightSet = G_SpawnFloat( "light", "100", &light );
	colorSet = G_SpawnVector( "color", "1 1 1", color );
	if ( lightSet || colorSet ) 
	{
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

	if(ent->spawnflags & 128)
	{//Can be used by the player's BUTTON_USE
		ent->svFlags |= SVF_PLAYER_USABLE;
	}

	ent->s.eType = ET_MOVER;
	gi.linkentity (ent);

	ent->s.pos.trType = TR_STATIONARY;
	VectorCopy( ent->pos1, ent->s.pos.trBase );
}

/*QUAKED func_breakable (0 .8 .5) ? INVINCIBLE x x x x x USE_NOT_BREAK PLAYER_USE
PLAYER_USE - Player can use it with the use button
USE_NOT_BREAK - Using it, doesn't make it break, still can be destroyed by damage

When destroyed, fired it's trigger and chunks and plays sound "noise" or sound for type if no noise specified

This will change soon, so don't count on setting the material type this way just yet

INVINCIBLE - can only be broken by being used
"targetname" entities with matching target will fire it
"paintarget" target to fire when hit (but not destroyed)
"wait"		how long minimum to wait between firing paintarget each time hit
"delay"		When killed or used, how long (in seconds) to wait before blowing up (none by default)
"model2"	.md3 model to also draw
"target"	all entities with a matching targetname will be used when this is destoryed
"health"	default is 10

Damage: default is none
"splashDamage" - damage to do (will make it explode on death
"splashRadius" - radius for above damage

"team" - This cannot take damage from members of this team:
	"starfleet"
	"borg"
	"parasite"
	"scavengers"
	"klingon"
	"malon"
	"hirogen"
	"imperial"
	"stasis"
	"species8472"
	"dreadnought"
	"forge"

Don't know if these work:  
"color"		constantLight color
"light"		constantLight radius

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
void SP_func_breakable( gentity_t *self ) 
{
	if(!(self->spawnflags & 1))
	{
		if(!self->health)
		{
			self->health = 10;
		}
	}

	if (self->health) 
	{
		self->takedamage = qtrue;
	}

	G_SoundIndex("sound/weapons/explosions/cargoexplode.wav");//precaching
	self->e_UseFunc = useF_funcBBrushUse;

	if(self->paintarget)
	{
		self->e_PainFunc = painF_funcBBrushPain;
	}

	if ( self->team && self->team[0] )
	{
		self->noDamageTeam = TranslateTeamName( self->team );
		if(self->noDamageTeam == TEAM_FREE)
		{
			G_Error("team name %s not recognized\n", self->team);
		}
	}
	self->team = NULL;
	if (!self->model) {
		G_Error("func_breakable with NULL model\n");
	}
	InitBBrush( self );
}

void Touch_AssimilationTrigger( gentity_t *ent, gentity_t *other, trace_t *trace ) 
{
	if ( !other )
	{
		return;
	}

	if( !other->client ) 
	{
		return;
	}

	if ( other->s.number != 0 )
	{
		return;
	}

	/*
	if ( ent->svFlags & SVF_INACTIVE )
	{//set by the model being broken?
		return;
	}
	*/

	if ( ent->owner->health <= 0 )
	{
		return;
	}

	other->s.eFlags |= EF_ASSIMILATED;
	player_locked = qtrue;
	ent->owner->enemy = other;

	if ( ent->owner->s.eType != ET_THINKER )
	{
		ent->owner->e_clThinkFunc = clThinkF_CG_Assimilator;
		ent->owner->s.eType = ET_THINKER;
		
		//assim sound
		G_Sound( ent->owner, G_SoundIndex( "sound/enemies/borg/borgass.wav" ) );
		G_Sound( ent->owner, G_SoundIndex( "sound/enemies/borg/alcovein.wav" ) );
	}
	//assim loop sound
	G_Sound( ent->owner, G_SoundIndex( "sound/enemies/borg/borgassloop.wav" ) );
	if ( !Q_irand(0, 5) )
	{
		G_Damage( other, ent->owner, ent->owner, NULL, NULL, 1, DAMAGE_NO_KNOCKBACK|DAMAGE_NO_ARMOR, MOD_ASSIMILATE );
	}
}

void SpawnAssimilationTrigger( gentity_t *ent ) 
{
	gentity_t		*other;
	vec3_t		mins, maxs;

	// find the bounds of everything on the team
	VectorCopy (ent->absmin, mins);
	VectorCopy (ent->absmax, maxs);
	mins[2] -= 64;

	// create a trigger with this size
	other = G_Spawn ();
	VectorCopy (mins, other->mins);
	VectorCopy (maxs, other->maxs);
	other->owner = ent;
	other->contents = CONTENTS_TRIGGER;
	other->e_TouchFunc = touchF_Touch_AssimilationTrigger;
	gi.linkentity( other );
}

#define CHUNK_SCALE_FACTOR	0.3f	// This was arrived at by trial and error
void misc_model_breakable_pain ( gentity_t *self, gentity_t *other, int damage )
{
	vec3_t	dir, dis;
	int		numChunks;
	float	damg = 0.25, //damg is a percentage of max_health
			scale;
	//FIXME: debounce time?

	//Throw off some chunks
	AngleVectors( self->s.apos.trBase, dir, NULL, NULL );
	VectorNormalize( dir );

	if( self->max_health )
	{
		damg = (float)damage * 0.15 / (float)self->max_health;
	}

	numChunks = random() * 8 + 8;

	// Scale the chunks, a function of damage done, size of object, num of chunks, etc.
	VectorSubtract( self->mins, self->maxs, dis );
	scale = ( VectorNormalize( dis ) * damg ) / numChunks * CHUNK_SCALE_FACTOR;

	if ( scale > 0.4f ) {
		scale = 0.4f;
	}
	if ( scale < 0.1f ) {
		scale = 0.1f;
	}

	if(self->health <= 0)
	{//dead, don't react to pain
		CG_Chunks( self->s.number, self->currentOrigin, dir, 300, numChunks, self->material, self->s.modelindex3, scale );
	}
	else
	{//still alive, react to the pain
		// Glass pain shouldn't throw chunks...
		if ( self->material != MAT_GLASS && self->material != MAT_GLASS_METAL )
			CG_Chunks( self->s.number, self->currentOrigin, dir, 300, numChunks, self->material, 0, scale );

		if ( self->paintarget )
		{
			G_UseTargets2 (self, self->activator, self->paintarget);
		}

		//Don't do script if dead
		G_ActivateBehavior( self, BSET_PAIN );
	}
}

void misc_model_breakable_die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath ) 
{
	vec3_t		dir, up;

	//NOTE: Stop any scripts that are currently running (FLUSH)... ?
	//Turn off animation
	self->s.frame = self->startFrame = self->endFrame = 0;
	self->svFlags &= ~SVF_ANIMATING;
			
	self->health = 0;
	//Throw some chunks
	misc_model_breakable_pain ( self, inflictor, damage );

	self->e_PainFunc = painF_NULL;
	self->e_DieFunc  = dieF_NULL;
//	self->e_UseFunc  = useF_NULL;

	self->takedamage = qfalse;

	if ( !(self->spawnflags & 4) )
	{//We don't want to stay solid
		self->s.solid = 0;
		self->contents = 0;
		self->clipmask = 0;
		gi.linkentity(self);
	}

	VectorSet(up, 0, 0, 1);

	if(self->target)
	{
		G_UseTargets(self, attacker);
	}

	//VectorSubtract(self->absmax, self->absmin, org);//size
	//scale = VectorLength(org)/64.0f;
	//VectorMA(self->absmin, 0.5, org, org);

	if(inflictor->client)
	{
		VectorSubtract( self->currentOrigin, inflictor->currentOrigin, dir );
		VectorNormalize( dir );
	}
	else
	{
		VectorCopy(up, dir);
	}

	if ( !(self->spawnflags & 2048) ) // NO_EXPLOSION
	{
		// Ok, we are allowed to explode, so do it now!
		if(self->splashDamage > 0 && self->splashRadius > 0)
		{
			//FIXME: specify type of explosion?  (barrel, electrical, etc.)  Also, maybe just use the explosion effect below since it's
			//				a bit better?
			G_RadiusDamage( self->currentOrigin, self, self->splashDamage, self->splashRadius, self, MOD_UNKNOWN );

			CG_SurfaceExplosion(self->currentOrigin, up, 20.0f, 12.0f, ((self->spawnflags&16)==qfalse) );
			G_Sound(self, G_SoundIndex("sound/weapons/explosions/cargoexplode.wav"));
		}
		else
		{
			// This is the default explosion
			CG_MiscModelExplosion( self->currentOrigin, dir );
			G_Sound(self, G_SoundIndex("sound/weapons/explosions/cargoexplode.wav"));
		}
	}

	if(self->s.modelindex2 != -1 && !(self->spawnflags & 8))
	{//FIXME: modelindex doesn't get set to -1 if the damage model doesn't exist
		self->svFlags |= SVF_BROKEN;
		self->s.modelindex = self->s.modelindex2;
		if ( self->behaviorSet[BSET_DEATH] )
		{
			G_ActivateBehavior( self, BSET_DEATH );
		}
	}
	else
	{
		G_FreeEntity( self );
	}
}

//------------------------------------------------------------
void reaver_vat_use( gentity_t *self, gentity_t *other, gentity_t *activator )
{
	self->s.eFlags &= ~EF_NODRAW;
	self->contents = CONTENTS_SOLID;
	gi.linkentity( self );
}

//------------------------------------------------------------
void reaver_vat_die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod )
{
	vec3_t dir;

	//Turn off animation
	self->s.frame = self->startFrame = self->endFrame = 0;
	self->svFlags &= ~SVF_ANIMATING;
			
	self->health = 0;
	self->takedamage = qfalse;
	self->contents = 0;

	//Throw some chunks
	misc_model_breakable_pain ( self, inflictor, damage );

	self->e_PainFunc = painF_NULL;
	self->e_DieFunc  = dieF_NULL;

	if ( self->target )
		G_UseTargets( self, attacker );

	self->svFlags |= SVF_BROKEN;
	self->s.modelindex = self->s.modelindex2;

	if ( self->behaviorSet[BSET_DEATH] )
		G_ActivateBehavior( self, BSET_DEATH );

	// Add in the main effects
	if ( inflictor->client )
	{
		VectorSubtract( self->currentOrigin, inflictor->currentOrigin, dir );
		VectorNormalize( dir );
	}
	else
	{
		VectorSet( dir, 0, 0, 1 ); // up
	}

	CG_Chunks( self->s.number, self->currentOrigin, dir, 400, 8, MAT_GLASS, 0, 1.0f );
	CG_MiscModelExplosion( self->currentOrigin, dir );

	G_AddEvent( self, EV_FX_MAGIC_SMOKE, 0 );

	gentity_t *ent = G_Spawn();

	VectorCopy( self->currentOrigin, ent->s.origin );
	VectorCopy( self->s.angles, ent->s.angles );
	ent->spawnflags |= 32;
	ent->enemy = attacker;
	ent->behaviorSet[BSET_SPAWN] = "BS_RUN_AND_SHOOT";
	ent->NPC_targetname = self->NPC_targetname;

	SP_NPC_Reaver( ent );

}

void misc_model_use (gentity_t *self, gentity_t *other, gentity_t *activator)
{
	if ( self->health <= 0 && self->max_health > 0)
	{//used while broken fired target3
		G_UseTargets2( self, activator, self->target3 );
		return;
	}

	G_ActivateBehavior( self, BSET_USE );
	//Don't explode if they've requested it to not
	if ( self->spawnflags & 64 )
	{//Usemodels toggling
		if ( self->spawnflags & 32 )
		{
			if( self->s.modelindex == self->sound1to2 )
			{
				self->s.modelindex = self->sound2to1;
			}
			else
			{
				self->s.modelindex = self->sound1to2;
			}
		}

		return;
	}

	misc_model_breakable_die( self, other, activator, self->health, MOD_UNKNOWN );
}

#define MDL_OTHER			0
#define MDL_ARMOR_HEALTH	1
#define MDL_AMMO			2

void misc_model_breakable_init( gentity_t *ent )
{
	int		type;

	// FIXME : these should be using the proper spawn functions
	if (!Q_stricmp(ent->model,"models/mapobjects/stasis/plugin2.md3"))	
	{
		type = MDL_ARMOR_HEALTH;
	}
	else if (!Q_stricmp(ent->model,"models/mapobjects/stasis/plugin.md3"))	
	{
		type = MDL_AMMO;
	}
	else if (!Q_stricmp(ent->model,"models/mapobjects/borg/plugin2.md3"))	
	{
		type = MDL_ARMOR_HEALTH;
	}
	else if (!Q_stricmp(ent->model,"models/mapobjects/borg/plugin.md3"))	
	{
		type = MDL_AMMO;
	}
	else
	{
		type = MDL_OTHER;
	}

	//Main model
	ent->s.modelindex = ent->sound2to1 = G_ModelIndex( ent->model );

	if ( ent->spawnflags & 1 )
	{//Blocks movement
		ent->contents = CONTENTS_BODY;//Was CONTENTS_SOLID, but only architecture should be this
	}
	else if ( ent->health )
	{//Can only be shot
		ent->contents = CONTENTS_SHOTCLIP;
	}

	if (type == MDL_OTHER)
	{
		ent->e_UseFunc = useF_misc_model_use;	
	}
	else if ( type == MDL_ARMOR_HEALTH )
	{
		G_SoundIndex("sound/player/suithealth.wav");
		ent->e_UseFunc = useF_health_use;
		if (!ent->count)
		{
			ent->count = 100;
		}
		ent->health = 60;
	}
	else if ( type == MDL_AMMO )
	{
		G_SoundIndex("sound/player/suitenergy.wav");
		ent->e_UseFunc = useF_ammo_use;
		if (!ent->count)
		{
			ent->count = 100;
		}
		ent->health = 60;
	}

	if ( ent->health ) 
	{
		G_SoundIndex("sound/weapons/explosions/cargoexplode.wav");
		ent->max_health = ent->health;
		ent->takedamage = qtrue;
		ent->e_PainFunc = painF_misc_model_breakable_pain;
		ent->e_DieFunc  = dieF_misc_model_breakable_die;
	}
}

/*QUAKED misc_model_breakable (1 0 0) (-16 -16 -16) (16 16 16) SOLID AUTOANIMATE DEADSOLID NO_DMODEL NO_SMOKE USE_MODEL USE_NOT_BREAK PLAYER_USE NO_EXPLOSION
SOLID - Movement is blocked by it, if not set, can still be broken by explosions and shots if it has health
AUTOANIMATE - Will cycle it's anim
DEADSOLID - Stay solid even when destroyed (in case damage model is rather large).
NO_DMODEL - Makes it NOT display a damage model when destroyed, even if one exists
USE_MODEL - When used, will toggle to it's usemodel (model name + "_u1.md3")... this obviously does nothing if USE_NOT_BREAK is not checked
USE_NOT_BREAK - Using it, doesn't make it break, still can be destroyed by damage
PLAYER_USE - Player can use it with the use button
NO_EXPLOSION - By default, will explode when it dies...this is your override.

"model"		arbitrary .md3 file to display
"health"	how much health to have - default is zero (not breakable)  If you don't set the SOLID flag, but give it health, it can be shot but will not block NPCs or players from moving
"targetname" when used, dies and displays damagemodel, if any (if not, removes itself)
"target" What to use when it dies
"target2" What to use when it's repaired
"target3" What to use when it's used while it's broken
"paintarget" target to fire when hit (but not destroyed)
"count"  the amount of armor/health/ammo given (default 50)

Damage: default is none
"splashDamage" - damage to do (will make it explode on death)
"splashRadius" - radius for above damage

"team" - This cannot take damage from members of this team:
	"starfleet"
	"borg"
	"parasite"
	"scavengers"
	"klingon"
	"malon"
	"hirogen"
	"imperial"
	"stasis"
	"species8472"
	"dreadnought"
	"forge"

"material" - default is "metal" - choose from this list:
0 = MAT_METAL		(default)
1 = MAT_GLASS
2 = MAT_ELECTRICAL	(sparks)
3 = MAT_ORGANIC		(not implemented)
4 =	MAT_BORG		(borg chunks)
5 =	MAT_STASIS		(stasis chunks)
6 =	MAT_GLASS_METAL (mixed chunk type)



FIXME/TODO: 
set size better?
multiple damage models?
multiple chunk models?
don't throw chunks on pain, or throw level 1 chunks only on pains?
custom explosion effect/sound?
*/
void SP_misc_model_breakable( gentity_t *ent ) 
{
	char	damageModel[MAX_QPATH];
	char	chunkModel[MAX_QPATH];
	char	useModel[MAX_QPATH];
	int		len;
	
	misc_model_breakable_init( ent );

	len = strlen( ent->model ) - 4;
	strncpy( damageModel, ent->model, len );
	damageModel[len] = 0;	//chop extension
	strncpy( chunkModel, damageModel, sizeof(chunkModel));
	strncpy( useModel, damageModel, sizeof(useModel));
	
	if (ent->takedamage) {
		//Dead/damaged model
		if( !(ent->spawnflags & 8) ) {	//no dmodel
			strcat( damageModel, "_d1.md3" );
			ent->s.modelindex2 = G_ModelIndex( damageModel );
		}
		
		//Chunk model
		strcat( chunkModel, "_c1.md3" );
		ent->s.modelindex3 = G_ModelIndex( chunkModel );
	}

	//Use model
	if( ent->spawnflags & 32 ) {	//has umodel
		strcat( useModel, "_u1.md3" );
		ent->sound1to2 = G_ModelIndex( useModel );
	}
	if ( !ent->mins[0] && !ent->mins[1] && !ent->mins[2] )
	{
		VectorSet (ent->mins, -16, -16, -16);
	}
	if ( !ent->maxs[0] && !ent->maxs[1] && !ent->maxs[2] )
	{
		VectorSet (ent->maxs, 16, 16, 16);
	}

	if ( ent->spawnflags & 2 )
	{
		ent->s.eFlags |= EF_ANIM_ALLFAST;
	}

	G_SetOrigin( ent, ent->s.origin );
	G_SetAngles( ent, ent->s.angles );
	gi.linkentity (ent);

	if ( ent->spawnflags & 128 )
	{//Can be used by the player's BUTTON_USE
		ent->svFlags |= SVF_PLAYER_USABLE;
	}

	if ( ent->team && ent->team[0] )
	{
		ent->noDamageTeam = TranslateTeamName( ent->team );
		if ( ent->noDamageTeam == TEAM_FREE )
		{
			G_Error("team name %s not recognized\n", ent->team);
		}
	}
	
	ent->team = NULL;

	/*
	//looks like crap and would be unfair
	if ( Q_stricmp( "models/mapobjects/borg/blite.md3", ent->model ) == 0 )
	{
		SpawnAssimilationTrigger( ent );
	}
	*/
}

