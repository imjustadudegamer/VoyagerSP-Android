//g_squad.cpp

#include "b_local.h"
#include "g_functions.h"
#include "g_nav.h"
#include "say.h"
#include "g_squad.h"
#include "g_navigator.h"

extern	CNavigator	navigator;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int				num_squad_paths;
squadPath_t		squadPaths[MAX_SQUAD_PATHS];
squadRoute_t	squadRoutes[MAX_SQUAD_PATHS];
static	float	squadPointCheckedDist[MAX_WAYPOINTS_IN_PATH];
static	float	search_depth;
static	float	search_depth_limit;
static	qboolean	branchFound[MAX_WAYPOINTS_IN_PATH][MAX_WAYPOINTS_IN_PATH];

#define	SEARCH_DEPTH_INC	32
#define	MAX_SEARCH_DEPTH	4096

extern int NPC_BuildSquadPointDistances( gentity_t *self, const vec3_t center, squadPath_t *squadPath, int keyWp );
/*
qboolean NAV_BranchFindsWp(int squadPathNum, int start, int end)

Recursive function goes down every branch until reaches end or finds goal waypoint
*/
qboolean NAV_BranchFindsWp(int squadPathNum, int start, int end)
{
	squadPath_t		*path = &squadPaths[squadPathNum];
	float			init_search_depth;
	int				i;
	
	init_search_depth = search_depth;
	squadPointCheckedDist[start] = search_depth;
	for(i = 0; i < MAX_PATH_BRANCHES; i++)
	{
		search_depth = init_search_depth;

		if(path->waypoints[start].nextWp[i] == -1)
		{//no nextWp
			continue;
		}

		search_depth += path->waypoints[start].nextWpDist[i];

		if(search_depth > search_depth_limit)
		{//too far for this check
			continue;
		}

		if(squadPointCheckedDist[path->waypoints[start].nextWp[i]] < search_depth)
		{//Already tried to go through this one at a shorter depth
			continue;
		}

		if(path->waypoints[start].nextWp[i] == end)
		{//a neighbor
			return qtrue;
		}
		else if( NAV_BranchFindsWp(squadPathNum, path->waypoints[start].nextWp[i], end))
		{//branch eventually gets there
			return qtrue;
		}
	}
	
	//no branches off here can get there
	search_depth = init_search_depth;
	return qfalse;
}

/*
int	NAV_NextSquadPoint(int squadPathNum, int start, int end)
	FIXME: doesn't find shortest route, just the first
*/
int	NAV_NextSquadPoint(int squadPathNum, int start, int end)
{
	squadPath_t		*path = &squadPaths[squadPathNum];
	float			init_search_depth;
	int				i;
	
	for(i = 0; i < MAX_WAYPOINTS_IN_PATH; i++)
	{
		squadPointCheckedDist[i] = Q3_INFINITE;
	}
	
	init_search_depth = search_depth = 0;
	squadPointCheckedDist[start] = search_depth;//0, don't backtrack through here at any time

	for(i = 0; i < MAX_PATH_BRANCHES; i++)
	{
		search_depth = init_search_depth;

		if(path->waypoints[start].nextWp[i] == -1)
		{//no nextWp
			continue;
		}

		search_depth += path->waypoints[start].nextWpDist[i];

		if(search_depth > search_depth_limit)
		{//too far for this check
			continue;
		}

		if(squadPointCheckedDist[path->waypoints[start].nextWp[i]] < search_depth)
		{//Already tried to go through this one
			continue;
		}

		if(path->waypoints[start].nextWp[i] == end)
		{//a neighbor
			return path->waypoints[start].nextWp[i];
		}
		else if( NAV_BranchFindsWp(squadPathNum, path->waypoints[start].nextWp[i], end))
		{
			return path->waypoints[start].nextWp[i];
		}
	}
	
	//no branches off here can get there, failed
	search_depth = Q3_INFINITE;
	return -1;
}

