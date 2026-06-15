//g_itemLoad.cpp
//reads in ext_data\items.dat to bg_itemlist[]

#include "g_local.h"
#include "g_items.h"

#define PICKUPSOUND "sound/weapons/w_pkup.wav"

qboolean G_ParseInt( char **data, int *i );
qboolean G_ParseString( char **data, char **s ); 
qboolean G_ParseFloat( char **data, float *f );

extern gitem_t	bg_itemlist[];

struct 
{
	int	itemNum;
} itemParms;


static void IT_ClassName (char **holdBuf);
static void IT_Count (char **holdBuf);
static void IT_Icon (char **holdBuf);
static void IT_Min (char **holdBuf);
static void IT_Max (char **holdBuf);
static void IT_Name (char **holdBuf);
static void IT_PickupName (char **holdBuf);
static void IT_PickupSound (char **holdBuf);
static void IT_Tag (char **holdBuf);
static void IT_Type (char **holdBuf);
static void IT_WorldModel (char **holdBuf);


typedef struct 
{
	char	*parmName;
	void	(*func)(char **holdBuf);
} itemParms_t;


#define IT_PARM_MAX 11

itemParms_t ItemParms[IT_PARM_MAX] = 
{
	"itemname",			IT_Name,
	"classname",		IT_ClassName,
	"count",			IT_Count,
	"icon",				IT_Icon,
	"min",				IT_Min,
	"max",				IT_Max,
	"pickupname",		IT_PickupName,
	"pickupsound",		IT_PickupSound,
	"tag",				IT_Tag,
	"type",				IT_Type,
	"worldmodel",		IT_WorldModel,
};

static void IT_SetDefaults()
{

	bg_itemlist[itemParms.itemNum].mins[0] = -16;
	bg_itemlist[itemParms.itemNum].mins[1] = -16;
	bg_itemlist[itemParms.itemNum].mins[2] = -2;

	bg_itemlist[itemParms.itemNum].maxs[0] = 16;
	bg_itemlist[itemParms.itemNum].maxs[1] = 16;
	bg_itemlist[itemParms.itemNum].maxs[2] = 16;


	bg_itemlist[itemParms.itemNum].pickup_sound = PICKUPSOUND;	//give it a default sound
	bg_itemlist[itemParms.itemNum].precaches = NULL;
	bg_itemlist[itemParms.itemNum].sounds = NULL;
}

