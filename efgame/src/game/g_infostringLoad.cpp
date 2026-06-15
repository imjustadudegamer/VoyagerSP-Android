//g_infostringLoad.cpp
//reads in ext_data\infostrings.dat

#include "g_local.h"
#include "g_items.h"
#include "g_infostrings.h"

qboolean G_ParseInt( char **data, int *i );
qboolean G_ParseString( char **data, char **s ); 

extern ginfoitem_t	bg_infoItemList[];

typedef struct  
{
	char	*infoString;
	int	infoItemNum;
} infoStringTable_s;

infoStringTable_s infoStringTable[II_NUM_ITEMS] = 
{
	"II_NONE",					II_NONE,
	"II_HEALTH_REGEN",			II_HEALTH_REGEN,		
	"II_ENERGY_REGEN",			II_ENERGY_REGEN,
	"II_AMMO_REGEN",			II_AMMO_REGEN,
	"II_PLASMA_FILTER",			II_PLASMA_FILTER,
	"II_ISODESIUM",				II_ISODESIUM,
	"II_DIS_NODE",				II_DIS_NODE,
	"II_IMOD",					II_IMOD,
	"II_SEDATIVE",				II_SEDATIVE,
	"II_TRUTH_SERUM",			II_TRUTH_SERUM,
	"II_CONTROL_SWITCH",		II_CONTROL_SWITCH,
	"II_POWER_CRYSTAL",			II_POWER_CRYSTAL,
	"II_STARFLEET_HELMET",		II_STARFLEET_HELMET,
	"II_ELEVATOR_SWITCH",		II_ELEVATOR_SWITCH,
	"II_TURBOLIFT_PANEL",		II_TURBOLIFT_PANEL,
	"II_LIFT_PANEL",			II_LIFT_PANEL,
	"II_CONTROL_PANEL",			II_CONTROL_PANEL,
	"II_BORG1",					II_BORG1,
	"II_BORG2",					II_BORG2,
	"II_HOLODECK",				II_HOLODECK,
	"II_VOY1",					II_VOY1,
	"II_VOY2",					II_VOY2,
	"II_VOY3",					II_VOY3,
	"II_VOY4",					II_VOY4,
	"II_VOY5",					II_VOY5,
	"II_STASIS1",				II_STASIS1,
	"II_STASIS2",				II_STASIS2,
	"II_STASIS3",				II_STASIS3,
	"II_VOY6",					II_VOY6,
	"II_VOY7",					II_VOY7,
	"II_VOY8", 					II_VOY8,
	"II_SCAV1",					II_SCAV1,
	"II_SCAV2",					II_SCAV2,
	"II_SCAV3",					II_SCAV3,
	"II_SCAV3B",				II_SCAV3B,
	"II_SCAV4",					II_SCAV4,
	"II_SCAV5",					II_SCAV5,
	"II_SCAVBOSS",				II_SCAVBOSS,
	"II_VOY9",					II_VOY9,
	"II_BORG3",					II_BORG3,
	"II_BORG4",					II_BORG4,
	"II_BORG5",					II_BORG5,
	"II_BORG6",					II_BORG6,
	"II_VOY13",					II_VOY13,
	"II_VOY14",					II_VOY14,
	"II_VOY15",					II_VOY15,
	"II_DN1",					II_DN1,
	"II_DN2",					II_DN2,
	"II_DN3",					II_DN3,
	"II_DN4",					II_DN4,
	"II_DN5",					II_DN5,
	"II_TRAIN",					II_TRAIN,
	"II_DN6",					II_DN6,
	"II_DN8",					II_DN8,
	"II_VOY16",					II_VOY16,
	"II_VOY17",					II_VOY17,
	"II_FORGE1",				II_FORGE1,
	"II_FORGE2",				II_FORGE2,
	"II_FORGE3",				II_FORGE3,
	"II_FORGE4",				II_FORGE4,
	"II_FORGE5",				II_FORGE5,
	"II_FORGEBOSS",				II_FORGEBOSS,
	"II_VOY20",					II_VOY20,
	"II_TUTORIAL",				II_TUTORIAL,
	"II_NOVEL",					II_NOVEL,
	"II_DECK_01",				II_DECK_01,
	"II_DECK_02",				II_DECK_02,
	"II_DECK_04",				II_DECK_04,
	"II_DECK_05",				II_DECK_05,
	"II_DECK_08",				II_DECK_08,
	"II_DECK_09",				II_DECK_09,
	"II_DECK_10",				II_DECK_10,
	"II_DECK_11",				II_DECK_11,
	"II_PHASER_CONTROL",		II_PHASER_CONTROL,
	"II_PHOTON_TORP_CONTROL",	II_PHOTON_TORP_CONTROL,
	"II_DECK_15",				II_DECK_15,
	"II_QTRS_JANEWAY",			II_QTRS_JANEWAY,
	"II_QTRS_CHAKOTAY",			II_QTRS_CHAKOTAY,
	"II_QTRS_TUVOK",			II_QTRS_TUVOK,
	"II_QTRS_TORRES",			II_QTRS_TORRES,
	"II_QTRS_PARIS",			II_QTRS_PARIS,
	"II_QTRS_KIM",				II_QTRS_KIM,
	"II_QTRS_NEELIX",			II_QTRS_NEELIX,
	"II_QTRS_MUNRO_MALE",		II_QTRS_MUNRO_MALE,
	"II_QTRS_MUNRO_FEMALE",		II_QTRS_MUNRO_FEMALE,
	"II_QTRS_CHANG",			II_QTRS_CHANG,
	"II_QTRS_BIESSMAN",			II_QTRS_BIESSMAN,
	"II_QTRS_MURPHY",			II_QTRS_MURPHY,
	"II_QTRS_CHELL",			II_QTRS_CHELL,
	"II_QTRS_JUROT",			II_QTRS_JUROT,
	"II_PANEL_AUTODESTRUCT",	II_PANEL_AUTODESTRUCT,
	"II_PANEL_SECURITY",		II_PANEL_SECURITY,
	"II_PANEL_TRANSPORTER",		II_PANEL_TRANSPORTER,
	"II_PANEL_REDALERT",		II_PANEL_REDALERT,
	"II_PANEL_SHUTTLELAUNCH",	II_PANEL_SHUTTLELAUNCH,
	"II_PANEL_HOLODECK",		II_PANEL_HOLODECK,
	"II_PANEL_LIBRARY",			II_PANEL_LIBRARY,
	"II_PANEL_ASTROMETRICS",	II_PANEL_ASTROMETRICS,
	"II_PERSONALLOG_FOSTER",	II_PERSONALLOG_FOSTER,
	"II_PERSONALLOG_MUNRO",		II_PERSONALLOG_MUNRO,
	"II_PERSONALLOG_CHANG",		II_PERSONALLOG_CHANG,
	"II_PERSONALLOG_BIESSMAN",	II_PERSONALLOG_BIESSMAN,
	"II_PERSONALLOG_MURPHY",	II_PERSONALLOG_MURPHY,
	"II_PERSONALLOG_CHELL",		II_PERSONALLOG_CHELL,
	"II_PERSONALLOG_JUROT",		II_PERSONALLOG_JUROT,
	"II_PERSONALLOG_CAPTAIN",	II_PERSONALLOG_CAPTAIN,
	"II_PERSONALLOG_CHAKOTAY",	II_PERSONALLOG_CHAKOTAY,
	"II_PERSONALLOG_TUVOK",		II_PERSONALLOG_TUVOK,
	"II_PERSONALLOG_TORRES",	II_PERSONALLOG_TORRES,
	"II_PERSONALLOG_PARIS",		II_PERSONALLOG_PARIS,
	"II_PERSONALLOG_KIM",		II_PERSONALLOG_KIM,
	"II_PERSONALLOG_DOCTOR",	II_PERSONALLOG_DOCTOR,
	"II_PERSONALLOG_SEVEN",		II_PERSONALLOG_SEVEN,
	"II_PERSONALLOG_NEELIX",	II_PERSONALLOG_NEELIX,
	"II_MEDICAL_LOG",			II_MEDICAL_LOG,
	"II_OBJECTIVE_LOG",			II_OBJECTIVE_LOG,
	"II_DECK_03",				II_DECK_03,
	"II_TURBOLIFT",				II_TURBOLIFT,

	"II_MESSHALL",				II_MESSHALL,
	"II_CHAKOTAY_OFFICE",		II_CHAKOTAY_OFFICE,
	"II_AUXILIARY_CARGO_BAY",	II_AUXILIARY_CARGO_BAY,
	"II_HAZARD_BRIEFING",		II_HAZARD_BRIEFING,
	"II_HAZARD_ARMORY",			II_HAZARD_ARMORY,
	"II_HAZARD_TRANSPORTER",	II_HAZARD_TRANSPORTER,
	"II_HAZARD_LOCKERROOM",		II_HAZARD_LOCKERROOM,
	"II_HAZARD_LOUNGE",			II_HAZARD_LOUNGE,
	"II_BRIG",					II_BRIG,
	"II_SICKBAY",				II_SICKBAY,
	"II_MAIN_CARGOBAY",			II_MAIN_CARGOBAY,
	"II_ASTROMETRICS",			II_ASTROMETRICS,
	"II_JEFFRIES_TUBE_ACCESS",	II_JEFFRIES_TUBE_ACCESS,
	"II_MAINENGINEERING",		II_MAINENGINEERING,
	"II_SHUTTLEBAY",			II_SHUTTLEBAY,
	"II_VENTRAL_MAINTENANCE",	II_VENTRAL_MAINTENANCE,

	"II_PANEL_TACTICAL",		II_PANEL_TACTICAL,
	"II_PANEL_OPS",				II_PANEL_OPS,
	"II_PANEL_ENGINEERING",		II_PANEL_ENGINEERING,
	"II_PANEL_NAVIGATION",		II_PANEL_NAVIGATION,
	"II_PADD_SHIPMAINTENANCE",	II_PADD_SHIPMAINTENANCE,
	"II_PADD_REDALERT_DRILLS",	II_PADD_REDALERT_DRILLS,
	"II_PADD_SENSORUSE",		II_PADD_SENSORUSE,
	"II_PADD_DEPARTMENTSTATUS",	II_PADD_DEPARTMENTSTATUS,
	"II_PADD_PROTONEPISODES",	II_PADD_PROTONEPISODES,
	"II_PADD_RECIPES",			II_PADD_RECIPES,
	"II_PADD_SOCIAL_EVENTS",	II_PADD_SOCIAL_EVENTS,
	"II_PANEL_SHOOTING_RANGE",	II_PANEL_SHOOTING_RANGE,
	"II_PANEL_WEAPONLIBRARY",	II_PANEL_WEAPONLIBRARY,
	"II_PANEL_DISEASELIBRARY",	II_PANEL_DISEASELIBRARY,
	"II_PANEL_CARGO_LOG",		II_PANEL_CARGO_LOG,
	"II_PANEL_ENGINEERING_DATABASE",II_PANEL_ENGINEERING_DATABASE,
	"II_PERSONALLOG_DISCHLER",	II_PERSONALLOG_DISCHLER,
	"II_HAZARDGAME",			II_HAZARDGAME,
	"II_STARFLEET_PADD",		II_STARFLEET_PADD,
	"II_SHUTTLEBAY_DOOR",		II_SHUTTLEBAY_DOOR,
	"II_ATMOSPHERIC_CONTAINMENT",II_ATMOSPHERIC_CONTAINMENT,
	"II_HARVESTER_LEG",			II_HARVESTER_LEG,
	"II_EF_POSTER",				II_EF_POSTER,
	"II_ACTION_FIGURE",			II_ACTION_FIGURE,
	"II_COLLECTORS_PLATE",		II_COLLECTORS_PLATE,
	"II_STASIS_FIREFLIES",		II_STASIS_FIREFLIES,
	"II_ARC_WELDER",			II_ARC_WELDER,
	"II_GRENADE_LAUNCHER",		II_GRENADE_LAUNCHER,
	"II_BORG_SLAYER_GAME",		II_BORG_SLAYER_GAME,
	"II_RAVEN_PLAQUE",			II_RAVEN_PLAQUE,

	"II_WARPCORE",				II_WARPCORE,
	"II_SHUTTLECRAFT",			II_SHUTTLECRAFT,
	"II_DOOR",					II_DOOR,
	"II_PLANT",					II_PLANT,
	"II_MEDICAL_EQUIPMENT",		II_MEDICAL_EQUIPMENT,
	"II_BARREL",				II_BARREL,
	"II_UNIFORM",				II_UNIFORM,
	"II_HELMET",				II_HELMET,
	"II_CARGO_CONTAINER",		II_CARGO_CONTAINER,
	"II_TOOLBOX",				II_TOOLBOX,
	"II_CHAOTICA_BUST",			II_CHAOTICA_BUST,
	"II_DECK_MAP",				II_DECK_MAP,
	"II_SWORD",					II_SWORD,
	"II_PHONOGRAPH",			II_PHONOGRAPH,
	"II_PADD",					II_PADD,
	"II_BOOK",					II_BOOK,
	"II_MICROSCOPE",			II_MICROSCOPE,
	"II_TEASET",				II_TEASET,
	"II_ART",					II_ART,
	"II_MUNRO_PICTURE",			II_MUNRO_PICTURE,
	"II_MAINTENANCE_CONTROL_PANEL",II_MAINTENANCE_CONTROL_PANEL,
	"II_SECURITY_ACCESS_CONTROL_PANEL",II_SECURITY_ACCESS_CONTROL_PANEL,
	"II_COMPUTER_CORE_CONTROL_PANEL",II_COMPUTER_CORE_CONTROL_PANEL,
	"II_BOWL",					II_BOWL,
	"II_KITCHENWARE",			II_KITCHENWARE,
	"II_TABLE",					II_TABLE,
	"II_FORCEFIELD",			II_FORCEFIELD,
	"II_COMPUTER",				II_COMPUTER,
	"II_WORF",					II_WORF,
	"II_LANDINGGEAR",			II_LANDINGGEAR,

	"II_ROASTBEEF",				II_ROASTBEEF,
	"II_PORKCHOPS",				II_PORKCHOPS,
	"II_RUMPROAST",				II_RUMPROAST,
	"II_STEAK",					II_STEAK,

	"II_WINE",					II_WINE,
	"II_BRANDY",				II_BRANDY,
	"II_LATOUR",				II_LATOUR,
	"II_BERGUNDY",				II_BERGUNDY,

	"II_COFFEE",				II_COFFEE,
	"II_JUICE",					II_JUICE,
	"II_TEA",					II_TEA,
	"II_LANDRASCOFFEE",			II_LANDRASCOFFEE,

	"II_PASTA",					II_PASTA,
	"II_BANTAN",				II_BANTAN,
	"II_CARDAWAY",				II_CARDAWAY,
	"II_CRISPS",				II_CRISPS,

	"II_STEAK_SALAD",			II_STEAK_SALAD,
	"II_STEW_VEGG",				II_STEW_VEGG,
	"II_SOUP_BISCUITS",			II_SOUP_BISCUITS,
	"II_SOUP_FRITTERS",			II_SOUP_FRITTERS,
	"II_PHOTO",					II_PHOTO,
	"II_CHESS_SET",				II_CHESS_SET,
	"II_RUGBY_TROPHY",			II_RUGBY_TROPHY,
	"II_EVIL_STRONGHOLD",		II_EVIL_STRONGHOLD,
	"II_LABYRINTH",				II_LABYRINTH,
};

