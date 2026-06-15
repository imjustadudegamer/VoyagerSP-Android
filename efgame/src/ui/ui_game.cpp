#include "ui_local.h"
#include "gameinfo.h"

static qhandle_t		cornerPic;

static void LeaveGame_Action( void );
void UI_LCARSOut_Menu(void);

// Precache stuff for New Game Screen
static struct 
{
	qhandle_t		currentGameTopLeft;		// Upper left corner of current game box
	qhandle_t		swooshTop;
	qhandle_t		warpCore;
	qhandle_t		cornerUpper;
	qhandle_t		cornerLower;
	qhandle_t		swooshBottom;
	qhandle_t		gridbar;

	sfxHandle_t		warpconstant;
	sfxHandle_t		warpthrumslow;
	sfxHandle_t		warpthrummed;
	sfxHandle_t		warpthrumfast;

} newgame_cache;


menuframework_s s_leavegame_menu;
menubitmap_s s_leavegame_yes;
menubitmap_s s_leavegame_no;

#define ID_LEAVE_YES	8
#define ID_LEAVE_NO		9


#define LISTBUFSIZE 10240

static struct 
{
	menuframework_s menu;
	menubitmap_s	deletegame;
	menubitmap_s	arrowdwn;
	menubitmap_s	arrowup;

	menubitmap_s	yes;
	menubitmap_s	no;

	qhandle_t		currentGameTopLeft;		// Upper left corner of current game box
	qhandle_t		currentGameBotLeft;		// Bottom left corner of current game box
	qhandle_t		currentGameTopRight;	// Upper right corner of current game box
	qhandle_t		currentGameBotRight;	// Bottom right corner of current game box

	qhandle_t		corner_ur_16_18;		// Upper corner of directory box
	qhandle_t		corner_lr_8_16;			// Lower corner of directory box

	char	listBuf[LISTBUFSIZE];			//	The list of file names read in

	// For scrolling through file names 
	int				currentLine;		//	Index to currentSaveFileComments[] currently highlighted
	int				currentId;			//currently selected item id
	int				startISaveFile;		//	Index to first line in currentSaveFileComments[] displayed in window
	int				saveFileCnt;		//	Number of save files read in

	int				awaitingSave;		//	Flag to see if user wants to overwrite a game.

	char			*mapName;


	char			autoSaveComment[iSG_COMMENT_SIZE];	// Comment for autoesave game
	int				savegameFromFlag;
} s_savegame;
#define	AUTOSAVENAME "auto"
#define CURRENTLINE_NONE -1

#define HARD_INC 8
#define MEDIUM_INC 4
#define EASY_INC 2
#define SUPEREASY_INC 1

menubitmap_s	s_savegame_mainmenu;
menubitmap_s	s_savegame_start;

static int gameWindow;
static float gameWindowTime;

byte screenShotBuf[SG_SCR_WIDTH * SG_SCR_HEIGHT * 4];

static menufield_s		s_file_desc_field;
menubitmap_s	s_savegamedata;

menufield_s	s_savedline1;
menufield_s	s_savedline2;
menufield_s	s_savedline3;
menufield_s	s_savedline4;
menufield_s	s_savedline5;
menufield_s	s_savedline6;
menufield_s	s_savedline7;
menufield_s	s_savedline8;
menufield_s	s_savedline9;
menufield_s	s_savedline10;
menufield_s	s_savedline11;
menufield_s	s_savedline12;

menufield_s	s_savedate1;
menufield_s	s_savedate2;
menufield_s	s_savedate3;
menufield_s	s_savedate4;
menufield_s	s_savedate5;
menufield_s	s_savedate6;
menufield_s	s_savedate7;
menufield_s	s_savedate8;
menufield_s	s_savedate9;
menufield_s	s_savedate10;
menufield_s	s_savedate11;
menufield_s	s_savedate12;

menufield_s	s_savetime1;
menufield_s	s_savetime2;
menufield_s	s_savetime3;
menufield_s	s_savetime4;
menufield_s	s_savetime5;
menufield_s	s_savetime6;
menufield_s	s_savetime7;
menufield_s	s_savetime8;
menufield_s	s_savetime9;
menufield_s	s_savetime10;
menufield_s	s_savetime11;
menufield_s	s_savetime12;

#define MAX_FILEDISP 12

static void* g_savedline[] =
{
	&s_savedline1, 
	&s_savedline2, 
	&s_savedline3, 
	&s_savedline4, 
	&s_savedline5, 
	&s_savedline6, 
	&s_savedline7, 
	&s_savedline8, 
	&s_savedline9, 
	&s_savedline10, 
	&s_savedline11, 
	&s_savedline12, 
	NULL,
};

static void* g_savedate[] =
{
	&s_savedate1, 
	&s_savedate2, 
	&s_savedate3, 
	&s_savedate4, 
	&s_savedate5, 
	&s_savedate6, 
	&s_savedate7, 
	&s_savedate8, 
	&s_savedate9, 
	&s_savedate10, 
	&s_savedate11, 
	&s_savedate12, 
	NULL,
};

static void* g_savetime[] =
{
	&s_savetime1, 
	&s_savetime2, 
	&s_savetime3, 
	&s_savetime4, 
	&s_savetime5, 
	&s_savetime6, 
	&s_savetime7, 
	&s_savetime8, 
	&s_savetime9, 
	&s_savetime10, 
	&s_savetime11, 
	&s_savetime12, 
	NULL,
};

#define MAX_SAVELOADFILES	100
#define MAX_SAVELOADNAME	32

typedef struct 
{
	char currentSaveFileComments[iSG_COMMENT_SIZE];	// Hold file comments
	char *currentSaveFileNames;	// Hold file names in save directory
	char *currentSaveFileDateTime;	// Hold file names in save directory
} savedata_t;

static savedata_t s_savedata[MAX_SAVELOADFILES];

//static char currentSaveFileComments[MAX_SAVELOADFILES][iSG_COMMENT_SIZE];	// Hold file comments
//static char *currentSaveFileNames[MAX_SAVELOADFILES];	// Hold file names in save directory
//static char *currentSaveFileDateTime[MAX_SAVELOADFILES];	// Hold file names in save directory


//===================================================================
//
// Save Game Menu
//
//===================================================================

// menu action identifiers
#define	ID_ARROWUP					10
#define	ID_ARROWDWN					11

#define	ID_YES						13
#define	ID_NO						14
#define ID_AUTOSAVE					15
#define	ID_MAINMENU					103
#define	ID_RESUMETOUR				204
#define	ID_TOUR_LINE1				220
#define	ID_TOUR_LINE2				221
#define	ID_TOUR_LINE3				222
#define	ID_TOUR_LINE4				223
#define	ID_TOUR_LINE5				224
#define	ID_TOUR_LINE6				225

#define ID_SAVEGAMEDATA				9000
#define ID_LOADGAMEDATA				9001
#define ID_DELETEGAMEDATA			9002

#define ID_SAVEDCOMMENT1				110
#define ID_SAVEDCOMMENT2				111
#define ID_SAVEDCOMMENT3				112
#define ID_SAVEDCOMMENT4				113
#define ID_SAVEDCOMMENT5				114
#define ID_SAVEDCOMMENT6				115
#define ID_SAVEDCOMMENT7				116
#define ID_SAVEDCOMMENT8				117
#define ID_SAVEDCOMMENT9				118
#define ID_SAVEDCOMMENT10				119
#define ID_SAVEDCOMMENT11				120
#define ID_SAVEDCOMMENT12				121

// Thumbnail locations
#define SGPIC_X 430							// Beginning X of thumbnail
#define SGPIC_Y 308							// Bottom Y of thumbnail
#define SGPIC_W 171							// Width of thumbnail
#define SGPIC_H (SGPIC_W * 0.75)			// Height of thumbnail
#define SGPIC_BRD 5							//border around
#define SGBLIP_W  5							// Blip width
#define SGBLIP_MIN_X  (SGPIC_X + SGBLIP_W )							// Blip starting X
#define SGBLIP_MAX_X  (SGPIC_X + SGPIC_W - SGBLIP_W -  SGPIC_BRD)	// Blip ending X

typedef enum 
{
	NGG_POWERBAND1_1,		// LH TOP
	NGG_POWERBAND1_2,		// LH BOTTOM
	NGG_WARPBAND1_1,		// LH TOP
	NGG_WARPBAND1_2,		// RH TOP
	NGG_WARPBAND1_3,		// LH BOTTOM
	NGG_WARPBAND1_4,		// RH BOTTOM

	NGG_TOP_NUMBERS,

	NGG_TOP_SPEC_ROW1,
	NGG_TOP_SPEC_ROW2,
	NGG_TOP_SPEC_ROW3,

	NGG_TOP_COL1_NUM1,
	NGG_TOP_COL1_NUM2,
	NGG_TOP_COL1_NUM3,
	NGG_TOP_COL1_NUM4,
	NGG_TOP_COL1_NUM5,

	NGG_TOP_COL2_NUM1,
	NGG_TOP_COL2_NUM2,
	NGG_TOP_COL2_NUM3,
	NGG_TOP_COL2_NUM4,
	NGG_TOP_COL2_NUM5,

	NGG_TOP_COL3_NUM1,
	NGG_TOP_COL3_NUM2,
	NGG_TOP_COL3_NUM3,
	NGG_TOP_COL3_NUM4,
	NGG_TOP_COL3_NUM5,

	NGG_NUMBERS,

	NGG_SPEC_ROW1,
	NGG_SPEC_ROW2,
	NGG_SPEC_ROW3,

	NGG_COL1_NUM1,
	NGG_COL1_NUM2,
	NGG_COL1_NUM3,
	NGG_COL1_NUM4,
	NGG_COL1_NUM5,
	NGG_COL1_NUM6,
	NGG_COL1_NUM7,

	NGG_COL2_NUM1,
	NGG_COL2_NUM2,
	NGG_COL2_NUM3,
	NGG_COL2_NUM4,
	NGG_COL2_NUM5,
	NGG_COL2_NUM6,
	NGG_COL2_NUM7,

	NGG_COL3_NUM1,
	NGG_COL3_NUM2,
	NGG_COL3_NUM3,
	NGG_COL3_NUM4,
	NGG_COL3_NUM5,
	NGG_COL3_NUM6,
	NGG_COL3_NUM7,

	NGG_COL4_NUM1,
	NGG_COL4_NUM2,
	NGG_COL4_NUM3,
	NGG_COL4_NUM4,
	NGG_COL4_NUM5,
	NGG_COL4_NUM6,
	NGG_COL4_NUM7,

	NGG_COL5_NUM1,
	NGG_COL5_NUM2,
	NGG_COL5_NUM3,
	NGG_COL5_NUM4,
	NGG_COL5_NUM5,
	NGG_COL5_NUM6,
	NGG_COL5_NUM7,

	NGG_COL6_NUM1,
	NGG_COL6_NUM2,
	NGG_COL6_NUM3,
	NGG_COL6_NUM4,
	NGG_COL6_NUM5,
	NGG_COL6_NUM6,
	NGG_COL6_NUM7,

	NGG_MASTER_ARROW,

	NGG_BAR1_LARROW,
	NGG_BAR1_RARROW,

	NGG_BAR2_LARROW,
	NGG_BAR2_RARROW,

	NGG_BAR3_LARROW,
	NGG_BAR3_RARROW,

	NGG_BAR4_LARROW,
	NGG_BAR4_RARROW,

	NGG_MAX
} newgame_graphics_t;

menugraphics_s newgamemenu_graphics[NGG_MAX] = 
{
//	type		timer	x		y		width	height	file/text						graphic,	min		max	target	inc		style	color
	MG_GRAPHIC,	0.0,	147,	281,	5,		7,		"menu/common/square.tga",0,0,		NULL,		147,	296,	0,		1,		0,		CT_WHITE,	NULL,	// NGG_POWERBAND1_1
	MG_GRAPHIC,	0.0,	147,	304,	5,		7,		"menu/common/square.tga",0,0,		NULL,		147,	296,	0,		1,		0,		CT_WHITE,	NULL,	// NGG_POWERBAND1_2

	MG_GRAPHIC,	0.0,	306,	  0,	17,		7,		"menu/common/square.tga",0,0,		NULL,		272,	189,	0,		1,		0,		CT_WHITE,	NULL,	// NGG_WARPBAND1_1
	MG_GRAPHIC,	0.0,	339,	  0,	17,		7,		"menu/common/square.tga",0,0,		NULL,		272,	189,	0,		1,		0,		CT_WHITE,	NULL,	// NGG_WARPBAND1_2
	MG_GRAPHIC,	0.0,	306,	  0,	17,		7,		"menu/common/square.tga",0,0,		NULL,		319,	400,	0,		1,		0,		CT_WHITE,	NULL,	// NGG_WARPBAND1_3
	MG_GRAPHIC,	0.0,	339,	  0,	17,		7,		"menu/common/square.tga",0,0,		NULL,		319,	400,	0,		1,		0,		CT_WHITE,	NULL,	// NGG_WARPBAND1_4

	MG_VAR,		0.0,	0,		0,		0,		0,		NULL,					0,0,		NULL,		0,		0,		0,		0,		0,			0,			NULL,	// NGG_TOP_NUMBERS

	MG_GRAPHIC,	0.0,	486,	  0,	4,		4,		"menu/common/circle.tga",0,0,		NULL,		0,		0,		0,		0,		0,		CT_LTGOLD1,	NULL,	// NGG_TOP_SPEC_ROW1
	MG_GRAPHIC,	0.0,	486,	  0,	4,		4,		"menu/common/circle.tga",0,0,		NULL,		0,		0,		0,		0,		0,		CT_LTGOLD1,	NULL,	// NGG_TOP_SPEC_ROW2
	MG_GRAPHIC,	0.0,	486,	  0,	4,		4,		"menu/common/circle.tga",0,0,		NULL,		0,		0,		0,		0,		0,		CT_LTGOLD1,	NULL,	// NGG_TOP_SPEC_ROW3

	MG_NUMBER,	0.0,	490,	60,		16,		10,		NULL,					0,0,		NULL,		0,		9,		0,		0,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// NGG_COL1_NUM1
	MG_NUMBER,	0.0,	490,	74,		16,		10,		NULL,					0,0,		NULL,		0,		9,		0,		0,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// NGG_COL1_NUM2
	MG_NUMBER,	0.0,	490,	88,		16,		10,		NULL,					0,0,		NULL,		0,		9,		0,		0,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// NGG_COL1_NUM3
	MG_NUMBER,	0.0,	490,	102,	16,		10,		NULL,					0,0,		NULL,		0,		9,		0,		0,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// NGG_COL1_NUM4
	MG_NUMBER,	0.0,	490,	116,	16,		10,		NULL,					0,0,		NULL,		0,		9,		0,		0,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// NGG_COL1_NUM5

	MG_NUMBER,	0.0,	545,	60,		16,		10,		NULL,					0,0,		NULL,		0,		4,		0,		0,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// NGG_COL2_NUM1
	MG_NUMBER,	0.0,	545,	74,		16,		10,		NULL,					0,0,		NULL,		0,		4,		0,		0,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// NGG_COL2_NUM2
	MG_NUMBER,	0.0,	545,	88,		16,		10,		NULL,					0,0,		NULL,		0,		4,		0,		0,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// NGG_COL2_NUM3
	MG_NUMBER,	0.0,	545,	102,	16,		10,		NULL,					0,0,		NULL,		0,		4,		0,		0,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// NGG_COL2_NUM4
	MG_NUMBER,	0.0,	545,	116,	16,		10,		NULL,					0,0,		NULL,		0,		4,		0,		0,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// NGG_COL2_NUM5

	MG_NUMBER,	0.0,	575,	60,		16,		10,		NULL,					0,0,		NULL,		0,		6,		0,		0,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// NGG_COL3_NUM1
	MG_NUMBER,	0.0,	575,	74,		16,		10,		NULL,					0,0,		NULL,		0,		6,		0,		0,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// NGG_COL3_NUM2
	MG_NUMBER,	0.0,	575,	88,		16,		10,		NULL,					0,0,		NULL,		0,		6,		0,		0,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// NGG_COL3_NUM3
	MG_NUMBER,	0.0,	575,	102,	16,		10,		NULL,					0,0,		NULL,		0,		6,		0,		0,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// NGG_COL3_NUM4
	MG_NUMBER,	0.0,	575,	116,	16,		10,		NULL,					0,0,		NULL,		0,		6,		0,		0,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// NGG_COL3_NUM5

	MG_VAR,		0.0,	0,		0,		0,		0,		NULL,					0,0,		NULL,		0,		0,		0,		0,		0,		0,			NULL,	// NGG_NUMBERS

	MG_GRAPHIC,	0.0,	402,	  0,	4,		4,		"menu/common/circle.tga",0,0,		NULL,		0,		0,		0,		0,		0,		CT_LTGOLD1,	NULL,	// NGG_SPEC_ROW1
	MG_GRAPHIC,	0.0,	402,	  0,	4,		4,		"menu/common/circle.tga",0,0,		NULL,		0,		0,		0,		0,		0,		CT_LTGOLD1,	NULL,	// NGG_SPEC_ROW2
	MG_GRAPHIC,	0.0,	402,	  0,	4,		4,		"menu/common/circle.tga",0,0,		NULL,		0,		0,		0,		0,		0,		CT_LTGOLD1,	NULL,	// NGG_SPEC_ROW3

	MG_NUMBER,	0.0,	410,	  0,	16,		10,		NULL,					0,0,		NULL,		0,		2,		0,		12,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// NGG_COL1_NUM1
	MG_NUMBER,	0.0,	410,	  0,	16,		10,		NULL,					0,0,		NULL,		0,		2,		0,		12,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// NGG_COL1_NUM2
	MG_NUMBER,	0.0,	410,	  0,	16,		10,		NULL,					0,0,		NULL,		0,		2,		0,		12,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// NGG_COL1_NUM3
	MG_NUMBER,	0.0,	410,	  0,	16,		10,		NULL,					0,0,		NULL,		0,		2,		0,		12,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// NGG_COL1_NUM4
	MG_NUMBER,	0.0,	410,	  0,	16,		10,		NULL,					0,0,		NULL,		0,		2,		0,		12,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// NGG_COL1_NUM5
	MG_NUMBER,	0.0,	410,	  0,	16,		10,		NULL,					0,0,		NULL,		0,		2,		0,		12,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// NGG_COL1_NUM6
	MG_NUMBER,	0.0,	410,	  0,	16,		10,		NULL,					0,0,		NULL,		0,		2,		0,		12,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// NGG_COL1_NUM7

	MG_NUMBER,	0.0,	429,	  0,	16,		10,		NULL,					0,0,		NULL,		0,		2,		0,		12,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// NGG_COL2_NUM1
	MG_NUMBER,	0.0,	429,	  0,	16,		10,		NULL,					0,0,		NULL,		0,		2,		0,		12,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// NGG_COL2_NUM2
	MG_NUMBER,	0.0,	429,	  0,	16,		10,		NULL,					0,0,		NULL,		0,		2,		0,		12,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// NGG_COL2_NUM3
	MG_NUMBER,	0.0,	429,	  0,	16,		10,		NULL,					0,0,		NULL,		0,		2,		0,		12,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// NGG_COL2_NUM4
	MG_NUMBER,	0.0,	429,	  0,	16,		10,		NULL,					0,0,		NULL,		0,		2,		0,		12,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// NGG_COL2_NUM5
	MG_NUMBER,	0.0,	429,	  0,	16,		10,		NULL,					0,0,		NULL,		0,		2,		0,		12,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// NGG_COL2_NUM6
	MG_NUMBER,	0.0,	429,	  0,	16,		10,		NULL,					0,0,		NULL,		0,		2,		0,		12,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// NGG_COL2_NUM7

	MG_NUMBER,	0.0,	448,	  0,	16,		10,		NULL,					0,0,		NULL,		0,		9,		0,		12,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// NGG_COL3_NUM1
	MG_NUMBER,	0.0,	448,	  0,	16,		10,		NULL,					0,0,		NULL,		0,		9,		0,		12,		UI_TINYFONT,CT_LTGOLD1,	NULL,// NGG_COL3_NUM2
	MG_NUMBER,	0.0,	448,	  0,	16,		10,		NULL,					0,0,		NULL,		0,		9,		0,		12,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// NGG_COL3_NUM3
	MG_NUMBER,	0.0,	448,	  0,	16,		10,		NULL,					0,0,		NULL,		0,		9,		0,		12,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// NGG_COL3_NUM4
	MG_NUMBER,	0.0,	448,	  0,	16,		10,		NULL,					0,0,		NULL,		0,		9,		0,		12,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// NGG_COL3_NUM5
	MG_NUMBER,	0.0,	448,	  0,	16,		10,		NULL,					0,0,		NULL,		0,		9,		0,		12,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// NGG_COL3_NUM6
	MG_NUMBER,	0.0,	448,	  0,	16,		10,		NULL,					0,0,		NULL,		0,		9,		0,		12,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// NGG_COL3_NUM7

	MG_NUMBER,	0.0,	509,	  0,	16,		10,		NULL,					0,0,		NULL,		0,		9,		0,		12,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// NGG_COL4_NUM1
	MG_NUMBER,	0.0,	509,	  0,	16,		10,		NULL,					0,0,		NULL,		0,		9,		0,		12,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// NGG_COL4_NUM2
	MG_NUMBER,	0.0,	509,	  0,	16,		10,		NULL,					0,0,		NULL,		0,		9,		0,		12,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// NGG_COL4_NUM3
	MG_NUMBER,	0.0,	509,	  0,	16,		10,		NULL,					0,0,		NULL,		0,		9,		0,		12,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// NGG_COL4_NUM4
	MG_NUMBER,	0.0,	509,	  0,	16,		10,		NULL,					0,0,		NULL,		0,		9,		0,		12,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// NGG_COL4_NUM5
	MG_NUMBER,	0.0,	509,	  0,	16,		10,		NULL,					0,0,		NULL,		0,		9,		0,		12,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// NGG_COL4_NUM6
	MG_NUMBER,	0.0,	509,	  0,	16,		10,		NULL,					0,0,		NULL,		0,		9,		0,		12,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// NGG_COL4_NUM7

	MG_NUMBER,	0.0,	570,	  0,	16,		10,		NULL,					0,0,		NULL,		0,		3,		0,		12,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// NGG_COL5_NUM1
	MG_NUMBER,	0.0,	570,	  0,	16,		10,		NULL,					0,0,		NULL,		0,		3,		0,		12,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// NGG_COL5_NUM2
	MG_NUMBER,	0.0,	570,	  0,	16,		10,		NULL,					0,0,		NULL,		0,		3,		0,		12,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// NGG_COL5_NUM3
	MG_NUMBER,	0.0,	570,	  0,	16,		10,		NULL,					0,0,		NULL,		0,		3,		0,		12,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// NGG_COL5_NUM4
	MG_NUMBER,	0.0,	570,	  0,	16,		10,		NULL,					0,0,		NULL,		0,		3,		0,		12,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// NGG_COL5_NUM5
	MG_NUMBER,	0.0,	570,	  0,	16,		10,		NULL,					0,0,		NULL,		0,		3,		0,		12,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// NGG_COL5_NUM6
	MG_NUMBER,	0.0,	570,	  0,	16,		10,		NULL,					0,0,		NULL,		0,		3,		0,		12,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// NGG_COL5_NUM7

	MG_NUMBER,	0.0,	593,	  0,	16,		10,		NULL,					0,0,		NULL,		0,		3,		0,		12,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// NGG_COL6_NUM1
	MG_NUMBER,	0.0,	593,	  0,	16,		10,		NULL,					0,0,		NULL,		0,		3,		0,		12,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// NGG_COL6_NUM2
	MG_NUMBER,	0.0,	593,	  0,	16,		10,		NULL,					0,0,		NULL,		0,		3,		0,		12,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// NGG_COL6_NUM3
	MG_NUMBER,	0.0,	593,	  0,	16,		10,		NULL,					0,0,		NULL,		0,		3,		0,		12,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// NGG_COL6_NUM4
	MG_NUMBER,	0.0,	593,	  0,	16,		10,		NULL,					0,0,		NULL,		0,		3,		0,		12,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// NGG_COL6_NUM5
	MG_NUMBER,	0.0,	593,	  0,	16,		10,		NULL,					0,0,		NULL,		0,		3,		0,		12,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// NGG_COL6_NUM6
	MG_NUMBER,	0.0,	593,	  0,	16,		10,		NULL,					0,0,		NULL,		0,		3,		0,		12,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// NGG_COL6_NUM7

	MG_NONE,	0.0,	0,		  0,	0,		0,		NULL,					0,0,		NULL,	    0,	    0,		0,		0,		0,		CT_NONE,	NULL,		// NGG_MASTER_ARROW

	MG_GRAPHIC,	0.0,	136,	 84,	8,		8,		"menu/common/right_arrow.tga",	0,0,	NULL,	   67,	  126,		0,		1,		0,		CT_LTPURPLE2,	NULL,	// NGG_BAR1_LARROW
	MG_GRAPHIC,	0.0,	155,	 89,	8,		8,		"menu/common/left_arrow.tga",	0,0,	NULL,	   67,	  126,		0,		1,		0,		CT_LTPURPLE2,	NULL,	// NGG_BAR1_RARROW

	MG_GRAPHIC,	0.0,	215,	 84,	8,		8,		"menu/common/right_arrow.tga",	0,0,NULL,	   67,	  126,		0,		1,		0,		CT_LTBLUE1,	NULL,		// NGG_BAR2_LARROW
	MG_GRAPHIC,	0.0,	234,	 89,	8,		8,		"menu/common/left_arrow.tga",	0,0,NULL,	   67,	  126,		0,		1,		0,		CT_LTBLUE1,	NULL,		// NGG_BAR2_RARROW

	MG_GRAPHIC,	0.0,	294,	 84,	8,		8,		"menu/common/right_arrow.tga",	0,0,NULL,	   67,	  126,		0,		1,		0,		CT_LTGOLD1,	NULL,		// NGG_BAR3_LARROW
	MG_GRAPHIC,	0.0,	313,	 89,	8,		8,		"menu/common/left_arrow.tga",	0,0,NULL,	   67,	  126,		0,		1,		0,		CT_LTGOLD1,		NULL,	// NGG_BAR3_RARROW

	MG_GRAPHIC,	0.0,	373,	 84,	8,		8,		"menu/common/right_arrow.tga",	0,0,NULL,	   67,	  126,		0,		5,		0,		CT_LTRED1,		NULL,	// NGG_BAR4_LARROW
	MG_GRAPHIC,	0.0,	392,	 89,	8,		8,		"menu/common/left_arrow.tga",	0,0,NULL,	   67,	  126,		0,		5,		0,		CT_LTRED1,	NULL,		// NGG_BAR4_RARROW
};
typedef enum 
{
	LGG_BLIP1,		
	LGG_BLIP2,		
	LGG_COL1_NUM1,
	LGG_COL1_NUM2,
	LGG_COL1_NUM3,
	LGG_COL2_NUM1,
	LGG_COL2_NUM2,
	LGG_COL2_NUM3,
	LGG_COL3_NUM1,
	LGG_COL3_NUM2,
	LGG_COL3_NUM3,
	LGG_COL4_NUM1,
	LGG_COL4_NUM2,
	LGG_COL4_NUM3,
	LGG_MAX
} loadgamemenu_graphics_t;

