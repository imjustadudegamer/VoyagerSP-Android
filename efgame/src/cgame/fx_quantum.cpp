// Quantum Burst FX

#include "cg_local.h"
#include "FX_Public.h"

/*
-------------------------
FX_QuantumThink
-------------------------
*/

void FX_QuantumThink( centity_t *cent, const struct weaponInfo_s *weapon )
{
	vec3_t	line1end, line2end, axis[3], rgb;

	AxisClear( axis ); 

	// convert direction of travel into axis
	if ( VectorNormalize2( cent->currentState.pos.trDelta, axis[0] ) == 0 ) {
		axis[0][2] = 1;
	}

	// spin as it moves
	RotateAroundDirection( axis, cg.time * 0.3f );// * 1.25f );

	VectorMA( cent->lerpOrigin, -24.0f, axis[1], line1end );
	VectorMA( cent->lerpOrigin,  24.0f, axis[1], line2end );
	FX_AddLine( line1end, line2end, 1.0f, random() * 6 + 2, 0.0f, 0.2 + random() * 0.2, 0.0f, 1, cgs.media.yellowParticleShader );

	AxisClear( axis ); 

	// convert direction of travel into axis
	if ( VectorNormalize2( cent->currentState.pos.trDelta, axis[0] ) == 0 ) {
		axis[0][2] = 1;
	}

	// spin as it moves
	RotateAroundDirection( axis, -cg.time * 0.3f );// * 1.25f );

	VectorMA( cent->lerpOrigin, -48.0f, axis[2], line1end );
	VectorMA( cent->lerpOrigin,  48.0f, axis[2], line2end );
	FX_AddLine( line1end, line2end, 1.0f, random() * 5 + 2, 0.0f, 0.1 + random() * 0.2, 0.0f, 1, cgs.media.yellowParticleShader );

	VectorSet( rgb, 1.0f, 0.45f, 0.15f ); // orange

	FX_AddSprite( cent->lerpOrigin, NULL, NULL, random() * 60 + 30, 4, 0.5f, 0.0f, 0, 0.0f, 1.0f, cgs.media.orangeParticleShader );	
	FX_AddSprite( cent->lerpOrigin, NULL, NULL, random() * 10 + 60, 0.0f, 0.1f, 0.1f, rgb, rgb, 0.0f, 0.0f, 1, cgs.media.whiteRingShader );
	FX_AddSprite( cent->lerpOrigin, NULL, NULL, random() * 16 + 8, 4, 0.5f, 0.0f, 0, 0.0f, 1.0f, cgs.media.yellowParticleShader );	
}

/*
-------------------------
FX_QuantumAltThink
-------------------------
*/
void FX_QuantumAltThink( centity_t *cent, const struct weaponInfo_s *weapon )
{
	vec3_t	line1end, line2end, axis[3], rgb, vel;
	float	scale;

	AxisClear( axis ); 

	// convert direction of travel into axis
	if ( VectorNormalize2( cent->currentState.pos.trDelta, axis[0] ) == 0 ) {
		axis[0][2] = 1;
	}

	// spin as it moves
	RotateAroundDirection( axis, cg.time * 0.3f );// * 1.25f );

	VectorMA( cent->lerpOrigin, -48.0f, axis[1], line1end );
	VectorMA( cent->lerpOrigin,  48.0f, axis[1], line2end );
	FX_AddLine( line1end, line2end, 1.0f, random() * 6 + 2, 0.0f, 0.2 + random() * 0.2, 0.0f, 1, cgs.media.yellowParticleShader );

	VectorMA( cent->lerpOrigin, -48.0f, axis[2], line1end );
	VectorMA( cent->lerpOrigin,  48.0f, axis[2], line2end );
	FX_AddLine( line1end, line2end, 1.0f, random() * 5 + 2, 0.0f, 0.2 + random() * 0.2, 0.0f, 1, cgs.media.yellowParticleShader );

	VectorSet( rgb, 1.0f, 0.45f, 0.15f ); // orange

	FX_AddSprite( cent->lerpOrigin, NULL, NULL, random() * 60 + 30, 4, 0.5f, 0.0f, 0, 0.0f, 1.0f, cgs.media.orangeParticleShader );	
	FX_AddSprite( cent->lerpOrigin, NULL, NULL, random() * 10 + 60, 0.0f, 0.1f, 0.1f, rgb, rgb, 0.0f, 0.0f, 1, cgs.media.whiteRingShader );
	FX_AddSprite( cent->lerpOrigin, NULL, NULL, random() * 16 + 8, 4, 0.5f, 0.0f, 0, 0.0f, 1.0f, cgs.media.yellowParticleShader );	

	scale = ( 2.0f + cos( cg.time * ( M_PI * 0.001f * 4 ))) * 0.5f;

	// Unlike the main fire, I'm leaving around this center core for a moment as a trail...
	VectorScale( cent->currentState.pos.trDelta, 0.25f, vel );
	FX_AddSprite( cent->lerpOrigin, NULL, NULL, scale * 8 + 2, scale * -5.0f, 0.8f, 0.0f, 0, 0, 300.0f, cgs.media.sunnyFlareShader );

	// Tack on a sprite trail so we can see the cool tracking at work.
	VectorSet( vel, Q_flrand(-12, 12), Q_flrand(-12, 12), Q_flrand(-12, 12) );
	VectorMA( vel, 0.25f, cent->currentState.pos.trDelta, vel );

	if ( rand() & 1 )
		FX_AddSprite( cent->lerpOrigin, vel, NULL, random() * 12.0f + scale * 14, -10, 0.2f + random() * 0.2f, 0.0, random()*360, 0, 800 + random() * 200.0f, 
						cgs.media.orangeRingShader );
	else
		FX_AddSprite( cent->lerpOrigin, vel, NULL, random() * 12.0f + scale * 14, -10, 0.5, 0.0, rgb, rgb, random()*360, 0, 800 + random() * 200.0f, 
						cgs.media.whiteRingShader );
}
 
