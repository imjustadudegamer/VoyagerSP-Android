#ifndef __BSTATE_H__
#define __BSTATE_H__

//bstate.h
typedef enum //# bState_e
{//These take over only if script allows them to be autonomous
	BS_DEFAULT = 0,//# whatever
	BS_IDLE,//# 1: move if have goal, update angles
	BS_ROAM,//# 2: Roam around, collect stuff
	BS_WALK,//# 3: Walk toward their goals
	BS_RUN,//# 4: Run toward their goals
	BS_STAND_AND_SHOOT,//# 5: Stay in one spot and shoot- duck when neccesary
	BS_STAND_GUARD,//# 6: Wait around for an enemy
	BS_PATROL,//# 7: Follow a path, looking for enemies
	BS_HUNT_AND_KILL,//# 8: Track down enemies and kill them
	BS_EVADE,//# 9: Run from enemies
	BS_EVADE_AND_SHOOT,//# 10: Run from enemies, shoot them if they hit you
	BS_RUN_AND_SHOOT,//# 11: Run to your goal and shoot enemy when possible
	BS_DEFEND,//# 12: Defend an entity or spot?
	BS_COMBAT,//# 14: Attack, evade, use cover, move about, etc.  Full combat AI - id Bot code
	BS_MEDIC,//# 15: Go for lowest health buddy, hide and heal him.
	BS_TAKECOVER,//# 16: Find nearest cover from enemies
	BS_GET_AMMO,//# 17: Go get some ammo
	BS_ADVANCE_FIGHT,//# 18: Anvance to captureGoal and shoot enemies if you can
	BS_FACE,//# 19: turn until facing desired angles
	BS_WAIT,//# 20: do abolutely nothing
	BS_FORMATION,//# 21: Maintain a formation
	BS_CROUCH,//# 22: Crouch-Walk toward their goals
	BS_MOVE,//# 23: Move in one dir, face another
	BS_WAITHEAL,//# 24: Do nothing until health > 75
	BS_SHOOT,//# 25: shoot straight ahead
	BS_SNIPER,//# 26: wait for a good shot
	BS_MEDIC_COMBAT,//# 27: Run to and heal people to a certain level then go back to hiding spot
	BS_MEDIC_HIDE,//# 28: Stay in hiding spot and wait for others to come to you
	BS_POINT_AND_SHOOT,//# 29: Face enemy and shoot once
	BS_FACE_ENEMY,//# 30: Face enemy
	BS_INVESTIGATE,//# 31: head towards temp goal and look for enemies and listen for sounds
	BS_SLEEP,//# 32: Play awake script when startled by sound
	//BS_FLEE,//# 33: Run away!
	//BS_RETREAT,//# 34: Back away while still engaging enemy
	//BS_COVER,//# 35: Watch your coverTarg and shoot any enemy around him, laying down supressing fire
	BS_SAY,//# 36: Turn head to sayTarg, use talk anim, say your sayString (look up string in your sounds table), exit tempBState when sound finished (anim of mouth should be timed to length of sound as well)
	BS_AIM,//# 37: Turn head and torso to facing, keep feet in place if you can
	BS_LOOK,//# 38: Turn head only to facing, keep torso and head in place if you can
	BS_POINT_COMBAT,//# 39: Head toward closest empty point_combat and shoot from there
	BS_FOLLOW_LEADER,//# 40: Follow your leader and shoot any enemies you come across
	BS_JUMP,//# 41: Face navgoal and jump to it.
	BS_REMOVE,//# 42: Waits for player to leave PVS then removes itself
	BS_SEARCH,//# 43: Using current waypoint as a base, search the immediate branches of waypoints for enemies
	BS_FLY,//# 44: Moves around without gravity
	BS_NOCLIP,//# 45: Moves through walls, etc.
	BS_WANDER,//# 46: Wander down random waypoint paths
	BS_FACE_LEADER,//# 47: They will stand still and keep facing whatever ent is set by SET_LEADER until their bState is changed
	//# #eol
	NUM_BSTATES
} bState_t;

#endif //#ifndef __BSTATE_H__
