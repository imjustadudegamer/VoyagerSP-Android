//Stasis weapon effects

#include "cg_local.h"
#include "FX_Public.h"

void FX_StasisDischarge( vec3_t origin, vec3_t normal, int count, float dist_out, float dist_side );

#define FX_STASIS_ALT_RIGHT_OFS	0.10
#define FX_STASIS_ALT_UP_OFS	0.02
#define FX_MAXRANGE_STASIS		8192

/*
-------------------------
FX_StasisShot

Alt-fire, beam that shrinks to its impact point
-------------------------
*/

void FX_SmallStasisBeam(centity_t *cent, vec3_t start, vec3_t dir)
{
	vec3_t end, org, vel = { 0,0,-4};
	trace_t tr;
	float r;
	int i, ct, t;

	VectorMA(start, FX_MAXRANGE_STASIS, dir, end);
	CG_Trace(&tr, start, NULL, NULL, end, cent->currentState.number, MASK_SHOT);

	// Beam
	FX_AddLine( start, tr.endpos, 1.0f, 3.0f, 4.0f, 0.8f, 0.0f, 400.0f, cgs.media.stasisAltShader );

	// Do a quick LOD for number of decay particles
	ct = tr.fraction * ( FX_MAXRANGE_STASIS * 0.02 );
	if ( ct < 12 )
		ct = 12;
	else if ( ct > 24 )
		ct = 24;

	for ( i = 0; i < ct; i++ )
	{
		r = random() * tr.fraction * ( FX_MAXRANGE_STASIS * 0.5 );
		VectorMA( start, r, dir, org );

		for ( t = 0; t < 3; t++ )
			org[t] += crandom();

		if ( rand() & 1 )
			FX_AddSprite( org, vel, NULL, random() + 1.5, -3, 1.0, 1.0, 0.0, 0.0, 500, cgs.media.blueParticleShader );
		else
			FX_AddSprite( org, vel, NULL, random() + 1.5, -3, 1.0, 1.0, 0.0, 0.0, 500, cgs.media.purpleParticleShader );	
	}

	// Impact graphic if needed.
	if ( cg_entities[tr.entityNum].currentState.eType == ET_PLAYER )
	{	// Hit an entity.
		// Expanding rings
		FX_AddSprite( tr.endpos, NULL, NULL, 1, 60, 0.8f, 0.2f, random() * 360, 0, 400, cgs.media.stasisRingShader );
		// Impact effect
		FX_AddSprite( tr.endpos, NULL, NULL, 5, 18, 1.0, 0.0, random() * 360, 0, 420, cgs.media.ltblueParticleShader );
	}
	else if ( !(tr.surfaceFlags & SURF_NOIMPACT) )
	{
		// Move me away from the wall a bit so that I don't z-buffer into it
		VectorMA( tr.endpos, 1.5, tr.plane.normal, end);

		// Expanding rings
		FX_AddQuad( end, tr.plane.normal, NULL, NULL, 1, 18, 0.8f, 0.2f, random() * 360, 0, 0, 400, cgs.media.stasisRingShader );
		FX_AddQuad( end, tr.plane.normal, NULL, NULL, 1, 45, 0.8f, 0.2f, random() * 360, 0, 0, 300, cgs.media.stasisRingShader );
		// Impact effect
		FX_AddQuad( end, tr.plane.normal, NULL, NULL, 5, 25, 1.0, 0.0, random() * 360, 0, 0, 500, cgs.media.blueParticleShader );
		FX_AddQuad( end, tr.plane.normal, NULL, NULL, 4, 18, 1.0, 0.0, random() * 360, 0, 0, 420, cgs.media.ltblueParticleShader );

		CG_ImpactMark( cgs.media.scavMarkShader, end, tr.plane.normal, random()*360, 1,1,1,0.6f, qfalse, 
					6 + random() * 2, qfalse );
	}

	FX_AddSprite( tr.endpos, NULL, NULL, Q_flrand(40,60), -50, 1.0, 0.0, random() * 360, 0, 500, cgs.media.blueParticleShader );

	// Pass the end position back to the calling function (yes, I know).
	VectorCopy(tr.endpos, dir);
}