menugraphics_s loadgamemenu_graphics[LGG_MAX] = 
{
//	type		timer	x				y						width			height	file/text						graphic,	min				max				target	inc		style	color
	MG_GRAPHIC,	0.0,	SGBLIP_MIN_X,	SGPIC_Y-4-SGPIC_H-4-4,	SGBLIP_W,		8,		"menu/common/square.tga",0,0,		NULL,	SGBLIP_MIN_X,	SGBLIP_MAX_X,	0,		3,		0,		CT_LTPURPLE1,	NULL,	// LGG_BLIP1
	MG_GRAPHIC,	0.0,	SGBLIP_MIN_X,	SGPIC_Y+4,				SGBLIP_W,		8,		"menu/common/square.tga",0,0,		NULL,	SGBLIP_MIN_X,	SGBLIP_MAX_X,	0,		3,		0,		CT_LTPURPLE1,	NULL,	// LGG_BLIP2

//	type		timer	x		y		width	height	file/text						graphic,	min		max		target	inc		style	color
	MG_NUMBER,	0.0,	426,	382,	16,		10,		NULL,					0,0,		NULL,		0,		8,		0,		0,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// LGG_COL1_NUM1
	MG_NUMBER,	0.0,	426,	396,	16,		10,		NULL,					0,0,		NULL,		0,		8,		0,		0,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// LGG_COL1_NUM2
	MG_NUMBER,	0.0,	426,	410,	16,		10,		NULL,					0,0,		NULL,		0,		8,		0,		0,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// LGG_COL1_NUM3

	MG_NUMBER,	0.0,	479,	382,	16,		10,		NULL,					0,0,		NULL,		0,		8,		0,		0,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// LGG_COL2_NUM1
	MG_NUMBER,	0.0,	479,	396,	16,		10,		NULL,					0,0,		NULL,		0,		8,		0,		0,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// LGG_COL2_NUM2
	MG_NUMBER,	0.0,	479,	410,	16,		10,		NULL,					0,0,		NULL,		0,		8,		0,		0,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// LGG_COL2_NUM3

	MG_NUMBER,	0.0,	539,	382,	16,		10,		NULL,					0,0,		NULL,		0,		6,		0,		0,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// LGG_COL3_NUM1
	MG_NUMBER,	0.0,	539,	396,	16,		10,		NULL,					0,0,		NULL,		0,		6,		0,		0,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// LGG_COL3_NUM2
	MG_NUMBER,	0.0,	539,	410,	16,		10,		NULL,					0,0,		NULL,		0,		6,		0,		0,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// LGG_COL3_NUM3

	MG_NUMBER,	0.0,	577,	382,	16,		10,		NULL,					0,0,		NULL,		0,		4,		0,		0,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// LGG_COL4_NUM1
	MG_NUMBER,	0.0,	577,	396,	16,		10,		NULL,					0,0,		NULL,		0,		4,		0,		0,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// LGG_COL4_NUM2
	MG_NUMBER,	0.0,	577,	410,	16,		10,		NULL,					0,0,		NULL,		0,		4,		0,		0,		UI_TINYFONT,CT_LTGOLD1,	NULL,	// LGG_COL4_NUM3

};

/*
=================
UI_WipeTourHubSaveDirectory

  FIXME: pass in the "tour" part so this could potentially be used for any hub system?
=================
*/
static void UI_WipeTourHubSaveDirectory (void)
{
	int		i;
	char	*holdChar;
	int		len;
	int		fileCnt;
	char	commandString[32];

	// Clear out save data
	memset(s_savedata,0,sizeof(s_savedata));

	// Get everything in saves directory
	fileCnt = ui.FS_GetFileList("saves/hub/tour", ".sav", s_savegame.listBuf, LISTBUFSIZE );

	holdChar = s_savegame.listBuf;
	for ( i = 0; i < fileCnt; i++, holdChar += len + 1 ) 
	{
		// strip extension
		len = strlen( holdChar );
		holdChar[len-4] = '\0';
		Com_sprintf( commandString,  sizeof(commandString), "wipe hub/tour/%s\n", holdChar);
		ui.Cmd_ExecuteText( EXEC_APPEND, commandString);
	}
}

/*
=======================
UI_SortSaveGames
=======================
*/
static int UI_SortSaveGames( const void *a, const void *b ) 
{
	char	*ea, *eb;
	char *timeA,*timeB;
	char *dateA,*dateB;
	int		value;

	ea = (char *) a;
	eb = (char *) b;

	timeA = (ea + iSG_COMMENT_SIZE/2);
	dateA = (ea + iSG_COMMENT_SIZE/2 + 9);

	timeB = (eb +iSG_COMMENT_SIZE/2);
	dateB = (eb + iSG_COMMENT_SIZE/2 + 9);

	value = Q_stricmp( dateB, dateA );
	if (!value)	//	Equal date
	{
		value = Q_stricmp( timeB, timeA );
		return value;
	}

	return value;
}

/*
=================
ReadSaveDirectory
=================
*/
static void ReadSaveDirectory (void)
{
	int		i,result;
	char	*holdChar;
	int		len;
	int		fileCnt;

	// Clear out save data
	memset(s_savedata,0,sizeof(s_savedata));

	// Get everything in saves directory
	fileCnt = ui.FS_GetFileList("saves", ".sav", s_savegame.listBuf, LISTBUFSIZE );

	holdChar = s_savegame.listBuf;
	for ( i = 0; i < fileCnt; i++ ) 
	{

		// NOTE: retail filtered "auto.sav" out of the list (it had a dedicated AUTOSAVE
		// button). This port surfaces the autosave INLINE in the normal list instead, so
		// only the internal engine slots (virtual/current) are excluded here.
		if	((Q_strncmp("virtual.sav",holdChar,11)!=0) &&
			(Q_strncmp("current.sav",holdChar,11)!=0))
		{
			s_savedata[i].currentSaveFileNames = holdChar;
		}
		
		// strip extension
		len = strlen( holdChar );
		holdChar[len-4] = '\0';
		holdChar += len + 1;
	}

	s_savegame.saveFileCnt = 0;

	// Get comments
	for (i=0;i<fileCnt;i++)
	{
		// Is this a valid file???
		result = ui.SG_ValidateForLoadSaveScreen(s_savedata[i].currentSaveFileNames);
		if (result == qfalse) // Bad save game - ignore it
		{
			continue;
		}

		// Get comment of file
		holdChar = (char *) ui.SG_GetSaveGameComment(s_savedata[i].currentSaveFileNames);
		if (holdChar)	//	Was there a file?
		{
			if (*holdChar)
			{
				// Get comment
				memmove(s_savedata[i].currentSaveFileComments,holdChar,iSG_COMMENT_SIZE);	//not strncpy because of NULLs in name

				// Get date/time at end of comment field
				s_savedata[i].currentSaveFileDateTime = s_savedata[i].currentSaveFileComments + (iSG_COMMENT_SIZE/2);

				// End of comment
				*(s_savedata[i].currentSaveFileComments + (iSG_COMMENT_SIZE/2) - 1) = '\0';
				// End of time
				*(s_savedata[i].currentSaveFileComments + (iSG_COMMENT_SIZE/2) - 1 + 9) = '\0';
				// End of date
				*(s_savedata[i].currentSaveFileComments + (iSG_COMMENT_SIZE/2) - 1 + 20) = '\0';

				// Place string end for map name
				holdChar = (s_savedata[i].currentSaveFileComments + iSG_COMMENT_SIZE - 2);
				// Work backwards
				while (*holdChar==' ')
				{
					holdChar--;
					// Past beginnning of string
					if (holdChar < s_savedata[i].currentSaveFileComments)
					{
						holdChar = s_savedata[i].currentSaveFileComments;
						break;
					}
				}
				holdChar++;
				*holdChar = '\0';
			}
		}

		s_savegame.saveFileCnt++;
	}	

	qsort( s_savedata, fileCnt,sizeof(savedata_t), UI_SortSaveGames );

	// Get date/time now that sorting is done
	for (i=0;i<fileCnt;i++)
	{
		// Get comment of file
		if (s_savedata[i].currentSaveFileNames)	//	Was there a file?
		{
			// Get date/time at end of comment field
			s_savedata[i].currentSaveFileDateTime = s_savedata[i].currentSaveFileComments + (iSG_COMMENT_SIZE/2);
		}
	}	
}

/*
=================
CreateNextSaveName
=================
*/
static int CreateNextSaveName(char *fileName)
{
	int i,length;

	// Loop through all the save games and look for the first open name
	for (i=0;i<MAX_SAVELOADFILES;i++)
	{
		if (i>9)	// eforce10 - eforce99
		{
			length = 9;
		}
		else		// eforce0 - eforce9
		{
			length = 8;
		}

		Com_sprintf( fileName, length, "eforce%d", i );

		if (!ui.SG_ValidateForLoadSaveScreen(fileName))
		{
			return qtrue;
		}
	}

	return qfalse;
}

/*
=================
SaveGame_UpdateWindowArrows
=================
*/
static void SaveGame_UpdateWindowArrows(void)
{
	// If no more files below, turn off down arrow
	if (s_savegame.saveFileCnt > (s_savegame.startISaveFile + MAX_FILEDISP))
	{
		s_savegame.arrowdwn.generic.flags &= ~QMF_HIDDEN;
		s_savegame.arrowdwn.generic.flags |= QMF_HIGHLIGHT_IF_FOCUS;
	}
	else
	{
		s_savegame.arrowdwn.generic.flags	= QMF_HIDDEN;
	}

	// If no more files above, turn off up arrow
	if (s_savegame.startISaveFile != 0)
	{
		s_savegame.arrowup.generic.flags &= ~QMF_HIDDEN;
		s_savegame.arrowup.generic.flags	|= QMF_HIGHLIGHT_IF_FOCUS;
	}
	else
	{
		s_savegame.arrowup.generic.flags	= QMF_HIDDEN;
	}
}

/*
=================
SaveGame_UpdateWindow
=================
*/
static void SaveGame_UpdateWindow(void)
{
	int saveFileI,i,result;

	// Get comments and file names
	ReadSaveDirectory();

	saveFileI = s_savegame.startISaveFile;


	// Set colors for the lines
	for (i=0;i<MAX_FILEDISP;i++)
	{
		((menubitmap_s *)g_savedline[i])->textcolor		= CT_DKGOLD1;
		((menubitmap_s *)g_savedline[i])->textcolor2	= CT_LTGOLD1;
		((menubitmap_s *)g_savedline[i])->color			= CT_DKPURPLE1;
		((menubitmap_s *)g_savedline[i])->color2		= CT_LTPURPLE1;

		((menubitmap_s *)g_savedate[i])->textcolor		= CT_DKGOLD1;
		((menubitmap_s *)g_savedate[i])->textcolor2		= CT_LTGOLD1;

		((menubitmap_s *)g_savetime[i])->textcolor		= CT_DKGOLD1;
		((menubitmap_s *)g_savetime[i])->textcolor2		= CT_LTGOLD1;
	}

	// Place file comments on fields 
	for (i=0;i<MAX_FILEDISP;i++)
	{
		if (g_savedline[i])
		{
			if (s_savedata[saveFileI].currentSaveFileComments[0])
			{
				result = ui.SG_ValidateForLoadSaveScreen(s_savedata[saveFileI].currentSaveFileNames);

				if (result == qfalse) 
				{
					i--;			//	Save this line for the next valid file
					saveFileI++;	//	Advance to next file
					continue;
				}

				((menubitmap_s *)g_savedline[i])->textPtr = s_savedata[saveFileI].currentSaveFileComments;
				((menubitmap_s *)g_savedline[i])->generic.flags = QMF_HIGHLIGHT_IF_FOCUS;

				// Is this the chosen file?
				if (i==(s_savegame.currentLine - s_savegame.startISaveFile))
				{
					((menubitmap_s *)g_savedline[i])->textcolor		= CT_YELLOW;
					((menubitmap_s *)g_savedline[i])->textcolor2	= CT_WHITE;
					((menubitmap_s *)g_savedline[i])->color			= CT_YELLOW;
					((menubitmap_s *)g_savedline[i])->color2		= CT_WHITE;

					((menubitmap_s *)g_savedate[i])->textcolor		= CT_YELLOW;
					((menubitmap_s *)g_savedate[i])->textcolor2		= CT_YELLOW;

					((menubitmap_s *)g_savetime[i])->textcolor		= CT_YELLOW;
					((menubitmap_s *)g_savetime[i])->textcolor2		= CT_YELLOW;

					s_savegame.mapName = s_savedata[saveFileI].currentSaveFileDateTime + 9 + 11;

				}

				((menubitmap_s *)g_savedate[i])->textPtr = s_savedata[saveFileI].currentSaveFileDateTime;
				((menubitmap_s *)g_savetime[i])->textPtr = s_savedata[saveFileI].currentSaveFileDateTime + 9;

			}
			else	//	No file for this line so set it inactive
			{
				((menubitmap_s *)g_savedline[i])->generic.flags = QMF_INACTIVE;
				((menubitmap_s *)g_savedline[i])->textPtr = NULL;
				((menubitmap_s *)g_savedate[i])->textPtr = NULL;
				((menubitmap_s *)g_savetime[i])->textPtr = NULL;
			}

			saveFileI++;	//	Advance to next file
		}
	}

	SaveGame_UpdateWindowArrows();

}

/*
=================
UI_DrawThumbNail
=================
*/
void UI_DrawThumbNail( float x, float y, float w, float h, byte *pic ) {

	UI_AdjustFrom640( &x, &y, &w, &h );
	ui.DrawStretchRaw( x, y, w, h, SG_SCR_WIDTH, SG_SCR_HEIGHT, pic, 1.0 );
}

/*
=================
UI_DrawThumbNailFrame
=================
*/
void UI_DrawThumbNailFrame(void) 
{

	ui.R_SetColor( colorTable[CT_DKORANGE]);

	// Bracket Graphic
	// Left Side
	UI_DrawHandlePic(SGPIC_X-SGPIC_BRD-8, SGPIC_Y-4-SGPIC_H-4-4, 16, 16, uis.graphicBracket1CornerLU);	//LU
	UI_DrawHandlePic(SGPIC_X-SGPIC_BRD-8, SGPIC_Y-4, 16, -16, uis.graphicBracket1CornerLU);				//LD
	UI_DrawHandlePic(SGPIC_X-SGPIC_BRD-8, SGPIC_Y-SGPIC_H,  8, SGPIC_H-3, uis.whiteShader);				// Left Side

	// Right Side
	UI_DrawHandlePic(SGPIC_X+SGPIC_W+SGPIC_BRD-8, SGPIC_Y-4-SGPIC_H-4-4, -16, 16, uis.graphicBracket1CornerLU);	//RU
	UI_DrawHandlePic(SGPIC_X+SGPIC_W+SGPIC_BRD-8, SGPIC_Y-4, -16, -16, uis.graphicBracket1CornerLU);			//RD
	UI_DrawHandlePic(SGPIC_X+SGPIC_W+SGPIC_BRD, SGPIC_Y-SGPIC_H,  8, SGPIC_H-3, uis.whiteShader);				// Right Side

	// Drawn mapname
	if (s_savegame.mapName) 
	{
		UI_DrawProportionalString(  516,  SGPIC_Y+18, s_savegame.mapName,UI_CENTER|UI_TINYFONT, colorTable[CT_LTBLUE1]);
	}
}

/*
=================
UI_DrawCurrentSavesFrame
=================
*/
void UI_DrawCurrentSavesFrame(void) 
{
	// Current save directory box
	ui.R_SetColor( colorTable[CT_DKPURPLE2]);
	UI_DrawHandlePic(90,168,  308, 18, uis.whiteShader);	// Top of current save directory box
	UI_DrawProportionalString(  97, 169, menu_normal_text[MNT_CURRENTSAVEGAME],UI_SMALLFONT, colorTable[CT_BLACK]);

	// Right side of box
	ui.R_SetColor( colorTable[CT_DKPURPLE2]);
	UI_DrawHandlePic(383, 168,  32,	32, s_savegame.corner_ur_16_18);	// Upper corner of directory box
	UI_DrawHandlePic(384, 421,  32,	16, s_savegame.corner_lr_8_16);		// Lower corner of directory box

	UI_DrawHandlePic(387,186,  16,   6, uis.whiteShader);	
	UI_DrawHandlePic(387,195,  16,  17, uis.whiteShader);
	UI_DrawHandlePic(387,215,  16, 180, uis.whiteShader);
	UI_DrawHandlePic(387,398,  16,  17, uis.whiteShader);
	UI_DrawHandlePic(387,419,  16,   6, uis.whiteShader);

	UI_DrawHandlePic(90, 424,  308, 8, uis.whiteShader);	// Bottom of current save directory box
}

/*
=================
UI_DrawCurrentFileFrame
=================
*/
void UI_DrawCurrentFileFrame(void) 
{
	// Column along left side of frame
	ui.R_SetColor( colorTable[CT_DKPURPLE2]);
	UI_DrawHandlePic(30,203,  47, 130, uis.whiteShader);	// Top left column square on bottom 2 3rds
	UI_DrawHandlePic(30,336,  47, 16, uis.whiteShader);	// Middle left column square on bottom 2 3rds
	UI_DrawHandlePic(30,355,  47, 34, uis.whiteShader);	// Bottom left column square on bottom 2 3rds

	// Current game box
	UI_DrawHandlePic(130,64,  88, 24, uis.whiteShader);	// Left Side of current game line box 3
	UI_DrawHandlePic(218,64,  353, 3, uis.whiteShader);	// Top of current game line
	UI_DrawHandlePic(218,85,  353, 3, uis.whiteShader);	// Bottom of current game line
	UI_DrawHandlePic(549,64,  10, 24, uis.whiteShader);	// Right side of current game line

	UI_DrawHandlePic(113, 64,  32,	32, s_savegame.currentGameTopLeft);		// Upper left corner of current game box
	UI_DrawHandlePic(113, 95,  32,	32, s_savegame.currentGameBotLeft);		// Bottom left corner of current game box
	UI_DrawHandlePic(559, 64,  32,	32, s_savegame.currentGameTopRight);	// Upper right corner of current game box
	UI_DrawHandlePic(560, 95,  32,	32, s_savegame.currentGameBotRight);	// Bottom right corner of current game box

	UI_DrawHandlePic(566,91,  18, 6, uis.whiteShader);	// Right side of current game line
	UI_DrawHandlePic(113,90,  18, 6, uis.whiteShader);	// Left side of current game line

	UI_DrawHandlePic(273,101,  50, 18, uis.whiteShader);	// Left side of line between buttons
	UI_DrawHandlePic(326,101,  45, 18, uis.whiteShader);	// Right side of line between buttons
	UI_DrawHandlePic(374,101,  50, 18, uis.whiteShader);	// Right side of line between buttons
}

/*
=================
M_SaveGame_Graphics
=================
*/

static void M_SaveGame_Graphics (void)
{
	UI_MenuFrame(&s_savegame.menu);

	UI_DrawCurrentFileFrame();
	UI_DrawProportionalString(  580,  67, "19-214",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);

	// Current save directory box
	UI_DrawCurrentSavesFrame();

	// Game pic
	UI_DrawThumbNail( SGPIC_X, SGPIC_Y, SGPIC_W, -SGPIC_H, (byte*) &screenShotBuf );

	UI_DrawThumbNailFrame();

	UI_DrawProportionalString(  74,  66, "990-7",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  84, "5233",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  188, "35621",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  206, "78-9099",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  395, "1121",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);

	// Update file window
	if ((!gameWindow) && (gameWindowTime < uis.realtime))
	{
		gameWindow = qtrue;
		SaveGame_UpdateWindow();	// Update file window
	}

	if (s_savegame.awaitingSave == qtrue)
	{
		UI_DrawProportionalString(  517,  350, menu_normal_text[MNT_SAVE_WARNING1],UI_SMALLFONT | UI_CENTER, colorTable[CT_RED]);
		UI_DrawProportionalString(  517,  370, menu_normal_text[MNT_SAVE_WARNING2],UI_SMALLFONT | UI_CENTER, colorTable[CT_RED]);
		UI_DrawProportionalString(  517,  390, menu_normal_text[MNT_SAVE_WARNING3],UI_SMALLFONT | UI_BLINK | UI_CENTER, colorTable[CT_RED]);
	}
}

/*
=================
M_SaveGame_MenuDraw
=================
*/
static void M_SaveGame_MenuDraw (void)
{
	M_SaveGame_Graphics();

	Menu_Draw( &s_savegame.menu );
}

static void SaveGame_DescriptionEvent( void* ptr, int notification )
{
	if (s_savegame.currentLine != CURRENTLINE_NONE)
	{
//		Field_Clear(&s_file_desc_field.field);	// Blank out comment 
		s_savegame.deletegame.generic.flags	= QMF_GRAYED;	// Turn off delete button
//		s_savegame.currentLine = CURRENTLINE_NONE;	// Release chosen line.

		memset(s_savedata[s_savegame.currentLine].currentSaveFileNames,0,sizeof(s_savedata[s_savegame.currentLine].currentSaveFileNames));

		SaveGame_UpdateWindow();	// Update file window
	}
}



/*
=================
SaveGameFile
=================
*/
void SaveGameFile(char *fileName)
{
	char	commandString[20];

	// Save description line
	ui.SG_StoreSaveGameComment(s_file_desc_field.field.buffer);

	Com_sprintf( commandString,  sizeof(commandString), "save %s\n", fileName);
	ui.Cmd_ExecuteText( EXEC_APPEND, commandString);

	s_savegame.deletegame.generic.flags	= QMF_GRAYED;	// Turn off delete button
	s_savegame.currentLine = CURRENTLINE_NONE;			//	Show no line is chosen
	s_savegame.currentId = 0;

	Field_Clear(&s_file_desc_field.field);	// Blank out comment 

	gameWindow = qfalse;
	gameWindowTime = uis.realtime + 1;
}


/*
=================
SaveGame_MenuEvent
=================
*/
static void SaveGame_MenuEvent( void* ptr, int notification )
{
	menuframework_s*	m;
	char fileName[MAX_SAVELOADNAME],commandString[20];
	int index;

	if (notification != QM_ACTIVATED)
		return;

	m = ((menucommon_s*)ptr)->parent;

	switch (((menucommon_s*)ptr)->id)
	{
		case ID_MAINMENU:
			UI_PopMenu();	// Can do this whether to get back to ingame menu or back to game
			break;
		case ID_SAVEDCOMMENT1:
		case ID_SAVEDCOMMENT2:
		case ID_SAVEDCOMMENT3:
		case ID_SAVEDCOMMENT4:
		case ID_SAVEDCOMMENT5:
		case ID_SAVEDCOMMENT6:
		case ID_SAVEDCOMMENT7:
		case ID_SAVEDCOMMENT8:
		case ID_SAVEDCOMMENT9:
		case ID_SAVEDCOMMENT10:
		case ID_SAVEDCOMMENT11:
		case ID_SAVEDCOMMENT12:

			//	Copy description to screen
			index = ((menucommon_s*)ptr)->id - ID_SAVEDCOMMENT1;
			if (((menubitmap_s *)g_savedline[index])->textPtr)
			{
				s_savegame.currentLine = s_savegame.startISaveFile + index;

				Field_Clear(&s_file_desc_field.field);	// Blank out comment to reset cursor
				strcpy(s_file_desc_field.field.buffer,((menubitmap_s *)g_savedline[index])->textPtr);

				// Move file picture on screen
				if (!ui.SG_GetSaveImage(s_savedata[s_savegame.currentLine].currentSaveFileNames, &screenShotBuf))
				{
					memset( screenShotBuf,0,(SG_SCR_WIDTH * SG_SCR_HEIGHT * 4)); 
				}

				if (!Q_stricmp(s_savedata[s_savegame.currentLine].currentSaveFileNames, AUTOSAVENAME )) 
				{
					s_savegame.deletegame.generic.flags	= QMF_GRAYED;	// Turn off delete button
				}
				else
				{
					//	Turn on DELETE button
					s_savegame.deletegame.generic.flags	= QMF_HIGHLIGHT_IF_FOCUS;
				}
			}

			s_savegame.awaitingSave = qfalse;
			s_savegame.yes.generic.flags	= QMF_HIDDEN;
			s_savegame.no.generic.flags		= QMF_HIDDEN;

			SaveGame_UpdateWindow();	// Update file window

			break;

		case ID_SAVEGAMEDATA:

			// Create a new save game
			if (s_savegame.currentLine == CURRENTLINE_NONE)
			{
				CreateNextSaveName(fileName);	// Get a name to save to
			}
			else	// Overwrite a current save game? Ask first.
			{
				s_savegame.yes.generic.flags	= QMF_HIGHLIGHT_IF_FOCUS;
				s_savegame.no.generic.flags		= QMF_HIGHLIGHT_IF_FOCUS;

				strcpy(fileName,s_savedata[s_savegame.currentLine].currentSaveFileNames);
				s_savegame.awaitingSave = qtrue;
				s_savegame.deletegame.generic.flags	= QMF_GRAYED;	// Turn off delete button
				break;
			}

			SaveGameFile(fileName);

			break;

		case ID_LOADGAMEDATA:

			if ((s_savedata[s_savegame.currentLine].currentSaveFileNames) && (*s_file_desc_field.field.buffer))
			{
				UI_ForceMenuOff ();
				Com_sprintf( commandString, sizeof(commandString), "load %s\n", s_savedata[s_savegame.currentLine].currentSaveFileNames);
				ui.Cmd_ExecuteText( EXEC_APPEND, commandString);
			}
			break;

		case ID_DELETEGAMEDATA:

			if (s_savegame.currentLine == CURRENTLINE_NONE)	// No line was chosen
			{
				break;
			}

			if (!*s_file_desc_field.field.buffer)	// Blank comment 
			{
				break;
			}
				
			ui.Printf( va("%s\n",menu_normal_text[MNT_ATTEMPTINGWIPE]));

			Com_sprintf( commandString,  sizeof(commandString), "wipe %s\n", s_savedata[s_savegame.currentLine].currentSaveFileNames);
			ui.Cmd_ExecuteText( EXEC_NOW, commandString);

			Field_Clear(&s_file_desc_field.field);	// Blank out comment 


			s_savegame.deletegame.generic.flags	= QMF_GRAYED;	// Turn off delete button
			s_savegame.currentLine = CURRENTLINE_NONE;	//	Show no line is chosen
			SaveGame_UpdateWindow();					// Update file window

			break;

		case ID_ARROWUP:

			if (s_savegame.saveFileCnt < MAX_FILEDISP)
			{
				break;
			}

			s_savegame.startISaveFile--;
			if (s_savegame.startISaveFile<0)
			{
				s_savegame.startISaveFile = 0;
			}
			SaveGame_UpdateWindow();	// Update file window
	
			break;

		case ID_ARROWDWN:

			if (s_savegame.saveFileCnt < MAX_FILEDISP)
			{
				break;
			}

			s_savegame.startISaveFile++;
			if (s_savegame.startISaveFile>= s_savegame.saveFileCnt)
			{
				s_savegame.startISaveFile = (s_savegame.saveFileCnt-1);
			}
			SaveGame_UpdateWindow();	// Update file window

			break;

		case ID_YES:
				SaveGameFile(s_savedata[s_savegame.currentLine].currentSaveFileNames);	

				s_savegame.yes.generic.flags	= QMF_HIDDEN;
				s_savegame.no.generic.flags		= QMF_HIDDEN;
				s_savegame.awaitingSave			= qfalse;

			break;

		case ID_NO:
				s_savegame.yes.generic.flags	= QMF_HIDDEN;
				s_savegame.no.generic.flags		= QMF_HIDDEN;
				s_savegame.awaitingSave			= qfalse;
				s_savegame.deletegame.generic.flags	= QMF_HIGHLIGHT_IF_FOCUS;	// Turn off delete button
			break;

		case ID_AUTOSAVE:
				UI_ForceMenuOff ();
				Com_sprintf( commandString, sizeof(commandString), "load %s\n", AUTOSAVENAME);
				ui.Cmd_ExecuteText( EXEC_APPEND, commandString);
			break;
	}
}


