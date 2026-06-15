#include "g_local.h"
#include "g_functions.h"
#include "bg_public.h"
#include "g_infostrings.h"

ginfoitem_t	bg_infoItemList[II_NUM_ITEMS];
const int	bg_numInfoItems = II_NUM_ITEMS;
extern cvar_t *g_spskill;

//
// Helper functions
//
//------------------------------------------------------------
void SetMiscModelModels( char *modelNameString, gentity_t *ent, qboolean damage_model )
{
	char	damageModel[MAX_QPATH];
	char	chunkModel[MAX_QPATH];
	int		len;

	//Main model
	ent->s.modelindex = G_ModelIndex( modelNameString );

	if ( damage_model )
	{
		len = strlen( modelNameString ) - 4; // extract the extension

		//Dead/damaged model
		strncpy( damageModel, modelNameString, len );
		damageModel[len] = 0;
		strncpy( chunkModel, damageModel, sizeof(chunkModel));
		strcat( damageModel, "_d1.md3" );
		ent->s.modelindex2 = G_ModelIndex( damageModel );

		ent->spawnflags |= 4; // deadsolid

		//Chunk model
		strcat( chunkModel, "_c1.md3" );
		ent->s.modelindex3 = G_ModelIndex( chunkModel );
	}
}

//------------------------------------------------------------
void SetMiscModelDefaults( gentity_t *ent, useFunc_t use_func, char *material, int solid_mask,int animFlag, 
									qboolean take_damage, qboolean damage_model = qfalse )
{
	// Apply damage and chunk models if they exist
	SetMiscModelModels( ent->model, ent, damage_model );

	ent->s.eFlags = animFlag;
	ent->svFlags |= SVF_PLAYER_USABLE;
	ent->contents = solid_mask;

	G_SetOrigin( ent, ent->s.origin );
	VectorCopy( ent->s.angles, ent->s.apos.trBase );
	gi.linkentity (ent);

	// Set a generic use function
	ent->e_UseFunc = use_func;	
	if (use_func == useF_health_use) {
		G_SoundIndex("sound/player/suithealth.wav");
	} 
	else if (use_func == useF_ammo_use ) {
		G_SoundIndex("sound/player/suitenergy.wav");
	}

	G_SpawnInt( "material", material, (int *)&ent->material );
	
	if (ent->health) 
	{
		ent->max_health = ent->health;
		ent->takedamage = take_damage;
		ent->e_PainFunc = painF_misc_model_breakable_pain;
		ent->e_DieFunc  = dieF_misc_model_breakable_die;
	}
}

void HealthStationSettings(gentity_t *ent)
{
	G_SpawnInt( "count", "0", &ent->count );

	if (!ent->count)
	{
		switch (g_spskill->integer)
		{
		case 0:	//	EASY
			ent->count = 100; 
			break;
		case 1:	//	MEDIUM
			ent->count = 75; 
			break;
		default :
		case 2:	//	HARD
			ent->count = 50; 
			break;
		}
	}
}

void EnergyAmmoStationSettings(gentity_t *ent)
{
	G_SpawnInt( "count", "0", &ent->count );

	if (!ent->count)
	{
		switch (g_spskill->integer)
		{
		case 0:	//	EASY
			ent->count = 150; 
			break;
		case 1:	//	MEDIUM
			ent->count = 100; 
			break;
		default :
		case 2:	//	HARD
			ent->count = 100; 
			break;
		}
	}
}

void CrystalAmmoSettings(gentity_t *ent)
{
	G_SpawnInt( "count", "0", &ent->count );

	if (!ent->count)
	{
		switch (g_spskill->integer)
		{
		case 0:	//	EASY
			ent->count = 75; 
			break;
		case 1:	//	MEDIUM
			ent->count = 75; 
			break;
		default :
		case 2:	//	HARD
			ent->count = 75; 
			break;
		}
	}
}


//------------------------------------------------------------

//------------------------------------------------------------
//
//	Borg misc models
//
//------------------------------------------------------------
/*QUAKED misc_model_borg_ammo (1 0 0) (-16 -16 -16) (16 16 16) x x x USETARGET
#MODELNAME="models/mapobjects/borg/plugin.md3"
Gives energy when used.

USETARGET - when used it fires off target

"health" - how much health the model has - default 60 (zero makes non-breakable)
"targetname" - dies and displays damagemodel when used, if any (if not, removes itself)
"target" - what to use when it dies
"paintarget" - target to fire when hit (but not destroyed)
"count" - the amount of ammo given when used (default 100)

"material" - default is "borg" - choose from this list:
0 = MAT_METAL		
1 = MAT_GLASS
2 = MAT_ELECTRICAL	(sparks)
3 = MAT_ORGANIC		(not implemented)
4 =	MAT_BORG		(default)
5 =	MAT_STASIS		(stasis chunks)
6 =	MAT_GLASS_METAL (mixed chunk type)
*/
//------------------------------------------------------------
void SP_misc_model_borg_ammo( gentity_t *ent )
{
	if (!ent->health)
	{
		ent->health = 60;
	}

	SetMiscModelDefaults( ent, useF_ammo_use, "4", CONTENTS_SOLID, NULL, qfalse );

	ent->infoString = bg_infoItemList[II_ENERGY_REGEN].infoString;
	VectorCopy( bg_infoItemList[II_ENERGY_REGEN].color, ent->startRGBA );

	EnergyAmmoStationSettings(ent);

	ent->s.modelindex2 = G_ModelIndex("/models/mapobjects/borg/plugin_in.md3");	// Precache model
}

