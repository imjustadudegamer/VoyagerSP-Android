// Scavenger Weapon and Teleport FX

#include "cg_local.h"
#include "FX_Public.h"

/*
-------------------------
FX_ScavengerProjectileThink
-------------------------
*/
#define SCAV_SPIN	0.3

void FX_ScavengerProjectileThink( centity_t *cent, const struct weaponInfo_s *weapon )
{
	vec3_t		forward, org;
	FXSprite	*fx = 0;
	qboolean	fired_from_NPC = (qboolean)cent->gent->count;
	qboolean	greyscale = (qboolean)(cent->currentState.weapon == WP_CHAOTICA_GUARD_GUN);
	qhandle_t	flareShader = greyscale ? cgs.media.tetrionFlareShaderBW: cgs.media.tetrionFlareShader;
	qhandle_t	flare2Shader = greyscale ? cgs.media.redFlareShaderBW: cgs.media.redFlareShader;
	qhandle_t	trailShader = greyscale ? cgs.media.tetrionTrail2ShaderBW: cgs.media.tetrionTrail2Shader;

	if ( VectorNormalize2( cent->currentState.pos.trDelta, forward ) == 0 )
		forward[2] = 1;

	// The effect you'd see from first person looks horrible from third person..or when shot by an NPC,
	//		so we'll just tone down the effect so it's not so horrible. :)
	if ( fired_from_NPC )
	{
		FXCylinder *fx2 = 0;

		VectorMA( cent->lerpOrigin, -6, forward, org );
		// Energy glow
		FX_AddSprite( org, 
						NULL, NULL, 
						8.0f + random() * 8.0f, 0.0f, 
						0.5f, 0.0f, 
						random()*360, 0.0f, 
						1.0f, 
						flareShader );

		// leave a cool tail
		FX_AddTrail( cent->lerpOrigin, 
						forward, NULL, 
						24, 0, 
						0.5f, 0.0f, 
						0.6f, 0.0f, 
						0, 
						50, 
						trailShader );

		VectorScale( forward, -1, forward );
		fx2 = FX_AddCylinder(cent->lerpOrigin, forward, 16, 0, 0.1f, 0, 3,0,0.6f, 0.6f,  1, flareShader, 0.1f );

		if ( fx2 == NULL )
			return;

		fx2->SetFlags( FXF_WRAP );
	}
	else
	{
		// Energy glow
		FX_AddSprite( cent->lerpOrigin, 
						NULL, NULL, 
						16.0f + random() * 16.0f, 0.0f, 
						0.5f, 0.0f, 
						random() * 360, 0.0f, 
						1.0f, 
						flareShader );

		// Spinning projectile
		fx = FX_AddSprite( cent->lerpOrigin, 
						NULL, NULL, 
						4.0f + random() * 10.0f, 0.0f, 
						0.6f, 0.0f, 
						0, 0.0f, 
						1.0f, 
						flare2Shader );

		if ( fx == NULL )
			return;

		fx->SetRoll( cg.time * SCAV_SPIN );

		// leave a cool tail
		FX_AddTrail( cent->lerpOrigin, 
						forward, NULL, 
						64, 0, 
						1.4f, 0.0f, 
						0.6f, 0.0f, 
						0, 
						50, 
						trailShader );
	}
}


/*
-------------------------
FX_ScavengerAltFireThink
-------------------------
*/
#define SCAV_TRAIL_SPACING 12

void FX_ScavengerAltFireThink( centity_t *cent, const struct weaponInfo_s *weapon )
{
	vec3_t	diff;
	float	len;

	// Make a trail that's reasonably consistent and not so much based on frame rate.
	VectorSubtract( cent->lerpOrigin, cent->gent->pos1, diff );
	len = VectorNormalize( diff );

	if ( len > SCAV_TRAIL_SPACING )
	{
		FXSprite	*fx;
		vec3_t		origin;
		int			i;

		for ( i = 0 ; i < len; i += SCAV_TRAIL_SPACING )
		{
			// Calc the right spot along the trail
			VectorMA( cent->lerpOrigin, -i, diff, origin );
			fx = FX_AddSprite( origin, 
								NULL, NULL, 
								18.0f + ( random() * 5.0f ), -35.0f, 
								0.4f, 0.0f, 
								0.f, 0.0f, 
								250.0f, 
								cgs.media.scavengerAltShader );


			if ( fx == NULL )
				return;
			
			fx->SetRoll( random() * 360 );
		}

		// Stash the current position
		VectorCopy( cent->lerpOrigin, cent->gent->pos1 );
	}

	// Glowing bit
	FX_AddSprite( cent->lerpOrigin, 
					NULL, NULL, 
					24.0f + ( random() * 16.0f ), 0.0f, 
					1.0f, 0.0f, 
					0, 0.0f, 
					1.0f, 
					cgs.media.tetrionFlareShader );
}

