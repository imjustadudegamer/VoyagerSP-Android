// Filename:-	g_functions.h
//

#ifndef G_FUNCTIONS
#define G_FUNCTIONS

#undef thinkFunc_t
#undef clThinkFunc_t
#undef reachedFunc_t
#undef blockedFunc_t
#undef touchFunc_t
#undef useFunc_t
#undef painFunc_t
#undef dieFunc_t

//	void		(*think)(gentity_t *self);
typedef enum 
{
	thinkF_NULL = 0,
	//
	thinkF_spark_think,
	thinkF_spark_link,
	thinkF_electric_fire_think,
	thinkF_drip_think,
	thinkF_steam_think,
	thinkF_steam_link,
	thinkF_bolt_think,
	thinkF_bolt_link,
	thinkF_plasma_think,
	thinkF_plasma_link,
	thinkF_smoke_think,
	thinkF_teleporter_think,
	thinkF_stream_think,
	thinkF_pj_bolt_fire,
	thinkF_pj_bolt_link,
	thinkF_pj_bolt_think,
	thinkF_funcBBrushDieGo,
	thinkF_ExplodeDeath,
	thinkF_RespawnItem,
	thinkF_G_FreeEntity,
	thinkF_FinishSpawningItem,	
	thinkF_locateCamera,
	thinkF_G_RunObject,
	thinkF_ReturnToPos1,
	thinkF_Use_BinaryMover_Go,
	thinkF_Think_MatchTeam,
	thinkF_Think_BeginMoving,
	thinkF_Think_SetupTrainTargets,
	thinkF_Think_SpawnNewDoorTrigger,
	thinkF_ref_link,
	thinkF_Think_Target_Delay,
	thinkF_target_laser_think,
	thinkF_target_laser_start,
	thinkF_target_location_linkup,
	thinkF_scriptrunner_run,
	thinkF_multi_wait,
	thinkF_multi_trigger_run,
	thinkF_trigger_always_think,
	thinkF_AimAtTarget,
	thinkF_func_timer_think,
	thinkF_NPC_RemoveBody,
	thinkF_Disappear,
	thinkF_NPC_Think,
	thinkF_NPC_Spawn_Go,
	thinkF_NPC_Begin,
	thinkF_moverCallback,
	thinkF_anglerCallback,
	thinkF_RemoveOwner,
	thinkF_MakeOwnerInvis,
	thinkF_MakeOwnerEnergy,
	thinkF_cooking_steam_think,
	thinkF_laser_think,
	thinkF_transporter_stream_think,
	thinkF_func_usable_think,
	thinkF_misc_dlight_think,
	thinkF_smoke_link,
	thinkF_stream_link,
	thinkF_electrical_explosion_link,
	thinkF_surface_explosion_link,
	thinkF_transporter_stream_link,
	thinkF_laser_link,
	thinkF_explosion_trail_link,
	thinkF_blow_chunks_link,
	thinkF_borg_energy_beam_link,
	thinkF_borg_energy_beam_think,
	thinkF_shimmery_thing_link,
	thinkF_shimmery_thing_think,
	thinkF_health_think,
	thinkF_ammo_think,
	thinkF_borg_bolt_think,
	thinkF_borg_bolt_link,
	thinkF_mine_think,
	thinkF_mine_explode,
	thinkF_flier_swoop_to_enemy,
	thinkF_flier_return_to_path,
	thinkF_flier_follow_path,
	thinkF_trigger_teleporter_find_closest_portal,
	thinkF_grenadeExplode,
	thinkF_grenadeSpewShrapnel,
	thinkF_trigger_hurt_reset,
	thinkF_turret_base_think,
	thinkF_turret_head_think,
	thinkF_HS_Think,
	thinkF_laser_arm_fire,
	thinkF_laser_arm_start,
	thinkF_trigger_visible_check_player_visibility,
	thinkF_target_relay_use_go,
	thinkF_trigger_cleared_fire,
	thinkF_forge_bolt_think,
	thinkF_forge_bolt_link,
	thinkF_stasis_teleporter_think,
	thinkF_stasis_shooter_active,
	thinkF_spawn_stasis_door_trig,
	thinkF_check_stasis_door_state,
	thinkF_flier_child,
	thinkF_stasis_mushroom_think,
	thinkF_spawn_stasis_control_trigger,
	thinkF_WP_QuantumAltThink,
	thinkF_WP_DreadnoughtBurstThink,
	thinkF_MoveOwner,
	thinkF_SolidifyOwner,
	thinkF_disable_stasis_door,
	thinkF_cycleCamera,
	thinkF_WaitNPCRespond,
	thinkF_bot_rocket_think,
	thinkF_spawn_ammo_crystal_trigger,
	thinkF_dn_beam_glow,
	thinkF_NPC_ShySpawn,
	thinkF_func_wait_return_solid,
	thinkF_InflateOwner,
	thinkF_mega_ammo_think,
	thinkF_tendril_think,
	thinkF_misc_replicator_item_finish_spawn,
	thinkF_garden_fountain_spurt

} thinkFunc_t;