//------------------------------------------------------------
/*QUAKED misc_model_borg_health (1 0 0) (-16 -16 -16) (16 16 16) x x x USETARGET
#MODELNAME="models/mapobjects/borg/plugin2.md3"
USETARGET - when used it fires off target

"health" - how much health the model has - default 60 (zero makes non-breakable)
"targetname" - dies and displays damagemodel when used, if any (if not, removes itself)
"target" - what to use when it dies
"paintarget" - target to fire when hit (but not destroyed)
"count" - the amount of health given when used (default 100)

"material" - default is "borg" - choose from this list:
0 = MAT_METAL		
1 = MAT_GLASS
2 = MAT_ELECTRICAL	(sparks)
3 = MAT_ORGANIC		(not implemented)
4 =	MAT_BORG		(default)
5 =	MAT_STASIS		(stasis chunks)
6 =	MAT_GLASS_METAL (mixed chunk type)
*/
//------------------------------------------------------------
void SP_misc_model_borg_health( gentity_t *ent )
{
	if (!ent->health)
	{
		ent->health = 60;
	}

	SetMiscModelDefaults( ent, useF_health_use,  "4", CONTENTS_SOLID, NULL, qfalse );

	ent->infoString = bg_infoItemList[II_HEALTH_REGEN].infoString;
	VectorCopy( bg_infoItemList[II_HEALTH_REGEN].color, ent->startRGBA );

	HealthStationSettings(ent);

	ent->s.modelindex2 = G_ModelIndex("/models/mapobjects/borg/plugin2_in.md3");
}


//------------------------------------------------------------
/*QUAKED misc_model_borg_disnode (1 0 0) (-16 -16 -16) (16 16 16) x x x USETARGET
#MODELNAME="models/mapobjects/borg/disnode.md3"
USETARGET - when used it fires off target

"health" - how much health the model has - default 40 (zero makes non-breakable)
"targetname" - dies and displays damagemodel when used, if any (if not, removes itself)
"target" - what to use when it dies
"paintarget" - target to fire when hit (but not destroyed)

"material" - default is "borg" - choose from this list:
0 = MAT_METAL		
1 = MAT_GLASS
2 = MAT_ELECTRICAL	(sparks)
3 = MAT_ORGANIC		(not implemented)
4 =	MAT_BORG		(default)
5 =	MAT_STASIS		(stasis chunks)
6 =	MAT_GLASS_METAL (mixed chunk type)
*/
//------------------------------------------------------------
void SP_misc_model_borg_disnode( gentity_t *ent )
{
	if (!ent->health)
	{
		ent->health = 40;
	}

	SetMiscModelDefaults( ent, useF_NULL,  "4", CONTENTS_SOLID, NULL, qtrue, qtrue );

	ent->infoString = bg_infoItemList[II_DIS_NODE].infoString;
	VectorCopy( bg_infoItemList[II_DIS_NODE].color, ent->startRGBA );
}


//------------------------------------------------------------
//
//	Stasis misc models
//
//------------------------------------------------------------
/*QUAKED misc_model_stasis_ammo (1 0 0) (-16 -16 -16) (16 16 16)  x x x USETARGET
#MODELNAME="models/mapobjects/stasis/plugin.md3"
USETARGET - when used it fires off target

"health" - how much health the model has - default 60 (zero makes non-breakable)
"targetname" - dies and displays damagemodel when used, if any (if not, removes itself)
"target" - what to use when it dies
"paintarget" - target to fire when hit (but not destroyed)
"count" - the amount of ammo given when used (default 100)

"material" - default is "stasis" - choose from this list:
0 = MAT_METAL		
1 = MAT_GLASS
2 = MAT_ELECTRICAL	(sparks)
3 = MAT_ORGANIC		(not implemented)
4 =	MAT_BORG		(borg chunks)
5 =	MAT_STASIS		(default)
6 =	MAT_GLASS_METAL (mixed chunk type)
*/
//------------------------------------------------------------
void SP_misc_model_stasis_ammo( gentity_t *ent )
{
	if (!ent->health)
	{
		ent->health = 60;
	}

	SetMiscModelDefaults( ent, useF_ammo_use, "5", CONTENTS_SOLID, EF_ANIM_ALLFAST, qfalse );

	ent->infoString = bg_infoItemList[II_ENERGY_REGEN].infoString;
	VectorCopy( bg_infoItemList[II_ENERGY_REGEN].color, ent->startRGBA );

	EnergyAmmoStationSettings(ent);

	ent->s.modelindex2 = G_ModelIndex("/models/mapobjects/stasis/plugin_in.md3");	// Precache model
}


