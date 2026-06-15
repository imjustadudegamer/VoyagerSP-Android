#include "ui_local.h"
#include "ui_playerinfo.h"
#include "../game/weapons.h"

#define ART_MOUSE1 "menu/common/mouse1.tga"
#define ART_MOUSE2 "menu/common/mouse2.tga"

static menuaction_s *vid_apply_action;

static void Controls_MenuEventNew (void* ptr, int event);

static struct 
{
	qhandle_t corner1;
	qhandle_t corner2;
} s_mousejoystick;


static struct 
{
	qhandle_t mon_bar;
} s_controlsother;


typedef enum 
{
	AMG_MIDLEFT,
	AMG_UPPERLEFT,
	AMG_LOWERLEFT,
	AMG_UPPERCORNER,
	AMG_LOWERCORNER,
	AMG_UPPERTOP1ST,
	AMG_LOWERTOP1ST,
	AMG_UPPERTOP2ND,
	AMG_LOWERTOP2ND,
	AMG_UPPERSWOOP,
	AMG_LOWERSWOOP,
	AMG_TOPRIGHT,
	AMG_BOTTOMRIGHT,
	AMG_PLAYERBKGRND,
	AMG_MAX
} attackmenu_graphics_t;

menugraphics_s attackmenu_graphics[AMG_MAX] = 
{
//	type		timer	x		y		width	height	file/text						graphic,	min		max	target	inc		style	color
	MG_GRAPHIC,	0.0,	158,	280,	4,		32,		"menu/common/square.tga",	0,0,	NULL,		0,		0,	0,		0,		0,		CT_VDKPURPLE1,	NULL,	// AMG_MIDLEFT
	MG_GRAPHIC,	0.0,	158,	180,	8,		97,		"menu/common/square.tga",	0,0,	NULL,		0,		0,	0,		0,		0,		CT_VDKPURPLE1,	NULL,	// AMG_UPPERLEFT
	MG_GRAPHIC,	0.0,	158,	315,	8,		100,	"menu/common/square.tga",	0,0,	NULL,		0,		0,	0,		0,		0,		CT_VDKPURPLE1,	NULL,	// AMG_LOWERLEFT
	MG_GRAPHIC,	0.0,	158,	164,	16,		 16,	"menu/common/corner_lu.tga",0,0,	NULL,		0,		0,	0,		0,		0,		CT_VDKPURPLE1,	NULL,	// AMG_UPPERCORNER
	MG_GRAPHIC,	0.0,	158,	406,	32,		 32,	"menu/common/newswoosh.tga",0,0,	NULL,		0,		0,	0,		0,		0,		CT_VDKPURPLE1,	NULL,	// AMG_LOWERCORNER

	MG_GRAPHIC,	0.0,	177,	164,	280,	  8,	"menu/common/square.tga",	0,0,	NULL,		0,		0,	0,		0,		0,		CT_VDKPURPLE1,	NULL,	// AMG_UPPERTOP1ST
	MG_GRAPHIC,	0.0,	175,	410,	282,	 18,	"menu/common/square.tga",	0,0,	NULL,		0,		0,	0,		0,		0,		CT_VDKPURPLE1,	NULL,	// AMG_LOWERTOP1ST

	MG_GRAPHIC,	0.0,	457,	164,	 34,	  8,	"menu/common/square.tga",	0,0,	NULL,		0,		0,	0,		0,		0,		CT_VDKPURPLE1,	NULL,	// AMG_UPPERTOP2ND
	MG_GRAPHIC,	0.0,	457,	410,	 34,	 18,	"menu/common/square.tga",	0,0,	NULL,		0,		0,	0,		0,		0,		CT_VDKPURPLE1,	NULL,	// AMG_LOWERTOP2ND

	MG_GRAPHIC,	0.0,	494,	164,	128,	128,	"menu/common/swoosh_top.tga",0,0,	NULL,		0,		0,	0,		0,		0,		CT_VDKPURPLE1,	NULL,	// AMG_UPPERSWOOP
	MG_GRAPHIC,	0.0,	483,	403,	128,	32,	"menu/common/newswoosh_long.tga",0,0,	NULL,		0,		0,	0,		0,		0,		CT_VDKPURPLE1,	NULL,	// AMG_LOWERSWOOP

	MG_GRAPHIC,	0.0,	501,	189,	110,	17,		"menu/common/square.tga",	0,0,	NULL,		0,		0,	0,		0,		0,		CT_VDKPURPLE1,	NULL,	// AMG_TOPRIGHT
	MG_GRAPHIC,	0.0,	501,	383,	110,	17,		"menu/common/square.tga",	0,0,	NULL,		0,		0,	0,		0,		0,		CT_VDKPURPLE1,	NULL,	// AMG_BOTTOMRIGHT


	MG_GRAPHIC,	0.0,	501,	206,	110,	177,	"menu/common/square.tga",	0,0,	NULL,		0,		0,	0,		0,		0,		CT_VDKPURPLE1,	NULL,	// AMG_PLAYERBKGRND
};


typedef struct
{
	char*	command;
	int		label;
	int		id;
	int		anim;
	int		defaultbind1;
//	int		defaultbind2;
	int		bind1;
	int		bind2;
	int		bind3;
	int		desc;
} bind_t;

typedef struct
{
	char*	name;
	float	defaultvalue;
	float	value;	
} configcvar_t;

#define SAVE_NOOP		0
#define SAVE_YES		1
#define SAVE_NO			2
#define SAVE_CANCEL		3

#define C_WEAPONS		0
#define C_QUICKKEYS		1
#define C_LOOK			2
#define C_MOVE			3
#define C_MOUSE			4
#define C_OTHER			5
#define C_MAX			6


// control sections
#define C_MOVEMENT		0
#define C_LOOKING		1
//#define C_WEAPONS		2
#define C_MISC			3
//#define C_MAX			4

#define ID_MOVEMENT		100
#define ID_LOOKING		101
#define ID_WEAPONS		102
#define ID_MISC			103
#define ID_CANCEL		104
#define ID_UNDO			105
#define ID_DEFAULTS		106
#define ID_ACCEPT		107
#define ID_LOAD			108
#define ID_SAVE			109
#define ID_CANCEL2		110

// bindable actions
#define ID_SHOWSCORES	0
#define ID_USEITEM		1	
#define ID_SPEED		2	
#define ID_FORWARD		3	
#define ID_BACKPEDAL	4
#define ID_MOVELEFT		5
#define ID_MOVERIGHT	6
#define ID_MOVEUP		7	
#define ID_MOVEDOWN		8
#define ID_LEFT			9	
#define ID_RIGHT		10	
#define ID_STRAFE		11	
#define ID_LOOKUP		12	
#define ID_LOOKDOWN		13
#define ID_MOUSELOOK	14
#define ID_CENTERVIEW	15
#define ID_ZOOMVIEW		16
#define ID_WEAPON1		17	
#define ID_WEAPON2		18	
#define ID_WEAPON3		19	
#define ID_WEAPON4		20	
#define ID_WEAPON5		21	
#define ID_WEAPON6		22	
#define ID_WEAPON7		23	
#define ID_WEAPON8		24	
#define ID_WEAPON9		25	
#define ID_WEAPON10		26
#define ID_ATTACK		27	
#define ID_ALT_ATTACK	28	
#define ID_WEAPPREV		29
#define ID_WEAPNEXT		30
#define ID_GESTURE		31
#define ID_DIE			32

#define ID_MISSIONOBJECTIVES	33
#define ID_CONSOLE				34
#define ID_USEHOLDITEM			35
#define ID_SAVEGAME				36
#define ID_LOADGAME				37
#define ID_AUTOSAVEGAME			38
#define ID_MISSIONANALYSIS		39


// all others
#define ID_INVERTMOUSE			37
#define ID_FREELOOK				38
#define ID_ALWAYSRUN			39
#define ID_AUTOSWITCH			40
#define ID_MOUSESPEED			41
#define ID_JOYENABLE			42
#define ID_JOYTHRESHOLD			43
#define ID_LOOKSPRING			44
#define ID_KEYTURNPEED			45
#define ID_DISABLE_ALTTAB		46
#define ID_CAPTIONING			47
#define ID_AUTOSWITCHWEAPONS	48


#define ANIM_IDLE		0
#define ANIM_RUN		1
#define ANIM_WALK		2
#define ANIM_BACK		3
#define ANIM_JUMP		4
#define ANIM_CROUCH		5
#define ANIM_STEPLEFT	6
#define ANIM_STEPRIGHT	7
#define ANIM_TURNLEFT	8
#define ANIM_TURNRIGHT	9
#define ANIM_LOOKUP		10
#define ANIM_LOOKDOWN	11
#define ANIM_WEAPON1	12
#define ANIM_WEAPON2	13
#define ANIM_WEAPON3	14
#define ANIM_WEAPON4	15
#define ANIM_WEAPON5	16
#define ANIM_WEAPON6	17
#define ANIM_WEAPON7	18
#define ANIM_WEAPON8	19
#define ANIM_WEAPON9	20
#define ANIM_WEAPON10	21
#define ANIM_ATTACK		22
#define ANIM_GESTURE	23
#define ANIM_DIE		24
#define ANIM_CHAT		25

static qhandle_t	swooshTop;
static qhandle_t	swooshBottom;
static menucommon_s	*chosenitem;

static menuframework_s		s_controls_menu;

// New Star Trek Stuff
static menuframework_s		s_weapons_menu;
static menubitmap_s			s_controls_mainmenu;
static menubitmap_s			s_controls_controls;
static menubitmap_s			s_controls_video;
static menubitmap_s			s_controls_sound;
static menubitmap_s			s_controls_game;
static menubitmap_s			s_controls_cdkey;
static menubitmap_s			s_controls_fonts;
static menubitmap_s			s_controls_weapon;
static menubitmap_s			s_controls_quickkey;
static menubitmap_s			s_controls_look;
static menubitmap_s			s_controls_movement;
static menubitmap_s			s_controls_mouse;
static menubitmap_s			s_controls_other;
static menubitmap_s			s_controls_loadconfig;
static menubitmap_s			s_controls_saveconfig;
static menubitmap_s			s_controls_playermdl;
static menubitmap_s			s_controls_default;

static menuframework_s		s_controlsquickkeys_menu;
static menuaction_s			s_command_advance_action;
static menuaction_s			s_command_regroup_action;
static menuaction_s			s_command_hold_action;
static menuaction_s			s_command_retreat_action;
static menuaction_s			s_command_help_action;
static menuaction_s			s_command_objectives_action;
static menuaction_s			s_mission_analysis_action;
static menuaction_s			s_skip_cinematic_action;
static menuaction_s			s_game_save;
static menuaction_s			s_game_load;
static menuaction_s			s_game_autosave;

static menuframework_s		s_controlslook_menu;

static menuframework_s		s_controlsmove_menu;
static menuaction_s			s_move_walkforward_action;
static menuaction_s			s_move_backpedal_action;
static menuaction_s			s_move_turnleft_action;
static menuaction_s			s_move_turnright_action;
static menuaction_s			s_move_run_action;
static menuaction_s			s_move_stepleft_action;
static menuaction_s			s_move_stepright_action;
static menuaction_s			s_move_sidestep_action;
static menuaction_s			s_move_moveup_action;
static menuaction_s			s_move_movedown_action;

static menuframework_s		s_controlsmouse_menu;
static menulist_s			s_freelook_box;
static menuslider_s			s_sensitivity_slider;
static menulist_s			s_invertmouse_box;
static menulist_s			s_smoothmouse_box;
static menulist_s			s_joyenable_box;
static menuslider_s			s_joythreshold_slider;
static menuslider_s			s_gplook_slider;
static menulist_s			s_forcefeedback_box;

static menuframework_s		s_controlsother_menu;
static menulist_s			s_alwaysrun_box;
static menulist_s			s_lookspring_box;
static menuslider_s			s_keyturnspeed_slider;
static menulist_s			s_captioning_box;
static menulist_s			s_autoswitch_box;

static menuframework_s		s_controlsdefault_menu;
static menubitmap_s			s_controls_default_yes;
static menubitmap_s			s_controls_default_no;


#define ID_MAINMENU		100
#define ID_CONTROLS		101
#define ID_VIDEO		102
#define ID_SOUND		103
#define ID_GAMEOPTIONS	104
#define ID_CDKEY		105
#define ID_FONTS		106


#define ID_CONTROLSQUICKKEYS	200
#define ID_CONTROLSLOOK			201
#define ID_CONTROLSMOVE			202
#define ID_CONTROLSMOUSE		203
#define ID_CONTROLSOTHER		204
#define ID_CONTROLSDEFAULT		205

#define ID_DEFAULT_YES		300
#define ID_DEFAULT_NO		301


#define ID_USE			1	
#define ID_WEAPON1		17	
#define ID_WEAPON2		18	
#define ID_WEAPON3		19	
#define ID_WEAPON4		20	
#define ID_WEAPON5		21	
#define ID_WEAPON6		22	
#define ID_WEAPON7		23	
#define ID_WEAPON8		24	
#define ID_WEAPON9		25	
#define ID_WEAPON10		26
#define ID_ATTACK		27	
#define ID_ALT_ATTACK	28	
#define ID_WEAPON_PREV	29
#define ID_WEAPON_NEXT	30



static menuaction_s			s_attack_attack_action;
static menuaction_s			s_attack_alt_attack_action;
static menuaction_s			s_spacer_action;

static menuaction_s			s_attack_use_action;
static menuaction_s			s_attack_weapon1_action;
static menuaction_s			s_attack_weapon2_action;
static menuaction_s			s_attack_weapon3_action;
static menuaction_s			s_attack_weapon4_action;
static menuaction_s			s_attack_weapon5_action;
static menuaction_s			s_attack_weapon6_action;
static menuaction_s			s_attack_weapon7_action;
static menuaction_s			s_attack_weapon8_action;
static menuaction_s			s_attack_weapon9_action;
static menuaction_s			s_attack_weapon_next_action;
static menuaction_s			s_attack_weapon_prev_action;
static menuaction_s			s_attack_apply_action;
static menuaction_s			s_attack_waiting_action;
static menuaction_s			s_look_lookup_action;
static menuaction_s			s_look_lookdown_action;
static menuaction_s			s_look_mouselook_action;
static menuaction_s			s_look_centerview_action;
static menuaction_s			s_zoomview_action;


static void* g_attack_controls[] =
{
	&s_attack_weapon1_action,
	&s_attack_weapon2_action,
	&s_attack_weapon3_action,
	&s_attack_weapon4_action,
	&s_attack_weapon5_action,
	&s_attack_weapon6_action,
	&s_attack_weapon7_action,
	&s_attack_weapon8_action,
	&s_attack_weapon9_action,
	&s_attack_weapon_next_action, 
	&s_attack_weapon_prev_action, 
	NULL,
};

static void* g_command_controls[] =
{
	&s_command_objectives_action,
	&s_mission_analysis_action,
	&s_skip_cinematic_action,
	&s_spacer_action,
	&s_game_save,
	&s_game_load,
	&s_game_autosave,
	NULL,
};


static void* g_attacklook_controls[] =
{
	&s_attack_attack_action,
	&s_attack_alt_attack_action,
	&s_attack_use_action,
	&s_spacer_action,
	&s_look_lookup_action, 
	&s_look_lookdown_action, 
	&s_look_mouselook_action, 
	&s_look_centerview_action, 
	&s_zoomview_action,
	NULL,
};

static void* g_move_controls[] =
{
	&s_move_walkforward_action,
	&s_move_backpedal_action,
	&s_move_turnleft_action,      
	&s_move_turnright_action,     
	&s_move_run_action,            
	&s_move_stepleft_action,      
	&s_move_stepright_action,     
	&s_move_sidestep_action,
	&s_move_moveup_action,        
	&s_move_movedown_action,      
//	&s_alwaysrun_box,     
	NULL
};


static void* g_mouse_controls[] =
{
	&s_freelook_box, 
	&s_sensitivity_slider,
	&s_invertmouse_box, 
	&s_smoothmouse_box,
	NULL,
};

static void* g_other_controls[] =
{
	NULL,
};

static void** g_controls[] =
{
	g_attack_controls,
	g_command_controls,
	g_attacklook_controls,
	g_move_controls,
	g_mouse_controls,
	g_other_controls
};

// Old Id stuff
//static menubitmap_s			s_controls_movement;
static menubitmap_s			s_controls_looking;
static menubitmap_s			s_controls_weapons;
static menubitmap_s			s_controls_misc;
static menubitmap_s			s_controls_load;
static menubitmap_s			s_controls_save;
static menubitmap_s			s_controls_cancel;
static menubitmap_s			s_controls_undo;
static menubitmap_s			s_controls_defaults;
static menubitmap_s			s_controls_accept;
static menubitmap_s			s_controls_frame;
static menubitmap_s			s_controls_player;
static menubitmap_s			s_controls_heading;
static menubitmap_s			s_controls_topcap;
static menubitmap_s			s_controls_botcap;
static menubitmap_s			s_controls_cancelchanges;
static int					g_section;
static qboolean				g_waitingforkey;
static menuaction_s			s_walkforward_action;
static menuaction_s			s_backpedal_action;
static menuaction_s			s_stepleft_action;
static menuaction_s			s_stepright_action;
static menuaction_s			s_moveup_action;
static menuaction_s			s_movedown_action;
static menuaction_s			s_turnleft_action;
static menuaction_s			s_turnright_action;
static menuaction_s			s_sidestep_action;
static menuaction_s			s_run_action;
static menuaction_s			s_attack_action;
static menuaction_s			s_prevweapon_action;
static menuaction_s			s_nextweapon_action;
static menuaction_s			s_lookup_action;
static menuaction_s			s_lookdown_action;
static menuaction_s			s_mouselook_action;
static menuaction_s			s_centerview_action;
static menuaction_s			s_gesture_action;
static menuaction_s			s_showscores_action;
static menuaction_s			s_useitem_action;
static playerInfo_t			g_playerinfo;
static qboolean				g_changesmade;
static menubitmap_s			s_controls_no;
static menubitmap_s			s_controls_yes;
static menubitmap_s			s_controls_slash;
static qboolean				g_prompttosave;
static menuaction_s			s_die_action;
static menuaction_s			s_chat_action;
static menuaction_s			s_chat2_action;
static menuaction_s			s_chat3_action;
static menuaction_s			s_chat4_action;


