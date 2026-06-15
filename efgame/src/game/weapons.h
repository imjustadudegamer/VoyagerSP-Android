// Filename:-	weapons.h
//
// Note that this is now included from both server and game modules, so don't include any other header files
//	within this one that might break stuff...

#ifndef __WEAPONS_H__
#define __WEAPONS_H__

typedef enum //# weapon_e
{
	WP_NONE,

	// Player weapons
	WP_PHASER,				 // NOTE: lots of code assumes this is the first weapon (... which is crap) so be careful -Ste.
	WP_COMPRESSION_RIFLE,
	WP_IMOD,
	WP_SCAVENGER_RIFLE,
	WP_STASIS,
	WP_GRENADE_LAUNCHER,
	WP_TETRION_DISRUPTOR,
	WP_QUANTUM_BURST,
	WP_DREADNOUGHT,

	//Special holodeck-only weapon
	WP_PROTON_GUN,

	//Non-weapon starfleet items
	WP_TRICORDER,
	WP_VOYAGER_HYPO,
	WP_BLUE_HYPO,		
	WP_RED_HYPO,

	//NOTE: player can only have up to 16 weapons, anything after that is enemy only
	
	// NPC enemy weapons
	WP_BORG_WEAPON,
	WP_BORG_TASER,
	WP_BORG_ASSIMILATOR,
	WP_BORG_DRILL,

	WP_BOT_WELDER,		// Scout bot	- Short range plasma weilding gun.  Acts like a very short lightning gun
	WP_CHAOTICA_GUARD_GUN,	//			- B/W version of scav rifle for Chaotica's guards.
	WP_BOT_ROCKET,		// Hunter Seeker - Rocket projectile
	WP_FORGE_PROJ,		// Reaver		- Energy projectile weapon (like borg's)
	WP_FORGE_PSYCH,		// Avatar		- "Psychic blast" projectile (?)
	WP_PARASITE,		// Parasite		- shoots acid balls at player

	WP_MELEE,			// Any ol' melee attack
	WP_STASIS_ATTACK,	// Stasis alien attack

	WP_DN_TURRET,		// turret guns on dreadnought level

	WP_KLINGON_BLADE,
	WP_IMPERIAL_BLADE,

	WP_DESPERADO,			// special holo-weapon
	WP_PALADIN,			// special holo-weapon

	//# #eol
	WP_NUM_WEAPONS
} weapon_t;

#define FIRST_WEAPON		WP_PHASER		// this is the first weapon for next and prev weapon switching
#define MAX_PLAYER_WEAPONS	WP_VOYAGER_HYPO	// this is the max you can switch to and get with the give all.

typedef enum //# ammo_e
{
	AMMO_NONE,
	AMMO_STARFLEET,
	AMMO_ALIEN,
	AMMO_PHASER,
	AMMO_MAX
} ammo_t;


typedef struct weaponData_s
{
	char	classname[32];		// Spawning name
	char	weaponMdl[64];		// Weapon Model
	vec3_t	flashColor;			// RGB Color of flash
	char	firingSnd[64];		// Sound made when fired
	char	altFiringSnd[64];	// Sound made when alt-fired
	char	flashSnd[64];		// Sound made by flash
	char	altFlashSnd[64];	// Sound made by an alt-flash
	char	stopSnd[64];		// Sound made when weapon stops firing

	int		ammoIndex;			// Index to proper ammo slot
	int		ammoLow;			// Count when ammo is low

	int		energyPerShot;		// Amount of energy used per shot
	int		fireTime;			// Amount of time between firings
	int		range;				// Range of weapon
	
	int		altEnergyPerShot;	// Amount of energy used for alt-fire
	int		altFireTime;		// Amount of time between alt-firings
	int		altRange;			// Range of alt-fire

	char	weaponIcon[64];		// Name of weapon icon file
	int		numBarrels;			// how many barrels should we expect for this weapon?

	char	missileMdl[64];		// Missile Model
	char	missileSound[64];	// Missile flight sound
	float  	missileDlight;		// what is says
	vec3_t 	missileDlightColor;	// ditto

	char	alt_missileMdl[64];		// Missile Model
	char	alt_missileSound[64];	// Missile sound
	float  	alt_missileDlight;		// what is says
	vec3_t 	alt_missileDlightColor;	// ditto

	char	missileHitSound[64];	// Missile impact sound
	char	altmissileHitSound[64];	// alt Missile impact sound
#ifndef _USRDLL
	void	*func;
	void	*altfunc;
#endif

} weaponData_t;


typedef struct ammoData_s
{
	char	icon[32];	// Name of ammo icon file
	int		max;		// Max amount player can hold of ammo
} ammoData_t;


#endif//#ifndef __WEAPONS_H__
