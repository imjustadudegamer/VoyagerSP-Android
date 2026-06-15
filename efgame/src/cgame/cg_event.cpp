// cg_event.c -- handle entity events at snapshot or playerstate transitions

#include "cg_local.h"
#include "cg_media.h"
#include "../game/anims.h"

extern void CG_TryPlayCustomSound( vec3_t origin, int entityNum, soundChannel_t channel, const char *soundName, int customSoundSet );
extern const char	*cg_customScavSoundNames[];

//==========================================================================

qboolean CG_IsFemale( const char *infostring ) {
	char		*sex;

	sex = Info_ValueForKey( infostring, "s" );
	if (sex[0] == 'f' || sex[0] == 'F')
		return qtrue;
	return qfalse;
}

const char	*CG_PlaceString( int rank ) {
	static char	str[64];
	char	*s, *t;

	if ( rank & RANK_TIED_FLAG ) {
		rank &= ~RANK_TIED_FLAG;
		t = "Tied for ";
	} else {
		t = "";
	}

	if ( rank == 1 ) {
		s = "\03341st\0337";		// draw in blue
	} else if ( rank == 2 ) {
		s = "\03312nd\0337";		// draw in red
	} else if ( rank == 3 ) {
		s = "\03333rd\0337";		// draw in yellow
	} else if ( rank == 11 ) {
		s = "11th";
	} else if ( rank == 12 ) {
		s = "12th";
	} else if ( rank == 13 ) {
		s = "13th";
	} else if ( rank % 10 == 1 ) {
		s = va("%ist", rank);
	} else if ( rank % 10 == 2 ) {
		s = va("%ind", rank);
	} else if ( rank % 10 == 3 ) {
		s = va("%ird", rank);
	} else {
		s = va("%ith", rank);
	}

	Com_sprintf( str, sizeof( str ), "%s%s", t, s );
	return str;
}


/*
================
CG_ItemPickup

A new item was picked up this frame
================
*/
void CG_ItemPickup( int itemNum ) {
	cg.itemPickup = itemNum;
	cg.itemPickupTime = cg.time;
	cg.itemPickupBlendTime = cg.time;
	// see if it should be the grabbed weapon
	if ( bg_itemlist[itemNum].giType == IT_WEAPON ) 
	{
		int nCurWpn = cg.predicted_player_state.weapon;
		int nNewWpn = bg_itemlist[itemNum].giTag;

		// kef -- check cg_autoswitch...
		//
		// 0 == no switching
		// 1 == automatically switch to best SAFE weapon
		// 2 == automatically switch to best weapon, safe or otherwise
		//
		// NOTE: automatically switching to any weapon you pick up is stupid and annoying and we won't do it.
		//

		if (0 == cg_autoswitch.integer || nNewWpn == WP_BLUE_HYPO || nNewWpn == WP_RED_HYPO )
		{
			// don't switch
		}
		else if (1 == cg_autoswitch.integer)
		{
			// safe switching
			if (	(nNewWpn > nCurWpn) &&
					!(nNewWpn == WP_GRENADE_LAUNCHER) &&
					!(nNewWpn == WP_QUANTUM_BURST) )
			{
				// switch to new wpn
				cg.weaponSelectTime = cg.time;
				cg.weaponSelect = nNewWpn;
			}
		}
		else if (2 == cg_autoswitch.integer)
		{
			// best
			if (nNewWpn > nCurWpn)
			{
				// switch to new wpn
				cg.weaponSelectTime = cg.time;
				cg.weaponSelect = nNewWpn;
			}
		}
	}
}