static bind_t g_bindings[] = 
{
	{"+scores",			MNT_SHORTCUT_SCORES,		ID_SHOWSCORES,	ANIM_IDLE,		K_TAB,			-1,		-1, -1,	MNT_SHORTCUT_KEY},
	{"+use",			MNT_SHORTCUT_USE,			ID_USEITEM,		ANIM_IDLE,		K_ENTER,		-1,		-1, -1,	MNT_SHORTCUT_KEY},
	{"+speed", 			MNT_SHORTCUT_RUNWALK,		ID_SPEED,		ANIM_RUN,		K_SHIFT,		-1,		-1,	-1,	MNT_SHORTCUT_KEY},
	{"+forward", 		MNT_SHORTCUT_WALKFORWARD,	ID_FORWARD,		ANIM_WALK,		K_UPARROW,		-1,		-1, -1,	MNT_SHORTCUT_KEY},
	{"+back", 			MNT_SHORTCUT_BACKPEDAL,		ID_BACKPEDAL,	ANIM_BACK,		K_DOWNARROW,	-1,		-1, -1,	MNT_SHORTCUT_KEY},
	{"+moveleft", 		MNT_SHORTCUT_STEPLEFT,		ID_MOVELEFT,	ANIM_STEPLEFT,	',',			-1,		-1, -1,	MNT_SHORTCUT_KEY},
	{"+moveright", 		MNT_SHORTCUT_STEPRIGHT,		ID_MOVERIGHT,	ANIM_STEPRIGHT,	'.',			-1,		-1, -1,	MNT_SHORTCUT_KEY},
	{"+moveup",			MNT_SHORTCUT_UPJUMP,		ID_MOVEUP,		ANIM_JUMP,		K_SPACE,		-1,		-1, -1,	MNT_SHORTCUT_KEY},
	{"+movedown",		MNT_SHORTCUT_DOWNCROUCH,	ID_MOVEDOWN,	ANIM_CROUCH,	'c',			-1,		-1, -1,	MNT_SHORTCUT_KEY},
	{"+left", 			MNT_SHORTCUT_TURNLEFT,		ID_LEFT,		ANIM_TURNLEFT,	K_LEFTARROW,	-1,		-1, -1,	MNT_SHORTCUT_KEY},
	{"+right", 			MNT_SHORTCUT_TURNRIGHT,		ID_RIGHT,		ANIM_TURNRIGHT,	K_RIGHTARROW,	-1,		-1, -1,	MNT_SHORTCUT_KEY},
	{"+strafe", 		MNT_SHORTCUT_SIDESTEPTURN,	ID_STRAFE,		ANIM_IDLE,		K_ALT,			-1,		-1, -1,	MNT_SHORTCUT_KEY},
	{"+lookup", 		MNT_SHORTCUT_LOOKUP,		ID_LOOKUP,		ANIM_LOOKUP,	K_PGDN,			-1,		-1, -1,	MNT_SHORTCUT_KEY},
	{"+lookdown", 		MNT_SHORTCUT_LOOKDOWN,		ID_LOOKDOWN,	ANIM_LOOKDOWN,	K_DEL,			-1,		-1, -1,	MNT_SHORTCUT_KEY},
	{"+mlook", 			MNT_SHORTCUT_MOUSELOOK,		ID_MOUSELOOK,	ANIM_IDLE,		'/',			-1,		-1, -1,	MNT_SHORTCUT_KEY},
	{"centerview", 		MNT_SHORTCUT_CENTERVIEW,	ID_CENTERVIEW,	ANIM_IDLE,		K_END,			-1,		-1, -1,	MNT_SHORTCUT_KEY},
	{"+zoom", 			MNT_SHORTCUT_ZOOMVIEW,		ID_ZOOMVIEW,	ANIM_IDLE,		-1,				-1,		-1, -1,	MNT_SHORTCUT_KEY},
	{"weapon 1",		MNT_SHORTCUT_WEAPON1,		ID_WEAPON1,		ANIM_WEAPON1,	'1',			-1,		-1, -1,	MNT_SHORTCUT_KEY},
	{"weapon 2",		MNT_SHORTCUT_WEAPON2,		ID_WEAPON2,		ANIM_WEAPON2,	'2',			-1,		-1, -1,	MNT_SHORTCUT_KEY},
	{"weapon 3",		MNT_SHORTCUT_WEAPON3,		ID_WEAPON3,		ANIM_WEAPON3,	'3',			-1,		-1, -1,	MNT_SHORTCUT_KEY},
	{"weapon 4",		MNT_SHORTCUT_WEAPON4,		ID_WEAPON4,		ANIM_WEAPON4,	'4',			-1,		-1, -1,	MNT_SHORTCUT_KEY},
	{"weapon 5",		MNT_SHORTCUT_WEAPON5,		ID_WEAPON5,		ANIM_WEAPON5,	'5',			-1,		-1, -1,	MNT_SHORTCUT_KEY},
	{"weapon 6",		MNT_SHORTCUT_WEAPON6,		ID_WEAPON6,		ANIM_WEAPON6,	'6',			-1,		-1, -1,	MNT_SHORTCUT_KEY},
	{"weapon 7",		MNT_SHORTCUT_WEAPON7,		ID_WEAPON7,		ANIM_WEAPON7,	'7',			-1,		-1, -1,	MNT_SHORTCUT_KEY},
	{"weapon 8",		MNT_SHORTCUT_WEAPON8,		ID_WEAPON8,		ANIM_WEAPON8,	'8',			-1,		-1, -1,	MNT_SHORTCUT_KEY},
	{"weapon 9",		MNT_SHORTCUT_WEAPON9,		ID_WEAPON9,		ANIM_WEAPON9,	'9',			-1,		-1, -1,	MNT_SHORTCUT_KEY},
	{"weapon 10",		MNT_SHORTCUT_WEAPON10,		ID_WEAPON10,	ANIM_WEAPON10,	'0',			-1,		-1, -1,	MNT_SHORTCUT_KEY},
	{"+attack", 		MNT_SHORTCUT_ATTACK,		ID_ATTACK,		ANIM_ATTACK,	K_CTRL,			-1,		-1, -1,	MNT_SHORTCUT_KEY},
	{"+altattack", 		MNT_SHORTCUT_ALTATTCK,		ID_ALT_ATTACK,	ANIM_ATTACK,	K_MOUSE2,		-1,		-1, -1,	MNT_SHORTCUT_KEY},
	{"weapprev",		MNT_SHORTCUT_PREVWEAPON,	ID_WEAPPREV,	ANIM_IDLE,		'[',			-1,		-1, -1,	MNT_SHORTCUT_KEY},
	{"weapnext", 		MNT_SHORTCUT_NEXTWEAPON,	ID_WEAPNEXT,	ANIM_IDLE,		']',			-1,		-1, -1,	MNT_SHORTCUT_KEY},
	{"+button3", 		MNT_SHORTCUT_GESTURE,		ID_GESTURE,		ANIM_GESTURE,	K_MOUSE3,		-1,		-1, -1,	MNT_SHORTCUT_KEY},
	{"kill", 			MNT_SHORTCUT_DIE,			ID_DIE,			ANIM_DIE,		-1,				-1,		-1, -1,	MNT_SHORTCUT_KEY},

	{"+info", 			MNT_SHORTCUT_MISSIONINFO,	ID_MISSIONOBJECTIVES,ANIM_DIE,	-1,				-1,		-1, -1,	MNT_SHORTCUT_KEY},
	{"+console", 		MNT_SHORTCUT_CONSOLE,		ID_CONSOLE,		ANIM_DIE,		-1,				-1,		-1, -1,	MNT_SHORTCUT_KEY},
	{"+button2",		MNT_SHORTCUT_USE_ITEM,		ID_USEHOLDITEM,	ANIM_IDLE,		K_ENTER,		-1,		-1, -1,	MNT_SHORTCUT_KEY},
	{"ui_teamOrders",	MNT_SHORTCUT_SAVEGAME,		ID_SAVEGAME,	ANIM_IDLE,		K_F3,			-1,		-1, -1,	MNT_SHORTCUT_KEY},
	{"levelselect",		MNT_SHORTCUT_LOADGAME,		ID_LOADGAME,	ANIM_IDLE,		K_F4,			-1,		-1, -1,	MNT_SHORTCUT_KEY},
	{"save quik*",		MNT_SHORTCUT_AUTOSAVEGAME,	ID_AUTOSAVEGAME,ANIM_IDLE,		-1,				-1,		-1, -1,	MNT_SHORTCUT_KEY},
	{"+analysis",		MNT_SHORTCUT_MISSIONANALYSIS,ID_MISSIONANALYSIS,ANIM_IDLE,	-1,				-1,		-1, -1,	MNT_SHORTCUT_KEY},

	{(char*)NULL,		NULL,						0,				0,				-1,				-1,		-1,	-1,	0},
};
//the array above must match the enums, like ID_MISSIONANALYSIS


static configcvar_t g_configcvars[] =
{
	{"cl_run",			0,					0},
	{"m_pitch",			0,					0},
	{"cl_freelook",		0,					0},
	{"cg_autoswitch",	0,					0},
	{"sensitivity",		0,					0},
	{"in_joystick",		0,					0},
	{"joy_threshold",	0,					0},
	{"gp_lookSens",		0,					0},	// MUST be here: Controls_GetCvarValue reads this cached table, not the live cvar; missing entry -> reads 0 -> slider always loads at minimum on menu re-open
	{"m_filter",		0,					0},
	{"cl_anglespeedkey",0,					0},
	{"use_ff",			0,					0},
	{"joy_xbutton",		0,					0},
	{"joy_ybutton",		0,					0},
	{NULL,				0,					0}
};
/*
static void** g_controls[] =
{
	g_movement_controls,
	g_looking_controls,
	g_weapons_controls,
	g_misc_controls,
};
*/

float setup_menubuttons[6][2] = 
{
{125,62},
{125,86},
{125,109},
{300,62},
{300,86},
{300,109},
};

static char		playerLModel[64];
static char		playerHModel[64];
static vec3_t	playerViewangles;
static vec3_t	playerMoveangles;
static int		playerLegs;
static int		playerTorso;
static int		playerWeapon;
static qboolean	playerChat;


static void Controls_UpdateNew( void );
static void UI_ControlsQuickKeysMenu( void );
static void SetupMenu_SideButtons(menuframework_s *menu,int menuType);
static void UI_ControlsAttackLookMenu( void );
static void UI_ControlsMoveMenu( void );
static void UI_ControlsMouseJoyStickMenu( void );
static void UI_ControlsOtherMenu( void );
static void UI_ControlsDefaultMenu( void );


/*
=================
FreeLookCallback
=================
*/
static void FreeLookCallback( void *s, int notification )
{
	menulist_s *s_freelook_box = (menulist_s *) s;

	if (notification != QM_ACTIVATED)
		return;

	// Set the value
	ui.Cvar_SetValue( "cl_freelook", s_freelook_box->curvalue );

	// Based on that value set mouse look on or off
	if ( s_freelook_box->curvalue )
	{
		s_look_mouselook_action.generic.flags |= QMF_GRAYED;
	}
	else
	{
		s_look_mouselook_action.generic.flags &= ~QMF_GRAYED;
	}

}


/*
=================
FreeLookGet
=================
*/
static void FreeLookGet(menulist_s *s)
{
	menulist_s *s_freelook_box = s;

	s_freelook_box->curvalue = ui.Cvar_VariableValue( "cl_freelook" );

	// Based on that value set mouse look on or off
	if ( s_freelook_box->curvalue )
	{
		s_look_mouselook_action.generic.flags |= QMF_GRAYED;
	}
	else
	{
		s_look_mouselook_action.generic.flags &= ~QMF_GRAYED;
	}

}


/*
=================
Controls_ClampCvar
=================
*/
static float Controls_ClampCvar( float min, float max, float value )
{
	if ( value < min ) return min;
	if ( value > max ) return max;
	return value;
}

/*
=================
Controls_InitCvars
=================
*/
static void Controls_InitCvars( void )
{
	int				i;
	configcvar_t*	cvarptr;

	cvarptr = g_configcvars;
	for (i=0; ;i++,cvarptr++)
	{
		if (!cvarptr->name)
			break;

		// get current value
		cvarptr->value = ui.Cvar_VariableValue( cvarptr->name );

		// get default value
		ui.Cvar_Reset( cvarptr->name );
		cvarptr->defaultvalue = ui.Cvar_VariableValue( cvarptr->name );

		// restore current value
		ui.Cvar_SetValue( cvarptr->name, cvarptr->value );
	}
}

/*
=================
Controls_GetCvarDefault
=================
*/
static float Controls_GetCvarDefault( char* name )
{
	configcvar_t*	cvarptr;
	int				i;

	cvarptr = g_configcvars;
	for (i=0; ;i++,cvarptr++)
	{
		if (!cvarptr->name)
			return (0);

		if (!strcmp(cvarptr->name,name))
			break;
	}

	return (cvarptr->defaultvalue);
}

/*
=================
Controls_GetCvarValue
=================
*/
static float Controls_GetCvarValue( char* name )
{
	configcvar_t*	cvarptr;
	int				i;

	cvarptr = g_configcvars;
	for (i=0; ;i++,cvarptr++)
	{
		if (!cvarptr->name)
			return (0);

		if (!strcmp(cvarptr->name,name))
			break;
	}

	return (cvarptr->value);
}

/*
=================
Controls_UpdateModel
=================
*/
void Controls_UpdateModel( int anim )
{
	VectorClear( playerViewangles );
	VectorClear( playerMoveangles );
//	playerViewangles[YAW]	= 180 + 45;
	playerViewangles[YAW]	= 180 + 15;
	playerMoveangles[YAW]	= playerViewangles[YAW];
	playerLegs				= BOTH_STAND1;
	playerTorso				= BOTH_STAND1;
	playerWeapon			= -1;
	playerChat				= qfalse;

	switch (anim)
	{
		case ANIM_RUN:	
			playerLegs = BOTH_RUN1;
			playerTorso = BOTH_RUN1;
			break;

		case ANIM_WALK:	
			playerLegs = BOTH_WALK1;
			playerTorso = BOTH_WALK1;
			break;

		case ANIM_BACK:	
			playerLegs = LEGS_RUNBACK1;
			break;

		case ANIM_JUMP:	
			playerLegs = BOTH_JUMP1;
			playerTorso = BOTH_JUMP1;
			break;

		case ANIM_CROUCH:	
			playerLegs = BOTH_CROUCH1IDLE;
			playerTorso = BOTH_CROUCH1IDLE;
			break;

		case ANIM_TURNLEFT:
			playerViewangles[YAW] += 90;
			break;

		case ANIM_TURNRIGHT:
			playerViewangles[YAW] -= 90;
			break;

		case ANIM_STEPLEFT:
			playerLegs = BOTH_WALK1;
			playerMoveangles[YAW] = playerViewangles[YAW] + 90;
			break;

		case ANIM_STEPRIGHT:
			playerLegs = BOTH_WALK1;
			playerMoveangles[YAW] = playerViewangles[YAW] - 90;
			break;

		case ANIM_LOOKUP:
			playerViewangles[PITCH] = -45;
			break;

		case ANIM_LOOKDOWN:
			playerViewangles[PITCH] = 45;
			break;

		case ANIM_WEAPON1:
		case ANIM_WEAPON2:
		case ANIM_WEAPON3:
		case ANIM_WEAPON4:
		case ANIM_WEAPON5:
		case ANIM_WEAPON6:
		case ANIM_WEAPON7:
		case ANIM_WEAPON8:
		case ANIM_WEAPON9:
		case ANIM_WEAPON10:
			playerWeapon = WP_PHASER + anim - ANIM_WEAPON1;
			break;

		case ANIM_ATTACK:
			playerTorso = BOTH_ATTACK1;
			break;

		case ANIM_GESTURE:
			playerTorso = BOTH_GESTURE1;
			break;

		case ANIM_DIE:
			playerLegs = BOTH_DEATH1;
			playerTorso = BOTH_DEATH1;
			playerWeapon = WP_NONE;
			break;

		case ANIM_CHAT:
			playerChat = qtrue;
			break;

		default:
			break;
	}

	UI_PlayerInfo_SetInfo( &g_playerinfo, playerLegs, playerTorso, playerViewangles, playerMoveangles, (weapon_t) playerWeapon, playerChat );
}

/*
=================
Controls_DrawKeyBinding
=================
*/
static void Controls_DrawKeyBinding( void *self )
{
	menuaction_s*	a;
	int				x,bindingX;
	int				y;
	int				b1;
	int				b2;
	qboolean		c;
	// name accumulates "bind1 OR bind2 ..." — must hold two long key names. Gamepad
	// binds like "PAD0_RIGHTSTICK_RIGHT" (21 chars) overflowed the old name[32] via
	// the strcat below (FORTIFY abort on the controls menu); size for two + separators.
	char			name[96];
	char			name2[32];
	int				color,bindingtextcolor, buttontextcolor;
	int				width;
	char			*text;

	a = (menuaction_s*) self;

	if (a->generic.flags & QMF_HIDDEN)	// It's bloody invisible
	{
		return;
	}

	x =	a->generic.x;
	y = a->generic.y;

	c = (Menu_ItemAtCursor( a->generic.parent ) == a);

	// Set up bindings
	b1 = g_bindings[a->generic.id].bind1;
	if (b1 == -1)
		strcpy(name,"???");
	else
	{
		ui.Key_KeynumToStringBuf( b1, name, 32 );
		Q_strupr(name);

		b2 = g_bindings[a->generic.id].bind2;
		if (b2 != -1)
		{
			ui.Key_KeynumToStringBuf( b2, name2, 32 );
			Q_strupr(name2);
			strcat( name, va(" %s ",menu_normal_text[MNT_OR]));
			strcat( name, name2 );
			if (g_bindings[a->generic.id].bind3>=0)
			{
				strcat( name," ...");
			}
		}
	}

	// Waiting for key input, and this isn't the chosen key so gray it out
	if ((g_waitingforkey) && (a != (menuaction_s*) chosenitem))
	{
		color = CT_MDGREY;
		bindingtextcolor = CT_MDGREY;
		buttontextcolor = CT_BLACK;
		a->generic.flags |= QMF_INACTIVE;
	}
	else	// Normal key colors
	{
		a->generic.flags &= ~QMF_INACTIVE;
		bindingtextcolor = CT_WHITE;

		// Keep the chosenitem lit up even when mouse isn't over it
		// or if mouse is over it
		if (((g_waitingforkey) && (a == (menuaction_s*) chosenitem)) || (c))
		{
			buttontextcolor = CT_WHITE;
			color = CT_LTORANGE;
		}
		else
		{
			buttontextcolor = CT_BLACK;
			color = CT_DKORANGE;
		}
	}	

	if ((c) && (g_bindings[a->generic.id].desc))
	{
		text = menu_normal_text[g_bindings[a->generic.id].desc];
		UI_DrawProportionalString( a->generic.parent->descX, a->generic.parent->descY,text, UI_LEFT|UI_TINYFONT, colorTable[CT_BLACK]);

	}


	width = a->width;

	if (!width)
	{
		// Print button
		ui.R_SetColor( colorTable[color]);
		UI_DrawHandlePic( x , y,  19,  MENU_BUTTON_MED_HEIGHT, uis.graphicButtonLeftEnd);
		UI_DrawHandlePic( x + 8 + (SMALLCHAR_WIDTH * 11), y,  -19,  MENU_BUTTON_MED_HEIGHT, uis.graphicButtonLeftEnd);	//right
		UI_DrawHandlePic( x + 8,  y,  (SMALLCHAR_WIDTH * 11),  MENU_BUTTON_MED_HEIGHT, uis.whiteShader);
		ui.R_SetColor( NULL );
		bindingX =x + (SMALLCHAR_WIDTH * 12);
	}
	else
	{
		width -= 8 + 8;

		// Print button
		ui.R_SetColor( colorTable[color]);
		// Left end
		UI_DrawHandlePic( x , y,  16,  MENU_BUTTON_MED_HEIGHT, uis.graphicButtonLeftEnd);
		// Right end
		UI_DrawHandlePic( x + width, y,  -16,  MENU_BUTTON_MED_HEIGHT, uis.graphicButtonLeftEnd);	//right
		// Middle
		UI_DrawHandlePic( x + 8,  y,  width,  MENU_BUTTON_MED_HEIGHT, uis.whiteShader);

		bindingX = x + 8 + width + 8 + 6;

		ui.R_SetColor( NULL );
	}

	// Button text
	if (g_bindings[a->generic.id].label)
	{
		UI_DrawProportionalString( x + a->textX, y + a->textY, menu_normal_text[g_bindings[a->generic.id].label], UI_LEFT|UI_SMALLFONT, colorTable[buttontextcolor] );
	}

	// Binding text
	UI_DrawProportionalString(  bindingX, y, name, UI_SMALLFONT, colorTable[bindingtextcolor] );

}

/*
=================
Controls_StatusBar
=================
*/
static void Controls_StatusBar( void *self )
{
	UI_DrawProportionalString(SCREEN_WIDTH * 0.50, SCREEN_HEIGHT * 0.80, "Use Arrow Keys or CLICK to change", UI_SMALLFONT|UI_CENTER, menu_highlight_color );
}

/*
=================
Controls_DrawPlayer
=================
*/
void Controls_DrawPlayer( void *self )
{
	menubitmap_s*	b;
	int				x;
	int				y;
	char			legsmodel[64];
	char			headmodel[64];

	b = (menubitmap_s*) self;
	x =	b->generic.x;
	y = b->generic.y;

	ui.Cvar_VariableStringBuffer( "legsmodel", legsmodel, sizeof( legsmodel ) );
	ui.Cvar_VariableStringBuffer( "headmodel", headmodel, sizeof( headmodel ) );

	if ( (strcmp( legsmodel, playerLModel ) != 0 ) && (strcmp( headmodel, playerHModel ) != 0 ) ) 
	{
		UI_PlayerInfo_SetModel( &g_playerinfo, legsmodel, headmodel );
		strcpy( playerLModel, legsmodel );
		strcpy( playerHModel, headmodel );
		UI_PlayerInfo_SetInfo( &g_playerinfo, playerLegs, playerTorso, playerViewangles, playerMoveangles, (weapon_t) playerWeapon, playerChat );
	}

	UI_DrawPlayer( x, y, b->width, b->height, &g_playerinfo, uis.realtime/2 );
}