//----------------------------------------------------
void FX_QuantumHitWall( vec3_t origin, vec3_t normal )
{
	localEntity_t	*le;
	vec3_t			dir, org;
	vec3_t			vel;
	float			scale;
	int				i;

	for ( i = 0; i < 12; i++ )
	{
		VectorSet( dir, normal[0] + crandom() * 2, normal[1] + crandom() * 2, normal[2] + crandom() );
		VectorNormalize( dir );

		scale = random() * 300 + 250;
		VectorScale( dir, scale, vel );
		vel[2] += 100;

		if ( rand() & 1 )
		{
			scale = random() * 12 + 2;
			FX_AddSprite( origin, vel, NULL, scale, -scale * 3, 0.9f, 0.5f, 0.0, 0.0, 200 + random() * 100, cgs.media.yellowParticleShader );
			FX_AddTrail( origin, vel, NULL, 96, -90, scale, -scale * 3, 0.8f, 0.4f, 0.0, 300, cgs.media.orangeTrailShader );
		}
		else
		{
			scale = random() * 14 + 6;
			FX_AddSprite( origin, vel, NULL, scale, -scale * 2, 0.9f, 0.5f, 0.0, 0.0, 350 + random() * 150, cgs.media.sunnyFlareShader );
			FX_AddTrail( origin, vel, NULL, 96, -80, scale, -scale * 2, 0.8f, 0.4f, 0.0, 500, cgs.media.orangeTrailShader );
		}
	}

	// Always face the camera
	VectorSubtract( cg.refdef.vieworg, origin, dir );
	VectorNormalize( dir );

	VectorMA( origin, 10, normal, org );

	// Main explosion, tag with light
	le = CG_MakeExplosion( org, normal, NULL, 0, cgs.media.quantumExplosionShader, 700, qtrue, 2.0f + ( crandom() * 0.3f)  );
	le->light = 250;
	le->refEntity.renderfx |= RF_NOSHADOW;
	VectorSet( le->lightColor, 1.0f, 1.0f, 0.6f );

	CG_MakeExplosion( origin, dir, cgs.media.explosionModel, 6, cgs.media.quantumFlashShader, 150, qfalse, 5.0f + ( crandom() * 0.3f)  );
	FX_AddSprite( org, NULL, NULL, 200, -450, 0.7f, 1.0, 0, 0, 400, cgs.media.sunnyFlareShader );

	CG_ImpactMark( cgs.media.compressionMarkShader, origin, normal, random()*360, 1,1,1,1.0, qfalse, 
					random() * 16 + 48, qfalse );

	CG_ExplosionEffects( origin, 6.0f, 350 );

	cgi_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, cg_weapons[WP_QUANTUM_BURST].missileHitSound );
}

