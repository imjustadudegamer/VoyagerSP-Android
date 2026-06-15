typedef enum
{
	SP_NONE,
	//Senior Officers
	SP_JANEWAY,
	SP_CHAKOTAY,	
	SP_TUVOK,
	SP_TORRES,
	SP_PARIS,
	SP_NEELIX,
	SP_SEVEN,
	SP_DOCTOR,
	SP_KIM,
	//HazTeam alpha
	SP_FOSTER,
	SP_MUNRO,
	SP_BIESSMAN,
	SP_CHANG,
	SP_CHELL,
	SP_JUROT,
	SP_TELSIA,
	//HazTeam beta
	SP_HOEKSTRA,
	SP_CSATLOS,
	SP_NELSON,
	SP_ODELL,
	SP_OVIEDO,
	SP_JAWORSKI,
	SP_LAIRD,
	//Misc crew
	SP_LANG,
	SP_RENNER,
	SP_PELLETIER,
	SP_GREEN,
	SP_SALMA,
	//Baddies
	SP_HIROGEN_BOSS,
	SP_KREEGE,
	//Other
	SP_COMPUTER,
	SP_MAX
} speakers_t;


typedef struct speakerTable_s
{
	char	*stringID;
	char	*headModelFile;
	qhandle_t		headModel;
	char	*skinName;
	qhandle_t	headSkin;
	qboolean	extensions;
} speakerTable_t;

extern speakerTable_t speakerTable [SP_MAX];
