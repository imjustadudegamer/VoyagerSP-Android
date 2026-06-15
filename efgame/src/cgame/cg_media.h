#ifndef __CG_MEDIA_H_
#define __CG_MEDIA_H_

#define	NUM_CROSSHAIRS		12

typedef enum {
	FOOTSTEP_NORMAL,
	FOOTSTEP_METAL,
	FOOTSTEP_SPLASH,
	FOOTSTEP_BORG,

	FOOTSTEP_TOTAL
} footstep_t;

// all of the model, shader, and sound references that are
// loaded at gamestate time are stored in cgMedia_t
// Other media that can be tied to clients, weapons, or items are
// stored in the clientInfo_t, itemInfo_t, weaponInfo_t, and powerupInfo_t
typedef struct {
	qhandle_t	charsetShader;
	qhandle_t	charsetPropTiny;
	qhandle_t	charsetProp;
	qhandle_t	charsetPropBig;
	qhandle_t	whiteShader;

	qhandle_t	loading1;
	qhandle_t	loading2;
	qhandle_t	loading3;
	qhandle_t	loading4;
	qhandle_t	loading5;
	qhandle_t	loading6;
	qhandle_t	loading7;
	qhandle_t	loading8;
	qhandle_t	loading9;
	qhandle_t	loadingcircle;
	qhandle_t	loadingquarter;
	qhandle_t	loadingcorner;
	qhandle_t	loadingtrim;

	qhandle_t	selectShader;
	qhandle_t	crosshairShader[NUM_CROSSHAIRS];
	qhandle_t	backTileShader;
	qhandle_t	noammoShader;
	qhandle_t	numberShaders[11];
	qhandle_t	smallnumberShaders[11];

	qhandle_t	status_corner_18_22;
	qhandle_t	status_corner_16_18;
	qhandle_t	status_corner_8_16_b;
	qhandle_t	status_corner_8_22;

//Chunks
	qhandle_t	chunkModels[1][6];
	sfxHandle_t	chunkSound;
	qhandle_t	borgChunkModels[1][3];
	sfxHandle_t	borgChunkSound;
	qhandle_t	stasisChunkModels[1][4];
	sfxHandle_t	stasisChunkSound;
	qhandle_t	glassChunkModels[1][6];
	sfxHandle_t	glassChunkSound;
	qhandle_t	warriorBotChunks[5];
	sfxHandle_t	warbotExplodeSounds[2];
	sfxHandle_t	scoutbotExplodeSounds[3];

	qhandle_t	borgLightningShaders[4];
	qhandle_t	BWLightningShaders[4];
	qhandle_t	phaserShader;
	qhandle_t	sparkShader;
	qhandle_t	spark2Shader;
	qhandle_t	spark3Shader;
	qhandle_t	steamShader;
	qhandle_t	smokeShader;
	qhandle_t	spooShader;
	qhandle_t	prifleImpactShader;
	qhandle_t	compressionAltBlastShader;
	qhandle_t	explosionModel;

	qhandle_t	altImodExplosion;// These are used to have a bit of variation in the explosions
	qhandle_t	electricalExplosionSlowShader;
	qhandle_t	surfaceExplosionShader;

	qhandle_t	scavExplosionFastShader;// These are used to have a bit of variation in the explosions
	qhandle_t	scavExplosionSlowShader;

	qhandle_t	IMODShader;
	qhandle_t	IMOD2Shader;
	qhandle_t	imodExplosionShader;

	qhandle_t	fountainShader;
	qhandle_t	rippleShader;
	qhandle_t	boltShader;
	qhandle_t	pjBoltShader;
	qhandle_t	trans1Shader;
	qhandle_t	trans2Shader;
	qhandle_t	borgFlareShader;
	qhandle_t	borgRingShader;
	qhandle_t	shockRingShader;
	qhandle_t	bigBoomShader;
	qhandle_t	quantumRingShader;

	qhandle_t	tetrionFlareShader;
	qhandle_t	tetrionTrail2Shader;

	qhandle_t	quantumExplosionShader;
	qhandle_t	quantumFlashShader;

	qhandle_t	protonBeamShader;
	qhandle_t	protonRingShader;
	qhandle_t	protonAltBeamShader;

	qhandle_t	plasmaShader;
	qhandle_t	bolt2Shader;
	qhandle_t	yellowBoltShader;
	qhandle_t	waterDropShader;
	qhandle_t	purpleParticleShader;
	qhandle_t	blueParticleShader;
	qhandle_t	ltblueParticleShader;
	qhandle_t	yellowParticleShader;
	qhandle_t	orangeParticleShader;
	qhandle_t	dkorangeParticleShader;

	qhandle_t	whiteLaserShader;
	qhandle_t	solidWhiteShader;
	qhandle_t	oilDropShader;
	qhandle_t	transportShader;
	qhandle_t	scavTransportShader;
	qhandle_t	disruptorShader;
	qhandle_t	phaserDisruptorShader;
	qhandle_t	quantumDisruptorShader;
	qhandle_t	electricBodyShader;
	qhandle_t	shieldShader;
	qhandle_t	redFlareShader;
	qhandle_t	redRingShader;
	qhandle_t	redRing2Shader;
	qhandle_t	stasisRingShader;

	qhandle_t	altIMODShader;
	qhandle_t	altIMOD2Shader;
	qhandle_t	altIMODEndcapShader;

	qhandle_t	blueHitShader;
	qhandle_t	ghostRingShader;
	qhandle_t	splosionShader;
	qhandle_t	bigShockShader;
	qhandle_t	spinShader;
	qhandle_t	flierShader;
	qhandle_t	dnBoltShader;
	qhandle_t	scavengerAltShader;
	qhandle_t	phaserFireShader;
	qhandle_t	sunnyFlareShader;
	qhandle_t	IMODendcapShader;
	
	qhandle_t	greenBurstShader;
	qhandle_t	greenTrailShader;
	qhandle_t	orangeRingShader;
	qhandle_t	orangeTrailShader;
	qhandle_t	yellowTrailShader;
	qhandle_t	zappyShader;

	qhandle_t	psychicShader;
	qhandle_t	psychicRingsShader;

	qhandle_t	whiteRingShader;
	qhandle_t	compressionRingShader;

	qhandle_t	stasisAltShader;
	qhandle_t	stasisBoltShader;

	// Warrior bot effects
	qhandle_t	muzzleBurst1Shader;
	qhandle_t	muzzleBurst3Shader;
	qhandle_t	frontMuzzleShader;

	// grenade test model
	qhandle_t	nukeModel;

	//greyscale chaotica gun effects
	qhandle_t	tetrionTrail2ShaderBW;
	qhandle_t	tetrionFlareShaderBW;
	qhandle_t	redFlareShaderBW;
	qhandle_t	redRingShaderBW;

	// Zoom
	qhandle_t	zoomMaskShader;
	qhandle_t	zoomBarShader;
	qhandle_t	zoomBar2Shader;
	qhandle_t	zoomInsertShader;
	qhandle_t	zoomInsert2Shader;
	qhandle_t	zoomArrowShader;

	// wall mark shaders
	qhandle_t	compressionMarkShader;
	qhandle_t	IMODMarkShader;
	qhandle_t	phaserMarkShader;
	qhandle_t	scavMarkShader;
	qhandle_t	bulletmarksShader;
	qhandle_t	rivetMarkShader;

	qhandle_t	shadowMarkShader;
	qhandle_t	wakeMarkShader;

	qhandle_t	laserShader;
	qhandle_t	borgEyeFlareShader;
	qhandle_t	assimTubesShader;
	qhandle_t	vohrConeShader;

	// special effects models / etc.
	qhandle_t	teleportEffectModel;
	qhandle_t	teleportEffectShader;
	qhandle_t	playerTeleportShader;
	qhandle_t	borgShieldShader;
	qhandle_t	borgFullBodyShieldShader;
	qhandle_t	fixitEffectShader;
	qhandle_t	forgeRemoveShader;
	qhandle_t	speciesPortalShader;
	qhandle_t	portalFlareShader;

	// Mission objectives
	qhandle_t	objcorner1;
	qhandle_t	objcorner2;
	qhandle_t	objcorner3;
	qhandle_t	pending;
	qhandle_t	notpending;

	// Interface media
	qhandle_t	ammoweapon;
	qhandle_t	ammoslider;


	qhandle_t	weaponbox;
	qhandle_t	weaponcap1;
	qhandle_t	weaponcap2;

	qhandle_t	talkingtop;
	qhandle_t	talkingbot;
	qhandle_t	talkingcap;

	qhandle_t	bracketlu;
	qhandle_t	bracketld;
	qhandle_t	bracketru;
	qhandle_t	bracketrd;

	//Tricorder alt-fire tactical display
	qhandle_t	TEDshader;
	qhandle_t	pICONshader;
	qhandle_t	eICONshader;
	qhandle_t	aICONshader;


	// sounds
	sfxHandle_t scavengerAltExplodeSnd;
	sfxHandle_t disintegrateSound;
	sfxHandle_t disintegrate2Sound;
	sfxHandle_t disintegrate3Sound;

	sfxHandle_t	grenadeBounce1;
	sfxHandle_t	grenadeBounce2;
	sfxHandle_t missileStick;
	sfxHandle_t	grenadeExplodeSnd;
	sfxHandle_t	grenadeAltExplodeSnd;
	sfxHandle_t	quantumBoom;

	sfxHandle_t	tetrionRicochet[3];

	sfxHandle_t	selectSound;
	sfxHandle_t	useNothingSound;
	sfxHandle_t	footsteps[FOOTSTEP_TOTAL][4];

	sfxHandle_t talkSound;
	sfxHandle_t	noAmmoSound;
	
	sfxHandle_t landSound;
	sfxHandle_t tedTextSound;

	sfxHandle_t interfaceSnd1;
	sfxHandle_t interfaceSnd2;
	sfxHandle_t interfaceSnd3;
	
	sfxHandle_t watrInSound;
	sfxHandle_t watrOutSound;
	sfxHandle_t watrUnSound;
	sfxHandle_t respawnSound;

	sfxHandle_t transporterSound;
	sfxHandle_t borgBeamInSound;
	sfxHandle_t scavBeamInSound;
	sfxHandle_t borgRecycleSound;
	sfxHandle_t stasisBeamInSound;
	sfxHandle_t stasisAppearSound;
	sfxHandle_t stasisBeamOutSound;
	sfxHandle_t spark1Sound;
	sfxHandle_t spark2Sound;
	sfxHandle_t spark3Sound;
	sfxHandle_t spark4Sound;
	sfxHandle_t spark5Sound;
	sfxHandle_t spark6Sound;

	sfxHandle_t hunterSeekerHoverSound;
	sfxHandle_t forgeRemovalSound;
	sfxHandle_t forgeBeaminSound;
	sfxHandle_t speciesBeaminSound;
	sfxHandle_t speciesBeamoutSound;

	//rifle
	sfxHandle_t rifleHitSound;
	sfxHandle_t ric1Sound;
	sfxHandle_t ric2Sound;
	sfxHandle_t ric3Sound;
	sfxHandle_t ric4Sound;
	sfxHandle_t ric5Sound;
	sfxHandle_t ric6Sound;

	//crossbow
	sfxHandle_t arrowMissSound;

	// Zoom
	sfxHandle_t	zoomStart;
	sfxHandle_t	zoomLoop;
	sfxHandle_t	zoomEnd;

	sfxHandle_t surfaceExpSound[3];
	sfxHandle_t electricExpSound[3];
	sfxHandle_t bigSurfExpSound;

	//tricorder alt-fire radar
	sfxHandle_t triRadarSound;
} cgMedia_t;

// The client game static (cgs) structure hold everything
// loaded or calculated from the gamestate.  It will NOT
// be cleared when a tournement restart is done, allowing
// all clients to begin playing instantly
typedef struct {
	gameState_t		gameState;			// gamestate from server
	glconfig_t		glconfig;			// rendering configuration
	float			screenXScale;		// derived from glconfig
	float			screenYScale;

//	float				charScale;		// Used by Proportional String print
//	float				bias;			// Used by Proportional String print

	int				serverCommandSequence;	// reliable command stream counter
	
	// parsed from serverinfo
	int				dmflags;
	int				teamflags;
	int				timelimit;
	int				maxclients;
	char			mapname[MAX_QPATH];

	//
	// locally derived information from gamestate
	//
	qhandle_t		model_draw[MAX_MODELS];
	sfxHandle_t		sound_precache[MAX_SOUNDS];

	int				numInlineModels;
	qhandle_t		inlineDrawModel[MAX_MODELS];
	vec3_t			inlineModelMidpoints[MAX_MODELS];

	clientInfo_t	clientinfo[MAX_CLIENTS];

	// media
	cgMedia_t	media;
} cgs_t;

extern	cgs_t			cgs;

#endif //__CG_MEDIA_H_