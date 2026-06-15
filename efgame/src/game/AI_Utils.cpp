// These utilities are meant for strictly non-player, non-team NPCs.  
// These functions are in their own file because they are only intended
// for use with NPCs who's logic has been overriden from the original
// AI code, and who's code resides in files with the AI_ prefix.

#include "b_local.h"
#include "g_nav.h"

#define	MAX_RADIUS_ENTS		128
#define	DEFAULT_RADIUS		45

/*
-------------------------
AI_GetGroupSize
-------------------------
*/

int	AI_GetGroupSize( vec3_t origin, int radius, team_t playerTeam, gentity_t *avoid )
{
	gentity_t	*radiusEnts[ MAX_RADIUS_ENTS ];
	vec3_t		mins, maxs;
	int			numEnts, realCount = 0;

	//Setup the bbox to search in
	for ( int i = 0; i < 3; i++ )
	{
		mins[i] = origin[i] - radius;
		maxs[i] = origin[i] + radius;
	}

	//Get the number of entities in a given space
	numEnts = gi.EntitiesInBox( mins, maxs, radiusEnts, MAX_RADIUS_ENTS );

	//Cull this list
	for ( int j = 0; j < numEnts; j++ )
	{
		//Validate clients
		if ( radiusEnts[ j ]->client == NULL )
			continue;

		//Skip the requested avoid ent if present
		if ( ( avoid != NULL ) && ( radiusEnts[ j ] == avoid ) )
			continue;

		//Must be on the same team
		if ( radiusEnts[ j ]->client->playerTeam != playerTeam )
			continue;

		//Must be alive
		if ( radiusEnts[ j ]->health <= 0 )
			continue;

		realCount++;
	}

	return realCount;
}

//Overload

int AI_GetGroupSize( gentity_t *ent, int radius )
{
	if ( ( ent == NULL ) || ( ent->client == NULL ) )
		return -1;

	return AI_GetGroupSize( ent->currentOrigin, radius, ent->client->playerTeam, ent );
}

/*
-------------------------
AI_GetGroup
-------------------------
*/

//NOTENOTE: I hate duplicating code, but this is so specific...

//FIXME: Use with caution, these are too special-cased at the moment

void AI_GetGroup( AIGroupInfo_t &group, vec3_t origin, vec3_t angles, int cone, int radius, team_t playerTeam, gentity_t *avoid, gentity_t *enemy )
{
	gentity_t	*radiusEnts[ MAX_RADIUS_ENTS ];
	vec3_t		mins, maxs;
	int			numEnts;

	memset( &group, 0, sizeof( group ) );

	//Setup the bbox to search in
	for ( int i = 0; i < 3; i++ )
	{
		mins[i] = origin[i] - radius;
		maxs[i] = origin[i] + radius;
	}

	//Get the number of entities in a given space
	numEnts = gi.EntitiesInBox( mins, maxs, radiusEnts, MAX_RADIUS_ENTS );

	//Cull this list
	for ( int j = 0; j < numEnts; j++ )
	{
		//Validate clients
		if ( radiusEnts[ j ]->client == NULL )
			continue;

		//Skip the requested avoid ent if present
		if ( ( avoid != NULL ) && ( radiusEnts[ j ] == avoid ) )
			continue;

		//Must be on the same team
		if ( radiusEnts[ j ]->client->playerTeam != playerTeam )
			continue;

		//Must be alive
		if ( radiusEnts[ j ]->health <= 0 )
			continue;

		group.numGroup++;

		//Check for in FOV
		if ( InFOV( radiusEnts[ j ]->currentOrigin, origin, angles, cone, cone ) )
		{
			//Check for a supplied enemy
			if ( enemy != NULL )
			{
				//If the enemy is closer to use than the teammate, then disregard them
				if ( DistanceSquared( enemy->currentOrigin, origin ) < DistanceSquared( radiusEnts[ j ]->currentOrigin, origin ) )
					continue;
			}

			//Another one found
			group.numFront++;
		}

		group.numState[ radiusEnts[ j ]->NPC->squadState ]++;
	}
}