//FIXME: calc once and save out in nav file, would speed up map load times and savegame load times
void NAV_GenerateSquadRoutes (int squadPathNum)
{
	squadPath_t		*path = &squadPaths[squadPathNum];
	squadRoute_t	*route = &squadRoutes[squadPathNum];
	int				start, end;

	for(start = 0; start < path->numWaypoints; start++)
	{
		for(end = 0; end < path->numWaypoints; end++)
		{
			route->nextSquadPoint[start][end] = -1;
			route->cost[start][end] = Q3_INFINITE;
			if(end == start)
			{
				continue;
			}

			search_depth_limit = 0;
			while(route->nextSquadPoint[start][end] == -1 && search_depth_limit < MAX_SEARCH_DEPTH)
			{//Keep looking until we find a route or hit the max dearch depth
				search_depth_limit += SEARCH_DEPTH_INC;
				route->nextSquadPoint[start][end] = NAV_NextSquadPoint(squadPathNum, start, end);
				route->cost[start][end] = search_depth;
			}
		}
	}
}

void NAV_insertWaypointInPath (squadPathWaypoint_t *squadPathWp, gentity_t *currWp)
{
	VectorCopy(currWp->currentOrigin, squadPathWp->origin);
	squadPathWp->leadDist = currWp->speed;

	/*
	if(currWp->paintarget && currWp->paintarget[0])
	{
		squadPathWp->target = G_NewString( currWp->paintarget );
	}
	else
	{
		squadPathWp->target = NULL;
	}
	*/
}

/*
void NAV_LinkSquadPointNeighbors(int squadPathNum, int wp1, int wp2, char *wpName)

  Links 2 squadpoints as their next free neighbors, avoids doing so if they were already connected
*/
void NAV_LinkSquadPointNeighbors(int squadPathNum, int wp1, int wp2, char *wpName)
{
	int	k;

	if(!branchFound[wp1][wp2])
	{
		for(k = 0; k < MAX_PATH_BRANCHES; k++)
		{
			if(squadPaths[num_squad_paths].waypoints[wp1].nextWp[k] == -1)
			{
				squadPaths[num_squad_paths].waypoints[wp1].nextWp[k] = wp2;
				if ( k > 1 )
				{//This one has three paths in and out, it branches
					squadPaths[num_squad_paths].waypoints[wp1].flags |= SPF_BRANCH;
				}
				break;
			}

			if( k == MAX_PATH_BRANCHES)
			{
				G_Error("ERROR: Squad Path %s has exceeded max number of branches on waypoint (%s)\n", squadPaths[num_squad_paths].ownername, wpName);
				return;
			}
		}
		branchFound[wp1][wp2] = qtrue;
	}

	if(!branchFound[wp2][wp1])
	{
		for(k = 0; k < MAX_PATH_BRANCHES; k++)
		{
			if(squadPaths[num_squad_paths].waypoints[wp2].nextWp[k] == -1)
			{
				squadPaths[num_squad_paths].waypoints[wp2].nextWp[k] = wp1;
				if ( k > 1 )
				{//This one has three paths in and out, it branches
					squadPaths[num_squad_paths].waypoints[wp2].flags |= SPF_BRANCH;
				}
				break;
			}

			if( k == MAX_PATH_BRANCHES)
			{
				G_Error("ERROR: Squad Path %s has exceeded max number of branches on waypoint (%s)\n", squadPaths[num_squad_paths].ownername, wpName);
				return;
			}
		}
		branchFound[wp2][wp1] = qtrue;
	}
}


/*
void NAV_GenerateSquadPaths (void)

  FIXME: make "waypoints" a pointer in squadPath struct
			use a static local waypoint array struct in this func to find all waypoints in a path
			When have count, G_Alloc a waypoint array of the needed size and point to it from the squadPath

  FIXME: even if has 1 branch, will find it 4 times
*/


#define USE_PRECALCED_SQD_FILES

#ifdef USE_PRECALCED_SQD_FILES

#define SQD_VERSION_NUMBER 2

	extern int giMapChecksum;

	typedef struct
	{
		// verification fields...
		//
		int iVersionNumber;
		int iMAP_CRC;
		int iSizeof_squadPaths;
		int iSizeof_squadRoutes;
		//
		// data fields...
		//
		int				iNumSquadPaths;
//		squadPath_t	 	squadPaths[MAX_SQUAD_PATHS];	// no longer declared, since only NZ entries saved out
//		squadRoute_t	squadRoutes[MAX_SQUAD_PATHS];		

	} SQDHEADER;

