// Teleporter type effects

#include "cg_local.h"
#include "FX_Public.h"
#include "../game/anims.h"

/*
-------------------------
TransporterParticle
-------------------------
*/

void TransporterParticle( vec3_t start, vec3_t end, vec3_t dir, vec3_t user )
{
	vec3_t		org, velocity	=	{ 0, 0, 26 };
	vec3_t		accel = { 0, 0, -11 };
	float		scale, dscale;
	qhandle_t	shader;

	VectorCopy( start, org );
	org[2] += 8;

	shader = ( dir[0] == 0 ) ? cgs.media.trans1Shader : cgs.media.trans2Shader;
	scale  = ( dir[0] == 0 ) ? 3.0 : 5.0;
	dscale  = ( dir[0] == 0 ) ? 6.0 : 26.0;

	dir[0]++;

	FX_AddSprite(	org,
					velocity,
					accel,
					scale,
					dscale,
					1.0f,
					0.1f,
					0,
					0.0f,
					2400.0f,
					shader );
	
	VectorScale( velocity, -1, velocity );
	VectorScale( accel, -1, accel );

	FX_AddSprite(	org,
					velocity,
					accel,
					scale,
					dscale,
					1.0f,
					0.1f,
					0,
					0.0f,
					2400.0f,
					shader );
}


/*
-------------------------
ReplicatorParticle
-------------------------
*/

void ReplicatorParticle( vec3_t start, vec3_t end, vec3_t dir, vec3_t user )
{
	vec3_t		org, velocity	=	{ 0, 0, 4 };
	vec3_t		accel = { 0, 0, -4 };
	float		scale, dscale;
	qhandle_t	shader;

	VectorCopy( start, org );
	org[2] += 4;

	shader = ( dir[0] == 0 ) ? cgs.media.trans1Shader : cgs.media.trans2Shader;
	scale  = ( dir[0] == 0 ) ? 3.0 : 5.0;
	dscale  = ( dir[0] == 0 ) ? 6.0 : 26.0;

	dir[0]++;

	FX_AddSprite(	org,
					velocity,
					accel,
					scale,
					dscale,
					1.0f,
					0.1f,
					0,
					0.0f,
					1000.0f,
					shader );
	
	VectorScale( velocity, -1, velocity );
	VectorScale( accel, -1, accel );

	FX_AddSprite(	org,
					velocity,
					accel,
					scale,
					dscale,
					1.0f,
					0.1f,
					0,
					0.0f,
					1000.0f,
					shader );
}

/*
-------------------------
FX_Transporter
-------------------------
*/

void FX_Transporter( vec3_t org )//,qboolean replicator
{
	//FIXME: maybe find head tag and center of feet and make balls go between them (for horizontal people)
	FX_AddSpawner( org, NULL, NULL, NULL, 500, 0, 650, (void *) TransporterParticle );
	cgi_S_StartSound( org, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.transporterSound );
}


/*
-------------------------
FX_PlayerTeleportIn
-------------------------
*/

void FX_PlayerTeleportIn( centity_t *cent, vec3_t position )
{
	FX_Transporter( position );//FIXME: let it know if it's a replicator
	if ( cent->gent->owner )
	{
		cent->gent->owner->fx_time = cg.time;
		if ( cent->gent->owner->client )
		{
			cent->gent->s.powerups |= ( 1 << PW_INVIS );
			cent->gent->owner->client->ps.powerups[PW_INVIS] = cg.time + 4000;
		}
		else
		{
			// uh oh.  We can't rely on using the powerups...so hack something in instead.
			cent->gent->owner->s.eFlags = EF_BEAM_IN;
			cent->gent->owner->delay = cg.time + 4000;
		}
	}
}


/*
-------------------------
FX_PlayerTeleportOut
-------------------------
*/

void FX_PlayerTeleportOut( centity_t *cent, vec3_t position )
{
	vec3_t org;

	VectorCopy( position, org );

	// Stupid hack for chang beaming out in borg2...if this animation changes, all will fall apart...sigh.
	if ( cent->gent->owner )
	{
		if ( (cent->gent->owner->s.legsAnim & ~ANIM_TOGGLEBIT) == BOTH_INJURED6 )
		{
			org[2] -= 13;
		}
	}

	FX_Transporter( org );
	if ( cent->gent->owner )
	{
		cent->gent->owner->fx_time = cg.time;
		if ( cent->gent->owner->client )
		{
			cent->gent->s.powerups |= ( 1 << PW_QUAD );
			cent->gent->owner->client->ps.powerups[PW_QUAD] = cg.time + 4000;
		}
		else
		{
			// uh oh.  We can't rely on using the powerups...so hack something in instead.
			cent->gent->owner->s.eFlags = EF_BEAM_OUT;
			cent->gent->owner->delay = cg.time + 4000;
		}
	}
}


/*
-------------------------
FX_Replicator
-------------------------
*/

void FX_Replicator( centity_t *cent, vec3_t position )
{
	//FIXME: smaller effect?
	FX_AddSpawner( position, NULL, NULL, NULL, 500, 0, 650, (void *) ReplicatorParticle );
	cgi_S_StartSound( position, ENTITYNUM_WORLD, CHAN_AUTO, cgi_S_RegisterSound( "sound/movers/switches/replicator.wav" ) );
	if ( cent->gent->owner )
	{
		cent->gent->owner->fx_time = cg.time;
		if ( cent->gent->owner->client )
		{
			cent->gent->s.powerups |= ( 1 << PW_INVIS );
			cent->gent->owner->client->ps.powerups[PW_INVIS] = cg.time + 4000;
		}
		else
		{
			// uh oh.  We can't rely on using the powerups...so hack something in instead.
			cent->gent->owner->s.eFlags = EF_BEAM_IN;
			cent->gent->owner->delay = cg.time + 4000;
		}
	}
}

