#include "ui_local.h"

/*
===============================================================================

CONNECTION SCREEN

===============================================================================
*/

char		connectionDialogString[1024];
char		connectionMessageString[1024];
qboolean	passwordNeeded = qtrue;
menufield_s passwordField;

/*
========================
UI_DrawConnectText

This will also be overlaid on the cgame info screen during loading
to prevent it from blinking away too rapidly on local or lan games.
matches CG_DrawInformation from cg_info.cpp.
========================
*/
void UI_DrawConnectText( const char *servername, const char *updateInfoString ) {
	char		*s;
	connstate_t	connState;
//	char			info[MAX_INFO_VALUE];
	int			textX,textY;
	int padY;

	Menu_Cache();

	connState = ui.GetClientState();

//	ui.R_SetColor( colorTable[CT_DKPURPLE2]);

//	info[0] = '\0';
//	ui.GetConfigString( CS_SERVERINFO, info, sizeof(info) );
//	if( info[0] ) 
//	{
//		UI_DrawProportionalString( 320, 16, va( "%s %s",menu_normal_text[MNT_TRANSPORTING], Info_ValueForKey( info, "mapname" ) ), UI_SMALLFONT|UI_CENTER, colorTable[CT_LTGOLD1] );
//		UI_DrawProportionalString( 320, 16, va( "%s ...",menu_normal_text[MNT_TRANSPORTING] ), UI_SMALLFONT|UI_CENTER, colorTable[CT_LTGOLD1] );
//	}

//	UI_DrawString( 16, 10, va("Connecting to %s", servername), UI_LEFT|UI_BIGFONT, menu_text_color );
//	UI_DrawString( 16, 26, "Press Esc to abort", UI_LEFT|UI_BIGFONT, menu_text_color );

	// display global MOTD at bottom
	UI_DrawString( SCREEN_WIDTH/2, SCREEN_HEIGHT-20, Info_ValueForKey( updateInfoString, "motd" ), UI_CENTER|UI_BIGFONT, menu_text_color );

	// print any server info
	if ( connState < CA_CONNECTED ) {
		UI_DrawString( 16, 160, connectionMessageString, UI_LEFT|UI_BIGFONT, menu_text_color );
	}


	if ( connState == CA_CONNECTING ) {
		s = connectionDialogString;		// try number
	} else {
		s = "";
	}

	textX = 267;
	textY = 270;
	padY = 24;

	// Awaiting callenge
	UI_DrawProportionalString(  textX,  textY, va("%s...%s",menu_normal_text[MNT_TARGETINGSCANNERS], s),UI_SMALLFONT, colorTable[CT_LTGOLD1]);
	ui.R_SetColor( colorTable[CT_LTGOLD1]);	
	UI_DrawHandlePic(textX - 24,textY + 1, 16, 16, uis.graphicEmptyCircle2);

	if ( connState == CA_CONNECTING ) 
	{
		ui.R_SetColor( colorTable[CT_WHITE]);	
		return;
	}

	ui.R_SetColor( colorTable[CT_LTGOLD1]);	
	UI_DrawHandlePic(textX - 24,textY + 1, 16, 16, uis.graphicCircle2);

	if ( connState == CA_CHALLENGING ) {
		s = connectionDialogString;		// try number
	} else {
		s = "";
	}

	// Awaiting connection
	textY += padY;
	UI_DrawProportionalString(  textX,  textY, va("%s...%s",menu_normal_text[MNT_SCANNINGAREA], s),UI_SMALLFONT, colorTable[CT_LTGOLD1]);
	ui.R_SetColor( colorTable[CT_LTGOLD1]);	
	UI_DrawHandlePic(textX - 24,textY + 1, 16, 16, uis.graphicEmptyCircle2);
	if ( connState == CA_CHALLENGING ) 
	{
		ui.R_SetColor( colorTable[CT_WHITE]);	
		return;
	}
	ui.R_SetColor( colorTable[CT_LTGOLD1]);	
	UI_DrawHandlePic(textX - 24,textY + 1, 16, 16, uis.graphicCircle2);

	// Awaiting gamestate
	textY += padY;
	UI_DrawProportionalString(  textX,  textY, va("%s...",menu_normal_text[MNT_CONFIRMINGBEAMIN]),UI_SMALLFONT, colorTable[CT_LTGOLD1]);
	ui.R_SetColor( colorTable[CT_LTGOLD1]);	
	UI_DrawHandlePic(textX - 24,textY + 1, 16, 16, uis.graphicEmptyCircle2);
	if ( connState == CA_CONNECTED ) 
	{
		ui.R_SetColor( colorTable[CT_WHITE]);	
		return;
	}
	ui.R_SetColor( colorTable[CT_LTGOLD1]);	
	UI_DrawHandlePic(textX - 24,textY + 1, 16, 16, uis.graphicCircle2);

	// Loading
	textY += padY;
	UI_DrawProportionalString(  textX,  textY, va("%s...",menu_normal_text[MNT_LOCKINGPOSTION]),UI_SMALLFONT, colorTable[CT_LTGOLD1]);
	ui.R_SetColor( colorTable[CT_LTGOLD1]);	
	UI_DrawHandlePic(textX - 24,textY + 1, 16, 16, uis.graphicEmptyCircle2);
	if ( connState == CA_LOADING ) 
	{
		ui.R_SetColor( colorTable[CT_WHITE]);	
		return;
	}
	ui.R_SetColor( colorTable[CT_LTGOLD1]);	
	UI_DrawHandlePic(textX - 24,textY + 1, 16, 16, uis.graphicCircle2);

	// Awaiting snap shot
	textY += padY;
	UI_DrawProportionalString(  textX, textY, va("%s . . .",menu_normal_text[MNT_INITIATINGTRANSPORT]),UI_SMALLFONT, colorTable[CT_LTGOLD1]);
	ui.R_SetColor( colorTable[CT_LTGOLD1]);	
	UI_DrawHandlePic(textX - 24,textY + 1, 16, 16, uis.graphicEmptyCircle2);
	if ( connState == CA_PRIMED ) 
	{
		ui.R_SetColor( colorTable[CT_WHITE]);	
		return;
	}
	ui.R_SetColor( colorTable[CT_LTGOLD1]);	
	UI_DrawHandlePic(textX - 24,textY + 1, 16, 16, uis.graphicCircle2);

}

