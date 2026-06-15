#include "ui_local.h"


int s_enable_Names[] =
{
	MNT_OFF,
	MNT_ON,
	MNT_NONE
};

// Precache stuff for VideoScreens
static struct 
{
	qhandle_t	swooshTop;
	qhandle_t	swooshBottom;
	qhandle_t	swooshTopSmall;
	qhandle_t	swooshBottomSmall;
} videodata_cache;

// Precache stuff for VideoScreens
static struct 
{
	qhandle_t	gamma;
	qhandle_t	top;
	menulist_s	cinematic_quality;
	menulist_s	anisotropicfiltering;
} videodata2;

// Precache stuff for Video Driver
#define MAX_VID_DRIVERS 128
static struct 
{
	menuframework_s		menu;

	char *drivers[MAX_VID_DRIVERS];
	char extensionsString[2*MAX_STRING_CHARS];

	menutext_s		line1;
	menutext_s		line2;
	menutext_s		line3;
	menutext_s		line4;
	menutext_s		line5;
	menutext_s		line6;
	menutext_s		line7;
	menutext_s		line8;
	menutext_s		line9;
	menutext_s		line10;
	menutext_s		line11;
	menutext_s		line12;
	menutext_s		line13;
	menutext_s		line14;
	menutext_s		line15;
	menutext_s		line16;
	menutext_s		line17;
	menutext_s		line18;
	menutext_s		line19;
	menutext_s		line20;
	menutext_s		line21;
	menutext_s		line22;
	menutext_s		line23;
	menutext_s		line24;

	qhandle_t	corner_ll_8_16;
	qhandle_t	corner_ll_16_16;
	qhandle_t	arrow_dn;
	menubitmap_s	arrowdwn;
	menubitmap_s	arrowup;
	int			currentDriverLine;
	int			driverCnt;

	int			activeArrowDwn;
	int			activeArrowUp;
} s_videodriver;


static void* g_videolines[] =
{
	&s_videodriver.line1, 
	&s_videodriver.line2, 
	&s_videodriver.line3, 
	&s_videodriver.line4, 
	&s_videodriver.line5, 
	&s_videodriver.line6, 
	&s_videodriver.line7, 
	&s_videodriver.line8, 
	&s_videodriver.line9, 
	&s_videodriver.line10, 
	&s_videodriver.line11, 
	&s_videodriver.line12, 
	&s_videodriver.line13, 
	&s_videodriver.line14, 
	&s_videodriver.line15, 
	&s_videodriver.line16, 
	&s_videodriver.line17, 
	&s_videodriver.line18, 
	&s_videodriver.line19, 
	&s_videodriver.line20, 
	&s_videodriver.line21, 
	&s_videodriver.line22, 
	&s_videodriver.line23, 
	&s_videodriver.line24, 
	NULL,
};


int video_sidebuttons[3][2] = 
{
	30, 250,	// Video Data Button
	30, 250 + 6 + (MENU_BUTTON_MED_HEIGHT * 1.5),	// Video Drivers Button
	30, 250 + (2 * (6 + (MENU_BUTTON_MED_HEIGHT * 1.5))),	// Video Drivers Button
};


#define ID_MAINMENU		100
#define ID_CONTROLS		101
#define ID_VIDEO		102
#define ID_SOUND		103
#define ID_GAMEOPTIONS	104
#define ID_CDKEY		105
#define ID_VIDEODATA	110
#define ID_VIDEODATA2	111
#define ID_VIDEODRIVERS	112
#define ID_ARROWDWN		113
#define ID_ARROWUP		114


static menuframework_s		s_video2_menu;
static menuslider_s			s_video2_gamma_slider;
//static menuslider_s			s_video2_intensity_slider;
static menuslider_s			s_video2_screensize_slider;


static menuframework_s		s_video_menu;
static menulist_s			s_video_mode_option_list;
static menulist_s			s_video_driver_list;
static menulist_s			s_video_extension_enable_list;
static menulist_s			s_video_mode_list;
static menulist_s			s_video_colordepth_list;
static menulist_s			s_video_fullscreen_list;
static menulist_s			s_video_lighting_list;
static menulist_s			s_video_geometry_list;
static menulist_s			s_video_texture_quality_list;
static menulist_s			s_video_tq_bits_list;
static menulist_s			s_video_texture_mode_list;
static menulist_s			s_video_simpleshaders_list;
static menulist_s			s_video_compresstextures;
static menuaction_s			s_video_apply_action;
static menubitmap_s			s_video_drivers;
static menubitmap_s			s_video_data;
static menubitmap_s			s_video_data2;
static menuaction_s			s_video_apply_action2;

void UI_VideoDriverMenu( void );
void Video_SideButtons(menuframework_s *menu,int menuType);
void VideoDriver_Lines(int increment);


typedef enum 
{
	VDG_LEFTNUMBERS,

	VDG_SPEC_ROW1,
	VDG_SPEC_ROW2,
	VDG_SPEC_ROW3,
	VDG_SPEC_ROW4,

	VDG_COL1_NUM1,
	VDG_COL1_NUM2,
	VDG_COL1_NUM3,
	VDG_COL1_NUM4,
	VDG_COL1_NUM5,

	VDG_COL2_NUM1,
	VDG_COL2_NUM2,
	VDG_COL2_NUM3,
	VDG_COL2_NUM4,
	VDG_COL2_NUM5,

	VDG_COL3_NUM1,
	VDG_COL3_NUM2,
	VDG_COL3_NUM3,
	VDG_COL3_NUM4,
	VDG_COL3_NUM5,

	VDG_COL4_NUM1,
	VDG_COL4_NUM2,
	VDG_COL4_NUM3,
	VDG_COL4_NUM4,
	VDG_COL4_NUM5,

	VDG_COL5_NUM1,
	VDG_COL5_NUM2,
	VDG_COL5_NUM3,
	VDG_COL5_NUM4,
	VDG_COL5_NUM5,

	VDG_COL6_NUM1,
	VDG_COL6_NUM2,
	VDG_COL6_NUM3,
	VDG_COL6_NUM4,
	VDG_COL6_NUM5,

	VDG_COL7_NUM1,
	VDG_COL7_NUM2,
	VDG_COL7_NUM3,
	VDG_COL7_NUM4,
	VDG_COL7_NUM5,

	VDG_MAX
} videodata_graphics_t;

menugraphics_s videodatamenu_graphics[VDG_MAX] = 
{
//	type		timer	x		y		width	height	file/text					graphic,	min		max		target	inc		style			color
	MG_VAR,		0.0,	0,		0,		0,		0,		NULL,					0,0,	NULL,		0,		0,		0,		0,		0,				0,				NULL,	// VDG_LEFTNUMBERS

	MG_GRAPHIC,	0.0,	408,	0,		4,		4,		"menu/common/circle.tga",0,0,	NULL,		0,		0,		0,		0,		0,				CT_LTGOLD1,		NULL,	// VDG_SPEC_ROW1
	MG_GRAPHIC,	0.0,	408,	0,		4,		4,		"menu/common/circle.tga",0,0,	NULL,		0,		0,		0,		0,		0,				CT_LTGOLD1,		NULL,	// VDG_SPEC_ROW2
	MG_GRAPHIC,	0.0,	408,	0,		4,		4,		"menu/common/circle.tga",0,0,	NULL,		0,		0,		0,		0,		0,				CT_LTGOLD1,		NULL,	// VDG_SPEC_ROW3
	MG_GRAPHIC,	0.0,	408,	0,		4,		4,		"menu/common/circle.tga",0,0,	NULL,		0,		0,		0,		0,		0,				CT_LTGOLD1,		NULL,	// VDG_SPEC_ROW4

	MG_NUMBER,	0.0,	412,	55,		16,		10,		NULL,			0,0,			NULL,		0,		5,		0,		0,		UI_TINYFONT,	CT_DKGOLD1,		NULL,	// VDG_COL1_NUM1
	MG_NUMBER,	0.0,	412,	69,		16,		10,		NULL,			0,0,			NULL,		0,		5,		0,		0,		UI_TINYFONT,	CT_DKGOLD1,		NULL,	// VDG_COL1_NUM2
	MG_NUMBER,	0.0,	412,	83,		16,		10,		NULL,			0,0,			NULL,		0,		5,		0,		0,		UI_TINYFONT,	CT_DKGOLD1,		NULL,	// VDG_COL1_NUM3
	MG_NUMBER,	0.0,	412,	97,		16,		10,		NULL,			0,0,			NULL,		0,		5,		0,		0,		UI_TINYFONT,	CT_DKGOLD1,		NULL,	// VDG_COL1_NUM4
	MG_NUMBER,	0.0,	412,	111,	16,		10,		NULL,			0,0,			NULL,		0,		5,		0,		0,		UI_TINYFONT,	CT_DKGOLD1,		NULL,	// VDG_COL1_NUM5

	MG_NUMBER,	0.0,	447,	55,		16,		10,		NULL,			0,0,			NULL,		0,		5,		0,		0,		UI_TINYFONT,	CT_DKGOLD1,		NULL,	// VDG_COL2_NUM1
	MG_NUMBER,	0.0,	447,	69,		16,		10,		NULL,			0,0,			NULL,		0,		5,		0,		0,		UI_TINYFONT,	CT_DKGOLD1,		NULL,	// VDG_COL2_NUM2
	MG_NUMBER,	0.0,	447,	83,		16,		10,		NULL,			0,0,			NULL,		0,		5,		0,		0,		UI_TINYFONT,	CT_DKGOLD1,		NULL,	// VDG_COL2_NUM3
	MG_NUMBER,	0.0,	447,	97,		16,		10,		NULL,			0,0,			NULL,		0,		5,		0,		0,		UI_TINYFONT,	CT_DKGOLD1,		NULL,	// VDG_COL2_NUM4
	MG_NUMBER,	0.0,	447,	111,	16,		10,		NULL,			0,0,			NULL,		0,		5,		0,		0,		UI_TINYFONT,	CT_DKGOLD1,		NULL,	// VDG_COL2_NUM5

	MG_NUMBER,	0.0,	482,	55,		16,		10,		NULL,			0,0,			NULL,		0,		5,		0,		0,		UI_TINYFONT,	CT_DKGOLD1,		NULL,	// VDG_COL3_NUM1
	MG_NUMBER,	0.0,	482,	69,		16,		10,		NULL,			0,0,			NULL,		0,		5,		0,		0,		UI_TINYFONT,	CT_DKGOLD1,		NULL,	// VDG_COL3_NUM2
	MG_NUMBER,	0.0,	482,	83,		16,		10,		NULL,			0,0,			NULL,		0,		5,		0,		0,		UI_TINYFONT,	CT_DKGOLD1,		NULL,	// VDG_COL3_NUM3
	MG_NUMBER,	0.0,	482,	97,		16,		10,		NULL,			0,0,			NULL,		0,		5,		0,		0,		UI_TINYFONT,	CT_DKGOLD1,		NULL,	// VDG_COL3_NUM4
	MG_NUMBER,	0.0,	482,	111,	16,		10,		NULL,			0,0,			NULL,		0,		5,		0,		0,		UI_TINYFONT,	CT_DKGOLD1,		NULL,	// VDG_COL3_NUM5

	MG_NUMBER,	0.0,	517,	55,		16,		10,		NULL,			0,0,			NULL,		0,		2,		0,		0,		UI_TINYFONT,	CT_DKGOLD1,		NULL,	// VDG_COL4_NUM1
	MG_NUMBER,	0.0,	517,	69,		16,		10,		NULL,			0,0,			NULL,		0,		2,		0,		0,		UI_TINYFONT,	CT_DKGOLD1,		NULL,	// VDG_COL4_NUM2
	MG_NUMBER,	0.0,	517,	83,		16,		10,		NULL,			0,0,			NULL,		0,		2,		0,		0,		UI_TINYFONT,	CT_DKGOLD1,		NULL,	// VDG_COL4_NUM3
	MG_NUMBER,	0.0,	517,	97,		16,		10,		NULL,			0,0,			NULL,		0,		2,		0,		0,		UI_TINYFONT,	CT_DKGOLD1,		NULL,	// VDG_COL4_NUM4
	MG_NUMBER,	0.0,	517,	111,	16,		10,		NULL,			0,0,			NULL,		0,		2,		0,		0,		UI_TINYFONT,	CT_DKGOLD1,		NULL,	// VDG_COL4_NUM5

	MG_NUMBER,	0.0,	532,	55,		16,		10,		NULL,			0,0,			NULL,		0,		2,		0,		0,		UI_TINYFONT,	CT_DKGOLD1,		NULL,	// VDG_COL5_NUM1
	MG_NUMBER,	0.0,	532,	69,		16,		10,		NULL,			0,0,			NULL,		0,		2,		0,		0,		UI_TINYFONT,	CT_DKGOLD1,		NULL,	// VDG_COL5_NUM2
	MG_NUMBER,	0.0,	532,	83,		16,		10,		NULL,			0,0,			NULL,		0,		2,		0,		0,		UI_TINYFONT,	CT_DKGOLD1,		NULL,	// VDG_COL5_NUM3
	MG_NUMBER,	0.0,	532,	97,		16,		10,		NULL,			0,0,			NULL,		0,		2,		0,		0,		UI_TINYFONT,	CT_DKGOLD1,		NULL,	// VDG_COL5_NUM4
	MG_NUMBER,	0.0,	532,	111,	16,		10,		NULL,			0,0,			NULL,		0,		2,		0,		0,		UI_TINYFONT,	CT_DKGOLD1,		NULL,	// VDG_COL5_NUM5

	MG_NUMBER,	0.0,	547,	55,		16,		10,		NULL,			0,0,			NULL,		0,		2,		0,		0,		UI_TINYFONT,	CT_DKGOLD1,		NULL,	// VDG_COL6_NUM1
	MG_NUMBER,	0.0,	547,	69,		16,		10,		NULL,			0,0,			NULL,		0,		2,		0,		0,		UI_TINYFONT,	CT_DKGOLD1,		NULL,	// VDG_COL6_NUM2
	MG_NUMBER,	0.0,	547,	83,		16,		10,		NULL,			0,0,			NULL,		0,		2,		0,		0,		UI_TINYFONT,	CT_DKGOLD1,		NULL,	// VDG_COL6_NUM3
	MG_NUMBER,	0.0,	547,	97,		16,		10,		NULL,			0,0,			NULL,		0,		2,		0,		0,		UI_TINYFONT,	CT_DKGOLD1,		NULL,	// VDG_COL6_NUM4
	MG_NUMBER,	0.0,	547,	111,	16,		10,		NULL,			0,0,			NULL,		0,		2,		0,		0,		UI_TINYFONT,	CT_DKGOLD1,		NULL,	// VDG_COL6_NUM5

	MG_NUMBER,	0.0,	562,	55,		16,		10,		NULL,			0,0,			NULL,		0,		6,		0,		0,		UI_TINYFONT,	CT_DKGOLD1,		NULL,	// VDG_COL7_NUM1
	MG_NUMBER,	0.0,	562,	69,		16,		10,		NULL,			0,0,			NULL,		0,		6,		0,		0,		UI_TINYFONT,	CT_DKGOLD1,		NULL,	// VDG_COL7_NUM2
	MG_NUMBER,	0.0,	562,	83,		16,		10,		NULL,			0,0,			NULL,		0,		6,		0,		0,		UI_TINYFONT,	CT_DKGOLD1,		NULL,	// VDG_COL7_NUM3
	MG_NUMBER,	0.0,	562,	97,		16,		10,		NULL,			0,0,			NULL,		0,		6,		0,		0,		UI_TINYFONT,	CT_DKGOLD1,		NULL,	// VDG_COL7_NUM4
	MG_NUMBER,	0.0,	562,	111,	16,		10,		NULL,			0,0,			NULL,		0,		6,		0,		0,		UI_TINYFONT,	CT_DKGOLD1,		NULL,	// VDG_COL7_NUM5
};


