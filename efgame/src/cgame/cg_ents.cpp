// cg_ents.c -- present snapshot entities, happens every single frame

#include "cg_local.h"
#include "FX_Public.h"
#include "../game/g_functions.h"
#include "../game/boltons.h"

extern qboolean CG_ApplyBoltOnToRefEnt (refEntity_t *newBoltOn, boltOn_t *boltOn, boltOnInfo_t *bOInfo, const vec3_t org, refEntity_t *targModel);
extern void CG_AssimilationTubules( vec3_t start, vec3_t end, vec3_t up, float scale );
/*
======================
CG_PositionEntityOnTag

Modifies the entities position and axis by the given
tag location
======================
*/
void CG_PositionEntityOnTag( refEntity_t *entity, const refEntity_t *parent, 
							qhandle_t parentModel, char *tagName ) {
	int				i;
	orientation_t	lerped;
	
	// lerp the tag
	cgi_R_LerpTag( &lerped, parentModel, parent->oldframe, parent->frame,
		1.0f - parent->backlerp, tagName );

	// FIXME: allow origin offsets along tag?
	VectorCopy( parent->origin, entity->origin );
	for ( i = 0 ; i < 3 ; i++ ) 
	{
		VectorMA( entity->origin, lerped.origin[i], parent->axis[i], entity->origin );
	}

	// had to cast away the const to avoid compiler problems...
	MatrixMultiply( lerped.axis, ((refEntity_t *)parent)->axis, entity->axis );
	entity->backlerp = parent->backlerp;
}

/*
======================
CG_PositionRotatedEntityOnTag

Modifies the entities position and axis by the given
tag location
======================
*/
void CG_PositionRotatedEntityOnTag( refEntity_t *entity, const refEntity_t *parent, 
							qhandle_t parentModel, char *tagName, orientation_t *tagOrient ) {
	int				i;
	orientation_t	lerped;
	vec3_t			tempAxis[3];

	// lerp the tag
	cgi_R_LerpTag( &lerped, parentModel, parent->oldframe, parent->frame,
		1.0f - parent->backlerp, tagName );

	if ( tagOrient )
	{
		VectorCopy( lerped.origin, tagOrient->origin );
		for ( i = 0 ; i < 3 ; i++ ) 
		{
			VectorCopy( lerped.axis[i], tagOrient->axis[i] );
		}
	}

	// FIXME: allow origin offsets along tag?
	VectorCopy( parent->origin, entity->origin );
	for ( i = 0 ; i < 3 ; i++ ) {
		VectorMA( entity->origin, lerped.origin[i], parent->axis[i], entity->origin );
	}

	MatrixMultiply( entity->axis, lerped.axis, tempAxis );
	MatrixMultiply( tempAxis, ((refEntity_t *)parent)->axis, entity->axis );
}



/*
==========================================================================

FUNCTIONS CALLED EACH FRAME

==========================================================================
*/

/*
======================
CG_SetEntitySoundPosition

Also called by event processing code
======================
*/
void CG_SetEntitySoundPosition( centity_t *cent ) {
	if ( cent->currentState.solid == SOLID_BMODEL ) {
		vec3_t	origin;
		float	*v;

		v = cgs.inlineModelMidpoints[ cent->currentState.modelindex ];
		VectorAdd( cent->lerpOrigin, v, origin );
		cgi_S_UpdateEntityPosition( cent->currentState.number, origin );
	} else {
		cgi_S_UpdateEntityPosition( cent->currentState.number, cent->lerpOrigin );
	}
}

/*
==================
CG_EntityEffects

Add continuous entity effects, like local entity emission and lighting
==================
*/
static void CG_EntityEffects( centity_t *cent ) {

	// update sound origins
	CG_SetEntitySoundPosition( cent );

	// add loop sound
	if ( cent->currentState.loopSound ) 
	{
		
		sfxHandle_t	sfx = ( cent->currentState.eType == ET_MOVER ) ? cent->currentState.loopSound : cgs.sound_precache[ cent->currentState.loopSound ];

		cgi_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, vec3_origin, sfx );
	}


	// constant light glow
	if ( cent->currentState.constantLight ) {
		int		cl;
		float	i, r, g, b;

		cl = cent->currentState.constantLight;
		// constantLight packs r/g/b as 0..255 bytes; the renderer wants 0..1 floats (it does
		// colors[n] = dl->color[n] * modulate, tr_shade.c). Passing 0..255 saturated the whole light
		// to white -> constantLight glows (wall fixtures, glowing props) blew out AND clipped the soft
		// radial falloff into a hard narrow disk. Normalize to 0..1 (matches HM/retail). Intensity
		// (the radius, (cl>>24)*4) is intentionally left unscaled.
		r = ( cl & 255 ) / 255.0f;
		g = ( ( cl >> 8 ) & 255 ) / 255.0f;
		b = ( ( cl >> 16 ) & 255 ) / 255.0f;
		i = ( ( cl >> 24 ) & 255 ) * 4.0f;
		cgi_R_AddLightToScene( cent->lerpOrigin, i, r, g, b );
	}
}

void CG_AddBoltOns( centity_t *cent, refEntity_t *refEnt )
{
	if ( !refEnt->hModel )
	{
		return;
	}

	if ( !cent->gent )
	{
		return;
	}

	if ( cent->gent->boltOn.index < 0 || cent->gent->boltOn.index >= numBoltOns )
	{//No boltOn
		return;
	}

	refEntity_t		boltOnRefEnt;

	if ( CG_ApplyBoltOnToRefEnt( &boltOnRefEnt, &knownBoltOns[cent->gent->boltOn.index], &cent->gent->boltOn, cent->lerpOrigin, refEnt ) )
	{
		cgi_R_AddRefEntityToScene( &boltOnRefEnt );
	}
}

