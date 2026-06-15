#ifndef __Q3_INTERFACE__
#define __Q3_INTERFACE__

//NOTENOTE: The enums and tables in this file will obviously bitch if they are included multiple times, don't do that

typedef enum //# setType_e
{
	//# #sep Parm strings
	SET_PARM1 = 0,//## %s="" # Set entity parm1
	SET_PARM2,//## %s="" # Set entity parm2
	SET_PARM3,//## %s="" # Set entity parm3
	SET_PARM4,//## %s="" # Set entity parm4
	SET_PARM5,//## %s="" # Set entity parm5
	SET_PARM6,//## %s="" # Set entity parm6
	SET_PARM7,//## %s="" # Set entity parm7
	SET_PARM8,//## %s="" # Set entity parm8
	SET_PARM9,//## %s="" # Set entity parm9
	SET_PARM10,//## %s="" # Set entity parm10
	SET_PARM11,//## %s="" # Set entity parm11
	SET_PARM12,//## %s="" # Set entity parm12
	SET_PARM13,//## %s="" # Set entity parm13
	SET_PARM14,//## %s="" # Set entity parm14
	SET_PARM15,//## %s="" # Set entity parm15
	SET_PARM16,//## %s="" # Set entity parm16

	// NOTE!!! If you add any other SET_xxxxxxSCRIPT types, make sure you update the 'case' statements in 
	//	ICARUS_InterrogateScript() (game/g_ICARUS.cpp), or the script-precacher won't find them.

	//# #sep Scripts and other file paths
	SET_SPAWNSCRIPT,//## %s="NULL" !!"Q:\quake\baseEF\real_scripts\!!#*.txt" # Script to run when spawned //0 - do not change these, these are equal to BSET_SPAWN, etc
	SET_IDLESCRIPT,//## %s="NULL" !!"Q:\quake\baseEF\real_scripts\!!#*.txt" # NOT IMPLEMENTED
	SET_TOUCHSCRIPT,//## %s="NULL" !!"Q:\quake\baseEF\real_scripts\!!#*.txt" # NOT IMPLEMENTED
	SET_USESCRIPT,//## %s="NULL" !!"Q:\quake\baseEF\real_scripts\!!#*.txt" # Script to run when used
	SET_AWAKESCRIPT,//## %s="NULL" !!"Q:\quake\baseEF\real_scripts\!!#*.txt" # Script to run when startled
	SET_ANGERSCRIPT,//## %s="NULL" !!"Q:\quake\baseEF\real_scripts\!!#*.txt" # Script run when find an enemy for the first time
	SET_ATTACKSCRIPT,//## %s="NULL" !!"Q:\quake\baseEF\real_scripts\!!#*.txt" # Script to run when you shoot
	SET_VICTORYSCRIPT,//## %s="NULL" !!"Q:\quake\baseEF\real_scripts\!!#*.txt" # Script to run when killed someone
	SET_LOSTENEMYSCRIPT,//## %s="NULL" !!"Q:\quake\baseEF\real_scripts\!!#*.txt" # Script to run when you can't find your enemy
	SET_PAINSCRIPT,//## %s="NULL" !!"Q:\quake\baseEF\real_scripts\!!#*.txt" # Script to run when hit
	SET_FLEESCRIPT,//## %s="NULL" !!"Q:\quake\baseEF\real_scripts\!!#*.txt" # Script to run when hit and low health
	SET_DEATHSCRIPT,//## %s="NULL" !!"Q:\quake\baseEF\real_scripts\!!#*.txt" # Script to run when killed
	SET_DELAYEDSCRIPT,//## %s="NULL" !!"Q:\quake\baseEF\real_scripts\!!#*.txt" # Script to run after a delay
	SET_BLOCKEDSCRIPT,//## %s="NULL" !!"Q:\quake\baseEF\real_scripts\!!#*.txt" # Script to run when blocked by teammate
	SET_FFIRESCRIPT,//## %s="NULL" !!"Q:\quake\baseEF\real_scripts\!!#*.txt" # Script to run when player has shot own team repeatedly
	SET_FFDEATHSCRIPT,//## %s="NULL" !!"Q:\quake\baseEF\real_scripts\!!#*.txt" # Script to run when player kills a teammate
	SET_PRECACHE,//## %s="NULL" !!"Q:\quake\baseEF\real_scripts\!!#*.pre" # DISABLED!
	SET_VIDEO_PLAY,//## %s="filename" !!"Q:\quake\baseEF\video\!!#*.bik" # Play a bink video (inGame)

	//# #sep Standard strings
	SET_ENEMY,//## %s="NULL" # Set enemy by targetname
	SET_LEADER,//## %s="NULL" # Set for BS_FOLLOW_LEADER
	SET_NAVGOAL,//## %s="NULL" # *Move to this navgoal then continue script
	SET_CAPTURE,//## %s="NULL" # Set captureGoal by targetname
	SET_HIDING,//## %s="NULL" # Set hidingGoal by targetname
	SET_VIEWTARGET,//## %s="NULL" # Set angles toward ent by targetname
	SET_SQUADNAME,//## %s="NULL" # Who's squad you're in
	SET_TARGETNAME,//## %s="NULL" # Set/change your targetname
	SET_PAINTARGET,//## %s="NULL" # Set/change what to use when hit
	SET_CAMERA_GROUP,//## %s="NULL" # all ents with this cameraGroup will be focused on
	SET_LOOK_TARGET,//## %s="NULL" # object for NPC to look at
	SET_CAPTIONTEXTCOLOR,	//## %s=""  # Color of text RED,WHITE,BLUE, YELLOW
	SET_CENTERTEXTCOLOR,	//## %s=""  # Color of text RED,WHITE,BLUE, YELLOW
	SET_SCROLLTEXTCOLOR,	//## %s=""  # Color of text RED,WHITE,BLUE, YELLOW
	SET_COPY_ORIGIN,//## %s="targetname"  # Copy the origin of the ent with targetname to your origin
	SET_DEFEND_TARGET,//## %s="targetname"  # This NPC will attack the target NPC's enemies
	SET_TARGET,//## %s="NULL" # Set/change your target
	SET_TARGET2,//## %s="NULL" # Set/change your target2, on NPC's, this fires when they're knocked out by the red hypo
	SET_LOCATION,//## %s="INVALID" # What trigger_location you're in - Can only be gotten, not set!
	SET_PLUG_TARGET,//## %s="INVALID" # A target for a head bot to plug into - will only return when it gets there
	SET_REMOVE_TARGET,//## %s="NULL" # Target that is fired when someone completes the BS_REMOVE behaviorState
	SET_LOADGAME,//## %s="exitholodeck" # Load the savegame that was auto-saved when you started the holodeck
	SET_CON_COMMAND,//## %s="disconnect" # DO NOT USE UNLESS YOU KNOW WHAT YOU'RE DOING!  Stuff a console command - default is disconnect
	SET_LOCKYAW,//## %s="off"  # Lock legs to a certain yaw angle (or "off" or "auto" uses current)
	SET_FULLNAME,//## %s="NULL" # This name will appear when ent is scanned by tricorder

	SET_SCROLLTEXT,	//## %s="" # key of text string to print
	SET_LCARSTEXT,	//## %s="" # key of text string to print in LCARS frame

	//# #sep vectors
	SET_ORIGIN,//## %v="0.0 0.0 0.0" # Set origin explicitly or with TAG
	SET_ANGLES,//## %v="0.0 0.0 0.0" # Set angles explicitly or with TAG
	SET_VELOCITY,//## %v="0.0 0.0 0.0"  # NOT IMPLEMENTED
	SET_AVELOCITY,//## %v="0.0 0.0 0.0"  # NOT IMPLEMENTED
	SET_TELEPORT_DEST,//## %v="0.0 0.0 0.0" # Set origin here as soon as the area is clear
	
	//# #sep floats
	SET_XVELOCITY,//## %f="0.0" # Velocity along X axis
	SET_YVELOCITY,//## %f="0.0" # Velocity along Y axis
	SET_ZVELOCITY,//## %f="0.0" # Velocity along Z axis
	SET_DPITCH,//## %f="0.0" # Pitch for NPC to turn to
	SET_DYAW,//## %f="0.0" # Yaw for NPC to turn to
	SET_TIMESCALE,//## %f="0.0" # Speed-up slow down game (0 - 1.0)
	SET_VISRANGE,//## %f="0.0" # How far away NPC can see
	SET_EARSHOT,//## %f="0.0" # How far an NPC can hear
	SET_VIGILANCE,//## %f="0.0" # How often to look for enemies (0 - 1.0)
	SET_GRAVITY,//## %f="0.0" # Change this ent's gravity - 800 default
	SET_PLAYER_WIDTH,//## %f="0.0" # 24 default, applies to player and all NPCs
	SET_FACEAUX,		//## %f="0.0" # Set face to Aux expression for number of seconds
	SET_FACEBLINK,		//## %f="0.0" # Set face to Blink expression for number of seconds
	SET_FACEBLINKFROWN,	//## %f="0.0" # Set face to Blinkfrown expression for number of seconds
	SET_FACEFROWN,		//## %f="0.0" # Set face to Frown expression for number of seconds
	SET_FACENORMAL,		//## %f="0.0" # Set face to Normal expression for number of seconds
	SET_WAIT,		//## %f="0.0" # Change an entity's wait field
	SET_FOLLOWDIST,		//## %f="0.0" # How far away to stay from leader in BS_FOLLOW_LEADER

	//# #sep ints
	SET_ANIM_HOLDTIME_LOWER,//## %d="0" # Hold lower anim for number of milliseconds
	SET_ANIM_HOLDTIME_UPPER,//## %d="0" # Hold upper anim for number of milliseconds
	SET_ANIM_HOLDTIME_BOTH,//## %d="0" # Hold lower and upper anims for number of milliseconds
	SET_HEALTH,//## %d="0" # Change health
	SET_ARMOR,//## %d="0" # Change armor
	SET_WALKSPEED,//## %d="0" # Change walkSpeed
	SET_RUNSPEED,//## %d="0" # Change runSpeed
	SET_YAWSPEED,//## %d="0" # Change yawSpeed
	SET_AGGRESSION,//## %d="0" # Change aggression 1-5
	SET_AIM,//## %d="0" # Change aim 1-5
	SET_FRICTION,//## %d="0" # Change ent's friction - 6 default
	SET_SHOOTDIST,//## %d="0" # How far the ent can shoot - 0 uses weapon
	SET_HFOV,//## %d="0" # Horizontal field of view
	SET_VFOV,//## %d="0" # Vertical field of view
	SET_DELAYSCRIPTTIME,//## %d="0" # How many milliseconds to wait before running delayscript
	SET_FORWARDMOVE,//## %d="0" # NPC move forward -127(back) to 127
	SET_RIGHTMOVE,//## %d="0" # NPC move right -127(left) to 127
	SET_STARTFRAME,	//## %d="0" # frame to start animation sequence on
	SET_ENDFRAME,	//## %d="0" # frame to end animation sequence on
	SET_WIDTH,//## %d="0" # width of player/NPC
	SET_HEIGHT,//## %d="0" # standing height of of player/NPC
	SET_CROUCHHEIGHT,//## %d="0" # crouching height of of player/NPC
	SET_COUNT,	//## %d="0" # Change an entity's count field
	SET_SHOT_SPACING,//## %d="1000" # Time between shots for an NPC - reset to defaults when changes weapon
	SET_MISSIONSTATUSTIME,//## %d="0" # Amount of time until Mission Status should be shown after death
	SET_MAX_AMMO,	//## %d="0" # Set max ammos to specific value

	//# #sep booleans
	SET_IGNOREPAIN,//## %t="BOOL_TYPES" # Do not react to pain
	SET_IGNOREENEMIES,//## %t="BOOL_TYPES" # Do not acquire enemies
	SET_IGNOREALERTS,//## %t="BOOL_TYPES" # Do not get enemy set by allies in area(ambush)
	SET_STRAIGHTTOGOAL,//## %t="BOOL_TYPES" # Don't use waypoint network to get to goal
	SET_DONTSHOOT,//## %t="BOOL_TYPES" # Others won't shoot you
	SET_NOTARGET,//## %t="BOOL_TYPES" # Others won't pick you as enemy
	SET_DONTFIRE,//## %t="BOOL_TYPES" # Don't fire your weapon
	SET_NOSLOWDOWN,//## %t="BOOL_TYPES" # Don't slow down approaching navgoals
	SET_LOCKED_ENEMY,//## %t="BOOL_TYPES" # Keep current enemy until dead
	SET_CROUCHED,//## %t="BOOL_TYPES" # Force NPC to crouch
	SET_WALKING,//## %t="BOOL_TYPES" # Force NPC to move at walkSpeed
	SET_RUNNING,//## %t="BOOL_TYPES" # Force NPC to move at runSpeed
	SET_CAREFUL,//## %t="BOOL_TYPES" # Force NPC to use careful, weapon ready anims
	SET_UNDYING,//## %t="BOOL_TYPES" # Can take damage down to 1 but not die
	SET_NOAVOID,//## %t="BOOL_TYPES" # Will not avoid other NPCs or architecture
	SET_BEAM,//## %t="BOOL_TYPES" # Turn on eyebeam, flashlight, etc.
	SET_CREATEFORMATION,//## %t="BOOL_TYPES" # Form the player's squad
	SET_SOLID,//## %t="BOOL_TYPES" # Make yourself notsolid or solid
	SET_PLAYER_USABLE,//## %t="BOOL_TYPES" # Can be activateby the player's "use" button
	SET_LOOP_ANIM,//## %t="BOOL_TYPES" # For non-NPCs, loop your animation sequence
	SET_INTERFACE,//## %t="BOOL_TYPES" # Player interface on/off
	SET_SHIELDS,//## %t="BOOL_TYPES" # NPC has no shields (Borg do not adapt)
	SET_INVISIBLE,//## %t="BOOL_TYPES" # Makes an NPC not solid and not visible
	SET_GREET_ALLIES,//## %t="BOOL_TYPES" # Makes an NPC greet teammates
	SET_VIDEO_FADE_IN,//## %t="BOOL_TYPES" # Makes video playback fade in
	SET_VIDEO_FADE_OUT,//## %t="BOOL_TYPES" # Makes video playback fade out
	SET_PLAYER_LOCKED,//## %t="BOOL_TYPES" # Makes it so player cannot move
	SET_NO_KNOCKBACK,//## %t="BOOL_TYPES" # Stops this ent from taking knockback from weapons
	SET_ALT_FIRE,//## %t="BOOL_TYPES" # Force NPC to use altfire when shooting
	SET_NO_RESPONSE,//## %t="BOOL_TYPES" # NPCs will do generic responses when this is on (usescripts override generic responses as well)
	SET_INVINCIBLE,//## %t="BOOL_TYPES" # Completely unkillable
	SET_MISSIONSTATUSACTIVE,	//# Turns on Mission Status Screen
	SET_NO_COMBAT_TALK,//## %t="BOOL_TYPES" # NPCs will not do their combat talking noises when this is on
	SET_TREASONED,//## %t="BOOL_TYPES" # Player has turned on his own- scripts will stop, NPCs will turn on him and level changes load the brig

	//# #sep calls
	SET_DETPACK,//## %t="BOOL_TYPES" # First call throws detpack, second detonates it
	SET_BOLTON_ON,//## %s="Bolton name" # Turns on specified bolton
	SET_BOLTON_OFF,//## %s="Bolton name" # Turns off specified bolton
	SET_BEAMIN_BOLTON,//## %s="Bolton name" # Beams in and turns on the specified bolton
	SET_BEAMOUT_BOLTON,//## %s="Bolton name" # Beams out the specified bolton
	SET_BOLTON_DROP,//## %s="Bolton name" # Drop the bolton into the world from it's current position - it will have a targetname of "<owner's targetname>_<bolton name>"
	SET_BOLTON_ACTIVE,//## %s="Bolton name" # Name of active boltOn - this boltOn will take all successive animation commands
	SET_BOLTON_STARTFRAME,//## %d="0" # Start frame of anim for active bolton
	SET_BOLTON_ENDFRAME,//## %d="0" # End frame of anim active bolton
	SET_BOLTON_ANIMLOOP,//## %t="BOOL_TYPES" # Whether or not to loop anim
	SET_SKILL,//## %r%d="0" # Cannot set this, only get it - valid values are 0 through 3
	SET_SEX,//## %r%s="NULL" # Cannot set this, only get it - valid values are 'male' or 'female'

	//# #sep Special tables
	SET_ANIM_UPPER,//## %t="ANIM_NAMES" # Torso and head anim
	SET_ANIM_LOWER,//## %t="ANIM_NAMES" # Legs anim
	SET_ANIM_BOTH,//## %t="ANIM_NAMES" # Set same anim on torso and legs
	SET_PLAYER_TEAM,//## %t="TEAM_NAMES" # Your team
	SET_ENEMY_TEAM,//## %t="TEAM_NAMES" # Team in which to look for enemies
	SET_BEHAVIOR_STATE,//## %t="BSTATE_STRINGS" # Change current bState
	SET_DEFAULT_BSTATE,//## %t="BSTATE_STRINGS" # Change fallback bState
	SET_TEMP_BSTATE,//## %t="BSTATE_STRINGS" # Set/Chang a temp bState
	SET_EVENT,//## %t="EVENT_NAMES" # Events you can initiate
	SET_WEAPON,//## %t="WEAPON_NAMES" # Change/Stow/Drop weapon
	SET_OBJECTIVE_SHOW,	//## %t="OBJECTIVES" # Show objective on mission screen
	SET_OBJECTIVE_HIDE,	//## %t="OBJECTIVES" # Hide objective from mission screen
	SET_OBJECTIVE_SUCCEEDED,//## %t="OBJECTIVES" # Mark objective as completed
	SET_OBJECTIVE_FAILED,	//## %t="OBJECTIVES" # Mark objective as failed
	SET_TACTICAL_SHOW,		//## %t="TACTICAL" # Show tactical info on mission objectives screen
	SET_TACTICAL_HIDE,		//## %t="TACTICAL" # Hide tactical info on mission objectives screen
	SET_OBJECTIVE_CLEARALL,	//## # Force all objectives to be hidden
	SET_OBJECTIVEFOSTER,	//## %t="OBJECTIVES" # Check status of Foster Objective
	SET_OBJECTIVEODELL,		//## %t="OBJECTIVES" # Check status of Odell Objective
	SET_OBJECTIVECSATLOS,	//## %t="OBJECTIVES" # Check status of Csatlos Objective
	SET_OBJECTIVEISODESIUM1,//## %t="OBJECTIVES" # Check status of  Isodesium1 Objective
	SET_OBJECTIVEISODESIUM2,//## %t="OBJECTIVES" # Check status of  Isodesium2 Objective
	SET_OBJECTIVEISODESIUM3,//## %t="OBJECTIVES" # Check status of  Isodesium3 Objective
	SET_OBJECTIVEISODESIUMALL,//## %t="OBJECTIVES" # Check status of  Isodesium All Objective
	SET_OBJECTIVESECURITYCODE,//## %t="OBJECTIVES" # Check status of  Security Code Objective

	SET_OBJECTIVE_COLLECTEDACTIONFIGURES,	//## %t="OBJECTIVES" # Check status of Objective
	SET_OBJECTIVE_HARVESTER_LEG,			//## %t="OBJECTIVES" # Check status of Objective
	SET_OBJECTIVE_EF_POSTER,				//## %t="OBJECTIVES" # Check status of Objective
	SET_OBJECTIVE_SEVEN_PLATE,				//## %t="OBJECTIVES" # Check status of Objective
	SET_OBJECTIVE_FIREFLIES,				//## %t="OBJECTIVES" # Check status of Objective
	SET_OBJECTIVE_ARC_WELDER,				//## %t="OBJECTIVES" # Check status of Objective
	SET_OBJECTIVE_GRENADE_LAUNCHER,			//## %t="OBJECTIVES" # Check status of Objective
	SET_OBJECTIVE_BIESSMAN_MORGUE,			//## %t="OBJECTIVES" # Check status of Objective
	SET_OBJECTIVE_BORG_SLAYER,				//## %t="OBJECTIVES" # Check status of Objective
	SET_OBJECTIVE_RAVEN_PLAQUE,				//## %t="OBJECTIVES" # Check status of Objective
	SET_OBJECTIVE_HERETIC_II_BOX,			//## %t="OBJECTIVES" # Check status of Objective
	SET_OBJECTIVE_SOF_BOX,					//## %t="OBJECTIVES" # Check status of Objective

	SET_OBJECTIVE_DOLL_MUNRO,				//## %t="OBJECTIVES" # Check status of Objective
	SET_OBJECTIVE_DOLL_FOSTER,				//## %t="OBJECTIVES" # Check status of Objective
	SET_OBJECTIVE_DOLL_TELSIA,				//## %t="OBJECTIVES" # Check status of Objective
	SET_OBJECTIVE_DOLL_ALEXANDRIA,			//## %t="OBJECTIVES" # Check status of Objective
	SET_OBJECTIVE_DOLL_BIESSMAN,			//## %t="OBJECTIVES" # Check status of Objective
	SET_OBJECTIVE_DOLL_CHELL,				//## %t="OBJECTIVES" # Check status of Objective
	SET_OBJECTIVE_DOLL_CHANG,				//## %t="OBJECTIVES" # Check status of Objective
	SET_OBJECTIVE_DOLL_JUROT,				//## %t="OBJECTIVES" # Check status of Objective
	SET_OBJECTIVE_DOLL_BORG_FOSTER,			//## %t="OBJECTIVES" # Check status of Objective

	SET_OBJECTIVE_PHOTONBURST,				//## %t="OBJECTIVES" # Check status of Objective
	SET_OBJECTIVE_TETRYONDISRUPTOR,			//## %t="OBJECTIVES" # Check status of Objective
	SET_OBJECTIVE_TOURSUCCESSFUL,			//## %t="OBJECTIVES" # Check status of Objective

	SET_MISSIONSTATUSTEXT,	//## %t="STATUSTEXT" # Text to appear in mission status screen
	SET_MENU_SCREEN,//## %t="MENUSCREENS" # Brings up specified menu screen

	SET_CLEAR_BORG_ADAPT,	//## # Reset the borg adapt hit counters, should only be used for start of Borg3
	SET_CLOSINGCREDITS,		//## # Show closing credits

	//in-bhc tables
	SET_LEAN,//## %t="LEAN_TYPES" # Lean left, right or stop leaning

	//# #eol
	SET_
} setType_t;	