/*QUAKED misc_model_stasis_ammo_floor (1 0 0) (-16 -16 -16) (16 16 16)  x x x USETARGET
#MODELNAME="models/mapobjects/stasis/plugin_floor.md3"
USETARGET - when used it fires off target

"model"		arbitrary .md3 file to display
"health"	how much health to have - default is zero (not breakable)  If you don't set the SOLID flag, but give it health, it can be shot but will not blocking NPCs or players from moving
"targetname" when used, dies and displays damagemodel, if any (if not, removes itself)
"target" What to use when it dies
"target2" What to use when it's repaired
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
//------------------------------------------------------------
void SP_misc_model_stasis_ammo_floor( gentity_t *ent )
{
	if (!ent->health)
	{
		ent->health = 60;
	}

	SetMiscModelDefaults( ent, useF_ammo_use, "5", CONTENTS_SOLID, EF_ANIM_ALLFAST, qfalse );

	ent->infoString = bg_infoItemList[II_ENERGY_REGEN].infoString;
	VectorCopy( bg_infoItemList[II_ENERGY_REGEN].color, ent->startRGBA );

	EnergyAmmoStationSettings(ent);

	ent->s.modelindex2 = G_ModelIndex("/models/mapobjects/stasis/plugin_floor_in.md3");
	G_SoundIndex("sound/ambience/stasis/shrinkage.wav");
	G_SoundIndex("sound/weapons/noammo.wav");

}

//------------------------------------------------------------
/*QUAKED misc_model_stasis_health (1 0 0) (-16 -16 -16) (16 16 16)  x x x USETARGET
#MODELNAME="models/mapobjects/stasis/plugin2.md3"
USETARGET - when used it fires off target

"health" - how much health the model has - default 60 (zero makes non-breakable)
"targetname" - dies and displays damagemodel when used, if any (if not, removes itself)
"target" - what to use when it dies
"paintarget" - target to fire when hit (but not destroyed)
"count" - the amount of health given when used (default 100)

"material" - default is "stasis" - choose from this list:
0 = MAT_METAL		
1 = MAT_GLASS
2 = MAT_ELECTRICAL	(sparks)
3 = MAT_ORGANIC		(not implemented)
4 =	MAT_BORG		(borg chunks)
5 =	MAT_STASIS		(default)
6 =	MAT_GLASS_METAL (mixed chunk type)
*/
//------------------------------------------------------------
void SP_misc_model_stasis_health( gentity_t *ent )
{
	if (!ent->health)
	{
		ent->health = 60;
	}

	SetMiscModelDefaults( ent, useF_health_use, "5", CONTENTS_SOLID,EF_ANIM_ALLFAST, qfalse );

	ent->infoString = bg_infoItemList[II_HEALTH_REGEN].infoString;
	VectorCopy( bg_infoItemList[II_HEALTH_REGEN].color, ent->startRGBA );

	HealthStationSettings(ent);

	ent->s.modelindex2 = G_ModelIndex("/models/mapobjects/stasis/plugin2_in.md3");
}

