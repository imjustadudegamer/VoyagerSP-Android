//NPC_senses.cpp
#include "b_local.h"

/*
qboolean G_ClearLineOfSight(const vec3_t point1, const vec3_t point2, int ignore, int clipmask)

returns true if can see from point 1 to 2, even through glass (1 pane)- doesn't work with portals
*/
qboolean G_ClearLineOfSight(const vec3_t point1, const vec3_t point2, int ignore, int clipmask)
{
	trace_t		tr;

	gi.trace ( &tr, point1, NULL, NULL, point2, ignore, clipmask );
	if ( tr.fraction == 1.0 ) 
	{
		return qtrue;
	}

	gentity_t	*hit = &g_entities[ tr.entityNum ];
	if(EntIsGlass(hit))
	{
		vec3_t	newpoint1;
		VectorCopy(tr.endpos, newpoint1);
		gi.trace (&tr, newpoint1, NULL, NULL, point2, hit->s.number, clipmask );

		if ( tr.fraction == 1.0 ) 
		{
			return qtrue;
		}
	}

	return qfalse;
}

/*
CanSee
determine if NPC can see an entity

This is a straight line trace check.  This function does not look at PVS or FOV,
or take any AI related factors (for example, the NPC's reaction time) into account

FIXME do we need fat and thin version of this?
*/
qboolean CanSee ( gentity_t *ent ) 
{
	trace_t		tr;
	vec3_t		eyes;
	vec3_t		spot;

	CalcEntitySpot( NPC, SPOT_HEAD_LEAN, eyes );

	CalcEntitySpot( ent, SPOT_ORIGIN, spot );
	gi.trace ( &tr, eyes, NULL, NULL, spot, NPC->s.number, MASK_OPAQUE );
	ShotThroughGlass (&tr, ent, spot, MASK_OPAQUE);
	if ( tr.fraction == 1.0 ) 
	{
		return qtrue;
	}

	CalcEntitySpot( ent, SPOT_HEAD, spot );
	gi.trace ( &tr, eyes, NULL, NULL, spot, NPC->s.number, MASK_OPAQUE );
	ShotThroughGlass (&tr, ent, spot, MASK_OPAQUE);
	if ( tr.fraction == 1.0 ) 
	{
		return qtrue;
	}

	CalcEntitySpot( ent, SPOT_LEGS, spot );
	gi.trace ( &tr, eyes, NULL, NULL, spot, NPC->s.number, MASK_OPAQUE );
	ShotThroughGlass (&tr, ent, spot, MASK_OPAQUE);
	if ( tr.fraction == 1.0 ) 
	{
		return qtrue;
	}

	return qfalse;
}


/*
InFOV

IDEA: further off to side of FOV range, higher chance of failing even if technically in FOV,
	keep core of 50% to sides as always succeeding
*/

//Position compares

qboolean InFOV( vec3_t spot, vec3_t from, vec3_t fromAngles, int hFOV, int vFOV )
{
	vec3_t	deltaVector, angles, deltaAngles;

	VectorSubtract ( spot, from, deltaVector );
	vectoangles ( deltaVector, angles );
	
	deltaAngles[PITCH]	= AngleDelta ( fromAngles[PITCH], angles[PITCH] );
	deltaAngles[YAW]	= AngleDelta ( fromAngles[YAW], angles[YAW] );

	if ( fabs ( deltaAngles[PITCH] ) <= vFOV && fabs ( deltaAngles[YAW] ) <= hFOV ) 
	{
		return qtrue;
	}

	return qfalse;
}

//NPC to position

qboolean InFOV( vec3_t origin, gentity_t *from, int hFOV, int vFOV ) 
{
	vec3_t	fromAngles, eyes;

	if( from->client )
	{
		VectorCopy(from->client->ps.viewangles, fromAngles);
	}
	else
	{
		VectorCopy(from->s.angles, fromAngles);
	}

	CalcEntitySpot( from, SPOT_HEAD, eyes );

	return InFOV( origin, eyes, fromAngles, hFOV, vFOV );
}

//Entity to entity

