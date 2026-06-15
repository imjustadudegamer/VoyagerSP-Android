// Dreadnought FX

#include "cg_local.h"
#include "FX_Public.h"

/*
-------------------------
FX_DreadnoughtHitWall
-------------------------
*/
void FX_DreadnoughtHitWall( vec3_t origin, vec3_t normal, qboolean spark )
{
	FXTrail	*particle;
	float	scale;
	int		num, i;

	FX_AddQuad( origin, normal, NULL, NULL, 48.0 + random() * 64, 0, 0.5, 0.5,0, 0,0,1,cgs.media.purpleParticleShader );
	FX_AddQuad( origin, normal, NULL, NULL, 32.0 + random() * 48, 0, 0.6f, 0.6f,0, 0,0,1,cgs.media.ltblueParticleShader );

	CG_ImpactMark( cgs.media.scavMarkShader, origin, normal, random()*360, 1,1,1,0.2f, qfalse, 
					random() * 4 + 8, qfalse );

	if ( spark )
	{
		cgi_R_AddLightToScene( origin, 75 + (rand()&31), 1.0, 0.8f, 1.0 );

		//Sparks
		num = 1 + (rand() & 3);
		
		for ( i = 0; i < num; i++ )
		{	
			scale = 1.0f + (random() * 0.5f);

			particle = FX_AddTrail( origin,
									NULL,
									NULL,
									8.0f + (random() * 8.0f),
									-32.0f,
									scale,
									0.0f,
									0.8f,
									0.0f,
									0.25f,
									600.0f + random() * 250.0f,
									cgs.media.spark2Shader );

			if ( particle == NULL )
				return;

			FXE_Spray( normal, 200, 75, 0.7f, 512, (FXPrimitive *) particle );
		}

/*	// OLD SPARKS

		// Drop some sparks
		num = (int)(random() * 2) + 2;

		for ( i = 0; i < num; i++ )
		{
			scale = 0.6f + random();
			particle = FX_AddTrail( origin, NULL, NULL, 8.0f + random() * 8, -48.0f,
									scale, -scale, 1.0f, 0.8f, 0.4f, 600.0f, cgs.media.spark2Shader, rand() & FXF_BOUNCE );

			if ( rand() & 1 )
				FXE_Spray( normal, 70, 80, 0.9f, 640, (FXPrimitive *) particle );
			else
				FXE_Spray( normal, 80, 200, 0.5f, 512, (FXPrimitive *) particle );
		}
*/
	}
}

/*
-------------------------
FX_DreadnoughtHitPlayer
-------------------------
*/
void FX_DreadnoughtHitPlayer( vec3_t origin, vec3_t normal )
{
	cgi_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, cgi_S_RegisterSound("sound/weapons/dreadnought/hit.wav") );
	CG_ExplosionEffects( origin, 1.0f, 100 );
}

/*
-------------------------
FX_DreadnoughtFire
-------------------------
*/
void FX_DreadnoughtFire( vec3_t origin, vec3_t end, vec3_t normal, qboolean spark )
{
	float	scale = 1.0f + ( random() * 1.0f );

	if ( spark )
		FX_AddElectricity( origin, end, 1.0f, scale * 0.5f, 0, 0.8f, 0.8f, 1, cgs.media.dnBoltShader, FXF_WRAP, 0.25f );

	FX_AddElectricity( origin, end, 1.0f, scale * 0.75f, 0, 0.8f, 0.8f, 1, cgs.media.dnBoltShader, FXF_WRAP, 0.18f );

	FX_AddLine( origin, end, 128.0f, scale * 6, 0.0f, 0.2f, 0.2f, 1, cgs.media.dnBoltShader, FXF_ADJUST_ST );
	FX_AddLine( origin, end, 256.0f, scale * 4.5, 0.0f, 0.8f, 0.8f, 1, cgs.media.dnBoltShader, FXF_ADJUST_ST );
//	FX_AddLine( origin, end, 2.0f, scale * 6, 0.0f, 0.2f, 0.2f, 1, cgs.media.dnBoltShader );
//	FX_AddLine( origin, end, 1.0f, scale * 4.5, 0.0f, 0.8f, 0.8f, 1, cgs.media.dnBoltShader );
	
	// Add a subtle screen shake
	CG_ExplosionEffects( origin, 1.0f, 15 );

	FX_DreadnoughtHitWall( end, normal, spark );
}

/*
-------------------------
FX_DreadnoughtProjectileThink

Freaky random lightning burst
-------------------------
*/
#define FX_DN_ALT_THINK_TIME	100

