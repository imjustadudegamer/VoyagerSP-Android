#define NUM_EXPLOSION_SHADERS	8
#define NUM_EXPLOSION_FRAMES	3

#define	CMD_BACKUP			64	
#define	CMD_MASK			(CMD_BACKUP - 1)
// allow a lot of command backups for very fast systems
// multiple commands may be combined into a single packet, so this
// needs to be larger than PACKET_BACKUP


// Bumped 256 -> 1024 (= MAX_GENTITIES) for the in-process SP loopback. Retail caps at 256 and PVS-culls
// because it sends snapshots over the network (bandwidth); the SP bridge is in-process (CG_GETSNAPSHOT is a
// direct struct copy, no netchan) and does NOT PVS-cull, so dense maps (borg1 ~561 ents) overflowed 256 and
// dropped visible entities by entity-number order. 1024 covers every possible entity. This header is shared
// by the bridge (libmain) AND the cgame (libefgame) — BOTH .so MUST be rebuilt together or snapshot_t sizes
// mismatch -> memory corruption. Also auto-resizes cg_solidEntities[]/cg_triggerEntities[] in cg_predict.
#define	MAX_ENTITIES_IN_SNAPSHOT	1024

#define	SNAPFLAG_RATE_DELAYED		1		// the server withheld a packet to save bandwidth
#define	SNAPFLAG_DROPPED_COMMANDS	2		// the server lost some cmds coming from the client

// snapshots are a view of the server at a given time

// Snapshots are generated at regular time intervals by the server,
// but they may not be sent if a client's rate level is exceeded, or
// they may be dropped by the network.
typedef struct {
	int				snapFlags;			// SNAPFLAG_RATE_DELAYED, SNAPFLAG_DROPPED_COMMANDS
	int				ping;

	int				serverTime;		// server time the message is valid for (in msec)

	byte			areamask[MAX_MAP_AREA_BYTES];		// portalarea visibility bits

	int				cmdNum;			// the next cmdNum the server is expecting
									// client side prediction should start with this cmd
	playerState_t	ps;						// complete information about the current player at this time

	int				numEntities;			// all of the entities that need to be presented
	entityState_t	entities[MAX_ENTITIES_IN_SNAPSHOT];	// at the time of this snapshot

	int				numConfigstringChanges;	// configstrings that have changed since the last
	int				configstringNum;		// acknowledged snapshot_t (which is usually NOT the previous snapshot!)

	int				numServerCommands;		// text based server commands to execute when this
	int				serverCommandSequence;	// snapshot becomes current
} snapshot_t;


/*
==================================================================

functions imported from the main executable

==================================================================
*/

#define	CGAME_IMPORT_API_VERSION	3

typedef enum {
	CG_PRINT,
	CG_ERROR,
	CG_MILLISECONDS,
	CG_CVAR_REGISTER,
	CG_CVAR_UPDATE,
	CG_CVAR_SET,
	CG_ARGC,
	CG_ARGV,
	CG_ARGS,
	CG_FS_FOPENFILE,
	CG_FS_READ,
	CG_FS_WRITE,
	CG_FS_FCLOSEFILE,
	CG_SENDCONSOLECOMMAND,
	CG_ADDCOMMAND,
	CG_SENDCLIENTCOMMAND,
	CG_UPDATESCREEN,
	CG_CM_LOADMAP,
	CG_CM_NUMINLINEMODELS,
	CG_CM_INLINEMODEL,
	CG_CM_TEMPBOXMODEL,
	CG_CM_POINTCONTENTS,
	CG_CM_TRANSFORMEDPOINTCONTENTS,
	CG_CM_BOXTRACE,
	CG_CM_TRANSFORMEDBOXTRACE,
	CG_CM_MARKFRAGMENTS,
	CG_S_STARTSOUND,
	CG_S_STARTLOCALSOUND,
	CG_S_CLEARLOOPINGSOUNDS,
	CG_S_ADDLOOPINGSOUND,
	CG_S_UPDATEENTITYPOSITION,
	CG_S_RESPATIALIZE,
	CG_S_REGISTERSOUND,
	CG_S_STARTBACKGROUNDTRACK,
	CG_FF_STARTFX,
	CG_FF_ENSUREFX,
	CG_FF_STOPFX,	
	CG_FF_STOPALLFX,
	CG_R_LOADWORLDMAP,
	CG_R_REGISTERMODEL,
	CG_R_REGISTERSKIN,
	CG_R_REGISTERSHADER,
	CG_R_REGISTERSHADERNOMIP,
	CG_R_CLEARSCENE,
	CG_R_ADDREFENTITYTOSCENE,
	CG_R_GETLIGHTING,
	CG_R_ADDPOLYTOSCENE,
	CG_R_ADDLIGHTTOSCENE,
	CG_R_RENDERSCENE,
	CG_R_SETCOLOR,
	CG_R_DRAWSTRETCHPIC,
	CG_R_DRAWSCREENSHOT,
	CG_R_MODELBOUNDS,
	CG_R_LERPTAG,
	CG_R_DRAWROTATEPIC,
	CG_R_SCISSOR,
	CG_GETGLCONFIG,
	CG_GETGAMESTATE,
	CG_GETCURRENTSNAPSHOTNUMBER,
	CG_GETSNAPSHOT,
	CG_GETSERVERCOMMAND,
	CG_GETCURRENTCMDNUMBER,
	CG_GETUSERCMD,
	CG_SETUSERCMDVALUE,
	CG_MEMORY_REMAINING,
	CG_S_UPDATEAMBIENTSET,
	CG_S_ADDLOCALSET,
	CG_AS_PARSESETS,
	CG_AS_ADDENTRY,
	CG_AS_GETBMODELSOUND,
	CG_S_GETSAMPLELENGTH,
} cgameImport_t;

//----------------------------------------------