qboolean InFOV ( gentity_t *ent, gentity_t *from, int hFOV, int vFOV ) 
{
	vec3_t	eyes;
	vec3_t	spot;
	vec3_t	deltaVector;
	vec3_t	angles, fromAngles;
	vec3_t	deltaAngles;

	if( from->client )
	{
		if( VectorLengthSquared( from->client->renderInfo.eyeAngles ) )
		{//Actual facing of tag_head!
			//NOTE: Stasis aliens may have a problem with this?
			VectorCopy( from->client->renderInfo.eyeAngles, fromAngles );
		}
		else
		{
			VectorCopy( from->client->ps.viewangles, fromAngles );
		}
	}
	else
	{
		VectorCopy(from->s.angles, fromAngles);
	}

	CalcEntitySpot( from, SPOT_HEAD_LEAN, eyes );

	CalcEntitySpot( ent, SPOT_ORIGIN, spot );
	VectorSubtract ( spot, eyes, deltaVector);

	vectoangles ( deltaVector, angles );
	deltaAngles[PITCH] = AngleDelta ( fromAngles[PITCH], angles[PITCH] );
	deltaAngles[YAW] = AngleDelta ( fromAngles[YAW], angles[YAW] );
	if ( fabs ( deltaAngles[PITCH] ) <= vFOV && fabs ( deltaAngles[YAW] ) <= hFOV ) 
	{
		return qtrue;
	}

	CalcEntitySpot( ent, SPOT_HEAD, spot );
	VectorSubtract ( spot, eyes, deltaVector);
	vectoangles ( deltaVector, angles );
	deltaAngles[PITCH] = AngleDelta ( fromAngles[PITCH], angles[PITCH] );
	deltaAngles[YAW] = AngleDelta ( fromAngles[YAW], angles[YAW] );
	if ( fabs ( deltaAngles[PITCH] ) <= vFOV && fabs ( deltaAngles[YAW] ) <= hFOV ) 
	{
		return qtrue;
	}

	CalcEntitySpot( ent, SPOT_LEGS, spot );
	VectorSubtract ( spot, eyes, deltaVector);
	vectoangles ( deltaVector, angles );
	deltaAngles[PITCH] = AngleDelta ( fromAngles[PITCH], angles[PITCH] );
	deltaAngles[YAW] = AngleDelta ( fromAngles[YAW], angles[YAW] );
	if ( fabs ( deltaAngles[PITCH] ) <= vFOV && fabs ( deltaAngles[YAW] ) <= hFOV ) 
	{
		return qtrue;
	}

	return qfalse;
}

qboolean InVisrange ( gentity_t *ent ) 
{//FIXME: make a calculate visibility for ents that takes into account
	//lighting, movement, turning, crouch/stand up, other anims, hide brushes, etc.
	vec3_t	eyes;
	vec3_t	spot;
	vec3_t	deltaVector;
	float	visrange = (NPCInfo->stats.visrange*NPCInfo->stats.visrange);

	CalcEntitySpot( NPC, SPOT_HEAD_LEAN, eyes );

	CalcEntitySpot( ent, SPOT_ORIGIN, spot );
	VectorSubtract ( spot, eyes, deltaVector);

	/*if(ent->client)
	{
		float	vel, avel;
		if(ent->client->ps.velocity[0] || ent->client->ps.velocity[1] || ent->client->ps.velocity[2])
		{
			vel = VectorLength(ent->client->ps.velocity);
			if(vel > 128)
			{
				visrange += visrange * (vel/256);
			}
		}

		if(ent->avelocity[0] || ent->avelocity[1] || ent->avelocity[2])
		{//FIXME: shouldn't they need to have line of sight to you to detect this?
			avel = VectorLength(ent->avelocity);
			if(avel > 15)
			{
				visrange += visrange * (avel/60);
			}
		}
	}*/

	if(VectorLengthSquared(deltaVector) > visrange)
	{
		return qfalse;
	}

	return qtrue;
}

/*
NPC_CheckVisibility
*/

visibility_t NPC_CheckVisibility ( gentity_t *ent, int flags ) 
{
	// flags should never be 0
	if ( !flags ) 
	{
		return VIS_NOT;
	}

	// check PVS
	if ( flags & CHECK_PVS ) 
	{
		if ( !gi.inPVS ( ent->currentOrigin, NPC->currentOrigin ) ) 
		{
			return VIS_NOT;
		}
	}
	if ( !(flags & (CHECK_360|CHECK_FOV|CHECK_SHOOT)) ) 
	{
		return VIS_PVS;
	}

	// check within visrange
	if (flags & CHECK_VISRANGE)
	{
		if( !InVisrange ( ent ) ) 
		{
			return VIS_PVS;
		}
	}

	// check 360 degree visibility
	//Meaning has to be a direct line of site
	if ( flags & CHECK_360 ) 
	{
		if ( !CanSee ( ent ) ) 
		{
			return VIS_PVS;
		}
	}
	if ( !(flags & (CHECK_FOV|CHECK_SHOOT)) ) 
	{
		return VIS_360;
	}

	// check FOV
	if ( flags & CHECK_FOV ) 
	{
		if ( !InFOV ( ent, NPC, NPCInfo->stats.hfov, NPCInfo->stats.vfov) ) 
		{
			return VIS_360;
		}
	}

	if ( !(flags & CHECK_SHOOT) ) 
	{
		return VIS_FOV;
	}

	// check shootability
	if ( flags & CHECK_SHOOT ) 
	{
		if ( !CanShoot ( ent, NPC ) ) 
		{
			return VIS_FOV;
		}
	}

	return VIS_SHOOT;
}

