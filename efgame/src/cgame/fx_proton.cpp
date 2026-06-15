#include "cg_local.h"
#include "FX_Public.h"


/*
-------------------------
-------------------------
// PROTON GUN 
-------------------------
-------------------------
*/


/*
-------------------------
FX_ProtonShockRing
-------------------------
*/

void FX_ProtonShockRing( vec3_t start, vec3_t end )
{
	vec3_t	shot_dir, org;

	// Faint shock ring created by shooting..looks like start and end got switched around...sigh
	VectorSubtract( start, end, shot_dir );
	VectorNormalize( shot_dir );
	VectorMA( end, 8, shot_dir, org );

	FX_AddSprite( start,
				NULL, NULL, 
				64.0f, -128.0f,
				1.0f, 1.0, 
				0.0, 0.0, 
				200, 
				cgs.media.rippleShader );
}

/*
-------------------------
FX_ProtonShot
-------------------------
*/

void FX_ProtonShot( vec3_t start, vec3_t end )
{
	float	length,
			repeat;
	vec3_t	dir;


	VectorSubtract( end, start, dir );
	length = VectorNormalize( dir );
	

	// thin inner line
	FX_AddLine( end, start, 
					1.0f, 
					2.0f, -3.0f, 
					1.0f, 0.0f, 
					350.0f, 
					cgs.media.whiteLaserShader );
	// thick outer glow
	FX_AddLine( end, start, 
					1.0f, 
					4.0f, 0.0f, 
					0.4f, 0.0f, 
					250.0f, 
					cgs.media.whiteLaserShader );
	
	// concentric rings
	FXCylinder *fx = FX_AddCylinder( start, dir, length, 0, 1.0f, 3.0f, 1.0f, 2.0f,
						0.4f, 0.0f, 300, cgs.media.protonBeamShader, 0.5f  );

	FX_AddSprite( end, NULL, NULL, 5, 0, 1, 0, 0, 0, 400, cgs.media.waterDropShader );
	FX_AddSprite( end, NULL, NULL, 3, 0, 1, 0, 0, 0, 400, cgs.media.waterDropShader );
	FX_AddSprite( end, NULL, NULL, 2, 0, 1, 0, 0, 0, 400, cgs.media.waterDropShader );
	FX_AddSprite( end, NULL, NULL, 1, 0, 1, 0, 0, 0, 400, cgs.media.waterDropShader );

	if( fx )
	{
		repeat = length / 12.0f;
		fx->SetFlags( FXF_WRAP | FXF_STRETCH | FXF_NON_LINEAR_FADE );
		fx->SetSTScale( repeat );
		
	}

//	FX_ProtonShockRing( start, end );
}

/*
-------------------------
FX_ProtonAltShot
-------------------------
*/
void FX_ProtonAltShot( vec3_t start, vec3_t end )
{
	float length = 20;
	vec3_t white = {1.0,1.0,1.0};
	vec3_t dir;

	FX_AddLine( start, end, 
					1.0f, 
					4.0f, -8.0f, 
					1.0f, 1.0f, 
					white, white,
					325.0f, 
					cgs.media.whiteLaserShader );

	FX_AddLine( start, end, 
					1.0f, 
					2.0f, -1.0f, 
					1.0f, 0.2f, 
					white, white,
					300.0f, 
					cgs.media.whiteLaserShader );


	VectorSubtract( end, start, dir );
	length = VectorNormalize( dir );

	FXCylinder *fx = FX_AddCylinder( start, dir, length, 0, 1, 3, 1, 3,
						0.6f, 0.1f, 500 , cgs.media.protonAltBeamShader, 0.2f );

	if( fx )
	{
		fx->SetFlags( FXF_WRAP | FXF_STRETCH );
		fx->SetSTScale( length / 56.0f );
		
	}

	fx = FX_AddCylinder( start, dir, length, 0, 2, 5, 2, 5,
						0.3f, 0.0, 600, cgs.media.protonAltBeamShader, 0.5f );

	if( fx )
	{
		fx->SetFlags( FXF_WRAP | FXF_STRETCH );
		fx->SetSTScale( length / 128.0f );
		
	}
}

/*
-------------------------
FX_ProtonExplosion
-------------------------
*/