void FX_DreadnoughtProjectileThink( centity_t *cent, const struct weaponInfo_s *weapon )
{
	float scale;
	
	scale = Q_flrand( 10.0f, 15.0f );

	// If this is a new thinking time, draw some starting stuff...
	if ( cent->miscTime < cg.time )
	{
		trace_t trace;
		vec3_t fwd, right, boltdir, boltend, mins={-2,-2,-2}, maxs={2,2,2};
		float len;
		int	playSound = 1;

		cent->miscTime = cg.time + FX_DN_ALT_THINK_TIME;

		VectorSubtract( cent->currentState.origin, cent->currentState.origin2, fwd );

		// Throw a sprite from the start to the end over the next 
		VectorScale( fwd, 1000.0 * ( 1.0 / FX_DN_ALT_THINK_TIME ), boltdir );
		FX_AddSprite( cent->currentState.origin2, boltdir, NULL, scale * 8, -scale*20, 1.0, 1.0, 0, 0, 
						FX_DN_ALT_THINK_TIME, cgs.media.blueParticleShader );
		FX_AddSprite( cent->currentState.origin2, boltdir, NULL, scale * 8, -scale*10, 1.0, 1.0, 0, 0, 
						FX_DN_ALT_THINK_TIME, cgs.media.purpleParticleShader );
		
		len = VectorNormalize( fwd );
		
		// Illegal if org and org2 are the same.
		if ( len <= 0 )
			return;

		// Draw a bolt from the old position to the new.
//		FX_AddLine( cent->currentState.origin2, cent->currentState.origin, 
//						1.0f, scale * 4, -scale * 20, 
//						0.75f, 0.0f, 
//						FX_DN_ALT_THINK_TIME * 2, cgs.media.dnBoltShader );
		FX_AddElectricity( cent->currentState.origin2, cent->currentState.origin, 
						1.0f, scale * 3, -scale * 15, 
						1.0f, 0.5f, 
						FX_DN_ALT_THINK_TIME * 2, cgs.media.dnBoltShader, FXF_WRAP | FXF_TAPER, 0.6f );

		// ALSO draw an electricity bolt from the old position to the new.
		FX_AddElectricity( cent->currentState.origin2, cent->currentState.origin, 
						1.0f, scale, -scale * 5, 
						1.0f, 0.5f, 
						FX_DN_ALT_THINK_TIME * 2, cgs.media.dnBoltShader, FXF_WRAP | FXF_TAPER, 0.6f );

		// And a bright new sprite at the current locale.
		FX_AddSprite( cent->currentState.origin, NULL, NULL, scale * 2, scale * 40, 
						1.0f, 1.0f,
						0, 0, FX_DN_ALT_THINK_TIME, cgs.media.blueParticleShader );

		// Put a sprite in the old position, fading away.
		FX_AddSprite( cent->currentState.origin2, NULL, NULL, scale * 5, -scale * 25, 
						1.0f, 1.0f, 
						0, 0, FX_DN_ALT_THINK_TIME * 2, cgs.media.blueParticleShader);

		// Shoot rays out (roughly) to the sides to connect with walls or whatever...
		// PerpendicularVector(right, fwd);
		right[0] = fwd[1];
		right[1] = -fwd[0];
		right[2] = -fwd[2];

		// Right vector
		// The boltdir uses a random offset to the perp vector.
		boltdir[0] = right[0] + Q_flrand(-0.25, 0.25);
		boltdir[1] = right[1] + Q_flrand(-0.25, 0.25);
		boltdir[2] = right[2] + Q_flrand(-1.0, 1.0);

		// Shoot a vector off to the side and trace till we hit a wall.
		VectorMA( cent->currentState.origin, 256, boltdir, boltend );
		CG_Trace( &trace, cent->currentState.origin, mins, maxs, boltend, cent->currentState.number, MASK_SOLID );
	
		if ( trace.fraction < 1.0 )
		{
			VectorCopy( trace.endpos, boltend );
			FX_AddElectricity( cent->currentState.origin, boltend, 
				1.0f, scale * 2, -scale * 10, 
				1.0f, 0.5f, 
				FX_DN_ALT_THINK_TIME * 2, cgs.media.dnBoltShader, FXF_WRAP | FXF_TAPER, 0.8f );

			// Put a sprite at the endpoint that stays.
			FX_AddQuad( trace.endpos, trace.plane.normal, NULL, NULL,
				scale, -scale * 2, 
				1.0f, 0.5f, 0.0, 0, 0,  
				FX_DN_ALT_THINK_TIME * 2, cgs.media.blueParticleShader );

			if ( playSound )
			{
				if ( Q_irand( 0, 1 ))
				{
					weaponInfo_t	*weaponInfo = &cg_weapons[WP_DREADNOUGHT];
					cgi_S_StartSound( trace.endpos, ENTITYNUM_WORLD, CHAN_AUTO, weaponInfo->altmissileHitSound );
					playSound = 0;
				}
			}
		}

		// Left vector
		// The boltdir uses a random offset to the perp vector.
		boltdir[0] = -right[0] + Q_flrand(-0.25, 0.25);
		boltdir[1] = -right[1] + Q_flrand(-0.25, 0.25);
		boltdir[2] = -right[2] + Q_flrand(-1.0, 1.0);

		// Shoot a vector off to the side and trace till we hit a wall.
		VectorMA( cent->currentState.origin, 256, boltdir, boltend );
		CG_Trace( &trace, cent->currentState.origin, mins, maxs, boltend, cent->currentState.number, MASK_SOLID );
	
		if ( trace.fraction < 1.0 )
		{
			VectorCopy( trace.endpos, boltend );
			FX_AddElectricity( cent->currentState.origin, boltend, 
								0.2f, scale * 2, -scale * 10, 
								1.0f, 0.5f, 
								FX_DN_ALT_THINK_TIME * 2, cgs.media.dnBoltShader, FXF_WRAP | FXF_TAPER, 0.8f );

			// Put a sprite at the endpoint that stays.
			FX_AddQuad( trace.endpos, trace.plane.normal, NULL, NULL,
								scale, -scale * 2, 
								1.0f, 0.5f, 0.0, 0, 0,
								FX_DN_ALT_THINK_TIME * 2, cgs.media.blueParticleShader );

			if ( playSound )
			{
				weaponInfo_t	*weaponInfo = &cg_weapons[WP_DREADNOUGHT];
				cgi_S_StartSound( trace.endpos, ENTITYNUM_WORLD, CHAN_AUTO, weaponInfo->altmissileHitSound );
				playSound = 0;
			}
		}
	}
}


