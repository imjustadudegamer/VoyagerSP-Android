// Federation Grenade Launcher FX

#include "cg_local.h"
#include "FX_Public.h"

/*
-------------------------
FX_GrenadeThink
-------------------------
*/ 

void FX_GrenadeThink( centity_t *cent, const struct weaponInfo_s *weapon )
{
	FX_AddSprite( cent->lerpOrigin, NULL, NULL, 16.0f + random() * 32.0f, 0.0f, 0.2f, 0.2f, 0, 0.0f, 1, cgs.media.dkorangeParticleShader );
	if ( rand() & 1 )
		FX_AddSprite( cent->lerpOrigin, NULL, NULL, 48.0f + random() * 48.0f, 0.0f, 0.2f, 0.2f, 0, 0.0f, 1, cgs.media.yellowParticleShader );
}

/*
-------------------------
FX_GrenadeAltThink
-------------------------
*/

void FX_GrenadeAltThink( centity_t *cent, const struct weaponInfo_s *weapon )
{
	vec3_t	dir, org;

	VectorCopy( cent->gent->s.pos.trDelta, dir );
	VectorNormalize( dir );

	// Move away from the ent origin so that the glow doesn't clip into the model so much.
	VectorMA( cent->lerpOrigin, 1.4, dir, org );

	FX_AddQuad( org, dir, NULL, NULL, 12.0f + random() * 32.0f, 0.0f, 0.75f, 0.75f, 0, 0, 0.0f, 1, cgs.media.dkorangeParticleShader );
	if ( rand() & 1 )
		FX_AddQuad( org, dir, NULL, NULL, 28.0f + random() * 32.0f, 0.0f, 0.5f, 0.5f, 0, 0, 0.0f, 1, cgs.media.yellowParticleShader );

	if ( cent->gent )
	{
		vec3_t rgb = { 1.0f, 1.0f, 0.4f }, rgb2 = { 1.0f, 0.5f, 0.0f };

		// This flag controls whether or not the thing was initial fire, or shrapnel
		if ( cent->gent->trigger_formation )
			// Shot from first person so beef it up a bit...
			FX_AddLine( cent->lerpOrigin,  cent->gent->pos2, 1.0f, 4.0f, 8.0f, 0.7f, 0.1f, rgb, rgb2, 200, cgs.media.whiteLaserShader );
		else
			FX_AddLine( cent->lerpOrigin,  cent->gent->pos2, 1.0f, 16.0f, 4.0f, 0.2f, 0.1f, rgb, rgb2, 100, cgs.media.whiteLaserShader );

		VectorCopy( cent->lerpOrigin, cent->gent->pos2 );
	}
}

/*
-------------------------
FX_GrenadeHitWall
-------------------------
*/

void FX_GrenadeHitWall( vec3_t origin, vec3_t normal )
{
	cgi_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.grenadeExplodeSnd );	
	CG_SurfaceExplosion( origin, normal, 11, 1, qfalse );
}

/*
-------------------------
FX_GrenadeHitPlayer
-------------------------
*/

void FX_GrenadeHitPlayer( vec3_t origin, vec3_t normal )
{
	cgi_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.grenadeExplodeSnd );	
	CG_SurfaceExplosion( origin, normal, 11, 1, qfalse );
}

/*
-------------------------
FX_GrenadeExplode
-------------------------
*/

