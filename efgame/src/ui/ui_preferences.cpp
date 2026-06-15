#include "ui_local.h"

// Precache stuff for Game Options Menu
static struct 
{
	qhandle_t	slant1;
	qhandle_t	slant2;
	qhandle_t	swooptop;
	qhandle_t	swoopbottom;
	qhandle_t	singraph;
	qhandle_t	graphbox;
	qhandle_t	lswoop;
	qhandle_t	lswoop2;
	qhandle_t	tallswoop;
	qhandle_t	tallswoop2;

	qhandle_t	round1;
	qhandle_t	round2;
	qhandle_t	round10;
	qhandle_t	round11;
} gameoptionsmenu_cache;

#define ID_SUBTITLE				125
#define ID_TEXTLANGUAGE			126
#define ID_CROSSHAIR			127
#define ID_LIGHTFLARES			129
#define ID_LIGHTFLARES_VALUE	130
#define ID_EJECTINGBRASS		131
#define ID_WALLMARKS			132
#define ID_WALLMARKS_VALUE		133
#define ID_DYNAMICLIGHTS		134
#define ID_DYNAMICLIGHTS_VALUE	135
#define ID_IDENTIFYTARGET		136
#define ID_IDENTIFYTARGET_VALUE	137
#define ID_SYNCEVERYFRAME		138
#define ID_SYNCEVERYFRAME_VALUE 139
#define ID_VOICELANGUAGE		140
#define ID_RETURN				141


#define ID_MAINMENU		100
#define ID_CONTROLS		101
#define ID_VIDEO		102
#define ID_SOUND		103
#define ID_GAMEOPTIONS	104
#define ID_CDKEY		105

static menubitmap_s			s_preferences_crosshair_box;
static menubitmap_s			s_preferences_wallmarks_box;
static menubitmap_s			s_preferences_wallmarks_value;
static menubitmap_s			s_preferences_dynamiclights_box;
static menubitmap_s			s_preferences_dynamiclights_value;
static menubitmap_s			s_preferences_lightflares_box;
static menubitmap_s			s_preferences_lightflares_value;
static menubitmap_s			s_preferences_identifytarget_box;
static menubitmap_s			s_preferences_identifytarget_value;
static menubitmap_s			s_preferences_synceveryframe_box;
static menubitmap_s			s_preferences_synceveryframe_value;
static menulist_s			s_subtitle;
static menulist_s			s_textlanguage;
static menulist_s			s_voicelanguage;

static menuframework_s		s_gameoptions_menu;

#define NUM_CROSSHAIRS 12
static qhandle_t			crosshairShader[NUM_CROSSHAIRS];
static int	curCrosshair;

int s_textlanguage_Names[] =
{
	MNT_ENGLISH,
	MNT_GERMAN,
	MNT_FRENCH,
	MNT_NONE
};

int s_voicelanguage_Names[] =
{
	MNT_ENGLISH,
	MNT_GERMAN,
	MNT_NONE
};


typedef enum 
{
	GOMG_CENTERDOT1,
	GOMG_CENTERDOT2,
	GOMG_CENTERDOT3,
	GOMG_CENTERDOT4,

	GOMG_BLEFTSIDE,
	GOMG_BRIGHTSIDE,

	GOMG_SINE1,
	GOMG_SINE2,
	GOMG_SINE3,

	GOMG_NUMBERS_START,

	GOMG_COL1_ROW1,
	GOMG_COL1_ROW2,
	GOMG_COL1_ROW3,
	GOMG_COL1_ROW4,

	GOMG_COL2_ROW1,
	GOMG_COL2_ROW2,
	GOMG_COL2_ROW3,
	GOMG_COL2_ROW4,

	GOMG_COL3_ROW1,
	GOMG_COL3_ROW2,
	GOMG_COL3_ROW3,
	GOMG_COL3_ROW4,

	GOMG_COL4_ROW1,
	GOMG_COL4_ROW2,
	GOMG_COL4_ROW3,
	GOMG_COL4_ROW4,

	GOMG_COL5_ROW1,
	GOMG_COL5_ROW2,
	GOMG_COL5_ROW3,
	GOMG_COL5_ROW4,

	GOMG_COL6_ROW1,
	GOMG_COL6_ROW2,
	GOMG_COL6_ROW3,
	GOMG_COL6_ROW4,

	GOMG_COL7_ROW1,
	GOMG_COL7_ROW2,
	GOMG_COL7_ROW3,
	GOMG_COL7_ROW4,

	GOMG_COL8_ROW1,
	GOMG_COL8_ROW2,
	GOMG_COL8_ROW3,
	GOMG_COL8_ROW4,

	GOMG_NUMBERS_END,

	GOMG_MAX
} mainmenu_graphics_t;