/*
-------------------------
FX_ForgeTeleportOut

Hmmm...needs some work..
-------------------------
*/
void ForgeThink( vec3_t start, vec3_t end, vec3_t vel, vec3_t user )
{
	FX_AddElectricity( start, end, 1.0f, 3.5f, 0.0f, 1.0f, 0.0f, random() * 100 + 100, cgs.media.pjBoltShader, FXF_WRAP, 0.7f );
	FX_AddSprite( start, NULL, NULL, 5.0f, 0.0f, 0.5f, 0.2f, 0.0f, 0, 100, cgs.media.orangeParticleShader );
	FX_AddSprite( end, NULL, NULL, 5.0f, 0.0f, 0.5f, 0.2f, 0.0f, 0, 100, cgs.media.orangeParticleShader );

	if ( rand() & 3 )
	{
		vec3_t	other;

		for ( int i = 0; i < 3; i++ )
		{
			other[i] = end[i] + crandom() * 24.0f;
		}

		FX_AddElectricity( end, other, 1.0f, 2.0f, 0.0f, 1.0f, 0.0f, random() * 200 + 100, cgs.media.pjBoltShader, FXF_WRAP | FXF_TAPER, 0.7f );
	}
}

void FX_ForgeTeleportOut( vec3_t position )
{
	int		i;
	vec3_t	down, angles, dir, start, end, mid;
	trace_t	trace;

	VectorSet( down, 0, 0, -1 );
	VectorMA( position, 4.0f, down, start );
	VectorMA( position, 96, down, end );

	CG_Trace( &trace, start, NULL, NULL, end, ENTITYNUM_WORLD, CONTENTS_SOLID );

	vectoangles( down, angles );

	for ( i = 0; i < 18; i++ )
	{
		angles[2] = i * 20;

		AngleVectors( angles, NULL, dir, NULL );
		
		VectorMA( trace.endpos, 25 + random() * 24, dir, end );

		VectorAdd( start, end, mid );
		VectorScale( mid, 0.5f, mid );
		VectorMA( mid, random() * -8.0f, down, mid );

		FX_AddSpawner( start, mid, NULL, NULL, 120, 60, 400, 0, ForgeThink, NULL, 1024 );
		FX_AddSpawner( mid, end, NULL, NULL, 120, 60, 400, 0, ForgeThink, NULL, 1024 );
	}
}

/*
-------------------------
FX_ForgeBeamIn
-------------------------
*/

void FX_ForgeBeamIn( vec3_t origin )
{
	vec3_t up = {0,0,1}, org, start, end;

	VectorCopy( origin, org );
	org[2] += 16;

	VectorMA( org, 60, up, start );
	VectorMA( org, -50, up, end );

	FX_AddCylinder( end, up, 100, 0, 0.01f, 100, 0.01f, 132, 1.0, 0.0, 1000, cgs.media.dkorangeParticleShader );

	cgi_S_StartSound( org, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.forgeBeaminSound );
}

/*
-------------------------
FX_8472Teleport
-------------------------
*/

void FX_8472Teleport( vec3_t org, int parm )
{
	int		i, t;
	vec3_t	dir = {0,0,1}, start, end, vel, color;

	if ( parm )
	{
		// Beaming out
		for ( i = 0; i < 32; i++ )
		{
			// Pick a random direction..
			VectorSet( dir, crandom(), crandom(), crandom() );
			VectorNormalize( dir );
			
			start[0] = org[0] + dir[0] * 26;
			start[1] = org[1] + dir[1] * 26;
			start[2] = org[2] + fabs(dir[2] * 10) - 18;

			// Now build the velocity vector
			vel[0] = dir[0] * 8;
			vel[1] = dir[1] * 8;
			vel[2] = dir[2] * 6;

			FX_AddSprite( start, vel, NULL, 
						24 + random() * 8, -18, 
						1.0, 0.0, 
						crandom()*180,0, 
						1000, cgs.media.portalFlareShader, FXF_NON_LINEAR_FADE );
		}

		cgi_S_StartSound( org, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.speciesBeamoutSound );
	}
	else
	{
		// Beaming in
		VectorMA( org, 65, dir, start );
		VectorMA( org, -40, dir, end );

		FX_AddLine( start, end, 1.0f, 0.5f, 48.0f, 0.4f, 0.1f, 1500, cgs.media.speciesPortalShader, FXF_NON_LINEAR_FADE );

		for ( i = -1; i <= 1; i++ )
		{ 
			VectorCopy( org, start );
			start[2] += 10.0f + i * 16.0f;

			for ( t = -1; t <= 1; t++ ) 
			{
				// create a 3 x 3 grid of starting points for the effect
				VectorClear( vel );
				VectorMA( vel, t * 16, cg.refdef.viewaxis[1], vel );
				
				FX_AddSprite( start, vel, NULL, 56.0f, -10.0f, 0.3f, 0.0f, -90.0f, 0, 1800, cgs.media.portalFlareShader, FXF_NON_LINEAR_FADE );
			}
		}

		cgi_S_StartSound( org, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.speciesBeaminSound );
	}

	VectorSet( color, 1.0f, 1.0f, 0.7f );
	CG_AddTempLight( org, 200, color, 1000 );
}
