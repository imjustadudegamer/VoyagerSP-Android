// Reever weapon effects

#include "cg_local.h"
#include "FX_Public.h"


/*
-----------------------------------------------------------------------
	REAVER
-----------------------------------------------------------------------
*/
//------------------------------------------------
void FX_ForgeProjectileThink( centity_t *cent, const struct weaponInfo_s *weapon )
//------------------------------------------------
{
	vec3_t forward, rgb;

	if ( VectorNormalize2( cent->currentState.pos.trDelta, forward ) == 0 )
		forward[2] = 1;

	VectorSet( rgb, 1.0f, 0.5f, 0.2f ); // orange

	FX_AddSprite( cent->lerpOrigin, NULL, NULL, 12.0f + random() * 16.0f, 0.0f, 0.6f, 0.0f, random()*360, 0.0f, 1.0f, cgs.media.sunnyFlareShader );
	FX_AddSprite( cent->lerpOrigin, NULL, NULL, 16.0f + random() * 16.0f, 0.0f, 0.6f, 0.0f, random()*360, 0.0f, 1.0f, cgs.media.dkorangeParticleShader );
	FX_AddSprite( cent->lerpOrigin, NULL, NULL, 38.0f + random() * 8.0f, 0.0f, 0.2f, 0.2f, rgb, rgb, 0.0f, 0.0f, 1.0f, cgs.media.whiteRingShader );
	FX_AddTrail( cent->lerpOrigin, forward, NULL, 64, 0, 4.0f + random(), 0, 0.7f, 0, 0, 1, cgs.media.orangeTrailShader );
}

//------------------------------------------------
void FX_ForgeProjHitPlayer( vec3_t origin, vec3_t dir )
//------------------------------------------------
{
	//Sound
	cgi_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, cg_weapons[WP_FORGE_PROJ].missileHitSound );
}

//------------------------------------------------
void FX_ForgeProjHitWall( vec3_t origin, vec3_t dir )
//------------------------------------------------
{
	FX_AddQuad( origin, dir, NULL, NULL, 25.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 250, cgs.media.orangeParticleShader );

	//Sound
	cgi_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, cg_weapons[WP_FORGE_PROJ].missileHitSound );
}