static const char *s_drivers[] =
{
	OPENGL_DRIVER_NAME,
	_3DFX_DRIVER_NAME,
	0
};


static void ApplyChanges( void *unused, int notification );
static void ApplyChanges2( void *unused, int notification );

/*
=======================================================================

DRIVER INFORMATION MENU

=======================================================================
*/
#define ID_RETURN	100

/*
=======================================================================

VIDEO MENU

=======================================================================
*/
typedef struct
{
	int mode;
	qboolean fullscreen;
	int tq;
	int lighting;
	int colordepth;
	int texturebits;
	int geometry;
	int filter;
	int driver;
	qboolean extensions;
	int simpleshaders;
	int compresstextures;
} InitialVideoOptions_s;

static InitialVideoOptions_s s_ivo;


static InitialVideoOptions_s s_ivo_templates[] =
{
	{
		2, qtrue, 3, 0, 2, 2, 2, 1, 0, qtrue, 0, 0,	// JDC: this was tq 3
	},
	{
		1, qtrue, 2, 0, 0, 0, 2, 0, 0, qtrue, 0, 0,
	},
	{
		0, qtrue, 1, 0, 1, 0, 0, 0, 0, qtrue, 0, 0,
	},
	{
		0, qtrue, 1, 1, 1, 0, 0, 0, 0, qtrue, 1, 0,
	},
	{
		1, qtrue, 1, 0, 0, 0, 1, 0, 0, qtrue, 0, 0,
	}
};

#define NUM_IVO_TEMPLATES ( sizeof( s_ivo_templates ) / sizeof( s_ivo_templates[0] ) )


/*
===============
GetInitialVideoVars - place values in s_ivo structure
===============
*/
static void GetInitialVideoVars( void )
{
	s_ivo.colordepth = s_video_colordepth_list.curvalue;
	s_ivo.driver = s_video_driver_list.curvalue;
	s_ivo.mode = s_video_mode_list.curvalue;
	s_ivo.fullscreen = s_video_fullscreen_list.curvalue;
	s_ivo.extensions = s_video_extension_enable_list.curvalue;
	s_ivo.tq = s_video_texture_quality_list.curvalue;
//	s_ivo.lighting = s_video_lighting_list.curvalue;
	s_ivo.geometry = s_video_geometry_list.curvalue;
	s_ivo.filter = s_video_texture_mode_list.curvalue;
	s_ivo.texturebits = s_video_tq_bits_list.curvalue;
	s_ivo.simpleshaders = s_video_simpleshaders_list.curvalue;
	s_ivo.compresstextures = s_video_compresstextures.curvalue;

}


/*
===============
CheckConfigVsTemplates
===============
*/
static void CheckConfigVsTemplates( void )
{
	int i;

	for ( i = 0; i < NUM_IVO_TEMPLATES; i++ )
	{
		if ( s_ivo_templates[i].colordepth != s_video_colordepth_list.curvalue )
			continue;
		if ( s_ivo_templates[i].driver != s_video_driver_list.curvalue )
			continue;
		if ( s_ivo_templates[i].mode != s_video_mode_list.curvalue )
			continue;
		if ( s_ivo_templates[i].fullscreen != s_video_fullscreen_list.curvalue )
			continue;
		if ( s_ivo_templates[i].tq != s_video_texture_quality_list.curvalue )
			continue;
//		if ( s_ivo_templates[i].lighting != s_video_lighting_list.curvalue )
//			continue;
		if ( s_ivo_templates[i].geometry != s_video_geometry_list.curvalue )
			continue;
		if ( s_ivo_templates[i].filter != s_video_texture_mode_list.curvalue )
			continue;
		if ( s_ivo_templates[i].simpleshaders != s_video_simpleshaders_list.curvalue )
			continue;

//		if ( s_ivo_templates[i].compresstextures != s_video_compresstextures.curvalue )
//			continue;

//		if ( s_ivo_templates[i].texturebits != s_texturebits_box.curvalue )
//			continue;
		s_video_mode_option_list.curvalue = i;
		return;
	}
	s_video_mode_option_list.curvalue = 4;

}

/*
===============
UpdateMenuItemValues
===============
*/
static void UpdateMenuItemValues( void )
{

	if ( s_video_driver_list.curvalue == 1 )
	{
		s_video_fullscreen_list.curvalue = 1;
		s_video_fullscreen_list.generic.flags |= QMF_GRAYED;
		s_video_colordepth_list.curvalue = 1;
	}
	else
	{
		s_video_fullscreen_list.generic.flags &= ~QMF_GRAYED;
	}

	if ( s_video_fullscreen_list.curvalue == 0 || s_video_driver_list.curvalue == 1 )
	{
		s_video_colordepth_list.curvalue = 0;
		s_video_colordepth_list.generic.flags |= QMF_GRAYED;
	}
	else
	{
		s_video_colordepth_list.generic.flags &= ~QMF_GRAYED;
	}

	// If you change the extension enable, texture quality changes automatically
	if ( s_video_extension_enable_list.curvalue == 0 )
	{
		if ( s_video_tq_bits_list.curvalue == 0 )
		{
			s_video_tq_bits_list.curvalue = 1;
		}
	}

	s_video_apply_action.generic.flags |= QMF_GRAYED;
	s_video_apply_action.generic.flags &= ~QMF_BLINK;


	// FIXME : this could be handled much better

	// Check and see if anything has changed from the original data
	s_video_mode_list.updated = 0;
	if ( s_ivo.mode != s_video_mode_list.curvalue )
	{
		s_video_mode_list.updated = 1;
		s_video_apply_action.generic.flags &= ~QMF_GRAYED;
		s_video_apply_action.generic.flags |= QMF_BLINK;
	}

	s_video_fullscreen_list.updated = 0;
	if ( s_ivo.fullscreen != s_video_fullscreen_list.curvalue )
	{
		s_video_fullscreen_list.updated = 1;
		s_video_apply_action.generic.flags &= ~QMF_GRAYED;
		s_video_apply_action.generic.flags |= QMF_BLINK;
	}

	s_video_extension_enable_list.updated = 0;
	if ( s_ivo.extensions != s_video_extension_enable_list.curvalue )
	{
		s_video_extension_enable_list.updated = 1;
		s_video_apply_action.generic.flags &= ~QMF_GRAYED;
		s_video_apply_action.generic.flags |= QMF_BLINK;
	}

	s_video_simpleshaders_list.updated = 0;
	if ( s_ivo.simpleshaders != s_video_simpleshaders_list.curvalue )
	{
		s_video_simpleshaders_list.updated = 1;
		s_video_apply_action.generic.flags &= ~QMF_GRAYED;
		s_video_apply_action.generic.flags |= QMF_BLINK;
	}

	s_video_compresstextures.updated = 0;
	if ( s_ivo.compresstextures != s_video_compresstextures.curvalue )
	{
		s_video_compresstextures.updated = 1;
		s_video_apply_action.generic.flags &= ~QMF_GRAYED;
		s_video_apply_action.generic.flags |= QMF_BLINK;
	}

	s_video_texture_quality_list.updated = 0;
	if ( s_ivo.tq != s_video_texture_quality_list.curvalue )
	{
		s_video_texture_quality_list.updated = 1;
		s_video_apply_action.generic.flags &= ~QMF_GRAYED;
		s_video_apply_action.generic.flags |= QMF_BLINK;
	}

/*	s_video_lighting_list.updated = 0;
	if ( s_ivo.lighting != s_video_lighting_list.curvalue )
	{
		s_video_lighting_list.updated = 1;
		s_video_apply_action.generic.flags &= ~QMF_GRAYED;
		s_video_apply_action.generic.flags |= QMF_BLINK;
	}
*/
	s_video_colordepth_list.updated = 0;
	if ( s_ivo.colordepth != s_video_colordepth_list.curvalue )
	{
		s_video_colordepth_list.updated = 1;
		s_video_apply_action.generic.flags &= ~QMF_GRAYED;
		s_video_apply_action.generic.flags |= QMF_BLINK;
	}

	s_video_driver_list.updated = 0;
	if ( s_ivo.driver != s_video_driver_list.curvalue )
	{
		s_video_driver_list.updated = 1;
		s_video_apply_action.generic.flags &= ~QMF_GRAYED;
		s_video_apply_action.generic.flags |= QMF_BLINK;
	}

	s_video_tq_bits_list.updated = 0;
	if ( s_ivo.texturebits != s_video_tq_bits_list.curvalue )
	{
		s_video_tq_bits_list.updated = 1;
		s_video_apply_action.generic.flags &= ~QMF_GRAYED;
		s_video_apply_action.generic.flags |= QMF_BLINK;
	}

	s_video_geometry_list.updated =0;
	if ( s_ivo.geometry != s_video_geometry_list.curvalue )
	{
		s_video_geometry_list.updated = 1;
		s_video_apply_action.generic.flags &= ~QMF_GRAYED;
		s_video_apply_action.generic.flags |= QMF_BLINK;
	}

	s_video_texture_mode_list.updated=0;
	if ( s_ivo.filter != s_video_texture_mode_list.curvalue )
	{
		s_video_texture_mode_list.updated =1;
		s_video_apply_action.generic.flags &= ~QMF_GRAYED;
		s_video_apply_action.generic.flags |= QMF_BLINK;
	}

	CheckConfigVsTemplates();
}	


