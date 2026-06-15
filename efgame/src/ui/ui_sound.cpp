#include "ui_local.h"
#include "gameinfo.h"

//===================================================================
//
// Sound Menu
//
//===================================================================

// Precache stuff for Sound Menu
static struct 
{
	qhandle_t	corner;
	qhandle_t	grid;
	qhandle_t	wave1;
	qhandle_t	wave2;
} soundmenu_cache;

static menuslider_s			s_sound_sfxvolume_slider;
static menuslider_s			s_sound_musicvolume_slider;
static menuslider_s			s_sound_voicevolume_slider;
static menulist_s			s_sound_quality_list;
static menulist_s			s_sound_A3D_list;

static int a3dtimer;

// menu action identifiers
#define ID_MAINMENU		100
#define ID_CONTROLS		101
#define ID_VIDEO		102
#define ID_SOUND		103
#define ID_GAMEOPTIONS	104
#define ID_CDKEY		105

static menuframework_s		s_sound_menu;

int					holdSoundQuality;

/*
=================
SoundMenu_SetVolumes
=================
*/
static void SoundMenu_SetVolumes( void *unused, int notification )
{
	if (notification != QM_ACTIVATED)
		return;
	
	ui.Cvar_SetValue( "s_volume", s_sound_sfxvolume_slider.curvalue / 100 );
	ui.Cvar_SetValue( "s_volumeVoice", s_sound_voicevolume_slider.curvalue / 100 );
	ui.Cvar_SetValue( "s_musicvolume", s_sound_musicvolume_slider.curvalue / 100 );
}


static void SoundQualityAction( qboolean result ) 
{
	if ( result ) 
	{
		holdSoundQuality = s_sound_quality_list.curvalue;
		if ( s_sound_quality_list.curvalue )
		{
			ui.Cvar_SetValue( "s_khz", 22 );
//			ui.Cvar_SetValue( "s_compression", qfalse );
		}
		else
		{
			ui.Cvar_SetValue( "s_khz", 11 );
//			ui.Cvar_SetValue( "s_compression", qtrue );
		}

		//UI_ForceMenuOff();
		ui.Cmd_ExecuteText( EXEC_APPEND, "snd_restart 1\n" );//1 means don't free mp3s
	}
	else
	{
		s_sound_quality_list.curvalue =	holdSoundQuality; 
	}
}

/*
=================
SoundMenu_UpdateSoundQuality
=================
*/
static void SoundMenu_UpdateSoundQuality( void *unused, int notification )
{
	if (notification != QM_ACTIVATED)
		return;

	//UI_ConfirmMenu(menu_normal_text[MNT_THISWILLRESETMENU],NULL,SoundQualityAction);
	SoundQualityAction(qtrue);
}

/*
=================
SoundMenu_UpdateA3D
=================
*/
static void SoundMenu_UpdateA3D( void *unused, int notification )
{
	int holdCurvalue;

	if (notification != QM_ACTIVATED)
		return;

	if( s_sound_A3D_list.curvalue ) {
		ui.Cmd_ExecuteText( EXEC_NOW, "s_enable_a3d\n" );
	}
	else {
		ui.Cmd_ExecuteText( EXEC_NOW, "s_disable_a3d\n" );
	}

//	s_sound_A3D_list.curvalue = (int)ui.Cvar_VariableValue( "s_usingA3D" );
	holdCurvalue = (int)ui.Cvar_VariableValue( "s_usingA3D" );
	if ((s_sound_A3D_list.curvalue == 1) && (holdCurvalue !=1))
	{
		a3dtimer = uis.realtime + 10000;
	}
	s_sound_A3D_list.curvalue = holdCurvalue;
}

/*
=================
SoundMenu_SetValues
=================
*/
static void SoundMenu_SetValues( void )
{
	s_sound_sfxvolume_slider.curvalue	= ui.Cvar_VariableValue( "s_volume" ) * 100;
	s_sound_voicevolume_slider.curvalue	= ui.Cvar_VariableValue( "s_volumevoice" ) * 100;
	s_sound_musicvolume_slider.curvalue	= ui.Cvar_VariableValue( "s_musicvolume" ) * 100;
	s_sound_quality_list.curvalue		= ui.Cvar_VariableValue( "s_khz" ) != 11;
	s_sound_A3D_list.curvalue = (int)ui.Cvar_VariableValue( "s_usingA3D" );

	holdSoundQuality = s_sound_quality_list.curvalue;

}