// this enum isn't used directly by the game, it's mainly for BehavEd to scan for...
//
typedef enum //# playType_e
{
	//# #sep Types of file to play
	PLAY_ROFF = 0,//## %s="filename" !!"Q:\quake\baseEF\real_scripts\!!#*.rof" # Play a ROFF file

	//# #eol
	PLAY_NUMBEROF

} playType_t;


const	int	Q3_TIME_SCALE	= 1;	//MILLISECONDS

//General
extern	void		Q3_TaskIDClear( int *taskID );
extern	qboolean	Q3_TaskIDPending( gentity_t *ent, taskID_t taskType );
extern	void		Q3_TaskIDComplete( gentity_t *ent, taskID_t taskType );
extern	void		Q3_DPrintf( const char *, ... );

extern	void		Q3_CameraRoll( float angle, float duration );
extern  void		Q3_CameraFollow( const char *name, float speed, float initLerp );
extern  void		Q3_CameraTrack( const char *name, float speed, float initLerp );
extern  void		Q3_CameraDistance( float distance, float initLerp );

//Not referenced directly as script function - all are called through Q3_Set
extern	void		Q3_SetAnimBoth( int entID, const char *anim_name );
extern	void		Q3_SetVelocity( int entID, vec3_t angles );

extern	void		Q3_DeclareVariable ( int type, const char *name );
extern	void		Q3_FreeVariable( const char *name );

extern	void		Q3_DebugPrint( int level, const char *format, ... );

#endif	//__Q3_INTERFACE__