/*
===============
GetVideoMenuItemValues
===============
*/
static void GetVideoMenuItemValues( void )
{
	// Aspect toggle: cg_forceAspect=1 means 4:3 pillarbox (curvalue 0); else 16:9/native (curvalue 1).
	s_video_mode_list.curvalue = ui.Cvar_VariableValue( "cg_forceAspect" ) ? 0 : 1;

	s_video_fullscreen_list.curvalue = 1;	// Android: always fullscreen (toggle removed from menu)
	s_video_extension_enable_list.curvalue = ui.Cvar_VariableValue("r_allowExtensions");
	s_video_simpleshaders_list.curvalue = ui.Cvar_VariableValue("r_lowEndVideo");
	s_video_compresstextures.curvalue = ui.Cvar_VariableValue("r_ext_compress_textures");

	s_video_texture_quality_list.curvalue = 3-ui.Cvar_VariableValue( "r_picmip");
	if ( s_video_texture_quality_list.curvalue < 0 )
	{
		s_video_texture_quality_list.curvalue = 0;
	}
	else if ( s_video_texture_quality_list.curvalue > 3 )
	{
		s_video_texture_quality_list.curvalue = 3;
	}

//	s_video_lighting_list.curvalue = ui.Cvar_VariableValue( "r_vertexLight" ) != 0;
	switch ( ( int ) ui.Cvar_VariableValue( "r_texturebits" ) )
	{
	default:
	case 0:
		s_video_tq_bits_list.curvalue = 0;
		break;
	case 16:
		s_video_tq_bits_list.curvalue = 1;
		break;
	case 32:
		s_video_tq_bits_list.curvalue = 2;
		break;
	}

	if ( !Q_stricmp( UI_Cvar_VariableString( "r_textureMode" ), "GL_LINEAR_MIPMAP_NEAREST" ) )
	{
		s_video_texture_mode_list.curvalue = 0;
	}
	else
	{
		s_video_texture_mode_list.curvalue = 1;
	}

	if ( ui.Cvar_VariableValue( "r_lodBias" ) > 0 )
	{
		if ( ui.Cvar_VariableValue( "r_subdivisions" ) >= 20 )
		{
			s_video_geometry_list.curvalue = 0;			// Setting LOW
		}
		else
		{
			s_video_geometry_list.curvalue = 1;			// Setting MED
		}
	}
	else 
	{
		s_video_geometry_list.curvalue = 2;			// Setting HIGH
	}

	switch ( ( int ) ui.Cvar_VariableValue( "r_colorbits" ) )
	{
	default:
	case 0:
		s_video_colordepth_list.curvalue = 0;
		break;
	case 16:
		s_video_colordepth_list.curvalue = 1;
		break;
	case 32:
		s_video_colordepth_list.curvalue = 2;
		break;
	}

	if ( s_video_fullscreen_list.curvalue == 0 )
	{
		s_video_colordepth_list.curvalue = 0;
	}
	if ( s_video_driver_list.curvalue == 1 )
	{
		s_video_colordepth_list.curvalue = 1;
	}
}

/*
===============
FullscreenCallback
===============
*/
static void FullscreenCallback( void *s, int notification )
{
	if (notification != QM_ACTIVATED)
		return;
}

/*
===============
ModeCallback
===============
*/
static void ModeCallback( void *s, int notification )
{
	if (notification != QM_ACTIVATED)
		return;

	// clamp 3dfx video modes
	if ( s_video_driver_list.curvalue == 1 )
	{
		if ( s_video_mode_list.curvalue < 2 )
		{
			s_video_mode_list.curvalue = 2;
		}
		else if ( s_video_mode_list.curvalue > 6 )
		{
			s_video_mode_list.curvalue = 6;
		}
	}

	// EFSP: apply the 4:3/16:9 aspect toggle ON THE FLY (user wants it live — no Apply button, no
	// vid_restart). s_video_mode_list is the Aspect spincontrol: curvalue 0 = "4:3", 1 = "16:9". Setting
	// cg_forceAspect here takes effect on the very next rendered frame because the cgame re-reads it every
	// frame (CG_UpdateCvars at cg_view.cpp -> CG_CalcVrect pillarboxes the 3D refdef to 4:3). Previously
	// cg_forceAspect was only written by the Apply button (ApplyChanges), so just changing the spinner did
	// nothing visible -> "4:3 doesn't work". Cvar_SetValue is wired to the real engine Cvar (sp_ui_bridge).
	ui.Cvar_SetValue( "cg_forceAspect", s_video_mode_list.curvalue ? 0 : 1 );

	// EFSP 4:3: re-pillarbox the 2D MENU live too (not just the 3D/HUD). UI_RecalcAspect re-reads
	// cg_forceAspect and updates uis.scalex/bias so the main + pause menus switch between centered-4:3
	// (curvalue 0) and full-width 16:9 fill (curvalue 1 = default) on the very next menu frame.
	UI_RecalcAspect();
}

/*
===============
GraphicsOptionsCallback
===============
*/
static void GraphicsOptionsCallback( void *s, int notification )
{
	InitialVideoOptions_s *ivo;

	if (notification != QM_ACTIVATED)
		return;

	ivo = &s_ivo_templates[s_video_mode_option_list.curvalue];

	s_video_colordepth_list.curvalue = ivo->colordepth;
	s_video_driver_list.curvalue = ivo->driver;
//	s_video_mode_list.curvalue = ivo->mode;	// aspect toggle is independent of quality presets
//	s_video_fullscreen_list.curvalue = ivo->fullscreen;	// fullscreen always on (toggle removed)
	s_video_texture_quality_list.curvalue = ivo->tq;
//	s_video_lighting_list.curvalue = ivo->lighting;
	s_video_geometry_list.curvalue = ivo->geometry;
	s_video_texture_mode_list.curvalue = ivo->filter;
	s_video_simpleshaders_list.curvalue = ivo->simpleshaders;
	s_video_compresstextures.curvalue = ivo->compresstextures;

	s_video_tq_bits_list.curvalue = ivo->texturebits;

}

/*
===============
TextureDetailCallback
===============
*/
static void TextureDetailCallback( void *s, int notification )
{
}

/*
===============
TextureQualityCallback
===============
*/
static void TextureQualityCallback( void *s, int notification )
{
}

/*
===============
ExtensionsCallback
===============
*/
static void ExtensionsCallback( void *s, int notification )
{
}

/*
===============
ColorDepthCallback
===============
*/
static void ColorDepthCallback( void *s, int notification )
{
}

/*
===============
LightingCallback
===============
*/
static void LightingCallback( void * s, int notification )
{
}

/*
=================
ApplyChanges - Apply the changes from the video data screen
=================
*/
static void ApplyChanges2( void *unused, int notification )
{
	if (notification != QM_ACTIVATED)
		return;

	ui.Cmd_ExecuteText( EXEC_APPEND, "vid_restart\n" );
}

/*
=================
ApplyChanges - Apply the changes from the video data screen
=================
*/
static void ApplyChanges( void *unused, int notification )
{
	if (notification != QM_ACTIVATED)
		return;


	// Texture Quality
	switch ( s_video_tq_bits_list.curvalue  )
	{
	case 0:
		ui.Cvar_SetValue( "r_texturebits", 0 );
		break;
	case 1:
		ui.Cvar_SetValue( "r_texturebits", 16 );
		break;
	case 2:
		ui.Cvar_SetValue( "r_texturebits", 32 );
		break;
	}

	// Texture Quality
	ui.Cvar_SetValue( "r_picmip", 3 - s_video_texture_quality_list.curvalue );
	
	// Allow Extensions
	ui.Cvar_SetValue( "r_allowExtensions", s_video_extension_enable_list.curvalue );
	
	ui.Cvar_SetValue( "r_lowEndVideo", s_video_simpleshaders_list.curvalue );

	ui.Cvar_SetValue( "r_ext_compress_textures", s_video_compresstextures.curvalue );

	// Adding 2 because we don't show 320x200 and MNT_400X300
	// Video Resolution Setting
	ui.Cvar_SetValue( "r_mode", s_video_mode_list.curvalue ? 11 : 6 );	// 16:9 -> 856x480 (mode 11), 4:3 -> 1024x768 (mode 6)
	ui.Cvar_SetValue( "cg_forceAspect", s_video_mode_list.curvalue ? 0 : 1 );	// 4:3 (curvalue 0) -> pillarbox the 3D view to 4:3; 16:9 -> native

	// Fullscreen Setting
	ui.Cvar_SetValue( "r_fullscreen", s_video_fullscreen_list.curvalue );

	// Set Video Driver
	ui.Cvar_Set( "r_glDriver", ( char * ) s_drivers[s_video_driver_list.curvalue] );

	// Color Depth
	switch ( s_video_colordepth_list.curvalue )
	{
	case 0:
		ui.Cvar_SetValue( "r_colorbits", 0 );
		ui.Cvar_SetValue( "r_depthbits", 0 );
		ui.Cvar_SetValue( "r_stencilbits", 0 );
		break;
	case 1:
		ui.Cvar_SetValue( "r_colorbits", 16 );
		ui.Cvar_SetValue( "r_depthbits", 16 );
		ui.Cvar_SetValue( "r_stencilbits", 0 );
		break;
	case 2:
		ui.Cvar_SetValue( "r_colorbits", 32 );
		ui.Cvar_SetValue( "r_depthbits", 24 );
		break;
	}
	
//	ui.Cvar_SetValue( "r_vertexLight", s_video_lighting_list.curvalue );

	// Geometric Detail
	if ( s_video_geometry_list.curvalue == 2)		//	Setting is HIGH
	{
		ui.Cvar_SetValue( "r_lodBias", 0 );
		ui.Cvar_SetValue( "r_subdivisions", 4 );
	}
	else if ( s_video_geometry_list.curvalue == 1)	//	Setting is MED
	{
		ui.Cvar_SetValue( "r_lodBias", 1 );
		ui.Cvar_SetValue( "r_subdivisions", 12 );
	}
	else											//	Setting is LOW
	{
		ui.Cvar_SetValue( "r_lodBias", 1 );
		ui.Cvar_SetValue( "r_subdivisions", 20 );
	}

	if ( s_video_texture_mode_list.curvalue )
	{
		ui.Cvar_Set( "r_textureMode", "GL_LINEAR_MIPMAP_LINEAR" );
	}
	else
	{
		ui.Cvar_Set( "r_textureMode", "GL_LINEAR_MIPMAP_NEAREST" );
	}

	ui.Cmd_ExecuteText( EXEC_APPEND, "vid_restart\n" );
}