void FX_StasisShot( centity_t *cent, vec3_t end, vec3_t start )
{
	vec3_t	fwd, newdir, org, vel = { 0,0,-4};
	int		i, t, ct;
	float	len, r;
	vec3_t	up={0, 0, 1}, right;
	int		bolt1, bolt2;
	vec3_t	bolt1vec, bolt2vec;

	// Choose which bolt will have the electricity accent.
	bolt1 = Q_irand(0,2);
	bolt2 = Q_irand(0,4);

	VectorSubtract( end, start, fwd );
	len = VectorNormalize( fwd );

	// Beam
	FX_AddLine( end, start, 1.0f, 4.0f, 6.0f, 0.8f, 0.0f, 500.0f, cgs.media.stasisAltShader );

	// Do a quick LOD for number of decay particles
	ct = len * 0.03;
	if ( ct < 16 )
		ct = 16;
	else if ( ct > 32 )
		ct = 32;

	for ( i = 0; i < ct; i++ )
	{
		r = random() * len * 0.5;
		VectorMA( start, r, fwd, org );

		for ( t = 0; t < 3; t++ )
			org[t] += crandom();

		if ( rand() & 1 )
			FX_AddSprite( org, vel, NULL, random() + 2, -4, 1.0, 1.0, 0.0, 0.0, 600, cgs.media.blueParticleShader);
		else
			FX_AddSprite( org, vel, NULL, random() + 2, -4, 1.0, 1.0, 0.0, 0.0, 600, cgs.media.purpleParticleShader);	
	}

	if (bolt1==0)
	{
		VectorCopy(end, bolt1vec);
	}
	else if (bolt2==0)
	{
		VectorCopy(end, bolt2vec);
	}

	CrossProduct(fwd, up, right);
	CrossProduct(right, fwd, up);	// Change the "fake up" (0,0,1) to a "real up" (perpendicular to the forward vector).

	// Fire a shot up and to the right.
	VectorMA(fwd, FX_STASIS_ALT_RIGHT_OFS, right, newdir);
	VectorMA(newdir, FX_STASIS_ALT_UP_OFS, up, newdir);
	FX_SmallStasisBeam(cent, start, newdir);

	if (bolt1==1)
	{
		VectorCopy(newdir, bolt1vec);
	}
	else if (bolt2==1)
	{
		VectorCopy(newdir, bolt2vec);
	}

	// Fire a shot up and to the left.
	VectorMA(fwd, -FX_STASIS_ALT_RIGHT_OFS, right, newdir);
	VectorMA(newdir, FX_STASIS_ALT_UP_OFS, up, newdir);
	FX_SmallStasisBeam(cent, start, newdir);

	if (bolt1==2)
	{
		VectorCopy(newdir, bolt1vec);
	}
	else if (bolt2==2)
	{
		VectorCopy(newdir, bolt2vec);
	}

	// Fire a shot a bit down and to the right.
	VectorMA(fwd, 2.0*FX_STASIS_ALT_RIGHT_OFS, right, newdir);
	VectorMA(newdir, -0.5*FX_STASIS_ALT_UP_OFS, up, newdir);
	FX_SmallStasisBeam(cent, start, newdir);

	if (bolt1==3)
	{
		VectorCopy(newdir, bolt1vec);
	}
	else if (bolt2==3)
	{
		VectorCopy(newdir, bolt2vec);
	}

	// Fire a shot up and to the left.
	VectorMA(fwd, -2.0*FX_STASIS_ALT_RIGHT_OFS, right, newdir);
	VectorMA(newdir, -0.5*FX_STASIS_ALT_UP_OFS, up, newdir);
	FX_SmallStasisBeam(cent, start, newdir);

	if (bolt1==4)
	{
		VectorCopy(newdir, bolt1vec);
	}
	else if (bolt2==4)
	{
		VectorCopy(newdir, bolt2vec);
	}
		
	// Put a big gigant-mo sprite at the muzzle end so people can't see the crappy edges of the line
	FX_AddSprite( start, NULL, NULL, random()*3 + 15, -20, 1.0, 0.5, 0.0, 0.0, 600, cgs.media.blueParticleShader);

	// Do an electrical arc to one of the impact points.
//	FX_AddElectricity( start, bolt1vec, 0.2f, 15.0, -15.0, 1.0, 0.5, 100, cgs.media.dnBoltShader, 0.1 );

//	if (bolt1 != bolt2)
//	{
		// ALSO do an electrical arc to another point.
//		FX_AddElectricity( bolt1vec, bolt2vec, 0.2f, 15.0, -15.0, 1.0, 0.5, Q_flrand(100,200), cgs.media.dnBoltShader, 0.5 );
//	}
}