void CG_AddRefEntWithTransportEffect ( centity_t *cent, refEntity_t *ent, qboolean boltOns )
{
	//NOTE: This WILL NOT work on BMODELS!!!  BMODELS cannot take custom shaders
	if ( cent->currentState.solid != SOLID_BMODEL && ((cent->gent->s.eFlags & EF_BEAM_IN) || (cent->gent->s.eFlags & EF_BEAM_OUT) || (cent->gent->s.eFlags & EF_SCAV_BEAM_OUT) || (cent->gent->s.eFlags & EF_DISINTEGRATION)) )
	{
		if ( cent->gent->s.eFlags & EF_BEAM_OUT && cent->gent->delay > cg.time + 2000 )
		{
			cgi_R_AddRefEntityToScene (ent);
			if ( boltOns )
			{
				CG_AddBoltOns( cent, ent );
			}
		}
		else if ( cent->gent->s.eFlags & EF_BEAM_IN && cent->gent->delay > 0 && cent->gent->delay < cg.time + 2000 )
		{
			cgi_R_AddRefEntityToScene (ent);
			if ( boltOns )
			{
				CG_AddBoltOns( cent, ent );
			}
		}

		// Clear the flags if necessary
		if ( cent->gent->s.eFlags & EF_BEAM_IN && cent->gent->delay < cg.time )
		{
			cent->gent->s.eFlags &= ~EF_BEAM_IN;
		}

		if ( cent->gent->s.eFlags & EF_BEAM_OUT && cent->gent->delay < cg.time )
		{
			cent->gent->s.eFlags &= ~EF_BEAM_OUT;
		}

		// We are beaming in or out, so it's ok to add in the beaming effect
		if ( cent->gent->s.eFlags & EF_DISINTEGRATION )
		{
			ent->customShader = cgs.media.disruptorShader;
		}
		else if ( cent->gent->s.eFlags & EF_SCAV_BEAM_OUT )
		{
			ent->customShader = cgs.media.scavTransportShader;
		}
		else
		{
			ent->customShader = cgs.media.transportShader;
		}
		ent->shaderTime = cent->gent->fx_time / 1000.0f;
		cgi_R_AddRefEntityToScene( ent );
		if ( boltOns )
		{
			CG_AddBoltOns( cent, ent );
		}
	}
	else
	{
		// We are a normal thing....
		cgi_R_AddRefEntityToScene (ent);
		if ( boltOns )
		{
			CG_AddBoltOns( cent, ent );
		}
	}
}

void CG_AddBorgEyeBeam( centity_t *cent, refEntity_t *ent )
{
	trace_t	trace;
	vec3_t	beamOrg, beamEnd, rgb = {1.0f,0.0f,0.0f};
	refEntity_t	temp;
	float	alpha;

	CG_PositionEntityOnTag( &temp, ent, ent->hModel, "tag_eye");

	VectorCopy( temp.origin, beamOrg );
	VectorMA( beamOrg, 1024, temp.axis[0], beamEnd);//forward to end

	cgi_CM_BoxTrace( &trace, beamOrg, beamEnd, NULL, NULL, 0, MASK_SHOT );
	VectorCopy(trace.endpos, beamEnd);
	alpha = 1.0f - (random() * 0.5);
	FX_AddLine( beamOrg, beamEnd, 1.0f, 0.35f + ( crandom() * 0.1 ), 0.0f, alpha, alpha, rgb,rgb,1.0f, cgs.media.whiteLaserShader );
	VectorMA(beamOrg, 0.5, ent->axis[0], beamOrg);//forward
	FX_AddSprite( beamOrg, NULL, NULL, 1.0f + (random() * 2.0f), 0.0f, 0.9f, 0.9f, 0.0f, 0.0f, 0.0f, cgs.media.borgEyeFlareShader );
	FX_AddQuad( beamEnd, trace.plane.normal, NULL, NULL, 2.0f + (crandom() * 1.0f), 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, cgs.media.borgEyeFlareShader );
}
/*
==================
CG_General
==================
*/
static void CG_General( centity_t *cent ) 
{
	refEntity_t			ent;
	entityState_t		*s1;

	s1 = &cent->currentState;

	// if set to invisible, skip
	if ( !s1->modelindex ) 
	{
		return;
	}

	memset (&ent, 0, sizeof(ent));

	// set frame

	
	if ( s1->eFlags & EF_ANIM_ONCE )
	{
		//s1->frame++;
		//ent.frame = s1->frame;
		ent.frame = cent->gent->s.frame;
		ent.renderfx|=RF_CAP_FRAMES;
	}
	else if ( s1->eFlags & EF_ANIM_ALLFAST )
	{
		ent.frame = (cg.time / 100);
		ent.renderfx|=RF_WRAP_FRAMES;
	}
	else
	{
		ent.frame = s1->frame;
	}
	ent.oldframe = ent.frame;
	ent.backlerp = 0;

	VectorCopy( cent->lerpOrigin, ent.origin);
	VectorCopy( cent->lerpOrigin, ent.oldorigin);

	ent.hModel = cgs.model_draw[s1->modelindex];

	if ( s1->eFlags & EF_AUTO_SIZE && cent->gent )
	{
		cgi_R_ModelBounds( ent.hModel, cent->gent->mins, cent->gent->maxs );
		//Only do this once
		cent->gent->s.eFlags &= ~EF_AUTO_SIZE;
	}

	// player model
	if (s1->number == cg.snap->ps.clientNum) 
	{
		ent.renderfx |= RF_THIRD_PERSON;	// only draw from mirrors
	}

	// convert angles to axis
	AnglesToAxis( cent->lerpAngles, ent.axis );

	if ( cent->gent->s.eFlags & EF_SCALE_UP )
	{
		for ( int i = 0; i < 3; i++ )
		{
			VectorScale( ent.axis[i], sin((cg.time - cent->gent->attackDebounceTime + 300) * 0.004f) * 0.2f + 1.0f, ent.axis[i] );
		}
	}

	CG_AddRefEntWithTransportEffect( cent, &ent, qtrue );

	// See if we are damaged and in the process of being fixed by a "fixit"
	if ( cent->gent->s.eFlags & EF_FIXING )
	{
		// The whole shader animation cycle lasts 4 seconds, though the model should be fixed after 2 secs.
		//	This is done so that the shader covers up as much of the popping between models as possible.
		if ( cent->gent->fx_time + 3000 < cg.time )
		{
			cent->gent->s.eFlags &= ~EF_FIXING;
			return;
		}

		// Add the custom shader onto what the undamaged model would be...heh heh.
		ent.hModel = cgi_R_RegisterModel( cent->gent->model );
		ent.customShader = cgs.media.fixitEffectShader;
		ent.shaderTime = cent->gent->fx_time / 1000.0f;
		cgi_R_AddRefEntityToScene (&ent);
	}

	if( cent->currentState.eFlags&EF_EYEBEAM )
	{
		CG_AddBorgEyeBeam( cent, &ent );
	}
}

