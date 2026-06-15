#include "g_local.h"
#include "g_functions.h"
#include "g_infostrings.h"
#include "g_items.h"

extern void CrystalAmmoSettings(gentity_t *ent);
extern	cvar_t	*g_spskill;

/*

  Items are any object that a player can touch to gain some effect.

  Pickup will return the number of seconds until they should respawn.

  all items should pop when dropped in lava or slime

  Respawnable items don't actually go away when picked up, they are
  just made invisible and untouchable.  This allows them to ride
  movers and respawn apropriately.
*/

// Item Spawn flags
#define ITMSF_SUSPEND		1
#define ITMSF_TEAM			2
#define ITMSF_MONSTER		4
#define ITMSF_NOTSOLID		8
#define ITMSF_VERTICAL		16
#define ITMSF_INVISIBLE		32

//======================================================================

int Pickup_Holdable( gentity_t *ent, gentity_t *other ) {

	other->client->ps.stats[STAT_HOLDABLE_ITEM] = ent->item - bg_itemlist;

	return 60;
}


//======================================================================
int Add_Ammo2 (gentity_t *ent, int ammoType, int count)
{
	ent->client->ps.ammo[ammoType] += count;

	if ( ent->client->ps.ammo[ammoType] > ammoData[ammoType].max ) 
	{
		ent->client->ps.ammo[ammoType] = ammoData[ammoType].max;
		return qfalse;
	}
	return qtrue;
}

void Add_Ammo (gentity_t *ent, int weapon, int count)
{
	Add_Ammo2(ent,weaponData[weapon].ammoIndex,count);
}

int Pickup_Ammo (gentity_t *ent, gentity_t *other)
{
	int		quantity;

	if ( ent->count ) {
		quantity = ent->count;
	} else {
		quantity = ent->item->quantity;
	}

	Add_Ammo2 (other, ent->item->giTag, quantity);

	return 30;
}

//======================================================================


int Pickup_Weapon (gentity_t *ent, gentity_t *other) {
	int		quantity;

	if (ent->count) 
	{
		quantity = ent->count;
	} 
	else 
	{
		quantity = ent->item->quantity;
	}

	// The Scav rifle is a special case
	if ( ent->item->giTag == WP_SCAVENGER_RIFLE )
	{
		switch (g_spskill->integer) 
		{
		case 0:	//easy			
			quantity = 40;
			break;
		case 1:	//medium			
			quantity = 30;
			break;
		default:
		case 2:	//hard			
			quantity = 20;
			break;
		}
	}

	// dropped items are always picked up
	if (!quantity)
	{
		if ( ! (ent->flags & FL_DROPPED_ITEM ) ) {
			quantity = 50;
		}
	}

	// add the weapon
	other->client->ps.stats[STAT_WEAPONS] |= ( 1 << ent->item->giTag );

	// Give ammo
	Add_Ammo( other, ent->item->giTag, quantity );

	return 5;
}


//======================================================================

int ITM_AddHealth (gentity_t *ent, int count) 
{
	
	ent->health += count;

	if (ent->health > ent->client->ps.stats[STAT_MAX_HEALTH])	// Past max health
	{
		ent->health = ent->client->ps.stats[STAT_MAX_HEALTH];

		return qfalse;
	}

	return qtrue;

}

int Pickup_Health (gentity_t *ent, gentity_t *other) {
	int			max;
	int			quantity;

	max = other->client->ps.stats[STAT_MAX_HEALTH];

	if ( ent->count ) {
		quantity = ent->count;
	} else {
		quantity = ent->item->quantity;
	}

	other->health += quantity;

	if (other->health > max ) {
		other->health = max;
	}

	if ( ent->item->giTag == 100 ) {		// mega health respawns slow
		return 120;
	}

	return 30;
}

//======================================================================

int ITM_AddArmor (gentity_t *ent, int count) 
{
	
	ent->client->ps.stats[STAT_ARMOR] += count;

	if (ent->client->ps.stats[STAT_ARMOR] > ent->client->ps.stats[STAT_MAX_HEALTH]) 
	{
		ent->client->ps.stats[STAT_ARMOR] = ent->client->ps.stats[STAT_MAX_HEALTH];
		return qfalse;
	}

	return qtrue;
}