/*
-------------------------
FX_StasisShotImpact

Alt-fire, impact effect
-------------------------
*/
void FX_StasisShotImpact( vec3_t end, vec3_t dir )
{
	vec3_t	org;

	// Move me away from the wall a bit so that I don't z-buffer into it
	VectorMA( end, 0.5, dir, org );

	// Expanding rings
	FX_AddQuad( org, dir, NULL, NULL, 1, 80, 0.8f, 0.2f, random() * 360, 360, 0, 400, cgs.media.stasisRingShader );

	// Impact effect
	FX_AddQuad( org, dir, NULL, NULL, 7, 35, 1.0, 0.0, random() * 360, 0, 0, 500, cgs.media.blueParticleShader );
	FX_AddQuad( org, dir, NULL, NULL, 5, 25, 1.0, 0.0, random() * 360, 0, 0, 420, cgs.media.ltblueParticleShader );
}

/*
-------------------------
FX_StasisShotMiss

Alt-fire, miss effect
-------------------------
*/
void FX_StasisShotMiss( vec3_t end, vec3_t dir )
{
	vec3_t	org;

	// Move me away from the wall a bit so that I don't z-buffer into it
	VectorMA( end, 0.5, dir, org );

	// Expanding rings
	FX_AddQuad( org, dir, NULL, NULL, 1, 24, 0.8f, 0.2f, random() * 360, 360, 0, 400, cgs.media.stasisRingShader );
	FX_AddQuad( org, dir, NULL, NULL, 1, 60, 0.8f, 0.2f, random() * 360, -360, 0, 300, cgs.media.stasisRingShader );
	// Impact effect
	FX_AddQuad( org, dir, NULL, NULL, 7, 35, 1.0, 0.0, random() * 360, 0, 0, 500, cgs.media.blueParticleShader );
	FX_AddQuad( org, dir, NULL, NULL, 5, 25, 1.0, 0.0, random() * 360, 0, 0, 420, cgs.media.ltblueParticleShader );

	CG_ImpactMark( cgs.media.scavMarkShader, org, dir, random()*360, 1,1,1,0.6f, qfalse, 
				8 + random() * 2, qfalse );

//	FX_AddSprite( end, NULL, qfalse, flrandom(40,60), -50, 1.0, 0.0, random() * 360, 0, 500, cgs.media.blueParticleShader );
}