// THINK functions...
//
extern void spark_think			( gentity_t *ent );
extern void spark_link			( gentity_t *ent );
extern void electric_fire_think	( gentity_t *ent );
extern void drip_think			( gentity_t *ent );
extern void steam_think			( gentity_t *ent );
extern void steam_link			( gentity_t *ent );
extern void bolt_think			( gentity_t *ent );
extern void bolt_link			( gentity_t *ent );
extern void plasma_think		( gentity_t *ent );
extern void plasma_link			( gentity_t *ent );
extern void smoke_think			( gentity_t *ent );
extern void teleporter_think	( gentity_t *ent );
extern void stream_think		( gentity_t *ent );
extern void pj_bolt_fire		( gentity_t *ent );
extern void pj_bolt_think		( gentity_t *ent );
extern void pj_bolt_link		( gentity_t *ent );
extern void funcBBrushDieGo		( gentity_t *ent );
extern void ExplodeDeath		( gentity_t *ent );
extern void RespawnItem			( gentity_t *ent );
extern void G_FreeEntity		( gentity_t *ent );
extern void FinishSpawningItem	( gentity_t *ent );
extern void locateCamera		( gentity_t *ent );
extern void ExplodeDeath		( gentity_t *ent );
extern void G_RunObject			( gentity_t *ent );
extern void ReturnToPos1		( gentity_t *ent );
extern void Use_BinaryMover_Go	( gentity_t *ent );
extern void Think_MatchTeam		( gentity_t *ent );
extern void Think_MatchTeam		( gentity_t *ent );
extern void Think_BeginMoving	( gentity_t *ent );
extern void Think_SetupTrainTargets	( gentity_t *ent );
extern void Think_SpawnNewDoorTrigger	( gentity_t *ent );
extern void ref_link			( gentity_t *ent );
extern void Think_Target_Delay	( gentity_t *ent );
extern void target_laser_think	( gentity_t *ent );
extern void target_laser_start	( gentity_t *ent );
extern void target_location_linkup	( gentity_t *ent );
extern void scriptrunner_run	( gentity_t *ent );
extern void multi_wait			( gentity_t *ent );
extern void multi_trigger_run	( gentity_t *ent );
extern void trigger_always_think	( gentity_t *ent );
extern void AimAtTarget			( gentity_t *ent );
extern void func_timer_think	( gentity_t *ent );
extern void ExplodeDeath		( gentity_t *ent );
extern void NPC_RemoveBody		( gentity_t *ent );
extern void Disappear			( gentity_t *ent );
extern void NPC_Think			( gentity_t *ent );
extern void NPC_Spawn_Go		( gentity_t *ent );
extern void NPC_Begin			( gentity_t *ent );
extern void moverCallback		( gentity_t *ent );
extern void anglerCallback		( gentity_t *ent );
extern void RemoveOwner			( gentity_t *ent );
extern void MakeOwnerInvis		( gentity_t *ent );
extern void MakeOwnerEnergy		( gentity_t *ent );
extern void cooking_steam_think	( gentity_t *ent );
extern void laser_think			( gentity_t *ent );
extern void transporter_stream_think ( gentity_t *ent ); 
extern void func_usable_think	( gentity_t *self );
extern void misc_dlight_think	( gentity_t *ent ); 
extern void smoke_link			( gentity_t *ent );
extern void stream_link			( gentity_t *ent );
extern void electrical_explosion_link	( gentity_t *ent );
extern void surface_explosion_link	( gentity_t *ent );
extern void transporter_stream_link	( gentity_t *ent );
extern void laser_link				( gentity_t *ent );
extern void explosion_trail_link	( gentity_t *ent );
extern void blow_chunks_link		( gentity_t *ent );
extern void borg_energy_beam_think	( gentity_t *ent );
extern void borg_energy_beam_link	( gentity_t *ent );
extern void shimmery_thing_think	( gentity_t *ent );
extern void shimmery_thing_link		( gentity_t *ent );
extern void health_think			( gentity_t *ent );
extern void ammo_think				( gentity_t *ent );
extern void borg_bolt_think			( gentity_t *ent );
extern void borg_bolt_link			( gentity_t *ent );
extern void mine_think				( gentity_t *self );
extern void mine_explode			( gentity_t *self );
extern void flier_follow_path		( gentity_t *self );
extern void flier_swoop_to_enemy	( gentity_t *self );
extern void flier_return_to_path	( gentity_t *self );
extern void trigger_teleporter_find_closest_portal ( gentity_t *self );
extern void grenadeExplode			( gentity_t *ent );
extern void grenadeSpewShrapnel		( gentity_t *ent );
extern void trigger_hurt_reset		( gentity_t *self );
extern void turret_base_think		( gentity_t *self );
extern void turret_head_think		( gentity_t *self );
extern void laser_arm_fire			( gentity_t *ent );
extern void laser_arm_start			( gentity_t *base );
extern void trigger_visible_check_player_visibility	( gentity_t *self );
extern void target_relay_use_go		( gentity_t *self );
extern void trigger_cleared_fire	( gentity_t *self );
extern void forge_bolt_think		( gentity_t *ent );
extern void forge_bolt_link			( gentity_t *ent );
extern void stasis_teleporter_think	( gentity_t *ent );
extern void stasis_shooter_active	( gentity_t *self );
extern void spawn_stasis_door_trig	( gentity_t *self );
extern void check_stasis_door_state	( gentity_t *self );
extern void flier_child				( gentity_t *self );
extern void stasis_mushroom_think	( gentity_t *ent );
extern void spawn_stasis_control_trigger	( gentity_t *ent );
extern void WP_QuantumAltThink		( gentity_t *ent );
extern void WP_DreadnoughtBurstThink( gentity_t *ent );
extern void MoveOwner				( gentity_t *self );
extern void SolidifyOwner			( gentity_t *self );
extern void disable_stasis_door		( gentity_t *self );
extern void cycleCamera				( gentity_t *self );
extern void WaitNPCRespond			( gentity_t *self );
extern void bot_rocket_think		( gentity_t *ent );
extern void spawn_ammo_crystal_trigger	( gentity_t *ent );
extern void dn_beam_glow			( gentity_t *ent );
extern void NPC_ShySpawn			( gentity_t *ent );
extern void func_wait_return_solid	( gentity_t *self );
extern void InflateOwner			( gentity_t *self );
extern void mega_ammo_think			( gentity_t *self );
extern void tendril_think			( gentity_t *self );
extern void misc_replicator_item_finish_spawn( gentity_t *self );
extern void garden_fountain_spurt	( gentity_t *self );

