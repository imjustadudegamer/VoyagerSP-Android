#include "b_local.h"
#include "boltons.h"
//#include "b_public.h"

//extern void G_ParseBoltOnList( boltOn_t *boltOn );
extern gentity_t *G_CreateObject ( gentity_t *owner, vec3_t origin, vec3_t angles, int modelIndex, int frame, trType_t trType );
extern qboolean G_ParseLiteral( char **data, const char *string );
extern qboolean G_ParseString( char **data, char **s ) ;
extern qboolean G_ParseInt( char **data, int *i );
extern qboolean G_ParseFloat( char **data, float *f );

#define BOLTON_NONE MAX_BOLT_ONS

boltOn_t	knownBoltOns[MAX_GAME_BOLTONS];
int			numBoltOns;
char	boltOnList[0x10000];

int G_GetBoltOnIndex( const char *boltOnName ) 
{
	for ( int i = 0; i < numBoltOns; i++ )
	{
		if ( Q_stricmp( knownBoltOns[i].name, boltOnName ) == 0 )
		{//found it!
			return i;
		}
	}

	G_Error( "ERROR: Unknown boltOn name: %s!\n", boltOnName );

	return MAX_GAME_BOLTONS;
}

void G_RegisterBoltOns (void)
{
	boltOn_t *boltOn = NULL;
	char	*token = NULL;
	char	*value;
	char	*p;
	int		n;
	float	f;

	p = boltOnList;
	COM_BeginParseSession();

	while(1)
	{
		if ( numBoltOns >= MAX_GAME_BOLTONS )
		{
			G_Error( "ERROR:  Too many boltOns (%d) in boltOn.cfg!\n", MAX_GAME_BOLTONS );
			return;
		}

		boltOn = &knownBoltOns[numBoltOns];

		while ( 1 )
		{
			token = COM_ParseExt( &p, qtrue );
			if ( token[0] == 0 )
			{
				//reached EOF
				return;
			}
			else if ( token[0] == '{' )
			{
				//WTF?  Unnamed boltOn?
				continue;
			}
			else if ( !p ) 
			{
				//???
				return;
			}
			else
			{
				//found a boltOn name
				break;
			}
		}

		Q_strncpyz( (char *)&boltOn->name, token, sizeof(boltOn->name), qtrue );

		if ( G_ParseLiteral( &p, "{" ) ) 
		{//couldn't find an open brace "{"!
			return;
		}

		//Set default scale
		VectorSet(boltOn->model.scaleXYZ, 100, 100, 100);
			
		// parse the boltOn info block
		while ( 1 ) 
		{
			token = COM_ParseExt( &p, qtrue );
			if ( !token[0] ) 
			{
				gi.Printf( S_COLOR_RED"ERROR: unexpected EOF while parsing '%s'\n", boltOn->name );
				return;
			}

			if ( !Q_stricmp( token, "}" ) ) 
			{
				//reached end of boltOn, on to next
				break;
			}

			// modelName
			if ( !Q_stricmp( token, "modelName" ) ) 
			{
				if ( G_ParseString( &p, &value ) ) 
				{
					continue;
				}
				boltOn->model.modelIndex = G_ModelIndex( value );
				continue;
			}
			
			// target model to attach to, head, torso, legs or weapon
			if ( !Q_stricmp( token, "targetModel" ) ) 
			{
				if ( G_ParseInt( &p, &n ) ) 
				{
					continue;
				}

				if ( n < 0 || n >= NUM_TARGET_MODELS )
				{
					gi.Printf("WARNING boltOn %s has targetModel out of range (<0 or >= %d)\n", boltOn->name, NUM_TARGET_MODELS);
					return;
				}

				boltOn->targetModel = (targetModel_t)n;			
				continue;
			}

			// target tag to attach to
			if ( !Q_stricmp( token, "targetTag" ) ) 
			{
				if ( G_ParseString( &p, &value ) ) 
				{
					continue;
				}
				//boltOn->targetTag = G_NewString(value);
				Q_strncpyz( boltOn->targetTag, value, sizeof(boltOn->targetTag), qtrue);
				continue;
			}

			// pitch offset
			if ( !Q_stricmp( token, "pitchOffset" ) ) 
			{
				if ( G_ParseFloat( &p, &f ) ) 
				{
					continue;
				}
				boltOn->angleOffsets[0] = f;
				continue;
			}

			// yaw offset
			if ( !Q_stricmp( token, "yawOffset" ) ) 
			{
				if ( G_ParseFloat( &p, &f ) ) 
				{
					continue;
				}
				boltOn->angleOffsets[1] = f;
				continue;
			}

			// roll offset
			if ( !Q_stricmp( token, "rollOffset" ) ) 
			{
				if ( G_ParseFloat( &p, &f ) ) 
				{
					continue;
				}
				boltOn->angleOffsets[2] = f;
				continue;
			}

			// x offset
			if ( !Q_stricmp( token, "xOffset" ) ) 
			{
				if ( G_ParseFloat( &p, &f ) ) 
				{
					continue;
				}
				boltOn->originOffsets[0] = f;
				continue;
			}

			// y offset
			if ( !Q_stricmp( token, "yOffset" ) ) 
			{
				if ( G_ParseFloat( &p, &f ) ) 
				{
					continue;
				}
				boltOn->originOffsets[1] = f;
				continue;
			}

			// z offset
			if ( !Q_stricmp( token, "zOffset" ) ) 
			{
				if ( G_ParseFloat( &p, &f ) ) 
				{
					continue;
				}
				boltOn->originOffsets[2] = f;
				continue;
			}

			// Uniform XYZ scale
			if ( !Q_stricmp( token, "scale" ) ) 
			{
				if ( G_ParseInt( &p, &n ) ) 
				{
					SkipRestOfLine( &p );
					continue;
				}
				if ( n < 0 ) 
				{
					gi.Printf(  "bad %s in boltOn '%s'\n", token, boltOn->name );
					continue;
				}
				boltOn->model.scaleXYZ[0] = boltOn->model.scaleXYZ[1] = boltOn->model.scaleXYZ[2] = n;
				continue;
			}

			//X scale
			if ( !Q_stricmp( token, "scaleX" ) ) 
			{
				if ( G_ParseInt( &p, &n ) ) 
				{
					SkipRestOfLine( &p );
					continue;
				}
				if ( n < 0 ) 
				{
					gi.Printf(  "bad %s in boltOn '%s'\n", token, boltOn->name );
					continue;
				}
				boltOn->model.scaleXYZ[0] = n;
				continue;
			}

			//Y scale
			if ( !Q_stricmp( token, "scaleY" ) ) 
			{
				if ( G_ParseInt( &p, &n ) ) 
				{
					SkipRestOfLine( &p );
					continue;
				}
				if ( n < 0 ) 
				{
					gi.Printf(  "bad %s in boltOn '%s'\n", token, boltOn->name );
					continue;
				}
				boltOn->model.scaleXYZ[1] = n;
				continue;
			}

			//Z scale
			if ( !Q_stricmp( token, "scaleZ" ) ) 
			{
				if ( G_ParseInt( &p, &n ) ) 
				{
					SkipRestOfLine( &p );
					continue;
				}
				if ( n < 0 ) 
				{
					gi.Printf(  "bad %s in boltOn '%s'\n", token, boltOn->name );
					continue;
				}
				boltOn->model.scaleXYZ[2] = n;
				continue;
			}
			
			// Uniform RGB tint
			if ( !Q_stricmp( token, "tint" ) ) 
			{
				if ( G_ParseInt( &p, &n ) ) 
				{
					SkipRestOfLine( &p );
					continue;
				}
				if ( n < 0 ) 
				{
					gi.Printf(  "bad %s in boltOn '%s'\n", token, boltOn->name );
					continue;
				}
				boltOn->model.customRGB[0] = boltOn->model.customRGB[1] = boltOn->model.customRGB[2] = n;
				continue;
			}

			//red tint
			if ( !Q_stricmp( token, "tintRed" ) ) 
			{
				if ( G_ParseInt( &p, &n ) ) 
				{
					SkipRestOfLine( &p );
					continue;
				}
				if ( n < 0 ) 
				{
					gi.Printf(  "bad %s in boltOn '%s'\n", token, boltOn->name );
					continue;
				}
				boltOn->model.customRGB[0] = n;
				continue;
			}

			//Green tint
			if ( !Q_stricmp( token, "tintGreen" ) ) 
			{
				if ( G_ParseInt( &p, &n ) ) 
				{
					SkipRestOfLine( &p );
					continue;
				}
				if ( n < 0 ) 
				{
					gi.Printf(  "bad %s in boltOn '%s'\n", token, boltOn->name );
					continue;
				}
				boltOn->model.customRGB[1] = n;
				continue;
			}

			//Blue tint
			if ( !Q_stricmp( token, "tintBlue" ) ) 
			{
				if ( G_ParseInt( &p, &n ) ) 
				{
					SkipRestOfLine( &p );
					continue;
				}
				if ( n < 0 ) 
				{
					gi.Printf(  "bad %s in boltOn '%s'\n", token, boltOn->name );
					continue;
				}
				boltOn->model.customRGB[2] = n;
				continue;
			}

			//alpha
			if ( !Q_stricmp( token, "alpha" ) ) 
			{
				if ( G_ParseInt( &p, &n ) ) 
				{
					SkipRestOfLine( &p );
					continue;
				}
				if ( n < 0 ) 
				{
					gi.Printf(  "bad %s in boltOn '%s'\n", token, boltOn->name );
					continue;
				}
				boltOn->model.customAlpha = n;
				continue;
			}
		}

		numBoltOns++;
	}
}

