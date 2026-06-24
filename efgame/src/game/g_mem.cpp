//
// g_mem.c
//


#include "g_local.h"


// This is a single, never-freed bump allocator: g_entities
// (MAX_GENTITIES * sizeof(gentity_t)) is carved out up front, and the rest
// feeds per-spawn NPC/client/parm allocs for the whole map. The original 2MB
// pool was sized for a 32-bit build; on 64-bit the wider pointers make
// gentity_t and the per-spawn structs larger, so g_entities takes a much
// bigger bite and leaves far less gameplay headroom -- spawn-heavy/long maps
// (e.g. voy16) then hit "G_Alloc: failed" and ERR_DROP back to the main menu.
// Enlarge the pool so the 64-bit build keeps comfortably more headroom. No
// logic change.
#define POOLSIZE	(4 * 1024 * 1024)

static char		memoryPool[POOLSIZE];
static int		allocPoint;
static cvar_t	*g_debugalloc;

void *G_Alloc( int size ) {
	char	*p;

	if ( g_debugalloc->integer ) {
		gi.Printf( "G_Alloc of %i bytes (%i left)\n", size, POOLSIZE - allocPoint - ( ( size + 31 ) & ~31 ) );
	}

	if ( allocPoint + size > POOLSIZE ) {
		G_Error( "G_Alloc: failed on allocation of %u bytes\n", size );
		return NULL;
	}

	p = &memoryPool[allocPoint];

	allocPoint += ( size + 31 ) & ~31;

	return p;
}

void G_InitMemory( void ) {
	allocPoint = 0;
	g_debugalloc = gi.cvar ("g_debugalloc", "0", 0);
}

void Svcmd_GameMem_f( void ) {
	gi.Printf( "Game memory status: %i out of %i bytes allocated\n", allocPoint, POOLSIZE );
}
