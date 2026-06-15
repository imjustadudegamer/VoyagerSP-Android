#include "cg_local.h"
#include "../game/anims.h"
#include "cg_media.h"

/////////////////////  this is a bit kludgy, but it only gives access to one
//							enum table because of the #define. May get changed.
#define CGAME_ONLY
#include "../client/fffx.h"
//
/////////////////////


extern void CG_LightningBolt( centity_t *cent, vec3_t origin );

#define	PHASER_HOLDFRAME	2

/*
=================
CG_RegisterWeapon

The server says this item is used on this level
=================
*/
void CG_RegisterWeapon( int weaponNum ) {
	weaponInfo_t	*weaponInfo;
	gitem_t			*item, *ammo;
	char			path[MAX_QPATH];
	vec3_t			mins, maxs;
	int				i;

	weaponInfo = &cg_weapons[weaponNum];

	// error checking
	if ( weaponNum == 0 ) {
		return;
	}

	if ( weaponInfo->registered ) {
		return;
	}

	// clear out the memory we use
	memset( weaponInfo, 0, sizeof( *weaponInfo ) );
	weaponInfo->registered = qtrue;

	// find the weapon in the item list
	for ( item = bg_itemlist + 1 ; item->classname ; item++ ) {
		if ( item->giType == IT_WEAPON && item->giTag == weaponNum ) {
			weaponInfo->item = item;
			break;
		}
	}
	// if we couldn't find which weapon this is, give us an error
	if ( !item->classname ) {
		CG_Error( "Couldn't find item for weapon %s\nNeed to update Items.dat!", weaponData[weaponNum].classname);
	}
	CG_RegisterItemVisuals( item - bg_itemlist );

	// set up in view weapon model
	weaponInfo->weaponModel = cgi_R_RegisterModel( weaponData[weaponNum].weaponMdl );

	if ( weaponInfo->weaponModel == NULL )
	{
		CG_Error( "Couldn't find weapon model %s\n", weaponData[weaponNum].classname);
		return;
	}

	// calc midpoint for rotation
	cgi_R_ModelBounds( weaponInfo->weaponModel, mins, maxs );
	for ( i = 0 ; i < 3 ; i++ ) {
		weaponInfo->weaponMidpoint[i] = mins[i] + 0.5 * ( maxs[i] - mins[i] );
	}
	// setup the shader we will use for the icon
	weaponInfo->weaponIcon = cgi_R_RegisterShaderNoMip( weaponData[weaponNum].weaponIcon);

	for ( ammo = bg_itemlist + 1 ; ammo->classname ; ammo++ ) {	
		if ( ammo->giType == IT_AMMO && ammo->giTag == weaponData[weaponNum].ammoIndex) {
			break;
		}
	}
	if ( ammo->classname && ammo->world_model ) {
		weaponInfo->ammoModel = cgi_R_RegisterModel( ammo->world_model );
	}

	for (i=0; i< weaponData[weaponNum].numBarrels; i++) {
		Q_strncpyz( path, weaponData[weaponNum].weaponMdl, MAX_QPATH );
		COM_StripExtension( path, path );
		if (i)
		{
			char	crap[50];
			sprintf(crap, "_barrel%d.md3", i+1);
			strcat( path, crap);
		}
		else
			strcat( path, "_barrel.md3" );
		weaponInfo->barrelModel[i] = cgi_R_RegisterModel( path );
	}


	// set up the world model for the weapon
	weaponInfo->weaponWorldModel = cgi_R_RegisterModel( item->world_model );
	if ( !weaponInfo->weaponWorldModel) {
		weaponInfo->weaponWorldModel = weaponInfo->weaponModel;
	}

	// set up the in view flash frame - 1
	strcpy( path, weaponData[weaponNum].weaponMdl );
	COM_StripExtension( path, path );
	strcat( path, "_flash.md3" );
	weaponInfo->flashModel = cgi_R_RegisterModel( path );

	// set up the in view flash frame - 2
	strcpy( path, weaponData[weaponNum].weaponMdl );
	COM_StripExtension( path, path );
	strcat( path, "_flash2.md3" );
	weaponInfo->flash2Model = cgi_R_RegisterModel( path );

	// set up the in view flash frame - alternate
	strcpy( path, weaponData[weaponNum].weaponMdl );
	COM_StripExtension( path, path );
	strcat( path, "_flasha.md3" );
	weaponInfo->flashaModel = cgi_R_RegisterModel( path );

	// set up the in view flash frame - alternate 2
	strcpy( path, weaponData[weaponNum].weaponMdl );
	COM_StripExtension( path, path );
	strcat( path, "_flasha2.md3" );
	weaponInfo->flasha2Model = cgi_R_RegisterModel( path );

	// set up the hand that holds the in view weapon - assuming we have one
	strcpy( path, weaponData[weaponNum].weaponMdl );
	COM_StripExtension( path, path );
	strcat( path, "_hand.md3" );
	weaponInfo->handsModel = cgi_R_RegisterModel( path );

	if ( !weaponInfo->handsModel ) {
		weaponInfo->handsModel = cgi_R_RegisterModel( "models/weapons2/prifle/prifle_hand.md3" );
	}

	// register the sounds for the weapon
	if (weaponData[weaponNum].firingSnd[0]) {
		weaponInfo->firingSound = cgi_S_RegisterSound( weaponData[weaponNum].firingSnd );
	}
	if (weaponData[weaponNum].altFiringSnd[0]) {
		weaponInfo->altFiringSound = cgi_S_RegisterSound( weaponData[weaponNum].altFiringSnd );
	}
	if (weaponData[weaponNum].flashSnd[0]) {
		weaponInfo->flashSound = cgi_S_RegisterSound( weaponData[weaponNum].flashSnd );
	}
	if (weaponData[weaponNum].altFlashSnd[0]) {
		weaponInfo->altFlashSound = cgi_S_RegisterSound( weaponData[weaponNum].altFlashSnd );
	}
	if (weaponData[weaponNum].stopSnd[0]) {
		weaponInfo->stopSound = cgi_S_RegisterSound( weaponData[weaponNum].stopSnd );
	}

	// give us missile models if we should
	if (weaponData[weaponNum].missileMdl[0]) 	{
		weaponInfo->missileModel = cgi_R_RegisterModel(weaponData[weaponNum].missileMdl );
	}
	if (weaponData[weaponNum].alt_missileMdl[0]) 	{
		weaponInfo->alt_missileModel = cgi_R_RegisterModel(weaponData[weaponNum].alt_missileMdl );
	}
	if (weaponData[weaponNum].missileHitSound[0]) {
		weaponInfo->missileHitSound = cgi_S_RegisterSound( weaponData[weaponNum].missileHitSound );
	}
	if (weaponData[weaponNum].altmissileHitSound[0]) {
		weaponInfo->altmissileHitSound = cgi_S_RegisterSound( weaponData[weaponNum].altmissileHitSound );
	}

	//fixme: don't really need to copy these, should just use directly
	// give ourselves the functions if we can
	if (weaponData[weaponNum].func)
	{
		weaponInfo->missileTrailFunc = (void (__cdecl *)(struct centity_s *,const struct weaponInfo_s *))weaponData[weaponNum].func;
	}
	if (weaponData[weaponNum].altfunc)
	{
		weaponInfo->alt_missileTrailFunc = (void (__cdecl *)(struct centity_s *,const struct weaponInfo_s *))weaponData[weaponNum].altfunc;
	}

	switch ( weaponNum )	//extra client only stuff
	{
	case WP_PHASER:
		cgs.media.phaserShader			= cgi_R_RegisterShader( "gfx/misc/phaser" );
		cgs.media.phaserFireShader		= cgi_R_RegisterShader( "gfx/misc/phaserbolt" ); 
		cgs.media.disintegrateSound			= cgi_S_RegisterSound( "sound/weapons/prifle/disint.wav" );
		cgs.media.disintegrate2Sound		= cgi_S_RegisterSound( "sound/weapons/prifle/disint2.wav" );
		cgs.media.disintegrate3Sound		= cgi_S_RegisterSound( "sound/weapons/prifle/disint3.wav" );
		break;

	case WP_COMPRESSION_RIFLE:
		cgs.media.prifleImpactShader		= cgi_R_RegisterShader( "gfx/effects/prifle_hit" );
		cgs.media.compressionAltBlastShader	= cgi_R_RegisterShader( "gfx/effects/prifle_altblast2" );
		cgs.media.compressionRingShader		= cgi_R_RegisterShader( "gfx/misc/compression_ring" );
		cgs.media.disintegrateSound			= cgi_S_RegisterSound( "sound/weapons/prifle/disint.wav" );
		cgs.media.disintegrate2Sound		= cgi_S_RegisterSound( "sound/weapons/prifle/disint2.wav" );
		cgs.media.disintegrate3Sound		= cgi_S_RegisterSound( "sound/weapons/prifle/disint3.wav" );
		break;

	case WP_IMOD:
		cgs.media.altImodExplosion		= cgi_R_RegisterShader( "electricalExplosionFast" );
		cgs.media.IMODShader			= cgi_R_RegisterShader( "gfx/misc/IMOD" );
		cgs.media.IMOD2Shader			= cgi_R_RegisterShader( "gfx/misc/IMOD2" );
		cgs.media.altIMODShader			= cgi_R_RegisterShader( "gfx/misc/IMODalt" );
		cgs.media.altIMOD2Shader		= cgi_R_RegisterShader( "gfx/misc/IMOD2alt" );
		cgs.media.IMODendcapShader		= cgi_R_RegisterShader( "gfx/misc/IMODendcap" );
		cgs.media.imodExplosionShader	= cgi_R_RegisterShader( "imodExplosion" );
		cgs.media.altIMODEndcapShader	= cgi_R_RegisterShader( "gfx/misc/IMODalt_endcap" );
		break;

	case WP_SCAVENGER_RIFLE:
		cgs.media.tetrionTrail2Shader	= cgi_R_RegisterShader( "gfx/misc/trail2" );
		cgs.media.tetrionFlareShader	= cgi_R_RegisterShader( "gfx/misc/tet1" );
		cgs.media.scavengerAltShader	= cgi_R_RegisterShader( "gfx/misc/scavaltfire" );
		cgs.media.scavExplosionFastShader	= cgi_R_RegisterShader( "scavExplosionFast" );
		cgs.media.scavExplosionSlowShader	= cgi_R_RegisterShader( "scavExplosionSlow" );
		cgs.media.redFlareShader			= cgi_R_RegisterShader( "gfx/misc/red_flare" );
		cgs.media.redRingShader				= cgi_R_RegisterShader( "gfx/misc/red_ring" );
		cgs.media.redRing2Shader			= cgi_R_RegisterShader( "gfx/misc/red_ring2" );
		cgs.media.scavengerAltExplodeSnd	= cgi_S_RegisterSound( "sound/weapons/scavenger/alt_explode.wav" );
		break;

	case WP_CHAOTICA_GUARD_GUN:
		cgs.media.tetrionTrail2ShaderBW		= cgi_R_RegisterShader( "gfx/misc/trail2BW" );
		cgs.media.tetrionFlareShaderBW		= cgi_R_RegisterShader( "gfx/misc/tet1BW" );
		cgs.media.redFlareShaderBW			= cgi_R_RegisterShader( "gfx/misc/red_flareBW" );
		cgs.media.redRingShaderBW			= cgi_R_RegisterShader( "gfx/misc/red_ringBW" );
		cgs.media.scavengerAltExplodeSnd	= cgi_S_RegisterSound( "sound/weapons/scavenger/alt_explode.wav" );
		break;

	case WP_STASIS:
		cgs.media.altIMOD2Shader		= cgi_R_RegisterShader( "gfx/misc/IMOD2alt" );
		cgs.media.stasisRingShader		= cgi_R_RegisterShader( "gfx/misc/stasis_ring" );
		cgs.media.stasisAltShader		= cgi_R_RegisterShader( "gfx/misc/stasis_altfire" );
		break;

	case WP_GRENADE_LAUNCHER:
		cgs.media.grenadeBounce1		= cgi_S_RegisterSound( "sound/weapons/glauncher/bounce1.wav" );
		cgs.media.grenadeBounce2		= cgi_S_RegisterSound( "sound/weapons/glauncher/bounce2.wav" );
		cgs.media.missileStick			= cgi_S_RegisterSound( "sound/weapons/glauncher/alt_stick.wav" );
		cgs.media.bigShockShader		= cgi_R_RegisterShader( "gfx/misc/bigshock" );
		cgs.media.orangeRingShader		= cgi_R_RegisterShader( "gfx/misc/orangering" );
		cgs.media.orangeTrailShader		= cgi_R_RegisterShader( "gfx/misc/orangetrail" );
		cgs.media.nukeModel				= cgi_R_RegisterModel ( "models/weaphits/nuke.md3" );
		cgs.media.grenadeExplodeSnd		= cgi_S_RegisterSound( "sound/weapons/glauncher/explode.wav" );
		cgs.media.grenadeAltExplodeSnd	= cgi_S_RegisterSound( "sound/weapons/glauncher/alt_explode.wav" );
		break;

	case WP_TETRION_DISRUPTOR:
		cgs.media.greenBurstShader		= cgi_R_RegisterShader( "gfx/misc/greenburst" );
		cgs.media.greenTrailShader		= cgi_R_RegisterShader( "gfx/misc/greentrail" );
		cgs.media.tetrionRicochet[0]	= cgi_S_RegisterSound ( "sound/weapons/tetrion/ricochet1.wav" );
		cgs.media.tetrionRicochet[1]	= cgi_S_RegisterSound ( "sound/weapons/tetrion/ricochet2.wav" );
		cgs.media.tetrionRicochet[2]	= cgi_S_RegisterSound ( "sound/weapons/tetrion/ricochet3.wav" );
		cgs.media.borgFlareShader		= cgi_R_RegisterShader( "gfx/misc/borgflare" );
		break;

	case WP_QUANTUM_BURST:
		cgs.media.nukeModel				= cgi_R_RegisterModel ( "models/weaphits/nuke.md3" );
		cgs.media.quantumExplosionShader= cgi_R_RegisterShader( "quantumExplosion" );
		cgs.media.quantumFlashShader	= cgi_R_RegisterShader( "yellowflash" );
		cgs.media.shockRingShader		= cgi_R_RegisterShader( "gfx/misc/shockring" );
		cgs.media.orangeTrailShader		= cgi_R_RegisterShader( "gfx/misc/orangetrail" );
		cgs.media.orangeRingShader		= cgi_R_RegisterShader( "gfx/misc/orangering" );
		cgs.media.quantumRingShader		= cgi_R_RegisterShader( "gfx/misc/detpack3" );
		cgs.media.quantumBoom			= cgi_S_RegisterSound ( "sound/weapons/explosions/explode5.wav" );
		break;

	case WP_DREADNOUGHT:
		cgs.media.dnBoltShader			= cgi_R_RegisterShader( "gfx/misc/dnBolt" );
		cgs.media.stasisRingShader		= cgi_R_RegisterShader( "gfx/misc/stasis_ring" );//for shot miss
		cgs.media.yellowTrailShader		= cgi_R_RegisterShader( "gfx/misc/yellowtrail" );
		cgs.media.zappyShader			= cgi_R_RegisterShader( "gfx/misc/disint_1" );
		cgs.media.electricBodyShader	= cgi_R_RegisterShader( "gfx/misc/electric" );
		break;

	case WP_FORGE_PROJ:		//reaver
		cgs.media.orangeTrailShader		= cgi_R_RegisterShader( "gfx/misc/orangetrail" );
		break;

	case WP_FORGE_PSYCH:	//avatar
		cgs.media.psychicRingsShader	= cgi_R_RegisterShader( "gfx/misc/psychic_rings" );
		break;

	case WP_BORG_WEAPON:
		cgs.media.borgRingShader		= cgi_R_RegisterShader( "gfx/misc/borgring" );
		break;

	case WP_DESPERADO:
		cgs.media.prifleImpactShader	= cgi_R_RegisterShader( "gfx/effects/prifle_hit" );
		cgs.media.compressionRingShader	= cgi_R_RegisterShader( "gfx/misc/compression_ring" );
		cgs.media.rifleHitSound			= cgi_S_RegisterSound( "sound/weapons/rifle/hit.wav" );
		cgs.media.ric1Sound				= cgi_S_RegisterSound( "sound/weapons/rifle/ric1.wav" );
		cgs.media.ric2Sound				= cgi_S_RegisterSound( "sound/weapons/rifle/ric2.wav" );
		cgs.media.ric3Sound				= cgi_S_RegisterSound( "sound/weapons/rifle/ric3.wav" );
		cgs.media.ric4Sound				= cgi_S_RegisterSound( "sound/weapons/rifle/ric4.wav" );
		cgs.media.ric5Sound				= cgi_S_RegisterSound( "sound/weapons/rifle/ric5.wav" );
		cgs.media.ric6Sound				= cgi_S_RegisterSound( "sound/weapons/rifle/ric6.wav" );
		break;

	case WP_PALADIN:
		cgs.media.orangeTrailShader		= cgi_R_RegisterShader( "gfx/misc/orangetrail" );
		cgs.media.tetrionFlareShader	= cgi_R_RegisterShader( "gfx/misc/tet1" );
		cgs.media.arrowMissSound		= cgi_S_RegisterSound( "sound/weapons/crossbow/miss.wav" );
		break;

	case WP_PROTON_GUN:
		cgs.media.protonBeamShader			= cgi_R_RegisterShader( "textures/rig/protonbeam" );
		cgs.media.protonRingShader			= cgi_R_RegisterShader( "textures/rig/protonring" );
		cgs.media.protonAltBeamShader		= cgi_R_RegisterShader( "textures/rig/bw_energy_ripples" );
//		cgs.media.compressionAltBlastShader	= cgi_R_RegisterShader( "gfx/effects/prifle_altblast2" );
//		cgs.media.compressionRingShader		= cgi_R_RegisterShader( "gfx/misc/compression_ring" );
//		cgs.media.disintegrateSound			= cgi_S_RegisterSound( "sound/weapons/prifle/disint.wav" );
//		cgs.media.disintegrate2Sound		= cgi_S_RegisterSound( "sound/weapons/prifle/disint2.wav" );
//		cgs.media.disintegrate3Sound		= cgi_S_RegisterSound( "sound/weapons/prifle/disint3.wav" );
		break;

	case WP_TRICORDER:
		cgs.media.TEDshader					= cgi_R_RegisterShader( "gfx/2d/TED" );
		cgs.media.pICONshader				= cgi_R_RegisterShader( "gfx/2d/picon" );
		cgs.media.eICONshader				= cgi_R_RegisterShader( "gfx/2d/eTEDicon" );
		cgs.media.aICONshader				= cgi_R_RegisterShader( "gfx/2d/aTEDicon" );
		cgs.media.triRadarSound				= cgi_S_RegisterSound( "sound/movers/triradar.mp3" );
		break;

	}
}

