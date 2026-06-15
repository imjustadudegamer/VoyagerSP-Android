#include "g_local.h"
#include "g_functions.h"
#include "anims.h"
#include "boltons.h"

extern void Q3_DebugPrint( int level, const char *format, ... );
// g_client.c -- client functions that don't happen every frame

float DEFAULT_MINS_0 = -12;
float DEFAULT_MINS_1 = -12;
float DEFAULT_MAXS_0 = 12;
float DEFAULT_MAXS_1 = 12;
float DEFAULT_PLAYER_RADIUS	= sqrt((DEFAULT_MAXS_0*DEFAULT_MAXS_0) + (DEFAULT_MAXS_1*DEFAULT_MAXS_1));
vec3_t playerMins = {DEFAULT_MINS_0, DEFAULT_MINS_1, DEFAULT_MINS_2};
vec3_t playerMaxs = {DEFAULT_MAXS_0, DEFAULT_MAXS_1, DEFAULT_MAXS_2};

void SP_misc_teleporter_dest (gentity_t *ent);

/*QUAKED info_player_deathmatch (1 0 1) (-16 -16 -24) (16 16 32) - - NODRAW
potential spawning position for deathmatch games.
Targets will be fired when someone spawns in on them.
*/
void SP_info_player_deathmatch(gentity_t *ent) {
	SP_misc_teleporter_dest (ent);
}

/*QUAKED info_player_start (1 0 0) (-16 -16 -24) (16 16 32) KEEP_PREV DROPTOFLOOR x x x SILENTSPAWN NOWEAPON x
KEEP_PREV - keep previous health/ammo/etc
DROPTOFLOOR - Player will start on the first solid structure under it
SILENTSPAWN - No teleporter sound

Targets will be fired when someone spawns in on them.
equivalant to info_player_deathmatch
*/
void SP_info_player_start(gentity_t *ent) {
	ent->classname = "info_player_deathmatch";

	ent->spawnflags |= 1;	// James suggests force-ORing the KEEP_PREV flag in for now

	SP_info_player_deathmatch( ent );
}



/*
=======================================================================

  SelectSpawnPoint

=======================================================================
*/

/*
================
SpotWouldTelefrag

================
*/
qboolean SpotWouldTelefrag( gentity_t *spot, team_t checkteam ) 
{
	int			i, num;
	gentity_t	*touch[MAX_GENTITIES], *hit;
	vec3_t		mins, maxs;

	// If we have a mins, use that instead of the hardcoded bounding box
	if ( spot->mins && VectorLength( spot->mins ) )
		VectorAdd( spot->s.origin, spot->mins, mins );
	else
		VectorAdd( spot->s.origin, playerMins, mins );

	// If we have a maxs, use that instead of the hardcoded bounding box
	if ( spot->maxs && VectorLength( spot->maxs ) )
		VectorAdd( spot->s.origin, spot->maxs, maxs );
	else
		VectorAdd( spot->s.origin, playerMaxs, maxs );

	num = gi.EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );

	for (i=0 ; i<num ; i++) 
	{
		hit = touch[i];
		if ( hit->client && hit->client->ps.stats[STAT_HEALTH] > 0 ) 
		{
			if ( hit->contents & CONTENTS_BODY )
			{
				if( checkteam == TEAM_FREE || hit->client->playerTeam == checkteam )
				{//checking against teammates only...?
					return qtrue;
				}
			}
		}
	}

	return qfalse;
}

