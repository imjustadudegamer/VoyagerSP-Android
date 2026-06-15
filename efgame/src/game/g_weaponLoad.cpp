// g_weaponLoad.cpp 
// fills in memory struct with ext_dat\weapons.dat

#ifdef _USRDLL	//UI dll

#include "../ui/gameinfo.h"
#include "weapons.h"
extern	gameinfo_import_t	gi;
extern weaponData_t weaponData[];
extern ammoData_t ammoData[];

#else	//we are in the game

// ONLY DO THIS ON THE GAME SIDE
#include "g_local.h"

typedef struct {
	char	*name;
	void	(*func)(centity_t *cent, const struct weaponInfo_s *weapon );
} func_t;

void FX_BorgProjectileThink( centity_t *cent, const struct weaponInfo_s *weapon );
void FX_ScavengerProjectileThink( centity_t *cent, const struct weaponInfo_s *weapon );
void FX_ScavengerAltFireThink( centity_t *cent, const struct weaponInfo_s *weapon );
void FX_StasisProjectileThink( centity_t *cent, const struct weaponInfo_s *weapon );
void FX_GrenadeThink( centity_t *cent, const struct weaponInfo_s *weapon );
void FX_GrenadeAltThink( centity_t *cent, const struct weaponInfo_s *weapon );
void FX_TetrionProjectileThink( centity_t *cent, const struct weaponInfo_s *weapon );
void FX_DreadnoughtProjectileThink( centity_t *cent, const struct weaponInfo_s *weapon );
void FX_QuantumThink( centity_t *cent, const struct weaponInfo_s *weapon );
void FX_QuantumAltThink( centity_t *cent, const struct weaponInfo_s *weapon );
void FX_BotRocketThink( centity_t *cent, const struct weaponInfo_s *weapon );
void FX_ForgeProjectileThink( centity_t *cent, const struct weaponInfo_s *weapon );
void FX_ForgePsychThink( centity_t *cent, const struct weaponInfo_s *weapon );
void FX_ParasiteAcidThink( centity_t *cent, const struct weaponInfo_s *weapon );
void FX_StasisAttackThink( centity_t *cent, const struct weaponInfo_s *weapon );
void FX_DreadnoughtTurretThink( centity_t *cent, const struct weaponInfo_s *weapon );
void FX_PaladinProjThink( centity_t *cent, const struct weaponInfo_s *weapon );
void FX_BotProjFunc( centity_t *cent, const struct weaponInfo_s *weapon );

func_t	funcs[] = {
	{"borgfunc", FX_BorgProjectileThink},
	{"scavengerfunc", FX_ScavengerProjectileThink},
	{"altscavengerfunc", FX_ScavengerAltFireThink},
	{"stasisfunc", FX_StasisProjectileThink},
	{"grenadefunc", FX_GrenadeThink},
	{"altgrenadefunc", FX_GrenadeAltThink},
	{"tetrionfunc", FX_TetrionProjectileThink},
	{"dreadnoughtfunc", FX_DreadnoughtProjectileThink},
	{"quantumfunc", FX_QuantumThink},
	{"quantumaltfunc", FX_QuantumAltThink},
	{"botrocketfunc", FX_BotRocketThink},
	{"forgeprojfunc", FX_ForgeProjectileThink},
	{"forgepsychfunc", FX_ForgePsychThink},
	{"parasiteacidfunc", FX_ParasiteAcidThink},
	{"stasisattackfunc", FX_StasisAttackThink},
	{"dnturretfunc", FX_DreadnoughtTurretThink},
	{"paladinfunc", FX_PaladinProjThink},
	{"botprojfunc", FX_BotProjFunc},
	{NULL, NULL}
};
#endif

qboolean G_ParseInt( char **data, int *i );
qboolean G_ParseString( char **data, char **s ); 
qboolean G_ParseFloat( char **data, float *f );

struct 
{
	int	weaponNum;	// Current weapon number
	int	ammoNum;
} wpnParms;

