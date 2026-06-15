/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
// cl_scrn.c -- master for refresh, status bar, console, chat, notify, etc

#include "client.h"
#include "../../sp/sp_ui.h"   // EF1 SP native UI (libefui.so) drive hooks

qboolean	scr_initialized;		// ready to draw
int			scr_placement;
float		scr_nativeScale = 1.0f;

cvar_t		*cl_timegraph;
cvar_t		*cl_debuggraph;
cvar_t		*cl_graphheight;
cvar_t		*cl_graphscale;
cvar_t		*cl_graphshift;

/*
================
SCR_DrawNamedPic

Coordinates are 640*480 virtual values
=================
*/
void SCR_DrawNamedPic( float x, float y, float width, float height, const char *picname ) {
	qhandle_t	hShader;

	assert( width != 0 );

	hShader = re.RegisterShader( picname );
	SCR_AdjustFrom640( &x, &y, &width, &height );
	re.DrawStretchPic( x, y, width, height, 0, 0, 1, 1, hShader );
}


/*
================
SCR_SetScreenPlacement
================
*/
void SCR_SetScreenPlacement( int placement ) {
	scr_placement = placement;
}

/*
================
SCR_SetNativeScale
================
*/
void SCR_SetNativeScale( float scale ) {
	scr_nativeScale = scale;
}

/*
================
SCR_AdjustFrom640

Adjusted for resolution and screen aspect ratio
================
*/
void SCR_AdjustFrom640( float *x, float *y, float *w, float *h ) {
	int placement = scr_placement;

	if ( ( placement & SCR_HOR_MASK ) == SCR_HOR_STRETCH ) {
		// scale for screen sizes (not aspect correct in wide screen)
		if ( w != NULL ) {
			*w *= cls.screenXScaleStretch;
		}
		if ( x != NULL ) {
			*x *= cls.screenXScaleStretch;
		}
	} else {
		// scale for screen sizes
		if ( placement & SCR_HOR_NATIVE ) {
			if ( w != NULL ) {
				*w *= scr_nativeScale;
			}

			if ( x != NULL ) {
				*x *= scr_nativeScale;
			}
		} else {
			if ( w != NULL ) {
				*w *= cls.screenXScale;
			}

			if ( x != NULL ) {
				*x *= cls.screenXScale;
			}
		}

		if ( x != NULL ) {
			if ( ( placement & SCR_HOR_MASK ) == SCR_HOR_CENTER ) {
				*x += cls.screenXBias;
			} else if ( ( placement & SCR_HOR_MASK ) == SCR_HOR_RIGHT ) {
				*x += cls.screenXBias*2;
			}
		}
	}

	if ( ( placement & SCR_VERT_MASK ) == SCR_VERT_STRETCH ) {
		if ( h != NULL ) {
			*h *= cls.screenYScaleStretch;
		}
		if ( y != NULL ) {
			*y *= cls.screenYScaleStretch;
		}
	} else {
		if ( placement & SCR_VERT_NATIVE ) {
			if ( h != NULL ) {
				*h *= scr_nativeScale;
			}

			if ( y != NULL ) {
				*y *= scr_nativeScale;
			}
		} else {
			if ( h != NULL ) {
				*h *= cls.screenYScale;
			}

			if ( y != NULL ) {
				*y *= cls.screenYScale;
			}
		}

		if ( y != NULL ) {
			if ( ( placement & SCR_VERT_MASK ) == SCR_VERT_CENTER ) {
				*y += cls.screenYBias;
			} else if ( ( placement & SCR_VERT_MASK ) == SCR_VERT_BOTTOM ) {
				*y += cls.screenYBias*2;
			}
		}
	}
}