/*
=================
SaveGame_LineEvent
=================
*/
static void SaveGame_LineEvent (void* ptr)
{
	int index;

	if (s_savegame.currentLine != CURRENTLINE_NONE)	// A line was chosen
	{
		return;
	}

	// Changed lines
	if (s_savegame.currentId != ((menucommon_s*)ptr)->id)
	{
		// Get new file's screen shot
		s_savegame.currentId =  ((menucommon_s*)ptr)->id;

		index =  s_savegame.startISaveFile;
		index += ((menucommon_s*)ptr)->id - ID_SAVEDCOMMENT1;
		if (!ui.SG_GetSaveImage(s_savedata[index].currentSaveFileNames, &screenShotBuf))
		{
			memset( screenShotBuf,0,(SG_SCR_WIDTH * SG_SCR_HEIGHT * 4)); 
		}

		s_savegame.mapName = s_savedata[index].currentSaveFileDateTime + 9 + 11;
	}
}

/*
=================
M_SaveGame_MenuKey
=================
*/
static sfxHandle_t M_SaveGame_MenuKey( int key )
{
	return Menu_DefaultKey( &s_savegame.menu, key );
}


/*
===============
UI_SaveGameMenu_Cache
===============
*/
void UI_SaveGameMenu_Cache( void ) 
{
	s_savegame.currentGameTopLeft = ui.R_RegisterShaderNoMip("menu/common/lg_topleft.tga");
	s_savegame.currentGameBotLeft = ui.R_RegisterShaderNoMip("menu/common/lg_lowleft.tga");
	s_savegame.currentGameTopRight = ui.R_RegisterShaderNoMip("menu/common/lg_topright.tga");
	s_savegame.currentGameBotRight = ui.R_RegisterShaderNoMip("menu/common/lg_lowright.tga");

	s_savegame.corner_ur_16_18 = ui.R_RegisterShaderNoMip("menu/common/corner_ur_16_18.tga");
	s_savegame.corner_lr_8_16 = ui.R_RegisterShaderNoMip("menu/common/corner_lr_8_16.tga");

	ui.R_RegisterShaderNoMip("menu/common/underline.tga");
	ui.R_RegisterShaderNoMip("menu/common/arrow_up_16.tga");
	ui.R_RegisterShaderNoMip("menu/common/arrow_dn_16.tga");

}


/*
===============
SaveGameMenu_Init
===============
*/
static void SaveGameMenu_Init(void) 
{
	int i,y,x;

	UI_SaveGameMenu_Cache();

	s_savegame.menu.nitems						= 0;
	s_savegame.menu.wrapAround					= qtrue;
	s_savegame.menu.draw						= M_SaveGame_MenuDraw;
	s_savegame.menu.key							= M_SaveGame_MenuKey;
	s_savegame.menu.fullscreen					= qtrue;
	s_savegame.menu.wrapAround					= qfalse;
	s_savegame.menu.titleX						= MENU_TITLE_X;
	s_savegame.menu.titleY						= MENU_TITLE_Y;
	s_savegame.menu.titleI						= MNT_SAVEGAMEMENU_TITLE;
	s_savegame.menu.descX						= MENU_DESC_X;
	s_savegame.menu.descY						= MENU_DESC_Y;
	s_savegame.menu.footNoteEnum				= MNT_SAVEGAME;

	s_savegame_mainmenu.generic.type			= MTYPE_BITMAP;      
	s_savegame_mainmenu.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_savegame_mainmenu.generic.x				= 482;
	s_savegame_mainmenu.generic.y				= 136;
	s_savegame_mainmenu.generic.name			= GRAPHIC_BUTTONRIGHT;
	s_savegame_mainmenu.generic.id				= ID_MAINMENU;
	s_savegame_mainmenu.generic.callback		= SaveGame_MenuEvent;
	s_savegame_mainmenu.width					= MENU_BUTTON_MED_WIDTH;
	s_savegame_mainmenu.height					= MENU_BUTTON_MED_HEIGHT;
	s_savegame_mainmenu.color					= CT_DKPURPLE1;
	s_savegame_mainmenu.color2					= CT_LTPURPLE1;
	s_savegame_mainmenu.textX					= MENU_BUTTON_TEXT_X;
	s_savegame_mainmenu.textY					= MENU_BUTTON_TEXT_Y;
	if (!s_savegame.savegameFromFlag)	// Did not come from a menu (Main or Ingame)
	{
		s_savegame_mainmenu.textEnum				= MBT_RETURNTOGAME;
	}
	else
	{
		s_savegame_mainmenu.textEnum				= MBT_INGAMEMENU;
	}
	s_savegame_mainmenu.textcolor				= CT_BLACK;
	s_savegame_mainmenu.textcolor2				= CT_WHITE;

	s_file_desc_field.generic.type				= MTYPE_FIELD;
	s_file_desc_field.generic.callback			= SaveGame_DescriptionEvent;
	s_file_desc_field.generic.x					= 210;
	s_file_desc_field.generic.y					= 68;
	s_file_desc_field.field.widthInChars		= 40;
	s_file_desc_field.field.maxchars			= 40;
	s_file_desc_field.field.style				= UI_TINYFONT;
	s_file_desc_field.field.textEnum			= MNT_DESCRIPTION;
	s_file_desc_field.field.textcolor			= CT_LTGOLD1;
	s_file_desc_field.field.textcolor2			= CT_LTGOLD1;

	s_savegamedata.generic.type					= MTYPE_BITMAP;      
	s_savegamedata.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS;
	s_savegamedata.generic.x					= 140;
	s_savegamedata.generic.y					= 101;
	s_savegamedata.generic.name					= "menu/common/square.tga";
	s_savegamedata.generic.id					= ID_SAVEGAMEDATA;
	s_savegamedata.generic.callback				= SaveGame_MenuEvent; 
	s_savegamedata.width						= MENU_BUTTON_MED_WIDTH;
	s_savegamedata.height						= MENU_BUTTON_MED_HEIGHT;
	s_savegamedata.color						= CT_DKPURPLE1;
	s_savegamedata.color2						= CT_LTPURPLE1;
	s_savegamedata.textX						= MENU_BUTTON_TEXT_X;
	s_savegamedata.textY						= MENU_BUTTON_TEXT_Y;
	s_savegamedata.textEnum						= MBT_SAVEGAMEDATA;
	s_savegamedata.textcolor					= CT_BLACK;
	s_savegamedata.textcolor2					= CT_WHITE;

	s_savegame.deletegame.generic.type			= MTYPE_BITMAP;      
	s_savegame.deletegame.generic.flags			= QMF_GRAYED;
	s_savegame.deletegame.generic.x				= 427;
	s_savegame.deletegame.generic.y				= 101;
	s_savegame.deletegame.generic.name			= "menu/common/square.tga";
	s_savegame.deletegame.generic.id			= ID_DELETEGAMEDATA;
	s_savegame.deletegame.generic.callback		= SaveGame_MenuEvent; 
	s_savegame.deletegame.width					= MENU_BUTTON_MED_WIDTH;
	s_savegame.deletegame.height				= MENU_BUTTON_MED_HEIGHT;
	s_savegame.deletegame.color					= CT_DKPURPLE1;
	s_savegame.deletegame.color2				= CT_LTPURPLE1;
	s_savegame.deletegame.textX					= MENU_BUTTON_TEXT_X;
	s_savegame.deletegame.textY					= MENU_BUTTON_TEXT_Y;
	s_savegame.deletegame.textEnum				= MBT_DELETEGAMEDATA;
	s_savegame.deletegame.textcolor				= CT_BLACK;
	s_savegame.deletegame.textcolor2			= CT_WHITE;

	s_savegame.arrowup.generic.type				= MTYPE_BITMAP;      
	s_savegame.arrowup.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_savegame.arrowup.generic.x				= 387;
	s_savegame.arrowup.generic.y				= 197;
	s_savegame.arrowup.generic.name				= "menu/common/arrow_up_16.tga";
	s_savegame.arrowup.generic.id				= ID_ARROWUP;
	s_savegame.arrowup.generic.callback			= SaveGame_MenuEvent; 
	s_savegame.arrowup.width					= 16;
	s_savegame.arrowup.height					= 16;
	s_savegame.arrowup.color					= CT_DKBLUE1;
	s_savegame.arrowup.color2					= CT_LTBLUE1;
	s_savegame.arrowup.textX					= 0;
	s_savegame.arrowup.textY					= 0;
	s_savegame.arrowup.textEnum					= MBT_NONE;
	s_savegame.arrowup.textcolor				= CT_BLACK;
	s_savegame.arrowup.textcolor2				= CT_WHITE;

	s_savegame.arrowdwn.generic.type			= MTYPE_BITMAP;      
	s_savegame.arrowdwn.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_savegame.arrowdwn.generic.x				= 387;
	s_savegame.arrowdwn.generic.y				= 400;
	s_savegame.arrowdwn.generic.name			= "menu/common/arrow_dn_16.tga";
	s_savegame.arrowdwn.generic.id				= ID_ARROWDWN;
	s_savegame.arrowdwn.generic.callback		= SaveGame_MenuEvent; 
	s_savegame.arrowdwn.width					= 16;
	s_savegame.arrowdwn.height					= 16;
	s_savegame.arrowdwn.color					= CT_DKBLUE1;
	s_savegame.arrowdwn.color2					= CT_LTBLUE1;
	s_savegame.arrowdwn.textX					= 0;
	s_savegame.arrowdwn.textY					= 0;
	s_savegame.arrowdwn.textEnum				= MBT_NONE;
	s_savegame.arrowdwn.textcolor				= CT_BLACK;
	s_savegame.arrowdwn.textcolor2				= CT_WHITE;

	s_savegame.yes.generic.type				= MTYPE_BITMAP;      
	s_savegame.yes.generic.flags			= QMF_HIDDEN;
	s_savegame.yes.generic.x				= 427;
	s_savegame.yes.generic.y				= 412;
	s_savegame.yes.generic.name				= "menu/common/square.tga";
	s_savegame.yes.generic.id				= ID_YES;
	s_savegame.yes.generic.callback			= SaveGame_MenuEvent; 
	s_savegame.yes.width					= 60;
	s_savegame.yes.height					= 18;
	s_savegame.yes.color					= CT_DKGOLD1;
	s_savegame.yes.color2					= CT_LTGOLD1;
	s_savegame.yes.textX					= MENU_BUTTON_TEXT_X;
	s_savegame.yes.textY					= MENU_BUTTON_TEXT_Y;
	s_savegame.yes.textEnum					= MBT_SAVEYES;
	s_savegame.yes.textcolor				= CT_BLACK;
	s_savegame.yes.textcolor2				= CT_WHITE;

	s_savegame.no.generic.type				= MTYPE_BITMAP;      
	s_savegame.no.generic.flags				= QMF_HIDDEN;
	s_savegame.no.generic.x					= 548;
	s_savegame.no.generic.y					= 412;
	s_savegame.no.generic.name				= "menu/common/square.tga";
	s_savegame.no.generic.id				= ID_NO;
	s_savegame.no.generic.callback			= SaveGame_MenuEvent; 
	s_savegame.no.width						= 60;
	s_savegame.no.height					= 18;
	s_savegame.no.color						= CT_DKGOLD1;
	s_savegame.no.color2					= CT_LTGOLD1;
	s_savegame.no.textX						= MENU_BUTTON_TEXT_X;
	s_savegame.no.textY						= MENU_BUTTON_TEXT_Y;
	s_savegame.no.textEnum					= MBT_SAVENO;
	s_savegame.no.textcolor					= CT_BLACK;
	s_savegame.no.textcolor2				= CT_WHITE;

	Menu_AddItem( &s_savegame.menu, ( void * )&s_savegame_mainmenu );
	Menu_AddItem( &s_savegame.menu, ( void * )&s_savegamedata );
	Menu_AddItem( &s_savegame.menu, ( void * )&s_file_desc_field );
	Menu_AddItem( &s_savegame.menu, ( void * )&s_savegamedata );
	Menu_AddItem( &s_savegame.menu, ( void * )&s_savegame.deletegame );
	Menu_AddItem( &s_savegame.menu, ( void * )&s_savegame.arrowup );
	Menu_AddItem( &s_savegame.menu, ( void * )&s_savegame.arrowdwn );
	Menu_AddItem( &s_savegame.menu, ( void * )&s_savegame.yes );
	Menu_AddItem( &s_savegame.menu, ( void * )&s_savegame.no );

	x = 90;
	y = 200;
	for (i=0;i<99;i++)
	{
		if (!g_savedline[i])
		{
			break;
		}

		((menubitmap_s *)g_savedline[i])->generic.type			= MTYPE_BITMAP;      
		((menubitmap_s *)g_savedline[i])->generic.flags			= QMF_INACTIVE;
		((menubitmap_s *)g_savedline[i])->generic.x				= x;
		((menubitmap_s *)g_savedline[i])->generic.y				= y;
		((menubitmap_s *)g_savedline[i])->generic.name			= "menu/common/underline.tga";
		((menubitmap_s *)g_savedline[i])->generic.callback		= SaveGame_MenuEvent;
		((menubitmap_s *)g_savedline[i])->generic.statusbarfunc	= SaveGame_LineEvent;
		((menubitmap_s *)g_savedline[i])->generic.id			= ID_SAVEDCOMMENT1+i;
		((menubitmap_s *)g_savedline[i])->width					= 200;
		((menubitmap_s *)g_savedline[i])->height				= 16;
		((menubitmap_s *)g_savedline[i])->color					= CT_DKPURPLE1;
		((menubitmap_s *)g_savedline[i])->color2				= CT_LTPURPLE1;
		((menubitmap_s *)g_savedline[i])->textPtr				= NULL;
		((menubitmap_s *)g_savedline[i])->textX					= 4;
		((menubitmap_s *)g_savedline[i])->textY					= 1;
		((menubitmap_s *)g_savedline[i])->textcolor				= CT_DKGOLD1;
		((menubitmap_s *)g_savedline[i])->textcolor2			= CT_LTGOLD1;
		((menubitmap_s *)g_savedline[i])->textStyle				= UI_TINYFONT;
		((menubitmap_s *)g_savedline[i])->textEnum				= MBT_SAVEGAMELINE;

		Menu_AddItem( &s_savegame.menu, ( void * )g_savedline[i]);
		y += 18;
	}


	x = 292;
	y = 200;
	for (i=0;i<99;i++)
	{
		if (!g_savedate[i])
		{
			break;
		}

		((menubitmap_s *)g_savedate[i])->generic.type			= MTYPE_BITMAP;      
		((menubitmap_s *)g_savedate[i])->generic.flags			= QMF_INACTIVE;
		((menubitmap_s *)g_savedate[i])->generic.x				= x;
		((menubitmap_s *)g_savedate[i])->generic.y				= y;
		((menubitmap_s *)g_savedate[i])->color					= CT_DKPURPLE1;
		((menubitmap_s *)g_savedate[i])->color2					= CT_LTPURPLE1;
		((menubitmap_s *)g_savedate[i])->textPtr				= NULL;
		((menubitmap_s *)g_savedate[i])->textX					= 4;
		((menubitmap_s *)g_savedate[i])->textY					= 1;
		((menubitmap_s *)g_savedate[i])->textcolor				= CT_DKGOLD1;
		((menubitmap_s *)g_savedate[i])->textcolor2				= CT_LTGOLD1;
		((menubitmap_s *)g_savedate[i])->textStyle				= UI_TINYFONT;
		Menu_AddItem( &s_savegame.menu, ( void * )g_savedate[i]);

		((menubitmap_s *)g_savetime[i])->generic.type			= MTYPE_BITMAP;      
		((menubitmap_s *)g_savetime[i])->generic.flags			= QMF_INACTIVE;
		((menubitmap_s *)g_savetime[i])->generic.x				= x + 36;
		((menubitmap_s *)g_savetime[i])->generic.y				= y;
		((menubitmap_s *)g_savetime[i])->color					= CT_DKPURPLE1;
		((menubitmap_s *)g_savetime[i])->color2					= CT_LTPURPLE1;
		((menubitmap_s *)g_savetime[i])->textPtr				= NULL;
		((menubitmap_s *)g_savetime[i])->textX					= 4;
		((menubitmap_s *)g_savetime[i])->textY					= 1;
		((menubitmap_s *)g_savetime[i])->textcolor				= CT_DKGOLD1;
		((menubitmap_s *)g_savetime[i])->textcolor2				= CT_LTGOLD1;
		((menubitmap_s *)g_savetime[i])->textStyle				= UI_TINYFONT;
		Menu_AddItem( &s_savegame.menu, ( void * )g_savetime[i]);

		y += 18;
	}


	// Get savegame file names
	SaveGame_UpdateWindow();

}

/*
===============
UI_SaveGameMenu
===============
*/
void UI_SaveGameMenu(int savegameFromFlag) 
{	
	s_savegame.savegameFromFlag = savegameFromFlag;

	if (!s_savegame.menu.initialized)
	{
		SaveGameMenu_Init(); 
	}

	if (!s_savegame.savegameFromFlag)	// Did not come from a menu (Main or Ingame)
	{
		Mouse_Show();
	}

	s_file_desc_field.generic.flags	= 0;

	s_savegame.currentLine = CURRENTLINE_NONE;
	s_savegame.currentId = 0;
	s_savegame.awaitingSave = qfalse;
	s_savegame.startISaveFile = 0;
	SaveGame_UpdateWindow();	// Update file window

	UI_PushMenu( &s_savegame.menu);
}

//===================================================================
//
// Load Game Menu
//
//===================================================================

// menu action identifiers
#define	ID_MAINMENU					103


static int loadgameFromFlag;

static struct 
{
	menuframework_s menu;
	menubitmap_s	mainmenu;
	menubitmap_s	arrowdwn;
	menubitmap_s	arrowup;
	menubitmap_s	load;
} s_loadgame;



/*
=================
LoadGame_Blinkies
=================
*/
static void LoadGame_Blinkies(void)
{
	int i;

	// Move bottom blip
	if ( loadgamemenu_graphics[LGG_BLIP1].timer < uis.realtime )
	{
		loadgamemenu_graphics[LGG_BLIP1].x += 3;
		loadgamemenu_graphics[LGG_BLIP2].x = loadgamemenu_graphics[LGG_BLIP1].x;

		if (loadgamemenu_graphics[LGG_BLIP1].x > loadgamemenu_graphics[LGG_BLIP1].max)
		{
			loadgamemenu_graphics[LGG_BLIP1].x = loadgamemenu_graphics[LGG_BLIP1].min;
			loadgamemenu_graphics[LGG_BLIP2].x = loadgamemenu_graphics[LGG_BLIP2].min;
		}

		loadgamemenu_graphics[LGG_BLIP1].timer = uis.realtime + 75;
	}

	// Make right hand numbers change
	if (loadgamemenu_graphics[LGG_COL1_NUM1].timer < uis.realtime)
	{
		loadgamemenu_graphics[LGG_COL1_NUM1].timer = uis.realtime + 3000;

		for (i=0;i<3;i++)
		{
			loadgamemenu_graphics[i + LGG_COL1_NUM1].target = UI_RandomNumbers(9);
			loadgamemenu_graphics[i + LGG_COL1_NUM1].color = CT_DKGOLD1;

			loadgamemenu_graphics[i + LGG_COL2_NUM1].target = UI_RandomNumbers(9);
			loadgamemenu_graphics[i + LGG_COL2_NUM1].color = CT_DKGOLD1;

			loadgamemenu_graphics[i + LGG_COL3_NUM1].target = UI_RandomNumbers(7);
			loadgamemenu_graphics[i + LGG_COL3_NUM1].color = CT_DKGOLD1;

			loadgamemenu_graphics[i + LGG_COL4_NUM1].target = UI_RandomNumbers(4);
			loadgamemenu_graphics[i + LGG_COL4_NUM1].color = CT_DKGOLD1;
		}
	}
}

/*
=================
M_LoadGame_Graphics
=================
*/
static void M_LoadGame_Graphics (void)
{
//	if (!loadgameFromFlag)
//	{
//		ui.R_SetColor( colorTable[CT_LTBROWN1]);
//		UI_DrawHandlePic( 482, 136,  MENU_BUTTON_MED_WIDTH - 22, MENU_BUTTON_MED_HEIGHT, uis.whiteShader);
//		UI_DrawHandlePic( 460 + MENU_BUTTON_MED_WIDTH - 4, 136,  -19,  MENU_BUTTON_MED_HEIGHT, uis.graphicButtonLeftEnd);	//right end 
//	}

	UI_DrawCurrentFileFrame();
	UI_DrawProportionalString(  580,  67, "19-212",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);

	// Current save directory box
	UI_DrawCurrentSavesFrame();

	UI_MenuFrame(&s_loadgame.menu);

	// Game pic
	UI_DrawThumbNail( SGPIC_X, SGPIC_Y, SGPIC_W, -SGPIC_H, (byte*) &screenShotBuf );

	UI_DrawThumbNailFrame();

	UI_DrawProportionalString(  74,  66, "55121",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  84, "68-01",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  188, "21050",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  206, "161",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  395, "4-789",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);


	LoadGame_Blinkies();

	UI_PrintMenuGraphics(loadgamemenu_graphics,LGG_MAX);

}

/*
=================
M_LoadGame_MenuDraw
=================
*/
static void M_LoadGame_MenuDraw (void)
{
	M_LoadGame_Graphics();

	Menu_Draw( &s_loadgame.menu );
}

/*
=================
LoadGame_MenuEvent
=================
*/
static void LoadGame_MenuEvent( void* ptr, int notification )
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
	}
}

/*
=================
M_LoadGame_MenuKey
=================
*/
static sfxHandle_t M_LoadGame_MenuKey( int key )
{
	return Menu_DefaultKey( &s_loadgame.menu, key );
}


/*
===============
UI_LoadGameMenu_Cache
===============
*/
void UI_LoadGameMenu_Cache( void ) 
{
	s_savegame.currentGameTopLeft = ui.R_RegisterShaderNoMip("menu/common/lg_topleft.tga");
	s_savegame.currentGameBotLeft = ui.R_RegisterShaderNoMip("menu/common/lg_lowleft.tga");
	s_savegame.currentGameTopRight = ui.R_RegisterShaderNoMip("menu/common/lg_topright.tga");
	s_savegame.currentGameBotRight = ui.R_RegisterShaderNoMip("menu/common/lg_lowright.tga");

	s_savegame.corner_ur_16_18 = ui.R_RegisterShaderNoMip("menu/common/corner_ur_16_18.tga");
	s_savegame.corner_lr_8_16 = ui.R_RegisterShaderNoMip("menu/common/corner_lr_8_16.tga");

	// Precache all menu graphics in array
	UI_PrecacheMenuGraphics(loadgamemenu_graphics,LGG_MAX);

	ui.R_RegisterShaderNoMip("menu/common/underline.tga");

}