/*
-------------------------
FX_ScavengerWeaponHitWall
-------------------------
*/
void FX_ScavengerWeaponHitWall( vec3_t origin, vec3_t normal, qboolean fired_by_NPC, qboolean greyscale )
{
	qhandle_t	flareShader = greyscale ? cgs.media.tetrionFlareShaderBW : cgs.media.tetrionFlareShader;
	qhandle_t	ringShader = greyscale ? cgs.media.redRingShaderBW : cgs.media.redRingShader;
	sfxHandle_t	missileSound = greyscale ? cg_weapons[WP_CHAOTICA_GUARD_GUN].missileHitSound : cg_weapons[WP_SCAVENGER_RIFLE].missileHitSound;

	// Tone down when shot by an NPC
	if ( fired_by_NPC )
	{
		// Expanding shock ring
		FX_AddQuad( origin, normal, 
						NULL, NULL, 
						0.5f, 24.0f, 
						0.5, 0.0, 
						random() * 360.0f, 0.0f, 
						0.0, 
						200, 
						ringShader );

		// Impact core
		FX_AddQuad( origin, normal, 
						NULL, NULL, 
						8.0f + ( random() * 6.0f ), 28.0f, 
						0.6f, 0.0f, 
						cg.time * SCAV_SPIN, 0, 
						0.0f, 
						250, 
						flareShader );
	}
	else
	{
		// Expanding shock ring
		FX_AddQuad( origin, normal, 
						NULL, NULL, 
						0.5f, 32.0f, 
						0.5, 0.0, 
						random() * 360.0f, 0.0f, 
						0.0, 
						200, 
						ringShader );

		// Impact core
		FX_AddQuad( origin, normal, 
						NULL, NULL, 
						12.0f + ( random() * 8.0f ), 32.0f, 
						0.8f, 0.0f, 
						cg.time * SCAV_SPIN, 0, 
						0.0f, 
						300, 
						flareShader );
	}

	//Sound
	cgi_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, missileSound );

	CG_ImpactMark( cgs.media.scavMarkShader, origin, normal, random()*360, 1,1,1,0.2f, qfalse, random() + 5.5f, qfalse );
}


/*
-------------------------
FX_ScavengerWeaponHitPlayer
-------------------------
*/
void FX_ScavengerWeaponHitPlayer( vec3_t origin, vec3_t normal, qboolean fired_by_NPC, qboolean greyscale )
{
	qhandle_t	ringShader = greyscale ? cgs.media.redRingShaderBW: cgs.media.redRingShader;
	sfxHandle_t	missileSound = greyscale ? cg_weapons[WP_CHAOTICA_GUARD_GUN].missileHitSound : cg_weapons[WP_SCAVENGER_RIFLE].missileHitSound;

	if ( fired_by_NPC )
	{
		// Smaller expanding shock ring
		FX_AddQuad( origin, normal, 
						NULL, NULL, 
						0.5f, 16.0f, 
						0.4f, 0.0, 
						random() * 360.0f, 0.0f, 
						0.0, 
						125, 
						ringShader );
	}
	else
	{
		// Expanding shock ring
		FX_AddQuad( origin, normal, 
						NULL, NULL, 
						1.0f, 24.0f, 
						0.4f, 0.0, 
						random() * 360.0f, 0.0f, 
						0.0, 
						125, 
						ringShader );
	}

	//Sound
	cgi_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, missileSound );
}

