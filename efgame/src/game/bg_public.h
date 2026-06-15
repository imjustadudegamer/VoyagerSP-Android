#ifndef __BG_PUBLIC_H__
#define __BG_PUBLIC_H__
// bg_public.h -- definitions shared by both the server game and client game modules
#include "weapons.h"
#include "teams.h"
#include "statindex.h"

#define	DEFAULT_GRAVITY		800
#define	GIB_HEALTH			-40
#define	ARMOR_PROTECTION	0.25

#define	MAX_ITEMS			256

#define	RANK_TIED_FLAG		0x4000

#define DEFAULT_SHOTGUN_SPREAD	700
#define DEFAULT_SHOTGUN_COUNT	11

#define	ITEM_RADIUS			15		// item sizes are needed for client side pickup detection

//Player sizes
extern float DEFAULT_MINS_0;
extern float DEFAULT_MINS_1;
extern float DEFAULT_MAXS_0;
extern float DEFAULT_MAXS_1;
extern float DEFAULT_PLAYER_RADIUS;
#define DEFAULT_MINS_2		-24
#define DEFAULT_MAXS_2		32
#define CROUCH_MAXS_2		16
//Player viewheights
#define	STANDARD_VIEWHEIGHT_OFFSET -4
//#define	RAVEN_VIEWHEIGHT_ADJ 2
//#define	DEFAULT_VIEWHEIGHT	26+RAVEN_VIEWHEIGHT_ADJ
//#define CROUCH_VIEWHEIGHT	12
#define	DEAD_VIEWHEIGHT		-16
//Player movement values
#define	MIN_WALK_NORMAL		0.7		// can't walk on very steep slopes
#define	JUMP_VELOCITY		225		// 270
#define	STEPSIZE			18


//
// config strings are a general means of communicating variable length strings
// from the server to all connected clients.
//

// CS_SERVERINFO and CS_SYSTEMINFO are defined in q_shared.h
#define	CS_MUSIC			2
#define	CS_MESSAGE			3		// from the map worldspawn's message field
#define	CS_ITEMS			4		// string of 0's and 1's that tell which items are present
#define CS_AMBIENT_SET		5		// ambient set information for the player

#define	CS_MODELS			10
#define	CS_SOUNDS			(CS_MODELS+MAX_MODELS)
#define	CS_PLAYERS			(CS_SOUNDS+MAX_SOUNDS)

#define CS_MAX				(CS_PLAYERS+MAX_CLIENTS)
#if (CS_MAX) > MAX_CONFIGSTRINGS
#error overflow: (CS_MAX) > MAX_CONFIGSTRINGS
#endif

/*
===================================================================================

PMOVE MODULE

The pmove code takes a player_state_t and a usercmd_t and generates a new player_state_t
and some other output data.  Used for local prediction on the client game and true
movement on the server game.
===================================================================================
*/

typedef enum {
	PM_NORMAL,		// can accelerate and turn
	PM_NOCLIP,		// noclip movement
	PM_SPECTATOR,	// still run into walls
	PM_DEAD,		// no acceleration or turning, but free falling
	PM_FREEZE,		// stuck in place with no control
	PM_INTERMISSION	// no movement or status bar
} pmtype_t;

typedef enum {
	WEAPON_READY, 
	WEAPON_RAISING,
	WEAPON_DROPPING,
	WEAPON_FIRING,
	WEAPON_CHARGING,
	WEAPON_IDLE, //lowered
} weaponstate_t;

// pmove->pm_flags
#define	PMF_DUCKED			1
#define	PMF_JUMP_HELD		2
#define	PMF_QUICK_JUMP		4		// true if jumped without minimum landing time
#define	PMF_BACKWARDS_JUMP	8		// go into backwards land
#define	PMF_BACKWARDS_RUN	16		// coast down to backwards run
#define	PMF_TIME_LAND		32		// pm_time is time before rejump
#define	PMF_TIME_KNOCKBACK	64		// pm_time is an air-accelerate only time
#define	PMF_BLOCKED			128		// Bumped into something you can't step up
#define	PMF_TIME_WATERJUMP	256		// pm_time is waterjump
#define	PMF_RESPAWNED		512		// clear after attack and jump buttons come up
#define	PMF_USE_ITEM_HELD	1024
#define PMF_JUMP_DUCKED		2048	// viewheight changes in mid-air
#define PMF_FOLLOW			4096	// spectate following another player
#define PMF_SCOREBOARD		8192	// spectate as a scoreboard
#define	PMF_BUMPED			16384	// Bumped into something and slid to the side