/*
===============
LoadGameMenu_Init
===============
*/
static void LoadGameMenu_Init(void) 
{
	int x,y,i;

	UI_LoadGameMenu_Cache();

	s_loadgame.menu.nitems					= 0;
	s_loadgame.menu.wrapAround				= qtrue;
	s_loadgame.menu.draw					= M_LoadGame_MenuDraw;
	s_loadgame.menu.key						= M_LoadGame_MenuKey;
	s_loadgame.menu.fullscreen				= qtrue;
	s_loadgame.menu.wrapAround				= qfalse;
	s_loadgame.menu.descX					= MENU_DESC_X;
	s_loadgame.menu.descY					= MENU_DESC_Y;
	s_loadgame.menu.titleX					= MENU_TITLE_X;
	s_loadgame.menu.titleY					= MENU_TITLE_Y;
	s_loadgame.menu.titleI					= MNT_LOADGAMEMENU_TITLE;
	s_loadgame.menu.footNoteEnum			= MNT_LOADGAME;

	s_loadgame.mainmenu.generic.type		= MTYPE_BITMAP;      
	s_loadgame.mainmenu.generic.flags		= QMF_HIGHLIGHT_IF_FOCUS;
	s_loadgame.mainmenu.generic.x			= 482;
	s_loadgame.mainmenu.generic.y			= 136;
	s_loadgame.mainmenu.generic.name		= GRAPHIC_BUTTONRIGHT;
	s_loadgame.mainmenu.generic.id			= ID_MAINMENU;
	s_loadgame.mainmenu.generic.callback	= LoadGame_MenuEvent;
	s_loadgame.mainmenu.width				= MENU_BUTTON_MED_WIDTH;
	s_loadgame.mainmenu.height				= MENU_BUTTON_MED_HEIGHT;
	s_loadgame.mainmenu.color				= CT_DKPURPLE1;
	s_loadgame.mainmenu.color2				= CT_LTPURPLE1;
	s_loadgame.mainmenu.textX				= MENU_BUTTON_TEXT_X;
	s_loadgame.mainmenu.textY				= MENU_BUTTON_TEXT_Y;

	if (!loadgameFromFlag)	// Quick key
	{
		s_loadgame.mainmenu.textEnum			= MBT_RETURNTOGAME;
	}
	else if (!ingameFlag)
	{
		s_loadgame.mainmenu.textEnum			= MBT_MAINMENU;
	}
	else	// In game menu
	{
		s_loadgame.mainmenu.textEnum			= MBT_INGAMEMENU;
	}

	s_loadgame.mainmenu.textcolor			= CT_BLACK;
	s_loadgame.mainmenu.textcolor2			= CT_WHITE;

	s_loadgame.load.generic.type				= MTYPE_BITMAP;      
	s_loadgame.load.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS;
	s_loadgame.load.generic.x					= 140;
	s_loadgame.load.generic.y					= 101;
	s_loadgame.load.generic.name				= "menu/common/square.tga";
	s_loadgame.load.generic.id					= ID_LOADGAMEDATA;
	s_loadgame.load.generic.callback			= SaveGame_MenuEvent; 
	s_loadgame.load.width						= MENU_BUTTON_MED_WIDTH;
	s_loadgame.load.height						= MENU_BUTTON_MED_HEIGHT;
	s_loadgame.load.color						= CT_DKPURPLE1;
	s_loadgame.load.color2						= CT_LTPURPLE1;
	s_loadgame.load.textX						= MENU_BUTTON_TEXT_X;
	s_loadgame.load.textY						= MENU_BUTTON_TEXT_Y;
	s_loadgame.load.textEnum					= MBT_LOADGAMEDATA;
	s_loadgame.load.textcolor					= CT_BLACK;
	s_loadgame.load.textcolor2					= CT_WHITE;

	s_savegame.deletegame.generic.type			= MTYPE_BITMAP;
	s_savegame.deletegame.generic.flags			= QMF_GRAYED;
	s_savegame.deletegame.generic.x				= 427;
	s_savegame.deletegame.generic.y				= 101;
	s_savegame.deletegame.generic.name			= "menu/common/square.tga";
	s_savegame.deletegame.generic.id			= ID_DELETEGAMEDATA;
	s_savegame.deletegame.generic.callback		= SaveGame_MenuEvent; 
	s_savegame.deletegame.width					= MENU_BUTTON_MED_WIDTH;
	s_savegame.deletegame.height				= MENU_BUTTON_MED_HEIGHT;
	s_savegame.deletegame.color					= CT_DKPURPLE1;
	s_savegame.deletegame.color2				= CT_LTPURPLE1;
	s_savegame.deletegame.textX					= MENU_BUTTON_TEXT_X;
	s_savegame.deletegame.textY					= MENU_BUTTON_TEXT_Y;
	s_savegame.deletegame.textEnum				= MBT_DELETEGAMEDATA;
	s_savegame.deletegame.textcolor				= CT_BLACK;
	s_savegame.deletegame.textcolor2			= CT_WHITE;

	s_savegame.arrowup.generic.type				= MTYPE_BITMAP;      
	s_savegame.arrowup.generic.flags			= QMF_HIDDEN;
	s_savegame.arrowup.generic.x				= 387;
	s_savegame.arrowup.generic.y				= 197;
	s_savegame.arrowup.generic.name				= "menu/common/arrow_up_16.tga";
	s_savegame.arrowup.generic.id				= ID_ARROWUP;
	s_savegame.arrowup.generic.callback			= SaveGame_MenuEvent; 
	s_savegame.arrowup.width					= 16;
	s_savegame.arrowup.height					= 16;
	s_savegame.arrowup.color					= CT_DKBLUE1;
	s_savegame.arrowup.color2					= CT_LTBLUE1;
	s_savegame.arrowup.textX					= 0;
	s_savegame.arrowup.textY					= 0;
	s_savegame.arrowup.textEnum					= MBT_NONE;
	s_savegame.arrowup.textcolor				= CT_BLACK;
	s_savegame.arrowup.textcolor2				= CT_WHITE;

	s_savegame.arrowdwn.generic.type			= MTYPE_BITMAP;      
	s_savegame.arrowdwn.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_savegame.arrowdwn.generic.x				= 387;
	s_savegame.arrowdwn.generic.y				= 400;
	s_savegame.arrowdwn.generic.name			= "menu/common/arrow_dn_16.tga";
	s_savegame.arrowdwn.generic.id				= ID_ARROWDWN;
	s_savegame.arrowdwn.generic.callback		= SaveGame_MenuEvent; 
	s_savegame.arrowdwn.width					= 16;
	s_savegame.arrowdwn.height					= 16;
	s_savegame.arrowdwn.color					= CT_DKBLUE1;
	s_savegame.arrowdwn.color2					= CT_LTBLUE1;
	s_savegame.arrowdwn.textX					= 0;
	s_savegame.arrowdwn.textY					= 0;
	s_savegame.arrowdwn.textEnum				= MBT_NONE;
	s_savegame.arrowdwn.textcolor				= CT_BLACK;
	s_savegame.arrowdwn.textcolor2				= CT_WHITE;

	s_file_desc_field.generic.type			= MTYPE_FIELD;
	s_file_desc_field.generic.flags			= QMF_INACTIVE;
	s_file_desc_field.generic.callback		= NULL;
	s_file_desc_field.generic.x				= 218;
	s_file_desc_field.generic.y				= 68;
	s_file_desc_field.field.widthInChars		= 40;
	s_file_desc_field.field.maxchars			= 40;
	s_file_desc_field.field.style			= UI_TINYFONT;
	s_file_desc_field.field.textEnum		= MNT_DESCRIPTION;
	s_file_desc_field.field.textcolor		= CT_LTGOLD1;
	s_file_desc_field.field.textcolor2		= CT_LTGOLD1;

	Menu_AddItem( &s_loadgame.menu, ( void * )&s_loadgame.mainmenu );
	Menu_AddItem( &s_loadgame.menu, ( void * )&s_file_desc_field );
	Menu_AddItem( &s_loadgame.menu, ( void * )&s_loadgame.load );
	Menu_AddItem( &s_loadgame.menu, ( void * )&s_savegame.deletegame );
	Menu_AddItem( &s_loadgame.menu, ( void * )&s_savegame.arrowup );
	Menu_AddItem( &s_loadgame.menu, ( void * )&s_savegame.arrowdwn );

	x = 90;
	y = 200;

	for (i=0;i<99;i++)
	{
		if (!g_savedline[i])
		{
			break;
		}

		((menubitmap_s *)g_savedline[i])->generic.type			= MTYPE_BITMAP;      
		((menubitmap_s *)g_savedline[i])->generic.flags			= QMF_INACTIVE;
		((menubitmap_s *)g_savedline[i])->generic.x				= x;
		((menubitmap_s *)g_savedline[i])->generic.y				= y;
		((menubitmap_s *)g_savedline[i])->generic.name			= "menu/common/underline.tga";
		((menubitmap_s *)g_savedline[i])->generic.callback		= SaveGame_MenuEvent;
		((menubitmap_s *)g_savedline[i])->generic.statusbarfunc	= SaveGame_LineEvent;
		((menubitmap_s *)g_savedline[i])->generic.id			= ID_SAVEDCOMMENT1+i;
		((menubitmap_s *)g_savedline[i])->width					= 200;
		((menubitmap_s *)g_savedline[i])->height				= 16;
		((menubitmap_s *)g_savedline[i])->color					= CT_DKPURPLE1;
		((menubitmap_s *)g_savedline[i])->color2				= CT_LTPURPLE1;
		((menubitmap_s *)g_savedline[i])->textPtr				= NULL;
		((menubitmap_s *)g_savedline[i])->textX					= 4;
		((menubitmap_s *)g_savedline[i])->textY					= 1;
		((menubitmap_s *)g_savedline[i])->textcolor				= CT_DKGOLD1;
		((menubitmap_s *)g_savedline[i])->textcolor2			= CT_LTGOLD1;
		((menubitmap_s *)g_savedline[i])->textStyle				= UI_TINYFONT;

		Menu_AddItem( &s_loadgame.menu, ( void * )g_savedline[i]);
		y += 18;
	}

	x = 292;
	y = 200;
	for (i=0;i<99;i++)
	{
		if (!g_savedate[i])
		{
			break;
		}

		((menubitmap_s *)g_savedate[i])->generic.type			= MTYPE_BITMAP;      
		((menubitmap_s *)g_savedate[i])->generic.flags			= QMF_INACTIVE;
		((menubitmap_s *)g_savedate[i])->generic.x				= x;
		((menubitmap_s *)g_savedate[i])->generic.y				= y;
		((menubitmap_s *)g_savedate[i])->color					= CT_DKPURPLE1;
		((menubitmap_s *)g_savedate[i])->color2					= CT_LTPURPLE1;
		((menubitmap_s *)g_savedate[i])->textPtr				= NULL;
		((menubitmap_s *)g_savedate[i])->textX					= 4;
		((menubitmap_s *)g_savedate[i])->textY					= 1;
		((menubitmap_s *)g_savedate[i])->textcolor				= CT_DKGOLD1;
		((menubitmap_s *)g_savedate[i])->textcolor2				= CT_LTGOLD1;
		((menubitmap_s *)g_savedate[i])->textStyle				= UI_TINYFONT;
		Menu_AddItem( &s_loadgame.menu, ( void * )g_savedate[i]);

		((menubitmap_s *)g_savetime[i])->generic.type			= MTYPE_BITMAP;      
		((menubitmap_s *)g_savetime[i])->generic.flags			= QMF_INACTIVE;
		((menubitmap_s *)g_savetime[i])->generic.x				= x + 36;
		((menubitmap_s *)g_savetime[i])->generic.y				= y;
		((menubitmap_s *)g_savetime[i])->color					= CT_DKPURPLE1;
		((menubitmap_s *)g_savetime[i])->color2					= CT_LTPURPLE1;
		((menubitmap_s *)g_savetime[i])->textPtr				= NULL;
		((menubitmap_s *)g_savetime[i])->textX					= 4;
		((menubitmap_s *)g_savetime[i])->textY					= 1;
		((menubitmap_s *)g_savetime[i])->textcolor				= CT_DKGOLD1;
		((menubitmap_s *)g_savetime[i])->textcolor2				= CT_LTGOLD1;
		((menubitmap_s *)g_savetime[i])->textStyle				= UI_TINYFONT;
		Menu_AddItem( &s_loadgame.menu, ( void * )g_savetime[i]);

		y += 18;
	}

	// Get savegame file names
	SaveGame_UpdateWindow();

	loadgamemenu_graphics[LGG_BLIP1].timer = uis.realtime;
	loadgamemenu_graphics[LGG_BLIP1].x = loadgamemenu_graphics[LGG_BLIP1].min;
	loadgamemenu_graphics[LGG_BLIP2].x = loadgamemenu_graphics[LGG_BLIP2].min;

}


/*
===============
UI_LoadGameMenu
===============
*/
void UI_LoadGameMenu(int fromFlag) 
{	
	loadgameFromFlag = fromFlag;

//	if (!s_loadgame.menu.initialized)
//	{
		LoadGameMenu_Init(); 
//	}

	if (!loadgameFromFlag)	// Did not come from a menu (Main or Ingame)
	{
		Mouse_Show();
	}

//	s_file_desc_field.generic.flags	= 0;

	s_savegame.currentLine = CURRENTLINE_NONE;
	s_savegame.currentId = 0;
	s_savegame.awaitingSave = qfalse;
	s_savegame.startISaveFile = 0;
	SaveGame_UpdateWindow();	// Update file window

	UI_PushMenu( &s_loadgame.menu);
}


//===================================================================
//
// New Game Menu
//
//===================================================================

// menu action identifiers
#define	ID_SUPEREASY				99
#define	ID_EASY						100
#define	ID_MEDIUM					101
#define	ID_HARD						102
#define	ID_MAINMENU					103
#define	ID_MALE						104
#define	ID_FEMALE					105
#define ID_STARTNEWGAME				106
#define ID_STARTTUTORIAL			107

static menuframework_s	s_newgame_menu;
menubitmap_s	s_newgame_mainmenu;
menubitmap_s	s_newgame_supereasy;
menubitmap_s	s_newgame_easy;
menubitmap_s	s_newgame_medium;
menubitmap_s	s_newgame_hard;

menubitmap_s	s_newgame_male;
menubitmap_s	s_newgame_female;

menubitmap_s	s_newgame_start;
menubitmap_s	s_newgame_tutorial;



/*
=================
NewGame_Numbers
=================
*/
static void NewGame_Numbers(void)
{
	int i;

	// Generate new numbers for top right
	if ((newgamemenu_graphics[NGG_NUMBERS].timer < uis.realtime) && (newgamemenu_graphics[NGG_NUMBERS].target==6))
	{

		for (i=0;i<7;i++)
		{
			// Get random number
			newgamemenu_graphics[NGG_COL1_NUM1 + i].target = UI_RandomNumbers(newgamemenu_graphics[NGG_COL1_NUM1 + i].max);
			newgamemenu_graphics[NGG_COL2_NUM1 + i].target = UI_RandomNumbers(newgamemenu_graphics[NGG_COL2_NUM1 + i].max);
			newgamemenu_graphics[NGG_COL3_NUM1 + i].target = UI_RandomNumbers(newgamemenu_graphics[NGG_COL3_NUM1 + i].max);
			newgamemenu_graphics[NGG_COL4_NUM1 + i].target = UI_RandomNumbers(newgamemenu_graphics[NGG_COL4_NUM1 + i].max);
			newgamemenu_graphics[NGG_COL5_NUM1 + i].target = UI_RandomNumbers(newgamemenu_graphics[NGG_COL5_NUM1 + i].max);
			newgamemenu_graphics[NGG_COL6_NUM1 + i].target = UI_RandomNumbers(newgamemenu_graphics[NGG_COL6_NUM1 + i].max);
		}

		newgamemenu_graphics[NGG_NUMBERS].target=1;

		// Turn off all but the first row
		for (i=0;i<6;i++)
		{
			newgamemenu_graphics[i + NGG_COL1_NUM2].type = MG_OFF;
			newgamemenu_graphics[i + NGG_COL2_NUM2].type = MG_OFF;
			newgamemenu_graphics[i + NGG_COL3_NUM2].type = MG_OFF;
			newgamemenu_graphics[i + NGG_COL4_NUM2].type = MG_OFF;
			newgamemenu_graphics[i + NGG_COL5_NUM2].type = MG_OFF;
			newgamemenu_graphics[i + NGG_COL6_NUM2].type = MG_OFF;
		}

		// Unhigh light old col 1,2,3
		i = newgamemenu_graphics[NGG_SPEC_ROW1].target;
		newgamemenu_graphics[i + NGG_COL1_NUM1].color = CT_DKGOLD1;
		newgamemenu_graphics[i + NGG_COL2_NUM1].color = CT_DKGOLD1;
		newgamemenu_graphics[i + NGG_COL3_NUM1].color = CT_DKGOLD1;

		// Choose a special row to highlight
		i = newgamemenu_graphics[NGG_SPEC_ROW1].target = random() * 6;

		// High light that row
		newgamemenu_graphics[i + NGG_COL1_NUM1].color = CT_LTGOLD1;
		newgamemenu_graphics[i + NGG_COL2_NUM1].color = CT_LTGOLD1;
		newgamemenu_graphics[i + NGG_COL3_NUM1].color = CT_LTGOLD1;
		newgamemenu_graphics[NGG_SPEC_ROW1].y = newgamemenu_graphics[i + NGG_COL1_NUM1].y + 6;


		// Unhigh light old col 4
		i = newgamemenu_graphics[NGG_SPEC_ROW2].target;
		newgamemenu_graphics[i + NGG_COL4_NUM1].color = CT_DKGOLD1;

		// Choose a special row to highlight
		i = newgamemenu_graphics[NGG_SPEC_ROW2].target = random() * 6;

		// High light that row
		newgamemenu_graphics[i + NGG_COL4_NUM1].color = CT_LTGOLD1;
		newgamemenu_graphics[NGG_SPEC_ROW2].y = newgamemenu_graphics[i + NGG_COL4_NUM1].y + 6;


		// Unhigh light old col 5,6
		i = newgamemenu_graphics[NGG_SPEC_ROW3].target;
		newgamemenu_graphics[i + NGG_COL5_NUM1].color = CT_DKGOLD1;
		newgamemenu_graphics[i + NGG_COL6_NUM1].color = CT_DKGOLD1;

		// Choose a special row to highlight
		i = newgamemenu_graphics[NGG_SPEC_ROW3].target = random() * 6;

		// High light that row
		newgamemenu_graphics[i + NGG_COL5_NUM1].color = CT_LTGOLD1;
		newgamemenu_graphics[i + NGG_COL6_NUM1].color = CT_LTGOLD1;
		newgamemenu_graphics[NGG_SPEC_ROW3].y = newgamemenu_graphics[i + NGG_COL5_NUM1].y + 6;

	}
	// Activate the next row of numbers.  When at the last row wait and then generate new numbers
	else if ((newgamemenu_graphics[NGG_NUMBERS].timer < uis.realtime) && (newgamemenu_graphics[NGG_NUMBERS].target<6))
	{
		ui.S_StartLocalSound( uis.menu_datadisp1_snd, CHAN_MENU1 );
		newgamemenu_graphics[NGG_NUMBERS].timer = uis.realtime + 50;

		i = newgamemenu_graphics[NGG_NUMBERS].target;
		newgamemenu_graphics[i + NGG_COL1_NUM1].type = MG_NUMBER;
		newgamemenu_graphics[i + NGG_COL2_NUM1].type = MG_NUMBER;
		newgamemenu_graphics[i + NGG_COL3_NUM1].type = MG_NUMBER;
		newgamemenu_graphics[i + NGG_COL4_NUM1].type = MG_NUMBER;
		newgamemenu_graphics[i + NGG_COL5_NUM1].type = MG_NUMBER;
		newgamemenu_graphics[i + NGG_COL6_NUM1].type = MG_NUMBER;

		newgamemenu_graphics[NGG_NUMBERS].target++;

		if (newgamemenu_graphics[NGG_NUMBERS].target == 6)
		{
			newgamemenu_graphics[NGG_NUMBERS].timer = uis.realtime + 6000;
		}
	}


	// Upper right numbers
	if (newgamemenu_graphics[NGG_TOP_NUMBERS].type == MG_VAR)
	{
		for (i=0;i<5;i++)
		{
			// Get random number
			newgamemenu_graphics[NGG_TOP_COL1_NUM1 + i].target = UI_RandomNumbers(newgamemenu_graphics[NGG_TOP_COL1_NUM1 + i].max);
			newgamemenu_graphics[NGG_TOP_COL2_NUM1 + i].target = UI_RandomNumbers(newgamemenu_graphics[NGG_TOP_COL2_NUM1 + i].max);
			newgamemenu_graphics[NGG_TOP_COL3_NUM1 + i].target = UI_RandomNumbers(newgamemenu_graphics[NGG_TOP_COL3_NUM1 + i].max);
			// And unhigh light everything
			newgamemenu_graphics[NGG_TOP_COL1_NUM1 + i].color = CT_DKGOLD1;
			newgamemenu_graphics[NGG_TOP_COL2_NUM1 + i].color = CT_DKGOLD1;
			newgamemenu_graphics[NGG_TOP_COL3_NUM1 + i].color = CT_DKGOLD1;

		}

		// Turn off all but the first row
		for (i=0;i<4;i++)
		{
			newgamemenu_graphics[NGG_TOP_COL1_NUM2 + i].type = MG_OFF;
			newgamemenu_graphics[NGG_TOP_COL2_NUM2 + i].type = MG_OFF;
			newgamemenu_graphics[NGG_TOP_COL3_NUM2 + i].type = MG_OFF;
		}

		// Highlight column 1 number
		newgamemenu_graphics[NGG_TOP_SPEC_ROW1].target = random() * 4;
		i = newgamemenu_graphics[NGG_TOP_SPEC_ROW1].target;
		newgamemenu_graphics[i + NGG_TOP_COL1_NUM1].color = CT_LTGOLD1;
		newgamemenu_graphics[NGG_TOP_SPEC_ROW1].y = newgamemenu_graphics[i + NGG_TOP_COL1_NUM1].y + 6;

		// Highlight column 2 number
		newgamemenu_graphics[NGG_TOP_SPEC_ROW2].target = random() * 4;
		i = newgamemenu_graphics[NGG_TOP_SPEC_ROW2].target;
		newgamemenu_graphics[i + NGG_TOP_COL2_NUM1].color = CT_LTGOLD1;
		newgamemenu_graphics[NGG_TOP_SPEC_ROW2].y = newgamemenu_graphics[i + NGG_TOP_COL2_NUM1].y + 6;

		// Highlight column 3 number
		newgamemenu_graphics[NGG_TOP_SPEC_ROW3].target = random() * 4;
		i = newgamemenu_graphics[NGG_TOP_SPEC_ROW3].target;
		newgamemenu_graphics[i + NGG_TOP_COL3_NUM1].color = CT_LTGOLD1;
		newgamemenu_graphics[NGG_TOP_SPEC_ROW3].y = newgamemenu_graphics[i + NGG_TOP_COL3_NUM1].y + 6;

		newgamemenu_graphics[NGG_TOP_NUMBERS].target=1;
		newgamemenu_graphics[NGG_TOP_NUMBERS].type = MG_OFF;
		newgamemenu_graphics[NGG_TOP_NUMBERS].timer = uis.realtime + 50;
	}
	else if ((newgamemenu_graphics[NGG_TOP_NUMBERS].timer < uis.realtime) && (newgamemenu_graphics[NGG_TOP_NUMBERS].target<5))
	{

		newgamemenu_graphics[NGG_TOP_NUMBERS].timer = uis.realtime + 50;

		i = newgamemenu_graphics[NGG_TOP_NUMBERS].target;

		newgamemenu_graphics[NGG_TOP_COL1_NUM1 + i].type = MG_NUMBER;
		newgamemenu_graphics[NGG_TOP_COL2_NUM1 + i].type = MG_NUMBER;
		newgamemenu_graphics[NGG_TOP_COL3_NUM1 + i].type = MG_NUMBER;

		newgamemenu_graphics[NGG_TOP_NUMBERS].target++;
	}
}

