// cg_effects.c -- these functions generate localentities

#include "cg_local.h"
#include "cg_media.h"
#include "FX_Public.h"

/*
====================
CG_MakeExplosion
====================
*/
localEntity_t *CG_MakeExplosion( vec3_t origin, vec3_t dir, 
								qhandle_t hModel, int numFrames, qhandle_t shader,
								int msec, qboolean isSprite, float scale, int flags )
{
	float			ang = 0;
	localEntity_t	*ex;
	int				offset;
	vec3_t			tmpVec, newOrigin;

	if ( msec <= 0 ) {
		CG_Error( "CG_MakeExplosion: msec = %i", msec );
	}

	// skew the time a bit so they aren't all in sync
	offset = rand() & 63;

	ex = CG_AllocLocalEntity();
	if ( isSprite ) {
		ex->leType = LE_SPRITE_EXPLOSION; 
		ex->refEntity.rotation = rand() % 360;
		ex->radius = scale;
		VectorScale( dir, 16, tmpVec );
		VectorAdd( tmpVec, origin, newOrigin );
	} else {
		ex->leType = LE_EXPLOSION;
		VectorCopy( origin, newOrigin );

		// set axis with random rotate when necessary
		if ( !dir )
		{
			AxisClear( ex->refEntity.axis );
		}
		else
		{
			if ( !(flags & LEF_NO_RANDOM_ROTATE) )
				ang = rand() % 360;
			VectorCopy( dir, ex->refEntity.axis[0] );
			RotateAroundDirection( ex->refEntity.axis, ang );
		}
	}

	ex->startTime = cg.time - offset;
	ex->endTime = ex->startTime + msec;
	
	// bias the time so all shader effects start correctly
	ex->refEntity.shaderTime = ex->startTime / 1000.0f;

	ex->refEntity.hModel = hModel;
	ex->refEntity.customShader = shader;
	ex->lifeRate = (float)numFrames / msec;
	ex->leFlags = flags;

	//Scale the explosion
	if (scale != 1) {
		ex->refEntity.nonNormalizedAxes = qtrue;

		VectorScale( ex->refEntity.axis[0], scale, ex->refEntity.axis[0] );
		VectorScale( ex->refEntity.axis[1], scale, ex->refEntity.axis[1] );
		VectorScale( ex->refEntity.axis[2], scale, ex->refEntity.axis[2] );
	}
	// set origin
	VectorCopy ( newOrigin, ex->refEntity.origin);
	VectorCopy ( newOrigin, ex->refEntity.oldorigin );

	ex->color[0] = ex->color[1] = ex->color[2] = 1.0;

	return ex;
}

// When calling this version, just pass in a zero for the flags
localEntity_t *CG_MakeExplosion( vec3_t origin, vec3_t dir, 
								qhandle_t hModel, int numFrames, qhandle_t shader,
								int msec, qboolean isSprite, float scale ) {
	return CG_MakeExplosion( origin, dir, hModel, numFrames, shader, msec, isSprite, scale, 0 );
}

/*
====================
CG_AddTempLight
====================
*/
localEntity_t *CG_AddTempLight( vec3_t origin, float scale, vec3_t color, int msec )
{
	localEntity_t	*ex;

	if ( msec <= 0 ) {
		CG_Error( "CG_AddTempLight: msec = %i", msec );
	}

	ex = CG_AllocLocalEntity();

	ex->leType = LE_LIGHT; 

	ex->startTime = cg.time;
	ex->endTime = ex->startTime + msec;
	
	// set origin
	VectorCopy ( origin, ex->refEntity.origin);
	VectorCopy ( origin, ex->refEntity.oldorigin );

	VectorCopy( color, ex->lightColor );
	ex->light = scale;	

	return ex;
}

/*
-------------------------
CG_ExplosionEffects

Used to find the player and shake the camera if close enough
intensity ranges from 1 (minor tremble) to 16 (major quake)
-------------------------
*/