#endif

void NAV_GenerateSquadPaths (void)
{
	gentity_t	*currWp = NULL;

#ifdef USE_PRECALCED_SQD_FILES

	char sFilenameSQD[MAX_QPATH];
	qboolean qbUseSQDFile = qfalse;

	// first, we can do a quick check as to whether or not there are any squad paths without having to load a file
	//	and read out a zero entry...
	//
	qboolean qbAtLeastOneSquadPathExists = qfalse;
	currWp = NULL;
	while(NULL != (currWp = G_Find(currWp, FOFS(classname), "waypoint_squadpath")) )
	{
		qbAtLeastOneSquadPathExists = qtrue;
		break;
	}

	if (qbAtLeastOneSquadPathExists)
	{
		// so do we have a pre-calc'd file of squad data?...
		//		
		Q_strncpyz(sFilenameSQD,va("maps/%s.sqd",level.mapname),sizeof(sFilenameSQD));

		byte *pbData = NULL;
		/*int iSize = */gi.FS_ReadFile( sFilenameSQD, (void **)&pbData);
		if (pbData)
		{
			SQDHEADER* pSquadHeader = (SQDHEADER*) pbData;

			// check the header is valid...
			//
			if (pSquadHeader->iVersionNumber		== SQD_VERSION_NUMBER	&&
				pSquadHeader->iMAP_CRC				== giMapChecksum		&&
				pSquadHeader->iSizeof_squadPaths	== sizeof(squadPaths)	&&
				pSquadHeader->iSizeof_squadRoutes	== sizeof(squadRoutes)
				)
			{
				// data looks valid, let's use it...
				//
				qbUseSQDFile = qtrue;	// tell rest of function not to re-calc

				num_squad_paths =  pSquadHeader->iNumSquadPaths;

				squadPath_t*	pLoadedSquadPathData	= (squadPath_t*)	&pSquadHeader[1];
				squadRoute_t*	pLoadedSquadRouteData	= (squadRoute_t*)	&pLoadedSquadPathData[num_squad_paths];
				
				memcpy(squadPaths, pLoadedSquadPathData,	num_squad_paths * sizeof(squadPath_t));
				memcpy(squadRoutes,pLoadedSquadRouteData,	num_squad_paths * sizeof(squadRoute_t));
				gi.Printf( "%d squadPaths loaded\n", num_squad_paths );
			}
			gi.FS_FreeFile(pbData);
		}
	}

	if (!qbUseSQDFile)
#endif	// #ifdef USE_PRECALCED_SQD_FILES
	{
		gentity_t	*nextWp, *targWp, *branchWp;
		vec3_t		vec;
		int			i, j, k;
		int			numWaypoints;
		int			branches[1024];
		int			branchNextWp[1024];
		int			branchWpIndex[1024];
		int			num_branches;
		int			lastWpIndex;

		num_squad_paths = 0;
		currWp = NULL;
		while(0!=(currWp = G_Find(currWp, FOFS(classname), "waypoint_squadpath")))
		{
			if(!currWp->ownername || !currWp->ownername[0])
			{//Only start on master ones
				continue;
			}

			for(i = 0; i < num_squad_paths; i++)
			{
				if(Q_stricmp(squadPaths[i].ownername, currWp->ownername) == 0)
				{
					gi.Printf("Warning, more than one squad path has ownername of %s\n", currWp->ownername);
					continue;
				}
			}
		
			if(num_squad_paths >= MAX_SQUAD_PATHS)
			{
				G_Error("Error: Too many squad paths!\n");
				return;
			}

			//Ok, add a new path
			for(i = 0; i < 1024; i++)
			{
				branches[i] = -1;
				branchNextWp[i] = -1;
				branchWpIndex[i] = -1;
			}

			for(i = 0; i < MAX_WAYPOINTS_IN_PATH; i++)
			{
				for(j = 0; j < MAX_WAYPOINTS_IN_PATH; j++)
				{
					branchFound[i][j] = qfalse;
				}
			}

			Q_strncpyz(squadPaths[num_squad_paths].ownername,currWp->ownername,sizeof(squadPaths[0].ownername));
			NAV_insertWaypointInPath(&squadPaths[num_squad_paths].waypoints[0], currWp);
			currWp->wpIndex = 1;

			nextWp = currWp;
			numWaypoints = 0;
			num_branches = 0;

			targWp = G_Find(NULL, FOFS(targetname), currWp->target);//Find the first one
			currWp->target = NULL;
			lastWpIndex = numWaypoints;
			while(targWp)
			{	//Check for branches
				branchWp = NULL;
				if(NULL != (branchWp = G_Find(NULL, FOFS(targetname), nextWp->target2)))
				{
					nextWp->target2 = NULL;
				}
				else if(NULL != (branchWp = G_Find(NULL, FOFS(targetname), nextWp->target3)) )
				{
					nextWp->target3 = NULL;
				}
				else if(NULL != (branchWp = G_Find(NULL, FOFS(targetname), nextWp->target4)) )
				{
					nextWp->target4 = NULL;
				}

				if(branchWp)
				{//This waypoint branches, we'll revisit it when we're done
					branches[num_branches] = nextWp->s.number;
					branchNextWp[num_branches] = branchWp->s.number;
					branchWpIndex[num_branches] = lastWpIndex;
					num_branches++;
					squadPaths[num_squad_paths].waypoints[lastWpIndex].flags |= SPF_BRANCH;
				}

				//Now see what we have here
				if(targWp->wpIndex > 0)
				{//This waypoint has already been linked in, don't loop
					//Tell the last wp to link to a previous one
					NAV_LinkSquadPointNeighbors(num_squad_paths, lastWpIndex, targWp->wpIndex - 1, nextWp->targetname);
					//NAV_LinkSquadPointNeighbors(num_squad_paths, lastWpIndex, targWp->wpIndex, nextWp->targetname);
					targWp = NULL;//End of branch
				}
				else
				{
					numWaypoints++;
					if(numWaypoints >= MAX_WAYPOINTS_IN_PATH)
					{
						G_Error("ERROR: Squad Path %s has exceeded max number of waypoints (%d)\n", squadPaths[num_squad_paths].ownername, MAX_WAYPOINTS_IN_PATH);
						return;
					}
					//copy into waypoint data
					NAV_insertWaypointInPath(&squadPaths[num_squad_paths].waypoints[numWaypoints], targWp);
					//Set nextWp's first empty branchWp to targWp, and vice-versa
					NAV_LinkSquadPointNeighbors(num_squad_paths, lastWpIndex, numWaypoints, nextWp->targetname);
					
					targWp->wpIndex = numWaypoints + 1;//+1 so we can test for nonzero below

					lastWpIndex = numWaypoints;

					nextWp = targWp;
					if(nextWp->target)
					{
						targWp = G_Find(NULL, FOFS(targetname), nextWp->target);
						nextWp->target = NULL;
					}
					else
					{
						targWp = NULL;
					}
				}
			}
			squadPaths[num_squad_paths].waypoints[numWaypoints].flags |= SPF_END_OF_BRANCH;

			//Ok, now go back and handle every unprocessed branch
			while(num_branches)
			{
				for(i = 0; i < 1024; i++)
				{
					if(branches[i] != -1)
					{
						nextWp = &g_entities[branches[i]];
						targWp = &g_entities[branchNextWp[i]];
						lastWpIndex = branchWpIndex[i];
						while(targWp)
						{//Fill in this entire branch
							//Look for branches off of it
							branchWp = NULL;
							if(NULL != (branchWp = G_Find(NULL, FOFS(targetname), nextWp->target2)) )
							{
								if(branchWp == targWp)
								{//We're processing this branch right now (Shouldn't happen)
									branchWp = NULL;
								}
								else
								{
									nextWp->target2 = NULL;
								}
							}
							if(branchWp == NULL)
							{
								if(NULL != (branchWp = G_Find(NULL, FOFS(targetname), nextWp->target3)) )
								{
									if(branchWp == targWp)
									{//We're processing this branch right now (Shouldn't happen)
										branchWp = NULL;
									}
									else
									{
										nextWp->target3 = NULL;
									}
								}
							}
							if(branchWp == NULL)
							{
								if(NULL != (branchWp = G_Find(NULL, FOFS(targetname), nextWp->target4)) )
								{
									if(branchWp == targWp)
									{//We're processing this branch right now (Shouldn't happen)
										branchWp = NULL;
									}
									else
									{
										nextWp->target4 = NULL;
									}
								}
							}
							if(branchWp)
							{//This waypoint branches, we'll revisit it when we're done
								for(j = 0; j < 1024; j++)
								{//Look for an empty slot
									if(branches[j] == -1)
									{//fill in the new branch info and we'll get back to it later
										branches[j] = nextWp->s.number;
										branchNextWp[j] = branchWp->s.number;
										branchWpIndex[j] = lastWpIndex;
										break;
									}
								}
								num_branches++;
								squadPaths[num_squad_paths].waypoints[lastWpIndex].flags |= SPF_BRANCH;
							}

							//Ok, let's check out this waypoint
							if(targWp->wpIndex > 0)
							{//This waypoint has already been linked in, don't loop
								//Tell the last wp to link to a previous one
								NAV_LinkSquadPointNeighbors(num_squad_paths, lastWpIndex, targWp->wpIndex - 1, nextWp->targetname);
								//NAV_LinkSquadPointNeighbors(num_squad_paths, lastWpIndex, targWp->wpIndex, nextWp->targetname);
								targWp = NULL;//End of branch
							}
							else
							{
								numWaypoints++;
								if(numWaypoints >= MAX_WAYPOINTS_IN_PATH)
								{
									G_Error("ERROR: Squad Path %s has exceeded max number of waypoints (%d)\n", squadPaths[num_squad_paths].ownername, MAX_WAYPOINTS_IN_PATH);
									return;
								}

								NAV_insertWaypointInPath(&squadPaths[num_squad_paths].waypoints[numWaypoints], targWp);
								//Set it is the next empty branch off last wp, & vice-versa
								NAV_LinkSquadPointNeighbors(num_squad_paths, lastWpIndex, numWaypoints, nextWp->targetname);

								targWp->wpIndex = numWaypoints + 1;//+1 so we can test for nonzero below

								lastWpIndex = numWaypoints;

								nextWp = targWp;
								if(nextWp->target)
								{
									targWp = G_Find(NULL, FOFS(targetname), nextWp->target);
									nextWp->target = NULL;
								}
								else
								{
									targWp = NULL;
								}
							}
						}
						
						squadPaths[num_squad_paths].waypoints[numWaypoints].flags |= SPF_END_OF_BRANCH;
						branches[i] = -1;
						branchNextWp[i] = -1;
						branchWpIndex[i] = -1;
						num_branches--;
					}
				}
			}
			squadPaths[num_squad_paths].numWaypoints = numWaypoints + 1;
			squadPaths[num_squad_paths].waypoints[numWaypoints].flags |= SPF_END_OF_LIST;

			num_squad_paths++;
		}
		//Precalc distances between them all, then calc the routes
		for(i = 0; i < num_squad_paths; i++)
		{
			for(j = 0; j < squadPaths[i].numWaypoints; j++)
			{
				for(k = 0; k < MAX_PATH_BRANCHES; k++)
				{
					if(squadPaths[i].waypoints[j].nextWp[k] != -1)
					{
						VectorSubtract(squadPaths[i].waypoints[squadPaths[i].waypoints[j].nextWp[k]].origin, squadPaths[i].waypoints[j].origin, vec);
						squadPaths[i].waypoints[j].nextWpDist[k] = VectorLength(vec);
					}
				}
			}
			NAV_GenerateSquadRoutes (i);
		}

		#ifdef USE_PRECALCED_SQD_FILES
		if (num_squad_paths)
		{
			// 
			// now save out this data so we can just reload it next time this map is run rather than recalculating it all...
			//
			fileHandle_t file;
			gi.FS_FOpenFile( sFilenameSQD, &file, FS_WRITE );
			if ( file  )
			{
				int iOut;	// ensure all fields are written as ints, regardless of original sizes

				// verification fields...

				iOut = SQD_VERSION_NUMBER;
				gi.FS_Write( &iOut, sizeof(iOut), file );

				iOut = giMapChecksum;
				gi.FS_Write( &iOut, sizeof(iOut), file );

				iOut = sizeof(squadPaths);
				gi.FS_Write( &iOut, sizeof(iOut), file );

				iOut = sizeof(squadRoutes);
				gi.FS_Write( &iOut, sizeof(iOut), file );

				// data fields...

				iOut = num_squad_paths;
				gi.FS_Write( &iOut, sizeof(iOut), file );

				gi.FS_Write( &squadPaths, num_squad_paths * sizeof(squadPath_t), file );
				gi.FS_Write( &squadRoutes,num_squad_paths * sizeof(squadRoute_t),file );
				
				gi.FS_FCloseFile( file );
			}
		}
		#endif
		gi.Printf( "%d squadPaths generated\n", num_squad_paths );
	}

	//All paths made, now free all waypoint_squadpaths
	currWp = NULL;
	while(NULL != (currWp = G_Find(currWp, FOFS(classname), "waypoint_squadpath")) )
	{
		currWp->e_ThinkFunc = thinkF_G_FreeEntity;
		currWp->nextthink = level.time + FRAMETIME;
	}
}