//Overload 

void AI_GetGroup( AIGroupInfo_t &group, gentity_t *ent, int radius )
{
	if ( ent->client == NULL )
		return;

	vec3_t	temp, angles;

	//FIXME: This is specialized code.. move?
	if ( ent->enemy )
	{
		VectorSubtract( ent->enemy->currentOrigin, ent->currentOrigin, temp );
		VectorNormalize( temp );	//FIXME: Needed?
		vectoangles( temp, angles );
	}
	else
	{
		VectorCopy( ent->currentAngles, angles );
	}

	AI_GetGroup( group, ent->currentOrigin, ent->currentAngles, DEFAULT_RADIUS, radius, ent->client->playerTeam, ent, ent->enemy );
}

/*
-------------------------
AI_CheckEnemyCollision
-------------------------
*/

qboolean AI_CheckEnemyCollision( gentity_t *ent, qboolean takeEnemy )
{
	if ( ent == NULL )
		return qfalse;

	if ( ent->svFlags & SVF_LOCKEDENEMY )
		return qfalse;

	navInfo_t	info;

	NAV_GetLastMove( info );

	//See if we've hit something
	if ( ( info.blocker ) && ( info.blocker != ent->enemy ) )
	{
		if ( ( info.blocker->client ) && ( info.blocker->client->playerTeam == ent->client->enemyTeam ) )
		{
			if ( takeEnemy )
				G_SetEnemy( ent, info.blocker );

			return qtrue;
		}
	}

	return qfalse;
}

/*
-------------------------
AI_DistributeAttack
-------------------------
*/

#define	MAX_RADIUS_ENTS		128

gentity_t *AI_DistributeAttack( gentity_t *attacker, gentity_t *enemy, team_t team, int threshold )
{
	//Don't take new targets
	if ( NPC->svFlags & SVF_LOCKEDENEMY )
		return enemy;

	int	numSurrounding = AI_GetGroupSize( enemy->currentOrigin, 48, team, attacker );

	//First, see if we should look for the player
	if ( enemy != &g_entities[0] )
	{
		int	aroundPlayer = AI_GetGroupSize( g_entities[0].currentOrigin, 48, team, attacker );

		//See if we're above our threshold
		if ( aroundPlayer < threshold )
		{
			return &g_entities[0];
		}
	}

	//See if our current enemy is still ok
	if ( numSurrounding < threshold )
		return enemy;

	//Otherwise we need to take a new enemy if possible
	vec3_t	mins, maxs;

	//Setup the bbox to search in
	for ( int i = 0; i < 3; i++ )
	{
		mins[i] = enemy->currentOrigin[i] - 512;
		maxs[i] = enemy->currentOrigin[i] + 512;
	}

	//Get the number of entities in a given space
	gentity_t	*radiusEnts[ MAX_RADIUS_ENTS ];

	int numEnts = gi.EntitiesInBox( mins, maxs, radiusEnts, MAX_RADIUS_ENTS );

	//Cull this list
	for ( int j = 0; j < numEnts; j++ )
	{
		//Validate clients
		if ( radiusEnts[ j ]->client == NULL )
			continue;

		//Skip the requested avoid ent if present
		if ( ( radiusEnts[ j ] == enemy ) )
			continue;

		//Must be on the same team
		if ( radiusEnts[ j ]->client->playerTeam != enemy->client->playerTeam )
			continue;

		//Must be alive
		if ( radiusEnts[ j ]->health <= 0 )
			continue;

		//Must not be overwhelmed
		if ( AI_GetGroupSize( radiusEnts[j]->currentOrigin, 48, team, attacker ) > threshold )
			continue;

		return radiusEnts[j];
	}

	return NULL;
}