/*
==================
CG_Speaker

Speaker entities can automatically play sounds
==================
*/
static void CG_Speaker( centity_t *cent ) {
	if ( ! cent->currentState.clientNum ) {	// FIXME: use something other than clientNum...
		return;		// not auto triggering
	}

	if ( cg.time < cent->miscTime ) {
		return;
	}

	cgi_S_StartSound (NULL, cent->currentState.number, CHAN_ITEM, cgs.sound_precache[cent->currentState.eventParm] );

	//	ent->s.frame = ent->wait * 10;
	//	ent->s.clientNum = ent->random * 10;
	cent->miscTime = (int)(cg.time + cent->currentState.frame * 100 + cent->currentState.clientNum * 100 * crandom());
}

/*
==================
CG_Item
==================
*/
static void CG_Item( centity_t *cent ) 
{
	refEntity_t			ent;
	entityState_t		*es;
	gitem_t				*item;
//	int					msec;
//	float				frac;
//	float				scale;

	es = &cent->currentState;
	if ( es->modelindex >= bg_numItems ) 
	{
		CG_Error( "Bad item index %i on entity", es->modelindex );
	}

	// if set to invisible, skip
	if ( !es->modelindex || ( es->eFlags & EF_NODRAW ) ) 
	{
		return;
	}

	item = &bg_itemlist[ es->modelindex ];

	if ( cg_simpleItems.integer ) 
	{
		memset( &ent, 0, sizeof( ent ) );
		ent.reType = RT_SPRITE;
		VectorCopy( cent->lerpOrigin, ent.origin );
		ent.radius = 14;
		ent.customShader = cg_items[es->modelindex].icon;
		ent.shaderRGBA[0] = 255;
		ent.shaderRGBA[1] = 255;
		ent.shaderRGBA[2] = 255;
		ent.shaderRGBA[3] = 255;
		cgi_R_AddRefEntityToScene(&ent);
		return;
	}

	// items bob up and down continuously
//	scale = 0.005f + cent->currentState.number * 0.00001f;
//	cent->lerpOrigin[2] += (float)(4 + cos( ( cg.time + 1000 ) *  scale ) * 4);

	memset (&ent, 0, sizeof(ent));


	// autorotate at one of two speeds
//	if ( item->giType == IT_HEALTH ) {
//		VectorCopy( cg.autoAnglesFast, cent->lerpAngles );
//		AxisCopy( cg.autoAxisFast, ent.axis );
//	} else {
//		VectorCopy( cg.autoAngles, cent->lerpAngles );
//		AxisCopy( cg.autoAxis, ent.axis );
//	}


	// the weapons have their origin where they attatch to player
	// models, so we need to offset them or they will rotate
	// eccentricly
//	if ( item->giType == IT_WEAPON ) {
//		weaponInfo_t	*wi;
//
//		wi = &cg_weapons[item->giTag];
//		cent->lerpOrigin[0] -= 
//			wi->weaponMidpoint[0] * ent.axis[0][0] +
//			wi->weaponMidpoint[1] * ent.axis[1][0] +
//			wi->weaponMidpoint[2] * ent.axis[2][0];
//		cent->lerpOrigin[1] -= 
//			wi->weaponMidpoint[0] * ent.axis[0][1] +
//			wi->weaponMidpoint[1] * ent.axis[1][1] +
//			wi->weaponMidpoint[2] * ent.axis[2][1];
//		cent->lerpOrigin[2] -= 
//			wi->weaponMidpoint[0] * ent.axis[0][2] +
//			wi->weaponMidpoint[1] * ent.axis[1][2] +
//			wi->weaponMidpoint[2] * ent.axis[2][2];

//		cent->lerpOrigin[2] += 8;	// an extra height boost
//	}
	vec3_t spinAngles;

	//AxisClear( ent.axis );
	VectorCopy( cent->gent->s.angles, spinAngles );

	ent.hModel = cg_items[es->modelindex].models;

	VectorCopy( cent->lerpOrigin, ent.origin);
	VectorCopy( cent->lerpOrigin, ent.oldorigin);

	ent.nonNormalizedAxes = qfalse;

	// lovely...this is for weapons that should be oriented vertically.  For weapons lockers and such.
	if ( cent->gent->spawnflags & 16 )
	{	//VectorClear( spinAngles );
		spinAngles[PITCH] = -80;
	}

	AnglesToAxis( spinAngles, ent.axis );

	// items without glow textures need to keep a minimum light value
	// so they are always visible
	if (( item->giType == IT_WEAPON ) || ( item->giType == IT_ARMOR )) 
	{
		ent.renderfx |= RF_MINLIGHT;
	}

	// increase the size of the weapons when they are presented as items
//	if ( item->giType == IT_WEAPON ) {
//		VectorScale( ent.axis[0], 1.5f, ent.axis[0] );
//		VectorScale( ent.axis[1], 1.5f, ent.axis[1] );
//		VectorScale( ent.axis[2], 1.5f, ent.axis[2] );
//		ent.nonNormalizedAxes = qtrue;
//	}

	// add to refresh list
	cgi_R_AddRefEntityToScene(&ent);
}