#define	PMF_ALL_TIMES	(PMF_TIME_WATERJUMP|PMF_TIME_LAND|PMF_TIME_KNOCKBACK)

#define	MAXTOUCH	32
typedef struct gentity_s gentity_t;
typedef struct {
	// state (in / out)
	playerState_t	*ps;

	// command (in)
	usercmd_t	cmd;
	int			tracemask;			// collide against these types of surfaces
	int			debugLevel;			// if set, diagnostic output will be printed
	qboolean	noFootsteps;		// if the game is setup for no footsteps by the server
	qboolean	gauntletHit;		// true if a gauntlet attack would actually hit something

	// results (out)
	int			numtouch;
	int			touchents[MAXTOUCH];

	int			useEvent;

	vec3_t		mins, maxs;			// bounding box size

	int			watertype;
	int			waterlevel;

	float		xyspeed;
	gentity_s	*gent;				// Pointer to entity in g_entities[]

	// callbacks to test the world
	// these will be different functions during game and cgame
	void		(*trace)( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentMask );
	int			(*pointcontents)( const vec3_t point, int passEntityNum );
} pmove_t;

// if a full pmove isn't done on the client, you can just update the angles
void PM_UpdateViewAngles( playerState_t *ps, usercmd_t *cmd, gentity_t *gent );
void Pmove( pmove_t *pmove );


#define SETANIM_TORSO 1
#define SETANIM_LEGS  2
#define SETANIM_BOTH  SETANIM_TORSO|SETANIM_LEGS//3

#define SETANIM_FLAG_NORMAL		0//Only set if timer is 0
#define SETANIM_FLAG_OVERRIDE	1//Override previous
#define SETANIM_FLAG_HOLD		2//Set the new timer
#define SETANIM_FLAG_RESTART	4//Allow restarting the anim if playing the same one (weapon fires)

void PM_SetAnimFinal(int *torsoAnim,int *legsAnim,int type,int anim,int priority,int *torsoAnimTimer,int *legsAnimTimer,gentity_t *gent);

//===================================================================================


// player_state->persistant[] indexes
// these fields are the only part of player_state that isn't
// cleared on respawn
//
//  NOTE!!! Even though this is an enum, the array that contains these uses #define MAX_PERSISTANT 16 in q_shared.h,
//		so be careful how many you add since it'll just overflow without telling you -slc
//
typedef enum {
	PERS_SCORE,						// !!! MUST NOT CHANGE, SERVER AND GAME BOTH REFERENCE !!!
	PERS_HITS,						// total points damage inflicted so damage beeps can sound on change
	PERS_TEAM,				
	PERS_SPAWN_COUNT,				// incremented every respawn
	PERS_REWARD_COUNT,				// incremented for each reward sound
	PERS_REWARD,					// a reward_t
	PERS_ATTACKER,					// clientnum of last damage inflicter
	PERS_KILLED,					// count of the number of times you died

	PERS_ACCURACY_SHOTS,			// scoreboard - number of player shots
	PERS_ACCURACY_HITS,				// scoreboard - number of player shots that hit an enemy
	PERS_ENEMIES_KILLED,				// scoreboard - number of enemies player killed
	PERS_TEAMMATES_KILLED			// scoreboard - number of teammates killed 
} persEnum_t;


