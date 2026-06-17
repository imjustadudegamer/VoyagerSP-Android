/**********************************************************************
	UI_ATOMS.C

	User interface building blocks and support functions.
**********************************************************************/
#include "ui_local.h"
#include "gameinfo.h"
#include "../qcommon/stv_version.h"

uiimport_t	ui;
uiStatic_t	uis;
qboolean	m_entersound;		// after a frame, so caching won't disrupt the sound

//externs
extern menuframework_s s_main_menu;
float UI_ProportionalSizeScale( int style );
void UI_LoadFonts( void );
void UI_LoadMenuText(void);
void UI_LoadButtonText(void);
void UI_LanguageFilename(char *baseName,char *baseExtension,char *finalName);
void UI_LoadMenu_f( void );
void UI_SaveMenu_f( void );
void UI_TurboliftMenu(void);
void UI_HolodeckMenu(int menuType);
void UI_VirtualLoadMenu(void);
int MS_CheckDefines(char *token);
static void MS_GetID(char **buffer,char *charValue);
void	UI_LoadGenericMenu( char *fileName);

qboolean G_ParseString( char **data, char **s );

vec4_t menuColors[24];

typedef struct 
{
	int				initialized;	// Has this structure been initialized
	qhandle_t		cornerUpper;
	qhandle_t		cornerUpper2;
	qhandle_t		cornerLower;
} menuframe_t;

static menuframe_t s_menuframe;

//locals
void MenuFrame_Cache(void);

/*
=================
UI_PushMenu
=================
*/
void UI_PushMenu( menuframework_s *menu )
{
	int		i;
	menucommon_s*	item;

	// avoid stacking menus invoked by hotkeys
	for (i=0 ; i<uis.menusp ; i++)
	{
		if (uis.stack[i] == menu)
		{
			uis.menusp = i;
			break;
		}
	}

	if (i == uis.menusp)
	{
		if (uis.menusp >= MAX_MENUDEPTH)
			ui.Error (ERR_FATAL, "UI_PushMenu: menu stack overflow");

		uis.stack[uis.menusp++] = menu;
	}

	uis.activemenu = menu;

	// default cursor position
	menu->cursor        = 0;
	menu->cursor_prev   = 0;

	m_entersound = qtrue;

	ui.Key_SetCatcher( KEYCATCH_UI );

	// force first available item to have focus
	for (i=0; i<menu->nitems; i++)
	{
		item = (menucommon_s *)menu->items[i];
		if (!(item->flags & (QMF_GRAYED|QMF_MOUSEONLY|QMF_INACTIVE)))
		{
			menu->cursor_prev = -1;
			Menu_SetCursor( menu, i );
			break;
		}
	}

	uis.firstdraw = qtrue;
}

/*
=================
UI_PopMenu
=================
*/
void UI_PopMenu (void)
{
	ui.S_StartLocalSound( menu_out_sound, CHAN_LOCAL_SOUND );

	uis.menusp--;

	if (uis.menusp < 0)
		ui.Error (ERR_FATAL, "UI_PopMenu: menu stack underflow");

	if (uis.menusp) {
		uis.activemenu = uis.stack[uis.menusp-1];
		uis.firstdraw = qtrue;
	} 
	else {
		UI_ForceMenuOff ();
	}
}

void UI_ForceMenuOff (void)
{
	uis.menusp     = 0;
	uis.activemenu = NULL;

	ui.Key_SetCatcher( ui.Key_GetCatcher() & ~KEYCATCH_UI );
	ui.Key_ClearStates();
	ui.Cvar_Set( "cl_paused", "0" );
}

/*
=================
UI_LerpColor
=================
*/
 void UI_LerpColor(vec4_t a, vec4_t b, vec4_t c, float t)
{
	int i;

	// lerp and clamp each component
	for (i=0; i<4; i++)
	{
		c[i] = a[i] + t*(b[i]-a[i]);
		if (c[i] < 0)
			c[i] = 0;
		else if (c[i] > 1.0)
			c[i] = 1.0;
	}
}

#define CHARMAX 256

/*
=================
UI_DrawProportionalString2
=================
*/

#define PROPB_GAP_WIDTH		4
#define PROPB_SPACE_WIDTH	12
#define PROPB_HEIGHT		36

static int	propMap[CHARMAX][3];
static int	propMapTiny[CHARMAX][3];
static int	propMapBig[CHARMAX][3];

static int propMapB[26][3] = {
{11, 12, 33},
{49, 12, 31},
{85, 12, 31},
{120, 12, 30},
{156, 12, 21},
{183, 12, 21},
{207, 12, 32},

{13, 55, 30},
{49, 55, 13},
{66, 55, 29},
{101, 55, 31},
{135, 55, 21},
{158, 55, 40},
{204, 55, 32},

{12, 97, 31},
{48, 97, 31},
{82, 97, 30},
{118, 97, 30},
{153, 97, 30},
{185, 97, 25},
{213, 97, 30},

{11, 139, 32},
{42, 139, 51},
{93, 139, 32},
{126, 139, 31},
{158, 139, 25},
};


int UI_ProportionalStringWidth( const char* str,int style ) {
	const char *	s;
//	int				ch;
	unsigned char ch;
	int				charWidth;
	int				width;

	if (style & UI_TINYFONT)
	{
		s = str;
		width = 0;
		while ( *s ) 
		{
			ch = *s & 255;
			charWidth = propMapTiny[ch][2];
			if ( charWidth != -1 ) 
			{
				width += charWidth;
				width += PROP_GAP_TINY_WIDTH;
			}
			s++;
		}

		width -= PROP_GAP_TINY_WIDTH;
	}
	else if (style & UI_BIGFONT)
	{
		s = str;
		width = 0;
		while ( *s ) 
		{
			ch = *s & 255;
			charWidth = propMapBig[ch][2];
			if ( charWidth != -1 ) 
			{
				width += charWidth;
				width += PROP_GAP_BIG_WIDTH;
			}
			s++;
		}

		width -= PROP_GAP_BIG_WIDTH;
	}
	else 
	{
		s = str;
		width = 0;
		while ( *s ) 
		{
			ch = *s & 255;
			charWidth = propMap[ch][2];
			if ( charWidth != -1 ) 
			{
				width += charWidth;
				width += PROP_GAP_WIDTH;
			}
			s++;
		}

		width -= PROP_GAP_WIDTH;
	}

	return width;
}

static int specialTinyPropChars[CHARMAX][2] = {
{0, 0},
{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},	// 10
{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},	// 20
{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},	// 30
{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},	// 40
{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},	// 50
{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},	// 60
{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},	// 70
{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},	// 80
{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},	// 90
{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},	// 100
{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},	// 110
{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},	// 120
{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},	// 130
{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},	// 140
{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},	// 150
{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{2,-3},{0, 0},	// 160
{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},	// 170
{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},	// 180
{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},	// 190
{0,-1},{2,-3},{2,-3},{2,-3},{2,-3},{2,-3},{2,-3},{0, 0},{2, 0},{2,-3},	// 200
{2,-3},{2,-3},{2,-3},{2,-3},{2,-3},{2,-3},{2,-3},{0,-1},{2,-3},{2,-3},	// 210
{2,-3},{3,-3},{2,-3},{2,-3},{0, 0},{0,-1},{2,-3},{2,-3},{2,-3},{2,-3},	// 220
{2,-3},{0,-1},{0,-1},{2,-3},{2,-3},{2,-3},{2,-3},{2,-3},{2,-3},{0, 0},	// 230
{2, 0},{2,-3},{2,-3},{2,-3},{2,-3},{2,-3},{2,-3},{2,-3},{2,-3},{0, 0},	// 240
{2,-3},{2,-3},{2,-3},{2,-3},{2,-3},{2,-3},{0, 0},{0,-1},{2,-3},{2,-3},	// 250
{2,-3},{2,-3},{2,-3},{0,-1},{2,-3}										// 255
}; 


static int specialPropChars[CHARMAX][2] = {
{0, 0},
{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},	// 10
{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},	// 20
{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},	// 30
{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},	// 40
{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},	// 50
{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},	// 60
{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},	// 70
{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},	// 80
{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},	// 90
{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},	// 100
{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},	// 110
{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},	// 120
{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},	// 130
{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},	// 140
{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},	// 150
{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},	// 160
{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},	// 170
{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},	// 180
{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},	// 190
{2,-2},{2,-2},{2,-2},{2,-2},{2,-2},{2,-2},{2,-2},{0, 0},{1, 1},{2,-2},	// 200
{2,-2},{2,-2},{2,-2},{2,-2},{2,-2},{2,-2},{2,-2},{0, 0},{2,-2},{2,-2},	// 210
{2,-2},{2,-2},{2,-2},{2,-2},{0, 0},{0, 0},{2,-2},{2,-2},{2,-2},{2,-2},	// 220
{2,-2},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},	// 230
{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},	// 240
{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},	// 250
{0, 0},{0, 0},{0, 0},{0, 0},{0, 0}										// 255
}; 


static int specialBigPropChars[CHARMAX][2] = {
{0, 0},
{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},	// 10
{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},	// 20
{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},	// 30
{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},	// 40
{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},	// 50
{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},	// 60
{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},	// 70
{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},	// 80
{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},	// 90
{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},	// 100
{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},	// 110
{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},	// 120
{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},	// 130
{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},	// 140
{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},	// 150
{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},	// 160
{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},	// 170
{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},	// 180
{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},	// 190
{3,-3},{3,-3},{3,-3},{3,-3},{3,-3},{3,-3},{3,-3},{0, 0},{3, 1},{3,-3},	// 200
{3,-3},{3,-3},{3,-3},{3,-3},{3,-3},{3,-3},{3,-3},{0, 0},{3,-3},{3,-3},	// 210
{3,-3},{3,-3},{3,-3},{3,-3},{0, 0},{0, 0},{3,-3},{3,-3},{3,-3},{3,-3},	// 220
{3,-3},{0, 0},{0, 0},{3,-3},{3,-3},{3,-3},{3,-3},{3,-3},{3,-3},{0, 0},	// 230
{3, 1},{3,-3},{3,-3},{3,-3},{3,-3},{3,-3},{3,-3},{3,-3},{3,-3},{0, 0},	// 240
{3,-3},{3,-3},{3,-3},{3,-3},{3,-3},{3,-3},{0, 0},{0, 0},{3,-3},{3,-3},	// 250
{3,-3},{3,-3},{3,-3},{0, 0},{3,-3}										// 255
}; 

static void UI_DrawProportionalString2( int x, int y, const char* str, vec4_t color, int style, qhandle_t charset )
{
	const char* s;
	unsigned char ch;
	float	ax;
	float	ay,holdY;
	float	aw;
	float	ah;
	float	frow;
	float	fcol;
	float	fwidth;
	float	fheight;
	float	sizeScale;
	int		special;

	assert (str);

	// draw the colored text
	ui.R_SetColor( color );
	
	// EFSP 4:3: proportional text is positioned manually here (bypasses UI_AdjustFrom640), so it MUST add the
	// same centering bias or every title/button label shifts left of its biased button art. bias=0 in 16:9.
	ax = x * uis.scalex + uis.bias;
	ay = y * uis.scaley;
	holdY = ay;

	sizeScale = UI_ProportionalSizeScale( style );

	if (style & UI_TINYFONT)
	{
		s = str;
		while ( *s )
		{
			ch = *s & 255;
			if ( ch == ' ' ) 
			{
				aw = (float)PROP_SPACE_TINY_WIDTH;
			}
			else if ( propMapTiny[ch][2] != -1 ) 
			{
				// Because some foreign characters were a little different
				special = specialTinyPropChars[ch][0];
				ay = holdY + (specialTinyPropChars[ch][1] * uis.scaley);

				fcol = (float ) propMapTiny[ch][0] / 256.0f;
				frow = (float)propMapTiny[ch][1] / 256.0f;
				fwidth = (float)propMapTiny[ch][2] / 256.0f;
				fheight = (float)(PROP_TINY_HEIGHT + special)  / 256.0f;
				aw = (float)propMapTiny[ch][2] * uis.scalex * sizeScale;
				ah = (float)(PROP_TINY_HEIGHT+ special) * uis.scaley * sizeScale;

				ui.R_DrawStretchPic( ax, ay, aw, ah, fcol, frow, fcol + fwidth, frow + fheight, charset );
			}
			else
			{
				aw = 0;
			}

			ax += (aw + (float)PROP_GAP_TINY_WIDTH * uis.scalex * sizeScale);
			s++;
		}
	}
	else if (style & UI_BIGFONT)
	{
		s = str;
		while ( *s )
		{
			ch = *s & 255;
			if ( ch == ' ' ) 
			{
				aw = (float)PROP_SPACE_BIG_WIDTH * uis.scalex;
			}
			else if ( propMapBig[ch][2] != -1 ) 
			{
				// Because some foreign characters were a little different
				special = specialBigPropChars[ch][0];
				ay = holdY + (specialBigPropChars[ch][1] * uis.scaley);

				fcol = (float ) propMapBig[ch][0] / 256.0f;
				frow = (float)propMapBig[ch][1] / 256.0f;
				fwidth = (float)propMapBig[ch][2] / 256.0f;
				fheight = (float)(PROP_BIG_HEIGHT + special) / 256.0f;
				aw = (float)propMapBig[ch][2] * uis.scalex * sizeScale;
				ah = (float)(PROP_BIG_HEIGHT + special) * uis.scaley * sizeScale;

				ui.R_DrawStretchPic( ax, ay, aw, ah, fcol, frow, fcol + fwidth, frow + fheight, charset );
			}
			else
			{
				aw = 0;
			}

			ax += (aw + (float)PROP_GAP_BIG_WIDTH * uis.scalex * sizeScale);
			s++;
		}
	} 
	else
	{
		s = str;
		while ( *s )
		{
			ch = *s & 255;
			if ( ch == ' ' ) 
			{
				aw = (float)PROP_SPACE_WIDTH * uis.scalex * sizeScale;
			}
			else if ( propMap[ch][2] != -1 ) 
			{
				// Because some foreign characters were a little different
				special = specialPropChars[ch][0];
				ay = holdY + (specialPropChars[ch][1] * uis.scaley);

				fcol = (float ) propMap[ch][0] / 256.0f;
				frow = (float)propMap[ch][1] / 256.0f;
				fwidth = (float)propMap[ch][2] / 256.0f;
				fheight = (float)(PROP_HEIGHT + special) / 256.0f;
				aw = (float)propMap[ch][2] * uis.scalex * sizeScale;
				ah = (float)(PROP_HEIGHT + special) * uis.scaley * sizeScale;

				ui.R_DrawStretchPic( ax, ay, aw, ah, fcol, frow, fcol + fwidth, frow + fheight, charset );
			}
			else
			{
				aw = 0;
			}

			ax += (aw + (float)PROP_GAP_WIDTH * uis.scalex * sizeScale);
			s++;
		}
	}

	ui.R_SetColor( NULL );

}

