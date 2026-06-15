#ifndef __G_SHARED_H__
#define __G_SHARED_H__

#include "bg_public.h"
#include "g_public.h"
#include "b_public.h"
#include "../icarus/icarus.h"
#include "../renderer/tr_types.h"
#include "../cgame/cg_public.h"
#include "bset.h"

#define	FOFS(x) ((int)(intptr_t)&(((gentity_t *)0)->x))	// (intptr_t) cast is LP64-safe (arm64); value is a small struct offset that still fits int

typedef enum //# taskID_e
{
	TID_CHAN_VOICE = 0,	// Waiting for a voice sound to complete
	TID_ANIM_UPPER,		// Waiting to finish a lower anim holdtime
	TID_ANIM_LOWER,		// Waiting to finish a lower anim holdtime
	TID_ANIM_BOTH,		// Waiting to finish lower and upper anim holdtimes or normal md3 animating
	TID_MOVE_NAV,		// Trying to get to a navgoal or For ET_MOVERS
	TID_ANGLE_FACE,		// Turning to an angle or facing
	TID_BSTATE,			// Waiting for a certain bState to finish
	TID_LOCATION,		// Waiting for ent to enter a specific trigger_location
//	TID_MISSIONSTATUS,	// Waiting for player to finish reading MISSION STATUS SCREEN
	TID_RESIZE,			// Waiting for clear bbox to inflate size
	NUM_TIDS,			// for def of taskID array
} taskID_t;


typedef enum //# material_e
{
	MAT_METAL = 0,
	MAT_GLASS,		
	MAT_ELECTRICAL,// (sparks)
	MAT_ORGANIC,// (not implemented)
	MAT_BORG,//borg chunks
	MAT_STASIS,//stasis chunks
	MAT_GLASS_METAL,//mixed chunk type
	NUM_MATERIALS
} material_t;

typedef enum //# lockMode_e
{
	LM_NONE = 0,	//# Swing part to catch up with next upper part
	LM_SWING,		//# Only swing part to get within swing tolerance (usually 30 - 50 % of clamp tolerance)
	LM_CLAMP,		//# Only swing part to get within clamp tolerance
	LM_LOCKED
} lockMode_t;

//===From cg_local.h================================================
#define	DEFAULT_HEADMODEL	"munro"
#define	DEFAULT_TORSOMODEL	"hazard"
#define	DEFAULT_LEGSMODEL	"hazard"

// each client has an associated clientInfo_t
// that contains media references necessary to present the
// client model and other color coded effects
// this is regenerated each time a userinfo configstring changes

#define	MAX_CUSTOM_BASIC_SOUNDS	10
#define	MAX_CUSTOM_COMBAT_SOUNDS	12
#define	MAX_CUSTOM_EXTRA_SOUNDS	24
#define	MAX_CUSTOM_SCAV_SOUNDS 14
#define	MAX_CUSTOM_SOUNDS	(MAX_CUSTOM_EXTRA_SOUNDS + MAX_CUSTOM_COMBAT_SOUNDS + MAX_CUSTOM_BASIC_SOUNDS + MAX_CUSTOM_SCAV_SOUNDS)
// !!!!!!!!!! LOADSAVE-affecting structure !!!!!!!!!!
typedef struct {
	qboolean		infoValid;

	char			name[MAX_QPATH];
	team_t			team;

	int				score;			// updated by score servercmds

	int				handicap;

	qhandle_t		legsModel;
	qhandle_t		legsSkin;

	qhandle_t		torsoModel;
	qhandle_t		torsoSkin;

	qhandle_t		headModel;
	qhandle_t		headSkin;
	qboolean		extensions;		// do we have extra face skins ?

	int				animFileIndex;

	sfxHandle_t		sounds[MAX_CUSTOM_SOUNDS];

	char			*customBasicSoundDir;
	char			*customCombatSoundDir;
	char			*customExtraSoundDir;
	char			*customScavSoundDir;
} clientInfo_t;


