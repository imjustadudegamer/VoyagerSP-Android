// Filename:-	g_functions.cpp
//
// This file contains the 8 (so far) function calls that replace the 8 function ptrs in the gentity_t structure

#include "g_local.h"
#include "../cgame/cg_local.h"
#include "g_functions.h"

void GEntity_ThinkFunc(gentity_t *self)
{
//#define THINKCASE(blah) case thinkF_ ## blah: blah(self); OutputDebugString(va("%s\n",#blah));break;
#define THINKCASE(blah) case thinkF_ ## blah: blah(self); break;

	switch (self->e_ThinkFunc)
	{
	case thinkF_NULL:																	
		break;

	THINKCASE( spark_think )
	THINKCASE( spark_link )
	THINKCASE( electric_fire_think )				
	THINKCASE( drip_think )					
	THINKCASE( steam_think )					
	THINKCASE( steam_link )					
	THINKCASE( bolt_think )					
	THINKCASE( bolt_link )					
	THINKCASE( plasma_think )				
	THINKCASE( plasma_link )				
	THINKCASE( smoke_think )				
	THINKCASE( teleporter_think )			
	THINKCASE( stream_think )				
	THINKCASE( pj_bolt_fire )				
	THINKCASE( pj_bolt_link )				
	THINKCASE( pj_bolt_think )				
	THINKCASE( funcBBrushDieGo )			
	THINKCASE( ExplodeDeath )				
	THINKCASE( RespawnItem )				
	THINKCASE( G_FreeEntity )				
	THINKCASE( FinishSpawningItem )			
	THINKCASE( locateCamera )				
	THINKCASE( G_RunObject )				
	THINKCASE( ReturnToPos1 )				
	THINKCASE( Use_BinaryMover_Go )			
	THINKCASE( Think_MatchTeam )			
	THINKCASE( Think_BeginMoving )			
	THINKCASE( Think_SetupTrainTargets )	
	THINKCASE( Think_SpawnNewDoorTrigger )	
	THINKCASE( ref_link )						
	THINKCASE( Think_Target_Delay )			
	THINKCASE( target_laser_think )			
	THINKCASE( target_laser_start )			
	THINKCASE( target_location_linkup )		
	THINKCASE( scriptrunner_run )			
	THINKCASE( multi_wait )					
	THINKCASE( multi_trigger_run )			
	THINKCASE( trigger_always_think )		
	THINKCASE( AimAtTarget )				
	THINKCASE( func_timer_think )			
	THINKCASE( NPC_RemoveBody )				
	THINKCASE( Disappear )					
	THINKCASE( NPC_Think )					
	THINKCASE( NPC_Spawn_Go )				
	THINKCASE( NPC_Begin )					
	THINKCASE( moverCallback )
	THINKCASE( anglerCallback )
	THINKCASE( RemoveOwner )				
	THINKCASE( MakeOwnerInvis )				
	THINKCASE( MakeOwnerEnergy )			
	THINKCASE( cooking_steam_think )			
	THINKCASE( laser_think )			
	THINKCASE( transporter_stream_think )			
	THINKCASE( func_usable_think )
	THINKCASE( misc_dlight_think )
	THINKCASE( smoke_link )
	THINKCASE( stream_link )
	THINKCASE( electrical_explosion_link )
	THINKCASE( surface_explosion_link )
	THINKCASE( transporter_stream_link )
	THINKCASE( laser_link )
	THINKCASE( explosion_trail_link )
	THINKCASE( blow_chunks_link )
	THINKCASE( borg_energy_beam_think )
	THINKCASE( borg_energy_beam_link )
	THINKCASE( shimmery_thing_think )
	THINKCASE( shimmery_thing_link )
	THINKCASE( health_think )
	THINKCASE( ammo_think )
	THINKCASE( borg_bolt_think )					
	THINKCASE( borg_bolt_link )	
	THINKCASE( mine_think )
	THINKCASE( mine_explode )
	THINKCASE( flier_follow_path )
	THINKCASE( flier_swoop_to_enemy ) 
	THINKCASE( flier_return_to_path )
	THINKCASE( trigger_teleporter_find_closest_portal )
	THINKCASE( grenadeExplode )
	THINKCASE( grenadeSpewShrapnel )
	THINKCASE( trigger_hurt_reset )
	THINKCASE( turret_base_think )
	THINKCASE( turret_head_think )
	THINKCASE( laser_arm_fire )
	THINKCASE( laser_arm_start )
	THINKCASE( trigger_visible_check_player_visibility )
	THINKCASE( target_relay_use_go )
	THINKCASE( trigger_cleared_fire )
	THINKCASE( forge_bolt_think )					
	THINKCASE( forge_bolt_link )					
	THINKCASE( stasis_teleporter_think )
	THINKCASE( stasis_shooter_active )
	THINKCASE( spawn_stasis_door_trig )
	THINKCASE( check_stasis_door_state )
	THINKCASE( flier_child )
	THINKCASE( stasis_mushroom_think )
	THINKCASE( spawn_stasis_control_trigger )
	THINKCASE( WP_QuantumAltThink )
	THINKCASE( WP_DreadnoughtBurstThink )
	THINKCASE( MoveOwner )
	THINKCASE( SolidifyOwner )
	THINKCASE( disable_stasis_door )
	THINKCASE( cycleCamera )
	THINKCASE( WaitNPCRespond )
	THINKCASE( bot_rocket_think )
	THINKCASE( spawn_ammo_crystal_trigger )
	THINKCASE( dn_beam_glow )
	THINKCASE( NPC_ShySpawn )
	THINKCASE( func_wait_return_solid )
	THINKCASE( InflateOwner )
	THINKCASE( mega_ammo_think )
	THINKCASE( tendril_think )
	THINKCASE( misc_replicator_item_finish_spawn )
	THINKCASE( garden_fountain_spurt )

	default:
		Com_Error(ERR_DROP, "GEntity_ThinkFunc: case %d not handled!\n",self->e_ThinkFunc);
		break;
	}
}