static void IT_Name(char **holdBuf)
{
	int itemNum;
	char	*tokenStr;

	if (G_ParseString(holdBuf,&tokenStr)) 
	{
		return;
	}


	if (!Q_stricmp(tokenStr,"ITM_NONE"))	
		itemNum = ITM_NONE;
	else if (!Q_stricmp(tokenStr,"ITM_BORG_ASSIMILATOR_PICKUP"))	
		itemNum = ITM_BORG_ASSIMILATOR_PICKUP;
	else if (!Q_stricmp(tokenStr,"ITM_BORG_DRILL_PICKUP"))	
		itemNum = ITM_BORG_DRILL_PICKUP;
	else if (!Q_stricmp(tokenStr,"ITM_BORG_TASER_PICKUP"))	
		itemNum = ITM_BORG_TASER_PICKUP;
	else if (!Q_stricmp(tokenStr,"ITM_BORG_WEAPON_PICKUP"))	
		itemNum = ITM_BORG_WEAPON_PICKUP;
	else if (!Q_stricmp(tokenStr,"ITM_COMPRESSION_RIFLE_PICKUP"))	
		itemNum = ITM_COMPRESSION_RIFLE_PICKUP;
	else if (!Q_stricmp(tokenStr,"ITM_DREADNOUGHT_PICKUP"))	
		itemNum = ITM_DREADNOUGHT_PICKUP;
	else if (!Q_stricmp(tokenStr,"ITM_GRENADE_LAUNCHER_PICKUP"))	
		itemNum = ITM_GRENADE_LAUNCHER_PICKUP;
	else if (!Q_stricmp(tokenStr,"ITM_IMOD_PICKUP"))	
		itemNum = ITM_IMOD_PICKUP;
	else if (!Q_stricmp(tokenStr,"ITM_PHASER_PICKUP"))	
		itemNum = ITM_PHASER_PICKUP;
	else if (!Q_stricmp(tokenStr,"ITM_SCAVENGER_RIFLE_PICKUP"))	
		itemNum = ITM_SCAVENGER_RIFLE_PICKUP;
	else if (!Q_stricmp(tokenStr,"ITM_STASIS_PICKUP"))	
		itemNum = ITM_STASIS_PICKUP;
	else if (!Q_stricmp(tokenStr,"ITM_TETRION_DISRUPTOR_PICKUP"))	
		itemNum = ITM_TETRION_DISRUPTOR_PICKUP;
	else if (!Q_stricmp(tokenStr,"ITM_TRICORDER_PICKUP"))	
		itemNum = ITM_TRICORDER_PICKUP;
	else if (!Q_stricmp(tokenStr,"ITM_QUANTUM_BURST_PICKUP"))	
		itemNum = ITM_QUANTUM_BURST_PICKUP;
	else if (!Q_stricmp(tokenStr,"ITM_ISODESIUM_PICKUP"))	
		itemNum = ITM_ISODESIUM_PICKUP;
	else if (!Q_stricmp(tokenStr,"ITM_BOT_WELDER_PICKUP"))	
		itemNum = ITM_BOT_WELDER_PICKUP;
	else if (!Q_stricmp(tokenStr,"ITM_CHAOS_PICKUP"))	
		itemNum = ITM_CHAOS_PICKUP;
	else if (!Q_stricmp(tokenStr,"ITM_BOT_ROCKET_PICKUP"))	
		itemNum = ITM_BOT_ROCKET_PICKUP;
	else if (!Q_stricmp(tokenStr,"ITM_FORGE_PROJ_PICKUP"))	
		itemNum = ITM_FORGE_PROJ_PICKUP;
	else if (!Q_stricmp(tokenStr,"ITM_FORGE_PSYCH_PICKUP"))	
		itemNum = ITM_FORGE_PSYCH_PICKUP;
	else if (!Q_stricmp(tokenStr,"ITM_PARASITE_PICKUP"))	
		itemNum = ITM_PARASITE_PICKUP;
	else if (!Q_stricmp(tokenStr,"ITM_MELEE"))	
		itemNum = ITM_MELEE;
	else if (!Q_stricmp(tokenStr,"ITM_STASIS_ATTACK"))	
		itemNum = ITM_STASIS_ATTACK;
	else if (!Q_stricmp(tokenStr,"ITM_DN_TURRET_PICKUP"))
		itemNum = ITM_DN_TURRET_PICKUP;
	else if (!Q_stricmp(tokenStr,"ITM_STARFLEET_HELMET"))
		itemNum = ITM_STARFLEET_HELMET;
	else if (!Q_stricmp(tokenStr,"ITM_AMMO_CRYSTAL_BORG"))
		itemNum = ITM_AMMO_CRYSTAL_BORG;
	else if (!Q_stricmp(tokenStr,"ITM_AMMO_CRYSTAL_DN"))
		itemNum = ITM_AMMO_CRYSTAL_DN;
	else if (!Q_stricmp(tokenStr,"ITM_AMMO_CRYSTAL_FORGE"))
		itemNum = ITM_AMMO_CRYSTAL_FORGE;
	else if (!Q_stricmp(tokenStr,"ITM_AMMO_CRYSTAL_SCAVENGER"))
		itemNum = ITM_AMMO_CRYSTAL_SCAVENGER;
	else if (!Q_stricmp(tokenStr,"ITM_AMMO_CRYSTAL_STASIS"))
		itemNum = ITM_AMMO_CRYSTAL_STASIS;
	else if (!Q_stricmp(tokenStr,"ITM_BLUE_HYPO"))
		itemNum = ITM_BLUE_HYPO;
	else if (!Q_stricmp(tokenStr,"ITM_RED_HYPO"))
		itemNum = ITM_RED_HYPO;
	else if (!Q_stricmp(tokenStr,"ITM_VOYAGER_HYPO"))
		itemNum = ITM_VOYAGER_HYPO;
	else if (!Q_stricmp(tokenStr,"ITM_KLINGON_BLADE"))
		itemNum = ITM_KLINGON_BLADE;
	else if (!Q_stricmp(tokenStr,"ITM_IMPERIAL_BLADE"))
		itemNum = ITM_IMPERIAL_BLADE;
	else if (!Q_stricmp(tokenStr,"ITM_DESPERADO_WEAPON"))
		itemNum = ITM_DESPERADO_WEAPON;
	else if (!Q_stricmp(tokenStr,"ITM_PALADIN_WEAPON"))
		itemNum = ITM_PALADIN_WEAPON;
	else if (!Q_stricmp(tokenStr,"ITM_PROTON_GUN_PICKUP"))	
		itemNum = ITM_PROTON_GUN_PICKUP;
	else
	{
		itemNum = 0;
		gi.Printf("WARNING: bad itemname in external item data '%s'\n", tokenStr);
	}

	itemParms.itemNum = itemNum;	
//	++bg_numItems;

	IT_SetDefaults();
}