/*
================
SCR_AdjustTo640

Adjusted for resolution and screen aspect ratio
================
*/
void SCR_AdjustTo640( float *x, float *y, float *w, float *h ) {
	int placement = scr_placement;

	if ( ( placement & SCR_HOR_MASK ) == SCR_HOR_STRETCH ) {
		// scale for screen sizes (not aspect correct in wide screen)
		if ( w != NULL ) {
			*w /= cls.screenXScaleStretch;
		}
		if ( x != NULL ) {
			*x /= cls.screenXScaleStretch;
		}
	} else {
		if ( x != NULL ) {
			if ( ( placement & SCR_HOR_MASK ) == SCR_HOR_CENTER ) {
				*x -= cls.screenXBias;
			} else if ( ( placement & SCR_HOR_MASK ) == SCR_HOR_RIGHT ) {
				*x -= cls.screenXBias*2;
			}
		}

		// scale for screen sizes
		if ( placement & SCR_HOR_NATIVE ) {
			if ( w != NULL ) {
				*w /= scr_nativeScale;
			}

			if ( x != NULL ) {
				*x /= scr_nativeScale;
			}
		} else {
			if ( w != NULL ) {
				*w /= cls.screenXScale;
			}

			if ( x != NULL ) {
				*x /= cls.screenXScale;
			}
		}
	}

	if ( ( placement & SCR_VERT_MASK ) == SCR_VERT_STRETCH ) {
		if ( h != NULL ) {
			*h /= cls.screenYScaleStretch;
		}
		if ( y != NULL ) {
			*y /= cls.screenYScaleStretch;
		}
	} else {
		if ( y != NULL ) {
			if ( ( placement & SCR_VERT_MASK ) == SCR_VERT_CENTER ) {
				*y -= cls.screenYBias;
			} else if ( ( placement & SCR_VERT_MASK ) == SCR_VERT_BOTTOM ) {
				*y -= cls.screenYBias*2;
			}
		}

		if ( placement & SCR_HOR_NATIVE ) {
			if ( h != NULL ) {
				*h /= scr_nativeScale;
			}

			if ( y != NULL ) {
				*y /= scr_nativeScale;
			}
		} else {
			if ( h != NULL ) {
				*h /= cls.screenYScale;
			}

			if ( y != NULL ) {
				*y /= cls.screenYScale;
			}
		}
	}
}

/*
================
SCR_FillRect

Coordinates are 640*480 virtual values
=================
*/
void SCR_FillRect( float x, float y, float width, float height, const float *color ) {
	re.SetColor( color );

	SCR_AdjustFrom640( &x, &y, &width, &height );
	re.DrawStretchPic( x, y, width, height, 0, 0, 0, 0, cls.whiteShader );

	re.SetColor( NULL );
}


/*
================
SCR_DrawPic

Coordinates are 640*480 virtual values
=================
*/
void SCR_DrawPic( float x, float y, float width, float height, qhandle_t hShader ) {
	SCR_AdjustFrom640( &x, &y, &width, &height );
	re.DrawStretchPic( x, y, width, height, 0, 0, 1, 1, hShader );
}



/*
** SCR_DrawChar
** chars are drawn at 640*480 virtual screen size
*/
static void SCR_DrawChar( int x, int y, float size, int ch ) {
	int row, col;
	float frow, fcol;
	float	ax, ay, aw, ah;

	ch &= 255;

	if ( ch == ' ' ) {
		return;
	}

	if ( y < -size ) {
		return;
	}

	ax = x;
	ay = y;
	aw = size;
	ah = size;
	SCR_AdjustFrom640( &ax, &ay, &aw, &ah );

	row = ch>>4;
	col = ch&15;

	frow = row*0.0625;
	fcol = col*0.0625;
	size = 0.0625;

	re.DrawStretchPic( ax, ay, aw, ah,
					   fcol, frow, 
					   fcol + size, frow + size, 
					   cls.charSetShader );
}

/*
** SCR_DrawSmallChar
** small chars are drawn at native screen resolution
*/
void SCR_DrawSmallChar( int x, int y, int ch ) {
	int row, col;
	float frow, fcol;
	float	ax, ay, aw, ah;
	float vsize, hsize;

	ch &= 255;

	if ( ch == ' ' ) {
		return;
	}

	if ( y < -SMALLCHAR_HEIGHT ) {
		return;
	}

	ax = x;
	ay = y;
	aw = SMALLCHAR_WIDTH;
	ah = SMALLCHAR_HEIGHT;
	SCR_AdjustFrom640( &ax, &ay, &aw, &ah );

	row = ch>>4;
	col = ch&15;

	frow = row*0.0625;
	fcol = col*0.0625;
	vsize = 0.0625;
#ifdef ELITEFORCE
	hsize = 0.03125;
#else
	hsize = 0.0625;
#endif

	re.DrawStretchPic( ax, ay, aw, ah,
					   fcol, frow, 
					   fcol + hsize, frow + vsize, 
					   cls.charSetShader );
}


/*
==================
SCR_DrawBigString[Color]

Draws a multi-colored string with a drop shadow, optionally forcing
to a fixed color.

Coordinates are at 640 by 480 virtual resolution
==================
*/
void SCR_DrawStringExt( int x, int y, float size, const char *string, float *setColor, qboolean forceColor,
		qboolean noColorEscape ) {
	vec4_t		color;
	const char	*s;
	int			xx;

	// draw the drop shadow
	color[0] = color[1] = color[2] = 0;
	color[3] = setColor[3];
	re.SetColor( color );
	s = string;
	xx = x;
	while ( *s ) {
		if ( !noColorEscape && Q_IsColorString( s ) ) {
			s += 2;
			continue;
		}
		SCR_DrawChar( xx+2, y+2, size, *s );
		xx += size;
		s++;
	}


	// draw the colored text
	s = string;
	xx = x;
	re.SetColor( setColor );
	while ( *s ) {
		if ( Q_IsColorString( s ) ) {
			if ( !forceColor ) {
				Com_Memcpy( color, g_color_table[ColorIndex(*(s+1))], sizeof( color ) );
				color[3] = setColor[3];
				re.SetColor( color );
			}
			if ( !noColorEscape ) {
				s += 2;
				continue;
			}
		}
		SCR_DrawChar( xx, y, size, *s );
		xx += size;
		s++;
	}
	re.SetColor( NULL );
}