// note different switch-case code for CEntity as opposed to GEntity (CEntity goes through parent GEntity first)...
//
void CEntity_ThinkFunc(centity_s *cent)
{
//#define CLTHINKCASE(blah) case clThinkF_ ## blah: blah(cent); OutputDebugString(va("%s\n",#blah));break;
#define CLTHINKCASE(blah) case clThinkF_ ## blah: blah(cent); break;

	switch (cent->gent->e_clThinkFunc)
	{
	case clThinkF_NULL:
		break;

	CLTHINKCASE( CG_DLightThink )
	CLTHINKCASE( CG_StasisFixitsThink )
	CLTHINKCASE( CG_StasisFlierAttackThink )
	CLTHINKCASE( CG_StasisFlierIdleThink )
	CLTHINKCASE( CG_Assimilator )

	default:
		Com_Error(ERR_DROP, "CEntity_ThinkFunc: case %d not handled!\n",cent->gent->e_clThinkFunc);
		break;
	}
}


void GEntity_ReachedFunc(gentity_t *self)
{	
//#define REACHEDCASE(blah) case reachedF_ ## blah: blah(self); OutputDebugString(va("%s\n",#blah));break;
#define REACHEDCASE(blah) case reachedF_ ## blah: blah(self); break;

	switch (self->e_ReachedFunc)
	{	
	case reachedF_NULL:
		break;

	REACHEDCASE( Reached_BinaryMover )
	REACHEDCASE( Reached_Train )
	REACHEDCASE( moverCallback )
	REACHEDCASE( moveAndRotateCallback )

	default:
		Com_Error(ERR_DROP, "GEntity_ReachedFunc: case %d not handled!\n",self->e_ReachedFunc);
		break;
	}
}



void GEntity_BlockedFunc(gentity_t *self, gentity_t *other)
{
//#define BLOCKEDCASE(blah) case blockedF_ ## blah: blah(self,other); OutputDebugString(va("%s\n",#blah));break;
#define BLOCKEDCASE(blah) case blockedF_ ## blah: blah(self,other); break;

	switch (self->e_BlockedFunc)
	{
	case blockedF_NULL:
		break;

	BLOCKEDCASE( Blocked_Door )		
	BLOCKEDCASE( Blocked_Mover )		

	default:
		Com_Error(ERR_DROP, "GEntity_BlockedFunc: case %d not handled!\n",self->e_BlockedFunc);
		break;
	}
}

void GEntity_TouchFunc(gentity_t *self, gentity_t *other, trace_t *trace)
{
//#define TOUCHCASE(blah) case touchF_ ## blah: blah(self,other,trace); OutputDebugString(va("%s\n",#blah));break;
#define TOUCHCASE(blah) case touchF_ ## blah: blah(self,other,trace); break;

	switch (self->e_TouchFunc)
	{
	case touchF_NULL:
		break;

	TOUCHCASE( Touch_Item )
	TOUCHCASE( teleporter_touch )
	TOUCHCASE( charge_stick )
	TOUCHCASE( Touch_DoorTrigger )
	TOUCHCASE( Touch_PlatCenterTrigger )
	TOUCHCASE( Touch_Plat )
	TOUCHCASE( Touch_Button )
	TOUCHCASE( Touch_Multi )
	TOUCHCASE( trigger_push_touch )
	TOUCHCASE( trigger_teleporter_touch )
	TOUCHCASE( hurt_touch )
	TOUCHCASE( NPC_Touch )
	TOUCHCASE( mine_touch )
	TOUCHCASE( fixit_touch )
	TOUCHCASE( touch_stasis_door_trig )
	TOUCHCASE( flier_child_touch )
	TOUCHCASE( touch_stasis_control_trigger )
	TOUCHCASE( touch_ammo_crystal_tigger )
	TOUCHCASE( Touch_AssimilationTrigger )

	default:
		Com_Error(ERR_DROP, "GEntity_TouchFunc: case %d not handled!\n",self->e_TouchFunc);
	}
}

