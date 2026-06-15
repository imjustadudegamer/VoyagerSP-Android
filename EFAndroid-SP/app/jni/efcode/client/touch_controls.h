/*
===========================================================================
Android on-screen touch controls (LCARS styled), shared between the input
layer (code/sdl/sdl_input.c) and the drawing layer (code/client/cl_scrn.c).
Positions are normalized screen coords (0..1) so they scale to any device.
Round elements are aspect-corrected (circular on screen); pill (stadium)
elements use explicit normalized half-extents.
===========================================================================
*/
#ifndef TOUCH_CONTROLS_H
#define TOUCH_CONTROLS_H

#ifdef __ANDROID__

// --- in-game: round move stick (left) ---
#define TC_PAD_CX   0.135f
#define TC_PAD_CY   0.70f
#define TC_PAD_R    0.060f

// LCARS "classic theme" palette — consensus values verified across
// thelcars.com/colors.php, the trekcolors set and the joernweissenborn/lcars
// framework (no official Okuda/Paramount hex spec exists; these are the
// convergent fan-recreation values).
#define LCARS_ORANGE       1.00f, 0.60f, 0.00f   // golden-orange #ff9900
#define LCARS_GOLD         1.00f, 0.667f, 0.00f  // gold #ffaa00
#define LCARS_SUNFLOWER    1.00f, 0.80f, 0.60f   // sunflower #ffcc99
#define LCARS_ICE          0.60f, 0.80f, 1.00f   // ice #99ccff
#define LCARS_BUTTERSCOTCH 1.00f, 0.60f, 0.40f   // butterscotch #ff9966
#define LCARS_VIOLET       0.80f, 0.60f, 1.00f   // african-violet #cc99ff
#define LCARS_MARS         1.00f, 0.133f, 0.00f  // mars #ff2200
#define LCARS_HOPBUSH      0.80f, 0.40f, 0.60f   // hopbush #cc6699 — pressed/active swap
// legacy aliases still used by the stick drawing
#define LCARS_BLUE         LCARS_ICE
#define LCARS_SALMON       LCARS_BUTTERSCOTCH
#define LCARS_LILAC        LCARS_VIOLET
#define LCARS_RED          LCARS_MARS

#define TC_SHAPE_ROUND 0
#define TC_SHAPE_PILL  1

typedef struct {
	float cx, cy;        // center (normalized)
	float hw, hh;        // round: hw = radius (hh ignored); pill: half-extents
	int   shape;
	int   key;
	const char *label;
} tcNavButton_t;

typedef struct {
	float cx, cy, hw, hh; // pill half-extents (normalized)
	int   key;
	const char *label;
	float r, g, b;
	qboolean tap;         // tap = momentary (weapon switch); else hold
	qboolean aim;         // dragging while held also drives look (fire/alt-fire)
} tcActionButton_t;

typedef struct {
	qboolean controllerConnected;
	qboolean moveActive;
	float    padCx, padCy;
	float    thumbX, thumbY;
	int      actionPressed;   // bitmask over tcActionButtons (incl. the STICKY crouch-toggle bit)
	qboolean actionFingerHeld;// an action button is held by a REAL finger right now (excludes the
	                          // crouch-toggle, which sets actionPressed but tracks no finger). The
	                          // auto-hide keys off THIS, not actionPressed, so a crouch-toggle doesn't
	                          // pin the overlay on screen forever.
	int      menuNavPressed;  // index into tcNavButtons, -1 = none
	int      lastTouchMs;     // Sys_Milliseconds of last touch; touch UI auto-hides after idle
	float    deviceW, deviceH; // PHYSICAL screen size (px); touch UI is anchored to
	                           // this, not the in-game render resolution, so buttons
	                           // don't move/distort when the game resolution changes.
} touchControlState_t;