/*
=================
Controls_GetKeyAssignment
=================
*/
static void Controls_GetKeyAssignment (char *command, int *twokeys)
{
	int		count;
	int		j;
	char	b[256];

	twokeys[0] = twokeys[1] = twokeys[2] = -1;
	count = 0;

	// scan the full engine keyspace (K_CHAR_FLAG, not 256) so gamepad PAD0_*
	// binds — whose keycodes live above 256 — are found and shown in the menu.
	for ( j = 0; j < K_CHAR_FLAG; j++ )
	{
		ui.Key_GetBindingBuf( j, b, 256 );
		if ( *b == 0 ) {
			continue;
		}
		if ( !Q_stricmp( b, command ) ) {
			twokeys[count] = j;
			count++;
			if (count == 3)
				break;
		}
	}
}

/*
=================
Controls_ClearKeyAssignment
=================
*/
static void Controls_ClearKeyAssignment (char *command)
{
	int		i;
	char	b[256];

	// K_CHAR_FLAG, not 256, so a command's gamepad PAD0_* bind (keycode > 256)
	// is also cleared when the user reassigns it.
	for ( i = 0; i < K_CHAR_FLAG; i++ )
	{
		ui.Key_GetBindingBuf( i, b, 256 );
		if ( *b == 0 )
		{
			continue;
		}
		if ( !Q_stricmp( b, command ) )
		{
			ui.Key_SetBinding( i, "" );
		}
	}


}

/*
=================
Controls_GetConfig
=================
*/
static void Controls_GetConfig( void )
{
	int		i;
	int		twokeys[3];
	bind_t*	bindptr;

	// put the bindings into a local store
	bindptr = g_bindings;

	// iterate each command, get its numeric binding
	for (i=0; ;i++,bindptr++)
	{
		if (!bindptr->label)
			break;

		Controls_GetKeyAssignment(bindptr->command, twokeys);

		bindptr->bind1 = twokeys[0];
		bindptr->bind2 = twokeys[1];
		bindptr->bind3 = twokeys[2];
	}

	s_alwaysrun_box.curvalue       = Controls_ClampCvar( 0, 1, Controls_GetCvarValue( "cl_run" ) );
	s_autoswitch_box.curvalue      = Controls_ClampCvar( 0, 2, Controls_GetCvarValue( "cg_autoswitch" ) );
	s_joyenable_box.curvalue       = Controls_ClampCvar( 0, 1, Controls_GetCvarValue( "in_joystick" ) );
	s_joythreshold_slider.curvalue = Controls_ClampCvar( 0.05f, 0.75f, Controls_GetCvarValue( "joy_threshold" ) );
	s_gplook_slider.curvalue       = Controls_ClampCvar( 1, 10, ( Controls_GetCvarValue( "gp_lookSens" ) - 0.25f ) / 0.25f + 1.0f );	// cvar 0.25..2.5 -> notch 1..10
	s_keyturnspeed_slider.curvalue = Controls_ClampCvar( 1, 5, Controls_GetCvarValue( "cl_anglespeedkey" ) );
}

/*
=================
Controls_SetConfig
=================
*/
static void Controls_SetConfig( void )
{
	int		i;
	bind_t*	bindptr;

	// set the bindings from the local store
	bindptr = g_bindings;

	// iterate each command, get its numeric binding
	for (i=0; ;i++,bindptr++)
	{
		if (!bindptr->label)
			break;

		if (bindptr->bind1 != -1)
		{	
			ui.Key_SetBinding( bindptr->bind1, bindptr->command );

			if (bindptr->bind2 != -1)
				ui.Key_SetBinding( bindptr->bind2, bindptr->command );
		}
	}

	ui.Cvar_SetValue( "cl_run", s_alwaysrun_box.curvalue );
	ui.Cvar_SetValue( "cg_autoswitch", s_autoswitch_box.curvalue );
	ui.Cvar_SetValue( "in_joystick", s_joyenable_box.curvalue );
	ui.Cvar_SetValue( "joy_threshold", s_joythreshold_slider.curvalue );
	ui.Cvar_SetValue( "gp_lookSens", 0.25f + ( s_gplook_slider.curvalue - 1.0f ) * 0.25f );	// notch 1..10 -> cvar 0.25..2.5
	ui.Cvar_SetValue( "cl_anglespeedkey", s_keyturnspeed_slider.curvalue );

}

/*
=================
Controls_MenuKey
=================
*/
static sfxHandle_t Controls_MenuKey( int key )
{
	int			id;
	int			i;
	qboolean	found;
	bind_t*		bindptr;
	menuframework_s *current_menu;
	found = qfalse;

	switch (g_section)
	{
		case C_WEAPONS:
			current_menu = &s_weapons_menu;
			break;
		case C_QUICKKEYS:
			current_menu = &s_controlsquickkeys_menu;
			break;
		case C_LOOK:
			current_menu = &s_controlslook_menu;
			break;
		case C_MOVE:
			current_menu = &s_controlsmove_menu;
			break;
		case C_MOUSE:
			current_menu = &s_controlsmouse_menu;
			break;
	}


/*	if (g_prompttosave)
	{
		if (key == K_ESCAPE || key == K_MOUSE2)
		{
			g_prompttosave = qfalse;
			Controls_UpdateNew();
			return (menu_out_sound);
		}
		else
			goto ignorekey;
	}
*/
	// Not waiting for input
	if (!g_waitingforkey)
	{
		switch (key)
		{
			case K_BACKSPACE:
			case K_DEL:
			case K_KP_DEL:
				key = -1;
				break;
		
			case K_MOUSE2:
			case K_ESCAPE:

				if (!g_changesmade)
				{
					goto ignorekey;
				}
				else
				{
					g_prompttosave = qtrue;
					Controls_UpdateNew();
					return (menu_out_sound);
				}
				break;

			default:
				goto ignorekey;
		}
	}
	// Waiting for input
	else
	{
		if (key & K_CHAR_FLAG)
			goto ignorekey;

		// Accept the engine's extended keycodes (K_PAD0_* gamepad buttons live
		// above 256 in the engine's modern keymap; this UI's own enum predates
		// them but only ever round-trips the raw integer back to the engine for
		// naming/binding). The char-flagged keys are already filtered above, so
		// the only meaningful ceiling is K_CHAR_FLAG. Without this, pressing a
		// gamepad button while rebinding was silently rejected.
		if ((key<1) || (key>=K_CHAR_FLAG))
		{
			return (menu_null_sound);
		}

		switch (key)
		{
//			case K_BACKSPACE:
//				key = -1;
//				break;

			case K_ESCAPE:
				g_waitingforkey = qfalse;
				ui.Cvar_Set( "ui_bindGrab", "0" );   // grab cancelled

				// Turn off the waiting for key message
				s_attack_waiting_action.generic.flags= QMF_HIDDEN;

				if (g_changesmade)
				{
					s_attack_apply_action.generic.flags &= ~QMF_GRAYED;
					s_attack_apply_action.generic.flags = QMF_BLINK | QMF_HIGHLIGHT_IF_FOCUS;
				}

				Controls_UpdateNew();
				return (menu_out_sound);
	
			case '`':
				goto ignorekey;
		}
	}

	g_changesmade = qtrue;
	
	if (key != -1)
	{
		// remove from any other bind
		bindptr = g_bindings;
		for (i=0; ;i++,bindptr++)
		{
			if (!bindptr->label)	
				break;

			if (bindptr->bind3 == key)
			{
				bindptr->bind3 = -1;
			}
			else if (bindptr->bind2 == key)
			{
				bindptr->bind2 = bindptr->bind3;
				bindptr->bind3 = -1;
			}
			else if (bindptr->bind1 == key)
			{
				bindptr->bind1 = bindptr->bind2;	
				bindptr->bind2 = bindptr->bind3;	
				bindptr->bind3 = -1;
			}
		}
	}

	// Show this control has been updated
//	((menuaction_s*)(current_menu->items[current_menu->cursor]))->updated = 1;
	// Make Accept changes key blink
//	s_attack_apply_action.generic.flags &= ~QMF_GRAYED;
//	s_attack_apply_action.generic.flags = QMF_BLINK |QMF_HIGHLIGHT_IF_FOCUS;


	// Turn off the waiting for key message
	s_attack_waiting_action.generic.flags			= QMF_HIDDEN;

	// assign key to local store
	id      = ((menucommon_s*)(current_menu->items[current_menu->cursor]))->id;
	bindptr = g_bindings;
	for (i=0; ;i++,bindptr++)
	{
		if (!bindptr->label)	
			break;
		
		if (bindptr->id == id)
		{
			found = qtrue;
			if (key == -1)
			{
				Controls_ClearKeyAssignment (bindptr->command);
				bindptr->bind1 = -1;
				bindptr->bind2 = -1;
				bindptr->bind3 = -1;
/*
				if( bindptr->bind1 != -1 ) 
				{
					ui.Key_SetBinding( bindptr->bind1, "" );
					bindptr->bind1 = -1;
				}
				if( bindptr->bind2 != -1 ) 
				{
					ui.Key_SetBinding( bindptr->bind2, "" );
					bindptr->bind2 = -1;
				} */
			}
			else if (bindptr->bind1 == -1)
				bindptr->bind1 = key;
			else if (bindptr->bind1 != key && bindptr->bind2 == -1)
				bindptr->bind2 = key;
			else
			{
				bindptr->bind3 = bindptr->bind2;
				bindptr->bind2 = bindptr->bind1;
				bindptr->bind1 = key;
			}						
			break;
		}
	}				
		
	g_waitingforkey = qfalse;
	g_changesmade = qfalse;
	ui.Cvar_Set( "ui_bindGrab", "0" );   // key captured; stop raw-pad delivery

	if (found)
	{	
		Controls_SetConfig();	// Bind new key

		Controls_UpdateNew();
		return (menu_out_sound);
	}

ignorekey:
	return Menu_DefaultKey( current_menu, key );
}

/*
=================
Controls_ActionEvent
=================
*/
static void Controls_ActionEvent( void* ptr, int event )
{
	menuframework_s*	menu;

	if (event == QM_LOSTFOCUS)
	{
		Controls_UpdateModel( ANIM_IDLE );
	}
	else if (event == QM_GOTFOCUS)
	{
		Controls_UpdateModel( g_bindings[((menucommon_s*)ptr)->id].anim );
	}
	else if ((event == QM_ACTIVATED) && !g_waitingforkey)
	{
		g_waitingforkey = qtrue;
		// Tell the engine input layer to deliver raw gamepad keycodes (K_PAD0_*)
		// instead of translating pad buttons to menu-nav keys, so a button press
		// here is captured as the new binding. Cleared when the grab ends.
		ui.Cvar_Set( "ui_bindGrab", "1" );
		// Show your waiting for data
		s_attack_waiting_action.generic.flags &= ~QMF_HIDDEN;
		s_attack_waiting_action.generic.flags |= QMF_BLINK;
		// Turn off 'apply action' so it won't flash
		s_attack_apply_action.generic.flags = QMF_GRAYED;

		// Show this it the chosen item
		menu = ((menucommon_s*)ptr)->parent;
		chosenitem = (menucommon_s *) Menu_ItemAtCursor(((menuframework_s*)menu));

		Controls_UpdateNew();
	}
}

/*
=================
Controls_ModelEvent
=================
*/
static void Controls_ModelEvent( void* ptr, int event )
{
	menucommon_s*	menu = (menucommon_s*)ptr;
	if (event == QM_LOSTFOCUS)
	{
		g_playerinfo.looking = qfalse;
	}
	else if (event == QM_GOTFOCUS)
	{
		g_playerinfo.looking = menu->x;
	}
	else if ((event == QM_ACTIVATED) )
	{

	}
}

/*
=================
Controls_InitModel
=================
*/
static void Controls_InitModel( void )
{
	char			headmodel[64];

	memset( &g_playerinfo, 0, sizeof(playerInfo_t) );

	ui.Cvar_VariableStringBuffer( "headmodel", headmodel, sizeof( headmodel ) );
	UI_PlayerInfo_SetModel( &g_playerinfo, UI_Cvar_VariableString( "legsmodel" ), headmodel );

	Controls_UpdateModel( ANIM_IDLE );
}

/*
=================
Controls_InitWeapons
=================
*/
static void Controls_InitWeapons( void ) {
	int		i;
	char		path[MAX_QPATH];
	for ( i = 1 ; i <= MAX_PLAYER_WEAPONS ; i++ ) 
	{
		strcpy( path, weaponData[i].weaponMdl );
		COM_StripExtension( path, path );
		strcat( path, "_w.md3" );
		ui.R_RegisterModel( path);
	}

#if 0
	gitem_t *	item;
	for ( item = bg_itemlist + 1 ; item->classname ; item++ ) {
		if ( item->giType != IT_WEAPON ) {
			continue;
		}
		ui.R_RegisterModel( item->world_model[0] );
	}
#endif
}


/*
=================
Controls_Update
=================
*/
static void Controls_UpdateNew( void )
{
	int			i;
	int			j;
	void**		controlptr;
	menuframework_s *current_menu;


	switch (g_section)
	{
		case C_WEAPONS:
			current_menu = &s_weapons_menu;
			break;
		case C_QUICKKEYS:
			current_menu = &s_controlsquickkeys_menu;
			break;
		case C_LOOK:
			current_menu = &s_controlslook_menu;
			break;
		case C_MOVE:
			current_menu = &s_controlsmove_menu;
			break;
		case C_MOUSE:
			current_menu = &s_controlsmouse_menu;
			break;
	}


	// enable specified controls
	controlptr = g_controls[g_section];
	for (i=0,j=0; ;i++,j++,controlptr++)
	{
		if (!controlptr[0])
		{
			// end of list
			break;
		}
	
		((menucommon_s*)controlptr[0])->flags &= ~(QMF_GRAYED);
	}


	if (g_waitingforkey)
	{
		((menucommon_s*)(current_menu->items[current_menu->cursor]))->flags &= ~QMF_HIGHLIGHT;
		return;
	}

}


static void *holdControlPtr;
static int holdControlEvent;

void ControlsVideoDataAction( qboolean result ) 
{
	if ( result )	// Yes - do it
	{
		Controls_MenuEventNew (holdControlPtr, holdControlEvent);
	}
}

/*
=================
Controls_MenuEventNew
=================
*/
static void Controls_MenuEventVideo (void* ptr, int event)
{

	if (event != QM_ACTIVATED)
		return;

	holdControlPtr = ptr;
	holdControlEvent = event;

	if (vid_apply_action->generic.flags & QMF_BLINK)	// Video apply changes button is flashing
	{
		// EFSP: auto-apply pending video changes instead of the "lose video settings?" confirm (user
		// request — don't ask). vid_apply_action == s_video_apply_action (set at :3454), whose callback is
		// ui_video's ApplyChanges; invoke it via the pointer (ApplyChanges is static to ui_video.cpp).
		if ( vid_apply_action->generic.callback )
			vid_apply_action->generic.callback( vid_apply_action, QM_ACTIVATED );
		Controls_MenuEventNew (holdControlPtr, holdControlEvent);
	}
	else	// Go ahead, act normal
	{
		Controls_MenuEventNew (holdControlPtr, holdControlEvent);
	}
}

/*
=================
Controls_MenuEventNew
=================
*/
static void Controls_MenuEventNew (void* ptr, int event)
{
	menuframework_s*	m;

	if (event != QM_ACTIVATED)
		return;


	m = ((menucommon_s*)ptr)->parent;

	switch (((menucommon_s*)ptr)->id)
	{
		case ID_CONTROLS:
			UI_PopMenu();			// Get rid of whatever is ontop
			UI_SetupWeaponsMenu();			// Move to the Controls Menu
			break;

		case ID_ATTACK:
			UI_PopMenu();			// Get rid of whatever is ontop
			UI_SetupWeaponsMenu();	// Move to the Command Menu
			break;

		case ID_VIDEO:
			UI_PopMenu();			// Get rid of whatever is ontop
			UI_VideoDataMenu();		// Move to the Video Menu
			break;

		case ID_SOUND:
			UI_PopMenu();			// Get rid of whatever is ontop
			UI_SoundMenu();			// Move to the Sound Menu
			break;

		case ID_GAMEOPTIONS:
			UI_PopMenu();			// Get rid of whatever is ontop
			UI_GameOptionsMenu();	// Move to the Game Options Menu
			break;

		case ID_CDKEY:
			UI_PopMenu();			// Get rid of whatever is ontop
			UI_CDKeyMenu();			// Move to the CD Key Menu
			break;

		case ID_FONTS:
			UI_PopMenu();			// Get rid of whatever is ontop
			UI_FontsMenu();			// Move to the Fonts Menu
			break;

		case ID_MAINMENU:
			UI_PopMenu();
			break;

		case ID_CONTROLSQUICKKEYS:
			UI_PopMenu();				// Get rid of whatever is ontop
			UI_ControlsQuickKeysMenu();	// Move to the Quick Keys Menu
			break;

		case ID_CONTROLSLOOK:
			UI_PopMenu();				// Get rid of whatever is ontop
			UI_ControlsAttackLookMenu();		// Move to the Look Menu
			break;

		case ID_CONTROLSMOVE:
			UI_PopMenu();				// Get rid of whatever is ontop
			UI_ControlsMoveMenu();		// Move to the Move Menu
			break;

		case ID_CONTROLSMOUSE:
			UI_PopMenu();				// Get rid of whatever is ontop
			UI_ControlsMouseJoyStickMenu();		// Move to the Mouse Menu
			break;

		case ID_CONTROLSOTHER:
			UI_PopMenu();				// Get rid of whatever is ontop
			UI_ControlsOtherMenu();		// Move to the Other Menu
			break;

		case ID_CONTROLSDEFAULT:
			UI_PopMenu();				// Get rid of whatever is ontop
			UI_ControlsDefaultMenu();	// Move to the Default Menu
			break;
	}
}

/*
=================
M_WeaponsMenu_Blinkies
=================
*/
void M_WeaponsMenu_Blinkies (void)
{
	int i;

	for (i=0;i<AMG_MAX;++i)
	{
		attackmenu_graphics[i].color = CT_VDKPURPLE1;
	}

	// Don't flash frame unless waiting for input
	if (!g_waitingforkey)
	{
		return;
	}

	if (attackmenu_graphics[AMG_MIDLEFT].timer < uis.realtime)
	{
		attackmenu_graphics[AMG_MIDLEFT].timer = uis.realtime + 500;
		++attackmenu_graphics[AMG_MIDLEFT].target;
		if (attackmenu_graphics[AMG_MIDLEFT].target > 7)
		{
			attackmenu_graphics[AMG_MIDLEFT].target = 0;
		}
	}

	switch (attackmenu_graphics[AMG_MIDLEFT].target)
	{
		case 0:
			attackmenu_graphics[AMG_MIDLEFT].color = CT_LTPURPLE1;
			break;
		case 1:
			attackmenu_graphics[AMG_UPPERLEFT].color = CT_LTPURPLE1;
			attackmenu_graphics[AMG_LOWERLEFT].color = CT_LTPURPLE1;
			break;
		case 2:
			attackmenu_graphics[AMG_UPPERCORNER].color = CT_LTPURPLE1;
			attackmenu_graphics[AMG_LOWERCORNER].color = CT_LTPURPLE1;
			break;
		case 3:
			attackmenu_graphics[AMG_UPPERTOP1ST].color = CT_LTPURPLE1;
			attackmenu_graphics[AMG_LOWERTOP1ST].color = CT_LTPURPLE1;
			break;
		case 4:
			attackmenu_graphics[AMG_UPPERTOP2ND].color = CT_LTPURPLE1;
			attackmenu_graphics[AMG_LOWERTOP2ND].color = CT_LTPURPLE1;
			break;
		case 5:
			attackmenu_graphics[AMG_UPPERSWOOP].color = CT_LTPURPLE1;
			attackmenu_graphics[AMG_LOWERSWOOP].color = CT_LTPURPLE1;
			break;
		case 6:
			attackmenu_graphics[AMG_TOPRIGHT].color = CT_LTPURPLE1;
			attackmenu_graphics[AMG_BOTTOMRIGHT].color = CT_LTPURPLE1;
			break;
		case 7:
			attackmenu_graphics[AMG_PLAYERBKGRND].color = CT_LTPURPLE1;
			break;
	}

}