//==================================================================
typedef enum 
{
	MOVER_POS1,
	MOVER_POS2,
	MOVER_1TO2,
	MOVER_2TO1
} moverState_t;

// Rendering information structure
#define MAX_BOLT_ONS	8//No more than 8 bolt-ons

// !!!!!!!!!! LOADSAVE-affecting structure !!!!!!!!!!
typedef struct modelInfo_s
{
	int		modelIndex;
	vec3_t	scaleXYZ;//Multi-axis scale
	vec3_t	customRGB;//Red Green Blue, 0 = don't apply
	int		customAlpha;//Alpha to apply, 0 = none?
} modelInfo_t;

typedef enum 
{
	MODEL_LEGS = 0,
	MODEL_TORSO,
	MODEL_HEAD,
	MODEL_WEAPON1,
	MODEL_WEAPON2,
	MODEL_WEAPON3,
	MODEL_EXTRA1,
	MODEL_EXTRA2,
	NUM_TARGET_MODELS
} targetModel_t;
// Note declared char arrays rather than ptrs. Not nice to use hardwired numbers, but this is because nowhere else has to 
//	know the sizes except by doing sizeof(struct.field) instead of G_NewString(). So there. 
// Also, note that any size <32 is a gain since prev malloc system consumed min 32 bytes per alloc. -slc
//
// !!!!!!!!!! LOADSAVE-affecting structure !!!!!!!!!!
typedef struct boltOnInfo_s
{//a tagged object
	int			index;
	vec3_t		lastOrigin;	// When attached, where the boltOn last was
	vec3_t		lastAngles;	// When attached, how the boltOn was last facing
	int			frame;		// current Frame
	int			startFrame;	// start Frame for anim
	int			endFrame;	// end Frame for anim
	int			startTime;	// time the effect was started (if there is an effect...)
	int			fxFlags;	// 
	qboolean	loopAnim;	// whether or not to loop anim
} boltOnInfo_t;

// BoltOn effects
#define	BOLTON_BEAMIN	0x0001
#define BOLTON_BEAMOUT	0x0002

//renderFlags
#define	RF_LOCKEDANGLE	1

// !!!!!!!!!! LOADSAVE-affecting structure !!!!!!!!!!
typedef struct renderInfo_s
{
	// Legs model, or full model on one piece entities
	union
	{
		modelInfo_t	legsModel;	
		modelInfo_t	model;
	};

	union
	{
		char	legsModelName[32];		// -slc[]
		char	modelName[32];			// -slc[]
	};
	// Models for the other pieces (not used by one piece models)
	modelInfo_t	torsoModel;
	modelInfo_t	headModel;

	char	torsoModelName[32];			// -slc[]
	char	headModelName[32];			// -slc[]

	//In whole degrees, How far to let the different model parts yaw and pitch
	int		headYawRangeLeft;
	int		headYawRangeRight;
	int		headPitchRangeUp;
	int		headPitchRangeDown;

	int		torsoYawRangeLeft;
	int		torsoYawRangeRight;
	int		torsoPitchRangeUp;
	int		torsoPitchRangeDown;
	// controls turning tolerance
	lockMode_t	torsoYawLockMode;	
	lockMode_t	torsoPitchLockMode;	
	lockMode_t	legsYawLockMode;	

	int		legsFrame;
	int		torsoFrame;

	float	legsFpsMod;
	float	torsoFpsMod;

	//Fields to apply to entire model set, individual model's equivalents will modify this value
	vec3_t	scaleXYZ;//Multi-axis scale
	vec3_t	customRGB;//Red Green Blue, 0 = don't apply
	int		customAlpha;//Alpha to apply, 0 = none?

	boltOnInfo_t	boltOns[MAX_BOLT_ONS];//Tagged object array

	//RF?
	int			renderFlags;

	//
	vec3_t		muzzlePoint;
	int			mPCalcTime;//Last time muzzle point was calced

	//
	float		lockYaw;//

	//
	vec3_t		headPoint;//Where your tag_head is
	vec3_t		headAngles;//where the tag_head in the torso is pointing
	vec3_t		torsoPoint;//Where your tag_torso is
	vec3_t		torsoAngles;//Where the tag_torso in your legs are pointing
	vec3_t		eyePoint;//Where your eyes are
	vec3_t		eyeAngles;//Where your eyes face
	int			lookTarget;//Which ent to look at with lookAngles
	int			lookTargetClearTime;//Time to clear the lookTarget
	int			lastVoiceVolume;//Last frame's voice volume
	vec3_t		lastHeadAngles;//Last headAngles, NOT actual facing of head model
	vec3_t		headBobAngles;//headAngle offsets
	vec3_t		targetHeadBobAngles;//head bob angles will try to get to targetHeadBobAngles
	int			lookingDebounceTime;//When we can stop using head looking angle behavior
} renderInfo_t;

