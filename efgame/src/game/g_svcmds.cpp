
#include "g_local.h"
#include "boltons.h"

extern int ICARUS_RunScript( gentity_t *ent, const char *name );
extern team_t TranslateTeamName( const char *name );
extern char	*TeamNames[TEAM_NUM_TEAMS];

/*
===================
Svcmd_EntityList_f
===================
*/
void	Svcmd_EntityList_f (void) {
	int			e;
	gentity_t		*check;

	check = g_entities+1;
	for (e = 1; e < globals.num_entities ; e++, check++) {
		if ( !check->inuse ) {
			continue;
		}
		gi.Printf("%3i:", e);
		switch ( check->s.eType ) {
		case ET_GENERAL:
			gi.Printf("ET_GENERAL ");
			break;
		case ET_PLAYER:
			gi.Printf("ET_PLAYER  ");
			break;
		case ET_ITEM:
			gi.Printf("ET_ITEM    ");
			break;
		case ET_MISSILE:
			gi.Printf("ET_MISSILE ");
			break;
		case ET_MOVER:
			gi.Printf("ET_MOVER   ");
			break;
		case ET_BEAM:
			gi.Printf("ET_BEAM    ");
			break;
		default:
			gi.Printf("#%i", check->s.eType);
			break;
		}

		if ( check->classname ) {
			gi.Printf("%s", check->classname);
		}
		gi.Printf("\n");
	}
}

gclient_t	*ClientForString( const char *s ) {
	gclient_t	*cl;
	int			i;
	int			idnum;

	// numeric values are just slot numbers
	if ( s[0] >= '0' && s[0] <= '9' ) {
		idnum = atoi( s );
		if ( idnum < 0 || idnum >= level.maxclients ) {
			Com_Printf( "Bad client slot: %i\n", idnum );
			return NULL;
		}

		cl = &level.clients[idnum];
		if ( cl->pers.connected == CON_DISCONNECTED ) {
			gi.Printf( "Client %i is not connected\n", idnum );
			return NULL;
		}
		return cl;
	}

	// check for a name match
	for ( i=0 ; i < level.maxclients ; i++ ) {
		cl = &level.clients[i];
		if ( cl->pers.connected == CON_DISCONNECTED ) {
			continue;
		}
		if ( !Q_stricmp( cl->pers.netname, s ) ) {
			return cl;
		}
	}

	gi.Printf( "User %s is not on the server\n", s );

	return NULL;
}

/*
=================
ConsoleCommand

=================
*/
qboolean	ConsoleCommand( void ) {
	char	*cmd;

	cmd = gi.argv(0);

	if ( Q_stricmp (cmd, "entitylist") == 0 ) {
		Svcmd_EntityList_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "game_memory") == 0) {
		Svcmd_GameMem_f();
		return qtrue;
	}

//	if (Q_stricmp (cmd, "addbot") == 0) {
//		Svcmd_AddBot_f();
//		return qtrue;
//	}

	if (Q_stricmp (cmd, "nav") == 0) {
		Svcmd_Nav_f ();
		return qtrue;
	}

	if (Q_stricmp (cmd, "npc") == 0) {
		Svcmd_NPC_f ();
		return qtrue;
	}

	if (Q_stricmp (cmd, "Command") == 0) {
		Svcmd_Comm_f ();
		return qtrue;
	}

	if (Q_stricmp (cmd, "Hail") == 0) {
		Svcmd_Hail_f ();
		return qtrue;
	}
	
	if (Q_stricmp (cmd, "makeFormation") == 0) {
		Svcmd_Form_f ();
		return qtrue;
	}

	if (Q_stricmp (cmd, "use") == 0) {
		Svcmd_Use_f ();
		return qtrue;
	}

	if (Q_stricmp (cmd, "detpack") == 0) {
		UseCharge (0);
		return qtrue;
	}

	if ( Q_stricmp( cmd, "ICARUS" ) == 0 )	{
		Svcmd_ICARUS_f();
		return qtrue;
	}


	if ( Q_stricmp( cmd, "boltOn" ) == 0 ) 
	{
		char	*cmd2 = gi.argv(1);
		char	*cmd3 = gi.argv(2);

		if ( cmd2 && cmd3 )
		{
			if ( Q_stricmp( "add", cmd2 ) == 0 )
			{
				g_entities[0].activeBoltOn = G_AddBoltOn( &g_entities[0], cmd3 );
				return qtrue;
			}
			else if ( Q_stricmp( "remove", cmd2 ) == 0 )
			{
				G_RemoveBoltOn( &g_entities[0], cmd3 );
				return qtrue;
			}
			else if ( Q_stricmp( "frame", cmd2 ) == 0 )
			{
				int endFrame = atoi(cmd3);

				Q3_SetActiveBoltOnStartFrame( 0, g_entities[0].client->renderInfo.boltOns[g_entities[0].activeBoltOn].frame );
				Q3_SetActiveBoltOnEndFrame( 0, endFrame );
				return qtrue;
			}
		}
		
		//Print list of valid commands
		gi.Printf( "boltOn commands:\n" );
		gi.Printf( "	add [boltOnName]\n" );
		gi.Printf( "	remove [boltOnName]\n" );
		gi.Printf( "	frame [frameNumber]\n" );

		return qtrue;
	}

	if ( Q_stricmp( cmd, "runscript" ) == 0 ) 
	{
		char *cmd2 = gi.argv(1);

		if ( cmd2 && cmd2[0] )
		{
			gentity_t *found = NULL;
			if ( (found = G_Find(NULL, FOFS(targetname), cmd2 ) ) != NULL )
			{
				char *cmd3 = gi.argv(2);
				if ( cmd3 && cmd3[0] )
				{
					ICARUS_RunScript( found, va( "%s/%s", Q3_SCRIPT_DIR, cmd3 ) );
				}
			}
			else
			{
				ICARUS_RunScript( &g_entities[0], va( "%s/%s", Q3_SCRIPT_DIR, cmd2 ) );
			}
		}
		else
		{
			gi.Printf( S_COLOR_RED"usage: runscript <ent targetname> scriptname\n" );
		}
		//FIXME: else warning
		return qtrue;
	}

	if ( Q_stricmp( cmd, "playerteam" ) == 0 ) 
	{
		char	*cmd2 = gi.argv(1);
		int		n;

		if ( !*cmd2 || !cmd2[0] )
		{
			gi.Printf( S_COLOR_RED"'playerteam' - change player team, requires a team name!\n" );
			gi.Printf( S_COLOR_RED"Valid team names are:\n");
			for ( n = (TEAM_FREE + 1); n < TEAM_NUM_TEAMS; n++ )
			{
				gi.Printf( S_COLOR_RED"%s\n", TeamNames[n] );
			}
		}
		else
		{
			team_t	team;

			team = TranslateTeamName( cmd2 );
			if ( team == TEAM_FREE )
			{
				gi.Printf( S_COLOR_RED"'playerteam' unrecognized team name %s!\n", cmd2 );
				gi.Printf( S_COLOR_RED"Valid team names are:\n");
				for ( n = (TEAM_FREE + 1); n < TEAM_NUM_TEAMS; n++ )
				{
					gi.Printf( S_COLOR_RED"%s\n", TeamNames[n] );
				}
			}
			else
			{
				g_entities[0].client->playerTeam = team;
				//FIXME: convert Imperial, Malon, Hirogen and Klingon to Scavenger?
			}
		}
		return qtrue;
	}
	
	return qfalse;
}