/*
=================
M_WeaponsMenu_Graphics
=================
*/
void M_WeaponsMenu_Graphics (void)
{
	// Draw the basic screen layout
	UI_MenuFrame(&s_weapons_menu);

	UI_DrawProportionalString(  74,  66, "56-129",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  84, "33",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  188, "9893",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  395, "12799",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);

	M_WeaponsMenu_Blinkies();

	UI_PrintMenuGraphics(attackmenu_graphics,AMG_MAX);

	UI_DrawProportionalString(  607,  174, "981235",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  607,  406, "5672141",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);

}

/*
=================
Weapons_MenuDraw
=================
*/
static void Weapons_MenuDraw (void)
{
	M_WeaponsMenu_Graphics();

	UI_Setup_MenuButtons();

	if (s_attack_waiting_action.generic.flags & QMF_HIDDEN)
	{
		UI_DrawProportionalString(  207,  411, menu_normal_text[MNT_BACKSPACE],UI_SMALLFONT, colorTable[CT_WHITE]);
	}

	Menu_Draw( &s_weapons_menu );
}

/*
=================
UI_WeaponsMenu_Cache
=================
*/
void UI_WeaponsMenu_Cache( void )
{
	int i;

	swooshTop = ui.R_RegisterShaderNoMip("menu/common/swoosh_top.tga");
	swooshBottom= ui.R_RegisterShaderNoMip("menu/common/swoosh_bottom.tga");

	// Precache all menu graphics in array
	for (i=0;i<AMG_MAX;++i)
	{
		if (attackmenu_graphics[i].type == MG_GRAPHIC)
		{
			attackmenu_graphics[i].graphic = ui.R_RegisterShaderNoMip(attackmenu_graphics[i].file);
		}
	}

}

/*
=================
SetupActionButtons_Init
=================
*/
static void SetupActionButtons_Init(int section)
{
	int i,y;
	void**		controlptr;
	menuframework_s *current_menu;


	controlptr = g_controls[g_section];

	switch (g_section)
	{
		case C_WEAPONS:
			current_menu = &s_weapons_menu;
			break;
		case C_QUICKKEYS:
			current_menu = &s_controlsquickkeys_menu;
			break;
		case C_LOOK:
			current_menu = &s_controlslook_menu;
			break;
		case C_MOVE:
			current_menu = &s_controlsmove_menu;
			break;
		case C_MOUSE:
			current_menu = &s_controlsmouse_menu;
			break;
	}


	y = current_menu->listY;

	for (i=0; i<99; i++)	// The 99 is to avoid runaway loops
	{
		if (!controlptr[i])
		{
			break;
		}

		((menuaction_s*)controlptr[i])->generic.x	= current_menu->listX;
		((menuaction_s*)controlptr[i])->generic.y	= y;
		((menuaction_s*)controlptr[i])->textX		= 5;
		((menuaction_s*)controlptr[i])->textY		= 1;
		((menuaction_s*)controlptr[i])->height		= 18;
		((menuaction_s*)controlptr[i])->width		= 150;

		y += 20;
	}
}

/*
=================
Weapons_MenuInit
=================
*/
static void Playermodel_MenuInit( void )
{
	s_controls_playermdl.generic.type			= MTYPE_BITMAP;
	s_controls_playermdl.generic.flags			= QMF_INACTIVE;
	s_controls_playermdl.generic.callback		= Controls_ModelEvent;
	s_controls_playermdl.generic.ownerdraw		= Controls_DrawPlayer;
	s_controls_playermdl.generic.x				= 430;
	s_controls_playermdl.generic.y				= 95;
	s_controls_playermdl.width					= 32*7.6;
	s_controls_playermdl.height					= 56*7.6;
}

/*
=================
Weapons_MenuInit
=================
*/
static void Weapons_MenuInit( void )
{
	UI_WeaponsMenu_Cache();

	attackmenu_graphics[AMG_MIDLEFT].timer = uis.realtime + 500;

	s_weapons_menu.nitems						= 0;
	s_weapons_menu.wrapAround					= qtrue;
	s_weapons_menu.opening						= NULL;
	s_weapons_menu.closing						= NULL;
	s_weapons_menu.draw							= Weapons_MenuDraw;
	s_weapons_menu.key							= Controls_MenuKey;
	s_weapons_menu.fullscreen					= qtrue;
	s_weapons_menu.descX						= MENU_DESC_X;
	s_weapons_menu.descY						= MENU_DESC_Y;
	s_weapons_menu.listX						= 170;
	s_weapons_menu.listY						= 184;
	s_weapons_menu.titleX						= MENU_TITLE_X;
	s_weapons_menu.titleY						= MENU_TITLE_Y;
	s_weapons_menu.titleI						= MNT_CONTROLSMENU_TITLE;
	s_weapons_menu.footNoteEnum					= MNT_WEAPONKEY_SETUP;

	Playermodel_MenuInit();

	SetupMenu_TopButtons(&s_weapons_menu,MENU_CONTROLS,NULL);
	SetupMenu_SideButtons(&s_weapons_menu,MENU_CONTROLS_WEAPON);

	s_controls_weapon.textcolor			= CT_LTGOLD1;
	s_controls_weapon.textcolor2		= CT_LTGOLD1;

	s_attack_weapon1_action.generic.type		= MTYPE_ACTION;
	s_attack_weapon1_action.generic.flags		= QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT_IF_FOCUS;
	s_attack_weapon1_action.generic.callback	= Controls_ActionEvent;
	s_attack_weapon1_action.generic.ownerdraw	= Controls_DrawKeyBinding;
	s_attack_weapon1_action.generic.id			= ID_WEAPON1;

	s_attack_weapon2_action.generic.type		= MTYPE_ACTION;
	s_attack_weapon2_action.generic.flags		= QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT_IF_FOCUS;
	s_attack_weapon2_action.generic.callback	= Controls_ActionEvent;
	s_attack_weapon2_action.generic.ownerdraw	= Controls_DrawKeyBinding;
	s_attack_weapon2_action.generic.id			= ID_WEAPON2;

	s_attack_weapon3_action.generic.type		= MTYPE_ACTION;
	s_attack_weapon3_action.generic.flags		= QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT_IF_FOCUS;
	s_attack_weapon3_action.generic.callback	= Controls_ActionEvent;
	s_attack_weapon3_action.generic.ownerdraw	= Controls_DrawKeyBinding;
	s_attack_weapon3_action.generic.id			= ID_WEAPON3;

	s_attack_weapon3_action.generic.type		= MTYPE_ACTION;
	s_attack_weapon3_action.generic.flags		= QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT_IF_FOCUS;
	s_attack_weapon3_action.generic.callback	= Controls_ActionEvent;
	s_attack_weapon3_action.generic.ownerdraw	= Controls_DrawKeyBinding;
	s_attack_weapon3_action.generic.id			= ID_WEAPON3;

	s_attack_weapon4_action.generic.type		= MTYPE_ACTION;
	s_attack_weapon4_action.generic.flags		= QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT_IF_FOCUS;
	s_attack_weapon4_action.generic.callback	= Controls_ActionEvent;
	s_attack_weapon4_action.generic.ownerdraw	= Controls_DrawKeyBinding;
	s_attack_weapon4_action.generic.id			= ID_WEAPON4;

	s_attack_weapon5_action.generic.type	   = MTYPE_ACTION;
	s_attack_weapon5_action.generic.flags     = QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT_IF_FOCUS;
	s_attack_weapon5_action.generic.callback  = Controls_ActionEvent;
	s_attack_weapon5_action.generic.ownerdraw = Controls_DrawKeyBinding;
	s_attack_weapon5_action.generic.id        = ID_WEAPON5;

	s_attack_weapon6_action.generic.type	   = MTYPE_ACTION;
	s_attack_weapon6_action.generic.flags     = QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT_IF_FOCUS;
	s_attack_weapon6_action.generic.callback  = Controls_ActionEvent;
	s_attack_weapon6_action.generic.ownerdraw = Controls_DrawKeyBinding;
	s_attack_weapon6_action.generic.id        = ID_WEAPON6;

	s_attack_weapon6_action.generic.type	   = MTYPE_ACTION;
	s_attack_weapon6_action.generic.flags     = QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT_IF_FOCUS;
	s_attack_weapon6_action.generic.callback  = Controls_ActionEvent;
	s_attack_weapon6_action.generic.ownerdraw = Controls_DrawKeyBinding;
	s_attack_weapon6_action.generic.id        = ID_WEAPON6;

	s_attack_weapon7_action.generic.type	   = MTYPE_ACTION;
	s_attack_weapon7_action.generic.flags     = QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT_IF_FOCUS;
	s_attack_weapon7_action.generic.callback  = Controls_ActionEvent;
	s_attack_weapon7_action.generic.ownerdraw = Controls_DrawKeyBinding;
	s_attack_weapon7_action.generic.id        = ID_WEAPON7;

	s_attack_weapon8_action.generic.type	   = MTYPE_ACTION;
	s_attack_weapon8_action.generic.flags     = QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT_IF_FOCUS;
	s_attack_weapon8_action.generic.callback  = Controls_ActionEvent;
	s_attack_weapon8_action.generic.ownerdraw = Controls_DrawKeyBinding;
	s_attack_weapon8_action.generic.id        = ID_WEAPON8;

	s_attack_weapon9_action.generic.type	   = MTYPE_ACTION;
	s_attack_weapon9_action.generic.flags     = QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT_IF_FOCUS;
	s_attack_weapon9_action.generic.callback  = Controls_ActionEvent;
	s_attack_weapon9_action.generic.ownerdraw = Controls_DrawKeyBinding;
	s_attack_weapon9_action.generic.id        = ID_WEAPON9;

	s_attack_weapon_next_action.generic.type	   = MTYPE_ACTION;
	s_attack_weapon_next_action.generic.flags     = QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT_IF_FOCUS;
	s_attack_weapon_next_action.generic.callback  = Controls_ActionEvent;
	s_attack_weapon_next_action.generic.ownerdraw = Controls_DrawKeyBinding;
	s_attack_weapon_next_action.generic.id        = ID_WEAPON_NEXT;

	s_attack_weapon_prev_action.generic.type	   = MTYPE_ACTION;
	s_attack_weapon_prev_action.generic.flags     = QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT_IF_FOCUS;
	s_attack_weapon_prev_action.generic.callback  = Controls_ActionEvent;
	s_attack_weapon_prev_action.generic.ownerdraw = Controls_DrawKeyBinding;
	s_attack_weapon_prev_action.generic.id        = ID_WEAPON_PREV;

	s_attack_waiting_action.generic.type			= MTYPE_ACTION;
	s_attack_waiting_action.generic.flags			= QMF_HIDDEN;
	s_attack_waiting_action.generic.x				= 202;
	s_attack_waiting_action.generic.y				= 410;
	s_attack_waiting_action.textEnum				= MBT_WAITFORKEY;
	s_attack_waiting_action.textcolor				= CT_BLACK;
	s_attack_waiting_action.textcolor2				= CT_WHITE;
	s_attack_waiting_action.textcolor3				= CT_LTGREY;
	s_attack_waiting_action.color					= CT_DKPURPLE1;
	s_attack_waiting_action.color2					= CT_LTPURPLE1;
	s_attack_waiting_action.color3					= CT_DKGREY;
	s_attack_waiting_action.textX					= 5;
	s_attack_waiting_action.textY					= 1;
	s_attack_waiting_action.width					= 255;
	s_attack_waiting_action.height					= 18;

	g_section        = C_WEAPONS;

	SetupActionButtons_Init(g_section);		// Set up standard values

	Menu_AddItem( &s_weapons_menu, ( void * )&s_controls_playermdl);
	Menu_AddItem( &s_weapons_menu, ( void * )&s_attack_weapon1_action);
	Menu_AddItem( &s_weapons_menu, ( void * )&s_attack_weapon2_action);
	Menu_AddItem( &s_weapons_menu, ( void * )&s_attack_weapon3_action);
	Menu_AddItem( &s_weapons_menu, ( void * )&s_attack_weapon4_action);
	Menu_AddItem( &s_weapons_menu, ( void * )&s_attack_weapon5_action);
	Menu_AddItem( &s_weapons_menu, ( void * )&s_attack_weapon6_action);
	Menu_AddItem( &s_weapons_menu, ( void * )&s_attack_weapon8_action);
	Menu_AddItem( &s_weapons_menu, ( void * )&s_attack_weapon9_action);
	Menu_AddItem( &s_weapons_menu, ( void * )&s_attack_weapon7_action);
	Menu_AddItem( &s_weapons_menu, ( void * )&s_attack_weapon_next_action);
	Menu_AddItem( &s_weapons_menu, ( void * )&s_attack_weapon_prev_action);
	Menu_AddItem( &s_weapons_menu, ( void * )&s_attack_waiting_action);


	// initialize the configurable cvars
	Controls_InitCvars();

	// initialize the current config
	Controls_GetConfig();

	// intialize the model
	Controls_InitModel();

	// intialize the weapons
	Controls_InitWeapons ();

	// initial default section
	g_waitingforkey  = qfalse;
	g_changesmade    = qfalse;
	g_prompttosave   = qfalse;

	// update the ui
	Controls_UpdateNew();
}


/*
=================
UI_SetupWeaponsMenu
=================
*/
void UI_SetupWeaponsMenu( void )
{
	ui.Key_SetCatcher( KEYCATCH_UI );

	if (!s_weapons_menu.initialized)
	{
		Weapons_MenuInit();
	}


	UI_PushMenu( &s_weapons_menu );
}


/*
=================
M_ControlsCommandMenu_Graphics
=================
*/
void M_ControlsCommandMenu_Graphics (void)
{
	// Draw the basic screen layout
	UI_MenuFrame(&s_controlsquickkeys_menu);

	UI_DrawProportionalString(  74,  66, "7172-0",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  84, "676",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  188, "331",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  395, "45601",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);

	M_WeaponsMenu_Blinkies();

	UI_PrintMenuGraphics(attackmenu_graphics,AMG_MAX);

	UI_DrawProportionalString(  607,  174, "4678-0134",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  607,  406, "801244",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);

}

/*
=================
ControlsCommand_MenuDraw
=================
*/
static void ControlsCommand_MenuDraw (void)
{
	M_ControlsCommandMenu_Graphics();

	UI_Setup_MenuButtons();

	Menu_Draw( &s_controlsquickkeys_menu );
}

/*
=================
ControlsQuickKeys_MenuInit
=================
*/
static void ControlsQuickKeys_MenuInit( void )
{
	s_controlsquickkeys_menu.nitems				= 0;
	s_controlsquickkeys_menu.wrapAround			= qtrue;
	s_controlsquickkeys_menu.opening				= NULL;
	s_controlsquickkeys_menu.closing				= NULL;
	s_controlsquickkeys_menu.draw					= ControlsCommand_MenuDraw;
	s_controlsquickkeys_menu.key					= Controls_MenuKey;
	s_controlsquickkeys_menu.fullscreen			= qtrue;
	s_controlsquickkeys_menu.descX				= MENU_DESC_X;
	s_controlsquickkeys_menu.descY				= MENU_DESC_Y;
	s_controlsquickkeys_menu.listX				= 170;
	s_controlsquickkeys_menu.listY				= 225;
	s_controlsquickkeys_menu.titleX				= MENU_TITLE_X;
	s_controlsquickkeys_menu.titleY				= MENU_TITLE_Y;
	s_controlsquickkeys_menu.titleI				= MNT_CONTROLSMENU_TITLE;
	s_controlsquickkeys_menu.footNoteEnum			= MNT_QUICKKEYS_SETUP;

	Playermodel_MenuInit();

	SetupMenu_TopButtons(&s_controlsquickkeys_menu,MENU_CONTROLS,NULL);
	SetupMenu_SideButtons(&s_controlsquickkeys_menu,MENU_CONTROLS_QUICKKEYS);

	s_controls_quickkey.textcolor					= CT_LTGOLD1;
	s_controls_quickkey.textcolor2					= CT_LTGOLD1;

	s_skip_cinematic_action.generic.type			= MTYPE_ACTION;
	s_skip_cinematic_action.generic.flags			= QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT_IF_FOCUS;
	s_skip_cinematic_action.generic.callback		= Controls_ActionEvent;
	s_skip_cinematic_action.generic.ownerdraw		= Controls_DrawKeyBinding;
	s_skip_cinematic_action.generic.id				= ID_USEHOLDITEM;

	s_spacer_action.generic.type				= MTYPE_ACTION;
	s_spacer_action.generic.flags				= QMF_HIDDEN;
	s_spacer_action.generic.ownerdraw			= Controls_DrawKeyBinding;

	s_command_objectives_action.generic.type		= MTYPE_ACTION;
	s_command_objectives_action.generic.flags		= QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT_IF_FOCUS;
	s_command_objectives_action.generic.callback	= Controls_ActionEvent;
	s_command_objectives_action.generic.ownerdraw	= Controls_DrawKeyBinding;
	s_command_objectives_action.generic.id			= ID_MISSIONOBJECTIVES;

	s_mission_analysis_action.generic.type		= MTYPE_ACTION;
	s_mission_analysis_action.generic.flags		= QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT_IF_FOCUS;
	s_mission_analysis_action.generic.callback	= Controls_ActionEvent;
	s_mission_analysis_action.generic.ownerdraw	= Controls_DrawKeyBinding;
	s_mission_analysis_action.generic.id			= ID_MISSIONANALYSIS;

	s_game_save.generic.type		= MTYPE_ACTION;
	s_game_save.generic.flags		= QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT_IF_FOCUS;
	s_game_save.generic.callback	= Controls_ActionEvent;
	s_game_save.generic.ownerdraw	= Controls_DrawKeyBinding;
	s_game_save.generic.id			= ID_SAVEGAME;

	s_game_load.generic.type		= MTYPE_ACTION;
	s_game_load.generic.flags		= QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT_IF_FOCUS;
	s_game_load.generic.callback	= Controls_ActionEvent;
	s_game_load.generic.ownerdraw	= Controls_DrawKeyBinding;
	s_game_load.generic.id			= ID_LOADGAME;

	s_game_autosave.generic.type		= MTYPE_ACTION;
	s_game_autosave.generic.flags		= QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT_IF_FOCUS;
	s_game_autosave.generic.callback	= Controls_ActionEvent;
	s_game_autosave.generic.ownerdraw	= Controls_DrawKeyBinding;
	s_game_autosave.generic.id			= ID_AUTOSAVEGAME;

	s_attack_waiting_action.generic.type			= MTYPE_ACTION;
	s_attack_waiting_action.generic.flags			= QMF_HIDDEN;
	s_attack_waiting_action.generic.x				= 202;
	s_attack_waiting_action.generic.y				= 410;
	s_attack_waiting_action.textEnum				= MBT_WAITFORKEY;
	s_attack_waiting_action.textcolor				= CT_BLACK;
	s_attack_waiting_action.textcolor2				= CT_WHITE;
	s_attack_waiting_action.textcolor3				= CT_LTGREY;
	s_attack_waiting_action.color					= CT_DKPURPLE1;
	s_attack_waiting_action.color2					= CT_LTPURPLE1;
	s_attack_waiting_action.color3					= CT_DKGREY;
	s_attack_waiting_action.textX					= 5;
	s_attack_waiting_action.textY					= 1;
	s_attack_waiting_action.width					= 255;
	s_attack_waiting_action.height					= 18;

	g_section        = C_QUICKKEYS;

	SetupActionButtons_Init(g_section);		// Set up standard values

	Menu_AddItem( &s_controlsquickkeys_menu, ( void * )&s_command_objectives_action);
	Menu_AddItem( &s_controlsquickkeys_menu, ( void * )&s_mission_analysis_action);
	Menu_AddItem( &s_controlsquickkeys_menu, ( void * )&s_skip_cinematic_action);
	Menu_AddItem( &s_controlsquickkeys_menu, ( void * )&s_spacer_action);
	Menu_AddItem( &s_controlsquickkeys_menu, ( void * )&s_game_save);
	Menu_AddItem( &s_controlsquickkeys_menu, ( void * )&s_game_load);
	Menu_AddItem( &s_controlsquickkeys_menu, ( void * )&s_game_autosave);
	Menu_AddItem( &s_controlsquickkeys_menu, ( void * )&s_attack_waiting_action);

	// initialize the configurable cvars
	Controls_InitCvars();

	// initialize the current config
	Controls_GetConfig();

	// intialize the model
	Controls_InitModel();

	// intialize the weapons
	Controls_InitWeapons ();

	// initial default section
	g_waitingforkey  = qfalse;
	g_changesmade    = qfalse;
	g_prompttosave   = qfalse;

	// update the ui
	Controls_UpdateNew();

	s_controlsquickkeys_menu.initialized = qtrue;

}