void CG_ExplosionEffects( vec3_t origin, int intensity, int radius)
{
	//FIXME: When exactly is the vieworg calculated in relation to the rest of the frame?s

	vec3_t	dir;
	float	dist, intensityScale;
	float	realIntensity;

	VectorSubtract( cg.refdef.vieworg, origin, dir );
	dist = VectorNormalize( dir );

	//Use the dir to add kick to the explosion

	if ( dist > radius )
		return;

	intensityScale = 1 - ( dist / (float) radius );
	realIntensity = intensity * intensityScale;

	CGCam_Shake( realIntensity, 750 ); // 500 seemed a bit too quick
}


/*
-------------------------
CG_SmokeSpawn
-------------------------
*/

void CG_SmokeSpawn( vec3_t origin, vec3_t normal, vec3_t vel, vec3_t user )
{
	// This function will create directable smoke.
	CG_Smoke( origin, normal, 24.0f, 24.0f, cgs.media.smokeShader, FXF_USE_ALPHA_CHAN );
}

/*
-------------------------
CG_SurfaceExplosion

Adds an explosion to a surface
-------------------------
*/

#define NUM_SPARKS		12
#define NUM_PUFFS		1
#define NUM_EXPLOSIONS	4

void CG_SurfaceExplosion( vec3_t origin, vec3_t normal, float radius, float shake_speed, qboolean smoke )
{
	localEntity_t	*le;
	FXTrail			*particle;
	vec3_t			direction, new_org;
	vec3_t			velocity		= { 0, 0, 0 };
	vec3_t			temp_org, temp_vel;
	float			scale, dscale;
	int				i, numSparks;

	//Sparks
	numSparks = 16 + (random() * 16.0f);
	
	for ( i = 0; i < numSparks; i++ )
	{	
		scale = 0.25f + (random() * 2.0f);
		dscale = -scale*0.5;

		particle = FX_AddTrail( origin,
								NULL,
								NULL,
								32.0f,
								-64.0f,
								scale,
								-scale,
								1.0f,
								0.0f,
								0.25f,
								4000.0f,
								cgs.media.sparkShader,
								rand() & FXF_BOUNCE);

		if ( particle == NULL )
			return;

		FXE_Spray( normal, 500, 150, 1.0f, 768 + (rand() & 255), (FXPrimitive *) particle );
	}

	//Smoke
	//Move this out a little from the impact surface
	VectorMA( origin, 4, normal, new_org );
	VectorSet( velocity, 0.0f, 0.0f, 16.0f );

	for ( i = 0; i < 4; i++ )
	{
		VectorSet( temp_org, new_org[0] + (crandom() * 16.0f), new_org[1] + (crandom() * 16.0f), new_org[2] + (random() * 4.0f) );
		VectorSet( temp_vel, velocity[0] + (crandom() * 8.0f), velocity[1] + (crandom() * 8.0f), velocity[2] + (crandom() * 8.0f) );

		FX_AddSprite(	temp_org,
						temp_vel, 
						NULL, 
						64.0f + (random() * 32.0f), 
						16.0f, 
						1.0f, 
						0.0f,
						20.0f + (crandom() * 90.0f),
						0.5f,
						1500.0f, 
						cgs.media.smokeShader, FXF_USE_ALPHA_CHAN );
	}

	//Core of the explosion

	//Orient the explosions to face the camera
	VectorSubtract( cg.refdef.vieworg, origin, direction );
	VectorNormalize( direction );

	//Tag the last one with a light
	le = CG_MakeExplosion( origin, direction, cgs.media.explosionModel, 6, cgs.media.surfaceExplosionShader, 500, qfalse, radius * 0.02f + (random() * 0.3f) );
	le->light = 150;
	VectorSet( le->lightColor, 0.9f, 0.8f, 0.5f );

	for ( i = 0; i < NUM_EXPLOSIONS-1; i ++)
	{
		VectorSet( new_org, (origin[0] + (16 + (crandom() * 8))*crandom()), (origin[1] + (16 + (crandom() * 8))*crandom()), (origin[2] + (16 + (crandom() * 8))*crandom()) );
		le = CG_MakeExplosion( new_org, direction, cgs.media.explosionModel, 6, cgs.media.surfaceExplosionShader, 300 + (rand() & 99), qfalse, radius * 0.05f + (crandom() *0.3f) );
	}

	//Shake the camera
	CG_ExplosionEffects( origin, shake_speed, 350 );

	// The level designers wanted to be able to turn the smoke spawners off.  The rationale is that they
	//	want to blow up catwalks and such that fall down...when that happens, it shouldn't really leave a mark
	//	and a smoke spewer at the explosion point...
	if ( smoke )
	{
		VectorMA( origin, -8, normal, temp_org );
		FX_AddSpawner( temp_org, normal, NULL, NULL, 100, random()*25.0f, 5000.0f, (void *) CG_SmokeSpawn );

		//Impact mark
		//FIXME: Replace mark
		//CG_ImpactMark( cgs.media.burnMarkShader, origin, normal, random()*360, 1,1,1,1, qfalse, 8, qfalse );
	}
}