// entityState_t->eFlags
#define	EF_DEAD				0x00000001		// don't draw a foe marker over players with EF_DEAD
#define	EF_NPC				0x00000002		// An NPC
#define	EF_TELEPORT_BIT		0x00000004		// toggled every time the origin abruptly changes
#define	EF_EYEBEAM			0x00000008		// Eyebeams or flashlights on
#define	EF_BOUNCE			0x00000010		// for missiles
#define	EF_BOUNCE_HALF		0x00000020		// for missiles
#define EF_MISSILE_STICK	0x00000040		// missiles that stick to the wall.
#define	EF_NODRAW			0x00000080		// may have an event, but no model (unspawned items)
#define	EF_FIRING			0x00000100		// for lightning gun
#define EF_ALT_FIRING		0x00000200		// for alt-fires, mostly for lightning guns though
//#define	EF_MOVER_STOP		0x00000400		// will push otherwise
#define	EF_NO_TED			0x00000400		// won't show up on TED unless actively scan with tricorder
#define	EF_AUTO_SIZE		0x00000800		//CG_Ents will create the mins & max itself based on model bounds
#define	EF_TALK				0x00001000		// draw a talk balloon
#define	EF_CONNECTION		0x00002000		// draw a connection trouble sprite
#define EF_ANIM_ALLFAST		0x00004000		// automatically cycle through all frames at 10hz
#define EF_ANIM_ONCE		0x00008000		// cycle through all frames just once then stop

#define EF_PLANTED_CHARGE	0x00010000		// For detpack charge
#define EF_BORG_SPARKIES	0x00020000		// Borg sparking effect
#define EF_ASSIMILATED		0x00040000		// Put Borg Assimilation shader over view, tint green
#define EF_FIXING			0x00080000		// Etherian fixits set this on their "enemy" when they start to fix him
#define EF_SCALE_UP			0x00100000		// When something scales up
#define EF_SCALE_DOWN		0x00200000		// When something scales down
#define EF_BEAM_IN			0x00400000		// When something beams in
#define EF_BEAM_OUT			0x00800000		// When something beams out
#define EF_SCAV_BEAM_OUT	0x01000000		// When scav beams something out
#define EF_DISINTEGRATION	0x02000000		// Disruptor effect
#define EF_DISINT_1			0x02000000		// Disintegration effect 1
#define EF_DISINT_2			0x04000000		// Disintegration effect 2
#define EF_DISINT_3			0x08000000		// Disintegration effect 3
#define EF_DISINT_4			0x10000000		// Disintegration effect 4
#define EF_DISINT_5			0x20000000		// Disintegration effect 5
#define EF_DISINT_6			0x40000000		// Disintegration effect 6

#define EF_BANK_STRAFE		0x80000000		// hunterseeker- shared with next
#define EF_BLOCKED_MOVER	0x80000000		// for movers that are blocked - shared with previous

typedef enum {
	PW_NONE,

	PW_QUAD,
	PW_BATTLESUIT,
	PW_HASTE,
	PW_INVIS,
	PW_REGEN,
	PW_HIROGEN_SHIELD,

	PW_BORG_SHIELD,
	PW_FORGE_DEATH,
	PW_BALL,
	PW_DISINT_1,
	PW_DISINT_2,
	PW_DISINT_3,
	PW_DISINT_4,
	PW_DISINT_5,
	PW_DISINT_6,

	PW_NUM_POWERUPS
} powerup_t;

typedef enum {
	HI_NONE,

	HI_TELEPORTER,
	HI_MEDKIT,

	HI_NUM_HOLDABLE
} holdable_t;

// entityState_t->event values
// entity events are for effects that take place relative
// to an existing entities origin.  Very network efficient.

// two bits at the top of the entityState->event field
// will be incremented with each change in the event so
// that an identical event started twice in a row can
// be distinguished.  And off the value with ~EV_EVENT_BITS
// to retrieve the actual event number
#define	EV_EVENT_BIT1		0x00000100
#define	EV_EVENT_BIT2		0x00000200
#define	EV_EVENT_BITS		(EV_EVENT_BIT1|EV_EVENT_BIT2)