/*
-------------------------
FX_DreadnoughtTurrentThink

Projectile that the turrets shoot..
-------------------------
*/
void FX_DreadnoughtTurretThink( centity_t *cent, const struct weaponInfo_s *weapon )
{
	float	scale, alpha;
	vec3_t	forward;

	if ( VectorNormalize2( cent->currentState.pos.trDelta, forward ) == 0 )
		forward[2] = 1;


	scale = random() * 16.0f + 24.0f;
	alpha = random() * 0.2f + 0.2f;

	FX_AddTrail( cent->lerpOrigin, 
					forward, NULL, 
					64.0f, 0.0f, 
					1.5f, 0.0f, 
					0.4f, 0.4f, 
					0.0f, 1, 
					cgs.media.yellowTrailShader );
	FX_AddSprite( cent->lerpOrigin, NULL, NULL, scale, scale, alpha, alpha, 0.0f, 0.0f, 1, cgs.media.orangeParticleShader );
	FX_AddSprite( cent->lerpOrigin, NULL, NULL, 6.0f, 6.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1, cgs.media.yellowParticleShader );
}



/*
-------------------------
FX_DNTurretHitWall
-------------------------
*/
void FX_DNTurretHitWall( vec3_t origin, vec3_t normal )
{
	FX_AddQuad( origin, normal, 
						NULL, NULL, 
						0.5f, 32.0f, 
						0.8f, 0.0f, 
						random() * 360.0f, 0.0f, 
						0.0, 
						200, 
						cgs.media.yellowParticleShader );

	FX_AddSprite( origin,
						NULL, NULL,
						1.0f, 32.0f,
						0.8f, 0.0f,
						0.0f, 0.0f,
						200,
						cgs.media.yellowParticleShader );

	//Sound
//	cgi_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, cg_weapons[WP_DN_TURRET].missileHitSound );

	CG_ImpactMark( cgs.media.scavMarkShader, origin, normal, random()*360, 1,1,1,0.2f, qfalse, random() + 5.5f, qfalse );
}


/*
-------------------------
FX_DNTurretHitPlayer
-------------------------
*/
void FX_DNTurretHitPlayer( vec3_t origin, vec3_t normal )
{
	FX_AddQuad( origin, normal, 
					NULL, NULL, 
					0.5f, 32.0f, 
					0.8f, 0.0f, 
					random() * 360.0f, 0.0f, 
					0.0f, 
					125, 
					cgs.media.yellowParticleShader );

	FX_AddSprite( origin,
						NULL, NULL,
						1.0f, 32.0f,
						0.8f, 0.0f,
						0.0f, 0.0f,
						200,
						cgs.media.yellowParticleShader );

	//Sound
//	cgi_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, cg_weapons[WP_DN_TURRET].missileHitSound );
}

/*
-------------------------
FX_DreadnoughtShotMiss

Alt-fire, miss effect
-------------------------
*/
void FX_DreadnoughtShotMiss( vec3_t end, vec3_t dir )
{
	vec3_t	org;

	// Move me away from the wall a bit so that I don't z-buffer into it
	VectorMA( end, 0.5f, dir, org );

	// Expanding rings
	FX_AddQuad( org, dir, NULL, NULL, 1, 54, 0.8f, 0.2f, random() * 360, 0, 0, 400, cgs.media.stasisRingShader );
	FX_AddQuad( org, dir, NULL, NULL, 1, 120, 0.8f, 0.2f, random() * 360, 0, 0, 300, cgs.media.stasisRingShader );
	// Impact effect
	FX_AddQuad( org, dir, NULL, NULL, 7, 70, 1.0f, 0.0f, random() * 360, 0, 0, 500, cgs.media.blueParticleShader );
	FX_AddQuad( org, dir, NULL, NULL, 5, 55, 1.0f, 0.0f, random() * 360, 0, 0, 420, cgs.media.purpleParticleShader );

	CG_ImpactMark( cgs.media.scavMarkShader, org, dir, random()*360, 1,1,1,0.6f, qfalse, 
				8 + random() * 2, qfalse );

	FX_AddSprite( end, NULL, NULL, Q_flrand( 40, 60 ), -100, 1.0f, 0.0f, random() * 360, 0, 500, cgs.media.blueParticleShader );
}