// Movement information structure

typedef struct moveInfo_s	// !!!!!!!!!! LOADSAVE-affecting struct !!!!!!!!
{
	vec3_t	desiredAngles;	// Desired facing angles
	float	speed;	// Speed of movement
	float	aspeed;	// Speed of angular movement
	vec3_t	moveDir;	// Direction of movement
	vec3_t	velocity;	// movement velocity
	int		flags;			// Special state flags
} moveInfo_t;


typedef enum {
	CON_DISCONNECTED,
	CON_CONNECTING,
	CON_CONNECTED
} clientConnected_t;

typedef enum {
	TEAM_BEGIN,		// Beginning a team game, spawn at base
	TEAM_ACTIVE		// Now actively playing
} playerTeamStateState_t;

typedef enum //# race_e
{
	RACE_NONE = 0,
	RACE_HUMAN,
	RACE_BORG,
	RACE_KLINGON,
	RACE_HIROGEN,
	RACE_MALON,
	RACE_STASIS,
	RACE_8472,
	RACE_BOT,
	RACE_HARVESTER,
	RACE_REAVER,
	RACE_AVATAR,
	RACE_PARASITE,
	RACE_VULCAN,
	RACE_BETAZOID,
	RACE_BOLIAN,
	RACE_TALAXIAN,
	RACE_BAJORAN,
	RACE_HOLOGRAM
} race_t;

// !!!!!!!!!! LOADSAVE-affecting structure !!!!!!!!!!
typedef struct {
	playerTeamStateState_t	state;

	int			captures;
	int			basedefense;
	int			carrierdefense;
	int			flagrecovery;
	int			fragcarrier;
	int			assists;

	float		lasthurtcarrier;
	float		lastreturnedflag;
	float		flagsince;
	float		lastfraggedcarrier;
} playerTeamState_t;

typedef struct objectives_s
{
	qboolean	display;	// A displayable objective?
	int			status;	// Succeed or fail or pending
} objectives_t;
#define MAX_MISSION_OBJ 80

// the auto following clients don't follow a specific client
// number, but instead follow the first two active players
#define	FOLLOW_ACTIVE1	-1
#define	FOLLOW_ACTIVE2	-2

// client data that stays across multiple levels or tournament restarts
// this is achieved by writing all the data to cvar strings at game shutdown
// time and reading them back at connection time.  Anything added here
// MUST be dealt with in G_InitSessionData() / G_ReadSessionData() / G_WriteSessionData()
//
// !!!!!!!!!! LOADSAVE-affecting structure !!!!!!!!!!
typedef struct {
	int				missionObjectivesShown;	// Number of times mission objectives have been updated
	team_t			sessionTeam;
	objectives_t	mission_objectives[MAX_MISSION_OBJ];
} clientSession_t;