menugraphics_s gameoptionsmenu_graphics[GOMG_MAX] = 
{
//	type		timer	x		y		width	height	file/text						graphic,	min		max	target	inc		style	color
	MG_GRAPHIC,	0.0,	157,	260,	64,		64,		"menu/lcarscontrols/round14.tga",0,0,NULL,		0,		0,	0,		1,		0,		CT_DKGOLD1,	NULL,	// GOMG_CENTERDOT1
	MG_GRAPHIC,	0.0,	196,	259,	64,		64,		"menu/lcarscontrols/round15.tga",0,0,NULL,		0,		0,	0,		1,		0,		CT_LTBLUE1,	NULL,	// GOMG_CENTERDOT2
	MG_GRAPHIC,	0.0,	196,	298,	64,		64,		"menu/lcarscontrols/round16.tga",0,0,NULL,		0,		0,	0,		1,		0,		CT_LTBLUE1,	NULL,	// GOMG_CENTERDOT3
	MG_GRAPHIC,	0.0,	157,	297,	64,		64,		"menu/lcarscontrols/round17.tga",0,0,NULL,		0,		0,	0,		1,		0,		CT_LTBLUE2,	NULL,	// GOMG_CENTERDOT4

	MG_VAR,		0.0,	0,		0,		0,		0,		NULL,						0,0,	NULL,		0,		0,	0,		1,		0,		CT_NONE,	NULL,	// GOMG_BLEFTSIDE
	MG_VAR,		0.0,	0,		0,		0,		0,		NULL,						0,0,	NULL,		0,		0,	0,		1,		0,		CT_NONE,	NULL,	// GOMG_BRIGHTSIDE

	MG_GRAPHIC,	0.0,	548,	214,	64,		166,	"menu/lcarscontrols/sin1.tga",0,0,	NULL,		0,		0,	0,		1,		0,		CT_RED,		NULL,	// GOMG_SINE1
	MG_GRAPHIC,	0.0,	548,	214,	64,		166,	"menu/lcarscontrols/sin2.tga",0,0,	NULL,		0,		0,	0,		1,		0,		CT_BLUE,	NULL,	// GOMG_SINE2
	MG_GRAPHIC,	0.0,	548,	214,	64,		166,	"menu/lcarscontrols/sin3.tga",0,0,	NULL,		0,		0,	0,		1,		0,		CT_YELLOW,	NULL,	// GOMG_SINE3

	MG_VAR,		0.0,	0,		0,		0,		0,		NULL,						0,0,	NULL,		0,		0,	0,		1,		0,		CT_NONE,	NULL,	// GOMG_NUMBERS_START

	MG_NUMBER,	0.0,	460,	 68,	16,		10,		NULL,						0,0,	NULL,		0,		2,	0,		12,		UI_TINYFONT,				CT_LTGOLD1,	NULL,	// GOMG_COL1_ROW1
	MG_NUMBER,	0.0,	460,	 80,	16,		10,		NULL,						0,0,	NULL,		0,		2,	0,		12,		UI_TINYFONT,				CT_LTGOLD1,	NULL,	// GOMG_COL1_ROW2
	MG_NUMBER,	0.0,	460,	 92,	16,		10,		NULL,						0,0,	NULL,		0,		2,	0,		12,		UI_TINYFONT,				CT_LTGOLD1,	NULL,	// GOMG_COL1_ROW3
	MG_NUMBER,	0.0,	460,	104,	16,		10,		NULL,						0,0,	NULL,		0,		2,	0,		12,		UI_TINYFONT,				CT_LTGOLD1,	NULL,	// GOMG_COL1_ROW4

	MG_NUMBER,	0.0,	474,	 68,	16,		10,		NULL,						0,0,	NULL,		0,		2,	0,		12,		UI_TINYFONT,				CT_LTGOLD1,	NULL,	// GOMG_COL2_ROW1
	MG_NUMBER,	0.0,	474,	 80,	16,		10,		NULL,						0,0,	NULL,		0,		2,	0,		12,		UI_TINYFONT,				CT_LTGOLD1,	NULL,	// GOMG_COL2_ROW2
	MG_NUMBER,	0.0,	474,	 92,	16,		10,		NULL,						0,0,	NULL,		0,		2,	0,		12,		UI_TINYFONT,				CT_LTGOLD1,	NULL,	// GOMG_COL2_ROW3
	MG_NUMBER,	0.0,	474,	104,	16,		10,		NULL,						0,0,	NULL,		0,		2,	0,		12,		UI_TINYFONT,				CT_LTGOLD1,	NULL,	// GOMG_COL2_ROW4

	MG_NUMBER,	0.0,	516,	 68,	16,		10,		NULL,						0,0,	NULL,		0,		2,	0,		12,		UI_TINYFONT,				CT_LTGOLD1,	NULL,	// GOMG_COL3_ROW1
	MG_NUMBER,	0.0,	516,	 80,	16,		10,		NULL,						0,0,	NULL,		0,		2,	0,		12,		UI_TINYFONT,				CT_LTGOLD1,	NULL,	// GOMG_COL3_ROW2
	MG_NUMBER,	0.0,	516,	 92,	16,		10,		NULL,						0,0,	NULL,		0,		2,	0,		12,		UI_TINYFONT,				CT_LTGOLD1,	NULL,	// GOMG_COL3_ROW3
	MG_NUMBER,	0.0,	516,	104,	16,		10,		NULL,						0,0,	NULL,		0,		2,	0,		12,		UI_TINYFONT,				CT_LTGOLD1,	NULL,	// GOMG_COL3_ROW4

	MG_NUMBER,	0.0,	530,	 68,	16,		10,		NULL,						0,0,	NULL,		0,		2,	0,		12,		UI_TINYFONT,				CT_LTGOLD1,	NULL,	// GOMG_COL4_ROW1
	MG_NUMBER,	0.0,	530,	 80,	16,		10,		NULL,						0,0,	NULL,		0,		2,	0,		12,		UI_TINYFONT,				CT_LTGOLD1,	NULL,	// GOMG_COL4_ROW2
	MG_NUMBER,	0.0,	530,	 92,	16,		10,		NULL,						0,0,	NULL,		0,		2,	0,		12,		UI_TINYFONT,				CT_LTGOLD1,	NULL,	// GOMG_COL4_ROW3
	MG_NUMBER,	0.0,	530,	104,	16,		10,		NULL,						0,0,	NULL,		0,		2,	0,		12,		UI_TINYFONT,				CT_LTGOLD1,	NULL,	// GOMG_COL4_ROW4

	MG_NUMBER,	0.0,	544,	 68,	16,		10,		NULL,						0,0,	NULL,		0,		2,	0,		12,		UI_TINYFONT,				CT_LTGOLD1,	NULL,	// GOMG_COL5_ROW1
	MG_NUMBER,	0.0,	544,	 80,	16,		10,		NULL,						0,0,	NULL,		0,		2,	0,		12,		UI_TINYFONT,				CT_LTGOLD1,	NULL,	// GOMG_COL5_ROW2
	MG_NUMBER,	0.0,	544,	 92,	16,		10,		NULL,						0,0,	NULL,		0,		2,	0,		12,		UI_TINYFONT,				CT_LTGOLD1,	NULL,	// GOMG_COL5_ROW3
	MG_NUMBER,	0.0,	544,	104,	16,		10,		NULL,						0,0,	NULL,		0,		2,	0,		12,		UI_TINYFONT,				CT_LTGOLD1,	NULL,	// GOMG_COL5_ROW4

	MG_NUMBER,	0.0,	558,	 68,	16,		10,		NULL,						0,0,	NULL,		0,		2,	0,		12,		UI_TINYFONT,				CT_LTGOLD1,	NULL,	// GOMG_COL6_ROW1
	MG_NUMBER,	0.0,	558,	 80,	16,		10,		NULL,						0,0,	NULL,		0,		2,	0,		12,		UI_TINYFONT,				CT_LTGOLD1,	NULL,	// GOMG_COL6_ROW2
	MG_NUMBER,	0.0,	558,	 92,	16,		10,		NULL,						0,0,	NULL,		0,		2,	0,		12,		UI_TINYFONT,				CT_LTGOLD1,	NULL,	// GOMG_COL6_ROW3
	MG_NUMBER,	0.0,	558,	104,	16,		10,		NULL,						0,0,	NULL,		0,		2,	0,		12,		UI_TINYFONT,				CT_LTGOLD1,	NULL,	// GOMG_COL6_ROW4

	MG_NUMBER,	0.0,	572,	 68,	16,		10,		NULL,						0,0,	NULL,		0,		2,	0,		12,		UI_TINYFONT,				CT_LTGOLD1,	NULL,	// GOMG_COL7_ROW1
	MG_NUMBER,	0.0,	572,	 80,	16,		10,		NULL,						0,0,	NULL,		0,		2,	0,		12,		UI_TINYFONT,				CT_LTGOLD1,	NULL,	// GOMG_COL7_ROW2
	MG_NUMBER,	0.0,	572,	 92,	16,		10,		NULL,						0,0,	NULL,		0,		2,	0,		12,		UI_TINYFONT,				CT_LTGOLD1,	NULL,	// GOMG_COL7_ROW3
	MG_NUMBER,	0.0,	572,	104,	16,		10,		NULL,						0,0,	NULL,		0,		2,	0,		12,		UI_TINYFONT,				CT_LTGOLD1,	NULL,	// GOMG_COL7_ROW4

	MG_NUMBER,	0.0,	596,	 68,	16,		10,		NULL,						0,0,	NULL,		0,		2,	0,		12,		UI_TINYFONT,				CT_LTGOLD1,	NULL,	// GOMG_COL8_ROW1
	MG_NUMBER,	0.0,	596,	 80,	16,		10,		NULL,						0,0,	NULL,		0,		2,	0,		12,		UI_TINYFONT,				CT_LTGOLD1,	NULL,	// GOMG_COL8_ROW2
	MG_NUMBER,	0.0,	596,	 92,	16,		10,		NULL,						0,0,	NULL,		0,		2,	0,		12,		UI_TINYFONT,				CT_LTGOLD1,	NULL,	// GOMG_COL8_ROW3
	MG_NUMBER,	0.0,	596,	104,	16,		10,		NULL,						0,0,	NULL,		0,		2,	0,		12,		UI_TINYFONT,				CT_LTGOLD1,	NULL,	// GOMG_COL8_ROW4

	MG_VAR,		0.0,	0,		0,		0,		0,		NULL,						0,0,	NULL,		0,		0,	0,		1,		0,		CT_NONE,	NULL,	// GOMG_NUMBERS_END

};