/*
-------------------------
FX_StasisProjectileThink

Main fire, with crazy bits swirling around main projectile
-------------------------
*/
void FX_StasisProjectileThink( centity_t *cent, const struct weaponInfo_s *weapon )
{
	int			size = 0;

	if ( cent->gent == NULL )
		return;

	size = cent->gent->count;

	if ( size < 3 )
		FX_AddSprite( cent->lerpOrigin, NULL, NULL, size * 18.0f + ( random() * size * 4.0f ), 0.0f, 1.0f, 0.0f, 0, 0.0f, 1, cgs.media.blueParticleShader );
	else
	{
		// Only do this extra crap if you're the big cheese
		vec3_t  forward, right, up;
		float	radius, temp;
		vec3_t	org;

		// convert direction of travel into normalized forward vector
		if ( VectorNormalize2( cent->currentState.pos.trDelta, forward ) == 0 ) {
			forward[2] = 1;
		}
		MakeNormalVectors( forward, right, up );

		// Main projectile
		FX_AddSprite( cent->lerpOrigin, NULL, NULL, ( size * size ) * 4.5f + ( random() * size * 4.0f ), 0, 1.0, 1.0, 0.0, 0.0, 1, cgs.media.blueParticleShader );

		// Crazy polar coordinate plotting stuff--for particle swarm
		// FIXME:  SIN and COS hell!
		radius = 30 * sin( cg.time * 0.002 * 5 );
		temp = radius * cos( cg.time * 0.002 );
		VectorMA( cent->lerpOrigin, temp, right, org );
		temp = radius * sin( cg.time * 0.002 );
		VectorMA( org, temp, up, org );

		// Main bit
		FX_AddSprite( org, NULL, NULL, random() * 8 + 4, 0, 1.0, 1.0, 0.0, 0.0, 1, cgs.media.purpleParticleShader );
		// Glowy bit
		FX_AddSprite( org, NULL, NULL, random() * 8 + 24, 0, 0.3f, 0.3f, 0.0, 0.0, 1, cgs.media.purpleParticleShader );
		// Tail bit
		FX_AddLine( org, cent->gent->pos1, 1.0, 2.0, -1.0, 0.5, 0.0, 300, cgs.media.altIMOD2Shader );
		VectorCopy( org, cent->gent->pos1 );

		radius = 30 * sin( cg.time * 0.002 * 3 );
		temp = radius * cos( cg.time * 0.002 + 3.141 );
		VectorMA( cent->lerpOrigin, temp, right, org );
		temp = radius * sin( cg.time * 0.002 + 3.141 );
		VectorMA( org, temp, up, org );

		// Main bit
		FX_AddSprite( org, NULL, NULL, random() * 8 + 4, 0, 1.0, 1.0, 0.0, 0.0, 1, cgs.media.purpleParticleShader );
		// Glowy bit
		FX_AddSprite( org, NULL, NULL, random() * 8 + 24, 0, 0.3f, 0.3f, 0.0, 0.0, 1, cgs.media.purpleParticleShader );
		// Tail bit
		FX_AddLine( org, cent->gent->pos2, 1.0, 2.0, -1.0, 0.5, 0.0, 300, cgs.media.altIMOD2Shader );
		VectorCopy( org, cent->gent->pos2 );

		radius = 30 * sin( cg.time * 0.002 * 3.5 );
		temp = radius * cos( cg.time * 0.002 + 3.141 * 0.5);
		VectorMA( cent->lerpOrigin, temp, right, org );
		temp = radius * sin( cg.time * 0.002 + 3.141 * 0.5 );
		VectorMA( org, temp, up, org );

		// Main bit
		FX_AddSprite( org, NULL, NULL, random() * 8 + 4, 0, 1.0, 1.0, 0.0, 0.0, 1, cgs.media.purpleParticleShader );
		// Glowy bit
		FX_AddSprite( org, NULL, NULL, random() * 8 + 24, 0, 0.3f, 0.3f, 0.0, 0.0, 1, cgs.media.purpleParticleShader );

		radius = 30 * sin( cg.time * 0.002 * 4.5 );
		temp = radius * cos( cg.time * 0.002 + 3.141 * 1.5);
		VectorMA( cent->lerpOrigin, temp, right, org );
		temp = radius * sin( cg.time * 0.002 + 3.141 * 1.5 );
		VectorMA( org, temp, up, org );

		// Main bit
		FX_AddSprite( org, NULL, NULL, random() * 8 + 4, 0, 1.0, 1.0, 0.0, 0.0, 1, cgs.media.purpleParticleShader );
		// Glowy bit
		FX_AddSprite( org, NULL, NULL, random() * 8 + 24, 0, 0.3f, 0.3f, 0.0, 0.0, 1, cgs.media.purpleParticleShader );
	}

	return;
}

/*
-------------------------
FX_StasisWeaponHitWall

Main fire impact
-------------------------
*/

#define NUM_DISCHARGES		2
#define	DISCHARGE_DIST		8
#define	DISCHARGE_SIDE_DIST	24

void FX_StasisWeaponHitWall( vec3_t origin, vec3_t dir, int size )
{
	// Generate "crawling" electricity
	FX_StasisDischarge( origin, dir, NUM_DISCHARGES, DISCHARGE_DIST, DISCHARGE_SIDE_DIST );

	// Set an oriented residual glow effect
	FX_AddQuad( origin, dir, NULL, NULL, size * size * 12.0f, -60.0f, 
				1.0f, 1.0f, 0, 0, 0, 300, cgs.media.blueParticleShader );

	CG_ImpactMark( cgs.media.scavMarkShader, origin, dir, random()*360, 1,1,1,0.6f, qfalse, 
					size * size * 4 + 1, qfalse );

	// Only play the impact sound and throw off the purple particles when it's the main projectile
	if ( size < 3 )
		return;

	vec3_t	vel, accel;
	int		i, t;

	for ( i = 0; i < 4; i++ )
	{
		for ( t = 0; t < 3; t++ )
			vel[t] = ( dir[t] + crandom() * 0.9 ) * ( random() * 100 + 250 );

		VectorScale( vel, -2.2, accel );
		FX_AddSprite( origin, vel, accel, random() * 8 + 8, 0, 1.0, 0.0, 0.0, 0.0, 200, cgs.media.purpleParticleShader );

	}
	cgi_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, cg_weapons[WP_STASIS].missileHitSound );
}