// client data that stays across multiple respawns, but is cleared
// on each level change or team change at ClientBegin()
// !!!!!!!!!! LOADSAVE-affecting structure !!!!!!!!!!
typedef struct {
	clientConnected_t	connected;	
	usercmd_t	lastCommand;
	qboolean	localClient;		// true if "ip" info key is "localhost"
	char		netname[34];
	int			maxHealth;			// for handicapping
	int			enterTime;			// level.time the client entered the game
	short		cmd_angles[3];		// angles sent over in the last command

	playerTeamState_t teamState;	// status in teamplay games
} clientPersistant_t;


// !!!!!!!!!! LOADSAVE-affecting structure !!!!!!!!!!
typedef struct {
	int				tourObjectivesShown;	// Number of times tour objectives have been updated
	objectives_t	tour_objectives[MAX_MISSION_OBJ];
} clientTourSession_t;

// this structure is cleared on each ClientSpawn(),
// except for 'client->pers' and 'client->sess'
struct gclient_s {
	// ps MUST be the first element, because the server expects it
	playerState_t	ps;				// communicated by server to clients

	// private to game
	clientPersistant_t	pers;
	clientSession_t		sess;

	qboolean	noclip;

	int			lastCmdTime;		// level.time of last usercmd_t, for EF_CONNECTION

	usercmd_t	usercmd;			// most recent usercmd

	int			buttons;
	int			oldbuttons;
	int			latched_buttons;

	// sum up damage over an entire frame, so
	// shotgun blasts give a single big kick
	int			damage_armor;		// damage absorbed by armor
	int			damage_blood;		// damage taken out of health
	int			damage_knockback;	// impact damage
	vec3_t		damage_from;		// origin for vector calculation
	qboolean	damage_fromWorld;	// if true, don't use the damage_from vector

	int			accurateCount;		// for "impressive" reward sound

	// timers
	int			respawnTime;		// can respawn when time > this, force after g_forcerespwan
	int			inactivityTime;		// kick players when time > this
	qboolean	inactivityWarning;	// qtrue if the five seoond warning has been given

	int			airOutTime;

	// timeResidual is used to handle events that happen every second
	// like health / armor countdowns and regeneration
	int			timeResidual;
	
	// Facial Expression Timers

	float		facial_blink;		// time before next blink. If a minus value, we are in blink mode
	float		facial_frown;		// time before next frown. If a minus value, we are in frown mode
	float		facial_aux;			// time before next aux. If a minus value, we are in aux mode

	//Client info - updated when ClientInfoChanged is called, instead of using configstrings
	clientInfo_t	clientInfo;
	signed char		forced_forwardmove;
	signed char		forced_rightmove;
	int				fireDelay;		//msec to delay calling G_FireWeapon after EV_FIREWEAPON event is called

	//Used to be in gentity_t, now here.. mostly formation stuff
	team_t		playerTeam;
	team_t		enemyTeam;
	race_t		race;
	char		*squadname;
	gentity_t	*team_leader;
	gentity_t	*leader;
	gentity_t	*follower;
	int			numFollowers;
	gentity_t	*formationGoal;
	int			nextFormGoal;

	//FIXME: could combine these
	float		hiddenDist;//How close ents have to be to pick you up as an enemy
	vec3_t		hiddenDir;//Normalized direction in which NPCs can't see you (you are hidden)

	renderInfo_t	renderInfo;

	int			standheight;
	int			crouchheight;

	clientTourSession_t		tourSess;
};

#define	MAX_PARMS	16
#define	MAX_PARM_STRING_LENGTH	MAX_QPATH//was 16, had to lengthen it so they could take a valid file path
typedef struct
{	
	char	parm[MAX_PARMS][MAX_PARM_STRING_LENGTH];
} parms_t;

#ifdef GAME_INCLUDE
//these hold the place for the enums in functions.h so i don't have to recompile everytime it changes
#define thinkFunc_t int
#define clThinkFunc_t int
#define reachedFunc_t int
#define blockedFunc_t int
#define touchFunc_t int
#define useFunc_t int
#define painFunc_t int
#define dieFunc_t int