int s_subtitle_Names[] =
{
	MNT_OFF,
	MNT_ON,
	MNT_CINEMATIC,
	MNT_NONE
};

//static menulist_s			s_preferences_crosshair_box;
//static menuradiobutton_s	s_preferences_brass_box;
//static menuradiobutton_s	s_preferences_highqualitysky_box;
//static menubitmap_s			s_preferences_return;

/*
=================
ClampCvar
=================
*/
static float ClampCvar( float min, float max, float value )
{
	if ( value < min ) 
	{
		return min;
	}

	if ( value > max ) 
	{
		return max;
	}

	return value;
}


/*
=================
Preferences_CvarOnOff
=================
*/
void Preferences_CvarOnOff(char *cvarName,menubitmap_s *bitMap,menubitmap_s *buttonBitMap)
{
	int	curValue;

	curValue = ui.Cvar_VariableValue( cvarName );

	if (curValue)	// It's on - turn if off
	{
		bitMap->textEnum = MBT_OFF;
		bitMap->color =buttonBitMap->color;
		curValue = 0;
	}
	else
	{
		bitMap->textEnum = MBT_ON;
		bitMap->color =buttonBitMap->color2;
		curValue = 1;
	}

	ui.Cvar_SetValue( cvarName, curValue );

}

/*
=================
Preferences_CvarSet
=================
*/
void Preferences_CvarSet(char *cvarName,menubitmap_s *bitMap,int colorOn,int colorOff)
{
	int	curValue;

	curValue = ui.Cvar_VariableValue( cvarName );

	if (curValue)	
	{
		bitMap->textEnum = MBT_ON;
		bitMap->color = colorOn;
		bitMap->color2 = colorOn;
	}
	else
	{
		bitMap->textEnum = MBT_OFF;
		bitMap->color = colorOff;
		bitMap->color2 = colorOff;
	}
}

/*
=================
Preferences_Event
=================
*/
static void Preferences_Event( void* ptr, int notification )
{
	menuframework_s*	m;
	int					curValue;

	if (notification != QM_ACTIVATED)
	{
		return;
	}

	m = ((menucommon_s*)ptr)->parent;

	switch (((menucommon_s*)ptr)->id)
	{
		case ID_SUBTITLE:
			ui.Cvar_SetValue( "g_subtitles", s_subtitle.curvalue );
			break;

		case ID_TEXTLANGUAGE:
			ui.Cvar_Set( "g_language", menu_normal_text[s_textlanguage_Names[s_textlanguage.curvalue]] );
			UI_LoadButtonText();
			UI_LoadMenuText();
			break;

		case ID_VOICELANGUAGE:
			ui.Cvar_Set( "s_language", menu_normal_text[s_voicelanguage_Names[s_voicelanguage.curvalue]] );
			ui.Cmd_ExecuteText( EXEC_APPEND, "snd_restart\n" );
			break;

		case ID_CROSSHAIR:
			curCrosshair++;
			if (curCrosshair == NUM_CROSSHAIRS)
			{
				curCrosshair=0;
			}
			ui.Cvar_SetValue( "cg_drawCrosshair", curCrosshair );
			break;

		case ID_LIGHTFLARES: 

			curValue = !ui.Cvar_VariableValue( "r_flares" );

			if (curValue)
			{
				s_preferences_lightflares_value.color =s_preferences_lightflares_box.color2;
				s_preferences_lightflares_value.textEnum = MBT_ON;
				gameoptionsmenu_graphics[GOMG_BLEFTSIDE].target = 1;
			}
			else
			{
				s_preferences_lightflares_value.color =s_preferences_lightflares_box.color;
				s_preferences_lightflares_value.textEnum = MBT_OFF;
				gameoptionsmenu_graphics[GOMG_BLEFTSIDE].target = 0;
				gameoptionsmenu_graphics[GOMG_CENTERDOT1].color = CT_DKORANGE;
				gameoptionsmenu_graphics[GOMG_CENTERDOT4].color = CT_DKORANGE;
			}

			ui.Cvar_SetValue( "r_flares", curValue );

			break;			

		case ID_WALLMARKS:
			Preferences_CvarOnOff("cg_marks",&s_preferences_wallmarks_value,&s_preferences_wallmarks_box);  

			if (s_preferences_wallmarks_value.textEnum == MBT_ON)
			{
				gameoptionsmenu_graphics[GOMG_SINE1].type = MG_GRAPHIC;
			}
			else
			{
				gameoptionsmenu_graphics[GOMG_SINE1].type = MG_OFF;
			}
			break;

		case ID_DYNAMICLIGHTS:
			Preferences_CvarOnOff("r_dynamiclight",&s_preferences_dynamiclights_value,&s_preferences_dynamiclights_box);  
			if (s_preferences_dynamiclights_value.textEnum == MBT_ON)
			{
				gameoptionsmenu_graphics[GOMG_SINE2].type = MG_GRAPHIC;
			}
			else
			{
				gameoptionsmenu_graphics[GOMG_SINE2].type = MG_OFF;
			}
			break;		

		case ID_IDENTIFYTARGET:
			Preferences_CvarOnOff("cg_drawCrosshairNames",&s_preferences_identifytarget_value,&s_preferences_identifytarget_box);  
			if (s_preferences_identifytarget_value.textEnum == MBT_ON)
			{
				gameoptionsmenu_graphics[GOMG_SINE3].type = MG_GRAPHIC;
			}
			else
			{
				gameoptionsmenu_graphics[GOMG_SINE3].type = MG_OFF;
			}
			break;

		case ID_SYNCEVERYFRAME:
			Preferences_CvarOnOff("r_finish", &s_preferences_synceveryframe_value,&s_preferences_synceveryframe_box);  
			if (s_preferences_synceveryframe_value.textEnum == MBT_ON)
			{
				gameoptionsmenu_graphics[GOMG_BRIGHTSIDE].target = 1;
			}
			else
			{
				gameoptionsmenu_graphics[GOMG_BRIGHTSIDE].target = 0;
				gameoptionsmenu_graphics[GOMG_CENTERDOT2].color = CT_DKBLUE1;
				gameoptionsmenu_graphics[GOMG_CENTERDOT3].color = CT_DKBLUE1;
			}
			break;
		
		case ID_RETURN:
			UI_PopMenu();
			break;
	}
}

