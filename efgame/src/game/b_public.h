#ifndef __B_PUBLIC_H__
#define __B_PUBLIC_H__

#include "bstate.h"

#define NPCAI_CHECK_WEAPON		0x00000001
#define NPCAI_BURST_WEAPON		0x00000002
#define NPCAI_MOVING			0x00000004
#define NPCAI_TOUCHED_GOAL		0x00000008
#define NPCAI_PUSHED			0x00000010
#define NPCAI_NO_COLL_AVOID		0x00000020
#define NPCAI_BLOCKED			0x00000040
#define NPCAI_AWAITING_COMM		0x00000080
#define NPCAI_OFF_PATH			0x00000100
#define NPCAI_IN_SQUADPOINT		0x00000200
#define NPCAI_STRAIGHT_TO_DESTPOS	0x00000400
#define NPCAI_DONT_FIRE			0x00000800
#define NPCAI_NO_SLOWDOWN		0x00001000
#define NPCAI_LOST				0x00002000	//Can't nav to his goal
#define NPCAI_SHIELDS			0x00004000	//Has shields, borg can adapt
#define NPCAI_GREET_ALLIES		0x00008000	//Say hi to nearby allies
#define NPCAI_FORM_TELE_NAV		0x00010000	//Tells formation people to use nav info to get to 
#define NPCAI_ENROUTE_TO_HOMEWP 0x00020000	//Lets us know to run our lostenemyscript when we get to homeWp
#define NPCAI_MATCHPLAYERWEAPON 0x00040000	//Match the player's weapon except when it changes during cinematics
#define NPCAI_IGNORE_ALERTS		0x00080000	//Do not get enemy set by allies in area(ambush)
//Script flags
#define	SCF_CROUCHED		0x00000001	//Force ucmd.upmove to be -127
#define	SCF_WALKING			0x00000002	//Force BUTTON_WALKING to be pressed
#define	SCF_CAREFUL			0x00000004	//Force BUTTON_CAREFUL to be pressed
#define	SCF_LEAN_RIGHT		0x00000008	//Force rightmove+BUTTON_USE
#define	SCF_LEAN_LEFT		0x00000010	//Force leftmove+BUTTON_USE
#define	SCF_RUNNING			0x00000020	//Takes off walking button, overrides SCF_WALKING
#define	SCF_ALT_FIRE		0x00000040	//Force to use alt-fire when firing
#define	SCF_NO_RESPONSE		0x00000080	//NPC will not do generic responses to being used
#define	SCF_FFDEATH			0x00000100	//Just tells player_die to run the friendly fire deathscript
#define	SCF_NO_COMBAT_TALK	0x00000200	//NPC will not use their generic combat chatter stuff

//#ifdef __DEBUG

//Debug flag definitions

#define	AID_IDLE		0x00000000	//Nothing is happening
#define AID_ACQUIRED	0x00000001	//A target has been found
#define AID_LOST		0x00000002	//Alert, but no target is in sight
#define AID_CONFUSED	0x00000004	//Is unable to come up with a course of action
#define AID_LOSTPATH	0x00000008	//Cannot make a valid movement due to lack of connections

//#endif //__DEBUG

//extern qboolean showWaypoints;

typedef enum {VIS_UNKNOWN, VIS_NOT, VIS_PVS, VIS_360, VIS_FOV, VIS_SHOOT} visibility_t;
typedef enum {SPOT_ORIGIN, SPOT_HEAD, SPOT_HEAD_LEAN, SPOT_WEAPON, SPOT_LEGS, SPOT_GROUND} spot_t;

typedef enum //# lookMode_e
{
	LT_NONE = 0,
	LT_GLANCE,
	LT_AIMSOFT,
	LT_NORMAL,
	LT_AIM,
	LT_AIMSWING,
	LT_FULLFACE
} lookMode_t;

typedef enum //# jumpState_e
{
	JS_WAITING = 0,
	JS_FACING,
	JS_CROUCHING,
	JS_JUMPING,
	JS_LANDING
} jumpState_t;

typedef enum //# rank_e
{
	RANK_CIVILIAN,
	RANK_CREWMAN,
	RANK_ENSIGN,
	RANK_LT_JG,
	RANK_LT,
	RANK_LT_COMM,
	RANK_COMMANDER,
	RANK_CAPTAIN
} rank_t;

typedef enum //# movetype_e
{
	MT_STATIC = 0,
	MT_WALK,
	MT_RUNJUMP,
	MT_FLYSWIM,
	NUM_MOVETYPES
} movetype_t;