//------------------------------------------------------------
/*QUAKED misc_model_stasis_health_floor (1 0 0) (-16 -16 -16) (16 16 16)  x x x USETARGET
#MODELNAME="models/mapobjects/stasis/plugin2_floor.md3"
USETARGET - when used it fires off target

"model"		arbitrary .md3 file to display
"health"	how much health to have - default is zero (not breakable)  If you don't set the SOLID flag, but give it health, it can be shot but will not blocking NPCs or players from moving
"targetname" when used, dies and displays damagemodel, if any (if not, removes itself)
"target" What to use when it dies
"target2" What to use when it's repaired
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
//------------------------------------------------------------
void SP_misc_model_stasis_health_floor( gentity_t *ent )
{
	if (!ent->health)
	{
		ent->health = 60;
	}

	SetMiscModelDefaults( ent, useF_health_use, "5", CONTENTS_SOLID, EF_ANIM_ALLFAST, qfalse );

	ent->infoString = bg_infoItemList[II_HEALTH_REGEN].infoString;
	VectorCopy( bg_infoItemList[II_HEALTH_REGEN].color, ent->startRGBA );

	HealthStationSettings(ent);

	ent->s.modelindex2 = G_ModelIndex("/models/mapobjects/stasis/plugin2_floor_in.md3");
	G_SoundIndex("sound/ambience/stasis/shrinkage1.wav");
	G_SoundIndex("sound/weapons/noammo.wav");
}


//------------------------------------------------------------
/*QUAKED misc_model_scavenger_ammo (1 0 0) (-16 -16 -16) (16 16 16)  x x x USETARGET
#MODELNAME="models/mapobjects/scavenger/power_up.md3"
USETARGET - when used it fires off target

"health" - how much health the model has - default 60 (zero makes non-breakable)
"targetname" - dies and displays damagemodel when used, if any (if not, removes itself)
"target" - what to use when it dies
"paintarget" - target to fire when hit (but not destroyed)
"count" - the amount of health given when used (default 100)

"material" - default is "stasis" - choose from this list:
0 = MAT_METAL		
1 = MAT_GLASS
2 = MAT_ELECTRICAL	(sparks)
3 = MAT_ORGANIC		(not implemented)
4 =	MAT_BORG		(borg chunks)
5 =	MAT_STASIS		(default)
6 =	MAT_GLASS_METAL (mixed chunk type)
*/
//------------------------------------------------------------
void SP_misc_model_scavenger_ammo( gentity_t *ent )
{
	if (!ent->health)
	{
		ent->health = 60;
	}

	SetMiscModelDefaults( ent, useF_ammo_use, "5", CONTENTS_SOLID, NULL, qfalse );

	ent->infoString = bg_infoItemList[II_ENERGY_REGEN].infoString;
	VectorCopy( bg_infoItemList[II_ENERGY_REGEN].color, ent->startRGBA );
	EnergyAmmoStationSettings(ent);
}

//------------------------------------------------------------
/*QUAKED misc_model_scavenger_health (1 0 0) (-16 -16 -16) (16 16 16)  x x x USETARGET
#MODELNAME="models/mapobjects/scavenger/power_up2.md3"
USETARGET - when used it fires off target

"health" - how much health the model has - default 60 (zero makes non-breakable)
"targetname" - dies and displays damagemodel when used, if any (if not, removes itself)
"target" - what to use when it dies
"paintarget" - target to fire when hit (but not destroyed)
"count" - the amount of health given when used (default 100)

"material" - default is "stasis" - choose from this list:
0 = MAT_METAL		
1 = MAT_GLASS
2 = MAT_ELECTRICAL	(sparks)
3 = MAT_ORGANIC		(not implemented)
4 =	MAT_BORG		(borg chunks)
5 =	MAT_STASIS		(default)
6 =	MAT_GLASS_METAL (mixed chunk type)
*/
//------------------------------------------------------------
void SP_misc_model_scavenger_health( gentity_t *ent )
{
	if (!ent->health)
	{
		ent->health = 60;
	}

	SetMiscModelDefaults( ent, useF_health_use, "5", CONTENTS_SOLID, NULL, qfalse );

	ent->infoString = bg_infoItemList[II_HEALTH_REGEN].infoString;
	VectorCopy( bg_infoItemList[II_HEALTH_REGEN].color, ent->startRGBA );

	HealthStationSettings(ent);
}

//------------------------------------------------------------

#define MEGA_AMMO 16

//------------------------------------------------------------
/*QUAKED misc_model_forge_ammo (1 0 0) (-16 -16 -16) (16 16 16)  x x x USETARGET MEGA
#MODELNAME="models/mapobjects/forge/power_up.md3"
USETARGET - when used it fires off target
MEGA - gives 999 ammo like the MEGA station
"health" - how much health the model has - default 60 (zero makes non-breakable)
"targetname" - dies and displays damagemodel when used, if any (if not, removes itself)
"target" - what to use when it dies
"paintarget" - target to fire when hit (but not destroyed)
"count" - the amount of health given when used (default 100)

"material" - default is "stasis" - choose from this list:
0 = MAT_METAL		
1 = MAT_GLASS
2 = MAT_ELECTRICAL	(sparks)
3 = MAT_ORGANIC		(not implemented)
4 =	MAT_BORG		(borg chunks)
5 =	MAT_STASIS		(default)
6 =	MAT_GLASS_METAL (mixed chunk type)
*/
//------------------------------------------------------------
void SP_misc_model_forge_ammo( gentity_t *ent )
{
	if (!ent->health)
	{
		ent->health = 60;
	}

	if (ent->spawnflags & MEGA_AMMO)
	{
		SetMiscModelDefaults( ent, useF_mega_ammo_use, "5", CONTENTS_SOLID, NULL, qfalse );
	}
	else
	{
		SetMiscModelDefaults( ent, useF_ammo_use, "5", CONTENTS_SOLID, NULL, qfalse );
	}

	ent->infoString = bg_infoItemList[II_ENERGY_REGEN].infoString;
	VectorCopy( bg_infoItemList[II_ENERGY_REGEN].color, ent->startRGBA );
	EnergyAmmoStationSettings(ent);

	G_SoundIndex( "sound/player/superenergy.wav" );
}