/*
==============
CG_EntityEvent

An entity has an event value
==============
*/
#define	DEBUGNAME(x) if(cg_debugEvents.integer){CG_Printf(x"\n");}
void CG_EntityEvent( centity_t *cent, vec3_t position ) {
	entityState_t	*es;
	int				event;
	vec3_t			dir;
	const char		*s;
	int				clientNum;
	//clientInfo_t	*ci;

	es = &cent->currentState;
	event = es->event & ~EV_EVENT_BITS;

	if ( cg_debugEvents.integer ) {
		CG_Printf( "ent:%3i  event:%3i ", es->number, event );
	}

	if ( !event ) {
		DEBUGNAME("ZEROEVENT");
		return;
	}

	if ( !cent->gent )//|| !cent->gent->client )
	{
		return;
	}

	//ci = &cent->gent->client->clientInfo;
	clientNum = cent->gent->s.number;

	switch ( event ) {
	//
	// movement generated events
	//
	case EV_FOOTSTEP:
		DEBUGNAME("EV_FOOTSTEP");
		if (cg_footsteps.integer) {
			if ( cent->gent && cent->gent->s.number == 0 && !cg_thirdPerson.integer )
			{//Everyone else has keyframed footsteps in animsounds.cfg
				cgi_S_StartSound (NULL, es->number, CHAN_BODY, 
					cgs.media.footsteps[ FOOTSTEP_NORMAL ][rand()&3] );
			}
		}
		break;
	case EV_FOOTSTEP_METAL:
		DEBUGNAME("EV_FOOTSTEP_METAL");
		if (cg_footsteps.integer) 
		{
			if ( cent->gent && cent->gent->s.number == 0 && !cg_thirdPerson.integer )
			{//Everyone else has keyframed footsteps in animsounds.cfg
				cgi_S_StartSound (NULL, es->number, CHAN_BODY, cgs.media.footsteps[ FOOTSTEP_METAL ][rand()&3] );
			}
		}
		break;
	case EV_FOOTSPLASH:
		DEBUGNAME("EV_FOOTSPLASH");
		if (cg_footsteps.integer) {
			cgi_S_StartSound (NULL, es->number, CHAN_BODY, 
				cgs.media.footsteps[ FOOTSTEP_SPLASH ][rand()&3] );
		}
		break;
	case EV_FOOTWADE:
		DEBUGNAME("EV_FOOTWADE");
		if (cg_footsteps.integer) {
			cgi_S_StartSound (NULL, es->number, CHAN_BODY, 
				cgs.media.footsteps[ FOOTSTEP_SPLASH ][rand()&3] );
		}
		break;
	case EV_SWIM:
		DEBUGNAME("EV_SWIM");
		if (cg_footsteps.integer) {
			cgi_S_StartSound (NULL, es->number, CHAN_BODY, 
				cgs.media.footsteps[ FOOTSTEP_SPLASH ][rand()&3] );
		}
		break;


	case EV_FALL_SHORT:
		DEBUGNAME("EV_FALL_SHORT");
		cgi_S_StartSound( NULL, es->number, CHAN_AUTO, cgs.media.landSound  );
		if ( clientNum == cg.predicted_player_state.clientNum ) {
			// smooth landing z changes
			cg.landChange = -8;
			cg.landTime = cg.time;
		}
		break;
	case EV_FALL_MEDIUM:
		DEBUGNAME("EV_FALL_MEDIUM");
		// use normal pain sound - 
		CG_TryPlayCustomSound( NULL, es->number, CHAN_BODY, "*pain100.wav", CS_BASIC );
		if ( clientNum == cg.predicted_player_state.clientNum ) {
			// smooth landing z changes
			cg.landChange = -16;
			cg.landTime = cg.time;
		}
		break;
	case EV_FALL_FAR:
		DEBUGNAME("EV_FALL_FAR");
		CG_TryPlayCustomSound(NULL, es->number, CHAN_AUTO, "*fall1.wav", CS_BASIC );
		cent->pe.painTime = cg.time;	// don't play a pain sound right after this
		if ( clientNum == cg.predicted_player_state.clientNum ) {
			// smooth landing z changes
			cg.landChange = -24;
			cg.landTime = cg.time;
		}
		break;

	case EV_STEP_4:
	case EV_STEP_8:
	case EV_STEP_12:
	case EV_STEP_16:		// smooth out step up transitions
		DEBUGNAME("EV_STEP");
	{
		float	oldStep;
		int		delta;
		int		step;

		if ( clientNum != cg.predicted_player_state.clientNum ) {
			break;
		}
		// if we are interpolating, we don't need to smooth steps
		if ( cg_timescale.value >= 1.0f )
		{
			break;
		}
		// check for stepping up before a previous step is completed
		delta = cg.time - cg.stepTime;
		if (delta < STEP_TIME) {
			oldStep = cg.stepChange * (STEP_TIME - delta) / STEP_TIME;
		} else {
			oldStep = 0;
		}

		// add this amount
		step = 4 * (event - EV_STEP_4 + 1 );
		cg.stepChange = oldStep + step;
		if ( cg.stepChange > MAX_STEP_CHANGE ) {
			cg.stepChange = MAX_STEP_CHANGE;
		}
		cg.stepTime = cg.time;
		break;
	}

	case EV_JUMP:
		DEBUGNAME("EV_JUMP");
		CG_TryPlayCustomSound(NULL, es->number, CHAN_VOICE, "*jump1.wav", CS_BASIC );
		break;

	case EV_WATER_TOUCH:
		DEBUGNAME("EV_WATER_TOUCH");
		cgi_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.watrInSound );
		break;
	
	case EV_WATER_LEAVE:
		DEBUGNAME("EV_WATER_LEAVE");
		cgi_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.watrOutSound );
		break;
	
	case EV_WATER_UNDER:
		DEBUGNAME("EV_WATER_UNDER");
		cgi_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.watrUnSound );
		break;
	
	case EV_WATER_CLEAR:
		DEBUGNAME("EV_WATER_CLEAR");
		CG_TryPlayCustomSound(NULL, es->number, CHAN_AUTO, "*gasp.wav", CS_BASIC );
		break;

	case EV_ITEM_PICKUP:
		DEBUGNAME("EV_ITEM_PICKUP");
		{
			gitem_t	*item;
			int		index;

			index = es->eventParm;		// player predicted

			if ( index < 1 || index >= bg_numItems ) {
				break;
			}
			item = &bg_itemlist[ index ];
			// Guard a data-driven empty pickup_sound: an item whose .dat pickupsound token is "" reaches
			// S_RegisterSound("") -> engine "WARNING: Sound name is empty" + a silent (handle-0) pickup.
			// Retail items always carry PICKUPSOUND ("sound/weapons/w_pkup.wav"); only register/play if set.
			if ( item->pickup_sound && item->pickup_sound[0] )
				cgi_S_StartSound (NULL, es->number, CHAN_AUTO,	cgi_S_RegisterSound( item->pickup_sound ) );

			// show icon and name on status bar
			if ( es->number == cg.snap->ps.clientNum ) {
				CG_ItemPickup( index );
			}
		}
		break;

	//
	// weapon events
	//
	case EV_NOAMMO:
		DEBUGNAME("EV_NOAMMO");
		//cgi_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.noAmmoSound );
		if ( es->number == cg.snap->ps.clientNum ) {
			CG_OutOfAmmoChange();
		}
		break;
	case EV_CHANGE_WEAPON:
		DEBUGNAME("EV_CHANGE_WEAPON");
		cgi_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.selectSound );
		break;

	case EV_FIRE_WEAPON:
		DEBUGNAME("EV_FIRE_WEAPON");
		CG_FireWeapon( cent, qfalse );
		break;

	case EV_ALT_FIRE:
		DEBUGNAME("EV_ALT_FIRE");
		CG_FireWeapon( cent, qtrue );
		break;

	//=================================================================

	//
	// other events
	//
	case EV_PLAYER_TELEPORT_IN:
		DEBUGNAME("EV_PLAYER_TELEPORT_IN");
		FX_PlayerTeleportIn( cent, position );
		break;

	case EV_PLAYER_TELEPORT_OUT:
		DEBUGNAME("EV_PLAYER_TELEPORT_OUT");
		FX_PlayerTeleportOut( cent, position );
		break;

	case EV_REPLICATOR:
		DEBUGNAME("EV_REPLICATOR");
		FX_Replicator( cent, position );
		break;

	case EV_DISINTEGRATION:
		{
			DEBUGNAME("EV_DISINTEGRATION");
			qboolean makeNotSolid = qfalse;
			int disintPW = es->eventParm;
			int disintEffect = 0;
			int disintLength = 0;
			qhandle_t disintSound1 = NULL;
			qhandle_t disintSound2 = NULL;
			qhandle_t disintSound3 = NULL;

			switch( disintPW )
			{
			case PW_REGEN:// sniper rifle
				if ( cent->gent->owner )
				{
					if ( cent->gent->owner->client->playerTeam != TEAM_PARASITE )
					{
						FX_Disruptor( position, 1300 );
					}
				}
				disintEffect = EF_DISINTEGRATION;//ef_
				disintSound1 = cgs.media.disintegrateSound;//with scream
				disintSound2 = cgs.media.disintegrate2Sound;//no scream
				disintSound3 = cgs.media.disintegrate3Sound;//with inhuman scream
				disintLength = 2000;
				makeNotSolid = qtrue;
				break;
			case PW_DISINT_1:// arc welder
				disintEffect = EF_DISINT_1;//ef_
				disintSound1 = NULL;//with scream
				disintSound2 = NULL;//no scream
				disintSound3 = NULL;//with inhuman scream
				disintLength = 4000;
				break;
			case PW_DISINT_2:// Quantum nuke
				disintEffect = EF_DISINT_2;//ef_
				disintSound1 = NULL;//with scream
				disintSound2 = NULL;//no scream
				disintSound3 = NULL;//with inhuman scream
				disintLength = 2000;
				makeNotSolid = qtrue;

				if ( cent->gent->owner )
				{
					if ( cent->gent->owner->client && cent->gent->owner->client->playerTeam == TEAM_BOTS )
					{
						cent->gent->owner->NPC->timeOfDeath = cg.time;
					}

					CG_EnergyGibs( cent->gent->owner->s.number, position );
					FX_QuantumColumns( position );
				}
				break;
			case PW_DISINT_3://???
				disintEffect = EF_DISINT_3;//ef_
				disintSound1 = cgs.media.disintegrateSound;//with scream
				disintSound2 = cgs.media.disintegrate2Sound;//no scream
				disintSound3 = cgs.media.disintegrate3Sound;//with inhuman scream
				disintLength = 2000;
				makeNotSolid = qtrue;
				break;
			case PW_DISINT_4://???
				disintEffect = EF_DISINT_4;//ef_
				disintSound1 = NULL;//with scream
				disintSound2 = NULL;//no scream
				disintSound3 = NULL;//with inhuman scream
				disintLength = 2000;
				break;
			case PW_DISINT_5://???
				disintEffect = EF_DISINT_5;//ef_
				disintSound1 = NULL;//with scream
				disintSound2 = NULL;//no scream
				disintSound3 = NULL;//with inhuman scream
				disintLength = 2000;
				break;
			case PW_DISINT_6://Not a disintegration, used for weapon select effect
				disintEffect = EF_DISINT_6;//ef_
				disintSound1 = NULL;//with scream
				disintSound2 = NULL;//no scream
				disintSound3 = NULL;//with inhuman scream
				disintLength = 2000;
				break;
			default:
				return;
				break;
			}

			if ( cent->gent->owner )
			{
				cent->gent->owner->fx_time = cg.time;
				if ( cent->gent->owner->client )
				{
					if ( disintSound1 && disintSound2 )
					{//play an extra sound
						if ( cent->gent->owner->client->playerTeam == TEAM_STARFLEET ||
								cent->gent->owner->client->playerTeam == TEAM_SCAVENGERS ||
								cent->gent->owner->client->playerTeam == TEAM_MALON ||
								cent->gent->owner->client->playerTeam == TEAM_IMPERIAL ||
								cent->gent->owner->client->playerTeam == TEAM_HIROGEN ||
								cent->gent->owner->client->playerTeam == TEAM_DISGUISE ||
								cent->gent->owner->client->playerTeam == TEAM_KLINGON )
						{//Only the humanoids scream
							cgi_S_StartSound ( NULL, cent->gent->owner->s.number, CHAN_VOICE, disintSound1 );
						}
						else if ( cent->gent->owner->client->playerTeam == TEAM_FORGE ||
								cent->gent->owner->client->playerTeam == TEAM_8472 )
						{
							cgi_S_StartSound ( NULL, cent->gent->s.number, CHAN_VOICE, disintSound3 );
						}
						else
						{
							cgi_S_StartSound ( NULL, cent->gent->s.number, CHAN_AUTO, disintSound2 );
						}
					}
					cent->gent->owner->s.powerups |= ( 1 << disintPW );
					cent->gent->owner->client->ps.powerups[disintPW] = cg.time + disintLength;

					// Things that are being disintegrated should probably not be solid...
					if ( makeNotSolid && cent->gent->owner->client->playerTeam != TEAM_BOTS )
					{
						cent->gent->contents = CONTENTS_NONE;
					}
				}
				else
				{
					cent->gent->owner->s.eFlags = disintEffect;//FIXME: |= ?
					cent->gent->owner->delay = cg.time + disintLength;
				}
			}
		}
		break;

	case EV_FORGE_FADE:
		DEBUGNAME("EV_FORGE_FADE");
		if ( cent->gent && cent->gent->client )
		{
			switch( es->eventParm )
			{
			case 0:
				cgi_S_StartSound( cent->lerpOrigin, cent->gent->s.number, CHAN_AUTO, cgs.media.forgeRemovalSound );
				cent->gent->s.powerups |= ( 1 << PW_FORGE_DEATH );
				cent->gent->client->ps.powerups[PW_FORGE_DEATH] = cg.time + 2000;
				break;

			case 1:
				FX_ForgeTeleportOut( position );
				break;

			default:
				FX_ForgeBeamIn( cent->lerpOrigin );
				break;
			}
		}
		break;

	case EV_SCAV_BEAMING:
		DEBUGNAME("EV_SCAV_BEAMING");
		cgi_S_StartSound ( NULL, es->number, CHAN_AUTO, cgs.media.scavBeamInSound );
		if ( cent->gent->owner )
		{
			cent->gent->owner->fx_time = cg.time;
			if ( cent->gent->owner->client )
			{
				cent->gent->owner->s.powerups |= ( 1 << PW_HASTE );
				cent->gent->owner->client->ps.powerups[PW_HASTE] = cg.time + 1000;
			}
			else
			{
				cent->gent->owner->s.eFlags = EF_SCAV_BEAM_OUT;
				cent->gent->owner->delay = cg.time + 1000;
			}
		}
		break;

	case EV_BORG_TELEPORT:
		DEBUGNAME("EV_BORG_TELEPORT");
		FX_BorgTeleport( position );
		break;

	case EV_BORG_RECYCLE:
		DEBUGNAME("EV_BORG_RECYCLE");
		FX_BorgRecycle( position );
		cgi_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.borgRecycleSound );
		break;

	case EV_STASIS_TELEPORT_IN:
		DEBUGNAME("EV_STASIS_TELEPORT_IN");
		FX_StasisBeamIn( position );
		if ( cent->gent->count )
			cgi_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.stasisAppearSound );
		else
			cgi_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.stasisBeamInSound );

		break;

	case EV_STASIS_TELEPORT_OUT:
		DEBUGNAME("EV_STASIS_TELEPORT_OUT");
		FX_StasisBeamOut( position );
		cgi_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.stasisBeamOutSound );
		break;

	case EV_SPECIES_TELEPORT:
		DEBUGNAME("EV_SPECIES_TELEPORT");
		FX_8472Teleport( position, es->eventParm );