void FX_GrenadeExplode( vec3_t origin, vec3_t normal )
{
	localEntity_t	*le;
	FXTrail			*fx;
	vec3_t			direction, org;

	VectorSet( direction, 0,0,1 );

	// Add an explosion and tag a light to it
	le = CG_MakeExplosion( origin, direction, cgs.media.nukeModel, 5, NULL, 250, qfalse, 25.0f, LEF_FADE_RGB );
	le->light = 150;
	le->refEntity.renderfx |= RF_NOSHADOW;

	VectorSet( le->lightColor, 1.0f, 0.6f, 0.2f );

	// Ground ring
	FX_AddQuad( origin, normal, NULL, NULL, 5, 330, 1.0, 0.0, random() * 360, 0, 0, 300, cgs.media.bigShockShader );
	// Flare
	VectorMA( origin, 12, direction, org );
	FX_AddSprite( org, NULL, NULL, 160.0, -540.0, 1.0, 0.0, 0.0, 0.0, 200, cgs.media.sunnyFlareShader );

	for ( int i = 0; i < 12; i++)
	{
		fx = FX_AddTrail( origin, NULL, NULL, 24.0f + random() * 12, -40.0f,
								0.5 + random() * 2, -3.0, 1.0f, 1.0f, 0.5f, 1000.0f,  cgs.media.orangeTrailShader, rand() & FXF_BOUNCE );


		if ( fx == NULL )
			return;
		
		FXE_Spray( normal, 470, 325, 0.5f, 700, (FXPrimitive *) fx );
	}

	cgi_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.grenadeExplodeSnd );	

	// Smoke and impact
	CG_ImpactMark( cgs.media.compressionMarkShader, origin, normal, random()*360, 1,1,1,1.0, qfalse, 
					random() * 16 + 48, qfalse );

	CG_ExplosionEffects( origin, 3.0f, 400 );
}

/*
-------------------------
FX_GrenadeShrapnelBits
-------------------------
*/

void FX_BlowBits( vec3_t start, vec3_t end, vec3_t dir, vec3_t user )
{
	vec3_t	diff, org;
	float	len;
	FXLine	*fx;

	VectorSubtract( end, start, diff );
	len = VectorNormalize( diff ) * ( 0.2 + random() * 0.3 );
	VectorMA( start, len, diff, org );

	fx = FX_AddLine( end, start, (int)(random() * 3.2f), 2.0f + random() * 2, 0, 0.5f, 0.1f, 150 + random() * 150, cgs.media.orangeTrailShader ); 

	if ( fx == NULL )
		return;
	
	fx->SetFlags( FXF_SHRINK );

	FX_AddQuad( end, dir, NULL, NULL, 1.0f, 64.0f, 1.0, 0.0, random() * 360.0f, 0.0f, 0.0, 200, cgs.media.orangeRingShader );
	// FX_AddQuad( end, dir, NULL, NULL, 20.0, -15.0, 0.6, 0.4, 0.0,0.0,0.0,450, cgs.media.borgEyeFlareShader );
}

void FX_GrenadeShrapnelBits( vec3_t start, vec3_t end, vec3_t dir )
{
	FX_AddSpawner( start, end, dir, NULL, 50, 175, 450, FXF_DELAY_SPAWN | FXF_SPAWN_ONCE, FX_BlowBits, NULL, 1024 );
}

void FX_GrenadeShrapnelExplode( vec3_t origin, vec3_t norm )
{
	localEntity_t	*le;
	FXTrail			*fx;
	vec3_t			direction, org;

	//Orient the explosions to face the camera
	VectorSubtract( cg.refdef.vieworg, origin, direction );
	VectorNormalize( direction );

	VectorMA( origin, 12, direction, org );
	// Add an explosion and tag a light to it
	le = CG_MakeExplosion( org, direction, cgs.media.explosionModel, 6, cgs.media.surfaceExplosionShader, 700, qfalse, 1.2f + (random()*0.3f) );
	le->light = 150;
	le->refEntity.renderfx |= RF_NOSHADOW;
	VectorSet( le->lightColor, 1.0f, 0.6f, 0.6f );

	for ( int i = 0; i < 6; i++)
	{
	fx = FX_AddTrail( origin, NULL, NULL, 16.0f, -15.0f,
								1.5, -1.5, 1.0f, 1.0f, 0.2f, 1000.0f,  cgs.media.orangeTrailShader, rand() & FXF_BOUNCE );

	if ( fx == NULL )
		return;

	FXE_Spray( norm, 500, 175, 0.8f, 512, (FXPrimitive *) fx );
	}

	cgi_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.grenadeAltExplodeSnd );	

	CG_ImpactMark( cgs.media.compressionMarkShader, origin, norm, random()*360, 1,1,1,1.0, qfalse, 
					random() * 16 + 48, qfalse );

	CG_ExplosionEffects( origin, 2.0, 350 );
}