/*
=================
CG_RegisterItemVisuals

The server says this item is used on this level
=================
*/
void CG_RegisterItemVisuals( int itemNum ) {
	itemInfo_t		*itemInfo;
	gitem_t			*item;

	itemInfo = &cg_items[ itemNum ];
	if ( itemInfo->registered ) {
		return;
	}

	item = &bg_itemlist[ itemNum ];

	memset( itemInfo, 0, sizeof( *itemInfo ) );	// was sizeof(&itemInfo) (id bug): zeroed only a pointer's width (8B on LP64/arm64), not the itemInfo_t struct
	itemInfo->registered = qtrue;

	itemInfo->models = cgi_R_RegisterModel( item->world_model );

	itemInfo->icon = cgi_R_RegisterShaderNoMip( item->icon );

	if ( item->giType == IT_WEAPON ) {
		CG_RegisterWeapon( item->giTag );
	}
}


/*
========================================================================================

VIEW WEAPON

========================================================================================
*/

/*
=================
CG_MapTorsoToWeaponFrame

=================
*/
extern qboolean ValidAnimFileIndex ( int index );
int CG_MapTorsoToWeaponFrame( clientInfo_t *ci, int frame, int animNum, int weaponNum, int firing ) 
{
//we should use the animNum to map a weapon frame instead of relying on the torso frame
	if ( !ValidAnimFileIndex( ci->animFileIndex ) )
	{
		return 0;
	}
	animation_t *animations = knownAnimFileSets[ci->animFileIndex].animations;

	switch (animNum)
	{
	case TORSO_DROPWEAP1:
	case TORSO_RAISEWEAP1:
		// change weapon
		if ( frame >= animations[TORSO_DROPWEAP1].firstFrame 
			&& frame < animations[TORSO_DROPWEAP1].firstFrame + 5 ) {
			return frame - animations[TORSO_DROPWEAP1].firstFrame + 6;
		}

		if ( frame >= animations[TORSO_RAISEWEAP1].firstFrame 
			&& frame < animations[TORSO_RAISEWEAP1].firstFrame + 4 ) {
			return frame - animations[TORSO_RAISEWEAP1].firstFrame + 6 + 5;
		}
		break;
	case BOTH_ATTACK1:
	case BOTH_ATTACK2:
	case BOTH_ATTACK3:
		
		if ( weaponNum == WP_PHASER && firing)
		{
			return PHASER_HOLDFRAME;	//FIXME: Heh heh
		}
		else
		{
			if ( frame >= animations[animNum].firstFrame && frame < animations[animNum].firstFrame + 6 ) 
				return 1 + ( frame - animations[animNum].firstFrame );
		}

		break;
	}	
	return 0;
}