//============================================================================

/*
===============
CG_Missile
===============
*/
static void CG_Missile( centity_t *cent ) {
	refEntity_t			ent;
	entityState_t		*s1;
	const weaponInfo_t	*weapon;
	const weaponData_t  *wData;

	if ( !cent->gent->inuse )
		return;

	s1 = &cent->currentState;
	if ( s1->weapon >= WP_NUM_WEAPONS ) {
		s1->weapon = 0;
	}
	weapon = &cg_weapons[s1->weapon];
	wData = &weaponData[s1->weapon];

	// calculate the axis
	VectorCopy( s1->angles, cent->lerpAngles);

	if ( cent->gent->alt_fire )
	{
		// add trails
		if ( weapon->alt_missileTrailFunc )  
			weapon->alt_missileTrailFunc( cent, weapon );

		// add dynamic light
		if ( wData->alt_missileDlight )
				cgi_R_AddLightToScene(cent->lerpOrigin, wData->alt_missileDlight, 
					wData->alt_missileDlightColor[0], wData->alt_missileDlightColor[1], wData->alt_missileDlightColor[2] );

		// add missile sound
		if ( weapon->alt_missileSound )
			cgi_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, vec3_origin, weapon->alt_missileSound );

		//Don't draw something without a model
		if ( weapon->alt_missileModel == NULL )
			return;
	}
	else
	{
		// add trails
		if ( weapon->missileTrailFunc )  
			weapon->missileTrailFunc( cent, weapon );

		// add dynamic light
		if ( wData->missileDlight )
			cgi_R_AddLightToScene(cent->lerpOrigin, wData->missileDlight, 
				wData->missileDlightColor[0], wData->missileDlightColor[1], wData->missileDlightColor[2] );

		// add missile sound
		if ( weapon->missileSound )
			cgi_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, vec3_origin, weapon->missileSound );

		//Don't draw something without a model
		if ( weapon->missileModel == NULL )
			return;
	}

	// create the render entity
	memset (&ent, 0, sizeof(ent));
	VectorCopy( cent->lerpOrigin, ent.origin);
	VectorCopy( cent->lerpOrigin, ent.oldorigin);

	// flicker between two skins
	ent.skinNum = cg.clientFrame & 1;
	ent.renderfx = /*weapon->missileRenderfx | */RF_NOSHADOW;

	if ( cent->gent->alt_fire )
		ent.hModel = weapon->alt_missileModel;
	else
		ent.hModel = weapon->missileModel;

	// convert direction of travel into axis
	if ( VectorNormalize2( s1->pos.trDelta, ent.axis[0] ) == 0 ) {
		ent.axis[0][2] = 1;
	}

	// spin as it moves
	if ( s1->pos.trType != TR_STATIONARY )
	{
		if ( s1->eFlags & EF_MISSILE_STICK )
			RotateAroundDirection( ent.axis, cg.time * 0.5f );//Did this so regular missiles don't get broken
		else
			RotateAroundDirection( ent.axis, cg.time * 0.25f );//JFM:FLOAT FIX
	}
	else
	{
		if ( s1->eFlags & EF_MISSILE_STICK )
			RotateAroundDirection( ent.axis, (float)s1->pos.trTime * 0.5f );
		else
			RotateAroundDirection( ent.axis, (float)s1->time );
	}

	// add to refresh list, possibly with quad glow
	CG_AddRefEntityWithPowerups( &ent, s1->powerups, NULL );
}

/*
===============
CG_Mover

NOTE: Using customShader, can we have triggerable/scripted shader changes on brushes?
===============
*/

#define DOOR_OPENING	1
#define DOOR_CLOSING	2
#define DOOR_OPEN		3
#define DOOR_CLOSED		4