int Pickup_Armor( gentity_t *ent, gentity_t *other ) {
	other->client->ps.stats[STAT_ARMOR] += ent->item->quantity;
	if ( other->client->ps.stats[STAT_ARMOR] > other->client->ps.stats[STAT_MAX_HEALTH] ) {
		other->client->ps.stats[STAT_ARMOR] = other->client->ps.stats[STAT_MAX_HEALTH];
	}

	return 30;
}

//======================================================================

/*
===============
RespawnItem
===============
*/
void RespawnItem( gentity_t *ent ) {
}


/*
===============
Touch_Item
===============
*/
void Touch_Item (gentity_t *ent, gentity_t *other, trace_t *trace) {
	int			respawn;

	if (!other->client)
		return;
	if (other->health < 1)
		return;		// dead people can't pickup

	// Only monsters can pick it up
	if ((ent->spawnflags &  ITMSF_MONSTER) && (other->client->playerTeam == TEAM_STARFLEET))
	{
		return;
	}

	// Only star fleet can pick it up
	if ((ent->spawnflags &  ITMSF_TEAM) && (other->client->playerTeam != TEAM_STARFLEET))
	{
		return;
	}

	// Only player can pick it up
	if (!(ent->spawnflags &  ITMSF_TEAM) && !(ent->spawnflags &  ITMSF_MONSTER))
	{
		if (!other->s.number == 0)	// Not the player?
		{
			return;
		}
	}

	// the same pickup rules are used for client side and server side
	if ( !BG_CanItemBeGrabbed( &ent->s, &other->client->ps ) ) {
		return;
	}

	if (!ent->item) {		//not an item!
		gi.Printf( "Touch_Item: %s is not an item!\n", ent->classname);
		return;
	}
	// call the item-specific pickup function
	switch( ent->item->giType ) 
	{
	case IT_WEAPON:
		respawn = Pickup_Weapon(ent, other);
		break;
	case IT_AMMO:
		respawn = Pickup_Ammo(ent, other);
		break;
	case IT_ARMOR:
		respawn = Pickup_Armor(ent, other);
		break;
	case IT_HEALTH:
		respawn = Pickup_Health(ent, other);
		break;
	case IT_HOLDABLE:
		respawn = Pickup_Holdable(ent, other);
		break;
	default:
		return;
	}

	if ( !respawn ) 
	{
		return;
	}

	// play the normal pickup sound
	G_AddEvent( other, EV_ITEM_PICKUP, ent->s.modelindex );

	// fire item targets
	G_UseTargets (ent, other);

	// wait of -1 will not respawn
//	if ( ent->wait == -1 ) 
	{
		ent->svFlags |= SVF_NOCLIENT;
		ent->s.eFlags |= EF_NODRAW;
		ent->contents = 0;
		ent->unlinkAfterEvent = qtrue;
		return;
	}
}


//======================================================================

/*
================
LaunchItem

Spawns an item and tosses it forward
================
*/
gentity_t *LaunchItem( gitem_t *item, vec3_t origin, vec3_t velocity, char *target ) {
	gentity_t	*dropped;

	// Invariant: an ET_ITEM must resolve to a real item (modelindex >= 1).
	// Never launch a null/degenerate item (NULL or the null slot bg_itemlist[0]).
	if ( !item || (item - bg_itemlist) < 1 )
	{
		return NULL;
	}

	dropped = G_Spawn();

	dropped->s.eType = ET_ITEM;
	dropped->s.modelindex = item - bg_itemlist;	// store item number in modelindex
	dropped->s.modelindex2 = 1; // This is non-zero is it's a dropped item

	dropped->classname = item->classname;
	dropped->item = item;
	VectorSet (dropped->mins, -ITEM_RADIUS, -ITEM_RADIUS, -ITEM_RADIUS);
	VectorSet (dropped->maxs, ITEM_RADIUS, ITEM_RADIUS, ITEM_RADIUS);
	dropped->contents = CONTENTS_TRIGGER|CONTENTS_ITEM;//CONTENTS_TRIGGER;//not CONTENTS_BODY for dropped items, don't need to ID them

	if ( target && target[0] )
	{
		dropped->target = G_NewString( target );
	}
	else
	{
		// if not targeting something, auto-remove after 30 seconds
		dropped->e_ThinkFunc = thinkF_G_FreeEntity;
		dropped->nextthink = level.time + 30000;
	}

	dropped->e_TouchFunc = touchF_Touch_Item;

	G_SetOrigin( dropped, origin );
	dropped->s.pos.trType = TR_GRAVITY;
	dropped->s.pos.trTime = level.time;
	VectorCopy( velocity, dropped->s.pos.trDelta );

	dropped->s.eFlags |= EF_BOUNCE_HALF;

	dropped->flags = FL_DROPPED_ITEM;

	gi.linkentity (dropped);

	return dropped;
}