void G_SquadPathsInit (void)
{
	int	h, i, j;

	num_squad_paths = 0;
	for(h = 0; h < MAX_SQUAD_PATHS; h++)
	{
		squadPaths[h].numWaypoints = 0;
		for(i = 0; i < MAX_WAYPOINTS_IN_PATH; i ++)
		{
			for(j = 0; j < MAX_PATH_BRANCHES; j++)
			{
				squadPaths[h].waypoints[i].nextWp[j] = -1;
			}
		}
	}
}

/*QUAKED waypoint_squadpath  (0.3 0.7 1) (-12 -12 -24) (12 12 32)
A waypoint for squad paths not navigation

target/targetname - Link these in a path starting with the first waypoint
branch using target2, target3 and target4
IMPORTANT!  don't use target2 without having a first target, and so on...

ownername - bot_targetname of bot that should use this waypoint when following player
	WARNING: Only the start waypoint on this path should have the ownername field!

leadDist - How far ahead/behind the NPC should be when the player is close to this squadpoint

<squadtarget - Entity to use when squadmate gets here> - being removed
FIXME: these points are easy to miss because of collision avoidance, so squadtargets are probably not the best way to do this, use trigger_formation instead

Future - be able to set a script or bstate for a bot to use when get here?!
*/

extern	unsigned int waypoint_getRadius( gentity_t *ent );

