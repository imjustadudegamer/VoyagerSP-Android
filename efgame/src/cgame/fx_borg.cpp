// Borg FX

#include "cg_local.h"
#include "FX_Public.h"

/*
-------------------------
FX_BorgHit
-------------------------
*/

void FX_BorgHit( vec3_t origin, vec3_t dir )
{
	localEntity_t	*le;
	FXTrail			*particle;
	vec3_t			direction;
	float			scale;
	int				i, numSparks;

	//Inner spray

	numSparks = 8 + (random() * 2 );

	for ( i = 0; i < numSparks; i++ )
	{	
		scale = 0.5f + (random() * 0.5f);

		particle = FX_AddTrail( origin,
								NULL,
								NULL,
								16.0f,
								-16.0f,
								scale,
								-scale,
								1.0f,
								1.0f,
								0.25f,
								250.0f,
								cgs.media.sparkShader,
								FXF_BOUNCE );

		if ( particle == NULL )
			return;

		FXE_Spray( dir, 400, 150, 0.25, 512, (FXPrimitive *) particle );
	}

	//Outer spray

	numSparks = 4 + (random() * 2 );

	for ( i = 0; i < numSparks; i++ )
	{	
		scale = 0.5f + (random() * 0.5f);

		particle = FX_AddTrail( origin,
								NULL,
								NULL,
								8.0f,
								-16.0f,
								scale,
								-scale,
								1.0f,
								1.0f,
								0.25f,
								500.0f,
								cgs.media.sparkShader,
								FXF_BOUNCE );

		if ( particle == NULL )
			return;

		FXE_Spray( dir, 300, 50, 0.5, 512, (FXPrimitive *) particle );
	}

	//Orient the explosions to face the camera
	VectorSubtract( cg.refdef.vieworg, origin, direction );
	VectorNormalize( direction );

	//Tag the last one with a light
	le = CG_MakeExplosion( origin, direction, cgs.media.explosionModel, 6, cgs.media.surfaceExplosionShader, 500, qfalse, 0.2f + (random() * 0.25f) );
	le->light = 150;
	VectorSet( le->lightColor, 0.9f, 0.8f, 0.5f );
}

/*
-------------------------
FX_BorgTeleportParticles
-------------------------
*/

void FX_BorgTeleportParticles( vec3_t origin, vec3_t dir, vec3_t accel, vec3_t user )
{
	const int	radius = 32;
	vec3_t		neworg, vel, acc;
	int			numParts = 8;

	for ( int i = 0; i < numParts; i++ )
	{
		VectorSet( neworg, origin[0] + ( crandom() * ( radius * 0.5 ) ), origin[1] + ( crandom() * ( radius * 0.5 ) ), origin[2] + ( crandom() * 4.0f ) );
		VectorScale( dir, 128 + ( random() * 256 ), vel );
		VectorScale( vel, -2.0, acc );

		FX_AddSprite( neworg, vel, acc, 1.0f + ( crandom() * 2.0f ), 0.0f, 1.0f, 0.0f, random() * 360, 0.0f, 500, cgs.media.borgFlareShader );
	}
}

/*
-------------------------
FX_BorgTeleport
-------------------------
*/

void FX_BorgTeleport( vec3_t origin )
{
	vec3_t	org, org2, angles;

	VectorSet( angles, 0, 0, 1 );
	VectorSet( org, origin[0], origin[1], origin[2] - 32 );
	FX_AddSpawner( origin, angles, NULL, NULL, 50, 25, 1000, FX_BorgTeleportParticles, NULL );

	VectorSet( angles, 0, 0, -1 );
	VectorSet( org2, origin[0], origin[1], origin[2] + 32 );
	FX_AddSpawner( org2, angles, NULL, NULL, 50, 25, 1000, FX_BorgTeleportParticles, NULL );

	vec3_t	dir;
	VectorSubtract( org2, org, dir );
	VectorNormalize( dir );

	FX_AddCylinder( org, dir, 96.0f, 0.0f, 1.0f, 32.0f, 1.0f, 32.0f, 1.0f, 0.0f, 1500, cgs.media.borgFlareShader, 0.5 );

	cgi_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.borgBeamInSound );
}

/*
-------------------------
FX_BorgShield
-------------------------
*/

void FX_BorgShield( vec3_t position, vec3_t dir )
{
	localEntity_t	*le;
	vec3_t			direction;

	//TODO: dir is for later expansion

	//Orient the explosions to face the camera
	VectorSubtract( cg.refdef.vieworg, position, direction );
	VectorNormalize( direction );

	le = CG_MakeExplosion( position, direction, cgs.media.explosionModel, 6, cgs.media.borgShieldShader, 250, qfalse, 0.25f );
	le->light = 50;
	VectorSet( le->lightColor, 0.0f, 1.0f, 0.0f );
}

/*
-------------------------
FX_BorgProjectileThink
-------------------------
*/

void FX_BorgProjectileThink( centity_t *cent, const struct weaponInfo_s *weapon )
{
	FX_AddSprite( cent->lerpOrigin, NULL, NULL, 8.0f + ( random() * 24.0f ), 0.0f, 1.0f, 1.0f, 0, 0.0f, 1, cgs.media.borgFlareShader);
}