/*
==============
CG_CalculateWeaponPosition
==============
*/
void CG_CalculateWeaponPosition( vec3_t origin, vec3_t angles ) 
{
	float	scale;
	int		delta;
	float	fracsin;

	VectorCopy( cg.refdef.vieworg, origin );
	VectorCopy( cg.refdefViewAngles, angles );

	// on odd legs, invert some angles
	if ( cg.bobcycle & 1 ) {
		scale = -cg.xyspeed;
	} else {
		scale = cg.xyspeed;
	}

	// gun angles from bobbing
	angles[ROLL] += scale * cg.bobfracsin * 0.0075;
	angles[YAW] += scale * cg.bobfracsin * 0.01;
	angles[PITCH] += cg.xyspeed * cg.bobfracsin * 0.0075;

	// drop the weapon when landing
	delta = cg.time - cg.landTime;
	if ( delta < LAND_DEFLECT_TIME ) {
		origin[2] += cg.landChange*0.25 * delta / LAND_DEFLECT_TIME;
	} else if ( delta < LAND_DEFLECT_TIME + LAND_RETURN_TIME ) {
		origin[2] += cg.landChange*0.25 * 
			(LAND_DEFLECT_TIME + LAND_RETURN_TIME - delta) / LAND_RETURN_TIME;
	}

#if 0
	// drop the weapon when stair climbing
	delta = cg.time - cg.stepTime;
	if ( delta < STEP_TIME/2 ) {
		origin[2] -= cg.stepChange*0.25 * delta / (STEP_TIME/2);
	} else if ( delta < STEP_TIME ) {
		origin[2] -= cg.stepChange*0.25 * (STEP_TIME - delta) / (STEP_TIME/2);
	}
#endif

	// idle drift
	scale = /*cg.xyspeed + */40;
	fracsin = sin( cg.time * 0.001 );
	angles[ROLL] += scale * fracsin * 0.01;
	angles[YAW] += scale * fracsin * 0.01;
	angles[PITCH] += (scale * 0.5f ) * fracsin * 0.01;
}