typedef struct gNPCstats_e
{//Stats, loaded in, and can be set by scripts
	//AI
	int		aggression;			//			"
	int		aim;				//			"
	float	earshot;			//			"
	int		evasion;			//			"
	int		hfov;				// horizontal field of view
	int		intelligence;		//			"
	int		move;				//			"
	int		reactions;			// 1-5, higher is better
	float	shootDistance;		//Maximum range- overrides range set for weapon if nonzero
	int		vfov;				// vertical field of view
	float	vigilance;			//			"
	float	visrange;			//			"
	//Movement
	movetype_t	moveType;
	int		runSpeed;
	int		walkSpeed;
	float	yawSpeed;			// 1 - whatever, default is 50
	int		acceleration;
} gNPCstats_t;

// NOTE!!!  If you add any ptr fields into this structure could you please tell me so I can update the load/save code?
//	so far the only things I've got to cope with are a bunch of gentity_t*'s, but tell me if any more get added -slc
//
typedef struct 
{
	//FIXME: Put in playerInfo or something
	int			timeOfDeath;			//FIXME do we really need both of these
	gentity_t	*touchedByPlayer;

	visibility_t	enemyLastVisibility;

	int			aimTime;
	float		desiredYaw;
	float		desiredPitch;
	float		lockedDesiredYaw;
	float		lockedDesiredPitch;
	gentity_t	*aimingBeam;		// debugging aid

	vec3_t		enemyLastSeenLocation;
	int			enemyLastSeenTime;
	vec3_t		enemyLastHeardLocation;
	int			enemyLastHeardTime;

	int			eFlags;
	int			aiFlags;

	int			currentAmmo;		// this sucks, need to find a better way
	int			shotTime;
	int			burstCount;
	int			burstMin;
	int			burstMean;
	int			burstMax;
	int			burstSpacing;
	int			attackHold;
	int			attackHoldTime;
	vec3_t		shootAngles;		//Angles to where bot is shooting - fixme: make he torso turn to reflect these

	//extra character info
	rank_t		rank;		//for pips

	//Behavior state info
	bState_t	behaviorState;	//determines what actions he should be doing
	bState_t	defaultBehavior;//State bot will default to if none other set
	bState_t	tempBehavior;//While valid, overrides other behavior

	qboolean	ignorePain;		//only play pain scripts when take pain

	int			duckDebounceTime;//Keeps them ducked for a certain time
	int			walkDebounceTime;
	int			enemyCheckDebounceTime;
	int			investigateDebounceTime;
	int			investigateCount;
	vec3_t		investigateGoal;
	int			investigateSoundDebounceTime;
	int			greetingDebounceTime;//when we can greet someone next
	gentity_t	*eventOwner;
	
	//bState-specific fields
	gentity_t	*coverTarg;
	jumpState_t	jumpState;
	float		followDist;

	// goal, navigation & pathfinding
	gentity_t	*tempGoal;			// used for locational goals (player's last seen/heard position)
	gentity_t	*goalEntity;
	gentity_t	*lastGoalEntity;
	gentity_t	*eventualGoal;
	gentity_t	*hidingGoal;		//Where we should go to hide
	gentity_t	*captureGoal;		//Where we should try to capture
	gentity_t	*defendEnt;			//Who we're trying to protect
	gentity_t	*greetEnt;			//Who we're greeting
	int			goalTime;	//FIXME: This is never actually used
	qboolean	straightToGoal;	//move straight at navgoals
	float		distToGoal;
	int			navTime;
	int			blockingEntNum;
	int			blockedSpeechDebounceTime;
	int			homeWp;

	//Squad stuff - should be in an allocated struct so not every NPC has this
	int			iSquadPathIndex;	// (-1 is treated as NULL ptr for above would be)
	int			iSquadRouteIndex;	// (-1 is treated as NULL ptr for above would be)

	vec3_t		lastLeaderPoint;	//Last place our leader was when we stopped
	int			currentSquadPoint;	//Current squadPoint I'm in right now, if any
	int			lastSquadPoint;		//The last squadPoint I was actually IN
	vec3_t		lastSPCalcedOrg;	//Spot from where my closestWaypoints was last calced
	float		goalDistToPathSeg;	//My leader's distance from his closest point to my path
	vec3_t		sPDestPos;			//Where on my path I'm trying to get
	int			sPDestSegPoint1;	//My goalPos' closest squadpoint on my path
	int			sPDestSegPoint2;	//My goalPos' next closest squadpoint on my path
	int			sPCurSegPoint1;		//My closest squadpoint on my path
	int			sPCurSegPoint2;		//My next closest squadpoint on my path
	int			sPLastSegDist;		//Last dist to closest point on closest squadpath segment
	int			curSegLastWp;		//The squadpoint "behind" me on my current segment
	int			curSegNextWp;		//The squadpoint "ahead" of me on my current segment
	int			destSegLastWp;		//The squadpoint "behind" my destPos on its current segment
	int			destSegNextWp;		//The squadpoint "ahead" my destPos on its current segment
	float		sPDestPosPathDist;	//distance to my sPDestPos along the path
	vec3_t		lastPathAngles;		//So we know which way to face generally when we stop
	vec3_t		leaderTeleportSpot;	//If the leader teleported, we head straight here rather than follow path

	//stats
	gNPCstats_t	stats;
	int			aimErrorDebounceTime;
	float		lastAimErrorYaw;
	float		lastAimErrorPitch;
	vec3_t		aimOfs;

	//scriptflags
	int			scriptFlags;//in b_local.h

	//
	int			commWaitTime;
	int			sayString;
	gentity_t	*sayTarg;

	//Should this be on all clients?  In clientInfo or moveInfo?  renderInfo?
	lookMode_t	lookMode;
	int			idleCounter;

	//moveInfo
	int			desiredSpeed;
	int			currentSpeed;
	char		last_forwardmove;
	char		last_rightmove;
	vec3_t		lastClearOrigin;
	int			consecutiveBlockedMoves;
	int			blockedDebounceTime;
	qboolean	canShove;
	int			shoveCount;
	float		lastShoveDir;
	int			shoveDebounce;

	//
	int			combatPoint;//NPCs in bState BS_COMBAT_POINT will find their closest empty combat_point

	//Testing physics at 20fps
	int			nextBStateThink;
	usercmd_t	last_ucmd;

	//
	//JWEIER ADDITIONS START

	qboolean	combatMove;
	int			goalRadius;

	//FIXME: These may be redundant
	
	int			pauseTime;		//Time to stand still
	int			weaponTime;		//Time until refire is valid
	int			standTime;
	int			jumpTime;

	int			localState;		//Tracking information local to entity
	int			squadState;		//Tracking information for team level interaction

	//JWEIER ADDITIONS END
	//

} gNPC_t;