/*
=================
NewGame_Blinkies
=================
*/
static void NewGame_Blinkies(void)
{
	int i,inc_y;

	// Check out numbers, generate new ones if needed
	NewGame_Numbers();

	ui.R_SetColor( colorTable[CT_WHITE]);
	// The horizontal power bar has reached it's max
	if (newgamemenu_graphics[NGG_POWERBAND1_1].x > newgamemenu_graphics[NGG_POWERBAND1_1].max)
	{
		newgamemenu_graphics[NGG_POWERBAND1_1].x = newgamemenu_graphics[NGG_POWERBAND1_1].min;
		newgamemenu_graphics[NGG_POWERBAND1_2].x = newgamemenu_graphics[NGG_POWERBAND1_1].min;
		newgamemenu_graphics[NGG_WARPBAND1_1].type = MG_GRAPHIC;	// Turn on warp core power band
		newgamemenu_graphics[NGG_WARPBAND1_2].type = MG_GRAPHIC;	// Turn on warp core power band
		newgamemenu_graphics[NGG_WARPBAND1_3].type = MG_GRAPHIC;	// Turn on warp core power band
		newgamemenu_graphics[NGG_WARPBAND1_4].type = MG_GRAPHIC;	// Turn on warp core power band
		newgamemenu_graphics[NGG_WARPBAND1_1].y = newgamemenu_graphics[NGG_WARPBAND1_1].min;
		newgamemenu_graphics[NGG_WARPBAND1_2].y = newgamemenu_graphics[NGG_WARPBAND1_2].min;
		newgamemenu_graphics[NGG_WARPBAND1_3].y = newgamemenu_graphics[NGG_WARPBAND1_3].min;
		newgamemenu_graphics[NGG_WARPBAND1_4].y = newgamemenu_graphics[NGG_WARPBAND1_4].min;

		newgamemenu_graphics[NGG_TOP_NUMBERS].type = MG_VAR;	// Change upper right numbers

		if (newgamemenu_graphics[NGG_POWERBAND1_1].inc == SUPEREASY_INC)
		{
			newgamemenu_graphics[NGG_BAR4_LARROW].inc = -6;
		}
		else if (newgamemenu_graphics[NGG_POWERBAND1_1].inc == EASY_INC)
		{
			newgamemenu_graphics[NGG_BAR4_LARROW].inc = -6;
		}
		else if (newgamemenu_graphics[NGG_POWERBAND1_1].inc == MEDIUM_INC)
		{
			newgamemenu_graphics[NGG_BAR4_LARROW].inc = -3;
		}
		else
		{
			newgamemenu_graphics[NGG_BAR4_LARROW].inc = -3;
		}

		newgamemenu_graphics[NGG_BAR4_LARROW].target = newgamemenu_graphics[NGG_BAR4_LARROW].min + (random() * 4);
		newgamemenu_graphics[NGG_BAR4_RARROW].target = newgamemenu_graphics[NGG_BAR4_LARROW].target;
		newgamemenu_graphics[NGG_BAR4_RARROW].inc = newgamemenu_graphics[NGG_BAR4_LARROW].inc;


		if ((newgamemenu_graphics[NGG_POWERBAND1_1].inc == SUPEREASY_INC) ||
			(newgamemenu_graphics[NGG_POWERBAND1_1].inc == EASY_INC))
		{
			ui.S_StartLocalSound(newgame_cache.warpthrumslow, CHAN_LOCAL_SOUND);
		}
		else if (newgamemenu_graphics[NGG_POWERBAND1_1].inc == MEDIUM_INC)
		{
			ui.S_StartLocalSound(newgame_cache.warpthrummed, CHAN_LOCAL_SOUND);
		}
		else if (newgamemenu_graphics[NGG_POWERBAND1_1].inc == HARD_INC)
		{
			ui.S_StartLocalSound(newgame_cache.warpthrumfast, CHAN_LOCAL_SOUND);
		}
	}

	if ( newgamemenu_graphics[NGG_POWERBAND1_1].timer < uis.realtime )
	{
		newgamemenu_graphics[NGG_POWERBAND1_1].x += newgamemenu_graphics[NGG_POWERBAND1_1].inc;
		newgamemenu_graphics[NGG_POWERBAND1_1].timer = uis.realtime + 20;
		newgamemenu_graphics[NGG_POWERBAND1_2].x += newgamemenu_graphics[NGG_POWERBAND1_1].inc;
	}	

	for (i=0;i<2;i++)
	{
		if (newgamemenu_graphics[NGG_WARPBAND1_1 + (i*4)].type == MG_GRAPHIC)
		{
			newgamemenu_graphics[NGG_WARPBAND1_1 + (i*4)].y -=newgamemenu_graphics[NGG_WARPBAND1_1 + (i*4)].inc;
			newgamemenu_graphics[NGG_WARPBAND1_2 + (i*4)].y -=newgamemenu_graphics[NGG_WARPBAND1_1 + (i*4)].inc;
			newgamemenu_graphics[NGG_WARPBAND1_3 + (i*4)].y +=newgamemenu_graphics[NGG_WARPBAND1_1 + (i*4)].inc;
			newgamemenu_graphics[NGG_WARPBAND1_4 + (i*4)].y +=newgamemenu_graphics[NGG_WARPBAND1_1 + (i*4)].inc;
			if (newgamemenu_graphics[NGG_WARPBAND1_1 + (i*4)].y < newgamemenu_graphics[NGG_WARPBAND1_1 + (i*4)].max)
			{
				newgamemenu_graphics[NGG_WARPBAND1_1 + (i*4)].type = MG_NONE;	// Turn off warp core power band
				newgamemenu_graphics[NGG_WARPBAND1_2 + (i*4)].type = MG_NONE;	// Turn off warp core power band
				newgamemenu_graphics[NGG_WARPBAND1_3 + (i*4)].type = MG_NONE;	// Turn off warp core power band
				newgamemenu_graphics[NGG_WARPBAND1_4 + (i*4)].type = MG_NONE;	// Turn off warp core power band

				// Set target position for arrows when there is no warp power
				if (newgamemenu_graphics[NGG_POWERBAND1_1].inc == SUPEREASY_INC)
				{
					newgamemenu_graphics[NGG_BAR4_LARROW].inc = 6;
					inc_y = ((newgamemenu_graphics[NGG_BAR4_LARROW].max - newgamemenu_graphics[NGG_BAR4_LARROW].min) * .333) * 2;
				}
				else if (newgamemenu_graphics[NGG_POWERBAND1_1].inc == EASY_INC)
				{
					newgamemenu_graphics[NGG_BAR4_LARROW].inc = 6;
					inc_y = ((newgamemenu_graphics[NGG_BAR4_LARROW].max - newgamemenu_graphics[NGG_BAR4_LARROW].min) * .333) * 2;
				}
				else if (newgamemenu_graphics[NGG_POWERBAND1_1].inc == MEDIUM_INC)
				{
					newgamemenu_graphics[NGG_BAR4_LARROW].inc = 3;
					inc_y = (newgamemenu_graphics[NGG_BAR4_LARROW].max - newgamemenu_graphics[NGG_BAR4_LARROW].min) * .333;
				}
				else
				{
					newgamemenu_graphics[NGG_BAR4_LARROW].inc = 3;
					inc_y = 10;
				}

				// Make warp measuring arrows go down
				newgamemenu_graphics[NGG_BAR4_LARROW].target = newgamemenu_graphics[NGG_BAR4_LARROW].min + (random() * 4) +  inc_y;
				newgamemenu_graphics[NGG_BAR4_RARROW].target = newgamemenu_graphics[NGG_BAR4_LARROW].target;
				newgamemenu_graphics[NGG_BAR4_RARROW].inc = newgamemenu_graphics[NGG_BAR4_LARROW].inc;
			}
		}
	}

	// All arrows are keyed off the NGG_MASTER_ARROW timer
	if ( newgamemenu_graphics[NGG_MASTER_ARROW].timer < uis.realtime )
	{
		// Move ARROW positions
		for (i=NGG_BAR1_LARROW;i<=NGG_BAR4_RARROW ;i++)
		{
			// The bar measuring the warp power has it's own way of setting target position
			if ((i != NGG_BAR4_LARROW) && (i != NGG_BAR4_RARROW))
			{
				// Time to change this arrows target position
				if ( newgamemenu_graphics[i].timer < uis.realtime )
				{
					// See difficulty level we're at
					if (newgamemenu_graphics[NGG_POWERBAND1_1].inc == SUPEREASY_INC)
					{
						inc_y = ((newgamemenu_graphics[i].max - newgamemenu_graphics[i].min) * .333) * 2;
					}
					else if (newgamemenu_graphics[NGG_POWERBAND1_1].inc == EASY_INC)
					{
						inc_y = ((newgamemenu_graphics[i].max - newgamemenu_graphics[i].min) * .333) * 2;
					}
					else if (newgamemenu_graphics[NGG_POWERBAND1_1].inc == MEDIUM_INC)
					{
						inc_y = (newgamemenu_graphics[i].max - newgamemenu_graphics[i].min) * .333;
					}
					else
					{
						inc_y = 0;
					}

					newgamemenu_graphics[i].target = (random() * 10) + newgamemenu_graphics[i].min + inc_y;
					newgamemenu_graphics[i].timer = uis.realtime + 500;
					if (newgamemenu_graphics[i].y > newgamemenu_graphics[i].target)
					{
						newgamemenu_graphics[i].inc = -1 * abs(newgamemenu_graphics[i].inc);
					}
					else
					{
						newgamemenu_graphics[i].inc =  abs(newgamemenu_graphics[i].inc);
					}
				}
			}

			newgamemenu_graphics[i].y += newgamemenu_graphics[i].inc;

			if (newgamemenu_graphics[i].inc > 0)	// Moving higher
			{
				if (newgamemenu_graphics[i].y > newgamemenu_graphics[i].target)
				{
					newgamemenu_graphics[i].y = newgamemenu_graphics[i].target;
				}
			}
			else if (newgamemenu_graphics[i].inc < 0)	// Moving lower
			{
				if (newgamemenu_graphics[i].y < newgamemenu_graphics[i].target)
				{
					newgamemenu_graphics[i].y = newgamemenu_graphics[i].target;
				}
			}
		}

		newgamemenu_graphics[NGG_MASTER_ARROW].timer = uis.realtime + 100;
	}
}


/*
=================
M_NewGame_Graphics
=================
*/
static void M_NewGame_Graphics (void)
{
	int y;

	UI_MenuFrame(&s_newgame_menu);

	// Bracketing off top warp bars
	ui.R_SetColor( colorTable[CT_DKPURPLE2]);
	UI_DrawHandlePic(422, 54,  64,  64, newgame_cache.swooshTop);			// Top swoosh
	UI_DrawHandlePic( 80, 54, 343,   8, uis.whiteShader);	// Line across top
	UI_DrawHandlePic(429, 67,  50,  66, uis.whiteShader);	// Line of right hand column

	y = 67;
	ui.R_SetColor( colorTable[CT_DKPURPLE2]);
	UI_DrawHandlePic(145, y,  32, 64, newgame_cache.gridbar);	// 1st upper grid bar

	ui.R_SetColor( colorTable[CT_DKBLUE1]);
	UI_DrawHandlePic(224, y,  32, 64, newgame_cache.gridbar);	// 2nd upper grid bar

	ui.R_SetColor( colorTable[CT_DKGOLD1]);
	UI_DrawHandlePic(303, y,  32, 64, newgame_cache.gridbar);	// 3rd upper grid bar

	ui.R_SetColor( colorTable[CT_DKRED1]);
	UI_DrawHandlePic(382, y,  32, 64, newgame_cache.gridbar);	// 4th upper grid bar

	UI_DrawProportionalString(  371,  122, "33541",UI_RIGHT|UI_TINYFONT, colorTable[CT_LTRED1]);
	UI_DrawProportionalString(  292,  122, "1221",UI_RIGHT|UI_TINYFONT, colorTable[CT_LTGOLD1]);
	UI_DrawProportionalString(  213,  122, "1219",UI_RIGHT|UI_TINYFONT, colorTable[CT_LTBLUE1]);
	UI_DrawProportionalString(  134,  122, "1213",UI_RIGHT|UI_TINYFONT, colorTable[CT_LTPURPLE2]);


	ui.R_SetColor( colorTable[CT_DKPURPLE2]);
	UI_DrawHandlePic(30,203,  47, 186, uis.whiteShader);	// Long left hand column square

	ui.R_SetColor( colorTable[CT_WHITE]);
	UI_DrawHandlePic( 300, 167, 128,   256, newgame_cache.warpCore);

	ui.R_SetColor( colorTable[CT_DKPURPLE2]);
	UI_DrawHandlePic(  95, 268,  128,  64, newgame_cache.cornerUpper);		// Corner
	UI_DrawHandlePic( 174, 281,  127,   7, uis.whiteShader);	// Line from game difficulty to warp core

	ui.R_SetColor( colorTable[CT_DKBLUE1]);
	UI_DrawHandlePic( 95,  304,  128,  64, newgame_cache.cornerLower);		// Corner
	UI_DrawHandlePic( 174, 304,  127,   7, uis.whiteShader);	// Line from player gender to warp core

	ui.R_SetColor( colorTable[CT_DKGOLD1]);
	UI_DrawHandlePic(476,  290,  256,  -16, newgame_cache.swooshBottom);		// Bottom swoosh
	UI_DrawHandlePic(359,  293,  136,   8, uis.whiteShader);	// Line from engage button to warp core
	UI_DrawHandlePic( 482,  303,  130,  12, uis.whiteShader);	

	ui.R_SetColor( colorTable[CT_DKBLUE1]);
	UI_DrawHandlePic( 95, 157,  130, 24, uis.whiteShader);	// Game Difficulty title
	UI_DrawProportionalString(  95 + MENU_BUTTON_TEXT_X, 157 + 2, menu_normal_text[MNT_GAME_DIFFICULTY], UI_SMALLFONT, colorTable[CT_LTPURPLE1]);

	ui.R_SetColor( colorTable[CT_DKBLUE1]);
	UI_DrawHandlePic( 95, 382,  130, 24, uis.whiteShader);	// Player Gender title 
	UI_DrawProportionalString(  95 + MENU_BUTTON_TEXT_X, 382 + 2, menu_normal_text[MNT_GENDER], UI_SMALLFONT, colorTable[CT_LTPURPLE1]);

	ui.R_SetColor( NULL );

	NewGame_Blinkies();

	UI_PrintMenuGraphics(newgamemenu_graphics,NGG_MAX);

	UI_DrawProportionalString(  475,  60, "56A-88",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);

	UI_DrawProportionalString(  74,  66, "8-45611",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  84, "9214",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  188, "45",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  206, "7689200",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  395, "9955",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);


	ui.R_SetColor(colorTable[s_newgame_tutorial.color]);
	UI_DrawHandlePic(s_newgame_tutorial.generic.x - 14,
		s_newgame_tutorial.generic.y, MENU_BUTTON_MED_HEIGHT, MENU_BUTTON_MED_HEIGHT, 
		uis.graphicButtonLeftEnd);
}


/*
=================
M_NewGame_MenuDraw
=================
*/
static void M_NewGame_MenuDraw (void)
{
	M_NewGame_Graphics();

	Menu_Draw( &s_newgame_menu );
}

/*
=================
PlayerConfig_Male
=================
*/
static void PlayerConfig_SetMale( qboolean male)
{
	if (male) {
		//ui.Cvar_Set( "name", "Alex");
		ui.Cvar_Set( "legsmodel", "hazard/default" );
		ui.Cvar_Set( "torsomodel","hazard/default" );
		ui.Cvar_Set( "headmodel", "munro/default" );
		ui.Cvar_Set( "sex", "male" );
	} else {
		//ui.Cvar_Set( "name", "Alexandria");
		ui.Cvar_Set( "legsmodel", "hazardfemale/default" );
		ui.Cvar_Set( "torsomodel","hazardfemale/default" );
		ui.Cvar_Set( "headmodel", "alexandria/default" );
		ui.Cvar_Set( "sex", "female" );
	}
}

/*
=================
NewGame_MenuEvent
=================
*/
static void NewGame_MenuEvent( void* ptr, int notification )
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

		case ID_SUPEREASY:
			s_newgame_supereasy.textcolor				= CT_WHITE;
			s_newgame_easy.textcolor					= CT_BLACK;
			s_newgame_medium.textcolor					= CT_BLACK;
			s_newgame_hard.textcolor					= CT_BLACK;

			ui.Cvar_SetValue( "g_spskill", 0 );
			ui.Cvar_Set( "handicap", "200" );

			newgamemenu_graphics[NGG_POWERBAND1_1].inc	= SUPEREASY_INC;
			newgamemenu_graphics[NGG_WARPBAND1_1].inc	= SUPEREASY_INC;

			s_newgame_supereasy.generic.flags			|= QMF_BLINK;
			s_newgame_easy.generic.flags				&= ~ QMF_BLINK;
			s_newgame_medium.generic.flags				&= ~ QMF_BLINK;
			s_newgame_hard.generic.flags				&= ~ QMF_BLINK;

			break;
		case ID_EASY:
			s_newgame_supereasy.textcolor			= CT_BLACK;
			s_newgame_easy.textcolor				= CT_WHITE;
			s_newgame_medium.textcolor				= CT_BLACK;
			s_newgame_hard.textcolor				= CT_BLACK;

			ui.Cvar_SetValue( "g_spskill", 0 );
			ui.Cvar_Set( "handicap", "100" );

			newgamemenu_graphics[NGG_POWERBAND1_1].inc = EASY_INC;
			newgamemenu_graphics[NGG_WARPBAND1_1].inc = EASY_INC;

			s_newgame_supereasy.generic.flags	&= ~ QMF_BLINK;
			s_newgame_easy.generic.flags		|= QMF_BLINK;
			s_newgame_medium.generic.flags		&= ~ QMF_BLINK;
			s_newgame_hard.generic.flags		&= ~ QMF_BLINK;

			break;
		case ID_MEDIUM:
			s_newgame_supereasy.textcolor			= CT_BLACK;
			s_newgame_easy.textcolor				= CT_BLACK;
			s_newgame_medium.textcolor				= CT_WHITE;
			s_newgame_hard.textcolor				= CT_BLACK;
			ui.Cvar_SetValue( "g_spskill", 1 );
			ui.Cvar_Set( "handicap", "100" );

			s_newgame_supereasy.generic.flags	&= ~ QMF_BLINK;
			s_newgame_easy.generic.flags		&= ~ QMF_BLINK;
			s_newgame_medium.generic.flags		|= QMF_BLINK;
			s_newgame_hard.generic.flags		&= ~ QMF_BLINK;

			newgamemenu_graphics[NGG_POWERBAND1_1].inc = MEDIUM_INC;
			newgamemenu_graphics[NGG_WARPBAND1_1].inc = MEDIUM_INC;
			break;
		case ID_HARD:
			s_newgame_supereasy.textcolor			= CT_BLACK;
			s_newgame_easy.textcolor				= CT_BLACK;
			s_newgame_medium.textcolor				= CT_BLACK;
			s_newgame_hard.textcolor				= CT_WHITE;

			ui.Cvar_SetValue( "g_spskill", 2 );
			ui.Cvar_Set( "handicap", "100" );

			s_newgame_supereasy.generic.flags	&= ~ QMF_BLINK;
			s_newgame_easy.generic.flags		&= ~ QMF_BLINK;
			s_newgame_medium.generic.flags		&= ~ QMF_BLINK;
			s_newgame_hard.generic.flags		|= QMF_BLINK;

			newgamemenu_graphics[NGG_POWERBAND1_1].inc = HARD_INC;
			newgamemenu_graphics[NGG_WARPBAND1_1].inc = HARD_INC;
			break;

		case ID_MALE:
			s_newgame_male.textcolor				= CT_WHITE;
			s_newgame_female.textcolor				= CT_BLACK;

			s_newgame_female.generic.flags &= ~ QMF_BLINK;
			s_newgame_male.generic.flags |= QMF_BLINK;

			PlayerConfig_SetMale (qtrue);
			break;

		case ID_FEMALE:
			s_newgame_male.textcolor				= CT_BLACK;
			s_newgame_female.textcolor				= CT_WHITE;

			s_newgame_male.generic.flags &= ~ QMF_BLINK;
			s_newgame_female.generic.flags |= QMF_BLINK;

			PlayerConfig_SetMale (qfalse);
			break;

		case ID_STARTNEWGAME:
			UI_ForceMenuOff ();
			ui.Cvar_SetValue( "cg_virtualVoyager", 0 );
			ui.Cmd_ExecuteText( EXEC_APPEND, "map borg1\n");
			break;

		case ID_STARTTUTORIAL:
			UI_ForceMenuOff ();
			ui.Cvar_SetValue( "cg_virtualVoyager", 0 );
			ui.Cmd_ExecuteText( EXEC_APPEND, "map tutorial\n");
			break;
	}
}


/*
=================
M_NewGame_MenuKey
=================
*/
static sfxHandle_t M_NewGame_MenuKey( int key )
{
	return Menu_DefaultKey( &s_newgame_menu, key );
}


/*
===============
UI_NewGameMenu_Cache
===============
*/
void UI_NewGameMenu_Cache( void ) 
{
	newgame_cache.warpCore		= ui.R_RegisterShaderNoMip("menu/common/warpcore2.tga");

	newgame_cache.cornerUpper	= ui.R_RegisterShaderNoMip("menu/common/corner_love.tga");
	newgame_cache.cornerLower	= ui.R_RegisterShaderNoMip("menu/common/corner_love_2.tga");

	newgame_cache.swooshBottom	= ui.R_RegisterShaderNoMip("menu/common/corner_lr_8_130.tga");
	newgame_cache.gridbar		= ui.R_RegisterShaderNoMip( "menu/common/mon_bar.tga");
	newgame_cache.swooshTop		= ui.R_RegisterShaderNoMip("menu/common/swoosh_topsmall.tga");

	newgame_cache.warpconstant	= ui.S_RegisterSound( "sound/interface/warpcore.wav" );
	newgame_cache.warpthrumslow	= ui.S_RegisterSound( "sound/interface/warpthrob.wav" );
	newgame_cache.warpthrummed	= ui.S_RegisterSound( "sound/interface/warpthrobmed.wav" );
	newgame_cache.warpthrumfast	= ui.S_RegisterSound( "sound/interface/warpthrobhi.wav" );

	// Precache all menu graphics in array
	UI_PrecacheMenuGraphics(newgamemenu_graphics,NGG_MAX);
}