struct 
{
	int	infoItemNum;
} infoItemParms;


static void IS_InfoItem (char **holdBuf);
static void IS_Infostring (char **holdBuf);
static void IS_TextColor (char **holdBuf);

typedef struct 
{
	char	*parmName;
	void	(*func)(char **holdBuf);
} infostringParms_t;


#define IS_PARM_MAX 3

infostringParms_t infostringParms[IS_PARM_MAX] = 
{
	"infoitem",			IS_InfoItem,
	"infostring",		IS_Infostring,
	"textcolor",		IS_TextColor,
};


//------------------------------------------------
int IS_GetInfostring( char *infostring_enum )
{
	int		infoItemNum,i;

	infoItemNum = II_NONE;

	// Search for matching string
	for (i=0;i<II_NUM_ITEMS ;i++)
	{
		if (!Q_stricmp(infostring_enum,infoStringTable[i].infoString))
		{
			infoItemNum = infoStringTable[i].infoItemNum;
			break;
		}
	}

	if (i>= II_NUM_ITEMS)
	{
		infoItemNum = II_NONE;
#ifdef FINAL_BUILD
		gi.Printf( S_COLOR_RED"ERROR: Bad infostring_enum name '%s'\n", infostring_enum);
#else
		G_Error("Bad infostring_enum name '%s'\n", infostring_enum);
#endif
	}

	return infoItemNum;
}