/*
=================
UI_ProportionalSizeScale
=================
*/
 float UI_ProportionalSizeScale( int style ) 
{

	if(  style & UI_SMALLFONT ) 
	{
		return PROP_SMALL_SIZE_SCALE;
	}
	else if(  style & UI_TINYFONT ) 
	{
		return PROP_TINY_SIZE_SCALE;
	}
	else if(  style & UI_BIGFONT ) 
	{
		return PROP_BIG_SIZE_SCALE;
	}
	else if(  style & UI_GIANTFONT ) 
	{
		return PROP_GIANT_SIZE_SCALE;
	}

	return 1.00;
}


/*
=================
UI_DrawProportionalString
=================
*/
void UI_DrawProportionalString( int x, int y, const char* str, int style, vec4_t color ) {
	vec4_t	drawcolor;
	int		width;
	float	sizeScale;
	int		charstyle=0;

	assert (str);

	if ((style & UI_BLINK) && ((uis.realtime/BLINK_DIVISOR) & 1))
		return;

	// Get char style
	if (style & UI_TINYFONT)
	{
		charstyle = UI_TINYFONT;
	}
	else if (style & UI_SMALLFONT)
	{
		charstyle = UI_SMALLFONT;
	}
	else if (style & UI_BIGFONT)
	{
		charstyle = UI_BIGFONT;
	}
	else if (style & UI_GIANTFONT)
	{
		charstyle = UI_GIANTFONT;
	}

	sizeScale = UI_ProportionalSizeScale( charstyle );

	switch( style & UI_FORMATMASK ) {
		case UI_CENTER:
			width = UI_ProportionalStringWidth( str,charstyle) * sizeScale;
			x -= width / 2;
			break;

		case UI_RIGHT:
			width = UI_ProportionalStringWidth( str,charstyle ) * sizeScale;
			x -= width;
			break;

		case UI_LEFT:
		default:
			break;
	}

	if ( style & UI_DROPSHADOW ) {
		drawcolor[0] = drawcolor[1] = drawcolor[2] = 0;
		drawcolor[3] = color[3];
		if (style & UI_TINYFONT)
		{
			UI_DrawProportionalString2( x+2, y+2, str, drawcolor, charstyle, uis.charsetPropTiny );
		}
		else if (style & UI_BIGFONT)
		{
			UI_DrawProportionalString2( x+2, y+2, str, drawcolor, charstyle, uis.charsetPropBig );
		}
		else
		{
			UI_DrawProportionalString2( x+2, y+2, str, drawcolor, charstyle, uis.charsetProp );
		}
	}

	if ( style & UI_INVERSE ) {
		drawcolor[0] = color[0] * 0.7;
		drawcolor[1] = color[1] * 0.7;
		drawcolor[2] = color[2] * 0.7;
		drawcolor[3] = color[3];
		if (style & UI_TINYFONT)
		{
			UI_DrawProportionalString2( x, y, str, drawcolor, charstyle, uis.charsetPropTiny );
		}
		else if (style & UI_BIGFONT)
		{
			UI_DrawProportionalString2( x, y, str, drawcolor, charstyle, uis.charsetPropBig );
		}
		else
		{
			UI_DrawProportionalString2( x, y, str, drawcolor, charstyle, uis.charsetProp );
		}
		return;
	}

	if ( style & UI_PULSE ) {
		drawcolor[0] = color[0] * 0.7;
		drawcolor[1] = color[1] * 0.7;
		drawcolor[2] = color[2] * 0.7;
		drawcolor[3] = color[3];
		if (style & UI_TINYFONT)
		{
			UI_DrawProportionalString2( x, y, str, color, charstyle, uis.charsetPropTiny );
		}
		else if (style & UI_BIGFONT)
		{
			UI_DrawProportionalString2( x, y, str, color, charstyle, uis.charsetPropBig );
		}
		else 
		{
			UI_DrawProportionalString2( x, y, str, color, charstyle, uis.charsetProp );
		}


		drawcolor[0] = color[0];
		drawcolor[1] = color[1];
		drawcolor[2] = color[2];
		drawcolor[3] = 0.5 + 0.5 * sin( uis.realtime / PULSE_DIVISOR );
		if (style & UI_TINYFONT)
		{
			UI_DrawProportionalString2( x, y, str, color, charstyle, uis.charsetPropTiny );
		}
		else if (style & UI_BIGFONT)
		{
			UI_DrawProportionalString2( x, y, str, color, charstyle, uis.charsetPropBig );
		}
		else 
		{
			UI_DrawProportionalString2( x, y, str, drawcolor, charstyle, uis.charsetProp );
		}
		return;
	}

	if (style & UI_TINYFONT)
	{
		UI_DrawProportionalString2( x, y, str, color, charstyle, uis.charsetPropTiny );
	}
	else if (style & UI_BIGFONT)
	{
		UI_DrawProportionalString2( x, y, str, color, charstyle, uis.charsetPropBig );
	}
	else
	{
		UI_DrawProportionalString2( x, y, str, color, charstyle, uis.charsetProp );
	}
}

/*
=================
UI_DrawChar2
=================
*/
static void UI_DrawChar2( int x, int y, int ch, int charw, int charh) {
	int row, col;
	float frow, fcol;
	float size;
	float	ax, ay, aw, ah;
	float size2;

	if ( y < -charh ) {
		return;
	}

	ch &= 255;

	if ( ch == ' ' ) {
		return;
	}

	ax = x;
	ay = y;
	aw = charw;
	ah = charh;
	UI_AdjustFrom640( &ax, &ay, &aw, &ah );

	row = ch>>4;
	col = ch&15;

	frow = row*0.0625;
	fcol = col*0.0625;
	size = 0.03125;
	size2 = 0.0625;

	ui.R_DrawStretchPic( ax, ay, aw, ah, fcol, frow, fcol + size, frow + size2, uis.charset );
}


/*
=================
UI_DrawString2
=================
*/
static void UI_DrawString2( int x, int y, const char* str, vec4_t color, int charw, int charh )
{
	const char* s;
	int	xx;
	int forceColor = 1; //APSFIXME;
	vec4_t	tempcolor;

	// draw the colored text
	s  = str;
	xx = x;
	ui.R_SetColor( color );

	while ( *s ) {
		if ( Q_IsColorString( s ) )
		{
			if ( !forceColor ) {
				memcpy( tempcolor, g_color_table[ColorIndex(*(s+1))], sizeof( tempcolor ) );
				tempcolor[3] = color[3];
				ui.R_SetColor( tempcolor );
			}
			s += 2;
			continue;
		}

		UI_DrawChar2( xx, y, *s, charw, charh );
		xx += charw;
		s++;
	}

	ui.R_SetColor( NULL );
}

/*
=================
UI_DrawString
=================
*/
void UI_DrawString( int x, int y, const char* str, int style, vec4_t color )
{
	int		len;
	int		charw;
	int		charh;
	vec4_t	newcolor;
	vec4_t	lowlight;
	vec4_t	dropcolor;
	float	*drawcolor;

	if ((style & UI_BLINK) && ((uis.realtime/BLINK_DIVISOR) & 1))
		return;

	if (style & UI_SMALLFONT)
	{
		charw =	SMALLCHAR_WIDTH;
		charh =	SMALLCHAR_HEIGHT;
	}
	else
	{
		charw =	BIGCHAR_WIDTH;
		charh =	BIGCHAR_HEIGHT;
	}

	if (style & UI_PULSE)
	{
		lowlight[0] = 0.8*color[0]; 
		lowlight[1] = 0.8*color[1];
		lowlight[2] = 0.8*color[2];
		lowlight[3] = 0.8*color[3];
		UI_LerpColor(color,lowlight,newcolor,0.5+0.5*sin(uis.realtime/PULSE_DIVISOR));
		drawcolor = newcolor;
	}	
	else
		drawcolor = color;

	switch (style & UI_FORMATMASK)
	{
		case UI_CENTER:
			// center justify at x
			len = strlen(str);
			x   = x - len*charw/2;
			break;

		case UI_RIGHT:
			// right justify at x
			len = strlen(str);
			x   = x - len*charw;
			break;

		default:
			// left justify at x
			break;
	}

	if (style & UI_INVERSE)
	{
		len = strlen(str);
		UI_FillRect(x,y,len*charw,charh,drawcolor);
		drawcolor = g_color_table[ColorIndex(COLOR_YELLOW)];
	}

	if ((style & UI_DROPSHADOW))
	{
		dropcolor[0] = dropcolor[1] = dropcolor[2] = 0;
		dropcolor[3] = drawcolor[3];
		UI_DrawString2(x+2,y+2,str,dropcolor,charw,charh);
	}

	UI_DrawString2(x,y,str,drawcolor,charw,charh);

	if (style & UI_UNDERLINE)
	{
		len = strlen(str);
		UI_FillRect(x,y + charh,len*charw,2,drawcolor);
	}
}

/*
=================
UI_DrawChar
=================
*/
void UI_DrawChar( int x, int y, int ch, int style, vec4_t color )
{
	char	buff[2];

	buff[0] = ch;
	buff[1] = '\0';

	UI_DrawString( x, y, buff, style, color );
}

 void UI_GetActiveMenu( char **menuname, qboolean *fullscreen ) {
	if ( uis.activemenu && ( ui.Key_GetCatcher() & KEYCATCH_UI ) ) {
		if ( menuname ) {
			strcpy( *menuname, "unknown" );
		}
		*fullscreen = uis.activemenu->fullscreen;
		return;
	}

	if ( menuname ) {
		*menuname = NULL;
	}
	*fullscreen = qfalse;
}

static void NeedCDAction( qboolean result ) {
	if ( !result ) {
		ui.Cmd_ExecuteText( EXEC_APPEND, "quit\n" );
	}
}

 void UI_SetActiveMenu( const char* menuname,const char *menuID ) {
	// this should be the ONLY way the menu system is brought up (besides the UI_ConsoleCommand below)

	if (!ui.SG_GameAllowedToSaveHere(qtrue))	//don't check full sytem, only if incamera
	{
		return;
	}

	// enusure minumum menu data is cached
	Menu_Cache();

	if ( !menuname ) {
		UI_ForceMenuOff();
		return;
	}

	if ( Q_stricmp (menuname, "main") == 0 ) 
	{
		UI_MainMenu();
		return;
	}

	if ( Q_stricmp (menuname, "ingame") == 0 ) {
		ui.Cvar_Set( "cl_paused", "1" );
		UI_InGameMenu(menuID);
		return;
	}

	if ( Q_stricmp (menuname, "newgame") == 0 ) {
		UI_NewGameMenu();
		return;
	}

	if ( Q_stricmp (menuname, "genericholomenu") == 0 ) 
	{
		if ( Q_stricmp (menuID, "holodeck") == 0 ) 
		{
			ui.Cvar_Set( "cl_paused", "1" );
			UI_HolodeckMenu(1);
			return;
		}
	}

	if ( Q_stricmp (menuname, "endholomenu") == 0 ) 
	{
		ui.Cvar_Set( "cl_paused", "1" );
		UI_HolodeckMenu(2);
		return;
	}


	if ( Q_stricmp (menuname, "genericmenu") == 0 ) 
	{
		if ( Q_stricmp (menuID, "turbolift") == 0 ) 
		{
			ui.Cvar_Set( "cl_paused", "1" );
			UI_TurboliftMenu();
			return;
		}

		if ( Q_stricmp (menuID, "holodeck") == 0 ) 
		{
			ui.Cvar_Set( "cl_paused", "1" );
			UI_HolodeckMenu(0);
			return;
		}

		if ( Q_stricmp (menuID, "transporter") == 0 ) 
		{
			ui.Cvar_Set( "cl_paused", "1" );
			UI_TransporterMenu();	
			return;
		}

		if (!Q_strncmp(menuID,"padd",4))
		{
			ui.Cvar_Set( "cl_paused", "1" );
			UI_Padd2Menu((char *)menuID);
//			UI_PaddMenu((char *)menuID);
		}

		if (!Q_strncmp(menuID,"log",3))
		{
			ui.Cvar_Set( "cl_paused", "1" );
			UI_LogMenu((char *)menuID,0);
		}

		if ( Q_stricmp (menuID, "hazardgame") == 0 ) 
		{
			ui.Cvar_Set( "cl_paused", "1" );
			UI_DischlerGameMenu();	
			return;
		}

		if ( Q_stricmp (menuID, "tactical") == 0 ) 
		{
			ui.Cvar_Set( "cl_paused", "1" );
			UI_TacticalMenu();	
			return;
		}

		if ( Q_stricmp (menuID, "engineeringStatus") == 0 ) 
		{
			ui.Cvar_Set( "cl_paused", "1" );
			UI_EngineeringStatusMenu();	
			return;
		}

		if ( Q_stricmp (menuID, "navigation") == 0 ) 
		{
			ui.Cvar_Set( "cl_paused", "1" );
			UI_NavigationMenu();
			return;
		}

		if ( Q_stricmp (menuID, "astrometrics") == 0 ) 
		{
			ui.Cvar_Set( "cl_paused", "1" );
			UI_AccessingMenu(1);	// Show it's the astrometrics
			return;
		}

		if ( Q_stricmp (menuID, "library") == 0 ) 
		{
			ui.Cvar_Set( "cl_paused", "1" );
			UI_AccessingMenu(0);	// Show it's the library
			return;
		}

		if ( Q_stricmp (menuID, "diseaselibrary") == 0 ) 
		{
			ui.Cvar_Set( "cl_paused", "1" );
			UI_LogMenu("padd8",1);
			return;
		}

		if ( Q_stricmp (menuID, "shootingrange") == 0 ) 
		{
			ui.Cvar_Set( "cl_paused", "1" );
			UI_LogMenu("padd9",1);
			return;
		}

		if ( Q_stricmp (menuID, "weaponlibrary") == 0 ) 
		{
			ui.Cvar_Set( "cl_paused", "1" );
			UI_LogMenu("padd10",1);
			return;
		}

		if ( Q_stricmp (menuID, "cargo") == 0 ) 
		{
			ui.Cvar_Set( "cl_paused", "1" );
			UI_LogMenu("padd11",1);
			return;
		}

		if ( Q_stricmp (menuID, "engineeringdata") == 0 ) 
		{
			ui.Cvar_Set( "cl_paused", "1" );
			UI_LogMenu("padd12",1);
			return;
		}
	}

	if (Q_stricmp (menuname, "needcd") == 0 ) 
	{
		UI_ConfirmMenu( menu_normal_text[MNT_INSERTCD], NULL, NeedCDAction );
		return;
	}
}