/*
===============
NewGameMenu_Init
===============
*/
static void NewGameMenu_Init(void) 
{
	int x,y,i,i2,i3,inc,pad;

	UI_NewGameMenu_Cache();

	s_newgame_menu.nitems					= 0;
	s_newgame_menu.wrapAround				= qtrue;
	s_newgame_menu.draw						= M_NewGame_MenuDraw;
	s_newgame_menu.key						= M_NewGame_MenuKey;
	s_newgame_menu.fullscreen				= qtrue;
	s_newgame_menu.wrapAround				= qfalse;
	s_newgame_menu.descX					= MENU_DESC_X;
	s_newgame_menu.descY					= MENU_DESC_Y;
	s_newgame_menu.titleX					= MENU_TITLE_X;
	s_newgame_menu.titleY					= MENU_TITLE_Y;
	s_newgame_menu.titleI					= MNT_NEWGAMEMENU_TITLE;
	s_newgame_menu.footNoteEnum				= MNT_WARP_CORE_STARTUP;

	s_newgame_mainmenu.generic.type			= MTYPE_BITMAP;      
	s_newgame_mainmenu.generic.flags		= QMF_HIGHLIGHT_IF_FOCUS;
	s_newgame_mainmenu.generic.x			= 482;
	s_newgame_mainmenu.generic.y			= 136;
	s_newgame_mainmenu.generic.name			= GRAPHIC_BUTTONRIGHT;
	s_newgame_mainmenu.generic.id			= ID_MAINMENU;
	s_newgame_mainmenu.generic.callback		= NewGame_MenuEvent;
	s_newgame_mainmenu.width				= MENU_BUTTON_MED_WIDTH;
	s_newgame_mainmenu.height				= MENU_BUTTON_MED_HEIGHT;
	s_newgame_mainmenu.color				= CT_DKPURPLE1;
	s_newgame_mainmenu.color2				= CT_LTPURPLE1;
	s_newgame_mainmenu.textX				= MENU_BUTTON_TEXT_X;
	s_newgame_mainmenu.textY				= MENU_BUTTON_TEXT_Y;
	s_newgame_mainmenu.textEnum				= MBT_MAINMENU;
	s_newgame_mainmenu.textcolor			= CT_BLACK;
	s_newgame_mainmenu.textcolor2			= CT_WHITE;

	x = 95; 
	y = 185;
	pad = 3;
	s_newgame_supereasy.generic.type				= MTYPE_BITMAP;      
	s_newgame_supereasy.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_newgame_supereasy.generic.x				= x;
	s_newgame_supereasy.generic.y				= y;
	s_newgame_supereasy.generic.name				= GRAPHIC_BUTTONRIGHT;
	s_newgame_supereasy.generic.id				= ID_SUPEREASY;
	s_newgame_supereasy.generic.callback			= NewGame_MenuEvent;
	s_newgame_supereasy.width					= MENU_BUTTON_MED_WIDTH;
	s_newgame_supereasy.height					= MENU_BUTTON_MED_HEIGHT;
	s_newgame_supereasy.color					= CT_DKPURPLE1;
	s_newgame_supereasy.color2					= CT_LTPURPLE1;
	s_newgame_supereasy.textX					= MENU_BUTTON_TEXT_X;
	s_newgame_supereasy.textY					= MENU_BUTTON_TEXT_Y;
	s_newgame_supereasy.textEnum				= MBT_SUPEREASY;
	s_newgame_supereasy.textcolor				= CT_BLACK;
	s_newgame_supereasy.textcolor2				= CT_WHITE;

	y += MENU_BUTTON_MED_HEIGHT + pad;
	s_newgame_easy.generic.type				= MTYPE_BITMAP;      
	s_newgame_easy.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_newgame_easy.generic.x				= x;
	s_newgame_easy.generic.y				= y;
	s_newgame_easy.generic.name				= GRAPHIC_BUTTONRIGHT;
	s_newgame_easy.generic.id				= ID_EASY;
	s_newgame_easy.generic.callback			= NewGame_MenuEvent;
	s_newgame_easy.width					= MENU_BUTTON_MED_WIDTH;
	s_newgame_easy.height					= MENU_BUTTON_MED_HEIGHT;
	s_newgame_easy.color					= CT_DKPURPLE1;
	s_newgame_easy.color2					= CT_LTPURPLE1;
	s_newgame_easy.textX					= MENU_BUTTON_TEXT_X;
	s_newgame_easy.textY					= MENU_BUTTON_TEXT_Y;
	s_newgame_easy.textEnum					= MBT_EASY;
	s_newgame_easy.textcolor				= CT_BLACK;
	s_newgame_easy.textcolor2				= CT_WHITE;

	y += MENU_BUTTON_MED_HEIGHT + pad;
	s_newgame_medium.generic.type			= MTYPE_BITMAP;      
	s_newgame_medium.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_newgame_medium.generic.x				= x;
	s_newgame_medium.generic.y				= y;
	s_newgame_medium.generic.name			= GRAPHIC_BUTTONRIGHT;
	s_newgame_medium.generic.id				= ID_MEDIUM;
	s_newgame_medium.generic.callback		= NewGame_MenuEvent;
	s_newgame_medium.width					= MENU_BUTTON_MED_WIDTH;
	s_newgame_medium.height					= MENU_BUTTON_MED_HEIGHT;
	s_newgame_medium.color					= CT_DKPURPLE1;
	s_newgame_medium.color2					= CT_LTPURPLE1;
	s_newgame_medium.textX					= MENU_BUTTON_TEXT_X;
	s_newgame_medium.textY					= MENU_BUTTON_TEXT_Y;
	s_newgame_medium.textEnum				= MBT_MEDIUM;
	s_newgame_medium.textcolor				= CT_BLACK;
	s_newgame_medium.textcolor2				= CT_WHITE;

	y += MENU_BUTTON_MED_HEIGHT + pad;
	s_newgame_hard.generic.type				= MTYPE_BITMAP;      
	s_newgame_hard.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_newgame_hard.generic.x				= x;
	s_newgame_hard.generic.y				= y;
	s_newgame_hard.generic.name				= GRAPHIC_BUTTONRIGHT;
	s_newgame_hard.generic.id				= ID_HARD;
	s_newgame_hard.generic.callback			= NewGame_MenuEvent;
	s_newgame_hard.width					= MENU_BUTTON_MED_WIDTH;
	s_newgame_hard.height					= MENU_BUTTON_MED_HEIGHT;
	s_newgame_hard.color					= CT_DKPURPLE1;
	s_newgame_hard.color2					= CT_LTPURPLE1;
	s_newgame_hard.textX					= MENU_BUTTON_TEXT_X;
	s_newgame_hard.textY					= MENU_BUTTON_TEXT_Y;
	s_newgame_hard.textEnum					= MBT_HARD;
	s_newgame_hard.textcolor				= CT_BLACK;
	s_newgame_hard.textcolor2				= CT_WHITE;


	y = 335;
	s_newgame_female.generic.type			= MTYPE_BITMAP;      
	s_newgame_female.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;	//QMF_GRAYED;
	s_newgame_female.generic.x				= x;
	s_newgame_female.generic.y				= y;
	s_newgame_female.generic.name			= GRAPHIC_BUTTONRIGHT;
	s_newgame_female.generic.id				= ID_FEMALE;
	s_newgame_female.generic.callback		= NewGame_MenuEvent;
	s_newgame_female.width					= MENU_BUTTON_MED_WIDTH;
	s_newgame_female.height					= MENU_BUTTON_MED_HEIGHT;
	s_newgame_female.color					= CT_DKPURPLE1;
	s_newgame_female.color2					= CT_LTPURPLE1;
	s_newgame_female.textX					= MENU_BUTTON_TEXT_X;
	s_newgame_female.textY					= MENU_BUTTON_TEXT_Y;
	s_newgame_female.textEnum				= MBT_FEMALE;
	s_newgame_female.textcolor				= CT_BLACK;
	s_newgame_female.textcolor2				= CT_WHITE;


	s_newgame_male.generic.type				= MTYPE_BITMAP;      
	s_newgame_male.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_newgame_male.generic.x				= x;
	s_newgame_male.generic.y				= y + MENU_BUTTON_MED_HEIGHT + 6;
	s_newgame_male.generic.name				= GRAPHIC_BUTTONRIGHT;
	s_newgame_male.generic.id				= ID_MALE;
	s_newgame_male.generic.callback			= NewGame_MenuEvent;
	s_newgame_male.width					= MENU_BUTTON_MED_WIDTH;
	s_newgame_male.height					= MENU_BUTTON_MED_HEIGHT;
	s_newgame_male.color					= CT_DKPURPLE1;
	s_newgame_male.color2					= CT_LTPURPLE1;
	s_newgame_male.textX					= MENU_BUTTON_TEXT_X;
	s_newgame_male.textY					= MENU_BUTTON_TEXT_Y;
	s_newgame_male.textEnum					= MBT_MALE;
	s_newgame_male.textcolor				= CT_BLACK;
	s_newgame_male.textcolor2				= CT_WHITE;


	s_newgame_start.generic.type			= MTYPE_BITMAP;
	s_newgame_start.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_newgame_start.generic.x				= 482;
	s_newgame_start.generic.y				= 319;
	s_newgame_start.generic.name			= "menu/common/square.tga";
	s_newgame_start.generic.id				= ID_STARTNEWGAME;
	s_newgame_start.generic.callback		= NewGame_MenuEvent;
	s_newgame_start.width					= MENU_BUTTON_MED_WIDTH;
	s_newgame_start.height					= 79;
	s_newgame_start.color					= CT_DKPURPLE1;
	s_newgame_start.color2					= CT_LTPURPLE1;
	s_newgame_start.textX					= MENU_BUTTON_TEXT_X;
	s_newgame_start.textY					= 56;
	s_newgame_start.textEnum				= MBT_ENGAGE;
	s_newgame_start.textcolor				= CT_BLACK;
	s_newgame_start.textcolor2				= CT_WHITE;

	s_newgame_tutorial.generic.type				= MTYPE_BITMAP;      
	s_newgame_tutorial.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_newgame_tutorial.generic.x				= 482;
	s_newgame_tutorial.generic.y				= 168;
	s_newgame_tutorial.generic.name				= GRAPHIC_BUTTONRIGHT;
	s_newgame_tutorial.generic.id				= ID_STARTTUTORIAL;
	s_newgame_tutorial.generic.callback			= NewGame_MenuEvent;
	s_newgame_tutorial.width					= MENU_BUTTON_MED_WIDTH;
	s_newgame_tutorial.height					= MENU_BUTTON_MED_HEIGHT;
	s_newgame_tutorial.color					= CT_DKPURPLE1;
	s_newgame_tutorial.color2					= CT_LTPURPLE1;
	s_newgame_tutorial.textX					= MENU_BUTTON_TEXT_X;
	s_newgame_tutorial.textY					= MENU_BUTTON_TEXT_Y;
	s_newgame_tutorial.textEnum					= MBT_TUTORIAL;
	s_newgame_tutorial.textcolor				= CT_BLACK;
	s_newgame_tutorial.textcolor2				= CT_WHITE;


	Menu_AddItem( &s_newgame_menu, ( void * )&s_newgame_mainmenu );
	Menu_AddItem( &s_newgame_menu, ( void * )&s_newgame_supereasy );
	Menu_AddItem( &s_newgame_menu, ( void * )&s_newgame_easy );
	Menu_AddItem( &s_newgame_menu, ( void * )&s_newgame_medium );
	Menu_AddItem( &s_newgame_menu, ( void * )&s_newgame_hard );

	Menu_AddItem( &s_newgame_menu, ( void * )&s_newgame_male );
	Menu_AddItem( &s_newgame_menu, ( void * )&s_newgame_female );

	Menu_AddItem( &s_newgame_menu, ( void * )&s_newgame_start );

	Menu_AddItem( &s_newgame_menu, ( void * )&s_newgame_tutorial );

	newgamemenu_graphics[NGG_WARPBAND1_1].type = MG_NONE;
	newgamemenu_graphics[NGG_WARPBAND1_2].type = MG_NONE;
	newgamemenu_graphics[NGG_WARPBAND1_3].type = MG_NONE;
	newgamemenu_graphics[NGG_WARPBAND1_4].type = MG_NONE;

	newgamemenu_graphics[NGG_POWERBAND1_1].inc = EASY_INC;
	newgamemenu_graphics[NGG_WARPBAND1_1].inc = EASY_INC;

//	ui.S_StartLocalSound(newgame_cache.warpconstant, CHAN_AMBIENT);

	inc = NGG_COL2_NUM1 - NGG_COL1_NUM1;
	for (i=0;i<6;i++)	// For each row
	{
		i3 = NGG_COL1_NUM1 + (i *inc);
		y = 199;
		// Set y values
		for (i2=0;i2<7;i2++)
		{
			newgamemenu_graphics[i3+i2].y = y;
			newgamemenu_graphics[i3+i2].color = CT_DKGOLD1;

			y += 14;
		}
	}

	// Set up arrow graphics data 
	for (i=NGG_BAR1_LARROW;i<=NGG_BAR4_RARROW ;i++)
	{
		newgamemenu_graphics[i].target =  (.33 * (newgamemenu_graphics[i].max - newgamemenu_graphics[i].min))
			+	newgamemenu_graphics[i].min;

		if (newgamemenu_graphics[i].y > newgamemenu_graphics[i].target)
		{
			newgamemenu_graphics[i].inc *= -1;
		}

		newgamemenu_graphics[i].y = newgamemenu_graphics[i].max - 8;
	}

	newgamemenu_graphics[NGG_BAR4_RARROW].target = newgamemenu_graphics[NGG_BAR4_LARROW].target;

	// Force numbers to change
	newgamemenu_graphics[NGG_NUMBERS].timer = 0;	// To get numbers right away
	newgamemenu_graphics[NGG_NUMBERS].target=6;
	newgamemenu_graphics[NGG_SPEC_ROW1].target = 0;
	newgamemenu_graphics[NGG_SPEC_ROW2].target = 0;
	newgamemenu_graphics[NGG_SPEC_ROW3].target = 0;

	if (!Q_stricmp( UI_Cvar_VariableString( "sex" ), "female" )) {
		PlayerConfig_SetMale(qfalse);	//force model

		s_newgame_male.textcolor				= CT_BLACK;
		s_newgame_female.textcolor				= CT_WHITE;

		s_newgame_male.generic.flags &= ~ QMF_BLINK;
		s_newgame_female.generic.flags |= QMF_BLINK;

	} else {
		PlayerConfig_SetMale(qtrue);
		s_newgame_female.generic.flags &= ~ QMF_BLINK;
		s_newgame_male.generic.flags |= QMF_BLINK;
		s_newgame_male.textcolor		= CT_WHITE;
	}

	switch ((int)ui.Cvar_VariableValue("g_spskill")) {
	case 0:	//easy
			s_newgame_easy.textcolor				= CT_WHITE;
			s_newgame_medium.textcolor				= CT_BLACK;
			s_newgame_hard.textcolor				= CT_BLACK;

			s_newgame_easy.generic.flags |= QMF_BLINK;
			s_newgame_medium.generic.flags &= ~ QMF_BLINK;
			s_newgame_hard.generic.flags &= ~ QMF_BLINK;
		break;
	case 1:	//med
			s_newgame_easy.textcolor				= CT_BLACK;
			s_newgame_medium.textcolor				= CT_WHITE;
			s_newgame_hard.textcolor				= CT_BLACK;
			newgamemenu_graphics[NGG_POWERBAND1_1].inc = MEDIUM_INC;
			newgamemenu_graphics[NGG_WARPBAND1_1].inc = MEDIUM_INC;

			s_newgame_easy.generic.flags &= ~ QMF_BLINK;
			s_newgame_medium.generic.flags |= QMF_BLINK;
			s_newgame_hard.generic.flags &= ~ QMF_BLINK;

		break;
	default:
	case 2:	//hard
			s_newgame_easy.textcolor				= CT_BLACK;
			s_newgame_medium.textcolor				= CT_BLACK;
			s_newgame_hard.textcolor				= CT_WHITE;
			newgamemenu_graphics[NGG_POWERBAND1_1].inc = HARD_INC;
			newgamemenu_graphics[NGG_WARPBAND1_1].inc = HARD_INC;

			s_newgame_easy.generic.flags &= ~ QMF_BLINK;
			s_newgame_medium.generic.flags &= ~ QMF_BLINK;
			s_newgame_hard.generic.flags |= QMF_BLINK;
		break;
	}
}


/*
===============
UI_NewGameMenu
===============
*/
void UI_NewGameMenu( void) 
{
	
	if (!s_newgame_menu.initialized)
	{
		NewGameMenu_Init(); 
	}

	UI_PushMenu( &s_newgame_menu);
}


static struct 
{
	menuframework_s menu;

	menubitmap_s	yes;
	menubitmap_s	no;

} s_continue;


/*
=================
M_VirtualSave_Graphics
=================
*/
void M_VirtualSave_Graphics (void)
{
	// Draw the basic screen layout
	UI_MenuFrame(&s_continue.menu);

	ui.R_SetColor( colorTable[CT_LTBROWN1]);
	UI_DrawHandlePic( 482, 136,  MENU_BUTTON_MED_WIDTH - 22, MENU_BUTTON_MED_HEIGHT, uis.whiteShader);
	UI_DrawHandlePic( 460 + MENU_BUTTON_MED_WIDTH - 4, 136,  -19,  MENU_BUTTON_MED_HEIGHT, uis.graphicButtonLeftEnd);	//right


	ui.R_SetColor( colorTable[CT_LTBROWN1]);
	UI_DrawHandlePic(30,203,  47, 186, uis.whiteShader);		// Long left column square on bottom 3rd

	ui.R_SetColor( colorTable[CT_LTRED1]);
	UI_DrawHandlePic( 132, 142,  128,  -64, cornerPic);	// Top Left corner
	UI_DrawHandlePic( 132, 352,  128,   64, cornerPic);	// Bottom Left corner

	UI_DrawHandlePic( 429, 142, -128,  -64, cornerPic);	// Top Right corner
	UI_DrawHandlePic( 429, 352, -128,   64, cornerPic);	// Bottom Right corner

	UI_DrawHandlePic(145,175,  395, 18, uis.whiteShader);		// Top
	UI_DrawHandlePic(132,193,  47, 175, uis.whiteShader);		// Left side
	UI_DrawHandlePic(510,193,  47, 175, uis.whiteShader);		// Right side
	UI_DrawHandlePic(147,365,  65, 18, uis.whiteShader);		// Bottom Left
	UI_DrawHandlePic(477,365,  65, 18, uis.whiteShader);		// Bottom Right

	UI_DrawProportionalString(345,249,menu_normal_text[MNT_TOURSAVEWARN1],UI_SMALLFONT | UI_CENTER,colorTable[CT_RED]);
	UI_DrawProportionalString(345,274,menu_normal_text[MNT_TOURSAVEWARN2],UI_SMALLFONT | UI_CENTER,colorTable[CT_RED]);
	UI_DrawProportionalString(345,310,menu_normal_text[MNT_TOURSAVEWARN3],UI_SMALLFONT | UI_CENTER,colorTable[CT_LTGOLD1]);

}


/*
=================
VirtualSave_MenuDraw
=================
*/
static void VirtualSave_MenuDraw (void)
{
	M_VirtualSave_Graphics();

	Menu_Draw( &s_continue.menu );
}

/*
=================
M_VirtualSave_Event
=================
*/
void M_VirtualSave_Event (void* ptr, int event)
{
	menuframework_s*	m;
	char	commandString[128];

	if (event != QM_ACTIVATED)
	{
		return;
	}

	m = ((menucommon_s*)ptr)->parent;

	switch (((menucommon_s*)ptr)->id)
	{
		// Do a new save then disconnect.
		case ID_LEAVE_YES:
			Com_sprintf( commandString,  sizeof(commandString), "save virtual\n");
			ui.Cmd_ExecuteText( EXEC_APPEND, commandString);


		// No, do nothing
		case ID_LEAVE_NO:
			LeaveGame_Action();
			break;
	}


}

/*
=================
M_VirtualSave_Event1
=================
*/
void M_VirtualSave_Event1 (void* ptr, int event)
{
	menuframework_s*	m;
	char	commandString[128];

	if (event != QM_ACTIVATED)
	{
		return;
	}

	m = ((menucommon_s*)ptr)->parent;

	switch (((menucommon_s*)ptr)->id)
	{
		// Do a new save then disconnect.
		case ID_LEAVE_YES:
			Com_sprintf( commandString,  sizeof(commandString), "save virtual\n");
			ui.Cmd_ExecuteText( EXEC_APPEND, commandString);

		// No, do nothing
		case ID_LEAVE_NO:
			UI_LCARSOut_Menu();
			break;
	}
}


/*
=================
M_VirtualSave_Event2
=================
*/
void M_VirtualSave_Event2 (void* ptr, int event)
{
	menuframework_s*	m;
	char	commandString[128];

	if (event != QM_ACTIVATED)
	{
		return;
	}

	m = ((menucommon_s*)ptr)->parent;

	switch (((menucommon_s*)ptr)->id)
	{
		// Do a new save
		case ID_LEAVE_YES:
			Com_sprintf( commandString,  sizeof(commandString), "save virtual\n");
			ui.Cmd_ExecuteText( EXEC_APPEND, commandString);

		// No, do nothing
		case ID_LEAVE_NO:
			UI_ForceMenuOff ();
			break;
	}
}



/*
=================
VirtualSave_MenuKey
=================
*/
static sfxHandle_t VirtualSave_MenuKey( int key )
{
	return Menu_DefaultKey( &s_continue.menu, key );
}

/*
=================
UI_VirtualSaveMenu_Cache
=================
*/
void UI_VirtualSaveMenu_Cache( void )
{
	cornerPic = ui.R_RegisterShaderNoMip("menu/common/corner_ll_47_18.tga");
}

/*
=================
VirtualSaveMenu_Init
=================
*/
static void VirtualSaveMenu_Init(int from)
{
	UI_VirtualSaveMenu_Cache();

	s_continue.menu.nitems					= 0;
	s_continue.menu.wrapAround				= qtrue;
	s_continue.menu.opening					= NULL;
	s_continue.menu.closing					= NULL;
	s_continue.menu.draw						= VirtualSave_MenuDraw;
	s_continue.menu.key						= VirtualSave_MenuKey;
	s_continue.menu.fullscreen					= qtrue;
	s_continue.menu.descX						= MENU_DESC_X;
	s_continue.menu.descY						= MENU_DESC_Y;
	s_continue.menu.listX						= 230;
	s_continue.menu.listY						= 188;
	s_continue.menu.titleX						= MENU_TITLE_X;
	s_continue.menu.titleY						= MENU_TITLE_Y;
	s_continue.menu.titleI						= MNT_CONTROLSMENU_TITLE;
	s_continue.menu.footNoteEnum				= MNT_DEFAULT_SETUP;

	s_continue.yes.generic.type				= MTYPE_BITMAP;      
	s_continue.yes.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS;
	s_continue.yes.generic.x					= 215;
	s_continue.yes.generic.y					= 365;
	s_continue.yes.generic.name				= "menu/common/square.tga";
	s_continue.yes.generic.id					= ID_LEAVE_YES;
	if (from==1)
	{
		s_continue.yes.generic.callback			= M_VirtualSave_Event1; 
	}
	else if (from==2)	// In Virtual Voyager
	{
		s_continue.yes.generic.callback			= M_VirtualSave_Event2; 
	}
	else
	{
		s_continue.yes.generic.callback			= M_VirtualSave_Event; 
	}
	s_continue.yes.width						= 103;
	s_continue.yes.height						= MENU_BUTTON_MED_HEIGHT;
	s_continue.yes.color						= CT_DKPURPLE1;
	s_continue.yes.color2						= CT_LTPURPLE1;
	s_continue.yes.textX						= MENU_BUTTON_TEXT_X;
	s_continue.yes.textY						= MENU_BUTTON_TEXT_Y;
	s_continue.yes.textEnum					= MBT_LEAVEGAME_YES;
	s_continue.yes.textcolor					= CT_BLACK;
	s_continue.yes.textcolor2					= CT_WHITE;
	s_continue.yes.textStyle					= UI_SMALLFONT;

	s_continue.no.generic.type					= MTYPE_BITMAP;      
	s_continue.no.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS;
	s_continue.no.generic.x					= 371;
	s_continue.no.generic.y					= 365;
	s_continue.no.generic.name					= "menu/common/square.tga";
	s_continue.no.generic.id					= ID_LEAVE_NO;
	s_continue.no.generic.callback				= M_VirtualSave_Event; 
	if (from==1)
	{
		s_continue.no.generic.callback			= M_VirtualSave_Event1; 
	}
	else if (from==2)	// In Virtual Voyager
	{
		s_continue.no.generic.callback			= M_VirtualSave_Event2; 
	}
	else
	{
		s_continue.no.generic.callback			= M_VirtualSave_Event; 
	}

	s_continue.no.width						= 103;
	s_continue.no.height						= MENU_BUTTON_MED_HEIGHT;
	s_continue.no.color						= CT_DKPURPLE1;
	s_continue.no.color2						= CT_LTPURPLE1;
	s_continue.no.textX						= MENU_BUTTON_TEXT_X;
	s_continue.no.textY						= MENU_BUTTON_TEXT_Y;
	s_continue.no.textEnum						= MBT_LEAVEGAME_NO;
	s_continue.no.textcolor					= CT_BLACK;
	s_continue.no.textcolor2					= CT_WHITE;
	s_continue.no.textStyle					= UI_SMALLFONT;

	Menu_AddItem( &s_continue.menu, ( void * )&s_continue.yes);
	Menu_AddItem( &s_continue.menu, ( void * )&s_continue.no);

}

/*
===============
UI_VirtualSaveMenu
===============
*/
void UI_VirtualSaveMenu(int from) 
{
	VirtualSaveMenu_Init(from); 

	UI_PushMenu( &s_continue.menu);
}


/*
=================
M_VirtualLoad_Event
=================
*/
void M_VirtualLoad_Event (void* ptr, int event)
{
	menuframework_s*	m;
	char	commandString[128];

	if (event != QM_ACTIVATED)
	{
		return;
	}

	m = ((menucommon_s*)ptr)->parent;

	switch (((menucommon_s*)ptr)->id)
	{
		// Do a new save
		case ID_LEAVE_YES:
			Com_sprintf( commandString,  sizeof(commandString), "load virtual\n");
			ui.Cmd_ExecuteText( EXEC_APPEND, commandString);
			break;

		// No, do nothing
		case ID_LEAVE_NO:
			UI_ForceMenuOff ();
			break;
	}
}

/*
=================
M_VirtualLoad_Graphics
=================
*/
void M_VirtualLoad_Graphics (void)
{
	// Draw the basic screen layout
	UI_MenuFrame(&s_continue.menu);

	ui.R_SetColor( colorTable[CT_LTBROWN1]);
	UI_DrawHandlePic( 482, 136,  MENU_BUTTON_MED_WIDTH - 22, MENU_BUTTON_MED_HEIGHT, uis.whiteShader);
	UI_DrawHandlePic( 460 + MENU_BUTTON_MED_WIDTH - 4, 136,  -19,  MENU_BUTTON_MED_HEIGHT, uis.graphicButtonLeftEnd);	//right


	ui.R_SetColor( colorTable[CT_LTBROWN1]);
	UI_DrawHandlePic(30,203,  47, 186, uis.whiteShader);		// Long left column square on bottom 3rd

	ui.R_SetColor( colorTable[CT_LTRED1]);
	UI_DrawHandlePic( 132, 142,  128,  -64, cornerPic);	// Top Left corner
	UI_DrawHandlePic( 132, 352,  128,   64, cornerPic);	// Bottom Left corner

	UI_DrawHandlePic( 429, 142, -128,  -64, cornerPic);	// Top Right corner
	UI_DrawHandlePic( 429, 352, -128,   64, cornerPic);	// Bottom Right corner

	UI_DrawHandlePic(145,175,  395, 18, uis.whiteShader);		// Top
	UI_DrawHandlePic(132,193,  47, 175, uis.whiteShader);		// Left side
	UI_DrawHandlePic(510,193,  47, 175, uis.whiteShader);		// Right side
	UI_DrawHandlePic(147,365,  65, 18, uis.whiteShader);		// Bottom Left
	UI_DrawHandlePic(477,365,  65, 18, uis.whiteShader);		// Bottom Right

	UI_DrawProportionalString(345,274,menu_normal_text[MNT_TOURLOADWARN1],UI_SMALLFONT | UI_CENTER,colorTable[CT_RED]);

}


/*
=================
VirtualLoad_MenuDraw
=================
*/
static void VirtualLoad_MenuDraw (void)
{
	M_VirtualLoad_Graphics();

	Menu_Draw( &s_continue.menu );
}

/*
=================
VirtualLoadMenu_Init
=================
*/
static void VirtualLoadMenu_Init(void)
{
	UI_VirtualSaveMenu_Cache();

	s_continue.menu.nitems					= 0;
	s_continue.menu.wrapAround				= qtrue;
	s_continue.menu.opening					= NULL;
	s_continue.menu.closing					= NULL;
	s_continue.menu.draw						= VirtualLoad_MenuDraw;
	s_continue.menu.key						= VirtualSave_MenuKey;
	s_continue.menu.fullscreen					= qtrue;
	s_continue.menu.descX						= MENU_DESC_X;
	s_continue.menu.descY						= MENU_DESC_Y;
	s_continue.menu.listX						= 230;
	s_continue.menu.listY						= 188;
	s_continue.menu.titleX						= MENU_TITLE_X;
	s_continue.menu.titleY						= MENU_TITLE_Y;
	s_continue.menu.titleI						= MNT_CONTROLSMENU_TITLE;
	s_continue.menu.footNoteEnum				= MNT_DEFAULT_SETUP;

	s_continue.yes.generic.type				= MTYPE_BITMAP;      
	s_continue.yes.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS;
	s_continue.yes.generic.x					= 215;
	s_continue.yes.generic.y					= 365;
	s_continue.yes.generic.name				= "menu/common/square.tga";
	s_continue.yes.generic.id					= ID_LEAVE_YES;
	s_continue.yes.generic.callback			= M_VirtualLoad_Event; 
	s_continue.yes.width						= 103;
	s_continue.yes.height						= MENU_BUTTON_MED_HEIGHT;
	s_continue.yes.color						= CT_DKPURPLE1;
	s_continue.yes.color2						= CT_LTPURPLE1;
	s_continue.yes.textX						= MENU_BUTTON_TEXT_X;
	s_continue.yes.textY						= MENU_BUTTON_TEXT_Y;
	s_continue.yes.textEnum					= MBT_LEAVEGAME_YES;
	s_continue.yes.textcolor					= CT_BLACK;
	s_continue.yes.textcolor2					= CT_WHITE;
	s_continue.yes.textStyle					= UI_SMALLFONT;

	s_continue.no.generic.type					= MTYPE_BITMAP;      
	s_continue.no.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS;
	s_continue.no.generic.x					= 371;
	s_continue.no.generic.y					= 365;
	s_continue.no.generic.name					= "menu/common/square.tga";
	s_continue.no.generic.id					= ID_LEAVE_NO;
	s_continue.no.generic.callback				= M_VirtualSave_Event; 
	s_continue.no.generic.callback			= M_VirtualLoad_Event; 

	s_continue.no.width						= 103;
	s_continue.no.height						= MENU_BUTTON_MED_HEIGHT;
	s_continue.no.color						= CT_DKPURPLE1;
	s_continue.no.color2						= CT_LTPURPLE1;
	s_continue.no.textX						= MENU_BUTTON_TEXT_X;
	s_continue.no.textY						= MENU_BUTTON_TEXT_Y;
	s_continue.no.textEnum						= MBT_LEAVEGAME_NO;
	s_continue.no.textcolor					= CT_BLACK;
	s_continue.no.textcolor2					= CT_WHITE;
	s_continue.no.textStyle					= UI_SMALLFONT;

	Menu_AddItem( &s_continue.menu, ( void * )&s_continue.yes);
	Menu_AddItem( &s_continue.menu, ( void * )&s_continue.no);

}

void UI_VirtualLoadMenu(void)
{
	VirtualLoadMenu_Init(); 

	UI_PushMenu( &s_continue.menu);
}

/*
=================
M_LeaveGameMenu_Graphics
=================
*/
void M_LeaveGameMenu_Graphics (void)
{
	// Draw the basic screen layout
	UI_MenuFrame(&s_leavegame_menu);

	ui.R_SetColor( colorTable[CT_LTBROWN1]);
	UI_DrawHandlePic(30,203,  47, 186, uis.whiteShader);		// Long left column square on bottom 3rd

	ui.R_SetColor( colorTable[CT_LTBLUE1]);
	UI_DrawHandlePic( 132, 142,  128,  -64, cornerPic);	// Top Left corner
	UI_DrawHandlePic( 132, 352,  128,   64, cornerPic);	// Bottom Left corner

	UI_DrawHandlePic( 429, 142, -128,  -64, cornerPic);	// Top Right corner
	UI_DrawHandlePic( 429, 352, -128,   64, cornerPic);	// Bottom Right corner

	UI_DrawHandlePic(145,175,  395, 18, uis.whiteShader);		// Top
	UI_DrawHandlePic(132,193,  47, 175, uis.whiteShader);		// Left side
	UI_DrawHandlePic(510,193,  47, 175, uis.whiteShader);		// Right side
	UI_DrawHandlePic(147,365,  65, 18, uis.whiteShader);		// Bottom Left
	UI_DrawHandlePic(477,365,  65, 18, uis.whiteShader);		// Bottom Right

	UI_DrawProportionalString(345,269,menu_normal_text[MNT_LEAVEGAME_WARNING1],UI_SMALLFONT | UI_CENTER,colorTable[CT_LTBLUE1]);

}