void WPN_Ammo (char **holdBuf);
void WPN_AmmoIcon (char **holdBuf);
void WPN_AmmoMax (char **holdBuf);
void WPN_AmmoLowCnt (char **holdBuf);
void WPN_AmmoType (char **holdBuf);
void WPN_EnergyPerShot (char **holdBuf);
void WPN_FireTime (char **holdBuf);
void WPN_FiringSnd (char **holdBuf);
void WPN_AltFiringSnd( char **holdBuf );
void WPN_StopSnd( char **holdBuf );
void WPN_FlashColor (char **holdBuf);
void WPN_FlashSnd (char **holdBuf);
void WPN_AltFlashSnd (char **holdBuf);
void WPN_Range (char **holdBuf);
void WPN_WeaponClass (char **holdBuf);
void WPN_WeaponIcon (char **holdBuf);
void WPN_WeaponModel (char **holdBuf);
void WPN_WeaponType (char **holdBuf);
void WPN_AltEnergyPerShot (char **holdBuf);
void WPN_AltFireTime (char **holdBuf);
void WPN_AltRange (char **holdBuf);
void WPN_BarrelCount(char **holdBuf);
void WPN_MissileName(char **holdBuf);
void WPN_AltMissileName(char **holdBuf);
void WPN_MissileSound(char **holdBuf);
void WPN_AltMissileSound(char **holdBuf);
void WPN_MissileLight(char **holdBuf);
void WPN_AltMissileLight(char **holdBuf);
void WPN_MissileLightColor(char **holdBuf);
void WPN_AltMissileLightColor(char **holdBuf);
void WPN_FuncName(char **holdBuf);
void WPN_AltFuncName(char **holdBuf);
void WPN_MissileHitSound(char **holdBuf);
void WPN_AltMissileHitSound(char **holdBuf);

typedef struct 
{
	char	*parmName;
	void	(*func)(char **holdBuf);
} wpnParms_t;

wpnParms_t WpnParms[] = 
{
	"ammo",				WPN_Ammo,	//ammo
	"ammoicon",			WPN_AmmoIcon,
	"ammomax",			WPN_AmmoMax,
	"ammolowcount",		WPN_AmmoLowCnt, //weapons
	"ammotype",			WPN_AmmoType,
	"energypershot",	WPN_EnergyPerShot,
	"fireTime",			WPN_FireTime,
	"firingsound",		WPN_FiringSnd,
	"altfiringsound",	WPN_AltFiringSnd,
	"flashsound",		WPN_FlashSnd,
	"altflashsound",	WPN_AltFlashSnd,
	"stopsound",		WPN_StopSnd,
	"flashcolor",		WPN_FlashColor,
	"range",			WPN_Range,
	"weaponclass",		WPN_WeaponClass,
	"weaponicon",		WPN_WeaponIcon,
	"weaponmodel",		WPN_WeaponModel,
	"weapontype",		WPN_WeaponType,
	"altenergypershot",	WPN_AltEnergyPerShot,
	"altfireTime",		WPN_AltFireTime,
	"altrange",			WPN_AltRange,
	"barrelcount",		WPN_BarrelCount,
	"missileModel",		WPN_MissileName,
	"altmissileModel", 	WPN_AltMissileName,
	"missileSound",		WPN_MissileSound,
	"altmissileSound", 	WPN_AltMissileSound,
	"missileLight",		WPN_MissileLight,
	"altmissileLight", 	WPN_AltMissileLight,
	"missileLightColor",WPN_MissileLightColor,
	"altmissileLightColor",	WPN_AltMissileLightColor,
	"missileFuncName",		WPN_FuncName,
	"altmissileFuncName",	WPN_AltFuncName,
	"missileHitSound",		WPN_MissileHitSound,
	"altmissileHitSound",	WPN_AltMissileHitSound,
};

const int WPN_PARM_MAX =  sizeof(WpnParms) / sizeof(WpnParms[0]);