/*
=================
UI_KeyEvent
=================
*/
 void UI_KeyEvent( int key ) {
	sfxHandle_t		s;

	if (!uis.activemenu) {
		return;
	}

	if (uis.activemenu->key)
		s = uis.activemenu->key( key );
	else
		s = Menu_DefaultKey( uis.activemenu, key );

	if ((s > 0) && (s != menu_null_sound))
		ui.S_StartLocalSound( s, CHAN_LOCAL_SOUND );
}

/*
=================
UI_MouseEvent
=================
*/
static void UI_MouseEvent( int dx, int dy )
{
	int				i;
	menucommon_s*	m;

	if (!uis.activemenu)
		return;

	// update mouse screen position
	uis.cursorx += dx;
	if (uis.cursorx < 0)
		uis.cursorx = 0;
	else if (uis.cursorx > SCREEN_WIDTH)
		uis.cursorx = SCREEN_WIDTH;

	uis.cursory += dy;
	if (uis.cursory < 0)
		uis.cursory = 0;
	else if (uis.cursory > SCREEN_HEIGHT)
		uis.cursory = SCREEN_HEIGHT;

	// region test the active menu items
	for (i=0; i<uis.activemenu->nitems; i++)
	{
		m = (menucommon_s*)uis.activemenu->items[i];

		if (m->flags & (QMF_GRAYED|QMF_HIDDEN|QMF_INACTIVE))
			continue;

		if ((uis.cursorx < m->left) ||
			(uis.cursorx > m->right) ||
			(uis.cursory < m->top) ||
			(uis.cursory > m->bottom))
		{
			// cursor out of item bounds
			continue;
		}

		// set focus to item at cursor
		if (uis.activemenu->cursor != i)
		{
			Menu_SetCursor( uis.activemenu, i );
			((menucommon_s*)(uis.activemenu->items[uis.activemenu->cursor_prev]))->flags &= ~QMF_HASMOUSEFOCUS;

//			if ( !(((menucommon_s*)(uis.activemenu->items[uis.activemenu->cursor]))->flags & QMF_SILENT ) ) 
			 ui.S_StartLocalSound( menu_move_sound, CHAN_LOCAL_SOUND);
		}

		((menucommon_s*)(uis.activemenu->items[uis.activemenu->cursor]))->flags |= QMF_HASMOUSEFOCUS;

		// menu item will handle internal focus
		Menu_Focus( (menucommon_s*)uis.activemenu->items[i] );

		return;
	}  

	if (uis.activemenu->nitems > 0) {
		// out of any region
		((menucommon_s*)(uis.activemenu->items[uis.activemenu->cursor]))->flags &= ~QMF_HASMOUSEFOCUS;
	}
}

 char *UI_Argv( int arg ) {
	static char	buffer[MAX_STRING_CHARS];

	ui.Argv( arg, buffer, sizeof( buffer ) );

	return buffer;
}


char *UI_Cvar_VariableString( const char *var_name ) {
	static char	buffer[MAX_STRING_CHARS];

	ui.Cvar_VariableStringBuffer( var_name, buffer, sizeof( buffer ) );

	return buffer;
}

/*
=================
UI_Cache
=================
*/
extern void CDKeyMenu_Cache(void);
extern void ControlsMouseJoyStick_Cache();
extern void MainMenu_Cache();
extern void QuitMenu_Cache();
extern void LCARSInMenu_Cache (void);
extern void LCARSOutMenu_Cache (void);
extern void UI_TurboliftMenu_Cache (void);
extern void UI_HolodeckMenu_Cache (void);
extern void UI_LogMenu_Cache(void);
extern void UI_TransporterMenu_Cache(void);
extern void UI_LibraryMenu_Cache(void);
extern void UI_DischlerGameMenu_Cache(void);
extern void UI_LogMenu_SpecialCache(void);
extern void UI_PaddMenu_SpecialCache(void);
extern void UI_Library_SpecialCache(void);
extern void UI_Astrometrics_SpecialCache(void);
extern void	UI_AccessingMenu_Cache(void);
extern void UI_VirtualSaveMenu_Cache(void);

static void UI_Cache_f( void ) {
	MenuFrame_Cache();
	CDKeyMenu_Cache();				//ui.cdkey

	LCARSInMenu_Cache();		// ui_menu
	LCARSOutMenu_Cache ();		// ui_menu

	MainMenu_Cache();			//ui_menu
	QuitMenu_Cache();			//ui_menu
	UI_PreferencesMenu_Cache();	// Preferences
	UI_LoadGameMenu_Cache();
	UI_ControlsMouseJoyStick_Cache();
	UI_CrewMenu_Cache();
	UI_SaveGameMenu_Cache(); 
	UI_FontsMenu_Cache();
	UI_NewGameMenu_Cache();
	Menu_Cache();
	UI_SoundMenu_Cache();
	UI_VideoDataMenu_Cache();
	UI_Video2Menu_Cache();
	UI_VideoDriverMenu_Cache();
	UI_LeaveGameMenu_Cache();
	UI_WeaponsMenu_Cache();
	UI_RavenMenu_Cache();
	UI_InGameMenu_Cache();
	UI_ControlsOther_Cache();
	UI_CreditsMenu_Cache();
	UI_ClosingCreditsMenu_Cache();
//	UI_DemoEndMenu_Cache();
	UI_TurboliftMenu_Cache();
	UI_HolodeckMenu_Cache();
	UI_LogMenu_Cache();
	UI_TransporterMenu_Cache();
	UI_LibraryMenu_Cache();
	UI_DischlerGameMenu_Cache();

	UI_LogMenu_SpecialCache();
	UI_PaddMenu_SpecialCache();
	UI_Library_SpecialCache();
	UI_Astrometrics_SpecialCache();
	UI_AccessingMenu_Cache();
	UI_VirtualSaveMenu_Cache();
}


/*
=================
UI_ConsoleCommand
=================
*/
static qboolean UI_ConsoleCommand( void ) {
	char	*cmd;
	int		fromMenu;

	if (!ui.SG_GameAllowedToSaveHere(qtrue))	//only check if incamera
	{
		return qfalse;
	}

	cmd = UI_Argv( 0 );

	// ensure minimum menu data is available
	Menu_Cache();

	if ( Q_stricmp (cmd, "ui_cache") == 0 ) {
		UI_Cache_f();
		return qtrue;
	}

	if ( Q_stricmp (cmd, "levelselect") == 0 ) {
		UI_LoadMenu_f();
		return qtrue;
	}
	
	if ( Q_stricmp (cmd, "ui_teamOrders") == 0 ) {
		UI_SaveMenu_f();
		return qtrue;
	}

	if ( Q_stricmp (cmd, "ui_cdkey") == 0 ) {
//		UI_CDKeyMenu_f();
		return qtrue;
	}

	if ( Q_stricmp (cmd, "ui_closingcredits") == 0 ) {
		fromMenu = atoi(UI_Argv( 1 ));
		UI_ClosingCreditsMenu(fromMenu);
		return qtrue;
	}

	// Interfaces activated from the maps
	// This is just for texting purposes and can be commented out eventually
	if ( Q_stricmp (cmd, "ui_astrometrics") == 0 ) {
		UI_AccessingMenu(1);
		return qtrue;
	}

	if ( Q_stricmp (cmd, "ui_turbolift") == 0 ) {
		UI_TurboliftMenu();
		return qtrue;
	}

	if ( Q_stricmp (cmd, "ui_holodeck") == 0 ) {
		UI_HolodeckMenu(0);
		return qtrue;
	}

	if ( Q_stricmp (cmd, "ui_log") == 0 ) {
		UI_LogMenu(UI_Argv( 1 ),0);
		return qtrue;
	}

	if ( Q_stricmp (cmd, "ui_padd") == 0 ) {
//		UI_PaddMenu(UI_Argv( 1 ));
		UI_Padd2Menu(UI_Argv( 1 ));
		return qtrue;
	}

	if ( Q_stricmp (cmd, "ui_navigation") == 0 ) {
		UI_NavigationMenu();
		return qtrue;
	}

	if ( Q_stricmp (cmd, "ui_transporter") == 0 ) {
		UI_TransporterMenu();
		return qtrue;
	}

	if ( Q_stricmp (cmd, "ui_library") == 0 ) {
		UI_AccessingMenu(0);
		return qtrue;
	}

	if ( Q_stricmp (cmd, "ui_hazardgame") == 0 ) {
		UI_DischlerGameMenu();
		return qtrue;
	}

	if ( Q_stricmp (cmd, "ui_tactical") == 0 ) {
		UI_TacticalMenu();
		return qtrue;
	}

	if ( Q_stricmp (cmd, "ui_engineeringstatus") == 0 ) {
		UI_EngineeringStatusMenu();
		return qtrue;
	}

	if ( Q_stricmp (cmd, "ui_ops") == 0 ) {
		UI_OpsMenu();
		return qtrue;
	}

	if ( Q_stricmp (cmd, "ui_diseases") == 0 ) {
		UI_LogMenu("padd8",1);
		return qtrue;
	}

	if ( Q_stricmp (cmd, "ui_shootingrange") == 0 ) {
		UI_LogMenu("padd9",1);
		return qtrue;
	}

	if ( Q_stricmp (cmd, "ui_weaponlibrary") == 0 ) {
		UI_LogMenu("padd10",1);
		return qtrue;
	}

	if ( Q_stricmp (cmd, "ui_cargo") == 0 ) {
		UI_LogMenu("padd11",1);
		return qtrue;
	}

	if ( Q_stricmp (cmd, "ui_engineeringdata") == 0 ) {
		UI_LogMenu("padd12",1);
		return qtrue;
	}

	// EFSP fix (Virtual Voyager USE): route the in-game LCARS panel/console commands to
	// UI_SetActiveMenu, where the turbolift/holodeck/log/transporter/padd/... handlers live.
	// target_interface_use (g_target.cpp) sends "genericmenu <id>"; the holodeck flow sends
	// genericholomenu/endholomenu. RETAIL registered these as ENGINE commands (stvoy:
	// genericmenu->retail, endholodeckmenu->retail, Cmd_AddCommand) that forwarded to
	// the UI. The Quake3e-derived port engine never registered them, and UI_ConsoleCommand only
	// handled the "ui_*" verbs -> "genericmenu turbolift"/"genericmenu log1" fell through every
	// command handler and was dropped, so the turbolift + personal-log + all LCARS menus never
	// opened in Virtual Voyager (USE fired fine; the menu command was simply unrouted).
	if ( Q_stricmp (cmd, "genericmenu") == 0
	  || Q_stricmp (cmd, "genericholomenu") == 0
	  || Q_stricmp (cmd, "endholomenu") == 0 )
	{
		// UI_Argv() returns a SHARED static buffer, so calling UI_Argv(1) clobbers what `cmd`
		// (UI_Argv(0)) points to. Copy BOTH the menu name and id out to locals before dispatch,
		// or UI_SetActiveMenu would receive the id as the name ("log1","log1") and match nothing.
		char menuName[64], menuID[64];
		Q_strncpyz( menuName, UI_Argv(0), sizeof(menuName) );
		Q_strncpyz( menuID,   UI_Argv(1), sizeof(menuID) );
		UI_SetActiveMenu( menuName, menuID );
		return qtrue;
	}

	return qfalse;
}


static void UI_Shutdown( void ) {
}