/*
=================
UI_ControlsQuickKeysMenu
=================
*/
static void UI_ControlsQuickKeysMenu( void )
{
	ui.Key_SetCatcher( KEYCATCH_UI );

//	if (!s_controlsquickkeys_menu.initialized)
//	{
		ControlsQuickKeys_MenuInit();
//	}

	UI_PushMenu( &s_controlsquickkeys_menu );
}





/*
=================
M_ControlsLookMenu_Graphics
=================
*/
void M_ControlsLookMenu_Graphics (void)
{
	// Draw the basic screen layout
	UI_MenuFrame(&s_controlslook_menu);

	UI_DrawProportionalString(  74,  66, "3567",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  84, "9003",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  188, "1425",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  395, "6780-1",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);

	M_WeaponsMenu_Blinkies();

	UI_PrintMenuGraphics(attackmenu_graphics,AMG_MAX);

	UI_DrawProportionalString(  607,  174, "556541",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  607,  406, "909090",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);

}

/*
=================
ControlsAttackLook_MenuDraw
=================
*/
static void ControlsAttackLook_MenuDraw (void)
{
	M_ControlsLookMenu_Graphics();

	UI_Setup_MenuButtons();

	if (s_attack_waiting_action.generic.flags & QMF_HIDDEN)
	{
		UI_DrawProportionalString(  207,  411, menu_normal_text[MNT_BACKSPACE],UI_SMALLFONT, colorTable[CT_WHITE]);
	}

	Menu_Draw( &s_controlslook_menu );
}

/*
=================
ControlsAttackLook_MenuInit
=================
*/
static void ControlsAttackLook_MenuInit( void )
{
	s_controlslook_menu.nitems					= 0;
	s_controlslook_menu.wrapAround				= qtrue;
	s_controlslook_menu.opening					= NULL;
	s_controlslook_menu.closing					= NULL;
	s_controlslook_menu.draw					= ControlsAttackLook_MenuDraw;
	s_controlslook_menu.key						= Controls_MenuKey;
	s_controlslook_menu.fullscreen				= qtrue;
	s_controlslook_menu.descX					= MENU_DESC_X;
	s_controlslook_menu.descY					= MENU_DESC_Y;
	s_controlslook_menu.listX					= 170;
	s_controlslook_menu.listY					= 188;
	s_controlslook_menu.titleX					= MENU_TITLE_X;
	s_controlslook_menu.titleY					= MENU_TITLE_Y;
	s_controlslook_menu.titleI					= MNT_CONTROLSMENU_TITLE;
	s_controlslook_menu.footNoteEnum			= MNT_ATTACKLOOKKEY_SETUP;

	Playermodel_MenuInit();

	SetupMenu_TopButtons(&s_controlslook_menu,MENU_CONTROLS,NULL);
	SetupMenu_SideButtons(&s_controlslook_menu,MENU_CONTROLS_LOOK);

	s_controls_look.textcolor					= CT_LTGOLD1;
	s_controls_look.textcolor2					= CT_LTGOLD1;

	s_attack_attack_action.generic.type			= MTYPE_ACTION;
	s_attack_attack_action.generic.flags		= QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT_IF_FOCUS;
	s_attack_attack_action.generic.callback		= Controls_ActionEvent;
	s_attack_attack_action.generic.ownerdraw	= Controls_DrawKeyBinding;
	s_attack_attack_action.generic.id			= ID_ATTACK;

	s_attack_alt_attack_action.generic.type			= MTYPE_ACTION;
	s_attack_alt_attack_action.generic.flags		= QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT_IF_FOCUS;
	s_attack_alt_attack_action.generic.callback		= Controls_ActionEvent;
	s_attack_alt_attack_action.generic.ownerdraw	= Controls_DrawKeyBinding;
	s_attack_alt_attack_action.generic.id			= ID_ALT_ATTACK;

	s_attack_use_action.generic.type			= MTYPE_ACTION;
	s_attack_use_action.generic.flags			= QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT_IF_FOCUS;
	s_attack_use_action.generic.callback		= Controls_ActionEvent;
	s_attack_use_action.generic.ownerdraw		= Controls_DrawKeyBinding;
	s_attack_use_action.generic.id				= ID_USE;

	s_spacer_action.generic.type				= MTYPE_ACTION;
	s_spacer_action.generic.flags				= QMF_HIDDEN;
	s_spacer_action.generic.ownerdraw			= Controls_DrawKeyBinding;

	s_look_lookup_action.generic.type			= MTYPE_ACTION;
	s_look_lookup_action.generic.flags			= QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT_IF_FOCUS;
	s_look_lookup_action.generic.callback		= Controls_ActionEvent;
	s_look_lookup_action.generic.ownerdraw		= Controls_DrawKeyBinding;
	s_look_lookup_action.generic.id				= ID_LOOKUP;

	s_look_lookdown_action.generic.type	    = MTYPE_ACTION;
	s_look_lookdown_action.generic.flags     = QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT_IF_FOCUS;
	s_look_lookdown_action.generic.callback  = Controls_ActionEvent;
	s_look_lookdown_action.generic.ownerdraw = Controls_DrawKeyBinding;
	s_look_lookdown_action.generic.id        = ID_LOOKDOWN;

	s_look_mouselook_action.generic.type	     = MTYPE_ACTION;
	s_look_mouselook_action.generic.flags     = QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT_IF_FOCUS;
	s_look_mouselook_action.generic.callback  = Controls_ActionEvent;
	s_look_mouselook_action.generic.ownerdraw = Controls_DrawKeyBinding;
	s_look_mouselook_action.generic.id        = ID_MOUSELOOK;

	s_look_centerview_action.generic.type	  = MTYPE_ACTION;
	s_look_centerview_action.generic.flags     = QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT_IF_FOCUS;
	s_look_centerview_action.generic.callback  = Controls_ActionEvent;
	s_look_centerview_action.generic.ownerdraw = Controls_DrawKeyBinding;
	s_look_centerview_action.generic.id        = ID_CENTERVIEW;

	s_zoomview_action.generic.type	    = MTYPE_ACTION;
	s_zoomview_action.generic.flags     = QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT_IF_FOCUS;
	s_zoomview_action.generic.callback  = Controls_ActionEvent;
	s_zoomview_action.generic.ownerdraw = Controls_DrawKeyBinding;
	s_zoomview_action.generic.id        = ID_ZOOMVIEW;

	s_attack_waiting_action.generic.type			= MTYPE_ACTION;
	s_attack_waiting_action.generic.flags			= QMF_HIDDEN;
	s_attack_waiting_action.generic.x				= 202;
	s_attack_waiting_action.generic.y				= 410;
	s_attack_waiting_action.textEnum				= MBT_WAITFORKEY;
	s_attack_waiting_action.textcolor				= CT_BLACK;
	s_attack_waiting_action.textcolor2				= CT_WHITE;
	s_attack_waiting_action.textcolor3				= CT_LTGREY;
	s_attack_waiting_action.color					= CT_DKPURPLE1;
	s_attack_waiting_action.color2					= CT_LTPURPLE1;
	s_attack_waiting_action.color3					= CT_DKGREY;
	s_attack_waiting_action.textX					= 5;
	s_attack_waiting_action.textY					= 1;
	s_attack_waiting_action.width					= 255;
	s_attack_waiting_action.height					= 18;

	g_section        = C_LOOK;
	SetupActionButtons_Init(g_section);		// Set up standard values

	Menu_AddItem( &s_controlslook_menu, ( void * )&s_controls_playermdl);

	Menu_AddItem( &s_controlslook_menu, ( void * )&s_attack_attack_action);
	Menu_AddItem( &s_controlslook_menu, ( void * )&s_attack_alt_attack_action);
	Menu_AddItem( &s_controlslook_menu, ( void * )&s_attack_use_action);
	Menu_AddItem( &s_controlslook_menu, ( void * )&s_spacer_action);

	Menu_AddItem( &s_controlslook_menu, ( void * )&s_look_lookup_action);
	Menu_AddItem( &s_controlslook_menu, ( void * )&s_look_lookdown_action);
	Menu_AddItem( &s_controlslook_menu, ( void * )&s_look_mouselook_action);
	Menu_AddItem( &s_controlslook_menu, ( void * )&s_look_centerview_action);
	Menu_AddItem( &s_controlslook_menu, ( void * )&s_zoomview_action);
	Menu_AddItem( &s_controlslook_menu, ( void * )&s_attack_waiting_action);

	// initialize the configurable cvars
	Controls_InitCvars();

	// initialize the current config
	Controls_GetConfig();

	// intialize the model
	Controls_InitModel();

	// intialize the weapons
	Controls_InitWeapons ();

	// initial default section
	g_waitingforkey  = qfalse;
	g_changesmade    = qfalse;
	g_prompttosave   = qfalse;

	// update the ui
	Controls_UpdateNew();
}


/*
=================
UI_ControlsAttackLookMenu
=================
*/
static void UI_ControlsAttackLookMenu( void )
{
	ui.Key_SetCatcher( KEYCATCH_UI );

//	s_controls_menu.openingStart = uis.realtime;
//	if (!s_controls_menu.openingStart)	// This only happens at the very beginning
//	{
//		s_controls_menu.openingStart = 1;
//	}

	if (!s_controlslook_menu.initialized)
	{
		ControlsAttackLook_MenuInit();
	}

	UI_PushMenu( &s_controlslook_menu );
}


/*
=================
M_ControlsMoveMenu_Graphics
=================
*/
void M_ControlsMoveMenu_Graphics (void)
{
	// Draw the basic screen layout
	UI_MenuFrame(&s_controlsmove_menu);

	UI_DrawProportionalString(  74,  66, "7-2345",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  84, "803",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  188, "32811",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  395, "560-99",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);

	M_WeaponsMenu_Blinkies();

	UI_PrintMenuGraphics(attackmenu_graphics,AMG_MAX);

	UI_DrawProportionalString(  607,  174, "634579",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  607,  406, "2815689",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);

}

/*
=================
ControlsMove_MenuDraw
=================
*/
static void ControlsMove_MenuDraw (void)
{
	M_ControlsMoveMenu_Graphics();

	UI_Setup_MenuButtons();

	if (s_attack_waiting_action.generic.flags & QMF_HIDDEN)
	{
		UI_DrawProportionalString(  207,  411, menu_normal_text[MNT_BACKSPACE],UI_SMALLFONT, colorTable[CT_WHITE]);
	}

	UI_DrawProportionalString(  176,  376, menu_normal_text[MNT_LEANLEFT],UI_SMALLFONT, colorTable[CT_LTGOLD1]);
	UI_DrawProportionalString(  176,  393, menu_normal_text[MNT_LEANRIGHT],UI_SMALLFONT, colorTable[CT_LTGOLD1]);

	Menu_Draw( &s_controlsmove_menu );
}

/*
=================
ControlsMove_MenuInit
=================
*/
static void ControlsMove_MenuInit( void )
{
	s_controlsmove_menu.nitems				= 0;
	s_controlsmove_menu.wrapAround			= qtrue;
	s_controlsmove_menu.opening				= NULL;
	s_controlsmove_menu.closing				= NULL;
	s_controlsmove_menu.draw				= ControlsMove_MenuDraw;
	s_controlsmove_menu.key					= Controls_MenuKey;
	s_controlsmove_menu.fullscreen			= qtrue;
	s_controlsmove_menu.descX				= MENU_DESC_X;
	s_controlsmove_menu.descY				= MENU_DESC_Y;
	s_controlsmove_menu.listX				= 170;
	s_controlsmove_menu.listY				= 174;
	s_controlsmove_menu.titleX				= MENU_TITLE_X;
	s_controlsmove_menu.titleY				= MENU_TITLE_Y;
	s_controlsmove_menu.titleI				= MNT_CONTROLSMENU_TITLE;
	s_controlsmove_menu.footNoteEnum		= MNT_MOVEMENTKEYS_SETUP;

	Playermodel_MenuInit();

	SetupMenu_TopButtons(&s_controlsmove_menu,MENU_CONTROLS,NULL);
	SetupMenu_SideButtons(&s_controlsmove_menu,MENU_CONTROLS_MOVEMENT);

	s_controls_movement.textcolor					= CT_LTGOLD1;
	s_controls_movement.textcolor2					= CT_LTGOLD1;

	s_move_walkforward_action.generic.type	   = MTYPE_ACTION;
	s_move_walkforward_action.generic.flags     = QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT_IF_FOCUS;
	s_move_walkforward_action.generic.callback  = Controls_ActionEvent;
	s_move_walkforward_action.generic.ownerdraw = Controls_DrawKeyBinding;
	s_move_walkforward_action.generic.id 	   = ID_FORWARD;

	s_move_backpedal_action.generic.type	     = MTYPE_ACTION;
	s_move_backpedal_action.generic.flags     = QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT_IF_FOCUS;
	s_move_backpedal_action.generic.callback  = Controls_ActionEvent;
	s_move_backpedal_action.generic.ownerdraw = Controls_DrawKeyBinding;
	s_move_backpedal_action.generic.id 		 = ID_BACKPEDAL;

	s_move_turnleft_action.generic.type	    = MTYPE_ACTION;
	s_move_turnleft_action.generic.flags     = QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT_IF_FOCUS;
	s_move_turnleft_action.generic.callback  = Controls_ActionEvent;
	s_move_turnleft_action.generic.ownerdraw = Controls_DrawKeyBinding;
	s_move_turnleft_action.generic.id        = ID_LEFT;

	s_move_turnright_action.generic.type	     = MTYPE_ACTION;
	s_move_turnright_action.generic.flags     = QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT_IF_FOCUS;
	s_move_turnright_action.generic.callback  = Controls_ActionEvent;
	s_move_turnright_action.generic.ownerdraw = Controls_DrawKeyBinding;
	s_move_turnright_action.generic.id        = ID_RIGHT;

	s_move_run_action.generic.type	   = MTYPE_ACTION;
	s_move_run_action.generic.flags     = QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT_IF_FOCUS;
	s_move_run_action.generic.callback  = Controls_ActionEvent;
	s_move_run_action.generic.ownerdraw = Controls_DrawKeyBinding;
	s_move_run_action.generic.id        = ID_SPEED;

	s_move_stepleft_action.generic.type	    = MTYPE_ACTION;
	s_move_stepleft_action.generic.flags     = QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT_IF_FOCUS;
	s_move_stepleft_action.generic.callback  = Controls_ActionEvent;
	s_move_stepleft_action.generic.ownerdraw = Controls_DrawKeyBinding;
	s_move_stepleft_action.generic.id 		= ID_MOVELEFT;

	s_move_stepright_action.generic.type	     = MTYPE_ACTION;
	s_move_stepright_action.generic.flags     = QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT_IF_FOCUS;
	s_move_stepright_action.generic.callback  = Controls_ActionEvent;
	s_move_stepright_action.generic.ownerdraw = Controls_DrawKeyBinding;
	s_move_stepright_action.generic.id        = ID_MOVERIGHT;

	s_move_sidestep_action.generic.type	    = MTYPE_ACTION;
	s_move_sidestep_action.generic.flags     = QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT_IF_FOCUS;
	s_move_sidestep_action.generic.callback  = Controls_ActionEvent;
	s_move_sidestep_action.generic.ownerdraw = Controls_DrawKeyBinding;
	s_move_sidestep_action.generic.id        = ID_STRAFE;

	s_move_moveup_action.generic.type	  = MTYPE_ACTION;
	s_move_moveup_action.generic.flags     = QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT_IF_FOCUS;
	s_move_moveup_action.generic.callback  = Controls_ActionEvent;
	s_move_moveup_action.generic.ownerdraw = Controls_DrawKeyBinding;
	s_move_moveup_action.generic.id        = ID_MOVEUP;

	s_move_movedown_action.generic.type	    = MTYPE_ACTION;
	s_move_movedown_action.generic.flags     = QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT_IF_FOCUS;
	s_move_movedown_action.generic.callback  = Controls_ActionEvent;
	s_move_movedown_action.generic.ownerdraw = Controls_DrawKeyBinding;
	s_move_movedown_action.generic.id        = ID_MOVEDOWN;

	s_attack_waiting_action.generic.type			= MTYPE_ACTION;
	s_attack_waiting_action.generic.flags			= QMF_HIDDEN;
	s_attack_waiting_action.generic.x				= 202;
	s_attack_waiting_action.generic.y				= 410;
	s_attack_waiting_action.textEnum				= MBT_WAITFORKEY;
	s_attack_waiting_action.textcolor				= CT_BLACK;
	s_attack_waiting_action.textcolor2				= CT_WHITE;
	s_attack_waiting_action.textcolor3				= CT_LTGREY;
	s_attack_waiting_action.color					= CT_DKPURPLE1;
	s_attack_waiting_action.color2					= CT_LTPURPLE1;
	s_attack_waiting_action.color3					= CT_DKGREY;
	s_attack_waiting_action.textX					= 5;
	s_attack_waiting_action.textY					= 1;
	s_attack_waiting_action.width					= 255;
	s_attack_waiting_action.height					= 18;

	g_section        = C_MOVE;
	SetupActionButtons_Init(g_section);		// Set up standard values

	Menu_AddItem( &s_controlsmove_menu, ( void * )&s_controls_playermdl);
	Menu_AddItem( &s_controlsmove_menu, ( void * )&s_move_walkforward_action);
	Menu_AddItem( &s_controlsmove_menu, ( void * )&s_move_backpedal_action);
	Menu_AddItem( &s_controlsmove_menu, ( void * )&s_move_turnleft_action);
	Menu_AddItem( &s_controlsmove_menu, ( void * )&s_move_turnright_action);
	Menu_AddItem( &s_controlsmove_menu, ( void * )&s_move_run_action);
	Menu_AddItem( &s_controlsmove_menu, ( void * )&s_move_stepleft_action);
	Menu_AddItem( &s_controlsmove_menu, ( void * )&s_move_stepright_action);
	Menu_AddItem( &s_controlsmove_menu, ( void * )&s_move_sidestep_action);
	Menu_AddItem( &s_controlsmove_menu, ( void * )&s_move_moveup_action);
	Menu_AddItem( &s_controlsmove_menu, ( void * )&s_move_movedown_action);
	Menu_AddItem( &s_controlsmove_menu, ( void * )&s_attack_waiting_action);

	// initialize the configurable cvars
	Controls_InitCvars();

	// initialize the current config
	Controls_GetConfig();

	// intialize the model
	Controls_InitModel();

	// intialize the weapons
	Controls_InitWeapons ();

	// initial default section
	g_waitingforkey  = qfalse;
	g_changesmade    = qfalse;
	g_prompttosave   = qfalse;

	// update the ui
	Controls_UpdateNew();
}


/*
=================
UI_ControlsMoveMenu
=================
*/
static void UI_ControlsMoveMenu( void )
{
	ui.Key_SetCatcher( KEYCATCH_UI );

//	s_controls_menu.openingStart = uis.realtime;
//	if (!s_controls_menu.openingStart)	// This only happens at the very beginning
//	{
//		s_controls_menu.openingStart = 1;
//	}

//	if (!s_controls_menu.initialized)
//	{
		ControlsMove_MenuInit();
//	}

	UI_PushMenu( &s_controlsmove_menu );
}