/*
=================
M_Sound_Graphics
=================
*/
void M_Sound_Graphics (void)
{

	UI_MenuFrame(&s_sound_menu);

	ui.R_SetColor( colorTable[CT_LTBROWN1]);
	UI_DrawHandlePic(30,203,  47, 186, uis.whiteShader);		// Long left column square on bottom 3rd

	UI_DrawHandlePic(88,164,  36, 36, uis.whiteShader);		// Column to side of sliders
	UI_DrawHandlePic(88,203,  36, 36, uis.whiteShader);		// 
	UI_DrawHandlePic(88,242,  36, 36, uis.whiteShader);		// 
	UI_DrawHandlePic(88,281,  36, 14, uis.whiteShader);		// 

	UI_DrawProportionalString(  120,  188, "336",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  120,  206, "724",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  120,  266, "836",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);

	if (a3dtimer > uis.realtime)
	{
		UI_DrawProportionalString(  490,  303, menu_normal_text[MNT_A3DNOTAVAILABLE],UI_CENTER|UI_SMALLFONT, colorTable[CT_WHITE]);
	}

	ui.R_SetColor( colorTable[CT_LTBROWN1]);
	UI_DrawHandlePic(112,297,  500, 4, uis.whiteShader);		// Middle long line
	UI_DrawHandlePic( 88, 294, 64, 8, soundmenu_cache.corner);	// Corner graphic

	ui.R_SetColor( colorTable[CT_DKBROWN1]);
	UI_DrawHandlePic(356,342,  8, 93, uis.whiteShader);		// Middle Bottom Left column
	ui.R_SetColor( colorTable[CT_LTPURPLE1]);
	UI_DrawHandlePic( 98, 359, 248, 64,soundmenu_cache.grid); 
	ui.R_SetColor( colorTable[CT_LTBLUE1]);
	UI_DrawHandlePic( 98, 359, 248, 64, soundmenu_cache.wave1); 
	ui.R_SetColor( colorTable[CT_DKBROWN1]);
	UI_DrawHandlePic(96,359,  4, 64, uis.whiteShader);		// Left side of frame
	UI_DrawHandlePic(342,359,  4, 64, uis.whiteShader);		// Right side of frame
	UI_DrawHandlePic(96,355,  250, 4, uis.whiteShader);		// Top of frame
	UI_DrawHandlePic(96,423,  250, 4, uis.whiteShader);		// Bottom of frame


	UI_DrawHandlePic(367,342,  8, 93, uis.whiteShader);		// Middle Bottom Right column
	ui.R_SetColor( colorTable[CT_LTPURPLE1]);
	UI_DrawHandlePic( 392, 359, 220, 64, soundmenu_cache.grid); 
	ui.R_SetColor( colorTable[CT_LTRED1]);
	UI_DrawHandlePic( 392, 359, 220, 64, soundmenu_cache.wave2); 
	ui.R_SetColor( colorTable[CT_DKBROWN1]);
	UI_DrawHandlePic(392, 359,   4, 64, uis.whiteShader);		// Left side of frame
	UI_DrawHandlePic(608, 359,   4, 64, uis.whiteShader);		// Right side of frame
	UI_DrawHandlePic(392,355,  220, 4, uis.whiteShader);		// Top of frame
	UI_DrawHandlePic(392,423,  220, 4, uis.whiteShader);		// Bottom of frame

	UI_Setup_MenuButtons();

	UI_DrawProportionalString(  74,  66, "29876",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  84, "568",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  188, "45-009",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  206, "18856",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  395, "18857",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);

}

/*
=================
M_Sound_MenuDraw
=================
*/
void M_Sound_MenuDraw (void)
{
	M_Sound_Graphics();

	Menu_Draw( &s_sound_menu );
}

