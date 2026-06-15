#include "ui_local.h"
#include "gameinfo.h"

//===================================================================
//
// CDKey Menu
//
//===================================================================

// menu action identifiers
#define ID_MAINMENU		100
#define ID_CONTROLS		101
#define ID_VIDEO		102
#define ID_SOUND		103
#define ID_CDKEY		104

#define ART_FRAME		"menu/common/cdkey"
#define ART_WORMHOLE	"menu/wormhole/wormhole"

static menuframework_s	s_cdkey_menu;

void M_CDKey_Graphics (void)
{
	UI_MenuFrame(&s_cdkey_menu);

	UI_Setup_MenuButtons();

	ui.R_SetColor( colorTable[CT_DKPURPLE2]);

	UI_DrawHandlePic(30,203,  47, 121, uis.whiteShader);	// Long left hand column square
	UI_DrawHandlePic(30,327,  47, 28, uis.whiteShader);	// Long left hand column square
	UI_DrawHandlePic(30,358,  47, 31, uis.whiteShader);	// Long left hand column square

	// Numbers for left hand column squares
	UI_DrawProportionalString( 73, 206, "ST-181",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString( 73, 330, "65",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString( 73, 361, "201",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);

	ui.R_SetColor( colorTable[CT_WHITE]);
	UI_DrawNamedPic(97, 160, 512,   256,  ART_WORMHOLE);

	UI_DrawProportionalString(  611, 165, "WORMHOLE STRUCTURE", UI_RIGHT | UI_SMALLFONT, colorTable[CT_LTORANGE]);

	UI_DrawProportionalString(  419, 321, "POINT SINGULARITY", UI_TINYFONT, colorTable[CT_LTORANGE]);
	UI_DrawProportionalString(  360, 221, "VERTERONE MEMBRANE", UI_TINYFONT, colorTable[CT_LTORANGE]);
	UI_DrawProportionalString(  265, 329, "POSITIVE CTL REGION", UI_TINYFONT, colorTable[CT_LTORANGE]);
	UI_DrawProportionalString(  199, 288, "NEGATIVE CTL REGION", UI_TINYFONT, colorTable[CT_LTORANGE]);
	UI_DrawProportionalString(  287, 184, "RING SINGULARITY", UI_TINYFONT, colorTable[CT_LTORANGE]);
	UI_DrawProportionalString(  158, 230, "FTR PIPELINE", UI_TINYFONT, colorTable[CT_LTORANGE]);

	ui.R_SetColor( colorTable[CT_DKBLUE1]);
	UI_DrawHandlePic( 80,358,  185, 18, uis.whiteShader);	// Bar behind ENTER CD KEY
	UI_DrawHandlePic( 118,417,  128, 18, uis.whiteShader);	// Bar behind ACCEPT
	UI_DrawHandlePic( 267, 376,  8, 59, uis.whiteShader);	// Right hand side of box
	UI_DrawNamedPic(263, 358, 16,   32,  ART_FRAME);


	UI_DrawProportionalString(  146, 358, "ENTER CD KEY", UI_SMALLFONT, colorTable[CT_LTORANGE]);
	UI_DrawProportionalString(  152, 418, "ACCEPT", UI_SMALLFONT, colorTable[CT_LTORANGE]);

	ui.R_SetColor( colorTable[CT_DKGREY2]);
	UI_DrawHandlePic( 95,383,  160, 20, uis.whiteShader);	// Grey square to type in

	ui.R_SetColor( colorTable[CT_WHITE]);
	UI_DrawHandlePic( 95,402,  160, 1, uis.whiteShader);	// White line to type above

	ui.R_SetColor( colorTable[CT_DKPURPLE1]);
	UI_DrawHandlePic( 516,208,  21, 8, uis.whiteShader);	// Bar above labels
	UI_DrawHandlePic( 541,208,  21, 8, uis.whiteShader);	// Bar above labels
	UI_DrawHandlePic( 566,208,  21, 8, uis.whiteShader);	// Bar above labels
	UI_DrawHandlePic( 591,208,  21, 8, uis.whiteShader);	// Bar above labels

	UI_DrawHandlePic( 516,399,  21, 8, uis.whiteShader);	// Bar above labels
	UI_DrawHandlePic( 541,399,  21, 8, uis.whiteShader);	// Bar above labels
	UI_DrawHandlePic( 566,399,  21, 8, uis.whiteShader);	// Bar above labels
	UI_DrawHandlePic( 591,399,  21, 8, uis.whiteShader);	// Bar above labels

	UI_DrawProportionalString(  516, 223, "DIM    : 74156", UI_TINYFONT, colorTable[CT_LTORANGE]);
	UI_DrawProportionalString(  516, 237, "XYS DG : 21", UI_TINYFONT, colorTable[CT_LTORANGE]);
	UI_DrawProportionalString(  516, 251, "VGVH-A : 129430", UI_TINYFONT, colorTable[CT_LTORANGE]);

	UI_DrawProportionalString(  516, 275, "TTFN : 98231", UI_TINYFONT, colorTable[CT_LTORANGE]);
	UI_DrawProportionalString(  516, 289, "BB IO : 45", UI_TINYFONT, colorTable[CT_LTORANGE]);
	UI_DrawProportionalString(  516, 303, "R-T-PPL : 32", UI_TINYFONT, colorTable[CT_LTORANGE]);
	UI_DrawProportionalString(  516, 317, "B1 YOT : 810257", UI_TINYFONT, colorTable[CT_LTORANGE]);

	UI_DrawProportionalString(  516, 341, "XTR NTL: 171", UI_TINYFONT, colorTable[CT_LTORANGE]);
	UI_DrawProportionalString(  516, 355, "HGH NTL: 1348", UI_TINYFONT, colorTable[CT_LTORANGE]);
	UI_DrawProportionalString(  516, 369, "MED NTL: 45333", UI_TINYFONT, colorTable[CT_LTORANGE]);
	UI_DrawProportionalString(  516, 383, "LOW NTL: 29", UI_TINYFONT, colorTable[CT_LTORANGE]);

	// Menu frame numbers
	UI_DrawProportionalString(  74,  66, "67811",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  84, "5656",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  188, "76-0021",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  395, "456181",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);

}

/*
=================
M_CDKey_MenuDraw
=================
*/
void M_CDKey_MenuDraw (void)
{
	M_CDKey_Graphics();

	Menu_Draw( &s_cdkey_menu );
}

/*
=================
CDKey_MenuEvent
=================
*/
static void CDKey_MenuEvent( void* ptr, int notification )
{
	menuframework_s*	m;

	if (notification != QM_ACTIVATED)
		return;

	m = ((menucommon_s*)ptr)->parent;

	switch (((menucommon_s*)ptr)->id)
	{
		case ID_VIDEO:
			UI_PopMenu();
			UI_VideoDataMenu();
			return;
		case ID_CONTROLS:
			UI_PopMenu();
			UI_SetupWeaponsMenu();
			break;
		case ID_SOUND:
			UI_PopMenu();
			UI_SoundMenu();
			break;
		case ID_CDKEY:
			break;
		case ID_MAINMENU:
			UI_PopMenu();
			break;
	}
}

/*
=================
M_CDKey_MenuKey
=================
*/
static sfxHandle_t M_CDKey_MenuKey( int key )
{
	return Menu_DefaultKey( &s_cdkey_menu, key );
}


/*
===============
CDKeyMenu_Cache
===============
*/
void CDKeyMenu_Cache( void ) 
{
	ui.R_RegisterShaderNoMip(ART_FRAME);
	ui.R_RegisterShaderNoMip(ART_WORMHOLE);
}


/*
===============
CDKeyMenu_Init
===============
*/
void CDKeyMenu_Init(void) 
{

	CDKeyMenu_Cache();

	s_cdkey_menu.nitems					= 0;
	s_cdkey_menu.wrapAround				= qtrue;
	s_cdkey_menu.draw					= M_CDKey_MenuDraw;
	s_cdkey_menu.key					= M_CDKey_MenuKey;
	s_cdkey_menu.fullscreen				= qtrue;
	s_cdkey_menu.wrapAround				= qfalse;
	s_cdkey_menu.descX					= MENU_DESC_X;
	s_cdkey_menu.descY					= MENU_DESC_Y;
	s_cdkey_menu.titleX					= MENU_TITLE_X;
	s_cdkey_menu.titleY					= MENU_TITLE_Y;
	s_cdkey_menu.titleI					= MNT_CONTROLSMENU_TITLE;
	s_cdkey_menu.footNoteEnum			= MNT_CDKEY;

	SetupMenu_TopButtons(&s_cdkey_menu,MENU_CDKEY,NULL);

}

/*
===============
UI_CDKeyMenu
===============
*/
void UI_CDKeyMenu( void) 
{
	
	if (!s_cdkey_menu.initialized)
	{
		CDKeyMenu_Init(); 
	}

	UI_PushMenu( &s_cdkey_menu);
}

