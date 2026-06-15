#include "cg_local.h"
#include "FX_Public.h"

/*
-------------------------
FX_IMODBolt2
-------------------------
*/

void FX_IMODBolt2( vec3_t start, vec3_t end, vec3_t dir )
{
	vec3_t	control1, control2, control1_vel, control2_vel;
	vec3_t	direction, vr, vu;
	float	len;
	
	MakeNormalVectors( dir, vr, vu );

	VectorSubtract( end, start, direction );
	len = VectorNormalize( direction );

	VectorMA(start, len*0.5f, direction, control1 );
	VectorMA(start, len*0.25f, direction, control2 );

	vectoangles( direction, control1_vel );
	control1_vel[ROLL] += crandom() * 360;
	AngleVectors( control1_vel, NULL, NULL, control1_vel );

	vectoangles( direction, control2_vel );
	control2_vel[ROLL] += crandom() * 360;
	AngleVectors( control2_vel, NULL, NULL, control2_vel );

	VectorScale( control1_vel, 12.0f + (140.0f * random()), control1_vel );
	VectorScale( control2_vel, -12.0f + (-140.0f * random()), control2_vel );

	FX_AddBezier( start, end, 
					control1, control2, control1_vel, control2_vel, NULL, NULL, 
					0.5f,				//scale
					1.0f,				//dscale
					0.5f,				//alpha
					0.0f,				//dalpha
					1000.0f,			//killtime
					cgs.media.altIMOD2Shader );
}

/*
-------------------------
FX_AltIMODBolt
-------------------------
*/

void FX_AltIMODBolt( vec3_t start, vec3_t end, vec3_t dir )
{
	vec3_t	control1, control2, control1_vel, control2_vel;
	vec3_t	direction, vr, vu;
	float	len;
	
	MakeNormalVectors( dir, vr, vu );

	VectorSubtract( end, start, direction );
	len = VectorNormalize( direction );

	VectorMA(start, len*0.5f, direction, control1 );
	VectorMA(start, len*0.25f, direction, control2 );

	vectoangles( direction, control1_vel );
	control1_vel[ROLL] += crandom() * 360;
	AngleVectors( control1_vel, NULL, NULL, control1_vel );

	vectoangles( direction, control2_vel );
	control2_vel[ROLL] += crandom() * 360;
	AngleVectors( control2_vel, NULL, NULL, control2_vel );

	VectorScale( control1_vel, 10.0f + (100.0f * random()), control1_vel );
	VectorScale( control2_vel, -10.0f + (-100.0f * random()), control2_vel );

	FX_AddBezier( start, end, 
					control1, control2, control1_vel, control2_vel, NULL, NULL, 
					1.0f,				//scale
					2.0f,				//dscale
					0.5f,				//alpha
					0.0f,				//dalpha
					1000.0f,			//killtime
					cgs.media.IMOD2Shader );
}

/*
-------------------------
FX_IMODShot
-------------------------
*/

void FX_IMODShot( vec3_t end, vec3_t start, vec3_t dir)
{
	FXLine	*fx;
//	FX_AddLine( start, end, 1.0f, 6.0f, -32.0f, 1.0f, 1.0f, 300.0f, cgs.media.altIMODShader );
/*	fx = FX_AddLine( start, end, 1.0f, 6.0f, -32.0f, 1.0f, 1.0f, 300.0f, cgs.media.altIMODShader, FXF_DRAWCAPS );
	if ( fx == NULL )
		return;
	fx->SetEndcapShader( cgs.media.altIMODEndcapShader );
*/

	fx = FX_AddLine( start, end, 1.0f, 8.0f, -24.0f, 1.0f, 1.0f, 350.0f, cgs.media.altIMODShader, FXF_DRAWCAPS );
	if ( fx == NULL )
		return;
	fx->SetEndcapShader( cgs.media.altIMODEndcapShader );

	FX_AddSprite( start, NULL, NULL, 20.0f, -48.0f, 1.0f, 1.0f, 0.0f, 0.0f, 350, cgs.media.purpleParticleShader );
	FX_IMODBolt2( start, end, dir );
 }

/*
-------------------------
FX_AltIMODShot
-------------------------
*/