static void IT_PickupName(char **holdBuf)
{
	int len;
	char	*tokenStr;

	if (G_ParseString(holdBuf,&tokenStr)) 
	{
		return;
	}

	len = strlen(tokenStr);
	len++;
	if (len > 32)
	{
		len = 32;
		gi.Printf("WARNING: weaponclass too long in external ITEMS.DAT '%s'\n", tokenStr);
	}

	bg_itemlist[itemParms.itemNum].pickup_name = G_NewString(tokenStr);
}

static void IT_ClassName(char **holdBuf)
{
	int len;
	char	*tokenStr;

	if (G_ParseString(holdBuf,&tokenStr)) 
	{
		return;
	}

	len = strlen(tokenStr);
	len++;
	if (len > 32)
	{
		len = 32;
		gi.Printf("WARNING: weaponclass too long in external ITEMS.DAT '%s'\n", tokenStr);
	}

	bg_itemlist[itemParms.itemNum].classname = G_NewString(tokenStr);

//	Q_strncpyz(bg_itemlist[itemParms.itemNum].classname,tokenStr,len);

}

static void IT_WorldModel(char **holdBuf)
{
	int len;
	char	*tokenStr;

	if (G_ParseString(holdBuf,&tokenStr)) 
	{
		return;
	}

	len = strlen(tokenStr);
	len++;
	if (len > 64)
	{
		len = 64;
		gi.Printf("WARNING: world model too long in external ITEMS.DAT '%s'\n", tokenStr);
	}

	bg_itemlist[itemParms.itemNum].world_model = G_NewString(tokenStr);

//	Q_strncpyz(bg_itemlist[itemParms.itemNum].world_model[0],tokenStr,len);

}