/*
=================
VideoDataMenu_Blinkies
=================
*/
void VideoDataMenu_Blinkies(void)
{
	int i;

	// Generate new numbers for top right
	if ((videodatamenu_graphics[VDG_LEFTNUMBERS].timer < uis.realtime) && (videodatamenu_graphics[VDG_LEFTNUMBERS].target==6))
	{

		for (i=0;i<5;++i)
		{
			// Get random number
			videodatamenu_graphics[VDG_COL1_NUM1 + i].target = UI_RandomNumbers(videodatamenu_graphics[VDG_COL1_NUM1 + i].max);
			videodatamenu_graphics[VDG_COL2_NUM1 + i].target = UI_RandomNumbers(videodatamenu_graphics[VDG_COL2_NUM1 + i].max);
			videodatamenu_graphics[VDG_COL3_NUM1 + i].target = UI_RandomNumbers(videodatamenu_graphics[VDG_COL3_NUM1 + i].max);
			videodatamenu_graphics[VDG_COL4_NUM1 + i].target = UI_RandomNumbers(videodatamenu_graphics[VDG_COL4_NUM1 + i].max);
			videodatamenu_graphics[VDG_COL5_NUM1 + i].target = UI_RandomNumbers(videodatamenu_graphics[VDG_COL5_NUM1 + i].max);
			videodatamenu_graphics[VDG_COL6_NUM1 + i].target = UI_RandomNumbers(videodatamenu_graphics[VDG_COL6_NUM1 + i].max);
			videodatamenu_graphics[VDG_COL7_NUM1 + i].target = UI_RandomNumbers(videodatamenu_graphics[VDG_COL7_NUM1 + i].max);
		}

		videodatamenu_graphics[VDG_LEFTNUMBERS].timer = uis.realtime + 50;
		videodatamenu_graphics[VDG_LEFTNUMBERS].target=1;


		i = videodatamenu_graphics[VDG_SPEC_ROW1].target;
		videodatamenu_graphics[i + VDG_COL1_NUM1].color = CT_DKGOLD1;

		i = videodatamenu_graphics[VDG_SPEC_ROW2].target;
		videodatamenu_graphics[i + VDG_COL3_NUM1].color = CT_DKGOLD1;

		i = videodatamenu_graphics[VDG_SPEC_ROW3].target;
		videodatamenu_graphics[i + VDG_COL5_NUM1].color = CT_DKGOLD1;

		i = videodatamenu_graphics[VDG_SPEC_ROW4].target;
		videodatamenu_graphics[i + VDG_COL7_NUM1].color = CT_DKGOLD1;


		videodatamenu_graphics[VDG_SPEC_ROW1].target =  random() * 6;
		i = videodatamenu_graphics[VDG_SPEC_ROW1].target;
		videodatamenu_graphics[i + VDG_COL1_NUM1].color = CT_LTGOLD1;
		videodatamenu_graphics[VDG_SPEC_ROW1].y = videodatamenu_graphics[i + VDG_COL1_NUM1].y + 6;

		videodatamenu_graphics[VDG_SPEC_ROW2].target =  random() * 6;
		i = videodatamenu_graphics[VDG_SPEC_ROW2].target;
		videodatamenu_graphics[i + VDG_COL3_NUM1].color = CT_LTGOLD1;
		videodatamenu_graphics[VDG_SPEC_ROW2].y = videodatamenu_graphics[i + VDG_COL3_NUM1].y + 6;

		videodatamenu_graphics[VDG_SPEC_ROW3].target =  random() * 6;
		i = videodatamenu_graphics[VDG_SPEC_ROW3].target;
		videodatamenu_graphics[i + VDG_COL5_NUM1].color = CT_LTGOLD1;
		videodatamenu_graphics[VDG_SPEC_ROW3].y = videodatamenu_graphics[i + VDG_COL5_NUM1].y + 6;

		videodatamenu_graphics[VDG_SPEC_ROW4].target =  random() * 6;
		i = videodatamenu_graphics[VDG_SPEC_ROW4].target;
		videodatamenu_graphics[i + VDG_COL7_NUM1].color = CT_LTGOLD1;
		videodatamenu_graphics[VDG_SPEC_ROW4].y = videodatamenu_graphics[i + VDG_COL7_NUM1].y + 6;


		// Turn off all but the first row
		for (i=0;i<4;++i)
		{
			videodatamenu_graphics[i + VDG_COL1_NUM2].type = MG_OFF;
			videodatamenu_graphics[i + VDG_COL2_NUM2].type = MG_OFF;
			videodatamenu_graphics[i + VDG_COL3_NUM2].type = MG_OFF;
			videodatamenu_graphics[i + VDG_COL4_NUM2].type = MG_OFF;
			videodatamenu_graphics[i + VDG_COL5_NUM2].type = MG_OFF;
			videodatamenu_graphics[i + VDG_COL6_NUM2].type = MG_OFF;
			videodatamenu_graphics[i + VDG_COL7_NUM2].type = MG_OFF;

		}
	}
	// Activate the next row of numbers.  When at the last row wait and then generate new numbers
	else if ((videodatamenu_graphics[VDG_LEFTNUMBERS].timer < uis.realtime) && (videodatamenu_graphics[VDG_LEFTNUMBERS].target<6))
	{
		ui.S_StartLocalSound( uis.menu_datadisp1_snd, CHAN_MENU1 );
		videodatamenu_graphics[VDG_LEFTNUMBERS].timer = uis.realtime + 50;

		i = videodatamenu_graphics[VDG_LEFTNUMBERS].target;
		videodatamenu_graphics[i + VDG_COL1_NUM1].type = MG_NUMBER;
		videodatamenu_graphics[i + VDG_COL2_NUM1].type = MG_NUMBER;
		videodatamenu_graphics[i + VDG_COL3_NUM1].type = MG_NUMBER;
		videodatamenu_graphics[i + VDG_COL4_NUM1].type = MG_NUMBER;
		videodatamenu_graphics[i + VDG_COL5_NUM1].type = MG_NUMBER;
		videodatamenu_graphics[i + VDG_COL6_NUM1].type = MG_NUMBER;
		videodatamenu_graphics[i + VDG_COL7_NUM1].type = MG_NUMBER;

		++videodatamenu_graphics[VDG_LEFTNUMBERS].target;

		if (videodatamenu_graphics[VDG_LEFTNUMBERS].target == 6)
		{
			videodatamenu_graphics[VDG_LEFTNUMBERS].timer = uis.realtime + 6000;
		}
	}


}

/*
=================
M_VideoDataMenu_Graphics
=================
*/
void M_VideoDataMenu_Graphics (void)
{
	UI_MenuFrame(&s_video_menu);

	UI_DrawProportionalString(  74,  66, "207",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  84, "44909",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  188, "357",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  206, "250624",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  395, "456730-1",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);

	UI_Setup_MenuButtons();

	// Rest of Bottom1_Graphics
//	ui.R_SetColor( colorTable[CT_LTORANGE]);
//	UI_DrawHandlePic(  30, 203, 47, 69, uis.whiteShader);	// Top Left column above two buttons

//	UI_DrawHandlePic(  30, 344, 47, 45, uis.whiteShader);	// Top Left column below two buttons

	// Brackets around Video Data
	ui.R_SetColor( colorTable[CT_LTPURPLE1]);
	UI_DrawHandlePic(158,163, 16, 16, uis.graphicBracket1CornerLU);
	UI_DrawHandlePic(158,179,  8, 233, uis.whiteShader);
	UI_DrawHandlePic(158,412, 16, -16, uis.graphicBracket1CornerLU);	//LD

	UI_DrawHandlePic(174,163, 320, 8, uis.whiteShader);	// Top line

	UI_DrawHandlePic(494,163, 128, 128, videodata_cache.swooshTop);			// Top swoosh

	UI_DrawHandlePic(501,188, 110, 54, uis.whiteShader);	// Top right column

	UI_DrawHandlePic(501,348, 110, 55, uis.whiteShader);	// Bottom right column

	UI_DrawHandlePic(494,406, 128, 128, videodata_cache.swooshBottom);		// Bottom swoosh

	UI_DrawHandlePic(174,420, 320, 8, uis.whiteShader);	// Bottom line

//	VideoDataMenu_Blinkies();

//	UI_PrintMenuGraphics(videodatamenu_graphics,VDG_MAX);

}


/*
=================
VideoData_MenuDraw
=================
*/
static void VideoData_MenuDraw (void)
{
	UpdateMenuItemValues();

	M_VideoDataMenu_Graphics();

	Menu_Draw( &s_video_menu );
}

/*
=================
UI_VideoDataMenu_Cache
=================
*/
void UI_VideoDataMenu_Cache(void)
{
	videodata_cache.swooshTop = ui.R_RegisterShaderNoMip("menu/common/swoosh_top.tga");
	videodata_cache.swooshBottom= ui.R_RegisterShaderNoMip("menu/common/swoosh_bottom.tga");
	videodata_cache.swooshTopSmall= ui.R_RegisterShaderNoMip("menu/common/swoosh_topsmall.tga");
	videodata_cache.swooshBottomSmall= ui.R_RegisterShaderNoMip("menu/common/swoosh_bottomsmall.tga");

	// Precache all menu graphics in array
	UI_PrecacheMenuGraphics(videodatamenu_graphics,VDG_MAX);
}

void VideoDataAction( qboolean result ) 
{
	if ( result )	// Yes - do it
	{
		UI_PopMenu();
	}
}

/*
=================
VideoData_MenuKey
=================
*/
sfxHandle_t VideoData_MenuKey (int key)
{
	if (s_video_apply_action.generic.flags & QMF_BLINK)
	{
		switch (key)
		{
			case K_ESCAPE:
				// EFSP: auto-apply pending video changes on exit instead of the "lose settings?" confirm
				// (user request — don't ask). ApplyChanges commits the video cvars; then leave the menu.
				ApplyChanges( NULL, QM_ACTIVATED );
				UI_PopMenu();
				return(menu_null_sound);
				break;
		}
	}

	return ( Menu_DefaultKey( &s_video_menu, key ) );
}