/*
================
Drop_Item

Spawns an item and tosses it forward
================
*/
gentity_t *Drop_Item( gentity_t *ent, gitem_t *item, float angle, qboolean copytarget ) {
	vec3_t	velocity;
	vec3_t	angles;

	VectorCopy( ent->s.apos.trBase, angles );
	angles[YAW] += angle;
	angles[PITCH] = 0;	// always forward

	AngleVectors( angles, velocity, NULL, NULL );
	VectorScale( velocity, 150, velocity );
	velocity[2] += 200 + crandom() * 50;
	
	if ( copytarget )
	{
		return LaunchItem( item, ent->s.pos.trBase, velocity, ent->opentarget );
	}
	else
	{
		return LaunchItem( item, ent->s.pos.trBase, velocity, NULL );
	}
}


/*
================
Use_Item

Respawn the item
================
*/
void Use_Item( gentity_t *ent, gentity_t *other, gentity_t *activator ) 
{
	if ( ent->spawnflags & 32 ) // invisible
	{
		// If it was invisible, first use makes it visible....
		ent->s.eFlags &= ~EF_NODRAW;
		ent->contents = CONTENTS_TRIGGER|CONTENTS_ITEM;

		ent->spawnflags &= ~32;
		return;
	}

	if (ent->behaviorSet[BSET_USE])
	{
		G_ActivateBehavior(ent,BSET_USE);
	}

	RespawnItem( ent );
}

//======================================================================

