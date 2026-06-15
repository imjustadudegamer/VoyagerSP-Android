// included in both game dll and client

#include "q_shared.h"
#include "g_local.h"
#include "bg_public.h"
#include "g_items.h"

extern weaponData_t weaponData[WP_NUM_WEAPONS];
extern ammoData_t ammoData[AMMO_MAX];


#define PICKUPSOUND "sound/weapons/w_pkup.wav"

/*QUAKED weapon_***** ( 0 0 0 ) (-16 -16 -16) (16 16 16) suspended
DO NOT USE THIS CLASS, IT JUST HOLDS GENERAL INFORMATION for weapons, ammo, and item pickups.
The suspended flag will allow items to hang in the air, otherwise they are dropped to the next surface.
The STARFLEET flag allows only starfleets to pick it up.
The MONSTER flag allows only NON-starfleets to pick it up.

If an item is the target of another entity, it will spawn as normal, use INVISIBLE to hide it.

An item fires all of its targets when it is picked up.  If the toucher can't carry it, the targets won't be fired.

"wait"	override the default wait before respawning.  -1 = never respawn automatically, which can be used with targeted spawning.
"random" random number of plus or minus seconds varied from the respawn time
"count" override quantity or duration on most items.
*/

/*QUAKED weapon_borg_weapon (.3 .3 1) (-16 -16 -2) (16 16 16) SUSPEND STARFLEET MONSTER NOTSOLID VERTICAL INVISIBLE
*/
/*QUAKED weapon_compression_rifle (.3 .3 1) (-16 -16 -2) (16 16 16) SUSPEND STARFLEET MONSTER NOTSOLID VERTICAL INVISIBLE
*/
/*QUAKED weapon_IMOD (.3 .3 1) (-16 -16 -4) (16 16 16) SUSPEND STARFLEET MONSTER NOTSOLID VERTICAL INVISIBLE
*/
/*QUAKED weapon_phaser (.3 .3 1) (-16 -16 -2) (16 16 16) SUSPEND STARFLEET MONSTER NOTSOLID VERTICAL INVISIBLE
*/
/*QUAKED weapon_scavenger_rifle (.3 .3 1) (-16 -16 -4) (16 16 16) SUSPEND STARFLEET MONSTER NOTSOLID VERTICAL INVISIBLE
*/
/*QUAKED weapon_stasis (.3 .3 1) (-16 -16 -4) (16 16 16) SUSPEND STARFLEET MONSTER NOTSOLID VERTICAL INVISIBLE
*/
/*QUAKED weapon_grenade_launcher (.3 .3 1) (-16 -16 -4) (16 16 16) SUSPEND STARFLEET MONSTER NOTSOLID VERTICAL INVISIBLE
*/
/*QUAKED weapon_tetrion_disruptor (.3 .3 1) (-16 -16 -2) (16 16 16) SUSPEND STARFLEET MONSTER NOTSOLID VERTICAL INVISIBLE
*/
/*QUAKED weapon_dreadnought (.3 .3 1) (-16 -16 -2) (16 16 16) SUSPEND STARFLEET MONSTER NOTSOLID VERTICAL INVISIBLE
*/
/*QUAKED weapon_quantum_burst (.3 .3 1) (-16 -16 -2) (16 16 16) SUSPEND STARFLEET MONSTER NOTSOLID VERTICAL INVISIBLE
*/
/*QUAKED weapon_tricorder (.3 .3 1) (-8 -8 -4) (8 8 0) SUSPEND STARFLEET MONSTER NOTSOLID VERTICAL INVISIBLE
*/
/*QUAKED weapon_voyager_hypo (.3 .3 1) (-8 -8 -4) (8 8 0) SUSPEND STARFLEET MONSTER NOTSOLID VERTICAL INVISIBLE
*/
/*QUAKED weapon_klingon_blade (.3 .3 1) (-8 -8 -4) (8 8 0) SUSPEND STARFLEET MONSTER NOTSOLID VERTICAL INVISIBLE
*/
/*QUAKED weapon_imperial_blade (.3 .3 1) (-8 -8 -4) (8 8 0) SUSPEND STARFLEET MONSTER NOTSOLID VERTICAL INVISIBLE
*/
/*QUAKED weapon_blue_hypo (.3 .3 .5) (-16 -16 -4) (16 16 16) SUSPEND STARFLEET MONSTER NOTSOLID
*/
/*QUAKED weapon_red_hypo (.3 .3 .5) (-16 -16 -4) (16 16 16) SUSPEND STARFLEET MONSTER NOTSOLID
*/
/*QUAKED weapon_melee (.3 .3 .5) (-16 -16 -2) (16 16 16) SUSPEND STARFLEET MONSTER NOTSOLID
*/
/*QUAKED weapon_proton_gun (.3 .3 1) (-16 -16 -2) (16 16 16) SUSPEND STARFLEET MONSTER NOTSOLID VERTICAL INVISIBLE
*/
/*QUAKED isodesium_pickup (.3 .3 1) (-10 -10 -5) (10 10 24) SUSPEND STARFLEET MONSTER NOTSOLID
*/
/*QUAKED starfleet_helmet_pickup (.3 .3 .8) (-10 -10 -3) (10 10 16) SUSPEND STARFLEET MONSTER NOTSOLID
*/