//	void		(*clThink)(centity_s *cent);	//Think func for equivalent centity
typedef enum
{
	clThinkF_NULL = 0,	
	//
	clThinkF_CG_DLightThink,
	clThinkF_CG_StasisFixitsThink,
	clThinkF_CG_StasisFlierAttackThink,
	clThinkF_CG_StasisFlierIdleThink,
	clThinkF_CG_Assimilator,

} clThinkFunc_t;

// CEntity THINK functions...
//
extern void CG_DLightThink ( centity_t *cent );
extern void CG_StasisFixitsThink ( centity_t *cent );
extern void CG_StasisFlierAttackThink ( centity_t *cent );
extern void CG_StasisFlierIdleThink ( centity_t *cent );
extern void CG_Assimilator ( centity_t *cent );



//	void		(*reached)(gentity_t *self);	// movers call this when hitting endpoint
typedef enum
{
	reachedF_NULL = 0,
	//
	reachedF_Reached_BinaryMover,
	reachedF_Reached_Train,
	reachedF_moverCallback,
	reachedF_moveAndRotateCallback

} reachedFunc_t;

// REACHED functions...
//
extern void Reached_BinaryMover	( gentity_t *ent );
extern void Reached_Train		( gentity_t *ent );
extern void moverCallback		( gentity_t *ent );
extern void moveAndRotateCallback( gentity_t *ent );


