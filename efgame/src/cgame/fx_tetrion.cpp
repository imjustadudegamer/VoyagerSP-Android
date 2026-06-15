// Tetrion FX

#include "cg_local.h"
#include "FX_Public.h"

/*
-------------------------
FX_TetrionWeaponHitWall
-------------------------
*/
void FX_TetrionWeaponHitWall( vec3_t origin, vec3_t normal )
{
	vec3_t	vel, accel, org;
	int		t; 
	float	scale;

	scale = random() * 2.5 + 1.5;

	CG_ImpactMark( cgs.media.bulletmarksShader, origin, normal, random()*360, 1,1,1,0.2f, qfalse, 
					scale, qfalse );

	// Move out a hair to avoid z buffer nastiness
	VectorMA( origin, 0.5, normal, org );

	// Add a bit of variation every now and then
	if ( rand() & 1 )
	{
		FX_AddQuad( org, normal, 
					NULL, NULL, 
					scale * 2, -4, 
					0.5, 0.5, 
					0, 0, 
					0, 
					175, 
					cgs.media.sunnyFlareShader );
	}

	FX_AddQuad( org, normal, 
				NULL, NULL, 
				scale * 4, -8, 
				1.0, 1.0, 
				0, 0, 
				0, 
				175, 
				cgs.media.borgFlareShader );

	// Add some smoke puffs
	for ( int i = 0; i < 2; i ++ )
	{
		for ( t = 0; t < 3; t++ )
		{
			vel[t] = normal[t] + crandom();
		}

		VectorScale( vel, 12 + random() * 12, vel );

		VectorScale( vel, -0.25, accel );
		FX_AddSprite( origin, 
						vel, accel, 
						random() * 4 + 2, 12, 
						0.6 + random() * 0.4, 0.0, 
						random() * 180, 
						0.0, 
						random() * 200 + 300, 
						cgs.media.steamShader );
	}
}

/*
-------------------------
FX_TetrionShot
-------------------------
*/
void FX_TetrionShot( vec3_t start, vec3_t end, vec3_t forward )
{
	vec3_t	new_start, new_end, dir, org;
	float	off, len;
	
	// Get the length of the whole shot
	VectorSubtract( end, start, dir ); 
	len = VectorNormalize( dir );

	// Don't do tracers when it gets really short
	if ( len < 128 + random() * 128 )
		return;

	// Move the end_point in a bit so the tracer doesn't always trace the full line length--this isn't strictly necessary, but it does
	//		add a bit of variance
	VectorCopy( start, new_start );
	off = random() * 0.3 + 0.7;
	VectorMA( start, len * off, dir, new_end );

	// Trace back from the impact point toward the muzzle point get a new start (muzzle) point
	VectorMA( end, len, forward, org );

	// Now we are on the muzzle end, and due to spread, the start point could be much wider than the actual muzzle
	//		so we may need to move the point closer to the muzzle point so it doesn't get out of control
	VectorSubtract( start, org, dir );
	len = VectorNormalize( dir );

	// do a rough clamp to the size of the muzzle 
	if ( len > 3.5 )
	{
		len *= -0.06f;	// This value may or may not be ok?
		VectorMA( new_start, len, dir, new_start );
	}
	else
	{
		// The size was ok, so just use the unadjusted value
		VectorCopy( org, new_start );
	}

	// Draw the tracer
	FX_AddLine( new_end, new_start, 1.0f, 1.5f + random(), 0.0f, random() * .3 + .3, 0.0, 1.0, cgs.media.borgFlareShader );
}

/*
-------------------------
FX_TetrionProjectileThink
-------------------------
*/
void FX_TetrionProjectileThink( centity_t *cent, const struct weaponInfo_s *weapon )
{
	vec3_t forward, rgb;

	if ( VectorNormalize2( cent->currentState.pos.trDelta, forward ) == 0 )
		forward[2] = 1;

	VectorSet( rgb, 0.4f, 1.0f, 0.2f ); // green

	FX_AddSprite( cent->lerpOrigin, NULL, NULL, 4.0f + random() * 16.0f, 0.0f, 0.4f, 0.0f, random()*360, 0.0f, 1.0f, cgs.media.greenBurstShader );
	FX_AddSprite( cent->lerpOrigin, NULL, NULL, 16.0f + random() * 16.0f, 0.0f, 0.6f, 0.0f, random()*360, 0.0f, 1.0f, cgs.media.borgFlareShader );
	FX_AddSprite( cent->lerpOrigin, NULL, NULL, 32.0f + random() * 8.0f, 0.0f, 0.1f, 0.1f, rgb, rgb, 0.0f, 0.0f, 1.0f, cgs.media.whiteRingShader );
	FX_AddTrail( cent->lerpOrigin, forward, NULL, 96, 0, 4.0f + random(), 0, 0.4f, 0, 0, 1, cgs.media.greenTrailShader );
}

/*
-------------------------
FX_TetrionRicochet
-------------------------
*/
void FX_TetrionRicochet( vec3_t origin, vec3_t normal )
{
	vec3_t org;

	// Move away from the wall a bit to help avoid z buffer clipping.
	VectorMA( origin, 0.5, normal, org );

	FX_AddQuad( org, normal, NULL, NULL, 24, -24, 1.0, 0.0, 0,0,0,300, cgs.media.greenBurstShader );
	FX_AddQuad( org, normal, NULL, NULL, 48, -48, 0.5, 0.0, 0,0,0,300, cgs.media.borgFlareShader );
}

/*
-------------------------
FX_TetrionAltHitWall
-------------------------
*/
void FX_TetrionAltHitWall( vec3_t origin, vec3_t normal )
{
	vec3_t	org, vel, accel;
	float	scale;
	int		t;

	scale = random() * 3.0 + 8.0;

	CG_ImpactMark( cgs.media.bulletmarksShader, origin, normal, random()*360, 1,1,1,0.2f, qfalse, 
					scale, qfalse );

	// Move out a hair to avoid z buffer nastiness..could use polygon offset for this...sigh.
	VectorMA( origin, 0.5, normal, org );

	FX_AddQuad( origin, normal, NULL, NULL, 64, -96, 1.0, 0.0, 0,0,0,200, cgs.media.greenBurstShader );
	FX_AddQuad( origin, normal, NULL, NULL, 128, -192, 1.0, 0.0, 0,0,0,200, cgs.media.borgFlareShader );

	// Add some smoke puffs..this may just be a big waste....
	for ( int i = 0; i < 3; i ++ )
	{
		for ( t = 0; t < 3; t++ )
		{
			vel[t] = normal[t] + crandom();
		}

		VectorScale( vel, 12 + random() * 12, vel );

		VectorScale( vel, -0.25, accel );
		FX_AddSprite( origin, 
						vel, accel, 
						random() * 6 + 4, 24, 
						0.6 + random() * 0.4, 0.0, 
						random() * 180, 
						0.0, 
						random() * 300 + 300, 
						cgs.media.steamShader );
	}

	cgi_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, cg_weapons[WP_TETRION_DISRUPTOR].altmissileHitSound );
}
