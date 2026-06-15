//Vohrsoth weapon effects

#include "cg_local.h"
#include "FX_Public.h"

/*
-------------------------
FX_ChestBeam
-------------------------
*/

void FX_ChestBeam( vec3_t origin, float height )
{
	vec3_t	normal = { 0, 0, 1 };

	FX_AddCylinder( origin, normal, 64, 0, 64, 0, 32, 0, 1.0f, 1.0f, 100, cgs.media.vohrConeShader );
}

/*
-------------------------
FX_ChestBeamSpawner
-------------------------
*/

void FX_ChestBeamSpawner( vec3_t origin, float height )
{
}

/*
-------------------------
FX_GroundTendril
-------------------------
*/

void FX_GroundTendril( vec3_t position, vec3_t normal, vec3_t dir, vec3_t user )
{
	vec3_t	v, sp, sRGB, eRGB, start, end;
	float	detail, len, salpha, ealpha;
	
	vec4_t	startRGBA	= { 255, 255, 0, 0.25 };
	vec4_t	endRGBA		= { 255, 128, 0, 0 };

	VectorCopy( position, start );
	VectorMA( start, 64, normal, end );

	detail = FX_DetailLevel( start, 16, 1200 );
	if ( detail == 0 )
		return;

	salpha = Vector4to3( startRGBA, sRGB );
	ealpha = Vector4to3( endRGBA, eRGB );

	// Orient the plasma
	VectorSubtract( end, start, v );
	len = VectorNormalize( v );
	VectorMA( start, 0.5f, v, sp );

	// Stash a quad at the base to make the effect look a bit more solid
	FX_AddQuad( sp, v, NULL, NULL, len * 0.36f, 0.0f, salpha, salpha, sRGB, sRGB, 0.0f, 45.0f, 0.0f, 200, cgs.media.prifleImpactShader );
	
	// Add a subtle, random flutter to the cone direction
	v[0] += crandom() * 0.04;
	v[1] += crandom() * 0.04;
	v[2] += crandom() * 0.04;
	
	// Wanted the effect to be scalable based on the length of the jet. 
	FX_AddCylinder( start, v, len * 0.05, len * 2.0f, len * 0.25f, len * 0.5f, len * 0.40f, len * 0.9f, 
					salpha, ealpha, sRGB, eRGB, 200, cgs.media.plasmaShader, 0.3f );
	FX_AddCylinder( start, v, len * 0.05, len * 4.0f, len * 0.25f, len * 0.5f, len * 0.28f, len * 0.9f, 
					salpha, ealpha, sRGB, eRGB, 200, cgs.media.plasmaShader, 0.2f );
}

/*
-------------------------
FX_GroundTendrilSpawner
-------------------------
*/

void FX_GroundTendrilSpawner( vec3_t start, vec3_t end )
{
	vec3_t	normal;

	VectorSubtract( end, start, normal );
	VectorNormalize( normal );

	FX_AddSpawner( start, normal, NULL, NULL, 50, 0.0f, 3500, (void *) FX_GroundTendril, NULL, 1000 );

	VectorScale( normal, -1, normal );
	FX_AddSpawner( end, normal, NULL, NULL, 50, 0.0f, 3500, (void *) FX_GroundTendril, NULL, 1000 );
}

/*
-------------------------
FX_Vohrsoth_CreateGroundWarning
-------------------------
*/

void FX_Vohrsoth_CreateGroundWarning( vec3_t origin )
{
	vec3_t	normal = { 0, 0, 1 };

	FX_AddSpawner( origin, normal, NULL, NULL, 50, 0.0f, 1000, (void *) FX_GroundTendril, NULL, 1000 );
}