void SCR_DrawBigString( int x, int y, const char *s, float alpha, qboolean noColorEscape ) {
	float	color[4];

	color[0] = color[1] = color[2] = 1.0;
	color[3] = alpha;
	SCR_DrawStringExt( x, y, BIGCHAR_WIDTH, s, color, qfalse, noColorEscape );
}

void SCR_DrawBigStringColor( int x, int y, const char *s, vec4_t color, qboolean noColorEscape ) {
	SCR_DrawStringExt( x, y, BIGCHAR_WIDTH, s, color, qtrue, noColorEscape );
}

#ifdef ELITEFORCE
void SCR_DrawSmallString(int x, int y, const char *s, float alpha)
{
	float   color[4];
	
	color[0] = color[1] = color[2] = 1.0;
	color[3] = alpha;
	SCR_DrawSmallStringExt(x, y, s, color, qfalse, qfalse);
}
#endif

/*
==================
SCR_DrawSmallString[Color]

Draws a multi-colored string with a drop shadow, optionally forcing
to a fixed color.
==================
*/
void SCR_DrawSmallStringExt( int x, int y, const char *string, float *setColor, qboolean forceColor,
		qboolean noColorEscape ) {
	vec4_t		color;
	const char	*s;
	int			xx;

	// draw the colored text
	s = string;
	xx = x;
	re.SetColor( setColor );
	while ( *s ) {
		if ( Q_IsColorString( s ) ) {
			if ( !forceColor ) {
				Com_Memcpy( color, g_color_table[ColorIndex(*(s+1))], sizeof( color ) );
				color[3] = setColor[3];
				re.SetColor( color );
			}
			if ( !noColorEscape ) {
				s += 2;
				continue;
			}
		}
		SCR_DrawSmallChar( xx, y, *s );
		xx += SMALLCHAR_WIDTH;
		s++;
	}
	re.SetColor( NULL );
}



/*
** SCR_Strlen -- skips color escape codes
*/
static int SCR_Strlen( const char *str ) {
	const char *s = str;
	int count = 0;

	while ( *s ) {
		if ( Q_IsColorString( s ) ) {
			s += 2;
		} else {
			count++;
			s++;
		}
	}

	return count;
}

/*
** SCR_GetBigStringWidth
*/ 
int	SCR_GetBigStringWidth( const char *str ) {
	return SCR_Strlen( str ) * BIGCHAR_WIDTH;
}


//===============================================================================

/*
=================
SCR_DrawDemoRecording
=================
*/
void SCR_DrawDemoRecording( void ) {
	char	string[1024];
	int		pos;

	if ( !clc.demorecording ) {
		return;
	}
	if ( clc.spDemoRecording ) {
		return;
	}

	pos = FS_FTell( clc.demofile );
	sprintf( string, "RECORDING %s: %ik", clc.demoName, pos / 1024 );

	SCR_DrawStringExt( 320 - strlen( string ) * 4, 20, 8, string, g_color_table[7], qtrue, qfalse );
}


#ifdef USE_VOIP
/*
=================
SCR_DrawVoipMeter
=================
*/
void SCR_DrawVoipMeter( void ) {
	char	buffer[16];
	char	string[256];
	int limit, i;

	if (!cl_voipShowMeter->integer)
		return;  // player doesn't want to show meter at all.
	else if (!cl_voipSend->integer)
		return;  // not recording at the moment.
	else if (clc.state != CA_ACTIVE)
		return;  // not connected to a server.
	else if (!clc.voipEnabled)
		return;  // server doesn't support VoIP.
	else if (clc.demoplaying)
		return;  // playing back a demo.
	else if (!cl_voip->integer)
		return;  // client has VoIP support disabled.

	limit = (int) (clc.voipPower * 10.0f);
	if (limit > 10)
		limit = 10;

	for (i = 0; i < limit; i++)
		buffer[i] = '*';
	while (i < 10)
		buffer[i++] = ' ';
	buffer[i] = '\0';

	sprintf( string, "VoIP: [%s]", buffer );
	SCR_DrawStringExt( 320 - strlen( string ) * 4, 10, 8, string, g_color_table[7], qtrue, qfalse );
}
#endif




/*
===============================================================================

DEBUG GRAPH

===============================================================================
*/