/*
======================
CG_MachinegunSpinAngle
======================
*/
#define		SPIN_SPEED	0.9
#define		COAST_TIME	1000
static float	CG_MachinegunSpinAngle( centity_t *cent ) {
	int		delta;
	float	angle;
	float	speed;

	delta = cg.time - cent->pe.barrelTime;
	if ( cent->pe.barrelSpinning ) {
		angle = cent->pe.barrelAngle + delta * SPIN_SPEED;
	} else {
		if ( delta > COAST_TIME ) {
			delta = COAST_TIME;
		}

		speed = 0.5 * ( SPIN_SPEED + (float)( COAST_TIME - delta ) / COAST_TIME );
		angle = cent->pe.barrelAngle + delta * speed;
	}

	if ( cent->pe.barrelSpinning == !(cent->currentState.eFlags & EF_FIRING) ) {
		cent->pe.barrelTime = cg.time;
		cent->pe.barrelAngle = AngleMod( angle );
		cent->pe.barrelSpinning = !!(cent->currentState.eFlags & EF_FIRING);
	}

	return angle;
}

/*
==============
CG_AddViewWeapon

Add the weapon, and flash for the player's view
==============
*/
void CG_AddViewWeapon( playerState_t *ps ) {
	refEntity_t	hand;
	refEntity_t	gun;
	refEntity_t	flash;
	vec3_t		angles;
	const weaponInfo_t	*weapon;
	const weaponData_t  *wData;
	centity_t	*cent;
	clientInfo_t	*ci;
	float		fovOffset;

	// no gun if in third person view
	if ( cg.renderingThirdPerson )
		return;

	if ( ps->pm_type == PM_INTERMISSION )
		return;

	cent = &cg_entities[cg.snap->ps.clientNum];
	// allow the gun to be completely removed
	if ( !cg_drawGun.integer || cg.zoomed ) {
		vec3_t		origin;

		// special hack for lightning guns...
		VectorCopy( cg.refdef.vieworg, origin );
		VectorMA( origin, -8, cg.refdef.viewaxis[2], origin );
		VectorMA( origin, -6, cg.refdef.viewaxis[1], origin );
		CG_LightningBolt( cent, origin );

		return;
	}

	// don't draw if testing a gun model
	if ( cg.testGun )
		return;

	// drop gun lower at higher fov
	if ( cg_fov.integer > 80 ) {
		fovOffset = -0.1 * ( cg_fov.integer - 80 );
	} else {
		fovOffset = 0;
	}

	CG_RegisterWeapon( ps->weapon );
	weapon = &cg_weapons[ps->weapon];
	wData =  &weaponData[ps->weapon];

	memset (&hand, 0, sizeof(hand));

	// set up gun position
	CG_CalculateWeaponPosition( hand.origin, angles );

	VectorMA( hand.origin, cg_gun_x.value, cg.refdef.viewaxis[0], hand.origin );
	VectorMA( hand.origin, cg_gun_y.value, cg.refdef.viewaxis[1], hand.origin );
	VectorMA( hand.origin, (cg_gun_z.value+fovOffset), cg.refdef.viewaxis[2], hand.origin );

	AnglesToAxis( angles, hand.axis );

	// map torso animations to weapon animations
	if ( cg_gun_frame.integer ) {
		// development tool
		hand.frame = hand.oldframe = cg_gun_frame.integer;
		hand.backlerp = 0;
	} else {
		// get clientinfo for animation map
		ci = &cgs.clientinfo[ cent->currentState.clientNum ];
		hand.frame = CG_MapTorsoToWeaponFrame( ci, cent->pe.torso.frame, cent->pe.torso.animationNumber&~ANIM_TOGGLEBIT, cent->currentState.weapon, ( cent->currentState.eFlags & EF_FIRING ) );
		hand.oldframe = CG_MapTorsoToWeaponFrame( ci, cent->pe.torso.oldFrame, cent->pe.torso.animationNumber&~ANIM_TOGGLEBIT, cent->currentState.weapon, ( cent->currentState.eFlags & EF_FIRING ) );
		hand.backlerp = cent->pe.torso.backlerp;
	}

	// add the weapon
	memset (&gun, 0, sizeof(gun));

	// set custom shading for railgun refire rate
	gun.shaderRGBA[0] = 255;
	gun.shaderRGBA[1] = 255;
	gun.shaderRGBA[2] = 255;
	gun.shaderRGBA[3] = 255;

	gun.hModel = weapon->weaponModel;
	if (!gun.hModel) {
		return;
	}

	AnglesToAxis( angles, gun.axis );
	CG_PositionEntityOnTag( &gun, &hand, weapon->handsModel, "tag_weapon");

	gun.renderfx = RF_MINLIGHT | RF_DEPTHHACK | RF_FIRST_PERSON;

	cgi_R_AddRefEntityToScene( &gun );

	// add the spinning barrel[s]
	for (int i = 0; (i < wData->numBarrels); i++)	{
		refEntity_t	barrel;
		memset( &barrel, 0, sizeof( barrel ) );
		barrel.hModel = weapon->barrelModel[i];

		//VectorCopy( parent->lightingOrigin, barrel.lightingOrigin );
		//barrel.shadowPlane = parent->shadowPlane;
		barrel.renderfx = gun.renderfx;
		angles[YAW] = 0;
		angles[PITCH] = 0;
		if ( ps->weapon == WP_TETRION_DISRUPTOR) {
			angles[ROLL] = CG_MachinegunSpinAngle( cent );
		} else {
			angles[ROLL] = 0;//CG_MachinegunSpinAngle( cent );
		}

		AnglesToAxis( angles, barrel.axis );
//		CG_PositionRotatedEntityOnTag( &barrel, &gun, weapon->weaponModel, "tag_barrel" );
		if (!i) {
			CG_PositionRotatedEntityOnTag( &barrel, &hand, weapon->handsModel, "tag_barrel", NULL );
		} else {
			CG_PositionRotatedEntityOnTag( &barrel, &hand, weapon->handsModel, va("tag_barrel%d",i+1), NULL );
		}

		cgi_R_AddRefEntityToScene( &barrel );
	}

	memset (&flash, 0, sizeof(flash));

	// does this weapon have a flash model
	if (weapon->flashModel) {
		if ( cent->gent && cent->gent->alt_fire && weapon->flashaModel ) {
			if (weapon->flasha2Model && (rand()&1) ) {
				flash.hModel = weapon->flasha2Model;
			} else {
				flash.hModel = weapon->flashaModel;
			}
		} else {
			// always fallback to main flash
			if (weapon->flash2Model && (rand()&1)) {
				flash.hModel = weapon->flash2Model;
			} else {
				flash.hModel = weapon->flashModel;
			}
		}
		VectorClear( angles );
		AnglesToAxis( angles, flash.axis );

		CG_PositionRotatedEntityOnTag( &flash, &gun, weapon->weaponModel, "tag_flash", NULL);

		CG_LightningBolt( cent, flash.origin );

		// check to see if there should be an impulse flash
		if ( ( cg.time - cg_entities[ cg.snap->ps.clientNum ].muzzleFlashTime <= MUZZLE_FLASH_TIME ) ||
		   ( ( ps->weapon == WP_PHASER ) && (  cent->currentState.eFlags & EF_FIRING ) ) )
		{
			flash.renderfx = RF_DEPTHHACK | RF_FIRST_PERSON;
			cgi_R_AddRefEntityToScene( &flash );
		}
	}
	else
	{
		CG_PositionRotatedEntityOnTag( &flash, &gun, weapon->weaponModel, "tag_flash", NULL);
	}

	if (cent->gent && cent->gent->client)
	{
		VectorCopy(flash.origin, cent->gent->client->renderInfo.muzzlePoint);
		cent->gent->client->renderInfo.mPCalcTime = cg.time;
	}
}