typedef struct centity_s centity_t;
struct gentity_s {
	entityState_t	s;				// communicated by server to clients
	struct gclient_s	*client;	// NULL if not a player (unless it's NPC ( if (this->NPC != NULL)  )  <sigh>... -slc)
	qboolean	inuse;
	qboolean	linked;				// qfalse if not in any good cluster

	int			svFlags;			// SVF_NOCLIENT, SVF_BROADCAST, etc

	qboolean	bmodel;				// if false, assume an explicit mins / maxs bounding box
									// only set by gi.SetBrushModel
	vec3_t		mins, maxs;
	int			contents;			// CONTENTS_TRIGGER, CONTENTS_SOLID, CONTENTS_BODY, etc
									// a non-solid entity should set to 0

	vec3_t		absmin, absmax;		// derived from mins/maxs and origin + rotation

	// currentOrigin will be used for all collision detection and world linking.
	// it will not necessarily be the same as the trajectory evaluation for the current
	// time, because each entity must be moved one at a time after time is advanced
	// to avoid simultanious collision issues
	vec3_t		currentOrigin;
	vec3_t		currentAngles;

	gentity_t	*owner;				// objects never interact with their owners, to
									// prevent player missiles from immediately
									// colliding with their owner

	// DO NOT MODIFY ANYTHING ABOVE THIS, THE SERVER
	// EXPECTS THE FIELDS IN THAT ORDER!

//==========================================================================================

//Essential entity fields
	// note: all the char* fields from here on should be left as ptrs, not declared, because of the way that ent-parsing
	//	works by forcing field offset ptrs as char* and using G_NewString()!! (see G_ParseField() in gmae/g_spawn.cpp -slc
	//
	char		*classname;			// set in QuakeEd
	int			spawnflags;			// set in QuakeEd

	int			flags;				// FL_* variables

	char		*model;				// Normal model, or legs model on tri-models
	char		*model2;			// Torso model

	int			freetime;			// sv.time when the object was freed
	
	int			eventTime;			// events will be cleared EVENT_VALID_MSEC after set
	qboolean	freeAfterEvent;
	qboolean	unlinkAfterEvent;

//Physics and movement fields
	float		physicsBounce;		// 1.0 = continuous bounce, 0.0 = no bounce
	int			clipmask;			// brushes with this content value will be collided against
									// when moving.  items and corpses do not collide against
									// players, for instance
	moveInfo_t	moveInfo;		//FIXME: use this more?
	float		speed;
	vec3_t		movedir;
	vec3_t		lastOrigin;			//Where you were last frame

//Variables reflecting environment
	int			watertype;
	int			waterlevel;

//Targeting/linking fields
	float		angle;			// set in editor, -1 = up, -2 = down
	char		*target;
	char		*target2;		//For multiple targets, not used for firing/triggering/using, though, only for path branches
	char		*target3;		//For multiple targets, not used for firing/triggering/using, though, only for path branches
	char		*target4;		//For multiple targets, not used for firing/triggering/using, though, only for path branches
	char		*targetname;
	char		*team;