/*
-------------------------
CG_MiscModelExplosion

Adds an explosion to a misc model breakables
-------------------------
*/

void CG_MiscModelExplosion( vec3_t origin, vec3_t normal )
{
	localEntity_t	*le;
	FXTrail			*particle;
	vec3_t			direction, new_org;
	vec3_t			velocity		= { 0, 0, 0 };
	vec3_t			temp_org, temp_vel;
	float			scale, dscale;
	int				i, numSparks;

	//Sparks
	numSparks = 8 + (random() * 8.0f);
	
	for ( i = 0; i < numSparks; i++ )
	{	
		scale = 0.25f + (random() * 2.0f);
		dscale = -scale*0.5;

		particle = FX_AddTrail( origin,
								NULL,
								NULL,
								32.0f,
								-64.0f,
								scale,
								-scale,
								1.0f,
								0.0f,
								0.25f,
								4000.0f,
								cgs.media.sparkShader,
								rand() & FXF_BOUNCE);

		if ( particle == NULL )
			return;

		FXE_Spray( normal, 500, 150, 1.0f, 768 + (rand() & 255), (FXPrimitive *) particle );
	}

	//Smoke
	//Move this out a little from the impact surface
	VectorMA( origin, 4, normal, new_org );
	VectorScale( normal, -16, velocity );

	for ( i = 0; i < 3; i++ )
	{
		VectorSet( temp_org, new_org[0] + (crandom() * 8.0f), new_org[1] + (crandom() * 8.0f), new_org[2] + (random() * 8.0f) );
		VectorSet( temp_vel, velocity[0] + (crandom() * 16.0f), velocity[1] + (crandom() * 16.0f), velocity[2] + (crandom() * 16.0f) );

		FX_AddSprite(	temp_org,
						temp_vel, 
						NULL, 
						16.0f + (random() * 16.0f), 
						8.0f, 
						1.0f, 
						0.0f,
						20.0f + (crandom() * 90.0f),
						0.5f,
						1000.0f + random() * 1000.0f, 
						cgs.media.smokeShader, FXF_USE_ALPHA_CHAN );
	}

	//Orient the explosions to face the camera
	VectorSubtract( cg.refdef.vieworg, origin, direction );
	VectorNormalize( direction );

	//Tag the last one with a light
	le = CG_MakeExplosion( origin, direction, cgs.media.explosionModel, 6, cgs.media.surfaceExplosionShader, 500, qfalse, 0.7 + crandom() * 0.3f );
	le->light = 150;
	VectorSet( le->lightColor, 0.9f, 0.8f, 0.5f );

//	for ( i = 0; i < 2; i ++)
	{
		VectorSet( new_org, origin[0] + crandom() * 16, origin[1] + crandom() * 16, origin[2] + crandom() * 16 );
		le = CG_MakeExplosion( new_org, direction, cgs.media.explosionModel, 6, cgs.media.surfaceExplosionShader, 300 + (rand() & 99), qfalse, 0.7 + crandom() *0.3f );
	}
}