//------------------------------------------------------------

//------------------------------------------------------------
/*QUAKED misc_model_forge_health (1 0 0) (-16 -16 -16) (16 16 16)  x x x USETARGET
#MODELNAME="models/mapobjects/forge/power_up2.md3"
USETARGET - when used it fires off target

"health" - how much health the model has - default 60 (zero makes non-breakable)
"targetname" - dies and displays damagemodel when used, if any (if not, removes itself)
"target" - what to use when it dies
"paintarget" - target to fire when hit (but not destroyed)
"count" - the amount of health given when used (default 100)

"material" - default is "stasis" - choose from this list:
0 = MAT_METAL		
1 = MAT_GLASS
2 = MAT_ELECTRICAL	(sparks)
3 = MAT_ORGANIC		(not implemented)
4 =	MAT_BORG		(borg chunks)
5 =	MAT_STASIS		(default)
6 =	MAT_GLASS_METAL (mixed chunk type)
*/
//------------------------------------------------------------
void SP_misc_model_forge_health( gentity_t *ent )
{
	if (!ent->health)
	{
		ent->health = 60;
	}

	SetMiscModelDefaults( ent, useF_health_use, "5", CONTENTS_SOLID, NULL, qfalse  );

	ent->infoString = bg_infoItemList[II_HEALTH_REGEN].infoString;
	VectorCopy( bg_infoItemList[II_HEALTH_REGEN].color, ent->startRGBA );

	HealthStationSettings(ent);
}


//------------------------------------------------------------
/*QUAKED misc_model_dreadnought_ammo (1 0 0) (-16 -16 -16) (16 16 16)  x x x USETARGET
#MODELNAME="models/mapobjects/dn/powercell.md3"
USETARGET - when used it fires off target

"health" - how much health the model has - default 60 (zero makes non-breakable)
"targetname" - dies and displays damagemodel when used, if any (if not, removes itself)
"target" - what to use when it dies
"paintarget" - target to fire when hit (but not destroyed)
"count" - the amount of health given when used (default 100)

"material" - default is "stasis" - choose from this list:
0 = MAT_METAL		
1 = MAT_GLASS
2 = MAT_ELECTRICAL	(sparks)
3 = MAT_ORGANIC		(not implemented)
4 =	MAT_BORG		(borg chunks)
5 =	MAT_STASIS		(default)
6 =	MAT_GLASS_METAL (mixed chunk type)
*/
//------------------------------------------------------------
void SP_misc_model_dreadnought_ammo( gentity_t *ent )
{
	if (!ent->health)
	{
		ent->health = 60;
	}

	SetMiscModelDefaults( ent, useF_ammo_use, "5", CONTENTS_SOLID, NULL, qfalse );

	ent->infoString = bg_infoItemList[II_ENERGY_REGEN].infoString;
	VectorCopy( bg_infoItemList[II_ENERGY_REGEN].color, ent->startRGBA );
	EnergyAmmoStationSettings(ent);
}

//------------------------------------------------------------
/*QUAKED misc_model_dreadnought_health (1 0 0) (-16 -16 -16) (16 16 16)  x x x USETARGET
#MODELNAME="models/mapobjects/dn/powercell2.md3"
USETARGET - when used it fires off target

"health" - how much health the model has - default 60 (zero makes non-breakable)
"targetname" - dies and displays damagemodel when used, if any (if not, removes itself)
"target" - what to use when it dies
"paintarget" - target to fire when hit (but not destroyed)
"count" - the amount of health given when used (default 100)

"material" - default is "stasis" - choose from this list:
0 = MAT_METAL		
1 = MAT_GLASS
2 = MAT_ELECTRICAL	(sparks)
3 = MAT_ORGANIC		(not implemented)
4 =	MAT_BORG		(borg chunks)
5 =	MAT_STASIS		(default)
6 =	MAT_GLASS_METAL (mixed chunk type)
*/
//------------------------------------------------------------
void SP_misc_model_dreadnought_health( gentity_t *ent )
{
	if (!ent->health)
	{
		ent->health = 60;
	}

	SetMiscModelDefaults( ent, useF_health_use, "5", CONTENTS_SOLID, NULL, qfalse  );

	ent->infoString = bg_infoItemList[II_HEALTH_REGEN].infoString;
	VectorCopy( bg_infoItemList[II_HEALTH_REGEN].color, ent->startRGBA );

	HealthStationSettings(ent);
}