/*
=================
Sound_MenuEvent
=================
*/
static void Sound_MenuEvent( void* ptr, int notification )
{
	menuframework_s*	m;

	if (notification != QM_ACTIVATED)
		return;

	m = ((menucommon_s*)ptr)->parent;

	switch (((menucommon_s*)ptr)->id)
	{
		case ID_SOUND:
			break;

		case ID_CONTROLS:
			UI_PopMenu();			// Get rid of whatever is ontop
			UI_SetupWeaponsMenu();	// Move to the Controls Menu
			break;

		case ID_VIDEO:
			UI_PopMenu();			// Get rid of whatever is ontop
			UI_VideoDataMenu();	// Move to the Video Menu
			break;

		case ID_GAMEOPTIONS:
			UI_PopMenu();			// Get rid of whatever is ontop
			UI_GameOptionsMenu();	// Move to the Game Options Menu
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
M_Sound_MenuKey
=================
*/
static sfxHandle_t M_Sound_MenuKey( int key )
{
	return Menu_DefaultKey( &s_sound_menu, key );
}


/*
===============
UI_SoundMenu_Cache
===============
*/
void UI_SoundMenu_Cache( void ) 
{

	soundmenu_cache.corner =  ui.R_RegisterShaderNoMip("menu/common/con_ssetup.tga");
	soundmenu_cache.grid =  ui.R_RegisterShaderNoMip("menu/special/grid.tga");
	soundmenu_cache.wave1 =  ui.R_RegisterShaderNoMip("menu/special/sinwave_1.tga");
	soundmenu_cache.wave2 =  ui.R_RegisterShaderNoMip("menu/special/sinwave_2.tga");
	ui.R_RegisterShaderNoMip("menu/common/monbar_2.tga");
}


/*
===============
SoundMenu_Init
===============
*/
void SoundMenu_Init(void) 
{
	int x,y;

	UI_SoundMenu_Cache();

	s_sound_menu.nitems					= 0;
	s_sound_menu.wrapAround				= qtrue;
	s_sound_menu.draw					= M_Sound_MenuDraw;
	s_sound_menu.key					= M_Sound_MenuKey;
	s_sound_menu.fullscreen				= qtrue;
	s_sound_menu.wrapAround				= qfalse;
	s_sound_menu.descX					= MENU_DESC_X;
	s_sound_menu.descY					= MENU_DESC_Y;
	s_sound_menu.titleX					= MENU_TITLE_X;
	s_sound_menu.titleY					= MENU_TITLE_Y;
	s_sound_menu.titleI					= MNT_CONTROLSMENU_TITLE;
	s_sound_menu.footNoteEnum			= MNT_SOUND_SETUP;

	x = 212;
	y = 175;

	s_sound_sfxvolume_slider.generic.type	   = MTYPE_SLIDER;
	s_sound_sfxvolume_slider.generic.x		   = x + MENU_BUTTON_MED_WIDTH + 35;
	s_sound_sfxvolume_slider.generic.y		   = y;
	s_sound_sfxvolume_slider.generic.flags	   = QMF_SMALLFONT;
	s_sound_sfxvolume_slider.generic.callback  = SoundMenu_SetVolumes;
	s_sound_sfxvolume_slider.minvalue		   = 0;
	s_sound_sfxvolume_slider.maxvalue		   = 100;
	s_sound_sfxvolume_slider.color				= CT_DKPURPLE1;
	s_sound_sfxvolume_slider.color2				= CT_LTPURPLE1;
	s_sound_sfxvolume_slider.generic.name		= "menu/common/monbar_2.tga";
	s_sound_sfxvolume_slider.width				= 256;
	s_sound_sfxvolume_slider.height				= 32;
	s_sound_sfxvolume_slider.focusWidth			= 145;
	s_sound_sfxvolume_slider.focusHeight		= 18;
	s_sound_sfxvolume_slider.picName			= "menu/common/square.tga";
	s_sound_sfxvolume_slider.picX				= x;
	s_sound_sfxvolume_slider.picY				= y;
	s_sound_sfxvolume_slider.picWidth			= MENU_BUTTON_MED_WIDTH;
	s_sound_sfxvolume_slider.picHeight			= MENU_BUTTON_MED_HEIGHT;
	s_sound_sfxvolume_slider.textEnum			= MBT_EFFECTSVOLUME;
	s_sound_sfxvolume_slider.textX				= 5;
	s_sound_sfxvolume_slider.textY				= 1;
	s_sound_sfxvolume_slider.textcolor			= CT_BLACK;
	s_sound_sfxvolume_slider.textcolor2			= CT_WHITE;
	s_sound_sfxvolume_slider.thumbName			= GRAPHIC_BUTTONSLIDER;
	s_sound_sfxvolume_slider.thumbHeight		= 32;
	s_sound_sfxvolume_slider.thumbWidth			= 16;
	s_sound_sfxvolume_slider.thumbGraphicWidth	= 9;
	s_sound_sfxvolume_slider.thumbColor			= CT_DKBLUE1;
	s_sound_sfxvolume_slider.thumbColor2		= CT_LTBLUE1;

	y = 212;
	s_sound_musicvolume_slider.generic.type			= MTYPE_SLIDER;
	s_sound_musicvolume_slider.generic.x			= x + MENU_BUTTON_MED_WIDTH + 35;
	s_sound_musicvolume_slider.generic.y			= y;
	s_sound_musicvolume_slider.generic.flags		= QMF_SMALLFONT;
	s_sound_musicvolume_slider.generic.callback		= SoundMenu_SetVolumes;
	s_sound_musicvolume_slider.minvalue				= 0;
	s_sound_musicvolume_slider.maxvalue				= 100;
	s_sound_musicvolume_slider.color				= CT_DKPURPLE1;
	s_sound_musicvolume_slider.color2				= CT_LTPURPLE1;
	s_sound_musicvolume_slider.generic.name			= "menu/common/monbar_2.tga";
	s_sound_musicvolume_slider.width				= 256;
	s_sound_musicvolume_slider.height				= 32;
	s_sound_musicvolume_slider.focusWidth			= 145;
	s_sound_musicvolume_slider.focusHeight			= 18;
	s_sound_musicvolume_slider.picName				= "menu/common/square.tga";
	s_sound_musicvolume_slider.picX					= x;
	s_sound_musicvolume_slider.picY					= y;
	s_sound_musicvolume_slider.picWidth				= MENU_BUTTON_MED_WIDTH;
	s_sound_musicvolume_slider.picHeight			= MENU_BUTTON_MED_HEIGHT;
	s_sound_musicvolume_slider.textEnum				= MBT_MUSICVOLUME;
	s_sound_musicvolume_slider.textX				= 5;
	s_sound_musicvolume_slider.textY				= 1;
	s_sound_musicvolume_slider.textcolor			= CT_BLACK;
	s_sound_musicvolume_slider.textcolor2			= CT_WHITE;
	s_sound_musicvolume_slider.thumbName			= GRAPHIC_BUTTONSLIDER;
	s_sound_musicvolume_slider.thumbHeight			= 32;
	s_sound_musicvolume_slider.thumbWidth			= 16;
	s_sound_musicvolume_slider.thumbGraphicWidth	= 9;
	s_sound_musicvolume_slider.thumbColor			= CT_DKBLUE1;
	s_sound_musicvolume_slider.thumbColor2			= CT_LTBLUE1;


	s_sound_voicevolume_slider.generic.type		   = MTYPE_SLIDER;
	s_sound_voicevolume_slider.generic.x		   = x + MENU_BUTTON_MED_WIDTH + 35;
	s_sound_voicevolume_slider.generic.y		   = 250;
	s_sound_voicevolume_slider.generic.flags	   = QMF_SMALLFONT;
	s_sound_voicevolume_slider.generic.callback		= SoundMenu_SetVolumes;
	s_sound_voicevolume_slider.minvalue				= 0;
	s_sound_voicevolume_slider.maxvalue				= 100;
	s_sound_voicevolume_slider.color				= CT_DKPURPLE1;
	s_sound_voicevolume_slider.color2				= CT_LTPURPLE1;
	s_sound_voicevolume_slider.generic.name			= "menu/common/monbar_2.tga";
	s_sound_voicevolume_slider.width				= 256;
	s_sound_voicevolume_slider.height				= 32;
	s_sound_voicevolume_slider.focusWidth			= 145;
	s_sound_voicevolume_slider.focusHeight			= 18;
	s_sound_voicevolume_slider.picName				= "menu/common/square.tga";
	s_sound_voicevolume_slider.picX					= x;
	s_sound_voicevolume_slider.picY					= 250;
	s_sound_voicevolume_slider.picWidth				= MENU_BUTTON_MED_WIDTH;
	s_sound_voicevolume_slider.picHeight			= MENU_BUTTON_MED_HEIGHT;
	s_sound_voicevolume_slider.textEnum				= MBT_VOICEVOLUME;
	s_sound_voicevolume_slider.textX				= 5;
	s_sound_voicevolume_slider.textY				= 1;
	s_sound_voicevolume_slider.textcolor			= CT_BLACK;
	s_sound_voicevolume_slider.textcolor2			= CT_WHITE;
	s_sound_voicevolume_slider.thumbName			= GRAPHIC_BUTTONSLIDER;
	s_sound_voicevolume_slider.thumbHeight			= 32;
	s_sound_voicevolume_slider.thumbWidth			= 16;
	s_sound_voicevolume_slider.thumbGraphicWidth	= 9;
	s_sound_voicevolume_slider.thumbColor			= CT_DKBLUE1;
	s_sound_voicevolume_slider.thumbColor2			= CT_LTBLUE1;

	static int s_sndquality_Names[] =
	{
		MNT_LOW,
		MNT_HIGH,
		MNT_NONE
	};

	s_sound_quality_list.generic.type				= MTYPE_SPINCONTROL;
	s_sound_quality_list.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS;
	s_sound_quality_list.generic.x					= 120;
	s_sound_quality_list.generic.y					= 322;
	s_sound_quality_list.generic.callback			= SoundMenu_UpdateSoundQuality;
	s_sound_quality_list.textEnum					= MBT_SOUNDQUALITY;
	s_sound_quality_list.textcolor					= CT_BLACK;
	s_sound_quality_list.textcolor2					= CT_WHITE;
	s_sound_quality_list.color						= CT_DKPURPLE1;
	s_sound_quality_list.color2						= CT_LTPURPLE1;
	s_sound_quality_list.textX						= 5;
	s_sound_quality_list.textY						= 2;
	s_sound_quality_list.listnames					= s_sndquality_Names;

	static int s_sndenable_Names[] =
	{
		MNT_OFF,
		MNT_ON,
		MNT_NONE
	};

	s_sound_A3D_list.generic.type				= MTYPE_SPINCONTROL;
	s_sound_A3D_list.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS;
	s_sound_A3D_list.generic.x					= 416;
	s_sound_A3D_list.generic.y					= 322;
	s_sound_A3D_list.generic.callback			= SoundMenu_UpdateA3D;
	s_sound_A3D_list.textEnum					= MBT_A3D;
	s_sound_A3D_list.textcolor					= CT_BLACK;
	s_sound_A3D_list.textcolor2					= CT_WHITE;
	s_sound_A3D_list.color						= CT_DKPURPLE1;
	s_sound_A3D_list.color2						= CT_LTPURPLE1;
	s_sound_A3D_list.textX						= 5;
	s_sound_A3D_list.textY						= 2;
	s_sound_A3D_list.listnames					= s_sndenable_Names;


	SetupMenu_TopButtons(&s_sound_menu,MENU_SOUND,NULL);

	Menu_AddItem( &s_sound_menu, ( void * )&s_sound_sfxvolume_slider);
	Menu_AddItem( &s_sound_menu, ( void * )&s_sound_musicvolume_slider);
	Menu_AddItem( &s_sound_menu, ( void * )&s_sound_voicevolume_slider);
	Menu_AddItem( &s_sound_menu, ( void * )&s_sound_quality_list);
	Menu_AddItem( &s_sound_menu, ( void * )&s_sound_A3D_list);

	SoundMenu_SetValues();

	s_sound_menu.initialized = qtrue;

}

/*
===============
UI_SoundMenu
===============
*/
void UI_SoundMenu( void) 
{
	
	SoundMenu_Init(); 

	UI_PushMenu( &s_sound_menu);
}