void GEntity_UseFunc(gentity_t *self, gentity_t *other, gentity_t *activator)
{
//#define USECASE(blah) case useF_ ## blah: blah(self,other,activator); OutputDebugString(va("%s\n",#blah));break;
#define USECASE(blah) case useF_ ## blah: blah(self,other,activator); break;

	switch (self->e_UseFunc)
	{
	case useF_NULL:
		break;

	USECASE( plasma_use )
	USECASE( surface_explosion_use )
	USECASE( electrical_explosion_use )
	USECASE( funcBBrushUse )
	USECASE( misc_model_use )
	USECASE( Use_Item )
	USECASE( Use_Shooter )
	USECASE( GoExplodeDeath )
	USECASE( Use_BinaryMover )
	USECASE( use_wall )
	USECASE( target_create_formation_use )
	USECASE( Use_Target_Give )
	USECASE( Use_Target_Delay )
	USECASE( Use_Target_Score )
	USECASE( Use_Target_Print )
	USECASE( Use_Target_Speaker )
	USECASE( target_laser_use )
	USECASE( target_relay_use )
	USECASE( target_kill_use )
	USECASE( target_counter_use )
	USECASE( target_random_use )
	USECASE( target_scriptrunner_use )
	USECASE( target_gravity_change_use )
	USECASE( target_friction_change_use )
	USECASE( target_teleporter_use )
	USECASE( Use_Multi )
	USECASE( Use_target_push )
	USECASE( hurt_use )
	USECASE( func_timer_use )
	USECASE( trigger_entdist_use )
	USECASE( func_usable_use )
	USECASE( target_activate_use )
	USECASE( target_deactivate_use )
	USECASE( NPC_Use )
	USECASE( NPC_Spawn )
	USECASE( laser_use )
	USECASE( smoke_use )
	USECASE( transporter_use )
	USECASE( teleporter_use )
	USECASE( spark_use )
	USECASE( bolt_use )
	USECASE( cooking_steam_use )
	USECASE( stream_use )
	USECASE( misc_dlight_use )
	USECASE( explosion_trail_use )
	USECASE( blow_chunks_use )
	USECASE( borg_energy_beam_use )
	USECASE( drip_use )
	USECASE( shimmery_thing_use )
	USECASE( health_use )
	USECASE( ammo_use )
	USECASE( mega_ammo_use )
	USECASE( target_level_change_use )
	USECASE( borg_bolt_use )
	USECASE( target_change_parm_use )
	USECASE( steam_use )
	USECASE( crew_beam_in_use )
	USECASE( turret_base_use )
	USECASE( laser_arm_use )
	USECASE( func_static_use )
	USECASE( pj_bolt_use )
	USECASE( forge_bolt_use )
	USECASE( stasis_control_switch_use )
	USECASE( target_play_music_use )
	USECASE( stasis_teleporter_use )
	USECASE( stasis_shooter_toggle )
	USECASE( use_stasis_door )
	USECASE( stasis_mushroom_use )
	USECASE( misc_model_useup )
	USECASE( misc_portal_use )
	USECASE( target_autosave_use )
	USECASE( dn_beam_use )
	USECASE( switch_models )
	USECASE( reaver_vat_use )
	USECASE( misc_replicator_item_spawn )
	USECASE( misc_replicator_item_remove )
	USECASE( target_interface_use )
	USECASE( func_bobbing_use )

	default:
		Com_Error(ERR_DROP, "GEntity_UseFunc: case %d not handled!\n",self->e_UseFunc);
	}
}

void GEntity_PainFunc(gentity_t *self, gentity_t *attacker, int damage)
{
//#define PAINCASE(blah) case painF_ ## blah: blah(self,attacker,damage); OutputDebugString(va("%s\n",#blah));break;
#define PAINCASE(blah) case painF_ ## blah: blah(self,attacker,damage); break;

	switch (self->e_PainFunc)
	{
	case painF_NULL:
		break;

	PAINCASE( funcBBrushPain )
	PAINCASE( misc_model_breakable_pain	)
	PAINCASE( NPC_Pain )
	PAINCASE( station_pain )
	PAINCASE( func_usable_pain )
	PAINCASE( flier_pain )
	PAINCASE( NPC_Scav_Pain )

	default:
		Com_Error(ERR_DROP, "GEntity_PainFunc: case %d not handled!\n",self->e_PainFunc);
	}
}


void GEntity_DieFunc(gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod)
{
//#define DIECASE(blah) case dieF_ ## blah: blah(self,inflictor,attacker,damage,mod); OutputDebugString(va("%s\n",#blah));break;
#define DIECASE(blah) case dieF_ ## blah: blah(self,inflictor,attacker,damage,mod); break;

	switch (self->e_DieFunc)
	{
	case painF_NULL:
		break;

	DIECASE( funcBBrushDie )
	DIECASE( misc_model_breakable_die )
	DIECASE( player_die )
	DIECASE( ExplodeDeath_Wait )
	DIECASE( func_usable_die )
	DIECASE( mine_die )
	DIECASE( flier_die )
	DIECASE( turret_die )
	DIECASE( bot_rocket_die )
	DIECASE( reaver_vat_die )
	
	default:
		Com_Error(ERR_DROP, "GEntity_DieFunc: case %d not handled!\n",self->e_DieFunc);
	}
}

//////////////////// eof /////////////////////