void WPN_WeaponType(char **holdBuf)
{
	int weaponNum;
	char	*tokenStr;

	if (G_ParseString(holdBuf,&tokenStr)) 
	{
		return;
	}

	// FIXME : put this in an array (maybe a weaponDataInternal array???)
	if (!Q_stricmp(tokenStr,"WP_NONE"))	
		weaponNum = WP_NONE;
	else if (!Q_stricmp(tokenStr,"WP_PHASER"))	
		weaponNum = WP_PHASER;
	else if (!Q_stricmp(tokenStr,"WP_COMPRESSION_RIFLE"))	
		weaponNum = WP_COMPRESSION_RIFLE;
	else if (!Q_stricmp(tokenStr,"WP_IMOD"))	
		weaponNum = WP_IMOD;
	else if (!Q_stricmp(tokenStr,"WP_SCAVENGER_RIFLE"))	
		weaponNum = WP_SCAVENGER_RIFLE;
	else if (!Q_stricmp(tokenStr,"WP_STASIS"))	
		weaponNum = WP_STASIS;
	else if (!Q_stricmp(tokenStr,"WP_GRENADE_LAUNCHER"))	
		weaponNum = WP_GRENADE_LAUNCHER;
	else if (!Q_stricmp(tokenStr,"WP_TETRION_DISRUPTOR"))	
		weaponNum = WP_TETRION_DISRUPTOR;
	else if (!Q_stricmp(tokenStr,"WP_DREADNOUGHT"))	
		weaponNum = WP_DREADNOUGHT;
	else if (!Q_stricmp(tokenStr,"WP_QUANTUM_BURST"))	
		weaponNum = WP_QUANTUM_BURST;
	else if (!Q_stricmp(tokenStr,"WP_BORG_WEAPON"))	
		weaponNum = WP_BORG_WEAPON;
	else if (!Q_stricmp(tokenStr,"WP_BORG_TASER"))	
		weaponNum = WP_BORG_TASER;
	else if (!Q_stricmp(tokenStr,"WP_BORG_ASSIMILATOR"))	
		weaponNum = WP_BORG_ASSIMILATOR;
	else if (!Q_stricmp(tokenStr,"WP_BORG_DRILL"))	
		weaponNum = WP_BORG_DRILL;
	else if (!Q_stricmp(tokenStr,"WP_TRICORDER"))	
		weaponNum = WP_TRICORDER;
	else if (!Q_stricmp(tokenStr,"WP_BOT_WELDER"))	
		weaponNum = WP_BOT_WELDER;
	else if (!Q_stricmp(tokenStr,"WP_CHAOTICA_GUARD_GUN"))	
		weaponNum = WP_CHAOTICA_GUARD_GUN;
	else if (!Q_stricmp(tokenStr,"WP_BOT_ROCKET"))	
		weaponNum = WP_BOT_ROCKET;
	else if (!Q_stricmp(tokenStr,"WP_FORGE_PROJ"))	
		weaponNum = WP_FORGE_PROJ;
	else if (!Q_stricmp(tokenStr,"WP_FORGE_PSYCH"))	
		weaponNum = WP_FORGE_PSYCH;
	else if (!Q_stricmp(tokenStr,"WP_PARASITE"))	
		weaponNum = WP_PARASITE;
	else if (!Q_stricmp(tokenStr,"WP_MELEE"))	
		weaponNum = WP_MELEE;
	else if (!Q_stricmp(tokenStr,"WP_STASIS_ATTACK"))	
		weaponNum = WP_STASIS_ATTACK;
	else if (!Q_stricmp(tokenStr,"WP_DN_TURRET"))
		weaponNum = WP_DN_TURRET;
	else if (!Q_stricmp(tokenStr,"WP_BLUE_HYPO"))
		weaponNum = WP_BLUE_HYPO;
	else if (!Q_stricmp(tokenStr,"WP_RED_HYPO"))
		weaponNum = WP_RED_HYPO;
	else if (!Q_stricmp(tokenStr,"WP_VOYAGER_HYPO"))
		weaponNum = WP_VOYAGER_HYPO;
	else if (!Q_stricmp(tokenStr,"WP_KLINGON_BLADE"))
		weaponNum = WP_KLINGON_BLADE;
	else if (!Q_stricmp(tokenStr,"WP_IMPERIAL_BLADE"))
		weaponNum = WP_IMPERIAL_BLADE;
	else if (!Q_stricmp(tokenStr,"WP_PALADIN"))
		weaponNum = WP_PALADIN;
	else if (!Q_stricmp(tokenStr,"WP_DESPERADO"))
		weaponNum = WP_DESPERADO;
	else if (!Q_stricmp(tokenStr,"WP_PROTON_GUN"))	
		weaponNum = WP_PROTON_GUN;
	else
	{
		weaponNum = 0;
		gi.Printf(S_COLOR_YELLOW"WARNING: bad weapontype in external weapon data '%s'\n", tokenStr);
	}

	wpnParms.weaponNum = weaponNum;
}