void G_SquadPathsInit(void);
void NPC_InitGame( void );
void G_LoadBoltOns( void );
void Svcmd_NPC_f( void );
void NAV_DebugShowWaypoints (void);
void NAV_DebugShowBoxes (void);
void NAV_DebugShowSquadPaths (void);
/*
void Bot_InitGame( void );
void Bot_InitPreSpawn( void );
void Bot_InitPostSpawn( void );
void Bot_Shutdown( void );
void Bot_Think( gentity_t *ent, int msec );
void Bot_Connect( gentity_t *bot, char *botName );
void Bot_Begin( gentity_t *bot );
void Bot_Disconnect( gentity_t *bot );
void Svcmd_Bot_f( void );
void Nav_ItemSpawn( gentity_t *ent, int remaining );
*/

//
// This section should be moved to QFILES.H
//
/*
#define NAVFILE_ID			(('I')+('N'<<8)+('A'<<16)+('V'<<24))
#define NAVFILE_VERSION		6

typedef struct {
	unsigned	id;
	unsigned	version;
	unsigned	checksum;
	unsigned	surfaceCount;
	unsigned	edgeCount;
} navheader_t;


#define MAX_SURFACES	4096

#define NSF_PUSH			0x00000001
#define NSF_WATERLEVEL1		0x00000002
#define NSF_WATERLEVEL2		0x00000004
#define NSF_WATER_NOAIR		0x00000008
#define NSF_DUCK			0x00000010
#define NSF_PAIN			0x00000020
#define NSF_TELEPORTER		0x00000040
#define NSF_PLATHIGH		0x00000080
#define NSF_PLATLOW			0x00000100
#define NSF_DOOR_FLOOR		0x00000200
#define NSF_DOOR_SHOOT		0x00000400
#define NSF_DOOR_BUTTON		0x00000800
#define NSF_BUTTON			0x00001000

typedef struct {
	vec3_t		origin;
	vec2_t		absmin;
	vec2_t		absmax;
	int			parm;
	unsigned	flags;
	unsigned	edgeCount;
	unsigned	edgeIndex;
} nsurface_t;


#define NEF_DUCK			0x00000001
#define NEF_JUMP			0x00000002
#define NEF_HOLD			0x00000004
#define NEF_WALK			0x00000008
#define NEF_RUN				0x00000010
#define NEF_NOAIRMOVE		0x00000020
#define NEF_LEFTGROUND		0x00000040
#define NEF_PLAT			0x00000080
#define NEF_FALL1			0x00000100
#define NEF_FALL2			0x00000200
#define NEF_DOOR_SHOOT		0x00000400
#define NEF_DOOR_BUTTON		0x00000800
#define NEF_BUTTON			0x00001000

typedef struct {
	vec3_t		origin;
	vec2_t		absmin;		// region within this surface that is the portal to the other surface
	vec2_t		absmax;
	int			surfaceNum;
	unsigned	flags;		// jump, prerequisite button, will take falling damage, etc...
	float		cost;
	int			dirIndex;
	vec3_t		endSpot;
	int			parm;
} nedge_t;
*/
#endif