void SP_waypoint_squadpath (gentity_t *ent)
{
	VectorSet(ent->mins, DEFAULT_MINS_0, DEFAULT_MINS_1, DEFAULT_MINS_2);
	VectorSet(ent->maxs, DEFAULT_MAXS_0, DEFAULT_MAXS_1, DEFAULT_MAXS_2);
	
	ent->contents = CONTENTS_TRIGGER;
	ent->clipmask = MASK_DEADSOLID;

	gi.linkentity( ent );

	if(!ent->target || !ent->target[0])
	{
		if(!ent->target2 || !ent->target2[0])
		{
			if(!ent->target3 || !ent->target3[0])
			{
				if(!ent->target4 || !ent->target4[0])
				{
					//Hmm... dead end
				}
				else
				{
					G_Error("ERROR: waypoint_squadpath %s has a target4 but not target, target2 or target3!!!\n", ent->targetname);
					G_FreeEntity(ent);
					return;
				}
			}
			else
			{
				G_Error("ERROR: waypoint_squadpath %s has a target3 but not target or target2!!!\n", ent->targetname);
				G_FreeEntity(ent);
				return;
			}
		}
		else
		{
			G_Error("ERROR: waypoint_squadpath %s has a target2 but not target!!!\n", ent->targetname);
			G_FreeEntity(ent);
			return;
		}
	}

	if(G_CheckInSolid (ent, qtrue))
	{
		ent->maxs[2] = CROUCH_MAXS_2;
		if(G_CheckInSolid (ent, qtrue))
		{
			gi.Printf(S_COLOR_RED"ERROR: waypoint_squadpath %s at %s in solid!\n", ent->targetname, vtos(ent->currentOrigin));
			assert(0 && "ERROR: Waypoint_squadpath in solid!");
#ifndef FINAL_BUILD
			G_Error("Waypoint_squadpath %s at %s in solid!\n", ent->targetname, vtos(ent->currentOrigin));
#endif
			//G_FreeEntity(ent);
			return;
		}
	}
}