void FX_AltIMODShot( vec3_t end, vec3_t start, vec3_t dir)
{
	FXLine	*fx;

	fx = FX_AddLine( start, end, 1.0f, 16.0f, -32.0f, 1.0f, 1.0f, 500.0f, cgs.media.IMODShader, FXF_DRAWCAPS );
	if ( fx == NULL )
		return;
	fx->SetEndcapShader( cgs.media.IMODendcapShader );
	
//	FX_AddLine( start, end, 1.0f, 24.0f, -96.0f, 1.0f, 1.0f, 250.0f, cgs.media.IMODShader );
/*	fx = FX_AddLine( start, end, 1.0f, 32.0f, -128.0f, 1.0f, 1.0f, 250.0f, cgs.media.IMODShader, FXF_DRAWCAPS );
	if ( fx == NULL )
		return;
	fx->SetEndcapShader( cgs.media.IMODendcapShader );
*/
	FX_AddSprite( start, NULL, NULL, 24.0f, -48.0f, 1.0f, 1.0f, 0.0f, 0.0f, 500, cgs.media.blueParticleShader );
	for ( int i = 0; i < 2; i++ )
		FX_AltIMODBolt( start, end, dir );
}

/*
-------------------------
FX_IMODExplosion
-------------------------
*/

void FX_IMODExplosion( vec3_t origin, vec3_t normal )
{
	localEntity_t	*le;
	FXTrail			*particle;
	vec3_t			direction;
	float			scale, dscale;
	int				i, numSparks;

	//Orient the explosions to face the camera
	VectorSubtract( cg.refdef.vieworg, origin, direction );
	VectorNormalize( direction );

	//Tag the last one with a light
	le = CG_MakeExplosion( origin, direction, cgs.media.explosionModel, 6, cgs.media.imodExplosionShader, 400, qfalse, 0.6f + (random()*0.2f) );
	le->light = 75;
	VectorSet( le->lightColor, 1.0f, 0.8f, 0.5f );

	//Sparks
	numSparks = 3 + (rand() & 7);
	
	for ( i = 0; i < numSparks; i++ )
	{	
		scale = 1.0f + (random() * 0.5f);
		dscale = -scale*0.5;

		particle = FX_AddTrail( origin,
								NULL,
								NULL,
								32.0f + (random() * 64.0f),
								-256.0f,
								scale,
								0.0f,
								1.0f,
								0.0f,
								0.25f,
								750.0f,
								cgs.media.purpleParticleShader );

		if ( particle == NULL )
			return;

		FXE_Spray( normal, 500, 250, 0.75f, 256, (FXPrimitive *) particle );
	}

	CG_ImpactMark( cgs.media.IMODMarkShader, origin, normal, random()*360, 1,1,1,1, qfalse, 6, qfalse );

	CG_ExplosionEffects( origin, 1.0f, 150 );
}

/*
-------------------------
FX_AltIMODExplosion
-------------------------
*/
void FX_AltIMODExplosion( vec3_t origin, vec3_t normal )
{
	localEntity_t	*le;
	FXTrail			*particle;
	vec3_t			direction, org;
	float			scale, dscale;
	int				i, numSparks;

	//Orient the explosions to face the camera
	VectorSubtract( cg.refdef.vieworg, origin, direction );
	VectorNormalize( direction );

	//Tag the last one with a light
	le = CG_MakeExplosion( origin, direction, cgs.media.explosionModel, 6, cgs.media.electricalExplosionSlowShader, 475, qfalse, 0.7f + (random()*0.25f) );
	le->light = 150;
	VectorSet( le->lightColor, 1.0f, 0.8f, 0.5f );

	for ( i = 0; i < 2; i ++)
	{
		VectorSet( org, origin[0] + 16 * random(), origin[1] + 16 * random(), origin[2] + 16 * random() );
		CG_MakeExplosion( org, direction, cgs.media.explosionModel, 6, cgs.media.altImodExplosion, 
					250, qfalse, 0.5f + (random()*0.2f) );
	}

	//Sparks

	numSparks = 8 + (rand() & 7);
	
	for ( i = 0; i < numSparks; i++ )
	{	
		scale = 1.5f + (random() * 0.5f);
		dscale = -scale*0.5;

		particle = FX_AddTrail( origin,
								NULL,
								NULL,
								32.0f + (random() * 64.0f),
								-256.0f,
								scale,
								0.0f,
								1.0f,
								0.0f,
								0.25f,
								750.0f,
								cgs.media.spark2Shader );

		if ( particle == NULL )
			return;

		FXE_Spray( normal, 500, 250, 0.75f, 256, (FXPrimitive *) particle );
	}

	CG_ImpactMark( cgs.media.IMODMarkShader, origin, normal, random()*360, 1,1,1,1, qfalse, 8, qfalse );

	CG_ExplosionEffects( origin, 2.0f, 200 );
}