/*
========================
UI_DrawConnect

========================
*/
extern void UI_DrawThumbNail( float x, float y, float w, float h , byte *pic);	//ui_game

void UI_DrawConnect( const char *servername, const char *updateInfoString ) {
#if 0
	// if connecting to a local host, don't draw anything before the
	// gamestate message.  This allows cinematics to start seamlessly
	if ( connState < CA_LOADING && !strcmp( cls.servername, "localhost" ) ) {
		UI_SetColor( g_color_table[0] );
		re.DrawFill (0, 0, re.scrWidth, re.scrHeight);
		UI_SetColor( NULL );
		return;
	}
#endif

	qboolean qValid;
	byte *levelPic = ui.SCR_GetScreenshot(&qValid);
	// draw the dialog background
	if (!qValid) {
		UI_DrawHandlePic(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, uis.menuBackShader );
	} 
	else {
		UI_DrawThumbNail(0,SCREEN_HEIGHT, SCREEN_WIDTH, -SCREEN_HEIGHT, levelPic );
		// blend a detail texture over it
		qhandle_t detail = ui.R_RegisterShader( "levelShotDetail" );
		UI_DrawHandlePic( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, detail );
	}
	UI_DrawConnectText( servername, updateInfoString );
}


/*
========================
UI_UpdateConnectionString

========================
*/
void UI_UpdateConnectionString( char *string ) {
	Q_strncpyz( connectionDialogString, string, sizeof( connectionDialogString )  );
	UI_UpdateScreen();
}

/*
========================
UI_UpdateConnectionMessageString

========================
*/
void UI_UpdateConnectionMessageString( char *string ) {
	char		*s;

	Q_strncpyz( connectionMessageString, string, sizeof( connectionMessageString ) );

	// strip \n
	s = strstr( connectionMessageString, "\n" );
	if ( s ) {
		*s = 0;
	}
	UI_UpdateScreen();
}

/*
===================
UI_KeyConnect
===================
*/
void UI_KeyConnect( int key ) {
	if ( key == K_ESCAPE ) {
		ui.Cmd_ExecuteText( EXEC_APPEND, "disconnect\n" );
		return;
	}
}