/*
==============================================================================

WEAPON SELECTION

==============================================================================
*/

/*
===================
CG_DrawWeaponSelect
===================
*/
void CG_DrawWeaponSelect( void ) {
	int		i;
	int		bits;
	int		count;
	int		x, y;
	char	*name;
	float	*color;

	// don't display if dead
	if ( cg.predicted_player_state.stats[STAT_HEALTH] <= 0 ) {
		return;
	}

	color = CG_FadeColor( cg.weaponSelectTime, WEAPON_SELECT_TIME );
	if ( !color ) {
		return;
	}
	cgi_R_SetColor( color );

	// showing weapon select clears pickup item display, but not the blend blob
	cg.itemPickupTime = 0;

	bits = cg.snap->ps.stats[ STAT_WEAPONS ];
	// count the number of weapons owned
	count = 0;
	for ( i = 1 ; i < 16 ; i++ ) 
	{
		if ( bits & ( 1 << i ) ) 
		{
			count++;
		}
	}

	if (count == 0)	// If no weapons, don't display
	{
		return;
	}

	x = 364 - (count * 20);
	y = 432;

	// Left end cap
	cgi_R_SetColor(colorTable[CT_LTPURPLE2]);
	CG_DrawPic( x - 16, y - 4, 16, 50, cgs.media.weaponcap1);
	cgi_R_SetColor(NULL);	

	for ( i = 1 ; i < 14 ; i++ ) 
	{
		if ( !( bits & ( 1 << i ) ) ) 
		{
			continue;
		}



		if (weaponData[i].ammoIndex == AMMO_STARFLEET)
		{
			if ( i == cg.weaponSelect ) 
			{
				cgi_R_SetColor(colorTable[CT_LTBLUE2]);
			}
			else
			{
				cgi_R_SetColor(colorTable[CT_DKBLUE2]);
			}
		}
		else if (weaponData[i].ammoIndex == AMMO_ALIEN)
		{
			if ( i == cg.weaponSelect ) 
			{
				cgi_R_SetColor(colorTable[CT_LTPURPLE2]);
			}
			else
			{
				cgi_R_SetColor(colorTable[CT_DKPURPLE2]);
			}
		}
		else
		{
			if ( i == cg.weaponSelect ) 
			{
				cgi_R_SetColor(colorTable[CT_LTGOLD1]);
			}
			else
			{
				cgi_R_SetColor(colorTable[CT_DKGOLD1]);
			}
		}

		// draw selection marker
//		if ( i == cg.weaponSelect ) 
//		{
//			cgi_R_SetColor(colorTable[CT_LTPURPLE1]);
//		}
//		else
//		{
//			cgi_R_SetColor(colorTable[CT_DKPURPLE1]);
//		}


		CG_DrawPic( x-4,y-4,38, 38, cgs.media.weaponbox);
		cgi_R_SetColor(NULL);

		// draw weapon icon
		if (weaponData[i].weaponIcon)
		{
//			weaponInfo_t	*weaponInfo;
//			CG_RegisterWeapon( i );	
//			weaponInfo = &cg_weapons[i];
//			CG_DrawPic( x, y, 32, 32, weaponInfo->weaponIcon);
			CG_DrawPic( x, y, 32, 32, cgi_R_RegisterShaderNoMip( weaponData[i].weaponIcon) ); //only cache the icon for display
		}

		// no ammo icon on top of weapon icon
		if ( !cg.snap->ps.ammo[weaponData[i].ammoIndex] ) 
		{
			CG_DrawPic( x, y, 32, 32, cgs.media.noammoShader );
		}

		x += 40;
	}

	// Right end cap
	cgi_R_SetColor(colorTable[CT_LTPURPLE2]);
	CG_DrawPic( x - 20 + 18, y - 4, 16, 50, cgs.media.weaponcap2);
	cgi_R_SetColor(NULL);	

	// draw the selected name
	if ( cg_weapons[ cg.weaponSelect ].item ) 
	{
		name = cg_weapons[ cg.weaponSelect ].item->pickup_name;
		if ( name ) 
		{
			CG_DrawProportionalString(360, y - 22, name, CG_CENTER | CG_SMALLFONT, colorTable[CT_LTGOLD1]);
		}
	}

	cgi_R_SetColor( NULL );
}