/*
=================
UI_Init
=================
*/
static void UI_Init( int apiVersion, uiimport_t *uiimport ) {
	gameinfo_import_t	gameinfo_import;

	ui = *uiimport;

	if ( apiVersion != UI_API_VERSION ) {
		ui.Error( ERR_FATAL, "Bad UI_API_VERSION: expected %i, got %i\n", UI_API_VERSION, apiVersion );
	}

	UI_LoadButtonText();

	UI_LoadMenuText();

	UI_LoadFonts();

	// get static data (glconfig, media)
	ui.GetGlconfig( &uis.glconfig );

	// EFSP 4:3: ensure cg_forceAspect exists engine-side at boot (CVAR_ARCHIVE -> persists across runs and
	// is loaded before any menu draw/input). The SAME engine cvar is read by UI_RecalcAspect (draw side)
	// AND by the engine's CL_GetUIScreenPlacement (input/IME side), so the two never disagree. The cgame
	// re-registers it (with a vmCvar binding) at CG_INIT; Cvar_Create preserves the existing value.
	ui.Cvar_Create( "cg_forceAspect", "0", CVAR_ARCHIVE );

	UI_RecalcAspect();		// sets uis.scalex/scaley/bias from glconfig + cg_forceAspect (replaces the old static stretch)
	gameinfo_import.FS_FOpenFile = ui.FS_FOpenFile;
	gameinfo_import.FS_Read = ui.FS_Read;
	gameinfo_import.FS_ReadFile = ui.FS_ReadFile;
	gameinfo_import.FS_FreeFile = ui.FS_FreeFile;
	gameinfo_import.FS_FCloseFile = ui.FS_FCloseFile;
	gameinfo_import.Cvar_Set = ui.Cvar_Set;
	gameinfo_import.Cvar_VariableStringBuffer = ui.Cvar_VariableStringBuffer;
	gameinfo_import.Cvar_Create = ui.Cvar_Create;
	gameinfo_import.Printf = ui.Printf;
	GI_Init( &gameinfo_import );

	Menu_Cache( );

	ui.Cvar_Create( "cg_drawCrosshair", "1", CVAR_ARCHIVE );
	ui.Cvar_Create( "cg_drawCrosshairNames", "1", CVAR_ARCHIVE );
	ui.Cvar_Create( "cg_marks", "1", CVAR_ARCHIVE );
	ui.Cvar_Create ("ui_initialsetup", "0", CVAR_ARCHIVE );
	ui.Cvar_Create ("g_language", "", CVAR_ARCHIVE | CVAR_NORESTART);
	ui.Cvar_Create ("s_language", "", CVAR_ARCHIVE | CVAR_NORESTART);
	ui.Cvar_Create ("k_language", "", CVAR_ARCHIVE | CVAR_NORESTART);


}

uiexport_t *GetUIAPI( void ) {
	static uiexport_t	uiexport;

	memset( &uiexport, 0, sizeof( uiexport ) );

	uiexport.UI_Init = UI_Init;
	uiexport.UI_Shutdown = UI_Shutdown;
	uiexport.UI_KeyEvent = UI_KeyEvent;
	uiexport.UI_MouseEvent = UI_MouseEvent;
	uiexport.UI_Refresh = UI_Refresh;
	uiexport.UI_GetActiveMenu = UI_GetActiveMenu;
	uiexport.UI_SetActiveMenu = UI_SetActiveMenu;
	uiexport.UI_ConsoleCommand = UI_ConsoleCommand;
	uiexport.UI_DrawConnect = UI_DrawConnect;
	uiexport.UI_DrawConnectText = UI_DrawConnectText;
	uiexport.UI_UpdateConnectionString = UI_UpdateConnectionString;
	uiexport.UI_UpdateConnectionMessageString = UI_UpdateConnectionMessageString;

	return &uiexport;
}

// these are only here so the functions in q_shared.c can link

#ifndef UI_HARD_LINKED

void Com_Error( int level, const char *error, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, error);
	vsprintf (text, error, argptr);
	va_end (argptr);

	ui.Error( level, "%s", text);
}

void Com_Printf( const char *msg, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	vsprintf (text, msg, argptr);
	va_end (argptr);

	ui.Printf( "%s", text);
}

#endif

/*
================
UI_RecalcAspect

EFSP 4:3 (cg_forceAspect, the video-menu Aspect toggle): recompute the 2D menu scale + center bias.
When 4:3 is requested on a wider-than-4:3 screen, pillarbox the WHOLE menu into a centered 4:3 box with a
UNIFORM scale (the Y scale, so art isn't anamorphically squeezed) plus the wide-screen centering bias
0.5*(vidW - vidH*640/480) — IDENTICAL math to the in-game HUD fix (CG_AdjustFrom640, cg_drawtools.cpp) and
to the engine's SCR_HOR_CENTER placement (cls.screenXScale/screenXBias, cl_main.c CL_WindowResized). That
exact-match is what keeps the menu DRAW (here, via uis.bias/scalex) consistent with the engine's screen->640
INPUT mapping (CL_GetUIScreenPlacement returns SCR_HOR_CENTER when cg_forceAspect is set, so SCR_AdjustTo640
inverts this transform for touch/mouse hit-testing AND the soft-keyboard IME rect). When the toggle is off,
bias=0 and scalex=vidW/640 -> byte-identical to the original full-width stretch (the "16:9 FILL" default).
Resolution-agnostic (height-derived). Call from UI_Init and from the video-menu ModeCallback (live toggle).
================
*/
void UI_RecalcAspect( void ) {
	uis.scaley = uis.glconfig.vidHeight * (1.0/480.0);

	if ( ui.Cvar_VariableValue( "cg_forceAspect" )
		&& uis.glconfig.vidWidth * 480 > uis.glconfig.vidHeight * 640 ) {
		uis.scalex = uis.scaley;	// uniform (no anamorphic stretch)
		uis.bias   = 0.5 * ( uis.glconfig.vidWidth - ( uis.glconfig.vidHeight * (640.0/480.0) ) );
	} else {
		uis.scalex = uis.glconfig.vidWidth * (1.0/640.0);
		uis.bias   = 0;
	}
}

/*
================
UI_AdjustFrom640

Adjusted for resolution and screen aspect ratio
================
*/
void UI_AdjustFrom640( float *x, float *y, float *w, float *h ) {
	// expect valid pointers
	*x = *x * uis.scalex + uis.bias;	// EFSP 4:3: +uis.bias centers the 4:3 box (bias=0 in 16:9 -> pure stretch)
	*y *= uis.scaley;
	*w *= uis.scalex;
	*h *= uis.scaley;
}


void UI_DrawNamedPic( float x, float y, float width, float height, const char *picname ) {
	qhandle_t	hShader;

	hShader = ui.R_RegisterShaderNoMip( picname );
	UI_AdjustFrom640( &x, &y, &width, &height );
	ui.R_DrawStretchPic( x, y, width, height, 0, 0, 1, 1, hShader );
}

 void UI_DrawPic( float x, float y, float w, float h, char *pic ) 
{
	UI_DrawNamedPic (x, y, w, h, va("gfx/2d/%s", pic ) );
}

void UI_DrawHandlePic( float x, float y, float w, float h, qhandle_t hShader ) {
	float	s0;
	float	s1;
	float	t0;
	float	t1;

	if( w < 0 ) {	// flip about horizontal
		w  = -w;
		s0 = 1;
		s1 = 0;
	}
	else {
		s0 = 0;
		s1 = 1;
	}

	if( h < 0 ) {	// flip about vertical
		h  = -h;
		t0 = 1;
		t1 = 0;
	}
	else {
		t0 = 0;
		t1 = 1;
	}

	UI_AdjustFrom640( &x, &y, &w, &h );
	ui.R_DrawStretchPic( x, y, w, h, s0, t0, s1, t1, hShader );
}

/*
================
UI_FillRect

Coordinates are 640*480 virtual values
=================
*/
void UI_FillRect( float x, float y, float width, float height, const float *color ) {
	ui.R_SetColor( color );

	UI_AdjustFrom640( &x, &y, &width, &height );
	ui.R_DrawStretchPic( x, y, width, height, 0, 0, 0, 0, uis.whiteShader );

	ui.R_SetColor( NULL );
}

/*
================
UI_DrawRect

Coordinates are 640*480 virtual values
=================
*/
void UI_DrawRect( float x, float y, float width, float height, const float *color ) {
	ui.R_SetColor( color );

	UI_AdjustFrom640( &x, &y, &width, &height );

	ui.R_DrawStretchPic( x, y, width, 1, 0, 0, 0, 0, uis.whiteShader );
	ui.R_DrawStretchPic( x, y, 1, height, 0, 0, 0, 0, uis.whiteShader );
	ui.R_DrawStretchPic( x, y + height - 1, width, 1, 0, 0, 0, 0, uis.whiteShader );
	ui.R_DrawStretchPic( x + width - 1, y, 1, height, 0, 0, 0, 0, uis.whiteShader );

	ui.R_SetColor( NULL );
}

 void UI_SetColor( const float *rgba ) {
	ui.R_SetColor( rgba );
}

void UI_UpdateScreen( void ) {
	ui.UpdateScreen();
}

/*
=================
UI_Refresh
=================
*/
 void UI_Refresh( int realtime )
{
	vec4_t color;
	uis.frametime = realtime - uis.realtime;
	uis.realtime  = realtime;

	if ( !( ui.Key_GetCatcher() & KEYCATCH_UI ) ) {
		return;
	}

	// EFSP 4:3 (cg_forceAspect): recompute the menu's scale+bias EVERY frame from the LIVE cvar, exactly like
	// the cgame re-reads cg_forceAspect every frame (CG_UpdateCvars). Doing it only at UI_Init/ModeCallback
	// left the menu with a STALE aspect when cg_forceAspect settled after UI_Init (config-exec timing) or
	// changed elsewhere -> "main menu 16:9 but the 3D game 4:3". Per-frame recalc keeps the menu, the 3D/HUD,
	// and the engine input mapping (CL_GetUIScreenPlacement, also live) all in lock-step. Cheap (a few mults).
	UI_RecalcAspect();

	if ( uis.activemenu )
	{
		// EFSP 4:3 (cg_forceAspect): when the menu is pillarboxed into a centered 4:3 box (uis.bias>0), the
		// side columns [0,bias] and [vidW-bias,vidW] fall OUTSIDE that box and would show the uncleared
		// framebuffer (stale garbage / the "green border"). Paint them black in REAL screen coords (NOT via
		// UI_AdjustFrom640 — these are physical margins, not 640-virtual). Drawn before the menu; all menu
		// content lives inside the box so there's no overlap. Mirrors the HUD's CG_Draw2D bars (cg_draw.cpp).
		if ( uis.bias > 0 )
		{
			ui.R_SetColor( colorTable[CT_BLACK] );
			ui.R_DrawStretchPic( 0, 0, uis.bias, uis.glconfig.vidHeight, 0, 0, 0, 0, uis.whiteShader );
			ui.R_DrawStretchPic( uis.glconfig.vidWidth - uis.bias, 0, uis.bias, uis.glconfig.vidHeight, 0, 0, 0, 0, uis.whiteShader );
			ui.R_SetColor( NULL );
		}

		if (uis.activemenu->fullscreen)
		{
			// draw the background
			ui.R_SetColor( colorTable[CT_BLACK]);
			UI_DrawHandlePic(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, uis.whiteShader );
		}
		else
		{
			// draw the background
			color[0] = colorTable[CT_BLACK][0];
			color[1] = colorTable[CT_BLACK][1];
			color[2] = colorTable[CT_BLACK][1];
			color[3] = .75;

			ui.R_SetColor( color);
			UI_DrawHandlePic(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, uis.whiteShader );
		}

		// Android soft keyboard: reset the field-focus flag each frame; a focused
		// MenuField_Draw re-sets it below if one is on-screen this frame. The
		// engine (sdl_input.c IN_Frame) pops the IME when ui_textfield is set.
		ui.Cvar_Set( "ui_textfield", "0" );

		if ((uis.activemenu->openingStart) && (uis.activemenu->opening))
		{
			uis.activemenu->opening();
		}
		else if ((uis.activemenu->closingStart) && (uis.activemenu->closing))
		{
			uis.activemenu->closing();
		}
		else if (uis.activemenu->draw)
			uis.activemenu->draw();
		else
			Menu_Draw( uis.activemenu );	//jfm merged - check it

		// draw the version number
		if (!uis.noversion)
		{
			UI_DrawProportionalString(  371, 445, Q3_VERSION,UI_TINYFONT, colorTable[CT_BLACK]);
		}

		if( uis.firstdraw ) {
			UI_MouseEvent( 0, 0 );
			uis.firstdraw = qfalse;
		}

	}

	// draw cursor
	ui.R_SetColor( NULL );
	if (uis.cursorDraw)
	{
		UI_DrawHandlePic(uis.cursorx,uis.cursory,16,16, uis.cursor);
	}

#ifndef NDEBUG
	if (uis.debug)
	{
		// cursor coordinates
		UI_DrawString( 0, 0, va("(%d,%d)",uis.cursorx,uis.cursory), UI_LEFT|UI_SMALLFONT, colorTable[CT_RED] );
	}
#endif

	// delay playing the enter sound until after the
	// menu has been drawn, to avoid delay while
	// caching images
	if (m_entersound)
	{
		ui.S_StartLocalSound( menu_in_sound, CHAN_LOCAL_SOUND );
		m_entersound = qfalse;
	}
}

void UI_DrawTextBox (int x, int y, int width, int lines)
{
	UI_FillRect( x + BIGCHAR_WIDTH/2, y + BIGCHAR_HEIGHT/2, ( width + 1 ) * BIGCHAR_WIDTH, ( lines + 1 ) * BIGCHAR_HEIGHT, colorTable[CT_BLACK]);
	UI_DrawRect( x + BIGCHAR_WIDTH/2, y + BIGCHAR_HEIGHT/2, ( width + 1 ) * BIGCHAR_WIDTH, ( lines + 1 ) * BIGCHAR_HEIGHT, colorTable[CT_WHITE] );
}