/*
-------------------------
NPC_CheckSoundEvents
-------------------------
*/

int NPC_CheckSoundEvents( void )
{
	int	bestEvent = -1;
	int bestAlert = -1;

	for ( int i = 0; i < level.numAlertEvents; i++ )
	{
		//We're only concerned about sounds
		if ( level.alertEvents[i].type != AET_SOUND )
			continue;

		//Must be within range
		int radius = level.alertEvents[i].radius * level.alertEvents[i].radius;

		if ( DistanceSquared( level.alertEvents[i].position, NPC->currentOrigin ) > radius )
			continue;

		//See if this one takes precedence over the previous one
		if ( level.alertEvents[i].level > bestAlert )
		{
			bestEvent = i;
			bestAlert = level.alertEvents[i].level;
		}
	}

	return bestEvent;
}

/*
-------------------------
NPC_CheckSightEvents
-------------------------
*/

int NPC_CheckSightEvents( void )
{
	int	bestEvent = -1;
	int bestAlert = -1;

	for ( int i = 0; i < level.numAlertEvents; i++ )
	{
		//We're only concerned about sounds
		if ( level.alertEvents[i].type != AET_SIGHT )
			continue;

		//Must be within range
		int radius = level.alertEvents[i].radius * level.alertEvents[i].radius;

		if ( DistanceSquared( level.alertEvents[i].position, NPC->currentOrigin ) > radius )
			continue;

		//Must be visible
		if ( InFOV( level.alertEvents[i].position, NPC, NPCInfo->stats.hfov, NPCInfo->stats.vfov ) == qfalse )
			continue;

		if ( NPC_ClearLOS( level.alertEvents[i].position ) == qfalse )
			continue;

		//See if this one takes precedence over the previous one
		if ( level.alertEvents[i].level > bestAlert )
		{
			bestEvent = i;
			bestAlert = level.alertEvents[i].level;
		}
	}

	return bestEvent;
}

/*
-------------------------
NPC_CheckAlertEvents

    NOTE: Should all NPCs create alertEvents too so they can detect each other?
-------------------------
*/

int NPC_CheckAlertEvents( void )
{
	int bestSoundEvent = -1;
	int bestSightEvent = -1;

	bestSoundEvent = NPC_CheckSoundEvents();
	bestSightEvent = NPC_CheckSightEvents();

	//FIXME:	This doesn't take the relavence of the event into account, 
	//			so an important event far away will override a lesser one close up!

	return ( bestSoundEvent >= bestSightEvent ) ? bestSoundEvent : bestSightEvent;
}

/*
-------------------------
AddSoundEvent
-------------------------
*/

void AddSoundEvent( gentity_t *owner, vec3_t position, float radius, alertEventLevel_e alertLevel )
{
	//FIXME: Handle this in another manner?
	if ( level.numAlertEvents >= MAX_ALERT_EVENTS )
		return;

	if ( owner == NULL )
		return;

	VectorCopy( position, level.alertEvents[ level.numAlertEvents ].position );

	level.alertEvents[ level.numAlertEvents ].radius	= radius;
	level.alertEvents[ level.numAlertEvents ].level		= alertLevel;
	level.alertEvents[ level.numAlertEvents ].type		= AET_SOUND;
	level.alertEvents[ level.numAlertEvents ].owner		= owner;

	level.numAlertEvents++;
}

/*
-------------------------
AddSightEvent
-------------------------
*/

void AddSightEvent( gentity_t *owner, vec3_t position, float radius, alertEventLevel_e alertLevel )
{
	//FIXME: Handle this in another manner?
	if ( level.numAlertEvents >= MAX_ALERT_EVENTS )
		return;

	if ( owner == NULL )
		return;

	VectorCopy( position, level.alertEvents[ level.numAlertEvents ].position );

	level.alertEvents[ level.numAlertEvents ].radius	= radius;
	level.alertEvents[ level.numAlertEvents ].level		= alertLevel;
	level.alertEvents[ level.numAlertEvents ].type		= AET_SIGHT;
	level.alertEvents[ level.numAlertEvents ].owner		= owner;		

	level.numAlertEvents++;
}