/*
================
FinishSpawningItem

Traces down to find where an item should rest, instead of letting them
free fall from their spawn points
================
*/
void FinishSpawningItem( gentity_t *ent ) {
	trace_t		tr;
	vec3_t		dest;
	gitem_t		*item;
	int			itemNum;

	// Invariant: an ET_ITEM must resolve to a real item (modelindex >= 1).
	// A null/degenerate item (ent->item == NULL or the null slot bg_itemlist[0])
	// must never become a live, touchable item - remove it before it gets a
	// touch func / CONTENTS_ITEM / trigger and is linked into the world.
	if ( !ent->item || (ent->item - bg_itemlist) < 1 )
	{
		G_FreeEntity( ent );
		return;
	}

	itemNum=1;
	for ( item = bg_itemlist + 1 ; item->classname ; item++,itemNum++) 
	{
		if (!strcmp(item->classname,ent->classname))
		{
			break;
		}
	}

	// Set bounding box for item
	VectorSet( ent->mins, item->mins[0],item->mins[1] ,item->mins[2]);
	VectorSet( ent->maxs, item->maxs[0],item->maxs[1] ,item->maxs[2]);

	if ((!ent->mins[0] && !ent->mins[1] && !ent->mins[2]) && 
		(!ent->maxs[0] && !ent->maxs[1] && !ent->maxs[2]))
	{
		VectorSet (ent->mins, -ITEM_RADIUS, -ITEM_RADIUS, -ITEM_RADIUS);
		VectorSet (ent->maxs, ITEM_RADIUS, ITEM_RADIUS, ITEM_RADIUS);
	}

	if ((item->quantity) && (item->giType == IT_AMMO))
	{
		ent->count = item->quantity;
	}

	// Set crystal ammo amount based on skill level
	if ((itemNum == ITM_AMMO_CRYSTAL_BORG) ||
		(itemNum == ITM_AMMO_CRYSTAL_DN) ||
		(itemNum == ITM_AMMO_CRYSTAL_FORGE) ||
		(itemNum == ITM_AMMO_CRYSTAL_SCAVENGER) ||
		(itemNum == ITM_AMMO_CRYSTAL_STASIS))
	{
		CrystalAmmoSettings(ent);
	}

	ent->s.eType = ET_ITEM;
	ent->s.modelindex = ent->item - bg_itemlist;		// store item number in modelindex
	ent->s.modelindex2 = 0; // zero indicates this isn't a dropped item

	ent->contents = CONTENTS_TRIGGER|CONTENTS_ITEM;//CONTENTS_BODY;//CONTENTS_TRIGGER|
	ent->e_TouchFunc = touchF_Touch_Item;
	// useing an item causes it to respawn
	ent->e_UseFunc = useF_Use_Item;

	// Hang in air?
	if ( ent->spawnflags & ITMSF_SUSPEND) 
	{
		// suspended
		G_SetOrigin( ent, ent->s.origin );
	} 
	else 
	{
		// drop to floor
		VectorSet( dest, ent->s.origin[0], ent->s.origin[1], ent->s.origin[2] - 4096 );
		gi.trace( &tr, ent->s.origin, ent->mins, ent->maxs, dest, ent->s.number, MASK_SOLID );
		if ( tr.startsolid ) 
		{
			gi.Printf (S_COLOR_RED"FinishSpawningItem: removing %s startsolid at %s\n", ent->classname, vtos(ent->s.origin));
			assert( 0 && "item starting in solid");
#ifndef FINAL_BUILD
			G_Error("FinishSpawningItem: removing %s startsolid at %s\n", ent->classname, vtos(ent->s.origin));
#endif
			G_FreeEntity( ent );
			return;
		}

		// allow to ride movers
		ent->s.groundEntityNum = tr.entityNum;

		G_SetOrigin( ent, tr.endpos );
	}

/* ? don't need this
	// team slaves and targeted items aren't present at start
	if ( ( ent->flags & FL_TEAMSLAVE ) || ent->targetname ) {
		ent->s.eFlags |= EF_NODRAW;
		ent->contents = 0;
		return;
	}
*/
	if ( ent->spawnflags & ITMSF_INVISIBLE ) // invisible
	{
		ent->s.eFlags |= EF_NODRAW;
		ent->contents = 0;
	}

	if ( ent->spawnflags & ITMSF_NOTSOLID ) // not solid
	{
		ent->contents = 0;
	}

	gi.linkentity (ent);
}


qboolean	itemRegistered[MAX_ITEMS];


/*
==============
ClearRegisteredItems
==============
*/
void ClearRegisteredItems( void ) {
	memset( itemRegistered, 0, sizeof( itemRegistered ) );

	// players always start with the base weapon
	RegisterItem( FindItemForWeapon( WP_PHASER ) );	//these are given in g_client, ClientSpawn()
	RegisterItem( FindItemForWeapon( WP_COMPRESSION_RIFLE ) );

extern void Player_CacheFromPrevLevel(void);//g_client.cpp
	Player_CacheFromPrevLevel();	//reads from transition carry-over;
}

/*
===============
RegisterItem

The item will be added to the precache list
===============
*/
void RegisterItem( gitem_t *item ) {
	if ( !item ) {
		G_Error( "RegisterItem: NULL" );
	}
	itemRegistered[ item - bg_itemlist ] = qtrue;
}


/*
===============
SaveRegisteredItems

Write the needed items to a config string
so the client will know which ones to precache
===============
*/
void SaveRegisteredItems( void ) {
	char	string[MAX_ITEMS+1];
	int		i;
	int		count;

	count = 0;
	for ( i = 0 ; i < bg_numItems ; i++ ) {
		if ( itemRegistered[i] ) {
			count++;
			string[i] = '1';
		} else {
			string[i] = '0';
		}
	}
	string[ bg_numItems ] = 0;

	gi.Printf( "%i items registered\n", count );
	gi.SetConfigstring(CS_ITEMS, string);
}