static void CG_Mover( centity_t *cent ) {
	refEntity_t			ent;
	entityState_t		*s1;

	s1 = &cent->currentState;

	// create the render entity
	memset (&ent, 0, sizeof(ent));
	//FIXME: why are these always 0, 0, 0???!
	VectorCopy( cent->lerpOrigin, ent.origin);
	VectorCopy( cent->lerpOrigin, ent.oldorigin);
	AnglesToAxis( cent->lerpAngles, ent.axis );

	ent.renderfx = RF_NOSHADOW;

	// flicker between two skins (FIXME?)
	ent.skinNum = ( cg.time >> 6 ) & 1;

	// get the model, either as a bmodel or a modelindex
	if ( s1->solid == SOLID_BMODEL ) {
		ent.hModel = cgs.inlineDrawModel[s1->modelindex];
	} else {
		ent.hModel = cgs.model_draw[s1->modelindex];
	}

	// If there isn't an hModel for this mover, an RGB axis model will get drawn.
	if ( !ent.hModel )
	{
		return;
	}

	// add the secondary model
	if ( s1->solid == SOLID_BMODEL && s1->modelindex2 ) 
	{
		vec3_t	org;

		if ( !(s1->eFlags & EF_NODRAW) )
		{
			// add to refresh list
			CG_AddRefEntWithTransportEffect( cent, &ent, qfalse );
		}

		VectorAdd(cent->gent->absmin, cent->gent->absmax, org);
		VectorScale(org, 0.5, org);
		VectorCopy( org, ent.origin);
		VectorCopy( org, ent.oldorigin);

		if (s1->eFlags & EF_ANIM_ONCE)
		{//FIXME: needs to anim at once per 100 ms
			ent.frame = cent->gent->s.frame;
			ent.renderfx|=RF_CAP_FRAMES;
		}
		else if (s1->eFlags & EF_ANIM_ALLFAST)
		{
			ent.frame = (cg.time / 100);
			ent.renderfx|=RF_WRAP_FRAMES;
		}
		else
		{
			ent.frame = s1->frame;
		}

		ent.skinNum = 0;
		ent.hModel = cgs.model_draw[s1->modelindex2];

		// special case for fading out stasis doors
		if ( cent->gent->svFlags & SVF_STASIS_DOOR )
		{
			// Open stasis doors never get added into the scene
			if ( cent->gent->count != DOOR_OPEN )
			{
				int		clr = 255;
	
				if ( cent->gent->count == DOOR_CLOSING || cent->gent->count == DOOR_OPENING )
				{
					clr = (cent->gent->fx_time - cg.time) / 1000.0f * 255.0f;

					// quick clamp
					if ( clr < 0 ) 
						clr = 0;
					if ( clr > 255 )
						clr = 255;

					if ( cent->gent->count == DOOR_CLOSING )
					{
						// Fade in the opposite direction
						clr = 255 - clr;
					}
				}

				AnglesToAxis( cent->gent->s.angles, ent.axis );
 
				ent.shaderRGBA[3] = clr;
				cgi_R_AddRefEntityToScene(&ent);

				if ( cent->gent->count == DOOR_CLOSING || cent->gent->count == DOOR_OPENING )
				{
					ent.shaderRGBA[0] = 
					ent.shaderRGBA[1] = 
					ent.shaderRGBA[2] = clr;
					ent.shaderRGBA[3] = 255;

					ent.customShader = cgs.media.fixitEffectShader;
					// Nasty time hacking
					if ( cent->gent->count == DOOR_CLOSING )
						ent.shaderTime = (cent->gent->fx_time + 1800) * 0.001;
					else
						ent.shaderTime = (cent->gent->fx_time + 200) * 0.001;

					cgi_R_AddRefEntityToScene(&ent);
				}
			}
		}
		else
		{
			CG_AddRefEntWithTransportEffect( cent, &ent, qtrue );
		}
	}
	else
	{//We're a normal model being moved, animate our model1
		if (s1->eFlags & EF_ANIM_ONCE)
		{//FIXME: needs to anim at once per 100 ms
			ent.frame = cent->gent->s.frame;
			ent.renderfx|=RF_CAP_FRAMES;
		}
		else if (s1->eFlags & EF_ANIM_ALLFAST)
		{
			ent.frame = (cg.time / 100);
			ent.renderfx|=RF_WRAP_FRAMES;
		}
		else
		{
			ent.frame = s1->frame;
		}

		ent.skinNum = 0;

		if ( !(s1->eFlags & EF_NODRAW) )
		{
			// add to refresh list
			CG_AddRefEntWithTransportEffect( cent, &ent, qtrue );
		}
	}
}

/*
===============
CG_Beam

Also called as an event
===============
*/
void CG_Beam( centity_t *cent, int color ) {
	refEntity_t			ent;
	entityState_t		*s1;

	s1 = &cent->currentState;

	// create the render entity
	memset (&ent, 0, sizeof(ent));
	VectorCopy( s1->pos.trBase, ent.origin );
	VectorCopy( s1->origin2, ent.oldorigin );
	AxisClear( ent.axis );
	ent.reType = RT_BEAM;
	ent.skinNum = color;

	ent.renderfx = RF_NOSHADOW;

	// add to refresh list
	cgi_R_AddRefEntityToScene(&ent);
}

/*
===============
CG_Cylinder
===============
*/
void CG_Cylinder( vec3_t start, vec3_t end, float radius, vec3_t color ) 
{
	vec3_t	dir;
	float	length;

	VectorSubtract( end, start, dir );
	length = VectorNormalize( dir );

	FX_AddCylinder( start, 
					dir, 
					length, 
					0.0f, 
					radius,
					0.0f,
					radius,
					0.0f,
					1.0f,
					1.0f,
					color,
					color,
					100.0f,
					cgs.media.waterDropShader );
}

void CG_Cube( vec3_t mins, vec3_t maxs, vec3_t color, float alpha ) 
{
	vec3_t	point1, point2, point3, point4;
	int		vec[3];
	int		axis, i;

	for ( axis = 0, vec[0] = 0, vec[1] = 1, vec[2] = 2; axis < 3; axis++, vec[0]++, vec[1]++, vec[2]++ )
	{
		for ( i = 0; i < 3; i++ )
		{
			if ( vec[i] > 2 )
			{
				vec[i] = 0;
			}
		}

		point1[vec[1]] = mins[vec[1]];
		point1[vec[2]] = mins[vec[2]];

		point2[vec[1]] = mins[vec[1]];
		point2[vec[2]] = maxs[vec[2]];

		point3[vec[1]] = maxs[vec[1]];
		point3[vec[2]] = maxs[vec[2]];
		
		point4[vec[1]] = maxs[vec[1]];
		point4[vec[2]] = mins[vec[2]];

		//- face
		point1[vec[0]] = point2[vec[0]] = point3[vec[0]] = point4[vec[0]] = mins[vec[0]];

		FX_AddTri(point1, point2, point3, alpha, alpha, color, color, 100, cgs.media.whiteShader, FXF_USE_ALPHA_CHAN );
		FX_AddTri(point1, point4, point3, alpha, alpha, color, color, 100, cgs.media.whiteShader, FXF_USE_ALPHA_CHAN );

		//+ face
		point1[vec[0]] = point2[vec[0]] = point3[vec[0]] = point4[vec[0]] = maxs[vec[0]];

		FX_AddTri(point1, point2, point3, alpha, alpha, color, color, 100, cgs.media.whiteShader, FXF_USE_ALPHA_CHAN );
		FX_AddTri(point1, point4, point3, alpha, alpha, color, color, 100, cgs.media.whiteShader, FXF_USE_ALPHA_CHAN );
	}
}

void CG_Line( vec3_t start, vec3_t end, vec3_t color, float alpha ) 
{
	FX_AddLine( start,
				end,
				1.0f,
				1.0,
				1.0f,
				alpha,
				alpha,
				color, 
				color, 
				100.0f,
				cgs.media.whiteShader );
}