//CONSOLE COMMANDS===========================================================================
void NPC_SetSayState (gentity_t *self, gentity_t *to, int saying)
{
	self->NPC->tempBehavior = BS_SAY;
	self->NPC->sayTarg = to;
	self->NPC->sayString = (saying_t)saying;
}

void NPC_RecieveCommand(gentity_t *commander, gentity_t *self, int command, const char *extra)
{
	gentity_t	*commTarg = NULL;
	gentity_t	*next;

	if(extra && extra[0])
	{//Find target of command, if we can
		//next = Munro;//Munro is a global pointer to the player?
		next = &g_entities[0];
		while(next->client->follower)//This is the linked list of your followers...
		{
			next = next->client->follower;
			if(Q_stricmp(extra, next->script_targetname) == 0)
			{//Found the person we're hailing
				commTarg = next;
				break;
			}
		}
	}

	switch(command)
	{
	case CMD_REGROUP:
		//Get back into normal BS_FORMATION bState
		self->NPC->behaviorState = BS_FORMATION;
		//Set a position?  Only if don't have one?  How would we choose?
		//Should we have a func that finds which unfilled position you're closest to???
		break;
	case CMD_GUARD:
		//Go into BS_STAND_GUARD bState
		self->NPC->behaviorState = BS_STAND_GUARD;
		break;
	case CMD_RETREAT:
		//Find farthest path point from enemy that is in PVS of leader and head there (BS_FORMATION)
		//set FPOS to FPOS_NONE???  How do we know when to regroup?  Wait for manual command?
		if(self->enemy && self->NPC->iSquadPathIndex!=-1)
		{
			NPC_BuildSquadPointDistances( self, self->enemy->currentOrigin, &squadPaths[self->NPC->iSquadPathIndex], WAYPOINT_NONE );
		}
		else
		{//If no squadpath, should we find some other way to retreat?
			goto cannot_comply;
		}
		break;
	case CMD_COVER:
		if(commTarg == NULL)
		{
			goto cannot_comply;
		}
		//self->NPC->behaviorState = BS_COVER;
		self->NPC->coverTarg = commTarg;
		break;
	case CMD_ESCORT:
		if(commTarg == NULL)
		{
			goto cannot_comply;
		}
		//self->NPC->behaviorState = BS_COVER;
		self->NPC->coverTarg = commTarg;
		break;
	case CMD_UNKNOWN:
		//TempBState- turn to player and say "copy that again, sir?" or "I didn't quite get that" or "what?" or "I think I got some interference, can you repeat that?"
		NPC_SetSayState(self, commander, Q_irand(SAY_BADCOMM1, SAY_BADCOMM4));
		return;
		break;
	}
	//Say "copy" or "yes sir" or "okay" or "gotcha" or "right"
	NPC_SetSayState(self, commander, Q_irand(SAY_ACKCOMM1, SAY_ACKCOMM4));
	return;

cannot_comply:
	//say "can't do that, sir!" or "no way" or something
	NPC_SetSayState(self, commander, Q_irand(SAY_REFCOMM1, SAY_REFCOMM4));
}