//		cgi_S_StartSound (NULL, es->number, CHAN_AUTO, cgi_S_RegisterSound("sound/ambience/stasis/intostasis.wav") );
		break;

	// This does not necessarily have to be from a grenade...
	case EV_GRENADE_BOUNCE:
		DEBUGNAME("EV_GRENADE_BOUNCE");
		CG_BounceEffect( cent, es->weapon, position, cent->gent->pos1 );
		break;

	//
	// missile impacts
	//

	case EV_MISSILE_STICK:
		DEBUGNAME("EV_MISSILE_STICK");
		cgi_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.missileStick );
		break;

	case EV_MISSILE_HIT:
		DEBUGNAME("EV_MISSILE_HIT");
		ByteToDir( es->eventParm, dir );
		CG_MissileHitPlayer( cent, es->weapon, position, dir );
		break;

	case EV_MISSILE_MISS:
		DEBUGNAME("EV_MISSILE_MISS");
		ByteToDir( es->eventParm, dir );
		CG_MissileHitWall( cent, es->weapon, position, dir );
		break;

	case EV_BMODEL_SOUND:
		DEBUGNAME("EV_BMODEL_SOUND");
		cgi_S_StartSound( NULL, es->number, CHAN_AUTO, es->eventParm );
		break;

	case EV_GENERAL_SOUND:
		DEBUGNAME("EV_GENERAL_SOUND");
		if ( cgs.sound_precache[ es->eventParm ] ) 
		{
			cgi_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.sound_precache[ es->eventParm ] );
		} 
		else 
		{
			s = CG_ConfigString( CS_SOUNDS + es->eventParm );
			CG_TryPlayCustomSound(NULL, es->number, CHAN_AUTO, s, CS_BASIC );
		}
		break;

	case EV_GLOBAL_SOUND:	// play from the player's head so it never diminishes
		DEBUGNAME("EV_GLOBAL_SOUND");
		if ( cgs.sound_precache[ es->eventParm ] ) {
			cgi_S_StartSound (NULL, cg.snap->ps.clientNum, CHAN_AUTO, cgs.sound_precache[ es->eventParm ] );
		} else {
			s = CG_ConfigString( CS_SOUNDS + es->eventParm );
			CG_TryPlayCustomSound( NULL, cg.snap->ps.clientNum, CHAN_AUTO, s, CS_BASIC );
		}
		break;

	case EV_PAIN:
		{
		char	*snd;
		const int health = es->eventParm;

		DEBUGNAME("EV_PAIN");
		// don't do more than two pain sounds a second
		if ( cg.time - cent->pe.painTime < 500 ) {
			return;
		}

		if ( health < 25 ) {
			snd = "*pain25.wav";
		} else if ( health < 50 ) {
			snd = "*pain50.wav";
		} else if ( health < 75 ) {
			snd = "*pain75.wav";
		} else {
			snd = "*pain100.wav";
		}
		CG_TryPlayCustomSound( NULL, es->number, CHAN_VOICE, snd, CS_BASIC );

		// save pain time for programitic twitch animation
		cent->pe.painTime = cg.time;
		cent->pe.painDirection ^= 1;
		}
		break;

	case EV_DEATH1:
	case EV_DEATH2:
	case EV_DEATH3:
		DEBUGNAME("EV_DEATHx");
		CG_TryPlayCustomSound( NULL, es->number, CHAN_VOICE, va("*death%i.wav", event - EV_DEATH1 + 1), CS_BASIC );
		break;

	case EV_TETRION:
		DEBUGNAME("EV_TETRION");
		FX_TetrionShot( cent->currentState.origin2, cent->lerpOrigin, cent->gent->pos1 );
		break;

	case EV_TETRION_HIT:
		DEBUGNAME("EV_TETRION_HIT");
		ByteToDir( es->eventParm, dir );
		FX_TetrionWeaponHitWall( cent->lerpOrigin, dir );
		break;

	case EV_TETRION_MISS:
		DEBUGNAME("EV_TETRION_MISS");
		ByteToDir( es->eventParm, dir );
		FX_TetrionWeaponHitWall( cent->lerpOrigin, dir );
		break;

	case EV_DREADNOUGHT_HIT:
		DEBUGNAME("EV_DREADNOUGHT_HIT");
		ByteToDir( es->eventParm, dir );
		FX_TetrionWeaponHitWall( cent->lerpOrigin, dir );
		break;

	case EV_DREADNOUGHT_MISS:
		DEBUGNAME("EV_DREADNOUGHT_MISS");
		ByteToDir( es->eventParm, dir );
		FX_DreadnoughtShotMiss( cent->lerpOrigin, dir );
		break;

	case EV_PHASER:
		DEBUGNAME("EV_PHASER");