//------------------------------------------------------------
/*QUAKED misc_model_stasis_control_switch (1 0 0) (-16 -16 -16) (16 16 16)  NO_PLAYER NO_TRIGGER
#MODELNAME="models/mapobjects/stasis/control_switch.md3"
NO_PLAYER - player can't use this
NO_TRIGGER - doesn't spawn in a touch trigger around the control switch
*/
//------------------------------------------------------------
void SP_misc_model_stasis_control_switch( gentity_t *ent )
{
	SetMiscModelDefaults( ent, useF_stasis_control_switch_use, "5", CONTENTS_SOLID, NULL, qfalse  );

	// The stasis controls need to open when the player gets close to them, so spawn a trigger...use is a separate function though.
	if ( !(ent->spawnflags & 2) ) // NO_TRIGGER
	{
		ent->e_ThinkFunc = thinkF_spawn_stasis_control_trigger;
		ent->nextthink = level.time + 200;
	}

	// Precache necessary sounds
	G_SoundIndex( "sound/movers/switches/stasisopen.wav" );
	G_SoundIndex( "sound/movers/switches/stasispos.wav" );

//	ent->infoString = bg_infoItemList[II_CONTROL_SWITCH].infoString;
//	VectorCopy( bg_infoItemList[II_CONTROL_SWITCH].color, ent->startRGBA );
//	G_SpawnString( "infostring", "Control Switch", &ent->infoString );

}

/*QUAKED misc_model_plasma_filter (1 0 0) (-16 -16 -16) (16 16 16) x x x USETARGET NO_SMOKE
#MODELNAME="models/mapobjects/borg/tank.md3"

USETARGET - when used it fires off target
NO_SMOKE - do not leave residual smoke

"health" - how much health the model has - default 1 (zero makes non-breakable)
"targetname" - dies and displays damagemodel when used, if any (if not, removes itself)
"target" - what to use when it dies
"paintarget" - target to fire when hit (but not destroyed)
"count" - the amount of ammo given when used (default 100)

"material" - default is "glass metal" - choose from this list:
0 = MAT_METAL		
1 = MAT_GLASS
2 = MAT_ELECTRICAL	(sparks)
3 = MAT_ORGANIC		(not implemented)
4 =	MAT_BORG		(default)
5 =	MAT_STASIS		(stasis chunks)
6 =	MAT_GLASS_METAL (mixed chunk type)
*/
//------------------------------------------------------------
void SP_misc_model_plasma_filter( gentity_t *ent )
{
	if (!ent->health)
	{
		ent->health = 1;
	}

	SetMiscModelDefaults( ent, useF_NULL, "6", CONTENTS_SOLID, NULL, qtrue, qfalse );

	ent->infoString = bg_infoItemList[II_PLASMA_FILTER].infoString;
	VectorCopy( bg_infoItemList[II_PLASMA_FILTER].color, ent->startRGBA );
//	G_SpawnString( "infostring", "Plasma Filter", &ent->infoString );
}

//------------------------------------------------------------
/*QUAKED misc_model_forge_panel (1 0 0) (-16 -16 -16) (16 16 16)  x x x USETARGET
#MODELNAME="models/mapobjects/forge/panels2.md3"
USETARGET - when used it fires off target

"health" - how much health the model has - default 60 (zero makes non-breakable)
"targetname" - dies and displays damagemodel when used, if any (if not, removes itself)
"target" - what to use when it dies
"paintarget" - target to fire when hit (but not destroyed)

"material" - default is "metal" - choose from this list:
0 = MAT_METAL		
1 = MAT_GLASS
2 = MAT_ELECTRICAL	(sparks)
3 = MAT_ORGANIC		(not implemented)
4 =	MAT_BORG		(borg chunks)
5 =	MAT_STASIS		(default)
6 =	MAT_GLASS_METAL (mixed chunk type)
*/
//------------------------------------------------------------
void SP_misc_model_forge_panel( gentity_t *ent )
{
	if (!ent->health)
	{
		ent->health = 60;
	}

	SetMiscModelDefaults( ent, useF_misc_model_useup, "0", CONTENTS_SOLID,EF_ANIM_ALLFAST, qfalse );

	ent->infoString = bg_infoItemList[II_CONTROL_PANEL].infoString;
	VectorCopy( bg_infoItemList[II_CONTROL_PANEL].color, ent->startRGBA );
	G_SpawnInt( "count", "0", &ent->count );

	ent->s.modelindex2 = G_ModelIndex("models/mapobjects/forge/panels3.md3");

	// FIXME: this sound will have to be changed.
	G_SoundIndex("sound/player/suitenergy.wav");
}

//------------------------------------------------------------
/*QUAKED misc_model_stasis_crystal_station (1 0 0) (-16 -16 -16) (16 16 16)
#MODELNAME="models/mapobjects/stasis/alien_ammo2.md3"

 count - amount of alien ammo to give (default 100)
*/
//------------------------------------------------------------
void SP_misc_model_stasis_crystal_station( gentity_t *ent )
{
	SetMiscModelDefaults( ent, useF_NULL, "5", CONTENTS_SOLID, NULL, qfalse  );

	// The ammo crystal needs to be picked up when the player walks up to it, so spawn in a trigger for it
	ent->e_ThinkFunc = thinkF_spawn_ammo_crystal_trigger;
	ent->nextthink = level.time + 200;

	ent->s.modelindex2 = G_ModelIndex( "models/mapobjects/stasis/alien_ammo2_b.md3" );

	CrystalAmmoSettings(ent);

	ent->infoString = bg_infoItemList[II_POWER_CRYSTAL].infoString;
	VectorCopy( bg_infoItemList[II_POWER_CRYSTAL].color, ent->startRGBA );
}