//--------------------------------------------
void WPN_WeaponClass(char **holdBuf)
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
		gi.Printf(S_COLOR_YELLOW"WARNING: weaponclass too long in external WEAPONS.DAT '%s'\n", tokenStr);
	}

	Q_strncpyz(weaponData[wpnParms.weaponNum].classname,tokenStr,len);
}


//--------------------------------------------
void WPN_WeaponModel(char **holdBuf)
{
	int len;
	char	*tokenStr;

	if ( G_ParseString(holdBuf,&tokenStr)) 
	{
		return;
	}

	len = strlen(tokenStr);
	len++;
	if (len > 64)
	{
		len = 64;
		gi.Printf(S_COLOR_YELLOW"WARNING: weaponMdl too long in external WEAPONS.DAT '%s'\n", tokenStr);
	}

	Q_strncpyz(weaponData[wpnParms.weaponNum].weaponMdl,tokenStr,len);
}

//--------------------------------------------
void WPN_WeaponIcon(char **holdBuf)
{
	int len;
	char	*tokenStr;

	if ( G_ParseString(holdBuf,&tokenStr)) 
	{
		return;
	}

	len = strlen(tokenStr);
	len++;
	if (len > 64)
	{
		len = 64;
		gi.Printf(S_COLOR_YELLOW"WARNING: weaponIcon too long in external WEAPONS.DAT '%s'\n", tokenStr);
	}

	Q_strncpyz(weaponData[wpnParms.weaponNum].weaponIcon,tokenStr,len);
}

//--------------------------------------------
void WPN_AmmoType(char **holdBuf)
{
	int		tokenInt;

	if ( G_ParseInt(holdBuf,&tokenInt)) 
	{
		SkipRestOfLine(holdBuf);
		return;
	}

	if ((tokenInt < 0) || (tokenInt > 3 ))  // FIXME :What are the right values?
	{
		gi.Printf(S_COLOR_YELLOW"WARNING: bad Ammotype in external weapon data '%d'\n", tokenInt);
		return;
	}

	weaponData[wpnParms.weaponNum].ammoIndex = tokenInt;
}

//--------------------------------------------
void WPN_AmmoLowCnt(char **holdBuf)
{
	int		tokenInt;

	if ( G_ParseInt(holdBuf,&tokenInt)) 
	{
		SkipRestOfLine(holdBuf);
		return;
	}

	if ((tokenInt < 0) || (tokenInt > 100 )) // FIXME :What are the right values?
	{
		gi.Printf(S_COLOR_YELLOW"WARNING: bad Ammolowcount in external weapon data '%d'\n", tokenInt);
		return;
	}

	weaponData[wpnParms.weaponNum].ammoLow = tokenInt;
}

//--------------------------------------------
void WPN_FlashColor(char **holdBuf)
{
	int i;
	float	tokenFlt;

	for (i=0;i<3;++i)
	{
		if ( G_ParseFloat(holdBuf,&tokenFlt)) 
		{
			SkipRestOfLine(holdBuf);
			continue;
		}

		if ((tokenFlt < 0) || (tokenFlt > 1 ))
		{
			gi.Printf(S_COLOR_YELLOW"WARNING: bad flashcolor in external weapon data '%f'\n", tokenFlt);
			continue;
		}
		weaponData[wpnParms.weaponNum].flashColor[i] = tokenFlt;
	}
}

//--------------------------------------------
void WPN_FiringSnd(char **holdBuf)
{
	char	*tokenStr;
	int		len;

	if ( G_ParseString(holdBuf,&tokenStr)) 
	{
		return;
	}

	len = strlen(tokenStr);
	len++;
	if (len > 64)
	{
		len = 64;
		gi.Printf(S_COLOR_YELLOW"WARNING: firingSnd too long in external WEAPONS.DAT '%s'\n", tokenStr);
	}

	Q_strncpyz(weaponData[wpnParms.weaponNum].firingSnd,tokenStr,len);
}

//--------------------------------------------
void WPN_AltFiringSnd( char **holdBuf )
{
	char	*tokenStr;
	int		len;

	if ( G_ParseString(holdBuf,&tokenStr)) 
	{
		return;
	}

	len = strlen(tokenStr);
	len++;
	if (len > 64)
	{
		len = 64;
		gi.Printf(S_COLOR_YELLOW"WARNING: altFiringSnd too long in external WEAPONS.DAT '%s'\n", tokenStr);
	}

	Q_strncpyz(weaponData[wpnParms.weaponNum].altFiringSnd,tokenStr,len);
}