	char		*roff;			// the roff file to use, if there is one
	int			roff_ctr;		// current roff frame we are playing
	int			next_roff_time;	
	int			fx_time;		// timer for beam in/out effects.

//Think Functions
	int			nextthink;//Used to determine if it's time to call e_ThinkFunc again
	thinkFunc_t		e_ThinkFunc;//Called once every game frame for every ent
	clThinkFunc_t	e_clThinkFunc;//Think func for equivalent centity
	reachedFunc_t	e_ReachedFunc;// movers call this when hitting endpoint
	blockedFunc_t	e_BlockedFunc;
	touchFunc_t		e_TouchFunc;
	useFunc_t		e_UseFunc;	//Called by G_UseTargets
	painFunc_t		e_PainFunc;	//Called by G_Damage when damage is taken
	dieFunc_t		e_DieFunc;	//Called by G_Damage when health reaches <= 0

//Health and damage fields
	int			health;
	int			max_health;
	qboolean	takedamage;
	material_t	material;
	int			damage;
	int			dflags;
	//explosives, breakable brushes
	int			splashDamage;	// quad will increase this without increasing radius
	int			splashRadius;
	int			methodOfDeath;
	int			splashMethodOfDeath;

//Entity pointers
	gentity_t	*chain;
	gentity_t	*enemy;
	gentity_t	*activator;
	gentity_t	*teamchain;		// next entity in team
	gentity_t	*teammaster;	// master of the team
	gentity_t	*lastEnemy;

//Timing variables, counters and debounce times
	float		wait;
	float		random;
	int			delay;
	qboolean	alt_fire;
	int			count;
	int			bounceCount;
	int			fly_sound_debounce_time;	// wind tunnel
	int			painDebounceTime;
	int			disconnectDebounceTime;
	int			attackDebounceTime;
	int			pushDebounceTime;
	int			aimDebounceTime;

//Unions for miscellaneous fields used under very specific circumstances
	union
	{
		qboolean	trigger_formation;
		qboolean	misc_dlight_active;
		qboolean	etherian_fixit;
	};

	union
	{
		vec3_t		fixit_start_position;
	};
//Navigation
	int			waypoint;			//Set once per frame, if you've moved, and if someone asks
	int			lastWaypoint;		//To make sure you don't double-back
	int			lastValidWaypoint;	//ALWAYS valid -used for tracking someone you lost
	int			targetWaypoint;

//Animation
	qboolean	loopAnim;
	int		startFrame;
	int		endFrame;

//Script/ICARUS-related fields
	CSequencer		*sequencer;
	CTaskManager	*taskManager;
	int				taskID[NUM_TIDS];
	parms_t			*parms;
	char		*behaviorSet[NUM_BSETS];
	char		*script_targetname;
	int			delayScriptTime;
	char		*fullName;

// Ambient sound info
	char			*soundSet;	//Only used for local sets
	int				setTime;

//Used by cameras to locate subjects
	char			*cameraGroup;

//For damage
	team_t		noDamageTeam;

//For TED info display
	char		*infoString;

//==========================================================================================
	
//FIELDS USED EXCLUSIVELY BY SPECIFIC CLASSES OF ENTITIES
	//NPC/Player entity fields
	//FIXME: Make these client only?
	gNPC_t		*NPC;//Only allocated if the entity becomes an NPC

	//Other NPC/Player-related entity fields
	char		*ownername;//Used by squadpaths to locate owning NPC

//FIXME: Only used by NPCs, move it to gNPC_t
	int			cantHitEnemyCounter;//HACK - Makes them look for another enemy on the same team if the one they're after can't be hit

//Only used by NPC_spawners
	char		*NPC_type;
	char		*NPC_targetname;
	char		*NPC_target;

//Variables used by movers (most likely exclusively by them)
	moverState_t moverState;
	int			soundPos1;
	int			sound1to2;
	int			sound2to1;
	int			soundPos2;
	int			soundLoop;
	gentity_t	*nextTrain;
	gentity_t	*prevTrain;
	vec3_t		pos1, pos2;
	int			sounds;
	char		*closetarget;
	char		*opentarget;
	char		*paintarget;

//Variables used only by waypoints (for the most part)
	float		radius;
	int			wpIndex;

	int			noise_index;
	union
	{
		vec4_t		startRGBA;
		vec3_t		pos3;
	};
	union
	{
		vec4_t		finalRGBA;
		vec3_t		pos4;
	};

//FIXME: Are these being used anymore?
	gitem_t		*item;			// for bonus items - 
	char		*message;		//Used by triggers to print a message when activated

	float		lightLevel;