qboolean SpotWouldTelefrag2( gentity_t *mover, vec3_t dest ) 
{
	int			i, num;
	gentity_t	*touch[MAX_GENTITIES], *hit;
	vec3_t		mins, maxs;

	VectorAdd( dest, mover->mins, mins );
	VectorAdd( dest, mover->maxs, maxs );
	num = gi.EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );

	for (i=0 ; i<num ; i++) 
	{
		hit = touch[i];
		if ( hit == mover )
		{
			continue;
		}

		if ( hit->contents & mover->contents )
		{
			return qtrue;
		}
	}

	return qfalse;
}
/*
================
SelectNearestDeathmatchSpawnPoint

Find the spot that we DON'T want to use
================
*/
#define	MAX_SPAWN_POINTS	128
gentity_t *SelectNearestDeathmatchSpawnPoint( vec3_t from, team_t team ) {
	gentity_t	*spot;
	vec3_t		delta;
	float		dist, nearestDist;
	gentity_t	*nearestSpot;

	nearestDist = 999999;
	nearestSpot = NULL;
	spot = NULL;

	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL) {
		/*if ( team == TEAM_RED && ( spot->spawnflags & 2 ) ) {
			continue;
		}
		if ( team == TEAM_BLUE && ( spot->spawnflags & 1 ) ) {
			continue;
		}*/

		if ( spot->targetname != NULL ) {
			//this search routine should never find a spot that is targetted
			continue;
		}
		VectorSubtract( spot->s.origin, from, delta );
		dist = VectorLength( delta );
		if ( dist < nearestDist ) {
			nearestDist = dist;
			nearestSpot = spot;
		}
	}

	return nearestSpot;
}


/*
================
SelectRandomDeathmatchSpawnPoint

go to a random point that doesn't telefrag
================
*/
#define	MAX_SPAWN_POINTS	128
gentity_t *SelectRandomDeathmatchSpawnPoint( team_t team ) {
	gentity_t	*spot;
	int			count;
	int			selection;
	gentity_t	*spots[MAX_SPAWN_POINTS];

	count = 0;
	spot = NULL;

	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL) {
		/*if ( team == TEAM_RED && ( spot->spawnflags & 2 ) ) {
			continue;
		}
		if ( team == TEAM_BLUE && ( spot->spawnflags & 1 ) ) {
			continue;
		}*/

		if ( spot->targetname != NULL ) {
			//this search routine should never find a spot that is targetted
			continue;
		}
		if ( SpotWouldTelefrag( spot, TEAM_FREE ) ) {
			continue;
		}
		spots[ count ] = spot;
		count++;
	}

	if ( !count ) {	// no spots that won't telefrag
		spot = G_Find( NULL, FOFS(classname), "info_player_deathmatch");
		if ( spot->targetname != NULL ) 
		{
			//this search routine should never find a spot that is targetted
			return NULL;
		}
		else
		{
			return spot;
		}
	}

	selection = rand() % count;
	return spots[ selection ];
}


/*
===========
SelectSpawnPoint

Chooses a player start, deathmatch start, etc
============
*/
gentity_t *SelectSpawnPoint ( vec3_t avoidPoint, team_t team, vec3_t origin, vec3_t angles ) {
	gentity_t	*spot;
	gentity_t	*nearestSpot;

	if ( level.spawntarget != NULL && level.spawntarget[0] )
	{//we have a spawnpoint specified, try to find it
		if ( (nearestSpot = spot = G_Find( NULL, FOFS(targetname), level.spawntarget )) == NULL )
		{//you HAVE to be able to find the desired spot
			G_Error( "Couldn't find spawntarget %s\n", level.spawntarget );
			return NULL;
		}
	}
	else 
	{//not looking for a special startspot
		nearestSpot = SelectNearestDeathmatchSpawnPoint( avoidPoint, team );

		spot = SelectRandomDeathmatchSpawnPoint ( team );
		if ( spot == nearestSpot ) {
			// roll again if it would be real close to point of death
			spot = SelectRandomDeathmatchSpawnPoint ( team );
		}
	}

	// find a single player start spot
	if (!spot) {
		G_Error( "Couldn't find a spawn point\n" );
	}


	VectorCopy( spot->s.origin, origin );
	if ( spot->spawnflags & 2 )
	{
		trace_t		tr;

		origin[2] -= 4096;
		gi.trace(&tr, spot->s.origin, playerMins, playerMaxs, origin, -1, MASK_PLAYERSOLID );
		if ( tr.fraction < 1.0 && !tr.allsolid && !tr.startsolid )
		{//found a floor
			VectorCopy(tr.endpos, origin );
		}
		else
		{//In solid or too far
			VectorCopy( spot->s.origin, origin );
		}
	}

	origin[2] += 9;
	VectorCopy (spot->s.angles, angles);

	return spot;
}