/*
============
G_SpawnItem

Sets the clipping size and plants the object on the floor.

Items can't be immediately dropped to floor, because they might
be on an entity that hasn't spawned yet.
============
*/
void G_SpawnItem (gentity_t *ent, gitem_t *item) {
	G_SpawnFloat( "random", "0", &ent->random );
	G_SpawnFloat( "wait", "0", &ent->wait );

	RegisterItem( item );
	ent->item = item;

	// some movers spawn on the second frame, so delay item
	// spawns until the third frame so they can ride trains
	ent->nextthink = level.time + FRAMETIME * 2;
	ent->e_ThinkFunc = thinkF_FinishSpawningItem;

	ent->physicsBounce = 0.50;		// items are bouncy

	// Set a default infoString text color
	VectorSet( ent->startRGBA, 0.9f, 0.7f, 0.0f );	// yellow
	ent->infoString = item->pickup_name;
}


/*
================
G_BounceItem

================
*/
void G_BounceItem( gentity_t *ent, trace_t *trace ) {
	vec3_t	velocity;
	float	dot;
	int		hitTime;

	// reflect the velocity on the trace plane
	hitTime = level.previousTime + ( level.time - level.previousTime ) * trace->fraction;
	EvaluateTrajectoryDelta( &ent->s.pos, hitTime, velocity );
	dot = DotProduct( velocity, trace->plane.normal );
	VectorMA( velocity, -2*dot, trace->plane.normal, ent->s.pos.trDelta );

	// cut the velocity to keep from bouncing forever
	VectorScale( ent->s.pos.trDelta, ent->physicsBounce, ent->s.pos.trDelta );

	// check for stop
	if ( trace->plane.normal[2] > 0 && ent->s.pos.trDelta[2] < 40 ) {
		G_SetOrigin( ent, trace->endpos );
		ent->s.groundEntityNum = trace->entityNum;
		return;
	}

	VectorAdd( ent->currentOrigin, trace->plane.normal, ent->currentOrigin);
	VectorCopy( ent->currentOrigin, ent->s.pos.trBase );
	ent->s.pos.trTime = level.time;
}


/*
================
G_RunItem

================
*/
void G_RunItem( gentity_t *ent ) {
	vec3_t		origin;
	trace_t		tr;
	int			contents;
	int			mask;

	// if groundentity has been set to -1, it may have been pushed off an edge
	if ( ent->s.groundEntityNum == -1 ) {
		if ( ent->s.pos.trType != TR_GRAVITY ) {
			ent->s.pos.trType = TR_GRAVITY;
			ent->s.pos.trTime = level.time;
		}
	}

	if ( ent->s.pos.trType == TR_STATIONARY ) {
		// check think function
		G_RunThink( ent );
		return;
	}

	// get current position
	EvaluateTrajectory( &ent->s.pos, level.time, origin );

	// trace a line from the previous position to the current position
	if ( ent->clipmask ) {
		mask = ent->clipmask;
	} else {
		mask = MASK_SOLID;
	}
	gi.trace( &tr, ent->currentOrigin, ent->mins, ent->maxs, origin, 
		ent->owner ? ent->owner->s.number : ENTITYNUM_NONE, mask );

	VectorCopy( tr.endpos, ent->currentOrigin );

	if ( tr.startsolid ) {
		tr.fraction = 0;
	}

	gi.linkentity( ent );	// FIXME: avoid this for stationary?

	// check think function
	G_RunThink( ent );

	if ( tr.fraction == 1 ) {
		return;
	}

	// if it is in a nodrop volume, remove it
	contents = gi.pointcontents( ent->currentOrigin, -1 );
	if ( contents & CONTENTS_NODROP ) 
	{
		G_FreeEntity( ent );
		return;
	}

	G_BounceItem( ent, &tr );
}