/*
===============
CG_WeaponSelectable
===============
*/
qboolean CG_WeaponSelectable( int i ) 
{
	int	usage_for_weap;

	if (i > MAX_PLAYER_WEAPONS)
	{	
#ifndef FINAL_BUILD
		Com_Printf("CG_WeaponSelectable() passed illegal index of %d!\n",i);
#endif
		return qfalse;
	}

	if ( weaponData[i].ammoIndex != AMMO_NONE )
	{//weapon uses ammo, see if we have any
		usage_for_weap = weaponData[i].energyPerShot < weaponData[i].altEnergyPerShot 
									? weaponData[i].energyPerShot 
									: weaponData[i].altEnergyPerShot;

		if ( cg.snap->ps.ammo[weaponData[i].ammoIndex] - usage_for_weap <= 0 ) 
		{
			// This weapon doesn't have enough ammo to shoot either the main or the alt-fire
			return qfalse;
		}
	}

	if (!(cg.snap->ps.stats[ STAT_WEAPONS ] & ( 1 << i ))) 
	{
		// Don't have this weapon to start with.
		return qfalse;
	}

	return qtrue;
}

/*
===============
CG_NextWeapon_f
===============
*/
void CG_NextWeapon_f( void ) {
	int		i;
	int		original;

	if ( !cg.snap ) {
		return;
	}
	if ( cg.snap->ps.pm_flags & PMF_FOLLOW ) {
		return;
	}

	if ( cg.weaponSelect == WP_BLUE_HYPO || cg.weaponSelect == WP_RED_HYPO ) {
		return;
	}

	cg.weaponSelectTime = cg.time;
	original = cg.weaponSelect;

	
	for ( i = 0 ; i <= MAX_PLAYER_WEAPONS ; i++ ) {
		cg.weaponSelect++;

		if ( cg.weaponSelect < FIRST_WEAPON || cg.weaponSelect > MAX_PLAYER_WEAPONS) { 
			cg.weaponSelect = FIRST_WEAPON; 
		}
		
		if ( CG_WeaponSelectable( cg.weaponSelect ) ) {
			return;
		}
	}

	cg.weaponSelect = original;
}