/*QUAKED ammo_crystal_borg (.3 .5 1) (-8 -8 -0) (8 8 16) SUSPEND STARFLEET MONSTER NOTSOLID
*/

/*QUAKED ammo_crystal_dn (.3 .5 1) (-8 -8 -0) (8 8 16) SUSPEND STARFLEET MONSTER NOTSOLID
*/

/*QUAKED ammo_crystal_forge (.3 .5 1) (-8 -8 -0) (8 8 16) SUSPEND STARFLEET MONSTER NOTSOLID
*/

/*QUAKED ammo_crystal_scavenger (.3 .5 1) (-8 -8 -0) (8 8 16) SUSPEND STARFLEET MONSTER NOTSOLID
*/

/*QUAKED ammo_crystal_stasis (.3 .5 1) (-8 -8 -0) (8 8 16) SUSPEND STARFLEET MONSTER NOTSOLID
*/


gitem_t	bg_itemlist[ITM_NUM_ITEMS+1];//need a null on the end

//int		bg_numItems = sizeof(bg_itemlist) / sizeof(bg_itemlist[0]) ;
const int		bg_numItems = ITM_NUM_ITEMS;


/*
==============
FindItemForHoldable
==============
*/
gitem_t	*FindItemForHoldable( holdable_t pw ) {
	int		i;

	for ( i = 1 ; i < bg_numItems ; i++ ) {
		if ( bg_itemlist[i].giType == IT_HOLDABLE && bg_itemlist[i].giTag == pw ) {
			return &bg_itemlist[i];
		}
	}

	Com_Error( ERR_DROP, "HoldableItem not found" );

	return NULL;
}


/*
===============
FindItemForWeapon

===============
*/
gitem_t	*FindItemForWeapon( weapon_t weapon ) {
	int		i;

	for ( i = 1 ; i < bg_numItems ; i++ ) {
		if ( bg_itemlist[i].giType == IT_WEAPON && bg_itemlist[i].giTag == weapon ) {
			return &bg_itemlist[i];
		}
	}

	Com_Error( ERR_DROP, "Couldn't find item for weapon %i", weapon);
	return NULL;
}

/*
===============
FindItem

===============
*/
gitem_t	*FindItem( const char *pickupName ) {
	int		i;

	for ( i = 1 ; i < bg_numItems ; i++ ) {
		if ( !Q_stricmp( bg_itemlist[i].pickup_name, pickupName ) )
			return &bg_itemlist[i];
	}

	return NULL;
}


