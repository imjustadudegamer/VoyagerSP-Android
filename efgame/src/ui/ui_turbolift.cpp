/**********************************************************************
	UI_TURBOLIFT.C

	User interface trigger from within game
**********************************************************************/
#include "ui_local.h"

#define	MENUTYPE_LIBRARY		0
#define	MENUTYPE_ASTROMETRICS	1
#define	MENUTYPE_PERSONALLOG	2
#define	MENUTYPE_MEDICALLOG		3
#define	MENUTYPE_RECIPES		4
#define	MENUTYPE_SOCIALCALENDAR	5
#define	MENUTYPE_DISEASELIBRARY	6
#define	MENUTYPE_SHOOTINGRANGE	7
#define	MENUTYPE_WEAPONLIBRARY	8
#define	MENUTYPE_CARGO			9
#define MENUTYPE_ENGINEERINGLIBRARY	10

qboolean	inHolodeck;

static void UI_LibraryDrawMD3Model(qhandle_t modelHandle,int x, int y,int modelDistance,int modelYaw,int modelPitch,int modelRoll,int modelOriginY);

// Data for Closing Credits Menu
static struct 
{
	menuframework_s menu;
	int				maxDecks;
	int				chosenDeck;
	sfxHandle_t		openingVoice;
	menubitmap_s	quitmenu;
	menubitmap_s	engage;
	menubitmap_s	deck1;
	menubitmap_s	deck2;
	menubitmap_s	deck3;
	menubitmap_s	deck4;
	menubitmap_s	deck5;
	menubitmap_s	deck6;
	menubitmap_s	deck7;
	menubitmap_s	deck8;
	menubitmap_s	deck9;
	menubitmap_s	deck10;
	menubitmap_s	deck11;
	menubitmap_s	deck12;
	menubitmap_s	deck13;
	menubitmap_s	deck14;
	menubitmap_s	deck15;
	menubitmap_s	deck16;
} s_turbolift;

#define ID_ARROW1UP			2
#define ID_ARROW1DOWN		3
#define ID_ARROW2UP			4
#define ID_ARROW2DOWN		5
#define ID_COMPUTERVOICE	6

#define ID_QUIT		10
#define ID_DECK1	11
#define ID_DECK2	12
#define ID_DECK3	13
#define ID_DECK4	14
#define ID_DECK5	15
#define ID_DECK6	16
#define ID_DECK7	17
#define ID_DECK8	18
#define ID_DECK9	19
#define ID_DECK10	20
#define ID_DECK11	21
#define ID_DECK12	22
#define ID_DECK13	23
#define ID_DECK14	24
#define ID_DECK15	25
#define ID_DECK16	26
#define ID_ENGAGE	100

#define	MAX_TOKEN	1024
#define	MAX_DECKS	16

static char deckDesc[MAX_DECKS][MAX_TOKEN];
static char deckDesc2[MAX_DECKS][MAX_TOKEN];
static char deckCommand[MAX_DECKS][MAX_TOKEN];
static sfxHandle_t deckSoundHandle[MAX_DECKS];

void UI_TurboliftMenu_Cache (void);
void UI_HolodeckMenu_Cache (void);

/*
=================
Turbolift_StatusBar
=================
*/
static void Turbolift_StatusBar(void *itemptr) 
{
	int		id;

	id = ((menucommon_s*)itemptr)->id;

	switch (id)
	{
		case ID_DECK1:
		case ID_DECK2:
		case ID_DECK3:
		case ID_DECK4:
		case ID_DECK5:
		case ID_DECK6:
		case ID_DECK7:
		case ID_DECK8:
		case ID_DECK9:
		case ID_DECK10:
		case ID_DECK11:
		case ID_DECK12:
		case ID_DECK13:
		case ID_DECK14:
		case ID_DECK15:
		case ID_DECK16:

			if (deckDesc2[id-ID_DECK1][0])
			{
				UI_DrawProportionalString( 320, 396, deckDesc[id-ID_DECK1], UI_CENTER|UI_SMALLFONT, colorTable[CT_WHITE]);
				UI_DrawProportionalString( 320, 416, deckDesc2[id-ID_DECK1], UI_CENTER|UI_SMALLFONT, colorTable[CT_WHITE]);
			}
			else
			{
				UI_DrawProportionalString( 320, 406, deckDesc[id-ID_DECK1], UI_CENTER|UI_SMALLFONT, colorTable[CT_WHITE]);
			}
			break;

		case ID_QUIT:
		case ID_ENGAGE:	
			UI_DrawProportionalString( 320, 406, menu_normal_text[MNT_SPECFICYDECK], UI_CENTER|UI_SMALLFONT, colorTable[CT_WHITE]);
			break;
	}
}


/*
=================
M_Turbolift_Event
=================
*/
void M_Turbolift_Event (void* ptr, int notification)
{
	int	id;
	menubitmap_s	*holdDeck;

	if (notification != QM_ACTIVATED)
	{
		return;
	}

	id = ((menucommon_s*)ptr)->id;

	switch (id)
	{
		case ID_QUIT:
			UI_PopMenu();
			break;
		case ID_DECK1:
		case ID_DECK2:
		case ID_DECK3:
		case ID_DECK4:
		case ID_DECK5:
		case ID_DECK6:
		case ID_DECK7:
		case ID_DECK8:
		case ID_DECK9:
		case ID_DECK10:
		case ID_DECK11:
		case ID_DECK12:
		case ID_DECK13:
		case ID_DECK14:
		case ID_DECK15:
		case ID_DECK16:

			if (s_turbolift.chosenDeck >= 0)
			{
				holdDeck = &s_turbolift.deck1;
				holdDeck += s_turbolift.chosenDeck;
				holdDeck->textcolor	= CT_BLACK;
			}

			s_turbolift.chosenDeck	= id - ID_DECK1;
			s_turbolift.engage.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;

			holdDeck = &s_turbolift.deck1;
			holdDeck += s_turbolift.chosenDeck;
			holdDeck->textcolor	= CT_LTGOLD1;

			ui.S_StartLocalSound( deckSoundHandle[s_turbolift.chosenDeck], CHAN_MENU1 );

			break;
		case ID_ENGAGE:		// Active only if a deck has been chosen
			if (deckCommand[s_turbolift.chosenDeck])
			{
				UI_ForceMenuOff ();
				ui.Cmd_ExecuteText( EXEC_APPEND, deckCommand[s_turbolift.chosenDeck]);
			}
			break;
	}
}

#define MAXTURBOLIFTTEXT 10000
static char	TurboliftText[MAXTURBOLIFTTEXT];

/*
=================
UI_ParseButtonText
=================
*/
static void UI_ParseTurboliftText()
{
	char	*token;
	char *buffer;
	int i,deckNum;
	char	soundFile[1024];

	memset(deckDesc,0,sizeof(deckDesc));
	memset(deckDesc2,0,sizeof(deckDesc2));
	memset(deckCommand,0,sizeof(deckCommand));
	memset(deckSoundHandle,0,sizeof(deckSoundHandle));

	COM_BeginParseSession();

	buffer = TurboliftText;
	i = 1;	// Zero is null string
	while ( buffer ) 
	{

		token = COM_ParseExt( &buffer, qtrue );

		if (!Q_strncmp(token,"DECK",4))
		{
			deckNum = atoi(&token[4]);

			// Get description
			token = COM_ParseExt( &buffer, qfalse );
			Q_strncpyz( deckDesc[deckNum-1], token, sizeof(deckDesc[0]) );

			// Get description
			token = COM_ParseExt( &buffer, qfalse );
			Q_strncpyz( deckDesc2[deckNum-1], token, sizeof(deckDesc2[0]) );

			// Get command
			token = COM_ParseExt( &buffer, qfalse );
			Q_strncpyz( deckCommand[deckNum-1], token, sizeof(deckCommand[0]) );

			// Get sound
			token = COM_ParseExt( &buffer, qfalse );
			Q_strncpyz( soundFile, token, sizeof(soundFile) );
			deckSoundHandle[deckNum-1] = ui.S_RegisterSound( soundFile );			


		}
	}
}

/*
=================
TurboliftMenu_LoadText
=================
*/
void TurboliftMenu_LoadText (void)
{
	UI_TurboliftMenu_Cache();

}

/*
=================
TurboliftMenu_Key
=================
*/
sfxHandle_t TurboliftMenu_Key (int key)
{
	return ( Menu_DefaultKey( &s_turbolift.menu, key ) );
}

qhandle_t			leftRound;
qhandle_t			corner_ul_24_60;
qhandle_t			corner_ll_12_60;
qhandle_t			turbolift;

/*
=================
M_TurboliftMenu_Graphics
=================
*/
void M_TurboliftMenu_Graphics (void)
{
	menubitmap_s	*holdDeck;
	int		i,length,xTurboStart;
	int		numColor,roundColor;

	// Draw the basic screen frame

	// Upper corners
	ui.R_SetColor( colorTable[CT_LTPINK]);
	UI_DrawHandlePic( 20,  24,  64,  32, corner_ul_24_60);		// Upper corner
	UI_DrawHandlePic( 20, 353,  64,  16, corner_ll_12_60);		// Lower Corner
	ui.R_SetColor( colorTable[CT_LTBLUE1]);
	UI_DrawHandlePic( 100, 86, 128, 128, turbolift);			// Turbolift graphic

	// Lower corners
	ui.R_SetColor( colorTable[CT_VDKPURPLE2]);
	UI_DrawHandlePic( 20, 375,  64, -16, corner_ll_12_60);		// 
	UI_DrawHandlePic( 20, 440,  64,  16, corner_ll_12_60);		// 

	xTurboStart = 604;
	length = UI_ProportionalStringWidth( menu_normal_text[MNT_TURBOLIFT],UI_BIGFONT);
	length += 4;

	// Upper half
	ui.R_SetColor( colorTable[CT_LTPINK]);
	UI_DrawHandlePic( 79,  24, xTurboStart - (79 + length),  PROP_BIG_HEIGHT, uis.whiteShader);	// Top left line
	UI_DrawHandlePic( 20,  60,  60,  40, uis.whiteShader);		// 
	ui.R_SetColor( colorTable[CT_LTPURPLE1]);
	UI_DrawHandlePic( 20, 106,  60,  11, uis.whiteShader);		// 
	ui.R_SetColor( colorTable[CT_LTPINK]);
	UI_DrawHandlePic( 20, 123,  60, 240, uis.whiteShader);		//  Left hand column
	UI_DrawHandlePic( 69, 356, 245,  12, uis.whiteShader);		// 

	ui.R_SetColor( colorTable[CT_DKPURPLE1]);
	UI_DrawHandlePic(319, 356, 93,   5, uis.whiteShader);		// 
	UI_DrawHandlePic(319, 360, 31,   8, uis.whiteShader);		// 
	UI_DrawHandlePic(381, 360, 31,   8, uis.whiteShader);		// 

	UI_DrawHandlePic(417, 356, 93,   12, uis.whiteShader);		// 

	UI_DrawHandlePic(510, 356,114,   12, uis.whiteShader);		// 

	// Lower half
	ui.R_SetColor( colorTable[CT_VDKPURPLE2]);
	UI_DrawHandlePic( 20, 380,  60,  70, uis.whiteShader);		// Left Column

	ui.R_SetColor( colorTable[CT_VDKPURPLE2]);
	UI_DrawHandlePic( 69, 376, 158,  12, uis.whiteShader);		// Top line
	UI_DrawHandlePic(232, 376,  82,  12, uis.whiteShader);		// Top line
	UI_DrawHandlePic(417, 376,  12,  12, uis.whiteShader);		// 
	UI_DrawHandlePic(434, 376, 190,  12, uis.whiteShader);		// 

	// Funky indent
	ui.R_SetColor( colorTable[CT_LTPINK]);
	UI_DrawHandlePic(319, 383,  93,   5, uis.whiteShader);		// 
	UI_DrawHandlePic(319, 376,  31,   8, uis.whiteShader);		// 
	UI_DrawHandlePic(381, 376,  31,   8, uis.whiteShader);		// 

	// Bottom line
	ui.R_SetColor( colorTable[CT_VDKPURPLE2]);
	UI_DrawHandlePic( 69, 443, 287,   12, uis.whiteShader);		// 
	ui.R_SetColor( colorTable[CT_LTPINK]);
	UI_DrawHandlePic(364, 443, 260,   12, uis.whiteShader);		// Bottom line


	UI_DrawProportionalString( xTurboStart, 24,
		menu_normal_text[MNT_TURBOLIFT],   UI_BIGFONT | UI_RIGHT, colorTable[CT_WHITE]);		

	ui.R_SetColor( colorTable[CT_LTPINK]);
	UI_DrawHandlePic( 607,  24,-16,   32, leftRound);
		
	holdDeck = &s_turbolift.deck1;

	// Print deck buttons
	for (i=0;i<s_turbolift.maxDecks;i++)
	{
		if (deckDesc[i][0])
		{
			if (s_turbolift.chosenDeck == i)		// This deck was chosen
			{
				numColor = CT_LTGOLD1;
				roundColor = CT_LTGOLD1;
			}
			else								// This deck was not chosen
			{
				numColor = CT_WHITE;
				roundColor = CT_DKAQUA;
			}

			UI_DrawProportionalString( holdDeck->generic.x - 6,
				holdDeck->generic.y, 
				va("%d",i+1),   UI_BIGFONT|UI_RIGHT, colorTable[numColor]);		

			ui.R_SetColor( colorTable[roundColor]);
			UI_DrawHandlePic( holdDeck->generic.x - 45, 
				holdDeck->generic.y, 
				16,   32, leftRound);
		}

		holdDeck++;
	}

	// Round graphic on left of engage & quit button
	ui.R_SetColor( colorTable[s_turbolift.quitmenu.color]);
	UI_DrawHandlePic(s_turbolift.engage.generic.x - 14,
		s_turbolift.engage.generic.y, 
		MENU_BUTTON_MED_HEIGHT, s_turbolift.engage.height, uis.graphicButtonLeftEnd);

	UI_DrawHandlePic(s_turbolift.quitmenu.generic.x - 14,
		s_turbolift.quitmenu.generic.y, 
		MENU_BUTTON_MED_HEIGHT, s_turbolift.quitmenu.height, uis.graphicButtonLeftEnd);

}

/*
===============
TurboliftMenu_Draw
===============
*/
void TurboliftMenu_Draw(void)
{
	// Draw graphics particular to Main Menu
	M_TurboliftMenu_Graphics();
	
	Menu_Draw( &s_turbolift.menu );


}

/*
===============
UI_TurboliftMenu_Cache
===============
*/
void UI_TurboliftMenu_Cache (void)
{
	char	*buffer;
	char	filename[MAX_QPATH];
	int		len;

	s_turbolift.openingVoice = ui.S_RegisterSound( "sound/voice/computer/tour/trblftmenu.mp3" );			

	leftRound = ui.R_RegisterShaderNoMip("menu/common/halfroundl_24.tga");
	corner_ul_24_60 = ui.R_RegisterShaderNoMip("menu/common/corner_ul_24_60.tga");
	corner_ll_12_60 = ui.R_RegisterShaderNoMip("menu/common/corner_ll_12_60.tga");
	turbolift = ui.R_RegisterShaderNoMip("menu/common/lift_button.tga");

	UI_LanguageFilename("ext_data/sp_turbolift","dat",filename);

	len = ui.FS_ReadFile( filename,(void **) &buffer  );

	if ( len == -1 ) 
	{
		ui.Error(ERR_FATAL, "UI_LoadButtonText : SP_TURBOLIFT.DAT file not found!\n");
		return;
	}

	if ( len > MAXTURBOLIFTTEXT ) 
	{
		ui.Error(ERR_FATAL, "UI_LoadButtonText : SP_TURBOLIFT.DAT too big!\n");
		return;
	}

	strncpy( TurboliftText, buffer, sizeof( TurboliftText ) - 1 );
	ui.FS_FreeFile( buffer );

	UI_ParseTurboliftText();

}

/*
===============
TurboliftMenu_Init
===============
*/
void TurboliftMenu_Init(void)
{
	int	y,pad,x;
	menubitmap_s	*holdDeck;
	int		i,width;

	s_turbolift.menu.nitems						= 0;
	s_turbolift.menu.draw						= TurboliftMenu_Draw;
	s_turbolift.menu.key						= TurboliftMenu_Key;
	s_turbolift.menu.fullscreen					= qtrue;
	s_turbolift.menu.wrapAround					= qtrue;
	s_turbolift.menu.descX						= MENU_DESC_X;
	s_turbolift.menu.descY						= MENU_DESC_Y;
	s_turbolift.menu.titleX						= MENU_TITLE_X;
	s_turbolift.menu.titleY						= MENU_TITLE_Y;
	s_turbolift.menu.titleI						= MNT_CREDITSMENU_TITLE;
	s_turbolift.menu.footNoteEnum				= MNT_CREDITS;

	s_turbolift.chosenDeck = -1;

	pad = PROP_BIG_HEIGHT + 10;
	y =  72;
	x = 319;
	width = MENU_BUTTON_MED_WIDTH-20;

	s_turbolift.maxDecks = MAX_DECKS;
	holdDeck = &s_turbolift.deck1;

	for (i=0;i<s_turbolift.maxDecks;i++)
	{
		holdDeck->generic.type				= MTYPE_BITMAP;      
		holdDeck->generic.flags				= QMF_HIGHLIGHT_IF_FOCUS;
		holdDeck->generic.x					= x;
		holdDeck->generic.y					= y;
		holdDeck->generic.name				= GRAPHIC_BUTTONRIGHT;
		holdDeck->generic.id				= ID_DECK1 + i;
		holdDeck->generic.callback			= M_Turbolift_Event; 
		holdDeck->generic.statusbarfunc		= Turbolift_StatusBar;
		holdDeck->width						= width;
		holdDeck->height					= PROP_BIG_HEIGHT;
		holdDeck->color						= CT_DKAQUA;
		holdDeck->color2					= CT_LTAQUA;
		holdDeck->textX						= MENU_BUTTON_TEXT_X;
		holdDeck->textY						= 12;
		holdDeck->textEnum					= MBT_DECK;
		holdDeck->textcolor					= CT_BLACK;
		holdDeck->textcolor2				= CT_WHITE;
		holdDeck->textStyle					= UI_TINYFONT;

		holdDeck++;
		y += pad;

		// Start the next column
		if (i == ((s_turbolift.maxDecks-1)/2))
		{
			x += width + 90; 
			y = 80;
		}
	}

	s_turbolift.engage.generic.type				= MTYPE_BITMAP;      
	s_turbolift.engage.generic.flags			= QMF_GRAYED;
	s_turbolift.engage.generic.x				= 110;
	s_turbolift.engage.generic.y				= 72 + (pad * 5);
	s_turbolift.engage.generic.name				= GRAPHIC_BUTTONRIGHT;
	s_turbolift.engage.generic.id				= ID_ENGAGE;
	s_turbolift.engage.generic.callback			= M_Turbolift_Event; 
	s_turbolift.engage.width					= width;
	s_turbolift.engage.height					= PROP_BIG_HEIGHT;
	s_turbolift.engage.color					= CT_DKORANGE;
	s_turbolift.engage.color2					= CT_LTORANGE;
	s_turbolift.engage.textX					= MENU_BUTTON_TEXT_X;
	s_turbolift.engage.textY					= 6;
	s_turbolift.engage.textEnum					= MBT_ENGAGE;
	s_turbolift.engage.textcolor				= CT_BLACK;
	s_turbolift.engage.textcolor2				= CT_WHITE;
	s_turbolift.engage.generic.statusbarfunc	= Turbolift_StatusBar;

	s_turbolift.quitmenu.generic.type			= MTYPE_BITMAP;      
	s_turbolift.quitmenu.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_turbolift.quitmenu.generic.x				= 110;
	s_turbolift.quitmenu.generic.y				= 72 + (pad * 7);
	s_turbolift.quitmenu.generic.name			= GRAPHIC_BUTTONRIGHT;
	s_turbolift.quitmenu.generic.id				= ID_QUIT;
	s_turbolift.quitmenu.generic.callback		= M_Turbolift_Event; 
	s_turbolift.quitmenu.width					= width;
	s_turbolift.quitmenu.height					= PROP_BIG_HEIGHT;
	s_turbolift.quitmenu.color					= CT_DKORANGE;
	s_turbolift.quitmenu.color2					= CT_LTORANGE;
	s_turbolift.quitmenu.textX					= MENU_BUTTON_TEXT_X;
	s_turbolift.quitmenu.textY					= 6;
	s_turbolift.quitmenu.textEnum				= MBT_RETURN;
	s_turbolift.quitmenu.textcolor				= CT_BLACK;
	s_turbolift.quitmenu.textcolor2				= CT_WHITE;
	s_turbolift.quitmenu.generic.statusbarfunc	= Turbolift_StatusBar;

	Menu_AddItem( &s_turbolift.menu,	&s_turbolift.engage );
	Menu_AddItem( &s_turbolift.menu,	&s_turbolift.quitmenu );

	holdDeck = &s_turbolift.deck1;
	for (i=0;i<s_turbolift.maxDecks;i++)
	{
		if (deckDesc[i][0])
		{
			Menu_AddItem( &s_turbolift.menu,	holdDeck );
		}
		holdDeck++;
	}

}

/*
===============
UI_TurboliftMenu
===============
*/
void UI_TurboliftMenu(void)
{
	uis.menusp = 0;

	ingameFlag = qtrue;	// true when in game menu is in use

	Mouse_Show();

	TurboliftMenu_LoadText();

	TurboliftMenu_Init(); 

	ui.S_StartLocalSound( s_turbolift.openingVoice, CHAN_MENU1 );

	UI_PushMenu( &s_turbolift.menu );

	Menu_AdjustCursor( &s_turbolift.menu, 1 );	
}


// Data for Closing Credits Menu

static struct 
{
	menuframework_s menu;
	int				menuType;
	qhandle_t		leftRound;
	qhandle_t		corner_ll_12_60;
	qhandle_t		corner_ll_12_30;
	qhandle_t		corner_ur_24_30;
	int				maxMaps;
	int				chosenMap;
	menubitmap_s	endholodeckprogram;
	menubitmap_s	quitmenu;
	menubitmap_s	engage;
	menubitmap_s	resume;
	menubitmap_s	supereasy;
	menubitmap_s	easy;
	menubitmap_s	medium;
	menubitmap_s	hard;
	menubitmap_s	map1;
	menubitmap_s	map2;
	menubitmap_s	map3;
	menubitmap_s	map4;
	menubitmap_s	map5;
	menubitmap_s	map6;
	menubitmap_s	map7;
	menubitmap_s	map8;
	menubitmap_s	map9;
	menubitmap_s	map10;
	menubitmap_s	map11;
	menubitmap_s	map12;
	menubitmap_s	map13;
	menubitmap_s	map14;
	menubitmap_s	map15;
	menubitmap_s	map16;
} s_holodeck;

#define ID_QUIT		10
#define ID_MAP1		11
#define ID_MAP2		12
#define ID_MAP3		13
#define ID_MAP4		14
#define ID_MAP5		15
#define ID_MAP6		16
#define ID_MAP7		17
#define ID_MAP8		18
#define ID_MAP9		19
#define ID_MAP10	20
#define ID_MAP11	21
#define ID_MAP12	22
#define ID_MAP13	23
#define ID_MAP14	24
#define ID_MAP15	25
#define ID_MAP16	26
#define	ID_RESUME	27

#define ID_SUPEREASY		40
#define ID_EASY				41
#define ID_MEDIUM			42
#define ID_HARD				43
#define	ID_LEAVEHOLODECK	44

#define	MAX_MAPS	16

static char mapDesc[MAX_MAPS][MAX_TOKEN];
static char mapDesc2[MAX_MAPS][MAX_TOKEN];
static char mapCommand[MAX_MAPS][MAX_TOKEN];
static char mapInHoloCommand[MAX_MAPS][MAX_TOKEN];

static char returnDesc[MAX_TOKEN];
static char returnDesc2[MAX_TOKEN];
static char returnCommand[MAX_TOKEN];

#define MAXHOLODECKTEXT 5000
static char	HolodeckText[MAXHOLODECKTEXT];

/*
=================
Holodeck_StatusBar
=================
*/
static void Holodeck_StatusBar(void *itemptr) 
{
	int		id;

	id = ((menucommon_s*)itemptr)->id;

	switch (id)
	{
		case ID_MAP1:
		case ID_MAP2:
		case ID_MAP3:
		case ID_MAP4:
		case ID_MAP5:
		case ID_MAP6:
		case ID_MAP7:
		case ID_MAP8:
		case ID_MAP9:
		case ID_MAP10:
		case ID_MAP11:
		case ID_MAP12:
		case ID_MAP13:
		case ID_MAP14:
		case ID_MAP15:
		case ID_MAP16:

			if (mapDesc2[id-ID_MAP1][0])
			{
				UI_DrawProportionalString( 320, 396, mapDesc[id-ID_MAP1], UI_CENTER|UI_SMALLFONT, colorTable[CT_WHITE]);
				UI_DrawProportionalString( 320, 416, mapDesc2[id-ID_MAP1], UI_CENTER|UI_SMALLFONT, colorTable[CT_WHITE]);
			}
			else
			{
				UI_DrawProportionalString( 320, 406, mapDesc[id-ID_MAP1], UI_CENTER|UI_SMALLFONT, colorTable[CT_WHITE]);
			}
			break;
	}
}

/*
=================
M_Holodeck_Event
=================
*/
void M_Holodeck_Event (void* ptr, int notification)
{
	int	id;
	menubitmap_s	*holdDeck;
	char			*command;

	if (notification != QM_ACTIVATED)
	{
		return;
	}

	id = ((menucommon_s*)ptr)->id;

	switch (id)
	{
		// Controls
	case ID_RESUME:	// Resume holodeckprogram while in menu
	case ID_QUIT:	// Return to holodeck while in menu while on holodeck (confusing, eh?)
			UI_PopMenu();
			break;
		case ID_MAP1:
		case ID_MAP2:
		case ID_MAP3:
		case ID_MAP4:
		case ID_MAP5:
		case ID_MAP6:
		case ID_MAP7:
		case ID_MAP8:
		case ID_MAP9:
		case ID_MAP10:
		case ID_MAP11:
		case ID_MAP12:
		case ID_MAP13:
		case ID_MAP14:
		case ID_MAP15:
		case ID_MAP16:
			if (s_holodeck.chosenMap >= 0)
			{
				holdDeck = &s_holodeck.map1;
				holdDeck += s_holodeck.chosenMap;
				holdDeck->textcolor	= CT_BLACK;
			}

			s_holodeck.chosenMap	= id - ID_MAP1;
			s_holodeck.engage.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;

			holdDeck = &s_holodeck.map1;
			holdDeck += s_holodeck.chosenMap;
			holdDeck->textcolor	= CT_LTGOLD1;

			break;

		case ID_ENGAGE:

			if (inHolodeck == qtrue)
			{
				command = mapInHoloCommand[s_holodeck.chosenMap];
			}
			else
			{
				command = mapCommand[s_holodeck.chosenMap];
			}

			if (*command)
			{
				UI_ForceMenuOff ();
				ui.Cmd_ExecuteText( EXEC_APPEND, va("%s\n",command));
			}
			break;

		case ID_SUPEREASY:
			s_holodeck.supereasy.textcolor				= CT_WHITE;
			s_holodeck.easy.textcolor					= CT_BLACK;
			s_holodeck.medium.textcolor					= CT_BLACK;
			s_holodeck.hard.textcolor					= CT_BLACK;

			ui.Cvar_SetValue( "g_spskill", 0 );
			ui.Cvar_Set( "handicap", "200" );

			s_holodeck.supereasy.generic.flags			|= QMF_BLINK;
			s_holodeck.easy.generic.flags				&= ~ QMF_BLINK;
			s_holodeck.medium.generic.flags				&= ~ QMF_BLINK;
			s_holodeck.hard.generic.flags				&= ~ QMF_BLINK;

			break;
		case ID_EASY:
			s_holodeck.supereasy.textcolor			= CT_BLACK;
			s_holodeck.easy.textcolor				= CT_WHITE;
			s_holodeck.medium.textcolor				= CT_BLACK;
			s_holodeck.hard.textcolor				= CT_BLACK;

			ui.Cvar_SetValue( "g_spskill", 0 );
			ui.Cvar_Set( "handicap", "100" );

			s_holodeck.supereasy.generic.flags	&= ~ QMF_BLINK;
			s_holodeck.easy.generic.flags		|= QMF_BLINK;
			s_holodeck.medium.generic.flags		&= ~ QMF_BLINK;
			s_holodeck.hard.generic.flags		&= ~ QMF_BLINK;

			break;
		case ID_MEDIUM:
			s_holodeck.supereasy.textcolor			= CT_BLACK;
			s_holodeck.easy.textcolor				= CT_BLACK;
			s_holodeck.medium.textcolor				= CT_WHITE;
			s_holodeck.hard.textcolor				= CT_BLACK;
			ui.Cvar_SetValue( "g_spskill", 1 );
			ui.Cvar_Set( "handicap", "100" );

			s_holodeck.supereasy.generic.flags	&= ~ QMF_BLINK;
			s_holodeck.easy.generic.flags		&= ~ QMF_BLINK;
			s_holodeck.medium.generic.flags		|= QMF_BLINK;
			s_holodeck.hard.generic.flags		&= ~ QMF_BLINK;
			break;
		case ID_HARD:
			s_holodeck.supereasy.textcolor			= CT_BLACK;
			s_holodeck.easy.textcolor				= CT_BLACK;
			s_holodeck.medium.textcolor				= CT_BLACK;
			s_holodeck.hard.textcolor				= CT_WHITE;

			ui.Cvar_SetValue( "g_spskill", 2 );
			ui.Cvar_Set( "handicap", "100" );

			s_holodeck.supereasy.generic.flags	&= ~ QMF_BLINK;
			s_holodeck.easy.generic.flags		&= ~ QMF_BLINK;
			s_holodeck.medium.generic.flags		&= ~ QMF_BLINK;
			s_holodeck.hard.generic.flags		|= QMF_BLINK;
			break;

		case ID_LEAVEHOLODECK:
			if (returnCommand)
			{
				UI_ForceMenuOff ();
				ui.Cmd_ExecuteText( EXEC_APPEND, returnCommand);
			}
			break;

	}
}

/*
=================
HolodeckMenu_Key
=================
*/
sfxHandle_t HolodeckMenu_Key (int key)
{
	if ( key == K_ESCAPE ) 
	{
		return(0);	
	}

	return ( Menu_DefaultKey( &s_holodeck.menu, key ) );
}