/*
===============
CG_PrevWeapon_f
===============
*/
void CG_PrevWeapon_f( void ) {
	int		i;
	int		original;

	if ( !cg.snap ) {
		return;
	}
	if ( cg.snap->ps.pm_flags & PMF_FOLLOW ) {
		return;
	}

	if ( cg.weaponSelect == WP_BLUE_HYPO || cg.weaponSelect == WP_RED_HYPO ) {
		return;
	}

	cg.weaponSelectTime = cg.time;
	original = cg.weaponSelect;

	for ( i = 0 ; i <= MAX_PLAYER_WEAPONS ; i++ ) {
		cg.weaponSelect--;
		if ( cg.weaponSelect < FIRST_WEAPON || cg.weaponSelect > MAX_PLAYER_WEAPONS) { 
			cg.weaponSelect = MAX_PLAYER_WEAPONS;
		}
		
		if ( CG_WeaponSelectable( cg.weaponSelect ) ) {
			return;
		}
	}

	cg.weaponSelect = original;
}

/*
void CG_ChangeWeapon( int num )

  Meant to be called from the normal game, so checks the game-side weapon inventory data
*/
void CG_ChangeWeapon( int num )
{
	gentity_t	*player = &g_entities[0];

	if ( num < 1 || num >= WP_NUM_WEAPONS ) 
	{
		return;
	}

	cg.weaponSelectTime = cg.time;

	if ( player->client != NULL && !(player->client->ps.stats[STAT_WEAPONS] & ( 1 << num )) ) 
	{
		return;		// don't have the weapon
	}

	cg.weaponSelect = num;
}

/*
===============
CG_Weapon_f
===============
*/
void CG_Weapon_f( void ) {
	int		num;

	if ( !cg.snap ) {
		return;
	}
	if ( cg.snap->ps.pm_flags & PMF_FOLLOW ) {
		return;
	}

	if ( cg.weaponSelect == WP_BLUE_HYPO || cg.weaponSelect == WP_RED_HYPO ) {
		return;
	}

	if ( Q_stricmp( "messagemode2", CG_Argv( 0 ) ) == 0 )
	{//special hack for tricorder
		num = WP_TRICORDER;
	}
	else
	{
		num = atoi( CG_Argv( 1 ) );
		if ( num == WP_PHASER && cg.weaponSelect == WP_PHASER && (cg.snap->ps.stats[STAT_WEAPONS] & ( 1 << WP_PROTON_GUN )) ) 
		{
			num = WP_PROTON_GUN;
		}
		else if ( num == WP_PROTON_GUN && cg.weaponSelect == WP_PROTON_GUN && (cg.snap->ps.stats[STAT_WEAPONS] & ( 1 << WP_PHASER )) ) 
		{
			num = WP_PHASER;
		}
	}

	if ( num < WP_NONE || num >= WP_NUM_WEAPONS ) {
		return;
	}

	cg.weaponSelectTime = cg.time;

	if ( ! ( cg.snap->ps.stats[STAT_WEAPONS] & ( 1 << num ) ) ) {
		return;		// don't have the weapon
	}

	cg.weaponSelect = num;
}

/*
===================
CG_OutOfAmmoChange

The current weapon has just run out of ammo
===================
*/
void CG_OutOfAmmoChange( void ) {
	int		i;
	int		original;

	cgi_FF_StartFX( fffx_OutOfAmmo );

	if ( cg.weaponSelectTime + 200 > cg.time )
		return;

	cg.weaponSelectTime = cg.time;
	original = cg.weaponSelect;

	for ( i = WP_DREADNOUGHT; i > 0 ; i-- ) {	//We don't want the proton, tricorder or hypos here
		if ( original != i && CG_WeaponSelectable( i ) )
		{
			if ( 1 == cg_autoswitch.integer && ( i == WP_GRENADE_LAUNCHER || i == WP_QUANTUM_BURST) ) // safe weapon switch
			{
				// don't switch to this weapon
			}
			else
			{
				cg.weaponSelect = i;
				break;
			}
		}
	}
}



/*
===================================================================================================

WEAPON EVENTS

===================================================================================================
*/

/*
================
CG_FireWeapon

Caused by an EV_FIRE_WEAPON event
================
*/
void CG_FireWeapon( centity_t *cent, qboolean alt_fire ) 
{
	entityState_t *ent;
	weaponInfo_t	*weap;

	ent = &cent->currentState;
	if ( ent->weapon == WP_NONE ) {
		return;
	}
	if ( ent->weapon >= WP_NUM_WEAPONS ) {
		CG_Error( "CG_FireWeapon: ent->weapon >= WP_NUM_WEAPONS" );
		return;
	}
	weap = &cg_weapons[ ent->weapon ];

	// mark the entity as muzzle flashing, so when it is added it will
	// append the flash to the weapon model
	cent->muzzleFlashTime = cg.time;

	// lightning type guns only does this this on initial press
	if ( ent->weapon == WP_PHASER || ( ent->weapon == WP_DREADNOUGHT && !alt_fire )
									|| ent->weapon == WP_BORG_DRILL || ent->weapon == WP_BORG_ASSIMILATOR )
	{
		if ( cent->pe.lightningFiring ) 
		{
			if ( ent->weapon == WP_DREADNOUGHT )
			{
				cgi_FF_EnsureFX( fffx_Laser3 );
			}
			return;
		}
	}

	// force feedback...
	//
	switch (ent->weapon)
	{
		case WP_PHASER:				
			
			cgi_FF_StartFX( fffx_SwitchClick );	// repeat-fire handled above, but this just give an initial jolt
			break;

		case WP_STASIS:
		case WP_COMPRESSION_RIFLE:	
		case WP_IMOD:
		case WP_PROTON_GUN:
			
			cgi_FF_StartFX( alt_fire ? fffx_Shotgun : fffx_Pistol);	
			break;

		case WP_SCAVENGER_RIFLE:
		case WP_CHAOTICA_GUARD_GUN:
			
			cgi_FF_StartFX( alt_fire ? fffx_Missile : fffx_Pistol);	
			break;

		case WP_GRENADE_LAUNCHER:	
			
			cgi_FF_StartFX( alt_fire ? fffx_Jump : fffx_Punched);
			break;

		case WP_TETRION_DISRUPTOR:	
			
			cgi_FF_StartFX( alt_fire ? fffx_MachineGun : fffx_GatlingGun);
			break;

		case WP_QUANTUM_BURST:		
			
			cgi_FF_StartFX( alt_fire ? fffx_RocketLaunch : fffx_Punched);	
			break;

		case WP_DREADNOUGHT:		
			
			cgi_FF_StartFX( fffx_Shotgun );		// repeat-fire handled above
			break;

		case WP_TRICORDER:			
			
			cgi_FF_StartFX( fffx_SwitchClick);	
			break;

		case WP_BLUE_HYPO:			
		case WP_RED_HYPO:			
		case WP_VOYAGER_HYPO:
			
			cgi_FF_StartFX( fffx_SecretDoor);	
			break;
	}	

	if ( alt_fire )
	{
		if ( weap->altFlashSound )
		{
			cgi_S_StartSound( NULL, ent->number, CHAN_WEAPON, weap->altFlashSound );
		}
	}
	else
	{
		if ( weap->flashSound )
		{
			cgi_S_StartSound( NULL, ent->number, CHAN_WEAPON, weap->flashSound );
		}
	}
}