void Svcmd_Comm_f( void )
{
	char	*cmdName;
	int		command;
	gentity_t	*next;

	cmdName = gi.argv(1);

	if(!cmdName || !cmdName[0])
	{//FIXME: error message
		return;
	}
	//Commands will be issued to anyone who was hailed and is awaiting orders
	//While waiting, comm will take the first arg and translate it as a command, some
	//	are whole commands, some require a third command, such as:
	//	"regroup"
	//	"stand guard"
	//	"retreat"
	//	"take" <"cover" or position (scout, point, flank, backup, rear)>
	//	"cover" <"me" or NPC name>
	//	"escort" <"me" or NPC name>
	//NPC will acknowledge the command and change state accordingly.
	//After 3 seconds or upon receipt of a command, it will clear the waiting flags
	//	of each NPC for the next command.

	if(Q_stricmp("regroup", cmdName) == 0)
	{
		command = (int)CMD_REGROUP;
	}
	else if(Q_stricmp("stand guard", cmdName) == 0)
	{
		command = (int)CMD_GUARD;
	}
	else if(Q_stricmp("retreat", cmdName) == 0)
	{
		command = (int)CMD_RETREAT;
	}
	else if(Q_stricmp("cover", cmdName) == 0)
	{
		command = (int)CMD_COVER;
	}
	else if(Q_stricmp("escort", cmdName) == 0)
	{
		command = (int)CMD_ESCORT;
	}
	else 
	{
		command = (int)CMD_UNKNOWN;
	}

	//next = Munro;//Munro is a global pointer to the player?
	next = &g_entities[0];
	while(next->client && next->client->follower)//This is the linked list of your followers...
	{
		next = next->client->follower;
		if(next->NPC->aiFlags & NPCAI_AWAITING_COMM)//This gets cleared by a 3 sec timer on them in thier think
		{
			next->NPC->aiFlags &= ~NPCAI_AWAITING_COMM;
			NPC_RecieveCommand(&g_entities[0], next, command, gi.argv(2));
		}
	}
}