/*
-------------------------
FX_Scavenger_Alt_Explode
------------------------- 
*/
void FX_ScavengerAltExplode( vec3_t origin, vec3_t dir )
{
	FXTrail			*fx;
	FXCylinder		*fx2;
	localEntity_t	*le;
	vec3_t			direction, org;
	
	//Orient the explosions to face the camera
	VectorSubtract( cg.refdef.vieworg, origin, direction );
	VectorNormalize( direction );

	VectorMA( origin, 12, direction, org );
	// Add an explosion and tag a light to it
	le = CG_MakeExplosion( org, direction, cgs.media.explosionModel, 6, cgs.media.scavExplosionSlowShader, 675, qfalse, 1.0f + (random()*0.5f) );
	le->light = 150;
	le->refEntity.renderfx |= RF_NOSHADOW;
	VectorSet( le->lightColor, 1.0f, 0.6f, 0.6f );

	VectorSet( org, (org[0] + crandom() * 8), (org[1] + crandom() * 8), (org[2] + crandom() * 8) );
	CG_MakeExplosion( org, direction, cgs.media.explosionModel, 6, cgs.media.scavExplosionFastShader, 375, qfalse, 0.7f + (random()*0.5f) );

	//Sound
	cgi_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.scavengerAltExplodeSnd );	

	CG_ImpactMark( cgs.media.compressionMarkShader, origin, dir, random()*360, 1,1,1,1.0, qfalse, 
					random() * 8 + 64, qfalse );

	CG_ImpactMark( cgs.media.compressionMarkShader, origin, dir, random()*360, 1,1,1,1.0, qfalse, 
					random() * 8 + 8, qfalse );

	// Always orient horizontally
	VectorSet ( direction, 0,0,1 );

	fx2 = FX_AddCylinder( origin, direction, 4, 0, 20, 350, 14, 200, 1.0, 0.0, 600, cgs.media.redRing2Shader, 1.5 );

	if ( fx2 == NULL )
		return;

	fx2->SetFlags( FXF_WRAP );
	fx2->SetSTScale( 6 );

	for ( int i = 0; i < 6; i++)
	{
	fx = FX_AddTrail( origin, NULL, NULL, 12.0f, -15.0f,
								2, -2, 1.0f, 1.0f, 0.2f, 1000.0f,  cgs.media.tetrionTrail2Shader, rand() & FXF_BOUNCE );

	if ( fx == NULL )
		return;

	FXE_Spray( dir, 300, 175, 0.8f, 512, (FXPrimitive *) fx );
	}

	CG_ExplosionEffects( origin, 3.0f, 300 );
}


/*
-------------------------
FX_Disruptor
-------------------------
*/
void DisruptorShards( vec3_t org, vec3_t angles, vec3_t vel, vec3_t user )
{
	vec3_t normal, end;

	// Pick a random endpoint
	VectorSet( normal, crandom(), crandom(), crandom() );
	VectorNormalize( normal );

	end[0] = org[0] + ( normal[0] * ( 48 + crandom() * 16 ));
	end[1] = org[1] + ( normal[1] * ( 48 + crandom() * 16 ));
	end[2] = org[2] + ( normal[2] * ( 64 + crandom() * 24 ));

	// Draw a light shard, use a couple of different kinds so it doesn't look too homogeneous
	if( rand() & 1 )
	{
		FX_AddLine( org, end, 1.0, random() * 0.5 + 0.5, 12.0, random() * 0.1 + 0.1, 0.0, 200 + random() * 350, cgs.media.orangeParticleShader );
	}
	else
	{
		FX_AddLine( org, end, 1.0, random() * 0.5 + 0.5, 12.0, random() * 0.1 + 0.1, 0.0, 200 + random() * 350, cgs.media.yellowParticleShader );
	}
}

// Effect used when scav beams in--this wouldn't work well for a scav on the ground if they were to beam out
void FX_Disruptor( vec3_t org, float length )
{
	//FIXME: make it move with owner?
	vec3_t org1, org2, normal={0,0,1};

	VectorMA( org, 48, normal, org1 );
	VectorMA( org, -48, normal, org2 );

	// This is the core
	FX_AddLine( org1, org2, 1.0, 0.1f, 48.0f, 1.0, 0.0, length, cgs.media.dkorangeParticleShader );

	// Spawn a bunch to get the effect going
	for ( int t=0; t < 12; t++ )
	{
		DisruptorShards( org, NULL, NULL, NULL );
	}

	// Keep spawning the light shards for a while.
	FX_AddSpawner( org, NULL, NULL, NULL, 20, 10, length*0.75, 0, DisruptorShards, NULL );
}

/*
-------------------------
FX_HypoSpray
-------------------------
*/

#define NUM_HYPO_PUFFS	20