/*
-------------------------
FX_StasisWeaponHitPlayer

Main fire impact
-------------------------
*/
void FX_StasisWeaponHitPlayer( vec3_t origin, vec3_t dir, int size )
{
	int			i;
	vec3_t		right, up, pos, accel;

	// FIXME:  This is mostly just a stand-in, though it may be ok
	MakeNormalVectors( dir, right, up );

	// Send some particles scurrying along the impact plane
	for ( i = 0; i < 6; i++ )
	{
		VectorClear( pos );
		VectorMA( pos, crandom(), right, pos );
		VectorMA( pos, crandom(), up, pos );

		VectorNormalize( pos );
		VectorScale( pos, random() * size * 48 + 32, pos );
		VectorScale( pos, -2, accel );

		FX_AddSprite( origin, pos, accel, size * size * random() * 0.5f, -( size * random() * 2 ), 
					1.0f, 0.0f, 0, 0, 300, cgs.media.blueParticleShader );
	}
}

/*
-------------------------
FX_StasisBoltThink

Draw the bolts
-------------------------
*/ 

void FX_StasisBoltThink( vec3_t start, vec3_t end, vec3_t dir, vec3_t user ) 
{
	vec3_t	mid;

	VectorSubtract( end, start, mid );
	VectorScale( mid, 0.1f + (random() * 0.8), mid );
	VectorAdd( start, mid, mid );
	VectorMA(mid, 3.0f + (random() * 10.0f), dir, mid );

	FX_AddElectricity( mid, start, 0.5, 0.75 + random() * 0.75, 0.0, 1.0, 0.5, 75, cgs.media.bolt2Shader, 9 );
	FX_AddElectricity( mid, end, 0.5, 0.75 + random() * 0.75, 1.0, 1.0, 0.5, 75, cgs.media.bolt2Shader, FXF_TAPER );
}

/*
-------------------------
FX_OrientedBolt

Creates new bolts for a while
-------------------------
*/

void FX_OrientedBolt( vec3_t start, vec3_t end, vec3_t dir )
{
	FX_AddSpawner( start, end, dir, NULL, 50, random() * 75, 300.0f + random() * 300, (void *) FX_StasisBoltThink, NULL, 680);
}

/*
-------------------------
FX_StasisDischarge

Fun "crawling" electricity ( credit goes to Josh for this one )
-------------------------
*/

void FX_StasisDischarge( vec3_t origin, vec3_t normal, int count, float dist_out, float dist_side )
{
	trace_t	trace;
	vec3_t	org, dir, dest;
	vec3_t	vr;
	int		i;
	int		discharge = dist_side;

	vectoangles( normal, dir );
	dir[ROLL] += random() * 360;

	for (i = 0;	i < count; i++)
	{
		//Move out a set distance
		VectorMA( origin, dist_out, normal, org );
		
		//Even out the hits
		dir[ROLL] += (360 / count) + (rand() & 31);
		AngleVectors( dir, NULL, vr, NULL );

		//Move to the side in a random direction
		discharge += (int)( crandom() * 8.0f );
		VectorMA( org, discharge, vr, org );

		//Trace back to find a surface
		VectorMA( org, -dist_out * 3, normal, dest );

		cgi_CM_BoxTrace( &trace, org, dest, NULL, NULL, 0, MASK_SHOT );
		
		//No surface found, start over
		if (trace.fraction == 1) 
			continue;

		//Connect the two points with bolts
		FX_OrientedBolt( origin, trace.endpos, normal );
	}
}

/*
-------------------------
FX_StasisAmbientThings

Ambient sparks of light that swarm in a given area
-------------------------
*/

// This is a "random" table that has been tweaked to provide a nice distribution of ambient "thingies", the last field is a perc of max radius
// There are 10 here now....this had better correspond with the count the designers use or the results could be unpredictable
static int ang_offset[][3] = {	-138,	-208,	100,
								-292,	-284,	30,
								197,	67,		70,
								257,	-115,	40,
								177,	286,	80,
								19,		239,	20,
								-337,	-347,	60,
								-9,		-195,	10,
								-186,	-52,	50,
								-86,	-51,	90};