/*
=================
M_ControlsMouseJoyStickMenu_Graphics
=================
*/
void M_ControlsMouseJoyStickMenu_Graphics (void)
{
	// Draw the basic screen layout
	UI_MenuFrame(&s_controlsmouse_menu);

	ui.R_SetColor( colorTable[CT_DKBLUE2]);
	UI_DrawHandlePic( 235,  159,  377,  20, uis.whiteShader);	// Long thick line above mouse stuff
	UI_DrawHandlePic( 185,  185,  50,   98, uis.whiteShader);	// Thick column by mouse stuff
	UI_DrawHandlePic( 185,  275,  427,  8, uis.whiteShader);	// Long thin line below mouse stuff
	UI_DrawHandlePic( 185,  159,  64,  32, s_mousejoystick.corner1);	// Rounded corner

	UI_DrawHandlePic( 185,  309,  427,  8, uis.whiteShader);	// Long thin line below joystick stuff
	UI_DrawHandlePic( 185,  315,  50,  100, uis.whiteShader);	// Thick column by joystick stuff
	UI_DrawHandlePic( 235,  413,  377,  20, uis.whiteShader);	// Long thick line above joystick stuff
	UI_DrawHandlePic( 185,  408,  64,  32, s_mousejoystick.corner2);	// Rounded corner

	UI_DrawProportionalString(  608,  163, menu_normal_text[MNT_MOUSE],UI_RIGHT|UI_SMALLFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  608,  416, menu_normal_text[MNT_JOYSTICK],UI_RIGHT|UI_SMALLFONT, colorTable[CT_BLACK]);

	UI_DrawProportionalString(  74,  66, "4568",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  84, "71-49",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  188, "67014",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  395, "356-905",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);

}

void UI_ControlsMouseJoyStick_Cache (void ) 
{
	s_mousejoystick.corner1 = ui.R_RegisterShaderNoMip (ART_MOUSE1);
	s_mousejoystick.corner2 = ui.R_RegisterShaderNoMip (ART_MOUSE2);
}

/*
=================
ControlsMouseJoyStick_MenuDraw
=================
*/
static void ControlsMouseJoyStick_MenuDraw (void)
{
	M_ControlsMouseJoyStickMenu_Graphics();

	UI_Setup_MenuButtons();

	Menu_Draw( &s_controlsmouse_menu );
}

/*
=================
ControlsMouseJoyStick_MenuKey
=================
*/
static sfxHandle_t ControlsMouseJoyStick_MenuKey( int key )
{
	return Menu_DefaultKey( &s_controlsmouse_menu, key );
}


/*
=================
ControlsMouseJoyStick_MenuInit
=================
*/
static void ControlsMouseJoyStick_MenuInit( void )
{
	int x,y;

	UI_ControlsMouseJoyStick_Cache();

	s_controlsmouse_menu.nitems					= 0;
	s_controlsmouse_menu.wrapAround				= qtrue;
	s_controlsmouse_menu.opening				= NULL;
	s_controlsmouse_menu.closing				= NULL;
	s_controlsmouse_menu.draw					= ControlsMouseJoyStick_MenuDraw;
	s_controlsmouse_menu.key					= ControlsMouseJoyStick_MenuKey;
	s_controlsmouse_menu.fullscreen				= qtrue;
	s_controlsmouse_menu.descX					= MENU_DESC_X;
	s_controlsmouse_menu.descY					= MENU_DESC_Y;
	s_controlsmouse_menu.listX					= 230;
	s_controlsmouse_menu.listY					= 188;
	s_controlsmouse_menu.titleX					= MENU_TITLE_X;
	s_controlsmouse_menu.titleY					= MENU_TITLE_Y;
	s_controlsmouse_menu.titleI					= MNT_CONTROLSMENU_TITLE;
	s_controlsmouse_menu.footNoteEnum			= MNT_MOUSEJOYSTICK_SETUP;

	SetupMenu_TopButtons(&s_controlsmouse_menu,MENU_CONTROLS,NULL);
	SetupMenu_SideButtons(&s_controlsmouse_menu,MENU_CONTROLS_MOUSE);

	s_controls_mouse.textcolor					= CT_LTGOLD1;
	s_controls_mouse.textcolor2					= CT_LTGOLD1;

	static int s_freelook_Names[] =
	{
		MNT_OFF,
		MNT_ON,
		MNT_NONE
	};

	x = 250;
	y = 185;
	s_freelook_box.generic.type				= MTYPE_SPINCONTROL;
	s_freelook_box.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_freelook_box.generic.x				= x;
	s_freelook_box.generic.y				= y;
	s_freelook_box.generic.callback			= FreeLookCallback;
	s_freelook_box.textEnum					= MBT_MOUSEFREELOOK;
	s_freelook_box.textcolor				= CT_BLACK;
	s_freelook_box.textcolor2				= CT_WHITE;
	s_freelook_box.color					= CT_DKPURPLE1;
	s_freelook_box.color2					= CT_LTPURPLE1;
	s_freelook_box.textX					= MENU_BUTTON_TEXT_X;
	s_freelook_box.textY					= MENU_BUTTON_TEXT_Y;
	s_freelook_box.listnames				= s_freelook_Names;

	y += 22;
	s_sensitivity_slider.generic.type		= MTYPE_SLIDER;
	s_sensitivity_slider.generic.x			= x + 162;
	s_sensitivity_slider.generic.y			= y;
	s_sensitivity_slider.generic.flags		= QMF_SMALLFONT;
	s_sensitivity_slider.generic.id 		= ID_MOUSESPEED;
	s_sensitivity_slider.generic.callback	= MouseSpeedCallback;
	s_sensitivity_slider.minvalue			= 2;
	s_sensitivity_slider.maxvalue			= 30;
	s_sensitivity_slider.color				= CT_DKPURPLE1;
	s_sensitivity_slider.color2				= CT_LTPURPLE1;
	s_sensitivity_slider.generic.name		= "menu/common/monbar_2.tga";
	s_sensitivity_slider.width				= 256;
	s_sensitivity_slider.height				= 32;
	s_sensitivity_slider.focusWidth			= 145;
	s_sensitivity_slider.focusHeight		= 18;
	s_sensitivity_slider.picName			= "menu/common/square.tga";
	s_sensitivity_slider.picX				= x;
	s_sensitivity_slider.picY				= y;
	s_sensitivity_slider.picWidth			= MENU_BUTTON_MED_WIDTH + 21;
	s_sensitivity_slider.picHeight			= MENU_BUTTON_MED_HEIGHT;
	s_sensitivity_slider.textX				= MENU_BUTTON_TEXT_X;
	s_sensitivity_slider.textY				= MENU_BUTTON_TEXT_Y;
	s_sensitivity_slider.textEnum			= MBT_MOUSESPEED;
	s_sensitivity_slider.textcolor			= CT_BLACK;
	s_sensitivity_slider.textcolor2			= CT_WHITE;
	s_sensitivity_slider.thumbName			= GRAPHIC_BUTTONSLIDER;
	s_sensitivity_slider.thumbHeight		= 32;
	s_sensitivity_slider.thumbWidth			= 16;
	s_sensitivity_slider.thumbGraphicWidth	= 9;
	s_sensitivity_slider.thumbColor			= CT_DKBLUE1;
	s_sensitivity_slider.thumbColor2		= CT_LTBLUE1;

	y += 22;
	s_invertmouse_box.generic.type          = MTYPE_SPINCONTROL;
	s_invertmouse_box.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_invertmouse_box.generic.x				= x;
	s_invertmouse_box.generic.y				= y;
	s_invertmouse_box.generic.callback		= InvertMouseCallback;
	s_invertmouse_box.textEnum				= MBT_MOUSEINVERT;
	s_invertmouse_box.textcolor				= CT_BLACK;
	s_invertmouse_box.textcolor2			= CT_WHITE;
	s_invertmouse_box.color					= CT_DKPURPLE1;
	s_invertmouse_box.color2				= CT_LTPURPLE1;
	s_invertmouse_box.textX					= MENU_BUTTON_TEXT_X;
	s_invertmouse_box.textY					= MENU_BUTTON_TEXT_Y;
	s_invertmouse_box.listnames				= s_freelook_Names;

	y += 22;
	s_smoothmouse_box.generic.type          = MTYPE_SPINCONTROL;
	s_smoothmouse_box.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_smoothmouse_box.generic.x				= x;
	s_smoothmouse_box.generic.y				= y;
	s_smoothmouse_box.generic.callback		= SmoothMouseCallback;
	s_smoothmouse_box.textEnum				= MBT_MOUSESMOOTH;
	s_smoothmouse_box.textcolor				= CT_BLACK;
	s_smoothmouse_box.textcolor2			= CT_WHITE;
	s_smoothmouse_box.color					= CT_DKPURPLE1;
	s_smoothmouse_box.color2				= CT_LTPURPLE1;
	s_smoothmouse_box.textX					= MENU_BUTTON_TEXT_X;
	s_smoothmouse_box.textY					= MENU_BUTTON_TEXT_Y;
	s_smoothmouse_box.listnames				= s_freelook_Names;


	y = 287;
	s_forcefeedback_box.generic.type			= MTYPE_SPINCONTROL;
	s_forcefeedback_box.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_forcefeedback_box.generic.x				= 185;
	s_forcefeedback_box.generic.y				= y;
	s_forcefeedback_box.generic.callback		= ForcefeedBackCallback;
	s_forcefeedback_box.textEnum				= MBT_FORCEFEEDBACK;
	s_forcefeedback_box.textcolor				= CT_BLACK;
	s_forcefeedback_box.textcolor2				= CT_WHITE;
	s_forcefeedback_box.color					= CT_DKPURPLE1;
	s_forcefeedback_box.color2					= CT_LTPURPLE1;
	s_forcefeedback_box.textX					= MENU_BUTTON_TEXT_X;
	s_forcefeedback_box.textY					= MENU_BUTTON_TEXT_Y;
	s_forcefeedback_box.listnames				= s_freelook_Names;


	y = 323;
	s_joyenable_box.generic.type			= MTYPE_SPINCONTROL;
	s_joyenable_box.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_joyenable_box.generic.x				= x;
	s_joyenable_box.generic.y				= y;
	s_joyenable_box.generic.callback		= JoystickEnableCallback;
	s_joyenable_box.textEnum				= MBT_JOYSTICKENABLE;
	s_joyenable_box.textcolor				= CT_BLACK;
	s_joyenable_box.textcolor2				= CT_WHITE;
	s_joyenable_box.color					= CT_DKPURPLE1;
	s_joyenable_box.color2					= CT_LTPURPLE1;
	s_joyenable_box.textX					= MENU_BUTTON_TEXT_X;
	s_joyenable_box.textY					= MENU_BUTTON_TEXT_Y;
	s_joyenable_box.listnames				= s_freelook_Names;


	y += 22;
	s_joythreshold_slider.generic.type		= MTYPE_SLIDER;
	s_joythreshold_slider.generic.x			= x + 162;
	s_joythreshold_slider.generic.y			= y;
	s_joythreshold_slider.generic.flags		= QMF_SMALLFONT;
	s_joythreshold_slider.generic.id		= ID_MOUSESPEED;
	s_joythreshold_slider.generic.callback	= JoystickThresholdCallback;
//	s_joythreshold_slider.minvalue			= 2;
//	s_joythreshold_slider.maxvalue			= 30;
	s_joythreshold_slider.minvalue			= 0.05f;
	s_joythreshold_slider.maxvalue			= .75;
	s_joythreshold_slider.color				= CT_DKPURPLE1;
	s_joythreshold_slider.color2			= CT_LTPURPLE1;
	s_joythreshold_slider.generic.name		= "menu/common/monbar_2.tga";
	s_joythreshold_slider.width				= 256;
	s_joythreshold_slider.height			= 32;
	s_joythreshold_slider.focusWidth		= 145;
	s_joythreshold_slider.focusHeight		= 18;
	s_joythreshold_slider.picName			= "menu/common/square.tga";
	s_joythreshold_slider.picX				= x;
	s_joythreshold_slider.picY				= y;
	s_joythreshold_slider.picWidth			= MENU_BUTTON_MED_WIDTH + 21;
	s_joythreshold_slider.picHeight			= MENU_BUTTON_MED_HEIGHT;
	s_joythreshold_slider.textX				= MENU_BUTTON_TEXT_X;
	s_joythreshold_slider.textY				= MENU_BUTTON_TEXT_Y;
	s_joythreshold_slider.textEnum			= MBT_JOYSTICKTHRESHOLD;
	s_joythreshold_slider.textcolor			= CT_BLACK;
	s_joythreshold_slider.textcolor2		= CT_WHITE;
	s_joythreshold_slider.thumbName			= GRAPHIC_BUTTONSLIDER;
	s_joythreshold_slider.thumbHeight		= 32;
	s_joythreshold_slider.thumbWidth		= 16;
	s_joythreshold_slider.thumbGraphicWidth	= 9;
	s_joythreshold_slider.thumbColor		= CT_DKBLUE1;
	s_joythreshold_slider.thumbColor2		= CT_LTBLUE1;

	y += 22;
	s_gplook_slider.generic.type			= MTYPE_SLIDER;
	s_gplook_slider.generic.x				= x + 162;
	s_gplook_slider.generic.y				= y;
	s_gplook_slider.generic.flags			= QMF_SMALLFONT;
	s_gplook_slider.generic.id				= ID_MOUSESPEED;
	s_gplook_slider.generic.callback		= GamepadLookCallback;
	// 10 integer notches (0.25..2.5 step 0.25) mapped to the float cvar in the callback / load / save below.
	// The generic Slider_Key arrow step is a hardcoded +-1 (ui_qmenu.cpp); a float 0.25..2.5 range spanned
	// <2 steps so any d-pad nudge slammed the thumb to an end ("won't stay put"). Integer notches match how
	// retail's own sliders work (s_sensitivity_slider 2..30, s_keyturnspeed_slider 1..5).
	s_gplook_slider.minvalue				= 1;
	s_gplook_slider.maxvalue				= 10;
	s_gplook_slider.color					= CT_DKPURPLE1;
	s_gplook_slider.color2					= CT_LTPURPLE1;
	s_gplook_slider.generic.name			= "menu/common/monbar_2.tga";
	s_gplook_slider.width					= 256;
	s_gplook_slider.height					= 32;
	s_gplook_slider.focusWidth				= 145;
	s_gplook_slider.focusHeight				= 18;
	s_gplook_slider.picName					= "menu/common/square.tga";
	s_gplook_slider.picX					= x;
	s_gplook_slider.picY					= y;
	s_gplook_slider.picWidth				= MENU_BUTTON_MED_WIDTH + 21;
	s_gplook_slider.picHeight				= MENU_BUTTON_MED_HEIGHT;
	s_gplook_slider.textX					= MENU_BUTTON_TEXT_X;
	s_gplook_slider.textY					= MENU_BUTTON_TEXT_Y;
	s_gplook_slider.textEnum				= MBT_GAMEPADLOOK;
	s_gplook_slider.textcolor				= CT_BLACK;
	s_gplook_slider.textcolor2				= CT_WHITE;
	s_gplook_slider.thumbName				= GRAPHIC_BUTTONSLIDER;
	s_gplook_slider.thumbHeight				= 32;
	s_gplook_slider.thumbWidth				= 16;
	s_gplook_slider.thumbGraphicWidth		= 9;
	s_gplook_slider.thumbColor				= CT_DKBLUE1;
	s_gplook_slider.thumbColor2				= CT_LTBLUE1;

	Menu_AddItem( &s_controlsmouse_menu, ( void * )&s_freelook_box);
	Menu_AddItem( &s_controlsmouse_menu, ( void * )&s_sensitivity_slider);
	Menu_AddItem( &s_controlsmouse_menu, ( void * )&s_invertmouse_box);
	Menu_AddItem( &s_controlsmouse_menu, ( void * )&s_smoothmouse_box);
	Menu_AddItem( &s_controlsmouse_menu, ( void * )&s_forcefeedback_box);

	// The two gamepad-sensitivity sliders are appended MBT_ enums with no entry in the pk3-shipped
	// sp_buttontext.dat (they parse as "?"), so assign their labels here. menu_button_text[][0] is the
	// normal label, [][1] the highlighted label; both are plain char* (see UI_ParseButtonText).
	menu_button_text[MBT_GAMEPADLOOK][0] = "GAMEPAD LOOK";
	menu_button_text[MBT_GAMEPADLOOK][1] = "GAMEPAD LOOK";

	Menu_AddItem( &s_controlsmouse_menu, ( void * )&s_joyenable_box);
	Menu_AddItem( &s_controlsmouse_menu, ( void * )&s_joythreshold_slider);
	Menu_AddItem( &s_controlsmouse_menu, ( void * )&s_gplook_slider);

	// initialize the configurable cvars
	Controls_InitCvars();

	// initialize the current config
	Controls_GetConfig();

	// initial default section
	g_waitingforkey  = qfalse;
	g_section        = C_MOUSE;
	g_changesmade    = qfalse;
	g_prompttosave   = qfalse;

	// update the ui
	Controls_UpdateNew();
}

/*
=================
UI_ControlsMouseJoyStickGetCvars
=================
*/
static void	UI_ControlsMouseJoyStickGetCvars()
{

	FreeLookGet(&s_freelook_box);	// Get mouse free look

	// Mouse Speed
	s_sensitivity_slider.curvalue  = Controls_ClampCvar( 2, 30, Controls_GetCvarValue( "sensitivity" ) );
	s_invertmouse_box.curvalue     = Controls_GetCvarValue( "m_pitch" ) < 0;


	s_smoothmouse_box.curvalue = Controls_GetCvarValue( "m_filter" );

	s_forcefeedback_box.curvalue = Controls_GetCvarValue( "use_ff" );
}

/*
=================
UI_ControlsMouseJoyStickMenu
=================
*/
static void UI_ControlsMouseJoyStickMenu( void )
{
	ui.Key_SetCatcher( KEYCATCH_UI );

	ControlsMouseJoyStick_MenuInit();

	UI_ControlsMouseJoyStickGetCvars();

	UI_PushMenu( &s_controlsmouse_menu );
}



/*
=================
M_ControlsOtherMenu_Graphics
=================
*/
void M_ControlsOtherMenu_Graphics (void)
{
	// Draw the basic screen layout
	UI_MenuFrame(&s_controlsother_menu);

	ui.R_SetColor( colorTable[CT_DKPURPLE2]);
	UI_DrawHandlePic( 210,  204,  402,  8, uis.whiteShader);	// Long thin line below turn speed
	UI_DrawHandlePic( 545,  212,  67,  140, uis.whiteShader);	// Thick right column
	UI_DrawHandlePic( 210,  344,  402,  8, uis.whiteShader);	// Long thin line below autoswitch
	UI_DrawHandlePic( 210,  352,  10,  83, uis.whiteShader);	// Thin left column

	UI_DrawProportionalString(  608,  340, "1701 - B",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);

	UI_DrawProportionalString(  280,  419, "500987",UI_RIGHT|UI_TINYFONT, colorTable[CT_DKGOLD1]);
	UI_DrawProportionalString(  358,  419, "456732",UI_RIGHT|UI_TINYFONT, colorTable[CT_DKGOLD1]);
	UI_DrawProportionalString(  436,  419, "67024",UI_RIGHT|UI_TINYFONT, colorTable[CT_DKGOLD1]);
	UI_DrawProportionalString(  514,  419, "166110",UI_RIGHT|UI_TINYFONT, colorTable[CT_DKGOLD1]);
	UI_DrawProportionalString(  592,  419, "575",UI_RIGHT|UI_TINYFONT, colorTable[CT_DKGOLD1]);

	ui.R_SetColor( colorTable[CT_DKPURPLE3]);
	UI_DrawHandlePic( 288,  364,  32,  64, s_controlsother.mon_bar);	// 
	UI_DrawHandlePic( 366,  364,  32,  64, s_controlsother.mon_bar);	// 
	UI_DrawHandlePic( 444,  364,  32,  64, s_controlsother.mon_bar);	// 
	UI_DrawHandlePic( 522,  364,  32,  64, s_controlsother.mon_bar);	// 
	UI_DrawHandlePic( 600,  364,  32,  64, s_controlsother.mon_bar);	// 


	UI_DrawProportionalString(  74,  66, "5-0987",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  84, "16116",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  188, "28430",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  395, "6900",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);

}

/*
=================
ControlsOther_MenuDraw
=================
*/
static void ControlsOther_MenuDraw (void)
{
	M_ControlsOtherMenu_Graphics();

	UI_Setup_MenuButtons();

	Menu_Draw( &s_controlsother_menu );
}