static void IT_Tag(char **holdBuf)
{
	int tag;
	char	*tokenStr;

	if (G_ParseString(holdBuf,&tokenStr)) 
	{
		return;
	}

	if (!Q_stricmp(tokenStr,"WP_NONE"))	
		tag = WP_NONE;
	else if (!Q_stricmp(tokenStr,"WP_BORG_ASSIMILATOR"))	
		tag = WP_BORG_ASSIMILATOR;
	else if (!Q_stricmp(tokenStr,"WP_BORG_DRILL"))	
		tag = WP_BORG_DRILL;
	else if (!Q_stricmp(tokenStr,"WP_BORG_TASER"))	
		tag = WP_BORG_TASER;
	else if (!Q_stricmp(tokenStr,"WP_BORG_WEAPON"))	
		tag = WP_BORG_WEAPON;
	else if (!Q_stricmp(tokenStr,"WP_PHASER"))	
		tag = WP_PHASER;
	else if (!Q_stricmp(tokenStr,"WP_COMPRESSION_RIFLE"))	
		tag = WP_COMPRESSION_RIFLE;
	else if (!Q_stricmp(tokenStr,"WP_IMOD"))	
		tag = WP_IMOD;
	else if (!Q_stricmp(tokenStr,"WP_SCAVENGER_RIFLE"))	
		tag = WP_SCAVENGER_RIFLE;
	else if (!Q_stricmp(tokenStr,"WP_STASIS"))	
		tag = WP_STASIS;
	else if (!Q_stricmp(tokenStr,"WP_GRENADE_LAUNCHER"))	
		tag = WP_GRENADE_LAUNCHER;
	else if (!Q_stricmp(tokenStr,"WP_TETRION_DISRUPTOR"))	
		tag = WP_TETRION_DISRUPTOR;
	else if (!Q_stricmp(tokenStr,"WP_DREADNOUGHT"))	
		tag = WP_DREADNOUGHT;
	else if (!Q_stricmp(tokenStr,"WP_QUANTUM_BURST"))	
		tag = WP_QUANTUM_BURST;
	else if (!Q_stricmp(tokenStr,"WP_TRICORDER"))	
		tag = WP_TRICORDER;
	else if (!Q_stricmp(tokenStr,"ISODESIUM"))	
		tag = AMMO_ALIEN;
	else if (!Q_stricmp(tokenStr,"WP_BOT_WELDER"))	
		tag = WP_BOT_WELDER;
	else if (!Q_stricmp(tokenStr,"WP_CHAOTICA_GUARD_GUN"))	
		tag = WP_CHAOTICA_GUARD_GUN;
	else if (!Q_stricmp(tokenStr,"WP_BOT_ROCKET"))	
		tag = WP_BOT_ROCKET;
	else if (!Q_stricmp(tokenStr,"WP_FORGE_PROJ"))	
		tag = WP_FORGE_PROJ;
	else if (!Q_stricmp(tokenStr,"WP_FORGE_PSYCH"))	
		tag = WP_FORGE_PSYCH;
	else if (!Q_stricmp(tokenStr,"WP_PARASITE"))	
		tag = WP_PARASITE;
	else if (!Q_stricmp(tokenStr,"WP_MELEE"))	
		tag = WP_MELEE;
	else if (!Q_stricmp(tokenStr,"WP_STASIS_ATTACK"))	
		tag = WP_STASIS_ATTACK;
	else if (!Q_stricmp(tokenStr,"WP_DN_TURRET"))
		tag = WP_DN_TURRET;
	else if (!Q_stricmp(tokenStr,"WP_BLUE_HYPO"))
		tag = WP_BLUE_HYPO;
	else if (!Q_stricmp(tokenStr,"WP_RED_HYPO"))
		tag = WP_RED_HYPO;
	else if (!Q_stricmp(tokenStr,"WP_VOYAGER_HYPO"))
		tag = WP_VOYAGER_HYPO;
	else if (!Q_stricmp(tokenStr,"WP_KLINGON_BLADE"))
		tag = WP_KLINGON_BLADE;
	else if (!Q_stricmp(tokenStr,"WP_IMPERIAL_BLADE"))
		tag = WP_IMPERIAL_BLADE;
	else if (!Q_stricmp(tokenStr,"WP_PALADIN"))
		tag = WP_PALADIN;
	else if (!Q_stricmp(tokenStr,"WP_DESPERADO"))
		tag = WP_DESPERADO;
	else if (!Q_stricmp(tokenStr,"WP_PROTON_GUN"))	
		tag = WP_PROTON_GUN;
	else if (!Q_stricmp(tokenStr,"STARFLEET_HELMET"))	
		tag = AMMO_ALIEN;
	else if (!Q_stricmp(tokenStr,"AMMO_ALIEN"))	
		tag = AMMO_ALIEN;
	else
	{
		tag = WP_IMOD;
		gi.Printf("WARNING: bad tagname in external item data '%s'\n", tokenStr);
	}

	bg_itemlist[itemParms.itemNum].giTag = tag;

}

static void IT_Type(char **holdBuf)
{
	int type;
	char	*tokenStr;

	if (G_ParseString(holdBuf,&tokenStr)) 
	{
		return;
	}

	if (!Q_stricmp(tokenStr,"IT_BAD"))	
		type = IT_BAD;
	else if (!Q_stricmp(tokenStr,"IT_WEAPON"))	
		type = IT_WEAPON;
	else if (!Q_stricmp(tokenStr,"IT_AMMO"))	
		type = IT_AMMO;
	else if (!Q_stricmp(tokenStr,"IT_ARMOR"))
		type = IT_ARMOR;
	else if (!Q_stricmp(tokenStr,"IT_HEALTH"))
		type = IT_HEALTH;
	else if (!Q_stricmp(tokenStr,"IT_HOLDABLE"))	
		type = IT_HOLDABLE;
	else
	{
		type = IT_BAD;
		gi.Printf("WARNING: bad itemname in external item data '%s'\n", tokenStr);
	}

	bg_itemlist[itemParms.itemNum].giType = (itemType_t) type;

}