/*
=================
GameOptions_MenuEvent
=================
*/
static void GameOptions_MenuEvent( void* ptr, int notification )
{
	menuframework_s*	m;

	if (notification != QM_ACTIVATED)
		return;

	m = ((menucommon_s*)ptr)->parent;

	switch (((menucommon_s*)ptr)->id)
	{
		case ID_SOUND:
			UI_PopMenu();			// Get rid of whatever is ontop
			UI_SoundMenu();			// Move to the Sound Menu
			break;

		case ID_CONTROLS:
			UI_PopMenu();			// Get rid of whatever is ontop
			UI_SetupWeaponsMenu();	// Move to the Controls Menu
			break;

		case ID_VIDEO:
			UI_PopMenu();			// Get rid of whatever is ontop
			UI_VideoDataMenu();		// Move to the Video Menu
			break;

		case ID_GAMEOPTIONS:
			break;

		case ID_CDKEY:
			UI_PopMenu();			// Get rid of whatever is ontop
			UI_CDKeyMenu();			// Move to the CD Key Menu
			break;

		case ID_MAINMENU:
			UI_PopMenu();


	}
}

/*
=================
Preferences_MenuBlinkies
=================
*/
static void Preferences_MenuBlinkies( void )
{

	int i;

	if (gameoptionsmenu_graphics[GOMG_BLEFTSIDE].target)
	{
		// Change color every 2 to 6 seconds
		if (gameoptionsmenu_graphics[GOMG_CENTERDOT1].timer <  uis.realtime)
		{
			gameoptionsmenu_graphics[GOMG_CENTERDOT1].timer = uis.realtime + (random() * 4000) + 2000;
			if (gameoptionsmenu_graphics[GOMG_CENTERDOT1].color != CT_DKORANGE)
			{
				gameoptionsmenu_graphics[GOMG_CENTERDOT1].color = CT_DKORANGE;
			}
			else
			{
				gameoptionsmenu_graphics[GOMG_CENTERDOT1].color = CT_LTORANGE;
			}
		}

		// Change color every 1 seconds
		if (gameoptionsmenu_graphics[GOMG_CENTERDOT4].timer <  uis.realtime)
		{
			gameoptionsmenu_graphics[GOMG_CENTERDOT4].timer = uis.realtime +  1000;

			if (gameoptionsmenu_graphics[GOMG_CENTERDOT4].color != CT_DKORANGE)
			{
				gameoptionsmenu_graphics[GOMG_CENTERDOT4].color = CT_DKORANGE;
			}
			else
			{
				gameoptionsmenu_graphics[GOMG_CENTERDOT4].color = CT_LTORANGE;
			}
		}
	}


	if (gameoptionsmenu_graphics[GOMG_BRIGHTSIDE].target)
	{

		// Change color every 3 to 6 seconds
		if (gameoptionsmenu_graphics[GOMG_CENTERDOT2].timer <  uis.realtime)
		{
			gameoptionsmenu_graphics[GOMG_CENTERDOT2].timer = uis.realtime + (random() * 3000) + 3000;
			if (gameoptionsmenu_graphics[GOMG_CENTERDOT2].color != CT_DKBLUE1)
			{
				gameoptionsmenu_graphics[GOMG_CENTERDOT2].color = CT_DKBLUE1;
			}
			else
			{
				gameoptionsmenu_graphics[GOMG_CENTERDOT2].color = CT_LTBLUE1;
			}
		}

		// Change color every 2 to 4 seconds
		if (gameoptionsmenu_graphics[GOMG_CENTERDOT3].timer <  uis.realtime)
		{
			gameoptionsmenu_graphics[GOMG_CENTERDOT3].timer = uis.realtime + (random() * 2000) + 2000;
			if (gameoptionsmenu_graphics[GOMG_CENTERDOT3].color != CT_DKBLUE1)
			{
				gameoptionsmenu_graphics[GOMG_CENTERDOT3].color = CT_DKBLUE1;
			}
			else
			{
				gameoptionsmenu_graphics[GOMG_CENTERDOT3].color = CT_LTBLUE1;
			}
		}
	}


	if (gameoptionsmenu_graphics[GOMG_COL1_ROW1].timer < uis.realtime)
	{
		// Make numbers change
		gameoptionsmenu_graphics[GOMG_COL1_ROW1].timer = uis.realtime + 5000;

		for (i=GOMG_NUMBERS_START + 1;i<GOMG_NUMBERS_END;++i)
		{
			gameoptionsmenu_graphics[i].target = (random() * (90)) + 9;
			gameoptionsmenu_graphics[i].color = CT_DKGOLD1;
		}
	}
}