typedef enum {
	EV_NONE,
 
	EV_FOOTSTEP,
	EV_FOOTSTEP_METAL,
	EV_FOOTSPLASH,
	EV_FOOTWADE,
	EV_SWIM,

	EV_STEP_4,
	EV_STEP_8,
	EV_STEP_12,
	EV_STEP_16,

	EV_FALL_SHORT,
	EV_FALL_MEDIUM,
	EV_FALL_FAR,

	EV_JUMP,
	EV_WATER_TOUCH,	// foot touches
	EV_WATER_LEAVE,	// foot leaves
	EV_WATER_UNDER,	// head touches
	EV_WATER_CLEAR,	// head leaves

	EV_ITEM_PICKUP,

	EV_NOAMMO,
	EV_CHANGE_WEAPON,
	EV_FIRE_WEAPON,
	EV_ALT_FIRE,
	EV_USE,

	EV_PLAYER_TELEPORT_IN,
	EV_PLAYER_TELEPORT_OUT,
	EV_BORG_TELEPORT,
	EV_STASIS_TELEPORT_IN,
	EV_STASIS_TELEPORT_OUT,
	EV_SPECIES_TELEPORT,
	EV_FORGE_FADE,
	EV_REPLICATOR,

	EV_GRENADE_BOUNCE,		// eventParm will be the soundindex
	EV_MISSILE_STICK,		// eventParm will be the soundindex

	EV_BMODEL_SOUND,
	EV_GENERAL_SOUND,
	EV_GLOBAL_SOUND,		// no attenuation

	EV_PHASER,
	EV_PHASER_HIT,
	EV_PHASER_MISS,

	EV_COMPRESSION_RIFLE,
	EV_COMPRESSION_RIFLE_ALT,
	EV_COMPRESSION_RIFLE_HIT,
	EV_COMPRESSION_RIFLE_MISS,
	EV_COMPRESSION_RIFLE_ALT_MISS,

	EV_IMOD,
	EV_IMOD_HIT,
	EV_IMOD_MISS,
	EV_IMOD_ALTFIRE,
	EV_IMOD_ALTFIRE_HIT,
	EV_IMOD_ALTFIRE_MISS,

	EV_STASIS,
	EV_STASIS_HIT,
	EV_STASIS_MISS,

	EV_GRENADE_EXPLODE,
	EV_GRENADE_SHRAPNEL_EXPLODE,
	EV_GRENADE_SHRAPNEL,

	EV_DREADNOUGHT_HIT,
	EV_DREADNOUGHT_MISS,

	EV_TETRION,
	EV_TETRION_HIT,
	EV_TETRION_MISS,

	EV_QUANTUM,
	EV_QUANTUM_HIT,

	EV_BORG_RECYCLE,

	EV_BTASER_HIT,
	EV_BTASER_MISS,
	
	EV_RIFLE,
	EV_RIFLE_HIT,
	EV_RIFLE_MISS,

	EV_BOT_ROCKET_DIE,
	EV_BOT_EXPLODE,

	EV_HYPO_PUFF,

	EV_PROTON_GUN,
	EV_PROTON_GUN_ALT,
	EV_PROTON_GUN_HIT,
	EV_PROTON_GUN_MISS,
	EV_PROTON_GUN_ALT_MISS,

	EV_FX_SPARK,
	EV_FX_STEAM,
	EV_FX_COOKING_STEAM,
	EV_FX_BOLT,
	EV_FX_DRIP,
	EV_FX_PLASMA,
	EV_FX_ELECFIRE,
	EV_FX_SURFACE_EXPLOSION,
	EV_FX_ELECTRICAL_EXPLOSION,
	EV_FX_SMOKE,
	EV_FX_TELEPORTER,
	EV_FX_STREAM,
	EV_FX_TRANSPORTER_STREAM,
	EV_FX_LASER,
	EV_FX_EXPLOSION_TRAIL,
	EV_FX_BLOW_CHUNKS,
	EV_FX_BORG_ENERGY_BEAM,
	EV_FX_SHIMMERY_THING,
	EV_FX_BORG_BOLT,
	EV_FX_FORGE_BOLT,
	EV_FX_STASIS_TELEPORTER,
	EV_FX_STASIS_MUSHROOM,
	EV_FX_DN_BEAM_GLOW,
	EV_FX_MAGIC_SMOKE,
	EV_FX_GARDEN_FOUNTAIN_SPURT,

	EV_STASIS_MINE_EXPLODE,

	EV_PAIN,
	EV_DEATH1,
	EV_DEATH2,
	EV_DEATH3,

	EV_MISSILE_HIT,
	EV_MISSILE_MISS,

	EV_GREET1,
	EV_GREET2,
	EV_GREET3,

	EV_SCAV_BEAMING,
	EV_DISINTEGRATION,

	EV_RESPOND1,
	EV_RESPOND2,
	EV_RESPOND3,

	EV_BUSY1,
	EV_BUSY2,
	EV_BUSY3,

	EV_MISSION1,
	EV_MISSION2,
	EV_MISSION3,

	EV_ANGER1,	//Say when acquire an enemy when didn't have one before
	EV_ANGER2,
	EV_ANGER3,

	EV_COMBAT1,	//Say when acquire a new enemy during combat
	EV_COMBAT2,
	EV_COMBAT3,

	EV_VICTORY1,	//Say when killed an enemy
	EV_VICTORY2,
	EV_VICTORY3,

	EV_SETTLE1,	//Say when killed an enemy anc can't find anymore (battle over)
	EV_SETTLE2,
	EV_SETTLE3,

	EV_BLOCKED1,	//Say when someone in your way
	EV_BLOCKED2,
	EV_BLOCKED3,

	EV_FF_1A,	
	EV_FF_1B,
	EV_FF_1C,
	EV_FF_2A,	
	EV_FF_2B,
	EV_FF_2C,
	EV_FF_3A,	
	EV_FF_3B,
	EV_FF_3C,

	EV_CHATTER1,//"Cover me!"
	EV_CHATTER2,//"Get down!"
	EV_CHATTER3,//"Get him!"
	EV_CHATTER4,//"I got him!"
	EV_CHATTER5,//"Kill him!"
	EV_CHATTER6,//"He's mine!"
	EV_CHATTER7,//"Surround him!"
	EV_CHATTER8,//"Over there!"
	EV_CHATTER9,//"Stop the intruder!"
	EV_CHATTER10,//"Take cover!"
	EV_CHATTER11,//"What's that?!"
	EV_CHATTER12,//"Watch it!"
	EV_CHATTER13,//"I'll go check it out..."
	EV_CHATTER14,//"Where did he go?!"

} entity_event_t;