void FX_StasisAmbientThings( centity_t *cent )
{
	int		i, ct = 0;
	float	radius = 0, variance = 0;
	vec3_t	origin, angvect, temp;

	if ( cent->gent == NULL )
		return;

	ct			= cent->gent->count;
	variance	= cent->gent->random * 0.01;

	// Update the angle of the particle (position on the ellipsoid).
	VectorMA( cent->gent->pos1, cg.frametime * 0.001, cent->gent->pos2, cent->gent->pos1 );

	for ( i = 0; i < ct; i++ )
	{
		// Calc allowable variance in radius.
		radius = cent->gent->radius * ( ( 1 - variance) + (ang_offset[i][2] * 0.01 * variance) );

		// Refresh this every time.
		VectorCopy( cent->gent->pos1, temp );

		// This uses the angular offset table to create the swarm
		temp[0] += ang_offset[i][0];
		temp[1] += ang_offset[i][1];

		// Update the actual position of the particle.
		AngleVectors( temp, angvect, NULL, NULL );

		origin[0] = (8 * cos(cg.time * 0.004 + i ) + radius) * angvect[0] + cent->lerpOrigin[0];
		origin[1] = (8 * sin(cg.time * 0.005 + i ) + radius) * angvect[1] + cent->lerpOrigin[1];
		origin[2] = (8 * sin(cg.time * 0.006 + i ) + radius) * angvect[2] + cent->lerpOrigin[2];

		// Main core followed by much larger, fainter aura....sort of looks irridescent or something
		FX_AddSprite( origin, NULL, NULL, random() * 2.0f + 2.5f, 2.0f, 1.0f, 0.0f, 0.0f, 0.0f, 100.0f, cgs.media.ltblueParticleShader );
		FX_AddSprite( origin, NULL, NULL, random() * 6.0f + 6.0f, 2.0f, 0.3f, 0.0f, 0.0f, 0.0f, 100.0f, cgs.media.purpleParticleShader );

		if ( cent->gent->s.eFlags & EF_EYEBEAM && cent->gent->enemy && !Q_irand(0, 5) )
		{//Beam effect to entity it's fixing
			FX_AddElectricity( origin, cent->gent->enemy->currentOrigin, 1.0f, 
								0.75f + ( random() * 0.75f ), 0.0f, 
								Q_flrand(0.7f, 1.0f), 0.0f, 50.0f, 
								cgs.media.stasisBoltShader, FXF_TAPER );
		}
	}
}

/*
-------------------------
FX_StasisAttackThink

Stasis alien attack projectile
-------------------------
*/
void FX_StasisAttackThink( centity_t *cent, const struct weaponInfo_s *weapon )
{
	if ( cent->gent == NULL )
		return;

	FX_AddSprite( cent->lerpOrigin, NULL, NULL, 16.0f + random() * 27.0f, 0.0f, 0.2f, 0.2f, 0, 0.0f, 1, cgs.media.purpleParticleShader );
	FX_AddSprite( cent->lerpOrigin, NULL, NULL, 6.0f + random() * 8.0f, 0.0f, 1.0f, 0.0f, 0, 0.0f, 1, cgs.media.ltblueParticleShader );

	vec3_t	end, dir, angles, right;
	float	len;
	int		ct, i;

	// Convert direction of travel into dir vector
	VectorCopy( cent->gent->s.pos.trDelta, dir );
	VectorNormalize( dir );

	vectoangles( dir, angles );

	ct = rand() & 1; // return 1 or zero
	for ( i = 0; i <= ct; i++ )
	{
		angles[2] = random() * 360;
		AngleVectors( angles, NULL, right, NULL );

		len = crandom() * 6;
		len += (len > 0 ? 4 : -4);
		VectorMA( cent->lerpOrigin, len, right, end );

		FX_AddElectricity( cent->lerpOrigin, end, 1.0, 1.0, 0.0, 1.0, 0.0, 1.0, cgs.media.boltShader, FXF_TAPER, 1.4f );
	}
}

/*
-------------------------
FX_StasisAttackHitPlayer

Stasis alien attack projectile
-------------------------
*/
void FX_StasisAttackHitPlayer( vec3_t origin, vec3_t dir )
{
	// Just call this for now
	FX_StasisWeaponHitPlayer( origin, dir, 2 );

	cgi_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, cg_weapons[WP_STASIS_ATTACK].missileHitSound );
}