void FX_HypoSpray( vec3_t origin, vec3_t dir, qboolean red )  // When not red, it'll be blue
{
	vec3_t	color, vel, accel, angles, work;
	float	scale, dscale;

	vectoangles( dir, angles );

	for ( int i = 0; i < NUM_HYPO_PUFFS; i++ )
	{
		if ( red )
		{
			VectorSet( color, 1.0f, random() * 0.4f, random() * 0.4f ); // mostly red
		}
		else
		{
			VectorSet( color, random() * 0.5f, random() * 0.5f + 0.5f, 1.0f ); // mostly blue
		}

		VectorCopy( angles, work );

		work[0] += crandom() * 12.0f;
		work[1] += crandom() * 12.0f;

		AngleVectors( work, vel, NULL, NULL );

		scale = random() * 256.0f + 128.0f;

		VectorScale( vel, scale, vel );
		VectorScale( vel, random() * -0.3f, accel );

		scale = random() * 4.0f + 2.0f;
		dscale = random() * 64.0f + 24.0f;

		FX_AddSprite( origin, vel, accel, scale, dscale, 0.8f + random() * 0.2f, 0.0f, color, color, crandom() * 120, 0.0f, 1000, cgs.media.steamShader );
	}
}

/*
-------------------------
FX_PaladinProjThink
-------------------------
*/

void FX_PaladinProjThink( centity_t *cent, const struct weaponInfo_s *weapon )
{
	vec3_t		forward, org, color={1.0f, 0.5f, 0.1f};

	if ( VectorNormalize2( cent->currentState.pos.trDelta, forward ) == 0 )
		forward[2] = 1;

	FXCylinder *fx2 = 0;

	VectorMA( cent->lerpOrigin, -6, forward, org );
	// Energy glow
	FX_AddSprite( org, 
					NULL, NULL, 
					32.0f + random() * 8.0f, 0.0f, 
					0.4f, 0.0f, 
					random()*360, 0.0f, 
					1.0f, 
					cgs.media.dkorangeParticleShader );

	// leave a cool tail
	FX_AddTrail( cent->lerpOrigin, 
					forward, NULL, 
					16, 0, 
					1.0f, 4.0f, 
					0.6f, 0.0f, 
					0, 
					50, 
					cgs.media.orangeTrailShader	 );

	VectorScale( forward, -1, forward );
	fx2 = FX_AddCylinder( cent->lerpOrigin, forward, 16, 0, 
								0.1f, 0, 4, 0,
								0.6f, 0.6f,
								color, color,
								1, cgs.media.whiteLaserShader, 0.1f );

	if ( fx2 == NULL )
		return;
}

/*
-------------------------
FX_PaladinProjHitWall
-------------------------
*/

void FX_PaladinProjHitWall( vec3_t origin, vec3_t dir )
{
	vec3_t	angles;

	cgi_S_StartSound( origin, 0, CHAN_AUTO, cgs.media.arrowMissSound );

	//flash?
	CG_Spark( origin, dir );
	vectoangles( dir, angles );
}

/*
-------------------------
FX_PaladinProjHitPlayer
-------------------------
*/

void FX_PaladinProjHitPlayer( vec3_t origin, vec3_t dir )
{
	cgi_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, cg_weapons[WP_PALADIN].missileHitSound );
	//flash?
}

/*
-------------------------
FX_RifleShot
-------------------------
*/
void FX_RifleShot( vec3_t start, vec3_t end )
{
	extern void FX_CompressionShockRing( vec3_t start, vec3_t end );

	FX_AddLine( start, end, 
					1.0f, 
					1.0f, -3.0f, 
					0.6f, 0.0f, 
					100.0f, 
					cgs.media.sparkShader );

	FX_CompressionShockRing( start, end );
}

/*
-------------------------
FX_RifleHitWall
-------------------------
*/

void FX_RifleHitWall( vec3_t origin, vec3_t dir )
{
	vec3_t	angles;

	cgi_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, Q_irand(cgs.media.ric1Sound, cgs.media.ric6Sound) );

	vectoangles( dir, angles );
	CG_Spark( origin, angles );
}

/*
-------------------------
FX_RifleHitPlayer
-------------------------
*/

void FX_RifleHitPlayer( vec3_t origin )
{
	cgi_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.rifleHitSound );
	//flash?
}