/*
===============
CG_Portal
===============
*/
static void CG_Portal( centity_t *cent ) {
	refEntity_t			ent;
	entityState_t		*s1;

	s1 = &cent->currentState;

	//FIXME: this tends to give a bad axis[1], perhaps we
	//should just do the VectorSubtraction here rather than
	//on the game side.  Would also allow camera to follow
	//a moving target.

	// create the render entity
	memset (&ent, 0, sizeof(ent));
	VectorCopy( cent->lerpOrigin, ent.origin );
	VectorCopy( s1->origin2, ent.oldorigin );
	ByteToDir( s1->eventParm, ent.axis[0] );
	PerpendicularVector( ent.axis[1], ent.axis[0] );

	// negating this tends to get the directions like they want
	// we really should have a camera roll value
	VectorSubtract( vec3_origin, ent.axis[1], ent.axis[1] );

	CrossProduct( ent.axis[0], ent.axis[1], ent.axis[2] );
	ent.reType = RT_PORTALSURFACE;
	ent.frame = s1->frame;		// rotation speed
	ent.skinNum = (int)(s1->clientNum/256.0 * 360);	// roll offset

	// add to refresh list
	cgi_R_AddRefEntityToScene(&ent);
}


/*
=========================
CG_AdjustPositionForMover

Also called by client movement prediction code
=========================
*/
void CG_AdjustPositionForMover( const vec3_t in, int moverNum, int atTime, vec3_t out ) {
	centity_t	*cent;
	vec3_t	oldOrigin, origin, deltaOrigin;
	vec3_t	oldAngles, angles, deltaAngles;

	if ( moverNum <= 0 ) {
		VectorCopy( in, out );
		return;
	}

	cent = &cg_entities[ moverNum ];
	if ( cent->currentState.eType != ET_MOVER ) {
		VectorCopy( in, out );
		return;
	}

	EvaluateTrajectory( &cent->currentState.pos, cg.snap->serverTime, oldOrigin );
	EvaluateTrajectory( &cent->currentState.apos, cg.snap->serverTime, oldAngles );

	EvaluateTrajectory( &cent->currentState.pos, atTime, origin );
	EvaluateTrajectory( &cent->currentState.apos, atTime, angles );

	VectorSubtract( origin, oldOrigin, deltaOrigin );
	VectorSubtract( angles, oldAngles, deltaAngles );

	VectorAdd( in, deltaOrigin, out );

	// FIXME: origin change when on a rotating object
}


/*
===============
CG_CalcEntityLerpPositions

===============
*/
extern char	*vtos( const vec3_t v );
void CG_CalcEntityLerpPositions( centity_t *cent ) {
	if ( cent->currentState.number == cg.snap->ps.clientNum)
	{
		// if the player, take position from prediction
		VectorCopy( cg.predicted_player_state.origin, cent->lerpOrigin );
		VectorCopy( cg.predicted_player_state.viewangles, cent->lerpAngles );
		return;
	}
	
	if ( cent->interpolate && cent->currentState.pos.trType == TR_INTERPOLATE ) {
		// if the entity has a valid next state, interpolate a value between the frames
		// unless it is a mover with a known start and stop
		vec3_t		current, next;
		float		f;

		// it would be an internal error to find an entity that interpolates without
		// a snapshot ahead of the current one
		if ( cg.nextSnap == NULL ) {
			CG_Error( "CG_AddCEntity: cg.nextSnap == NULL" );
		}

		f = cg.frameInterpolation;

		// this will linearize a sine or parabolic curve, but it is important
		// to not extrapolate player positions if more recent data is available
		EvaluateTrajectory( &cent->currentState.pos, cg.snap->serverTime, current );
		EvaluateTrajectory( &cent->nextState.pos, cg.nextSnap->serverTime, next );

		cent->lerpOrigin[0] = current[0] + f * ( next[0] - current[0] );
		cent->lerpOrigin[1] = current[1] + f * ( next[1] - current[1] );
		cent->lerpOrigin[2] = current[2] + f * ( next[2] - current[2] );

		/*
		if(cent->gent && cent->currentState.eFlags & EF_NPC && !VectorCompare(current, next))
		{
			Com_Printf("%s last/next/lerp pos %s/%s/%s, f = %4.2f\n", cent->gent->script_targetname, vtos(current), vtos(next), vtos(cent->lerpOrigin), f);
		}
		*/
		EvaluateTrajectory( &cent->currentState.apos, cg.snap->serverTime, current );
		EvaluateTrajectory( &cent->nextState.apos, cg.nextSnap->serverTime, next );

		cent->lerpAngles[0] = LerpAngle( current[0], next[0], f );
		cent->lerpAngles[1] = LerpAngle( current[1], next[1], f );
		cent->lerpAngles[2] = LerpAngle( current[2], next[2], f );

		/*
		if(cent->gent && cent->currentState.eFlags & EF_NPC && !VectorCompare(current, next))
		{
			Com_Printf("%s last/next/lerp apos %s/%s/%s, f = %4.2f\n", cent->gent->script_targetname, vtos(current), vtos(next), vtos(cent->lerpAngles), f);
		}
		*/
		return;
	}
	
	// FIXME: if it's blocked, it wigs out, draws it in a predicted spot, but never
	// makes it there - we need to predict it in the right place if this is happens...

	// just use the current frame and evaluate as best we can
	trajectory_t *posData = &cent->currentState.pos;
	{
		gentity_t *ent = &g_entities[cent->currentState.number];

		if ( ent )
		{
			if ( ent->s.eFlags & EF_BLOCKED_MOVER || ent->s.pos.trType == TR_STATIONARY )
			{//this mover has stopped moving and is going to wig out if we predict it
				//based on last frame's info- cut across the network and use the currentOrigin
				VectorCopy( ent->currentOrigin, cent->lerpOrigin );
				posData = NULL;
			}
			else
			{
				posData = &ent->s.pos;
			}
		}
	}

	if ( posData )
	{
		EvaluateTrajectory( posData, cg.time, cent->lerpOrigin );
	}
	EvaluateTrajectory( &cent->currentState.apos, cg.time, cent->lerpAngles );

	// adjust for riding a mover
	CG_AdjustPositionForMover( cent->lerpOrigin, cent->currentState.groundEntityNum, cg.time, cent->lerpOrigin );

	// FIXME: perform general error decay?
}