/*
-------------------------
FX_BorgWeaponHitWall
-------------------------
*/

void FX_BorgWeaponHitWall( vec3_t origin, vec3_t normal )
{
	vec3_t			new_org;

	VectorMA( origin, 1, normal, new_org );
	FX_AddCylinder( new_org, normal, 1.0f, 0.0f, 1.0f, 256.0f, 32.0f, -128.0f, 1.0f, 0.0f, 250, cgs.media.borgRingShader );

	cgi_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, cg_weapons[WP_BORG_WEAPON].missileHitSound );
}

/*
-------------------------
FX_BorgWeaponHitPlayer
-------------------------
*/

void FX_BorgWeaponHitPlayer( vec3_t origin, vec3_t normal )
{
	FX_AddSprite( origin, NULL, NULL, 64.0f + ( random() * 32.0f ), 16.0f, 1.0f, 0.5f, 0, 0.0f, 50.0f, cgs.media.borgFlareShader);

	cgi_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, cg_weapons[WP_BORG_WEAPON].missileHitSound );
}

/*
-------------------------
FX_BorgTaser
-------------------------
*/

void FX_BorgTaser( vec3_t end, vec3_t start )
{
	FX_AddSprite( end, NULL, NULL, 4.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 250, cgs.media.borgLightningShaders[0] );

	FX_AddLine( start, end, 2.0f, 2.0f, 0.0f, 1.0f, 0.0f, 250, cgs.media.borgLightningShaders[1] );
	FX_AddElectricity( start, end, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 250, cgs.media.borgLightningShaders[2], FXF_TAPER, 0.25f );
	FX_AddElectricity( start, end, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 250, cgs.media.borgLightningShaders[3], FXF_TAPER, 0.25f );
}

/*
-------------------------
FX_BWTaser
-------------------------
*/

void FX_BWTaser( vec3_t end, vec3_t start )
{
	FX_AddSprite( end, NULL, NULL, 4.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 250, cgs.media.BWLightningShaders[0] );

	FX_AddLine( start, end, 2.0f, 2.0f, 0.0f, 1.0f, 0.0f, 250, cgs.media.BWLightningShaders[1] );
	FX_AddElectricity( start, end, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 250, cgs.media.BWLightningShaders[2], FXF_TAPER, 0.25f );
	FX_AddElectricity( start, end, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 250, cgs.media.BWLightningShaders[3], FXF_TAPER, 0.25f );
}

/*
-------------------------
FX_BorgRecycleParticles
-------------------------
*/

void FX_BorgRecycleParticles( vec3_t origin, vec3_t angles, vec3_t velocity, vec3_t user )
{
	vec3_t	vel, accel, org;
	int		numParts = 64;

	for ( int i = 0; i < numParts; i++ )
	{
		vel[0] = crandom();
		vel[1] = crandom();
		vel[2] = crandom();

		org[0] = origin[0] + (crandom() * 24 );
		org[1] = origin[1] + (crandom() * 24 );
		org[2] = origin[2] + (crandom() * 8 );

		VectorScale( vel, 16 + (random() * 32), vel );
		VectorScale( vel, -4, accel );
		
		FX_AddSprite( org, vel, accel, 2.0f, 16.0f, 1.0f, 0.0f, random()*360, 0, 250, cgs.media.borgFlareShader );
	}
}

/*
-------------------------
FX_BorgRecycle
-------------------------
*/

void FX_BorgRecycle( vec3_t origin )
{
	FX_AddSpawner( origin, NULL, NULL, NULL, 10, 5, 500 + (crandom() * 100), FX_BorgRecycleParticles, NULL );
}


/*
-------------------------
FX_BorgDeathSparksParticles
-------------------------
*/

void FX_BorgDeathSparkParticles( vec3_t origin, vec3_t angles, vec3_t vel, vec3_t user )
{
	FXTrail	*particle;
	vec3_t	start, dir;
	float	scale;

	VectorSet( dir, 0,0,1 );

	VectorCopy( origin, start );
	start[0] += crandom() * 4;
	start[1] += crandom() * 4;
	start[2] += random() * 4;

	scale = 0.2f + random() * 0.5f;
	particle = FX_AddTrail( start,
							NULL,
							NULL,
							3.0f + random() * 4.0f,
							-20.0f,
							scale,
							-scale,
							0.6f,
							0.6f,
							0.25f,
							2000.0f,
							cgs.media.sparkShader,
							rand() & FXF_BOUNCE );

	if ( particle == NULL )
		return;

	FXE_Spray( dir, 90, 80, 0.95f, 250 + random() * 400, (FXPrimitive *) particle );
}


/*
-------------------------
FX_BorgDeathSparks
-------------------------
*/

void FX_BorgDeathSparks( vec3_t origin )
{
	FX_AddSpawner( origin, NULL, NULL, NULL, 40, 30, 100 + random() * 50.0f, FX_BorgDeathSparkParticles, NULL );
}