/*
-------------------------
FX_QuantumHitPlayer
-------------------------
*/
void FX_QuantumHitPlayer( vec3_t origin, vec3_t normal )
{
//	cgi_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, cgi_S_RegisterSound("sound/weapons/quantum/hit.wav") );
	CG_ExplosionEffects( origin, 5.0f, 250 );
}

/*
-------------------------
FX_QuantumAltHitWall
-------------------------
*/
// Fun spinning concentric rings
bool QuantumAltThink( FXPrimitive *fx, centity_t *cent )
{
	FXCylinder	*fx2;
	float		scale;
	vec3_t		dir, cross, rgb1={1.0,1.0,0.0}, rgb2={1.0,0.8f,0.0}, rgb3={1.0,0.6f,0.0};

	fx->UpdateAlpha();

	// Create a rotating and scaling system
	scale = sin( (cg.time - fx->m_scale) * 0.007 ) * 120 + 140;
	RotatePointAroundVector( dir, fx->m_velocity, fx->m_acceleration, cg.time * 0.2 );
	// apply it to a cylinder
	fx2 = FX_AddCylinder( fx->m_origin, dir, 4,0, scale, 10, scale + 40, 20, fx->m_alpha,0, rgb1,rgb1, 20, cgs.media.psychicShader );

	if ( fx2 == NULL )
		return false;
	
	fx2->SetFlags( FXF_WRAP | FXF_NO_LOD );
	fx2->SetSTScale( 1 );
//	FX_AddQuad( fx->m_origin, dir, NULL, NULL, scale + 40, 0, fx->m_alpha, 0, rgb1, rgb1, random() * 360, 0, 0, 1, cgs.media.shockRingShader );

	// Create a rotating and scaling system
	scale = sin( (cg.time - fx->m_scale) * 0.005 ) * 80 + 100;
	RotatePointAroundVector( dir, fx->m_acceleration, fx->m_velocity, -cg.time * 0.4 );
	// apply it to a cylinder
	fx2 = FX_AddCylinder( fx->m_origin, dir, 4,0, scale, 0, scale + 20, 0, fx->m_alpha,0, rgb2,rgb2, 20, cgs.media.psychicShader );

	if ( fx2 == NULL )
		return false;

	fx2->SetFlags( FXF_WRAP | FXF_NO_LOD );
	fx2->SetSTScale( 1 );
	//FX_AddQuad( fx->m_origin, dir, NULL, NULL, scale + 20, 0, fx->m_alpha, 0, rgb1, rgb1, random() * 360, 0, 0, 1, cgs.media.shockRingShader );

	// Create a rotating and scaling system
	scale = sin( (cg.time - fx->m_scale) * 0.009 ) * 80 + 100;
	CrossProduct( fx->m_acceleration, fx->m_velocity, cross );
	RotatePointAroundVector( dir, cross, fx->m_velocity, -cg.time * 0.6 );
	// apply it to a cylinder
	fx2 = FX_AddCylinder( fx->m_origin, dir, 4,0, scale, 0, scale + 60, 0, fx->m_alpha,0, rgb3,rgb3, 20, cgs.media.psychicShader );

	if ( fx2 == NULL )
		return false;

	fx2->SetFlags( FXF_WRAP | FXF_NO_LOD );
	fx2->SetSTScale( 1 );
//	FX_AddQuad( fx->m_origin, dir, NULL, NULL, scale + 60, 0, fx->m_alpha, 0, rgb1, rgb1, random() * 360, 0, 0, 1, cgs.media.shockRingShader );

	return true;
}