typedef struct animation_s {
	int		firstFrame;
	int		numFrames;
	int		loopFrames;			// 0 to numFrames, -1 = no loop
	int		frameLerp;			// msec between frames
	int		initialLerp;		// msec to get to first frame
} animation_t;

#define	MAX_RANDOM_ANIMSOUNDS	8
typedef struct animsounds_s 
{
	int		keyFrame;			//Frame to play sound on
	int		soundIndex[MAX_RANDOM_ANIMSOUNDS];			//sound file to play - FIXME: should be an index, handle random some other way?
	int		numRandomAnimSounds;		//string variable min for va("...%d.wav", Q_irand(lowestVa, highestVa))
	int		probability;		//chance sound will play, zero value will not run this test (0 = 100% basically)
} animsounds_t;

// flip the togglebit every time an animation
// changes so a restart of the same anim can be detected
#define	ANIM_TOGGLEBIT		1024


// means of death
typedef enum {
	MOD_UNKNOWN,
//weapons
	MOD_PHASER,
	MOD_PHASER_ALT,
	MOD_CRIFLE,
	MOD_SNIPER,
	MOD_IMOD,
	MOD_SCAVENGER,
	MOD_STASIS,
	MOD_TETRION,
	MOD_GRENADE,
	MOD_GRENADE_SPLASH,
	MOD_QUANTUM,
	MOD_QUANTUM_SPLASH,
	MOD_DREADNOUGHT,
	MOD_KNOCKOUT,

//enemies
	MOD_ASSIMILATE,
	MOD_TASER,
	MOD_BOTROCKET,
	MOD_BOTROCKET_SPLASH,
	MOD_ENERGY,
	MOD_ENERGY_SPLASH,
	MOD_ACID,
	MOD_ACID_SPLASH,
	MOD_MELEE,

//world
	MOD_WATER,
	MOD_SLIME,
	MOD_LAVA,
	MOD_CRUSH,
	MOD_TELEFRAG,
	MOD_FALLING,
	MOD_SUICIDE,
	MOD_TARGET_LASER,
	MOD_TRIGGER_HURT,
	MOD_SURGICAL_LASER,

	NUM_MODS
} meansOfDeath_t;