//======================================================================


/*
==================
SetClientViewAngle

==================
*/
void SetClientViewAngle( gentity_t *ent, vec3_t angle ) {
	int			i;

	// set the delta angle
	for (i=0 ; i<3 ; i++) {
		ent->client->ps.delta_angles[i] = ANGLE2SHORT(angle[i]) - ent->client->pers.cmd_angles[i];
	}
	VectorCopy( angle, ent->s.angles );
	VectorCopy (ent->s.angles, ent->client->ps.viewangles);
}

/*
================
respawn
================
*/
void respawn( gentity_t *ent ) {

	if (Q_stricmpn(level.mapname,"_holo",5)) {
		gi.SendConsoleCommand("load *respawn\n");	// special case
	}
	else {//we're on the holodeck
		int			flags;
		gentity_t	*tent;

		// toggle the teleport bit so the client knows to not lerp
		flags = ent->client->ps.eFlags;
		ClientSpawn(ent, eNO/*qfalse*/);	// SavedGameJustLoaded_e
		ent->client->ps.eFlags = flags ^ EF_TELEPORT_BIT;

		// add a teleportation effect
		tent = G_TempEntity( ent->client->ps.origin, EV_PLAYER_TELEPORT_IN );
		tent->s.clientNum = ent->s.clientNum;
	}
}


/*
================
PickTeam

================
*/
team_t PickTeam( int ignoreClientNum ) {
	int		i;
	int		counts[TEAM_NUM_TEAMS];

	memset( counts, 0, sizeof( counts ) );

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( i == ignoreClientNum ) {
			continue;
		}
		if ( level.clients[i].pers.connected == CON_DISCONNECTED ) {
			continue;
		}
	}

	return TEAM_FREE;
}

/*
===========
ForceClientSkin

Forces a client's skin (for teamplay)
===========
*/
void ForceClientSkin( gclient_t *client, char *model, const char *skin ) {
	char *p;

	if ((p = strchr(model, '/')) != NULL) {
		*p = 0;
	}

	Q_strcat(model, MAX_QPATH, "/");
	Q_strcat(model, MAX_QPATH, skin);
}