//--------------------------------------------
void WPN_StopSnd( char **holdBuf )
{
	char	*tokenStr;
	int		len;

	if ( G_ParseString(holdBuf,&tokenStr)) 
	{
		return;
	}

	len = strlen(tokenStr);
	len++;
	if (len > 64)
	{
		len = 64;
		gi.Printf(S_COLOR_YELLOW"WARNING: stopSnd too long in external WEAPONS.DAT '%s'\n", tokenStr);
	}

	Q_strncpyz(weaponData[wpnParms.weaponNum].stopSnd,tokenStr,len);
}

//--------------------------------------------
void WPN_FlashSnd(char **holdBuf)
{
	char	*tokenStr;
	int		len;

	if ( G_ParseString(holdBuf,&tokenStr)) 
	{
		return;
	}

	len = strlen(tokenStr);
	len++;
	if (len > 64)
	{
		len = 64;
		gi.Printf(S_COLOR_YELLOW"WARNING: flashSnd too long in external WEAPONS.DAT '%s'\n", tokenStr);
	}

	Q_strncpyz(weaponData[wpnParms.weaponNum].flashSnd,tokenStr,len);
}

//--------------------------------------------
void WPN_AltFlashSnd(char **holdBuf)
{
	char	*tokenStr;
	int		len;

	if ( G_ParseString(holdBuf,&tokenStr)) 
	{
		return;
	}

	len = strlen(tokenStr);
	len++;
	if (len > 64)
	{
		len = 64;
		gi.Printf(S_COLOR_YELLOW"WARNING: altFlashSnd too long in external WEAPONS.DAT '%s'\n", tokenStr);
	}

	Q_strncpyz(weaponData[wpnParms.weaponNum].altFlashSnd,tokenStr,len);
}

//--------------------------------------------
void WPN_FireTime(char **holdBuf)
{
	int		tokenInt;

	if ( G_ParseInt(holdBuf,&tokenInt)) 
	{
		SkipRestOfLine(holdBuf);
		return;
	}

	if ((tokenInt < 0) || (tokenInt > 10000 )) // FIXME :What are the right values?
	{
		gi.Printf(S_COLOR_YELLOW"WARNING: bad Firetime in external weapon data '%d'\n", tokenInt);
		return;
	}
	weaponData[wpnParms.weaponNum].fireTime = tokenInt;
}

//--------------------------------------------
void WPN_Range(char **holdBuf)
{
	int		tokenInt;

	if ( G_ParseInt(holdBuf,&tokenInt)) 
	{
		SkipRestOfLine(holdBuf);
		return;
	}

	if ((tokenInt < 0) || (tokenInt > 10000 )) // FIXME :What are the right values?
	{
		gi.Printf(S_COLOR_YELLOW"WARNING: bad Range in external weapon data '%d'\n", tokenInt);
		return;
	}

	weaponData[wpnParms.weaponNum].range = tokenInt;
}

//--------------------------------------------
void WPN_EnergyPerShot(char **holdBuf)
{
	int		tokenInt;

	if ( G_ParseInt(holdBuf,&tokenInt)) 
	{
		SkipRestOfLine(holdBuf);
		return;
	}

	if ((tokenInt < 0) || (tokenInt > 1000 )) // FIXME :What are the right values?
	{
		gi.Printf(S_COLOR_YELLOW"WARNING: bad EnergyPerShot in external weapon data '%d'\n", tokenInt);
		return;
	}
	weaponData[wpnParms.weaponNum].energyPerShot = tokenInt;
}

//--------------------------------------------
void WPN_AltFireTime(char **holdBuf)
{
	int		tokenInt;

	if ( G_ParseInt(holdBuf,&tokenInt)) 
	{
		SkipRestOfLine(holdBuf);
		return;
	}

	if ((tokenInt < 0) || (tokenInt > 10000 )) // FIXME :What are the right values?
	{
		gi.Printf(S_COLOR_YELLOW"WARNING: bad altFireTime in external weapon data '%d'\n", tokenInt);
		return;
	}
	weaponData[wpnParms.weaponNum].altFireTime = tokenInt;
}