/*
=================
ControlsOther_MenuKey
=================
*/
static sfxHandle_t ControlsOther_MenuKey( int key )
{
	return Menu_DefaultKey( &s_controlsother_menu, key );
}

/*
=================
UI_ControlsOther_Cache
=================
*/
void UI_ControlsOther_Cache(void)
{
	s_controlsother.mon_bar = ui.R_RegisterShaderNoMip("menu/common/mon_bar.tga");
	ui.R_RegisterShaderNoMip("menu/common/monbar_2.tga");

}

/*
=================
ControlsOther_MenuInit
=================
*/
static void ControlsOther_MenuInit( void )
{
	int x,y;

	UI_ControlsOther_Cache();

	s_controlsother_menu.nitems					= 0;
	s_controlsother_menu.wrapAround				= qtrue;
	s_controlsother_menu.opening				= NULL;
	s_controlsother_menu.closing				= NULL;
	s_controlsother_menu.draw					= ControlsOther_MenuDraw;
	s_controlsother_menu.key					= ControlsOther_MenuKey;
	s_controlsother_menu.fullscreen				= qtrue;
	s_controlsother_menu.descX					= MENU_DESC_X;
	s_controlsother_menu.descY					= MENU_DESC_Y;
	s_controlsother_menu.listX					= 230;
	s_controlsother_menu.listY					= 188;
	s_controlsother_menu.titleX					= MENU_TITLE_X;
	s_controlsother_menu.titleY					= MENU_TITLE_Y;
	s_controlsother_menu.titleI					= MNT_CONTROLSMENU_TITLE;
	s_controlsother_menu.footNoteEnum			= MNT_OTHEROPTIONS_SETUP;

	SetupMenu_TopButtons(&s_controlsother_menu,MENU_CONTROLS,NULL);
	SetupMenu_SideButtons(&s_controlsother_menu,MENU_CONTROLS_OTHER);

	s_controls_other.textcolor					= CT_LTGOLD1;
	s_controls_other.textcolor2					= CT_LTGOLD1;

	static int s_alwaysrun_Names[] =
	{
		MNT_OFF,
		MNT_ON,
		MNT_NONE
	};

	static int s_autoswitch_Names[] =
	{
		MNT_OFF,
		MNT_SAFE,
		MNT_BEST,
		MNT_NONE
	};

	x = 210;
	y = 172;

	s_keyturnspeed_slider.generic.type		= MTYPE_SLIDER;
	s_keyturnspeed_slider.generic.x			= x + 162;
	s_keyturnspeed_slider.generic.y			= y;
	s_keyturnspeed_slider.generic.flags		= QMF_SMALLFONT;
	s_keyturnspeed_slider.generic.id		= ID_KEYTURNPEED;
	s_keyturnspeed_slider.generic.callback	= TurnSpeedCallback;
	s_keyturnspeed_slider.minvalue			= 1;
	s_keyturnspeed_slider.maxvalue			= 5;
	s_keyturnspeed_slider.color				= CT_DKPURPLE1;
	s_keyturnspeed_slider.color2			= CT_LTPURPLE1;
	s_keyturnspeed_slider.generic.name		= "menu/common/monbar_2.tga";
	s_keyturnspeed_slider.width				= 256;
	s_keyturnspeed_slider.height			= 32;
	s_keyturnspeed_slider.focusWidth		= 145;
	s_keyturnspeed_slider.focusHeight		= 18;
	s_keyturnspeed_slider.picName			= "menu/common/square.tga";
	s_keyturnspeed_slider.picX				= x;
	s_keyturnspeed_slider.picY				= y;
	s_keyturnspeed_slider.picWidth			= MENU_BUTTON_MED_WIDTH + 21;
	s_keyturnspeed_slider.picHeight			= MENU_BUTTON_MED_HEIGHT;
	s_keyturnspeed_slider.textX				= 5;
	s_keyturnspeed_slider.textY				= 1;
	s_keyturnspeed_slider.textEnum			= MBT_KEYTURNSPEED;
	s_keyturnspeed_slider.textcolor			= CT_BLACK;
	s_keyturnspeed_slider.textcolor2		= CT_WHITE;
	s_keyturnspeed_slider.thumbName			= GRAPHIC_BUTTONSLIDER;
	s_keyturnspeed_slider.thumbHeight		= 32;
	s_keyturnspeed_slider.thumbWidth		= 16;
	s_keyturnspeed_slider.thumbGraphicWidth	= 9;
	s_keyturnspeed_slider.thumbColor		= CT_DKBLUE1;
	s_keyturnspeed_slider.thumbColor2		= CT_LTBLUE1;


	y =225;
	s_alwaysrun_box.generic.type			= MTYPE_SPINCONTROL;
	s_alwaysrun_box.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_alwaysrun_box.generic.x				= x;
	s_alwaysrun_box.generic.y				= y;
	s_alwaysrun_box.generic.callback		= AlwaysRunCallback;
	s_alwaysrun_box.generic.id			    = ID_ALWAYSRUN;
	s_alwaysrun_box.textEnum				= MBT_ALWAYSRUN;
	s_alwaysrun_box.textcolor				= CT_BLACK;
	s_alwaysrun_box.textcolor2				= CT_WHITE;
	s_alwaysrun_box.color					= CT_DKPURPLE1;
	s_alwaysrun_box.color2					= CT_LTPURPLE1;
	s_alwaysrun_box.textX					= 5;
	s_alwaysrun_box.textY					= 2;
	s_alwaysrun_box.listnames				= s_alwaysrun_Names;

/*	y +=22;
	s_lookspring_box.generic.type			= MTYPE_SPINCONTROL;
	s_lookspring_box.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_lookspring_box.generic.x				= x;
	s_lookspring_box.generic.y				= y;
	s_lookspring_box.generic.callback		= Controls_MenuEventNew;
	s_lookspring_box.generic.id			    = ID_LOOKSPRING;
	s_lookspring_box.textEnum				= MBT_LOOKSPRING;
	s_lookspring_box.textcolor				= CT_BLACK;
	s_lookspring_box.textcolor2				= CT_WHITE;
	s_lookspring_box.color					= CT_DKPURPLE1;
	s_lookspring_box.color2					= CT_LTPURPLE1;
	s_lookspring_box.textX					= 5;
	s_lookspring_box.textY					= 2;
	s_lookspring_box.listnames				= s_alwaysrun_Names;
*/
	y +=22;
/*	s_captioning_box.generic.type			= MTYPE_SPINCONTROL;
	s_captioning_box.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_captioning_box.generic.x				= x;
	s_captioning_box.generic.y				= y;
	s_captioning_box.generic.callback		= Controls_MenuEventNew;
	s_captioning_box.generic.id			    = ID_CAPTIONING;
	s_captioning_box.textEnum				= MBT_CAPTIONING;
	s_captioning_box.textcolor				= CT_BLACK;
	s_captioning_box.textcolor2				= CT_WHITE;
	s_captioning_box.color					= CT_DKPURPLE1;
	s_captioning_box.color2					= CT_LTPURPLE1;
	s_captioning_box.textX					= 5;
	s_captioning_box.textY					= 2;
	s_captioning_box.listnames				= s_alwaysrun_Names;
*/

	y +=22;
	s_autoswitch_box.generic.type			= MTYPE_SPINCONTROL;
	s_autoswitch_box.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_autoswitch_box.generic.x				= x;
	s_autoswitch_box.generic.y				= y;
	s_autoswitch_box.generic.callback		= AutoswitchCallback;
	s_autoswitch_box.generic.id			    = ID_AUTOSWITCHWEAPONS;
	s_autoswitch_box.textEnum				= MBT_AUTOSWITCHWEAPONS;
	s_autoswitch_box.textcolor				= CT_BLACK;
	s_autoswitch_box.textcolor2				= CT_WHITE;
	s_autoswitch_box.color					= CT_DKPURPLE1;
	s_autoswitch_box.color2					= CT_LTPURPLE1;
	s_autoswitch_box.textX					= 5;
	s_autoswitch_box.textY					= 2;
	s_autoswitch_box.listnames				= s_autoswitch_Names;


	Menu_AddItem( &s_controlsother_menu, ( void * )&s_alwaysrun_box);
//	Menu_AddItem( &s_controlsother_menu, ( void * )&s_lookspring_box);
	Menu_AddItem( &s_controlsother_menu, ( void * )&s_keyturnspeed_slider);
//	Menu_AddItem( &s_controlsother_menu, ( void * )&s_captioning_box);
	Menu_AddItem( &s_controlsother_menu, ( void * )&s_autoswitch_box);

	// initialize the configurable cvars
	Controls_InitCvars();

	// initialize the current config
	Controls_GetConfig();

	// initial default section
	g_waitingforkey  = qfalse;
	g_section        = C_OTHER;
	g_changesmade    = qfalse;
	g_prompttosave   = qfalse;

	// update the ui
	Controls_UpdateNew();
}

/*
=================
UI_ControlsOtherGetCvars
=================
*/
void UI_ControlsOtherGetCvars(void)
{
	s_alwaysrun_box.curvalue = Controls_ClampCvar( 0, 1, Controls_GetCvarValue( "cl_run" ) );
	s_autoswitch_box.curvalue  = Controls_ClampCvar( 0, 2, Controls_GetCvarValue( "cg_autoswitch" ) );
}

/*
=================
UI_ControlsOtherMenu
=================
*/
static void UI_ControlsOtherMenu( void )
{
	ui.Key_SetCatcher( KEYCATCH_UI );

	UI_ControlsOtherGetCvars();

//	if (!s_controls_menu.initialized)
//	{
		ControlsOther_MenuInit();
//	}

	UI_PushMenu( &s_controlsother_menu );
}


/*
=================
M_ControlsOtherMenu_Graphics
=================
*/
void M_ControlsDefaultMenu_Graphics (void)
{
	// Draw the basic screen layout
	UI_MenuFrame(&s_controlsdefault_menu);

	ui.R_SetColor( colorTable[CT_LTBROWN1]);
	UI_DrawHandlePic(30,203,  47, 186, uis.whiteShader);		// Long left column square on bottom 3rd

	ui.R_SetColor( colorTable[CT_RED]);
	UI_DrawHandlePic(132,175,  425, 18, uis.whiteShader);		// Top
	UI_DrawHandlePic(132,193,  47, 175, uis.whiteShader);		// Left side
	UI_DrawHandlePic(510,193,  47, 175, uis.whiteShader);		// Right side
	UI_DrawHandlePic(132,365,  80, 18, uis.whiteShader);		// Bottom Left
	UI_DrawHandlePic(477,365,  80, 18, uis.whiteShader);		// Bottom Right


	UI_DrawProportionalString(345,210,menu_normal_text[MNT_DEFAULT_WARNING1],UI_BIGFONT | UI_BLINK | UI_CENTER,colorTable[CT_RED]);
	UI_DrawProportionalString(345,263,menu_normal_text[MNT_DEFAULT_WARNING2],UI_SMALLFONT | UI_CENTER,colorTable[CT_RED]);
	UI_DrawProportionalString(345,283,menu_normal_text[MNT_DEFAULT_WARNING3],UI_SMALLFONT | UI_CENTER,colorTable[CT_RED]);
	UI_DrawProportionalString(345,321,menu_normal_text[MNT_SAVE_WARNING3],UI_SMALLFONT | UI_CENTER,colorTable[CT_RED]);

}

/*
=================
ControlsDefault_MenuDraw
=================
*/
static void ControlsDefault_MenuDraw (void)
{
	M_ControlsDefaultMenu_Graphics();

	UI_Setup_MenuButtons();

	Menu_Draw( &s_controlsdefault_menu );
}

static void Default_ResetDefaults( void )
{
	ui.Cmd_ExecuteText( EXEC_APPEND, "exec default.cfg\n");
	ui.Cmd_ExecuteText( EXEC_APPEND, "cvar_restart\n");
	ui.Cmd_ExecuteText( EXEC_APPEND, "vid_restart\n" );
}


/*
=================
M_Default_Event
=================
*/
void M_Default_Event (void* ptr, int event)
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
		case ID_DEFAULT_YES:
			Default_ResetDefaults();
			break;

		// No, go to main menu
		case ID_DEFAULT_NO:
			UI_PopMenu();
			break;

	}
}

/*
=================
ControlsDefault_MenuKey
=================
*/
static sfxHandle_t ControlsDefault_MenuKey( int key )
{
	return Menu_DefaultKey( &s_controlsdefault_menu, key );
}

/*
=================
ControlsOther_MenuInit
=================
*/
static void ControlsDefault_MenuInit( void )
{

	s_controlsdefault_menu.nitems					= 0;
	s_controlsdefault_menu.wrapAround				= qtrue;
	s_controlsdefault_menu.opening					= NULL;
	s_controlsdefault_menu.closing					= NULL;
	s_controlsdefault_menu.draw						= ControlsDefault_MenuDraw;
	s_controlsdefault_menu.key						= ControlsDefault_MenuKey;
	s_controlsdefault_menu.fullscreen				= qtrue;
	s_controlsdefault_menu.descX					= MENU_DESC_X;
	s_controlsdefault_menu.descY					= MENU_DESC_Y;
	s_controlsdefault_menu.listX					= 230;
	s_controlsdefault_menu.listY					= 188;
	s_controlsdefault_menu.titleX					= MENU_TITLE_X;
	s_controlsdefault_menu.titleY					= MENU_TITLE_Y;
	s_controlsdefault_menu.titleI					= MNT_CONTROLSMENU_TITLE;
	s_controlsdefault_menu.footNoteEnum				= MNT_DEFAULT_SETUP;

	SetupMenu_TopButtons(&s_controlsdefault_menu,MENU_DEFAULT,NULL);

	s_controls_other.textcolor						= CT_LTGOLD1;
	s_controls_other.textcolor2						= CT_LTGOLD1;

	s_controls_default_yes.generic.type				= MTYPE_BITMAP;      
	s_controls_default_yes.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_controls_default_yes.generic.x				= 215;
	s_controls_default_yes.generic.y				= 365;
	s_controls_default_yes.generic.name				= "menu/common/square.tga";
	s_controls_default_yes.generic.id				= ID_DEFAULT_YES;
	s_controls_default_yes.generic.callback			= M_Default_Event; 
	s_controls_default_yes.width					= 103;
	s_controls_default_yes.height					= MENU_BUTTON_MED_HEIGHT;
	s_controls_default_yes.color					= CT_DKPURPLE1;
	s_controls_default_yes.color2					= CT_LTPURPLE1;
	s_controls_default_yes.textX					= MENU_BUTTON_TEXT_X;
	s_controls_default_yes.textY					= MENU_BUTTON_TEXT_Y;
	s_controls_default_yes.textEnum					= MBT_DEFAULT_YES;
	s_controls_default_yes.textcolor				= CT_BLACK;
	s_controls_default_yes.textcolor2				= CT_WHITE;
	s_controls_default_yes.textStyle				= UI_SMALLFONT;

	s_controls_default_no.generic.type				= MTYPE_BITMAP;      
	s_controls_default_no.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS;
	s_controls_default_no.generic.x					= 371;
	s_controls_default_no.generic.y					= 365;
	s_controls_default_no.generic.name				= "menu/common/square.tga";
	s_controls_default_no.generic.id				= ID_DEFAULT_NO;
	s_controls_default_no.generic.callback			= M_Default_Event; 
	s_controls_default_no.width						= 103;
	s_controls_default_no.height					= MENU_BUTTON_MED_HEIGHT;
	s_controls_default_no.color						= CT_DKPURPLE1;
	s_controls_default_no.color2					= CT_LTPURPLE1;
	s_controls_default_no.textX						= MENU_BUTTON_TEXT_X;
	s_controls_default_no.textY						= MENU_BUTTON_TEXT_Y;
	s_controls_default_no.textEnum					= MBT_DEFAULT_NO;
	s_controls_default_no.textcolor					= CT_BLACK;
	s_controls_default_no.textcolor2				= CT_WHITE;
	s_controls_default_no.textStyle					= UI_SMALLFONT;

	Menu_AddItem( &s_controlsdefault_menu, ( void * )&s_controls_default_yes);
	Menu_AddItem( &s_controlsdefault_menu, ( void * )&s_controls_default_no);

}

/*
=================
UI_ControlsDefaultMenu
=================
*/
static void UI_ControlsDefaultMenu( void )
{
	ui.Key_SetCatcher( KEYCATCH_UI );

	ControlsDefault_MenuInit();

	UI_PushMenu( &s_controlsdefault_menu );
}

/*
=================
SetupMenu_TopButtons
=================
*/