/*
===========
ClientUserInfoChanged

Called from ClientConnect when the player first connects and
directly by the server system when the player updates a userinfo variable.

The game can override any of the settings and call gi.SetUserinfo
if desired.
============
*/
void ClientUserinfoChanged( int clientNum ) {
	gentity_t *ent;
	char	*s;
	char	headModel[MAX_QPATH];
	char	torsoModel[MAX_QPATH];
	char	legsModel[MAX_QPATH];
	char	sound[MAX_QPATH];
	char	oldname[MAX_STRING_CHARS];
	gclient_t	*client;
	char	*sex;
	char	userinfo[MAX_INFO_STRING];

	ent = g_entities + clientNum;
	client = ent->client;

	gi.GetUserinfo( clientNum, userinfo, sizeof( userinfo ) );

	// check for malformed or illegal info strings
	if ( !Info_Validate(userinfo) ) {
		strcpy (userinfo, "\\name\\badinfo");
	}

	// check for lcoal client
	s = Info_ValueForKey( userinfo, "ip" );
	if ( !strcmp( s, "localhost" ) ) {
		client->pers.localClient = qtrue;
	}

	// set name
	Q_strncpyz ( oldname, client->pers.netname, sizeof( oldname ) );
	s = Info_ValueForKey (userinfo, "name");
	Q_strncpyz( client->pers.netname, s, sizeof(client->pers.netname) );

	if ( client->pers.connected == CON_CONNECTED ) {
		if ( strcmp( oldname, client->pers.netname ) ) {
			gi.SendServerCommand( -1, "print \"%s renamed to %s\n\"", oldname, 
				client->pers.netname );
		}
	}

	// set max health
	client->pers.maxHealth = atoi( Info_ValueForKey( userinfo, "handicap" ) );
	if ( client->pers.maxHealth < 1 || client->pers.maxHealth > 200 ) {
		client->pers.maxHealth = 200;
	}
extern	cvar_t	*g_spskill;
	if (g_spskill->integer && client->pers.maxHealth > 100) {
		client->pers.maxHealth = 100;	//no cheating here, only allow "easy" in normal skill
	}
	client->ps.stats[STAT_MAX_HEALTH] = client->pers.maxHealth;

	// sounds
	Q_strncpyz( sound, Info_ValueForKey (userinfo, "snd"), sizeof( sound ) );


	// set model
	Q_strncpyz( headModel, Info_ValueForKey (userinfo, "headModel"), sizeof( headModel ) );
	Q_strncpyz( torsoModel, Info_ValueForKey (userinfo, "torsoModel"), sizeof( torsoModel ) );
	Q_strncpyz( legsModel, Info_ValueForKey (userinfo, "legsModel"), sizeof( legsModel ) );

	// sex
	sex = Info_ValueForKey( userinfo, "sex" );
	if ( !sex[0] ) {
		sex = "m";
	}

	// send over a subset of the userinfo keys so other clients can
	// print scoreboards, display models, and play custom sounds
	s = va("n\\%s\\t\\%i\\headModel\\%s\\torsoModel\\%s\\legsModel\\%s\\sex\\%s\\hc\\%i",
		client->pers.netname, client->sess.sessionTeam, headModel, torsoModel, legsModel, sex,
		client->pers.maxHealth );

	gi.SetConfigstring( CS_PLAYERS+clientNum, s );
}


/*
===========
ClientConnect

Called when a player begins connecting to the server.
Called again for every map change or tournement restart.

The session information will be valid after exit.

Return NULL if the client should be allowed, otherwise return
a string with the reason for denial.

Otherwise, the client will be sent the current gamestate
and will eventually get to ClientBegin.

firstTime will be qtrue the very first time a client connects
to the server machine, but qfalse on map changes and tournement
restarts.
============
*/
char *ClientConnect( int clientNum, qboolean firstTime, SavedGameJustLoaded_e eSavedGameJustLoaded ) 
{
	gclient_t	*client;
	char		userinfo[MAX_INFO_STRING];
	gentity_t	*ent;
	clientSession_t		savedSess;
	clientTourSession_t		savedTourSess;

	ent = &g_entities[ clientNum ];
	gi.GetUserinfo( clientNum, userinfo, sizeof( userinfo ) );


	// they can connect
	ent->client = level.clients + clientNum;
	client = ent->client;

//	if (!qbFromSavedGame)
	if (eSavedGameJustLoaded != eFULL)
	{
		savedSess = client->sess;	// 
		savedTourSess = client->tourSess;	
		memset( client, 0, sizeof(*client) );
		client->sess = savedSess; 
		client->tourSess = savedTourSess; 

	}

	client->pers.connected = CON_CONNECTING;

	if (eSavedGameJustLoaded == eFULL)//qbFromSavedGame)
	{
		// G_WriteClientSessionData( client ); // forget it, this is DM stuff anyway
		// get and distribute relevent paramters
		ClientUserinfoChanged( clientNum );
	}
	else
	{
		// read or initialize the session data
		if ( firstTime ) {
			G_InitSessionData( client, userinfo );
		}
		G_ReadSessionData( client );

		// get and distribute relevent paramters
		ClientUserinfoChanged( clientNum );

		// don't do the "xxx connected" messages if they were caried over from previous level
		if ( firstTime ) {
			gi.SendServerCommand( -1, "print \"%s connected\n\"", client->pers.netname);
		}
	}

	return NULL;
}