qboolean UI_CursorInRect (int x, int y, int width, int height)
{
	if (uis.cursorx < x ||
		uis.cursory < y ||
		uis.cursorx > x+width ||
		uis.cursory > y+height)
		return qfalse;

	return qtrue;
}

/*
==============
UI_DrawNumField

Take x,y positions as if 640 x 480 and scales them to the proper resolution

==============
*/
 void UI_DrawNumField (int x, int y, int width, int value,int charWidth,int charHeight)
{
	char	num[16], *ptr;
	int		l;
	int		frame;
	int		xWidth;

	if (width < 1)
		return;

	// draw number string
	if (width > 15)
		width = 15;

	switch ( width ) 
	{
	case 1:
		value = value > 9 ? 9 : value;
		value = value < 0 ? 0 : value;
		break;
	case 2:
		value = value > 99 ? 99 : value;
		value = value < -9 ? -9 : value;
		break;
	case 3:
		value = value > 999 ? 999 : value;
		value = value < -99 ? -99 : value;
		break;
	case 4:
		value = value > 9999 ? 9999 : value;
		value = value < -999 ? -999 : value;
		break;
	}

	Com_sprintf (num, sizeof(num), "%i", value);
	l = strlen(num);
	if (l > width)
		l = width;

	xWidth = (charWidth/3);

	x += (xWidth)*(width - l);

	ptr = num;
	while (*ptr && l)
	{
//if (*ptr == '-')
//	frame = STAT_MINUS;
//else
			frame = *ptr -'0';

		UI_DrawHandlePic( x,y, 16, 16, uis.smallNumbers[frame] );

		x += (xWidth);
		ptr++;
		l--;
	}

}

static float fadePercent = 0.0;

/*
==============
UI_PrintMenuGraphics
==============
*/
void UI_PrintMenuGraphics(menugraphics_s *menuGraphics,int maxI)
{
	int i;
	char *text;
	vec4_t	drawcolor;

	// Now that all the changes are made, print up the graphics
	for (i=0;i<maxI;++i)
	{
		if (menuGraphics[i].type == MG_GRAPHIC)
		{	
			ui.R_SetColor( colorTable[menuGraphics[i].color]);
			UI_DrawHandlePic( menuGraphics[i].x, 
				menuGraphics[i].y,
				menuGraphics[i].width,
				menuGraphics[i].height,
				menuGraphics[i].graphic);
			ui.R_SetColor( colorTable[CT_NONE]);
		}
		else if (menuGraphics[i].type == MG_STRING)
		{
			if (menuGraphics[i].file)
			{
				text = menuGraphics[i].file;
			}
			else if (menuGraphics[i].normaltextEnum)
			{
				text = menu_normal_text[menuGraphics[i].normaltextEnum];
			}
			else if (menuGraphics[i].normaltextEnum)
			{
				text = menu_button_text[menuGraphics[i].buttontextEnum][0];
			}

			if (!fadePercent)
			{
				UI_DrawProportionalString( menuGraphics[i].x, 
					menuGraphics[i].y, 
					text,
					menuGraphics[i].style,
					colorTable[menuGraphics[i].color]);
			}
			else	// Fading in and out
			{
				drawcolor[0] = colorTable[menuGraphics[i].color][0];
				drawcolor[1] = colorTable[menuGraphics[i].color][1];
				drawcolor[2] = colorTable[menuGraphics[i].color][2];
				drawcolor[3] = fadePercent;

				UI_DrawProportionalString( menuGraphics[i].x, 
					menuGraphics[i].y, 
					text,
					menuGraphics[i].style,
					drawcolor);
			}
		}
		else if (menuGraphics[i].type == MG_FADE)
		{
			fadePercent= menuGraphics[i].timer;
		}
		else if (menuGraphics[i].type == MG_NUMBER)
		{
			ui.R_SetColor( colorTable[menuGraphics[i].color]);
			UI_DrawNumField (menuGraphics[i].x,
				menuGraphics[i].y, 
				menuGraphics[i].max, 
				menuGraphics[i].target,
				menuGraphics[i].width,	
				menuGraphics[i].height);
			ui.R_SetColor( colorTable[CT_NONE]);
		}
		else if (menuGraphics[i].type == MG_NONE)
		{
			;	// Don't print anything
		}
	}
}

/*
==============
UI_RandomNumbers

Generate random numbers based on the width passed
==============
*/
int UI_RandomNumbers(int max)
{
	int number;

	// How big a random number????
	switch (max)
	{
	case 0 :
		number = 0;
		break;
	case 1 :
		number = number = (random() * 9);
		break;
	case 2 :
		number = (random() * (90)) + 9;
		break;
	case 3 :
//number = (random() * (900)) + 99;
		number = (random() * (900)) + 9;
		break;
	case 4 :
		number = (random() * (9000)) + 99;
		break;
	case 5 :
		number = (random() * (90000)) + 999;
		break;
	case 6 :
		number = (random() * (900000)) + 9999;
		break;
	case 7 :
		number = (random() * (9000000)) + 99999;
		break;
	case 8 :
		number = (random() * (90000000)) + 999999;
		break;
	case 9 :
		number = (random() * (900000000)) + 9999999;
		break;
	default :
		number = (random() * (900000000)) + 9999999;
		break;
	}

	return(number);
}

/*
==============
UI_PrecacheMenuGraphics
==============
*/
void UI_PrecacheMenuGraphics(menugraphics_s *menuGraphics,int maxI)
{
	int i;

	for (i=0;i<maxI;++i)
	{
		if (menuGraphics[i].type == MG_GRAPHIC)
		{
			menuGraphics[i].graphic = ui.R_RegisterShaderNoMip(menuGraphics[i].file);
		}
	}

}

#define MAXBUTTONTEXT 14000
char	ButtonText[MAXBUTTONTEXT];

/*
=================
UI_ParseButtonText
=================
*/
static void UI_ParseButtonText()
{
	char	*token;
	char *buffer;
	int i;
	int len;

	COM_BeginParseSession();

	buffer = ButtonText;
	i = 1;	// Zero is null string
	while ( buffer ) 
	{
		G_ParseString( &buffer, &token);

		len = strlen(token);
		if (len)
		{
			if (i == MBT_MAX)
			{
				Com_Printf( S_COLOR_RED "UI_ParseButtonText : too many values!\n");
				return;
			}
			if ((len==1) && (*token == '/'))	// Is this a null field??
			{
				menu_button_text[i][0] = NULL;
			}
			else
			{
				menu_button_text[i][0] = (buffer - (len + 1));	// The +1 is to get rid of the " at the beginning of the sting.
			}

			*(buffer - 1) = '\0';		//	Place a string end where is belongs.

			G_ParseString( &buffer, &token);
			len = strlen(token);
			if (len)
			{
				if ((len==1) && (*token == '/'))	// Is this a null field??
				{
					menu_button_text[i][1] = NULL;
				}
				else
				{
					menu_button_text[i][1] = (buffer - (len+1));	// The +1 is to get rid of the " at the beginning of the sting.
				}
				*(buffer-1) = '\0';		//	Place a string end where is belongs.
			}
			i++;
		}

	}
	if (i != MBT_MAX)
	{
		// fewer pairs than the enum: back-fill silently (the only shortfall is the port-appended
		// MBT_GAMEPADLOOK, which is assigned in C at ui_controls2.cpp; was a red console error).
		for(;i<MBT_MAX;i++) {
			menu_button_text[i][0] = "?";
			menu_button_text[i][1] = "?";
		}
	}
}

/*
=================
UI_LoadButtonText
=================
*/
void UI_LoadButtonText(void)
{
	char	*buffer;
	char	filename[MAX_QPATH];
	int		len;


	UI_LanguageFilename("ext_data/sp_buttontext","dat",filename);

	len = ui.FS_ReadFile( filename,(void **) &buffer  );

	if ( len == -1 ) 
	{
		ui.Error(ERR_FATAL, "UI_LoadButtonText : SP_BUTTONTEXT.DAT file not found!\n");
		return;
	}

	if ( len > MAXBUTTONTEXT ) 
	{
		ui.Error(ERR_FATAL, "UI_LoadButtonText : SP_BUTTONTEXT.DAT too big!\n");
		return;
	}

	strncpy( ButtonText, buffer, sizeof( ButtonText ) - 1 );
	ui.FS_FreeFile( buffer );

	UI_ParseButtonText();
}

#define MAXMENUTEXT 55000
char	MenuText[MAXMENUTEXT];

/*
=================
UI_ParseMenuText
=================
*/
static void UI_ParseMenuText()
{
	char	*token;
	char *buffer;
	int i;
	char *holdBuffer;
	int len;

	COM_BeginParseSession();

	buffer = MenuText;
	i = 1;	// Zero is null string
	while ( buffer ) 
	{
		holdBuffer = buffer;
		G_ParseString( &buffer, &token);

		len = strlen(token);
		if (len)
		{
			if (i == MNT_MAX)
			{
				// sp_normaltext.dat ships more strings than MNT_MAX (true in retail too); benign --
				// every slot the UI references is already filled. Stop silently (was a S_COLOR_RED
				// "too many lines!" console error that looked like a real parse failure to users).
				return;
			}
			menu_normal_text[i] = (holdBuffer + 1);	// The +1 is to get rid of the " at the beginning of the sting.
			*(holdBuffer+(len) + 1) = '\0';		//	Place an string end where is belongs.
			i++;
		}

	}
	if (i != MNT_MAX)
	{
		// fewer strings than the enum: back-fill the rest with "?" (silent; was a red console error).
		for(;i<MNT_MAX;i++) {
			menu_normal_text[i] = "?";
		}
	}
}

/*
=================
UI_LoadMenuText
=================
*/
void UI_LoadMenuText(void)
{
	char	*buffer;
	char	filename[MAX_QPATH];
	int		len;

	UI_LanguageFilename("ext_data/sp_normaltext","dat",filename);

	len = ui.FS_ReadFile( filename,(void **) &buffer  );

	if ( len == -1 ) 
	{
		Com_Error( ERR_FATAL, "UI_LoadMenuText : SP_NORMALTEXT.DAT file not found!\n");
		return;
	}

	if ( len > MAXMENUTEXT ) 
	{
		Com_Error( ERR_FATAL, "UI_LoadMenuText : SP_NORMALTEXT.DAT size (%d) > max (%d)!\n", len, MAXMENUTEXT );
		return;
	}

	strncpy( MenuText, buffer, sizeof( MenuText ) - 1 );
	ui.FS_FreeFile( buffer );

	UI_ParseMenuText();
}

static char *UI_ParseFontParms(char *buffer,int	propArray[CHARMAX][3])
{
	char	*token;
	int		i,i2;
	
	while ( buffer ) 
	{
		token = COM_ParseExt( &buffer, qtrue );

		// Start with open bracket
		if ( !Q_stricmp( token, "{" ) ) 
		{
			// Load all characters for this size font
			for (i=0;i<CHARMAX;++i)
			{
				// X pos, Y pos, and length are enclosed in {}
				token = COM_ParseExt( &buffer, qtrue );
				if ( !Q_stricmp( token, "{" ) ) 
				{
					;
				}
				else
				{
					Com_Printf( S_COLOR_RED "UI_ParseFontParms : Need { near character %d!\n",i);
					return(NULL);
				}

				// X pos, Y pos, length
				for (i2=0;i2<3;++i2)
				{	
					token = COM_ParseExt( &buffer, qtrue );
					propArray[i][i2] = atoi(token);
				}

				// Closing bracket
				token = COM_ParseExt( &buffer, qtrue );
				if ( !Q_stricmp( token, "}" ) ) 
				{
					;
				}
				else
				{
					Com_Printf(S_COLOR_RED "UI_ParseFontParms : Need }, near character %d!\n",i);
					return(NULL);
				}
			}
		}

		token = COM_ParseExt( &buffer, qtrue );	// Grab closing bracket
		if ( !Q_stricmp( token, "}" ) ) 
		{
			break;
		}
	}

	return(buffer);
}

#define FONT_BUFF_LENGTH 20000

/*
=================
UI_LoadFonts
=================
*/
 void UI_LoadFonts( void ) 
{
	char buffer[FONT_BUFF_LENGTH];
	int len;
	fileHandle_t	f;
	char *holdBuf;

	len = ui.FS_FOpenFile( "ext_data/fonts.dat", &f, FS_READ );

	if ( !f ) 
	{
		Com_Printf( S_COLOR_RED "UI_LoadFonts : FONTS.DAT file not found!\n");
		return;
	}

	if (len > FONT_BUFF_LENGTH)
	{
		Com_Printf(S_COLOR_RED "UI_LoadFonts : FONTS.DAT file bigger than %d!\n",FONT_BUFF_LENGTH);
		return;
	}

	// initialise the data area
	memset(buffer, 0, sizeof(buffer));	

	ui.FS_Read( buffer, len, f );

	ui.FS_FCloseFile( f );

	COM_BeginParseSession();

	holdBuf = (char *) buffer;
	holdBuf = UI_ParseFontParms( holdBuf,propMapTiny);
	holdBuf = UI_ParseFontParms( holdBuf,propMap);
	holdBuf = UI_ParseFontParms( holdBuf,propMapBig);
}