static void IT_Icon(char **holdBuf)
{
	int len;
	char	*tokenStr;

	if (G_ParseString(holdBuf,&tokenStr)) 
	{
		return;
	}

	len = strlen(tokenStr);
	len++;
	if (len > 32)
	{
		len = 32;
		gi.Printf("WARNING: icon too long in external ITEMS.DAT '%s'\n", tokenStr);
	}

	bg_itemlist[itemParms.itemNum].icon = G_NewString(tokenStr);
}

static void IT_Count(char **holdBuf)
{
	int		tokenInt;

	if ( G_ParseInt(holdBuf,&tokenInt)) 
	{
		SkipRestOfLine(holdBuf);
		return;
	}

	if ((tokenInt < 0) || (tokenInt > 1000 )) // FIXME :What are the right values?
	{
		gi.Printf("WARNING: bad Count in external item data '%d'\n", tokenInt);
		return;
	}
	bg_itemlist[itemParms.itemNum].quantity = tokenInt;

}


static void IT_Min(char **holdBuf)
{
	int		tokenInt;
	int		i;

	for (i=0;i<3;++i)
	{
		if ( G_ParseInt(holdBuf,&tokenInt)) 
		{
			SkipRestOfLine(holdBuf);
			return;
		}

		bg_itemlist[itemParms.itemNum].mins[i] = tokenInt;
	}

}

static void IT_Max(char **holdBuf)
{
	int		tokenInt;
	int		i;

	for (i=0;i<3;++i)
	{
		if ( G_ParseInt(holdBuf,&tokenInt)) 
		{
			SkipRestOfLine(holdBuf);
			return;
		}

		bg_itemlist[itemParms.itemNum].maxs[i] = tokenInt;
	}

}

static void IT_PickupSound(char **holdBuf)
{
	int len;
	char	*tokenStr;

	if (G_ParseString(holdBuf,&tokenStr)) 
	{
		return;
	}

	len = strlen(tokenStr);
	len++;
	if (len > 32)
	{
		len = 32;
		gi.Printf("WARNING: Pickup Sound too long in external ITEMS.DAT '%s'\n", tokenStr);
	}

	bg_itemlist[itemParms.itemNum].pickup_sound = G_NewString(tokenStr);
}

static void IT_ParseWeaponParms(char **holdBuf)
{
	static int	weaponNum,ammoNum;
	char	*token;
	int		i;


	while (holdBuf)
	{
		token = COM_ParseExt( holdBuf, qtrue );

		if (!Q_stricmp( token, "}" ))	// End of data for this weapon
			break;

		// Loop through possible parameters
		for (i=0;i<IT_PARM_MAX;++i)
		{
			if (!Q_stricmp(token,ItemParms[i].parmName))	
			{
				ItemParms[i].func(holdBuf);
				break;
			}
		}

		if (i < IT_PARM_MAX)	// Find parameter???
		{
			continue;
		}

		gi.Printf("bad parameter in external weapon data '%s'\n", token);
		SkipRestOfLine(holdBuf);
		
	}
}


static void IT_ParseParms(char *buffer)
{
	char	*holdBuf;
	char	*token;

//	bg_numItems = 0;
	holdBuf = buffer;
	COM_BeginParseSession();

	while ( holdBuf ) 
	{
		token = COM_ParseExt( &holdBuf, qtrue );

		if ( !Q_stricmp( token, "{" ) ) 
		{
			token =token;
			IT_ParseWeaponParms(&holdBuf);
		}
		 
	}

//	--bg_numItems;

}


void IT_LoadItemParms (void)
{
	char *buffer;
	int len;
	char	finalName[MAX_QPATH];

	G_LanguageFilename("ext_data/items","dat",(char *) &finalName);

	len = gi.FS_ReadFile(finalName,(void **) &buffer);

	IT_ParseParms(buffer);

	gi.FS_FreeFile( buffer );	//let go of the buffer
}