void G_ClearBoltOnInfo( boltOnInfo_t *bOInfo )
{
	bOInfo->index = MAX_GAME_BOLTONS;
	VectorClear( bOInfo->lastOrigin );
	VectorClear( bOInfo->lastAngles );
	bOInfo->frame = bOInfo->startFrame = bOInfo->endFrame = 0;
	bOInfo->loopAnim = qfalse;
}

void G_LoadBoltOns( void ) 
{
	int			len;
	const char	filename[] = "ext_data/boltOns.cfg";
	char		*buffer;

	// Reset the count so boltOns don't ACCUMULATE across map loads. G_LoadBoltOns runs on every game
	// init (boot, savegame load, new map), and G_RegisterBoltOns does numBoltOns++ per entry without
	// ever zeroing it -> after a couple of reloads numBoltOns exceeds MAX_GAME_BOLTONS and G_Error
	// fatals ("Too many boltOns (64)"), e.g. loading a save then starting a new tutorial.
	numBoltOns = 0;

	gi.Printf( "Parsing %s\n", filename );
	len = gi.FS_ReadFile( filename, (void **) &buffer );
	if ( len == -1 ) 
	{
		gi.Printf( "file not found\n" );
		return;
	}

	if ( len >= sizeof( boltOnList ) ) 
	{
		G_Error( "ext_data/boltOns.cfg is too large" );
	}
	strncpy( boltOnList, buffer, sizeof( boltOnList ) - 1 );
	gi.FS_FreeFile( buffer );

	G_RegisterBoltOns();
}