/*
void NPC_HailSquadMate (gentity_t *squadMate)
	FIXME: they should know who the hail came form so they know who to respond to if it's unfinished

	FIXME: Should we queue up multiple hails?  Or only respond to last one?
*/
void NPC_HailSquadMate (gentity_t *self, gentity_t *squadMate)
{
	if(squadMate->NPC->aiFlags & NPCAI_AWAITING_COMM)
	{//Already hailed them
		//FIXME: Make sure you were last one to hail them before doing this?
		NPC_SetSayState(squadMate, self, Q_irand(SAY_BADHAIL1, SAY_BADHAIL4));
		squadMate->NPC->commWaitTime = level.time + COMM_WAIT_TIME;
	}
	else
	{
		squadMate->NPC->aiFlags |= NPCAI_AWAITING_COMM;
		squadMate->NPC->commWaitTime = level.time + COMM_WAIT_TIME;
	}
}

void Svcmd_Hail_f( void )
{
	char	*targName;
	gentity_t	*next;

	targName = gi.argv(1);

	//Comm will take a first arg of the NPC to talk to (or "all" for entire squad),
	//	will init them to wait 3 seconds for a command.  After 3 seconds, you have
	//	to hail them again.
	//	If no command comes in 1 second, they'll say "sir?" or "yeah?" or "what?"
	if(!targName || !targName[0])
	{//FIXME: error message
		return;
	}

	if(Q_stricmp("all", targName) == 0 || Q_stricmp("everyone", targName) == 0 || Q_stricmp("squad", targName) == 0)
	{//Hail everyone in your squad
		//next = Munro;//Munro is a global pointer to the player?
		next = &g_entities[0];
		while(next->client && next->client->follower)//This is the linked list of your followers...
		{
			next = next->client->follower;
			NPC_HailSquadMate(&g_entities[0], next);
		}
	}
	else
	{
		//next = Munro;//Munro is a global pointer to the player?
		next = &g_entities[0];
		while(next->client && next->client->follower)//This is the linked list of your followers...
		{
			next = next->client->follower;
			if(Q_stricmp(targName, next->script_targetname) == 0)
			{//Found the person we're hailing
				NPC_HailSquadMate(&g_entities[0], next);
				break;
			}
		}
	}
}

void G_CreateFormation (gentity_t *self);
void Svcmd_Form_f( void )
{
	G_CreateFormation(&g_entities[0]);
}

//===Spawn func==========================================================================

/*QUAKED target_create_formation (0 0.5 0) (-4 -4 -4) (4 4 4) 
player fires this off and will make a formation (one time only)
*/
void target_create_formation_use( gentity_t *self, gentity_t *other, gentity_t *activator ) 
{
	if (self->behaviorSet[BSET_USE])
	{
		G_ActivateBehavior(self,BSET_USE);
	}

	G_CreateFormation(&g_entities[0]);

	G_FreeEntity(self);
}

void SP_target_create_formation (gentity_t *self)
{
	self->e_UseFunc = useF_target_create_formation_use;
}