/*
=================
LeaveGame_MenuDraw
=================
*/
static void LeaveGame_MenuDraw (void)
{
	M_LeaveGameMenu_Graphics();

	Menu_Draw( &s_leavegame_menu );
}

static void LeaveGame_Action( void )
{
	ui.Cmd_ExecuteText( EXEC_APPEND, "disconnect\n" );
}


/*
=================
M_LeaveGame_Event
=================
*/
void M_LeaveGame_Event (void* ptr, int event)
{
	menuframework_s*	m;
//	char	commandString[128];

	if (event != QM_ACTIVATED)
	{
		return;
	}

	m = ((menucommon_s*)ptr)->parent;

	switch (((menucommon_s*)ptr)->id)
	{

		// Get default config
		case ID_LEAVE_YES:
			// If in virtual voyager (but not in holomatch) save the game.
			if ((ui.Cvar_VariableValue( "cg_virtualVoyager" )==1) && (holoMatch	== qfalse))
			{
				UI_VirtualSaveMenu(0);
			}
			else
			{
				LeaveGame_Action();
			}
			break;

		// No, go to main menu
		case ID_LEAVE_NO:
			UI_PopMenu();
			break;

	}
}

/*
=================
LeaveGame_MenuKey
=================
*/
static sfxHandle_t LeaveGame_MenuKey( int key )
{
	return Menu_DefaultKey( &s_leavegame_menu, key );
}

/*
=================
UI_LeaveGameMenu_Cache
=================
*/
void UI_LeaveGameMenu_Cache( void )
{
	cornerPic = ui.R_RegisterShaderNoMip("menu/common/corner_ll_47_18.tga");
}

/*
=================
LeaveGameMenu_Init
=================
*/
static void LeaveGameMenu_Init( void )
{
	UI_LeaveGameMenu_Cache();

	s_leavegame_menu.nitems					= 0;
	s_leavegame_menu.wrapAround				= qtrue;
	s_leavegame_menu.opening					= NULL;
	s_leavegame_menu.closing					= NULL;
	s_leavegame_menu.draw						= LeaveGame_MenuDraw;
	s_leavegame_menu.key						= LeaveGame_MenuKey;
	s_leavegame_menu.fullscreen					= qtrue;
	s_leavegame_menu.descX						= MENU_DESC_X;
	s_leavegame_menu.descY						= MENU_DESC_Y;
	s_leavegame_menu.listX						= 230;
	s_leavegame_menu.listY						= 188;
	s_leavegame_menu.titleX						= MENU_TITLE_X;
	s_leavegame_menu.titleY						= MENU_TITLE_Y;
	s_leavegame_menu.titleI						= MNT_CONTROLSMENU_TITLE;
	s_leavegame_menu.footNoteEnum				= MNT_DEFAULT_SETUP;

	s_newgame_mainmenu.generic.type			= MTYPE_BITMAP;      
	s_newgame_mainmenu.generic.flags		= QMF_HIGHLIGHT_IF_FOCUS;
	s_newgame_mainmenu.generic.x			= 482;
	s_newgame_mainmenu.generic.y			= 136;
	s_newgame_mainmenu.generic.name			= GRAPHIC_BUTTONRIGHT;
	s_newgame_mainmenu.generic.id			= ID_MAINMENU;
	s_newgame_mainmenu.generic.callback		= NewGame_MenuEvent;
	s_newgame_mainmenu.width				= MENU_BUTTON_MED_WIDTH;
	s_newgame_mainmenu.height				= MENU_BUTTON_MED_HEIGHT;
	s_newgame_mainmenu.color				= CT_DKPURPLE1;
	s_newgame_mainmenu.color2				= CT_LTPURPLE1;
	s_newgame_mainmenu.textX				= MENU_BUTTON_TEXT_X;
	s_newgame_mainmenu.textY				= MENU_BUTTON_TEXT_Y;
	s_newgame_mainmenu.textEnum				= MBT_INGAMEMENU;
	s_newgame_mainmenu.textcolor			= CT_BLACK;
	s_newgame_mainmenu.textcolor2			= CT_WHITE;

	s_leavegame_yes.generic.type				= MTYPE_BITMAP;      
	s_leavegame_yes.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS;
	s_leavegame_yes.generic.x					= 215;
	s_leavegame_yes.generic.y					= 365;
	s_leavegame_yes.generic.name				= "menu/common/square.tga";
	s_leavegame_yes.generic.id					= ID_LEAVE_YES;
	s_leavegame_yes.generic.callback			= M_LeaveGame_Event; 
	s_leavegame_yes.width						= 103;
	s_leavegame_yes.height						= MENU_BUTTON_MED_HEIGHT;
	s_leavegame_yes.color						= CT_DKPURPLE1;
	s_leavegame_yes.color2						= CT_LTPURPLE1;
	s_leavegame_yes.textX						= MENU_BUTTON_TEXT_X;
	s_leavegame_yes.textY						= MENU_BUTTON_TEXT_Y;
	s_leavegame_yes.textEnum					= MBT_LEAVEGAME_YES;
	s_leavegame_yes.textcolor					= CT_BLACK;
	s_leavegame_yes.textcolor2					= CT_WHITE;
	s_leavegame_yes.textStyle					= UI_SMALLFONT;

	s_leavegame_no.generic.type					= MTYPE_BITMAP;      
	s_leavegame_no.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS;
	s_leavegame_no.generic.x					= 371;
	s_leavegame_no.generic.y					= 365;
	s_leavegame_no.generic.name					= "menu/common/square.tga";
	s_leavegame_no.generic.id					= ID_LEAVE_NO;
	s_leavegame_no.generic.callback				= M_LeaveGame_Event; 
	s_leavegame_no.width						= 103;
	s_leavegame_no.height						= MENU_BUTTON_MED_HEIGHT;
	s_leavegame_no.color						= CT_DKPURPLE1;
	s_leavegame_no.color2						= CT_LTPURPLE1;
	s_leavegame_no.textX						= MENU_BUTTON_TEXT_X;
	s_leavegame_no.textY						= MENU_BUTTON_TEXT_Y;
	s_leavegame_no.textEnum						= MBT_LEAVEGAME_NO;
	s_leavegame_no.textcolor					= CT_BLACK;
	s_leavegame_no.textcolor2					= CT_WHITE;
	s_leavegame_no.textStyle					= UI_SMALLFONT;

	Menu_AddItem( &s_leavegame_menu, ( void * )&s_newgame_mainmenu);
	Menu_AddItem( &s_leavegame_menu, ( void * )&s_leavegame_yes);
	Menu_AddItem( &s_leavegame_menu, ( void * )&s_leavegame_no);

}

/*
===============
UI_LeaveGameMenu
===============
*/
void UI_LeaveGameMenu( void) 
{

	LeaveGameMenu_Init(); 

	UI_PushMenu( &s_leavegame_menu);
}



/*
=================
M_Continue_Graphics
=================
*/
void M_Continue_Graphics (void)
{
	// Draw the basic screen layout
	UI_MenuFrame(&s_continue.menu);

	ui.R_SetColor( colorTable[CT_LTBROWN1]);
	UI_DrawHandlePic( 482, 136,  MENU_BUTTON_MED_WIDTH - 22, MENU_BUTTON_MED_HEIGHT, uis.whiteShader);
	UI_DrawHandlePic( 460 + MENU_BUTTON_MED_WIDTH - 4, 136,  -19,  MENU_BUTTON_MED_HEIGHT, uis.graphicButtonLeftEnd);	//right


	ui.R_SetColor( colorTable[CT_LTBROWN1]);
	UI_DrawHandlePic(30,203,  47, 186, uis.whiteShader);		// Long left column square on bottom 3rd

	ui.R_SetColor( colorTable[CT_LTBLUE1]);
	UI_DrawHandlePic( 132, 142,  128,  -64, cornerPic);	// Top Left corner
	UI_DrawHandlePic( 132, 352,  128,   64, cornerPic);	// Bottom Left corner

	UI_DrawHandlePic( 429, 142, -128,  -64, cornerPic);	// Top Right corner
	UI_DrawHandlePic( 429, 352, -128,   64, cornerPic);	// Bottom Right corner

	UI_DrawHandlePic(145,175,  395, 18, uis.whiteShader);		// Top
	UI_DrawHandlePic(132,193,  47, 175, uis.whiteShader);		// Left side
	UI_DrawHandlePic(510,193,  47, 175, uis.whiteShader);		// Right side
	UI_DrawHandlePic(147,365,  65, 18, uis.whiteShader);		// Bottom Left
	UI_DrawHandlePic(477,365,  65, 18, uis.whiteShader);		// Bottom Right

	UI_DrawProportionalString(345,239,menu_normal_text[MNT_VV_ENGAGE_WARNING1],UI_SMALLFONT | UI_CENTER,colorTable[CT_LTBLUE1]);
	UI_DrawProportionalString(345,259,menu_normal_text[MNT_VV_ENGAGE_WARNING2],UI_SMALLFONT | UI_CENTER,colorTable[CT_LTBLUE1]);
	UI_DrawProportionalString(345,289,menu_normal_text[MNT_SAVE_WARNING3],UI_SMALLFONT | UI_CENTER,colorTable[CT_LTBLUE1]);

}


/*
=================
Continue_MenuDraw
=================
*/
static void Continue_MenuDraw (void)
{
	M_Continue_Graphics();

	Menu_Draw( &s_continue.menu );
}


/*
=================
StartTour

  Begin tour mode
=================
*/
static void StartTour (void)
{
	UI_ForceMenuOff ();
	UI_WipeTourHubSaveDirectory ();
	ui.Cvar_SetValue( "cg_virtualVoyager", 0 );	// So we don't get a warning. Gives it a default value of 0.
	ui.Cvar_SetValue( "cg_virtualVoyager", 1 );
	ui.Cmd_ExecuteText( EXEC_APPEND, "map tour/deck02\n");
}

/*
=================
M_Continue_Event
=================
*/
void M_Continue_Event (void* ptr, int event)
{
	menuframework_s*	m;

	if (event != QM_ACTIVATED)
	{
		return;
	}

	m = ((menucommon_s*)ptr)->parent;

	switch (((menucommon_s*)ptr)->id)
	{
		// Get default config
		case ID_LEAVE_YES:
			StartTour();
			break;

		// No, go to main menu
		case ID_LEAVE_NO:
			UI_PopMenu();
			break;
	}
}

/*
=================
Continue_MenuKey
=================
*/
static sfxHandle_t Continue_MenuKey( int key )
{
	return Menu_DefaultKey( &s_continue.menu, key );
}

/*
=================
UI_ContinueMenu_Cache
=================
*/
void UI_ContinueMenu_Cache( void )
{
	cornerPic = ui.R_RegisterShaderNoMip("menu/common/corner_ll_47_18.tga");
}

/*
=================
ContinueMenu_Init
=================
*/
static void ContinueMenu_Init( void )
{
	UI_ContinueMenu_Cache();

	s_continue.menu.nitems					= 0;
	s_continue.menu.wrapAround				= qtrue;
	s_continue.menu.opening					= NULL;
	s_continue.menu.closing					= NULL;
	s_continue.menu.draw						= Continue_MenuDraw;
	s_continue.menu.key						= Continue_MenuKey;
	s_continue.menu.fullscreen					= qtrue;
	s_continue.menu.descX						= MENU_DESC_X;
	s_continue.menu.descY						= MENU_DESC_Y;
	s_continue.menu.listX						= 230;
	s_continue.menu.listY						= 188;
	s_continue.menu.titleX						= MENU_TITLE_X;
	s_continue.menu.titleY						= MENU_TITLE_Y;
	s_continue.menu.titleI						= MNT_CONFIRMATION;
	s_continue.menu.footNoteEnum				= MNT_CONFIRMATION;

	s_continue.yes.generic.type				= MTYPE_BITMAP;      
	s_continue.yes.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS;
	s_continue.yes.generic.x					= 215;
	s_continue.yes.generic.y					= 365;
	s_continue.yes.generic.name				= "menu/common/square.tga";
	s_continue.yes.generic.id					= ID_LEAVE_YES;
	s_continue.yes.generic.callback			= M_Continue_Event; 
	s_continue.yes.width						= 103;
	s_continue.yes.height						= MENU_BUTTON_MED_HEIGHT;
	s_continue.yes.color						= CT_DKPURPLE1;
	s_continue.yes.color2						= CT_LTPURPLE1;
	s_continue.yes.textX						= MENU_BUTTON_TEXT_X;
	s_continue.yes.textY						= MENU_BUTTON_TEXT_Y;
	s_continue.yes.textEnum					= MBT_VIRTUALSAVE_YES;
	s_continue.yes.textcolor					= CT_BLACK;
	s_continue.yes.textcolor2					= CT_WHITE;
	s_continue.yes.textStyle					= UI_SMALLFONT;

	s_continue.no.generic.type					= MTYPE_BITMAP;      
	s_continue.no.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS;
	s_continue.no.generic.x					= 371;
	s_continue.no.generic.y					= 365;
	s_continue.no.generic.name					= "menu/common/square.tga";
	s_continue.no.generic.id					= ID_LEAVE_NO;
	s_continue.no.generic.callback				= M_Continue_Event; 
	s_continue.no.width						= 103;
	s_continue.no.height						= MENU_BUTTON_MED_HEIGHT;
	s_continue.no.color						= CT_DKPURPLE1;
	s_continue.no.color2						= CT_LTPURPLE1;
	s_continue.no.textX						= MENU_BUTTON_TEXT_X;
	s_continue.no.textY						= MENU_BUTTON_TEXT_Y;
	s_continue.no.textEnum						= MBT_VIRTUALSAVE_NO;
	s_continue.no.textcolor					= CT_BLACK;
	s_continue.no.textcolor2					= CT_WHITE;
	s_continue.no.textStyle					= UI_SMALLFONT;

	Menu_AddItem( &s_continue.menu, ( void * )&s_continue.yes);
	Menu_AddItem( &s_continue.menu, ( void * )&s_continue.no);

}

/*
===============
UI_ContinueMenu
===============
*/
void UI_ContinueMenu( void) 
{
	ContinueMenu_Init(); 

	UI_PushMenu( &s_continue.menu);
}


// Precache stuff for New Game Screen
typedef struct 
{
	menuframework_s menu;
	menubitmap_s	team;
	menubitmap_s	mainmenu;
	menubitmap_s	start;
	menubitmap_s	resume;
	menubitmap_s	female;
	menubitmap_s	male;
	int				holdeckCnt;
	qhandle_t		ship;
	qhandle_t		corner_ll_4_8;
	qhandle_t		corner_ul_8_18;
	qhandle_t		corner_ll_18_18;
	qhandle_t		corner_ul_18_22;
	menubitmap_s	line1_label;
	menubitmap_s	line2_label;
	menubitmap_s	line3_label;
	menubitmap_s	line4_label;
	menubitmap_s	line5_label;
	menubitmap_s	line6_label;
	menubitmap_s	line7_label;
	menubitmap_s	line8_label;
	menubitmap_s	line9_label;
} tourGame_t;


static tourGame_t	s_tourGame;

typedef enum 
{
	TMG_TIMER,		
	TMG_CURRENT_TEXT,		
	TMG_GRAPHIC1,
	TMG_GRAPHIC2,
	TMG_GRAPHIC3,
	TMG_GRAPHIC4,
	TMG_GRAPHIC5,
	TMG_GRAPHIC6,
	TMG_GRAPHIC7,
	TMG_GRAPHIC8,
	TMG_GRAPHIC9,

	TMG_LOCATION_DOT1,
	TMG_LOCATION_DOT2,
	TMG_LOCATION_DOT3,
	TMG_LOCATION_DOT4,
	TMG_LOCATION_DOT5,

	TMG_LINE1,
	TMG_LINE2,
	TMG_LINE3,
	TMG_LINE4,
	TMG_LINE5,

	TMG_MAX
} tourmenu_graphics_t;

#define TMG_TEXT_X 99
#define TMG_TEXT_Y 236
#define TMG_TEXT_PAD 14
#define TMG_TIMER_INC	4000
#define TMG_PIC_LENGTH	128
#define TMG_PIC_X_POS	425
#define TMG_PIC_Y_POS	200
#define	TMG_DOT_LENGTH	16
#define	TMG_SHIP_X_POS	-315
#define	TMG_SHIP_Y_POS	70


menugraphics_s tourmenu_graphics[TMG_MAX] = 
{

//	type		timer	x				y							width			height			file/text						normaltextEnum	buttontextEnum	graphic,	min		max	target	inc		style			color
	MG_VAR,		0.0,	0,				0,							0,				0,				NULL,								0,			0,				NULL,		0,		0,	0,		0,		0,				0,			NULL,	// TMG_TIMER
	MG_VAR,		0.0,	0,				0,							0,				0,				NULL,								0,			0,				NULL,		0,		0,	0,		0,		0,				0,			NULL,	// TMG_CURRENT_TEXT
	MG_GRAPHIC,	0.0,	TMG_PIC_X_POS,	TMG_PIC_Y_POS,				TMG_PIC_LENGTH,	TMG_PIC_LENGTH,	"menu/special/vv_bridge.tga",		0,			0,				NULL,		0,		0,	0,		0,		0,				CT_WHITE,	NULL,	// TMG_GRAPHIC1
	MG_GRAPHIC,	0.0,	TMG_PIC_X_POS,	TMG_PIC_Y_POS,				TMG_PIC_LENGTH,	TMG_PIC_LENGTH,	"menu/special/vv_explore.tga",		0,			0,				NULL,		0,		0,	0,		0,		0,				CT_WHITE,	NULL,	// TMG_GRAPHIC2
	MG_GRAPHIC,	0.0,	TMG_PIC_X_POS,	TMG_PIC_Y_POS,				TMG_PIC_LENGTH,	TMG_PIC_LENGTH,	"menu/special/vv_astrometric.tga",	0,			0,				NULL,		0,		0,	0,		0,		0,				CT_WHITE,	NULL,	// TMG_GRAPHIC3
	MG_GRAPHIC,	0.0,	TMG_PIC_X_POS,	TMG_PIC_Y_POS,				TMG_PIC_LENGTH,	TMG_PIC_LENGTH,	"menu/special/vv_shuttle.tga",		0,			0,				NULL,		0,		0,	0,		0,		0,				CT_WHITE,	NULL,	// TMG_GRAPHIC4
	MG_GRAPHIC,	0.0,	TMG_PIC_X_POS,	TMG_PIC_Y_POS,				TMG_PIC_LENGTH,	TMG_PIC_LENGTH,	"menu/special/vv_secrets.tga",		0,			0,				NULL,		0,		0,	0,		0,		0,				CT_WHITE,	NULL,	// TMG_GRAPHIC5
	MG_GRAPHIC,	0.0,	TMG_PIC_X_POS,	TMG_PIC_Y_POS,				TMG_PIC_LENGTH,	TMG_PIC_LENGTH,	"menu/special/vv_holodeck1.tga",	0,			0,				NULL,		0,		0,	0,		0,		0,				CT_WHITE,	NULL,	// TMG_GRAPHIC6
	MG_GRAPHIC,	0.0,	TMG_PIC_X_POS,	TMG_PIC_Y_POS,				TMG_PIC_LENGTH,	TMG_PIC_LENGTH,	"menu/special/vv_holodeck2.tga",	0,			0,				NULL,		0,		0,	0,		0,		0,				CT_WHITE,	NULL,	// TMG_GRAPHIC7
	MG_GRAPHIC,	0.0,	TMG_PIC_X_POS,	TMG_PIC_Y_POS,				TMG_PIC_LENGTH,	TMG_PIC_LENGTH,	"menu/special/vv_holodeck3.tga",	0,			0,				NULL,		0,		0,	0,		0,		0,				CT_WHITE,	NULL,	// TMG_GRAPHIC8
	MG_GRAPHIC,	0.0,	TMG_PIC_X_POS,	TMG_PIC_Y_POS,				TMG_PIC_LENGTH,	TMG_PIC_LENGTH,	"menu/special/vv_holodeck4.tga",	0,			0,				NULL,		0,		0,	0,		0,		0,				CT_WHITE,	NULL,	// TMG_GRAPHIC9

	MG_NONE,	0.0,	TMG_SHIP_X_POS+714,	TMG_SHIP_Y_POS+5,		TMG_DOT_LENGTH,	TMG_DOT_LENGTH,	"menu/objectives/circle_out.tga",	0,			0,				NULL,		0,		0,	0,		0,		0,				CT_LTGOLD1,	NULL,	// TMG_LOCATION_DOT1
	MG_NONE,	0.0,	TMG_SHIP_X_POS+350,	TMG_SHIP_Y_POS,			TMG_DOT_LENGTH,	TMG_DOT_LENGTH,	"menu/objectives/circle_out.tga",	0,			0,				NULL,		0,		0,	0,		0,		0,				CT_WHITE,	NULL,	// TMG_LOCATION_DOT2
	MG_NONE,	0.0,	TMG_SHIP_X_POS+350,	TMG_SHIP_Y_POS,			TMG_DOT_LENGTH,	TMG_DOT_LENGTH,	"menu/objectives/circle_out.tga",	0,			0,				NULL,		0,		0,	0,		0,		0,				CT_WHITE,	NULL,	// TMG_LOCATION_DOT3
	MG_NONE,	0.0,	TMG_SHIP_X_POS+350,	TMG_SHIP_Y_POS,			TMG_DOT_LENGTH,	TMG_DOT_LENGTH,	"menu/objectives/circle_out.tga",	0,			0,				NULL,		0,		0,	0,		0,		0,				CT_WHITE,	NULL,	// TMG_LOCATION_DOT4
	MG_NONE,	0.0,	TMG_SHIP_X_POS+350,	TMG_SHIP_Y_POS,			TMG_DOT_LENGTH,	TMG_DOT_LENGTH,	"menu/objectives/circle_out.tga",	0,			0,				NULL,		0,		0,	0,		0,		0,				CT_WHITE,	NULL,	// TMG_LOCATION_DOT5

	MG_VAR,		0.0,	TMG_TEXT_X,		TMG_TEXT_Y,					0,				0,				NULL,								0,			0,				NULL,		0,		0,	0,		0,		UI_TINYFONT,	CT_YELLOW,	(void *) &s_tourGame.line1_label,		// TMG_LINE1
	MG_VAR,		0.0,	TMG_TEXT_X,		TMG_TEXT_Y+(TMG_TEXT_PAD),	0,				0,				NULL,								0,			0,				NULL,		0,		0,	0,		0,		UI_TINYFONT,	CT_YELLOW,	(void *) &s_tourGame.line2_label,		// TMG_LINE2
	MG_VAR,		0.0,	TMG_TEXT_X,		TMG_TEXT_Y+(TMG_TEXT_PAD*2),0,				0,				NULL,								0,			0,				NULL,		0,		0,	0,		0,		UI_TINYFONT,	CT_YELLOW,	(void *) &s_tourGame.line3_label,		// TMG_LINE3
	MG_VAR,		0.0,	TMG_TEXT_X,		TMG_TEXT_Y+(TMG_TEXT_PAD*3),0,				0,				NULL,								0,			0,				NULL,		0,		0,	0,		0,		UI_TINYFONT,	CT_YELLOW,	(void *) &s_tourGame.line4_label,		// TMG_LINE4
	MG_VAR,		0.0,	TMG_TEXT_X,		TMG_TEXT_Y+(TMG_TEXT_PAD*4),0,				0,				NULL,								0,			0,				NULL,		0,		0,	0,		0,		UI_TINYFONT,	CT_YELLOW,	(void *) &s_tourGame.line5_label,		// TMG_LINE5
};


/*
=================
TourGame_Blinkies
=================
*/
static void TourGame_Blinkies(void)
{
	menubitmap_s	*holdLabel;

	// Time to go to next text????
	if ( tourmenu_graphics[TMG_TIMER].timer < uis.realtime )
	{

		holdLabel = &s_tourGame.line1_label;
		holdLabel += tourmenu_graphics[TMG_CURRENT_TEXT].target;
		holdLabel->textcolor = CT_DKGOLD1;

		tourmenu_graphics[TMG_GRAPHIC1 + tourmenu_graphics[TMG_CURRENT_TEXT].target + s_tourGame.holdeckCnt].type = MG_NONE;
//		tourmenu_graphics[TMG_LOCATION_DOT1 + tourmenu_graphics[TMG_CURRENT_TEXT].target].type = MG_NONE;	// Turn off dot

		if (tourmenu_graphics[TMG_CURRENT_TEXT].target==4)
		{
			s_tourGame.holdeckCnt=0;
		}
		else if (tourmenu_graphics[TMG_CURRENT_TEXT].target==5)
		{
			s_tourGame.holdeckCnt++;	// Count of current holodeck picture
		}

		tourmenu_graphics[TMG_CURRENT_TEXT].target++;
		if (tourmenu_graphics[TMG_CURRENT_TEXT].target >= 5) // Past end of text
		{
			tourmenu_graphics[TMG_CURRENT_TEXT].target=5;	
			if (s_tourGame.holdeckCnt>3)
			{
				tourmenu_graphics[TMG_CURRENT_TEXT].target=0;
				s_tourGame.holdeckCnt=0;
			}
		}

		holdLabel = &s_tourGame.line1_label;
		holdLabel += tourmenu_graphics[TMG_CURRENT_TEXT].target;
		holdLabel->textcolor = CT_LTGOLD1;

		// Three of the holodeck pictures don't get the beep
		if (tourmenu_graphics[TMG_CURRENT_TEXT].target< 5) 
		{
			ui.S_StartLocalSound( uis.menu_choice1_snd, CHAN_LOCAL_SOUND );	// Beep
		}

		// Turnon graphic
		tourmenu_graphics[TMG_GRAPHIC1 + tourmenu_graphics[TMG_CURRENT_TEXT].target + s_tourGame.holdeckCnt].type = MG_GRAPHIC;
//		tourmenu_graphics[TMG_LOCATION_DOT1 + tourmenu_graphics[TMG_CURRENT_TEXT].target].type = MG_GRAPHIC;		// Turn on dot

		// Holodeck text is special because it shows 4 pictures.
		if (tourmenu_graphics[TMG_CURRENT_TEXT].target>= 5)  
		{
			tourmenu_graphics[TMG_TIMER].timer = uis.realtime + TMG_TIMER_INC/4;
		}
		else
		{
			tourmenu_graphics[TMG_TIMER].timer = uis.realtime + TMG_TIMER_INC;
		}
	}
}