	//FIXME: can these be removed/condensed/absorbed?
	//Rendering info
	//int			color;
	boltOnInfo_t	boltOn;//Allowed one tagged object
	int				activeBoltOn;
};
#endif //#ifdef GAME_INCLUDE

extern	gentity_t		*g_entities;
#ifndef _USRDLL
extern	game_import_t	gi;
#endif

// each WP_* weapon enum has an associated weaponInfo_t
// that contains media references necessary to present the
// weapon and its effects
typedef struct weaponInfo_s {
	qboolean		registered;
	gitem_t			*item;

	qhandle_t		handsModel;			// the hands don't actually draw, they just position the weapon
	qhandle_t		weaponModel;		//for in view
	qhandle_t		weaponWorldModel;	//for in their hands
	qhandle_t		barrelModel[4];
	qhandle_t		flashModel;
	qhandle_t		flash2Model;
	qhandle_t		flashaModel;
	qhandle_t		flasha2Model;

	vec3_t			weaponMidpoint;		// so it will rotate centered instead of by tag

	qhandle_t		weaponIcon;
	qhandle_t		ammoIcon;

	qhandle_t		ammoModel;

	qhandle_t		missileModel;
	sfxHandle_t		missileSound;
	void			(*missileTrailFunc)( centity_t *, const struct weaponInfo_s *wi );

	qhandle_t		alt_missileModel;
	sfxHandle_t		alt_missileSound;
	void			(*alt_missileTrailFunc)( centity_t *, const struct weaponInfo_s *wi );

	sfxHandle_t		flashSound;
	sfxHandle_t		altFlashSound;

	sfxHandle_t		firingSound;
	sfxHandle_t		altFiringSound;

	sfxHandle_t		stopSound;

	sfxHandle_t		missileHitSound;
	sfxHandle_t		altmissileHitSound;

} weaponInfo_t;

extern sfxHandle_t CAS_GetBModelSound( const char *name, int stage );

enum
{
	EDGE_NORMAL,
	EDGE_PATH,
	EDGE_BROKEN,
};

enum
{
	NODE_NORMAL,
	NODE_START,
	NODE_GOAL,
	NODE_NAVGOAL,
};

#endif // #ifndef __G_SHARED_H__

/*
structures heirarchy

centity_t	cg_entities[MAX_GENTITIES]
{
	entityState_t	currentState, nextState
	{
		trajectory_t	pos, apos
	}
	playerEntity_t	pe
	{
		lerpFrame_t	legs, torso
		{
			animation_t	animation
		}
	}
	gentity_t		gent, g_entities[MAX_GENTITIES]
	{
		gentity_t	owner, nextTrain, prevTrain, chain, enemy, activator, teamchain, teammaster, team_leader, leader, follower,	formationGoal, lastEnemy
		entityState_t	s
		{
			trajectory_t	pos, apos
		}
		gclient_t		client
		{
			playerState_t	ps
			clientPersistant_t	pers;
			{
				playerTeamState_t teamState;	// status in teamplay games
			}
			clientSession_t		sess
			usercmd_t	usercmd
			clientInfo_t	clientInfo
			{
				animation_t		animation[MAX_ANIMATIONS]
				animsounds_t	torsoAnimSnds[MAX_ANIM_SOUNDS], legsAnimSnds[MAX_ANIM_SOUNDS]
			}
		}
		gitem_t			item
		gNPC_t			NPC
		{
			gNPCstats_t	stats;
			usercmd_t	last_ucmd;
		}
		moveInfo_t		moveInfo
		renderInfo_t	renderInfo
		{
			modelInfo_t	torsoModel, headModel
			boltOn_t	boltOns[MAX_BOLT_ONS]
		}
	}
}

level_locals_t
{
	gclient_t		clients
	gentity_t		locationHead
	interestPoint_t	interestPoints[MAX_INTEREST_POINTS]
	combatPoint_t	combatPoints[MAX_COMBAT_POINTS]
}
*/

