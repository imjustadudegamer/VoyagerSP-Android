// Parasite weapon effects

#include "cg_local.h"
#include "FX_Public.h"


//------------------------------------------------
void FX_ParasiteAcidThink( centity_t *cent, const struct weaponInfo_s *weapon )
//------------------------------------------------
{
	vec3_t		rgb, org;
	FXSprite	*fx;

	for ( int i = 0; i < 5; i++ )
	{
		VectorSet( org,
					cent->lerpOrigin[0] + crandom() * 2.0f,
					cent->lerpOrigin[1] + crandom() * 2.0f,
					cent->lerpOrigin[2] + crandom() * 2.0f );

		VectorSet( rgb,
					random() * 0.3f + 0.2f,
					random() * 0.4f + 0.4f,
					random() * 0.2f + 0.05f );

		fx = FX_AddSprite( org, NULL, NULL, 3.0f + ( random() * 4.0f ), -30.0f, 
					1.0f, 1.0f, 
					rgb, rgb, 
					0, 0.0f, 
					500, 
					cgs.media.spooShader );

		if ( fx == NULL )
			return;

		fx->SetRoll( 0 );
	}
}

//------------------------------------------------
void FX_ParasiteAcidHitPlayer( vec3_t origin, vec3_t dir )
//------------------------------------------------
{
	//Sound
	cgi_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, cg_weapons[WP_PARASITE].missileHitSound );

	vec3_t		rgb, org;
	FXSprite	*fx;
	vec3_t		moveDir;
	vec3_t		accel = { 0, 0, -400 };

	for ( int i = 0; i < 15; i++ )
	{
		VectorSet( org,
					origin[0] + crandom() * 2.5f,
					origin[1] + crandom() * 2.5f,
					origin[2] + crandom() * 2.5f );

		VectorScale( dir, 40.0f + random() * 120.0f, moveDir );
		
		for ( int j = 0; j < 3; j++ )
		{
			moveDir[j] += crandom() * 75.0;
		}
		moveDir[2] += 90.0f;

		VectorSet( rgb,
					random() * 0.3f + 0.2f,
					random() * 0.4f + 0.4f,
					random() * 0.3f + 0.05f );


		fx = FX_AddSprite( org, moveDir, accel, 2.0f + ( random() * 1.0f ), -2.0f, 
					1.0f, 1.0f, 
					rgb, rgb, 
					0, 0.0f, 
					400 + random() * 700, 
					cgs.media.spooShader );

		if ( fx == NULL )
			return;

		fx->SetRoll( 0 );
	}
}

//------------------------------------------------
void FX_ParasiteAcidHitWall( vec3_t origin, vec3_t dir )
//------------------------------------------------
{
	FXTrail		*particle;
	float		detail;
	vec3_t		rgb={0.2f,0.8f,0.0f};
	int			i;

	// Spawn some smoke from the acid burn
	for ( i = 0; i < 4; i ++ )
	{
		CG_Smoke( origin, dir, random() * 4.0f + 8.0f, random() * 16.0f + 2.0f, cgs.media.steamShader );
	}

	FX_AddQuad( origin, dir, NULL, NULL, 4.0, 6.0, 0.5, 0.0, rgb, rgb, 0.0, 0.0, 0.0, 550, cgs.media.waterDropShader );

	// Leave a melted spot
	CG_ImpactMark( cgs.media.bulletmarksShader, origin, dir, random()*360, 1,1,1,0.2f, qfalse, 
					random() * 3.0f + 8.0, qfalse );

	//Sound
	cgi_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, cg_weapons[WP_PARASITE].missileHitSound );

	// See if it's worth doing a splash
	detail = FX_DetailLevel( origin, 16, 400 );
	if (detail == 0)
		return;

	// splash the acid
	for ( i = 0; i < 10; i++ )
	{	
		particle = FX_AddTrail( origin, NULL, NULL, 3.0f, -1.0f, 2.0, -1.0,
								0.3f, 0.1f, rgb, rgb, 0.4f, 500.0f, cgs.media.waterDropShader, rand() & FXF_BOUNCE );

		if ( particle != NULL )
		{
			FXE_Spray( dir, 80, 100, 0.95f, 512, (FXPrimitive *) particle );
		}
	}

	// Make a real splash

	vec3_t		org;
	FXSprite	*fx;
	vec3_t		moveDir;
	vec3_t		accel = { 0, 0, -400 };

	for ( i = 0; i < 6; i++ )
	{
		VectorSet( org,
					origin[0] + crandom() * 2.5f,
					origin[1] + crandom() * 2.5f,
					origin[2] + crandom() * 2.5f );
	
		for ( int j = 0; j < 3; j++ )
		{
			moveDir[j] = crandom() * 55.0;
		}
		moveDir[2] += 120.0f;

		VectorSet( rgb,
					random() * 0.3f + 0.2f,
					random() * 0.4f + 0.5f,
					random() * 0.2f + 0.05f );


		fx = FX_AddSprite( org, moveDir, accel, 2.0f + ( random() * 1.0f ), -2.0f, 
					1.0f, 1.0f, 
					rgb, rgb, 
					0, 0.0f, 
					400 + random() * 700, 
					cgs.media.spooShader );

		if ( fx == NULL )
			return;

		fx->SetRoll( 0 );
	}

}