//--------------------------------------------
void WPN_AltRange(char **holdBuf)
{
	int		tokenInt;

	if ( G_ParseInt(holdBuf,&tokenInt)) 
	{
		SkipRestOfLine(holdBuf);
		return;
	}

	if ((tokenInt < 0) || (tokenInt > 10000 )) // FIXME :What are the right values?
	{
		gi.Printf(S_COLOR_YELLOW"WARNING: bad AltRange in external weapon data '%d'\n", tokenInt);
		return;
	}

	weaponData[wpnParms.weaponNum].altRange = tokenInt;
}

//--------------------------------------------
void WPN_AltEnergyPerShot(char **holdBuf)
{
	int		tokenInt;

	if ( G_ParseInt(holdBuf,&tokenInt)) 
	{
		SkipRestOfLine(holdBuf);
		return;
	}

	if ((tokenInt < 0) || (tokenInt > 1000 )) // FIXME :What are the right values?
	{
		gi.Printf(S_COLOR_YELLOW"WARNING: bad AltEnergyPerShot in external weapon data '%d'\n", tokenInt);
		return;
	}
	weaponData[wpnParms.weaponNum].altEnergyPerShot = tokenInt;
}

//--------------------------------------------
void WPN_Ammo(char **holdBuf)
{
	char	*tokenStr;

	if ( G_ParseString(holdBuf,&tokenStr)) 
	{
		return;
	}

	if (!Q_stricmp(tokenStr,"AMMO_NONE"))	
		wpnParms.ammoNum = AMMO_NONE;
	else if (!Q_stricmp(tokenStr,"AMMO_STARFLEET"))	
		wpnParms.ammoNum = AMMO_STARFLEET;
	else if (!Q_stricmp(tokenStr,"AMMO_ALIEN"))	
		wpnParms.ammoNum = AMMO_ALIEN;
	else if (!Q_stricmp(tokenStr,"AMMO_PHASER"))	
		wpnParms.ammoNum = AMMO_PHASER;
	else
	{
		gi.Printf(S_COLOR_YELLOW"WARNING: bad ammotype in external weapon data '%s'\n", tokenStr);
		wpnParms.ammoNum = 0;
	}
}

//--------------------------------------------
void WPN_AmmoIcon(char **holdBuf)
{
	char	*tokenStr;
	int		len;

	if ( G_ParseString(holdBuf,&tokenStr)) 
	{
		return;
	}

	len = strlen(tokenStr);
	len++;
	if (len > 64)
	{
		len = 64;
		gi.Printf(S_COLOR_YELLOW"WARNING: ammoicon too long in external WEAPONS.DAT '%s'\n", tokenStr);
	}

	Q_strncpyz(ammoData[wpnParms.ammoNum].icon,tokenStr,len);

}

//--------------------------------------------
void WPN_AmmoMax(char **holdBuf)
{
	int		tokenInt;

	if ( G_ParseInt(holdBuf,&tokenInt)) 
	{
		SkipRestOfLine(holdBuf);
		return;
	}

	if ((tokenInt < 0) || (tokenInt > 1000 )) 
	{
		gi.Printf(S_COLOR_YELLOW"WARNING: bad Ammo Max in external weapon data '%d'\n", tokenInt);
		return;
	}
	ammoData[wpnParms.ammoNum].max = tokenInt;
}

//--------------------------------------------
void WPN_BarrelCount(char **holdBuf)
{
	int		tokenInt;

	if ( G_ParseInt(holdBuf,&tokenInt)) 
	{
		SkipRestOfLine(holdBuf);
		return;
	}

	if ((tokenInt < 0) || (tokenInt > 4 )) 
	{
		gi.Printf(S_COLOR_YELLOW"WARNING: bad Range in external weapon data '%d'\n", tokenInt);
		return;
	}

	weaponData[wpnParms.weaponNum].numBarrels = tokenInt;
}


//--------------------------------------------
static void WP_ParseWeaponParms(char **holdBuf)
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
		for (i=0;i<WPN_PARM_MAX;++i)
		{
			if (!Q_stricmp(token,WpnParms[i].parmName))	
			{
				WpnParms[i].func(holdBuf);
				break;
			}
		}

		if (i < WPN_PARM_MAX)	// Find parameter???
		{
			continue;
		}
		Com_Error(ERR_FATAL,"bad parameter in external weapon data '%s'\n", token);		
	}
}