/*
=================
M_TourGame_Graphics
=================
*/
static void M_TourGame_Graphics (void)
{
	int holdX,holdLength;

	// Picture
	ui.R_SetColor( colorTable[CT_DKGREY]);
	UI_DrawHandlePic(TMG_PIC_X_POS-2, TMG_PIC_Y_POS-2, TMG_PIC_LENGTH+4,TMG_PIC_LENGTH+4, uis.whiteShader);	// Background

	// Main Frame
	ui.R_SetColor( colorTable[CT_LTPURPLE1]);
	UI_DrawHandlePic(TMG_SHIP_X_POS, TMG_SHIP_Y_POS, -924, 231, s_tourGame.ship);	// Voyager picture

	// Top 
	ui.R_SetColor( colorTable[CT_DKPURPLE2]);
	UI_DrawHandlePic(30, 25,  32,  32, s_tourGame.corner_ul_18_22);
	UI_DrawHandlePic(58, 25,  200,  22, uis.whiteShader);	// Top left hand column square


	holdX = 604 - ((UI_ProportionalStringWidth(menu_normal_text[MNT_TOURMENU_TITLE],UI_BIGFONT)) + 4);
	holdLength = holdX - 261;
	UI_DrawHandlePic( 261,  25,  holdLength,  22, uis.whiteShader);	// Bottom end line

	UI_DrawProportionalString(  604,  MENU_TITLE_Y, menu_normal_text[MNT_TOURMENU_TITLE],UI_BIGFONT|UI_RIGHT, colorTable[CT_LTBLUE1]);

	// Left side
	ui.R_SetColor( colorTable[CT_DKPURPLE2]);
	UI_DrawHandlePic(30, 32,  18, 182, uis.whiteShader);	// Top left hand column square
	UI_DrawHandlePic(30,217,  18, 107, uis.whiteShader);	// Middle left hand column square
	ui.R_SetColor( colorTable[CT_VDKPURPLE2]);
	UI_DrawHandlePic(30,327,  18, 115, uis.whiteShader);	// Bottom left hand column square

	UI_DrawHandlePic(30, 434,  32,  32, s_tourGame.corner_ll_18_18);
	UI_DrawHandlePic(48, 438,  45,  18, uis.whiteShader);	// Top left column


	ui.R_SetColor( colorTable[CT_LTBROWN1]);
	UI_DrawHandlePic( 96,  438, 268,  18, uis.whiteShader);	// Bottom front Line

	UI_MenuFrameBottomLineEnd_Graphics(menu_normal_text[MNT_TOURMENU_BY],CT_LTBROWN1);
	UI_DrawProportionalString(  MENU_TITLE_X, 440, menu_normal_text[MNT_TOURMENU_BY],UI_RIGHT | UI_SMALLFONT, colorTable[CT_LTORANGE]);

	// Gender frame
	ui.R_SetColor( colorTable[CT_DKBROWN1]);
	UI_DrawHandlePic(313,  345,  16,  32, s_tourGame.corner_ul_8_18);	// 
	ui.R_SetColor( colorTable[CT_DKBROWN1]);
	UI_DrawHandlePic(313,  418,  16,   8, s_tourGame.corner_ll_4_8);	// 

	ui.R_SetColor( colorTable[CT_DKBROWN1]);
	UI_DrawHandlePic( 317, 345,  162,  18, uis.whiteShader);	// Top 
	UI_DrawHandlePic( 313, 350,    8,  69, uis.whiteShader);	// Side
	UI_DrawHandlePic( 317, 420,  162,   4, uis.whiteShader);	// Bottom

	UI_DrawProportionalString( 330, 347, menu_normal_text[MNT_GENDER], UI_SMALLFONT, colorTable[CT_VLTBROWN1]);

	TourGame_Blinkies();

	UI_PrintMenuGraphics(tourmenu_graphics,TMG_MAX);

}


/*
=================
M_TourGame_MenuDraw
=================
*/
static void M_TourGame_MenuDraw (void)
{
	M_TourGame_Graphics();

	Menu_Draw( &s_tourGame.menu );
}


/*
=================
TourGame_MenuEvent
=================
*/
static void TourGame_MenuEvent( void* ptr, int notification )
{
	menuframework_s*	m;
	char		commandString[128];
	menubitmap_s	*holdLabel;

	if (notification != QM_ACTIVATED)
		return;

	m = ((menucommon_s*)ptr)->parent;

	switch (((menucommon_s*)ptr)->id)
	{
		case ID_MAINMENU:
			UI_PopMenu();
			break;

		case ID_MALE:
			s_tourGame.male.textcolor				= CT_WHITE;
			s_tourGame.female.textcolor				= CT_BLACK;

			s_tourGame.female.generic.flags &= ~ QMF_BLINK;
			s_tourGame.male.generic.flags |= QMF_BLINK;

			PlayerConfig_SetMale (qtrue);
			break;

		case ID_FEMALE:
			s_tourGame.male.textcolor				= CT_BLACK;
			s_tourGame.female.textcolor				= CT_WHITE;

			s_tourGame.male.generic.flags &= ~ QMF_BLINK;
			s_tourGame.female.generic.flags |= QMF_BLINK;

			PlayerConfig_SetMale (qfalse);
			break;

		case ID_STARTNEWGAME:
			// Does an old virtual save exist????
			if (ui.SG_ValidateForLoadSaveScreen("virtual"))	
			{
				UI_ContinueMenu();	// Ask to continue
			}
			else
			{
				StartTour ();
			}
			break;

		case ID_RESUMETOUR:
			ui.Cvar_SetValue( "cg_virtualVoyager", 0 );	// So we don't get a warning. Gives it a default value of 0.
			ui.Cvar_SetValue( "cg_virtualVoyager", 1 );
			Com_sprintf( commandString,  sizeof(commandString), "load virtual\n");
			ui.Cmd_ExecuteText( EXEC_APPEND, commandString);
			break;
		case ID_TOUR_LINE1:
		case ID_TOUR_LINE2:
		case ID_TOUR_LINE3:
		case ID_TOUR_LINE4:
		case ID_TOUR_LINE5:
		case ID_TOUR_LINE6:

			// Turn off old text hi-lite
			holdLabel = &s_tourGame.line1_label;
			holdLabel += tourmenu_graphics[TMG_CURRENT_TEXT].target;
			holdLabel->textcolor = CT_DKGOLD1;

			// Turn off picture
			tourmenu_graphics[TMG_GRAPHIC1 + tourmenu_graphics[TMG_CURRENT_TEXT].target].type = MG_OFF;
			tourmenu_graphics[TMG_LOCATION_DOT1 + tourmenu_graphics[TMG_CURRENT_TEXT].target].type = MG_OFF;		// Turn off dot

			tourmenu_graphics[TMG_CURRENT_TEXT].target = (((menucommon_s*)ptr)->id) - ID_TOUR_LINE1;

			// Turn on new picture
			tourmenu_graphics[TMG_GRAPHIC1 + tourmenu_graphics[TMG_CURRENT_TEXT].target].type = MG_GRAPHIC;
//			tourmenu_graphics[TMG_LOCATION_DOT1 + tourmenu_graphics[TMG_CURRENT_TEXT].target].type = MG_GRAPHIC;		// Turn on dot

			tourmenu_graphics[TMG_TIMER].timer = uis.realtime + TMG_TIMER_INC;

			// Turn on next text hi-lite
			holdLabel = &s_tourGame.line1_label;
			holdLabel += tourmenu_graphics[TMG_CURRENT_TEXT].target;
			holdLabel->textcolor = CT_LTGOLD1;

			// Special case for text line 5
			if (tourmenu_graphics[TMG_CURRENT_TEXT].target>= 5)  
			{
				tourmenu_graphics[TMG_TIMER].timer = uis.realtime + TMG_TIMER_INC/4;
			}
			s_tourGame.holdeckCnt=0;
			break;
	}
}

/*
=================
TourGame_MenuKey
=================
*/
static sfxHandle_t TourGame_MenuKey( int key )
{
	return Menu_DefaultKey( &s_tourGame.menu, key );
}

/*
=================
UI_TourGameMenu_Cache
=================
*/
void UI_TourGameMenu_Cache( void )
{
	s_tourGame.ship = ui.R_RegisterShaderNoMip("menu/special/voy_1.tga");
	s_tourGame.corner_ll_4_8 = ui.R_RegisterShaderNoMip("menu/common/corner_ll_4_8.tga");
	s_tourGame.corner_ul_8_18 = ui.R_RegisterShaderNoMip("menu/common/corner_ul_8_18.tga");
	s_tourGame.corner_ll_18_18 = ui.R_RegisterShaderNoMip("menu/common/corner_ll_18_18.tga");
	s_tourGame.corner_ul_18_22  = ui.R_RegisterShaderNoMip( "menu/common/corner_ul_18_22");

	UI_PrecacheMenuGraphics(tourmenu_graphics,TMG_MAX);

}

/*
=================
LeaveGameMenu_Init
=================
*/
static void TourGameMenu_Init( void )
{
	UI_TourGameMenu_Cache();

	s_tourGame.menu.nitems						= 0;
	s_tourGame.menu.wrapAround					= qtrue;
	s_tourGame.menu.opening						= NULL;
	s_tourGame.menu.closing						= NULL;
	s_tourGame.menu.draw						= M_TourGame_MenuDraw;
	s_tourGame.menu.key							= TourGame_MenuKey;
	s_tourGame.menu.fullscreen					= qtrue;
	s_tourGame.menu.descX						= MENU_DESC_X;
	s_tourGame.menu.descY						= MENU_DESC_Y;
	s_tourGame.menu.listX						= 230;
	s_tourGame.menu.listY						= 188;
	s_tourGame.menu.titleX						= MENU_TITLE_X;
	s_tourGame.menu.titleY						= MENU_TITLE_Y;
	s_tourGame.menu.titleI						= MNT_TOURVOYAGER;
	s_tourGame.menu.footNoteEnum				= MNT_TOURVOYAGER;

	s_tourGame.mainmenu.generic.type			= MTYPE_BITMAP;      
	s_tourGame.mainmenu.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_tourGame.mainmenu.generic.x				= 99;
	s_tourGame.mainmenu.generic.y				= 401;
	s_tourGame.mainmenu.generic.name			= GRAPHIC_BUTTONRIGHT;
	s_tourGame.mainmenu.generic.id				= ID_MAINMENU;
	s_tourGame.mainmenu.generic.callback		= NewGame_MenuEvent;
	s_tourGame.mainmenu.width					= MENU_BUTTON_MED_WIDTH;
	s_tourGame.mainmenu.height					= MENU_BUTTON_MED_HEIGHT;
	s_tourGame.mainmenu.color					= CT_DKPURPLE1;
	s_tourGame.mainmenu.color2					= CT_LTPURPLE1;
	s_tourGame.mainmenu.textX					= MENU_BUTTON_TEXT_X;
	s_tourGame.mainmenu.textY					= MENU_BUTTON_TEXT_Y;
	s_tourGame.mainmenu.textEnum				= MBT_MAINMENU;
	s_tourGame.mainmenu.textcolor				= CT_BLACK;
	s_tourGame.mainmenu.textcolor2				= CT_WHITE;


	s_tourGame.resume.generic.type			= MTYPE_BITMAP;
	if (ui.SG_ValidateForLoadSaveScreen("virtual"))
	{
		s_tourGame.resume.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	}
	else
	{
		s_tourGame.resume.generic.flags			= QMF_GRAYED;
	}

	s_tourGame.resume.generic.x				= 99;
	s_tourGame.resume.generic.y				= 345;
	s_tourGame.resume.generic.name			= "menu/common/square.tga";
	s_tourGame.resume.generic.id			= ID_RESUMETOUR;
	s_tourGame.resume.generic.callback		= TourGame_MenuEvent;
	s_tourGame.resume.width					= MENU_BUTTON_MED_WIDTH;
	s_tourGame.resume.height				= MENU_BUTTON_MED_HEIGHT + 20;
	s_tourGame.resume.color					= CT_DKPURPLE1;
	s_tourGame.resume.color2				= CT_LTPURPLE1;
	s_tourGame.resume.textX					= MENU_BUTTON_TEXT_X;
	s_tourGame.resume.textY					= MENU_BUTTON_TEXT_Y + 5;
	s_tourGame.resume.textEnum				= MBT_RESUMETOUR;
	s_tourGame.resume.textcolor				= CT_BLACK;
	s_tourGame.resume.textcolor2			= CT_WHITE;



	s_tourGame.start.generic.type			= MTYPE_BITMAP;
	s_tourGame.start.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_tourGame.start.generic.x				= 482;
	s_tourGame.start.generic.y				= 345;
	s_tourGame.start.generic.name			= "menu/common/square.tga";
	s_tourGame.start.generic.id				= ID_STARTNEWGAME;
	s_tourGame.start.generic.callback		= TourGame_MenuEvent;
	s_tourGame.start.width					= MENU_BUTTON_MED_WIDTH;
	s_tourGame.start.height					= 79;
	s_tourGame.start.color					= CT_DKPURPLE1;
	s_tourGame.start.color2					= CT_LTPURPLE1;
	s_tourGame.start.textX					= MENU_BUTTON_TEXT_X;
	s_tourGame.start.textY					= 56;
	s_tourGame.start.textEnum				= MBT_ENGAGE;
	s_tourGame.start.textcolor				= CT_BLACK;
	s_tourGame.start.textcolor2				= CT_WHITE;

	s_tourGame.male.generic.type			= MTYPE_BITMAP;      
	s_tourGame.male.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_tourGame.male.generic.x				= 338;
	s_tourGame.male.generic.y				= 370;
	s_tourGame.male.generic.name			= "menu/common/square.tga";
	s_tourGame.male.generic.id				= ID_MALE;
	s_tourGame.male.generic.callback		= TourGame_MenuEvent;
	s_tourGame.male.width					= MENU_BUTTON_MED_WIDTH;
	s_tourGame.male.height					= MENU_BUTTON_MED_HEIGHT;
	s_tourGame.male.color					= CT_DKPURPLE1;
	s_tourGame.male.color2					= CT_LTPURPLE1;
	s_tourGame.male.textX					= MENU_BUTTON_TEXT_X;
	s_tourGame.male.textY					= MENU_BUTTON_TEXT_Y;
	s_tourGame.male.textEnum				= MBT_MALE;
	s_tourGame.male.textcolor				= CT_BLACK;
	s_tourGame.male.textcolor2				= CT_WHITE;

	s_tourGame.female.generic.type			= MTYPE_BITMAP;      
	s_tourGame.female.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_tourGame.female.generic.x				= 338;
	s_tourGame.female.generic.y				= 395;
	s_tourGame.female.generic.name			= "menu/common/square.tga";
	s_tourGame.female.generic.id			= ID_FEMALE;
	s_tourGame.female.generic.callback		= TourGame_MenuEvent;
	s_tourGame.female.width					= MENU_BUTTON_MED_WIDTH;
	s_tourGame.female.height				= MENU_BUTTON_MED_HEIGHT;
	s_tourGame.female.color					= CT_DKPURPLE1;
	s_tourGame.female.color2				= CT_LTPURPLE1;
	s_tourGame.female.textX					= MENU_BUTTON_TEXT_X;
	s_tourGame.female.textY					= MENU_BUTTON_TEXT_Y;
	s_tourGame.female.textEnum				= MBT_FEMALE;
	s_tourGame.female.textcolor				= CT_BLACK;
	s_tourGame.female.textcolor2			= CT_WHITE;

	s_tourGame.line1_label.generic.type				= MTYPE_BITMAP;      
	s_tourGame.line1_label.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_tourGame.line1_label.generic.x				= TMG_TEXT_X;
	s_tourGame.line1_label.generic.y				= TMG_TEXT_Y;
	s_tourGame.line1_label.generic.name				= "menu/common/square2.tga";
	s_tourGame.line1_label.generic.id				= ID_TOUR_LINE1;
	s_tourGame.line1_label.generic.callback			= TourGame_MenuEvent; 
	s_tourGame.line1_label.width					= UI_ProportionalStringWidth((const char *) menu_button_text[MBT_TOURMENU_TEXT1][0],UI_TINYFONT);
	s_tourGame.line1_label.height					= 8;
	s_tourGame.line1_label.color					= CT_DKPURPLE1;
	s_tourGame.line1_label.color2					= CT_LTPURPLE1;
	s_tourGame.line1_label.textX					= 2;
	s_tourGame.line1_label.textY					= 1;
	s_tourGame.line1_label.textEnum					= MBT_TOURMENU_TEXT1;
	s_tourGame.line1_label.textcolor				= CT_LTGOLD1;
	s_tourGame.line1_label.textcolor2				= CT_WHITE;
	s_tourGame.line1_label.textStyle				= UI_TINYFONT;

	s_tourGame.line2_label.generic.type				= MTYPE_BITMAP;      
	s_tourGame.line2_label.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_tourGame.line2_label.generic.x				= TMG_TEXT_X;
	s_tourGame.line2_label.generic.y				= TMG_TEXT_Y+(TMG_TEXT_PAD);
	s_tourGame.line2_label.generic.name				= "menu/common/square2.tga";
	s_tourGame.line2_label.generic.id				= ID_TOUR_LINE2;
	s_tourGame.line2_label.generic.callback			= TourGame_MenuEvent; 
	s_tourGame.line2_label.width					= UI_ProportionalStringWidth((const char *) menu_button_text[MBT_TOURMENU_TEXT2][0],UI_TINYFONT);
	s_tourGame.line2_label.height					= 8;
	s_tourGame.line2_label.color					= CT_DKPURPLE1;
	s_tourGame.line2_label.color2					= CT_LTPURPLE1;
	s_tourGame.line2_label.textX					= 2;
	s_tourGame.line2_label.textY					= 1;
	s_tourGame.line2_label.textEnum					= MBT_TOURMENU_TEXT2;
	s_tourGame.line2_label.textcolor				= CT_DKGOLD1;
	s_tourGame.line2_label.textcolor2				= CT_WHITE;
	s_tourGame.line2_label.textStyle				= UI_TINYFONT;

	s_tourGame.line3_label.generic.type				= MTYPE_BITMAP;      
	s_tourGame.line3_label.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_tourGame.line3_label.generic.x				= TMG_TEXT_X;
	s_tourGame.line3_label.generic.y				= TMG_TEXT_Y+(TMG_TEXT_PAD*2);
	s_tourGame.line3_label.generic.name				= "menu/common/square2.tga";
	s_tourGame.line3_label.generic.id				= ID_TOUR_LINE3;
	s_tourGame.line3_label.generic.callback			= TourGame_MenuEvent; 
	s_tourGame.line3_label.width					= UI_ProportionalStringWidth((const char *) menu_button_text[MBT_TOURMENU_TEXT3][0],UI_TINYFONT);
	s_tourGame.line3_label.height					= 8;
	s_tourGame.line3_label.color					= CT_DKPURPLE1;
	s_tourGame.line3_label.color2					= CT_LTPURPLE1;
	s_tourGame.line3_label.textX					= 2;
	s_tourGame.line3_label.textY					= 1;
	s_tourGame.line3_label.textEnum					= MBT_TOURMENU_TEXT3;
	s_tourGame.line3_label.textcolor				= CT_DKGOLD1;
	s_tourGame.line3_label.textcolor2				= CT_WHITE;
	s_tourGame.line3_label.textStyle				= UI_TINYFONT;

	s_tourGame.line4_label.generic.type				= MTYPE_BITMAP;      
	s_tourGame.line4_label.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_tourGame.line4_label.generic.x				= TMG_TEXT_X;
	s_tourGame.line4_label.generic.y				= TMG_TEXT_Y+(TMG_TEXT_PAD*3);
	s_tourGame.line4_label.generic.name				= "menu/common/square2.tga";
	s_tourGame.line4_label.generic.id				= ID_TOUR_LINE4;
	s_tourGame.line4_label.generic.callback			= TourGame_MenuEvent; 
	s_tourGame.line4_label.width					= UI_ProportionalStringWidth((const char *) menu_button_text[MBT_TOURMENU_TEXT4][0],UI_TINYFONT);
	s_tourGame.line4_label.height					= 8;
	s_tourGame.line4_label.color					= CT_DKPURPLE1;
	s_tourGame.line4_label.color2					= CT_LTPURPLE1;
	s_tourGame.line4_label.textX					= 2;
	s_tourGame.line4_label.textY					= 1;
	s_tourGame.line4_label.textEnum					= MBT_TOURMENU_TEXT4;
	s_tourGame.line4_label.textcolor				= CT_DKGOLD1;
	s_tourGame.line4_label.textcolor2				= CT_WHITE;
	s_tourGame.line4_label.textStyle				= UI_TINYFONT;

	s_tourGame.line5_label.generic.type				= MTYPE_BITMAP;      
	s_tourGame.line5_label.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_tourGame.line5_label.generic.x				= TMG_TEXT_X;
	s_tourGame.line5_label.generic.y				= TMG_TEXT_Y+(TMG_TEXT_PAD*4);
	s_tourGame.line5_label.generic.name				= "menu/common/square2.tga";
	s_tourGame.line5_label.generic.id				= ID_TOUR_LINE5;
	s_tourGame.line5_label.generic.callback			= TourGame_MenuEvent; 
	s_tourGame.line5_label.width					= UI_ProportionalStringWidth((const char *) menu_button_text[MBT_TOURMENU_TEXT6][0],UI_TINYFONT);
	s_tourGame.line5_label.height					= 8;
	s_tourGame.line5_label.color					= CT_DKPURPLE1;
	s_tourGame.line5_label.color2					= CT_LTPURPLE1;
	s_tourGame.line5_label.textX					= 2;
	s_tourGame.line5_label.textY					= 1;
	s_tourGame.line5_label.textEnum					= MBT_TOURMENU_TEXT6;
	s_tourGame.line5_label.textcolor				= CT_DKGOLD1;
	s_tourGame.line5_label.textcolor2				= CT_WHITE;
	s_tourGame.line5_label.textStyle				= UI_TINYFONT;

	s_tourGame.line6_label.generic.type				= MTYPE_BITMAP;      
	s_tourGame.line6_label.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_tourGame.line6_label.generic.x				= TMG_TEXT_X;
	s_tourGame.line6_label.generic.y				= TMG_TEXT_Y+(TMG_TEXT_PAD*5);
	s_tourGame.line6_label.generic.name				= "menu/common/square2.tga";
	s_tourGame.line6_label.generic.id				= ID_TOUR_LINE6;
	s_tourGame.line6_label.generic.callback			= TourGame_MenuEvent; 
	s_tourGame.line6_label.width					= UI_ProportionalStringWidth((const char *) menu_button_text[MBT_TOURMENU_TEXT5][0],UI_TINYFONT);
	s_tourGame.line6_label.height					= 8;
	s_tourGame.line6_label.color					= CT_DKPURPLE1;
	s_tourGame.line6_label.color2					= CT_LTPURPLE1;
	s_tourGame.line6_label.textX					= 2;
	s_tourGame.line6_label.textY					= 1;
	s_tourGame.line6_label.textEnum					= MBT_TOURMENU_TEXT5;
	s_tourGame.line6_label.textcolor				= CT_DKGOLD1;
	s_tourGame.line6_label.textcolor2				= CT_WHITE;
	s_tourGame.line6_label.textStyle				= UI_TINYFONT;

	Menu_AddItem( &s_tourGame.menu, ( void * )&s_tourGame.mainmenu);
	Menu_AddItem( &s_tourGame.menu, ( void * )&s_tourGame.male);
	Menu_AddItem( &s_tourGame.menu, ( void * )&s_tourGame.female);
	Menu_AddItem( &s_tourGame.menu, ( void * )&s_tourGame.start);
	Menu_AddItem( &s_tourGame.menu, ( void * )&s_tourGame.resume);
	Menu_AddItem( &s_tourGame.menu, ( void * )&s_tourGame.line1_label);
	Menu_AddItem( &s_tourGame.menu, ( void * )&s_tourGame.line2_label);
	Menu_AddItem( &s_tourGame.menu, ( void * )&s_tourGame.line3_label);
	Menu_AddItem( &s_tourGame.menu, ( void * )&s_tourGame.line4_label);
	Menu_AddItem( &s_tourGame.menu, ( void * )&s_tourGame.line5_label);
	Menu_AddItem( &s_tourGame.menu, ( void * )&s_tourGame.line6_label);

	s_tourGame.male.textcolor				= CT_WHITE;
	s_tourGame.female.textcolor				= CT_BLACK;

	s_tourGame.male.generic.flags |= QMF_BLINK;

	PlayerConfig_SetMale (qtrue);

	// Precache all menu graphics in array
	UI_PrecacheMenuGraphics(tourmenu_graphics,TMG_MAX);

	tourmenu_graphics[TMG_TIMER].timer = uis.realtime + TMG_TIMER_INC;
	tourmenu_graphics[TMG_CURRENT_TEXT].target = 0;

	tourmenu_graphics[TMG_GRAPHIC1 + tourmenu_graphics[TMG_CURRENT_TEXT].target].type = MG_GRAPHIC;
	tourmenu_graphics[TMG_GRAPHIC2].type = MG_NONE;
	tourmenu_graphics[TMG_GRAPHIC3].type = MG_NONE;
	tourmenu_graphics[TMG_GRAPHIC4].type = MG_NONE;
	tourmenu_graphics[TMG_GRAPHIC5].type = MG_NONE;
	tourmenu_graphics[TMG_GRAPHIC6].type = MG_NONE;
	tourmenu_graphics[TMG_GRAPHIC7].type = MG_NONE;
	tourmenu_graphics[TMG_GRAPHIC8].type = MG_NONE;
	tourmenu_graphics[TMG_GRAPHIC9].type = MG_NONE;

	tourmenu_graphics[TMG_LOCATION_DOT2].type = MG_NONE;
	tourmenu_graphics[TMG_LOCATION_DOT3].type = MG_NONE;
	tourmenu_graphics[TMG_LOCATION_DOT4].type = MG_NONE;
	tourmenu_graphics[TMG_LOCATION_DOT5].type = MG_NONE;
}

/*
===============
UI_LeaveGameMenu
===============
*/
void UI_TourGameMenu( void) 
{

	TourGameMenu_Init(); 

	UI_PushMenu( &s_tourGame.menu);
}

