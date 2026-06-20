#ifndef __G_ROFF_H__
#define __G_ROFF_H__


#include "q_shared.h"


// ROFF Defines
//-------------------
#define ROFF_VERSION		1	// ver # for the (R)otation (O)bject (F)ile (F)ormat
#define MAX_ROFFS			16	// hard coded number of max roffs per level, sigh..
#define ROFF_SAMPLE_RATE	10	// 10hz


// ROFF Header file definition
//-------------------------------
typedef struct roff_hdr_s
{
	char	sHeader[4];		// should be "ROFF" (Rotation, Origin File Format)
	int		lVersion;		// on-disk field is 4 bytes; retail used `long` which is 4 on Win32/ILP32
							// but 8 on arm64/LP64 -- that bloated this header from 12 to 24 bytes and
							// broke the raw-cast parse (header validation failed -> ROFFs never played
							// -> e.g. voy8 vent door never opened). Keep it width-explicit (4 bytes).
	float	fCount;			// There isn't any reason for this to be anything other than an int, sigh...
		//
		//		Move - Rotate data follows....vec3_t delta_origin, vec3_t delta_rotation
		//
} roff_hdr_t;

#ifdef __cplusplus
// The struct is cast directly over the raw .rof file bytes, so its size MUST stay 12.
static_assert( sizeof(roff_hdr_t) == 12, "roff_hdr_t must match the 12-byte on-disk ROFF header" );
#endif


// ROFF move rotate data element
//--------------------------------
typedef struct move_rotate_s
{
	vec3_t	origin_delta;
	vec3_t	rotate_delta;

} move_rotate_t;


// a precached ROFF list
//-------------------------
typedef struct roff_list_s
{
	char			*fileName;	// roff filename
	int				frames;		// number of roff entries
	move_rotate_t	*data;		// delta move and rotate vector list

} roff_list_t;


extern roff_list_t roffs[];
extern int num_roffs;


// Function prototypes
//-------------------------
int		G_LoadRoff( const char *fileName );
void	G_Roff( gentity_t *ent );
void	G_SaveCachedRoffs();
void	G_LoadCachedRoffs();

#endif