static	int			current;
static	float		values[1024];

/*
==============
SCR_DebugGraph
==============
*/
void SCR_DebugGraph (float value)
{
	values[current] = value;
	current = (current + 1) % ARRAY_LEN(values);
}

/*
==============
SCR_DrawDebugGraph
==============
*/
void SCR_DrawDebugGraph (void)
{
	int		a, x, y, w, i, h;
	float	v;

	//
	// draw the graph
	//
	w = cls.glconfig.vidWidth;
	x = 0;
	y = cls.glconfig.vidHeight;
	re.SetColor( g_color_table[0] );
	re.DrawStretchPic(x, y - cl_graphheight->integer, 
		w, cl_graphheight->integer, 0, 0, 0, 0, cls.whiteShader );
	re.SetColor( NULL );

	for (a=0 ; a<w ; a++)
	{
		i = (ARRAY_LEN(values)+current-1-(a % ARRAY_LEN(values))) % ARRAY_LEN(values);
		v = values[i];
		v = v * cl_graphscale->integer + cl_graphshift->integer;
		
		if (v < 0)
			v += cl_graphheight->integer * (1+(int)(-v / cl_graphheight->integer));
		h = (int)v % cl_graphheight->integer;
		re.DrawStretchPic( x+w-1-a, y - h, 1, h, 0, 0, 0, 0, cls.whiteShader );
	}
}

//=============================================================================

/*
==================
SCR_Init
==================
*/
void SCR_Init( void ) {
	cl_timegraph = Cvar_Get ("timegraph", "0", CVAR_CHEAT);
	cl_debuggraph = Cvar_Get ("debuggraph", "0", CVAR_CHEAT);
	cl_graphheight = Cvar_Get ("graphheight", "32", CVAR_CHEAT);
	cl_graphscale = Cvar_Get ("graphscale", "1", CVAR_CHEAT);
	cl_graphshift = Cvar_Get ("graphshift", "0", CVAR_CHEAT);

	scr_initialized = qtrue;
}


//=======================================================

#ifdef __ANDROID__
#include "touch_controls.h"

static qhandle_t tc_circleShader;
static qhandle_t tc_ringShader;
static qhandle_t tc_pillShader;

// All touch UI is drawn in RAW SCREEN PIXELS (re.DrawStretchPic takes screen
// coords) so it is independent of the 2D placement/letterbox state. Normalized
// inputs (0..1) are scaled by vidWidth/vidHeight; round elements use vidWidth
// for both axes so they stay circular on any aspect ratio.

// Centered text label drawn in screen pixels via the small console font.
static void CL_DrawTouchLabel( float ncx, float ncy, const char *label, float *color )
{
	float W = cls.glconfig.vidWidth, H = cls.glconfig.vidHeight;
	float size = H * 0.028f;          // ~3% of height
	float lx, ly;
	if ( !label || !*label ) return;
	lx = ncx * W - strlen( label ) * size * 0.5f;
	ly = ncy * H - size * 0.5f;
	re.SetColor( color );
	{
		const char *s = label;
		float x = lx;
		while ( *s ) {
			int ch = *s & 255;
			float fcol = ( ch & 15 ) / 16.0f;
			float frow = ( ch >> 4 ) / 16.0f;
			re.DrawStretchPic( x, ly, size, size, fcol, frow, fcol + 0.0625f, frow + 0.0625f,
				cls.charSetShader );
			x += size;
			s++;
		}
	}
	re.SetColor( NULL );
}

static void CL_DrawTouchShader( float ncx, float ncy, float nr, qhandle_t shader,
	float cr, float cg, float cb, float alpha )
{
	float W = cls.glconfig.vidWidth, H = cls.glconfig.vidHeight;
	// device (physical) aspect so the button is circular on the screen even if
	// the in-game render resolution differs from the device resolution.
	float da = ( touchControl.deviceW > 0 && touchControl.deviceH > 0 )
		? touchControl.deviceW / touchControl.deviceH
		: ( H > 0 ? W / H : 2.0f );
	float hw = nr * W;            // render-space half-width
	float hh = nr * H * da;       // render-space half-height (round on device)
	float col[4];
	col[0] = cr; col[1] = cg; col[2] = cb; col[3] = alpha;
	re.SetColor( col );
	re.DrawStretchPic( ncx * W - hw, ncy * H - hh, 2.0f * hw, 2.0f * hh, 0, 0, 1, 1, shader );
	re.SetColor( NULL );
}