void FX_ProtonExplosion( vec3_t end, vec3_t dir )
{
	vec3_t	org;

	// Move me away from the wall a bit so that I don't z-buffer into it
	VectorMA( end, 0.5, dir, org );

	// Expanding rings
//	FX_AddQuad( org, dir, NULL, NULL, 1, 24, 0.8f, 0.2f, random() * 360, 360, 0, 400, cgs.media.protonRingShader );
//	FX_AddQuad( org, dir, NULL, NULL, 1, 60, 0.8f, 0.2f, random() * 360, -360, 0, 300, cgs.media.protonRingShader );

	// Impact effect
//	FX_AddQuad( org, dir, NULL, NULL, 7, 35, 1.0, 0.0, random() * 360, 0, 0, 500, cgs.media.blueParticleShader );
//	FX_AddQuad( org, dir, NULL, NULL, 5, 25, 1.0, 0.0, random() * 360, 0, 0, 420, cgs.media.ltblueParticleShader );

	// Test of using the ripple shader......
	FX_AddQuad( org, dir, NULL, NULL, 13, 16, 1.0f, 0.1f, random() * 360, 360, 0, 400, cgs.media.rippleShader );
	FX_AddQuad( org, dir, NULL, NULL, 9, 20, 0.8f, 0.1f, random() * 360, -360, 0, 300, cgs.media.rippleShader );
	FX_AddQuad( org, dir, NULL, NULL, 20, 8, 1.0f, 0.1f, random() * 360, 0, 0, 300, cgs.media.rippleShader );

	FX_AddQuad( org, dir, NULL, NULL, 30, -20, 1.0f, 0.8f, random() * 360, 0, 0, 300, cgs.media.waterDropShader );

	
/*	
	localEntity_t	*le;
	FXTrail			*particle;
	vec3_t			direction, new_org;
	vec3_t			velocity		=	{ 0, 0, 8 };
	float			scale, dscale;

	int				i, numSparks;

	//Sparks
	numSparks = 4 + (random() * 4.0f);

	for ( i = 0; i < numSparks; i++ )
	{	
		scale = 0.25f + (random() * 2.0f);
		dscale = -scale * 0.5f;

		particle = FX_AddTrail( origin,
								NULL,
								NULL,
								16.0f,
								-32.0f,
								scale,
								-scale,
								1.0f,
								1.0f,
								0.25f,
								2000.0f,
								cgs.media.sparkShader,
								rand() & FXF_BOUNCE );

		if ( particle == NULL )
			return;

		FXE_Spray( normal, 200, 50, 0.5f, 512, (FXPrimitive *) particle );
	}

	// Smoke puff
	VectorMA( origin, 8, normal, new_org );

	FX_AddSprite( new_org, 
					velocity, NULL, 
					16.0f, 16.0f,
					1.0f, 0.0f,
					random()*45.0f,
					0.0f,
					1000.0f,
					cgs.media.steamShader );

	scale = 0.4f + ( random() * 0.2f);



	//Orient the explosions to face the camera
	VectorSubtract( cg.refdef.vieworg, origin, direction );
	VectorNormalize( direction );


	// Add in the explosion and tag it with a light
	le = CG_MakeExplosion( origin, direction, cgs.media.explosionModel, 6, cgs.media.electricalExplosionSlowShader, 475, qfalse, scale );
	le->light = 150;
	le->refEntity.renderfx |= RF_NOSHADOW;
	VectorSet( le->lightColor, 0.8f, 0.8f, 1.0f );

	// Scorch mark
	CG_ImpactMark( cgs.media.compressionMarkShader, origin, normal, random()*360, 1,1,1,1, qfalse, 4, qfalse );

	CG_ExplosionEffects( origin, 1.0f, 150 );
	*/
}

/*
-------------------------
FX_ProtonHit
-------------------------
*/
void FX_ProtonHit( vec3_t origin )
{
	FX_AddSprite( origin, 
					NULL,
					NULL,
					32.0f,
					-128.0f,
					1.0f,
					1.0f,
					random()*360,
					0.0f,
					250.0f,
					cgs.media.prifleImpactShader );
}

/*
-------------------------
FX_ProtonAltMiss
-------------------------
*/

void FX_ProtonAltMiss( vec3_t origin, vec3_t normal )
{
	vec3_t	new_org;
	vec3_t	velocity;
	float	scale, dscale;
	int		i;

	// Smoke puffs
	for ( i = 0; i < 8; i++ )
	{
		scale = random() * 12.0f + 4.0f;
		dscale = random() * 12.0f + 8.0f;

		VectorMA( origin, random() * 8.0f, normal, new_org );
		
		velocity[0] = normal[0] * crandom() * 24.0f;
		velocity[1] = normal[1] * crandom() * 24.0f;
		velocity[2] = normal[2] * ( random() * 24.0f + 8.0f ) + 8.0f; // move mostly up

		FX_AddSprite( new_org, 
					velocity, NULL, 
					scale, dscale,
					random() * 0.5f + 0.5f, 0.0f,
					random() * 45.0f,
					0.0f,
					800 + random() * 300.0f,
					cgs.media.steamShader );
	}

	// Scorch mark
	CG_ImpactMark( cgs.media.bulletmarksShader, origin, normal, random()*360, 1,1,1,1, qfalse, 6, qfalse );
//	FX_ProtonHit( origin );

	CG_ExplosionEffects( origin, 1.0f, 150 );
}