byte G_AddBoltOn( gentity_t *ent, const char *boltOnName )
{
	int freeSlot, newIndex;

	if ( !ent || !boltOnName || !boltOnName[0] )
	{
		return BOLTON_NONE;
	}

	newIndex = G_GetBoltOnIndex( boltOnName );
	if ( newIndex < 0 || newIndex >= numBoltOns )
	{
		return BOLTON_NONE;
	}

	if ( !ent->client )
	{
		freeSlot = 0;

		G_ClearBoltOnInfo( &ent->boltOn );
		ent->boltOn.index = newIndex;
	}
	else
	{
		for ( freeSlot = 0; freeSlot < MAX_BOLT_ONS; freeSlot++ )
		{
			if ( ent->client->renderInfo.boltOns[freeSlot].index == MAX_GAME_BOLTONS )
			{//Found a free slot
				break;
			}
			else if ( ent->client->renderInfo.boltOns[freeSlot].index == newIndex )
			{
#ifndef FINAL_BUILD
				gi.Printf("WARNING: %s already has boltOn turned on!\n", ent->targetname, boltOnName );
#endif
				return BOLTON_NONE;
			}
		}

		if ( freeSlot >= MAX_BOLT_ONS )
		{
			gi.Printf("WARNING: %s out of free boltOn slots! (MAX = %d)\n", ent->targetname, MAX_BOLT_ONS );
			return BOLTON_NONE;
		}

		G_ClearBoltOnInfo( &ent->client->renderInfo.boltOns[freeSlot] );
		ent->client->renderInfo.boltOns[freeSlot].index = newIndex;
	}

	return freeSlot;
}

