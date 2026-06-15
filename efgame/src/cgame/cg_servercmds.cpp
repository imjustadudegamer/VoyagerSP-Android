// cg_servercmds.c -- text commands sent by the server

#include "cg_local.h"
#include "cg_media.h"


/*
================
CG_ParseServerinfo

This is called explicitly when the gamestate is first received,
and whenever the server updates any serverinfo flagged cvars
================
*/
void CG_ParseServerinfo( void ) {
	const char	*info;
	char	*mapname;

	info = CG_ConfigString( CS_SERVERINFO );
	cgs.dmflags = atoi( Info_ValueForKey( info, "dmflags" ) );
	cgs.teamflags = atoi( Info_ValueForKey( info, "teamflags" ) );
	cgs.timelimit = atoi( Info_ValueForKey( info, "timelimit" ) );
	cgs.maxclients = 1;
	mapname = Info_ValueForKey( info, "mapname" );
	Com_sprintf( cgs.mapname, sizeof( cgs.mapname ), "maps/%s.bsp", mapname );
}


/*
================
CG_ConfigStringModified

================
*/
void CG_RegisterClientModels (int entityNum);
static void CG_ConfigStringModified( void ) {
	const char	*str;
	int		num;

	num = atoi( CG_Argv( 1 ) );

	// get the gamestate from the client system, which will have the
	// new configstring already integrated
	cgi_GetGameState( &cgs.gameState );

	// look up the individual string that was modified
	str = CG_ConfigString( num );

	// do something with it if necessary
	if ( num == CS_MUSIC ) {
		CG_StartMusic();
	} else if ( num == CS_SERVERINFO ) {
		CG_ParseServerinfo();
	} else if ( num >= CS_MODELS && num < CS_MODELS+MAX_MODELS ) {
		cgs.model_draw[ num-CS_MODELS ] = cgi_R_RegisterModel( str );
	} else if ( num >= CS_SOUNDS && num < CS_SOUNDS+MAX_MODELS ) {
		if ( str[0] != '*' ) {
			cgs.sound_precache[ num-CS_SOUNDS] = cgi_S_RegisterSound( str );
		}
	} 
	else if ( num >= CS_PLAYERS && num < CS_PLAYERS+MAX_CLIENTS ) {
		CG_NewClientinfo( num - CS_PLAYERS );
		CG_RegisterClientModels( num - CS_PLAYERS );
	}
}

/*
=================
CG_ServerCommand

The string has been tokenized and can be retrieved with
Cmd_Argc() / Cmd_Argv()
=================
*/
static void CG_ServerCommand( void ) {
	const char	*cmd;

	cmd = CG_Argv(0);

	if ( !strcmp( cmd, "cp" ) ) {
		CG_CenterPrint( CG_Argv(1), SCREEN_HEIGHT * 0.25, BIGCHAR_WIDTH );
		return;
	}

	if ( !strcmp( cmd, "cs" ) ) {
		CG_ConfigStringModified();
		return;
	}

	if ( !strcmp( cmd, "print" ) ) {
		CG_Printf( "%s", CG_Argv(1) );
		return;
	}

	if ( !strcmp( cmd, "chat" ) ) {
		cgi_S_StartLocalSound ( cgs.media.talkSound, CHAN_LOCAL_SOUND );
		CG_Printf( "%s\n", CG_Argv(1) );
		return;
	}


	// Scroll text
	if ( !strcmp( cmd, "st" ) ) 
	{
		CG_ScrollText( CG_Argv(1), SCREEN_HEIGHT * 0.25, BIGCHAR_WIDTH );
		return;
	}

	// Cinematic text
	if ( !strcmp( cmd, "ct" ) )
	{
		// CG_Argv returns ONE shared static buffer, so reading CG_Argv(2) here would clobber the
		// CG_Argv(1) text key before CG_CaptionText reads it (-> "invalid text key" = the sound
		// handle number). Capture the key first.
		char ctKey[MAX_QPATH];
		Q_strncpyz( ctKey, CG_Argv(1), sizeof(ctKey) );
		CG_CaptionText( ctKey, cgs.sound_precache[atoi(CG_Argv(2))], SCREEN_HEIGHT * 0.25, SMALLCHAR_WIDTH );
		return;
	}

	// Text stop
	if ( !strcmp( cmd, "cts" ) ) 
	{
		CG_CaptionTextStop();
		return;
	}

	// Game text spoken by a character
	if ( !strcmp( cmd, "gt" ) ) 
	{
		CG_GameText(SCREEN_HEIGHT * 0.25, SMALLCHAR_WIDTH );
		return;
	}


	// Text to appear in center of screen with an LCARS frame around it. 
	if ( !strcmp( cmd, "lt" ) ) 
	{
		CG_LCARSText( CG_Argv(1), SCREEN_HEIGHT * 0.25, SMALLCHAR_WIDTH );
		return;
	}

	// clientLevelShot is sent before taking a special screenshot for
	// the menu system during development
	if ( !strcmp( cmd, "clientLevelShot" ) ) {
		cg.levelShot = qtrue;
		return;
	}

	if ( !strcmp( cmd, "vmsg" ) ) {
#if 0
		char snd[MAX_QPATH];

		Com_sprintf(snd, sizeof(snd), 
			"sound/teamplay/vmsg/%s.wav", CG_Argv(1) );
		cgi_S_StartSound (NULL, cg.snap->ps.clientNum, CHAN_AUTO, 
			cgi_S_RegisterSound (snd) );
#endif
		return;
	}

	CG_Printf( "Unknown client game command: %s\n", cmd );
}


/*
====================
CG_ExecuteNewServerCommands

Execute all of the server commands that were received along
with this this snapshot.
====================
*/
void CG_ExecuteNewServerCommands( int latestSequence ) {
	while ( cgs.serverCommandSequence < latestSequence ) {
		if ( cgi_GetServerCommand( ++cgs.serverCommandSequence ) ) {
			CG_ServerCommand();
		}
	}
}