void SetupMenu_TopButtons(menuframework_s *menu,int menuType,menuaction_s *s_video_apply_action)
{
	vid_apply_action = s_video_apply_action;

	s_controls_mainmenu.generic.type			= MTYPE_BITMAP;      
	s_controls_mainmenu.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_controls_mainmenu.generic.x				= 482;
	s_controls_mainmenu.generic.y				= 136;
	s_controls_mainmenu.generic.name			= GRAPHIC_BUTTONRIGHT;
	s_controls_mainmenu.generic.id				= ID_MAINMENU;
	if (menuType != MENU_VIDEODATA)
	{
		s_controls_mainmenu.generic.callback	= Controls_MenuEventNew;
	}
	else	// How do you spell HACK?
	{
		s_controls_mainmenu.generic.callback	= Controls_MenuEventVideo;
	}

	s_controls_mainmenu.width					= MENU_BUTTON_MED_WIDTH;
	s_controls_mainmenu.height					= MENU_BUTTON_MED_HEIGHT;
	s_controls_mainmenu.color					= CT_DKPURPLE1;
	s_controls_mainmenu.color2					= CT_LTPURPLE1;
	s_controls_mainmenu.textX					= 5;
	s_controls_mainmenu.textY					= 2;
	if (!ingameFlag)
	{
		s_controls_mainmenu.textEnum			= MBT_MAINMENU;
	}
	else	// In game menu
	{
		s_controls_mainmenu.textEnum			= MBT_INGAMEMENU;
	}
	s_controls_mainmenu.textcolor				= CT_BLACK;
	s_controls_mainmenu.textcolor2				= CT_WHITE;

	s_controls_controls.generic.type		= MTYPE_BITMAP;      
	s_controls_controls.generic.flags	= QMF_HIGHLIGHT_IF_FOCUS;
	s_controls_controls.generic.x		= setup_menubuttons[0][0];
	s_controls_controls.generic.y		= setup_menubuttons[0][1];
	s_controls_controls.generic.name	= GRAPHIC_BUTTONRIGHT;
	s_controls_controls.generic.id		= ID_CONTROLS;
	if (menuType != MENU_VIDEODATA)
	{
		s_controls_controls.generic.callback	= Controls_MenuEventNew;
	}
	else	// How do you spell HACK?
	{
		s_controls_controls.generic.callback	= Controls_MenuEventVideo;
	}
	s_controls_controls.width			= MENU_BUTTON_MED_WIDTH;
	s_controls_controls.height			= MENU_BUTTON_MED_HEIGHT;
	s_controls_controls.color			= CT_DKPURPLE1;
	s_controls_controls.color2			= CT_LTPURPLE1;
	s_controls_controls.textX			= 5;
	s_controls_controls.textY			= 2;
	s_controls_controls.textEnum		= MBT_CONTROLS;
	s_controls_controls.textcolor		= CT_BLACK;
	s_controls_controls.textcolor2		= CT_WHITE;

	s_controls_video.generic.type		= MTYPE_BITMAP;      
	s_controls_video.generic.flags		= QMF_HIGHLIGHT_IF_FOCUS;
	s_controls_video.generic.x			= setup_menubuttons[1][0];
	s_controls_video.generic.y			= setup_menubuttons[1][1];
	s_controls_video.generic.name		= GRAPHIC_BUTTONRIGHT;
	s_controls_video.generic.id			= ID_VIDEO;
	if (menuType != MENU_VIDEODATA)
	{
		s_controls_video.generic.callback	= Controls_MenuEventNew;
	}
	else
	{
		s_controls_video.generic.callback	= Controls_MenuEventVideo;
	}
	s_controls_video.width				= MENU_BUTTON_MED_WIDTH;
	s_controls_video.height				= MENU_BUTTON_MED_HEIGHT;
	s_controls_video.color				= CT_DKPURPLE1;
	s_controls_video.color2				= CT_LTPURPLE1;
	s_controls_video.textX				= 5;
	s_controls_video.textY				= 2;
	s_controls_video.textEnum			= MBT_VIDEOSETTINGS;
	s_controls_video.textcolor			= CT_BLACK;
	s_controls_video.textcolor2			= CT_WHITE;

	s_controls_sound.generic.type		= MTYPE_BITMAP;      
	s_controls_sound.generic.flags		= QMF_HIGHLIGHT_IF_FOCUS;
	s_controls_sound.generic.x			= setup_menubuttons[2][0];
	s_controls_sound.generic.y			= setup_menubuttons[2][1];
	s_controls_sound.generic.name		= GRAPHIC_BUTTONRIGHT;
	s_controls_sound.generic.id			= ID_SOUND;
	if (menuType != MENU_VIDEODATA)
	{
		s_controls_sound.generic.callback	= Controls_MenuEventNew;
	}
	else
	{
		s_controls_sound.generic.callback	= Controls_MenuEventVideo;
	}
	s_controls_sound.width				= MENU_BUTTON_MED_WIDTH;
	s_controls_sound.height				= MENU_BUTTON_MED_HEIGHT;
	s_controls_sound.color				= CT_DKPURPLE1;
	s_controls_sound.color2				= CT_LTPURPLE1;
	s_controls_sound.textX				= 5;
	s_controls_sound.textY				= 2;
	s_controls_sound.textEnum			= MBT_SOUNDSETTINGS;
	s_controls_sound.textcolor			= CT_BLACK;
	s_controls_sound.textcolor2			= CT_WHITE;

	s_controls_game.generic.type		= MTYPE_BITMAP;      
	s_controls_game.generic.flags		= QMF_HIGHLIGHT_IF_FOCUS;
	s_controls_game.generic.x			= setup_menubuttons[3][0];
	s_controls_game.generic.y			= setup_menubuttons[3][1];
	s_controls_game.generic.name		=GRAPHIC_BUTTONRIGHT;
	s_controls_game.generic.id			= ID_GAMEOPTIONS;
	if (menuType != MENU_VIDEODATA)
	{
		s_controls_game.generic.callback	= Controls_MenuEventNew;
	}
	else
	{
		s_controls_game.generic.callback	= Controls_MenuEventVideo;
	}
	s_controls_game.width				= MENU_BUTTON_MED_WIDTH;
	s_controls_game.height				= MENU_BUTTON_MED_HEIGHT;
	s_controls_game.color				= CT_DKPURPLE1;
	s_controls_game.color2				= CT_LTPURPLE1;
	s_controls_game.textX				= 5;
	s_controls_game.textY				= 2;
	s_controls_game.textEnum			= MBT_GAMEOPTIONS;
	s_controls_game.textcolor			= CT_BLACK;
	s_controls_game.textcolor2			= CT_WHITE;
/*
	s_controls_cdkey.generic.type		= MTYPE_BITMAP;      
	s_controls_cdkey.generic.flags	= QMF_HIGHLIGHT_IF_FOCUS;
	s_controls_cdkey.generic.x		= setup_menubuttons[4][0];
	s_controls_cdkey.generic.y		= setup_menubuttons[4][1];
	s_controls_cdkey.generic.name	= "menu/common/bar1.tga";
	s_controls_cdkey.generic.id		= ID_CDKEY;
	s_controls_cdkey.generic.callback	= Controls_MenuEventNew;
	s_controls_cdkey.width			= 100;
	s_controls_cdkey.height			= MENU_BUTTON_MED_HEIGHT;
	s_controls_cdkey.color			= CT_DKPURPLE1;
	s_controls_cdkey.color2			= CT_LTPURPLE1;
	s_controls_cdkey.textX			= 5;
	s_controls_cdkey.textY			= 2;
	s_controls_cdkey.textEnum		= MBT_CDKEY;
	s_controls_cdkey.textcolor		= CT_BLACK;
	s_controls_cdkey.textcolor2		= CT_WHITE;
*/
	s_controls_default.generic.type		= MTYPE_BITMAP;      
	s_controls_default.generic.flags	= QMF_HIGHLIGHT_IF_FOCUS;
	s_controls_default.generic.x		= setup_menubuttons[4][0];
	s_controls_default.generic.y		= setup_menubuttons[4][1];
	s_controls_default.generic.name		= GRAPHIC_BUTTONRIGHT;
	s_controls_default.generic.id		= ID_CONTROLSDEFAULT;
	if (menuType != MENU_VIDEODATA)
	{
		s_controls_default.generic.callback	= Controls_MenuEventNew;
	}
	else
	{
		s_controls_default.generic.callback	= Controls_MenuEventVideo;
	}
	s_controls_default.width			= MENU_BUTTON_MED_WIDTH;
	s_controls_default.height			= MENU_BUTTON_MED_HEIGHT;
	s_controls_default.color			= CT_DKPURPLE1;
	s_controls_default.color2			= CT_LTPURPLE1;
	s_controls_default.textX			= 5;
	s_controls_default.textY			= 2;
	s_controls_default.textEnum			= MBT_SETDEFAULT;
	s_controls_default.textcolor		= CT_BLACK;
	s_controls_default.textcolor2		= CT_WHITE;

	s_controls_fonts.generic.type		= MTYPE_BITMAP;      
	s_controls_fonts.generic.flags	= QMF_HIGHLIGHT_IF_FOCUS;
	s_controls_fonts.generic.x		= setup_menubuttons[5][0];
	s_controls_fonts.generic.y		= setup_menubuttons[5][1];
	s_controls_fonts.generic.name	= GRAPHIC_BUTTONRIGHT;
	s_controls_fonts.generic.id		= ID_FONTS;
	if (menuType != MENU_VIDEODATA)
	{
		s_controls_fonts.generic.callback	= Controls_MenuEventNew;
	}
	else
	{
		s_controls_fonts.generic.callback	= Controls_MenuEventVideo;
	}
	s_controls_fonts.width			= MENU_BUTTON_MED_WIDTH;
	s_controls_fonts.height			= MENU_BUTTON_MED_HEIGHT;
	s_controls_fonts.color			= CT_DKPURPLE1;
	s_controls_fonts.color2			= CT_LTPURPLE1;
	s_controls_fonts.textX			= 5;
	s_controls_fonts.textY			= 2;
	s_controls_fonts.textEnum		= MBT_FONTS;
	s_controls_fonts.textcolor		= CT_BLACK;
	s_controls_fonts.textcolor2		= CT_WHITE;

	Menu_AddItem( menu, ( void * )&s_controls_mainmenu);
	Menu_AddItem( menu, ( void * )&s_controls_controls);
	Menu_AddItem( menu, ( void * )&s_controls_video);
	Menu_AddItem( menu, ( void * )&s_controls_sound);
	Menu_AddItem( menu, ( void * )&s_controls_game);
	Menu_AddItem( menu, ( void * )&s_controls_default);
//	Menu_AddItem( menu, ( void * )&s_controls_cdkey);

	switch (menuType)
	{
	case MENU_CONTROLS :
		s_controls_controls.textcolor		= CT_LTGOLD1;	
		s_controls_controls.textcolor2		= CT_LTGOLD1;	
		s_controls_controls.generic.flags	= QMF_GRAYED;
		break;
	case MENU_VIDEO :
	case MENU_VIDEODATA :
		s_controls_video.textcolor		= CT_LTGOLD1;	
		s_controls_video.textcolor2		= CT_LTGOLD1;	
		s_controls_video.generic.flags	= QMF_GRAYED;
		break;
	case MENU_SOUND :
		s_controls_sound.textcolor		= CT_LTGOLD1;	
		s_controls_sound.textcolor2		= CT_LTGOLD1;	
		s_controls_sound.generic.flags	= QMF_GRAYED;
		break;
	case MENU_GAME :
		s_controls_game.textcolor		= CT_LTGOLD1;	
		s_controls_game.textcolor2		= CT_LTGOLD1;	
		s_controls_game.generic.flags	= QMF_GRAYED;
		break;
	case MENU_DEFAULT :
		s_controls_default.textcolor		= CT_LTGOLD1;	
		s_controls_default.textcolor2		= CT_LTGOLD1;	
		s_controls_default.generic.flags	= QMF_GRAYED;
		break;
	case MENU_CDKEY :
		s_controls_cdkey.textcolor		= CT_LTGOLD1;	
		s_controls_cdkey.textcolor2		= CT_LTGOLD1;	
		s_controls_cdkey.generic.flags	= QMF_GRAYED;
		break;
	case MENU_FONTS :
		s_controls_fonts.textcolor		= CT_LTGOLD1;	
		s_controls_fonts.textcolor2		= CT_LTGOLD1;	
		s_controls_fonts.generic.flags	= QMF_GRAYED;
		break;
	}


	if (ui.Cvar_VariableValue("developer"))
		Menu_AddItem( menu, ( void * )&s_controls_fonts);

}

/*
===============
SetupMenu_SideButtons
===============
*/
static void SetupMenu_SideButtons(menuframework_s *menu,int menuType)
{
	int x,y,inc;

	y = 204;
	inc = 6;
	x = 30;

	s_controls_weapon.generic.type			= MTYPE_BITMAP;      
	s_controls_weapon.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_controls_weapon.generic.x				= x;
	s_controls_weapon.generic.y				= y;
	s_controls_weapon.generic.name			= "menu/common/square.tga";
	s_controls_weapon.generic.id			= ID_ATTACK;
	s_controls_weapon.generic.callback		= Controls_MenuEventNew;
	s_controls_weapon.width					= MENU_BUTTON_MED_WIDTH - 10;
	s_controls_weapon.height				= MENU_BUTTON_MED_HEIGHT;
	s_controls_weapon.color					= CT_DKPURPLE1;
	s_controls_weapon.color2				= CT_LTPURPLE1;
	s_controls_weapon.textX					= 5;
	s_controls_weapon.textY					= 1;
	s_controls_weapon.textEnum				= MBT_WEAPONKEYS;
	s_controls_weapon.textcolor				= CT_BLACK;
	s_controls_weapon.textcolor2			= CT_WHITE;

	y += inc+MENU_BUTTON_MED_HEIGHT;
	s_controls_look.generic.type			= MTYPE_BITMAP;      
	s_controls_look.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_controls_look.generic.x				= x;
	s_controls_look.generic.y				= y;
	s_controls_look.generic.name			= "menu/common/square.tga";
	s_controls_look.generic.id				= ID_CONTROLSLOOK;
	s_controls_look.generic.callback		= Controls_MenuEventNew;
	s_controls_look.width					= MENU_BUTTON_MED_WIDTH - 10;
	s_controls_look.height					= MENU_BUTTON_MED_HEIGHT;
	s_controls_look.color					= CT_DKPURPLE1;
	s_controls_look.color2					= CT_LTPURPLE1;
	s_controls_look.textX					= 5;
	s_controls_look.textY					= 1;
	s_controls_look.textEnum				= MBT_LOOKKEYS;
	s_controls_look.textcolor				= CT_BLACK;
	s_controls_look.textcolor2				= CT_WHITE;

	y += inc+MENU_BUTTON_MED_HEIGHT;
	s_controls_movement.generic.type		= MTYPE_BITMAP;      
	s_controls_movement.generic.flags		= QMF_HIGHLIGHT_IF_FOCUS;
	s_controls_movement.generic.x			= x;
	s_controls_movement.generic.y			= y;
	s_controls_movement.generic.name		= "menu/common/square.tga";
	s_controls_movement.generic.id			= ID_CONTROLSMOVE;
	s_controls_movement.generic.callback	= Controls_MenuEventNew;
	s_controls_movement.width				= MENU_BUTTON_MED_WIDTH - 10;
	s_controls_movement.height				= MENU_BUTTON_MED_HEIGHT;
	s_controls_movement.color				= CT_DKPURPLE1;
	s_controls_movement.color2				= CT_LTPURPLE1;
	s_controls_movement.textX				= 5;
	s_controls_movement.textY				= 1;
	s_controls_movement.textEnum			= MBT_MOVEMENTKEYS;
	s_controls_movement.textcolor			= CT_BLACK;
	s_controls_movement.textcolor2			= CT_WHITE;

	y += inc+MENU_BUTTON_MED_HEIGHT;
	s_controls_quickkey.generic.type				= MTYPE_BITMAP;      
	s_controls_quickkey.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_controls_quickkey.generic.x				= x;
	s_controls_quickkey.generic.y				= y;
	s_controls_quickkey.generic.name				= "menu/common/square.tga";
	s_controls_quickkey.generic.id				= ID_CONTROLSQUICKKEYS;
	s_controls_quickkey.generic.callback			= Controls_MenuEventNew;
	s_controls_quickkey.width					= MENU_BUTTON_MED_WIDTH - 10;
	s_controls_quickkey.height					= MENU_BUTTON_MED_HEIGHT;
	s_controls_quickkey.color					= CT_DKPURPLE1;
	s_controls_quickkey.color2					= CT_LTPURPLE1;
	s_controls_quickkey.textX					= 5;
	s_controls_quickkey.textY					= 1;
	s_controls_quickkey.textEnum				= MBT_QUICKKEYS;
	s_controls_quickkey.textcolor				= CT_BLACK;
	s_controls_quickkey.textcolor2				= CT_WHITE;

	y += inc+MENU_BUTTON_MED_HEIGHT;
	y += inc+MENU_BUTTON_MED_HEIGHT;
	s_controls_mouse.generic.type				= MTYPE_BITMAP;      
	s_controls_mouse.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS;
	s_controls_mouse.generic.x					= x;
	s_controls_mouse.generic.y					= y;
	s_controls_mouse.generic.name				= "menu/common/square.tga";
	s_controls_mouse.generic.id					= ID_CONTROLSMOUSE;
	s_controls_mouse.generic.callback			= Controls_MenuEventNew;
	s_controls_mouse.width						= MENU_BUTTON_MED_WIDTH - 10;
	s_controls_mouse.height						= MENU_BUTTON_MED_HEIGHT;
	s_controls_mouse.color						= CT_DKPURPLE1;
	s_controls_mouse.color2						= CT_LTPURPLE1;
	s_controls_mouse.textX						= 5;
	s_controls_mouse.textY						= 1;
	s_controls_mouse.textEnum					= MBT_MOUSECONFIG;
	s_controls_mouse.textcolor					= CT_BLACK;
	s_controls_mouse.textcolor2					= CT_WHITE;

	y += inc+MENU_BUTTON_MED_HEIGHT;
	s_controls_other.generic.type				= MTYPE_BITMAP;      
	s_controls_other.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS;
	s_controls_other.generic.x					= x;
	s_controls_other.generic.y					= y;
	s_controls_other.generic.name				= "menu/common/square.tga";
	s_controls_other.generic.id					= ID_CONTROLSOTHER;
	s_controls_other.generic.callback			= Controls_MenuEventNew;
	s_controls_other.width						= MENU_BUTTON_MED_WIDTH - 10;
	s_controls_other.height						= MENU_BUTTON_MED_HEIGHT;
	s_controls_other.color						= CT_DKPURPLE1;
	s_controls_other.color2						= CT_LTPURPLE1;
	s_controls_other.textX						= 5;
	s_controls_other.textY						= 1;
	s_controls_other.textEnum					= MBT_OTHEROPTIONS;
	s_controls_other.textcolor					= CT_BLACK;
	s_controls_other.textcolor2					= CT_WHITE;
/*
	y += inc+MENU_BUTTON_MED_HEIGHT;
	s_controls_default.generic.type				= MTYPE_BITMAP;      
	s_controls_default.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_controls_default.generic.x				= x;
	s_controls_default.generic.y				= y;
	s_controls_default.generic.name				= "menu/common/square.tga";
	s_controls_default.generic.id				= ID_CONTROLSDEFAULT;
	s_controls_default.generic.callback			= Controls_MenuEventNew;
	s_controls_default.width					= MENU_BUTTON_MED_WIDTH - 10;
	s_controls_default.height					= MENU_BUTTON_MED_HEIGHT;
	s_controls_default.color					= CT_DKPURPLE1;
	s_controls_default.color2					= CT_LTPURPLE1;
	s_controls_default.textX					= 5;
	s_controls_default.textY					= 1;
	s_controls_default.textEnum					= MBT_SETDEFAULT;
	s_controls_default.textcolor				= CT_BLACK;
	s_controls_default.textcolor2				= CT_WHITE;
*/
/*	y += inc+MENU_BUTTON_MED_HEIGHT;
	s_controls_loadconfig.generic.type		= MTYPE_BITMAP;      
	s_controls_loadconfig.generic.flags		= QMF_HIGHLIGHT_IF_FOCUS;
	s_controls_loadconfig.generic.x			= x;
	s_controls_loadconfig.generic.y			= y;
	s_controls_loadconfig.generic.name		= "menu/common/square.tga";
	s_controls_loadconfig.generic.id		= ID_MAINMENU;
	s_controls_loadconfig.generic.callback	= Controls_MenuEventNew;
	s_controls_loadconfig.width				= MENU_BUTTON_MED_WIDTH - 10;
	s_controls_loadconfig.height			= MENU_BUTTON_MED_HEIGHT;
	s_controls_loadconfig.color				= CT_DKPURPLE1;
	s_controls_loadconfig.color2			= CT_LTPURPLE1;
	s_controls_loadconfig.textX				= 5;
	s_controls_loadconfig.textY				= 2;
	s_controls_loadconfig.textEnum			= MBT_LOADCONFIG;
	s_controls_loadconfig.textcolor			= CT_BLACK;
	s_controls_loadconfig.textcolor2		= CT_WHITE;

	y += inc+MENU_BUTTON_MED_HEIGHT;
	s_controls_saveconfig.generic.type		= MTYPE_BITMAP;      
	s_controls_saveconfig.generic.flags		= QMF_HIGHLIGHT_IF_FOCUS;
	s_controls_saveconfig.generic.x			= x;
	s_controls_saveconfig.generic.y			= y;
	s_controls_saveconfig.generic.name		= "menu/common/square.tga";
	s_controls_saveconfig.generic.id		= ID_MAINMENU;
	s_controls_saveconfig.generic.callback	= Controls_MenuEventNew;
	s_controls_saveconfig.width				= MENU_BUTTON_MED_WIDTH - 10;
	s_controls_saveconfig.height			= MENU_BUTTON_MED_HEIGHT;
	s_controls_saveconfig.color				= CT_DKPURPLE1;
	s_controls_saveconfig.color2			= CT_LTPURPLE1;
	s_controls_saveconfig.textX				= 5;
	s_controls_saveconfig.textY				= 2;
	s_controls_saveconfig.textEnum			= MBT_SAVECONFIG;
	s_controls_saveconfig.textcolor			= CT_BLACK;
	s_controls_saveconfig.textcolor2		= CT_WHITE;

*/

	switch (menuType)
	{
	case MENU_CONTROLS_WEAPON :
		s_controls_weapon.generic.flags			= QMF_GRAYED;
		break;
	case MENU_CONTROLS_LOOK :
		s_controls_look.generic.flags			= QMF_GRAYED;
		break;
	case MENU_CONTROLS_MOVEMENT :
		s_controls_movement.generic.flags		= QMF_GRAYED;
		break;
	case MENU_CONTROLS_QUICKKEYS :
		s_controls_quickkey.generic.flags		= QMF_GRAYED;
		break;
	case MENU_CONTROLS_MOUSE :
		s_controls_mouse.generic.flags			= QMF_GRAYED;
		break;
	case MENU_CONTROLS_OTHER :
		s_controls_other.generic.flags			= QMF_GRAYED;
		break;
//	case MENU_CONTROLS_DEFAULT :
//		s_controls_default.generic.flags		= QMF_GRAYED;
//		break;
	}

	Menu_AddItem( menu, ( void * )&s_controls_weapon);
	Menu_AddItem( menu, ( void * )&s_controls_look);
	Menu_AddItem( menu, ( void * )&s_controls_movement);
	Menu_AddItem( menu, ( void * )&s_controls_quickkey);
	Menu_AddItem( menu, ( void * )&s_controls_mouse);
	Menu_AddItem( menu, ( void * )&s_controls_other);
//	Menu_AddItem( menu, ( void * )&s_controls_default);
//	Menu_AddItem( menu, ( void * )&s_controls_loadconfig);
//	Menu_AddItem( menu, ( void * )&s_controls_saveconfig);

}