/*
-------------------------
FX_StasisAttackHitWall

Stasis alien attack projectile
-------------------------
*/
void FX_StasisAttackHitWall( vec3_t origin, vec3_t dir )
{
	// Just call this for now
	FX_StasisWeaponHitWall( origin, dir, 2 );

	// A sound will get played in stasis weapon hit wall, but put this in here for later
	//cgi_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, cgi_S_RegisterSound("sound/weapons/stasis_alien/hit_wall.wav") );	
	cgi_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, cg_weapons[WP_STASIS_ATTACK].missileHitSound );
}

/*
-------------------------
FX_StasisCharge

Stasis charge build-up that happens before a fire
-------------------------
*/
void FX_StasisCharge( vec3_t origin, vec3_t dir, float perc_done )
{
	FX_AddSprite( origin, NULL, NULL, 
					32.0f + random() * 54.0f, 0.0f, 
					0.2f * perc_done, 0.2f, 
					0, 0.0f, 
					175, 
					cgs.media.purpleParticleShader );

	FX_AddSprite( origin, NULL, NULL, 
					12.0f + random() * 16.0f, 0.0f, 
					1.0f * perc_done, 0.2f, 
					0, 0.0f, 
					175, 
					cgs.media.blueParticleShader );

	vec3_t	end, angles, forward;
	float	len;
	int		ct, i;

	// Convert direction of travel into dir vector
	vectoangles( dir, angles );

	ct = rand() & 1; // return 1 or zero
	for ( i = 0; i <= ct; i++ )
	{
		angles[2] = random() * 360;
		AngleVectors( angles, NULL, forward, NULL );

		len = crandom() * 12;
		len += (len > 0 ? 8 : -8);
		VectorMA( origin, len, forward, end );

		FX_AddElectricity( origin, end, 
							1.0, 
							1.5f, 0.0, 
							1.0 * perc_done, 0.2f, 
							150.0, 
							cgs.media.boltShader, 
							FXF_TAPER, 
							1.6f );
	}
}


/*
-------------------------
FX_StasisBeamIn

Stasis effect for when they are beaming in
Particles that race out from the core, then slow down to a stop at the end of their life
-------------------------
*/

void FX_StasisBeamInParticles( vec3_t origin, vec3_t angles, vec3_t velocity, vec3_t user )
{
	vec3_t	dir, vel, accel;
	float	len, acceleration, vf;
	int		time;

	for ( int i = 0; i < 4; i++ )
	{
		// Pick a random direction..
		VectorSet( dir, crandom(), crandom(), crandom() );
		VectorNormalize( dir );
		
		// ..pick a random length
		len = random() * 16 + 48;

		// Now build an acceleration vector
		acceleration = 256 + random() * 64;
		VectorScale( dir, -acceleration, accel );

		// FIXME: This mostly works like I'd like, though I wonder if this can/should be cleaned up
		// Calculate how long the thing will take to travel that distance--convert to the timescale we use
		time = sqrt( 2  / acceleration * len ) * 1000.0f;

		vf = sqrt( 2 * len * acceleration ); // calculate how fast it would be moving at the end of its path
		VectorScale( dir, vf, vel );		//	this will be the _initial_ velocity for those starting in the middle

		if ( rand() & 1 )
		{
			FX_AddSprite( origin, vel, accel, 1 + random(), 4 + random() * 4, 1.0, 0.0, 0,0, time, cgs.media.purpleParticleShader );
		}
		else
		{
			FX_AddSprite( origin, vel, accel, 1 + random(), 4 + random() * 4, 1.0, 0.0, 0,0, time, cgs.media.ltblueParticleShader );
		}
	}
}

void FX_StasisBeamIn( vec3_t origin )
{
	vec3_t up = {0,0,1}, org, start, end;

	VectorCopy( origin, org );
	org[2] += 16;

	FX_AddSpawner( org, NULL, NULL, NULL, 10, 5, 120, FX_StasisBeamInParticles, NULL );

	VectorMA( org, 60, up, start );
	VectorMA( org, -50, up, end );

	FX_AddLine( start, end, 1, 1, 200, 1.0, 0.0, 600, cgs.media.blueParticleShader );
	FX_AddLine( start, end, 1, 1, 100, 1.0, 0.0, 600, cgs.media.blueParticleShader );
}

/*
-------------------------
FX_StasisBeamOut

Stasis effect for when they are "dead" and need to beam-out
Particles that accelerate into the core of the effect
-------------------------
*/

