#include "g_local.h"
#include "objectives.h"


/*
=======================================================================

  SESSION DATA

Session data is the only data that stays persistant across level loads
and tournament restarts.
=======================================================================
*/

/*
================
G_WriteClientSessionData

Called on game shutdown
================
*/
void G_WriteClientSessionData( gclient_t *client ) {
	const char	*s;
	const char	*s2;
	const char	*var;
	int		i;

	s2 = "";
	// Throw all status info into a string
	for (i=0;i< MAX_OBJECTIVES; i++)
	{
		s2 = va("%s %i %i",
		s2, client->sess.mission_objectives[i].display,
		client->sess.mission_objectives[i].status);
	}

	for (i=0;i< MAX_TOUR_OBJECTIVES; i++)
	{
		s2 = va("%s %i %i",
		s2, client->tourSess.tour_objectives[i].display,
		client->tourSess.tour_objectives[i].status);
	}

	var = va( "sessionobj%i", client - level.clients );

	gi.cvar_set( var, s2 );

	s = va("%i", client->sess.sessionTeam );

	var = va( "session%i", client - level.clients );

	gi.cvar_set( var, s );
}

/*
================
G_ReadSessionData

Called on a reconnect
================
*/
void G_ReadSessionData( gclient_t *client ) {
	char	s[MAX_STRING_CHARS];
	const char	*var;
	int		i;

	var = va( "session%i", client - level.clients );
	gi.Cvar_VariableStringBuffer( var, s, sizeof(s) );

	sscanf( s, "%i", &client->sess.sessionTeam );

	var = va( "sessionobj%i", client - level.clients );
	gi.Cvar_VariableStringBuffer( var, s, sizeof(s) );

	var = s;
	var++;
	for (i=0;i< MAX_OBJECTIVES; i++)
	{
		sscanf( var, "%i %i", 
			&client->sess.mission_objectives[i].display,
			&client->sess.mission_objectives[i].status);
			var+=4;
	}
	
	for (i=0;i< MAX_TOUR_OBJECTIVES; i++)
	{
		sscanf( var, "%i %i", 
			&client->tourSess.tour_objectives[i].display,
			&client->tourSess.tour_objectives[i].status);
			var+=4;
	}
}


/*
================
G_InitSessionData

Called on a first-time connect
================
*/
void G_InitSessionData( gclient_t *client, char *userinfo ) {
	clientSession_t	*sess;

	sess = &client->sess;

	sess->sessionTeam = TEAM_FREE;

	G_WriteClientSessionData( client );
}


/*
==================
G_InitWorldSession

==================
*/
void G_InitWorldSession( void ) {
}

/*
==================
G_WriteSessionData

==================
*/
void G_WriteSessionData( void ) {
	int		i;

	gi.cvar_set( "session", 0) ;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( level.clients[i].pers.connected == CON_CONNECTED ) {
			G_WriteClientSessionData( &level.clients[i] );
		}
	}
}