/*
=================
Video_MenuInit
=================
*/
static void VideoData_MenuInit( void )
{
	int x,y,width,inc;

	UI_VideoDataMenu_Cache();

	// Menu Data
	s_video_menu.nitems						= 0;
	s_video_menu.wrapAround					= qtrue;
	s_video_menu.opening					= NULL;
	s_video_menu.closing					= NULL;
	s_video_menu.draw						= VideoData_MenuDraw;
	s_video_menu.key						= VideoData_MenuKey;
	s_video_menu.fullscreen					= qtrue;
	s_video_menu.descX						= MENU_DESC_X;
	s_video_menu.descY						= MENU_DESC_Y;
	s_video_menu.listX						= 230;
	s_video_menu.listY						= 188;
	s_video_menu.titleX						= MENU_TITLE_X;
	s_video_menu.titleY						= MENU_TITLE_Y;
	s_video_menu.titleI						= MNT_CONTROLSMENU_TITLE;
	s_video_menu.footNoteEnum				= MNT_VIDEOSETUP;

	static int s_graphics_options_Names[] =
	{
		MNT_VIDEO_HIGH_QUALITY,
		MNT_VIDEO_NORMAL,
		MNT_VIDEO_FAST,
		MNT_VIDEO_FASTEST,
		MNT_VIDEO_CUSTOM,
		MNT_NONE
	};

	static int s_driver_Names[] =
	{
		MNT_VIDEO_DRIVER_DEFAULT,
		MNT_VIDEO_DRIVER_VOODOO,
		MNT_NONE
	};

	static int s_resolutions[] = 
	{
//		MNT_320X200,
//		MNT_400X300,
		MNT_512X384,
		MNT_640X480,
		MNT_800X600,
		MNT_960X720,
		MNT_1024X768,
		MNT_1152X864,
		MNT_1280X960,
		MNT_1600X1200,
		MNT_2048X1536,
		MNT_856x480WIDE,
		MNT_NONE
	};

	static int s_aspect_names[] =
	{
		MNT_ASPECT_4_3,
		MNT_ASPECT_16_9,
		MNT_NONE
	};

	static int s_colordepth_Names[] =
	{
		MNT_DEFAULT,
		MNT_16BIT,
		MNT_32BIT,
		MNT_NONE
	};

	static int s_lighting_Names[] =
	{
		MNT_LIGHTMAP,
		MNT_VERTEX,
		MNT_NONE
	};

	static int s_quality_Names[] =
	{
		MNT_LOW,
		MNT_MEDIUM,
		MNT_HIGH,
		MNT_NONE
	};

	static int s_4quality_Names[] =
	{
		MNT_LOW,
		MNT_MEDIUM,
		MNT_HIGH,
		MNT_VERY_HIGH,
		MNT_NONE
	};

	static int s_tqbits_Names[] =
	{
		MNT_DEFAULT,
		MNT_16BIT,
		MNT_32BIT,
		MNT_NONE
	};

	static int s_filter_Names[] =
	{
		MNT_BILINEAR,
		MNT_TRILINEAR,
		MNT_NONE
	};

	x = 170;
	y = 178;
	width = 145;
	s_video_mode_option_list.generic.type			= MTYPE_SPINCONTROL;
	s_video_mode_option_list.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_video_mode_option_list.generic.x				= x;
	s_video_mode_option_list.generic.y				= y;
	s_video_mode_option_list.generic.callback		= GraphicsOptionsCallback;
	s_video_mode_option_list.textEnum				= MBT_VIDEOOPTIONS;
	s_video_mode_option_list.textcolor				= CT_BLACK;
	s_video_mode_option_list.textcolor2				= CT_WHITE;
	s_video_mode_option_list.color					= CT_DKPURPLE1;
	s_video_mode_option_list.color2					= CT_LTPURPLE1;
	s_video_mode_option_list.textX					= 5;
	s_video_mode_option_list.textY					= 2;
	s_video_mode_option_list.listnames				= s_graphics_options_Names;
	s_video_mode_option_list.width					= width;

	inc = 20;
	// GL Driver / GL Extensions options removed from the video menu (GL-era cvars, inert on the
	// Vulkan renderervk backend). Pin their menu state to OpenGL + extensions-on so the dependent
	// fullscreen / color-depth / graphics-template logic below keeps a sane default.
	s_video_driver_list.curvalue           = 0;	// 0 = OpenGL (non-Voodoo)
	s_video_extension_enable_list.curvalue = 1;	// extensions on



	y += inc;
	// references/modifies "r_mode"
	s_video_mode_list.generic.type					= MTYPE_SPINCONTROL;
	s_video_mode_list.generic.flags					= QMF_HIGHLIGHT_IF_FOCUS;
	s_video_mode_list.generic.x						= x;
	s_video_mode_list.generic.y						= y;
	s_video_mode_list.generic.callback				= ModeCallback;
	s_video_mode_list.textEnum						= MBT_VIDEOMODE;
	s_video_mode_list.textcolor						= CT_BLACK;
	s_video_mode_list.textcolor2					= CT_WHITE;
	s_video_mode_list.color							= CT_DKPURPLE1;
	s_video_mode_list.color2						= CT_LTPURPLE1;
	s_video_mode_list.textX							= 5;
	s_video_mode_list.textY							= 2;
	s_video_mode_list.listnames						= s_aspect_names;
	s_video_mode_list.width							= width;

	y += inc;
	s_video_colordepth_list.generic.type			= MTYPE_SPINCONTROL;
	s_video_colordepth_list.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_video_colordepth_list.generic.x				= x;
	s_video_colordepth_list.generic.y				= y;
	s_video_colordepth_list.generic.callback		= ColorDepthCallback;
	s_video_colordepth_list.textEnum				= MBT_VIDEOCOLORDEPTH;
	s_video_colordepth_list.textcolor				= CT_BLACK;
	s_video_colordepth_list.textcolor2				= CT_WHITE;
	s_video_colordepth_list.color					= CT_DKPURPLE1;
	s_video_colordepth_list.color2					= CT_LTPURPLE1;
	s_video_colordepth_list.textX					= 5;
	s_video_colordepth_list.textY					= 2;
	s_video_colordepth_list.listnames				= s_colordepth_Names;
	s_video_colordepth_list.width					= width;


/*	y += inc;
	s_video_lighting_list.generic.type				= MTYPE_SPINCONTROL;
	s_video_lighting_list.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS;
	s_video_lighting_list.generic.x					= x;
	s_video_lighting_list.generic.y					= y;
	s_video_lighting_list.generic.callback			= LightingCallback;
	s_video_lighting_list.textEnum					= MBT_VIDEOLIGHTING;
	s_video_lighting_list.textcolor					= CT_BLACK;
	s_video_lighting_list.textcolor2				= CT_WHITE;
	s_video_lighting_list.color						= CT_DKPURPLE1;
	s_video_lighting_list.color2					= CT_LTPURPLE1;
	s_video_lighting_list.textX						= 5;
	s_video_lighting_list.textY						= 2;
	s_video_lighting_list.listnames					= s_lighting_Names;
	s_video_lighting_list.width						= width;
*/
	y += inc;
	// references/modifies "r_lodBias" & "subdivisions"
	s_video_geometry_list.generic.type				= MTYPE_SPINCONTROL;
	s_video_geometry_list.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS;
	s_video_geometry_list.generic.x					= x;
	s_video_geometry_list.generic.y					= y;
	s_video_geometry_list.textEnum					= MBT_VIDEOGEOMETRY;
	s_video_geometry_list.textcolor					= CT_BLACK;
	s_video_geometry_list.textcolor2				= CT_WHITE;
	s_video_geometry_list.color						= CT_DKPURPLE1;
	s_video_geometry_list.color2					= CT_LTPURPLE1;
	s_video_geometry_list.textX						= 5;
	s_video_geometry_list.textY						= 2;
	s_video_geometry_list.listnames					= s_quality_Names;
	s_video_geometry_list.width						= width;


	y += inc;
	s_video_texture_quality_list.generic.type		= MTYPE_SPINCONTROL;
	s_video_texture_quality_list.generic.flags		= QMF_HIGHLIGHT_IF_FOCUS;
	s_video_texture_quality_list.generic.x			= x;
	s_video_texture_quality_list.generic.y			= y;
	s_video_texture_quality_list.generic.callback	= TextureDetailCallback;
	s_video_texture_quality_list.textEnum			= MBT_VIDEOTEXTUREDETAIL;
	s_video_texture_quality_list.textcolor			= CT_BLACK;
	s_video_texture_quality_list.textcolor2			= CT_WHITE;
	s_video_texture_quality_list.color				= CT_DKPURPLE1;
	s_video_texture_quality_list.color2				= CT_LTPURPLE1;
	s_video_texture_quality_list.textX				= 5;
	s_video_texture_quality_list.textY				= 2;
	s_video_texture_quality_list.listnames			= s_4quality_Names;
	s_video_texture_quality_list.width				= width;

	y += inc;
	// references/modifies "r_textureBits"
	s_video_tq_bits_list.generic.type				= MTYPE_SPINCONTROL;
	s_video_tq_bits_list.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS;
	s_video_tq_bits_list.generic.x					= x;
	s_video_tq_bits_list.generic.y					= y;
	s_video_tq_bits_list.generic.callback			= TextureQualityCallback;
	s_video_tq_bits_list.textEnum					= MBT_VIDEOTEXTUREBITS;
	s_video_tq_bits_list.textcolor					= CT_BLACK;
	s_video_tq_bits_list.textcolor2					= CT_WHITE;
	s_video_tq_bits_list.color						= CT_DKPURPLE1;
	s_video_tq_bits_list.color2						= CT_LTPURPLE1;
	s_video_tq_bits_list.textX						= 5;
	s_video_tq_bits_list.textY						= 2;
	s_video_tq_bits_list.listnames					= s_tqbits_Names;
	s_video_tq_bits_list.width						= width;

	y += inc;
	// references/modifies "r_textureMode"
	s_video_texture_mode_list.generic.type			= MTYPE_SPINCONTROL;
	s_video_texture_mode_list.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_video_texture_mode_list.generic.x				= x;
	s_video_texture_mode_list.generic.y				= y;
	s_video_texture_mode_list.textEnum				= MBT_VIDEOTEXTUREFILTER;
	s_video_texture_mode_list.textcolor				= CT_BLACK;
	s_video_texture_mode_list.textcolor2			= CT_WHITE;
	s_video_texture_mode_list.color					= CT_DKPURPLE1;
	s_video_texture_mode_list.color2				= CT_LTPURPLE1;
	s_video_texture_mode_list.textX					= 5;
	s_video_texture_mode_list.textY					= 2;
	s_video_texture_mode_list.listnames				= s_filter_Names;
	s_video_texture_mode_list.width					= width;

	y += inc;
	// references/modifies "r_textureMode"
	s_video_simpleshaders_list.generic.type			= MTYPE_SPINCONTROL;
	s_video_simpleshaders_list.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_video_simpleshaders_list.generic.x				= x;
	s_video_simpleshaders_list.generic.y				= y;
	s_video_simpleshaders_list.textEnum				= MBT_SIMPLESHADERS;
	s_video_simpleshaders_list.textcolor				= CT_BLACK;
	s_video_simpleshaders_list.textcolor2			= CT_WHITE;
	s_video_simpleshaders_list.color					= CT_DKPURPLE1;
	s_video_simpleshaders_list.color2				= CT_LTPURPLE1;
	s_video_simpleshaders_list.textX					= 5;
	s_video_simpleshaders_list.textY					= 2;
	s_video_simpleshaders_list.listnames				= s_enable_Names;
	s_video_simpleshaders_list.width					= width;

	y += inc;
	// references/modifies "r_ext_compress_textures"
	s_video_compresstextures.generic.type			= MTYPE_SPINCONTROL;
	s_video_compresstextures.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_video_compresstextures.generic.x				= x;
	s_video_compresstextures.generic.y				= y;
	s_video_compresstextures.textEnum				= MBT_COMPRESSEDTEXTURES;
	s_video_compresstextures.textcolor				= CT_BLACK;
	s_video_compresstextures.textcolor2				= CT_WHITE;
	s_video_compresstextures.color					= CT_DKPURPLE1;
	s_video_compresstextures.color2					= CT_LTPURPLE1;
	s_video_compresstextures.textX					= 5;
	s_video_compresstextures.textY					= 2;
	s_video_compresstextures.listnames				= s_enable_Names;
	s_video_compresstextures.width					= width;

	s_video_apply_action.generic.type				= MTYPE_ACTION;
	s_video_apply_action.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS|QMF_GRAYED;
	s_video_apply_action.generic.x					= 501;
	s_video_apply_action.generic.y					= 245;
	s_video_apply_action.generic.callback			= ApplyChanges;
	s_video_apply_action.textEnum					= MBT_VIDEOAPPLYCHANGES;
	s_video_apply_action.textcolor					= CT_BLACK;
	s_video_apply_action.textcolor2					= CT_WHITE;
	s_video_apply_action.textcolor3					= CT_LTGREY;
	s_video_apply_action.color						= CT_DKPURPLE1;
	s_video_apply_action.color2						= CT_LTPURPLE1;
	s_video_apply_action.color3						= CT_DKGREY;
	s_video_apply_action.textX						= 5;
	s_video_apply_action.textY						= 80;
	s_video_apply_action.width						= 110;
	s_video_apply_action.height						= 100;

	SetupMenu_TopButtons(&s_video_menu,MENU_VIDEODATA,&s_video_apply_action);

	// Repurposed the resolution control into a 4:3 / 16:9 aspect toggle (HM-style). The value strings
	// live in appended MNT_ enums (no sp_normaltext.dat entry) and the row label overrides MBT_VIDEOMODE;
	// assign both here in C.
	menu_normal_text[MNT_ASPECT_4_3]  = "4:3";
	menu_normal_text[MNT_ASPECT_16_9] = "16:9";
	menu_button_text[MBT_VIDEOMODE][0] = "ASPECT RATIO";
	menu_button_text[MBT_VIDEOMODE][1] = "ASPECT RATIO";

	Menu_AddItem( &s_video_menu, ( void * )&s_video_mode_option_list);
//	Menu_AddItem( &s_video_menu, ( void * )&s_video_driver_list);
//	Menu_AddItem( &s_video_menu, ( void * )&s_video_extension_enable_list);
	Menu_AddItem( &s_video_menu, ( void * )&s_video_mode_list);
	Menu_AddItem( &s_video_menu, ( void * )&s_video_colordepth_list);
//	Menu_AddItem( &s_video_menu, ( void * )&s_video_fullscreen_list);
//	Menu_AddItem( &s_video_menu, ( void * )&s_video_lighting_list);
	Menu_AddItem( &s_video_menu, ( void * )&s_video_geometry_list);
	Menu_AddItem( &s_video_menu, ( void * )&s_video_texture_quality_list);
	Menu_AddItem( &s_video_menu, ( void * )&s_video_tq_bits_list);
	Menu_AddItem( &s_video_menu, ( void * )&s_video_texture_mode_list);
	Menu_AddItem( &s_video_menu, ( void * )&s_video_simpleshaders_list);
	Menu_AddItem( &s_video_menu, ( void * )&s_video_compresstextures);
	Menu_AddItem( &s_video_menu, ( void * )&s_video_apply_action);

	Video_SideButtons(&s_video_menu,ID_VIDEODATA);

	// Force numbers to change
	videodatamenu_graphics[VDG_LEFTNUMBERS].timer = 0;	// To get numbers right away
	videodatamenu_graphics[VDG_LEFTNUMBERS].target=6;
	videodatamenu_graphics[VDG_SPEC_ROW1].target = 0;
	videodatamenu_graphics[VDG_SPEC_ROW2].target = 0;
	videodatamenu_graphics[VDG_SPEC_ROW3].target = 0;
	videodatamenu_graphics[VDG_SPEC_ROW4].target = 0;
}