/*
===============
CG_AddLocalSet
===============
*/

static void CG_AddLocalSet( centity_t *cent )
{
	cent->gent->setTime = cgi_S_AddLocalSet( cent->gent->soundSet, cg.refdef.vieworg, cent->lerpOrigin, cent->gent->s.number, cent->gent->setTime );
}

/*
-------------------------
CAS_GetBModelSound
-------------------------
*/

sfxHandle_t CAS_GetBModelSound( const char *name, int stage )
{
	return cgi_AS_GetBModelSound( name, stage );
}

void CG_DLightThink ( centity_t *cent )
{
	if(cent->gent)
	{
		float	tDelta = cg.time - cent->gent->painDebounceTime;
		float	percentage = ( tDelta/((float)cent->gent->speed) );
		vec3_t	org;
		vec4_t	currentRGBA;
		gentity_t	*owner = NULL;
		int		i;
		
		if ( percentage >= 1.0f )
		{//We hit the end
			percentage = 1.0f;
			switch( cent->gent->pushDebounceTime )
			{
			case 0://Fading from start to final
				if ( cent->gent->spawnflags & 8 )
				{//PULSER
					if ( tDelta - cent->gent->speed - cent->gent->wait >= 0 )
					{//Time to start fading down
						cent->gent->painDebounceTime = cg.time;
						cent->gent->pushDebounceTime = 1;
						percentage = 0.0f;
					}
				}
				else
				{//Stick on startRGBA
					percentage = 0.0f;
				}
				break;
			case 1://Fading from final to start
				if ( tDelta - cent->gent->speed - cent->gent->radius >= 0 )
				{//Time to start fading up
					cent->gent->painDebounceTime = cg.time;
					cent->gent->pushDebounceTime = 0;
					percentage = 0.0f;
				}
				break;
			case 2://Fading from 0 intensity to start intensity
				//Time to start fading from start to final
				cent->gent->painDebounceTime = cg.time;
				cent->gent->pushDebounceTime = 0;
				percentage = 0.0f;
				break;
			case 3://Fading from current intensity to 0 intensity
				//Time to turn off
				cent->gent->misc_dlight_active = qfalse;
				cent->gent->e_clThinkFunc = clThinkF_NULL;
				cent->gent->s.eType = ET_GENERAL;
				cent->gent->svFlags &= ~SVF_BROADCAST;
				return;		
				break;
			default:
				break;
			}
		}

		switch( cent->gent->pushDebounceTime )
		{
		case 0://Fading from start to final
			for ( i = 0; i < 4; i++ )
			{
				currentRGBA[i] = cent->gent->startRGBA[i] + ( (cent->gent->finalRGBA[i] - cent->gent->startRGBA[i]) * percentage );
			}
			break;
		case 1://Fading from final to start
			for ( i = 0; i < 4; i++ )
			{
				currentRGBA[i] = cent->gent->finalRGBA[i] + ( (cent->gent->startRGBA[i] - cent->gent->finalRGBA[i]) * percentage );
			}
			break;
		case 2://Fading from 0 intensity to start
			for ( i = 0; i < 3; i++ )
			{
				currentRGBA[i] = cent->gent->startRGBA[i];
			}
			currentRGBA[3] = cent->gent->startRGBA[3] * percentage;
			break;
		case 3://Fading from current intensity to 0
			for ( i = 0; i < 3; i++ )
			{//FIXME: use last
				currentRGBA[i] = cent->gent->startRGBA[i];
			}
			currentRGBA[3] = cent->gent->startRGBA[3] - (cent->gent->startRGBA[3] * percentage);
			break;
		default:
			return;
			break;
		}

		if ( cent->gent->owner )
		{
			owner = cent->gent->owner;
		}
		else
		{
			owner = cent->gent;
		}

		if ( owner->s.pos.trType == TR_INTERPOLATE )
		{
			VectorCopy( cg_entities[owner->s.number].lerpOrigin, org );
		}
		else
		{
			VectorCopy( owner->currentOrigin, org );
		}

		cgi_R_AddLightToScene(org, currentRGBA[3]*10, currentRGBA[0], currentRGBA[1], currentRGBA[2] );
	}
}

void CG_StasisFixitsThink ( centity_t *cent )
{
	FX_StasisAmbientThings( cent );
}

/*
CG_StasisFlierAttack
Temp effect- copy of stasis projectiles
*/
void CG_StasisFlierAttackThink ( centity_t *cent )
{
	if ( cent->gent == NULL )
	{
		return;
	}

	//FX_AddSprite( cent->lerpOrigin, NULL, NULL, 36.0f + ( random() * 16.0f ), 0.0f, 1.0f, 0.5f, 0, 0.0f, 25, cgs.media.blueParticleShader);
	FX_AddSprite( cent->lerpOrigin, NULL, NULL, 12.0f + ( random() * 8.0f ), 0.0f, 1.0f, 0.5f, 0, 0.0f, 25, cgs.media.blueParticleShader);
	
	vec3_t	dir, end;

	for ( int i = 0; i < 3; i++ )
	{
		if ( random() > 0.4 )
		{
			for ( int j = 0; j < 3; j++ )
				dir[j] = crandom();

			//VectorMA( cent->lerpOrigin, 56.0f + (random() * 16.0f), dir, end );
			VectorMA( cent->lerpOrigin, 20.0f + (random() * 10.0f), dir, end );

			FX_AddElectricity( cent->lerpOrigin, end, 1.0f, 1.0f + ( random() * 2.0f ), 0.0f, 0.7f, 0.0f, 50.0f, 
						cgs.media.stasisBoltShader, FXF_TAPER );
		}
	}
}