//---------------------------------------------------------

// gitem_t->type
typedef enum {
	IT_BAD,
	IT_WEAPON,				// EFX: rotate + upscale + minlight
	IT_AMMO,				// EFX: rotate
	IT_ARMOR,				// EFX: rotate + minlight
	IT_HEALTH,				// EFX: static external sphere + rotating internal
	IT_HOLDABLE,			// single use, holdable item
							// EFX: rotate + bob
} itemType_t;



typedef struct gitem_s {
	char		*classname;	// spawning name
	char		*pickup_sound;
	char		*world_model;

	char		*icon;
	char		*pickup_name;	// for printing on pickup

	int			quantity;		// for ammo how much, or duration of powerup
	itemType_t  giType;			// IT_* flags

	int			giTag;

	char		*precaches;		// string of all models and images this item will use
	char		*sounds;		// string of all sounds this item will use
	vec3_t		mins;			// Bbox
	vec3_t		maxs;			// Bbox
} gitem_t;

// included in both the game dll and the client
extern	gitem_t	bg_itemlist[];
extern	const int		bg_numItems;


//==============================================================================

typedef struct ginfoitem_s 
{
	char				*infoString;// Text message
	vec3_t				color;		// Text color

} ginfoitem_t;

//==============================================================================

extern weaponData_t weaponData[];

//==============================================================================
extern ammoData_t ammoData[];

//==============================================================================

gitem_t	*FindItem( const char *pickupName );
gitem_t	*FindItemForWeapon( weapon_t weapon );
gitem_t	*FindItemForHoldable( holdable_t pw );
#define	ITEM_INDEX(x) ((x)-bg_itemlist)

qboolean	BG_CanItemBeGrabbed( const entityState_t *ent, const playerState_t *ps );


// content masks
#define	MASK_ALL				(-1)
#define	MASK_SOLID				(CONTENTS_SOLID)
#define	MASK_PLAYERSOLID		(CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_BODY)
#define	MASK_NPCSOLID			(CONTENTS_SOLID|CONTENTS_MONSTERCLIP|CONTENTS_BODY)
#define	MASK_DEADSOLID			(CONTENTS_SOLID|CONTENTS_PLAYERCLIP)
#define	MASK_WATER				(CONTENTS_WATER|CONTENTS_LAVA|CONTENTS_SLIME)
#define	MASK_OPAQUE				(CONTENTS_SOLID|CONTENTS_SLIME|CONTENTS_LAVA)
#define	MASK_SHOT				(CONTENTS_SOLID|CONTENTS_BODY|CONTENTS_CORPSE|CONTENTS_SHOTCLIP)

//
// entityState_t->eType
//
typedef enum {
	ET_GENERAL,
	ET_PLAYER,
	ET_ITEM,
	ET_MISSILE,
	ET_MOVER,
	ET_BEAM,
	ET_PORTAL,
	ET_SPEAKER,
	ET_PUSH_TRIGGER,
	ET_TELEPORT_TRIGGER,
	ET_INVISIBLE,
	ET_THINKER,

	ET_EVENTS				// any of the EV_* events can be added freestanding
							// by setting eType to ET_EVENTS + eventNum
							// this avoids having to set eFlags and eventNum
} entityType_t;



void	EvaluateTrajectory( const trajectory_t *tr, int atTime, vec3_t result );
void	EvaluateTrajectoryDelta( const trajectory_t *tr, int atTime, vec3_t result );

void AddEventToPlayerstate( int newEvent, int eventParm, playerState_t *ps );
int	CurrentPlayerstateEvent( playerState_t *ps );

void PlayerStateToEntityState( playerState_t *ps, entityState_t *s );

qboolean	BG_PlayerTouchesItem( playerState_t *ps, entityState_t *item, int atTime );

#endif//#ifndef __BG_PUBLIC_H__