//------------------------------------------------------------
/*QUAKED misc_model_borg_crystal_station (1 0 0) (-16 -16 -16) (16 16 16)
#MODELNAME="models/mapobjects/borg/alien_ammo2.md3"

 count - amount of alien ammo to give (default 100)
*/
//------------------------------------------------------------
void SP_misc_model_borg_crystal_station( gentity_t *ent )
{
	SetMiscModelDefaults( ent, useF_NULL, "5", CONTENTS_SOLID, NULL, qfalse  );

	// The ammo crystal needs to be picked up when the player walks up to it, so spawn in a trigger for it
	ent->e_ThinkFunc = thinkF_spawn_ammo_crystal_trigger;
	ent->nextthink = level.time + 200;

	ent->s.modelindex2 = G_ModelIndex( "models/mapobjects/borg/alien_ammo2_b.md3" );

	CrystalAmmoSettings(ent);

	ent->infoString = bg_infoItemList[II_POWER_CRYSTAL].infoString;
	VectorCopy( bg_infoItemList[II_POWER_CRYSTAL].color, ent->startRGBA );
}

//------------------------------------------------------------
/*QUAKED misc_model_scav_crystal_station (1 0 0) (-16 -16 -16) (16 16 16)
#MODELNAME="models/mapobjects/scavenger/alien_ammo2.md3"

 count - amount of alien ammo to give (default 100)
*/
//------------------------------------------------------------
void SP_misc_model_scav_crystal_station( gentity_t *ent )
{
	SetMiscModelDefaults( ent, useF_NULL, "5", CONTENTS_SOLID, NULL, qfalse  );

	// The ammo crystal needs to be picked up when the player walks up to it, so spawn in a trigger for it
	ent->e_ThinkFunc = thinkF_spawn_ammo_crystal_trigger;
	ent->nextthink = level.time + 200;

	ent->s.modelindex2 = G_ModelIndex( "models/mapobjects/scavenger/alien_ammo2_b.md3" );

	CrystalAmmoSettings(ent);

	ent->infoString = bg_infoItemList[II_POWER_CRYSTAL].infoString;
	VectorCopy( bg_infoItemList[II_POWER_CRYSTAL].color, ent->startRGBA );
}

//------------------------------------------------------------
/*QUAKED misc_model_dn_crystal_station (1 0 0) (-16 -16 -16) (16 16 16)
#MODELNAME="models/mapobjects/dn/alien_ammo2.md3"

 count - amount of alien ammo to give (default 100)
*/
//------------------------------------------------------------
void SP_misc_model_dn_crystal_station( gentity_t *ent )
{
	SetMiscModelDefaults( ent, useF_NULL, "5", CONTENTS_SOLID, NULL, qfalse  );

	// The ammo crystal needs to be picked up when the player walks up to it, so spawn in a trigger for it
	ent->e_ThinkFunc = thinkF_spawn_ammo_crystal_trigger;
	ent->nextthink = level.time + 200;

	ent->s.modelindex2 = G_ModelIndex( "models/mapobjects/dn/alien_ammo2_b.md3" );

	CrystalAmmoSettings(ent);

	ent->infoString = bg_infoItemList[II_POWER_CRYSTAL].infoString;
	VectorCopy( bg_infoItemList[II_POWER_CRYSTAL].color, ent->startRGBA );
}


//------------------------------------------------------------
/*QUAKED misc_model_forge_defence_grid (1 0 0) (-16 -16 -16) (16 16 16)
#MODELNAME="models/mapobjects/forge/d_grid.md3"
*/
//------------------------------------------------------------
void SP_misc_model_forge_defence_grid( gentity_t *ent )
{
	SetMiscModelDefaults( ent, useF_switch_models, "5", CONTENTS_SOLID, NULL, qfalse  );
	ent->s.eFlags |= EF_ANIM_ALLFAST;

	ent->s.modelindex2 = G_ModelIndex( "models/mapobjects/forge/d_grid2.md3" );
}