//------------------------------------------------
static void IS_SetDefaults()
{
	// No defaults
}

//------------------------------------------------
static void IS_InfoItem(char **holdBuf)
{
	int		infoItemNum,i;
	char	*tokenStr;

	if (G_ParseString(holdBuf,&tokenStr)) 
	{
		return;
	}

	infoItemNum = II_NONE;

	// Search for matching string
	for (i=0;i<II_NUM_ITEMS ;i++)
	{
		if (!Q_stricmp(tokenStr,infoStringTable[i].infoString))
		{
			infoItemNum = infoStringTable[i].infoItemNum;
			break;
		}
	}

	if (i>= II_NUM_ITEMS)
	{
		infoItemNum = 0;
		gi.Printf( S_COLOR_YELLOW"WARNING: bad infoItem name in external infostrings data '%s'\n", tokenStr);
	}

	infoItemParms.infoItemNum = infoItemNum;	

	IS_SetDefaults();
}

//------------------------------------------------
static void IS_Infostring( char **holdBuf )
{
	int		len;
	char	*tokenStr;

	if ( G_ParseString( holdBuf, &tokenStr ) ) 
	{
		return;
	}

	len = strlen( tokenStr );
	len++;

	if ( len > 64 )
	{
		len = 64;
		gi.Printf( S_COLOR_YELLOW"WARNING: infostring text too long in external infostrings.DAT '%s'\n", tokenStr );
	}

	bg_infoItemList[infoItemParms.infoItemNum].infoString = G_NewString(tokenStr);
}