//		ByteToDir( es->eventParm, dir );
//		FX_PhaserHit( cent->lerpOrigin, dir );
		break;

	case EV_PHASER_MISS:
		break;

	case EV_PHASER_HIT:
		break;

	case EV_COMPRESSION_RIFLE:
		DEBUGNAME("EV_COMPRESSION_RIFLE");
		FX_CompressionShot( cent->lerpOrigin, cent->currentState.origin2 );
		break;

	case EV_COMPRESSION_RIFLE_ALT:
		DEBUGNAME("EV_COMPRESSION_RIFLE_ALT");
		FX_CompressionAltShot( cent->lerpOrigin, cent->currentState.origin2 );
		break;

	case EV_COMPRESSION_RIFLE_MISS:
		DEBUGNAME("EV_COMPRESSION_RIFLE_MISS");
		ByteToDir( es->eventParm, dir );
		FX_CompressionExplosion( cent->lerpOrigin, dir );
		break;

	case EV_COMPRESSION_RIFLE_HIT:
		DEBUGNAME("EV_COMPRESSION_RIFLE_HIT");
		FX_CompressionHit( cent->lerpOrigin );
		break;

	case EV_COMPRESSION_RIFLE_ALT_MISS:
		DEBUGNAME("EV_COMPRESSION_RIFLE_ALT_MISS");
		ByteToDir( es->eventParm, dir );
		FX_CompressionAltMiss( cent->lerpOrigin, dir );
		break;

	case EV_RIFLE:
		DEBUGNAME("EV_RIFLE");
		FX_RifleShot( cent->lerpOrigin, cent->currentState.origin2 );
		break;

	case EV_RIFLE_MISS:
		DEBUGNAME("EV_RIFLE_MISS");
		ByteToDir( es->eventParm, dir );
		FX_RifleHitWall( cent->lerpOrigin, dir );
		break;

	case EV_RIFLE_HIT:
		DEBUGNAME("EV_RIFLE_HIT");
		FX_RifleHitPlayer( cent->lerpOrigin );
		break;

	case EV_STASIS:
		DEBUGNAME("EV_STASIS");
		FX_StasisShot( cent, cent->lerpOrigin, cent->currentState.origin2 );
		break;

	case EV_STASIS_MISS:
		DEBUGNAME("EV_STASIS_MISS");
		ByteToDir( es->eventParm, dir );		
		FX_StasisShotMiss( cent->lerpOrigin, dir );
		break;

	case EV_STASIS_HIT:
		DEBUGNAME("EV_STASIS_HIT");
		ByteToDir( es->eventParm, dir );		
		FX_StasisShotImpact( cent->lerpOrigin, dir );
		break;

	case EV_IMOD:
		DEBUGNAME("EV_IMOD");
		ByteToDir( es->eventParm, dir );		
		FX_IMODShot( cent->lerpOrigin, cent->currentState.origin2, dir );
		break;

	case EV_IMOD_ALTFIRE:
		DEBUGNAME("EV_IMOD_ALTFIRE");
		ByteToDir( es->eventParm, dir );		
		FX_AltIMODShot( cent->lerpOrigin, cent->currentState.origin2, dir );
		break;
	
	case EV_IMOD_MISS:
		DEBUGNAME("EV_IMOD_MISS");
		ByteToDir( es->eventParm, dir );
		FX_IMODExplosion( cent->lerpOrigin, dir );
		break;

	case EV_IMOD_HIT:
		DEBUGNAME("EV_IMOD_HIT");
		ByteToDir( es->eventParm, dir );
		FX_IMODExplosion( cent->lerpOrigin, dir );
		break;

	case EV_IMOD_ALTFIRE_HIT:
		DEBUGNAME("EV_IMOD_ALTFIRE_HIT");
		ByteToDir( es->eventParm, dir );
		FX_AltIMODExplosion( cent->lerpOrigin, dir );
		break;

	case EV_IMOD_ALTFIRE_MISS:
		DEBUGNAME("EV_IMOD_ALTFIRE_MISS");
		ByteToDir( es->eventParm, dir );
		FX_AltIMODExplosion( cent->lerpOrigin, dir );
		break;

	case EV_GRENADE_EXPLODE:
		DEBUGNAME("EV_GRENADE_EXPLODE");
		FX_GrenadeExplode( cent->lerpOrigin, cent->currentState.origin2 );
		break;

	case EV_GRENADE_SHRAPNEL_EXPLODE:
		DEBUGNAME("EV_GRENADE_SHRAPNEL_EXPLODE");
		FX_GrenadeShrapnelExplode( cent->lerpOrigin, cent->gent->pos1 );		
		break;

	case EV_GRENADE_SHRAPNEL: 
		DEBUGNAME("EV_GRENADE_SHRAPNEL");
		FX_GrenadeShrapnelBits( cent->lerpOrigin, cent->currentState.origin2, cent->gent->pos1 );
		break;

	case EV_HYPO_PUFF:
		DEBUGNAME("EV_HYPO_PUFF");
		FX_HypoSpray( cent->lerpOrigin, cent->gent->pos1, es->eventParm );
		break;

	case EV_BOT_ROCKET_DIE:
		FX_BotRocketHitWall( cent->lerpOrigin, NULL );
		break;

	case EV_BOT_EXPLODE:
		FX_BotExplode( cent );
		break;

	case EV_PROTON_GUN:
		DEBUGNAME("EV_PROTON_GUN");
		FX_ProtonShot( cent->lerpOrigin, cent->currentState.origin2 );
		break;

	case EV_PROTON_GUN_ALT:
		DEBUGNAME("EV_PROTON_GUN_ALT");
		FX_ProtonAltShot( cent->lerpOrigin, cent->currentState.origin2 );
		break;

	case EV_PROTON_GUN_MISS:
		DEBUGNAME("EV_PROTON_GUN_MISS");
		ByteToDir( es->eventParm, dir );
		FX_ProtonExplosion( cent->lerpOrigin, dir );
		break;

	case EV_PROTON_GUN_HIT:
		DEBUGNAME("EV_PROTON_GUN_HIT");
		FX_ProtonHit( cent->lerpOrigin );
		break;

	case EV_PROTON_GUN_ALT_MISS:
		DEBUGNAME("EV_PROTON_GUN_ALT_MISS");
		ByteToDir( es->eventParm, dir );
		FX_ProtonAltMiss( cent->lerpOrigin, dir );
		break;

	// Environmental Events

	case EV_FX_STEAM:
		DEBUGNAME("EV_FX_STEAM");
		CG_Steam( cent->lerpOrigin, cent->currentState.origin2, cent->currentState.angles, cent->gent->radius, es->eventParm );
		break;

	case EV_FX_COOKING_STEAM:
		DEBUGNAME("EV_FX_COOKING_STEAM");
		CG_CookingSteam( cent->lerpOrigin, cent->gent->radius );
		break;

	case EV_FX_SPARK:
		DEBUGNAME("EV_FX_SPARK");
		cgi_S_StartSound (NULL, es->number, CHAN_BODY, cgi_S_RegisterSound ( va("sound/ambience/spark%d.wav", Q_irand(1, 6)) ) );
		CG_Spark( cent->lerpOrigin, cent->currentState.angles );
		break;

	case EV_FX_BOLT:
		DEBUGNAME("EV_FX_BOLT");
		CG_Bolt( cent );
		break;

	case EV_FX_FORGE_BOLT:
		DEBUGNAME("EV_FX_FORGE_BOLT");
		CG_ForgeBolt( cent );
		break;

	case EV_FX_DRIP:
		DEBUGNAME("EV_FX_DRIP");
		if ( cent->gent->spawnflags & 2 )
		{
			CG_Drip( cent->lerpOrigin, cent->gent->startRGBA, 
						cent->gent->radius * 2, cent->gent->nextthink, cgs.media.oilDropShader );
		}
		else
		{
			CG_Drip( cent->lerpOrigin, cent->gent->startRGBA, 
						cent->gent->radius * 2, cent->gent->nextthink, cgs.media.waterDropShader );
		}
		break;

	case EV_FX_PLASMA:
		DEBUGNAME("EV_FX_PLASMA");
		CG_Plasma( cent->lerpOrigin, cent->currentState.origin2, cent->gent->startRGBA, cent->gent->finalRGBA );		
		break;

	case EV_FX_ELECFIRE:
		DEBUGNAME("EV_FX_ELECFIRE");
		// Don't play this sound quite so much...
		if ( rand() & 1 )
		{
			cgi_S_StartSound (NULL, es->number, CHAN_BODY, cgi_S_RegisterSound ( va("sound/ambience/spark%d.wav", Q_irand(1,6)) ));
		}
		CG_ElectricFire( cent->lerpOrigin, cent->currentState.angles );
		break;

	case EV_FX_SURFACE_EXPLOSION:
		DEBUGNAME("EV_FX_SURFACE_EXPLOSION");
		// This is kind of cheap, but tom wanted a louder explosion...sigh.
		if ( cent->gent->spawnflags & 2 )
		{
			cgi_S_StartSound(cent->lerpOrigin, es->number, CHAN_VOICE, cgs.media.bigSurfExpSound );
		}
		else
		{
			cgi_S_StartSound( cent->lerpOrigin, es->number, CHAN_AUTO, cgs.media.surfaceExpSound[Q_irand(0,2)] );
		}

		CG_SurfaceExplosion( cent->lerpOrigin, cent->currentState.origin2, cent->gent->radius, cent->gent->speed,
								!(cent->gent->spawnflags & 1) );
		break;

	case EV_FX_ELECTRICAL_EXPLOSION:
		DEBUGNAME("EV_FX_ELECTRICAL_EXPLOSION");
		cgi_S_StartSound( cent->lerpOrigin, es->number, CHAN_AUTO, cgs.media.electricExpSound[Q_irand(0,2)] );
		CG_ElectricalExplosion( cent->lerpOrigin, cent->currentState.origin2, cent->gent->radius );
		break;

	case EV_FX_SMOKE:
		DEBUGNAME("EV_FX_SMOKE");
		VectorSubtract( cent->currentState.origin2, cent->lerpOrigin, dir );
		VectorNormalize( dir );
		CG_Smoke( cent->lerpOrigin, dir, cent->gent->radius, 24.0f, cgs.media.smokeShader, 8 );
		break;

	case EV_FX_TELEPORTER:
		DEBUGNAME("EV_FX_TELEPORTER");
		CG_Teleporter( cent );
		break;

	case EV_FX_STREAM:
		DEBUGNAME("EV_FX_STREAM");
		CG_ParticleStream( cent );
		break;

	case EV_FX_TRANSPORTER_STREAM:
		DEBUGNAME("EV_FX_TRANSPORTER_STREAM");
		CG_TransporterStream( cent );
		break;

	case EV_FX_LASER:
		DEBUGNAME("EV_FX_LASER");
		if ( cent->gent->activator )
		{
			CG_FireLaser( cent->lerpOrigin, cent->gent->pos1, cent->gent->pos2, cent->gent->startRGBA, qtrue );
		}
		else
		{
			CG_FireLaser( cent->lerpOrigin, cent->gent->pos1, cent->gent->pos2, cent->gent->startRGBA, qfalse );
		}
		break;

	case EV_FX_EXPLOSION_TRAIL:
		DEBUGNAME("EV_FX_EXPLOSION_TRAIL");
		CG_ExplosionTrail( cent );
		break;

	case EV_FX_BLOW_CHUNKS:
		DEBUGNAME("EV_FX_BLOW_CHUNKS");
		VectorSubtract( cent->currentState.origin2, cent->lerpOrigin, dir );
		VectorNormalize( dir );
		CG_Chunks( cent->gent->s.number, cent->lerpOrigin, dir, cent->gent->speed, cent->gent->count, cent->gent->material, 0, cent->gent->radius * 0.1f );
		break;

	case EV_FX_BORG_ENERGY_BEAM:
		DEBUGNAME("EV_FX_BORG_ENERGY_BEAM");
		CG_BorgEnergyBeam( cent );
		break;

	case EV_FX_SHIMMERY_THING:
		DEBUGNAME("EV_FX_SHIMMERY_THING");
		CG_ShimmeryThing( cent->lerpOrigin, cent->currentState.origin2, cent->gent->radius, cent->gent->spawnflags & 2 );
		break;

	case EV_FX_BORG_BOLT:
		DEBUGNAME("EV_FX_BORG_BOLT");
		CG_Borg_Bolt( cent );
		break;

	case EV_FX_STASIS_TELEPORTER:
		DEBUGNAME("EV_FX_STASIS_TELEPORTER");
		CG_StasisTeleporter( cent->lerpOrigin, cent->gent->radius, (cent->gent->spawnflags & 2) );
		break;

	case EV_FX_STASIS_MUSHROOM:
		DEBUGNAME("EV_FX_STASIS_MUSHROOM");
		CG_StasisMushrooms( cent->lerpOrigin, cent->gent->count );
		break;

	case EV_FX_DN_BEAM_GLOW:
		DEBUGNAME("EV_FX_DN_BEAM_GLOW");
		CG_DreadnoughtBeamGlow( cent->gent->pos1, cent->gent->pos2, cent->gent->radius );
		break;

	case EV_FX_MAGIC_SMOKE:
		DEBUGNAME("EV_FX_MAGIC_SMOKE");
		CG_MagicSmoke( cent->lerpOrigin );
		break;

	case EV_FX_GARDEN_FOUNTAIN_SPURT:
		DEBUGNAME("EV_FX_GARDEN_FOUNTAIN_SPURT");
		CG_FountainSpurt( cent->lerpOrigin );
		break;

	case EV_STASIS_MINE_EXPLODE:
		DEBUGNAME("EV_STASIS_MINE_EXPLODE");
		FX_StasisMineExplode( position );
		break;

	case EV_BTASER_HIT:
	case EV_BTASER_MISS:
		DEBUGNAME("EV_BTASER_X");
		if ( cent->gent && cent->gent->count == 1 )
		{//black & white version
			FX_BWTaser( cent->lerpOrigin, cent->currentState.origin2 );
		}
		else
		{
			FX_BorgTaser( cent->lerpOrigin, cent->currentState.origin2 );
		}
		break;

	case EV_GREET1:
	case EV_GREET2:
	case EV_GREET3:
		DEBUGNAME("EV_GREETx");
		CG_TryPlayCustomSound( NULL, es->number, CHAN_VOICE, va("*greeting%i.wav", event - EV_GREET1 + 1), CS_EXTRA );
		break;

	case EV_RESPOND1:
	case EV_RESPOND2:
	case EV_RESPOND3:
		DEBUGNAME("EV_RESPONDx");
		CG_TryPlayCustomSound( NULL, es->number, CHAN_VOICE, va("*response%i.wav", event - EV_RESPOND1 + 1), CS_EXTRA );
		break;

	case EV_BUSY1:
	case EV_BUSY2:
	case EV_BUSY3:
		DEBUGNAME("EV_BUSYx");
		CG_TryPlayCustomSound( NULL, es->number, CHAN_VOICE, va("*busy%i.wav", event - EV_BUSY1 + 1), CS_EXTRA );
		break;

	case EV_MISSION1:
	case EV_MISSION2:
	case EV_MISSION3:
		DEBUGNAME("EV_MISSIONx");
		CG_TryPlayCustomSound( NULL, es->number, CHAN_VOICE, va("*mission%i.wav", event - EV_MISSION1 + 1), CS_EXTRA );
		break;

	case EV_ANGER1:	//Say when acquire an enemy when didn't have one before
	case EV_ANGER2:
	case EV_ANGER3:
		DEBUGNAME("EV_ANGERx");
		CG_TryPlayCustomSound( NULL, es->number, CHAN_VOICE, va("*anger%i.wav", event - EV_ANGER1 + 1), CS_COMBAT );
		break;

	case EV_COMBAT1:	//Say when acquire a new enemy during combat
	case EV_COMBAT2:
	case EV_COMBAT3:
		DEBUGNAME("EV_COMBATx");
		CG_TryPlayCustomSound( NULL, es->number, CHAN_VOICE, va("*combat%i.wav", event - EV_COMBAT1 + 1), CS_COMBAT );
		break;

	case EV_VICTORY1:	//Say when killed an enemy
	case EV_VICTORY2:
	case EV_VICTORY3:
		DEBUGNAME("EV_VICTORYx");
		CG_TryPlayCustomSound( NULL, es->number, CHAN_VOICE, va("*victory%i.wav", event - EV_VICTORY1 + 1), CS_COMBAT );
		break;

	case EV_SETTLE1:	//Say when killed an enemy anc can't find anymore (battle over)
	case EV_SETTLE2:
	case EV_SETTLE3:
		DEBUGNAME("EV_SETTLEx");
		CG_TryPlayCustomSound( NULL, es->number, CHAN_VOICE, va("*settle%i.wav", event - EV_SETTLE1 + 1), CS_COMBAT );
		break;

	case EV_BLOCKED1:	//Say when someone in your way
	case EV_BLOCKED2:
	case EV_BLOCKED3:
		DEBUGNAME("EV_BLOCKEDx");
		CG_TryPlayCustomSound( NULL, es->number, CHAN_VOICE, va("*blocked%i.wav", event - EV_BLOCKED1 + 1), CS_EXTRA );
		break;

	case EV_FF_1A:	
	case EV_FF_1B:
	case EV_FF_1C:
	case EV_FF_2A:	
	case EV_FF_2B:
	case EV_FF_2C:
	case EV_FF_3A:	
	case EV_FF_3B:
	case EV_FF_3C:
		{
		char *ffnames[] =
		{
			"*ff_1a.wav",
			"*ff_1b.wav",
			"*ff_1c.wav",
			"*ff_2a.wav",
			"*ff_2b.wav",
			"*ff_2c.wav",
			"*ff_3a.wav",
			"*ff_3b.wav",
			"*ff_3c.wav"
		};
		DEBUGNAME("EV_FF_xx");

		CG_TryPlayCustomSound( NULL, es->number, CHAN_VOICE, ffnames[event-EV_FF_1A], CS_EXTRA );
		}
	break;

	case EV_CHATTER1:
	case EV_CHATTER2:
	case EV_CHATTER3:
	case EV_CHATTER4:
	case EV_CHATTER5:
	case EV_CHATTER6:
	case EV_CHATTER7:
	case EV_CHATTER8:
	case EV_CHATTER9:
	case EV_CHATTER10:
	case EV_CHATTER11:
	case EV_CHATTER12:
	case EV_CHATTER13:
	case EV_CHATTER14:
		DEBUGNAME("EV_CHATTER_xx");
		CG_TryPlayCustomSound( NULL, es->number, CHAN_VOICE, cg_customScavSoundNames[event-EV_CHATTER1], CS_SCAV );
		break;

	default:
		DEBUGNAME("UNKNOWN");
		CG_Error( "Unknown event: %i", event );
		break;
	}

}


/*
==============
CG_CheckEvents

==============
*/
void CG_CheckEvents( centity_t *cent ) {
	// check for event-only entities
	if ( cent->currentState.eType > ET_EVENTS ) {
		if ( cent->previousEvent ) {
			return;	// already fired
		}
		cent->previousEvent = 1;

		cent->currentState.event = cent->currentState.eType - ET_EVENTS;
	} else {
		// check for events riding with another entity
		if ( cent->currentState.event == cent->previousEvent ) {
			return;
		}
		cent->previousEvent = cent->currentState.event;
		if ( ( cent->currentState.event & ~EV_EVENT_BITS ) == 0 ) {
			return;
		}
	}

	// calculate the position at exactly the frame time
	EvaluateTrajectory( &cent->currentState.pos, cg.snap->serverTime, cent->lerpOrigin );
	CG_SetEntitySoundPosition( cent );

	CG_EntityEvent( cent, cent->lerpOrigin );
}