/*
===========
ClientBegin

called when a client has finished connecting, and is ready
to be placed into the level.  This will happen every level load,
and on transition between teams, but doesn't happen on respawns
============
*/
void ClientBegin( int clientNum, usercmd_t *cmd, SavedGameJustLoaded_e eSavedGameJustLoaded)		
//												qboolean qbFromSavedGame 
{
	gentity_t	*ent;
	gclient_t	*client;

	ent = g_entities + clientNum;
	client = level.clients + clientNum;

	if (eSavedGameJustLoaded == eFULL)//qbFromSavedGame)
	{
		client->pers.connected = CON_CONNECTED;
		ent->client = client;
		ClientSpawn( ent, eSavedGameJustLoaded );
	}
	else
	{
		if ( ent->linked ) {
			gi.unlinkentity( ent );
		}
		G_InitGentity( ent );
		ent->e_TouchFunc = touchF_NULL;
		ent->e_PainFunc  = painF_NULL;
		ent->client = client;

		client->pers.connected = CON_CONNECTED;
		client->pers.enterTime = level.time;
		client->pers.teamState.state = TEAM_BEGIN;
		VectorCopy( cmd->angles, client->pers.cmd_angles );

		memset( &client->ps, 0, sizeof( client->ps ) );

		// locate ent at a spawn point
		if ( ClientSpawn( ent, eSavedGameJustLoaded) )	// SavedGameJustLoaded_e
		{
			gentity_t	*tent;
			// send teleport event
			tent = G_TempEntity( ent->client->ps.origin, EV_PLAYER_TELEPORT_IN );
			tent->owner = ent;
			tent->s.clientNum = ent->s.clientNum;
		}
	}
}



/*
============
Player_CacheFromPrevLevel
  Description	: just need to grab the weapon items we're going to have when we spawn so they'll be cached
  Return type	: void 
  Argument		: void
============
*/
void Player_CacheFromPrevLevel(void)
{
	char	s[MAX_STRING_CHARS];
	
	gi.Cvar_VariableStringBuffer( sCVARNAME_PLAYERSAVE, s, sizeof(s) );
	
	if (strlen(s))	// actually this would be safe anyway because of the way sscanf() works, but this is clearer
	{
		int iDummy, bits;

		sscanf( s, "%i %i %i", 
			&iDummy, //client->ps.stats[STAT_HEALTH],
			&iDummy, //client->ps.stats[STAT_ARMOR],
			&bits //client->ps.stats[STAT_WEAPONS]
			);

		for ( int i = 1 ; i < 16 ; i++ ) 
		{
			if ( bits & ( 1 << i ) ) 
			{
				RegisterItem( FindItemForWeapon( (weapon_t)i ) );
			}
		}

	}
}

/*
============
Player_RestoreFromPrevLevel
  Description	: retrieve maptransition data recorded by server when exiting previous level (to carry over weapons/ammo/health/etc)
  Return type	: void 
  Argument		: gentity_t *ent
============
*/
void Player_RestoreFromPrevLevel(gentity_t *ent)
{
	gclient_t	*client = ent->client;
	int			i;

	assert(client);
	if (client)	// though I can't see it not being true...
	{
		char	s[MAX_STRING_CHARS];
						
		gi.Cvar_VariableStringBuffer( sCVARNAME_PLAYERSAVE, s, sizeof(s) );

		if (strlen(s))	// actually this would be safe anyway because of the way sscanf() works, but this is clearer
		{
			sscanf( s, "%i %i %i %i %i %f %f %f", 
								&client->ps.stats[STAT_HEALTH],
								&client->ps.stats[STAT_ARMOR],
								&client->ps.stats[STAT_WEAPONS],
								&client->ps.weapon,
								&client->ps.weaponstate,
								&client->ps.viewangles[0],
								&client->ps.viewangles[1],
								&client->ps.viewangles[2]
					);
			ent->health = client->ps.stats[STAT_HEALTH];

// slight issue with ths for the moment in that although it'll correctly restore angles it doesn't take into account
//	the overall map orientation, so (eg) exiting east to enter south will be out by 90 degrees, best keep spawn angles for now
//
//			VectorClear (ent->client->pers.cmd_angles);
//
//			SetClientViewAngle( ent, ent->client->ps.viewangles);

			for ( i = 0; i < AMMO_MAX; i++ )
			{
				gi.Cvar_VariableStringBuffer( va("playerammo%d",i), s, sizeof(s) );
				sscanf( s,"%i",&client->ps.ammo[i]);
			}

			// Get the borg adapt hits per weapon
			for ( i = 0; i < MAX_WEAPONS; i++ )
			{
				gi.Cvar_VariableStringBuffer( va("borgadapt%d",i), s, sizeof(s) );
				sscanf( s,"%i",&client->ps.borgAdaptHits[i]);
			}
		}
	}
}

