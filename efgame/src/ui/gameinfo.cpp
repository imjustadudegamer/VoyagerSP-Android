//
// gameinfo.c
//

// *** This file is used by both the game and the user interface ***


#include "gameinfo.h"
#include "../game/weapons.h"


// In the UI build (_USRDLL) g_weaponLoad.cpp does `extern gameinfo_import_t gi;`, so this global
// must be named `gi` there. In the game build it was renamed `gameinfo_gi` to avoid colliding with
// g_main's `game_import_t gi`. GI_Init below assigns the matching name. (Both halves required.)
#ifdef _USRDLL
gameinfo_import_t	gi;
#else
gameinfo_import_t	gameinfo_gi;
#endif

weaponData_t weaponData[WP_NUM_WEAPONS];
ammoData_t ammoData[AMMO_MAX];

extern void WP_LoadWeaponParms (void);

//
// Initialization - Read in files and parse into infos
//

/*
===============
GI_Init
===============
*/
void GI_Init( gameinfo_import_t *import ) {
#ifdef _USRDLL
	gi = *import;			// UI build: same symbol WP_LoadWeaponParms reads
#else
	gameinfo_gi = *import;
#endif

	WP_LoadWeaponParms ();
}