/*
=================
UI_VideoDataMenu
=================
*/
void UI_VideoDataMenu( void )
{
	ui.Key_SetCatcher( KEYCATCH_UI );

//	if (!s_video_menu.initialized)
//	{
		VideoData_MenuInit();
//	}

	GetVideoMenuItemValues();	//	Get current video settings
	GetInitialVideoVars();		// Save video settings in s_vio structure

	if ((uis.glconfig.driverType == GLDRV_ICD) &&
		 (uis.glconfig.hardwareType == GLHW_3DFX_2D3D))
	{
		s_video_driver_list.generic.flags |= QMF_GRAYED;
	}

	UI_PushMenu( &s_video_menu );
}

/*
=================
GammaCallback2
=================
*/
void GammaCallback2( void *s, int notification )
{
	menuslider_s *slider = ( menuslider_s * ) s;

	if (notification != QM_ACTIVATED)
		return;

	s_video_apply_action2.generic.flags &= ~QMF_GRAYED;
	s_video_apply_action2.generic.flags |= QMF_BLINK;


	GammaCallback(s,notification );

}


/*
=================
M_Video2Menu_Graphics
=================
*/
void M_Video2Menu_Graphics (void)
{
	int y;

	UI_MenuFrame(&s_video2_menu);

	UI_DrawProportionalString(  74,  66, "925",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  84, "88PK",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  188, "8125",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  206, "358677",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  395, "3-679",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);

	UI_Setup_MenuButtons();

	y = 191;
	if (uis.glconfig.deviceSupportsGamma)
	{
		ui.R_SetColor( colorTable[CT_DKGREY]);
		UI_DrawHandlePic(  178, y, 68, 68, uis.whiteShader);	// 
		ui.R_SetColor( colorTable[CT_WHITE]);
		UI_DrawHandlePic(  180, y+2, 64, 64, videodata2.gamma);	// Starfleet graphic

		UI_DrawProportionalString( 256,  y + 5, menu_normal_text[MNT_GAMMA_LINE1],UI_SMALLFONT, colorTable[CT_LTGOLD1]);
		UI_DrawProportionalString( 256,  y + 25, menu_normal_text[MNT_GAMMA_LINE2],UI_SMALLFONT, colorTable[CT_LTGOLD1]);
		UI_DrawProportionalString( 256,  y + 45, menu_normal_text[MNT_GAMMA_LINE3],UI_SMALLFONT,colorTable[CT_LTGOLD1]);
	}
	else 
	{
		UI_DrawProportionalString( 178,  y + 5, menu_normal_text[MNT_GAMMA2_LINE1],UI_SMALLFONT, colorTable[CT_LTGOLD1]);
		UI_DrawProportionalString( 178,  y + 25,menu_normal_text[MNT_GAMMA2_LINE2],UI_SMALLFONT, colorTable[CT_LTGOLD1]);
	}

	// Brackets around Video Data
	ui.R_SetColor( colorTable[CT_LTPURPLE1]);
	UI_DrawHandlePic(158,163, 16, 16, uis.graphicBracket1CornerLU);
	UI_DrawHandlePic(158,179,  8, 233, uis.whiteShader);
	UI_DrawHandlePic(158,412, 16, -16, uis.graphicBracket1CornerLU);	//LD

	UI_DrawHandlePic(174,163, 408, 8, uis.whiteShader);	// Top line

	UI_DrawHandlePic(579,163, 32, 16, videodata2.top);	// Corner, UR
	UI_DrawHandlePic(581,179, 30, 121, uis.whiteShader);	// Top right column
	UI_DrawHandlePic(581,303, 30, 109, uis.whiteShader);	// Bottom right column
	UI_DrawHandlePic(579,412, 32, -16, videodata2.top);	// Corner, LR

	UI_DrawHandlePic(174,420, 408, 8, uis.whiteShader);	// Bottom line

}

/*
=================
Video2_MenuDraw
=================
*/
static void Video2_MenuDraw (void)
{

	M_Video2Menu_Graphics();

	Menu_Draw( &s_video2_menu );
}


/*
=================
UI_Video2Menu_Cache
=================
*/
void UI_Video2Menu_Cache(void)
{
	videodata2.gamma = ui.R_RegisterShaderNoMip("menu/special/gamma_test.tga");
	videodata2.top = ui.R_RegisterShaderNoMip("menu/common/corner_ur_8_30.tga");

	ui.R_RegisterShaderNoMip("menu/common/monbar_2.tga");
}

/*
=================
Video2_MenuInit
=================
*/
static void Video2_MenuInit( void )
{
	int x,y;

	UI_Video2Menu_Cache();

	static int s_quality_Names[] =
	{
		MNT_HIGH,
		MNT_LOW,
		MNT_NONE
	};

	// Menu Data
	s_video2_menu.nitems						= 0;
	s_video2_menu.wrapAround					= qtrue;
	s_video2_menu.opening						= NULL;
	s_video2_menu.closing						= NULL;
	s_video2_menu.draw							= Video2_MenuDraw;
	s_video2_menu.fullscreen					= qtrue;
	s_video2_menu.descX							= MENU_DESC_X;
	s_video2_menu.descY							= MENU_DESC_Y;
	s_video2_menu.listX							= 230;
	s_video2_menu.listY							= 188;
	s_video2_menu.titleX						= MENU_TITLE_X;
	s_video2_menu.titleY						= MENU_TITLE_Y;
	s_video2_menu.titleI						= MNT_CONTROLSMENU_TITLE;
	s_video2_menu.footNoteEnum					= MNT_VIDEOSETUP;

	SetupMenu_TopButtons(&s_video2_menu,MENU_VIDEO,NULL);

	Video_SideButtons(&s_video2_menu,ID_VIDEODATA2);

	x = 180;
	y = 269;
	s_video2_gamma_slider.generic.type		= MTYPE_SLIDER;
	s_video2_gamma_slider.generic.x			= x + 162;
	s_video2_gamma_slider.generic.y			= y;
	s_video2_gamma_slider.generic.flags		= QMF_SMALLFONT;
	s_video2_gamma_slider.generic.callback	= GammaCallback2;
	s_video2_gamma_slider.minvalue			= 5;
	s_video2_gamma_slider.maxvalue			= 30;
	s_video2_gamma_slider.color				= CT_DKPURPLE1;
	s_video2_gamma_slider.color2			= CT_LTPURPLE1;
	s_video2_gamma_slider.generic.name		= "menu/common/monbar_2.tga";
	s_video2_gamma_slider.width				= 256;
	s_video2_gamma_slider.height			= 32;
	s_video2_gamma_slider.focusWidth		= 145;
	s_video2_gamma_slider.focusHeight		= 18;
	s_video2_gamma_slider.picName			= GRAPHIC_SQUARE;
	s_video2_gamma_slider.picX				= x;
	s_video2_gamma_slider.picY				= y;
	s_video2_gamma_slider.picWidth			= MENU_BUTTON_MED_WIDTH + 21;
	s_video2_gamma_slider.picHeight			= MENU_BUTTON_MED_HEIGHT;
	s_video2_gamma_slider.textX				= MENU_BUTTON_TEXT_X;
	s_video2_gamma_slider.textY				= MENU_BUTTON_TEXT_Y;
	s_video2_gamma_slider.textEnum			= MBT_BRIGHTNESS;
	s_video2_gamma_slider.textcolor			= CT_BLACK;
	s_video2_gamma_slider.textcolor2		= CT_WHITE;
	s_video2_gamma_slider.thumbName			= GRAPHIC_BUTTONSLIDER;
	s_video2_gamma_slider.thumbHeight		= 32;
	s_video2_gamma_slider.thumbWidth		= 16;
	s_video2_gamma_slider.thumbGraphicWidth	= 9;
	s_video2_gamma_slider.thumbColor		= CT_DKBLUE1;
	s_video2_gamma_slider.thumbColor2		= CT_LTBLUE1;

	s_video_apply_action2.generic.type				= MTYPE_ACTION;
	s_video_apply_action2.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS|QMF_GRAYED;
	s_video_apply_action2.generic.x					= 490;
	s_video_apply_action2.generic.y					= 191;
	s_video_apply_action2.generic.callback			= ApplyChanges2;
	s_video_apply_action2.textEnum					= MBT_ACCEPT;
	s_video_apply_action2.textcolor					= CT_BLACK;
	s_video_apply_action2.textcolor2				= CT_WHITE;
	s_video_apply_action2.textcolor3				= CT_LTGREY;
	s_video_apply_action2.color						= CT_DKPURPLE1;
	s_video_apply_action2.color2					= CT_LTPURPLE1;
	s_video_apply_action2.color3					= CT_DKGREY;
	s_video_apply_action2.textX						= 5;
	s_video_apply_action2.textY						= 30;
	s_video_apply_action2.width						= 82;
	s_video_apply_action2.height					= 70;


	y = 330;
	s_video2_screensize_slider.generic.type		= MTYPE_SLIDER;
	s_video2_screensize_slider.generic.x			= x + 162;
	s_video2_screensize_slider.generic.y			= y;
	s_video2_screensize_slider.generic.flags		= QMF_SMALLFONT;
	s_video2_screensize_slider.generic.callback	= ScreensizeCallback;
	s_video2_screensize_slider.minvalue			= 30;
	s_video2_screensize_slider.maxvalue			= 100;
	s_video2_screensize_slider.color				= CT_DKPURPLE1;
	s_video2_screensize_slider.color2			= CT_LTPURPLE1;
	s_video2_screensize_slider.generic.name		= "menu/common/monbar_2.tga";
	s_video2_screensize_slider.width			= 256;
	s_video2_screensize_slider.height			= 32;
	s_video2_screensize_slider.focusWidth		= 145;
	s_video2_screensize_slider.focusHeight		= 18;
	s_video2_screensize_slider.picName			= GRAPHIC_SQUARE;
	s_video2_screensize_slider.picX				= x;
	s_video2_screensize_slider.picY				= y;
	s_video2_screensize_slider.picWidth			= MENU_BUTTON_MED_WIDTH + 21;
	s_video2_screensize_slider.picHeight			= MENU_BUTTON_MED_HEIGHT;
	s_video2_screensize_slider.textX				= MENU_BUTTON_TEXT_X;
	s_video2_screensize_slider.textY				= MENU_BUTTON_TEXT_Y;
	s_video2_screensize_slider.textEnum			= MBT_SCREENSIZE;
	s_video2_screensize_slider.textcolor			= CT_BLACK;
	s_video2_screensize_slider.textcolor2		= CT_WHITE;
	s_video2_screensize_slider.thumbName			= GRAPHIC_BUTTONSLIDER;
	s_video2_screensize_slider.thumbHeight		= 32;
	s_video2_screensize_slider.thumbWidth		= 16;
	s_video2_screensize_slider.thumbGraphicWidth	= 9;
	s_video2_screensize_slider.thumbColor		= CT_DKBLUE1;
	s_video2_screensize_slider.thumbColor2		= CT_LTBLUE1;

	y = 366;
	videodata2.cinematic_quality.generic.type			= MTYPE_SPINCONTROL;      
	videodata2.cinematic_quality.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	videodata2.cinematic_quality.generic.x				= x;
	videodata2.cinematic_quality.generic.y				= y;
	videodata2.cinematic_quality.generic.name			= GRAPHIC_BUTTONRIGHT;
	videodata2.cinematic_quality.generic.callback		= VideoUsingLoResCallback; 
	videodata2.cinematic_quality.color					= CT_DKPURPLE1;
	videodata2.cinematic_quality.color2					= CT_LTPURPLE1;
	videodata2.cinematic_quality.textX					= MENU_BUTTON_TEXT_X;
	videodata2.cinematic_quality.textY					= MENU_BUTTON_TEXT_Y;
	videodata2.cinematic_quality.textEnum				= MBT_CINEMATIC_QUALITY;
	videodata2.cinematic_quality.textcolor				= CT_BLACK;
	videodata2.cinematic_quality.textcolor2				= CT_WHITE;	
	videodata2.cinematic_quality.listnames				= s_quality_Names;

	y += 24;
	videodata2.anisotropicfiltering.generic.type			= MTYPE_SPINCONTROL;      
	videodata2.anisotropicfiltering.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	videodata2.anisotropicfiltering.generic.x				= x;
	videodata2.anisotropicfiltering.generic.y				= y;
	videodata2.anisotropicfiltering.generic.name			= GRAPHIC_BUTTONRIGHT;
	videodata2.anisotropicfiltering.generic.callback		= AnisotropicFilteringCallback; 
	videodata2.anisotropicfiltering.color					= CT_DKPURPLE1;
	videodata2.anisotropicfiltering.color2					= CT_LTPURPLE1;
	videodata2.anisotropicfiltering.textX					= MENU_BUTTON_TEXT_X;
	videodata2.anisotropicfiltering.textY					= MENU_BUTTON_TEXT_Y;
	videodata2.anisotropicfiltering.textEnum				= MBT_ANISOTROPICFILTERING;
	videodata2.anisotropicfiltering.textcolor				= CT_BLACK;
	videodata2.anisotropicfiltering.textcolor2				= CT_WHITE;	
	videodata2.anisotropicfiltering.listnames				= s_enable_Names;

	Menu_AddItem( &s_video2_menu, ( void * )&s_video2_gamma_slider);
	if (!uis.glconfig.deviceSupportsGamma)
	{
		Menu_AddItem( &s_video2_menu, ( void * )&s_video_apply_action2);
	}
	Menu_AddItem( &s_video2_menu, ( void * )&s_video2_screensize_slider);
	Menu_AddItem( &s_video2_menu, ( void * )&videodata2.cinematic_quality);
	Menu_AddItem( &s_video2_menu, ( void * )&videodata2.anisotropicfiltering);
}