//	void		(*blocked)(gentity_t *self, gentity_t *other);
typedef enum
{
	blockedF_NULL = 0,
	//
	blockedF_Blocked_Door,
	blockedF_Blocked_Mover	

} blockedFunc_t;

// BLOCKED functions...
//
extern void Blocked_Door		(gentity_t *self, gentity_t *other);
extern void Blocked_Mover		(gentity_t *self, gentity_t *other);



//	void		(*touch)(gentity_t *self, gentity_t *other, trace_t *trace);
typedef enum
{	
	touchF_NULL = 0,
	//
	touchF_Touch_Item,
	touchF_teleporter_touch,
	touchF_charge_stick,
	touchF_Touch_DoorTrigger,
	touchF_Touch_PlatCenterTrigger,
	touchF_Touch_Plat,
	touchF_Touch_Button,
	touchF_Touch_Multi,
	touchF_trigger_push_touch,
	touchF_trigger_teleporter_touch,
	touchF_hurt_touch,
	touchF_NPC_Touch,
	touchF_mine_touch,
	touchF_fixit_touch,
	touchF_touch_stasis_door_trig,
	touchF_flier_child_touch, 
	touchF_touch_stasis_control_trigger,
	touchF_touch_ammo_crystal_tigger,
	touchF_Touch_AssimilationTrigger

} touchFunc_t;

// TOUCH functions...
//
extern void Touch_Item				(gentity_t *self, gentity_t *other, trace_t *trace);
extern void teleporter_touch		(gentity_t *self, gentity_t *other, trace_t *trace);
extern void charge_stick			(gentity_t *self, gentity_t *other, trace_t *trace);
extern void Touch_DoorTrigger		(gentity_t *self, gentity_t *other, trace_t *trace);
extern void Touch_PlatCenterTrigger	(gentity_t *self, gentity_t *other, trace_t *trace);
extern void Touch_Plat				(gentity_t *self, gentity_t *other, trace_t *trace);
extern void Touch_Button			(gentity_t *self, gentity_t *other, trace_t *trace);
extern void Touch_Multi				(gentity_t *self, gentity_t *other, trace_t *trace);
extern void trigger_push_touch		(gentity_t *self, gentity_t *other, trace_t *trace);
extern void trigger_teleporter_touch(gentity_t *self, gentity_t *other, trace_t *trace);
extern void hurt_touch				(gentity_t *self, gentity_t *other, trace_t *trace);
extern void NPC_Touch				(gentity_t *self, gentity_t *other, trace_t *trace);
extern void mine_touch				(gentity_t *self, gentity_t *other, trace_t *trace);
extern void fixit_touch				(gentity_t *self, gentity_t *other, trace_t *trace);
extern void touch_stasis_door_trig	( gentity_t *self, gentity_t *other, trace_t *trace );
extern void flier_child_touch		( gentity_t *self, gentity_t *other, trace_t *trace );
extern void touch_stasis_control_trigger	( gentity_t *self, gentity_t *other, trace_t *trace );
extern void touch_ammo_crystal_tigger	( gentity_t *self, gentity_t *other, trace_t *trace );
extern void Touch_AssimilationTrigger	( gentity_t *ent, gentity_t *other, trace_t *trace ) ;
//	void		(*use)(gentity_t *self, gentity_t *other, gentity_t *activator);
typedef enum
{
	useF_NULL = 0,
	//
	useF_plasma_use,
	useF_surface_explosion_use,
	useF_electrical_explosion_use,
	useF_funcBBrushUse,
	useF_misc_model_use,
	useF_Use_Item,
	useF_Use_Shooter,
	useF_GoExplodeDeath,
	useF_Use_BinaryMover,
	useF_use_wall,
	useF_target_create_formation_use,
	useF_Use_Target_Give,
	useF_Use_Target_Delay,
	useF_Use_Target_Score,
	useF_Use_Target_Print,
	useF_Use_Target_Speaker,
	useF_target_laser_use,
	useF_target_relay_use,
	useF_target_kill_use,
	useF_target_counter_use,
	useF_target_random_use,
	useF_target_scriptrunner_use,
	useF_target_gravity_change_use,
	useF_target_friction_change_use,
	useF_target_teleporter_use,
	useF_Use_Multi,
	useF_Use_target_push,
	useF_hurt_use,
	useF_func_timer_use,
	useF_trigger_entdist_use,
	useF_func_usable_use,
	useF_target_activate_use,
	useF_target_deactivate_use,
	useF_NPC_Use,
	useF_NPC_Spawn,
	useF_laser_use,
	useF_smoke_use,
	useF_transporter_use,
	useF_teleporter_use,
	useF_spark_use,
	useF_bolt_use,
	useF_cooking_steam_use,
	useF_stream_use,
	useF_misc_dlight_use,
	useF_explosion_trail_use,
	useF_blow_chunks_use,
	useF_borg_energy_beam_use,
	useF_drip_use,
	useF_shimmery_thing_use,
	useF_health_use,
	useF_ammo_use,
	useF_mega_ammo_use,
	useF_target_level_change_use,
	useF_borg_bolt_use,
	useF_target_change_parm_use,
	useF_steam_use,
	useF_crew_beam_in_use,
	useF_turret_base_use,
	useF_laser_arm_use,
	useF_func_static_use,
	useF_pj_bolt_use,
	useF_forge_bolt_use,
	useF_stasis_control_switch_use,
	useF_target_play_music_use,
	useF_stasis_teleporter_use,
	useF_stasis_shooter_toggle,
	useF_use_stasis_door,
	useF_stasis_mushroom_use,
	useF_misc_model_useup,
	useF_misc_portal_use,
	useF_target_autosave_use,
	useF_dn_beam_use,
	useF_switch_models,
	useF_reaver_vat_use,
	useF_misc_replicator_item_spawn,
	useF_misc_replicator_item_remove,
	useF_target_interface_use,
	useF_garden_fountain_use,
	useF_func_bobbing_use

} useFunc_t;