/*
===============
UI_FrameTop_Graphics
===============
*/
static void UI_FrameTop_Graphics(menuframework_s *menu)
{
	if (!ingameFlag)
	{
		menu->fullscreen			= qtrue;
	}
	else	// In game menu
	{
		menu->fullscreen			= qfalse;
	}

	ui.R_SetColor( colorTable[CT_DKPURPLE2] );
	UI_DrawHandlePic(  30,  24,  47,  54, uis.whiteShader);	// Top left hand column

	ui.R_SetColor( colorTable[CT_DKPURPLE3]);
	UI_DrawHandlePic(  30,  81,  47,  34, uis.whiteShader);	// Middle left hand column
	UI_DrawHandlePic(  30,  115,  128,  64, s_menuframe.cornerUpper);	// Corner
	UI_DrawHandlePic(  109, 136,  40,  7, uis.whiteShader);	// Start of line across bottom of top third section


	ui.R_SetColor( colorTable[CT_LTBROWN1]);
	UI_DrawHandlePic( 152, 136, 135,   7, uis.whiteShader);	// 2nd line across bottom of top third section

	ui.R_SetColor( colorTable[CT_DKPURPLE2]);
	UI_DrawHandlePic( 290, 136,  12,   7, uis.whiteShader);	// 3rd line across bottom of top third section
	UI_DrawHandlePic( 305, 139,  60,   4, uis.whiteShader);	// 4th line across bottom of top third section

	ui.R_SetColor( colorTable[CT_LTBROWN1]);
	UI_DrawHandlePic( 368, 136,  111,   7, uis.whiteShader);	// 5th line across bottom of top third section

	if (menu->titleI)
	{
		UI_DrawProportionalString( menu->titleX, menu->titleY ,menu_normal_text[menu->titleI],
			 UI_RIGHT|UI_BIGFONT, colorTable[CT_LTORANGE]);
	}
}

/*
===============
UI_FrameBottom_Graphics
===============
*/
static void UI_FrameBottom_Graphics(void)
{
	ui.R_SetColor( colorTable[CT_DKBROWN1]);
	UI_DrawHandlePic(  30, 147, 128,  64, s_menuframe.cornerUpper2);	// Top corner
	UI_DrawHandlePic(  50, 147,  99,   7, uis.whiteShader);
	UI_DrawHandlePic( 152, 147, 135,   7, uis.whiteShader);

	ui.R_SetColor( colorTable[CT_DKBROWN1]);
	UI_DrawHandlePic( 290, 147,  12,   7, uis.whiteShader);

	ui.R_SetColor( colorTable[CT_LTBROWN1]);
	UI_DrawHandlePic( 305, 147,  60,   4, uis.whiteShader);

	ui.R_SetColor( colorTable[CT_DKBROWN1]);
	UI_DrawHandlePic( 368, 147, 111,   7, uis.whiteShader);

	ui.R_SetColor( colorTable[CT_DKBROWN1]);
	UI_DrawHandlePic(  30, 173,  47,  27, uis.whiteShader);	// Top left column
	UI_DrawHandlePic(  30, 392,  47,  33, uis.whiteShader);	// Bottom left column
	UI_DrawHandlePic(  30, 425, 128,  64, s_menuframe.cornerLower);// Bottom Left Corner

	ui.R_SetColor( colorTable[CT_LTBROWN1]);
	UI_DrawHandlePic( 96,  438, 268,  18, uis.whiteShader);	// Bottom front Line

	ui.R_SetColor(NULL);
}

/*
=================
UI_MenuBottomLineEnd_Graphics
=================
*/
void UI_MenuFrameBottomLineEnd_Graphics (char *string,int color)
{
	int holdX,holdLength;

	ui.R_SetColor( colorTable[color]);
	holdX = MENU_TITLE_X - (UI_ProportionalStringWidth( string,UI_SMALLFONT));
	holdLength = (367 + 6) - holdX;
	UI_DrawHandlePic( 367,  438,  holdLength,  18, uis.whiteShader);	// Bottom end line
}

/*
===============
MenuFrame_Cache
===============
*/
void MenuFrame_Cache(void)
{
	s_menuframe.cornerUpper = ui.R_RegisterShaderNoMip("menu/common/corner_ll_47_7.tga");
	s_menuframe.cornerUpper2= ui.R_RegisterShaderNoMip("menu/common/corner_ul_47_7.tga");
	s_menuframe.cornerLower = ui.R_RegisterShaderNoMip("menu/common/corner_ll_47_18.tga");
}


/*
=================
UI_MenuFrame_Graphic
=================
*/
void UI_MenuFrame(menuframework_s *menu)
{
	if (!s_menuframe.initialized)
	{
		MenuFrame_Cache();
	}

	// Graphic frame
	UI_FrameTop_Graphics(menu);	// Top third
	UI_FrameBottom_Graphics();	// Bottom two thirds

	// Add foot note
	if (menu->footNoteEnum)
	{
		UI_DrawProportionalString(  MENU_TITLE_X, 440, menu_normal_text[menu->footNoteEnum],UI_RIGHT | UI_SMALLFONT, colorTable[CT_LTORANGE]);
		UI_MenuFrameBottomLineEnd_Graphics (menu_normal_text[menu->footNoteEnum],CT_LTBROWN1);
	}
}


/*
=================
UI_MenuFrame2
=================
*/
void UI_MenuFrame2(menuframework_s *menu)
{
	if (!s_menuframe.initialized)
	{
		MenuFrame_Cache();
	}

	if (!ingameFlag)
	{
		menu->fullscreen			= qtrue;
	}
	else	// In game menu
	{
		menu->fullscreen			= qfalse;
	}

	if (menu->titleI)
	{
		UI_DrawProportionalString( menu->titleX, menu->titleY ,menu_normal_text[menu->titleI],
			 UI_RIGHT|UI_BIGFONT, colorTable[CT_LTORANGE]);
	}

	ui.R_SetColor( colorTable[CT_DKBROWN1]);
	UI_DrawHandlePic(  30, 25,  47,  119, uis.whiteShader);	// Top left column
	UI_DrawHandlePic(  30, 147,  47,  53, uis.whiteShader);	// left column

	UI_DrawHandlePic(  30, 175,  47,  25, uis.whiteShader);	// Mid left column
	UI_DrawHandlePic(  30, 392,  47,  33, uis.whiteShader);	// Bottom left column
	UI_DrawHandlePic(  30, 425, 128,  64, s_menuframe.cornerLower);// Bottom Left Corner

	ui.R_SetColor( colorTable[CT_LTBROWN1]);
	UI_DrawHandlePic( 96,  438, 268,  18, uis.whiteShader);	// Bottom front Line

	// Add foot note
	if (menu->footNoteEnum)
	{
		UI_DrawProportionalString(  MENU_TITLE_X, 440, menu_normal_text[menu->footNoteEnum],UI_RIGHT | UI_SMALLFONT, colorTable[CT_LTORANGE]);
		UI_MenuFrameBottomLineEnd_Graphics (menu_normal_text[menu->footNoteEnum],CT_LTBROWN1);
	}
	ui.R_SetColor(NULL);

	// Print version
	UI_DrawProportionalString(  371, 445, Q3_VERSION,UI_TINYFONT, colorTable[CT_BLACK]);
}

/*
UI_LanguageFilename - create a filename with an extension based on the value in g_language
*/
void UI_LanguageFilename(char *baseName,char *baseExtension,char *finalName)
{
	char	language[32];
	fileHandle_t	file;

	ui.Cvar_VariableStringBuffer("g_language", language, 32 );

	// If it's English then no extension
	if (language[0]=='\0' || Q_stricmp("ENGLISH",language)==0)
	{
		Com_sprintf(finalName,MAX_QPATH,"%s.%s",baseName,baseExtension);
	}
	else
	{
		Com_sprintf(finalName,MAX_QPATH,"%s_%s.%s",baseName,language,baseExtension);

		//Attempt to load the file
		ui.FS_FOpenFile( finalName, &file, FS_READ );

		if ( file == 0 )	//	This extension doesn't exist, go English.
		{
			Com_sprintf(finalName,MAX_QPATH,"%s.%s",baseName,baseExtension);
		}
		else
		{
			ui.FS_FCloseFile( file );
		}
	}
}

/*
===============
UI_LoadMenu_f
===============
*/
 void UI_LoadMenu_f( void ) 
{
	if ((ui.Cvar_VariableValue( "cg_virtualVoyager" )==1)&& (holoMatch	== qfalse))
	{
		Mouse_Show();
		UI_VirtualLoadMenu();
	}
	else
	{
		UI_LoadGameMenu(qfalse); 
	}
}

void UI_VirtualSaveMenu(int from);

/*
===============
UI_SaveMenu_f
===============
*/
 void UI_SaveMenu_f( void ) 
{


	if (!ui.SG_GameAllowedToSaveHere(qfalse))
	{
		Com_Printf(S_COLOR_RED "%s\n",menu_normal_text[MNT_CANNOTSAVE]);
//		Com_Printf(S_COLOR_RED "Cannot save in while on holodeck, in brig or in Borg Slayer.\n");
		return;
	}

	ui.PrecacheScreenshot();

	if ((ui.Cvar_VariableValue( "cg_virtualVoyager" )==1)&& (holoMatch	== qfalse))
	{
		Mouse_Show();
		UI_VirtualSaveMenu(2);
	}
	else
	{
		UI_SaveGameMenu(qfalse); 
	}
}


#define	FADE_TIME			200

/*
================
UI_FadeAmount
================
*/
float UI_FadeAmount( int startMsec, int totalMsec, int fadeFlag ) 
{
	float amount;
	int			t;

	if ( startMsec == 0 ) 
	{
		return NULL;
	}

	t = uis.realtime - startMsec;

	if ( t >= totalMsec ) 
	{
		return NULL;
	}


	if (fadeFlag == UI_FADEOUT)
	{
		// fade out
		if ( totalMsec - t < FADE_TIME ) 
		{
			amount = ( totalMsec - t ) * 1.0/FADE_TIME;
		} 
		else 
		{
			amount = 1.0;
		}
	}
	else
	{
		// fade in
		if ( totalMsec - t < FADE_TIME ) 
		{
			amount = 1 - (( totalMsec - t ) * 1.0/FADE_TIME);
		} 
		else 
		{
			amount = 0.01f;
		}
	}

	return amount;
}


#define ID_CONFIRM_NO		10
#define ID_CONFIRM_YES		11

typedef struct 
{
	menuframework_s menu;

	menubitmap_s	no;
	menubitmap_s	yes;

	int				slashX;
	const char *	question;
	void			(*draw)( void );
	void			(*action)( qboolean result );
} confirmMenu_t;


static confirmMenu_t	s_confirm;

/*
=================
ConfirmMenu_Event
=================
*/
static void ConfirmMenu_Event( void* ptr, int event ) 
{
	qboolean	result;

	if( event != QM_ACTIVATED ) 
	{
		return;
	}

	UI_PopMenu();

	if( ((menucommon_s*)ptr)->id == ID_CONFIRM_NO ) 
	{
		result = qfalse;
	}
	else 
	{
		result = qtrue;
	}

	if( s_confirm.action ) 
	{
		s_confirm.action( result );
	}
}


/*
=================
ConfirmMenu_Key
=================
*/
static sfxHandle_t ConfirmMenu_Key( int key ) 
{
	switch ( key ) 
	{
	case K_KP_LEFTARROW:
	case K_LEFTARROW:
	case K_KP_RIGHTARROW:
	case K_RIGHTARROW:
		key = K_TAB;
		break;

	case 'n':
	case 'N':
		ConfirmMenu_Event( &s_confirm.no, QM_ACTIVATED );
		break;

	case 'y':
	case 'Y':
		ConfirmMenu_Event( &s_confirm.yes, QM_ACTIVATED );
		break;
	}

	return Menu_DefaultKey( &s_confirm.menu, key );
}

stringID_table_t menuTextTable [] =
{
	ENUM2STRING(MT_NONE),
	ENUM2STRING(MT_NEWGAME),
	ENUM2STRING(MT_MAX)
};


stringID_table_t menuEventIdTable [] =
{
	ENUM2STRING(MEI_NONE),
	ENUM2STRING(MEI_NEWGAME),
	ENUM2STRING(MEI_MAX)
};

#define ATOM_GRAPHIC	1
#define ATOM_TEXT		2
#define ATOM_BUTTON		3

// Only used during loading of menu screen script
typedef struct 
{
	char		currentMenuId[MENULAYOUT_IDSIZE];	// ID of the current Menu
	char		currentAtomId[MENULAYOUT_IDSIZE];	// ID of the current Atom
	qboolean	currentMenuData;	// qtrue if looking at MENU_DATA
	int			errorCnt;			// count of errors during loading
	int			currentMenuIndex;	// index to menulayout array of current menu being loaded
	int			currentAtomIndex;	// index to menuatoms array of current atom being loaded
	char		menuTextStringPos;	// current position in menuTextString[]
} menuloadingvars_t;

static menuloadingvars_t s_menuloadvars;

typedef struct 
{
	char	*colorString;			// Color string
	int		colorDefinition;		// #define
} colordefinitions_t;