/*
=================
Preferences_MenuDraw
=================
*/
static void Preferences_MenuDraw( void )
{
	Preferences_MenuBlinkies();

	UI_MenuFrame(&s_gameoptions_menu);

	UI_Setup_MenuButtons();

	ui.R_SetColor( colorTable[CT_DKPURPLE2]);
	UI_DrawHandlePic(30,203,  47, 186, uis.whiteShader);	// Long left hand column square

//	UI_DrawHandlePic(304,171,  306, 12, uis.whiteShader);	// Top bar off round buttons
//	UI_DrawHandlePic(315,408,  295, 12, uis.whiteShader);	// Bottom bar off round buttons

	UI_DrawHandlePic(304,171,  299, 12, uis.whiteShader);	// Top bar off round buttons
	UI_DrawHandlePic(315,408,  288, 12, uis.whiteShader);	// Bottom bar off round buttons

	UI_DrawHandlePic( 293, 171, 16,   16,  gameoptionsmenu_cache.slant1);	// Slant at top
	UI_DrawHandlePic( 303, 408, 16,   16,  gameoptionsmenu_cache.slant2);	// Slant at bottom

	UI_DrawHandlePic( 543, 171, 128,	32,	gameoptionsmenu_cache.swooptop);
	UI_DrawHandlePic( 543, 396, 128,	32,	gameoptionsmenu_cache.swoopbottom);

	UI_DrawHandlePic(548, 193,  64,10,		uis.whiteShader);	// Top of right hand column
	UI_DrawHandlePic(548, 389,  64,	7,		uis.whiteShader);	// Bottom of right hand column

	UI_DrawHandlePic( 548, 206, 64,   256,  gameoptionsmenu_cache.singraph);	// The graph
	ui.R_SetColor( colorTable[CT_DKBLUE1]);
	UI_DrawHandlePic( 548, 206, 64,   180,  gameoptionsmenu_cache.graphbox);	// The frame around the graph

	ui.R_SetColor( colorTable[CT_DKBLUE1]);
	UI_DrawHandlePic(377, 265, 8,   87,	uis.whiteShader);	// Lefthand side of CROSSHAIR box
	UI_DrawHandlePic(503, 265, 8,   87,	uis.whiteShader);	// Righthand side of CROSSHAIR box
	UI_DrawHandlePic(385, 355, 116, 3,	uis.whiteShader);	// Bottom of CROSSHAIR box
	UI_DrawHandlePic( 377, 352, 16,  16,	gameoptionsmenu_cache.lswoop);	// lower left hand swoop
	UI_DrawHandlePic( 500, 352, 16,  16,	gameoptionsmenu_cache.lswoop2);	// lower right hand swoop

	UI_DrawHandlePic( 377, 244, 16,  32,  gameoptionsmenu_cache.tallswoop);	// upper left hand swoop
	UI_DrawHandlePic( 497, 244, 16,  32,  gameoptionsmenu_cache.tallswoop2);	// upper right hand swoop

	ui.R_SetColor( colorTable[CT_YELLOW]);
	if (curCrosshair)
	{
		UI_DrawHandlePic(427,293,  32, 32, crosshairShader[curCrosshair]);	// Draw crosshair
	}
	else
	{
		UI_DrawProportionalString( 427, 293, menu_normal_text[MNT_CROSSHAIR_NONE],UI_CENTER|UI_SMALLFONT, colorTable[CT_LTGOLD1]);	// No crosshair
	}

	ui.R_SetColor( colorTable[CT_DKPURPLE2]);
	UI_DrawHandlePic( 97, 298, 128,   64,  gameoptionsmenu_cache.round1);
	UI_DrawHandlePic( 97, 339, 128,   128, gameoptionsmenu_cache.round2);

	ui.R_SetColor( colorTable[CT_LTBROWN1]);
	UI_DrawHandlePic(111, 234, 64,    64,  gameoptionsmenu_cache.round10);
	ui.R_SetColor( colorTable[CT_DKBROWN1]);
	UI_DrawHandlePic(138, 254, 32,    64,  gameoptionsmenu_cache.round11);


	// Numbers along right hand column
	UI_DrawProportionalString( 606, 191, "11663",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);	// Top number
	UI_DrawProportionalString( 606, 393, "71961",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);	// Bottom number

	UI_PrintMenuGraphics(gameoptionsmenu_graphics,GOMG_MAX);

	// Numbers in center of round button
	UI_DrawProportionalString( 163, 283, "551",UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString( 163, 300, "29",UI_TINYFONT, colorTable[CT_BLACK]);

	UI_DrawProportionalString( 224, 283, "336",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);	
	UI_DrawProportionalString( 224, 300, "1040",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);	

	// Lower left hand squares
	UI_DrawProportionalString( 100, 326, "761-0903",UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString( 100, 408, "8236",UI_TINYFONT, colorTable[CT_BLACK]);


	// Menu frame numbers
	UI_DrawProportionalString(  74,  66, "1776",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  84, "9214",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  188, "2510-81",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  206, "644",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  395, "1001001",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);

	Menu_Draw( &s_gameoptions_menu );
}

/*
=================
Preferences_MenuKey
=================
*/
static sfxHandle_t Preferences_MenuKey( int key )
{
	return Menu_DefaultKey( &s_gameoptions_menu, key );
}

/*
===============
Preferences_SetValues
===============
*/
void Preferences_SetValues(void)
{
	char buffer[32];
	int *language;
	int curValue;

	ui.Cvar_VariableStringBuffer( "g_language", buffer, 32 );
	language = s_textlanguage_Names;
	
	s_textlanguage.curvalue=0;
	if (buffer[0]) {
		while (*language)
		{
			if (Q_stricmp(menu_normal_text[*language],buffer)==0)
			{
				break;
			}
			language++;
			s_textlanguage.curvalue++;
		}

		if (!*language)
		{
			s_textlanguage.curvalue = 0;
		}
	}

	ui.Cvar_VariableStringBuffer( "s_language", buffer, 32 );
	language = s_voicelanguage_Names;
	
	s_voicelanguage.curvalue=0;
	if (buffer[0]) {
		while (*language)
		{
			if (Q_stricmp(menu_normal_text[*language],buffer)==0)
			{
				break;
			}
			language++;
			s_voicelanguage.curvalue++;
		}

		if (!*language)
		{
			s_voicelanguage.curvalue = 0;
		}
	}

	s_subtitle.curvalue = (int)ui.Cvar_VariableValue( "g_subtitles" );

	curCrosshair = (int)ui.Cvar_VariableValue( "cg_drawCrosshair" ) % NUM_CROSSHAIRS;

	curValue = ui.Cvar_VariableValue( "cg_marks" );

	if (curValue)	
	{
		gameoptionsmenu_graphics[GOMG_SINE1].type = MG_GRAPHIC;
	}
	else
	{
		gameoptionsmenu_graphics[GOMG_SINE1].type = MG_OFF;
	}


	curValue = ui.Cvar_VariableValue("r_dynamiclight");

	if (curValue)	
	{
		gameoptionsmenu_graphics[GOMG_SINE2].type = MG_GRAPHIC;
	}
	else
	{
		gameoptionsmenu_graphics[GOMG_SINE2].type = MG_OFF;
	}


	curValue = ui.Cvar_VariableValue("cg_drawCrosshairNames");

	if (curValue)	
	{
		gameoptionsmenu_graphics[GOMG_SINE3].type = MG_GRAPHIC;
	}
	else
	{
		gameoptionsmenu_graphics[GOMG_SINE3].type = MG_OFF;
	}

	curValue = ui.Cvar_VariableValue("r_flares");

	if (curValue)
	{
		gameoptionsmenu_graphics[GOMG_BLEFTSIDE].target = 1;
	}
	else
	{
		gameoptionsmenu_graphics[GOMG_BLEFTSIDE].target = 0;
		gameoptionsmenu_graphics[GOMG_CENTERDOT1].color = CT_DKORANGE;
		gameoptionsmenu_graphics[GOMG_CENTERDOT4].color = CT_DKORANGE;
	}

	curValue = ui.Cvar_VariableValue("r_finish");

	if (curValue)
	{
		gameoptionsmenu_graphics[GOMG_BRIGHTSIDE].target = 1;
	}
	else
	{
		gameoptionsmenu_graphics[GOMG_BRIGHTSIDE].target = 0;
		gameoptionsmenu_graphics[GOMG_CENTERDOT2].color = CT_DKBLUE1;
		gameoptionsmenu_graphics[GOMG_CENTERDOT3].color = CT_DKBLUE1;
	}


}

/*
===============
UI_PreferencesMenu_Cache
===============
*/
void UI_PreferencesMenu_Cache( void ) 
{
	int i;

	gameoptionsmenu_cache.slant1 = ui.R_RegisterShaderNoMip("menu/lcarscontrols/slant1.tga");
	gameoptionsmenu_cache.slant2 = ui.R_RegisterShaderNoMip("menu/lcarscontrols/slant2.tga");

	gameoptionsmenu_cache.swooptop = ui.R_RegisterShaderNoMip("menu/lcarscontrols/bigswooptop.tga");
	gameoptionsmenu_cache.swoopbottom = ui.R_RegisterShaderNoMip("menu/lcarscontrols/bigswoopbottom.tga");

	gameoptionsmenu_cache.singraph = ui.R_RegisterShaderNoMip("menu/lcarscontrols/singraph.tga");
	gameoptionsmenu_cache.graphbox = ui.R_RegisterShaderNoMip("menu/lcarscontrols/graphbox.tga");

	gameoptionsmenu_cache.lswoop = ui.R_RegisterShaderNoMip("menu/lcarscontrols/lswoop.tga");
	gameoptionsmenu_cache.lswoop2 = ui.R_RegisterShaderNoMip("menu/lcarscontrols/lswoop2.tga");
	gameoptionsmenu_cache.tallswoop = ui.R_RegisterShaderNoMip("menu/lcarscontrols/tallswoop.tga");
	gameoptionsmenu_cache.tallswoop2 = ui.R_RegisterShaderNoMip("menu/lcarscontrols/tallswoop2.tga");

	gameoptionsmenu_cache.round1 = ui.R_RegisterShaderNoMip("menu/lcarscontrols/round1.tga");
	gameoptionsmenu_cache.round2 = ui.R_RegisterShaderNoMip("menu/lcarscontrols/round2.tga");
	gameoptionsmenu_cache.round10 = ui.R_RegisterShaderNoMip("menu/lcarscontrols/round10.tga");
	gameoptionsmenu_cache.round11 = ui.R_RegisterShaderNoMip("menu/lcarscontrols/round11.tga");

	// Precache crosshairs
	for( i = 0; i < NUM_CROSSHAIRS; i++ ) 
	{
		crosshairShader[i] = ui.R_RegisterShaderNoMip( va("gfx/2d/crosshair%c", 'a' + i ) );
	}

	// Precache all menu graphics in array
	UI_PrecacheMenuGraphics(gameoptionsmenu_graphics,GOMG_MAX);

	ui.R_RegisterShaderNoMip("menu/lcarscontrols/round3.tga");
	ui.R_RegisterShaderNoMip("menu/lcarscontrols/round4.tga");
	ui.R_RegisterShaderNoMip("menu/lcarscontrols/round5.tga");
	ui.R_RegisterShaderNoMip("menu/lcarscontrols/round18.tga");
	ui.R_RegisterShaderNoMip("menu/lcarscontrols/round12.tga");
	ui.R_RegisterShaderNoMip("menu/lcarscontrols/round9.tga");
	ui.R_RegisterShaderNoMip("menu/lcarscontrols/round6.tga");
	ui.R_RegisterShaderNoMip("menu/lcarscontrols/round7.tga");
	ui.R_RegisterShaderNoMip("menu/lcarscontrols/round13.tga");
	ui.R_RegisterShaderNoMip("menu/lcarscontrols/round8.tga");
	ui.R_RegisterShaderNoMip("menu/common/full_button2.tga");

}

/*
=================
Preferences_MenuInit
=================
*/
static void Preferences_MenuInit( void )
{
	UI_PreferencesMenu_Cache();

	s_gameoptions_menu.nitems				= 0;
	s_gameoptions_menu.wrapAround			= qtrue;
//	s_gameoptions_menu.opening				= M_Main_Opening;
//	s_gameoptions_menu.closing				= M_Main_Closing;
	s_gameoptions_menu.draw					= Preferences_MenuDraw;
	s_gameoptions_menu.key					= Preferences_MenuKey;
	s_gameoptions_menu.fullscreen			= qtrue;
	s_gameoptions_menu.descX				= MENU_DESC_X;
	s_gameoptions_menu.descY				= MENU_DESC_Y;
	s_gameoptions_menu.titleX				= MENU_TITLE_X;
	s_gameoptions_menu.titleY				= MENU_TITLE_Y;
	s_gameoptions_menu.titleI				= MNT_CONTROLSMENU_TITLE;
	s_gameoptions_menu.footNoteEnum			= MNT_GAMEOPTION_LABEL;

	SetupMenu_TopButtons(&s_gameoptions_menu,MENU_GAME,NULL);

	s_preferences_wallmarks_box.generic.type			= MTYPE_BITMAP;      
	s_preferences_wallmarks_box.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_preferences_wallmarks_box.generic.x				= 196;
	s_preferences_wallmarks_box.generic.y				= 358;
	s_preferences_wallmarks_box.generic.name			= "menu/lcarscontrols/round4.tga";
	s_preferences_wallmarks_box.generic.id				= ID_WALLMARKS;
	s_preferences_wallmarks_box.generic.callback		= Preferences_Event;
	s_preferences_wallmarks_box.width					= 128;
	s_preferences_wallmarks_box.height					= 64;
	s_preferences_wallmarks_box.color					= CT_DKBLUE1;
	s_preferences_wallmarks_box.color2					= CT_LTBLUE1;
	s_preferences_wallmarks_box.textX					= 1;
	s_preferences_wallmarks_box.textY					= 25;
	s_preferences_wallmarks_box.textEnum				= MBT_WALLMARKS1;
	s_preferences_wallmarks_box.textEnum2				= MBT_WALLMARKS2;
	s_preferences_wallmarks_box.textcolor				= CT_BLACK;
	s_preferences_wallmarks_box.textcolor2				= CT_WHITE;
	s_preferences_wallmarks_box.focusX					= 196;
	s_preferences_wallmarks_box.focusY					= 384;
	s_preferences_wallmarks_box.focusHeight				= 37;
	s_preferences_wallmarks_box.focusWidth				= 95;

	s_preferences_wallmarks_value.generic.type			= MTYPE_BITMAP;      
	s_preferences_wallmarks_value.generic.flags			= QMF_INACTIVE;
	s_preferences_wallmarks_value.generic.x				= 196;
	s_preferences_wallmarks_value.generic.y				= 328;
	s_preferences_wallmarks_value.generic.name			= "menu/lcarscontrols/round3.tga";
	s_preferences_wallmarks_value.generic.id			= ID_WALLMARKS_VALUE;
	s_preferences_wallmarks_value.generic.callback		= Preferences_Event;
	s_preferences_wallmarks_value.width					= 64;
	s_preferences_wallmarks_value.height				= 64;
	s_preferences_wallmarks_value.textX					= 12;
	s_preferences_wallmarks_value.textY					= 18;
	Preferences_CvarSet("cg_marks",&s_preferences_wallmarks_value,
		s_preferences_wallmarks_box.color2,s_preferences_wallmarks_box.color);  
	s_preferences_wallmarks_value.textcolor				= CT_WHITE;
	s_preferences_wallmarks_value.textcolor2			= CT_WHITE;


	s_preferences_dynamiclights_box.generic.type			= MTYPE_BITMAP;      
	s_preferences_dynamiclights_box.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_preferences_dynamiclights_box.generic.x				= 269;
	s_preferences_dynamiclights_box.generic.y				= 298;
	s_preferences_dynamiclights_box.generic.name			= "menu/lcarscontrols/round5.tga";
	s_preferences_dynamiclights_box.generic.id				= ID_DYNAMICLIGHTS;
	s_preferences_dynamiclights_box.generic.callback		= Preferences_Event;
	s_preferences_dynamiclights_box.width					= 128;
	s_preferences_dynamiclights_box.height					= 128;
	s_preferences_dynamiclights_box.color					= CT_DKBLUE1;
	s_preferences_dynamiclights_box.color2					= CT_LTBLUE1;
	s_preferences_dynamiclights_box.textX					= 14;
	s_preferences_dynamiclights_box.textY					= 2;
	s_preferences_dynamiclights_box.textEnum				= MBT_DYNAMICLIGHTS1;
	s_preferences_dynamiclights_box.textEnum2				= MBT_DYNAMICLIGHTS2;
	s_preferences_dynamiclights_box.textcolor				= CT_BLACK;
	s_preferences_dynamiclights_box.textcolor2				= CT_WHITE;
	s_preferences_dynamiclights_box.focusX					= 279;
	s_preferences_dynamiclights_box.focusY					= 298;
	s_preferences_dynamiclights_box.focusHeight				= 54;
	s_preferences_dynamiclights_box.focusWidth				= 61;

	s_preferences_dynamiclights_value.generic.type			= MTYPE_BITMAP;      
	s_preferences_dynamiclights_value.generic.flags			= QMF_INACTIVE;
	s_preferences_dynamiclights_value.generic.x				= 226;
	s_preferences_dynamiclights_value.generic.y				= 298;
	s_preferences_dynamiclights_value.generic.name			= "menu/lcarscontrols/round18.tga";
	s_preferences_dynamiclights_value.generic.id			= ID_DYNAMICLIGHTS_VALUE;
	s_preferences_dynamiclights_value.generic.callback		= Preferences_Event;
	s_preferences_dynamiclights_value.width					= 64;
	s_preferences_dynamiclights_value.height				= 64;
	s_preferences_dynamiclights_value.textX					= 19;
	s_preferences_dynamiclights_value.textY					= 10;
	s_preferences_dynamiclights_value.textEnum				= MBT_ON;
	Preferences_CvarSet("r_dynamiclight",&s_preferences_dynamiclights_value,
		s_preferences_dynamiclights_box.color2,s_preferences_dynamiclights_box.color);  
	s_preferences_dynamiclights_value.textcolor				= CT_WHITE;
	s_preferences_dynamiclights_value.textcolor2			= CT_WHITE;


	s_preferences_lightflares_box.generic.type				= MTYPE_BITMAP;      
	s_preferences_lightflares_box.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS;
	s_preferences_lightflares_box.generic.x					= 95;
	s_preferences_lightflares_box.generic.y					= 171;
	s_preferences_lightflares_box.generic.name				= "menu/lcarscontrols/round12.tga";
	s_preferences_lightflares_box.generic.id				= ID_LIGHTFLARES;
	s_preferences_lightflares_box.generic.callback			= Preferences_Event;
	s_preferences_lightflares_box.width						= 128;
	s_preferences_lightflares_box.height					= 64;
	s_preferences_lightflares_box.color						= CT_DKORANGE;
	s_preferences_lightflares_box.color2					= CT_LTORANGE;
	s_preferences_lightflares_box.textX						= 36;
	s_preferences_lightflares_box.textY						= 2;
	s_preferences_lightflares_box.textEnum					= MBT_LIGHTFLARES1;
	s_preferences_lightflares_box.textEnum2					= MBT_LIGHTFLARES2;
	s_preferences_lightflares_box.textcolor					= CT_BLACK;
	s_preferences_lightflares_box.textcolor2				= CT_WHITE;
	s_preferences_lightflares_box.focusX					= 119;
	s_preferences_lightflares_box.focusY					= 171;
	s_preferences_lightflares_box.focusHeight				= 42;
	s_preferences_lightflares_box.focusWidth				= 76;

	s_preferences_lightflares_value.generic.type			= MTYPE_BITMAP;      
	s_preferences_lightflares_value.generic.flags			= QMF_INACTIVE;
	s_preferences_lightflares_value.generic.x				= 145;
	s_preferences_lightflares_value.generic.y				= 217;
	s_preferences_lightflares_value.generic.name			= "menu/lcarscontrols/round9.tga";
	s_preferences_lightflares_value.generic.id				= ID_LIGHTFLARES_VALUE;
	s_preferences_lightflares_value.generic.callback		= Preferences_Event;
	s_preferences_lightflares_value.width					= 64;
	s_preferences_lightflares_value.height					= 64;
	s_preferences_lightflares_value.textX					= 19;
	s_preferences_lightflares_value.textY					= 15;
	Preferences_CvarSet("r_flares",&s_preferences_lightflares_value,
		s_preferences_lightflares_box.color2,s_preferences_lightflares_box.color);  
	s_preferences_lightflares_value.textcolor				= CT_WHITE;
	s_preferences_lightflares_value.textcolor2				= CT_WHITE;

	s_preferences_identifytarget_box.generic.type			= MTYPE_BITMAP;      
	s_preferences_identifytarget_box.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_preferences_identifytarget_box.generic.x				= 268;
	s_preferences_identifytarget_box.generic.y				= 222;
	s_preferences_identifytarget_box.generic.name			= "menu/lcarscontrols/round6.tga";
	s_preferences_identifytarget_box.generic.id				= ID_IDENTIFYTARGET;
	s_preferences_identifytarget_box.generic.callback		= Preferences_Event;
	s_preferences_identifytarget_box.width					= 128;
	s_preferences_identifytarget_box.height					= 128;
	s_preferences_identifytarget_box.color					= CT_DKPURPLE3;
	s_preferences_identifytarget_box.color2					= CT_LTPURPLE3;
	s_preferences_identifytarget_box.textX					= 14;
	s_preferences_identifytarget_box.textY					= 37;
	s_preferences_identifytarget_box.textEnum				= MBT_IDENTIFYTARGET1;
	s_preferences_identifytarget_box.textEnum2				= MBT_IDENTIFYTARGET2;
	s_preferences_identifytarget_box.textcolor				= CT_BLACK;
	s_preferences_identifytarget_box.textcolor2				= CT_WHITE;
	s_preferences_identifytarget_box.focusX					= 278;
	s_preferences_identifytarget_box.focusY					= 255;
	s_preferences_identifytarget_box.focusHeight			= 41;
	s_preferences_identifytarget_box.focusWidth				= 62;

	s_preferences_identifytarget_value.generic.type			= MTYPE_BITMAP;      
	s_preferences_identifytarget_value.generic.flags		= QMF_INACTIVE;
	s_preferences_identifytarget_value.generic.x			= 226;
	s_preferences_identifytarget_value.generic.y			= 242;
	s_preferences_identifytarget_value.generic.name			= "menu/lcarscontrols/round7.tga";
	s_preferences_identifytarget_value.generic.id			= ID_IDENTIFYTARGET_VALUE;
	s_preferences_identifytarget_value.generic.callback		= Preferences_Event;
	s_preferences_identifytarget_value.width				= 64;
	s_preferences_identifytarget_value.height				= 64;
	s_preferences_identifytarget_value.textX				= 19;
	s_preferences_identifytarget_value.textY				= 29;
	Preferences_CvarSet("cg_drawCrosshairNames",&s_preferences_identifytarget_value,
		s_preferences_identifytarget_box.color2,s_preferences_identifytarget_box.color);  
	s_preferences_identifytarget_value.textcolor			= CT_WHITE;
	s_preferences_identifytarget_value.textcolor2			= CT_WHITE;


	s_preferences_synceveryframe_box.generic.type			= MTYPE_BITMAP;      
	s_preferences_synceveryframe_box.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_preferences_synceveryframe_box.generic.x				= 197;
	s_preferences_synceveryframe_box.generic.y				= 171;
	s_preferences_synceveryframe_box.generic.name			= "menu/lcarscontrols/round13.tga";
	s_preferences_synceveryframe_box.generic.id				= ID_SYNCEVERYFRAME;
	s_preferences_synceveryframe_box.generic.callback		= Preferences_Event;
	s_preferences_synceveryframe_box.width					= 128;
	s_preferences_synceveryframe_box.height					= 64;
	s_preferences_synceveryframe_box.color					= CT_DKBLUE1;
	s_preferences_synceveryframe_box.color2					= CT_LTBLUE1;
	s_preferences_synceveryframe_box.textX					= 4;
	s_preferences_synceveryframe_box.textY					= 2;
	s_preferences_synceveryframe_box.textEnum				= MBT_SYNCEVERYFRAME1;
	s_preferences_synceveryframe_box.textEnum2				= MBT_SYNCEVERYFRAME2;
	s_preferences_synceveryframe_box.textcolor				= CT_BLACK;
	s_preferences_synceveryframe_box.textcolor2				= CT_WHITE;
	s_preferences_synceveryframe_box.focusX					= 197;
	s_preferences_synceveryframe_box.focusY					= 171;
	s_preferences_synceveryframe_box.focusHeight			= 38;
	s_preferences_synceveryframe_box.focusWidth				= 79;

	s_preferences_synceveryframe_value.generic.type			= MTYPE_BITMAP;      
	s_preferences_synceveryframe_value.generic.flags		= QMF_INACTIVE;
	s_preferences_synceveryframe_value.generic.x			= 196;
	s_preferences_synceveryframe_value.generic.y			= 216;
	s_preferences_synceveryframe_value.generic.name			= "menu/lcarscontrols/round8.tga";
	s_preferences_synceveryframe_value.generic.id			= ID_SYNCEVERYFRAME_VALUE;
	s_preferences_synceveryframe_value.generic.callback		= Preferences_Event;
	s_preferences_synceveryframe_value.width				= 64;
	s_preferences_synceveryframe_value.height				= 64;
	s_preferences_synceveryframe_value.textX				= 12;
	s_preferences_synceveryframe_value.textY				= 15;
	Preferences_CvarSet("r_finish",&s_preferences_synceveryframe_value,
		s_preferences_synceveryframe_box.color2,s_preferences_synceveryframe_box.color);  
	s_preferences_synceveryframe_value.textcolor			= CT_WHITE;
	s_preferences_synceveryframe_value.textcolor2			= CT_WHITE;


	s_preferences_crosshair_box.generic.type				= MTYPE_BITMAP;      
	s_preferences_crosshair_box.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS;
	s_preferences_crosshair_box.generic.x					= 394;
	s_preferences_crosshair_box.generic.y					= 244;
	s_preferences_crosshair_box.generic.name				= "menu/common/full_button2.tga";
	s_preferences_crosshair_box.generic.id					= ID_CROSSHAIR;
	s_preferences_crosshair_box.generic.callback			= Preferences_Event;
	s_preferences_crosshair_box.width						= 100;
	s_preferences_crosshair_box.height						= 32;
	s_preferences_crosshair_box.color						= CT_DKBLUE1;
	s_preferences_crosshair_box.color2						= CT_LTBLUE1;
	s_preferences_crosshair_box.textX						= 20;
	s_preferences_crosshair_box.textY						= 1;
	s_preferences_crosshair_box.textEnum					= MBT_CROSSHAIR;
	s_preferences_crosshair_box.textcolor					= CT_BLACK;
	s_preferences_crosshair_box.textcolor2					= CT_WHITE;
	s_preferences_crosshair_box.focusX						= 380;
	s_preferences_crosshair_box.focusY						= 244;
	s_preferences_crosshair_box.focusHeight					= 18;
	s_preferences_crosshair_box.focusWidth					= 128;

	s_textlanguage.generic.type									= MTYPE_SPINCONTROL;      
	s_textlanguage.generic.flags								= QMF_HIGHLIGHT_IF_FOCUS;
	s_textlanguage.generic.x									= 320;
	s_textlanguage.generic.y									= 184;
	s_textlanguage.generic.name									= GRAPHIC_BUTTONRIGHT;
	s_textlanguage.generic.callback								= Preferences_Event; 
	s_textlanguage.generic.id									= ID_TEXTLANGUAGE; 
	s_textlanguage.color										= CT_DKPURPLE1;
	s_textlanguage.color2										= CT_LTPURPLE1;
	s_textlanguage.textX										= MENU_BUTTON_TEXT_X;
	s_textlanguage.textY										= MENU_BUTTON_TEXT_Y;
	s_textlanguage.width										= 80;
	s_textlanguage.textEnum										= MBT_TEXTLANGUAGE;
	s_textlanguage.textcolor									= CT_BLACK;
	s_textlanguage.textcolor2									= CT_WHITE;	
	s_textlanguage.listnames									= s_textlanguage_Names;

	s_voicelanguage.generic.type									= MTYPE_SPINCONTROL;      
	s_voicelanguage.generic.flags								= QMF_HIGHLIGHT_IF_FOCUS;
	s_voicelanguage.generic.x									= 320;
	s_voicelanguage.generic.y									= 204;
	s_voicelanguage.generic.name									= GRAPHIC_BUTTONRIGHT;
	s_voicelanguage.generic.callback								= Preferences_Event; 
	s_voicelanguage.generic.id									= ID_VOICELANGUAGE; 
	s_voicelanguage.color										= CT_DKPURPLE1;
	s_voicelanguage.color2										= CT_LTPURPLE1;
	s_voicelanguage.textX										= MENU_BUTTON_TEXT_X;
	s_voicelanguage.textY										= MENU_BUTTON_TEXT_Y;
	s_voicelanguage.width										= 80;
	s_voicelanguage.textEnum									= MBT_VOICELANGUAGE;
	s_voicelanguage.textcolor									= CT_BLACK;
	s_voicelanguage.textcolor2									= CT_WHITE;	
	s_voicelanguage.listnames									= s_voicelanguage_Names;

	s_subtitle.generic.type									= MTYPE_SPINCONTROL;      
	s_subtitle.generic.flags								= QMF_HIGHLIGHT_IF_FOCUS;
	s_subtitle.generic.x									= 320;
	s_subtitle.generic.y									= 384;
	s_subtitle.generic.name									= GRAPHIC_BUTTONRIGHT;
	s_subtitle.generic.callback								= Preferences_Event; 
	s_subtitle.generic.id									= ID_SUBTITLE; 
	s_subtitle.color										= CT_DKPURPLE1;
	s_subtitle.color2										= CT_LTPURPLE1;
	s_subtitle.textX										= MENU_BUTTON_TEXT_X;
	s_subtitle.textY										= MENU_BUTTON_TEXT_Y;
	s_subtitle.width										= 80;
	s_subtitle.textEnum										= MBT_SUBTITLES;
	s_subtitle.textcolor									= CT_BLACK;
	s_subtitle.textcolor2									= CT_WHITE;	
	s_subtitle.listnames									= s_subtitle_Names;

	Menu_AddItem( &s_gameoptions_menu, &s_preferences_wallmarks_box );
	Menu_AddItem( &s_gameoptions_menu, &s_preferences_wallmarks_value );
	Menu_AddItem( &s_gameoptions_menu, &s_preferences_dynamiclights_box );
	Menu_AddItem( &s_gameoptions_menu, &s_preferences_dynamiclights_value );
	Menu_AddItem( &s_gameoptions_menu, &s_preferences_lightflares_box );
	Menu_AddItem( &s_gameoptions_menu, &s_preferences_lightflares_value );
	Menu_AddItem( &s_gameoptions_menu, &s_preferences_identifytarget_box );
	Menu_AddItem( &s_gameoptions_menu, &s_preferences_identifytarget_value );
	Menu_AddItem( &s_gameoptions_menu, &s_preferences_synceveryframe_box );
	Menu_AddItem( &s_gameoptions_menu, &s_preferences_synceveryframe_value );
	Menu_AddItem( &s_gameoptions_menu, &s_preferences_crosshair_box);
	Menu_AddItem( &s_gameoptions_menu, &s_textlanguage);
	Menu_AddItem( &s_gameoptions_menu, &s_voicelanguage);
	Menu_AddItem( &s_gameoptions_menu, &s_subtitle);

	s_gameoptions_menu.initialized = qtrue;		// Show we've been here

	Preferences_SetValues();
}

/*
=================
UI_GameOptionsMenu
=================
*/
void UI_GameOptionsMenu( void )
{
	Preferences_MenuInit();

	UI_PushMenu( &s_gameoptions_menu );
}