//void FX_StasisBeamOutParticles( vec3_t origin, vec3_t angles, vec3_t velocity, vec3_t user )
void FX_StasisBeamOutParticles( vec3_t origin )
{
	vec3_t	dir, vel, accel;
	float	acceleration;
	int		time;

	for ( int i = 0; i < 40; i++ )
	{
		// Pick a random direction..
		VectorSet( dir, crandom(), crandom(), crandom() );
		VectorNormalize( dir );
		
		// Now build an acceleration vector
		acceleration = 220;
		VectorScale( dir, acceleration, vel );
		VectorScale( vel, -0.4, accel );
		time = 800.0f;

		if ( rand() & 1 )
		{
			FX_AddSprite( origin, vel, accel, 6 + random() * 6, -(4 + random() * 6), 1.0, 0.0, 0,0, time, cgs.media.purpleParticleShader );
		}
		else if ( rand() & 1 )
		{
			FX_AddSprite( origin, vel, accel, 6 + random() * 6, -(4 + random() * 6), 1.0, 0.0, 0,0, time, cgs.media.ltblueParticleShader );
		}
		else
		{
			FX_AddSprite( origin, vel, accel, 6 + random() * 6, -(4 + random() * 6), 1.0, 0.0, 0,0, time, cgs.media.blueParticleShader );
		}
	}
}

void FX_StasisBeamOut( vec3_t origin )
{
	vec3_t up = {0,0,1}, org, start, end;

	VectorCopy( origin, org );
	org[2] += 16;

	//FX_AddSpawner( org, NULL, NULL, NULL, 20, 0, 150, FX_StasisBeamOutParticles, NULL );
	FX_StasisBeamOutParticles( org );
	
	VectorMA( org, 80, up, start );
	VectorMA( org, -50, up, end );

	FX_AddLine( start, end, 1, 40, 128, 1.0, 0.0, 400, cgs.media.blueHitShader );

	VectorMA( org, 20, up, start );
	VectorMA( org, -15, up, end );

	FX_AddLine( start, end, 1, 40, 500, 1.0, 0.0, 400, cgs.media.ghostRingShader );
}

/*
-------------------------
FX_StasisMineExplode
-------------------------
*/
void FX_StasisMineExplode( vec3_t origin )
{
	FXSprite	*fx;
	FXTrail		*particle;
	vec3_t		rgb, org, moveDir, accel = { 0, 0, -400 };
	int			i;

	// make some elongated splashes
	for ( i = 0; i < 8; i++ )
	{	
		VectorSet( rgb,
					random() * 0.2f + 0.3f,
					random() * 0.6f + 0.1f,
					random() * 0.4f + 0.6f );

		VectorSet( moveDir, crandom(), crandom(), random() );
		VectorNormalize( moveDir );

		particle = FX_AddTrail( origin, NULL, NULL, 8.0f, -1.0f, 4.0, -1.0,
								1.0f, 0.5f, rgb, rgb, 0.4f, 1400.0f + random() * 500.0f, cgs.media.spooShader, rand() & FXF_BOUNCE );

		if ( particle != NULL )
		{
			FXE_Spray( moveDir, 80, 160, 0.1f, 256, (FXPrimitive *) particle );
		}
	}

	// Make a real splash
	for ( i = 0; i < 12; i++ )
	{
		VectorSet( org,
					origin[0] + crandom() * 4.0f,
					origin[1] + crandom() * 4.0f,
					origin[2] + crandom() * 4.0f );
	
		for ( int j = 0; j < 3; j++ )
		{
			moveDir[j] = crandom() * 55.0;
		}
		moveDir[2] += 120.0f;

		VectorSet( rgb,
					random() * 0.3f + 0.5f,
					random() * 0.2f + 0.3f,
					random() * 0.3f + 0.6f );

		fx = FX_AddSprite( org, moveDir, accel, 4.0f + random() * 6.0f, -4.0f, 
					1.0f, 0.5f, 
					rgb, rgb, 
					0, 0.0f, 
					250 + random() * 400, 
					cgs.media.spooShader );

		if ( fx == NULL )
			return;

		fx->SetRoll( 0 );
	}

	CG_Chunks( 0, org, moveDir, random() * 2 + 2, Q_irand(4, 6), MAT_STASIS, 0, 0.3f );

	// Orient the explosion so that it faces the viewer
	VectorScale( cg.refdef.viewaxis[0], -1, moveDir );
	CG_MakeExplosion( org, moveDir, cgs.media.explosionModel, 6, cgs.media.electricalExplosionSlowShader, 500, qfalse, random() * 0.2f + 0.7f );
}