colordefinitions_t colordefinitions[] = 
{
//"NONE",			CT_NONE,
"BLACK",		CT_BLACK,
"RED",			CT_RED,
"GREEN",		CT_GREEN,
"BLUE",			CT_BLUE,
"YELLOW",		CT_YELLOW,
"MAGENTA",		CT_MAGENTA,
"CYAN",			CT_CYAN,
"WHITE",		CT_WHITE,
"LTGREY",		CT_LTGREY,
"MDGREY",		CT_MDGREY,
"DKGREY",		CT_DKGREY,
"DKGREY2",		CT_DKGREY2,

"VLTORANGE",	CT_VLTORANGE,
"LTORANGE",		CT_LTORANGE,
"DKORANGE",		CT_DKORANGE,
"VDKORANGE",	CT_VDKORANGE,

"VLTBLUE1",		CT_VLTBLUE1,
"LTBLUE1",		CT_LTBLUE1,
"DKBLUE1",		CT_DKBLUE1,
"VDKBLUE1",		CT_VDKBLUE1,

"VLTBLUE2",		CT_VLTBLUE2,
"LTBLUE2",		CT_LTBLUE2,
"DKBLUE2",		CT_DKBLUE2,
"VDKBLUE2",		CT_VDKBLUE2,

"VLTBROWN1",	CT_VLTBROWN1,
"LTBROWN1",		CT_LTBROWN1,
"DKBROWN1",		CT_DKBROWN1,
"VDKBROWN1",	CT_VDKBROWN1,

"VLTGOLD1",		CT_VLTGOLD1,
"LTGOLD1",		CT_LTGOLD1,
"DKGOLD1",		CT_DKGOLD1,
"VDKGOLD1",		CT_VDKGOLD1,

"VLTPURPLE1",	CT_VLTPURPLE1,
"LTPURPLE1",	CT_LTPURPLE1,
"DKPURPLE1",	CT_DKPURPLE1,
"VDKPURPLE1",	CT_VDKPURPLE1,

"VLTPURPLE2",	CT_VLTPURPLE2,
"LTPURPLE2",	CT_LTPURPLE2,
"DKPURPLE2",	CT_DKPURPLE2,
"VDKPURPLE2",	CT_VDKPURPLE2,

"VLTPURPLE3",	CT_VLTPURPLE3,
"LTPURPLE3",	CT_LTPURPLE3,
"DKPURPLE3",	CT_DKPURPLE3,
"VDKPURPLE3",	CT_VDKPURPLE3,

"VLTRED1",		CT_VLTRED1,
"LTRED1",		CT_LTRED1,
"DKRED1",		CT_DKRED1,
"VDKRED1",		CT_VDKRED1,
"VDKRED",		CT_VDKRED,
"DKRED",		CT_DKRED,

	"",			NULL,
};

#define	MENULAYOUTLENGTH	50000
#define LAYOUTDEFINES_MAX	500

typedef struct 
{
	char	define[MENULAYOUT_IDSIZE];			// #define
	char	value[MENULAYOUT_IDSIZE];			// value of #define
} layoutdefines_t;

layoutdefines_t layoutDefines[LAYOUTDEFINES_MAX];

char	menuLayoutBuffer[MENULAYOUTLENGTH];

menuatoms_t menuatoms[MENUATOMS_MAX];
menulayout_t menulayout[MENULAYOUT_MAX];

// Handle ATOM_ID token
static void UI_ParseAtomID(char **buffer)
{
	char	*token2;

	s_menuloadvars.currentAtomIndex++;							// New atom
	menulayout[s_menuloadvars.currentMenuIndex].atomCnt++;		

	// If this is the first one for the current menu point to it
	if (menulayout[s_menuloadvars.currentMenuIndex].atomCnt==1)
	{
		menulayout[s_menuloadvars.currentMenuIndex].atoms=
			&menuatoms[s_menuloadvars.currentAtomIndex];
	}
	else	// Point last atom to this atom
	{
		 menuatoms[s_menuloadvars.currentAtomIndex-1].nextAtom = 
			&menuatoms[s_menuloadvars.currentAtomIndex];
	}

	MS_GetID(buffer,s_menuloadvars.currentAtomId);

	// Move in MENU_ID data
	Q_strncpyz( menuatoms[s_menuloadvars.currentAtomIndex].atomId, 
		s_menuloadvars.currentAtomId, sizeof( menuatoms[0].atomId) );

	token2 = COM_ParseExt( buffer, qtrue );
	if (Q_stricmp(token2,"{"))	
	{
		Com_Printf(S_COLOR_RED "No opening bracket after ATOM_ID %s!\n",s_menuloadvars.currentAtomId);
		s_menuloadvars.errorCnt++;
	}
}

// Handle TYPE token
static void UI_ParseType(char **buffer)
{
	char	type[MENULAYOUT_IDSIZE];

	MS_GetID(buffer,type);

	if (!Q_stricmp(type,"GRAPHIC"))
	{
		menuatoms[s_menuloadvars.currentAtomIndex].type = ATOM_GRAPHIC;
	}
	else if (!Q_stricmp(type,"TEXT"))
	{
		menuatoms[s_menuloadvars.currentAtomIndex].type = ATOM_TEXT;
	}
	else if (!Q_stricmp(type,"BUTTON"))
	{
		menuatoms[s_menuloadvars.currentAtomIndex].type = ATOM_BUTTON;
	}
	else	// Invalid type
	{
		Com_Printf(S_COLOR_RED "Invalid type %s for ATOM %s!\n",type,menuatoms[s_menuloadvars.currentAtomIndex].atomId);
		s_menuloadvars.errorCnt++;
	}
}

int MS_CheckDefines(char *token)
{
	int	i;

	i = 0;
	while (layoutDefines[i].define[0])
	{
		if (!Q_stricmp(token,layoutDefines[i].define))
		{
			return(i);
		}
		i++;
	}
	return(-1);
}

static void MS_GetString(char **buffer,char **charValue)
{
	int	index,len;
	char	*token;

	token = COM_ParseExt( buffer, qtrue );

	index = MS_CheckDefines(token);
	if (index<0)
	{
		len = strlen(token);
		*charValue = (*buffer - (len + 1));
		*(*charValue + len) = NULL;
	}
	else if (layoutDefines[index].define[0])
	{
		*charValue = layoutDefines[index].value;
	}
	else 
	{
		Com_Printf(S_COLOR_RED "MS_GetString : String: '%s' not found!\n",token);
	}
}

static void MS_GetID(char **buffer,char *charValue)
{
	char	*token;
	int		index;

	token = COM_ParseExt( buffer, qtrue );

	index = MS_CheckDefines(token);
	if (index<0)
	{
		Q_strncpyz( charValue, token, MENULAYOUT_IDSIZE );
	}
	else
	{
		Q_strncpyz( charValue, layoutDefines[index].value, strlen(layoutDefines[index].value) + 1);
	}
}

// Handle COLOR token
static void UI_ParseColor(char **buffer)
{
	char	color[MENULAYOUT_IDSIZE];
	int		i;

	MS_GetID(buffer,color);

	i = 0;
	while (colordefinitions[i].colorDefinition != NULL)
	{	
		if (!Q_stricmp(color,colordefinitions[i].colorString))
		{
			menuatoms[s_menuloadvars.currentAtomIndex].color = colordefinitions[i].colorDefinition;
			break;
		}
		i++;
	}

	if (colordefinitions[i].colorDefinition == NULL)
	{
		Com_Printf(S_COLOR_RED "Invalid color for ATOM %s in MENU %s!\n",color,menuatoms[s_menuloadvars.currentAtomIndex].atomId,s_menuloadvars.currentMenuId);
		s_menuloadvars.errorCnt++;

		menuatoms[s_menuloadvars.currentAtomIndex].color = CT_WHITE;
	}
}

static void UI_ParseTextEnum(char **buffer)
{
	char	textEnum[MENULAYOUT_IDSIZE];

	MS_GetID(buffer,textEnum);
	menuatoms[s_menuloadvars.currentAtomIndex].textEnum = GetIDForString(menuTextTable,textEnum);

}

static void UI_ParseEventID(char **buffer)
{
	char	eventID[MENULAYOUT_IDSIZE];

	MS_GetID(buffer,eventID);
	menuatoms[s_menuloadvars.currentAtomIndex].eventId = GetIDForString(menuEventIdTable,eventID);
}



// Handle TEXT_COLOR token
static void UI_ParseTextColor(char **buffer)
{
	char	color[MENULAYOUT_IDSIZE];
	int		i;

	MS_GetID(buffer,color);

	i = 0;
	while (colordefinitions[i].colorDefinition != NULL)
	{	
		if (!Q_stricmp(color,colordefinitions[i].colorString))
		{
			menuatoms[s_menuloadvars.currentAtomIndex].textcolor = colordefinitions[i].colorDefinition;	
			break;
		}
		i++;
	}

	if (colordefinitions[i].colorDefinition == NULL)
	{
		Com_Printf(S_COLOR_RED "Invalid color for ATOM %s in MENU %s!\n",color,menuatoms[s_menuloadvars.currentAtomIndex].atomId,s_menuloadvars.currentMenuId);
		s_menuloadvars.errorCnt++;

		menuatoms[s_menuloadvars.currentAtomIndex].textcolor = CT_WHITE;
	}

}

// Handle TEXT_HI_COLOR token
static void UI_ParseTextHiColor(char **buffer)
{
	char	color[MENULAYOUT_IDSIZE];
	int		i;

	MS_GetID(buffer,color);

	i = 0;
	while (colordefinitions[i].colorDefinition != NULL)
	{	
		if (!Q_stricmp(color,colordefinitions[i].colorString))
		{
			menuatoms[s_menuloadvars.currentAtomIndex].textcolor2 = colordefinitions[i].colorDefinition;	
			break;
		}
		i++;
	}

	if (colordefinitions[i].colorDefinition == NULL)
	{
		Com_Printf(S_COLOR_RED "Invalid color for ATOM %s in MENU %s!\n",color,menuatoms[s_menuloadvars.currentAtomIndex].atomId,s_menuloadvars.currentMenuId);
		s_menuloadvars.errorCnt++;

		menuatoms[s_menuloadvars.currentAtomIndex].textcolor2 = CT_WHITE;
	}

}

// Handle HI_COLOR token
static void UI_ParseHiColor(char **buffer)
{
	char	color[MENULAYOUT_IDSIZE];
	int		i;

	MS_GetID(buffer,color);

	i = 0;
	while (colordefinitions[i].colorDefinition != NULL)
	{	
		if (!Q_stricmp(color,colordefinitions[i].colorString))
		{
			menuatoms[s_menuloadvars.currentAtomIndex].color2 = colordefinitions[i].colorDefinition;
			break;
		}
		i++;
	}

	if (colordefinitions[i].colorDefinition == NULL)
	{
		Com_Printf(S_COLOR_RED "Invalid hi color for ATOM %s in MENU %s!\n",color,menuatoms[s_menuloadvars.currentAtomIndex].atomId,s_menuloadvars.currentMenuId);
		s_menuloadvars.errorCnt++;

		menuatoms[s_menuloadvars.currentAtomIndex].color2 = CT_WHITE;
	}

}


// Handle FONT token
static void UI_ParseFont(char **buffer)
{
	char	font[MENULAYOUT_IDSIZE];

	MS_GetID(buffer,font);

	if (!Q_stricmp(font,"TINY"))
	{
		menuatoms[s_menuloadvars.currentAtomIndex].style |= UI_TINYFONT;
	}
	else if (!Q_stricmp(font,"SMALL"))
	{
		menuatoms[s_menuloadvars.currentAtomIndex].style |= UI_SMALLFONT;
	}
	else if (!Q_stricmp(font,"BIG"))
	{
		menuatoms[s_menuloadvars.currentAtomIndex].style |= UI_BIGFONT;
	}
	else if (!Q_stricmp(font,"GIANT"))
	{
		menuatoms[s_menuloadvars.currentAtomIndex].style |= UI_GIANTFONT;
	}
	else
	{
		Com_Printf(S_COLOR_RED "Invalid font for ATOM %s in MENU %s!\n",font,menuatoms[s_menuloadvars.currentAtomIndex].atomId,s_menuloadvars.currentMenuId);
		s_menuloadvars.errorCnt++;
	}
}

// Handle JUSTIFY token
static void UI_ParseJustify(char **buffer)
{
	char	token[MENULAYOUT_IDSIZE];

	MS_GetID(buffer,token);

	if (!Q_stricmp(token,"LEFT"))
	{
		menuatoms[s_menuloadvars.currentAtomIndex].style |= UI_LEFT;
	}
	else if (!Q_stricmp(token,"RIGHT"))
	{
		menuatoms[s_menuloadvars.currentAtomIndex].style |= UI_RIGHT;
	}
	else if (!Q_stricmp(token,"CENTER"))
	{
		menuatoms[s_menuloadvars.currentAtomIndex].style |= UI_CENTER;
	}
	else
	{
		Com_Printf(S_COLOR_RED "Invalid justify for ATOM %s in MENU %s!\n",token,menuatoms[s_menuloadvars.currentAtomIndex].atomId,s_menuloadvars.currentMenuId);
		s_menuloadvars.errorCnt++;
	}
}

// Handle SHADER token
static void UI_ParseShader(char **buffer)
{
	MS_GetString(buffer,&menuatoms[s_menuloadvars.currentAtomIndex].shader);
	menuatoms[s_menuloadvars.currentAtomIndex].shaderHandle	= ui.R_RegisterShaderNoMip(menuatoms[s_menuloadvars.currentAtomIndex].shader);
	if (menuatoms[s_menuloadvars.currentAtomIndex].shaderHandle<1)
	{
		Com_Printf(S_COLOR_RED "Invalid shader %s for ATOM %s!\n",menuatoms[s_menuloadvars.currentAtomIndex].shader,menuatoms[s_menuloadvars.currentAtomIndex].atomId);
	}
}