// LCARS stadium "pill" button with normalized half-extents + black label.
// Pressed state = flat swap of the fill to the hopbush highlight (#cc6699),
// the convention LCARS recreations use — no glow/scale effects.
// Labels are uppercase, anchored to the bottom-right inside the end cap,
// matching the LCARS right-aligned/bottom-aligned text grammar.
static void CL_DrawTouchPill( float ncx, float ncy, float nhw, float nhh, qboolean pressed,
	const char *label, float cr, float cg, float cb )
{
	float W = cls.glconfig.vidWidth, H = cls.glconfig.vidHeight;
	float black[4] = { 0, 0, 0, 1 };
	float col[4];
	if ( pressed ) {
		col[0] = 0.80f; col[1] = 0.40f; col[2] = 0.60f; col[3] = 1.0f;   // LCARS_HOPBUSH
	} else {
		col[0] = cr; col[1] = cg; col[2] = cb; col[3] = 0.82f;
	}
	re.SetColor( col );
	re.DrawStretchPic( ( ncx - nhw ) * W, ( ncy - nhh ) * H, 2.0f * nhw * W, 2.0f * nhh * H,
		0, 0, 1, 1, tc_pillShader );
	re.SetColor( NULL );
	if ( label && *label ) {
		// Fit-to-button label: glyphs are square cells. Earlier the cell size was
		// keyed only to screen HEIGHT while the pill width scales with screen
		// WIDTH, so on less-wide aspect ratios long labels (SWITCH/CROUCH)
		// overflowed the pill. Size the text to the pill's flat interior (width
		// between the two round end caps) AND its height, then center it both
		// axes so it always sits cleanly inside the button at any resolution.
		int   n      = (int)strlen( label );
		float pillW  = 2.0f * nhw * W;
		float pillH  = 2.0f * nhh * H;
		float cap    = nhh * H;                       // end-cap radius = half height
		float avail  = pillW - 2.0f * cap;            // flat interior between caps
		float size   = H * 0.030f;                    // preferred glyph cell
		float lx, ly;
		const char *s;
		if ( avail < pillW * 0.5f ) avail = pillW * 0.5f;  // guard stubby pills
		if ( n > 0 && n * size > avail ) size = avail / n; // shrink to fit width
		if ( size > pillH * 0.72f )      size = pillH * 0.72f; // and to fit height
		lx = ncx * W - n * size * 0.5f;               // centered horizontally
		ly = ncy * H - size * 0.5f;                   // centered vertically
		s = label;
		re.SetColor( black );
		while ( *s ) {
			int ch = *s & 255;
			float fcol = ( ch & 15 ) / 16.0f;
			float frow = ( ch >> 4 ) / 16.0f;
			re.DrawStretchPic( lx, ly, size, size, fcol, frow, fcol + 0.0625f, frow + 0.0625f,
				cls.charSetShader );
			lx += size;
			s++;
		}
		re.SetColor( NULL );
	}
}

static qhandle_t tc_arrowShader[4];

// Registered once at renderer-init time (NOT mid-frame — registering a shader
// during the 2D draw pass uploads its texture at the wrong time and corrupts it).
void CL_TouchRegisterShaders( void )
{
	tc_circleShader   = re.RegisterShaderNoMip( "gfx/touch/circle" );
	tc_ringShader     = re.RegisterShaderNoMip( "gfx/touch/ring" );
	tc_pillShader     = re.RegisterShaderNoMip( "gfx/touch/pill" );
	tc_arrowShader[0] = re.RegisterShaderNoMip( "gfx/touch/arrow_up" );
	tc_arrowShader[1] = re.RegisterShaderNoMip( "gfx/touch/arrow_down" );
	tc_arrowShader[2] = re.RegisterShaderNoMip( "gfx/touch/arrow_left" );
	tc_arrowShader[3] = re.RegisterShaderNoMip( "gfx/touch/arrow_right" );
	// 0 = image failed to load (RegisterShaderNoMip returns 0 on default
	// shader). This happens when the FS turns PURE on local-map start and the
	// images are only available as loose files — they are therefore shipped in
	// zpak-android.pk3 (extracted with the other assets), which the pure list
	// includes. If registration still fails, warn loudly and fall back to the
	// plain white shader so the overlay degrades to visible blocks instead of
	// silently vanishing.
	if ( !tc_circleShader || !tc_ringShader || !tc_pillShader ) {
		Com_Printf( S_COLOR_RED "WARNING: touch overlay images missing "
			"(circle=%d ring=%d pill=%d) — using white fallback. "
			"Is zpak-android.pk3 present in %s?\n",
			tc_circleShader, tc_ringShader, tc_pillShader, BASEGAME );
	}
	if ( !tc_circleShader )   tc_circleShader   = cls.whiteShader;
	if ( !tc_ringShader )     tc_ringShader     = cls.whiteShader;
	if ( !tc_pillShader )     tc_pillShader     = cls.whiteShader;
	if ( !tc_arrowShader[0] ) tc_arrowShader[0] = cls.whiteShader;
	if ( !tc_arrowShader[1] ) tc_arrowShader[1] = cls.whiteShader;
	if ( !tc_arrowShader[2] ) tc_arrowShader[2] = cls.whiteShader;
	if ( !tc_arrowShader[3] ) tc_arrowShader[3] = cls.whiteShader;
}