/*
============
Player_CacheForNextLevel
  Description	: record the player's loadout (health/armor/weapons/ammo/borg-adapt/viewangles) into the
				engine carry-cvars so the NEXT level's Player_RestoreFromPrevLevel can carry it across a
				forward map transition. In retail this was done by the ENGINE (stvoy SV save-for-transition,
				retail); this Android port replaced the engine with a bridge, and the bridge spawns the
				new map FRESH (it does NOT restore the old map's entity table — that would drag the old map's
				inline-model *N indices into the new BSP -> CM_InlineModel crash). So the game records the
				carry here; the cvars live in the engine module and survive the per-map libefgame reload.
				Field order MUST match Player_RestoreFromPrevLevel's sscanf.
============
*/
void Player_CacheForNextLevel(void)
{
	gentity_t	*ent = &g_entities[0];

	if (!ent->client)
		return;

	gclient_t	*client = ent->client;

	gi.cvar_set( sCVARNAME_PLAYERSAVE, va("%i %i %i %i %i %f %f %f",
		client->ps.stats[STAT_HEALTH],
		client->ps.stats[STAT_ARMOR],
		client->ps.stats[STAT_WEAPONS],
		client->ps.weapon,
		client->ps.weaponstate,
		client->ps.viewangles[0],
		client->ps.viewangles[1],
		client->ps.viewangles[2] ) );

	for ( int i = 0; i < AMMO_MAX; i++ )
	{
		char v[32];
		Com_sprintf( v, sizeof(v), "%i", client->ps.ammo[i] );
		gi.cvar_set( va("playerammo%d", i), v );
	}

	for ( int i = 0; i < MAX_WEAPONS; i++ )
	{
		char v[32];
		Com_sprintf( v, sizeof(v), "%i", client->ps.borgAdaptHits[i] );
		gi.cvar_set( va("borgadapt%d", i), v );
	}
}

