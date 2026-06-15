// Avatar weapon effects

#include "cg_local.h"
#include "FX_Public.h"

#define BEAM_LEN	96.0f

//------------------------------------------------
void FX_ForgePsychThink( centity_t *cent, const struct weaponInfo_s *weapon )
//------------------------------------------------
{
	vec3_t		normal, end;
	FXCylinder	*fx;

	// Convert the direction of travel in to a normal;
	VectorCopy( cent->gent->s.pos.trDelta, normal );
	VectorNormalize( normal );
	VectorScale( normal, -1, normal );

	VectorMA( cent->lerpOrigin, 8, normal, end );
	FX_AddSprite( end, NULL, NULL, 64.0f, 0.0f, 0.2f, 0.2f, 0.0f, 0.0f, 2, cgs.media.blueParticleShader );

	fx = FX_AddCylinder( cent->lerpOrigin, 
						normal, BEAM_LEN, 0, 
						20 + random() * 3, 0, 
						8 - random() * 3, 0, 
						0.8f + random() * 0.2, 0.8f, 
						1, 
						cgs.media.psychicRingsShader );
	if ( fx == NULL )
		return;

	// Set the texture to wrap around the cylinder
	fx->SetFlags( FXF_WRAP | FXF_NO_LOD );
}

//------------------------------------------------
void FX_ForgePsychHitPlayer( vec3_t origin, vec3_t dir )
//------------------------------------------------
{
	//Sound
	cgi_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, cg_weapons[WP_FORGE_PSYCH].missileHitSound );
}

//------------------------------------------------
void FX_ForgePsychHitWall( vec3_t origin, vec3_t dir )
//------------------------------------------------
{
	vec3_t rgb={0.5f, 0.1f, 1.0f };

	FX_AddQuad( origin, dir, NULL, NULL, 25.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 250, cgs.media.blueParticleShader );
	FX_AddQuad( origin, dir, NULL, NULL, 25.0f, 32.0f, 1.0f, 0.0f, rgb, rgb, 0.0f, 0.0f, 0.0f, 250, cgs.media.whiteRingShader );

	//Sound
	cgi_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, cg_weapons[WP_FORGE_PSYCH].missileHitSound );	
}