/*
================
BG_CanItemBeGrabbed

Returns false if the item should not be picked up.
This needs to be the same for client side prediction and server use.
================
*/
qboolean	BG_CanItemBeGrabbed( const entityState_t *ent, const playerState_t *ps ) {
	gitem_t	*item;

	if ( ent->modelindex < 1 || ent->modelindex >= bg_numItems ) {
		// EF1 SP: a stray item entity with an out-of-range bg_itemlist index must NOT kill the whole
		// mission. Retail ERR_DROPs here as a sanity guard (it never triggers in retail), but the port
		// hits it on the High Noon holodeck spawn (the player touches a bad item on frame 1, exposed once
		// the level clock starts at 1000 so pickup is immediate) -> ERR_DROP -> kicked to the main menu.
		// Refuse the grab instead, and log the offending index so the root (the misindexed item) can be
		// traced. (Touch path only; an ungrabbable item is far better than a load-killing crash.)
		Com_Printf( S_COLOR_YELLOW "BG_CanItemBeGrabbed: item modelindex %d out of range [1,%d) -> not grabbable\n",
			ent->modelindex, bg_numItems );
		return qfalse;
	}

	item = &bg_itemlist[ent->modelindex];

	switch( item->giType ) {

	case IT_WEAPON:
		// See if we already have this weapon.
		if ( !(ps->stats[ STAT_WEAPONS ] & ( 1 << item->giTag ))) 
		{			
			// Don't have this weapon yet, so pick it up.
			return qtrue;
		}

		// Make sure that we aren't already full on ammo for this weapon
		if ( ps->ammo[weaponData[item->giTag].ammoIndex] >= ammoData[weaponData[item->giTag].ammoIndex].max )
		{
			// full, so don't grab the item
			return qfalse; 
		}

		return qtrue; // could use more of this type of ammo, so grab the item

	case IT_AMMO:
		if ( ps->ammo[ item->giTag ] >= ammoData[item->giTag].max )	// checkme			
		{
			return qfalse;		// can't hold any more
		}
		return qtrue;

	case IT_ARMOR:
		// we also clamp armor to the maxhealth for handicapping
		if ( ps->stats[STAT_ARMOR] >= ps->stats[STAT_MAX_HEALTH] ) {
			return qfalse;
		}
		return qtrue;

	case IT_HEALTH:
		// don't pick up if already at max
		if ( ps->stats[STAT_HEALTH] >= ps->stats[STAT_MAX_HEALTH] ) {
			return qfalse;
		}
		return qtrue;

	case IT_HOLDABLE:
		return qtrue;
	}

	return qfalse;
}

//======================================================================

/*
================
EvaluateTrajectory

================
*/
void EvaluateTrajectory( const trajectory_t *tr, int atTime, vec3_t result ) {
	float		deltaTime;
	float		phase;

	switch( tr->trType ) {
	case TR_STATIONARY:
	case TR_INTERPOLATE:
		VectorCopy( tr->trBase, result );
		break;
	case TR_LINEAR:
		deltaTime = ( atTime - tr->trTime ) * 0.001F;	// milliseconds to seconds
		VectorMA( tr->trBase, deltaTime, tr->trDelta, result );
		break;
	case TR_SINE:
		deltaTime = ( atTime - tr->trTime ) / (float) tr->trDuration;
		phase = (float)sin( deltaTime * M_PI * 2 );
		VectorMA( tr->trBase, phase, tr->trDelta, result );
		break;
	case TR_LINEAR_STOP:
		if ( atTime > tr->trTime + tr->trDuration ) {
			atTime = tr->trTime + tr->trDuration;
		}
		deltaTime = ( atTime - tr->trTime ) * 0.001F;	// milliseconds to seconds
		if ( deltaTime < 0 ) {//going past the total duration
			deltaTime = 0;
		}
		VectorMA( tr->trBase, deltaTime, tr->trDelta, result );
		break;
	case TR_GRAVITY:
		deltaTime = ( atTime - tr->trTime ) * 0.001F;	// milliseconds to seconds
		VectorMA( tr->trBase, deltaTime, tr->trDelta, result );
		result[2] -= 0.5F * DEFAULT_GRAVITY * deltaTime * deltaTime;		// FIXME: local gravity...
		break;
	default:
		Com_Error( ERR_DROP, "EvaluateTrajectory: unknown trType: %i", tr->trTime );
		break;
	}
}

/*
================
EvaluateTrajectoryDelta

================
*/
void EvaluateTrajectoryDelta( const trajectory_t *tr, int atTime, vec3_t result ) {
	float	deltaTime;
	float	phase;

	switch( tr->trType ) {
	case TR_STATIONARY:
	case TR_INTERPOLATE:
		VectorClear( result );
		break;
	case TR_LINEAR:
		VectorCopy( tr->trDelta, result );
		break;
	case TR_SINE:
		deltaTime = ( atTime - tr->trTime ) / (float) tr->trDuration;
		phase = (float)cos( deltaTime * M_PI * 2 );	// derivative of sin = cos
		phase *= 0.5;
		VectorScale( tr->trDelta, phase, result );
		break;
	case TR_LINEAR_STOP:
		if ( atTime > tr->trTime + tr->trDuration ) {
			VectorClear( result );
			return;
		}
		VectorCopy( tr->trDelta, result );
		break;
	case TR_GRAVITY:
		deltaTime = ( atTime - tr->trTime ) * 0.001F;	// milliseconds to seconds
		VectorCopy( tr->trDelta, result );
		result[2] -= DEFAULT_GRAVITY * deltaTime;		// FIXME: local gravity...
		break;
	default:
		Com_Error( ERR_DROP, "EvaluateTrajectoryDelta: unknown trType: %i", tr->trTime );
		break;
	}
}