//------------------------------------------------------------
/*QUAKED misc_model_forge_reaver_container (1 0 0) (-16 -16 -16) (16 16 16) INVISIBLE
#MODELNAME="models/mapobjects/forge/capsule2.md3"

INVISIBLE - invisible until used

"health" - how much health the model has - default 1 (zero makes non-breakable)
"targetname" - dies and displays damagemodel when used, if any (if not, removes itself)
"target" - what to use when it dies
*/
//------------------------------------------------------------
void SP_misc_model_forge_reaver_container( gentity_t *ent )
{
	useFunc_t useFunction = useF_reaver_vat_use;

	if ( !ent->spawnflags & 1 )
		useFunction = useF_NULL;

	SetMiscModelDefaults( ent, useFunction, "1", CONTENTS_SOLID, NULL, qtrue, qtrue );

	G_SpawnInt( "health", "1", &ent->health );

	ent->max_health = ent->health;
	ent->takedamage = qtrue;
	ent->e_PainFunc = painF_NULL;
	ent->e_DieFunc  = dieF_reaver_vat_die;

	if ( ent->spawnflags & 1 ) // invisible
	{
		ent->s.eFlags |= EF_NODRAW;
		ent->contents = 0;
		gi.unlinkentity( ent );
	}
//	ent->s.modelindex2 = G_ModelIndex( "models/mapobjects/forge/capsule2.md3" );
}

//------------------------------------------------------------
/*QUAKED misc_model_forge_mega_ammo (1 0 0) (-16 -16 -16) (16 16 16)
#MODELNAME="models/mapobjects/forge/power_up_boss.md3"
Give the player mega ammo when used
*/
//------------------------------------------------------------
void SP_misc_model_forge_mega_ammo( gentity_t *ent )
{
	SetMiscModelDefaults( ent, useF_mega_ammo_use, "5", CONTENTS_SOLID, NULL, qfalse );

	ent->infoString = bg_infoItemList[II_ENERGY_REGEN].infoString;
	VectorCopy( bg_infoItemList[II_ENERGY_REGEN].color, ent->startRGBA );

	ent->s.modelindex2 = G_ModelIndex("models/mapobjects/forge/power_up_bossoff.md3");	// Precache model
	G_SoundIndex( "sound/player/superenergy.wav" );
}

//------------------------------------------------------------
//
//	Captain Protein (tm) misc models
//
//------------------------------------------------------------
/*QUAKED misc_model_proton_ammo (1 0 0) (-16 -16 -16) (16 16 16) x x x USETARGET
#MODELNAME="models/mapobjects/proton/weapon_e.md3"
Gives energy when used.

USETARGET - when used it fires off target

"health" - how much health the model has - default 60 (zero makes non-breakable)
"targetname" - dies and displays damagemodel when used, if any (if not, removes itself)
"target" - what to use when it dies
"paintarget" - target to fire when hit (but not destroyed)
"count" - the amount of ammo given when used (default 100)

"material" - default is "borg" - choose from this list:
0 = MAT_METAL		
1 = MAT_GLASS
2 = MAT_ELECTRICAL	(sparks)
3 = MAT_ORGANIC		(not implemented)
4 =	MAT_BORG		(default)
5 =	MAT_STASIS		(stasis chunks)
6 =	MAT_GLASS_METAL (mixed chunk type)
*/
//------------------------------------------------------------
void SP_misc_model_proton_ammo( gentity_t *ent )
{
	if (!ent->health)
	{
		ent->health = 60;
	}

	SetMiscModelDefaults( ent, useF_ammo_use, "4", CONTENTS_SOLID, NULL, qfalse );

	ent->infoString = bg_infoItemList[II_ENERGY_REGEN].infoString;
	VectorSet( ent->startRGBA, 1.0f, 1.0f, 1.0f );

	EnergyAmmoStationSettings(ent);
}

//------------------------------------------------------------
/*QUAKED misc_model_proton_health (1 0 0) (-16 -16 -16) (16 16 16) x x x USETARGET
#MODELNAME="models/mapobjects/proton/health.md3"
USETARGET - when used it fires off target

"health" - how much health the model has - default 60 (zero makes non-breakable)
"targetname" - dies and displays damagemodel when used, if any (if not, removes itself)
"target" - what to use when it dies
"paintarget" - target to fire when hit (but not destroyed)
"count" - the amount of health given when used (default 100)

"material" - default is "borg" - choose from this list:
0 = MAT_METAL		
1 = MAT_GLASS
2 = MAT_ELECTRICAL	(sparks)
3 = MAT_ORGANIC		(not implemented)
4 =	MAT_BORG		(default)
5 =	MAT_STASIS		(stasis chunks)
6 =	MAT_GLASS_METAL (mixed chunk type)
*/
//------------------------------------------------------------
void SP_misc_model_proton_health( gentity_t *ent )
{
	if (!ent->health)
	{
		ent->health = 60;
	}

	SetMiscModelDefaults( ent, useF_health_use,  "4", CONTENTS_SOLID, NULL, qfalse );

	ent->infoString = bg_infoItemList[II_HEALTH_REGEN].infoString;
	VectorSet( ent->startRGBA, 1.0f, 1.0f, 1.0f );

	HealthStationSettings(ent);
}