/*
=================
M_HolodeckMenu_Graphics
=================
*/
void M_HolodeckMenu_Graphics (void)
{
	menubitmap_s	*holdDeck;
	int		i,length,xHolodeckStart;
	int		numColor,roundColor;

	// Draw the basic screen frame


	xHolodeckStart = 40;
	length = UI_ProportionalStringWidth( menu_normal_text[MNT_HOLODECK],UI_BIGFONT);
	length += 4;

	ui.R_SetColor( colorTable[CT_LTBLUE1]);

	UI_DrawHandlePic( 20,  24, 16,   32, s_holodeck.leftRound);
	// Top corners
	UI_DrawHandlePic( 388,  24,  32,  32, s_holodeck.corner_ur_24_30);
	UI_DrawHandlePic( 390, 354,  32,  32, s_holodeck.corner_ll_12_30);

	// Bottom corners
	ui.R_SetColor( colorTable[CT_LTBLUE1]);
	UI_DrawHandlePic( 560, 375,  -64,  -16, s_holodeck.corner_ll_12_60);
	UI_DrawHandlePic( 560, 440,  -64,  16, s_holodeck.corner_ll_12_60);


	ui.R_SetColor( colorTable[CT_LTBLUE1]);
	UI_DrawHandlePic( xHolodeckStart + length,  24, 
		412 - (xHolodeckStart + length), 24, uis.whiteShader);		// 

	ui.R_SetColor( colorTable[CT_LTBROWN1]);
	UI_DrawHandlePic( 390,  72, 30, 262, uis.whiteShader);		// Center column
	ui.R_SetColor( colorTable[CT_LTBLUE1]);
	UI_DrawHandlePic( 390,  36, 30,  33, uis.whiteShader);		// 
	UI_DrawHandlePic( 390, 338, 30,  26, uis.whiteShader);		// 

	UI_DrawHandlePic( 403, 356, 221, 12, uis.whiteShader);		// 
	UI_DrawHandlePic(  21, 376, 204, 12, uis.whiteShader);		// 
	UI_DrawHandlePic( 230, 376, 384, 12, uis.whiteShader);		// 

	ui.R_SetColor( colorTable[CT_LTBROWN1]);
	UI_DrawHandlePic(  21, 443, 338, 12, uis.whiteShader);		// 

	ui.R_SetColor( colorTable[CT_LTBLUE1]);
	UI_DrawHandlePic( 564, 380,  60, 64, uis.whiteShader);		// 
	UI_DrawHandlePic( 364, 443, 254, 12, uis.whiteShader);		// 

	UI_DrawProportionalString( xHolodeckStart, 24,
		menu_normal_text[MNT_HOLODECK],   UI_BIGFONT, colorTable[CT_WHITE]);		
		
	UI_DrawProportionalString( 393,  75, "721",UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString( 393, 342, "929",UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString( 234, 378, "1021",UI_TINYFONT, colorTable[CT_BLACK]);

	holdDeck = &s_holodeck.map1;

	for (i=0;i<s_holodeck.maxMaps;i++)
	{
		if (mapDesc[i][0])
		{
			if (s_holodeck.chosenMap == i)		// This deck was chosen
			{
				numColor = CT_LTGOLD1;
				roundColor = CT_LTGOLD1;
			}
			else								// This deck was not chosen
			{
				numColor = CT_WHITE;
				roundColor = CT_DKAQUA;
			}

			UI_DrawProportionalString( holdDeck->generic.x - 6,
				holdDeck->generic.y, 
				va("%d",i+1),   UI_BIGFONT|UI_RIGHT, colorTable[numColor]);		

			ui.R_SetColor( colorTable[roundColor]);
			UI_DrawHandlePic( holdDeck->generic.x - 45, 
				holdDeck->generic.y, 
				16,   32, s_holodeck.leftRound);
		}
		holdDeck++;
	}

	// Round graphic on left of quit button
	ui.R_SetColor( colorTable[s_holodeck.quitmenu.color]);

	UI_DrawHandlePic(s_holodeck.engage.generic.x - 14,
		s_holodeck.engage.generic.y, 
		MENU_BUTTON_MED_HEIGHT, s_holodeck.engage.height, uis.graphicButtonLeftEnd);

	UI_DrawHandlePic(s_holodeck.quitmenu.generic.x - 14,
		s_holodeck.quitmenu.generic.y, 
		MENU_BUTTON_MED_HEIGHT, s_holodeck.quitmenu.height, uis.graphicButtonLeftEnd);


	if (!(s_holodeck.resume.generic.flags & QMF_HIDDEN) && (s_holodeck.menuType!=2))
	{
		UI_DrawHandlePic(s_holodeck.resume.generic.x - 14,
			s_holodeck.resume.generic.y, 
			MENU_BUTTON_MED_HEIGHT, s_holodeck.resume.height, 
			uis.graphicButtonLeftEnd);
	}

}

/*
===============
HolodeckMenu_Draw
===============
*/
void HolodeckMenu_Draw(void)
{
	M_HolodeckMenu_Graphics();
	
	Menu_Draw( &s_holodeck.menu );


}

/*
===============
HolodeckMenu_Init
===============
*/
void HolodeckMenu_Init(void)
{
	int	y,pad,x;
	menubitmap_s	*holdDeck;
	int		i,width;

	s_holodeck.menu.nitems						= 0;
	s_holodeck.menu.draw						= HolodeckMenu_Draw;
	s_holodeck.menu.key							= HolodeckMenu_Key;
	s_holodeck.menu.fullscreen					= qtrue;
	s_holodeck.menu.wrapAround					= qtrue;
	s_holodeck.menu.descX						= MENU_DESC_X;
	s_holodeck.menu.descY						= MENU_DESC_Y;
	s_holodeck.menu.titleX						= MENU_TITLE_X;
	s_holodeck.menu.titleY						= MENU_TITLE_Y;
	s_holodeck.menu.titleI						= MNT_CREDITSMENU_TITLE;
	s_holodeck.menu.footNoteEnum				= MNT_CREDITS;

	pad = PROP_BIG_HEIGHT + 10;
	y =  72;
	x = 65;
	width = MENU_BUTTON_MED_WIDTH-20;

	s_holodeck.chosenMap = -1;
	s_holodeck.maxMaps = MAX_MAPS;
	holdDeck = &s_holodeck.map1;

	for (i=0;i<s_holodeck.maxMaps;i++)
	{
		holdDeck->generic.type				= MTYPE_BITMAP;      
		holdDeck->generic.flags				= QMF_HIGHLIGHT_IF_FOCUS;
		holdDeck->generic.x					= x;
		holdDeck->generic.y					= y;
		holdDeck->generic.name				= GRAPHIC_BUTTONRIGHT;
		holdDeck->generic.id				= ID_DECK1 + i;
		holdDeck->generic.callback			= M_Holodeck_Event; 
		holdDeck->generic.statusbarfunc		= Holodeck_StatusBar;
		holdDeck->width						= width;
		holdDeck->height					= PROP_BIG_HEIGHT;
		holdDeck->color						= CT_DKAQUA;
		holdDeck->color2					= CT_LTAQUA;
		holdDeck->textX						= MENU_BUTTON_TEXT_X;
		holdDeck->textY						= 12;
		holdDeck->textEnum					= MBT_PROGRAM;
		holdDeck->textcolor					= CT_BLACK;
		holdDeck->textcolor2				= CT_WHITE;
		holdDeck->textStyle					= UI_TINYFONT;

		holdDeck++;
		y += pad;

		// Start the next column
		if (i == ((s_holodeck.maxMaps-1)/2))
		{
			x += width + 80; 
			y = 80;
		}
	}



	s_holodeck.engage.generic.type				= MTYPE_BITMAP;      
	s_holodeck.engage.generic.flags				= QMF_GRAYED;
	s_holodeck.engage.generic.x					= 480;
	s_holodeck.engage.generic.y					= 72 + (pad * 6);
	s_holodeck.engage.generic.name				= GRAPHIC_BUTTONRIGHT;
	s_holodeck.engage.generic.id				= ID_ENGAGE;
	s_holodeck.engage.generic.callback			= M_Holodeck_Event; 
	s_holodeck.engage.width						= width;
	s_holodeck.engage.height					= PROP_BIG_HEIGHT;
	s_holodeck.engage.color						= CT_DKORANGE;
	s_holodeck.engage.color2					= CT_LTORANGE;
	s_holodeck.engage.textX						= MENU_BUTTON_TEXT_X;
	s_holodeck.engage.textY						= 6;
	s_holodeck.engage.textEnum					= MBT_PROGRAMENGAGE;
	s_holodeck.engage.textcolor					= CT_BLACK;
	s_holodeck.engage.textcolor2				= CT_WHITE;

	s_holodeck.quitmenu.generic.type			= MTYPE_BITMAP;      
	s_holodeck.quitmenu.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_holodeck.quitmenu.generic.x				= 480;
	s_holodeck.quitmenu.generic.y				= 72 + (pad * 7);
	s_holodeck.quitmenu.generic.name			= GRAPHIC_BUTTONRIGHT;
	s_holodeck.quitmenu.generic.id				= ID_QUIT;
	s_holodeck.quitmenu.generic.callback		= M_Holodeck_Event; 
	s_holodeck.quitmenu.width					= width;
	s_holodeck.quitmenu.height					= PROP_BIG_HEIGHT;
	s_holodeck.quitmenu.color					= CT_DKORANGE;
	s_holodeck.quitmenu.color2					= CT_LTORANGE;
	s_holodeck.quitmenu.textX					= MENU_BUTTON_TEXT_X;
	s_holodeck.quitmenu.textY					= 6;
	s_holodeck.quitmenu.textEnum				= MBT_HOLODECKRETURN;
	s_holodeck.quitmenu.textcolor				= CT_BLACK;
	s_holodeck.quitmenu.textcolor2				= CT_WHITE;

	if (s_holodeck.menuType!=2)
	{
		s_holodeck.resume.generic.type				= MTYPE_BITMAP;      
		s_holodeck.resume.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS | QMF_HIDDEN;
		s_holodeck.resume.generic.x					= 180;
		s_holodeck.resume.generic.y					= 72 + (pad * 8);
		s_holodeck.resume.generic.name				= GRAPHIC_BUTTONRIGHT;
		s_holodeck.resume.generic.id				= ID_RESUME;
		s_holodeck.resume.generic.callback			= M_Holodeck_Event; 
		s_holodeck.resume.width						= width;
		s_holodeck.resume.height					= PROP_BIG_HEIGHT;
		s_holodeck.resume.color						= CT_DKORANGE;
		s_holodeck.resume.color2					= CT_LTORANGE;
		s_holodeck.resume.textX						= MENU_BUTTON_TEXT_X;
		s_holodeck.resume.textY						= 6;
		s_holodeck.resume.textEnum					= MBT_RESUMEPROGRAM;
		s_holodeck.resume.textcolor					= CT_BLACK;
		s_holodeck.resume.textcolor2				= CT_WHITE;
	}

	x = 450;
	y = 52;
	pad = 10;
	s_holodeck.supereasy.generic.type			= MTYPE_BITMAP;      
	s_holodeck.supereasy.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_holodeck.supereasy.generic.x				= x;
	s_holodeck.supereasy.generic.y				= y;
	s_holodeck.supereasy.generic.name			= GRAPHIC_BUTTONRIGHT;
	s_holodeck.supereasy.generic.id				= ID_SUPEREASY;
	s_holodeck.supereasy.generic.callback		= M_Holodeck_Event;
	s_holodeck.supereasy.width					= MENU_BUTTON_MED_WIDTH;
	s_holodeck.supereasy.height					= MENU_BUTTON_MED_HEIGHT;
	s_holodeck.supereasy.color					= CT_DKPURPLE1;
	s_holodeck.supereasy.color2					= CT_LTPURPLE1;
	s_holodeck.supereasy.textX					= MENU_BUTTON_TEXT_X;
	s_holodeck.supereasy.textY					= MENU_BUTTON_TEXT_Y;
	s_holodeck.supereasy.textEnum				= MBT_SUPEREASY;
	s_holodeck.supereasy.textcolor				= CT_BLACK;
	s_holodeck.supereasy.textcolor2				= CT_WHITE;

	y += MENU_BUTTON_MED_HEIGHT + pad;
	s_holodeck.easy.generic.type				= MTYPE_BITMAP;      
	s_holodeck.easy.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS;
	s_holodeck.easy.generic.x					= x;
	s_holodeck.easy.generic.y					= y;
	s_holodeck.easy.generic.name				= GRAPHIC_BUTTONRIGHT;
	s_holodeck.easy.generic.id					= ID_EASY;
	s_holodeck.easy.generic.callback			= M_Holodeck_Event;
	s_holodeck.easy.width						= MENU_BUTTON_MED_WIDTH;
	s_holodeck.easy.height						= MENU_BUTTON_MED_HEIGHT;
	s_holodeck.easy.color						= CT_DKPURPLE1;
	s_holodeck.easy.color2						= CT_LTPURPLE1;
	s_holodeck.easy.textX						= MENU_BUTTON_TEXT_X;
	s_holodeck.easy.textY						= MENU_BUTTON_TEXT_Y;
	s_holodeck.easy.textEnum					= MBT_EASY;
	s_holodeck.easy.textcolor					= CT_BLACK;
	s_holodeck.easy.textcolor2					= CT_WHITE;

	y += MENU_BUTTON_MED_HEIGHT + pad;
	s_holodeck.medium.generic.type				= MTYPE_BITMAP;      
	s_holodeck.medium.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS;
	s_holodeck.medium.generic.x					= x;
	s_holodeck.medium.generic.y					= y;
	s_holodeck.medium.generic.name				= GRAPHIC_BUTTONRIGHT;
	s_holodeck.medium.generic.id				= ID_MEDIUM;
	s_holodeck.medium.generic.callback			= M_Holodeck_Event;
	s_holodeck.medium.width						= MENU_BUTTON_MED_WIDTH;
	s_holodeck.medium.height					= MENU_BUTTON_MED_HEIGHT;
	s_holodeck.medium.color						= CT_DKPURPLE1;
	s_holodeck.medium.color2					= CT_LTPURPLE1;
	s_holodeck.medium.textX						= MENU_BUTTON_TEXT_X;
	s_holodeck.medium.textY						= MENU_BUTTON_TEXT_Y;
	s_holodeck.medium.textEnum					= MBT_MEDIUM;
	s_holodeck.medium.textcolor					= CT_BLACK;
	s_holodeck.medium.textcolor2				= CT_WHITE;

	y += MENU_BUTTON_MED_HEIGHT + pad;
	s_holodeck.hard.generic.type				= MTYPE_BITMAP;      
	s_holodeck.hard.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS;
	s_holodeck.hard.generic.x					= x;
	s_holodeck.hard.generic.y					= y;
	s_holodeck.hard.generic.name				= GRAPHIC_BUTTONRIGHT;
	s_holodeck.hard.generic.id					= ID_HARD;
	s_holodeck.hard.generic.callback			= M_Holodeck_Event;
	s_holodeck.hard.width						= MENU_BUTTON_MED_WIDTH;
	s_holodeck.hard.height						= MENU_BUTTON_MED_HEIGHT;
	s_holodeck.hard.color						= CT_DKPURPLE1;
	s_holodeck.hard.color2						= CT_LTPURPLE1;
	s_holodeck.hard.textX						= MENU_BUTTON_TEXT_X;
	s_holodeck.hard.textY						= MENU_BUTTON_TEXT_Y;
	s_holodeck.hard.textEnum					= MBT_HARD;
	s_holodeck.hard.textcolor					= CT_BLACK;
	s_holodeck.hard.textcolor2					= CT_WHITE;

	y += MENU_BUTTON_MED_HEIGHT + pad;
	s_holodeck.endholodeckprogram.generic.type				= s_holodeck.quitmenu.generic.type;      
	s_holodeck.endholodeckprogram.generic.flags				= s_holodeck.quitmenu.generic.flags;
	s_holodeck.endholodeckprogram.generic.flags				|= QMF_HIDDEN;
	s_holodeck.endholodeckprogram.generic.x					= s_holodeck.quitmenu.generic.x;
	s_holodeck.endholodeckprogram.generic.y					= s_holodeck.quitmenu.generic.y;
	s_holodeck.endholodeckprogram.generic.name				= s_holodeck.quitmenu.generic.name;
	s_holodeck.endholodeckprogram.generic.id				= ID_LEAVEHOLODECK;
	s_holodeck.endholodeckprogram.generic.callback			= M_Holodeck_Event;
	s_holodeck.endholodeckprogram.width						= s_holodeck.quitmenu.width;
	s_holodeck.endholodeckprogram.height					= s_holodeck.quitmenu.height;
	s_holodeck.endholodeckprogram.color						= s_holodeck.quitmenu.color;
	s_holodeck.endholodeckprogram.color2					= s_holodeck.quitmenu.color2;
	s_holodeck.endholodeckprogram.textX						= s_holodeck.quitmenu.textX;
	s_holodeck.endholodeckprogram.textY						= s_holodeck.quitmenu.textY;
	s_holodeck.endholodeckprogram.textEnum					= MBT_QUITHOLODECK;
	s_holodeck.endholodeckprogram.textcolor					= s_holodeck.quitmenu.textcolor;
	s_holodeck.endholodeckprogram.textcolor2				= s_holodeck.quitmenu.textcolor2;


	Menu_AddItem( &s_holodeck.menu,	&s_holodeck.supereasy );
	Menu_AddItem( &s_holodeck.menu,	&s_holodeck.easy );
	Menu_AddItem( &s_holodeck.menu,	&s_holodeck.medium );
	Menu_AddItem( &s_holodeck.menu,	&s_holodeck.hard );
	Menu_AddItem( &s_holodeck.menu,	&s_holodeck.endholodeckprogram );
	if (s_holodeck.menuType!=2)
	{
		Menu_AddItem( &s_holodeck.menu,	&s_holodeck.resume );
	}

	if (inHolodeck==qtrue)
	{
		s_holodeck.endholodeckprogram.generic.flags &= ~QMF_HIDDEN;
		s_holodeck.resume.generic.flags &= ~QMF_HIDDEN;
	}
	else
	{
		Menu_AddItem( &s_holodeck.menu,	&s_holodeck.quitmenu );
	}
	Menu_AddItem( &s_holodeck.menu,	&s_holodeck.engage );


	holdDeck = &s_holodeck.map1;
	for (i=0;i<s_holodeck.maxMaps;i++)
	{
		if (mapDesc[i][0])
		{
			Menu_AddItem( &s_holodeck.menu,	holdDeck );
		}
		holdDeck++;
	}

	switch ((int)ui.Cvar_VariableValue("g_spskill")) {
	case 0:	//easy
			s_holodeck.supereasy.textcolor			= CT_BLACK;
			s_holodeck.easy.textcolor				= CT_WHITE;
			s_holodeck.medium.textcolor				= CT_BLACK;
			s_holodeck.hard.textcolor				= CT_BLACK;

			s_holodeck.supereasy.generic.flags &= ~ QMF_BLINK;
			s_holodeck.easy.generic.flags |= QMF_BLINK;
			s_holodeck.medium.generic.flags &= ~ QMF_BLINK;
			s_holodeck.hard.generic.flags &= ~ QMF_BLINK;
		break;
	case 1:	//med
			s_holodeck.supereasy.textcolor			= CT_BLACK;
			s_holodeck.easy.textcolor				= CT_BLACK;
			s_holodeck.medium.textcolor				= CT_WHITE;
			s_holodeck.hard.textcolor				= CT_BLACK;

			s_holodeck.supereasy.generic.flags &= ~ QMF_BLINK;
			s_holodeck.easy.generic.flags &= ~ QMF_BLINK;
			s_holodeck.medium.generic.flags |= QMF_BLINK;
			s_holodeck.hard.generic.flags &= ~ QMF_BLINK;

		break;
	default:
	case 2:	//hard
			s_holodeck.supereasy.textcolor			= CT_BLACK;
			s_holodeck.easy.textcolor				= CT_BLACK;
			s_holodeck.medium.textcolor				= CT_BLACK;
			s_holodeck.hard.textcolor				= CT_WHITE;

			s_holodeck.supereasy.generic.flags &= ~ QMF_BLINK;
			s_holodeck.easy.generic.flags &= ~ QMF_BLINK;
			s_holodeck.medium.generic.flags &= ~ QMF_BLINK;
			s_holodeck.hard.generic.flags |= QMF_BLINK;
		break;
	}
}

/*
=================
UI_ParseHolodeckText
=================
*/
static void UI_ParseHolodeckText()
{
	char	*token;
	char *buffer;
	int i,deckNum;

	memset(mapDesc,0,sizeof(mapDesc));
	memset(mapDesc2,0,sizeof(mapDesc2));
	memset(mapCommand,0,sizeof(mapCommand));
	memset(mapInHoloCommand,0,sizeof(mapInHoloCommand));

	memset(returnDesc,0,sizeof(returnDesc));
	memset(returnDesc2,0,sizeof(returnDesc2));
	memset(returnCommand,0,sizeof(returnCommand));

	COM_BeginParseSession();

	buffer = HolodeckText;
	i = 1;	// Zero is null string
	while ( buffer ) 
	{

		token = COM_ParseExt( &buffer, qtrue );

		if (!Q_strncmp(token,"RETURNBUTTON",12))
		{
			// Get description
			token = COM_ParseExt( &buffer, qfalse );
			Q_strncpyz(returnDesc, token, sizeof(returnDesc) );

			// Get description
			token = COM_ParseExt( &buffer, qfalse );
			Q_strncpyz( returnDesc2, token, sizeof(returnDesc2) );

			// Get command	- this is ignored
			token = COM_ParseExt( &buffer, qfalse );

			// Get command
			token = COM_ParseExt( &buffer, qfalse );
			Q_strncpyz( returnCommand, token, sizeof(returnCommand) );
		}

		else if (!Q_strncmp(token,"MAP",3))
		{
			deckNum = atoi(&token[3]);

			// Get description
			token = COM_ParseExt( &buffer, qfalse );
			Q_strncpyz( mapDesc[deckNum-1], token, sizeof(mapDesc[0]) );

			// Get description
			token = COM_ParseExt( &buffer, qfalse );
			Q_strncpyz( mapDesc2[deckNum-1], token, sizeof(mapDesc2[0]) );

			// Get holodeck command
			token = COM_ParseExt( &buffer, qfalse );
			Q_strncpyz( mapCommand[deckNum-1], token, sizeof(mapCommand[0]) );

			// Get holomap command
			token = COM_ParseExt( &buffer, qfalse );
			Q_strncpyz( mapInHoloCommand[deckNum-1], token, sizeof(mapInHoloCommand[0]) );
		}
	}
}

/*
===============
UI_HolodeckMenu_Cache
===============
*/
void UI_HolodeckMenu_Cache (void)
{
	char	*buffer;
	char	filename[MAX_QPATH];
	int		len;

	s_holodeck.leftRound = ui.R_RegisterShaderNoMip("menu/common/halfroundl_24.tga");
	s_holodeck.corner_ll_12_60 = ui.R_RegisterShaderNoMip("menu/common/corner_ll_12_60.tga");
	s_holodeck.corner_ur_24_30 = ui.R_RegisterShaderNoMip("menu/common/corner_ur_24_30.tga");
	s_holodeck.corner_ll_12_30 = ui.R_RegisterShaderNoMip("menu/common/corner_ll_12_30.tga");

	UI_LanguageFilename("ext_data/sp_holodeck","dat",filename);

	len = ui.FS_ReadFile( filename,(void **) &buffer  );

	if ( len == -1 ) 
	{
		ui.Error(ERR_FATAL, "UI_HolodeckMenu_Cache : SP_HOLODECK.DAT file not found!\n");
		return;
	}

	if ( len > MAXHOLODECKTEXT ) 
	{
		ui.Error(ERR_FATAL, "UI_HolodeckMenu_Cache : SP_HOLODECK.DAT too big!\n");
		return;
	}

	strncpy( HolodeckText, buffer, sizeof( HolodeckText ) - 1 );
	ui.FS_FreeFile( buffer );

}

/*
===============
HolodeckMenu_LoadText
===============
*/
void HolodeckMenu_LoadText (void)
{

	UI_HolodeckMenu_Cache();

	UI_ParseHolodeckText();
}

/*
===============
UI_HolodeckMenu
===============
*/
void UI_HolodeckMenu(int menuType)
{
	uis.menusp = 0;

	ingameFlag = qtrue;	// true when in game menu is in use

	inHolodeck = qfalse;
	if (menuType==1)
	{
		inHolodeck = qtrue;	
	}
	else if (menuType==2)
	{
		inHolodeck = qtrue;	
	}

	s_holodeck.menuType = menuType;

	Mouse_Show();

	HolodeckMenu_LoadText();

	HolodeckMenu_Init(); 

	UI_PushMenu( &s_holodeck.menu );

	Menu_AdjustCursor( &s_holodeck.menu, 1 );	
}

void ChangeLogDesc(int id);

#define ID_LOGBUTTON1		11
#define ID_LOGBUTTON2		12
#define ID_LOGBUTTON3		13
#define ID_LOGBUTTON4		14
#define ID_LOGBUTTON5		15
#define ID_LOGBUTTON6		16
#define ID_LOGBUTTON7		17
#define ID_LOGBUTTON8		18
#define ID_LOGBUTTON9		19
#define ID_LOGBUTTON10		20	
#define ID_LOGBUTTON11		21	
#define ID_LOGBUTTON12		22	


static struct 
{
	menuframework_s menu;
	qhandle_t		leftRound;
	qhandle_t		corner_ur_20_24;
	qhandle_t		corner_lr_18_20;
	qhandle_t		corner_ll_8_47;
	qhandle_t		corner_ll_18_47;

	int				lineCnt;	// # of lines in description
	int				currentLog;	// Index to current log being read in
	int				maxButtons;	// Count of stardate buttons
	int				currentButton;	// Currently selected stardate button
	int				currentText;
	int				screenType;		// 0=Normal Logs, 1=PADDs
	menubitmap_s	quitMenu;
	menubitmap_s	buttonArrowUp;
	menubitmap_s	buttonArrowDown;
	menubitmap_s	textArrowDown;
	menubitmap_s	textArrowUp;
	int				topButton;		// Which line is at the top of the displayed buttons
	int				topText;		// Which line is at the top of the text
	int				chosenButton;				// Hi-lite subtopic button
	menubitmap_s	logButton1;
	menubitmap_s	logButton2;
	menubitmap_s	logButton3;
	menubitmap_s	logButton4;
	menubitmap_s	logButton5;
	menubitmap_s	logButton6;
	menubitmap_s	logButton7;
	menubitmap_s	logButton8;
	menubitmap_s	logButton9;
	menubitmap_s	logButton10;
	menubitmap_s	logButton11;
	menubitmap_s	logButton12;
} s_log;

#define LOGWAITTIME				25
#define MAXLOGS					60	// # of logs
#define MAXLOGBUTTONS			30	// # of buttons per log
#define MAXLOGSHOWBUTTONS		12	// # of displayable buttons 

typedef struct 
{
	char	*logName;							// Name to index log
	char	*name;								// Full name of crewmember
	char	*stardate;							// Stardate of PADD
	char	*buttonText[MAXLOGBUTTONS];			// Text for each button on crewmember's log
	char	*headingText[MAXLOGBUTTONS];		// Heading of each button
	char	*logText[MAXLOGBUTTONS];			// Log text for current button
	int		textY[MAXLOGBUTTONS];				// Starting Y position for each text
	int		menuType;							// Type of menu  MENUTYPE_
	int		cntButton;
	char	*model[MAXLOGBUTTONS];				// Model
	qhandle_t	modelHandle[MAXLOGBUTTONS];		// Handle to model
	int		modelX[MAXLOGBUTTONS];				// Model x location
	int		modelY[MAXLOGBUTTONS];				// Model y location
	int		modelDistance[MAXLOGBUTTONS];		// Model's distance from camera
	int		modelOriginY[MAXLOGBUTTONS];		// Model change in Y origin
	int		modelYaw[MAXLOGBUTTONS];			// Model's YAW from camera (0 to rotate)
	int		modelPitch[MAXLOGBUTTONS];			// Model's PITCH 
	int		modelRoll[MAXLOGBUTTONS];			// Model's ROLL 
} logText_t;

logText_t logText[MAXLOGS];

#define MAXLOGSTEXT 110000
static char	LogsText[MAXLOGSTEXT];

static struct 
{
	menuframework_s menu;
	qhandle_t		leftRound;
	qhandle_t		corner_ur_20_24;
	qhandle_t		corner_lr_18_20;
	qhandle_t		corner_ll_8_47;
	qhandle_t		corner_ll_18_47;

	int				lineCnt;	// # of lines in description
	int				currentPadd;	// Index to current log being read in
	menubitmap_s	quitMenu;
} s_padd;

#define MAXPADDS				30	// # of padds
#define MAXPADDHEADINGS			4
#define MAXPADDTEXTS			4

typedef struct 
{
	char *paddName;							// Name to index log
	char *headingText[MAXPADDHEADINGS];		// Heading of each button
	char *paddText[MAXPADDTEXTS];			// Log text for current button
	int	 menuType;							// Type of menu  MENUTYPE_
	int	 cntButton;
} paddText_t;

paddText_t paddText[MAXPADDS];

#define MAXPADDSTEXT 25000
static char	PaddsText[MAXPADDSTEXT];

// Log Menu Graphics
typedef enum 
{
	LMG_CURRENT_DESC,
	LMG_BIO_DESC1,
	LMG_BIO_DESC2,
	LMG_BIO_DESC3,
	LMG_BIO_DESC4,
	LMG_BIO_DESC5,
	LMG_BIO_DESC6,
	LMG_BIO_DESC7,
	LMG_BIO_DESC8,
	LMG_BIO_DESC9,
	LMG_BIO_DESC10,
	LMG_BIO_DESC11,
	LMG_BIO_DESC12,
	LMG_BIO_DESC13,
	LMG_BIO_DESC14,
	LMG_BIO_DESC15,
	LMG_BIO_DESC16,
	LMG_BIO_DESC17,
	LMG_BIO_DESC18,
	LMG_BIO_DESC19,
	LMG_BIO_DESC20,
	LMG_BIO_DESC21,
	LMG_BIO_DESC22,
	LMG_BIO_DESC23,
	LMG_BIO_DESC24,
	LMG_BIO_DESC25,
	LMG_MAX
} logmenu_graphics_t;

#define LOG_MAXDESC 25
char logDesc[LOG_MAXDESC][512];

menugraphics_s logmenu_graphics[LMG_MAX] = 
{
	MG_VAR,		0.0,	0,		0,		0,		0,		NULL,		0,0,	NULL,		0,		0,		0,					0,		0,							CT_NONE,		NULL,	// LMG_CURRENT_DESC

//	type		timer	x		y		width	height	file/text			graphic,	min		max		target					inc		style						color		pointer
	MG_STRING,	0.0,	242,	  0,	0,		0,		logDesc[0],0,0,		NULL,		0,		0,		LMG_BIO_DESC2,		0,		UI_TINYFONT,				CT_VLTGOLD1,	NULL,	// LMG_BIO_DESC1
	MG_STRING,	0.0,	242,	  0,	0,		0,		logDesc[1],0,0,		NULL,		0,		0,		LMG_BIO_DESC3,		0,		UI_TINYFONT,				CT_VLTGOLD1,	NULL,	// LMG_BIO_DESC2
	MG_STRING,	0.0,	242,	  0,	0,		0,		logDesc[2],0,0,		NULL,		0,		0,		LMG_BIO_DESC4,		0,		UI_TINYFONT,				CT_VLTGOLD1,	NULL,	// LMG_BIO_DESC3
	MG_STRING,	0.0,	242,	  0,	0,		0,		logDesc[3],0,0,		NULL,		0,		0,		LMG_BIO_DESC5,		0,		UI_TINYFONT,				CT_VLTGOLD1,	NULL,	// LMG_BIO_DESC4
	MG_STRING,	0.0,	242,	  0,	0,		0,		logDesc[4],0,0,		NULL,		0,		0,		LMG_BIO_DESC6,		0,		UI_TINYFONT,				CT_VLTGOLD1,	NULL,	// LMG_BIO_DESC5
	MG_STRING,	0.0,	242,	  0,	0,		0,		logDesc[5],0,0,		NULL,		0,		0,		LMG_BIO_DESC7,		0,		UI_TINYFONT,				CT_VLTGOLD1,	NULL,	// LMG_BIO_DESC6
	MG_STRING,	0.0,	242,	  0,	0,		0,		logDesc[6],0,0,		NULL,		0,		0,		LMG_BIO_DESC8,		0,		UI_TINYFONT,				CT_VLTGOLD1,	NULL,	// LMG_BIO_DESC7
	MG_STRING,	0.0,	242,	  0,	0,		0,		logDesc[7],0,0,		NULL,		0,		0,		LMG_BIO_DESC9,		0,		UI_TINYFONT,				CT_VLTGOLD1,	NULL,	// LMG_BIO_DESC8
	MG_STRING,	0.0,	242,	  0,	0,		0,		logDesc[8],0,0,		NULL,		0,		0,		LMG_BIO_DESC10,		0,		UI_TINYFONT,				CT_VLTGOLD1,	NULL,	// LMG_BIO_DESC9
	MG_STRING,	0.0,	242,	  0,	0,		0,		logDesc[9],0,0,		NULL,		0,		0,		LMG_BIO_DESC11,		0,		UI_TINYFONT,				CT_VLTGOLD1,	NULL,	// LMG_BIO_DESC10
	MG_STRING,	0.0,	242,	  0,	0,		0,		logDesc[10],0,0,	NULL,		0,		0,		LMG_BIO_DESC12,		0,		UI_TINYFONT,				CT_VLTGOLD1,	NULL,	// LMG_BIO_DESC11
	MG_STRING,	0.0,	242,	  0,	0,		0,		logDesc[11],0,0,	NULL,		0,		0,		LMG_BIO_DESC13,		0,		UI_TINYFONT,				CT_VLTGOLD1,	NULL,	// LMG_BIO_DESC12
	MG_STRING,	0.0,	242,	  0,	0,		0,		logDesc[12],0,0,	NULL,		0,		0,		LMG_BIO_DESC14,		0,		UI_TINYFONT,				CT_VLTGOLD1,	NULL,	// LMG_BIO_DESC13
	MG_STRING,	0.0,	242,	  0,	0,		0,		logDesc[13],0,0,	NULL,		0,		0,		LMG_BIO_DESC15,		0,		UI_TINYFONT,				CT_VLTGOLD1,	NULL,	// LMG_BIO_DESC14
	MG_STRING,	0.0,	242,	  0,	0,		0,		logDesc[14],0,0,	NULL,		0,		0,		LMG_BIO_DESC16,		0,		UI_TINYFONT,				CT_VLTGOLD1,	NULL,	// LMG_BIO_DESC15
	MG_STRING,	0.0,	242,	  0,	0,		0,		logDesc[15],0,0,	NULL,		0,		0,		LMG_BIO_DESC17,		0,		UI_TINYFONT,				CT_VLTGOLD1,	NULL,	// LMG_BIO_DESC16
	MG_STRING,	0.0,	242,	  0,	0,		0,		logDesc[16],0,0,	NULL,		0,		0,		LMG_BIO_DESC18,		0,		UI_TINYFONT,				CT_VLTGOLD1,	NULL,	// LMG_BIO_DESC17
	MG_STRING,	0.0,	242,	  0,	0,		0,		logDesc[17],0,0,	NULL,		0,		0,		LMG_BIO_DESC19,		0,		UI_TINYFONT,				CT_VLTGOLD1,	NULL,	// LMG_BIO_DESC18
	MG_STRING,	0.0,	242,	  0,	0,		0,		logDesc[18],0,0,	NULL,		0,		0,		LMG_BIO_DESC20,		0,		UI_TINYFONT,				CT_VLTGOLD1,	NULL,	// LMG_BIO_DESC19
	MG_STRING,	0.0,	242,	  0,	0,		0,		logDesc[19],0,0,	NULL,		0,		0,		LMG_BIO_DESC21,				0,		UI_TINYFONT,				CT_VLTGOLD1,	NULL,	// LMG_BIO_DESC20
	MG_STRING,	0.0,	242,	  0,	0,		0,		logDesc[20],0,0,	NULL,		0,		0,		LMG_BIO_DESC22,				0,		UI_TINYFONT,				CT_VLTGOLD1,	NULL,	// LMG_BIO_DESC21
	MG_STRING,	0.0,	242,	  0,	0,		0,		logDesc[21],0,0,	NULL,		0,		0,		LMG_BIO_DESC23,				0,		UI_TINYFONT,				CT_VLTGOLD1,	NULL,	// LMG_BIO_DESC22
	MG_STRING,	0.0,	242,	  0,	0,		0,		logDesc[22],0,0,	NULL,		0,		0,		LMG_BIO_DESC24,				0,		UI_TINYFONT,				CT_VLTGOLD1,	NULL,	// LMG_BIO_DESC23
	MG_STRING,	0.0,	242,	  0,	0,		0,		logDesc[23],0,0,	NULL,		0,		0,		LMG_BIO_DESC25,				0,		UI_TINYFONT,				CT_VLTGOLD1,	NULL,	// LMG_BIO_DESC24
	MG_STRING,	0.0,	242,	  0,	0,		0,		logDesc[24],0,0,	NULL,		0,		0,		NULL,				0,		UI_TINYFONT,				CT_VLTGOLD1,	NULL,	// LMG_BIO_DESC25
};




// Padd Menu Graphics
typedef enum 
{
	PMG_CURRENT_DESC,
	PMG_BIO_DESC1,
	PMG_BIO_DESC2,
	PMG_BIO_DESC3,
	PMG_BIO_DESC4,
	PMG_BIO_DESC5,
	PMG_BIO_DESC6,
	PMG_BIO_DESC7,
	PMG_BIO_DESC8,
	PMG_BIO_DESC9,
	PMG_BIO_DESC10,
	PMG_BIO_DESC11,
	PMG_BIO_DESC12,
	PMG_BIO_DESC13,
	PMG_BIO_DESC14,
	PMG_BIO_DESC15,
	PMG_BIO_DESC16,
	PMG_BIO_DESC17,
	PMG_BIO_DESC18,
	PMG_BIO_DESC19,
	PMG_BIO_DESC20,
	PMG_BIO_DESC21,
	PMG_BIO_DESC22,
	PMG_BIO_DESC23,
	PMG_BIO_DESC24,
	PMG_BIO_DESC25,
	PMG_MAX
} paddmenu_graphics_t;

#define PADD_MAXDESC 25
char paddDesc[PADD_MAXDESC][512];

menugraphics_s paddmenu_graphics[PMG_MAX] = 
{
	MG_VAR,		0.0,	0,		0,		0,		0,		NULL,		0,0,	NULL,		0,		0,		0,					0,		0,							CT_NONE,		NULL,	// LMG_CURRENT_DESC

//	type		timer	x		y		width	height	file/text			graphic,	min		max		target					inc		style						color		pointer
	MG_STRING,	0.0,	100,	  0,	0,		0,		paddDesc[0],0,0,		NULL,		0,		0,		LMG_BIO_DESC2,		0,		UI_TINYFONT,				CT_VLTGOLD1,	NULL,	// LMG_BIO_DESC1
	MG_STRING,	0.0,	100,	  0,	0,		0,		paddDesc[1],0,0,		NULL,		0,		0,		LMG_BIO_DESC3,		0,		UI_TINYFONT,				CT_VLTGOLD1,	NULL,	// LMG_BIO_DESC2
	MG_STRING,	0.0,	100,	  0,	0,		0,		paddDesc[2],0,0,		NULL,		0,		0,		LMG_BIO_DESC4,		0,		UI_TINYFONT,				CT_VLTGOLD1,	NULL,	// LMG_BIO_DESC3
	MG_STRING,	0.0,	100,	  0,	0,		0,		paddDesc[3],0,0,		NULL,		0,		0,		LMG_BIO_DESC5,		0,		UI_TINYFONT,				CT_VLTGOLD1,	NULL,	// LMG_BIO_DESC4
	MG_STRING,	0.0,	100,	  0,	0,		0,		paddDesc[4],0,0,		NULL,		0,		0,		LMG_BIO_DESC6,		0,		UI_TINYFONT,				CT_VLTGOLD1,	NULL,	// LMG_BIO_DESC5
	MG_STRING,	0.0,	242,	  0,	0,		0,		paddDesc[5],0,0,		NULL,		0,		0,		LMG_BIO_DESC7,		0,		UI_TINYFONT,				CT_VLTGOLD1,	NULL,	// LMG_BIO_DESC6
	MG_STRING,	0.0,	242,	  0,	0,		0,		paddDesc[6],0,0,		NULL,		0,		0,		LMG_BIO_DESC8,		0,		UI_TINYFONT,				CT_VLTGOLD1,	NULL,	// LMG_BIO_DESC7
	MG_STRING,	0.0,	242,	  0,	0,		0,		paddDesc[7],0,0,		NULL,		0,		0,		LMG_BIO_DESC9,		0,		UI_TINYFONT,				CT_VLTGOLD1,	NULL,	// LMG_BIO_DESC8
	MG_STRING,	0.0,	242,	  0,	0,		0,		paddDesc[8],0,0,		NULL,		0,		0,		LMG_BIO_DESC10,		0,		UI_TINYFONT,				CT_VLTGOLD1,	NULL,	// LMG_BIO_DESC9
	MG_STRING,	0.0,	242,	  0,	0,		0,		paddDesc[9],0,0,		NULL,		0,		0,		LMG_BIO_DESC11,		0,		UI_TINYFONT,				CT_VLTGOLD1,	NULL,	// LMG_BIO_DESC10
	MG_STRING,	0.0,	242,	  0,	0,		0,		paddDesc[10],0,0,	NULL,		0,		0,		LMG_BIO_DESC12,		0,		UI_TINYFONT,				CT_VLTGOLD1,	NULL,	// LMG_BIO_DESC11
	MG_STRING,	0.0,	242,	  0,	0,		0,		paddDesc[11],0,0,	NULL,		0,		0,		LMG_BIO_DESC13,		0,		UI_TINYFONT,				CT_VLTGOLD1,	NULL,	// LMG_BIO_DESC12
	MG_STRING,	0.0,	242,	  0,	0,		0,		paddDesc[12],0,0,	NULL,		0,		0,		LMG_BIO_DESC14,		0,		UI_TINYFONT,				CT_VLTGOLD1,	NULL,	// LMG_BIO_DESC13
	MG_STRING,	0.0,	242,	  0,	0,		0,		paddDesc[13],0,0,	NULL,		0,		0,		LMG_BIO_DESC15,		0,		UI_TINYFONT,				CT_VLTGOLD1,	NULL,	// LMG_BIO_DESC14
	MG_STRING,	0.0,	242,	  0,	0,		0,		paddDesc[14],0,0,	NULL,		0,		0,		LMG_BIO_DESC16,		0,		UI_TINYFONT,				CT_VLTGOLD1,	NULL,	// LMG_BIO_DESC15
	MG_STRING,	0.0,	242,	  0,	0,		0,		paddDesc[15],0,0,	NULL,		0,		0,		LMG_BIO_DESC17,		0,		UI_TINYFONT,				CT_VLTGOLD1,	NULL,	// LMG_BIO_DESC16
	MG_STRING,	0.0,	242,	  0,	0,		0,		paddDesc[16],0,0,	NULL,		0,		0,		LMG_BIO_DESC18,		0,		UI_TINYFONT,				CT_VLTGOLD1,	NULL,	// LMG_BIO_DESC17
	MG_STRING,	0.0,	242,	  0,	0,		0,		paddDesc[17],0,0,	NULL,		0,		0,		LMG_BIO_DESC19,		0,		UI_TINYFONT,				CT_VLTGOLD1,	NULL,	// LMG_BIO_DESC18
	MG_STRING,	0.0,	242,	  0,	0,		0,		paddDesc[18],0,0,	NULL,		0,		0,		LMG_BIO_DESC20,		0,		UI_TINYFONT,				CT_VLTGOLD1,	NULL,	// LMG_BIO_DESC19
	MG_STRING,	0.0,	242,	  0,	0,		0,		paddDesc[19],0,0,	NULL,		0,		0,		LMG_BIO_DESC21,				0,		UI_TINYFONT,				CT_VLTGOLD1,	NULL,	// LMG_BIO_DESC20
	MG_STRING,	0.0,	242,	  0,	0,		0,		paddDesc[20],0,0,	NULL,		0,		0,		LMG_BIO_DESC22,				0,		UI_TINYFONT,				CT_VLTGOLD1,	NULL,	// LMG_BIO_DESC21
	MG_STRING,	0.0,	242,	  0,	0,		0,		paddDesc[21],0,0,	NULL,		0,		0,		LMG_BIO_DESC23,				0,		UI_TINYFONT,				CT_VLTGOLD1,	NULL,	// LMG_BIO_DESC22
	MG_STRING,	0.0,	242,	  0,	0,		0,		paddDesc[22],0,0,	NULL,		0,		0,		LMG_BIO_DESC24,				0,		UI_TINYFONT,				CT_VLTGOLD1,	NULL,	// LMG_BIO_DESC23
	MG_STRING,	0.0,	242,	  0,	0,		0,		paddDesc[23],0,0,	NULL,		0,		0,		LMG_BIO_DESC25,				0,		UI_TINYFONT,				CT_VLTGOLD1,	NULL,	// LMG_BIO_DESC24
	MG_STRING,	0.0,	242,	  0,	0,		0,		paddDesc[24],0,0,	NULL,		0,		0,		NULL,				0,		UI_TINYFONT,				CT_VLTGOLD1,	NULL,	// LMG_BIO_DESC25
};

/*
=================
M_Log_Event
=================
*/
void M_Log_Event (void* ptr, int notification)
{
	int	id,i;
	menubitmap_s	*holdButton;

	if (notification != QM_ACTIVATED)
	{
		return;
	}

	id = ((menucommon_s*)ptr)->id;

	switch (id)
	{
		case ID_QUIT:
			UI_PopMenu();
			break;
		case ID_LOGBUTTON1:
		case ID_LOGBUTTON2:
		case ID_LOGBUTTON3:
		case ID_LOGBUTTON4:
		case ID_LOGBUTTON5:
		case ID_LOGBUTTON6:
		case ID_LOGBUTTON7:
		case ID_LOGBUTTON8:
		case ID_LOGBUTTON9:
		case ID_LOGBUTTON10:
		case ID_LOGBUTTON11:
		case ID_LOGBUTTON12:
			ChangeLogDesc(s_log.topButton + (id-ID_LOGBUTTON1));

			holdButton = &s_log.logButton1;
			holdButton += s_log.chosenButton;
			holdButton->textcolor	= CT_BLACK;

			holdButton = &s_log.logButton1;
			holdButton += (id-ID_LOGBUTTON1);
			holdButton->textcolor	= CT_VLTGOLD1;

			s_log.chosenButton = (id-ID_LOGBUTTON1);

			break;

		case ID_ARROW1UP:
			if ((s_log.topButton - 1)  >= 0)
			{
				s_log.topButton--;

				holdButton = &s_log.logButton1;

				for (i=s_log.topButton;i<(s_log.topButton + MAXLOGSHOWBUTTONS);i++)
				{
					holdButton->textPtr	= logText[s_log.currentLog].buttonText[i];
					holdButton++;
				}

				// Dark old hi-lite
				if ((s_log.chosenButton >=0) && (s_log.chosenButton < MAXLOGSHOWBUTTONS))
				{
					holdButton = &s_log.logButton1;
					holdButton += s_log.chosenButton;
					holdButton->textcolor	= CT_BLACK;
				}

				s_log.chosenButton++;

				// Hi-lite new button
				if ((s_log.chosenButton >=0) && (s_log.chosenButton < MAXLOGSHOWBUTTONS))
				{
					holdButton = &s_log.logButton1;
					holdButton += s_log.chosenButton;
					holdButton->textcolor	= CT_VLTGOLD1;
				}
			}

			// Show down arrow indicator
			s_log.buttonArrowDown.generic.flags &= ~QMF_HIDDEN;

			// Show up arrow indicator???
			if (s_log.topButton == 0)
			{
				s_log.buttonArrowUp.generic.flags |= QMF_HIDDEN;
			}
			else
			{
				s_log.buttonArrowUp.generic.flags &= ~QMF_HIDDEN;
			}

			break;

		case ID_ARROW1DOWN:
			if (((s_log.topButton + 1) + MAXLOGSHOWBUTTONS) <= logText[s_log.currentLog].cntButton)
			{
				s_log.topButton++;

				holdButton = &s_log.logButton1;

				for (i=s_log.topButton;i<(s_log.topButton + MAXLOGSHOWBUTTONS);i++)
				{
					holdButton->textPtr	= logText[s_log.currentLog].buttonText[i];
					holdButton++;
				}

				// Dark old hi-lite
				if ((s_log.chosenButton >=0) && (s_log.chosenButton < MAXLOGSHOWBUTTONS))
				{
					holdButton = &s_log.logButton1;
					holdButton += s_log.chosenButton;
					holdButton->textcolor	= CT_BLACK;
				}

				s_log.chosenButton--;

				// Hi-lite new button
				if ((s_log.chosenButton >=0) && (s_log.chosenButton < MAXLOGSHOWBUTTONS))
				{
					holdButton = &s_log.logButton1;
					holdButton += s_log.chosenButton;
					holdButton->textcolor	= CT_VLTGOLD1;
				}
			}

			// Show up arrow indicator
			s_log.buttonArrowUp.generic.flags &= ~QMF_HIDDEN;

			// Show down arrow indicator???
			if ((s_log.topButton + MAXLOGSHOWBUTTONS) >= logText[s_log.currentLog].cntButton)
			{
				s_log.buttonArrowDown.generic.flags |= QMF_HIDDEN;
			}
			else
			{
				s_log.buttonArrowDown.generic.flags &= ~QMF_HIDDEN;
			}

			break;

	}
}

/*
=================
Log_StatusBar
=================
*/
static void Log_StatusBar(void *itemptr) 
{
	int		id;

	id = ((menucommon_s*)itemptr)->id;

	switch (id)
	{
		case ID_LOGBUTTON1:
		case ID_LOGBUTTON2:
		case ID_LOGBUTTON3:
		case ID_LOGBUTTON4:
		case ID_LOGBUTTON5:
		case ID_LOGBUTTON6:
		case ID_LOGBUTTON7:
		case ID_LOGBUTTON8:
		case ID_LOGBUTTON9:
		case ID_LOGBUTTON10:
		case ID_LOGBUTTON11:
		case ID_LOGBUTTON12:
			if (logText[s_log.currentLog].headingText[s_log.topButton + (id-ID_LOGBUTTON1)])
			{
				UI_DrawProportionalString( 320, 410, logText[s_log.currentLog].headingText[s_log.topButton + (id-ID_LOGBUTTON1)], UI_CENTER|UI_SMALLFONT, colorTable[CT_VLTGOLD1]);
			}
			break;
	}
}
/*
=================
LogMenu_Key
=================
*/
sfxHandle_t LogMenu_Key (int key)
{
	return ( Menu_DefaultKey( &s_log.menu, key ) );
}

/*
=================
CrewMenu_Blinkies
=================
*/
void LogMenu_Blinkies (void)
{
	int descI;

	// Turning on description a line at a time
	if ((logmenu_graphics[LMG_CURRENT_DESC].timer < uis.realtime) && (logmenu_graphics[LMG_CURRENT_DESC].type == MG_VAR))
	{
		descI = logmenu_graphics[LMG_CURRENT_DESC].target;
		if (!descI)
		{
			logmenu_graphics[LMG_CURRENT_DESC].type = MG_OFF;
		}
		else
		{
			if (s_log.lineCnt >= (descI - LMG_BIO_DESC1))
			{
				logmenu_graphics[descI].type = MG_STRING;	// Turnon string
				ui.S_StartLocalSound( uis.menu_datadisp2_snd, CHAN_MENU1 );
				logmenu_graphics[LMG_CURRENT_DESC].target = logmenu_graphics[descI].target;	// Set up next line
				logmenu_graphics[LMG_CURRENT_DESC].timer = uis.realtime + LOGWAITTIME;
			}
		}
	}	
}

/*
=================
SplitLogDesc
=================
*/
static void SplitLogDesc(char *s,int width)
{
	int	lineWidth,currentWidth,charCnt,currentLineI;
	char *holds;
	char holdChar[2];
	int	nextLine;

	// Clean out any old data
	memset(logDesc,0,sizeof(logDesc));

	// Break into individual lines
	holds = s;

	lineWidth = width;	// How long (in pixels) a line can be
	currentWidth = 0;
	holdChar[1] = '\0';
	charCnt= 0;
	currentLineI = 0;
	s_log.lineCnt = 0;

	while( *s ) 
	{
		++charCnt;
		holdChar[0] = *s;

		// Advance to next line.
		if ((*s == '/') && (*(s + 1)  == 'n'))
		{
			s++;	// Advance to next character
			currentWidth = 0;
			nextLine = qtrue;
		}
		else
		{
			nextLine = qfalse;
			currentWidth += UI_ProportionalStringWidth(holdChar,UI_TINYFONT);
			currentWidth +=1; // The space between characters
		}

		if ((currentWidth >= lineWidth ) || (nextLine))
		{//Reached max length of this line
			//step back until we find a space

			while((currentWidth) && (*s != ' '))
			{
				holdChar[0] = *s;
				currentWidth -= UI_ProportionalStringWidth(holdChar,UI_TINYFONT);
				--s;
				--charCnt;
			}

			Q_strncpyz( logDesc[currentLineI], holds, charCnt);
			logDesc[currentLineI][charCnt] = NULL;

			++currentLineI;
			currentWidth = 0;
			charCnt = 0;

			holds = s;
			++holds;

			s_log.lineCnt++;

			if (currentLineI > LOG_MAXDESC)
			{
				break;
			}

		}	
		++s;
	}

	++charCnt;  // So the NULL will be properly placed at the end of the string of Q_strncpyz
	Q_strncpyz( logDesc[currentLineI], holds, charCnt);
	logDesc[currentLineI][charCnt] = NULL;

}

/*
=================
TurnOnLogDesc
=================
*/
static void TurnOnLogDesc(char *s,int lineWidth,int startY)
{
	int	y,i;

	logmenu_graphics[LMG_BIO_DESC1].type = MG_STRING;
	logmenu_graphics[LMG_CURRENT_DESC].target = logmenu_graphics[LMG_BIO_DESC1].target;	// Set up next line

	logmenu_graphics[LMG_CURRENT_DESC].type = MG_VAR;
	logmenu_graphics[LMG_CURRENT_DESC].timer = uis.realtime + LOGWAITTIME;

	// Split up big description line
	SplitLogDesc(s,lineWidth);

	y = startY;

	for (i=LMG_BIO_DESC1; i<(LMG_BIO_DESC1 +LOG_MAXDESC );i++)
	{
		logmenu_graphics[i].x = 242;
		logmenu_graphics[i].y = y;
		y += 12;
	}


}

/*
=================
ChangeLogDesc
=================
*/
void ChangeLogDesc(int id)
{
	int i;

	if (s_log.currentButton == id)
	{
		return;	//	Just hitting the same key again
	}
	else	// Turn off old description 
	{

		s_log.currentButton = id;

		for (i=0;i<(LMG_MAX - LMG_BIO_DESC1);++i)
		{
			logmenu_graphics[LMG_BIO_DESC1 + i].type = MG_OFF;	// Turn off text
		}
	}

	// Turn on description for new ID
	TurnOnLogDesc(logText[s_log.currentLog].logText[s_log.currentButton],360,logText[s_log.currentLog].textY[s_log.currentButton]);

}


/*
=================
M_Padd2Menu_Graphics
=================
*/
void M_Padd2Menu_Graphics (void)
{
	int nameX,length,stardateX;
	char	*title1Ptr,*title2Ptr;
	int	color1,color2,color3,color4;



	if (logText[s_log.currentLog].menuType == MENUTYPE_ENGINEERINGLIBRARY)
	{
		title1Ptr = menu_normal_text[MNT_DATA];
		title2Ptr = logText[s_log.currentLog].name;
		color1 = CT_VDKPURPLE2;
		color2 = CT_VDKORANGE;
		color3 = CT_DKPURPLE2;
		color4 = CT_LTPURPLE2;
	}
	else if (logText[s_log.currentLog].menuType == MENUTYPE_CARGO)
	{
		title1Ptr = menu_normal_text[MNT_DATA];
		title2Ptr = logText[s_log.currentLog].name;
		color1 = CT_VDKPURPLE1;
		color2 = CT_VDKORANGE;
		color3 = CT_DKPURPLE1;
		color4 = CT_LTPURPLE1;
	}
	else if (logText[s_log.currentLog].menuType == MENUTYPE_WEAPONLIBRARY)
	{
		title1Ptr = menu_normal_text[MNT_DATA];
		title2Ptr = logText[s_log.currentLog].name;
		color1 = CT_VDKPURPLE3;
		color2 = CT_VDKORANGE;
		color3 = CT_DKPURPLE3;
		color4 = CT_LTPURPLE3;
	}
	else if (logText[s_log.currentLog].menuType == MENUTYPE_SHOOTINGRANGE)
	{
		title1Ptr = menu_normal_text[MNT_DATA];
		title2Ptr = logText[s_log.currentLog].name;
		color1 = CT_VDKRED1;
		color2 = CT_VDKORANGE;
		color3 = CT_DKRED1;
		color4 = CT_LTRED1;
	}
	else if (logText[s_log.currentLog].menuType == MENUTYPE_DISEASELIBRARY)
	{
		title1Ptr = menu_normal_text[MNT_MEDICAL];
		title2Ptr = logText[s_log.currentLog].name;
		color1 = CT_VDKBLUE1;
		color2 = CT_VDKORANGE;
		color3 = CT_DKBLUE1;
		color4 = CT_LTBLUE1;
	}
	else
	{
		title1Ptr = "PADD";
		title2Ptr = logText[s_log.currentLog].name;
		color1 = CT_VDKBROWN1;
		color2 = CT_VDKORANGE;
		color3 = CT_DKBROWN1;
		color4 = CT_LTBROWN1;
	}


	// Draw the basic screen frame
	ui.R_SetColor( colorTable[color1]);
	UI_DrawHandlePic( 30,  24,16,   32, s_log.leftRound);

	// Left corners
	UI_DrawHandlePic( 181,  24, 32,   32, s_log.corner_ur_20_24);
	UI_DrawHandlePic( 179, 354, 32,   32, s_log.corner_lr_18_20);

	// Right corners
	UI_DrawHandlePic( 202,  24, -32,  32, s_log.corner_ur_20_24);
	UI_DrawHandlePic( 204, 354, -32,  32, s_log.corner_lr_18_20);

	// Lower corners
	ui.R_SetColor( colorTable[color3]);
	UI_DrawHandlePic(  30, 386,  64, -16, s_log.corner_ll_8_47);
	UI_DrawHandlePic(  30, 425, 128,  64, s_log.corner_ll_18_47);

	stardateX  = 50;
	UI_DrawProportionalString( stardateX , 24,
		title1Ptr,  UI_BIGFONT , colorTable[color4]);	

	length = UI_ProportionalStringWidth( title1Ptr,UI_BIGFONT);
	ui.R_SetColor( colorTable[color1]);
	UI_DrawHandlePic( stardateX + 4 + length,  24, (196 - (stardateX + 4 + length)), 24, uis.whiteShader);

	// Left side
	ui.R_SetColor( colorTable[color1]);
	UI_DrawHandlePic( 184,  36,  20,  27, uis.whiteShader);		// Side line1

	ui.R_SetColor( colorTable[color2]);
	UI_DrawHandlePic( 184,  66,  20,  18, uis.whiteShader);		// Upper Arrow button background

	ui.R_SetColor( colorTable[color2]);
	UI_DrawHandlePic( 184,  87,  20, 240, uis.whiteShader);		// Side line2

	ui.R_SetColor( colorTable[color2]);
	UI_DrawHandlePic( 184,  330,  20,  18, uis.whiteShader);		// Lower Arrow button background

	ui.R_SetColor( colorTable[color1]);
	UI_DrawHandlePic( 184, 351,  20,  19, uis.whiteShader);		// Side line3
	UI_DrawHandlePic( 164, 368,  35,  18, uis.whiteShader);		// Bottom line


	// Right side
	nameX = 592;

	UI_DrawProportionalString( nameX, 24,
		title2Ptr,   UI_BIGFONT | UI_RIGHT, colorTable[color4]);	
	length = UI_ProportionalStringWidth( title2Ptr,UI_BIGFONT);

	ui.R_SetColor( colorTable[color1]);
	UI_DrawHandlePic( nameX + 4,  24,-16,   32, s_log.leftRound);
	UI_DrawHandlePic( 218,  24, (nameX - (length +4)) - 218, 24, uis.whiteShader);
	
	// For personal logs
	if (logText[s_log.currentLog].menuType == MENUTYPE_PERSONALLOG)
	{
		UI_DrawProportionalString( 240, 58,
		logText[s_log.currentLog].buttonText[s_log.currentButton],   UI_SMALLFONT , colorTable[CT_VLTGOLD1]);	

		UI_DrawProportionalString( 240, 84,logText[s_log.currentLog].logName,
		   UI_TINYFONT , colorTable[CT_DKGOLD1]);

		if (logText[s_log.currentLog].stardate)
		{
			UI_DrawProportionalString( 240, 96,va("%s : %s",menu_normal_text[MNT_STARDATE],logText[s_log.currentLog].stardate),
				UI_TINYFONT , colorTable[CT_DKGOLD1]);	
		}
	}
	else if ((logText[s_log.currentLog].menuType == MENUTYPE_DISEASELIBRARY) || 
			(logText[s_log.currentLog].menuType == MENUTYPE_SHOOTINGRANGE)	||
			(logText[s_log.currentLog].menuType == MENUTYPE_WEAPONLIBRARY)	||
			(logText[s_log.currentLog].menuType == MENUTYPE_CARGO)			||
			(logText[s_log.currentLog].menuType == MENUTYPE_ENGINEERINGLIBRARY))
	{
		UI_DrawProportionalString( 240, 58,
		logText[s_log.currentLog].buttonText[s_log.currentButton],   UI_SMALLFONT , colorTable[CT_VLTGOLD1]);	
	}

	ui.R_SetColor( colorTable[color1]);
	UI_DrawHandlePic( 211,  32,  20, 22, uis.whiteShader);		// Side line
	ui.R_SetColor( colorTable[color3]);
	UI_DrawHandlePic( 211,  57,  20, 53, uis.whiteShader);		// Side line2
	UI_DrawHandlePic( 211,  113,  20, 248, uis.whiteShader);	// Side line3

	ui.R_SetColor( colorTable[color1]);
	UI_DrawHandlePic( 218, 368, 394,  18, uis.whiteShader);		// Bottom line

	// Bottom
	ui.R_SetColor( colorTable[color3]);
	UI_DrawHandlePic(  33, 391, 578,   8, uis.whiteShader);		// Top line
	UI_DrawHandlePic(  30, 396,  47,  39, uis.whiteShader);		// Side line
	UI_DrawHandlePic(  54, 438,  39,  18, uis.whiteShader);		// Bottom line 1
	ui.R_SetColor( colorTable[color1]);
	UI_DrawHandlePic(  96, 438, 268,  18, uis.whiteShader);		// Bottom line 2
	UI_DrawHandlePic( 367, 438, 245,  18, uis.whiteShader);		// Bottom line 3


	if (logText[s_log.currentLog].model[s_log.currentButton])
	{
		UI_LibraryDrawMD3Model(logText[s_log.currentLog].modelHandle[s_log.currentButton],
			logText[s_log.currentLog].modelX[s_log.currentButton],
			logText[s_log.currentLog].modelY[s_log.currentButton],
			logText[s_log.currentLog].modelDistance[s_log.currentButton],
			logText[s_log.currentLog].modelYaw[s_log.currentButton],
			logText[s_log.currentLog].modelPitch[s_log.currentButton],
			logText[s_log.currentLog].modelRoll[s_log.currentButton],
			logText[s_log.currentLog].modelOriginY[s_log.currentButton]);
	}


	LogMenu_Blinkies();

	UI_PrintMenuGraphics(logmenu_graphics,LMG_MAX);
}

/*
=================
M_LogMenu_Graphics
=================
*/
void M_LogMenu_Graphics (void)
{
	int nameX,length,stardateX;
	char	*title1Ptr,*title2Ptr;
	int	color1,color2,color3,color4;

	if (logText[s_log.currentLog].menuType == MENUTYPE_PERSONALLOG)
	{
		title1Ptr = menu_normal_text[MNT_STARDATE];
		title2Ptr = menu_normal_text[MNT_PERSONALLOG];
		color1 = CT_VDKPURPLE2;
		color2 = CT_VDKPURPLE3;
		color3 = CT_DKBROWN1;
		color4 = CT_LTPURPLE2;
	}
	else if (logText[s_log.currentLog].menuType == MENUTYPE_MEDICALLOG)
	{
		title1Ptr = menu_normal_text[MNT_TEAMMEMBERS];
		title2Ptr = menu_normal_text[MNT_MEDICALLOG];
		color1 = CT_VDKBROWN1;
		color2 = CT_VDKORANGE;
		color3 = CT_DKBROWN1;
		color4 = CT_LTBROWN1;
	}
	

	// Draw the basic screen frame

	ui.R_SetColor( colorTable[color1]);
	UI_DrawHandlePic( 30,  24,16,   32, s_log.leftRound);

	// Left corners
	UI_DrawHandlePic( 181,  24, 32,   32, s_log.corner_ur_20_24);
	UI_DrawHandlePic( 179, 354, 32,   32, s_log.corner_lr_18_20);

	// Right corners
	UI_DrawHandlePic( 202,  24, -32,  32, s_log.corner_ur_20_24);
	UI_DrawHandlePic( 204, 354, -32,  32, s_log.corner_lr_18_20);

	// Lower corners
	ui.R_SetColor( colorTable[color3]);
	UI_DrawHandlePic(  30, 386,  64, -16, s_log.corner_ll_8_47);
	UI_DrawHandlePic(  30, 425, 128,  64, s_log.corner_ll_18_47);

	stardateX  = 50;
	UI_DrawProportionalString( stardateX , 24,
		title1Ptr,  UI_BIGFONT , colorTable[color4]);	

	length = UI_ProportionalStringWidth( title1Ptr,UI_BIGFONT);
	ui.R_SetColor( colorTable[color1]);
	UI_DrawHandlePic( stardateX + 4 + length,  24, (196 - (stardateX + 4 + length)), 24, uis.whiteShader);

	// Left side
	ui.R_SetColor( colorTable[color1]);
	UI_DrawHandlePic( 184,  36,  20,  27, uis.whiteShader);		// Side line1

	ui.R_SetColor( colorTable[color2]);
	UI_DrawHandlePic( 184,  66,  20,  18, uis.whiteShader);		// Upper Arrow button background

	ui.R_SetColor( colorTable[color2]);
	UI_DrawHandlePic( 184,  87,  20, 240, uis.whiteShader);		// Side line2

	ui.R_SetColor( colorTable[color2]);
	UI_DrawHandlePic( 184,  330,  20,  18, uis.whiteShader);		// Lower Arrow button background

	ui.R_SetColor( colorTable[color1]);
	UI_DrawHandlePic( 184, 351,  20,  19, uis.whiteShader);		// Side line3
	UI_DrawHandlePic( 164, 368,  35,  18, uis.whiteShader);		// Bottom line


	// Right side
	nameX = 592;


	UI_DrawProportionalString( nameX, 24,
		title2Ptr,   UI_BIGFONT | UI_RIGHT, colorTable[color4]);	
	length = UI_ProportionalStringWidth( title2Ptr,UI_BIGFONT);

	ui.R_SetColor( colorTable[color1]);
	UI_DrawHandlePic( nameX + 4,  24,-16,   32, s_log.leftRound);
	UI_DrawHandlePic( 218,  24, (nameX - (length +4)) - 218, 24, uis.whiteShader);
	
	// For personal logs
	if (logText[s_log.currentLog].menuType == MENUTYPE_PERSONALLOG)
	{
		UI_DrawProportionalString( 240, 58,
		logText[s_log.currentLog].name,   UI_SMALLFONT , colorTable[CT_VLTGOLD1]);	

		if (logText[s_log.currentLog].headingText[s_log.currentButton])
		{
			UI_DrawProportionalString( 240, 84,va("%s : %s",menu_normal_text[MNT_TITLE],logText[s_log.currentLog].headingText[s_log.currentButton]),
			   UI_TINYFONT , colorTable[CT_DKGOLD1]);	
		}

		UI_DrawProportionalString( 240, 96,va("%s : %s",menu_normal_text[MNT_STARDATE],logText[s_log.currentLog].buttonText[s_log.currentButton]),
			UI_TINYFONT , colorTable[CT_DKGOLD1]);	
	}
	else if (logText[s_log.currentLog].menuType == MENUTYPE_MEDICALLOG)
	{
		UI_DrawProportionalString( 240, 84,va("%s",logText[s_log.currentLog].headingText[s_log.currentButton]),
		   UI_TINYFONT , colorTable[CT_DKGOLD1]);	

	}

	ui.R_SetColor( colorTable[color1]);
	UI_DrawHandlePic( 211,  32,  20, 22, uis.whiteShader);		// Side line
	ui.R_SetColor( colorTable[color3]);
	UI_DrawHandlePic( 211,  57,  20, 53, uis.whiteShader);		// Side line2
	UI_DrawHandlePic( 211,  113,  20, 248, uis.whiteShader);	// Side line3

	ui.R_SetColor( colorTable[color1]);
	UI_DrawHandlePic( 218, 368, 394,  18, uis.whiteShader);		// Bottom line

	// Bottom
	ui.R_SetColor( colorTable[color3]);
	UI_DrawHandlePic(  33, 391, 578,   8, uis.whiteShader);		// Top line
	UI_DrawHandlePic(  30, 396,  47,  39, uis.whiteShader);		// Side line
	UI_DrawHandlePic(  54, 438,  39,  18, uis.whiteShader);		// Bottom line 1
	ui.R_SetColor( colorTable[color1]);
	UI_DrawHandlePic(  96, 438, 268,  18, uis.whiteShader);		// Bottom line 2
	UI_DrawHandlePic( 367, 438, 245,  18, uis.whiteShader);		// Bottom line 3

	LogMenu_Blinkies();

	UI_PrintMenuGraphics(logmenu_graphics,LMG_MAX);
}

/*
===============
LogMenu_Draw
===============
*/
void LogMenu_Draw(void)
{
	if (s_log.screenType==1)	// PADD Screen layout
	{
		M_Padd2Menu_Graphics();
	}
	else						// LOG Screen layout
	{
		M_LogMenu_Graphics();
	}

	Menu_Draw( &s_log.menu );
}

/*
===============
LogMenu_Init
===============
*/
void LogMenu_Init(void)
{
	int	y,pad,x;
	menubitmap_s	*holdLogButton;
	int		i;

	s_log.menu.nitems						= 0;
	s_log.menu.draw							= LogMenu_Draw;
	s_log.menu.key							= LogMenu_Key;
	s_log.menu.fullscreen					= qtrue;
	s_log.menu.wrapAround					= qtrue;
	s_log.menu.descX						= MENU_DESC_X;
	s_log.menu.descY						= MENU_DESC_Y;
	s_log.menu.titleX						= MENU_TITLE_X;
	s_log.menu.titleY						= MENU_TITLE_Y;
	s_log.menu.titleI						= MNT_CREDITSMENU_TITLE;
	s_log.menu.footNoteEnum					= MNT_CREDITS;

	s_log.currentButton = 0;	// Use first button

	s_log.topButton = 0;

	holdLogButton = &s_log.logButton1;
	pad = 24;
	y =  66;
	x = 30;

	for (i=0;i<MAXLOGSHOWBUTTONS;i++)
	{
		holdLogButton->generic.type				= MTYPE_BITMAP;      
		holdLogButton->generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
		holdLogButton->generic.x				= x;
		holdLogButton->generic.y				= y;
		holdLogButton->generic.name				= GRAPHIC_BUTTONRIGHT;
		holdLogButton->generic.id				= ID_DECK1 + i;
		holdLogButton->generic.callback			= M_Log_Event; 
		holdLogButton->generic.statusbarfunc	= Log_StatusBar;
		holdLogButton->width					= MENU_BUTTON_MED_WIDTH + 20;
		holdLogButton->height					= MENU_BUTTON_MED_HEIGHT;
		holdLogButton->color					= CT_DKAQUA;
		holdLogButton->color2					= CT_LTAQUA;
		holdLogButton->textX					= MENU_BUTTON_TEXT_X;
		holdLogButton->textY					= MENU_BUTTON_TEXT_Y;
		holdLogButton->textPtr					= logText[s_log.currentLog].buttonText[i];
		holdLogButton->textcolor				= CT_BLACK;
		holdLogButton->textcolor2				= CT_WHITE;
		holdLogButton->textStyle				= UI_SMALLFONT;

		holdLogButton++;
		y += pad;
	}

	holdLogButton = &s_log.logButton1;
	for (i=0;i<MAXLOGSHOWBUTTONS;i++)
	{
		if (logText[s_log.currentLog].buttonText[i])
		{
			Menu_AddItem( &s_log.menu,	holdLogButton );
		}
		holdLogButton++;
	}

	s_log.buttonArrowUp.generic.type				= MTYPE_BITMAP;      
	s_log.buttonArrowUp.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS | QMF_HIDDEN;
	s_log.buttonArrowUp.generic.x					= 185;
	s_log.buttonArrowUp.generic.y					= 68;
	s_log.buttonArrowUp.generic.name				= "menu/common/arrow_up_16.tga";
	s_log.buttonArrowUp.generic.id					= ID_ARROW1UP;
	s_log.buttonArrowUp.generic.callback			= M_Log_Event; 
	s_log.buttonArrowUp.width						= 18;
	s_log.buttonArrowUp.height						= 18;
	s_log.buttonArrowUp.color						= CT_DKORANGE;
	s_log.buttonArrowUp.color2						= CT_LTORANGE;

	s_log.buttonArrowDown.generic.type				= MTYPE_BITMAP;      
	s_log.buttonArrowDown.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS | QMF_HIDDEN;
	s_log.buttonArrowDown.generic.x					= 185;
	s_log.buttonArrowDown.generic.y					= 332;
	s_log.buttonArrowDown.generic.name				= "menu/common/arrow_dn_16.tga";
	s_log.buttonArrowDown.generic.id				= ID_ARROW1DOWN;
	s_log.buttonArrowDown.generic.callback			= M_Log_Event; 
	s_log.buttonArrowDown.width						= 18;
	s_log.buttonArrowDown.height					= 18;
	s_log.buttonArrowDown.color						= CT_DKORANGE;
	s_log.buttonArrowDown.color2					= CT_LTORANGE;

	s_log.textArrowDown.generic.type				= MTYPE_BITMAP;      
	s_log.textArrowDown.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS;
	s_log.textArrowDown.generic.x					= 285;
	s_log.textArrowDown.generic.y					= 130;
	s_log.textArrowDown.generic.name				= "menu/common/arrow_dn_16.tga";
	s_log.textArrowDown.generic.id					= ID_ARROW2DOWN;
	s_log.textArrowDown.generic.callback			= M_Log_Event; 
	s_log.textArrowDown.width						= 18;
	s_log.textArrowDown.height						= 18;
	s_log.textArrowDown.color						= CT_DKORANGE;
	s_log.textArrowDown.color2						= CT_LTORANGE;

	s_log.quitMenu.generic.type				= MTYPE_BITMAP;      
	s_log.quitMenu.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_log.quitMenu.generic.x				= 30;
	s_log.quitMenu.generic.y				= 368;
	s_log.quitMenu.generic.name				= GRAPHIC_SQUARE;
	s_log.quitMenu.generic.id				= ID_QUIT;
	s_log.quitMenu.generic.callback			= M_Log_Event; 
	s_log.quitMenu.width					= MENU_BUTTON_MED_WIDTH;
	s_log.quitMenu.height					= MENU_BUTTON_MED_HEIGHT;
	s_log.quitMenu.color					= CT_DKORANGE;
	s_log.quitMenu.color2					= CT_LTORANGE;
	s_log.quitMenu.textX					= MENU_BUTTON_TEXT_X;
	s_log.quitMenu.textY					= MENU_BUTTON_TEXT_Y;

	if (s_log.screenType==1)	// PADD Screen layout
	{
		s_log.quitMenu.textEnum					= MBT_PADDRETURN;
	}
	else
	{
		s_log.quitMenu.textEnum					= MBT_PERSONALLOGRETURN;
	}

	s_log.quitMenu.textcolor				= CT_BLACK;
	s_log.quitMenu.textcolor2				= CT_WHITE;

	Menu_AddItem( &s_log.menu,	&s_log.buttonArrowUp );
	Menu_AddItem( &s_log.menu,	&s_log.buttonArrowDown );
	Menu_AddItem( &s_log.menu,	&s_log.quitMenu );
//	Menu_AddItem( &s_log.menu,	&s_log.textArrowDown );


	y = logText[s_log.currentLog].textY[s_log.currentButton];
	for (i=LMG_BIO_DESC1; i<(LMG_BIO_DESC1 +LOG_MAXDESC );i++)
	{
		logmenu_graphics[i].x = 242;
		logmenu_graphics[i].y = y;
		y += 12;
	}

	if (s_log.currentLog < 0)
	{
		s_log.currentLog = 0;
	}


	if (!logText[s_log.currentLog].logText[0])
	{
		UI_PopMenu();
	}
	else
	{
		SplitLogDesc(logText[s_log.currentLog].logText[s_log.currentButton],360);
	}

	if (logText[s_log.currentLog].cntButton>=MAXLOGSHOWBUTTONS)
	{	
		s_log.buttonArrowDown.generic.flags				&= ~QMF_HIDDEN;
	}
}

static void UI_ParseLogLog(char **buffer)
{
	char *token,*holdPtr;
	int	len;


	token = COM_ParseExt( buffer, qtrue );
	len = strlen(token);

	logText[s_log.currentLog].logName = *buffer - (len+1); 

	holdPtr = logText[s_log.currentLog].logName + len;
	*holdPtr = NULL;
}

static void UI_ParseLogName(char **buffer)
{
	char *token,*holdPtr;
	int	len;

	token = COM_ParseExt( buffer, qtrue );	
	len = strlen(token);

	logText[s_log.currentLog].name = *buffer - (len+1); 

	holdPtr = logText[s_log.currentLog].name + len;
	*holdPtr = NULL;
}

static void UI_ParseLogButton(char **buffer,int index)
{
	char *token,*holdPtr;
	int	len;

	token = COM_ParseExt( buffer, qtrue );
	len = strlen(token);
	
	logText[s_log.currentLog].buttonText[index] = *buffer - (len+1); 

	holdPtr = logText[s_log.currentLog].buttonText[index] + len;
	*holdPtr = NULL;
}

static void UI_ParseLogHeading(char **buffer,int index)
{
	char *token,*holdPtr;
	int	len;

	token = COM_ParseExt( buffer, qtrue );
	len = strlen(token);
	
	logText[s_log.currentLog].headingText[index] = *buffer - (len+1);  

	holdPtr = logText[s_log.currentLog].headingText[index] + len;
	*holdPtr = NULL;
}

static void UI_ParseLogStardate(char **buffer,int index)
{
	char *token,*holdPtr;
	int	len;

	token = COM_ParseExt( buffer, qtrue );
	len = strlen(token);
	
	logText[s_log.currentLog].stardate = *buffer - (len+1);  

	holdPtr = logText[s_log.currentLog].stardate + len;
	*holdPtr = NULL;
}
static void UI_ParseLogLongText(char **buffer,int index)
{
	char *token,*holdPtr;
	int	len;

	token = COM_ParseExt( buffer, qtrue );
	len = strlen(token);
	
	logText[s_log.currentLog].logText[index] = *buffer - (len+1);  

	holdPtr = logText[s_log.currentLog].logText[index] + len;
	*holdPtr = NULL;
}

static void UI_ParseMenuTypeName(char **buffer)
{
	char *token;

	token = COM_ParseExt( buffer, qtrue );

	if (!Q_strncmp(token,"MEDICALLOG",10))
	{
		logText[s_log.currentLog].menuType = MENUTYPE_MEDICALLOG;
	}
	else if (!Q_strncmp(token,"PERSONALLOG",11))
	{
		logText[s_log.currentLog].menuType = MENUTYPE_PERSONALLOG;
	}
	else if (!Q_strncmp(token,"DISEASELIBRARY",14))
	{
		logText[s_log.currentLog].menuType = MENUTYPE_DISEASELIBRARY;
	}
	else if (!Q_strncmp(token,"SHOOTINGRANGE",13))
	{
		logText[s_log.currentLog].menuType = MENUTYPE_SHOOTINGRANGE;
	}
	else if (!Q_strncmp(token,"WEAPONLIBRARY",13))
	{
		logText[s_log.currentLog].menuType = MENUTYPE_WEAPONLIBRARY;
	}
	else if (!Q_strncmp(token,"CARGO",5))
	{
		logText[s_log.currentLog].menuType = MENUTYPE_CARGO;
	}
	else if (!Q_strncmp(token,"ENGINEERINGLIBRARY",18))
	{
		logText[s_log.currentLog].menuType = MENUTYPE_ENGINEERINGLIBRARY;
	}

	
}

/*
=================
UI_ParseLogText
=================
*/
static void UI_ParseLogText()
{
	char	*token;
	char *buffer,*holdPtr;
	int i,holdNum,len;

	memset(logText,0,sizeof(logText));

	COM_BeginParseSession();

	buffer = LogsText;
	i = 1;	// Zero is null string
	while ( buffer ) 
	{

		token = COM_ParseExt( &buffer, qtrue );

		if (!Q_strncmp(token,"LOG",3))
		{
			s_log.currentLog = atoi(&token[3]);
			s_log.currentLog--;

			UI_ParseLogLog(&buffer);

			logText[s_log.currentLog].cntButton=0;
		}
		else if (!Q_strncmp(token,"PADD",4))
		{
			s_log.currentLog = atoi(&token[4]);
			s_log.currentLog--;

			UI_ParseLogLog(&buffer);

			logText[s_log.currentLog].cntButton=0;
		}
		else if (!Q_strncmp(token,"MENUTYPE",8))
		{
			UI_ParseMenuTypeName(&buffer);
		}
		else if (!Q_strncmp(token,"NAME",4))
		{
			UI_ParseLogName(&buffer);
		}
		else if (!Q_strncmp(token,"BUTTON",6))
		{
			holdNum = atoi(&token[6]);
			holdNum--;
			s_log.currentText = holdNum;
			UI_ParseLogButton(&buffer,holdNum);

			logText[s_log.currentLog].cntButton++;

		}
		else if (!Q_strncmp(token,"HEADING",7))
		{
			holdNum = atoi(&token[7]);
			holdNum--;
			UI_ParseLogHeading(&buffer,holdNum);
		}
		else if (!Q_strncmp(token,"TEXT",4))
		{
			holdNum = atoi(&token[4]);
			holdNum--;
			UI_ParseLogLongText(&buffer,holdNum);
		}
		else if (!Q_strncmp(token,"STARDATE",8))
		{
			holdNum = atoi(&token[8]);
			holdNum--;
			UI_ParseLogStardate(&buffer,holdNum);
		}
		else if (!Q_strncmp(token,"STARTTEXTY",10))
		{
			token = COM_ParseExt( &buffer, qtrue );	
			logText[s_log.currentLog].textY[s_log.currentText] = atoi(token);  
		}
		else if (!Q_strncmp(token,"MODELX",6))
		{
			token = COM_ParseExt( &buffer, qtrue );	
			logText[s_log.currentLog].modelX[s_log.currentText] = atoi(token);
		}
		else if (!Q_strncmp(token,"MODELYAW",8))
		{
			token = COM_ParseExt( &buffer, qtrue );	
			logText[s_log.currentLog].modelYaw[s_log.currentText] = atoi(token);
		}
		else if (!Q_strncmp(token,"MODELPITCH",10))
		{
			token = COM_ParseExt( &buffer, qtrue );	
			logText[s_log.currentLog].modelPitch[s_log.currentText] = atoi(token);
		}
		else if (!Q_strncmp(token,"MODELROLL",9))
		{
			token = COM_ParseExt( &buffer, qtrue );	
			logText[s_log.currentLog].modelRoll[s_log.currentText] = atoi(token);
		}
		else if (!Q_strncmp(token,"MODELY",6))
		{
			token = COM_ParseExt( &buffer, qtrue );	
			logText[s_log.currentLog].modelY[s_log.currentText] = atoi(token);
		}
		else if (!Q_strncmp(token,"MODELDISTANCE",13))	// Model's distance from camera
		{
			token = COM_ParseExt( &buffer, qtrue );	
			logText[s_log.currentLog].modelDistance[s_log.currentText] = atoi(token);
		}
		else if (!Q_strncmp(token,"MODELORIGINY",12))	// Model's change in Y origin
		{
			token = COM_ParseExt( &buffer, qtrue );	
			logText[s_log.currentLog].modelOriginY[s_log.currentText] = atoi(token);
		}
		else if (!Q_strncmp(token,"MODEL",5))
		{
			// Get main topic desc
			token = COM_ParseExt( &buffer, qtrue );	
			len = strlen(token);
			logText[s_log.currentLog].model[s_log.currentText] = buffer - (len+1); 
			holdPtr = logText[s_log.currentLog].model[s_log.currentText] + len;
			*holdPtr = NULL;

			logText[s_log.currentLog].modelHandle[s_log.currentText] = 
				ui.R_RegisterModel(logText[s_log.currentLog].model[s_log.currentText]);
		}
	}
}

/*
===============
UI_LogMenu_Cache
===============
*/
void UI_LogMenu_Cache (void)
{
	char	*buffer;
	char	filename[MAX_QPATH];
	int		len;

	s_log.leftRound = ui.R_RegisterShaderNoMip("menu/common/halfroundl_24.tga");
	s_log.corner_ur_20_24 = ui.R_RegisterShaderNoMip("menu/common/corner_ur_20_24.tga");
	s_log.corner_lr_18_20 = ui.R_RegisterShaderNoMip("menu/common/corner_lr_18_20.tga");
	s_log.corner_ll_18_47 = ui.R_RegisterShaderNoMip("menu/common/corner_ll_47_18.tga");
	s_log.corner_ll_8_47  = ui.R_RegisterShaderNoMip("menu/common/corner_ll_8_47.tga");

	if (s_log.screenType==1)
	{
		UI_LanguageFilename("ext_data/sp_padd2s","dat",filename);
	}
	else 
	{
		UI_LanguageFilename("ext_data/sp_logs","dat",filename);
	}

	len = ui.FS_ReadFile( filename,(void **) &buffer  );

	if ( len == -1 ) 
	{
		ui.Error(ERR_FATAL, "UI_LogMenu_Cache : SP_LOGS.DAT file not found!\n");
		return;
	}

	if ( len > MAXLOGSTEXT ) 
	{
		ui.Error(ERR_FATAL, "UI_LogMenu_Cache : SP_LOGS.DAT is too big! MAX: %d, current length:%d\n",MAXLOGSTEXT,len);
		return;
	}

	strncpy( LogsText, buffer, sizeof( LogsText ) - 1 );
	ui.FS_FreeFile( buffer );

}

// Special Cache function for logs
void UI_LogMenu_SpecialCache(void)
{
	s_log.screenType=0;
	UI_LogMenu_Cache ();
	UI_ParseLogText();
}

// Special Cache function for padds
void UI_PaddMenu_SpecialCache(void)
{
	s_log.screenType=1;
	UI_LogMenu_Cache ();
	UI_ParseLogText();
}


/*
===============
LogMenu_LoadText
===============
*/
void LogMenu_LoadText (void)
{

	UI_LogMenu_Cache();

	UI_ParseLogText();
}


/*
===============
UI_LogMenu
===============
*/
void UI_LogMenu(char *menuId,int screenType)
{
	uis.menusp = 0;
	s_log.screenType=screenType;
	ingameFlag = qtrue;	// true when in game menu is in use

	Mouse_Show();

	LogMenu_LoadText();

	if (s_log.screenType==1)	// PADD
	{
		s_log.currentLog = atoi((char *) (menuId+4));
	}
	else						// LOG
	{
		s_log.currentLog = atoi((char *) (menuId+3));
	}

	s_log.currentLog--;

	UI_PushMenu( &s_log.menu );

	LogMenu_Init(); 

	Menu_AdjustCursor( &s_log.menu, 1 );	
}


/*
=================
SplitPaddDesc
=================
*/
static void SplitPaddDesc(char *s,int width)
{
	int	lineWidth,currentWidth,charCnt,currentLineI;
	char *holds;
	char holdChar[2];
	int	nextLine;

	// Clean out any old data
	memset(paddDesc,0,sizeof(paddDesc));

	// Break into individual lines
	holds = s;

	lineWidth = width;	// How long (in pixels) a line can be
	currentWidth = 0;
	holdChar[1] = '\0';
	charCnt= 0;
	currentLineI = 0;
	s_padd.lineCnt = 0;

	while( *s ) 
	{
		++charCnt;
		holdChar[0] = *s;

		// Advance to next line.
		if ((*s == '/') && (*(s + 1)  == 'n'))
		{
			s++;	// Advance to next character
			currentWidth = 0;
			nextLine = qtrue;
		}
		else
		{
			nextLine = qfalse;
			currentWidth += UI_ProportionalStringWidth(holdChar,UI_TINYFONT);
			currentWidth +=1; // The space between characters
		}

		if ((currentWidth >= lineWidth ) || (nextLine))
		{//Reached max length of this line
			//step back until we find a space

			while((currentWidth) && (*s != ' '))
			{
				holdChar[0] = *s;
				currentWidth -= UI_ProportionalStringWidth(holdChar,UI_TINYFONT);
				--s;
				--charCnt;
			}

			Q_strncpyz( paddDesc[currentLineI], holds, charCnt);
			paddDesc[currentLineI][charCnt] = NULL;

			++currentLineI;
			currentWidth = 0;
			charCnt = 0;

			holds = s;
			++holds;

			s_padd.lineCnt++;

			if (currentLineI > PADD_MAXDESC)
			{
				break;
			}

		}	
		++s;
	}

	++charCnt;  // So the NULL will be properly placed at the end of the string of Q_strncpyz
	Q_strncpyz( paddDesc[currentLineI], holds, charCnt);
	paddDesc[currentLineI][charCnt] = NULL;

}

/*
=================
PaddMenu_Blinkies
=================
*/
void PaddMenu_Blinkies (void)
{
	int descI;

	// Turning on description a line at a time
	if ((paddmenu_graphics[PMG_CURRENT_DESC].timer < uis.realtime) && (paddmenu_graphics[PMG_CURRENT_DESC].type == MG_VAR))	{
		descI = paddmenu_graphics[PMG_CURRENT_DESC].target;
		if (!descI)
		{
			paddmenu_graphics[PMG_CURRENT_DESC].type = MG_OFF;
		}
		else
		{
			if (s_padd.lineCnt >= (descI - PMG_BIO_DESC1))
			{
				paddmenu_graphics[descI].type = MG_STRING;	// Turnon string
				ui.S_StartLocalSound( uis.menu_datadisp2_snd, CHAN_MENU1 );
				paddmenu_graphics[PMG_CURRENT_DESC].target = paddmenu_graphics[descI].target;	// Set up next line
				paddmenu_graphics[PMG_CURRENT_DESC].timer = uis.realtime + LOGWAITTIME;
			}
		}
	}	
}

/*
=================
M_Padd_Event
=================
*/
void M_Padd_Event (void* ptr, int notification)
{
	int	id;

	if (notification != QM_ACTIVATED)
	{
		return;
	}

	id = ((menucommon_s*)ptr)->id;

	switch (id)
	{
		case ID_QUIT:
			UI_PopMenu();
			break;
	}
}

/*
=================
PaddMenu_Key
=================
*/
sfxHandle_t PaddMenu_Key (int key)
{
	return ( Menu_DefaultKey( &s_padd.menu, key ) );
}

/*
=================
M_PaddMenu_Graphics
=================
*/
void M_PaddMenu_Graphics (void)
{
	int nameX,length;
	char	*title1Ptr;
	int	color1,color2,color3,color4;

	title1Ptr = "DATA PADD";
	color1 = CT_VDKPURPLE2;
	color2 = CT_VDKPURPLE3;
	color3 = CT_DKBROWN1;
	color4 = CT_LTPURPLE2;

	// Draw the basic screen frame

	// Right corners
	ui.R_SetColor( colorTable[color1]);
	UI_DrawHandlePic( 20,  24, -32,  32, s_padd.corner_ur_20_24);
	UI_DrawHandlePic( 22, 354, -32,  32, s_padd.corner_lr_18_20);

	// Lower corners
	ui.R_SetColor( colorTable[color3]);
	UI_DrawHandlePic(  30, 386,  64, -16, s_padd.corner_ll_8_47);
	UI_DrawHandlePic(  30, 425, 128,  64, s_padd.corner_ll_18_47);


	// Right side
	nameX = 592;


	UI_DrawProportionalString( nameX, 24,
		title1Ptr,   UI_BIGFONT | UI_RIGHT, colorTable[color4]);	
	length = UI_ProportionalStringWidth( title1Ptr,UI_BIGFONT);

	ui.R_SetColor( colorTable[color1]);
	UI_DrawHandlePic( nameX + 4,  24,-16,   32, s_padd.leftRound);
	UI_DrawHandlePic( 36,  24, (nameX - (length +4)) - 36, 24, uis.whiteShader);
	
	UI_DrawProportionalString( 320, 58,
	paddText[s_padd.currentPadd].headingText[0],  UI_BIGFONT | UI_CENTER , colorTable[CT_VLTGOLD1]);	

	UI_DrawProportionalString( 320, 90,
	paddText[s_padd.currentPadd].headingText[1],  UI_SMALLFONT | UI_CENTER , colorTable[CT_VLTGOLD1]);	

	ui.R_SetColor( colorTable[color1]);
	UI_DrawHandlePic( 30,  32,  20, 22, uis.whiteShader);		// Side line
	ui.R_SetColor( colorTable[color3]);
	UI_DrawHandlePic( 30,  57,  20, 53, uis.whiteShader);		// Side line2
	UI_DrawHandlePic( 30,  113,  20, 248, uis.whiteShader);	// Side line3

	ui.R_SetColor( colorTable[color1]);
	UI_DrawHandlePic( 218, 368, 394,  18, uis.whiteShader);		// Bottom line

	// Bottom
	ui.R_SetColor( colorTable[color3]);
	UI_DrawHandlePic(  33, 391, 578,   8, uis.whiteShader);		// Top line
	UI_DrawHandlePic(  30, 396,  47,  39, uis.whiteShader);		// Side line
	UI_DrawHandlePic(  54, 438,  39,  18, uis.whiteShader);		// Bottom line 1
	ui.R_SetColor( colorTable[color1]);
	UI_DrawHandlePic(  96, 438, 268,  18, uis.whiteShader);		// Bottom line 2
	UI_DrawHandlePic( 367, 438, 245,  18, uis.whiteShader);		// Bottom line 3

	PaddMenu_Blinkies();

	UI_PrintMenuGraphics(paddmenu_graphics,PMG_MAX);
}

/*
===============
PaddMenu_Draw
===============
*/
void PaddMenu_Draw(void)
{
	M_PaddMenu_Graphics();
	
	Menu_Draw( &s_padd.menu );
}

/*
===============
PaddMenu_Init
===============
*/
void PaddMenu_Init(void)
{
	int	y;
	int		i;

	s_padd.menu.nitems						= 0;
	s_padd.menu.draw						= PaddMenu_Draw;
	s_padd.menu.key							= PaddMenu_Key;
	s_padd.menu.fullscreen					= qtrue;
	s_padd.menu.wrapAround					= qtrue;
	s_padd.menu.descX						= MENU_DESC_X;
	s_padd.menu.descY						= MENU_DESC_Y;
	s_padd.menu.titleX						= MENU_TITLE_X;
	s_padd.menu.titleY						= MENU_TITLE_Y;
	s_padd.menu.titleI						= MNT_CREDITSMENU_TITLE;
	s_padd.menu.footNoteEnum				= MNT_CREDITS;

	s_padd.quitMenu.generic.type				= MTYPE_BITMAP;      
	s_padd.quitMenu.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_padd.quitMenu.generic.x				= 30;
	s_padd.quitMenu.generic.y				= 368;
	s_padd.quitMenu.generic.name				= GRAPHIC_SQUARE;
	s_padd.quitMenu.generic.id				= ID_QUIT;
	s_padd.quitMenu.generic.callback			= M_Padd_Event; 
	s_padd.quitMenu.width					= MENU_BUTTON_MED_WIDTH;
	s_padd.quitMenu.height					= MENU_BUTTON_MED_HEIGHT;
	s_padd.quitMenu.color					= CT_DKORANGE;
	s_padd.quitMenu.color2					= CT_LTORANGE;
	s_padd.quitMenu.textX					= MENU_BUTTON_TEXT_X;
	s_padd.quitMenu.textY					= MENU_BUTTON_TEXT_Y;
	s_padd.quitMenu.textEnum					= MBT_PADDRETURN;
	s_padd.quitMenu.textcolor				= CT_BLACK;
	s_padd.quitMenu.textcolor2				= CT_WHITE;

	Menu_AddItem( &s_padd.menu,	&s_padd.quitMenu );

	y = 116;
	for (i=PMG_BIO_DESC1; i<(PMG_BIO_DESC1 +PADD_MAXDESC );i++)
	{
		paddmenu_graphics[i].x = 80;
		paddmenu_graphics[i].y = y;
		y += 12;
	}

	if (s_padd.currentPadd < 0)
	{
		s_padd.currentPadd = 0;
	}

	// No text???
	if (!paddText[s_padd.currentPadd].paddText[0])
	{
		UI_PopMenu();
	}
	else
	{
		SplitPaddDesc(paddText[s_padd.currentPadd].paddText[0],560);
	}
}

static void UI_ParsePaddLongText(char **buffer,int index)
{
	char *token,*holdPtr;
	int	len;

	token = COM_ParseExt( buffer, qtrue );
	len = strlen(token);
	
	paddText[s_padd.currentPadd].paddText[index] = *buffer - (len+1);  

	holdPtr = paddText[s_padd.currentPadd].paddText[index] + len;
	*holdPtr = NULL;
}

static void UI_ParsePaddPadd(char **buffer)
{
	char *token,*holdPtr;
	int	len;

	token = COM_ParseExt( buffer, qtrue );
	len = strlen(token);

	paddText[s_padd.currentPadd].paddName = *buffer - (len+1); 

	holdPtr = paddText[s_padd.currentPadd].paddName + len;
	*holdPtr = NULL;
}

static void UI_ParsePaddHeading(char **buffer,int index)
{
	char *token,*holdPtr;
	int	len;

	token = COM_ParseExt( buffer, qtrue );
	len = strlen(token);
	
	paddText[s_padd.currentPadd].headingText[index] = *buffer - (len+1);  

	holdPtr = paddText[s_padd.currentPadd].headingText[index] + len;
	*holdPtr = NULL;
}

/*
=================
UI_ParsePaddText
=================
*/
static void UI_ParsePaddText()
{
	char	*token;
	char *buffer;
	int i,holdNum;

	memset(paddText,0,sizeof(paddText));

	COM_BeginParseSession();

	buffer = PaddsText;
	i = 1;	// Zero is null string
	while ( buffer ) 
	{

		token = COM_ParseExt( &buffer, qtrue );

		if (!Q_strncmp(token,"PADD",4))
		{
			s_padd.currentPadd = atoi(&token[4]);
			s_padd.currentPadd--;

			UI_ParsePaddPadd(&buffer);
		}
		else if (!Q_strncmp(token,"HEADING",7))
		{
			holdNum = atoi(&token[7]);
			holdNum--;
			UI_ParsePaddHeading(&buffer,holdNum);
		}
		else if (!Q_strncmp(token,"TEXT",4))
		{
			holdNum = atoi(&token[4]);
			holdNum--;
			UI_ParsePaddLongText(&buffer,holdNum);
		}
	}
}

/*
===============
UI_PaddMenu_Cache
===============
*/
void UI_PaddMenu_Cache (void)
{
	char	*buffer;
	char	filename[MAX_QPATH];
	int		len;

	s_padd.leftRound = ui.R_RegisterShaderNoMip("menu/common/halfroundl_24.tga");
	s_padd.corner_ur_20_24 = ui.R_RegisterShaderNoMip("menu/common/corner_ur_20_24.tga");
	s_padd.corner_lr_18_20 = ui.R_RegisterShaderNoMip("menu/common/corner_lr_18_20.tga");
	s_padd.corner_ll_18_47 = ui.R_RegisterShaderNoMip("menu/common/corner_ll_47_18.tga");
	s_padd.corner_ll_8_47  = ui.R_RegisterShaderNoMip("menu/common/corner_ll_8_47.tga");

	UI_LanguageFilename("ext_data/sp_padds","dat",filename);

	len = ui.FS_ReadFile( filename,(void **) &buffer  );

	if ( len == -1 ) 
	{
		ui.Error(ERR_FATAL, "UI_PaddMenu_Cache : SP_PADDS.DAT file not found!\n");
		return;
	}

	if ( len > MAXPADDSTEXT ) 
	{
		ui.Error(ERR_FATAL, "UI_PaddMenu_Cache : SP_PADDS.DAT too big!\n");
		return;
	}

	strncpy( PaddsText, buffer, sizeof( PaddsText ) - 1 );
	ui.FS_FreeFile( buffer );

}

/*
===============
PaddMenu_LoadText
===============
*/
void PaddMenu_LoadText (void)
{

	UI_PaddMenu_Cache();

	UI_ParsePaddText();
}

/*
===============
UI_PaddMenu
===============
*/
void UI_PaddMenu(char *menuId)
{
	uis.menusp = 0;

	ingameFlag = qtrue;	// true when in game menu is in use

	Mouse_Show();

	PaddMenu_LoadText();

	s_padd.currentPadd = atoi((char *) (menuId+4));
	s_padd.currentPadd--;

	UI_PushMenu( &s_padd.menu );

	Menu_AdjustCursor( &s_padd.menu, 1 );	

	PaddMenu_Init(); 

}


/*
===============
UI_Padd2Menu
===============
*/
void UI_Padd2Menu(char *menuId)
{
	UI_LogMenu(menuId,1);
}

static struct 
{
	menuframework_s menu;
	int				maxSites;
	int				chosenSite;
	menubitmap_s	quitMenu;
	menubitmap_s	engage;
	qhandle_t		figure;
	qhandle_t		leftRound;
	qhandle_t		slider;
	qhandle_t		corner_ur_20_24;
	qhandle_t		corner_lr_18_20;
	qhandle_t		corner_ll_18_130;
	qhandle_t		corner_lu_24_130;
	qhandle_t		corner_ll_8_47;
	qhandle_t		corner_ll_18_47;
	qhandle_t		arrow;
	menubitmap_s	siteButton1;
	menubitmap_s	siteButton2;
	menubitmap_s	siteButton3;
	menubitmap_s	siteButton4;
	menubitmap_s	siteButton5;
	menubitmap_s	siteButton6;
	menubitmap_s	siteButton7;
	menubitmap_s	siteButton8;
	menubitmap_s	siteButton9;
	menubitmap_s	siteButton10;
} s_transporter;

#define	MAXSITEBUTTON		10

#define MAXTRANSPORTERTEXT 5000
static char	TransporterText[MAXTRANSPORTERTEXT];

typedef struct 
{
	char *descriptionText;	// Description of each button
	char *command;			// Coomand of button
} transporterText_t;

#define MAXTRANSPORTER 20

transporterText_t transporterText[MAXTRANSPORTER];

#define ID_SITEBUTTON1		11
#define ID_SITEBUTTON2		12
#define ID_SITEBUTTON3		13
#define ID_SITEBUTTON4		14
#define ID_SITEBUTTON5		15
#define ID_SITEBUTTON6		16
#define ID_SITEBUTTON7		17
#define ID_SITEBUTTON8		18
#define ID_SITEBUTTON9		19
#define ID_SITEBUTTON10		20

/*
=================
M_Transporter_Event
=================
*/
void M_Transporter_Event (void* ptr, int notification)
{
	int	id;
	menubitmap_s	*holdSite;

	if (notification != QM_ACTIVATED)
	{
		return;
	}

	id = ((menucommon_s*)ptr)->id;

	switch (id)
	{
		case ID_QUIT:
			UI_PopMenu();
			break;
		case ID_SITEBUTTON1:
		case ID_SITEBUTTON2:
		case ID_SITEBUTTON3:
		case ID_SITEBUTTON4:
		case ID_SITEBUTTON5:
		case ID_SITEBUTTON6:
		case ID_SITEBUTTON7:
		case ID_SITEBUTTON8:
		case ID_SITEBUTTON9:
		case ID_SITEBUTTON10:
			if (s_transporter.chosenSite >= 0)
			{
				holdSite = &s_transporter.siteButton1;
				holdSite += s_transporter.chosenSite;
				holdSite->textcolor	= CT_BLACK;
			}

			s_transporter.chosenSite	= id - ID_SITEBUTTON1;
			s_transporter.engage.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;

			holdSite = &s_transporter.siteButton1;
			holdSite += s_transporter.chosenSite;
			holdSite->textcolor	= CT_LTGOLD1;
			break;

		case ID_ENGAGE:
			if (transporterText[s_transporter.chosenSite].command)
			{
//				ui.S_StartLocalSound( , CHAN_MENU2 );

				UI_ForceMenuOff ();
				ui.Cmd_ExecuteText( EXEC_APPEND, transporterText[s_transporter.chosenSite].command);
			}
			break;

	}
}





/*
=================
Transporter_StatusBar
=================
*/
static void Transporter_StatusBar(void *itemptr) 
{
	int		id;

	id = ((menucommon_s*)itemptr)->id;

	switch (id)
	{
		case ID_SITEBUTTON1:
		case ID_SITEBUTTON2:
		case ID_SITEBUTTON3:
		case ID_SITEBUTTON4:
		case ID_SITEBUTTON5:
		case ID_SITEBUTTON6:
		case ID_SITEBUTTON7:
		case ID_SITEBUTTON8:
		case ID_SITEBUTTON9:
		case ID_SITEBUTTON10:
				UI_DrawProportionalString( 320, 410, transporterText[id-ID_SITEBUTTON1].descriptionText, UI_CENTER|UI_SMALLFONT, colorTable[CT_VLTGOLD1]);
			break;
	}
}

/*
=================
TransporterMenu_Key
=================
*/
sfxHandle_t TransporterMenu_Key (int key)
{
	return ( Menu_DefaultKey( &s_transporter.menu, key ) );
}

/*
=================
M_TransporterMenu_Graphics
=================
*/
void M_TransporterMenu_Graphics (void)
{
	int	stardateX,length,numColor,roundColor,i;
	menubitmap_s	*holdSite;

	ui.R_SetColor( colorTable[CT_VDKPURPLE2]);
	UI_DrawHandlePic( 30,  24,16,   32, s_transporter.leftRound);

	stardateX  = 50;
	UI_DrawProportionalString( stardateX , 24,
		menu_normal_text[MNT_TRANSPORTER1],  UI_BIGFONT , colorTable[CT_LTPURPLE2]);	

	length = UI_ProportionalStringWidth( menu_normal_text[MNT_TRANSPORTER1],UI_BIGFONT);
	ui.R_SetColor( colorTable[CT_VDKPURPLE2]);
	UI_DrawHandlePic( stardateX + 4 + length,  24, (240 - (stardateX + 4 + length)), 24, uis.whiteShader);

	// Draw the basic screen frame
	ui.R_SetColor( colorTable[CT_VLTBLUE2]);
	UI_DrawHandlePic( 475,  80,  256, 256, s_transporter.figure);

	ui.R_SetColor( colorTable[CT_LTGOLD1]);
	UI_DrawHandlePic( 326,  117,  52,  64, s_transporter.slider);
	UI_DrawHandlePic( 326,  180,  52,  64, s_transporter.slider);
	UI_DrawHandlePic( 374,  117,  52,  64, s_transporter.slider);
	UI_DrawHandlePic( 374,  180,  52,  64, s_transporter.slider);
	UI_DrawHandlePic( 422,  117,  52,  64, s_transporter.slider);
	UI_DrawHandlePic( 422,  180,  52,  64, s_transporter.slider);
	ui.R_SetColor( colorTable[CT_DKGOLD1]);
	UI_DrawHandlePic( 318,  238,  8,  8, s_transporter.arrow);
	UI_DrawHandlePic( 340,  238, -8,  8, s_transporter.arrow);
	UI_DrawHandlePic( 366,  238,  8,  8, s_transporter.arrow);
	UI_DrawHandlePic( 388,  238, -8,  8, s_transporter.arrow);
	UI_DrawHandlePic( 414,  238,  8,  8, s_transporter.arrow);
	UI_DrawHandlePic( 436,  238, -8,  8, s_transporter.arrow);


	ui.R_SetColor( colorTable[CT_VDKPURPLE2]);
	// Left side
	UI_DrawHandlePic( 237,  24, 32,   32, s_transporter.corner_ur_20_24);
	UI_DrawHandlePic( 235, 354, 32,   32, s_transporter.corner_lr_18_20);

	// Right side
	ui.R_SetColor( colorTable[CT_DKBROWN1]);
	UI_DrawHandlePic( 261,  24, -32,   32, s_transporter.corner_ur_20_24);
	UI_DrawHandlePic( 350,  24, -256,   32, s_transporter.corner_lu_24_130);
	UI_DrawHandlePic( 350, 360, -256,   32, s_transporter.corner_ll_18_130);
	UI_DrawHandlePic( 328,  24, 216,  24, uis.whiteShader);		// Top line
	UI_DrawHandlePic( 326, 368, 216,  18, uis.whiteShader);		// Bottom line
	UI_DrawHandlePic( 270,  46,  20,  67, uis.whiteShader);		// Column Side
	
	ui.R_SetColor( colorTable[CT_LTBLUE1]);
	UI_DrawHandlePic( 270, 115,  10,  127, uis.whiteShader);		// Column Side
	UI_DrawHandlePic( 298, 368,  25,  18, uis.whiteShader);		// Bottom line

	ui.R_SetColor( colorTable[CT_LTBROWN1]);
	UI_DrawHandlePic( 296,  24,  29,  24, uis.whiteShader);		// Top line

	ui.R_SetColor( colorTable[CT_DKBROWN1]);
	UI_DrawHandlePic( 263, 354, -32,   32, s_transporter.corner_lr_18_20);
	UI_DrawHandlePic( 270, 244,  20, 134, uis.whiteShader);		// Column Side
	UI_DrawHandlePic( 250, 115,  10,  127, uis.whiteShader);	// Column Side

	// Top Left side
	ui.R_SetColor( colorTable[CT_VDKPURPLE2]);
	UI_DrawHandlePic( 240,  46,  20, 67, uis.whiteShader);		// Column Side 
	UI_DrawHandlePic( 240, 244,  20, 134, uis.whiteShader);		// Column Side

	UI_DrawHandlePic(  30, 368, 37,  18, uis.whiteShader);		// Bottom line
	UI_DrawHandlePic( 203, 368, 48,  18, uis.whiteShader);		// Bottom line

	// Bottom
	ui.R_SetColor( colorTable[CT_VDKGOLD1]);
	UI_DrawHandlePic(  30, 386,  64, -16, s_transporter.corner_ll_8_47);
	UI_DrawHandlePic(  30, 425, 128,  64, s_transporter.corner_ll_18_47);

	ui.R_SetColor( colorTable[CT_VDKGOLD1]);
	UI_DrawHandlePic(  33, 391, 578,   8, uis.whiteShader);		// Top line
	UI_DrawHandlePic(  30, 396,  47,  39, uis.whiteShader);		// Side line
	UI_DrawHandlePic(  54, 438,  39,  18, uis.whiteShader);		// Bottom line 1
	ui.R_SetColor( colorTable[CT_DKGOLD1]);
	UI_DrawHandlePic(  96, 438, 268,  18, uis.whiteShader);		// Bottom line 2
	UI_DrawHandlePic( 367, 438, 245,  18, uis.whiteShader);		// Bottom line 3


	holdSite = &s_transporter.siteButton1;

	for (i=0;i<s_transporter.maxSites;i++)
	{
		if (transporterText[i].descriptionText )
		{
			if (s_transporter.chosenSite == i)		// This site was chosen
			{
				numColor = CT_LTGOLD1;
				roundColor = CT_LTGOLD1;
			}
			else								// This site was not chosen
			{
				numColor = CT_WHITE;
				roundColor = CT_DKAQUA;
			}

			UI_DrawProportionalString( holdSite->generic.x - 6,
				holdSite->generic.y, 
				va("%d",i+1),   UI_BIGFONT|UI_RIGHT, colorTable[numColor]);		

			ui.R_SetColor( colorTable[roundColor]);
			UI_DrawHandlePic( holdSite->generic.x - 45, 
				holdSite->generic.y, 
				16,   32, s_transporter.leftRound);
		}
		holdSite++;
	}

}

/*
===============
TransporterMenu_Draw
===============
*/
void TransporterMenu_Draw(void)
{
	// Draw graphics particular to Main Menu
	M_TransporterMenu_Graphics();
	
	Menu_Draw( &s_transporter.menu );
}

/*
===============
TransporterMenu_Init
===============
*/
void TransporterMenu_Init(void)
{
	menubitmap_s	*holdSiteButton;
	int				x,y,pad,i;

	s_transporter.menu.nitems						= 0;
	s_transporter.menu.draw							= TransporterMenu_Draw;
	s_transporter.menu.key							= TransporterMenu_Key;
	s_transporter.menu.fullscreen					= qtrue;
	s_transporter.menu.wrapAround					= qtrue;
	s_transporter.menu.descX						= MENU_DESC_X;
	s_transporter.menu.descY						= MENU_DESC_Y;
	s_transporter.menu.titleX						= MENU_TITLE_X;
	s_transporter.menu.titleY						= MENU_TITLE_Y;
	s_transporter.menu.titleI						= MNT_CREDITSMENU_TITLE;
	s_transporter.menu.footNoteEnum					= MNT_CREDITS;

	s_transporter.quitMenu.generic.type				= MTYPE_BITMAP;      
	s_transporter.quitMenu.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_transporter.quitMenu.generic.x				= 70;
	s_transporter.quitMenu.generic.y				= 368;
	s_transporter.quitMenu.generic.name				= GRAPHIC_SQUARE;
	s_transporter.quitMenu.generic.id				= ID_QUIT;
	s_transporter.quitMenu.generic.callback			= M_Transporter_Event; 
	s_transporter.quitMenu.width					= MENU_BUTTON_MED_WIDTH;
	s_transporter.quitMenu.height					= MENU_BUTTON_MED_HEIGHT;
	s_transporter.quitMenu.color					= CT_DKORANGE;
	s_transporter.quitMenu.color2					= CT_LTORANGE;
	s_transporter.quitMenu.textX					= MENU_BUTTON_TEXT_X;
	s_transporter.quitMenu.textY					= MENU_BUTTON_TEXT_Y;
	s_transporter.quitMenu.textEnum					= MBT_PERSONALLOGRETURN;
	s_transporter.quitMenu.textcolor				= CT_BLACK;
	s_transporter.quitMenu.textcolor2				= CT_WHITE;

	s_transporter.engage.generic.type				= MTYPE_BITMAP;      
	s_transporter.engage.generic.flags				= QMF_GRAYED;
	s_transporter.engage.generic.x					= 316;
	s_transporter.engage.generic.y					= 295;
	s_transporter.engage.generic.name				= GRAPHIC_SQUARE;
	s_transporter.engage.generic.id					= ID_ENGAGE;
	s_transporter.engage.generic.callback			= M_Transporter_Event; 
	s_transporter.engage.width						= MENU_BUTTON_MED_WIDTH;
	s_transporter.engage.height						= MENU_BUTTON_MED_HEIGHT + 40;
	s_transporter.engage.color						= CT_DKORANGE;
	s_transporter.engage.color2						= CT_LTORANGE;
	s_transporter.engage.textX						= MENU_BUTTON_TEXT_X;
	s_transporter.engage.textY						= MENU_BUTTON_TEXT_Y;
	s_transporter.engage.textEnum					= MBT_ENGAGE;
	s_transporter.engage.textcolor					= CT_BLACK;
	s_transporter.engage.textcolor2					= CT_WHITE;

	Menu_AddItem( &s_transporter.menu,	&s_transporter.quitMenu );
	Menu_AddItem( &s_transporter.menu,	&s_transporter.engage );

	s_transporter.chosenSite = -1;

	holdSiteButton = &s_transporter.siteButton1;
	pad = 28;
	y =  70;
	x = 75;
	s_transporter.maxSites = 0;

	for (i=0;i<MAXSITEBUTTON;i++)
	{
		holdSiteButton->generic.type				= MTYPE_BITMAP;      
		holdSiteButton->generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
		holdSiteButton->generic.x				= x;
		holdSiteButton->generic.y				= y;
		holdSiteButton->generic.name			= GRAPHIC_BUTTONRIGHT;
		holdSiteButton->generic.id				= ID_SITEBUTTON1 + i;
		holdSiteButton->generic.callback		= M_Transporter_Event; 
		holdSiteButton->generic.statusbarfunc	= Transporter_StatusBar;
		holdSiteButton->width					= MENU_BUTTON_MED_WIDTH;
		holdSiteButton->height					= PROP_BIG_HEIGHT;
		holdSiteButton->color					= CT_DKAQUA;
		holdSiteButton->color2					= CT_LTAQUA;
		holdSiteButton->textX					= MENU_BUTTON_TEXT_X;
		holdSiteButton->textY					= MENU_BUTTON_TEXT_Y + 6;
		holdSiteButton->textPtr					= menu_normal_text[MNT_SITE];
		holdSiteButton->textcolor				= CT_BLACK;
		holdSiteButton->textcolor2				= CT_WHITE;
		holdSiteButton->textStyle				= UI_TINYFONT;

		holdSiteButton++;
		s_transporter.maxSites++;

		y += pad;
	}

	holdSiteButton = &s_transporter.siteButton1;
	for (i=0;i<MAXSITEBUTTON;i++)
	{
		if (transporterText[i].descriptionText)
		{
			Menu_AddItem( &s_transporter.menu,	holdSiteButton );
		}
		holdSiteButton++;
	}
}


/*
=================
UI_ParseTransporterText
=================
*/
static void UI_ParseTransporterText()
{
	char	*token;
	char *buffer,*holdPtr;
	int i,site,len;

	memset(transporterText,0,sizeof(transporterText));

	COM_BeginParseSession();

	buffer = TransporterText;
	i = 1;	// Zero is null string
	while ( buffer ) 
	{

		token = COM_ParseExt( &buffer, qtrue );

		if (!Q_strncmp(token,"SITE",4))
		{
			site = atoi(&token[4]);
			site--;

			// Get desc
			token = COM_ParseExt( &buffer, qtrue );	
			len = strlen(token);
			transporterText[site].descriptionText = buffer - (len+1); 
			holdPtr = transporterText[site].descriptionText + len;
			*holdPtr = NULL;

			// Get command
			token = COM_ParseExt( &buffer, qtrue );	
			len = strlen(token);
			transporterText[site].command = buffer - (len+1); 
			holdPtr = transporterText[site].command + len;
			*holdPtr = NULL;

		}
	}
}

/*
===============
UI_TransporterMenu_Cache
===============
*/
void UI_TransporterMenu_Cache (void)
{
	char	*buffer;
	char	filename[MAX_QPATH];
	int		len;

	s_transporter.figure = ui.R_RegisterShaderNoMip("menu/suit/transporter.tga");
	s_transporter.leftRound = ui.R_RegisterShaderNoMip("menu/common/halfroundl_24.tga");

	s_transporter.corner_ur_20_24 = ui.R_RegisterShaderNoMip("menu/common/corner_ur_20_24.tga");
	s_transporter.corner_lr_18_20 = ui.R_RegisterShaderNoMip("menu/common/corner_lr_18_20.tga");
	s_transporter.corner_ll_18_130 = ui.R_RegisterShaderNoMip("menu/common/corner_ll_18_130.tga");
	s_transporter.corner_lu_24_130 = ui.R_RegisterShaderNoMip("menu/common/corner_lu_24_130.tga");
	s_transporter.arrow = ui.R_RegisterShaderNoMip("menu/common/right_arrow.tga");
	s_transporter.slider = ui.R_RegisterShaderNoMip("menu/common/mon_bar.tga");
	s_transporter.corner_ll_8_47  = ui.R_RegisterShaderNoMip("menu/common/corner_ll_8_47.tga");
	s_transporter.corner_ll_18_47 = ui.R_RegisterShaderNoMip("menu/common/corner_ll_47_18.tga");

	UI_LanguageFilename("ext_data/sp_transporter","dat",filename);

	len = ui.FS_ReadFile( filename,(void **) &buffer  );

	if ( len == -1 ) 
	{
		ui.Error(ERR_FATAL, "UI_TransporterMenu_Cache : SP_TRANSPORTER.DAT file not found!\n");
		return;
	}

	if ( len > MAXTRANSPORTERTEXT ) 
	{
		ui.Error(ERR_FATAL, "UI_TransporterMenu_Cache : SP_TRANSPORTER.DAT too big!\n");
		return;
	}

	strncpy( TransporterText, buffer, sizeof( TransporterText ) - 1 );
	ui.FS_FreeFile( buffer );

}


/*
===============
TransporterMenu_LoadText
===============
*/
void TransporterMenu_LoadText (void)
{

	UI_TransporterMenu_Cache();

	UI_ParseTransporterText();
}

/*
===============
UI_TransporterMenu
===============
*/
void UI_TransporterMenu(void)
{
	uis.menusp = 0;

	ingameFlag = qtrue;	// true when in game menu is in use

	Mouse_Show();

	TransporterMenu_LoadText();

	TransporterMenu_Init(); 

	UI_PushMenu( &s_transporter.menu );

	Menu_AdjustCursor( &s_transporter.menu, 1 );	
}




#define ID_MAINTOPICBUTTON1		11
#define ID_MAINTOPICBUTTON2		12
#define ID_MAINTOPICBUTTON3		13
#define ID_MAINTOPICBUTTON4		14

#define ID_SUBTOPICBUTTON1		15
#define ID_SUBTOPICBUTTON2		16
#define ID_SUBTOPICBUTTON3		17
#define ID_SUBTOPICBUTTON4		18
#define ID_SUBTOPICBUTTON5		19
#define ID_SUBTOPICBUTTON6		20
#define ID_SUBTOPICBUTTON7		21
#define ID_SUBTOPICBUTTON8		22
#define ID_SUBTOPICBUTTON9		23
#define ID_SUBTOPICBUTTON10		24

#define MAXMAINTOPICDISPLAYED		4
#define MAXSUBTOPICDISPLAYED	8

#define MAXMAINTOPIC			25
#define MAXSUBTOPIC				25

static struct 
{
	menuframework_s menu;
	int				mainTopic;
	int				subTopic;
	int				maxMainTopics;
	int				topMainTopic;				// Index to Maintopic at top of viewing screen
	int				cntMainTopic;				// Count of topics loaded in
	int				topSubTopic;				// Index to Subtopic at top of viewing screen
	int				cntSubTopic[MAXSUBTOPIC];	// Count of subtopics loaded in
	int				menuType;					// 0 if library, 1 if astrometrics
	int				chosenButton;				// Hi-lite subtopic button
	qhandle_t		corner_ur_20_24;
	qhandle_t		corner_lr_18_20;
	qhandle_t		corner_ll_8_47;
	qhandle_t		corner_ll_18_47;
	qhandle_t		corner_lr_8_20;
	qhandle_t		leftRound;
	menubitmap_s	playerModel;
	menubitmap_s	quitMenu;
	menubitmap_s	topicArrowDown;
	menubitmap_s	topicArrowUp;
	menubitmap_s	subTopicArrowDown;
	menubitmap_s	subTopicArrowUp;
	menubitmap_s	computerVoice;
	menubitmap_s	mainTopicButton1;
	menubitmap_s	mainTopicButton2;
	menubitmap_s	mainTopicButton3;
	menubitmap_s	mainTopicButton4;
	menubitmap_s	subTopicButton1;
	menubitmap_s	subTopicButton2;
	menubitmap_s	subTopicButton3;
	menubitmap_s	subTopicButton4;
	menubitmap_s	subTopicButton5;
	menubitmap_s	subTopicButton6;
	menubitmap_s	subTopicButton7;
	menubitmap_s	subTopicButton8;
	menubitmap_s	subTopicButton9;
	menubitmap_s	subTopicButton10;
} s_library;

static struct 
{
	menuframework_s menu;
	float			timer;
	qhandle_t		cornerPic;
	int				menuType;		// 0 = library, 1 = astrometrics
} s_libraryaccessing;

#define MAXLIBRARYTEXT 50000
static char	LibraryText[MAXLIBRARYTEXT];

typedef struct 
{
	char	*mainTopic;						// Description of main topic
	char	*mainTopicDesc;					// Description of main topic desc
	char	*subTopic[MAXSUBTOPIC];			// Description of subtopics
	char	*subTopicDesc[MAXSUBTOPIC];		// Description of subtopic desc
	int		textY[MAXSUBTOPIC];				// Y starting point of text
	int		textWidth[MAXSUBTOPIC];			// X width of text 
	char	*text[MAXSUBTOPIC];				// Text
	char	*model[MAXSUBTOPIC];			// Model
	int		modelX[MAXSUBTOPIC];			// Model x location
	int		modelY[MAXSUBTOPIC];			// Model y location
	int		modelDistance[MAXSUBTOPIC];		// Model's distance from camera
	int		modelOriginY[MAXSUBTOPIC];		// Model change in Y origin
	int		modelYaw[MAXSUBTOPIC];			// Model's YAW from camera (0 to rotate)
	int		modelPitch[MAXSUBTOPIC];		// Model's PITCH 
	int		modelRoll[MAXSUBTOPIC];			// Model's ROLL 
	char	*modelLegs[MAXSUBTOPIC];
	char	*modelHead[MAXSUBTOPIC];
	char	*modelTorso[MAXSUBTOPIC];
	qhandle_t	modelHandle[MAXSUBTOPIC];	// Handle to model
	char	*shader[MAXSUBTOPIC];			// Shader
	qhandle_t	shaderHandle[MAXSUBTOPIC];	// Handle to shader
	int		shaderXpos[MAXSUBTOPIC];		// Shader x location
	int		shaderYpos[MAXSUBTOPIC];		// Shader y location
	int		shaderXlength[MAXSUBTOPIC];		// Shader x length
	int		shaderYlength[MAXSUBTOPIC];		// Shader y length
	char	*command[MAXSUBTOPIC];			// Command given when exiting the Astrometrics menu
	char	*sound[MAXSUBTOPIC];			// Wav file to play along with the text
	sfxHandle_t		soundHandle[MAXSUBTOPIC];	// Handle of wav file to play
} libraryText_t;


sfxHandle_t		nullSound;	

libraryText_t libraryText[MAXMAINTOPIC];


static void UI_Draw3DModel( float x, float y, float w, float h, qhandle_t model, vec3_t origin, vec3_t angles) 
{
	refdef_t		refdef;
	refEntity_t		ent;

	UI_AdjustFrom640( &x, &y, &w, &h );

	memset( &refdef, 0, sizeof( refdef ) );

	memset( &ent, 0, sizeof( ent ) );
	AnglesToAxis( angles, ent.axis );
	
	VectorCopy( origin, ent.origin );
	ent.hModel = model;
//	if (Cvar_VariableValue("r_dynamiclight") ) {
//		ent.renderfx = RF_LOWLIGHT|RF_NOSHADOW;		// keep it dark, and no stencil shadows
//	} else {
//		ent.renderfx = RF_NOSHADOW;		// no stencil shadows
//	}
	refdef.rdflags = RDF_NOWORLDMODEL;

	AxisClear( refdef.viewaxis );

	refdef.fov_x = 40;
	refdef.fov_y = 30;

	refdef.x = x;
	refdef.y = y;
	refdef.width = w;
	refdef.height = h;

	refdef.time = uis.realtime;

	ui.R_ClearScene();
	ui.R_AddRefEntityToScene( &ent );

	origin[0] -= 50;	// + = behind, - = in front
	origin[1] += 50;	// + = left, - = right
	origin[2] += 50;	// + = above, - = below
	ui.R_AddLightToScene( origin, 500, 1.0, 1.0, 1.0 );

	ui.R_RenderScene( &refdef );
}

void Controls_DrawPlayer( void *self );
void Controls_UpdateModel( int anim );

/*
================
UI_LibraryDrawMD3Model
================
*/
static void UI_LibraryDrawMD3Model(qhandle_t modelHandle,int x, int y,int modelDistance,int modelYaw,int modelPitch,int modelRoll,int modelOriginY)
{
	vec3_t	origin = {50,0,2};
	vec3_t	angles;

	if (modelDistance)
	{
		origin[0] = modelDistance;
	}
	else
	{
		origin[0] = 50;
	}

	if (modelOriginY)
	{
		origin[2] = modelOriginY;
	}
	else
	{
		origin[2] = 2;
	}

	angles[PITCH] = modelPitch;

	if (modelYaw)
	{
		angles[YAW]   = modelYaw;
	}
	else
	{
		angles[YAW]   = 20.0/300.0 * uis.realtime;
	}


	if (modelRoll)
	{
		angles[ROLL]  = modelRoll;
	}
	else
	{
		angles[ROLL]  = 0;
	}

	UI_Draw3DModel( x, y, 447, 305, modelHandle, origin, angles);

}

/*
================
UI_DrawLibraryMDRModel

================
*/
static void UI_DrawLibraryMDRModel(qhandle_t modelHandle,int x, int y)
{
	void *voidPtr;

	vec3_t	origin = {50,0,2};
	vec3_t	angles;

	angles[PITCH] = 0;
	angles[YAW]   = 20.0/300.0 * uis.realtime;
	angles[ROLL]  = 0;

	voidPtr = (void *) &s_library.playerModel;
	Controls_DrawPlayer(voidPtr);
	Controls_UpdateModel( 0 );

}

/*
=================
ClearLibraryDesc
=================
*/
void ClearLibraryDesc(void)
{
	int i;

	for (i=0;i<(LMG_MAX - LMG_BIO_DESC1);++i)
	{
		logmenu_graphics[LMG_BIO_DESC1 + i].type = MG_OFF;	// Turn off text
	}
}

/*
=================
ChangeLibraryDesc
=================
*/
void ChangeLibraryDesc(int id)
{
	int i,y,width;

	if (s_library.subTopic == id)
	{
		return;	//	Just hitting the same key again
	}
	else	// Turn off old description 
	{

		s_library.subTopic = id;

		for (i=0;i<(LMG_MAX - LMG_BIO_DESC1);++i)
		{
			logmenu_graphics[LMG_BIO_DESC1 + i].type = MG_OFF;	// Turn off text
		}
	}

	if (!libraryText[s_library.mainTopic].textY[s_library.subTopic])
	{
		y = 120;
	}
	else
	{
		y = libraryText[s_library.mainTopic].textY[s_library.subTopic];
	}

	for (i=LMG_BIO_DESC1; i<(LMG_BIO_DESC1 +LOG_MAXDESC );i++)
	{
		logmenu_graphics[i].x = 242;
		logmenu_graphics[i].y = y;
		y += 12;
	}

	if (!libraryText[s_library.mainTopic].textWidth[s_library.subTopic])
	{
		width = 360;
	}
	else
	{
		width = libraryText[s_library.mainTopic].textWidth[s_library.subTopic];
	}

	// Turn on description for new ID
	TurnOnLogDesc(libraryText[s_library.mainTopic].text[s_library.subTopic],
		width,libraryText[s_library.mainTopic].textY[s_library.subTopic]);

	if ((libraryText[s_library.mainTopic].soundHandle[s_library.subTopic]) && 
		(s_library.computerVoice.textEnum == MBT_COMPUTERVOICEON))
	{
		ui.S_StartLocalSound( libraryText[s_library.mainTopic].soundHandle[s_library.subTopic], CHAN_MENU2 );
	}

}

/*
=================
M_Transporter_Event
=================
*/
void M_Library_Event (void* ptr, int notification)
{
	int	id,i;
	menubitmap_s	*holdSubTopicButton,*holdMainTopicButton;

	if (notification != QM_ACTIVATED)
	{
		return;
	}

	id = ((menucommon_s*)ptr)->id;

	switch (id)
	{
		case ID_COMPUTERVOICE:

			if (s_library.computerVoice.textEnum == MBT_COMPUTERVOICEON)
			{
				s_library.computerVoice.textEnum = MBT_COMPUTERVOICEOFF;
				ui.S_StartLocalSound( nullSound, CHAN_MENU2 );
			}
			else
			{
				s_library.computerVoice.textEnum = MBT_COMPUTERVOICEON;
			}
			break;

		case ID_ARROW1UP:
			if ((s_library.topMainTopic - 1)  >= 0)
			{
				s_library.topMainTopic--;

				holdMainTopicButton = &s_library.mainTopicButton1;

				for (i=s_library.topMainTopic;i<(s_library.topMainTopic + MAXMAINTOPICDISPLAYED);i++)
				{
					holdMainTopicButton->textPtr	= libraryText[i].mainTopic;
					holdMainTopicButton++;
				}
			}

			// Show down arrow indicator
			s_library.topicArrowDown.generic.flags &= ~QMF_HIDDEN;

			// Show up arrow indicator???
			if (s_library.topMainTopic == 0)
			{
				s_library.topicArrowUp.generic.flags |= QMF_HIDDEN;
			}
			else
			{
				s_library.topicArrowUp.generic.flags &= ~QMF_HIDDEN;
			}

			break;

		case ID_ARROW1DOWN:
			if (((s_library.topMainTopic + 1) + MAXMAINTOPICDISPLAYED) <= s_library.cntMainTopic)
			{

				s_library.topMainTopic++;

				holdMainTopicButton = &s_library.mainTopicButton1;

				for (i=s_library.topMainTopic;i<(s_library.topMainTopic + MAXMAINTOPICDISPLAYED);i++)
				{
					holdMainTopicButton->textPtr	= libraryText[i].mainTopic;
					holdMainTopicButton++;
				}
			}

			// Show up arrow indicator
			s_library.topicArrowUp.generic.flags &= ~QMF_HIDDEN;

			// Show down arrow indicator???
			if ((s_library.topMainTopic + MAXMAINTOPICDISPLAYED) >= s_library.cntMainTopic)
			{
				s_library.topicArrowDown.generic.flags |= QMF_HIDDEN;
			}
			else
			{
				s_library.topicArrowDown.generic.flags &= ~QMF_HIDDEN;
			}

			break;

		case ID_ARROW2UP:
			if ((s_library.topSubTopic - 1)  >= 0)
			{
				s_library.topSubTopic--;

				holdSubTopicButton = &s_library.subTopicButton1;

				for (i=s_library.topSubTopic;i<(s_library.topSubTopic + MAXSUBTOPICDISPLAYED);i++)
				{
					holdSubTopicButton->textPtr	= libraryText[s_library.mainTopic].subTopic[i];
					holdSubTopicButton++;
				}

				// Hi-lite button when moved
				if ((s_library.chosenButton >= 0) &&
					(s_library.chosenButton <=MAXSUBTOPICDISPLAYED))	// Hi-lite new button
				{
					holdSubTopicButton = &s_library.subTopicButton1 + (s_library.chosenButton);
					holdSubTopicButton->textcolor	= CT_BLACK;
				}

				s_library.chosenButton++;
				if ((s_library.chosenButton >= 0) &&
					(s_library.chosenButton <=MAXSUBTOPICDISPLAYED))	// Hi-lite new button
				{
					holdSubTopicButton = &s_library.subTopicButton1 + (s_library.chosenButton);
					holdSubTopicButton->textcolor	= CT_VLTGOLD1;
				}
			}

			// Show down arrow indicator
			s_library.subTopicArrowDown.generic.flags &= ~QMF_HIDDEN;

			// Show up arrow indicator???
			if (s_library.topSubTopic == 0)
			{
				s_library.subTopicArrowUp.generic.flags |= QMF_HIDDEN;
			}
			else
			{
				s_library.subTopicArrowUp.generic.flags &= ~QMF_HIDDEN;				
			}

			break;

		case ID_ARROW2DOWN:
			if (((s_library.topSubTopic + 1) + MAXSUBTOPICDISPLAYED) <= s_library.cntSubTopic[s_library.mainTopic])
			{
				s_library.topSubTopic++;

				holdSubTopicButton = &s_library.subTopicButton1;

				for (i=s_library.topSubTopic;i<(s_library.topSubTopic + MAXSUBTOPICDISPLAYED);i++)
				{
					holdSubTopicButton->textPtr	= libraryText[s_library.mainTopic].subTopic[i];
					holdSubTopicButton++;
				}

				// Hi-lite button when moved
				if ((s_library.chosenButton >= 0) &&
					(s_library.chosenButton <=MAXSUBTOPICDISPLAYED))	// Hi-lite new button
				{
					holdSubTopicButton = &s_library.subTopicButton1 + (s_library.chosenButton);
					holdSubTopicButton->textcolor	= CT_BLACK;
				}

				s_library.chosenButton--;
				if ((s_library.chosenButton >= 0) &&
					(s_library.chosenButton <=MAXSUBTOPICDISPLAYED))	// Hi-lite new button
				{
					holdSubTopicButton = &s_library.subTopicButton1 + (s_library.chosenButton);
					holdSubTopicButton->textcolor	= CT_VLTGOLD1;
				}
			}

			// Show up arrow indicator
			s_library.subTopicArrowUp.generic.flags &= ~QMF_HIDDEN;

			// Show down arrow indicator???
			if ((s_library.topSubTopic + MAXSUBTOPICDISPLAYED) >= s_library.cntSubTopic[s_library.mainTopic])
			{
				s_library.subTopicArrowDown.generic.flags |= QMF_HIDDEN;
			}
			else
			{
				s_library.subTopicArrowDown.generic.flags &= ~QMF_HIDDEN;
			}

			break;

		case ID_QUIT:
			UI_PopMenu();
			if (libraryText[s_library.mainTopic].command[s_library.subTopic])
			{
				ui.Cmd_ExecuteText( EXEC_APPEND, libraryText[s_library.mainTopic].command[s_library.subTopic]);
			}

			ui.S_StartLocalSound( nullSound, CHAN_MENU2 );

			char			sex[16];
			ui.Cvar_VariableStringBuffer( "sex", sex, sizeof( sex ) );

			if (!Q_strncmp(sex,"male",4))
			{
				ui.Cvar_Set( "legsmodel", "hazard/default" );
				ui.Cvar_Set( "torsomodel","hazard/default" );
				ui.Cvar_Set( "headmodel", "munro/default" );
			} 
			else 
			{
				ui.Cvar_Set( "legsmodel", "hazardfemale/default" );
				ui.Cvar_Set( "torsomodel","hazardfemale/default" );
				ui.Cvar_Set( "headmodel", "alexandria/default" );
			}

			break;
		case ID_MAINTOPICBUTTON1:
		case ID_MAINTOPICBUTTON2:
		case ID_MAINTOPICBUTTON3:
		case ID_MAINTOPICBUTTON4:

			// Blackout all main topic buttons
			holdMainTopicButton = &s_library.mainTopicButton1;
			for (i=0;i<MAXMAINTOPICDISPLAYED;i++)
			{
				holdMainTopicButton->textcolor	= CT_BLACK;
				++holdMainTopicButton;
			}

			// Highlight chosen button
			holdMainTopicButton = &s_library.mainTopicButton1 + (id - ID_MAINTOPICBUTTON1);
			holdMainTopicButton->textcolor	= CT_LTGOLD1;



			// Blackout all subtopic buttons
			holdSubTopicButton = &s_library.subTopicButton1;
			for (i=0;i<MAXSUBTOPICDISPLAYED;i++)
			{
				holdSubTopicButton->textcolor	= CT_BLACK;
				++holdSubTopicButton;
			}

			// Highlight top subtopic button
			holdSubTopicButton = &s_library.subTopicButton1;
 			holdSubTopicButton->textcolor	= CT_VLTGOLD1;


			s_library.mainTopic = s_library.topMainTopic + (id-ID_MAINTOPICBUTTON1);

			holdSubTopicButton = &s_library.subTopicButton1;
			for (i=0;i<MAXSUBTOPICDISPLAYED;i++)
			{
				holdSubTopicButton->textPtr	= libraryText[s_library.mainTopic].subTopic[i];
				if (libraryText[s_library.mainTopic].subTopicDesc[i])
				{
					holdSubTopicButton->generic.flags	= QMF_HIGHLIGHT_IF_FOCUS;
				}
				else
				{
					holdSubTopicButton->generic.flags	= QMF_HIDDEN;
				}
				holdSubTopicButton++;
			}
			ClearLibraryDesc();

			s_library.subTopic = -1;
			s_library.topSubTopic = 0;
			ChangeLibraryDesc(s_library.topSubTopic);

			s_library.subTopicArrowUp.generic.flags = QMF_HIDDEN;
			s_library.topSubTopic = 0;
			// Show down arrow indicator???
			if ((s_library.topSubTopic + MAXSUBTOPICDISPLAYED) >= s_library.cntSubTopic[s_library.mainTopic])
			{
				s_library.subTopicArrowDown.generic.flags = QMF_HIDDEN;
			}
			else
			{
				s_library.subTopicArrowDown.generic.flags &= ~QMF_HIDDEN;
			}

			break;
		case ID_SUBTOPICBUTTON1:
		case ID_SUBTOPICBUTTON2:
		case ID_SUBTOPICBUTTON3:
		case ID_SUBTOPICBUTTON4:
		case ID_SUBTOPICBUTTON5:
		case ID_SUBTOPICBUTTON6:
		case ID_SUBTOPICBUTTON7:
		case ID_SUBTOPICBUTTON8:
		case ID_SUBTOPICBUTTON9:
		case ID_SUBTOPICBUTTON10:

			// Blackout all subtopic buttons
			holdSubTopicButton = &s_library.subTopicButton1;
			for (i=0;i<MAXSUBTOPICDISPLAYED;i++)
			{
				holdSubTopicButton->textcolor	= CT_BLACK;
				++holdSubTopicButton;
			}

			// Highlight chosen button
//			if (((id + s_library.topSubTopic - ID_SUBTOPICBUTTON1) > 0) &&
//				((id + s_library.topSubTopic - ID_SUBTOPICBUTTON1) < MAXSUBTOPICDISPLAYED))
			holdSubTopicButton = &s_library.subTopicButton1 + (id - ID_SUBTOPICBUTTON1);
			holdSubTopicButton->textcolor	= CT_VLTGOLD1;

			s_library.chosenButton = id - ID_SUBTOPICBUTTON1;

			ChangeLibraryDesc(s_library.topSubTopic + (id-ID_SUBTOPICBUTTON1));

//			if ((libraryText[s_library.mainTopic].soundHandle[s_library.subTopic]) && 
//				(s_library.computerVoice.textEnum == MBT_COMPUTERVOICEON))
//			{
//				ui.S_StartLocalSound( libraryText[s_library.mainTopic].soundHandle[s_library.subTopic], CHAN_MENU2 );
//			}

			// If there's a .mdr
			if (libraryText[s_library.mainTopic].modelLegs[s_library.subTopic])
			{
				ui.Cvar_Set( "legsmodel", libraryText[s_library.mainTopic].modelLegs[s_library.subTopic] );
			}

			if (libraryText[s_library.mainTopic].modelHead[s_library.subTopic])
			{
				ui.Cvar_Set( "headmodel", libraryText[s_library.mainTopic].modelHead[s_library.subTopic] );
			}

			if (libraryText[s_library.mainTopic].modelTorso[s_library.subTopic])
			{
				ui.Cvar_Set( "torsomodel", libraryText[s_library.mainTopic].modelTorso[s_library.subTopic] );
			}
		
			s_library.playerModel.generic.x	= libraryText[s_library.mainTopic].modelX[s_library.subTopic];
			s_library.playerModel.generic.y	= libraryText[s_library.mainTopic].modelY[s_library.subTopic];

			break;
	}
}

/*
=================
Library_StatusBar
=================
*/
static void Library_StatusBar(void *itemptr) 
{
	int		id;

	id = ((menucommon_s*)itemptr)->id;

	switch (id)
	{
		case ID_MAINTOPICBUTTON1:
		case ID_MAINTOPICBUTTON2:
		case ID_MAINTOPICBUTTON3:
		case ID_MAINTOPICBUTTON4:
			UI_DrawProportionalString( 320, 410, libraryText[id + s_library.topMainTopic-ID_MAINTOPICBUTTON1].mainTopicDesc, UI_CENTER|UI_SMALLFONT, colorTable[CT_VLTGOLD1]);
			break;
		case ID_SUBTOPICBUTTON1:
		case ID_SUBTOPICBUTTON2:
		case ID_SUBTOPICBUTTON3:
		case ID_SUBTOPICBUTTON4:
		case ID_SUBTOPICBUTTON5:
		case ID_SUBTOPICBUTTON6:
		case ID_SUBTOPICBUTTON7:
		case ID_SUBTOPICBUTTON8:
		case ID_SUBTOPICBUTTON9:
		case ID_SUBTOPICBUTTON10:

			UI_DrawProportionalString( 320, 410, 
			libraryText[s_library.mainTopic].subTopicDesc[id + s_library.topSubTopic - ID_SUBTOPICBUTTON1], 
			UI_CENTER|UI_SMALLFONT, colorTable[CT_VLTGOLD1]);
			break;
	}
}

/*
=================
LibraryMenu_Key
=================
*/
sfxHandle_t LibraryMenu_Key (int key)
{
	if ( key == K_ESCAPE ) 
	{
		ui.S_StartLocalSound( nullSound, CHAN_MENU2 );
		if (libraryText[s_library.mainTopic].command[s_library.subTopic])
		{
			ui.Cmd_ExecuteText( EXEC_APPEND, libraryText[s_library.mainTopic].command[s_library.subTopic]);
		}
	}

	return ( Menu_DefaultKey( &s_library.menu, key ) );
}

/*
=================
M_LibraryMenu_Graphics
=================
*/
void M_LibraryMenu_Graphics (void)
{
	int nameX,length,stardateX,textIndex;
	int	color1,color2,color3,color4,color5;

	// Draw the basic screen frame
	if (s_library.menuType == MENUTYPE_LIBRARY)
	{
		color1 = CT_VDKPURPLE2;
		color2 = CT_DKBROWN1;
		color3 = CT_DKPURPLE2;
		color4 = CT_LTPURPLE2;
		color5 = CT_VDKPURPLE3;
	}
	else
	{
		color1 = CT_VDKBLUE1;
		color2 = CT_DKGOLD1;
		color3 = CT_DKBLUE1;
		color4 = CT_LTBLUE1;
		color5 = CT_VDKBROWN1;
	}

	ui.R_SetColor( colorTable[color1]);
	UI_DrawHandlePic( 30,  24,16,   32, s_library.leftRound);

	// Left corners
	UI_DrawHandlePic( 181,  24, 32,   32, s_library.corner_ur_20_24);

	ui.R_SetColor( colorTable[color3]);
	UI_DrawHandlePic(  23, 354, -32,   32, s_library.corner_lr_18_20);
	UI_DrawHandlePic(  181, 157,  32,   16, s_library.corner_lr_8_20);
	UI_DrawHandlePic(   21, 153, -32,  -16, s_library.corner_lr_8_20);

	// Right corners
	ui.R_SetColor( colorTable[color1]);
	UI_DrawHandlePic( 202,  24, -32,  32, s_library.corner_ur_20_24);
	UI_DrawHandlePic( 204, 354, -32,  32, s_library.corner_lr_18_20);

	// Lower corners
	ui.R_SetColor( colorTable[color2]);
	UI_DrawHandlePic(  30, 386,  64, -16, s_library.corner_ll_8_47);
	UI_DrawHandlePic(  30, 425, 128,  64, s_library.corner_ll_18_47);

	stardateX  = 50;
	UI_DrawProportionalString( stardateX , 24,
		menu_normal_text[MNT_TOPICS],  UI_BIGFONT , colorTable[color4]);	

	length = UI_ProportionalStringWidth( menu_normal_text[MNT_TOPICS],UI_BIGFONT);
	ui.R_SetColor( colorTable[color1]);
	UI_DrawHandlePic( stardateX + 4 + length,  24, (196 - (stardateX + 4 + length)), 24, uis.whiteShader);

	// Left side
	ui.R_SetColor( colorTable[color1]);
	UI_DrawHandlePic( 184,  36,  20,  23, uis.whiteShader);		// Side line1

	ui.R_SetColor( colorTable[color5]);
	UI_DrawHandlePic( 184,  62,  20,  18, uis.whiteShader);		// Side line2
	UI_DrawHandlePic( 184,  83,  20,  42, uis.whiteShader);		// Side line3
	UI_DrawHandlePic( 184, 128,  20,  18, uis.whiteShader);		// Side line4

	ui.R_SetColor( colorTable[color3]);
	UI_DrawHandlePic( 184, 149,  20,  13, uis.whiteShader);		// Side line3
	UI_DrawHandlePic(  37, 159, 162,   8, uis.whiteShader);		// Bar across

	UI_DrawHandlePic(  30, 165,  20,  12, uis.whiteShader);		// Lower Left side
	UI_DrawHandlePic(  30, 180,  20,  18, uis.whiteShader);		// Lower Left side2
	UI_DrawHandlePic(  30, 201,  20, 130, uis.whiteShader);		// Lower Left side3
	UI_DrawHandlePic(  30, 334,  20,  18, uis.whiteShader);		// Lower Left side4
	UI_DrawHandlePic(  30, 355,  20,  11, uis.whiteShader);		// Lower Left side5

	UI_DrawHandlePic(  51, 368,   8,  18, uis.whiteShader);		// Bottom
	UI_DrawHandlePic( 195, 368,   7,  18, uis.whiteShader);		// Bottom2

	// Right side
	nameX = 592;

	if (s_library.menuType == MENUTYPE_LIBRARY)
	{
		textIndex = MNT_LIBRARYSTATION;
	}
	else if (s_library.menuType == MENUTYPE_RECIPES)
	{
		textIndex = MNT_RECIPES;
	}
	else if (s_library.menuType == MENUTYPE_SOCIALCALENDAR)
	{
		textIndex = MNT_SOCIALCALENDAR;
	}
	else
	{
		textIndex = MNT_ASTROMETRICS;
	}

	UI_DrawProportionalString( nameX, 24,
		menu_normal_text[textIndex],   UI_BIGFONT | UI_RIGHT, colorTable[color4]);	
	length = UI_ProportionalStringWidth( menu_normal_text[textIndex],UI_BIGFONT);

	ui.R_SetColor( colorTable[color1]);
	UI_DrawHandlePic( nameX + 4,  24,-16,   32, s_library.leftRound);
	UI_DrawHandlePic( 218,  24, (nameX - (length +4)) - 218, 24, uis.whiteShader);
	
	UI_DrawProportionalString( 240, 58,va("%s / %s", libraryText[s_library.mainTopic].mainTopic,libraryText[s_library.mainTopic].subTopic[s_library.subTopic]),
	   UI_SMALLFONT , colorTable[CT_VLTGOLD1]);	

	ui.R_SetColor( colorTable[color1]);
	UI_DrawHandlePic( 211,  32,  20, 22, uis.whiteShader);		// Side line
	ui.R_SetColor( colorTable[color2]);
	UI_DrawHandlePic( 211,  57,  20, 53, uis.whiteShader);		// Side line2
	UI_DrawHandlePic( 211,  113,  20, 248, uis.whiteShader);	// Side line3

	ui.R_SetColor( colorTable[color1]);
	UI_DrawHandlePic( 218, 368, 394,  18, uis.whiteShader);		// Bottom line

	// Bottom
	ui.R_SetColor( colorTable[color2]);
	UI_DrawHandlePic(  33, 391, 578,   8, uis.whiteShader);		// Top line
	UI_DrawHandlePic(  30, 396,  47,  39, uis.whiteShader);		// Side line
	UI_DrawHandlePic(  54, 438,  39,  18, uis.whiteShader);		// Bottom line 1
	ui.R_SetColor( colorTable[color1]);
	UI_DrawHandlePic(  96, 438, 268,  18, uis.whiteShader);		// Bottom line 2
	UI_DrawHandlePic( 367, 438, 245,  18, uis.whiteShader);		// Bottom line 3
	
	if (libraryText[s_library.mainTopic].model[s_library.subTopic])
	{
		UI_LibraryDrawMD3Model(libraryText[s_library.mainTopic].modelHandle[s_library.subTopic],
			libraryText[s_library.mainTopic].modelX[s_library.subTopic],
			libraryText[s_library.mainTopic].modelY[s_library.subTopic],
			libraryText[s_library.mainTopic].modelDistance[s_library.subTopic],
			libraryText[s_library.mainTopic].modelYaw[s_library.subTopic],
			libraryText[s_library.mainTopic].modelPitch[s_library.subTopic],
			libraryText[s_library.mainTopic].modelRoll[s_library.subTopic],
			libraryText[s_library.mainTopic].modelOriginY[s_library.subTopic]);
	}

	if ((libraryText[s_library.mainTopic].modelHead[s_library.subTopic]) ||
		(libraryText[s_library.mainTopic].modelTorso[s_library.subTopic]) ||
		(libraryText[s_library.mainTopic].modelLegs[s_library.subTopic]))
	{
		UI_DrawLibraryMDRModel(libraryText[s_library.mainTopic].modelHandle[s_library.subTopic],
			libraryText[s_library.mainTopic].modelX[s_library.subTopic],
			libraryText[s_library.mainTopic].modelY[s_library.subTopic]);
	}

	if (libraryText[s_library.mainTopic].shaderHandle[s_library.subTopic])
	{
		ui.R_SetColor( colorTable[CT_WHITE]);
		UI_DrawHandlePic( libraryText[s_library.mainTopic].shaderXpos[s_library.subTopic],
			libraryText[s_library.mainTopic].shaderYpos[s_library.subTopic],  
			libraryText[s_library.mainTopic].shaderXlength[s_library.subTopic],
			libraryText[s_library.mainTopic].shaderYlength[s_library.subTopic], 
			libraryText[s_library.mainTopic].shaderHandle[s_library.subTopic]);
	}

	LogMenu_Blinkies();

	UI_PrintMenuGraphics(logmenu_graphics,LMG_MAX);

}

/*
===============
LibraryMenu_Draw
===============
*/
void LibraryMenu_Draw(void)
{
	M_LibraryMenu_Graphics();
	
	Menu_Draw( &s_library.menu );
}

/*
===============
LibraryMenu_Init
===============
*/
void LibraryMenu_Init(void)
{
	menubitmap_s	*holdMainTopicButton,*holdSubTopicButton;
	int				x,y,pad,i;

	s_library.menu.nitems						= 0;
	s_library.menu.draw							= LibraryMenu_Draw;
	s_library.menu.key							= LibraryMenu_Key;
	s_library.menu.fullscreen					= qtrue;
	s_library.menu.wrapAround					= qtrue;
	s_library.menu.descX						= MENU_DESC_X;
	s_library.menu.descY						= MENU_DESC_Y;
	s_library.menu.titleX						= MENU_TITLE_X;
	s_library.menu.titleY						= MENU_TITLE_Y;
	s_library.menu.titleI						= MNT_CREDITSMENU_TITLE;
	s_library.menu.footNoteEnum					= MNT_CREDITS;

	s_library.quitMenu.generic.type				= MTYPE_BITMAP;      
	s_library.quitMenu.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_library.quitMenu.generic.x				= 62;
	s_library.quitMenu.generic.y				= 368;
	s_library.quitMenu.generic.name				= GRAPHIC_SQUARE;
	s_library.quitMenu.generic.id				= ID_QUIT;
	s_library.quitMenu.generic.callback			= M_Library_Event; 
	s_library.quitMenu.width					= MENU_BUTTON_MED_WIDTH;
	s_library.quitMenu.height					= MENU_BUTTON_MED_HEIGHT;
	s_library.quitMenu.color					= CT_DKORANGE;
	s_library.quitMenu.color2					= CT_LTORANGE;
	s_library.quitMenu.textX					= MENU_BUTTON_TEXT_X;
	s_library.quitMenu.textY					= MENU_BUTTON_TEXT_Y;
	s_library.quitMenu.textEnum					= MBT_PERSONALLOGRETURN;
	s_library.quitMenu.textcolor				= CT_BLACK;
	s_library.quitMenu.textcolor2				= CT_WHITE;

	s_library.topicArrowUp.generic.type					= MTYPE_BITMAP;      
	s_library.topicArrowUp.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS | QMF_HIDDEN;
	s_library.topicArrowUp.generic.x					= 185;
	s_library.topicArrowUp.generic.y					= 64;
	s_library.topicArrowUp.generic.name					= "menu/common/arrow_up_16.tga";
	s_library.topicArrowUp.generic.id					= ID_ARROW1UP;
	s_library.topicArrowUp.generic.callback				= M_Library_Event; 
	s_library.topicArrowUp.width						= 18;
	s_library.topicArrowUp.height						= 18;
	s_library.topicArrowUp.color						= CT_DKORANGE;
	s_library.topicArrowUp.color2						= CT_LTORANGE;

	s_library.topicArrowDown.generic.type				= MTYPE_BITMAP;      
	s_library.topicArrowDown.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS;
	s_library.topicArrowDown.generic.x					= 185;
	s_library.topicArrowDown.generic.y					= 130;
	s_library.topicArrowDown.generic.name				= "menu/common/arrow_dn_16.tga";
	s_library.topicArrowDown.generic.id					= ID_ARROW1DOWN;
	s_library.topicArrowDown.generic.callback			= M_Library_Event; 
	s_library.topicArrowDown.width						= 18;
	s_library.topicArrowDown.height						= 18;
	s_library.topicArrowDown.color						= CT_DKORANGE;
	s_library.topicArrowDown.color2						= CT_LTORANGE;

	s_library.subTopicArrowUp.generic.type				= MTYPE_BITMAP;      
	s_library.subTopicArrowUp.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS | QMF_HIDDEN;
	s_library.subTopicArrowUp.generic.x					= 31;
	s_library.subTopicArrowUp.generic.y					= 182;
	s_library.subTopicArrowUp.generic.name				= "menu/common/arrow_up_16.tga";
	s_library.subTopicArrowUp.generic.id				= ID_ARROW2UP;
	s_library.subTopicArrowUp.generic.callback			= M_Library_Event; 
	s_library.subTopicArrowUp.width						= 18;
	s_library.subTopicArrowUp.height					= 18;
	s_library.subTopicArrowUp.color						= CT_DKORANGE;
	s_library.subTopicArrowUp.color2					= CT_LTORANGE;

	s_library.subTopicArrowDown.generic.type			= MTYPE_BITMAP;      
	s_library.subTopicArrowDown.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_library.subTopicArrowDown.generic.x				= 31;
	s_library.subTopicArrowDown.generic.y				= 336;
	s_library.subTopicArrowDown.generic.name			= "menu/common/arrow_dn_16.tga";
	s_library.subTopicArrowDown.generic.id				= ID_ARROW2DOWN;
	s_library.subTopicArrowDown.generic.callback		= M_Library_Event; 
	s_library.subTopicArrowDown.width					= 18;
	s_library.subTopicArrowDown.height					= 18;
	s_library.subTopicArrowDown.color					= CT_DKORANGE;
	s_library.subTopicArrowDown.color2					= CT_LTORANGE;

	s_library.computerVoice.generic.type			= MTYPE_BITMAP;      
	s_library.computerVoice.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_library.computerVoice.generic.x				= 360;
	s_library.computerVoice.generic.y				= 368;
	s_library.computerVoice.generic.name			= GRAPHIC_SQUARE;
	s_library.computerVoice.generic.id				= ID_COMPUTERVOICE;
	s_library.computerVoice.generic.callback		= M_Library_Event; 
	s_library.computerVoice.width					= MENU_BUTTON_MED_WIDTH + 30;
	s_library.computerVoice.height					= MENU_BUTTON_MED_HEIGHT;
	s_library.computerVoice.color					= CT_DKORANGE;
	s_library.computerVoice.color2					= CT_LTORANGE;
	s_library.computerVoice.textEnum				= MBT_COMPUTERVOICEON;
	s_library.computerVoice.textX					= MENU_BUTTON_TEXT_X + 10;

	Menu_AddItem( &s_library.menu,	&s_library.quitMenu );
	Menu_AddItem( &s_library.menu,	&s_library.topicArrowUp );
	Menu_AddItem( &s_library.menu,	&s_library.topicArrowDown );
	Menu_AddItem( &s_library.menu,	&s_library.subTopicArrowUp );
	Menu_AddItem( &s_library.menu,	&s_library.subTopicArrowDown );
	Menu_AddItem( &s_library.menu,	&s_library.computerVoice );

	holdMainTopicButton = &s_library.mainTopicButton1;
	pad = 22;
	y =  62;
	x = 30;
	s_library.maxMainTopics = 0;
	s_library.topMainTopic = 0;		// Top of Main topics starts at index 0

	for (i=0;i<MAXMAINTOPICDISPLAYED;i++)
	{
		holdMainTopicButton->generic.type				= MTYPE_BITMAP;      
		holdMainTopicButton->generic.flags				= QMF_HIGHLIGHT_IF_FOCUS;
		holdMainTopicButton->generic.x					= x;
		holdMainTopicButton->generic.y					= y;
		holdMainTopicButton->generic.name				= GRAPHIC_BUTTONRIGHT;
		holdMainTopicButton->generic.id					= ID_MAINTOPICBUTTON1 + i;
		holdMainTopicButton->generic.callback			= M_Library_Event; 
		holdMainTopicButton->generic.statusbarfunc		= Library_StatusBar;
		holdMainTopicButton->width						= MENU_BUTTON_MED_WIDTH + 20;
		holdMainTopicButton->height						= MENU_BUTTON_MED_HEIGHT;
		holdMainTopicButton->color						= CT_DKAQUA;
		holdMainTopicButton->color2						= CT_LTAQUA;
		holdMainTopicButton->textX						= MENU_BUTTON_TEXT_X;
		holdMainTopicButton->textY						= MENU_BUTTON_TEXT_Y;
		holdMainTopicButton->textPtr					= libraryText[i].mainTopic;
		holdMainTopicButton->textcolor					= CT_BLACK;
		holdMainTopicButton->textcolor2					= CT_WHITE;
		holdMainTopicButton->textStyle					= UI_SMALLFONT;

		holdMainTopicButton++;
		s_library.maxMainTopics++;

		y += pad;
	}

	holdMainTopicButton = &s_library.mainTopicButton1;
	for (i=0;i<MAXMAINTOPICDISPLAYED;i++)
	{
		if (libraryText[i].mainTopic)
		{
			Menu_AddItem( &s_library.menu,	holdMainTopicButton );
		}
		holdMainTopicButton++;
	}

	// Make top main topic button text gold
	holdMainTopicButton = &s_library.mainTopicButton1;
	holdMainTopicButton->textcolor	= CT_LTGOLD1;

	s_library.topSubTopic = 0;	// Top of subtopics starts at index 0

	holdSubTopicButton = &s_library.subTopicButton1;
	pad = 22;
	y =  180;
	x = 62;

	for (i=0;i<MAXSUBTOPICDISPLAYED;i++)
	{
		holdSubTopicButton->generic.type				= MTYPE_BITMAP;      
		holdSubTopicButton->generic.flags				= QMF_HIGHLIGHT_IF_FOCUS;
		holdSubTopicButton->generic.x					= x;
		holdSubTopicButton->generic.y					= y;
		holdSubTopicButton->generic.name				= GRAPHIC_SQUARE;
		holdSubTopicButton->generic.id					= ID_SUBTOPICBUTTON1 + i;
		holdSubTopicButton->generic.callback			= M_Library_Event; 
		holdSubTopicButton->generic.statusbarfunc		= Library_StatusBar;
		holdSubTopicButton->width						= MENU_BUTTON_MED_WIDTH;
		holdSubTopicButton->height						= MENU_BUTTON_MED_HEIGHT;
		holdSubTopicButton->color						= CT_DKAQUA;
		holdSubTopicButton->color2						= CT_LTAQUA;
		holdSubTopicButton->textX						= MENU_BUTTON_TEXT_X;
		holdSubTopicButton->textY						= MENU_BUTTON_TEXT_Y;
		holdSubTopicButton->textPtr						= libraryText[0].subTopic[i];
		holdSubTopicButton->textcolor					= CT_BLACK;
		holdSubTopicButton->textcolor2					= CT_WHITE;
		holdSubTopicButton->textStyle					= UI_SMALLFONT;

		if (libraryText[0].subTopicDesc[i])
		{
			holdSubTopicButton->generic.flags	= QMF_HIGHLIGHT_IF_FOCUS;
		}
		else
		{
			holdSubTopicButton->generic.flags	= QMF_HIDDEN;
		}

		Menu_AddItem( &s_library.menu,	holdSubTopicButton );

		holdSubTopicButton++;

		y += pad;
	}

	// Make top subtopic button text gold
	holdSubTopicButton = &s_library.subTopicButton1;
	holdSubTopicButton->textcolor	= CT_VLTGOLD1;

	y = 120;
	for (i=LMG_BIO_DESC1; i<(LMG_BIO_DESC1 +LOG_MAXDESC );i++)
	{
		logmenu_graphics[i].x = 242;
		logmenu_graphics[i].y = y;
		y += 12;
	}

	s_library.mainTopic = 0;
	s_library.subTopic = 0;

	ClearLibraryDesc();

	s_library.playerModel.generic.type			= MTYPE_BITMAP;
	s_library.playerModel.generic.flags			= QMF_INACTIVE;
	s_library.playerModel.generic.x				= 430;
	s_library.playerModel.generic.y				= 95;
	s_library.playerModel.width					= 32*7.6;
	s_library.playerModel.height				= 56*7.6;

	// Show down arrow indicator???
	if ((s_library.topMainTopic + MAXMAINTOPICDISPLAYED) >= s_library.cntMainTopic)
	{
		s_library.topicArrowDown.generic.flags |= QMF_HIDDEN;
	}
	else
	{
		s_library.topicArrowDown.generic.flags &= ~QMF_HIDDEN;
	}

	// Show down arrow indicator???
	if ((s_library.topSubTopic + MAXSUBTOPICDISPLAYED) >= s_library.cntSubTopic[s_library.mainTopic])
	{
		s_library.subTopicArrowDown.generic.flags |= QMF_HIDDEN;
	}
	else
	{
		s_library.subTopicArrowDown.generic.flags &= ~QMF_HIDDEN;
	}

	s_library.subTopic = -1;
	ChangeLibraryDesc(0);
}

/*
=================
UI_ParseLibraryText
=================
*/
static void UI_ParseLibraryText()
{
	char	*token;
	char *buffer,*holdPtr;
	int i,len;

	memset(libraryText,0,sizeof(libraryText));

	s_library.mainTopic = 0;
	s_library.subTopic = 0;
	s_library.cntMainTopic=0;

	COM_BeginParseSession();

	buffer = LibraryText;
	i = 1;	// Zero is null string
	while ( buffer ) 
	{
		token = COM_ParseExt( &buffer, qtrue );

		if (!Q_strncmp(token,"MAINTOPIC",9))
		{
			s_library.mainTopic = atoi(&token[9]);
			s_library.mainTopic--;

			// Get main topic 
			token = COM_ParseExt( &buffer, qtrue );	
			len = strlen(token);
			libraryText[s_library.mainTopic].mainTopic = buffer - (len+1); 
			holdPtr = libraryText[s_library.mainTopic].mainTopic + len;
			*holdPtr = NULL;

			s_library.cntSubTopic[s_library.mainTopic] = 0;
			s_library.cntMainTopic++;
		}
		else if (!Q_strncmp(token,"MAINDESC",8))
		{
			// Get main topic desc
			token = COM_ParseExt( &buffer, qtrue );	
			len = strlen(token);
			libraryText[s_library.mainTopic].mainTopicDesc = buffer - (len+1); 
			holdPtr = libraryText[s_library.mainTopic].mainTopicDesc + len;
			*holdPtr = NULL;
		}
		else if (!Q_strncmp(token,"SUBTOPIC",8))
		{
			s_library.subTopic = atoi(&token[8]);
			s_library.subTopic--;

			// Get main topic desc
			token = COM_ParseExt( &buffer, qtrue );	
			len = strlen(token);
			libraryText[s_library.mainTopic].subTopic[s_library.subTopic] = buffer - (len+1); 
			holdPtr = libraryText[s_library.mainTopic].subTopic[s_library.subTopic] + len;
			*holdPtr = NULL;

			s_library.cntSubTopic[s_library.mainTopic]++;

		}
		else if (!Q_strncmp(token,"SUBDESC",7))
		{
			// Get main topic desc
			token = COM_ParseExt( &buffer, qtrue );	
			len = strlen(token);
			libraryText[s_library.mainTopic].subTopicDesc[s_library.subTopic] = buffer - (len+1); 
			holdPtr = libraryText[s_library.mainTopic].subTopicDesc[s_library.subTopic] + len;
			*holdPtr = NULL;
		}
		else if (!Q_strncmp(token,"TEXTWIDTH",9))
		{
			// Get main topic desc
			token = COM_ParseExt( &buffer, qtrue );	
			libraryText[s_library.mainTopic].textWidth[s_library.subTopic] = atoi(token);
		}
		else if (!Q_strncmp(token,"TEXT",4))
		{
			// Get main topic desc
			token = COM_ParseExt( &buffer, qtrue );	
			len = strlen(token);
			libraryText[s_library.mainTopic].text[s_library.subTopic] = buffer - (len+1); 
			holdPtr = libraryText[s_library.mainTopic].text[s_library.subTopic] + len;
			*holdPtr = NULL;
		}
		else if (!Q_strncmp(token,"STARTTEXTY",10))
		{
			token = COM_ParseExt( &buffer, qtrue );	
			libraryText[s_library.mainTopic].textY[s_library.subTopic] = atoi(token);
		}
		else if (!Q_strncmp(token,"MODELLEGS",9))
		{
			token = COM_ParseExt( &buffer, qtrue );	
			len = strlen(token);
			libraryText[s_library.mainTopic].modelLegs[s_library.subTopic] = buffer - (len+1); 
			holdPtr = libraryText[s_library.mainTopic].modelLegs[s_library.subTopic] + len;
			*holdPtr = NULL;
		}
		else if (!Q_strncmp(token,"MODELHEAD",9))
		{
			token = COM_ParseExt( &buffer, qtrue );	
			len = strlen(token);
			libraryText[s_library.mainTopic].modelHead[s_library.subTopic] = buffer - (len+1); 
			holdPtr = libraryText[s_library.mainTopic].modelHead[s_library.subTopic] + len;
			*holdPtr = NULL;
		}
		else if (!Q_strncmp(token,"MODELTORSO",10))
		{
			token = COM_ParseExt( &buffer, qtrue );	
			len = strlen(token);
			libraryText[s_library.mainTopic].modelTorso[s_library.subTopic] = buffer - (len+1); 
			holdPtr = libraryText[s_library.mainTopic].modelTorso[s_library.subTopic] + len;
			*holdPtr = NULL;
		}
		else if (!Q_strncmp(token,"MODELX",6))
		{
			token = COM_ParseExt( &buffer, qtrue );	
			libraryText[s_library.mainTopic].modelX[s_library.subTopic] = atoi(token);
		}
		else if (!Q_strncmp(token,"MODELYAW",8))
		{
			token = COM_ParseExt( &buffer, qtrue );	
			libraryText[s_library.mainTopic].modelYaw[s_library.subTopic] = atoi(token);
		}
		else if (!Q_strncmp(token,"MODELPITCH",10))
		{
			token = COM_ParseExt( &buffer, qtrue );	
			libraryText[s_library.mainTopic].modelPitch[s_library.subTopic] = atoi(token);
		}
		else if (!Q_strncmp(token,"MODELROLL",9))
		{
			token = COM_ParseExt( &buffer, qtrue );	
			libraryText[s_library.mainTopic].modelRoll[s_library.subTopic] = atoi(token);
		}
		else if (!Q_strncmp(token,"MODELY",6))
		{
			token = COM_ParseExt( &buffer, qtrue );	
			libraryText[s_library.mainTopic].modelY[s_library.subTopic] = atoi(token);
		}
		else if (!Q_strncmp(token,"MODELDISTANCE",13))	// Model's distance from camera
		{
			token = COM_ParseExt( &buffer, qtrue );	
			libraryText[s_library.mainTopic].modelDistance[s_library.subTopic] = atoi(token);
		}
		else if (!Q_strncmp(token,"MODELORIGINY",12))	// Model's change in Y origin
		{
			token = COM_ParseExt( &buffer, qtrue );	
			libraryText[s_library.mainTopic].modelOriginY[s_library.subTopic] = atoi(token);
		}
		else if (!Q_strncmp(token,"MODEL",5))
		{
			// Get main topic desc
			token = COM_ParseExt( &buffer, qtrue );	
			len = strlen(token);
			libraryText[s_library.mainTopic].model[s_library.subTopic] = buffer - (len+1); 
			holdPtr = libraryText[s_library.mainTopic].model[s_library.subTopic] + len;
			*holdPtr = NULL;

			libraryText[s_library.mainTopic].modelHandle[s_library.subTopic] = 
				ui.R_RegisterModel(libraryText[s_library.mainTopic].model[s_library.subTopic]);
		}
		else if (!Q_strncmp(token,"SHADERXPOS",10))
		{
			token = COM_ParseExt( &buffer, qtrue );	
			libraryText[s_library.mainTopic].shaderXpos[s_library.subTopic] = atoi(token);
		}
		else if (!Q_strncmp(token,"SHADERYPOS",10))
		{
			token = COM_ParseExt( &buffer, qtrue );	
			libraryText[s_library.mainTopic].shaderYpos[s_library.subTopic] = atoi(token);
		}
		else if (!Q_strncmp(token,"SHADERXLENGTH",13))
		{
			token = COM_ParseExt( &buffer, qtrue );	
			libraryText[s_library.mainTopic].shaderXlength[s_library.subTopic] = atoi(token);
		}
		else if (!Q_strncmp(token,"SHADERYLENGTH",13))
		{
			token = COM_ParseExt( &buffer, qtrue );	
			libraryText[s_library.mainTopic].shaderYlength[s_library.subTopic] = atoi(token);
		}
		else if (!Q_strncmp(token,"SHADER",6))
		{
			// Get main topic desc
			token = COM_ParseExt( &buffer, qtrue );	
			len = strlen(token);
			libraryText[s_library.mainTopic].shader[s_library.subTopic] = buffer - (len+1); 
			holdPtr = libraryText[s_library.mainTopic].shader[s_library.subTopic] + len;
			*holdPtr = NULL;

			libraryText[s_library.mainTopic].shaderHandle[s_library.subTopic] = 
				ui.R_RegisterShaderNoMip(libraryText[s_library.mainTopic].shader[s_library.subTopic]);
		}
		else if (!Q_strncmp(token,"COMMAND",7))
		{
			token = COM_ParseExt( &buffer, qtrue );	
			len = strlen(token);
			libraryText[s_library.mainTopic].command[s_library.subTopic] = buffer - (len+1); 
			holdPtr = libraryText[s_library.mainTopic].command[s_library.subTopic] + len;
			*holdPtr = NULL;
		}
		else if (!Q_strncmp(token,"SOUND",5))
		{
			token = COM_ParseExt( &buffer, qtrue );	
			len = strlen(token);
			libraryText[s_library.mainTopic].sound[s_library.subTopic] = buffer - (len+1); 
			holdPtr = libraryText[s_library.mainTopic].sound[s_library.subTopic] + len;
			*holdPtr = NULL;
		}
	}
}

/*
===============
UI_LibraryMenu_Cache
===============
*/
void UI_LibraryMenu_Cache (void)
{
	char	*buffer,*filePtr;
	char	filename[MAX_QPATH];
	int		len,i,i2;

	s_library.leftRound = ui.R_RegisterShaderNoMip("menu/common/halfroundl_24.tga");
	s_library.corner_ur_20_24 = ui.R_RegisterShaderNoMip("menu/common/corner_ur_20_24.tga");
	s_library.corner_lr_18_20 = ui.R_RegisterShaderNoMip("menu/common/corner_lr_18_20.tga");
	s_library.corner_ll_18_47 = ui.R_RegisterShaderNoMip("menu/common/corner_ll_47_18.tga");
	s_library.corner_ll_8_47  = ui.R_RegisterShaderNoMip("menu/common/corner_ll_8_47.tga");
	s_library.corner_lr_8_20  = ui.R_RegisterShaderNoMip("menu/common/corner_lr_8_20.tga");

	if (s_library.menuType == MENUTYPE_LIBRARY)
	{
		UI_LanguageFilename("ext_data/sp_library","dat",filename);
		filePtr = "SP_LIBRARY.DAT";
	}
	else
	{
		UI_LanguageFilename("ext_data/sp_astrometrics","dat",filename);
		filePtr = "SP_ASTROMETRICS.DAT";
	}

	len = ui.FS_ReadFile( filename,(void **) &buffer  );

	if ( len == -1 ) 
	{
		ui.Error(ERR_FATAL, va("UI_LibraryMenu_Cache : %s file not found!\n",filePtr));
		return;
	}

	if ( len > MAXLIBRARYTEXT ) 
	{
		ui.Error(ERR_FATAL, va("UI_LibraryMenu_Cache : %s too big! Max:%d  Current Length:%d\n",filePtr,MAXLIBRARYTEXT,len));
		return;
	}

	strncpy( LibraryText, buffer, sizeof( LibraryText ) - 1 );
	ui.FS_FreeFile( buffer );

	UI_ParseLibraryText();

	nullSound =	ui.S_RegisterSound("sound/null.wav");

	for (i=0;i<=s_library.mainTopic;i++)		
	{
		for (i2=0;i2<s_library.cntSubTopic[i];i2++)
		{
			if (libraryText[i].sound[i2])
			{
				libraryText[i].soundHandle[i2] = ui.S_RegisterSound(libraryText[i].sound[i2]);
			}
		}
	}
}

/*
===============
UI_Library_SpecialCache
===============
*/
void UI_Library_SpecialCache(void)
{
	s_library.menuType = MENUTYPE_LIBRARY;
	UI_LibraryMenu_Cache();
}

/*
===============
UI_Library_SpecialCache
===============
*/
void UI_Astrometrics_SpecialCache(void)
{
	s_library.menuType = MENUTYPE_ASTROMETRICS;
	UI_LibraryMenu_Cache();
}

/*
===============
LibraryMenu_LoadText
===============
*/
void LibraryMenu_LoadText (void)
{
	UI_LibraryMenu_Cache();
}

/*
===============
UI_LibraryMenu
===============
*/
void UI_LibraryMenu(void)
{
	uis.menusp = 0;

	ingameFlag = qtrue;	// true when in game menu is in use

	Mouse_Show();

	s_library.menuType = MENUTYPE_LIBRARY;

	LibraryMenu_LoadText();

	LibraryMenu_Init(); 

	UI_PushMenu( &s_library.menu );

	Menu_AdjustCursor( &s_library.menu, 1 );	
}


/*
=================
M_Accessing_Graphics
=================
*/
void LibraryAccessingMenu_Draw (void)
{
	int y;

	y = 50;

	if (s_libraryaccessing.menuType==1)	// ASTROMETRICS
	{
		ui.R_SetColor( colorTable[CT_DKBLUE1]);
	}
	else								// LIBRARY
	{
		ui.R_SetColor( colorTable[CT_DKPURPLE2]);
	}
	UI_DrawHandlePic( 132, y+ 42,  128,  -64, s_libraryaccessing.cornerPic);	// Top Left corner
	UI_DrawHandlePic( 132, y+252,  128,   64, s_libraryaccessing.cornerPic);	// Bottom Left corner

	UI_DrawHandlePic( 429, y+ 42, -128,  -64, s_libraryaccessing.cornerPic);	// Top Right corner
	UI_DrawHandlePic( 429, y+252, -128,   64, s_libraryaccessing.cornerPic);	// Bottom Right corner

	UI_DrawHandlePic(145, y+75,  395, 18, uis.whiteShader);		// Top
	UI_DrawHandlePic(132, y+93,  47, 175, uis.whiteShader);		// Left side
	UI_DrawHandlePic(510, y+93,  47, 175, uis.whiteShader);		// Right side
	UI_DrawHandlePic(147,y+265,  65, 18, uis.whiteShader);		// Bottom Left
	UI_DrawHandlePic(477,y+265,  65, 18, uis.whiteShader);		// Bottom Right
	UI_DrawHandlePic(214,y+265,  261, 18, uis.whiteShader);		// Bottom

	UI_DrawProportionalString(345,y+159,menu_normal_text[MNT_ACCESSING],UI_SMALLFONT | UI_CENTER,colorTable[CT_LTGOLD1]);

	// Wait a second to display the accessing screen and then go to the Library stuff 
	if (s_libraryaccessing.timer <= uis.realtime)
	{
		UI_PopMenu();

		if (s_libraryaccessing.menuType==1)
		{
			UI_AstrometricsMenu();	
		}
		else
		{
			UI_LibraryMenu();
		}
	}
}

/*
===============
UI_AccessingMenu_Cache
===============
*/
void UI_AccessingMenu_Cache (void)
{
	s_libraryaccessing.cornerPic = ui.R_RegisterShaderNoMip("menu/common/corner_ll_47_18.tga");
}

/*
===============
AccessingMenu_Init
===============
*/
void AccessingMenu_Init(void)
{

	UI_AccessingMenu_Cache();

	s_libraryaccessing.menu.nitems						= 0;
	s_libraryaccessing.menu.draw						= LibraryAccessingMenu_Draw;
	s_libraryaccessing.menu.key							= NULL;
	s_libraryaccessing.menu.fullscreen					= qtrue;
	s_libraryaccessing.menu.wrapAround					= qtrue;
	s_libraryaccessing.menu.descX						= MENU_DESC_X;
	s_libraryaccessing.menu.descY						= MENU_DESC_Y;
	s_libraryaccessing.menu.titleX						= MENU_TITLE_X;
	s_libraryaccessing.menu.titleY						= MENU_TITLE_Y;
	s_libraryaccessing.menu.titleI						= MNT_CREDITSMENU_TITLE;
	s_libraryaccessing.menu.footNoteEnum				= MNT_CREDITS;

	s_libraryaccessing.timer = uis.realtime + 1000;
}

/*
===============
UI_AccessingMenu
===============
*/
void UI_AccessingMenu(int menuType)
{
	ingameFlag = qtrue;	// true when in game menu is in use

	Mouse_Hide();

	s_libraryaccessing.menuType = menuType;

	AccessingMenu_Init(); 
	UI_PushMenu( &s_libraryaccessing.menu );
}

/*
===============
UI_AstrometricsMenu
===============
*/
void UI_AstrometricsMenu(void)
{
	uis.menusp = 0;

	ingameFlag = qtrue;	// true when in game menu is in use

	Mouse_Show();

	s_library.menuType = MENUTYPE_ASTROMETRICS;

	LibraryMenu_LoadText();

	LibraryMenu_Init(); 

	UI_PushMenu( &s_library.menu );

	Menu_AdjustCursor( &s_library.menu, 1 );	
}

// Precache stuff for Dischler Game Screen
typedef struct 
{
	menuframework_s menu;
	menubitmap_s	team;
	menubitmap_s	mainmenu;
	menubitmap_s	start;
	menubitmap_s	level1;
	menubitmap_s	level2;
	qhandle_t		plaque;
	qhandle_t		corner_ll_4_8;
	qhandle_t		corner_ul_8_18;
	qhandle_t		cornerLower;
} dischlerGame_t;


static dischlerGame_t	s_dischlerGame;

#define	ID_MAINMENU					103
#define	ID_LEVEL1					104
#define	ID_LEVEL2					105
#define ID_STARTGAME				106

/*
=================
DischlerGame_MenuEvent
=================
*/
static void DischlerGame_MenuEvent( void* ptr, int notification )
{
	menuframework_s*	m;

	if (notification != QM_ACTIVATED)
		return;

	m = ((menucommon_s*)ptr)->parent;

	switch (((menucommon_s*)ptr)->id)
	{
		case ID_MAINMENU:
			UI_PopMenu();
			break;

		case ID_LEVEL1:
			s_dischlerGame.level1.textcolor				= CT_WHITE;
			s_dischlerGame.level2.textcolor				= CT_BLACK;

			s_dischlerGame.level2.generic.flags &= ~ QMF_BLINK;
			s_dischlerGame.level1.generic.flags |= QMF_BLINK;
			break;

		case ID_LEVEL2:
			s_dischlerGame.level1.textcolor				= CT_BLACK;
			s_dischlerGame.level2.textcolor				= CT_WHITE;

			s_dischlerGame.level1.generic.flags &= ~ QMF_BLINK;
			s_dischlerGame.level2.generic.flags |= QMF_BLINK;
			break;

		case ID_STARTGAME:
			UI_ForceMenuOff ();
			ui.Cvar_SetValue( "cg_virtualVoyager", 1 );
//			ui.Cmd_ExecuteText( EXEC_APPEND, "map tour/_holodeck_minigame\n");
			ui.Cmd_ExecuteText( EXEC_APPEND, "use _holodeck_minigame\n");

			break;

	}
}

/*
=================
DischlerGame_MenuKey
=================
*/
static sfxHandle_t DischlerGame_MenuKey( int key )
{
	return Menu_DefaultKey( &s_dischlerGame.menu, key );
}

/*
=================
M_DischlerGame_Graphics
=================
*/
static void M_DischlerGame_Graphics (void)
{
	ui.R_SetColor( colorTable[CT_DKBROWN1]);
	UI_DrawHandlePic(  30, 425, 128,  64, s_dischlerGame.cornerLower);// Bottom Left Corner

	ui.R_SetColor( colorTable[CT_LTBROWN1]);
	UI_DrawHandlePic( 96,  438, 268,  18, uis.whiteShader);	// Bottom front Line


	ui.R_SetColor( colorTable[CT_WHITE]);
	UI_DrawHandlePic(56,  55, 512, 256, s_dischlerGame.plaque);	// 

	UI_DrawProportionalString( 320, 10,
		menu_normal_text[MNT_BORGSLAYER],   UI_GIANTFONT | UI_CENTER, colorTable[CT_YELLOW]);	

	ui.R_SetColor( colorTable[CT_LTBROWN1]);
	UI_DrawHandlePic( 368,  438, 232,  18, uis.whiteShader);	// Bottom back Line

	// Gender frame
/*	ui.R_SetColor( colorTable[CT_DKBROWN1]);
	UI_DrawHandlePic(313,  345,  16,  32, s_dischlerGame.corner_ul_8_18);	// 
	ui.R_SetColor( colorTable[CT_DKBROWN1]);
	UI_DrawHandlePic(313,  418,  16,   8, s_dischlerGame.corner_ll_4_8);	// 

	ui.R_SetColor( colorTable[CT_DKBROWN1]);
	UI_DrawHandlePic( 317, 345,  162,  18, uis.whiteShader);	// Top 
	UI_DrawHandlePic( 313, 350,    8,  69, uis.whiteShader);	// Side
	UI_DrawHandlePic( 317, 420,  162,   4, uis.whiteShader);	// Bottom

	UI_DrawProportionalString( 330, 347, menu_normal_text[MNT_LEVELS], UI_SMALLFONT, colorTable[CT_VLTBROWN1]);
*/
}

/*
===============
DischlerMenu_Draw
===============
*/
void DischlerMenu_Draw(void)
{
	M_DischlerGame_Graphics();
	
	Menu_Draw( &s_dischlerGame.menu );
}

/*
=================
UI_DischlerGameMenu_Cache
=================
*/
void UI_DischlerGameMenu_Cache( void )
{
	s_dischlerGame.plaque = ui.R_RegisterShaderNoMip("menu/common/menusign.tga");
//	s_dischlerGame.plaque = ui.R_RegisterShaderNoMip("menu/special/borgslayer2.tga");
	s_dischlerGame.corner_ll_4_8 = ui.R_RegisterShaderNoMip("menu/common/corner_ll_4_8.tga");
	s_dischlerGame.corner_ul_8_18 = ui.R_RegisterShaderNoMip("menu/common/corner_ul_8_18.tga");

	s_dischlerGame.cornerLower = ui.R_RegisterShaderNoMip("menu/common/corner_ll_47_18.tga");
}

/*
=================
DischlerGameMenu_Init
=================
*/
static void DischlerGameMenu_Init( void )
{
	UI_DischlerGameMenu_Cache();

	s_dischlerGame.menu.nitems						= 0;
	s_dischlerGame.menu.wrapAround					= qtrue;
	s_dischlerGame.menu.opening						= NULL;
	s_dischlerGame.menu.closing						= NULL;
	s_dischlerGame.menu.draw						= DischlerMenu_Draw;
	s_dischlerGame.menu.key							= DischlerGame_MenuKey;
	s_dischlerGame.menu.fullscreen					= qtrue;
	s_dischlerGame.menu.descX						= MENU_DESC_X;
	s_dischlerGame.menu.descY						= MENU_DESC_Y;
	s_dischlerGame.menu.listX						= 230;
	s_dischlerGame.menu.listY						= 188;
	s_dischlerGame.menu.titleX						= MENU_TITLE_X;
	s_dischlerGame.menu.titleY						= MENU_TITLE_Y;
	s_dischlerGame.menu.titleI						= MNT_TOURVOYAGER;
	s_dischlerGame.menu.footNoteEnum				= MNT_TOURVOYAGER;

	s_dischlerGame.mainmenu.generic.type			= MTYPE_BITMAP;      
	s_dischlerGame.mainmenu.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_dischlerGame.mainmenu.generic.x				= 30;
	s_dischlerGame.mainmenu.generic.y				= 398;
	s_dischlerGame.mainmenu.generic.name			= GRAPHIC_BUTTONRIGHT;
	s_dischlerGame.mainmenu.generic.id				= ID_MAINMENU;
	s_dischlerGame.mainmenu.generic.callback		= DischlerGame_MenuEvent;
	s_dischlerGame.mainmenu.width					= MENU_BUTTON_MED_WIDTH;
	s_dischlerGame.mainmenu.height					= MENU_BUTTON_MED_HEIGHT;
	s_dischlerGame.mainmenu.color					= CT_DKPURPLE1;
	s_dischlerGame.mainmenu.color2					= CT_LTPURPLE1;
	s_dischlerGame.mainmenu.textX					= MENU_BUTTON_TEXT_X;
	s_dischlerGame.mainmenu.textY					= MENU_BUTTON_TEXT_Y;
	s_dischlerGame.mainmenu.textEnum				= MBT_HAZARDGAMERETURN;
	s_dischlerGame.mainmenu.textcolor				= CT_BLACK;
	s_dischlerGame.mainmenu.textcolor2				= CT_WHITE;

	s_dischlerGame.start.generic.type			= MTYPE_BITMAP;
	s_dischlerGame.start.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_dischlerGame.start.generic.x				= 470;
	s_dischlerGame.start.generic.y				= 337;
	s_dischlerGame.start.generic.name			= "menu/common/square.tga";
	s_dischlerGame.start.generic.id				= ID_STARTGAME;
	s_dischlerGame.start.generic.callback		= DischlerGame_MenuEvent;
	s_dischlerGame.start.width					= MENU_BUTTON_MED_WIDTH;
	s_dischlerGame.start.height					= 79;
	s_dischlerGame.start.color					= CT_DKPURPLE1;
	s_dischlerGame.start.color2					= CT_LTPURPLE1;
	s_dischlerGame.start.textX					= MENU_BUTTON_TEXT_X;
	s_dischlerGame.start.textY					= 56;
	s_dischlerGame.start.textEnum				= MBT_ENGAGE;
	s_dischlerGame.start.textcolor				= CT_BLACK;
	s_dischlerGame.start.textcolor2				= CT_WHITE;

	s_dischlerGame.level1.generic.type			= MTYPE_BITMAP;      
	s_dischlerGame.level1.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_dischlerGame.level1.generic.x				= 338;
	s_dischlerGame.level1.generic.y				= 370;
	s_dischlerGame.level1.generic.name			= "menu/common/square.tga";
	s_dischlerGame.level1.generic.id				= ID_LEVEL1;
	s_dischlerGame.level1.generic.callback		= DischlerGame_MenuEvent;
	s_dischlerGame.level1.width					= MENU_BUTTON_MED_WIDTH;
	s_dischlerGame.level1.height					= MENU_BUTTON_MED_HEIGHT;
	s_dischlerGame.level1.color					= CT_DKPURPLE1;
	s_dischlerGame.level1.color2					= CT_LTPURPLE1;
	s_dischlerGame.level1.textX					= MENU_BUTTON_TEXT_X;
	s_dischlerGame.level1.textY					= MENU_BUTTON_TEXT_Y;
	s_dischlerGame.level1.textEnum				= MBT_LEVEL1;
	s_dischlerGame.level1.textcolor				= CT_BLACK;
	s_dischlerGame.level1.textcolor2			= CT_WHITE;

	s_dischlerGame.level2.generic.type			= MTYPE_BITMAP;      
	s_dischlerGame.level2.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_dischlerGame.level2.generic.x				= 338;
	s_dischlerGame.level2.generic.y				= 395;
	s_dischlerGame.level2.generic.name			= "menu/common/square.tga";
	s_dischlerGame.level2.generic.id			= ID_LEVEL2;
	s_dischlerGame.level2.generic.callback		= DischlerGame_MenuEvent;
	s_dischlerGame.level2.width					= MENU_BUTTON_MED_WIDTH;
	s_dischlerGame.level2.height				= MENU_BUTTON_MED_HEIGHT;
	s_dischlerGame.level2.color					= CT_DKPURPLE1;
	s_dischlerGame.level2.color2				= CT_LTPURPLE1;
	s_dischlerGame.level2.textX					= MENU_BUTTON_TEXT_X;
	s_dischlerGame.level2.textY					= MENU_BUTTON_TEXT_Y;
	s_dischlerGame.level2.textEnum				= MBT_LEVEL2;
	s_dischlerGame.level2.textcolor				= CT_BLACK;
	s_dischlerGame.level2.textcolor2			= CT_WHITE;

	Menu_AddItem( &s_dischlerGame.menu, ( void * )&s_dischlerGame.mainmenu);
//	Menu_AddItem( &s_dischlerGame.menu, ( void * )&s_dischlerGame.level1);
//	Menu_AddItem( &s_dischlerGame.menu, ( void * )&s_dischlerGame.level2);
	Menu_AddItem( &s_dischlerGame.menu, ( void * )&s_dischlerGame.start);

	s_dischlerGame.level1.textcolor				= CT_WHITE;
	s_dischlerGame.level2.textcolor				= CT_BLACK;

	s_dischlerGame.level1.generic.flags |= QMF_BLINK;

}

/*
===============
DischlerGameMenu_LoadText
===============
*/
/*
void DischlerGameMenu_LoadText (void)
{

	UI_DischlerGameMenu_Cache();

	UI_ParseLibraryText();
}
*/
/*
===============
UI_DischlerGameMenu
===============
*/
void UI_DischlerGameMenu(void)
{
	uis.menusp = 0;

	ingameFlag = qtrue;	// true when in game menu is in use

	Mouse_Show();

//	DischlerGameMenu_LoadText();
	UI_DischlerGameMenu_Cache();

	DischlerGameMenu_Init(); 

	UI_PushMenu( &s_dischlerGame.menu );

	Menu_AdjustCursor( &s_dischlerGame.menu, 1 );	
}


typedef struct 
{
	menuframework_s menu;

	qhandle_t		cornerUpper;
	qhandle_t		cornerUpper2;
	qhandle_t		cornerLower;

	menubitmap_s	mainmenu;
	menubitmap_s	firephasers;
	menubitmap_s	phaserUpper;
	menubitmap_s	phaserLower;
	menubitmap_s	phaserVentral;
	menubitmap_s	phaserPort;
	menubitmap_s	phaserStarboard;
	
	menubitmap_s	photonFore;
	menubitmap_s	photonAft;
	menubitmap_s	firephotons;
} tactical_t;


static tactical_t	s_tactical;

#define	ID_PHASERUPPER					104
#define	ID_PHASERLOWER					105
#define ID_PHASERFIRE					106
#define ID_PHASERAFT					107
#define ID_PHASERPORT					108
#define ID_PHASERVENTRAL				109
#define ID_PHASERSTARBOARD				110
#define ID_PHOTONFIRE					111
#define ID_PHOTONFORE					112
#define ID_PHOTONAFT					113

/*
=================
Tactical_MenuEvent
=================
*/
static void Tactical_MenuEvent( void* ptr, int notification )
{
	menuframework_s*	m;

	if (notification != QM_ACTIVATED)
		return;

	m = ((menucommon_s*)ptr)->parent;

	switch (((menucommon_s*)ptr)->id)
	{
		case ID_MAINMENU:
			UI_PopMenu();
			break;

		case ID_PHASERUPPER:
			if (s_tactical.phaserUpper.color == CT_DKPURPLE1)
			{
				s_tactical.phaserUpper.color					= CT_VDKPURPLE1;
				s_tactical.phaserUpper.color2					= CT_DKPURPLE1;
			}
			else
			{
				s_tactical.phaserUpper.color					= CT_DKPURPLE1;
				s_tactical.phaserUpper.color2					= CT_LTPURPLE1;
			}
			break;

		case ID_PHASERLOWER:
			if (s_tactical.phaserLower.color == CT_DKPURPLE1)
			{
				s_tactical.phaserLower.color					= CT_VDKPURPLE1;
				s_tactical.phaserLower.color2					= CT_DKPURPLE1;
			}
			else
			{
				s_tactical.phaserLower.color					= CT_DKPURPLE1;
				s_tactical.phaserLower.color2					= CT_LTPURPLE1;
			}
			break;

		case ID_PHASERSTARBOARD:
			if (s_tactical.phaserStarboard.color == CT_DKPURPLE1)
			{
				s_tactical.phaserStarboard.color					= CT_VDKPURPLE1;
				s_tactical.phaserStarboard.color2					= CT_DKPURPLE1;
			}
			else
			{
				s_tactical.phaserStarboard.color					= CT_DKPURPLE1;
				s_tactical.phaserStarboard.color2					= CT_LTPURPLE1;
			}
			break;

		case ID_PHASERPORT:
			if (s_tactical.phaserPort.color == CT_DKPURPLE1)
			{
				s_tactical.phaserPort.color					= CT_VDKPURPLE1;
				s_tactical.phaserPort.color2					= CT_DKPURPLE1;
			}
			else
			{
				s_tactical.phaserPort.color					= CT_DKPURPLE1;
				s_tactical.phaserPort.color2					= CT_LTPURPLE1;
			}
			break;

		case ID_PHASERVENTRAL:
			if (s_tactical.phaserVentral.color == CT_DKPURPLE1)
			{
				s_tactical.phaserVentral.color					= CT_VDKPURPLE1;
				s_tactical.phaserVentral.color2					= CT_DKPURPLE1;
			}
			else
			{
				s_tactical.phaserVentral.color					= CT_DKPURPLE1;
				s_tactical.phaserVentral.color2					= CT_LTPURPLE1;
			}
			break;

		case ID_PHOTONFORE:
			if (s_tactical.photonFore.color == CT_DKPURPLE1)
			{
				s_tactical.photonFore.color					= CT_VDKPURPLE1;
				s_tactical.photonFore.color2					= CT_DKPURPLE1;
			}
			else
			{
				s_tactical.photonFore.color					= CT_DKPURPLE1;
				s_tactical.photonFore.color2					= CT_LTPURPLE1;
			}
			break;

		case ID_PHOTONAFT:
			if (s_tactical.photonAft.color == CT_DKPURPLE1)
			{
				s_tactical.photonAft.color					= CT_VDKPURPLE1;
				s_tactical.photonAft.color2					= CT_DKPURPLE1;
			}
			else
			{
				s_tactical.photonAft.color					= CT_DKPURPLE1;
				s_tactical.photonAft.color2					= CT_LTPURPLE1;
			}
			break;

		case ID_STARTGAME:
			break;
	}
}

/*
=================
Tactical_MenuKey
=================
*/
static sfxHandle_t Tactical_MenuKey( int key )
{
	return Menu_DefaultKey( &s_tactical.menu, key );
}

/*
=================
M_Tactical_Graphics
=================
*/
static void M_Tactical_Graphics (void)
{

	ui.R_SetColor( colorTable[CT_DKBROWN1]);
	UI_DrawHandlePic( 160,  60, 330,  18, uis.whiteShader);	// Phaser Line
	UI_DrawProportionalString( 180, 62, menu_normal_text[MNT_PHASERS], UI_SMALLFONT, colorTable[CT_LTGOLD1]);

	ui.R_SetColor( colorTable[CT_DKBROWN1]);
	UI_DrawHandlePic( 160,  258, 330,  18, uis.whiteShader);	// Torpodoes Line
	UI_DrawProportionalString( 180, 260, menu_normal_text[MNT_PHOTONTORPEDOES], UI_SMALLFONT, colorTable[CT_LTGOLD1]);


	// Frame
	ui.R_SetColor( colorTable[CT_DKBROWN1]);
	UI_DrawHandlePic(  30, 20, 128,  64, s_tactical.cornerUpper2);	// Top corner
	UI_DrawHandlePic(  50, 20,  99,   7, uis.whiteShader);
	UI_DrawHandlePic( 152, 20, 135,   7, uis.whiteShader);

	ui.R_SetColor( colorTable[CT_DKBROWN1]);
	UI_DrawHandlePic( 290, 20,  12,   7, uis.whiteShader);

	ui.R_SetColor( colorTable[CT_LTBROWN1]);
	UI_DrawHandlePic( 305, 20,  60,   4, uis.whiteShader);

	ui.R_SetColor( colorTable[CT_DKBROWN1]);
	UI_DrawHandlePic( 368, 20, 111,   7, uis.whiteShader);

	// Side left column
	ui.R_SetColor( colorTable[CT_DKBROWN1]);
	UI_DrawHandlePic( 30, 51,   47,   119, uis.whiteShader);
	UI_DrawHandlePic( 30,203,   47,   186, uis.whiteShader);




	ui.R_SetColor( colorTable[CT_DKBROWN1]);
	UI_DrawHandlePic(  30, 173,  47,  27, uis.whiteShader);	// Top left column
	UI_DrawHandlePic(  30, 392,  47,  33, uis.whiteShader);	// Bottom left column
	UI_DrawHandlePic(  30, 425, 128,  64, s_tactical.cornerLower);// Bottom Left Corner

	ui.R_SetColor( colorTable[CT_LTBROWN1]);
	UI_DrawHandlePic( 96,  438, 268,  18, uis.whiteShader);	// Bottom front Line


}

/*
===============
TacticalMenu_Draw
===============
*/
void TacticalMenu_Draw(void)
{
	M_Tactical_Graphics();
	
	Menu_Draw( &s_tactical.menu );
}

/*
=================
UI_TacticalMenu_Cache
=================
*/
void UI_TacticalMenu_Cache( void )
{
	s_tactical.cornerUpper = ui.R_RegisterShaderNoMip("menu/common/corner_ll_47_7.tga");
	s_tactical.cornerUpper2= ui.R_RegisterShaderNoMip("menu/common/corner_ul_47_7.tga");
	s_tactical.cornerLower = ui.R_RegisterShaderNoMip("menu/common/corner_ll_47_18.tga");
}

/*
=================
TacticalMenu_Init
=================
*/
static void TacticalMenu_Init( void )
{
	int x,x2;

	UI_TacticalMenu_Cache();

	s_tactical.menu.nitems						= 0;
	s_tactical.menu.wrapAround					= qtrue;
	s_tactical.menu.opening						= NULL;
	s_tactical.menu.closing						= NULL;
	s_tactical.menu.draw						= TacticalMenu_Draw;
	s_tactical.menu.key							= Tactical_MenuKey;
	s_tactical.menu.fullscreen					= qtrue;
	s_tactical.menu.descX						= MENU_DESC_X;
	s_tactical.menu.descY						= MENU_DESC_Y;
	s_tactical.menu.listX						= 230;
	s_tactical.menu.listY						= 188;
	s_tactical.menu.titleX						= MENU_TITLE_X;
	s_tactical.menu.titleY						= MENU_TITLE_Y;
	s_tactical.menu.titleI						= MNT_TACTICAL;
	s_tactical.menu.footNoteEnum				= MNT_TACTICAL;

	x2 = 340;
	s_tactical.mainmenu.generic.type			= MTYPE_BITMAP;      
	s_tactical.mainmenu.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_tactical.mainmenu.generic.x				= 130;
	s_tactical.mainmenu.generic.y				= 398;
	s_tactical.mainmenu.generic.name			= GRAPHIC_BUTTONRIGHT;
	s_tactical.mainmenu.generic.id				= ID_MAINMENU;
	s_tactical.mainmenu.generic.callback		= Tactical_MenuEvent;
	s_tactical.mainmenu.width					= MENU_BUTTON_MED_WIDTH;
	s_tactical.mainmenu.height					= MENU_BUTTON_MED_HEIGHT;
	s_tactical.mainmenu.color					= CT_DKPURPLE1;
	s_tactical.mainmenu.color2					= CT_LTPURPLE1;
	s_tactical.mainmenu.textX					= MENU_BUTTON_TEXT_X;
	s_tactical.mainmenu.textY					= MENU_BUTTON_TEXT_Y;
	s_tactical.mainmenu.textEnum				= MBT_HAZARDGAMERETURN;
	s_tactical.mainmenu.textcolor				= CT_BLACK;
	s_tactical.mainmenu.textcolor2				= CT_WHITE;

	x = 178;
	s_tactical.firephasers.generic.type			= MTYPE_BITMAP;
	s_tactical.firephasers.generic.flags		= QMF_HIGHLIGHT_IF_FOCUS;
	s_tactical.firephasers.generic.x			= x2;
	s_tactical.firephasers.generic.y			= 114;
	s_tactical.firephasers.generic.name			= "menu/common/square.tga";
	s_tactical.firephasers.generic.id			= ID_PHASERFIRE;
	s_tactical.firephasers.generic.callback		= Tactical_MenuEvent;
	s_tactical.firephasers.width				= MENU_BUTTON_MED_WIDTH;
	s_tactical.firephasers.height				= 79;
	s_tactical.firephasers.color				= CT_DKPURPLE1;
	s_tactical.firephasers.color2				= CT_LTPURPLE1;
	s_tactical.firephasers.textX				= MENU_BUTTON_TEXT_X;
	s_tactical.firephasers.textY				= 56;
	s_tactical.firephasers.textEnum				= MBT_FIREPHASERS;
	s_tactical.firephasers.textcolor			= CT_BLACK;
	s_tactical.firephasers.textcolor2			= CT_WHITE;

	s_tactical.phaserUpper.generic.type			= MTYPE_BITMAP;      
	s_tactical.phaserUpper.generic.flags		= QMF_HIGHLIGHT_IF_FOCUS;
	s_tactical.phaserUpper.generic.x			= x;
	s_tactical.phaserUpper.generic.y			= 100;
	s_tactical.phaserUpper.generic.name			= "menu/common/square.tga";
	s_tactical.phaserUpper.generic.id			= ID_PHASERUPPER;
	s_tactical.phaserUpper.generic.callback		= Tactical_MenuEvent;
	s_tactical.phaserUpper.width				= MENU_BUTTON_MED_WIDTH;
	s_tactical.phaserUpper.height				= MENU_BUTTON_MED_HEIGHT;
	s_tactical.phaserUpper.color				= CT_VDKPURPLE1;
	s_tactical.phaserUpper.color2				= CT_DKPURPLE1;
	s_tactical.phaserUpper.textX				= MENU_BUTTON_TEXT_X;
	s_tactical.phaserUpper.textY				= MENU_BUTTON_TEXT_Y;
	s_tactical.phaserUpper.textEnum				= MBT_PHASERUPPER;
	s_tactical.phaserUpper.textcolor			= CT_BLACK;
	s_tactical.phaserUpper.textcolor2			= CT_WHITE;

	s_tactical.phaserLower.generic.type			= MTYPE_BITMAP;      
	s_tactical.phaserLower.generic.flags		= QMF_HIGHLIGHT_IF_FOCUS;
	s_tactical.phaserLower.generic.x			= x;
	s_tactical.phaserLower.generic.y			= 120;
	s_tactical.phaserLower.generic.name			= "menu/common/square.tga";
	s_tactical.phaserLower.generic.id			= ID_PHASERLOWER;
	s_tactical.phaserLower.generic.callback		= Tactical_MenuEvent;
	s_tactical.phaserLower.width				= MENU_BUTTON_MED_WIDTH;
	s_tactical.phaserLower.height				= MENU_BUTTON_MED_HEIGHT;
	s_tactical.phaserLower.color				= CT_VDKPURPLE1;
	s_tactical.phaserLower.color2				= CT_DKPURPLE1;
	s_tactical.phaserLower.textX				= MENU_BUTTON_TEXT_X;
	s_tactical.phaserLower.textY				= MENU_BUTTON_TEXT_Y;
	s_tactical.phaserLower.textEnum				= MBT_PHASERLOWER;
	s_tactical.phaserLower.textcolor			= CT_BLACK;
	s_tactical.phaserLower.textcolor2			= CT_WHITE;

	s_tactical.phaserPort.generic.type			= MTYPE_BITMAP;      
	s_tactical.phaserPort.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_tactical.phaserPort.generic.x				= x;
	s_tactical.phaserPort.generic.y				= 140;
	s_tactical.phaserPort.generic.name			= "menu/common/square.tga";
	s_tactical.phaserPort.generic.id			= ID_PHASERPORT;
	s_tactical.phaserPort.generic.callback		= Tactical_MenuEvent;
	s_tactical.phaserPort.width					= MENU_BUTTON_MED_WIDTH;
	s_tactical.phaserPort.height				= MENU_BUTTON_MED_HEIGHT;
	s_tactical.phaserPort.color					= CT_VDKPURPLE1;
	s_tactical.phaserPort.color2				= CT_DKPURPLE1;
	s_tactical.phaserPort.textX					= MENU_BUTTON_TEXT_X;
	s_tactical.phaserPort.textY					= MENU_BUTTON_TEXT_Y;
	s_tactical.phaserPort.textEnum				= MBT_PHASERPORT;
	s_tactical.phaserPort.textcolor				= CT_BLACK;
	s_tactical.phaserPort.textcolor2			= CT_WHITE;

	s_tactical.phaserStarboard.generic.type		= MTYPE_BITMAP;      
	s_tactical.phaserStarboard.generic.flags	= QMF_HIGHLIGHT_IF_FOCUS;
	s_tactical.phaserStarboard.generic.x		= x;
	s_tactical.phaserStarboard.generic.y		= 160;
	s_tactical.phaserStarboard.generic.name		= "menu/common/square.tga";
	s_tactical.phaserStarboard.generic.id		= ID_PHASERSTARBOARD;
	s_tactical.phaserStarboard.generic.callback	= Tactical_MenuEvent;
	s_tactical.phaserStarboard.width			= MENU_BUTTON_MED_WIDTH;
	s_tactical.phaserStarboard.height			= MENU_BUTTON_MED_HEIGHT;
	s_tactical.phaserStarboard.color			= CT_VDKPURPLE1;
	s_tactical.phaserStarboard.color2			= CT_DKPURPLE1;
	s_tactical.phaserStarboard.textX			= MENU_BUTTON_TEXT_X;
	s_tactical.phaserStarboard.textY			= MENU_BUTTON_TEXT_Y;
	s_tactical.phaserStarboard.textEnum			= MBT_PHASERSTARBOARD;
	s_tactical.phaserStarboard.textcolor		= CT_BLACK;
	s_tactical.phaserStarboard.textcolor2		= CT_WHITE;

	s_tactical.phaserVentral.generic.type		= MTYPE_BITMAP;      
	s_tactical.phaserVentral.generic.flags		= QMF_HIGHLIGHT_IF_FOCUS;
	s_tactical.phaserVentral.generic.x			= x;
	s_tactical.phaserVentral.generic.y			= 180;
	s_tactical.phaserVentral.generic.name		= "menu/common/square.tga";
	s_tactical.phaserVentral.generic.id			= ID_PHASERVENTRAL;
	s_tactical.phaserVentral.generic.callback	= Tactical_MenuEvent;
	s_tactical.phaserVentral.width				= MENU_BUTTON_MED_WIDTH;
	s_tactical.phaserVentral.height				= MENU_BUTTON_MED_HEIGHT;
	s_tactical.phaserVentral.color				= CT_VDKPURPLE1;
	s_tactical.phaserVentral.color2				= CT_DKPURPLE1;
	s_tactical.phaserVentral.textX				= MENU_BUTTON_TEXT_X;
	s_tactical.phaserVentral.textY				= MENU_BUTTON_TEXT_Y;
	s_tactical.phaserVentral.textEnum			= MBT_PHASERVENTRAL;
	s_tactical.phaserVentral.textcolor			= CT_BLACK;
	s_tactical.phaserVentral.textcolor2			= CT_WHITE;

	
	s_tactical.photonFore.generic.type			= MTYPE_BITMAP;      
	s_tactical.photonFore.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_tactical.photonFore.generic.x				= x;
	s_tactical.photonFore.generic.y				= 300;
	s_tactical.photonFore.generic.name			= "menu/common/square.tga";
	s_tactical.photonFore.generic.id			= ID_PHOTONFORE;
	s_tactical.photonFore.generic.callback		= Tactical_MenuEvent;
	s_tactical.photonFore.width					= MENU_BUTTON_MED_WIDTH;
	s_tactical.photonFore.height				= MENU_BUTTON_MED_HEIGHT;
	s_tactical.photonFore.color					= CT_VDKPURPLE1;
	s_tactical.photonFore.color2				= CT_DKPURPLE1;
	s_tactical.photonFore.textX					= MENU_BUTTON_TEXT_X;
	s_tactical.photonFore.textY					= MENU_BUTTON_TEXT_Y;
	s_tactical.photonFore.textEnum				= MBT_PHOTONFORE;
	s_tactical.photonFore.textcolor				= CT_BLACK;
	s_tactical.photonFore.textcolor2			= CT_WHITE;

	s_tactical.photonAft.generic.type			= MTYPE_BITMAP;      
	s_tactical.photonAft.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_tactical.photonAft.generic.x				= x;
	s_tactical.photonAft.generic.y				= 320;
	s_tactical.photonAft.generic.name			= "menu/common/square.tga";
	s_tactical.photonAft.generic.id				= ID_PHOTONAFT;
	s_tactical.photonAft.generic.callback		= Tactical_MenuEvent;
	s_tactical.photonAft.width					= MENU_BUTTON_MED_WIDTH;
	s_tactical.photonAft.height					= MENU_BUTTON_MED_HEIGHT;
	s_tactical.photonAft.color					= CT_VDKPURPLE1;
	s_tactical.photonAft.color2					= CT_DKPURPLE1;
	s_tactical.photonAft.textX					= MENU_BUTTON_TEXT_X;
	s_tactical.photonAft.textY					= MENU_BUTTON_TEXT_Y;
	s_tactical.photonAft.textEnum				= MBT_PHOTONAFT;
	s_tactical.photonAft.textcolor				= CT_BLACK;
	s_tactical.photonAft.textcolor2				= CT_WHITE;

	
	s_tactical.firephotons.generic.type			= MTYPE_BITMAP;
	s_tactical.firephotons.generic.flags		= QMF_HIGHLIGHT_IF_FOCUS;
	s_tactical.firephotons.generic.x			= x2;
	s_tactical.firephotons.generic.y			= 292;
	s_tactical.firephotons.generic.name			= "menu/common/square.tga";
	s_tactical.firephotons.generic.id			= ID_STARTGAME;
	s_tactical.firephotons.generic.callback		= Tactical_MenuEvent;
	s_tactical.firephotons.width				= MENU_BUTTON_MED_WIDTH;
	s_tactical.firephotons.height				= 79;
	s_tactical.firephotons.color				= CT_DKPURPLE1;
	s_tactical.firephotons.color2				= CT_LTPURPLE1;
	s_tactical.firephotons.textX				= MENU_BUTTON_TEXT_X;
	s_tactical.firephotons.textY				= 56;
	s_tactical.firephotons.textEnum				= MBT_FIREPHOTONS;
	s_tactical.firephotons.textcolor			= CT_BLACK;
	s_tactical.firephotons.textcolor2			= CT_WHITE;

	Menu_AddItem( &s_tactical.menu, ( void * )&s_tactical.mainmenu);
	Menu_AddItem( &s_tactical.menu, ( void * )&s_tactical.firephasers);
	Menu_AddItem( &s_tactical.menu, ( void * )&s_tactical.phaserUpper);
	Menu_AddItem( &s_tactical.menu, ( void * )&s_tactical.phaserLower);
	Menu_AddItem( &s_tactical.menu, ( void * )&s_tactical.phaserPort);
	Menu_AddItem( &s_tactical.menu, ( void * )&s_tactical.phaserStarboard);
	Menu_AddItem( &s_tactical.menu, ( void * )&s_tactical.phaserVentral);

	Menu_AddItem( &s_tactical.menu, ( void * )&s_tactical.photonFore);
	Menu_AddItem( &s_tactical.menu, ( void * )&s_tactical.photonAft);
	Menu_AddItem( &s_tactical.menu, ( void * )&s_tactical.firephotons);

}

/*
===============
UI_TacticalMenu
===============
*/
void UI_TacticalMenu(void)
{
	uis.menusp = 0;

	ingameFlag = qtrue;	// true when in game menu is in use

	Mouse_Show();

	UI_TacticalMenu_Cache();

	TacticalMenu_Init(); 

	UI_PushMenu( &s_tactical.menu );

	Menu_AdjustCursor( &s_tactical.menu, 1 );	
}

typedef struct 
{
	menuframework_s menu;

	qhandle_t		cornerUpper;
	qhandle_t		cornerUpper2;
	qhandle_t		cornerLower;
	qhandle_t		monitorBar;
	qhandle_t		arrowRight;
	qhandle_t		arrowLeft;

	menubitmap_s	mainmenu;
	
} engineeringStatus_t;


static engineeringStatus_t	s_engineeringStatus;

/*
=================
EngineeringStatus_MenuEvent
=================
*/
static void EngineeringStatus_MenuEvent( void* ptr, int notification )
{
	menuframework_s*	m;

	if (notification != QM_ACTIVATED)
		return;

	m = ((menucommon_s*)ptr)->parent;

	switch (((menucommon_s*)ptr)->id)
	{
		case ID_MAINMENU:
			UI_PopMenu();
			break;

		case ID_STARTGAME:
			break;
	}
}

/*
=================
EngineeringStatus_MenuKey
=================
*/
static sfxHandle_t EngineeringStatus_MenuKey( int key )
{
	return Menu_DefaultKey( &s_engineeringStatus.menu, key );
}

/*
=================
M_EngineeringStatus_Graphics
=================
*/
static void M_EngineeringStatus_Graphics (void)
{

	ui.R_SetColor( colorTable[CT_DKBROWN1]);
	UI_DrawHandlePic( 160,  60, 330,  18, uis.whiteShader);	// Phaser Line
	UI_DrawProportionalString( 180, 62, "WARP DRIVE STATUS", UI_SMALLFONT, colorTable[CT_LTGOLD1]);

	ui.R_SetColor( colorTable[CT_DKBROWN1]);
	UI_DrawHandlePic( 160,  228, 330,  18, uis.whiteShader);	// Torpodoes Line
	UI_DrawProportionalString( 180, 230, "IMPULSE DRIVE STATUS", UI_SMALLFONT, colorTable[CT_LTGOLD1]);

	// Frame
	ui.R_SetColor( colorTable[CT_DKBROWN1]);
	UI_DrawHandlePic(  30, 20, 128,  64, s_engineeringStatus.cornerUpper2);	// Top corner
	UI_DrawHandlePic(  50, 20,  99,   7, uis.whiteShader);
	UI_DrawHandlePic( 152, 20, 135,   7, uis.whiteShader);

	ui.R_SetColor( colorTable[CT_DKBROWN1]);
	UI_DrawHandlePic( 290, 20,  12,   7, uis.whiteShader);

	ui.R_SetColor( colorTable[CT_LTBROWN1]);
	UI_DrawHandlePic( 305, 20,  60,   4, uis.whiteShader);

	ui.R_SetColor( colorTable[CT_DKBROWN1]);
	UI_DrawHandlePic( 368, 20, 111,   7, uis.whiteShader);

	ui.R_SetColor( colorTable[CT_LTRED1]);
	UI_DrawHandlePic( 176, 100, 32,   64, s_engineeringStatus.monitorBar);
	UI_DrawHandlePic( 256, 100, 32,   64, s_engineeringStatus.monitorBar);
	UI_DrawHandlePic( 336, 100, 32,   64, s_engineeringStatus.monitorBar);

	ui.R_SetColor( colorTable[CT_LTPURPLE2]);
	UI_DrawHandlePic( 168, 120, 8,   8, s_engineeringStatus.arrowRight);
	UI_DrawHandlePic( 184, 140, 8,   8, s_engineeringStatus.arrowLeft);

	ui.R_SetColor( colorTable[CT_LTPURPLE2]);
	UI_DrawHandlePic( 248, 110, 8,   8, s_engineeringStatus.arrowRight);
	UI_DrawHandlePic( 264, 154, 8,   8, s_engineeringStatus.arrowLeft);

	ui.R_SetColor( colorTable[CT_LTPURPLE2]);
	UI_DrawHandlePic( 328, 140, 8,   8, s_engineeringStatus.arrowRight);
	UI_DrawHandlePic( 344, 110, 8,   8, s_engineeringStatus.arrowLeft);

	UI_DrawProportionalString( 180, 180, "Warp Core", UI_TINYFONT | UI_CENTER, colorTable[CT_LTGOLD1]);
	UI_DrawProportionalString( 260, 180, "Port Nacelle", UI_TINYFONT | UI_CENTER, colorTable[CT_LTGOLD1]);
	UI_DrawProportionalString( 340, 180, "Starboard Nacelle", UI_TINYFONT | UI_CENTER, colorTable[CT_LTGOLD1]);

	ui.R_SetColor( colorTable[CT_LTRED1]);
	UI_DrawHandlePic( 176, 260, 32,   64, s_engineeringStatus.monitorBar);
	UI_DrawHandlePic( 256, 260, 32,   64, s_engineeringStatus.monitorBar);

	ui.R_SetColor( colorTable[CT_LTPURPLE2]);
	UI_DrawHandlePic( 168, 300, 8,   8, s_engineeringStatus.arrowRight);
	UI_DrawHandlePic( 184, 290, 8,   8, s_engineeringStatus.arrowLeft);

	ui.R_SetColor( colorTable[CT_LTPURPLE2]);
	UI_DrawHandlePic( 248, 305, 8,   8, s_engineeringStatus.arrowRight);
	UI_DrawHandlePic( 264, 305, 8,   8, s_engineeringStatus.arrowLeft);


	UI_DrawProportionalString( 180, 340, "Engine 1", UI_TINYFONT| UI_CENTER, colorTable[CT_LTGOLD1]);
	UI_DrawProportionalString( 260, 340, "Engine 2", UI_TINYFONT| UI_CENTER, colorTable[CT_LTGOLD1]);


	// Side left column
	ui.R_SetColor( colorTable[CT_DKBROWN1]);
	UI_DrawHandlePic( 30, 51,   47,   119, uis.whiteShader);
	UI_DrawHandlePic( 30,203,   47,   186, uis.whiteShader);

	ui.R_SetColor( colorTable[CT_DKBROWN1]);
	UI_DrawHandlePic(  30, 173,  47,  27, uis.whiteShader);	// Top left column
	UI_DrawHandlePic(  30, 392,  47,  33, uis.whiteShader);	// Bottom left column
	UI_DrawHandlePic(  30, 425, 128,  64, s_engineeringStatus.cornerLower);// Bottom Left Corner

	ui.R_SetColor( colorTable[CT_LTBROWN1]);
	UI_DrawHandlePic( 96,  438, 268,  18, uis.whiteShader);	// Bottom front Line

}

/*
===============
EngineeringStatusMenu_Draw
===============
*/
void EngineeringStatusMenu_Draw(void)
{
	M_EngineeringStatus_Graphics();
	
	Menu_Draw( &s_engineeringStatus.menu );
}

/*
=================
UI_EngineeringStatusMenu_Cache
=================
*/
void UI_EngineeringStatusMenu_Cache( void )
{
	s_engineeringStatus.cornerUpper = ui.R_RegisterShaderNoMip("menu/common/corner_ll_47_7.tga");
	s_engineeringStatus.cornerUpper2= ui.R_RegisterShaderNoMip("menu/common/corner_ul_47_7.tga");
	s_engineeringStatus.cornerLower = ui.R_RegisterShaderNoMip("menu/common/corner_ll_47_18.tga");
	s_engineeringStatus.monitorBar = ui.R_RegisterShaderNoMip("menu/common/mon_bar.tga");

	s_engineeringStatus.arrowRight = ui.R_RegisterShaderNoMip("menu/common/right_arrow.tga");
	s_engineeringStatus.arrowLeft = ui.R_RegisterShaderNoMip("menu/common/left_arrow.tga");

}
/*
=================
EngineeringStatusMenu_Init
=================
*/
static void EngineeringStatusMenu_Init( void )
{
	int x2;

	UI_EngineeringStatusMenu_Cache();

	s_engineeringStatus.menu.nitems						= 0;
	s_engineeringStatus.menu.wrapAround					= qtrue;
	s_engineeringStatus.menu.opening						= NULL;
	s_engineeringStatus.menu.closing						= NULL;
	s_engineeringStatus.menu.draw						= EngineeringStatusMenu_Draw;
	s_engineeringStatus.menu.key							= EngineeringStatus_MenuKey;
	s_engineeringStatus.menu.fullscreen					= qtrue;
	s_engineeringStatus.menu.descX						= MENU_DESC_X;
	s_engineeringStatus.menu.descY						= MENU_DESC_Y;
	s_engineeringStatus.menu.listX						= 230;
	s_engineeringStatus.menu.listY						= 188;
	s_engineeringStatus.menu.titleX						= MENU_TITLE_X;
	s_engineeringStatus.menu.titleY						= MENU_TITLE_Y;
	s_engineeringStatus.menu.titleI						= MNT_TACTICAL;
	s_engineeringStatus.menu.footNoteEnum				= MNT_TACTICAL;

	x2 = 340;
	s_engineeringStatus.mainmenu.generic.type			= MTYPE_BITMAP;      
	s_engineeringStatus.mainmenu.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_engineeringStatus.mainmenu.generic.x				= 130;
	s_engineeringStatus.mainmenu.generic.y				= 398;
	s_engineeringStatus.mainmenu.generic.name			= GRAPHIC_BUTTONRIGHT;
	s_engineeringStatus.mainmenu.generic.id				= ID_MAINMENU;
	s_engineeringStatus.mainmenu.generic.callback		= EngineeringStatus_MenuEvent;
	s_engineeringStatus.mainmenu.width					= MENU_BUTTON_MED_WIDTH;
	s_engineeringStatus.mainmenu.height					= MENU_BUTTON_MED_HEIGHT;
	s_engineeringStatus.mainmenu.color					= CT_DKPURPLE1;
	s_engineeringStatus.mainmenu.color2					= CT_LTPURPLE1;
	s_engineeringStatus.mainmenu.textX					= MENU_BUTTON_TEXT_X;
	s_engineeringStatus.mainmenu.textY					= MENU_BUTTON_TEXT_Y;
	s_engineeringStatus.mainmenu.textEnum				= MBT_HAZARDGAMERETURN;
	s_engineeringStatus.mainmenu.textcolor				= CT_BLACK;
	s_engineeringStatus.mainmenu.textcolor2				= CT_WHITE;

	Menu_AddItem( &s_engineeringStatus.menu, ( void * )&s_engineeringStatus.mainmenu);
}

/*
===============
UI_EngineeringStatusMenu
===============
*/
void UI_EngineeringStatusMenu(void)
{
	uis.menusp = 0;

	ingameFlag = qtrue;	// true when in game menu is in use

	Mouse_Show();

	UI_EngineeringStatusMenu_Cache();

	EngineeringStatusMenu_Init(); 

	UI_PushMenu( &s_engineeringStatus.menu );

	Menu_AdjustCursor( &s_engineeringStatus.menu, 1 );	
}


typedef struct 
{
	menuframework_s menu;

	qhandle_t		cornerUpper;
	qhandle_t		cornerUpper2;
	qhandle_t		cornerLower;

	menuslider_s	warp_slider;
	menuslider_s	impulse_slider;

	menubitmap_s	mainmenu;
	menubitmap_s	engage;
	menufield_s		heading1;
	menufield_s		heading2;

} navigation_t;


static navigation_t	s_navigation;

/*
=================
WarpCallback
=================
*/
void WarpCallback( void *s, int notification )
{
	menuslider_s *slider = ( menuslider_s * ) s;

	if (notification != QM_ACTIVATED)
		return;

	s_navigation.warp_slider.color		= CT_DKPURPLE1;
	s_navigation.warp_slider.color2		= CT_LTPURPLE1;

	s_navigation.impulse_slider.color		= CT_VDKPURPLE1;
	s_navigation.impulse_slider.color2		= CT_DKPURPLE1;
	s_navigation.impulse_slider.curvalue = 0;

}

/*
=================
ImpulseCallback
=================
*/
void ImpulseCallback( void *s, int notification )
{
	menuslider_s *slider = ( menuslider_s * ) s;

	if (notification != QM_ACTIVATED)
		return;

	s_navigation.warp_slider.color		= CT_VDKPURPLE1;
	s_navigation.warp_slider.color2		= CT_DKPURPLE1;
	s_navigation.warp_slider.curvalue = 0;

	s_navigation.impulse_slider.color		= CT_DKPURPLE1;
	s_navigation.impulse_slider.color2		= CT_LTPURPLE1;
}

/*
=================
Navigation_MenuEvent
=================
*/
static void Navigation_MenuEvent( void* ptr, int notification )
{
	menuframework_s*	m;

	if (notification != QM_ACTIVATED)
		return;

	m = ((menucommon_s*)ptr)->parent;

	switch (((menucommon_s*)ptr)->id)
	{
		case ID_MAINMENU:
			UI_PopMenu();
			break;

		case ID_STARTGAME:
			break;
	}
}

/*
=================
Navigation_MenuKey
=================
*/
static sfxHandle_t Navigation_MenuKey( int key )
{
	return Menu_DefaultKey( &s_navigation.menu, key );
}

/*
=================
M_Navigation_Graphics
=================
*/
static void M_Navigation_Graphics (void)
{
	// Frame
	ui.R_SetColor( colorTable[CT_DKBROWN1]);
	UI_DrawHandlePic(  30, 20, 128,  64, s_engineeringStatus.cornerUpper2);	// Top corner
	UI_DrawHandlePic(  50, 20,  99,   7, uis.whiteShader);
	UI_DrawHandlePic( 152, 20, 135,   7, uis.whiteShader);

	ui.R_SetColor( colorTable[CT_DKBROWN1]);
	UI_DrawHandlePic( 290, 20,  12,   7, uis.whiteShader);

	ui.R_SetColor( colorTable[CT_LTBROWN1]);
	UI_DrawHandlePic( 305, 20,  60,   4, uis.whiteShader);

	ui.R_SetColor( colorTable[CT_DKBROWN1]);
	UI_DrawHandlePic( 368, 20, 111,   7, uis.whiteShader);


	// Side left column
	ui.R_SetColor( colorTable[CT_DKBROWN1]);
	UI_DrawHandlePic( 30, 51,   47,   119, uis.whiteShader);
	UI_DrawHandlePic( 30,203,   47,   186, uis.whiteShader);

	ui.R_SetColor( colorTable[CT_DKBROWN1]);
	UI_DrawHandlePic(  30, 173,  47,  27, uis.whiteShader);	// Top left column
	UI_DrawHandlePic(  30, 392,  47,  33, uis.whiteShader);	// Bottom left column
	UI_DrawHandlePic(  30, 425, 128,  64, s_engineeringStatus.cornerLower);// Bottom Left Corner

	ui.R_SetColor( colorTable[CT_LTBROWN1]);
	UI_DrawHandlePic( 96,  438, 268,  18, uis.whiteShader);	// Bottom front Line

	ui.R_SetColor( colorTable[CT_DKBROWN1]);
	UI_DrawHandlePic( 160,  58, 330,  18, uis.whiteShader);	// Torpodoes Line
	UI_DrawProportionalString( 180, 60, "SPEED", UI_SMALLFONT, colorTable[CT_LTGOLD1]);

	ui.R_SetColor( colorTable[CT_DKBROWN1]);
	UI_DrawHandlePic( 160,  195, 330,  18, uis.whiteShader);	// Torpodoes Line
	UI_DrawProportionalString( 180, 197, "COURSE HEADING", UI_SMALLFONT, colorTable[CT_LTGOLD1]);

	UI_DrawProportionalString( 300, 226, "MARK", UI_SMALLFONT|UI_CENTER, colorTable[CT_LTGOLD1]);

}

/*
===============
NavigationMenu_Draw
===============
*/
void NavigationMenu_Draw(void)
{
	M_Navigation_Graphics();
	
	Menu_Draw( &s_navigation.menu );
}

/*
=================
UI_NavigationMenu_Cache
=================
*/
void UI_NavigationMenu_Cache( void )
{
	s_navigation.cornerUpper = ui.R_RegisterShaderNoMip("menu/common/corner_ll_47_7.tga");
	s_navigation.cornerUpper2= ui.R_RegisterShaderNoMip("menu/common/corner_ul_47_7.tga");
	s_navigation.cornerLower = ui.R_RegisterShaderNoMip("menu/common/corner_ll_47_18.tga");
}

/*
=================
NavigationMenu_Init
=================
*/
static void NavigationMenu_Init( void )
{
	int x2;

	UI_NavigationMenu_Cache();

	s_navigation.menu.nitems					= 0;
	s_navigation.menu.wrapAround				= qtrue;
	s_navigation.menu.opening					= NULL;
	s_navigation.menu.closing					= NULL;
	s_navigation.menu.draw						= NavigationMenu_Draw;
	s_navigation.menu.key						= Navigation_MenuKey;
	s_navigation.menu.fullscreen				= qtrue;
	s_navigation.menu.descX						= MENU_DESC_X;
	s_navigation.menu.descY						= MENU_DESC_Y;
	s_navigation.menu.listX						= 230;
	s_navigation.menu.listY						= 188;
	s_navigation.menu.titleX						= MENU_TITLE_X;
	s_navigation.menu.titleY						= MENU_TITLE_Y;
	s_navigation.menu.titleI						= MNT_TACTICAL;
	s_navigation.menu.footNoteEnum				= MNT_TACTICAL;

	x2 = 340;
	s_navigation.mainmenu.generic.type			= MTYPE_BITMAP;      
	s_navigation.mainmenu.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_navigation.mainmenu.generic.x				= 130;
	s_navigation.mainmenu.generic.y				= 398;
	s_navigation.mainmenu.generic.name			= GRAPHIC_BUTTONRIGHT;
	s_navigation.mainmenu.generic.id			= ID_MAINMENU;
	s_navigation.mainmenu.generic.callback		= Navigation_MenuEvent;
	s_navigation.mainmenu.width					= MENU_BUTTON_MED_WIDTH;
	s_navigation.mainmenu.height					= MENU_BUTTON_MED_HEIGHT;
	s_navigation.mainmenu.color					= CT_DKPURPLE1;
	s_navigation.mainmenu.color2					= CT_LTPURPLE1;
	s_navigation.mainmenu.textX					= MENU_BUTTON_TEXT_X;
	s_navigation.mainmenu.textY					= MENU_BUTTON_TEXT_Y;
	s_navigation.mainmenu.textEnum				= MBT_HAZARDGAMERETURN;
	s_navigation.mainmenu.textcolor				= CT_BLACK;
	s_navigation.mainmenu.textcolor2				= CT_WHITE;


	int x,y;

	x = 180;
	y = 90;
	s_navigation.warp_slider.generic.type		= MTYPE_SLIDER;
	s_navigation.warp_slider.generic.x			= x + 162;
	s_navigation.warp_slider.generic.y			= y;
	s_navigation.warp_slider.generic.flags		= QMF_SMALLFONT;
	s_navigation.warp_slider.generic.callback	= WarpCallback;
	s_navigation.warp_slider.minvalue			= 5;
	s_navigation.warp_slider.maxvalue			= 30;
	s_navigation.warp_slider.color				= CT_DKPURPLE1;
	s_navigation.warp_slider.color2			= CT_LTPURPLE1;
	s_navigation.warp_slider.generic.name		= "menu/common/monbar_2.tga";
	s_navigation.warp_slider.width				= 256;
	s_navigation.warp_slider.height			= 32;
	s_navigation.warp_slider.focusWidth		= 145;
	s_navigation.warp_slider.focusHeight		= 18;
	s_navigation.warp_slider.picName			= GRAPHIC_SQUARE;
	s_navigation.warp_slider.picX				= x;
	s_navigation.warp_slider.picY				= y;
	s_navigation.warp_slider.picWidth			= MENU_BUTTON_MED_WIDTH + 21;
	s_navigation.warp_slider.picHeight			= MENU_BUTTON_MED_HEIGHT;
	s_navigation.warp_slider.textX				= MENU_BUTTON_TEXT_X;
	s_navigation.warp_slider.textY				= MENU_BUTTON_TEXT_Y;
	s_navigation.warp_slider.textEnum			= MBT_WARPDRIVE;
	s_navigation.warp_slider.textcolor			= CT_BLACK;
	s_navigation.warp_slider.textcolor2		= CT_WHITE;
	s_navigation.warp_slider.thumbName			= GRAPHIC_BUTTONSLIDER;
	s_navigation.warp_slider.thumbHeight		= 32;
	s_navigation.warp_slider.thumbWidth		= 16;
	s_navigation.warp_slider.thumbGraphicWidth	= 9;
	s_navigation.warp_slider.thumbColor		= CT_DKBLUE1;
	s_navigation.warp_slider.thumbColor2		= CT_LTBLUE1;


	s_navigation.impulse_slider.generic.type	= MTYPE_SLIDER;
	s_navigation.impulse_slider.generic.x		= x + 162;
	s_navigation.impulse_slider.generic.y		= y + 40;
	s_navigation.impulse_slider.generic.flags	= QMF_SMALLFONT;
	s_navigation.impulse_slider.generic.callback	= ImpulseCallback;
	s_navigation.impulse_slider.minvalue		= 5;
	s_navigation.impulse_slider.maxvalue		= 30;
	s_navigation.impulse_slider.color			= CT_VDKPURPLE1;
	s_navigation.impulse_slider.color2			= CT_DKPURPLE1;
	s_navigation.impulse_slider.generic.name	= "menu/common/monbar_2.tga";
	s_navigation.impulse_slider.width			= 256;
	s_navigation.impulse_slider.height			= 32;
	s_navigation.impulse_slider.focusWidth		= 145;
	s_navigation.impulse_slider.focusHeight		= 18;
	s_navigation.impulse_slider.picName			= GRAPHIC_SQUARE;
	s_navigation.impulse_slider.picX			= x;
	s_navigation.impulse_slider.picY			= y + 40;
	s_navigation.impulse_slider.picWidth		= MENU_BUTTON_MED_WIDTH + 21;
	s_navigation.impulse_slider.picHeight		= MENU_BUTTON_MED_HEIGHT;
	s_navigation.impulse_slider.textX			= MENU_BUTTON_TEXT_X;
	s_navigation.impulse_slider.textY			= MENU_BUTTON_TEXT_Y;
	s_navigation.impulse_slider.textEnum		= MBT_IMPULSEDRIVE;
	s_navigation.impulse_slider.textcolor		= CT_BLACK;
	s_navigation.impulse_slider.textcolor2		= CT_WHITE;
	s_navigation.impulse_slider.thumbName		= GRAPHIC_BUTTONSLIDER;
	s_navigation.impulse_slider.thumbHeight		= 32;
	s_navigation.impulse_slider.thumbWidth		= 16;
	s_navigation.impulse_slider.thumbGraphicWidth	= 9;
	s_navigation.impulse_slider.thumbColor		= CT_DKBLUE1;
	s_navigation.impulse_slider.thumbColor2		= CT_LTBLUE1;

	s_navigation.heading1.generic.type				= MTYPE_FIELD;
	s_navigation.heading1.generic.x					= 200;
	s_navigation.heading1.generic.y					= 226;
	s_navigation.heading1.field.style				= UI_SMALLFONT;
	s_navigation.heading1.field.widthInChars		= 6;
	s_navigation.heading1.field.maxchars			= 6;
	s_navigation.heading1.field.style				= UI_TINYFONT;

	s_navigation.heading2.generic.type				= MTYPE_FIELD;
	s_navigation.heading2.generic.x					= 350;
	s_navigation.heading2.generic.y					= 226;
	s_navigation.heading2.field.style				= UI_SMALLFONT;
	s_navigation.heading2.field.widthInChars		= 6;
	s_navigation.heading2.field.maxchars			= 6;
	s_navigation.heading2.field.style				= UI_TINYFONT;

	s_navigation.engage.generic.type		= MTYPE_BITMAP;
	s_navigation.engage.generic.flags		= QMF_HIGHLIGHT_IF_FOCUS;
	s_navigation.engage.generic.x			= 262;
	s_navigation.engage.generic.y			= 316;
	s_navigation.engage.generic.name		= "menu/common/square.tga";
	s_navigation.engage.generic.id			= ID_STARTGAME;
	s_navigation.engage.generic.callback	= Navigation_MenuEvent;
	s_navigation.engage.width				= MENU_BUTTON_MED_WIDTH + 40;
	s_navigation.engage.height				= 60;
	s_navigation.engage.color				= CT_DKPURPLE1;
	s_navigation.engage.color2				= CT_LTPURPLE1;
	s_navigation.engage.textX				= MENU_BUTTON_TEXT_X + 30;
	s_navigation.engage.textY				= 40;
	s_navigation.engage.textEnum			= MBT_ENGAGE;
	s_navigation.engage.textcolor			= CT_BLACK;
	s_navigation.engage.textcolor2			= CT_WHITE;

	Menu_AddItem( &s_navigation.menu, ( void * )&s_navigation.mainmenu);
	Menu_AddItem( &s_navigation.menu, ( void * )&s_navigation.warp_slider);
	Menu_AddItem( &s_navigation.menu, ( void * )&s_navigation.impulse_slider);
	Menu_AddItem( &s_navigation.menu, ( void * )&s_navigation.heading1);
	Menu_AddItem( &s_navigation.menu, ( void * )&s_navigation.heading2);
	Menu_AddItem( &s_navigation.menu, ( void * )&s_navigation.engage);
}

/*
===============
UI_NavigationMenu
===============
*/
void UI_NavigationMenu(void)
{
	uis.menusp = 0;

	ingameFlag = qtrue;	// true when in game menu is in use

	Mouse_Show();

	UI_NavigationMenu_Cache();

	NavigationMenu_Init(); 

	UI_PushMenu( &s_navigation.menu );

	Menu_AdjustCursor( &s_navigation.menu, 1 );	
}


typedef struct 
{
	menuframework_s menu;

	qhandle_t		cornerUpper;
	qhandle_t		cornerUpper2;
	qhandle_t		cornerLower;

	menubitmap_s	mainGenerator1;
	menubitmap_s	mainGenerator2;
	menubitmap_s	mainGenerator3;
	menubitmap_s	auxGenerator1;
	menubitmap_s	auxGenerator2;

	menuslider_s	sensors;
	menuslider_s	phasers;
	menuslider_s	shields;
	menuslider_s	engines;
	menuslider_s	environment;
	menuslider_s	misc;

	menubitmap_s	mainmenu;
} ops_t;

static ops_t	s_ops;


#define	ID_GENERATOR1					104
#define	ID_GENERATOR2					105
#define	ID_GENERATOR3					106
#define	ID_AUXGENERATOR1				107
#define	ID_AUXGENERATOR2				108



/*
=================
Ops_MenuEvent
=================
*/
static void Ops_MenuEvent( void* ptr, int notification )
{
	menuframework_s*	m;

	if (notification != QM_ACTIVATED)
		return;

	m = ((menucommon_s*)ptr)->parent;

	switch (((menucommon_s*)ptr)->id)
	{
		case ID_MAINMENU:
			UI_PopMenu();
			break;

		case ID_GENERATOR1:
			if (s_ops.mainGenerator1.color	== CT_DKPURPLE1)
			{
				s_ops.mainGenerator1.color				= CT_VDKPURPLE1;
				s_ops.mainGenerator1.color2				= CT_DKPURPLE1;
			}
			else
			{
				s_ops.mainGenerator1.color				= CT_DKPURPLE1;
				s_ops.mainGenerator1.color2				= CT_LTPURPLE1;
			}
			break;

		case ID_GENERATOR2:
			if (s_ops.mainGenerator2.color	== CT_DKPURPLE1)
			{
				s_ops.mainGenerator2.color				= CT_VDKPURPLE1;
				s_ops.mainGenerator2.color2				= CT_DKPURPLE1;
			}
			else
			{
				s_ops.mainGenerator2.color				= CT_DKPURPLE1;
				s_ops.mainGenerator2.color2				= CT_LTPURPLE1;
			}
			break;

		case ID_GENERATOR3:
			if (s_ops.mainGenerator3.color	== CT_DKPURPLE1)
			{
				s_ops.mainGenerator3.color				= CT_VDKPURPLE1;
				s_ops.mainGenerator3.color2				= CT_DKPURPLE1;
			}
			else
			{
				s_ops.mainGenerator3.color				= CT_DKPURPLE1;
				s_ops.mainGenerator3.color2				= CT_LTPURPLE1;
			}
			break;

		case ID_AUXGENERATOR1:
			if (s_ops.auxGenerator1.color	== CT_DKPURPLE1)
			{
				s_ops.auxGenerator1.color				= CT_VDKPURPLE1;
				s_ops.auxGenerator1.color2				= CT_DKPURPLE1;
			}
			else
			{
				s_ops.auxGenerator1.color				= CT_DKPURPLE1;
				s_ops.auxGenerator1.color2				= CT_LTPURPLE1;
			}
			break;

		case ID_AUXGENERATOR2:
			if (s_ops.auxGenerator2.color	== CT_DKPURPLE1)
			{
				s_ops.auxGenerator2.color				= CT_VDKPURPLE1;
				s_ops.auxGenerator2.color2				= CT_DKPURPLE1;
			}
			else
			{
				s_ops.auxGenerator2.color				= CT_DKPURPLE1;
				s_ops.auxGenerator2.color2				= CT_LTPURPLE1;
			}
			break;
	}
}

/*
=================
Ops_MenuKey
=================
*/
static sfxHandle_t Ops_MenuKey( int key )
{
	return Menu_DefaultKey( &s_ops.menu, key );
}

/*
=================
M_Ops_Graphics
=================
*/
static void M_Ops_Graphics (void)
{
	// Frame
	ui.R_SetColor( colorTable[CT_DKBROWN1]);
	UI_DrawHandlePic(  30, 20, 128,  64, s_engineeringStatus.cornerUpper2);	// Top corner
	UI_DrawHandlePic(  50, 20,  99,   7, uis.whiteShader);
	UI_DrawHandlePic( 152, 20, 135,   7, uis.whiteShader);

	ui.R_SetColor( colorTable[CT_DKBROWN1]);
	UI_DrawHandlePic( 290, 20,  12,   7, uis.whiteShader);

	ui.R_SetColor( colorTable[CT_LTBROWN1]);
	UI_DrawHandlePic( 305, 20,  60,   4, uis.whiteShader);

	ui.R_SetColor( colorTable[CT_DKBROWN1]);
	UI_DrawHandlePic( 368, 20, 111,   7, uis.whiteShader);


	// Side left column
	ui.R_SetColor( colorTable[CT_DKBROWN1]);
	UI_DrawHandlePic( 30, 51,   20,   119, uis.whiteShader);
	UI_DrawHandlePic( 30,203,   20,   186, uis.whiteShader);

	ui.R_SetColor( colorTable[CT_DKBROWN1]);
	UI_DrawHandlePic(  30, 173,  20,  27, uis.whiteShader);	// Top left column
	UI_DrawHandlePic(  30, 392,  20,  33, uis.whiteShader);	// Bottom left column
	UI_DrawHandlePic(  30, 425, 128,  64, s_engineeringStatus.cornerLower);// Bottom Left Corner

	ui.R_SetColor( colorTable[CT_LTBROWN1]);
	UI_DrawHandlePic( 96,  438, 268,  18, uis.whiteShader);	// Bottom front Line

	ui.R_SetColor( colorTable[CT_DKBROWN1]);
	UI_DrawHandlePic( 160,  60, 330,  18, uis.whiteShader);	// Torpodoes Line
	UI_DrawProportionalString( 180, 62, "POWER AVAILABLE", UI_SMALLFONT, colorTable[CT_LTGOLD1]);


	ui.R_SetColor( colorTable[CT_DKBROWN1]);
	UI_DrawHandlePic( 160,  250, 330,  18, uis.whiteShader);	// Torpodoes Line
	UI_DrawProportionalString( 180, 252, "POWER CONSUMPTION", UI_SMALLFONT, colorTable[CT_LTGOLD1]);

}

/*
===============
OpsMenu_Draw
===============
*/
void OpsMenu_Draw(void)
{
	M_Ops_Graphics();
	
	Menu_Draw( &s_ops.menu );
}

/*
=================
UI_OpsMenu_Cache
=================
*/
void UI_OpsMenu_Cache( void )
{
	s_ops.cornerUpper = ui.R_RegisterShaderNoMip("menu/common/corner_ll_47_7.tga");
	s_ops.cornerUpper2= ui.R_RegisterShaderNoMip("menu/common/corner_ul_47_7.tga");
	s_ops.cornerLower = ui.R_RegisterShaderNoMip("menu/common/corner_ll_47_18.tga");
}

/*
=================
OpsMenu_Init
=================
*/
static void OpsMenu_Init( void )
{
	int x2;

	UI_OpsMenu_Cache();

	s_ops.menu.nitems					= 0;
	s_ops.menu.wrapAround				= qtrue;
	s_ops.menu.opening					= NULL;
	s_ops.menu.closing					= NULL;
	s_ops.menu.draw						= OpsMenu_Draw;
	s_ops.menu.key						= Ops_MenuKey;
	s_ops.menu.fullscreen				= qtrue;
	s_ops.menu.descX						= MENU_DESC_X;
	s_ops.menu.descY						= MENU_DESC_Y;
	s_ops.menu.listX						= 230;
	s_ops.menu.listY						= 188;
	s_ops.menu.titleX						= MENU_TITLE_X;
	s_ops.menu.titleY						= MENU_TITLE_Y;
	s_ops.menu.titleI						= MNT_TACTICAL;
	s_ops.menu.footNoteEnum				= MNT_TACTICAL;


	s_ops.mainGenerator1.generic.type		= MTYPE_BITMAP;
	s_ops.mainGenerator1.generic.flags		= QMF_HIGHLIGHT_IF_FOCUS;
	s_ops.mainGenerator1.generic.x			= 80;
	s_ops.mainGenerator1.generic.y			= 98;
	s_ops.mainGenerator1.generic.name		= "menu/common/square.tga";
	s_ops.mainGenerator1.generic.id			= ID_GENERATOR1;
	s_ops.mainGenerator1.generic.callback	= Ops_MenuEvent;
	s_ops.mainGenerator1.width				= MENU_BUTTON_MED_WIDTH + 10;
	s_ops.mainGenerator1.height				= 40;
	s_ops.mainGenerator1.color				= CT_DKPURPLE1;
	s_ops.mainGenerator1.color2				= CT_LTPURPLE1;
	s_ops.mainGenerator1.textX				= MENU_BUTTON_TEXT_X + 30;
	s_ops.mainGenerator1.textY				= 20;
	s_ops.mainGenerator1.textEnum			= MBT_GENERATOR1;
	s_ops.mainGenerator1.textcolor			= CT_BLACK;
	s_ops.mainGenerator1.textcolor2			= CT_WHITE;

	s_ops.mainGenerator2.generic.type		= MTYPE_BITMAP;
	s_ops.mainGenerator2.generic.flags		= QMF_HIGHLIGHT_IF_FOCUS;
	s_ops.mainGenerator2.generic.x			= 280;
	s_ops.mainGenerator2.generic.y			= 98;
	s_ops.mainGenerator2.generic.name		= "menu/common/square.tga";
	s_ops.mainGenerator2.generic.id			= ID_GENERATOR2;
	s_ops.mainGenerator2.generic.callback	= Ops_MenuEvent;
	s_ops.mainGenerator2.width				= MENU_BUTTON_MED_WIDTH + 10;
	s_ops.mainGenerator2.height				= 40;
	s_ops.mainGenerator2.color				= CT_DKPURPLE1;
	s_ops.mainGenerator2.color2				= CT_LTPURPLE1;
	s_ops.mainGenerator2.textX				= MENU_BUTTON_TEXT_X + 30;
	s_ops.mainGenerator2.textY				= 20;
	s_ops.mainGenerator2.textEnum			= MBT_GENERATOR2;
	s_ops.mainGenerator2.textcolor			= CT_BLACK;
	s_ops.mainGenerator2.textcolor2			= CT_WHITE;

	s_ops.mainGenerator3.generic.type		= MTYPE_BITMAP;
	s_ops.mainGenerator3.generic.flags		= QMF_HIGHLIGHT_IF_FOCUS;
	s_ops.mainGenerator3.generic.x			= 480;
	s_ops.mainGenerator3.generic.y			= 98;
	s_ops.mainGenerator3.generic.name		= "menu/common/square.tga";
	s_ops.mainGenerator3.generic.id			= ID_GENERATOR3;
	s_ops.mainGenerator3.generic.callback	= Ops_MenuEvent;
	s_ops.mainGenerator3.width				= MENU_BUTTON_MED_WIDTH + 10;
	s_ops.mainGenerator3.height				= 40;
	s_ops.mainGenerator3.color				= CT_VDKPURPLE1;
	s_ops.mainGenerator3.color2				= CT_DKPURPLE1;
	s_ops.mainGenerator3.textX				= MENU_BUTTON_TEXT_X + 30;
	s_ops.mainGenerator3.textY				= 20;
	s_ops.mainGenerator3.textEnum			= MBT_GENERATOR3;
	s_ops.mainGenerator3.textcolor			= CT_BLACK;
	s_ops.mainGenerator3.textcolor2			= CT_WHITE;


	s_ops.auxGenerator1.generic.type		= MTYPE_BITMAP;
	s_ops.auxGenerator1.generic.flags		= QMF_HIGHLIGHT_IF_FOCUS;
	s_ops.auxGenerator1.generic.x			= 188;
	s_ops.auxGenerator1.generic.y			= 158;
	s_ops.auxGenerator1.generic.name		= "menu/common/square.tga";
	s_ops.auxGenerator1.generic.id			= ID_AUXGENERATOR1;
	s_ops.auxGenerator1.generic.callback	= Ops_MenuEvent;
	s_ops.auxGenerator1.width				= MENU_BUTTON_MED_WIDTH;
	s_ops.auxGenerator1.height				= 30;
	s_ops.auxGenerator1.color				= CT_VDKPURPLE1;
	s_ops.auxGenerator1.color2				= CT_DKPURPLE1;
	s_ops.auxGenerator1.textX				= MENU_BUTTON_TEXT_X;
	s_ops.auxGenerator1.textY				= 10;
	s_ops.auxGenerator1.textEnum			= MBT_AUXGENERATOR1;
	s_ops.auxGenerator1.textcolor			= CT_BLACK;
	s_ops.auxGenerator1.textcolor2			= CT_WHITE;

	s_ops.auxGenerator2.generic.type		= MTYPE_BITMAP;
	s_ops.auxGenerator2.generic.flags		= QMF_HIGHLIGHT_IF_FOCUS;
	s_ops.auxGenerator2.generic.x			= 388;
	s_ops.auxGenerator2.generic.y			= 158;
	s_ops.auxGenerator2.generic.name		= "menu/common/square.tga";
	s_ops.auxGenerator2.generic.id			= ID_AUXGENERATOR2;
	s_ops.auxGenerator2.generic.callback	= Ops_MenuEvent;
	s_ops.auxGenerator2.width				= MENU_BUTTON_MED_WIDTH;
	s_ops.auxGenerator2.height				= 30;
	s_ops.auxGenerator2.color				= CT_VDKPURPLE1;
	s_ops.auxGenerator2.color2				= CT_DKPURPLE1;
	s_ops.auxGenerator2.textX				= MENU_BUTTON_TEXT_X;
	s_ops.auxGenerator2.textY				= 10;
	s_ops.auxGenerator2.textEnum			= MBT_AUXGENERATOR2;
	s_ops.auxGenerator2.textcolor			= CT_BLACK;
	s_ops.auxGenerator2.textcolor2			= CT_WHITE;



	x2 = 340;
	s_ops.mainmenu.generic.type			= MTYPE_BITMAP;      
	s_ops.mainmenu.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_ops.mainmenu.generic.x				= 130;
	s_ops.mainmenu.generic.y				= 398;
	s_ops.mainmenu.generic.name			= GRAPHIC_BUTTONRIGHT;
	s_ops.mainmenu.generic.id			= ID_MAINMENU;
	s_ops.mainmenu.generic.callback		= Ops_MenuEvent;
	s_ops.mainmenu.width					= MENU_BUTTON_MED_WIDTH;
	s_ops.mainmenu.height					= MENU_BUTTON_MED_HEIGHT;
	s_ops.mainmenu.color					= CT_DKPURPLE1;
	s_ops.mainmenu.color2					= CT_LTPURPLE1;
	s_ops.mainmenu.textX					= MENU_BUTTON_TEXT_X;
	s_ops.mainmenu.textY					= MENU_BUTTON_TEXT_Y;
	s_ops.mainmenu.textEnum				= MBT_HAZARDGAMERETURN;
	s_ops.mainmenu.textcolor				= CT_BLACK;
	s_ops.mainmenu.textcolor2				= CT_WHITE;


	int x,y;

	x = 60;
	y = 280;
	s_ops.sensors.generic.type		= MTYPE_SLIDER;
	s_ops.sensors.generic.x			= x + 121;
	s_ops.sensors.generic.y			= y;
	s_ops.sensors.generic.flags		= QMF_SMALLFONT;
	s_ops.sensors.generic.callback	= WarpCallback;
	s_ops.sensors.minvalue			= 5;
	s_ops.sensors.maxvalue			= 30;
	s_ops.sensors.color				= CT_DKPURPLE1;
	s_ops.sensors.color2			= CT_LTPURPLE1;
	s_ops.sensors.generic.name		= "menu/common/monbar_2.tga";
	s_ops.sensors.width				= 256;
	s_ops.sensors.height			= 32;
	s_ops.sensors.focusWidth		= 145;
	s_ops.sensors.focusHeight		= 18;
	s_ops.sensors.picName			= GRAPHIC_SQUARE;
	s_ops.sensors.picX				= x;
	s_ops.sensors.picY				= y;
	s_ops.sensors.picWidth			= MENU_BUTTON_MED_WIDTH - 20;
	s_ops.sensors.picHeight			= MENU_BUTTON_MED_HEIGHT;
	s_ops.sensors.textX				= MENU_BUTTON_TEXT_X;
	s_ops.sensors.textY				= MENU_BUTTON_TEXT_Y;
	s_ops.sensors.textEnum			= MBT_POWER_SENSORS;
	s_ops.sensors.textcolor			= CT_BLACK;
	s_ops.sensors.textcolor2		= CT_WHITE;
	s_ops.sensors.thumbName			= GRAPHIC_BUTTONSLIDER;
	s_ops.sensors.thumbHeight		= 32;
	s_ops.sensors.thumbWidth		= 16;
	s_ops.sensors.thumbGraphicWidth	= 9;
	s_ops.sensors.thumbColor		= CT_DKBLUE1;
	s_ops.sensors.thumbColor2		= CT_LTBLUE1;

	y += 30;
	s_ops.phasers.generic.type		= MTYPE_SLIDER;
	s_ops.phasers.generic.x			= x + 121;
	s_ops.phasers.generic.y			= y;
	s_ops.phasers.generic.flags		= QMF_SMALLFONT;
	s_ops.phasers.generic.callback	= WarpCallback;
	s_ops.phasers.minvalue			= 5;
	s_ops.phasers.maxvalue			= 30;
	s_ops.phasers.color				= CT_DKPURPLE1;
	s_ops.phasers.color2			= CT_LTPURPLE1;
	s_ops.phasers.generic.name		= "menu/common/monbar_2.tga";
	s_ops.phasers.width				= 256;
	s_ops.phasers.height			= 32;
	s_ops.phasers.focusWidth		= 145;
	s_ops.phasers.focusHeight		= 18;
	s_ops.phasers.picName			= GRAPHIC_SQUARE;
	s_ops.phasers.picX				= x;
	s_ops.phasers.picY				= y;
	s_ops.phasers.picWidth			= MENU_BUTTON_MED_WIDTH - 20;
	s_ops.phasers.picHeight			= MENU_BUTTON_MED_HEIGHT;
	s_ops.phasers.textX				= MENU_BUTTON_TEXT_X;
	s_ops.phasers.textY				= MENU_BUTTON_TEXT_Y;
	s_ops.phasers.textEnum			= MBT_POWER_PHASERS;
	s_ops.phasers.textcolor			= CT_BLACK;
	s_ops.phasers.textcolor2		= CT_WHITE;
	s_ops.phasers.thumbName			= GRAPHIC_BUTTONSLIDER;
	s_ops.phasers.thumbHeight		= 32;
	s_ops.phasers.thumbWidth		= 16;
	s_ops.phasers.thumbGraphicWidth	= 9;
	s_ops.phasers.thumbColor		= CT_DKBLUE1;
	s_ops.phasers.thumbColor2		= CT_LTBLUE1;

	y += 30;
	s_ops.shields.generic.type		= MTYPE_SLIDER;
	s_ops.shields.generic.x			= x + 121;
	s_ops.shields.generic.y			= y;
	s_ops.shields.generic.flags		= QMF_SMALLFONT;
	s_ops.shields.generic.callback	= WarpCallback;
	s_ops.shields.minvalue			= 5;
	s_ops.shields.maxvalue			= 30;
	s_ops.shields.color				= CT_DKPURPLE1;
	s_ops.shields.color2			= CT_LTPURPLE1;
	s_ops.shields.generic.name		= "menu/common/monbar_2.tga";
	s_ops.shields.width				= 256;
	s_ops.shields.height			= 32;
	s_ops.shields.focusWidth		= 145;
	s_ops.shields.focusHeight		= 18;
	s_ops.shields.picName			= GRAPHIC_SQUARE;
	s_ops.shields.picX				= x;
	s_ops.shields.picY				= y;
	s_ops.shields.picWidth			= MENU_BUTTON_MED_WIDTH - 20;
	s_ops.shields.picHeight			= MENU_BUTTON_MED_HEIGHT;
	s_ops.shields.textX				= MENU_BUTTON_TEXT_X;
	s_ops.shields.textY				= MENU_BUTTON_TEXT_Y;
	s_ops.shields.textEnum			= MBT_POWER_SHIELDS;
	s_ops.shields.textcolor			= CT_BLACK;
	s_ops.shields.textcolor2		= CT_WHITE;
	s_ops.shields.thumbName			= GRAPHIC_BUTTONSLIDER;
	s_ops.shields.thumbHeight		= 32;
	s_ops.shields.thumbWidth		= 16;
	s_ops.shields.thumbGraphicWidth	= 9;
	s_ops.shields.thumbColor		= CT_DKBLUE1;
	s_ops.shields.thumbColor2		= CT_LTBLUE1;


	x = 360;
	y = 280;
	s_ops.engines.generic.type		= MTYPE_SLIDER;
	s_ops.engines.generic.x			= x + 121;
	s_ops.engines.generic.y			= y;
	s_ops.engines.generic.flags		= QMF_SMALLFONT;
	s_ops.engines.generic.callback	= WarpCallback;
	s_ops.engines.minvalue			= 5;
	s_ops.engines.maxvalue			= 30;
	s_ops.engines.color				= CT_DKPURPLE1;
	s_ops.engines.color2			= CT_LTPURPLE1;
	s_ops.engines.generic.name		= "menu/common/monbar_2.tga";
	s_ops.engines.width				= 256;
	s_ops.engines.height			= 32;
	s_ops.engines.focusWidth		= 145;
	s_ops.engines.focusHeight		= 18;
	s_ops.engines.picName			= GRAPHIC_SQUARE;
	s_ops.engines.picX				= x;
	s_ops.engines.picY				= y;
	s_ops.engines.picWidth			= MENU_BUTTON_MED_WIDTH - 20;
	s_ops.engines.picHeight			= MENU_BUTTON_MED_HEIGHT;
	s_ops.engines.textX				= MENU_BUTTON_TEXT_X;
	s_ops.engines.textY				= MENU_BUTTON_TEXT_Y;
	s_ops.engines.textEnum			= MBT_POWER_ENGINES;
	s_ops.engines.textcolor			= CT_BLACK;
	s_ops.engines.textcolor2		= CT_WHITE;
	s_ops.engines.thumbName			= GRAPHIC_BUTTONSLIDER;
	s_ops.engines.thumbHeight		= 32;
	s_ops.engines.thumbWidth		= 16;
	s_ops.engines.thumbGraphicWidth	= 9;
	s_ops.engines.thumbColor		= CT_DKBLUE1;
	s_ops.engines.thumbColor2		= CT_LTBLUE1;

	y += 30;
	s_ops.environment.generic.type		= MTYPE_SLIDER;
	s_ops.environment.generic.x			= x + 121;
	s_ops.environment.generic.y			= y;
	s_ops.environment.generic.flags		= QMF_SMALLFONT;
	s_ops.environment.generic.callback	= WarpCallback;
	s_ops.environment.minvalue			= 5;
	s_ops.environment.maxvalue			= 30;
	s_ops.environment.color				= CT_DKPURPLE1;
	s_ops.environment.color2			= CT_LTPURPLE1;
	s_ops.environment.generic.name		= "menu/common/monbar_2.tga";
	s_ops.environment.width				= 256;
	s_ops.environment.height			= 32;
	s_ops.environment.focusWidth		= 145;
	s_ops.environment.focusHeight		= 18;
	s_ops.environment.picName			= GRAPHIC_SQUARE;
	s_ops.environment.picX				= x;
	s_ops.environment.picY				= y;
	s_ops.environment.picWidth			= MENU_BUTTON_MED_WIDTH - 20;
	s_ops.environment.picHeight			= MENU_BUTTON_MED_HEIGHT;
	s_ops.environment.textX				= MENU_BUTTON_TEXT_X;
	s_ops.environment.textY				= MENU_BUTTON_TEXT_Y;
	s_ops.environment.textEnum			= MBT_POWER_ENVIRONMENT;
	s_ops.environment.textcolor			= CT_BLACK;
	s_ops.environment.textcolor2		= CT_WHITE;
	s_ops.environment.thumbName			= GRAPHIC_BUTTONSLIDER;
	s_ops.environment.thumbHeight		= 32;
	s_ops.environment.thumbWidth		= 16;
	s_ops.environment.thumbGraphicWidth	= 9;
	s_ops.environment.thumbColor		= CT_DKBLUE1;
	s_ops.environment.thumbColor2		= CT_LTBLUE1;

	y += 30;
	s_ops.misc.generic.type		= MTYPE_SLIDER;
	s_ops.misc.generic.x			= x + 121;
	s_ops.misc.generic.y			= y;
	s_ops.misc.generic.flags		= QMF_SMALLFONT;
	s_ops.misc.generic.callback	= WarpCallback;
	s_ops.misc.minvalue			= 5;
	s_ops.misc.maxvalue			= 30;
	s_ops.misc.color				= CT_DKPURPLE1;
	s_ops.misc.color2			= CT_LTPURPLE1;
	s_ops.misc.generic.name		= "menu/common/monbar_2.tga";
	s_ops.misc.width				= 256;
	s_ops.misc.height			= 32;
	s_ops.misc.focusWidth		= 145;
	s_ops.misc.focusHeight		= 18;
	s_ops.misc.picName			= GRAPHIC_SQUARE;
	s_ops.misc.picX				= x;
	s_ops.misc.picY				= y;
	s_ops.misc.picWidth			= MENU_BUTTON_MED_WIDTH - 20;
	s_ops.misc.picHeight			= MENU_BUTTON_MED_HEIGHT;
	s_ops.misc.textX				= MENU_BUTTON_TEXT_X;
	s_ops.misc.textY				= MENU_BUTTON_TEXT_Y;
	s_ops.misc.textEnum			= MBT_POWER_MISC;
	s_ops.misc.textcolor			= CT_BLACK;
	s_ops.misc.textcolor2		= CT_WHITE;
	s_ops.misc.thumbName			= GRAPHIC_BUTTONSLIDER;
	s_ops.misc.thumbHeight		= 32;
	s_ops.misc.thumbWidth		= 16;
	s_ops.misc.thumbGraphicWidth	= 9;
	s_ops.misc.thumbColor		= CT_DKBLUE1;
	s_ops.misc.thumbColor2		= CT_LTBLUE1;

	Menu_AddItem( &s_ops.menu, ( void * )&s_ops.mainmenu);

	Menu_AddItem( &s_ops.menu, ( void * )&s_ops.mainGenerator1);
	Menu_AddItem( &s_ops.menu, ( void * )&s_ops.mainGenerator2);
	Menu_AddItem( &s_ops.menu, ( void * )&s_ops.mainGenerator3);

	Menu_AddItem( &s_ops.menu, ( void * )&s_ops.auxGenerator1);
	Menu_AddItem( &s_ops.menu, ( void * )&s_ops.auxGenerator2);

	Menu_AddItem( &s_ops.menu, ( void * )&s_ops.sensors);
	Menu_AddItem( &s_ops.menu, ( void * )&s_ops.phasers);
	Menu_AddItem( &s_ops.menu, ( void * )&s_ops.shields);

	Menu_AddItem( &s_ops.menu, ( void * )&s_ops.engines);
	Menu_AddItem( &s_ops.menu, ( void * )&s_ops.environment);
	Menu_AddItem( &s_ops.menu, ( void * )&s_ops.misc);

	s_ops.sensors.curvalue	= 8;
	s_ops.phasers.curvalue	= 28;
	s_ops.shields.curvalue	= 18;
	s_ops.engines.curvalue	= 15;
	s_ops.environment.curvalue	= 20;
	s_ops.misc.curvalue			= 25;

}

/*
===============
UI_OpsMenu
===============
*/
void UI_OpsMenu(void)
{
	uis.menusp = 0;

	ingameFlag = qtrue;	// true when in game menu is in use

	Mouse_Show();

	UI_OpsMenu_Cache();

	OpsMenu_Init(); 

	UI_PushMenu( &s_ops.menu );

	Menu_AdjustCursor( &s_ops.menu, 1 );	
}