void FX_QuantumAltHitWall( vec3_t origin, vec3_t normal )
{
	localEntity_t	*le;
	vec3_t			dir, right, up, org;
	//FXSprite		*fx;

	CG_ImpactMark( cgs.media.compressionMarkShader, origin, normal, random()*360, 1,1,1,1.0, qfalse, 
					random() * 16 + 48, qfalse );

	VectorMA( origin, 24, normal, org );

	// Random orient
	VectorSet( dir, crandom(), crandom(), crandom() );
	VectorNormalize( dir );

	cgi_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, cg_weapons[WP_QUANTUM_BURST].altmissileHitSound );

	// Do this up front and pass it in
	MakeNormalVectors( dir, right, up );

	// A cheesy way to get custom things added into the scene every client frame.
	FX_AddParticle( NULL, org, dir, up, cg.time,0,0.4f,0,0,0,900,cgs.media.purpleParticleShader, FXF_NODRAW,
						QuantumAltThink );
	
	// Always face the camera
	VectorSubtract( cg.refdef.vieworg, origin, dir );
	VectorNormalize( dir );

	CG_MakeExplosion( origin, dir, cgs.media.explosionModel, 6, cgs.media.quantumFlashShader, 150, qfalse, 4.6f + ( crandom() * 0.3f)  );

	// Cheesy core
	FX_AddSprite( org, NULL, NULL, 200, -450, 1.0, 1.0, 0, 0, 400, cgs.media.sunnyFlareShader );

	VectorMA( origin, 10, normal, org );

	// Main explosion, tag with light
	le = CG_MakeExplosion( org, normal, NULL, 0, cgs.media.quantumExplosionShader, 700, qtrue, 1.6f + ( crandom() * 0.3f)  );
	le->light = 250;
	le->refEntity.renderfx |= RF_NOSHADOW;
	VectorSet( le->lightColor, 1.0f, 1.0f, 0.6f );

	CG_ExplosionEffects( origin, 5.0f, 500 );
}

void FX_QuantumSparkle( vec3_t origin, vec3_t vel, vec3_t angles, vec3_t user )
{
	int		t, i;
	vec3_t	org, v;

	for ( i = 0; i < 4; i ++ )
	{
		VectorCopy( origin, org );

		for ( t = 0; t < 3; t++ )
		{
			org[t] = origin[t] + crandom() * 12;
			v[t] = crandom() * 18.0f;
		}

		FX_AddSprite( org, v, NULL, random() * 1 + 1, -4, 0.5f, 1.0f, 0.0f, 0.0f, 125 + random() * 100, cgs.media.yellowParticleShader, 0 );
	}
}

void FX_QuantumFizzles( vec3_t origin )
{
	float	v;
	vec3_t	dir, vel, org;

	for ( int i = 0; i < 32; i++ )
	{
		v = random() * 6.0f + 6.0f;

		VectorSet( dir, crandom(), crandom(), crandom() );
		VectorNormalize( dir );
		VectorScale( dir, v, vel );

		org[0] = origin[0] + dir[0] * 48;
		org[1] = origin[1] + dir[1] * 48;
		org[2] = origin[2] + dir[2] * 64;

		FX_AddSpawner( org, NULL, vel, NULL, 125, 10 + random() * 30, 200 + random() * 400, 0, FX_QuantumSparkle, NULL, 1024 );
	}
}

void FX_QuantumColumns( vec3_t origin )
{
	vec3_t		dir, bottom, top;
	vec3_t		sizeMin = {-4, -4, -1};
	vec3_t		sizeMax = {-4, -4, 1};
	trace_t		trace;
	FXCylinder	*fx1, *fx2;

	//Orient the explosions to face the camera
	VectorSubtract( cg.refdef.vieworg, origin, dir );
	VectorNormalize( dir );

	//=== Sound === 
	cgi_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.quantumBoom );	

	//=== columns === 
	VectorCopy( origin, bottom );
	bottom[2] -= 256;

	cgi_CM_BoxTrace( &trace, origin, bottom, sizeMin, sizeMax, 0, MASK_OPAQUE );
	VectorCopy( trace.endpos, bottom );

	VectorCopy( origin, top );
	top[2] += 256;

	cgi_CM_BoxTrace( &trace, origin, top, sizeMin, sizeMax, 0, MASK_OPAQUE );
	VectorCopy( trace.endpos, top );

	//found floor and ceiling, now do columns and ring explosions:
	//ceiling
	VectorSet( dir, 0, 0, -1 );

	fx1 = FX_AddCylinder( top, dir, top[2] - origin[2], (origin[2] - top[2]), 40, 100, 20, 50, 1.0, 0.0, 1000, cgs.media.quantumRingShader, 1.5 );

	if ( fx1 != NULL )
	{
		fx1->SetFlags( FXF_WRAP );
		fx1->SetSTScale( 3 );
	}

	//floor
	VectorSet( dir, 0, 0, 1 );

	fx2 = FX_AddCylinder( bottom, dir, origin[2] - bottom[2], (bottom[2] - origin[2]), 40, 100, 20, 50, 1.0, 0.0, 1000, cgs.media.quantumRingShader, 1.5 );

	if ( fx2 != NULL )
	{
		fx2->SetFlags( FXF_WRAP );
		fx2->SetSTScale( 3 );
	}

	FX_QuantumFizzles( origin );
}