/*
=================
CG_BounceEffect

Caused by an EV_BOUNCE | EV_BOUNCE_HALF event
=================
*/
void CG_BounceEffect( centity_t *cent, int weapon, vec3_t origin, vec3_t normal )
{
	switch( weapon )
	{
	case WP_GRENADE_LAUNCHER:
		if ( rand() & 1 ) {
			cgi_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.grenadeBounce1 );
		} else {
			cgi_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.grenadeBounce2 );
		}
		break;

	case WP_TETRION_DISRUPTOR:
		cgi_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.tetrionRicochet[Q_irand(1, 3)] );
		FX_TetrionRicochet( origin, normal );	
		break;

	default:
		if ( rand() & 1 ) {
			cgi_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.grenadeBounce1 );
		} else {
			cgi_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.grenadeBounce2 );
		}
		break;
	}
}

/*
=================
CG_MissileHitWall

Caused by an EV_MISSILE_MISS event, or directly by local bullet tracing
=================
*/
void CG_MissileHitWall( centity_t *cent, int weapon, vec3_t origin, vec3_t dir ) 
{
	switch( weapon )
	{
	case WP_BORG_WEAPON:
		FX_BorgWeaponHitWall( origin, dir );
		break;
	
	case WP_TETRION_DISRUPTOR:
		FX_TetrionAltHitWall( origin, dir );
		break;

	case WP_STASIS:
		FX_StasisWeaponHitWall( origin, dir, cent->gent->count );
		break;

	case WP_SCAVENGER_RIFLE:
		if ( cent->gent->alt_fire )
		{
			FX_ScavengerAltExplode( origin, dir );
		}
		else
		{
			FX_ScavengerWeaponHitWall( origin, dir, (qboolean)cent->gent->count, qfalse );
		}
		break;

	case WP_CHAOTICA_GUARD_GUN:
		if ( cent->gent->alt_fire )
		{
			FX_ScavengerAltExplode( origin, dir );
		}
		else
		{
			FX_ScavengerWeaponHitWall( origin, dir, (qboolean)cent->gent->count, qtrue );
		}
		break;

	case WP_DREADNOUGHT:
		FX_DreadnoughtHitWall( origin, dir, qtrue );
		break;

	case WP_QUANTUM_BURST:
		if ( cent->gent->alt_fire )
			FX_QuantumAltHitWall( origin, dir );
		else
			FX_QuantumHitWall( origin, dir );
		break;

	case WP_GRENADE_LAUNCHER:
		FX_GrenadeHitWall( origin, dir );
		break;

	case WP_BOT_ROCKET:
		FX_BotRocketHitWall( origin, dir );
		break;

	case WP_FORGE_PROJ:
		FX_ForgeProjHitWall( origin, dir );
		break;

	case WP_FORGE_PSYCH:
		FX_ForgePsychHitWall( origin, dir );
		break;

	case WP_PARASITE:
		FX_ParasiteAcidHitWall( origin, dir );
		break;

	case WP_STASIS_ATTACK:
		FX_StasisAttackHitWall( origin, dir );
		break;

	case WP_DN_TURRET:
		FX_DNTurretHitWall( origin, dir );
		break;

	case WP_PALADIN:
		FX_PaladinProjHitWall( origin, dir );
		break;
	
	case WP_DESPERADO:
		FX_RifleHitWall( origin, dir );
		break;
	}
}

/*
-------------------------
CG_MissileHitPlayer
-------------------------
*/

void CG_MissileHitPlayer( centity_t *cent, int weapon, vec3_t origin, vec3_t dir ) 
{
	switch( weapon )
	{
	case WP_BORG_WEAPON:
		FX_BorgWeaponHitPlayer( origin, dir );
		break;

	case WP_SCAVENGER_RIFLE:
		if ( cent->gent->alt_fire )
			FX_ScavengerAltExplode( origin, dir );
		else
			FX_ScavengerWeaponHitPlayer( origin, dir, (qboolean)cent->gent->count, qfalse );
		break;

	case WP_CHAOTICA_GUARD_GUN:
		if ( cent->gent->alt_fire )
			FX_ScavengerAltExplode( origin, dir );
		else
			FX_ScavengerWeaponHitPlayer( origin, dir, (qboolean)cent->gent->count, qtrue );
		break;

	case WP_TETRION_DISRUPTOR:
//		FX_TetrionAltHitPlayer( origin, dir );
		break;

	case WP_STASIS:
		FX_StasisWeaponHitWall( origin, dir, cent->gent->count );
		break;
		
	case WP_DREADNOUGHT:
		FX_DreadnoughtHitPlayer( origin, dir );
		break;

	case WP_QUANTUM_BURST:
		if ( cent->gent->alt_fire )
			FX_QuantumAltHitWall( origin, dir );
		else
			FX_QuantumHitWall( origin, dir );
		break;

	case WP_GRENADE_LAUNCHER:
		FX_GrenadeHitPlayer( origin, dir );
		break;

	case WP_BOT_ROCKET:
		FX_BotRocketHitPlayer( origin, dir );
		break;

	case WP_FORGE_PROJ:
		FX_ForgeProjHitPlayer( origin, dir );
		break;

	case WP_FORGE_PSYCH:
		FX_ForgePsychHitPlayer( origin, dir );
		break;

	case WP_PARASITE:
		FX_ParasiteAcidHitPlayer( origin, dir );
		break;

	case WP_STASIS_ATTACK:
		FX_StasisAttackHitPlayer( origin, dir );
		break;

	case WP_DN_TURRET:
		FX_DNTurretHitWall( origin, dir );
		break;

	case WP_PALADIN:
		FX_PaladinProjHitPlayer( origin, dir );
		break;

	case WP_DESPERADO:
		FX_RifleHitPlayer( origin );
		break;
	}
}
