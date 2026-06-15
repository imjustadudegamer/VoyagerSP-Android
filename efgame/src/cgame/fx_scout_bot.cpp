// Scout Bot weapon effects

#include "cg_local.h"
#include "FX_Public.h"

#define	LASER_TRAIL_MIN_DISTANCE	2

//------------------------------------------------
void FX_BotLaser( vec3_t start, vec3_t end, vec3_t normal, qboolean impact )
//------------------------------------------------
{
	vec3_t	rgb = {1.0f,0.8f,0.4f}, rgb2 = {1.0f, 0.7f, 0.2f};
	float	scale, life;

	FX_AddSprite( start, NULL, NULL, 
					1.5f, 1.0f, 
					0.75, 0.0f, 
					rgb, rgb, 
					0.0f, 
					0.0f, 
					20, 
					cgs.media.waterDropShader );

	FX_AddLine( start, end, 
					1.0f, 
					1.0f, 8.0f, 
					0.75, 0.0f, 
					rgb, rgb, 
					20, 
					cgs.media.whiteLaserShader );

	// Doing all of this extra stuff would look weird if it hits a player ent.
	if ( impact )
	{
		FX_AddQuad( end, normal, NULL, NULL, 
					3.0f, 1.0f, 
					0.75, 0.0f, 
					rgb, rgb, 
					0.0f, 0.0f, 
					0.0f, 
					20, 
					cgs.media.waterDropShader );

		scale = crandom() * 0.5f + 1.75f;
		life = crandom() * 300 + 1000;

		FX_AddQuad( end, normal, NULL, NULL, 
					scale, -0.1f, 
					1.0f, 0.0f, 
					rgb2, rgb2, 
					0, 0, 
					0, 
					life, 
					cgs.media.waterDropShader );
		
		FX_AddQuad( end, normal, NULL, NULL, 
					scale * 2.3f, 0.0f, 
					1.0f, 0.0f, 
					0, 0, 
					0, 
					life * 2, 
					cgs.media.smokeShader );
	}

	scale = 1.0f + (random() * 3.0f);

	CG_Smoke( end, normal, scale, 12.0f, cgs.media.steamShader );
}

/*
-------------------------
FX_BotProjFunc
-------------------------
*/

void FX_BotProjFunc( centity_t *cent, const struct weaponInfo_s *weapon )
{
	vec3_t		diff, org;
	float		len;

	// Make a laser trail that's reasonably consistent and not so much based on frame rate.
	VectorSubtract( cent->lerpOrigin, cent->gent->pos1, diff );
	len = VectorNormalize( diff );

	if ( len > LASER_TRAIL_MIN_DISTANCE )
	{
		for ( float i = 0 ; i < len; i += LASER_TRAIL_MIN_DISTANCE )
		{
			VectorMA( cent->lerpOrigin, i, diff, org );

			// glowing crap
			if ( rand() & 1 )
				FX_AddSprite( org, NULL, NULL, 3, -6, 1.0, 0.0f, random() * 360, 0, 60, cgs.media.yellowParticleShader );
			else
				FX_AddSprite( org, NULL, NULL, 3, -6, 1.0, 0.0f, random() * 360, 0, 60, cgs.media.orangeParticleShader );

			FX_AddSprite( org, NULL, NULL, 8, -12, 0.2f, 0.2f, random() * 360, 0, 60, cgs.media.dkorangeParticleShader );
		}

		VectorCopy( cent->lerpOrigin, cent->gent->pos1 );
	}
}