/*
===========
ClientSpawn

Called every time a client is placed fresh in the world:
after the first ClientBegin, and after each respawn
Initializes all non-persistant parts of playerState
============
*/
qboolean ClientSpawn(gentity_t *ent, SavedGameJustLoaded_e eSavedGameJustLoaded ) 
{
	int		index;
	vec3_t	spawn_origin, spawn_angles;
	gclient_t	*client;
	int		i;
	clientPersistant_t	saved;
	clientSession_t		savedSess;
	clientTourSession_t	savedTourSess;
	clientInfo_t		savedCi;
	int		persistant[MAX_PERSISTANT];
	usercmd_t	ucmd;
	gentity_t	*spawnPoint;
	qboolean	beamInEffect = qfalse;
	extern qboolean g_qbLoadTransition;

	index = ent - g_entities;
	client = ent->client;

	if ( eSavedGameJustLoaded == eFULL && g_qbLoadTransition == qfalse )//qbFromSavedGame)
	{
		ent->client->pers.teamState.state = TEAM_ACTIVE;

		// increment the spawncount so the client will detect the respawn
		client->ps.persistant[PERS_SPAWN_COUNT]++;
		client->ps.persistant[PERS_TEAM] = client->sess.sessionTeam;

		client->airOutTime = level.time + 12000;

		for (i=0; i<3; i++)
		{
			ent->client->pers.cmd_angles[i] = 0.0f;
		}

		SetClientViewAngle( ent, ent->client->ps.viewangles);//spawn_angles );

		gi.linkentity (ent);

		// run the presend to set anything else
		ClientEndFrame( ent );

		// clear entity state values
		PlayerStateToEntityState( &client->ps, &ent->s );
	}
	else
	{
		// find a spawn point
		// do it before setting health back up, so farthest
		// ranging doesn't count this client
		// don't spawn near existing origin if possible
		spawnPoint = SelectSpawnPoint ( ent->client->ps.origin, 
			(team_t) ent->client->ps.persistant[PERS_TEAM], spawn_origin, spawn_angles);

		if ( !(spawnPoint->spawnflags & 32) )
		{//Do teleport effect
			beamInEffect = qtrue;
		}

		ent->client->pers.teamState.state = TEAM_ACTIVE;

		// clear everything but the persistant data
		saved = client->pers;
		savedSess = client->sess;
		savedTourSess = client->tourSess;
		for ( i = 0 ; i < MAX_PERSISTANT ; i++ ) 
		{
			persistant[i] = client->ps.persistant[i];
		}
		//Preserve clientInfo
		memcpy (&savedCi, &client->clientInfo, sizeof(clientInfo_t));
		
		memset (client, 0, sizeof(*client));

		memcpy (&client->clientInfo, &savedCi, sizeof(clientInfo_t));

		client->pers = saved;
		client->sess = savedSess;
		client->tourSess = savedTourSess;
		for ( i = 0 ; i < MAX_PERSISTANT ; i++ ) 
		{
			client->ps.persistant[i] = persistant[i];
		}

		// increment the spawncount so the client will detect the respawn
		client->ps.persistant[PERS_SPAWN_COUNT]++;
		client->ps.persistant[PERS_TEAM] = client->sess.sessionTeam;

		client->airOutTime = level.time + 12000;

		// clear entity values
		client->ps.stats[STAT_MAX_HEALTH] = client->pers.maxHealth;
		ent->s.groundEntityNum = -1;
		ent->client = &level.clients[index];
		ent->takedamage = qtrue;
		ent->inuse = qtrue;
		ent->classname = "player";
		client->squadname = "Munro";
		ent->targetname = "Munro";
		ent->script_targetname = "Munro";
		ent->NPC_type = "munro";
		client->playerTeam = TEAM_STARFLEET;
		ent->contents = CONTENTS_BODY;
		ent->clipmask = MASK_PLAYERSOLID;
		ent->e_DieFunc = dieF_player_die;
		ent->waterlevel = 0;
		ent->watertype = 0;
		client->ps.friction = 6;
		client->ps.gravity = g_gravity->value;
		ent->flags &= ~FL_NO_KNOCKBACK;
		client->renderInfo.scaleXYZ[0] = client->renderInfo.scaleXYZ[1] = client->renderInfo.scaleXYZ[2] = 100;
		client->renderInfo.lookTarget = ENTITYNUM_NONE;

		VectorCopy (playerMins, ent->mins);
		VectorCopy (playerMaxs, ent->maxs);
		client->crouchheight = CROUCH_MAXS_2;
		client->standheight = DEFAULT_MAXS_2;

		client->ps.clientNum = index;
		//give default weapons
		client->ps.stats[STAT_WEAPONS] = ( 1 << WP_NONE );
		client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_PHASER );	//these are precached in g_items, ClearRegisteredItems()
		client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_COMPRESSION_RIFLE );

		client->ps.ammo[AMMO_STARFLEET] = ammoData[AMMO_STARFLEET].max;
		client->ps.ammo[AMMO_ALIEN] = ammoData[AMMO_ALIEN].max;
		client->ps.ammo[AMMO_PHASER] = ammoData[AMMO_PHASER].max;

		ent->health = client->ps.stats[STAT_ARMOR] = client->ps.stats[STAT_HEALTH] = client->ps.stats[STAT_MAX_HEALTH];

		VectorCopy( spawn_origin, client->ps.origin );
		VectorCopy( spawn_origin, ent->currentOrigin );

		// the respawned flag will be cleared after the attack and jump keys come up
		client->ps.pm_flags |= PMF_RESPAWNED;

		SetClientViewAngle( ent, spawn_angles );

		{
			G_KillBox( ent );
			gi.linkentity (ent);
			// force the base weapon up
			client->ps.weapon = WP_COMPRESSION_RIFLE;
			client->ps.weaponstate = WEAPON_READY;
		}

		// don't allow full run speed for a bit
		client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
		client->ps.pm_time = 100;

		client->respawnTime = level.time;
		client->inactivityTime = level.time + g_inactivity->integer * 1000;
		client->latched_buttons = 0;

		// set default animations
		client->ps.torsoAnim = BOTH_STAND2;
		client->ps.legsAnim = BOTH_STAND2;

		// restore some player data if this is a spawn point with KEEP_REV (spawnflags&1) set...	
		//
		if ( eSavedGameJustLoaded == eAUTO ||
			(spawnPoint->spawnflags&1) ||		// KEEP_PREV
			g_qbLoadTransition == qtrue )
		{
			Player_RestoreFromPrevLevel(ent);
		}

		// run a client frame to drop exactly to the floor,
		// initialize animations and other things
		client->ps.commandTime = level.time - 100;
		ucmd = client->pers.lastCommand;
		ucmd.serverTime = level.time;
		VectorCopy( client->pers.cmd_angles, ucmd.angles );
		ClientThink( ent-g_entities, &ucmd );

		// run the presend to set anything else
		ClientEndFrame( ent );

		// clear entity state values
		PlayerStateToEntityState( &client->ps, &ent->s );

		//ICARUS include
		ICARUS_FreeEnt( ent );	//FIXME: This shouldn't need to be done...?
		ICARUS_InitEnt( ent );

		G_InitBoltOnData( ent );

		if ( spawnPoint->spawnflags & 64 )
		{//player starts with absolutely no weapons
			ent->client->ps.stats[STAT_WEAPONS] = ( 1 << WP_NONE );
			ent->client->ps.ammo[weaponData[WP_NONE].ammoIndex] = 32000;	// checkme	
			ent->client->ps.weapon = WP_NONE;
			ent->client->ps.weaponstate = WEAPON_READY;
		}

		{
			// fire the targets of the spawn point
			G_UseTargets( spawnPoint, ent );
			//Designers needed them to fire off target2's as well... this is kind of messy
			G_UseTargets2( spawnPoint, ent, spawnPoint->target2 );

			/*
			// select the highest weapon number available, after any
			// spawn given items have fired
			client->ps.weapon = 1;
			for ( i = WP_NUM_WEAPONS - 1 ; i > 0 ; i-- ) {
				if ( client->ps.stats[STAT_WEAPONS] & ( 1 << i ) ) {
					client->ps.weapon = i;
					break;
				}
			}*/
		}
	}
	ent->max_health = client->ps.stats[STAT_MAX_HEALTH];
	return beamInEffect;
}


/*
===========
ClientDisconnect

Called when a player drops from the server.
Will not be called between levels.
============
*/
void ClientDisconnect( int clientNum ) {
	gentity_t	*ent;

	ent = g_entities + clientNum;
	if ( !ent->client ) {
		return;
	}

	// send effect if they were completely connected
/*	if ( ent->client->pers.connected == CON_CONNECTED ) {
		gentity_t	*tent;
		tent = G_TempEntity( ent->client->ps.origin, EV_PLAYER_TELEPORT_OUT );
		tent->s.clientNum = ent->s.clientNum;

		// They don't get to take powerups with them!
		// Especially important for stuff like CTF flags
		TossClientItems ( ent );
	}
*/
	gi.unlinkentity (ent);
	ent->s.modelindex = 0;
	ent->inuse = qfalse;
	ent->classname = "disconnected";
	ent->client->pers.connected = CON_DISCONNECTED;
	ent->client->ps.persistant[PERS_TEAM] = TEAM_FREE;

	gi.SetConfigstring( CS_PLAYERS + clientNum, "");

}