// USE functions...
//
extern void plasma_use				( gentity_t *self, gentity_t *other, gentity_t *activator);
extern void surface_explosion_use	( gentity_t *self, gentity_t *other, gentity_t *activator);
extern void electrical_explosion_use( gentity_t *self, gentity_t *other, gentity_t *activator);
extern void funcBBrushUse			( gentity_t *self, gentity_t *other, gentity_t *activator);
extern void misc_model_use			( gentity_t *self, gentity_t *other, gentity_t *activator);
extern void Use_Item				( gentity_t *self, gentity_t *other, gentity_t *activator);
extern void Use_Shooter				( gentity_t *self, gentity_t *other, gentity_t *activator);					 
extern void GoExplodeDeath			( gentity_t *self, gentity_t *other, gentity_t *activator);					 
extern void Use_BinaryMover			( gentity_t *self, gentity_t *other, gentity_t *activator);					 
extern void use_wall				( gentity_t *self, gentity_t *other, gentity_t *activator);					 
extern void target_create_formation_use ( gentity_t *self, gentity_t *other, gentity_t *activator);					 
extern void Use_Target_Give			( gentity_t *self, gentity_t *other, gentity_t *activator);					 
extern void Use_Target_Delay		( gentity_t *self, gentity_t *other, gentity_t *activator);					 
extern void Use_Target_Score		( gentity_t *self, gentity_t *other, gentity_t *activator);					 
extern void Use_Target_Print		( gentity_t *self, gentity_t *other, gentity_t *activator);					 
extern void Use_Target_Speaker		( gentity_t *self, gentity_t *other, gentity_t *activator);					 
extern void target_laser_use		( gentity_t *self, gentity_t *other, gentity_t *activator);
extern void target_relay_use		( gentity_t *self, gentity_t *other, gentity_t *activator);
extern void target_kill_use			( gentity_t *self, gentity_t *other, gentity_t *activator);
extern void target_counter_use		( gentity_t *self, gentity_t *other, gentity_t *activator);
extern void target_random_use		( gentity_t *self, gentity_t *other, gentity_t *activator);							
extern void target_scriptrunner_use	( gentity_t *self, gentity_t *other, gentity_t *activator);
extern void target_gravity_change_use	( gentity_t *self, gentity_t *other, gentity_t *activator);
extern void target_friction_change_use	( gentity_t *self, gentity_t *other, gentity_t *activator);
extern void target_teleporter_use	( gentity_t *self, gentity_t *other, gentity_t *activator);
extern void Use_Multi				( gentity_t *self, gentity_t *other, gentity_t *activator);
extern void Use_target_push			( gentity_t *self, gentity_t *other, gentity_t *activator);
extern void hurt_use				( gentity_t *self, gentity_t *other, gentity_t *activator);
extern void func_timer_use			( gentity_t *self, gentity_t *other, gentity_t *activator);
extern void trigger_entdist_use		( gentity_t *self, gentity_t *other, gentity_t *activator);
extern void func_usable_use			( gentity_t *self, gentity_t *other, gentity_t *activator);
extern void target_activate_use		( gentity_t *self, gentity_t *other, gentity_t *activator);
extern void	target_deactivate_use	( gentity_t *self, gentity_t *other, gentity_t *activator);
extern void NPC_Use					( gentity_t *self, gentity_t *other, gentity_t *activator);
extern void NPC_Spawn				( gentity_t *self, gentity_t *other, gentity_t *activator);
extern void laser_use				( gentity_t *self, gentity_t *other, gentity_t *activator);
extern void smoke_use				( gentity_t *self, gentity_t *other, gentity_t *activator);
extern void transporter_use			( gentity_t *self, gentity_t *other, gentity_t *activator);
extern void teleporter_use			( gentity_t *self, gentity_t *other, gentity_t *activator);
extern void spark_use				( gentity_t *self, gentity_t *other, gentity_t *activator);
extern void bolt_use				( gentity_t *self, gentity_t *other, gentity_t *activator);
extern void cooking_steam_use		( gentity_t *self, gentity_t *other, gentity_t *activator);
extern void stream_use				( gentity_t *self, gentity_t *other, gentity_t *activator);
extern void misc_dlight_use			( gentity_t *ent, gentity_t *other, gentity_t *activator );
extern void explosion_trail_use		( gentity_t *self, gentity_t *other, gentity_t *activator );
extern void blow_chunks_use			( gentity_t *self, gentity_t *other, gentity_t *activator );
extern void borg_energy_beam_use	( gentity_t *self, gentity_t *other, gentity_t *activator );
extern void drip_use				( gentity_t *self, gentity_t *other, gentity_t *activator );
extern void shimmery_thing_use		( gentity_t *self, gentity_t *other, gentity_t *activator );
extern void health_use				( gentity_t *self, gentity_t *other, gentity_t *activator );
extern void ammo_use				( gentity_t *self, gentity_t *other, gentity_t *activator );
extern void mega_ammo_use			( gentity_t *self, gentity_t *other, gentity_t *activator );
extern void target_level_change_use	( gentity_t *self, gentity_t *other, gentity_t *activator );
extern void borg_bolt_use			( gentity_t *self, gentity_t *other, gentity_t *activator );
extern void target_change_parm_use	( gentity_t *self, gentity_t *other, gentity_t *activator );
extern void steam_use				( gentity_t *self, gentity_t *other, gentity_t *activator );
extern void crew_beam_in_use		( gentity_t *self, gentity_t *other, gentity_t *activator );
extern void turret_base_use			( gentity_t *self, gentity_t *other, gentity_t *activator );
extern void laser_arm_use			( gentity_t *self, gentity_t *other, gentity_t *activator );
extern void func_static_use			( gentity_t *self, gentity_t *other, gentity_t *activator );
extern void pj_bolt_use				( gentity_t *self, gentity_t *other, gentity_t *activator );
extern void forge_bolt_use			( gentity_t *self, gentity_t *other, gentity_t *activator );
extern void stasis_control_switch_use	( gentity_t *self, gentity_t *other, gentity_t *activator );
extern void target_play_music_use	( gentity_t *self, gentity_t *other, gentity_t *activator );
extern void stasis_teleporter_use	( gentity_t *self, gentity_t *other, gentity_t *activator );
extern void stasis_shooter_toggle	( gentity_t *self, gentity_t *other, gentity_t *activator );
extern void use_stasis_door			( gentity_t *self, gentity_t *other, gentity_t *activator );
extern void stasis_mushroom_use		( gentity_t *self, gentity_t *other, gentity_t *activator );
extern void misc_model_useup		( gentity_t *self, gentity_t *other, gentity_t *activator );
extern void misc_portal_use			( gentity_t *self, gentity_t *other, gentity_t *activator );
extern void target_autosave_use 	( gentity_t *self, gentity_t *other, gentity_t *activator );
extern void dn_beam_use				( gentity_t *self, gentity_t *other, gentity_t *activator );
extern void switch_models			( gentity_t *self, gentity_t *other, gentity_t *activator );
extern void reaver_vat_use			( gentity_t *self, gentity_t *other, gentity_t *activator );
extern void misc_replicator_item_spawn ( gentity_t *self, gentity_t *other, gentity_t *activator );
extern void misc_replicator_item_remove ( gentity_t *self, gentity_t *other, gentity_t *activator );
extern void target_interface_use	( gentity_t *self, gentity_t *other, gentity_t *activator );
extern void garden_fountain_use		( gentity_t *self, gentity_t *other, gentity_t *activator );
extern void func_bobbing_use		( gentity_t *self, gentity_t *other, gentity_t *activator );