/*
==================
CL_DrawTouchControls   (Android, in-game, no controller) — LCARS styled
==================
*/
void CL_DrawTouchControls( void ) {
	float padCx, padCy;
	int i;

	// touch controls stay available even with a gamepad connected — the two
	// input methods coexist (user request; gamepad no longer hides touch UI)
	if ( !tc_pillShader )
		return;

	// Auto-hide after 4s without touch so controller players aren't
	// distracted. A finger actively holding the stick or a button keeps the
	// overlay visible even when perfectly still (a held finger emits no
	// events, so the timestamp alone is not enough); any new tap brings the
	// overlay back instantly.
	// Auto-hide keys off a finger PHYSICALLY HELD (moveActive / actionFingerHeld), NOT actionPressed --
	// the crouch-toggle leaves its actionPressed bit set with no tracked finger, which used to pin the
	// overlay on screen forever (user-confirmed: toggle+untoggle crouch made it hide again).
	if ( !touchControl.moveActive && !touchControl.actionFingerHeld
		&& touchControl.lastTouchMs
		&& Sys_Milliseconds() - touchControl.lastTouchMs > 4000 )
		return;

	// --- movement stick: LCARS orange ring + gold thumb ---
	if ( touchControl.moveActive ) {
		padCx = touchControl.padCx; padCy = touchControl.padCy;
	} else {
		padCx = TC_PAD_CX; padCy = TC_PAD_CY;
	}
	CL_DrawTouchShader( padCx, padCy, TC_PAD_R, tc_ringShader, LCARS_ORANGE,
		touchControl.moveActive ? 0.7f : 0.45f );
	CL_DrawTouchShader(
		touchControl.moveActive ? touchControl.thumbX : padCx,
		touchControl.moveActive ? touchControl.thumbY : padCy,
		TC_PAD_R * 0.45f, tc_circleShader, LCARS_GOLD,
		touchControl.moveActive ? 1.0f : 0.7f );

	// --- action pills (FIRE / ALT / JUMP / DUCK / WEAP) ---
	for ( i = 0; i < TC_NUM_ACTION; i++ ) {
		const tcActionButton_t *ab = &tcActionButtons[i];
		qboolean pressed = ( touchControl.actionPressed & ( 1 << i ) ) != 0;
		CL_DrawTouchPill( ab->cx, ab->cy, ab->hw, ab->hh, pressed, ab->label,
			ab->r, ab->g, ab->b );
	}
}

/*
==================
CL_DrawTouchMenuNav

On-screen menu-navigation buttons (Android): D-pad + Select + Back. Drawn only
while a menu is open and no controller is connected.
==================
*/
void CL_DrawTouchMenuNav( void ) {
	int i;

	// On-screen menu nav buttons removed (user request 2026-06-06): menus are
	// navigated by direct tap (touch-as-mouse) and/or gamepad, so the overlay
	// only added clutter. The tcNavButtons table stays for possible revival.
	return;

	// drawn even with a gamepad connected (touch + gamepad coexist)
	if ( !tc_circleShader )
		return;

	// auto-hide the menu nav buttons after 3s with no touch
	if ( touchControl.lastTouchMs && Sys_Milliseconds() - touchControl.lastTouchMs > 3000 )
		return;

	for ( i = 0; i < TC_NUM_NAV; i++ ) {
		const tcNavButton_t *b = &tcNavButtons[i];
		qboolean pressed = ( touchControl.menuNavPressed == i );
		int      ai = -1;

		if      ( b->key == K_UPARROW )    ai = 0;
		else if ( b->key == K_DOWNARROW )  ai = 1;
		else if ( b->key == K_LEFTARROW )  ai = 2;
		else if ( b->key == K_RIGHTARROW ) ai = 3;

		if ( b->shape == TC_SHAPE_PILL ) {
			// OK = LCARS gold, BACK = LCARS salmon
			if ( b->key == K_ENTER )
				CL_DrawTouchPill( b->cx, b->cy, b->hw, b->hh, pressed, b->label, LCARS_GOLD );
			else
				CL_DrawTouchPill( b->cx, b->cy, b->hw, b->hh, pressed, b->label, LCARS_SALMON );
		} else {
			// LCARS orange round d-pad button + white arrow glyph
			CL_DrawTouchShader( b->cx, b->cy, b->hw, tc_circleShader, LCARS_ORANGE,
				pressed ? 1.0f : 0.8f );
			if ( ai >= 0 )
				CL_DrawTouchShader( b->cx, b->cy, b->hw * 0.62f, tc_arrowShader[ai],
					0.0f, 0.0f, 0.0f, pressed ? 1.0f : 0.9f );  // black arrow on orange
		}
	}
}
#endif