/*
===============
AddEventToPlayerstate

Handles the sequence numbers
===============
*/
void AddEventToPlayerstate( int newEvent, int eventParm, playerState_t *ps ) {
	ps->events[ps->eventSequence & (MAX_PS_EVENTS-1)] = newEvent;
	ps->eventParms[ps->eventSequence & (MAX_PS_EVENTS-1)] = eventParm;
	ps->eventSequence++;
}


/*
===============
CurrentPlayerstateEvent

===============
*/
int	CurrentPlayerstateEvent( playerState_t *ps ) {
	return ps->events[ (ps->eventSequence-1) & (MAX_PS_EVENTS-1) ];
}

/*
========================
PlayerStateToEntityState

This is done after each set of usercmd_t on the server,
and after local prediction on the client
========================
*/
void PlayerStateToEntityState( playerState_t *ps, entityState_t *s ) {
	int		i;

	if ( ps->pm_type == PM_INTERMISSION || ps->pm_type == PM_SPECTATOR ) 
	{
		s->eType = ET_INVISIBLE;
	} 
	/*else if ( ps->stats[STAT_HEALTH] <= GIB_HEALTH ) 
	{
		s->eType = ET_INVISIBLE;
	} */
	else
	{
		s->eType = ET_PLAYER;
	}

	s->number = ps->clientNum;

	s->pos.trType = TR_INTERPOLATE;
	VectorCopy( ps->origin, s->pos.trBase );
	SnapVector( s->pos.trBase );

	s->apos.trType = TR_INTERPOLATE;
	VectorCopy( ps->viewangles, s->apos.trBase );
	SnapVector( s->apos.trBase );

	s->angles2[YAW] = ps->movementDir;
	s->legsAnim = ps->legsAnim;
	s->torsoAnim = ps->torsoAnim;
	s->clientNum = ps->clientNum;		// ET_PLAYER looks here instead of at number
										// so corpses can also reference the proper config
	s->eFlags = ps->eFlags;
	if ( ps->stats[STAT_HEALTH] <= 0 ) {
		s->eFlags |= EF_DEAD;
	} else {
		s->eFlags &= ~EF_DEAD;
	}

	s->weapon = ps->weapon;
	s->groundEntityNum = ps->groundEntityNum;

	s->powerups = 0;
	for ( i = 0 ; i < MAX_POWERUPS ; i++ ) {
		if ( ps->powerups[ i ] ) {
			s->powerups |= 1 << i;
		}
	}
#if 0
	if ( ps->externalEvent ) {
		s->event = ps->externalEvent;
		s->eventParm = ps->externalEventParm;
	} else {
		int		seq;

		seq = (ps->eventSequence-1) & (MAX_PS_EVENTS-1);
		s->event = ps->events[ seq ] | ( ( ps->eventSequence & 3 ) << 8 );
		s->eventParm = ps->eventParms[ seq ];
	}

	// show some roll in the body based on velocity and angle
	if ( ps->stats[STAT_HEALTH] > 0 ) {
		vec3_t		right;
		float		sign;
		float		side;
		float		value;

		AngleVectors( ps->viewangles, NULL, right, NULL );

		side = DotProduct (ps->velocity, right);
		sign = side < 0 ? -1 : 1;
		side = fabs(side);
		
		value = 2;	// g_rollangle->value;

		if (side < 200 /* g_rollspeed->value */ )
			side = side * value / 200; // g_rollspeed->value;
		else
			side = value;
		
		s->angles[ROLL] = (int)(side*sign * 4);
	}
#endif
}


/*
============
BG_PlayerTouchesItem

Items can be picked up without actually touching their physical bounds
============
*/
qboolean	BG_PlayerTouchesItem( playerState_t *ps, entityState_t *item, int atTime ) {
	vec3_t		origin;

	EvaluateTrajectory( &item->pos, atTime, origin );

	// we are ignoring ducked differences here
	if ( ps->origin[0] - origin[0] > 44
		|| ps->origin[0] - origin[0] < -50
		|| ps->origin[1] - origin[1] > 36
		|| ps->origin[1] - origin[1] < -36
		|| ps->origin[2] - origin[2] > 36
		|| ps->origin[2] - origin[2] < -36 ) {
		return qfalse;
	}

	return qtrue;
}