//--------------------------------------------
void WPN_MissileName(char **holdBuf)
{
	int len;
	char	*tokenStr;

	if ( G_ParseString(holdBuf,&tokenStr)) 
	{
		return;
	}

	len = strlen(tokenStr);
	len++;
	if (len > 64)
	{
		len = 64;
		gi.Printf(S_COLOR_YELLOW"WARNING: MissileName too long in external WEAPONS.DAT '%s'\n", tokenStr);
	}

	Q_strncpyz(weaponData[wpnParms.weaponNum].missileMdl,tokenStr,len);

}

//--------------------------------------------
void WPN_AltMissileName(char **holdBuf)
{
	int len;
	char	*tokenStr;

	if ( G_ParseString(holdBuf,&tokenStr)) 
	{
		return;
	}

	len = strlen(tokenStr);
	len++;
	if (len > 64)
	{
		len = 64;
		gi.Printf(S_COLOR_YELLOW"WARNING: AltMissileName too long in external WEAPONS.DAT '%s'\n", tokenStr);
	}

	Q_strncpyz(weaponData[wpnParms.weaponNum].alt_missileMdl,tokenStr,len);

}


//--------------------------------------------
void WPN_MissileHitSound(char **holdBuf)
{
	int len;
	char	*tokenStr;

	if ( G_ParseString(holdBuf,&tokenStr)) 
	{
		return;
	}

	len = strlen(tokenStr);
	len++;
	if (len > 64)
	{
		len = 64;
		gi.Printf(S_COLOR_YELLOW"WARNING: MissileHitSound too long in external WEAPONS.DAT '%s'\n", tokenStr);
	}

	Q_strncpyz(weaponData[wpnParms.weaponNum].missileHitSound,tokenStr,len);
}

//--------------------------------------------
void WPN_AltMissileHitSound(char **holdBuf)
{
	int len;
	char	*tokenStr;

	if ( G_ParseString(holdBuf,&tokenStr)) 
	{
		return;
	}

	len = strlen(tokenStr);
	len++;
	if (len > 64)
	{
		len = 64;
		gi.Printf(S_COLOR_YELLOW"WARNING: AltMissileHitSound too long in external WEAPONS.DAT '%s'\n", tokenStr);
	}

	Q_strncpyz(weaponData[wpnParms.weaponNum].altmissileHitSound,tokenStr,len);
}

//--------------------------------------------
void WPN_MissileSound(char **holdBuf)
{
	int len;
	char	*tokenStr;

	if ( G_ParseString(holdBuf,&tokenStr)) 
	{
		return;
	}

	len = strlen(tokenStr);
	len++;
	if (len > 64)
	{
		len = 64;
		gi.Printf(S_COLOR_YELLOW"WARNING: MissileSound too long in external WEAPONS.DAT '%s'\n", tokenStr);
	}

	Q_strncpyz(weaponData[wpnParms.weaponNum].missileSound,tokenStr,len);

}


//--------------------------------------------
void WPN_AltMissileSound(char **holdBuf)
{
	int len;
	char	*tokenStr;

	if ( G_ParseString(holdBuf,&tokenStr)) 
	{
		return;
	}

	len = strlen(tokenStr);
	len++;
	if (len > 64)
	{
		len = 64;
		gi.Printf(S_COLOR_YELLOW"WARNING: AltMissileSound too long in external WEAPONS.DAT '%s'\n", tokenStr);
	}

	Q_strncpyz(weaponData[wpnParms.weaponNum].alt_missileSound,tokenStr,len);

}

//--------------------------------------------
void WPN_MissileLightColor(char **holdBuf)
{
	int i;
	float	tokenFlt;

	for (i=0;i<3;++i)
	{
		if ( G_ParseFloat(holdBuf,&tokenFlt)) 
		{
			SkipRestOfLine(holdBuf);
			continue;
		}

		if ((tokenFlt < 0) || (tokenFlt > 1 ))
		{
			gi.Printf(S_COLOR_YELLOW"WARNING: bad missilelightcolor in external weapon data '%f'\n", tokenFlt);
			continue;
		}
		weaponData[wpnParms.weaponNum].missileDlightColor[i] = tokenFlt;
	}

}

