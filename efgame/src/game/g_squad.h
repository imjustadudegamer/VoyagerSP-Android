#ifndef __G_SQUAD_H__
#define __G_SQUAD_H__

/*
#define MAX_PMOVES			64
#define PMOVES_INCR			10	//Num pmoves to increase RouteGen surf depth by
#define PMOVE_MSEC			50
#define PLAYER_RUNSPEED		320
#define PLAYER_WALKSPEED	75
#define PLAYER_SWIMSPEED	320
#define MAX_JUMPHEIGHT		32
#define	PMOVE_YAWSPEED		50.0f
#define MIN_EDGELIP			4		//Minimum lip that you can stand on

#define	MAX_WAYPOINTS			128
#define	MAX_NEIGHBORS			8
#define	MAX_ROUTES_PER_WP_PAIR	4


#define MAX_NUM_CONSECUTIVE_BLOCKED_MOVES	4//4 moves in a row were blocked totally

#define	WPF_DUCK		1
#define	WPF_NOT_FIRST	128

#define	NF_DUCK			1
#define	NF_FALL1		2
#define	NF_FALL2		4

typedef enum //# movetype_e
{
	MT_STATIC = 0,
	MT_WALK,
	MT_RUNJUMP,
	MT_FLYSWIM,
	NUM_MOVETYPES
} movetype_t;

typedef struct
{//FIXME:Structure kinda big, optimize?
	vec3_t	origin;
	float	radius;
	int		entNum[MAX_NEIGHBORS][NUM_MOVETYPES];
	int		nextWaypoint[MAX_NEIGHBORS][NUM_MOVETYPES];
	int		nextWaypointCost[MAX_NEIGHBORS][NUM_MOVETYPES];//# of Pmoves
	int		nextWaypointFlags[MAX_NEIGHBORS][NUM_MOVETYPES];//long fall involved, etc.
	int		flags;
	float	height;
	char	*script;
	char	*targetname;
	char	*ownername;
	byte	numNeighbors;
} waypoint_t;

typedef struct
{
	int			nextWaypoint;
	int			cost;//approximate # of Pmoves, probably higher, depending on radius of waypoints involved
	int			flags;//add up all flags of every waypoint connection along route
	//Add conditions?  Or should that be per waypoint connection?
	//Store the entire path as waypoint number indices?!
	int			number;//This is annoying, but until I have some better way of distinguishing between routes, this will remain
} route_t;

typedef struct
{
	route_t		routes[MAX_WAYPOINTS][MAX_WAYPOINTS][MAX_ROUTES_PER_WP_PAIR];
} routeTable_t;

//RouteGen
#define		MAX_WP_DEPTH	128//There IS a reason for this...

typedef struct {
	int		waypoint[MAX_WP_DEPTH];
	byte	stepCost[MAX_WP_DEPTH];
} path_t;

#define		ROUTE_BAD		-1
#define		WAYPOINT_NONE	-1

//MoveToGoal
#define		MAX_COLL_AVOID_DIST	128
*/
//===SQUAD PATHS==========================================================================

#define	MAX_SQUAD_PATHS			8	//Never more than 8 additional squad members?
#define MAX_WAYPOINTS_IN_PATH	128	//Never more than 128 waypoints in a squad path?
#define MAX_PATH_BRANCHES		8	//Each squadpoint never has more than 8 branches off it
#define	MAX_SEGMENT_DIST_FROM_LEADER_SQUARED	16384//128^2 Yes, I know, it's an overly long name!
#define MAX_WAYPOINT_REACHED_DIST_SQUARED		1024//32 squared

//squadPath waypoint flags
#define	SPF_BRANCH			1
#define	SPF_END_OF_BRANCH	2
#define	SPF_END_OF_LIST		4

#define COMM_WAIT_TIME		3000//3 sec

typedef enum //# squadCmd_e
{
	CMD_REGROUP,
	CMD_GUARD,
	CMD_RETREAT,
	CMD_COVER,
	CMD_ESCORT,
	CMD_UNKNOWN,
	//# #eol
	NUM_CMDS
} squadCmd_t;

typedef struct
{
	vec3_t	origin;
	//char	*target;
	int		flags;
	int		leadDist;
	int		nextWp[MAX_PATH_BRANCHES];
	float	nextWpDist[MAX_PATH_BRANCHES];
} squadPathWaypoint_t;

typedef struct
{
	char	ownername[MAX_QPATH];
	squadPathWaypoint_t	waypoints[MAX_WAYPOINTS_IN_PATH];
	int		numWaypoints;
	float	distances[MAX_WAYPOINTS_IN_PATH];
	int		closestWaypoints[MAX_WAYPOINTS_IN_PATH];
} squadPath_t;

typedef struct
{
	int		nextSquadPoint[MAX_WAYPOINTS_IN_PATH][MAX_WAYPOINTS_IN_PATH];
	float	cost[MAX_WAYPOINTS_IN_PATH][MAX_WAYPOINTS_IN_PATH];
} squadRoute_t;

// NOTENOTE!!!! These 3 items not REM'd below are the only ones saved out in the precalc'd file for squad routes,
//					if you ever change them or add more fields then tell me! -ste
//

//extern	int				num_waypoints;
//extern	int				num_routes;
extern	int				num_squad_paths;
//extern	waypoint_t		waypoints[MAX_WAYPOINTS];
//extern	routeTable_t	routeTables[NUM_MOVETYPES];
//extern	int				waypointConnections[MAX_WAYPOINTS][MAX_WAYPOINTS];
extern	squadPath_t		squadPaths[MAX_SQUAD_PATHS];
extern	squadRoute_t	squadRoutes[MAX_SQUAD_PATHS];

#endif //#ifndef __G_SQUAD_H__