//------------------------------------------------
static void IS_TextColor(char **holdBuf)
{
	vec3_t	yellow = { 0.9f, 0.7f, 0.0 };
	vec3_t	red	= { 1.0, 0.0, 0.0 };
	char	*tokenStr;

	if (G_ParseString(holdBuf,&tokenStr)) 
	{
		return;
	}

	if (!Q_stricmp(tokenStr,"TC_YELLOW"))
		VectorCopy( yellow, bg_infoItemList[infoItemParms.infoItemNum].color );
	else if (!Q_stricmp(tokenStr,"TC_RED"))	
		VectorCopy( red, bg_infoItemList[infoItemParms.infoItemNum].color );
	else
	{
		gi.Printf( S_COLOR_YELLOW"WARNING: bad infoItem color in external infostrings data '%s'\n", tokenStr);
	}
}

//------------------------------------------------
static void IS_ParseInfostringParms( char **holdBuf )
{
	char	*token;
	int		i;

	while (holdBuf)
	{
		token = COM_ParseExt( holdBuf, qtrue );

		if (!Q_stricmp( token, "}" ))	// End of data for this infostring
			break;

		// Loop through possible parameters
		for ( i = 0; i < IS_PARM_MAX; ++i )
		{
			if (!Q_stricmp(token,infostringParms[i].parmName))	
			{
				infostringParms[i].func(holdBuf);
				break;
			}
		}

		if (i < IS_PARM_MAX)	// Find parameter???
		{
			continue;
		}

		gi.Printf( S_COLOR_YELLOW"WARNING: bad parameter in external weapon data '%s'\n", token);
		SkipRestOfLine(holdBuf);
		
	}
}

//------------------------------------------------
static void IS_ParseParms(char *buffer)
{
	char	*holdBuf;
	char	*token;

	holdBuf = buffer;
	COM_BeginParseSession();

	while ( holdBuf ) 
	{
		token = COM_ParseExt( &holdBuf, qtrue );

		if ( !Q_stricmp( token, "{" ) ) 
		{
			token =token;
			IS_ParseInfostringParms(&holdBuf);
		}
	}
}

//------------------------------------------------
void IS_LoadInfoItemParms (void)
{
	char	*buffer;
	char	finalName[MAX_QPATH];
	int		len;

	G_LanguageFilename("ext_data/infostrings","dat",(char *) &finalName);

	len = gi.FS_ReadFile(finalName,(void **) &buffer);

	IS_ParseParms(buffer);

	gi.FS_FreeFile( buffer );	//let go of the buffer
}

