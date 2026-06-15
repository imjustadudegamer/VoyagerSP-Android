//Phaser

#include "cg_local.h"
#include "FX_Public.h"

/*
-------------------------
FX_PhaserFire
-------------------------
*/

void FX_PhaserFire( vec3_t start, vec3_t end, vec3_t normal, qboolean spark, qboolean impact, qboolean weak )
{
	FXTrail		*particle;
	vec3_t		dir, org;
	vec3_t		rgb = { 1,0.9f,0.6f}, rgb2={1,0.3f,0};
	float		scale = 2.4f + ( crandom() * 0.6f ),len;
	int			sparks;
 
	// Phaser beam
	if ( weak )
		FX_AddLine( start, end, 20.0f, scale, 0.0f, 0.4f, 0.4f, 1, cgs.media.phaserFireShader, FXF_ADJUST_ST );
	else
		FX_AddLine( start, end, 20.0f, scale, 0.0f, 1.0f, 1.0f, 1, cgs.media.phaserFireShader, FXF_ADJUST_ST );

	// Per frame impact mark
	FX_AddQuad( end, normal, NULL, NULL, random() * 1.5f + 1.75f, 0.0f, 0.5f, 0.5f, 0.0f, 0, 0, 1, cgs.media.sparkShader );
	FX_AddQuad( end, normal, NULL, NULL, random() * 1.5 + 1.7f, 0.0f, 1.0f, 1.0f, random()*360.0f, 0, 0, 1, cgs.media.yellowParticleShader );

	// Multi frame impacts--never do this when it hits a player because it will just look stupid
	if ( impact )
	{
		if ( !weak )
		FX_AddQuad( end, normal, NULL, NULL, random() * 1.25 + 1.5f, 2.0f, 1.0f, 0.0f, rgb, rgb2,random() * 360.0f, 2, 0, 500 + random() * 200, 
						cgs.media.sunnyFlareShader );

		CG_ImpactMark( cgs.media.scavMarkShader, end, normal, random()*360, 1,1,1,0.2f, qfalse, 
					random() + 1, qfalse );
	}

	if ( weak )
		return;

	// "Fun" sparks
	if ( spark )
	{
		sparks = rand() & 1 + 1;
		for( int i = 0; i < sparks; i++ )
		{	
			scale = 0.2f + (random() * 0.4);
			particle = FX_AddTrail( end, NULL, NULL, 5.0f, -15.0f,
									scale, -scale, 1.0f, 0.5f, 0.4f, 500.0f, cgs.media.sparkShader, rand() & FXF_BOUNCE );

			if ( particle == NULL )
				return;

			FXE_Spray( normal, 200, 75, 0.8f, 1024, (FXPrimitive *) particle );
		}
	}

	// Beam pulsing -- Is this really adding anything useful??
	VectorSubtract( end, start, dir );
	len = VectorNormalize( dir );

	for (int t = 0; t < 6; t++ )
	{
		scale = random();

		VectorMA( start, len * scale, dir, org );
		FX_AddSprite( org, NULL, NULL, random() + 5.3, 0.0, 0.25, 0.25, 0.0, 0.0, 1, cgs.media.yellowParticleShader );
	}
}

/*
-------------------------
FX_PhaserAltFire
-------------------------
*/

void FX_PhaserAltFire( vec3_t start, vec3_t end, vec3_t normal, qboolean spark, qboolean impact )
{
	float		scale = 1.5f + ( crandom() * 0.5f ), scale2, len;
	vec3_t		rgb = { 1,0.9f,0.6f}, rgb2={1,0.3f,0};
	vec3_t		org, dir;
	int			sparks;
	FXTrail		*particle;
	FXLine2		*fx;

	VectorSubtract( end, start, dir );
	len = VectorNormalize( dir );
 
	if ( len < 160 )
	{
		scale2 = (len / 160) * scale * 3.5f + scale;

		// Draw just the flared beam
		fx = FX_AddLine2( start, end, 1.0f, scale, 0.0f, scale2 * 1.25f, 0.0f, 1.0f, 0.9f, 1, cgs.media.whiteLaserShader );
		FX_AddLine2( start, end, 1.0f, scale * 0.6f, 0.0f, scale2, 0.0f, 0.8f, 0.8f, 1, cgs.media.phaserShader );
	}
	else
	{
		scale2 = scale * 4.5f;

		VectorMA( start, 160, dir, org );

		// Draw flared bit
		fx = FX_AddLine2( start, org, 1.0f, scale, 0.0f, scale2 * 1.25f, 0.0f, 1.0f, 0.9f, 1, cgs.media.whiteLaserShader );
		FX_AddLine2( start, org, 1.0f, scale * 0.6f, 0.0f, scale2, 0.0f, 0.8f, 0.8f, 1, cgs.media.phaserShader );

		// Now draw the rest
		FX_AddLine( org, end, 1.0f, scale2 * 1.25f, 0.0f, 1.0f, 0.9f, rgb2, rgb2, 1, cgs.media.whiteLaserShader );
		FX_AddLine( org, end, 1.0f, scale2, 0.0f, 0.8f, 0.8f, 1, cgs.media.phaserShader );
	}

	if ( fx )
	{
		fx->SetStartRGB( rgb2 );
		fx->SetEndRGB( rgb2 );
	}

	FX_AddLine( start, end, 1.0f, scale * 0.4f, 0.0f, 0.6f, 0.6f, 1, cgs.media.whiteLaserShader );

	// Per frame impact mark
	FX_AddQuad( end, normal, NULL, NULL, random() * 3.0 + scale2 * 2.5f, 0.0f, 1.0f, 1.0f, random() * 360.0f, 2, 0, 1, cgs.media.sunnyFlareShader );
	FX_AddQuad( end, normal, NULL, NULL, random() * 4 + scale2 * 3.0f, 0.0f, 1.0f, 1.0f, random()*360.0f, 0, 0, 1, cgs.media.yellowParticleShader );

	// Multi frame impacts--never do this when it hits a player because it will just look stupid
	if ( impact )
	{
		FX_AddQuad( end, normal, NULL, NULL, random() * 3.0 + scale2 * 2, 2.0f, 1.0f, 0.0f, rgb, rgb2,random() * 360.0f, 2, 0, 800 + random() * 200, 
					cgs.media.sunnyFlareShader );

		CG_ImpactMark( cgs.media.scavMarkShader, end, normal, random()*360, 1,1,1,0.2f, qfalse, 
					random() + scale2, qfalse );
	}

	sparks = rand() & 3 + 3;

	for( int i = 0; i < sparks; i++ )
	{	
		scale = 0.2f + (random() * 0.4);
		particle = FX_AddTrail( end, NULL, NULL, 5.0f, -15.0f,
								scale, -scale, 0.6f, 0.0f, 0.0f, 500.0f, cgs.media.orangeParticleShader );

		if ( particle == NULL )
			return;

		FXE_Spray( normal, 200, 75, 0.8f, 128, (FXPrimitive *) particle );
	}
}