/*
=================
UI_Video2SettingsGetCvars
=================
*/
static void	UI_Video2SettingsGetCvars()
{

	s_video2_gamma_slider.curvalue = ui.Cvar_VariableValue( "r_gamma" ) *  10.0f;

	videodata2.cinematic_quality.curvalue = ui.Cvar_VariableValue( "cl_VideoUsingLoRes" );

	videodata2.anisotropicfiltering.curvalue = ui.Cvar_VariableValue( "r_ext_texture_filter_anisotropic" );

	s_video2_screensize_slider.curvalue = ui.Cvar_VariableValue( "cg_viewsize" );
}

/*
=================
UI_Video2SettingsMenu
=================
*/
void UI_Video2SettingsMenu( void )
{
	ui.Key_SetCatcher( KEYCATCH_UI );

//	if (!s_video2_menu.initialized)
//	{
		Video2_MenuInit();
//	}

	UI_Video2SettingsGetCvars();

	UI_PushMenu( &s_video2_menu );
}


/*
=================
Video_MenuEvent
=================
*/
static void Video_MenuEvent (void* ptr, int event)
{
	menuframework_s*	m;

	if (event != QM_ACTIVATED)
		return;

	m = ((menucommon_s*)ptr)->parent;

	switch (((menucommon_s*)ptr)->id)
	{
		case ID_ARROWDWN:
			VideoDriver_Lines(1);
			break;

		case ID_ARROWUP:
			VideoDriver_Lines(-1);
			break;

		case ID_VIDEO:				// You're already in video menus, doofus
			break;

		case ID_VIDEODRIVERS:
			if (m != &s_videodriver.menu)	//	Not already in menu?
			{
				UI_PopMenu();			// Get rid of whatever is ontop
				UI_VideoDriverMenu();	// Move to the Controls Menu
			}
			break;

		case ID_VIDEODATA:
			if (m != &s_video_menu)	//	Not already in menu?
			{
				UI_PopMenu();			// Get rid of whatever is ontop
				UI_VideoDataMenu();		// Move to the Controls Menu
			}
			break;

		case ID_VIDEODATA2:
			if (m != &s_video2_menu)	//	Not already in menu?
			{
				UI_PopMenu();				// Get rid of whatever is ontop
				UI_Video2SettingsMenu();	// Move to the Controls Menu
			}
			break;

		case ID_CONTROLS:
			UI_PopMenu();			// Get rid of whatever is ontop
			UI_SetupWeaponsMenu();	// Move to the Controls Menu
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

		case ID_MAINMENU:
			UI_PopMenu();

	}
}

/*
=================
VideoDriver_Lines
=================
*/
void VideoDriver_Lines(int increment)
{
	int		i,i2;

	s_videodriver.currentDriverLine += increment;

	i=0;
	i2 = 0;

	i = (s_videodriver.currentDriverLine * 2);
	if (i<0)
	{
		s_videodriver.currentDriverLine = 0;
		return;
	}

	if (i>s_videodriver.driverCnt)
	{
		s_videodriver.currentDriverLine = (s_videodriver.driverCnt/2);
		return;
	}
	else if (i==s_videodriver.driverCnt)
	{
		s_videodriver.currentDriverLine = (s_videodriver.driverCnt/2) - 1;
		return;
	}

	if (!s_videodriver.drivers[i + 22])
	{
		s_videodriver.currentDriverLine -= increment;
		s_videodriver.activeArrowDwn = qfalse;
		return;
	}

	for (; i < MAX_VID_DRIVERS; i++)
	{
		if (s_videodriver.drivers[i])
		{
			if (i2<24)
			{
				((menutext_s *)g_videolines[i2])->string	= s_videodriver.drivers[i];      
				i2++;
			}
		}
		else 
		{
			if (i2<24)
			{
				((menutext_s *)g_videolines[i2])->string	= NULL;      
				i2++;
			}
			else 
			{
				break;
			}
		}
	}	

	// Set up arrows

	if (increment > 0)
	{
		s_videodriver.activeArrowUp = qtrue;
	}

	if (s_videodriver.currentDriverLine < 1)
	{
		s_videodriver.activeArrowUp = qfalse;
	}

	if (i2>= 24)
	{
		s_videodriver.activeArrowDwn = qtrue;
	}

	i = (s_videodriver.currentDriverLine * 2);
	if (!s_videodriver.drivers[i + 24]) 
	{
		s_videodriver.activeArrowDwn = qfalse;
		return;
	}

}

/*
=================
VideoDriver_LineSetup
=================
*/
void VideoDriver_LineSetup(void)
{
	char	*bufhold;
	char	*eptr;
	int		i;

	strcpy( s_videodriver.extensionsString, uis.glconfig.extensions_string );
	eptr = s_videodriver.extensionsString;
	i=0;

	s_videodriver.driverCnt = 0;

	while ( i < MAX_VID_DRIVERS && *eptr )
	{
		while ( *eptr )
		{
			bufhold = eptr;

			while(*bufhold !=  ' ')
			{
				++bufhold;
			}
			*bufhold = NULL;

			s_videodriver.drivers[i] = eptr;

			if (i<24)
			{
				((menutext_s *)g_videolines[i])->string	= eptr;      
			}

			bufhold++;	
			eptr = bufhold;
			s_videodriver.driverCnt++;
			i++;
		}
	}	

	// Set down arrows
	if (i> 24)
	{
		s_videodriver.activeArrowDwn = qtrue;
	}

	s_videodriver.currentDriverLine = 0;

}

/*
=================
VideoDriver_MenuKey
=================
*/
sfxHandle_t VideoDriver_MenuKey (int key)
{
	return ( Menu_DefaultKey( &s_videodriver.menu, key ) );
}


/*
=================
M_VideoDriverMenu_Graphics
=================
*/
void M_VideoDriverMenu_Graphics (void)
{
	float labelColor[] = { 0, 1.0, 0, 1.0 };
	float textColor[] = { 1, 1, 1, 1 };
	int i = 14;
	int x,y,x2,x3;

	UI_MenuFrame(&s_videodriver.menu);

	UI_DrawProportionalString(  74,  66, "207",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  84, "44909",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  188, "357",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  206, "250624",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  395, "456730-1",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);

	UI_Setup_MenuButtons();

//	ui.R_SetColor( colorTable[CT_DKPURPLE1]);
//	UI_DrawHandlePic(  30, 203, 47, 69, uis.whiteShader);	// Top Left column above two buttons
//	UI_DrawHandlePic(  30, 344, 47, 45, uis.whiteShader);	// Top Left column below two buttons

	// Top Frame
	UI_DrawHandlePic( 178, 136,  32, -32, s_videodriver.corner_ll_8_16);	// UL
	UI_DrawHandlePic( 178, 221,  32,  32, s_videodriver.corner_ll_8_16);	// LL
	UI_DrawHandlePic( 556, 136, -32, -32, s_videodriver.corner_ll_8_16);	// UR
	UI_DrawHandlePic( 556, 221, -32,  32, s_videodriver.corner_ll_8_16);	// LR
	UI_DrawHandlePic(194,157, 378,  8, uis.whiteShader);	// Top line
	UI_DrawHandlePic(178,165,  16, 60, uis.whiteShader);	// Left side
	UI_DrawHandlePic(572,165,  16, 60, uis.whiteShader);	// Right side
	UI_DrawHandlePic(194,224, 378,  8, uis.whiteShader);	// Bottom line


	// Lower Frame
	UI_DrawHandlePic( 178, 226,  32, -32, s_videodriver.corner_ll_16_16);	// UL
	UI_DrawHandlePic( 178, 414,  32,  32, s_videodriver.corner_ll_16_16);	// LL
	UI_DrawHandlePic( 556, 226, -32, -32, s_videodriver.corner_ll_16_16);	// UR
	UI_DrawHandlePic( 556, 414, -32,  32, s_videodriver.corner_ll_16_16);	// LR

	UI_DrawHandlePic( 194, 237, 378,  16, uis.whiteShader);	// Top line
	UI_DrawHandlePic( 178, 252,  16, 168, uis.whiteShader);	// Left side

	UI_DrawHandlePic( 572, 261,  16,  15, uis.whiteShader);	// Right side
	UI_DrawHandlePic( 572, 279,  16, 114, uis.whiteShader);	// Right side
	UI_DrawHandlePic( 572, 396,  16,  15, uis.whiteShader);	// Right side

	UI_DrawHandlePic( 194, 419, 378,  16, uis.whiteShader);	// Bottom line

	ui.R_SetColor( colorTable[CT_LTGOLD1]);
	if (s_videodriver.activeArrowUp)
	{
		UI_DrawHandlePic( 382, 237, 32,  -14, s_videodriver.arrow_dn);	
	}

	if (s_videodriver.activeArrowDwn)
	{
		UI_DrawHandlePic( 382, 422, 32,   14, s_videodriver.arrow_dn);	
	}

	x = 204;
	x2 = 259;
	x3 = x2 + 150;
	y = 168;
	UI_DrawProportionalString( x, y, menu_normal_text[MNT_VENDOR], UI_LEFT|UI_TINYFONT, labelColor );
	UI_DrawProportionalString( x2, y, uis.glconfig.vendor_string, UI_LEFT|UI_TINYFONT, textColor );
	y += 14;
	UI_DrawProportionalString( x, y, menu_normal_text[MNT_VERSION], UI_LEFT|UI_TINYFONT, labelColor );
	UI_DrawProportionalString( x2, y, uis.glconfig.version_string, UI_LEFT|UI_TINYFONT, textColor );
	y += 14;
	UI_DrawProportionalString( x, y, menu_normal_text[MNT_RENDERER], UI_LEFT|UI_TINYFONT, labelColor );
	UI_DrawProportionalString( x2, y, uis.glconfig.renderer_string, UI_LEFT|UI_TINYFONT, textColor );
	y += 14;
	UI_DrawProportionalString( x, y, menu_normal_text[MNT_PIXELFORMAT], UI_LEFT|UI_TINYFONT, labelColor );
	UI_DrawProportionalString( x2, y, va("color(%d-bits) Z(%d-bit) stencil(%d-bits)", uis.glconfig.colorBits, uis.glconfig.depthBits, uis.glconfig.stencilBits), UI_LEFT|UI_TINYFONT, textColor );

}

