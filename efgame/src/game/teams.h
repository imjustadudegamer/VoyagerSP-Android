typedef enum //# team_e
{
	TEAM_FREE,			// caution, some code checks a team_t via "if (!team_t_varname)" so I guess this should stay as entry 0, great or what? -slc
	TEAM_STARFLEET,
	TEAM_BORG,
	TEAM_PARASITE,
	TEAM_SCAVENGERS,
	TEAM_KLINGON,
	TEAM_MALON,
	TEAM_HIROGEN,
	TEAM_IMPERIAL,
	TEAM_STASIS,
	TEAM_8472,
	TEAM_BOTS,
	TEAM_FORGE,
	TEAM_DISGUISE,
	TEAM_PLAYER,

	//# #eol
	TEAM_NUM_TEAMS
} team_t;