/*
==================
SCR_DrawScreenField

This will be called twice if rendering in stereo mode
==================
*/
void SCR_DrawScreenField( stereoFrame_t stereoFrame ) {
	qboolean uiFullscreen;

	re.BeginFrame( stereoFrame );

	// EF1 SP: guard against an intermittent stuck load (CA_ACTIVE but the SP cgame never came up ->
	// black screen with only the touch overlay). No-op in the happy path; retries a starved efsptrans
	// then falls back to the main menu.
	{ extern void SP_LoadWatchdog( void ); SP_LoadWatchdog(); }

	// EF1 SP: during a map load (spmap -> efsp), the SP cgame is torn down / not yet inited,
	// so the parallel HM cgame+ui would draw the "SOLO HOLOMATCH" loading splash across the
	// CA_CONNECTING..CA_ACTIVE window. Suppress all of that and show the real per-map SP
	// levelshot instead (cold-boot New Game, maptransition, loadtransition, load all hit this).
	if ( SP_IsLoading() ) {
		SP_DrawLoadingScreen();
		Con_DrawConsole();        // keep the console reachable during load
		return;                   // re.EndFrame() is issued by the SCR_UpdateScreen caller
	}

	uiFullscreen = SPUI_IsActive() ? SPUI_IsFullscreen() : (uivm && VM_Call( uivm, UI_IS_FULLSCREEN ));

	// wide aspect ratio screens need to have the sides cleared
	// unless they are displaying game renderings
	if ( uiFullscreen || clc.state < CA_LOADING
#ifdef USE_FLEXIBLE_DISPLAY
	  || ( cl_flexibleDisplay->integer && ( clc.state != CA_ACTIVE || cl_viewmode->integer == 1 ) )
#endif
	  ) {
		if ( cls.screenXBias || cls.screenYBias ) {
			int left = cls.screenXBias + 0.5f;
			int right = cls.glconfig.vidWidth - left;
			int top = cls.screenYBias + 0.5f;
			int bottom = cls.glconfig.vidHeight - top;

			re.SetColor( g_color_table[0] );

			if ( cls.screenXBias ) {
				// clear left
				re.DrawStretchPic( 0, 0, left, cls.glconfig.vidHeight, 0, 0, 0, 0, cls.whiteShader );

				// clear right
				re.DrawStretchPic( right, 0, cls.glconfig.vidWidth - right, cls.glconfig.vidHeight, 0, 0, 0, 0, cls.whiteShader );
			}

			if ( cls.screenYBias ) {
				// clear top
				re.DrawStretchPic( left, 0, right - left, top, 0, 0, 0, 0, cls.whiteShader );

				// clear bottom
				re.DrawStretchPic( left, bottom, right - left, cls.glconfig.vidHeight - bottom, 0, 0, 0, 0, cls.whiteShader );
			}

			re.SetColor( NULL );
		}
	}

	SCR_SetScreenPlacement( SCR_VERT_STRETCH | SCR_HOR_STRETCH );

	// if the menu is going to cover the entire screen, we
	// don't need to render anything under it
	if ( ( uivm || SPUI_IsActive() ) && !uiFullscreen ) {
		switch( clc.state ) {
		default:
			Com_Error( ERR_FATAL, "SCR_DrawScreenField: bad clc.state" );
			break;
		case CA_CINEMATIC:
			SCR_DrawCinematic();
			break;
		case CA_DISCONNECTED:
			// force menu up
			S_StopAllSounds();
			if ( SPUI_IsActive() ) SPUI_SetActiveMenu( "main" );
			else VM_Call( uivm, UI_SET_ACTIVE_MENU, UIMENU_MAIN );
#ifdef USE_FLEXIBLE_DISPLAY
			cls.syncUICursor = qtrue;
#endif
			break;
		case CA_CONNECTING:
		case CA_CHALLENGING:
		case CA_CONNECTED:
			// connecting clients will only show the connection dialog
			// refresh to update the time
			if ( SPUI_IsActive() ) { SPUI_Refresh( cls.realtime ); SPUI_DrawConnect( "", "" ); }
			else { VM_Call( uivm, UI_REFRESH, cls.realtime ); VM_Call( uivm, UI_DRAW_CONNECT_SCREEN, qfalse ); }
			break;
		case CA_LOADING:
		case CA_PRIMED:
			// draw the game information screen and loading progress
			CL_CGameRendering(stereoFrame);

			// also draw the connection information, so it doesn't
			// flash away too briefly on local or lan games
			// refresh to update the time
			if ( SPUI_IsActive() ) { SPUI_Refresh( cls.realtime ); SPUI_DrawConnect( "", "" ); }
			else { VM_Call( uivm, UI_REFRESH, cls.realtime ); VM_Call( uivm, UI_DRAW_CONNECT_SCREEN, qtrue ); }
			break;
		case CA_ACTIVE:
			// always supply STEREO_CENTER as vieworg offset is now done by the engine.
			CL_CGameRendering(stereoFrame);
			// EF1 SP 1:1: an in-game cinematic (inGameCinematic / CIN_inGame) stays CA_ACTIVE
			// and overlays the FMV on the live (paused) game — retail retail:2923-2931
			// never enters CA_CINEMATIC for it. The world was just drawn (frozen, since the
			// SP bridge holds the sim while cl_paused); draw the video stretched over it.
			if ( CL_InGameCinematicActive() ) {
				SCR_DrawCinematic();
			}
#ifdef USE_FLEXIBLE_DISPLAY
			if ( cl_flexibleDisplay->integer ) {
				if ( cl_viewmode->integer <= 2 ) {
					SCR_SetScreenPlacement( SCR_VERT_CENTER | SCR_HOR_CENTER );
				} else if ( cl_viewmode->integer == 3 ) {
					SCR_SetScreenPlacement( SCR_VERT_TOP | SCR_HOR_CENTER );
				} else {
					SCR_SetScreenPlacement( SCR_VERT_STRETCH | SCR_HOR_STRETCH );
				}
			}
#endif
			SCR_DrawDemoRecording();
#ifdef USE_VOIP
			SCR_DrawVoipMeter();
#endif
			break;
		}
	}

#ifdef __ANDROID__
	// on-screen touch controls (in-game, no menu/console up). Gate must stay
	// IDENTICAL to IN_TouchControlsActive in sdl_input.c — including hiding them
	// under an in-game cinematic overlay (a tap there skips the FMV, it is not a HUD press).
	if ( clc.state == CA_ACTIVE && !( Key_GetCatcher( ) & ( KEYCATCH_CONSOLE | KEYCATCH_UI ) )
	     && !CL_InGameCinematicActive( )
	     && !Cvar_VariableIntegerValue( "cg_cameraActive" ) ) {   // hide during a scripted CGCam cutscene
		CL_DrawTouchControls( );
	}
#endif

	// the menu draws next
	if ( Key_GetCatcher( ) & KEYCATCH_UI && ( uivm || SPUI_IsActive() ) ) {
#ifdef USE_FLEXIBLE_DISPLAY
		if ( cls.syncUICursor ) {
			IN_SyncMousePosition();
			cls.syncUICursor = qfalse;
		}
#endif
		if ( SPUI_IsActive() ) SPUI_Refresh( cls.realtime );
		else VM_Call( uivm, UI_REFRESH, cls.realtime );
#ifdef __ANDROID__
		// menu-navigation buttons on top of the menu
		CL_DrawTouchMenuNav( );
#endif
	} else {
#ifdef USE_FLEXIBLE_DISPLAY
		cls.syncUICursor = qtrue;
#endif
	}

	// console draws next
	Con_DrawConsole ();

	// debug graph can be drawn on top of anything
	if ( cl_debuggraph->integer || cl_timegraph->integer || cl_debugMove->integer ) {
		SCR_DrawDebugGraph ();
	}
}