/*
-------------------------
CG_Chunks

Fun chunk spewer
-------------------------
*/

void CG_Chunks( int owner, vec3_t origin, const vec3_t normal, float speed, int numChunks, material_t chunkType, int customChunk, float baseScale )
{
	localEntity_t	*le;
	refEntity_t		*re;
	vec3_t			dir;
	int				i, j, k;
	int				chunkModel=0, chunkSound;
	qboolean		chunk = qfalse;

	switch(chunkType)
	{
	case MAT_METAL:
	case MAT_BORG:
		// FIXME: Precache sounds?
		chunkSound = cgs.media.borgChunkSound;
		break;
	case MAT_GLASS:
		chunkSound = cgs.media.glassChunkSound;
		break;
	case MAT_GLASS_METAL:
		chunkSound = cgs.media.chunkSound;
		break;
	case MAT_ELECTRICAL:// (sparks)
		cgi_S_StartSound (NULL, owner, CHAN_BODY, cgi_S_RegisterSound (va("sound/ambience/spark%d.wav", Q_irand(1, 6))) );
		vectoangles(normal, dir);
		CG_Spark( origin, dir );
		return;
		break;
	case MAT_ORGANIC:// (not implemented)
	case MAT_STASIS:
		chunkSound = cgs.media.stasisChunkSound;
		break;
	default:
		//Invalid material?  Assume MAT_METAL?
		CG_Error( "CG_Chunks: Invalid chunk type" );
		return;
		break;
	}

	if(baseScale <= 0)
	{
		baseScale = 1;
	}

	//FIXME: LOD
	//Chunks
	for ( i = 0; i < numChunks; i++ )
	{
		if ( customChunk > 0 )
		{
			// Try to use a custom chunk.
			if ( cgs.model_draw[customChunk] )
			{
				chunk = qtrue;
				chunkModel = cgs.model_draw[customChunk];
			}
		}

		if ( !chunk )
		{
			// No custom chunk.  Pick a random chunk type at run-time so we don't get the same chunks
			switch( chunkType )
			{
			case MAT_GLASS:
				chunkModel = cgs.media.glassChunkModels[0][Q_irand(0, 5)];
				break;
			case MAT_ORGANIC:
				// FIXME: These are actually metal chunks.
				chunkModel = cgs.media.chunkModels[0][Q_irand(0, 5)];
				break;
			case MAT_BORG:
				chunkModel = cgs.media.borgChunkModels[0][Q_irand(0, 2)];
				break;
			case MAT_STASIS:
				chunkModel = cgs.media.stasisChunkModels[0][Q_irand(0, 3)];
				break;
			case MAT_GLASS_METAL:
				// Do some random chunk types
				if ( rand() & 1 )
				{
					chunkModel = cgs.media.glassChunkModels[0][Q_irand(0, 5)];
				}
				else
				{
					chunkModel = cgs.media.chunkModels[0][Q_irand(0, 5)];
				}
				break;
			default:
				// Metal chunks, etc..
				chunkModel = cgs.media.chunkModels[0][Q_irand(0, 5)];
				break;
			}
		}

		le = CG_AllocLocalEntity();
		re = &le->refEntity;

		le->leType = LE_FRAGMENT;
		le->endTime = cg.time + 2000;

		VectorCopy( origin, re->origin );

		for ( j = 0; j < 3; j++ )
		{
			re->origin[j] += crandom() * 12;
		}
		VectorCopy( re->origin, le->pos.trBase );

		//Velocity
		for ( j = 0; j < 3; j ++ )
		{
			dir[j] = normal[j] + Q_flrand(-0.8f, 0.8f);
		}

//		VectorNormalize(dir);
		VectorScale( dir, Q_flrand( speed * 0.25f, speed * 1.75f ), le->pos.trDelta );

		//Angular Velocity
		VectorSet( le->angles.trBase, Q_flrand(0, 360), Q_flrand(0, 360), Q_flrand(0, 360) );
		for ( j = 0; j < 3; j ++ )
		{
			le->angles.trDelta[j] = Q_flrand(-24.0f, 24.0f);
		}

		VectorNormalize(le->angles.trDelta);
		VectorScale( le->angles.trDelta, Q_flrand(200.0f, 800.0f), le->angles.trDelta );

		AxisCopy( axisDefault, re->axis );

		le->radius = Q_flrand(baseScale * 0.7f, baseScale * 1.3f );

		re->nonNormalizedAxes = qtrue;
		re->hModel = chunkModel;
		
		le->pos.trType = TR_GRAVITY;
		le->pos.trTime = cg.time;
		le->angles.trType = TR_INTERPOLATE;
		le->angles.trTime = cg.time;
		le->bounceFactor = 0.2f + random() * 0.2f;
		le->leFlags |= LEF_TUMBLE;
		le->ownerGentNum = owner;

		// Make sure that we have the desired start size set
		for( k = 0; k < 3; k++)
		{
			VectorScale(le->refEntity.axis[k], le->radius, le->refEntity.axis[k]);
		}
	}

	//Sound
	cgi_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, chunkSound );
}