/*
=================
VideoDriver_MenuDraw
=================
*/
static void VideoDriver_MenuDraw (void)
{

	M_VideoDriverMenu_Graphics();

	Menu_Draw( &s_videodriver.menu );
}

/*
=================
UI_VideoDriverMenu_Cache
=================
*/
void UI_VideoDriverMenu_Cache(void)
{
	s_videodriver.corner_ll_16_16 = ui.R_RegisterShaderNoMip("menu/common/corner_ll_16_16.tga");
	s_videodriver.corner_ll_8_16 = ui.R_RegisterShaderNoMip("menu/common/corner_ll_8_16.tga");
	s_videodriver.arrow_dn = ui.R_RegisterShaderNoMip("menu/common/arrow_dn_16.tga");
	ui.R_RegisterShaderNoMip("menu/common/arrow_up_16.tga");

}

/*
=================
Video_MenuInit
=================
*/
static void VideoDriver_MenuInit( void )
{
	int		i,x,y,x2;

	UI_VideoDriverMenu_Cache();

	s_videodriver.menu.nitems					= 0;
	s_videodriver.menu.wrapAround				= qtrue;
	s_videodriver.menu.opening					= NULL;
	s_videodriver.menu.closing					= NULL;
	s_videodriver.menu.draw						= VideoDriver_MenuDraw;
	s_videodriver.menu.key						= VideoDriver_MenuKey;
	s_videodriver.menu.fullscreen				= qtrue;
	s_videodriver.menu.descX					= MENU_DESC_X;
	s_videodriver.menu.descY					= MENU_DESC_Y;
	s_videodriver.menu.listX					= 230;
	s_videodriver.menu.listY					= 188;
	s_videodriver.menu.titleX					= MENU_TITLE_X;
	s_videodriver.menu.titleY					= MENU_TITLE_Y;
	s_videodriver.menu.titleI					= MNT_CONTROLSMENU_TITLE;
	s_videodriver.menu.footNoteEnum				= MNT_VIDEODRIVERS;

	SetupMenu_TopButtons(&s_videodriver.menu,MENU_VIDEO,NULL);

	Video_SideButtons(&s_videodriver.menu,ID_VIDEODRIVERS);

	s_videodriver.arrowup.generic.type				= MTYPE_BITMAP;      
	s_videodriver.arrowup.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS;
	s_videodriver.arrowup.generic.x					= 572;
	s_videodriver.arrowup.generic.y					= 262;
	s_videodriver.arrowup.generic.name				= "menu/common/arrow_up_16.tga";
	s_videodriver.arrowup.generic.id				= ID_ARROWUP;
	s_videodriver.arrowup.generic.callback			= Video_MenuEvent;
	s_videodriver.arrowup.width						= 16;
	s_videodriver.arrowup.height					= 16;
	s_videodriver.arrowup.color						= CT_DKBLUE1;
	s_videodriver.arrowup.color2					= CT_LTBLUE1;
	s_videodriver.arrowup.textX						= 0;
	s_videodriver.arrowup.textY						= 0;
	s_videodriver.arrowup.textEnum					= MBT_NONE;
	s_videodriver.arrowup.textcolor					= CT_BLACK;
	s_videodriver.arrowup.textcolor2				= CT_WHITE;
	Menu_AddItem( &s_videodriver.menu,( void * ) &s_videodriver.arrowup);

	s_videodriver.arrowdwn.generic.type				= MTYPE_BITMAP;      
	s_videodriver.arrowdwn.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_videodriver.arrowdwn.generic.x				= 572;
	s_videodriver.arrowdwn.generic.y				= 397;
	s_videodriver.arrowdwn.generic.name				= "menu/common/arrow_dn_16.tga";
	s_videodriver.arrowdwn.generic.id				= ID_ARROWDWN;
	s_videodriver.arrowdwn.generic.callback			= Video_MenuEvent;
	s_videodriver.arrowdwn.width					= 16;
	s_videodriver.arrowdwn.height					= 16;
	s_videodriver.arrowdwn.color					= CT_DKBLUE1;
	s_videodriver.arrowdwn.color2					= CT_LTBLUE1;
	s_videodriver.arrowdwn.textX					= 0;
	s_videodriver.arrowdwn.textY					= 0;
	s_videodriver.arrowdwn.textEnum					= MBT_NONE;
	s_videodriver.arrowdwn.textcolor				= CT_BLACK;
	s_videodriver.arrowdwn.textcolor2				= CT_WHITE;
	Menu_AddItem( &s_videodriver.menu, ( void * ) &s_videodriver.arrowdwn);

	s_videodriver.activeArrowDwn = qfalse;
	s_videodriver.activeArrowUp = qfalse;

	x = 204;
	x2 = 404;
	y = 260;

	for (i=0;i<24;i++)
	{
		((menutext_s *)g_videolines[i])->generic.type		= MTYPE_TEXT;      
		((menutext_s *)g_videolines[i])->generic.flags		= QMF_LEFT_JUSTIFY | QMF_INACTIVE;
		((menutext_s *)g_videolines[i])->generic.y			= y;
		if ((i % 2 ) == 0)
		{
			((menutext_s *)g_videolines[i])->generic.x			= x;
		}
		else 
		{
			((menutext_s *)g_videolines[i])->generic.x			= x2;
			y +=13;
		}


		((menutext_s *)g_videolines[i])->buttontextEnum		= MBT_NONE;
		((menutext_s *)g_videolines[i])->style				= UI_TINYFONT | UI_LEFT;
		((menutext_s *)g_videolines[i])->color				= CT_LTPURPLE1;
		Menu_AddItem( &s_videodriver.menu, ( void * )g_videolines[i]);

	}

	// Print extensions
	VideoDriver_LineSetup();
}


/*
=================
UI_VideoDriverMenu
=================
*/
void UI_VideoDriverMenu( void )
{
	ui.Key_SetCatcher( KEYCATCH_UI );

//	if (!s_videodriver_menu.initialized)
//	{
		VideoDriver_MenuInit();
//	}

	UI_PushMenu( &s_videodriver.menu );
}


static void *holdControlPtr;
static int holdControlEvent;

void VideoSideButtonsAction( qboolean result ) 
{
	if ( result )	// Yes - do it
	{
		Video_MenuEvent(holdControlPtr, holdControlEvent);
	}
}

/*
=================
VideoSideButtons_MenuEvent
=================
*/
static void VideoSideButtons_MenuEvent (void* ptr, int event)
{

	if (event != QM_ACTIVATED)
		return;

	holdControlPtr = ptr;
	holdControlEvent = event;

	if (s_video_apply_action.generic.flags & QMF_BLINK)	// Video apply changes button is flashing
	{
		// EFSP: auto-apply pending video changes instead of the "lose settings?" confirm (user request).
		ApplyChanges( NULL, QM_ACTIVATED );
		Video_MenuEvent (holdControlPtr, holdControlEvent);
	}
	else	// Go ahead, act normal
	{
		Video_MenuEvent (holdControlPtr, holdControlEvent);
	}
}
/*
=================
Video_SideButtons
=================
*/
void Video_SideButtons(menuframework_s *menu,int menuType)
{

	// Button Data
	s_video_data.generic.type				= MTYPE_BITMAP;      
	s_video_data.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS;
	s_video_data.generic.x					= video_sidebuttons[0][0];
	s_video_data.generic.y					= video_sidebuttons[0][1];
	s_video_data.generic.name				= GRAPHIC_SQUARE;
	s_video_data.generic.id					= ID_VIDEODATA;
	if (menuType == ID_VIDEODATA)
	{
		s_video_data.generic.callback			= VideoSideButtons_MenuEvent;
	}
	else
	{
		s_video_data.generic.callback			= Video_MenuEvent;
	}
	s_video_data.width						= MENU_BUTTON_MED_WIDTH - 10;
	s_video_data.height						= MENU_BUTTON_MED_HEIGHT;
	s_video_data.color						= CT_DKPURPLE1;
	s_video_data.color2						= CT_LTPURPLE1;
	s_video_data.textX						= 5;
	s_video_data.textY						= 2;
	s_video_data.textEnum					= MBT_VIDEODATA;
	if (menuType == ID_VIDEODATA)
	{
		s_video_data.textcolor				= CT_WHITE;
		s_video_data.textcolor2				= CT_WHITE;
		s_video_data.generic.flags			= QMF_GRAYED;
	}
	else
	{
		s_video_data.textcolor				= CT_BLACK;
		s_video_data.textcolor2				= CT_WHITE;
	}

	s_video_data2.generic.type				= MTYPE_BITMAP;      
	s_video_data2.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS;
	s_video_data2.generic.x					= video_sidebuttons[1][0];
	s_video_data2.generic.y					= video_sidebuttons[1][1];
	s_video_data2.generic.name				= GRAPHIC_SQUARE;
	s_video_data2.generic.id				= ID_VIDEODATA2;
	s_video_data2.generic.callback			= Video_MenuEvent;
	if (menuType == ID_VIDEODATA)
	{
		s_video_data2.generic.callback			= VideoSideButtons_MenuEvent;
	}
	else
	{
		s_video_data2.generic.callback			= Video_MenuEvent;
	}
	s_video_data2.width						= MENU_BUTTON_MED_WIDTH - 10;
	s_video_data2.height					= MENU_BUTTON_MED_HEIGHT;
	s_video_data2.color						= CT_DKPURPLE1;
	s_video_data2.color2					= CT_LTPURPLE1;
	s_video_data2.textX						= 5;
	s_video_data2.textY						= 2;
	s_video_data2.textEnum					= MBT_VIDEODATA2;
	s_video_data2.textcolor					= CT_WHITE;
	s_video_data2.textcolor2				= CT_WHITE;
	if (menuType == ID_VIDEODATA2)
	{
		s_video_data2.textcolor				= CT_WHITE;
		s_video_data2.textcolor2			= CT_WHITE;
		s_video_data2.generic.flags			= QMF_GRAYED;
	}
	else
	{
		s_video_data2.textcolor				= CT_BLACK;
		s_video_data2.textcolor2			= CT_WHITE;
	}

	s_video_drivers.generic.type			= MTYPE_BITMAP;      
	s_video_drivers.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_video_drivers.generic.x				= video_sidebuttons[2][0];
	s_video_drivers.generic.y				= video_sidebuttons[2][1];
	s_video_drivers.generic.name			= GRAPHIC_SQUARE;
	s_video_drivers.generic.id				= ID_VIDEODRIVERS;
	if (menuType == ID_VIDEODATA)
	{
		s_video_drivers.generic.callback			= VideoSideButtons_MenuEvent;
	}
	else
	{
		s_video_drivers.generic.callback			= Video_MenuEvent;
	}
	s_video_drivers.width					= MENU_BUTTON_MED_WIDTH - 10;
	s_video_drivers.height					= MENU_BUTTON_MED_HEIGHT;
	s_video_drivers.color					= CT_DKPURPLE1;
	s_video_drivers.color2					= CT_LTPURPLE1;
	s_video_drivers.textX					= 5;
	s_video_drivers.textY					= 2;
	s_video_drivers.textEnum				= MBT_VIDEODRIVERS;
	if (menuType == ID_VIDEODRIVERS)
	{
		s_video_drivers.textcolor			= CT_WHITE;
		s_video_drivers.textcolor2			= CT_WHITE;
		s_video_drivers.generic.flags		= QMF_GRAYED;
	}
	else
	{
		s_video_drivers.textcolor			= CT_BLACK;
		s_video_drivers.textcolor2			= CT_WHITE;
	}

	Menu_AddItem( menu, ( void * )&s_video_data);
	Menu_AddItem( menu, ( void * )&s_video_data2);
	Menu_AddItem( menu, ( void * )&s_video_drivers);

}