//	void		(*pain)(gentity_t *self, gentity_t *attacker, int damage);
typedef enum
{
	painF_NULL = 0,
	//
	painF_funcBBrushPain,
	painF_misc_model_breakable_pain,
	painF_NPC_Pain,
	painF_station_pain,
	painF_func_usable_pain,
	painF_flier_pain,
	painF_NPC_Scav_Pain,

} painFunc_t;

// PAIN functions...
//
extern void funcBBrushPain				(gentity_t *self, gentity_t *attacker, int damage);
extern void misc_model_breakable_pain	(gentity_t *self, gentity_t *attacker, int damage);
extern void NPC_Pain					(gentity_t *self, gentity_t *attacker, int damage);
extern void station_pain				(gentity_t *self, gentity_t *attacker, int damage);
extern void func_usable_pain			(gentity_t *self, gentity_t *attacker, int damage);
extern void flier_pain					(gentity_t *self, gentity_t *attacker, int damage);
extern void NPC_Scav_Pain				(gentity_t *self, gentity_t *attacker, int damage);

//	void		(*die)(gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod);
typedef enum
{
	dieF_NULL = 0,
	//
	dieF_funcBBrushDie,
	dieF_misc_model_breakable_die,
	dieF_player_die,
	dieF_ExplodeDeath_Wait,
	dieF_func_usable_die,
	dieF_mine_die,
	dieF_flier_die,
	dieF_turret_die,
	dieF_bot_rocket_die,
	dieF_reaver_vat_die

} dieFunc_t;

// DIE functions...
//
extern void funcBBrushDie				(gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod);
extern void misc_model_breakable_die	(gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod);
extern void player_die					(gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod);
extern void ExplodeDeath_Wait			(gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod);
extern void func_usable_die				(gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod);
extern void mine_die					(gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod);
extern void flier_die					(gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod);
extern void turret_die					(gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod);
extern void bot_rocket_die				(gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod);
extern void reaver_vat_die				(gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod);

void GEntity_ThinkFunc(gentity_t *self);
void CEntity_ThinkFunc(centity_s *cent);	//Think func for equivalent centity
void GEntity_ReachedFunc(gentity_t *self);	// movers call this when hitting endpoint
void GEntity_BlockedFunc(gentity_t *self, gentity_t *other);
void GEntity_TouchFunc(gentity_t *self, gentity_t *other, trace_t *trace);
void GEntity_UseFunc(gentity_t *self, gentity_t *other, gentity_t *activator);
void GEntity_PainFunc(gentity_t *self, gentity_t *attacker, int damage);
void GEntity_DieFunc(gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod);

// external functions that I now refer to...


#endif	// #ifndef G_FUNCTIONS

/////////////////// eof ///////////////////