void CG_EnergyGibs( int owner, vec3_t origin )
{
	localEntity_t	*le;
	refEntity_t		*re;
	vec3_t			dir;
	int				i, j, k;
	int				chunkModel=0;
	float			baseScale = 0.7f;
	int				numChunks = Q_irand( 13, 20 );//FIXME: LOD

	for ( i = 0; i < numChunks; i++ )
	{
		if ( rand() & 1 )
		{
			chunkModel = cgs.media.glassChunkModels[0][Q_irand(0, 5)];
		}
		else
		{
			chunkModel = cgs.media.chunkModels[0][Q_irand(0, 5)];
		}

		le = CG_AllocLocalEntity();
		re = &le->refEntity;

		le->leType = LE_FRAGMENT;
		le->endTime = cg.time + 2000;

		VectorCopy( origin, re->origin );

		for ( j = 0; j < 3; j++ )
		{
			re->origin[j] += crandom() * 12;
		}
		VectorCopy( re->origin, le->pos.trBase );

		//Velocity
		VectorSet( dir, crandom(), crandom(), crandom() );
		VectorScale( dir, Q_flrand( 300, 500 ), le->pos.trDelta );

		//Angular Velocity
		VectorSet( le->angles.trBase, crandom() * 360, crandom() * 360, crandom() * 360 );
		VectorSet( le->angles.trDelta, crandom() * 90, crandom() * 90, crandom() * 90 );

		AxisCopy( axisDefault, re->axis );

		le->radius = Q_flrand(baseScale * 0.7f, baseScale * 1.3f );

		re->nonNormalizedAxes = qtrue;
		re->hModel = chunkModel;
		re->customShader = cgs.media.quantumDisruptorShader;
		re->shaderTime = cg.time/1000.0f;
		
		le->pos.trType = TR_GRAVITY;
		le->pos.trTime = cg.time;
		le->angles.trType = TR_INTERPOLATE;
		le->angles.trTime = cg.time;
		le->bounceFactor = 0.3f + random() * 0.3f;
		le->leFlags |= LEF_TUMBLE;
		le->ownerGentNum = owner;

		re->shaderRGBA[0] = re->shaderRGBA[1] = re->shaderRGBA[2] = re->shaderRGBA[3] = 255;

		// Make sure that we have the desired start size set
		for( k = 0; k < 3; k++)
		{
			VectorScale(le->refEntity.axis[k], le->radius, le->refEntity.axis[k]);
		}
	}
}
