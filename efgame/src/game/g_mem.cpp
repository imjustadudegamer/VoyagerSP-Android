//
// g_mem.c
//


#include "g_local.h"


#define POOLSIZE	(2 * 1024 * 1024)

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