// In-game action buttons (right side). FIRE/ALT/JUMP/DUCK are held; WEAP is a tap.
// Keys reuse the gamepad PAD0_* binds set in autoexec.cfg (+altattack/+movedown/etc).
//
// Layout follows the Microsoft GDK Touch Adaptation Kit slot model (the guide
// distilled from 200+ touch-adapted titles): the right inner wheel holds ONLY
// the primary action (FIRE), large hit area, kept INBOARD of the screen edge so
// drag-to-aim swipes have room and the EF HUD's bottom-right ammo arc stays
// visible; secondary actions (ALT/WEAP) sit in the upper-outside slots along
// the right edge above the HUD; tertiary jump-like actions (JUMP/DUCK) sit
// directly below the inner ring, kept out of the far bottom-right corner.
// USE sits inboard-left of the FIRE wheel, above the JUMP/CROUCH row, so it is
// reachable by the right thumb without colliding with FIRE's drag-to-aim area.
// It is a HOLD button on the gamepad X face button (PAD0_X -> +use in
// android_defaults.cfg), so doors / consoles / turbolifts / mission-computers
// are operable on touch. ALT therefore moves to the left trigger keycode
// (PAD0_LEFTTRIGGER, which is bound to +altattack), keeping alt-fire on touch.
static const tcActionButton_t tcActionButtons[] = {
	{ 0.855f, 0.770f, 0.062f, 0.050f, K_MOUSE1,             "FIRE",   LCARS_MARS,      qfalse, qtrue  },
	{ 0.950f, 0.640f, 0.042f, 0.034f, K_PAD0_LEFTTRIGGER,   "ALT",    LCARS_ORANGE,    qfalse, qtrue  },
	{ 0.730f, 0.775f, 0.046f, 0.034f, K_PAD0_X,             "USE",    LCARS_GOLD,      qfalse, qfalse },
	{ 0.790f, 0.930f, 0.050f, 0.038f, K_PAD0_A,             "JUMP",   LCARS_ICE,       qfalse, qfalse },
	{ 0.665f, 0.930f, 0.054f, 0.034f, K_PAD0_B,             "CROUCH", LCARS_VIOLET,    qfalse, qfalse },
	{ 0.950f, 0.520f, 0.054f, 0.034f, K_PAD0_RIGHTSHOULDER, "SWITCH", LCARS_SUNFLOWER, qtrue,  qfalse },
	// MISSION info: left side, above the move stick. HOLD to show mission objectives, release to hide.
	// Reuses K_PAD0_BACK, which android_defaults.cfg binds to "+info"/"-info" (cgame CG_InfoDown/Up_f).
	{ 0.135f, 0.520f, 0.062f, 0.034f, K_PAD0_BACK,          "MISSION", LCARS_BUTTERSCOTCH, qfalse, qfalse },
};
#define TC_NUM_ACTION ( (int)( sizeof( tcActionButtons ) / sizeof( tcActionButtons[0] ) ) )

// Menu navigation: 4-way arrows + OK / BACK (LCARS pills), laid out as a single
// bottom strip BELOW the menu's footer bar — the one band of screen the
// stretched 16:9 menus leave free. The old layout sat on top of the menu's
// left sidebar tabs and right-edge APPLY/column widgets. UP/DOWN move the
// cursor; LEFT/RIGHT change spin-control values (stock EF menu semantics).
// One button active at a time (4-way exclusive, per the GDK touch guide).
static const tcNavButton_t tcNavButtons[] = {
	{ 0.300f, 0.940f, 0.026f, 0.0f,   TC_SHAPE_ROUND, K_LEFTARROW,  ""     },
	{ 0.385f, 0.940f, 0.026f, 0.0f,   TC_SHAPE_ROUND, K_UPARROW,    ""     },
	{ 0.470f, 0.940f, 0.026f, 0.0f,   TC_SHAPE_ROUND, K_DOWNARROW,  ""     },
	{ 0.555f, 0.940f, 0.026f, 0.0f,   TC_SHAPE_ROUND, K_RIGHTARROW, ""     },
	{ 0.700f, 0.945f, 0.052f, 0.030f, TC_SHAPE_PILL,  K_ESCAPE,     "BACK" },
	{ 0.845f, 0.945f, 0.052f, 0.030f, TC_SHAPE_PILL,  K_ENTER,      "OK"   },
};
#define TC_NUM_NAV ( (int)( sizeof( tcNavButtons ) / sizeof( tcNavButtons[0] ) ) )

extern touchControlState_t touchControl;

#endif // __ANDROID__
#endif // TOUCH_CONTROLS_H