//--------------------------------------------
void WPN_AltMissileLightColor(char **holdBuf)
{
	int i;
	float	tokenFlt;

	for (i=0;i<3;++i)
	{
		if ( G_ParseFloat(holdBuf,&tokenFlt)) 
		{
			SkipRestOfLine(holdBuf);
			continue;
		}

		if ((tokenFlt < 0) || (tokenFlt > 1 ))
		{
			gi.Printf(S_COLOR_YELLOW"WARNING: bad altmissilelightcolor in external weapon data '%f'\n", tokenFlt);
			continue;
		}
		weaponData[wpnParms.weaponNum].alt_missileDlightColor[i] = tokenFlt;
	}

}


//--------------------------------------------
void WPN_MissileLight(char **holdBuf)
{
	float	tokenFlt;

	if ( G_ParseFloat(holdBuf,&tokenFlt)) 
	{
		SkipRestOfLine(holdBuf);
	}

	if ((tokenFlt < 0) || (tokenFlt > 255 )) // FIXME :What are the right values?
	{
		gi.Printf(S_COLOR_YELLOW"WARNING: bad missilelight in external weapon data '%f'\n", tokenFlt);
	}
	weaponData[wpnParms.weaponNum].missileDlight = tokenFlt;
}

//--------------------------------------------
void WPN_AltMissileLight(char **holdBuf)
{
	float	tokenFlt;

	if ( G_ParseFloat(holdBuf,&tokenFlt)) 
	{
		SkipRestOfLine(holdBuf);
	}

	if ((tokenFlt < 0) || (tokenFlt > 255 )) // FIXME :What are the right values?
	{
		gi.Printf(S_COLOR_YELLOW"WARNING: bad altmissilelight in external weapon data '%f'\n", tokenFlt);
	}
	weaponData[wpnParms.weaponNum].alt_missileDlight = tokenFlt;
}


//--------------------------------------------
void WPN_FuncName(char **holdBuf)
{
	char	*tokenStr;

	if ( G_ParseString(holdBuf,&tokenStr)) 
	{
		return;
	}
	// ONLY DO THIS ON THE GAME SIDE
#ifndef _USRDLL
	int len = strlen(tokenStr);

	len++;
	if (len > 64)
	{
		len = 64;
		gi.Printf(S_COLOR_YELLOW"WARNING: FuncName too long in external WEAPONS.DAT '%s'\n", tokenStr);
	}

	for ( func_t* s=funcs ; s->name ; s++ ) {
		if ( !Q_stricmp(s->name, tokenStr) ) {
			// found it
			weaponData[wpnParms.weaponNum].func = (void*)s->func;
			return;
		}
	}
	gi.Printf(S_COLOR_YELLOW"WARNING: FuncName %s in external WEAPONS.DAT '%s' does not exist\n", tokenStr);
#endif
}


//--------------------------------------------
void WPN_AltFuncName(char **holdBuf)
{
	char	*tokenStr;

	if ( G_ParseString(holdBuf,&tokenStr)) 
	{
		return;
	}

	// ONLY DO THIS ON THE GAME SIDE
#ifndef _USRDLL
	int len = strlen(tokenStr);
	len++;
	if (len > 64)
	{
		len = 64;
		gi.Printf(S_COLOR_YELLOW"WARNING: FuncName too long in external WEAPONS.DAT '%s'\n", tokenStr);
	}

	for ( func_t* s=funcs ; s->name ; s++ ) {
		if ( !Q_stricmp(s->name, tokenStr) ) {
			// found it
			weaponData[wpnParms.weaponNum].altfunc = (void*)s->func;
			return;
		}
	}
	gi.Printf(S_COLOR_YELLOW"WARNING: AltFuncName %s in external WEAPONS.DAT '%s' does not exist\n", tokenStr);

#endif
}


//--------------------------------------------
static void WP_ParseParms(char *buffer)
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
			WP_ParseWeaponParms(&holdBuf);
		}
		 
	}

}

//--------------------------------------------
void WP_LoadWeaponParms (void)
{
	char *buffer;
	int len;

	len = gi.FS_ReadFile("ext_data/weapons.dat",(void **) &buffer);

	// initialise the data area
	memset(weaponData, 0, WP_NUM_WEAPONS * sizeof(weaponData_t));	

	WP_ParseParms(buffer);

	gi.FS_FreeFile( buffer );	//let go of the buffer
}