void CG_StasisFlierIdleThink ( centity_t *cent )
{
	if ( cent->gent == NULL )
	{
		return;
	}

	// very simple effect for when the trailing orbs are chasing their parent
	FX_AddSprite( cent->lerpOrigin, NULL, NULL, 6.0f + ( random() * 4.0f ), 0.0f, 1.0f, 0.5f, 0, 0.0f, 25, cgs.media.blueParticleShader);
}

void CG_StasisFlierChildDeath ( vec3_t pos )
{
	vec3_t	spawnPosition;
	float	scale;
	vec3_t	vel = { 0, 0, 20 };

	for ( int i = 0; i < 8; i++ )
	{
		VectorCopy( pos, spawnPosition );
		for ( int j = 0; j < 3; j++ )
		{
			spawnPosition[j] += random() * 5.0f;
		}
		scale =3.0f + ( random() * 2.0f );

		FX_AddSprite( spawnPosition, vel, NULL, scale, scale / -1.0f, 1.0f, 1.0f, 0, 0.0f, 1000, cgs.media.blueParticleShader);
	}
}


void CG_Assimilator ( centity_t *cent )
{
	vec3_t	start1, start2, forward, right, targetSpot;

	if ( !cent->gent->enemy || !cent->gent->enemy->client )
	{
		cent->gent->e_clThinkFunc = clThinkF_NULL;
		return;
	}

	//find 2 points, draw the tubes to enemy
	VectorCopy( cent->lerpOrigin, start1 );
	AngleVectors( cent->lerpAngles, forward, right, NULL );
	VectorMA( start1, -6, forward, start1 );
	start1[2] -= 15;
	VectorCopy( start1, start2 );
	VectorMA( start1, 12, right, start1 );
	VectorMA( start2, -12, right, start2 );

	VectorCopy( cent->gent->enemy->client->renderInfo.headPoint, targetSpot );
	targetSpot[2] -= 8;

	VectorMA( targetSpot, 2, right, targetSpot );
	CG_AssimilationTubules( start1, targetSpot, forward, 3.0f );
	VectorMA( targetSpot, -4, right, targetSpot );
	CG_AssimilationTubules( start2, targetSpot, forward, 3.0f );
}

static void CG_Think ( centity_t *cent )
{
	if(!cent->gent)
	{
		return;
	}

	CEntity_ThinkFunc(cent);	//	cent->gent->clThink(cent);
}

/*
===============
CG_AddCEntity

===============
*/
static void CG_AddCEntity( centity_t *cent ) 
{
	// event-only entities will have been dealt with already
	if ( cent->currentState.eType >= ET_EVENTS ) {
		return;
	}

	//we must have restarted the game
	if (!cent->gent)
	{
		return;
	}

	// calculate the current origin
	CG_CalcEntityLerpPositions( cent );

	// add automatic effects
	CG_EntityEffects( cent );

	// add local sound set if any
	if ( cent->gent && cent->gent->soundSet && cent->gent->soundSet[0] && cent->currentState.eType != ET_MOVER )
		CG_AddLocalSet( cent );

	switch ( cent->currentState.eType ) {
	default:
		CG_Error( "Bad entity type: %i\n", cent->currentState.eType );
		break;
	case ET_INVISIBLE:
	case ET_PUSH_TRIGGER:
	case ET_TELEPORT_TRIGGER:
		break;
	case ET_GENERAL:
		CG_General( cent );
		break;
	case ET_PLAYER:
		CG_Player( cent );
		break;
	case ET_ITEM:
		CG_Item( cent );
		break;
	case ET_MISSILE:
		CG_Missile( cent );
		break;
	case ET_MOVER:
		CG_Mover( cent );
		break;
	case ET_BEAM:
		CG_Beam( cent, 0 );
		break;
	case ET_PORTAL:
		CG_Portal( cent );
		break;
	case ET_SPEAKER:
		if ( cent->gent && cent->gent->soundSet && cent->gent->soundSet[0] )
		{
			break;
		}
		CG_Speaker( cent );
		break;
	case ET_THINKER:
		CG_General( cent );
		CG_Think( cent );
		break;
	}
}

/*
===============
CG_AddPacketEntities

===============
*/
void CG_AddPacketEntities( void ) {
	int					num;
	centity_t			*cent;
	playerState_t		*ps;

	// set cg.frameInterpolation
	if ( cg.nextSnap ) {
		int		delta;

		delta = (cg.nextSnap->serverTime - cg.snap->serverTime);
		if ( delta == 0 ) {
			cg.frameInterpolation = 0;
		} else {
			cg.frameInterpolation = (float)( cg.time - cg.snap->serverTime ) / delta;
		}
	} else {
		cg.frameInterpolation = 0;	// actually, it should never be used, because 
									// no entities should be marked as interpolating
	}

	// the auto-rotating items will all have the same axis
	cg.autoAngles[0] = 0;
	cg.autoAngles[1] = ( cg.time & 2047 ) * 360 / 2048.0f;
	cg.autoAngles[2] = 0;

	cg.autoAnglesFast[0] = 0;
	cg.autoAnglesFast[1] = ( cg.time & 1023 ) * 360 / 1024.0f;
	cg.autoAnglesFast[2] = 0;

	AnglesToAxis( cg.autoAngles, cg.autoAxis );
	AnglesToAxis( cg.autoAnglesFast, cg.autoAxisFast );

	// generate and add the entity from the playerstate
	ps = &cg.predicted_player_state;
	PlayerStateToEntityState( ps, &cg_entities[ ps->clientNum ].currentState );
//	cent = &cg_entities[ ps->clientNum ];	// not needed now that player is in the snap packet
//	CG_AddCEntity( cent );					//

	// add each entity sent over by the server
	for ( num = 0 ; num < cg.snap->numEntities ; num++ ) {
		cent = &cg_entities[ cg.snap->entities[ num ].number ];
		CG_AddCEntity( cent );
	}
}