// Handle FLAG token
static void UI_ParseFlag(char **buffer)
{
	char	flag[MENULAYOUT_IDSIZE];

	MS_GetID(buffer,flag);

	if (!Q_stricmp(flag,"HIGHLIGHT_IF_FOCUS"))
	{
		menuatoms[s_menuloadvars.currentAtomIndex].flags |= QMF_HIGHLIGHT_IF_FOCUS;
	}
//	else if (!Q_stricmp(flag,"ACTIVE"))
//	{
//		menuatoms[s_menuloadvars.currentAtomIndex].flags |= QMF_ACTIVE;
//	}
	else
	{
		Com_Printf(S_COLOR_RED "Invalid Flag for ATOM %s in MENU %s!\n",flag,menuatoms[s_menuloadvars.currentAtomIndex].atomId,s_menuloadvars.currentMenuId);
		s_menuloadvars.errorCnt++;
	}
}

// Handle TEXT token
static void UI_ParseText(char **buffer)
{
	MS_GetString(buffer,&menuatoms[s_menuloadvars.currentAtomIndex].text);
}

// Handle TEXT2 token
static void UI_ParseText2(char **buffer)
{
	MS_GetString(buffer,&menuatoms[s_menuloadvars.currentAtomIndex].text2);
}

// Handle MENU_DATA token
static void UI_ParseMenuData(char **buffer)
{
	char	*token1;

	token1 = COM_ParseExt( buffer, qtrue );

	s_menuloadvars.currentMenuData=qtrue;

	if (Q_stricmp(token1,"{"))	
	{
		s_menuloadvars.currentMenuData=qfalse;
		Com_Printf(S_COLOR_RED "No opening bracket after MENU_DATA within MENU_ID %s!\n",s_menuloadvars.currentMenuId);
		s_menuloadvars.errorCnt++;
	}
}

// Handle TITLE token
static void UI_ParseTitle(char **buffer)
{
	MS_GetID(buffer,menulayout[s_menuloadvars.currentMenuIndex].title);
}


static void UI_InitializeMenuLayoutParms(void)
{
	memset(s_menuloadvars.currentAtomId,0,sizeof(s_menuloadvars.currentAtomId));
	memset(s_menuloadvars.currentMenuId,0,sizeof(s_menuloadvars.currentMenuId));
	s_menuloadvars.currentMenuData=qfalse;
}

// Handle closing bracket
static void UI_ParseCloseBracket(char **buffer)
{
	if (s_menuloadvars.currentAtomId[0])	// Is an Atom the current focus?
	{
		memset(s_menuloadvars.currentAtomId,0,sizeof(s_menuloadvars.currentAtomId));
	}
	else if (s_menuloadvars.currentMenuData)	// Is Menu_Data the current focus?
	{
		s_menuloadvars.currentMenuData=qfalse;
	}
	else if (s_menuloadvars.currentMenuId[0])	// Is a Menu_Id the current focus?
	{
		memset(s_menuloadvars.currentMenuId,0,sizeof(s_menuloadvars.currentMenuId));
	}
	else
	{
		Com_Printf(S_COLOR_RED "Invalid closing bracket!\n");
		s_menuloadvars.errorCnt++;
	}
}

// Handle MENU_ID token
static void UI_ParseMenuID(char **buffer)
{
//	char	*token1;
	char	*token2;

	if (s_menuloadvars.currentMenuId[0])	// Is a Menu_Id the current focus?
	{
		Com_Printf(S_COLOR_RED "No closing bracket found for MENU_ID %s!\n",s_menuloadvars.currentMenuId);
		s_menuloadvars.errorCnt++;
		UI_InitializeMenuLayoutParms();
	}

	s_menuloadvars.currentMenuIndex++;
	menulayout[s_menuloadvars.currentMenuIndex].atomCnt=0;		
	menulayout[s_menuloadvars.currentMenuIndex].atoms=NULL;		

	MS_GetID(buffer,s_menuloadvars.currentMenuId);	
	Q_strncpyz( menulayout[s_menuloadvars.currentMenuIndex].menuId, 
		s_menuloadvars.currentMenuId, MENULAYOUT_IDSIZE );

	token2 = COM_ParseExt( buffer, qtrue );
	if (Q_stricmp(token2,"{"))	
	{
		Com_Printf(S_COLOR_RED "No opening bracket after MENU_ID %s!\n",s_menuloadvars.currentMenuId);
		s_menuloadvars.errorCnt++;
	}
}

void MS_GetInt(char **buffer,int *intValue)
{
	int	index;
	char	*token;

	token = COM_ParseExt( buffer, qtrue );

	index = MS_CheckDefines(token);
	if (index<0)
	{
		*intValue = atoi(token);
	}
	else if (layoutDefines[index].define[0])
	{
		*intValue = atoi(layoutDefines[index].value);
	}
}

// Handle SUBTYPE token
static void UI_ParseSubType(char **buffer)
{
	char	subType[MENULAYOUT_IDSIZE];

	MS_GetID(buffer,subType);

	if (!Q_stricmp(subType,"BITMAP"))
	{
		menuatoms[s_menuloadvars.currentAtomIndex].subtype = MTYPE_BITMAP;
	}
	else	// Invalid type
	{
		Com_Printf(S_COLOR_RED "Invalid sub type %s for ATOM %s!\n",subType,menuatoms[s_menuloadvars.currentAtomIndex].atomId);
		s_menuloadvars.errorCnt++;
	}
}

//--------------------------------------------
static void UI_ParseMenuLayoutParms(char *buffer)
{
	char	*holdBuf;
	char	*token;

	holdBuf = buffer;
	COM_BeginParseSession();

	s_menuloadvars.currentMenuIndex = -1;
	s_menuloadvars.currentAtomIndex = -1;
	s_menuloadvars.errorCnt = 0;


	UI_InitializeMenuLayoutParms();

	while ( holdBuf ) 
	{
		token = COM_ParseExt( &holdBuf, qtrue );

		if (!Q_stricmp(token,"ATOM_ID"))
		{
			UI_ParseAtomID(&holdBuf);
		}
		else if (!Q_stricmp(token,"EVENT_ID"))
		{
			UI_ParseEventID(&holdBuf);
		}
		else if (!Q_stricmp(token,"}"))
		{
			UI_ParseCloseBracket(&holdBuf);
		}
		else if (!Q_stricmp(token,"COLOR"))
		{
			UI_ParseColor(&holdBuf);
		}
		else if (!Q_stricmp(token,"DESC_X"))
		{
			MS_GetInt(&holdBuf,&menulayout[s_menuloadvars.currentMenuIndex].descX);
		}
		else if (!Q_stricmp(token,"DESC_Y"))
		{
			MS_GetInt(&holdBuf,&menulayout[s_menuloadvars.currentMenuIndex].descY);
		}
		else if (!Q_stricmp(token,"FLAG"))
		{
			UI_ParseFlag(&holdBuf);
		}
		else if (!Q_stricmp(token,"FONT"))
		{
			UI_ParseFont(&holdBuf);
		}
		else if (!Q_stricmp(token,"HEIGHT"))
		{
			MS_GetInt(&holdBuf,&menuatoms[s_menuloadvars.currentAtomIndex].height);
		}
		else if (!Q_stricmp(token,"HI_COLOR"))
		{
			UI_ParseHiColor(&holdBuf);
		}
		else if (!Q_stricmp(token,"JUSTIFY"))
		{
			UI_ParseJustify(&holdBuf);
		}
		else if (!Q_stricmp(token,"MENU_DATA"))
		{
			UI_ParseMenuData(&holdBuf);
		}
		else if (!Q_stricmp(token,"MENU_ID"))
		{
			UI_ParseMenuID(&holdBuf);
		}
		else if (!Q_stricmp(token,"SHADER"))
		{
			UI_ParseShader(&holdBuf);
		}
		else if (!Q_stricmp(token,"SUBTYPE"))
		{
			UI_ParseSubType(&holdBuf);
		}
		else if (!Q_stricmp(token,"TEXT"))
		{
			UI_ParseText(&holdBuf);
		}
		else if (!Q_stricmp(token,"TEXT2"))
		{
			UI_ParseText2(&holdBuf);
		}
		else if (!Q_stricmp(token,"TEXT_COLOR"))
		{
			UI_ParseTextColor(&holdBuf);
		}
		else if (!Q_stricmp(token,"TEXT_ENUM"))
		{
			UI_ParseTextEnum(&holdBuf);
		}
		else if (!Q_stricmp(token,"TEXT_HI_COLOR"))
		{
			UI_ParseTextHiColor(&holdBuf);
		}
		else if (!Q_stricmp(token,"TEXT_X"))
		{
			MS_GetInt(&holdBuf,&menuatoms[s_menuloadvars.currentAtomIndex].textX);
		}
		else if (!Q_stricmp(token,"TEXT_Y"))
		{
			MS_GetInt(&holdBuf,&menuatoms[s_menuloadvars.currentAtomIndex].textY);
		}
		else if (!Q_stricmp(token,"TITLE"))
		{
			UI_ParseTitle(&holdBuf);
		}
		else if (!Q_stricmp(token,"TITLE_X"))
		{
			MS_GetInt(&holdBuf,&menulayout[s_menuloadvars.currentMenuIndex].titleX);
		}
		else if (!Q_stricmp(token,"TITLE_Y"))
		{
			MS_GetInt(&holdBuf,&menulayout[s_menuloadvars.currentMenuIndex].titleY);
		}
		else if (!Q_stricmp(token,"TYPE"))
		{
			UI_ParseType(&holdBuf);
		}
		else if (!Q_stricmp(token,"WIDTH"))
		{
			MS_GetInt(&holdBuf,&menuatoms[s_menuloadvars.currentAtomIndex].width);
		}
		else if (!Q_stricmp(token,"X_POS"))
		{
			MS_GetInt(&holdBuf,&menuatoms[s_menuloadvars.currentAtomIndex].xPos);
		}
		else if (!Q_stricmp(token,"Y_POS"))
		{
			MS_GetInt(&holdBuf,&menuatoms[s_menuloadvars.currentAtomIndex].yPos);
		}

		if (s_menuloadvars.errorCnt > 20)
		{
			Com_Printf(S_COLOR_RED "Quitting load of MenuLayout - more than 20 errors.\n");
			return;
		}
	}

}

void UI_MenuLayoutDefines(char *buffer)
{
	char	*holdBuf;
	char	*token;
	int		len,i;

	memset(layoutDefines,0,sizeof(layoutDefines));

	holdBuf = buffer;
	COM_BeginParseSession();

	i=0;
	while ( holdBuf ) 
	{
		token = COM_ParseExt( &holdBuf, qtrue );

		if (!Q_stricmp(token,"#DEFINE"))
		{
			token = COM_ParseExt( &holdBuf, qtrue );
			len = strlen(token);
			Q_strncpyz( layoutDefines[i].define, token, len+1);

			token = COM_ParseExt( &holdBuf, qtrue );
			len = strlen(token);
			Q_strncpyz( layoutDefines[i].value, token, len+1);

			i++;
			if (i>=LAYOUTDEFINES_MAX)
			{
				Com_Printf(S_COLOR_RED "Stopped processing defines - went past LAYOUTDEFINES_MAX.\n");
				break;
			}
		}
	}
}

void UI_MenuLayoutSetup(char *buffer)
{
	// Get DEFINE values
	UI_MenuLayoutDefines(buffer);

}

void UI_LoadGenericMenu(char *fileName)
{
	char			fullName[MAX_QPATH];
	fileHandle_t	fileHandle;
	int				len;

	Com_sprintf( fullName,  MAX_QPATH, "ext_data/%s.dat", fileName);

	len = ui.FS_FOpenFile( fullName, &fileHandle, FS_READ );

	if (len > MENULAYOUTLENGTH)
	{
		Com_Printf(S_COLOR_RED "UI_LoadGenericMenu : Menu Layout file bigger than %d!\n",MENULAYOUTLENGTH);
		return;
	}
	else if (len < 0)
	{
		Com_Printf(S_COLOR_RED "UI_LoadGenericMenu : Unable to locate Menu Layout file %s!\n",fileName);
		return;
	}

	if ( fileHandle != NULL )
	{
		// initialise the data area
		memset(menuLayoutBuffer, 0, sizeof(menuLayoutBuffer));	

		ui.FS_Read( menuLayoutBuffer, len, fileHandle );

		ui.FS_FCloseFile( fileHandle );

		UI_MenuLayoutSetup(menuLayoutBuffer);

		UI_ParseMenuLayoutParms(menuLayoutBuffer);
	}
}

int UI_FindMenuID(char *menuID)
{
	int i;

	for (i=0;i<=s_menuloadvars.currentMenuIndex;++i)
	{
		if (!Q_stricmp(menulayout[i].menuId,menuID))
		{
			return i;
		}
	}

	return(-1);
}

void UI_PrintMenuAtoms(menulayout_t *menulayout)
{
	int			i;
	menuatoms_t	*atom;

	atom = menulayout->atoms;
	for (i=0;i<menulayout->atomCnt;i++)
	{
		// Is it a graphic??????
		if (atom->type==ATOM_GRAPHIC)
		{
			ui.R_SetColor( colorTable[atom->color] );
			if (atom->shaderHandle)
			{
				UI_DrawHandlePic( atom->xPos,atom->yPos , atom->width, atom->height, atom->shaderHandle);
			}
		}
		// Is it text??????
		else if (atom->type==ATOM_TEXT)
		{
			if (atom->text)
			{
				UI_DrawProportionalString( atom->xPos, atom->yPos ,atom->text,
					 atom->style, colorTable[atom->color]);
			}
			else
			{
				UI_DrawProportionalString( atom->xPos, atom->yPos ,atom->text,
					 atom->flags, colorTable[atom->color]);
			}
		}
		atom = (menuatoms_t	*) atom->nextAtom;
	}
}

void UI_PrintMenu(char *menuID)
{
	int menuI;

	// Find Menu Layout
	menuI = UI_FindMenuID(menuID);

	if (menuI >= 0)
	{
		UI_PrintMenuAtoms(&menulayout[menuI]);
	}
}