void G_RemoveBoltOn( gentity_t *ent, const char *boltOnName )
{
	int	namedIndex;
	
	if ( !ent || !boltOnName || !boltOnName[0] )
	{
		return;
	}

	namedIndex = G_GetBoltOnIndex( boltOnName );
	if ( namedIndex < 0 || namedIndex >= numBoltOns )
	{
		return;
	}

	if ( !ent->client )
	{
		if ( ent->boltOn.index == namedIndex )
		{
			G_ClearBoltOnInfo( &ent->boltOn );
		}
	}
	else
	{
		for ( int i = 0; i < MAX_BOLT_ONS; i++ )
		{
			if ( ent->client->renderInfo.boltOns[i].index == namedIndex )
			{//Found it, clear it
				G_ClearBoltOnInfo( &ent->client->renderInfo.boltOns[i] );
			}
		}
	}
}

void G_DropBoltOn( gentity_t *ent, const char *boltOnName )
{
	boltOn_t	*boltOn;
	int			namedIndex;

	if ( !ent || !boltOnName || !boltOnName[0] )
	{
		return;
	}

	namedIndex = G_GetBoltOnIndex( boltOnName );
	if ( namedIndex < 0 || namedIndex >= numBoltOns )
	{
		return;
	}

	if ( !ent->client )
	{
		if ( ent->boltOn.index != namedIndex )
		{
			//FIXME: error msg
			return;
		}

		if ( ent->boltOn.index < 0 || ent->boltOn.index >= numBoltOns )
		{
			return;
		}

		boltOn = &knownBoltOns[ent->boltOn.index];
		//FIXME: what about tint, alpha and scale?  And animation?
		gentity_t *newObject = G_CreateObject( ent, ent->boltOn.lastOrigin, ent->boltOn.lastAngles,
			boltOn->model.modelIndex, ent->boltOn.frame, TR_GRAVITY );
		if ( newObject )
		{
			newObject->targetname = G_NewString( va( "%s_%s", ent->targetname, boltOnName ) );
		}

		//Remove it
		G_RemoveBoltOn( ent, boltOnName );
	}
	else
	{
		for ( int i = 0; i < MAX_BOLT_ONS; i++ )
		{
			if ( ent->client->renderInfo.boltOns[i].index < 0 || ent->client->renderInfo.boltOns[i].index >= numBoltOns )
			{
				continue;
			}

			if ( ent->client->renderInfo.boltOns[i].index == namedIndex )
			{//Found it
				boltOn = &knownBoltOns[ent->client->renderInfo.boltOns[i].index];
				//GUH!!! Move these onto ENT!!!
				gentity_t *newObject = G_CreateObject( ent, ent->client->renderInfo.boltOns[i].lastOrigin,
					ent->client->renderInfo.boltOns[i].lastAngles,
					boltOn->model.modelIndex,
					ent->client->renderInfo.boltOns[i].frame, 
					TR_GRAVITY );//TR_STATIONARY );//
				if ( newObject )
				{
					newObject->targetname = G_NewString( va( "%s_%s", ent->targetname, boltOnName ) );
				}
				//FIXME: what about tint, alpha and scale?  And animation?

				//Remove it
				G_RemoveBoltOn( ent, boltOnName );
				//Done
				return;
			}
		}
	}
}

byte G_BoltOnNumberForName( gentity_t *ent, const char *boltOnName )
{
	if ( !ent || !boltOnName || !boltOnName[0] )
	{
		return MAX_BOLT_ONS;
	}

	if ( !ent->client )
	{//non-clients only have one
		return 0;
	}

	for ( int i = 0; i < MAX_BOLT_ONS; i++ )
	{
		if ( !Q_stricmp( boltOnName, knownBoltOns[ent->client->renderInfo.boltOns[i].index].name ) )
		{//Found it, return it
			return i;
		}
	}

	return MAX_BOLT_ONS;
}

void G_InitBoltOnData ( gentity_t *ent )
{
	//UG, crappy here, but need to initialize boltOn index to invalid number
	ent->activeBoltOn = MAX_BOLT_ONS;

	G_ClearBoltOnInfo( &ent->boltOn );
	if ( ent->client )
	{
		for ( int i = 0; i < MAX_BOLT_ONS; i++ )
		{
			G_ClearBoltOnInfo( &ent->client->renderInfo.boltOns[i] );
		}
	}
}

