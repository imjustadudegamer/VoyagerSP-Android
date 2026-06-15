#ifndef __AI__
#define __AI__

#include "b_local.h"

//Distance ratings
enum distance_e
{
	DIST_MELEE,
	DIST_LONG,
};

//Attack types
enum attack_e
{
	ATTACK_MELEE,
	ATTACK_RANGE,
};

enum
{
	SQUAD_IDLE,					//No target found, waiting
	SQUAD_STAND_AND_SHOOT,		//Standing in position and shoot (no cover)
	SQUAD_RETREAT,				//Running away from combat
	SQUAD_COVER,				//Under protective cover
	SQUAD_TRANSITION,			//Moving between points, not firing
	SQUAD_POINT,				//On point, laying down suppressive fire
	SQUAD_SCOUT,				//Poking out to draw enemy
	NUM_SQUAD_STATES,
};

//AI_SCAVENGER
void NPC_BSScav_Idle( void );
void NPC_BSScav_Attack( void );
void NPC_BSScav_Patrol( void );
void NPC_BSScav_Investigate( void );
void NPC_BSScav_TakeCover( void );
void NPC_BSScav_Sleep( void );

//AI_WARRIORBOT
void NPC_BSWarrior_Idle( void );
void NPC_BSWarrior_Attack( void );

//AI_SCOUTBOT
void NPC_BSScout_Idle( void );
void NPC_BSScout_Attack( void );

//AI_HUNTERSEEKER
void NPC_BSHunterSeeker_Attack( void );
void NPC_BSHunterSeeker_Idle( void );

//AI_HARVESTER
void NPC_BSHarvester_Idle( void );
void NPC_BSHarvester_Attack( void );

//AI_REAVER
void NPC_BSReaver_Attack( void );
void NPC_BSReaver_Idle( void );

//AI_AVATAR
void NPC_BSAvatar_Attack( void );
void NPC_BSAvatar_Idle( void );

//AI_HEADBOT
void NPC_BSHeadBot_Run( void );

//AI_SPECIES8472
void NPC_BSSpecies_Attack( void );
void NPC_BSSpecies_Idle( void );

//AI_ETHERIAN
void NPC_BSEtherian_Idle( void );
void NPC_BSEtherian_Attack( void );

//AI_PARASITE
void NPC_BSParasite_Idle( void );
void NPC_BSParasite_Attack( void );

//AI_HIROGEN_ALPHA
void NPC_BSHirogenAlpha_Attack( void );
void NPC_BSHirogenAlpha_Idle( void );

//AI_BORG
void NPC_BSBorg_Attack( void );
void NPC_BSBorg_Idle( void );
void NPC_BSBorg_StandAndShoot( void );

//AI_VOHRSOTH
void NPC_BSVohrsoth_Attack( void );

//Utilities
typedef struct AIGroupInfo_s
{
	int	numGroup;
	int	numFront;
	int	numState[ NUM_SQUAD_STATES ];
} AIGroupInfo_t;

int	AI_GetGroupSize( vec3_t origin, int radius, team_t playerTeam, gentity_t *avoid = NULL );
int AI_GetGroupSize( gentity_t *ent, int radius );

void AI_GetGroup( AIGroupInfo_t &group, vec3_t origin, vec3_t angles, int cone, int radius, team_t playerTeam, gentity_t *avoid = NULL, gentity_t *enemy = NULL );
void AI_GetGroup( AIGroupInfo_t &group, gentity_t *ent, int radius );

qboolean AI_CheckEnemyCollision( gentity_t *ent, qboolean takeEnemy = qtrue );
gentity_t *AI_DistributeAttack( gentity_t *attacker, gentity_t *enemy, team_t team, int threshold );

#endif	//__AI__