/*
-------------------------
ClearPlayerAlertEvents
-------------------------
*/

void ClearPlayerAlertEvents( void )
{
	level.numAlertEvents = 0;
}

//NOTENOTE: Sigh... the lengths I go to making things simple...
/*
-------------------------
NPC_ClearLOS
-------------------------
*/

// Position to position

qboolean NPC_ClearLOS( vec3_t start, vec3_t end )
{
	trace_t		tr;
	
	//FIXME: ENTITYNUM_NONE ok?
	gi.trace ( &tr, start, NULL, NULL, end, ENTITYNUM_NONE, CONTENTS_SOLID/*(CONTENTS_SOLID|CONTENTS_MONSTERCLIP)*/ );

	if ( tr.fraction == 1.0 ) 
		return qtrue;

	return qfalse;
}

//Entity to position

qboolean NPC_ClearLOS( gentity_t *ent, vec3_t end )
{
	vec3_t	eyes;

	CalcEntitySpot( ent, SPOT_HEAD, eyes );

	return NPC_ClearLOS( eyes, end );
}

//Position to entity

qboolean NPC_ClearLOS( vec3_t start, gentity_t *ent )
{
	vec3_t		spot;

	//Look for the chest first
	CalcEntitySpot( ent, SPOT_ORIGIN, spot );

	if ( NPC_ClearLOS( start, spot ) )
		return qtrue;

	//Look for the head next
	CalcEntitySpot( ent, SPOT_HEAD, spot );

	if ( NPC_ClearLOS( start, spot ) )
		return qtrue;

	return qfalse;
}

//NPC's eyes to entity

qboolean NPC_ClearLOS( gentity_t *ent ) 
{
	vec3_t	eyes;

	//Calculate the NPC's position
	CalcEntitySpot( NPC, SPOT_HEAD, eyes );
	
	return NPC_ClearLOS( eyes, ent );
}

//NPC's eyes to position

qboolean NPC_ClearLOS( vec3_t end )
{
	vec3_t	eyes;

	//Calculate the NPC's position
	CalcEntitySpot( NPC, SPOT_HEAD, eyes );
	
	return NPC_ClearLOS( eyes, end );
}

/*
-------------------------
NPC_ClearShot
-------------------------
*/

qboolean NPC_ClearShot( gentity_t *ent )
{
	if ( ( NPC == NULL ) || ( ent == NULL ) )
		return qfalse;

	vec3_t	muzzle;
	trace_t	tr;

	CalcEntitySpot( NPC, SPOT_WEAPON, muzzle );
	
	//Hack for scavenger aim error
	if ( NPC->client->playerTeam == TEAM_SCAVENGERS )
	{
		vec3_t	mins = { -2, -2, -2 };
		vec3_t	maxs = {  2,  2,  2 };

		gi.trace ( &tr, muzzle, mins, maxs, ent->currentOrigin, NPC->s.number, MASK_SHOT );
	}
	else
	{
		gi.trace ( &tr, muzzle, NULL, NULL, ent->currentOrigin, NPC->s.number, MASK_SHOT );
	}

	if ( tr.entityNum == ent->s.number ) 
		return qtrue;
	
	return qfalse;
}

/*
-------------------------
NPC_GetFOVPercentage
-------------------------
*/

float NPC_GetHFOVPercentage( vec3_t spot, vec3_t from, vec3_t facing, float hFOV )
{
	vec3_t	deltaVector, angles;
	float	delta;

	VectorSubtract ( spot, from, deltaVector );

	vectoangles ( deltaVector, angles );
	
	delta = fabs( AngleDelta ( facing[YAW], angles[YAW] ) );

	if ( delta > hFOV )
		return 0.0f; 

	return ( ( hFOV - delta ) / hFOV );
}

/*
-------------------------
NPC_GetVFOVPercentage
-------------------------
*/

float NPC_GetVFOVPercentage( vec3_t spot, vec3_t from, vec3_t facing, float vFOV )
{
	vec3_t	deltaVector, angles;
	float	delta;

	VectorSubtract ( spot, from, deltaVector );

	vectoangles ( deltaVector, angles );
	
	delta = fabs( AngleDelta ( facing[PITCH], angles[PITCH] ) );

	if ( delta > vFOV )
		return 0.0f; 

	return ( ( vFOV - delta ) / vFOV );
}