/*
==================
SCR_UpdateScreen

This is called every frame, and can also be called explicitly to flush
text to the screen.
==================
*/
void SCR_UpdateScreen( void ) {
	static int	recursive;

	if ( !scr_initialized ) {
		return;				// not initialized yet
	}

	if ( ++recursive > 2 ) {
		Com_Error( ERR_FATAL, "SCR_UpdateScreen: recursively called" );
	}
	recursive = 1;

	// If there is no VM, there are also no rendering commands issued. Stop the renderer in
	// that case. EF1 SP: the native SP UI has NO uivm (uivm==NULL) but DOES render via SPUI_*,
	// so SPUI_IsActive() must also enable the render path or the whole screen stays black.
	if( uivm || SPUI_IsActive() || com_dedicated->integer )
	{
		// XXX
		int in_anaglyphMode = Cvar_VariableIntegerValue("r_anaglyphMode");
		// if running in stereo, we need to draw the frame twice
		if ( cls.glconfig.stereoEnabled || in_anaglyphMode) {
			SCR_DrawScreenField( STEREO_LEFT );
			SCR_DrawScreenField( STEREO_RIGHT );
		} else {
			SCR_DrawScreenField( STEREO_CENTER );
		}

		if ( com_speeds->integer ) {
			re.EndFrame( &time_frontend, &time_backend );
		} else {
			re.EndFrame( NULL, NULL );
		}
	}
	
	recursive = 0;
}

