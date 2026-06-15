#ifndef __ANIMS_H__
#define __ANIMS_H__
// playerAnimations


typedef enum //# animNumber_e
{
	//=================================================
	//ANIMS IN WHICH UPPER AND LOWER OBJECTS ARE IN MD3
	//=================================================
	//# #sep BOTH_ DEATHS
	BOTH_DEATH1 = 0,		//# First Death anim
	BOTH_DEATH2,			//# Second Death anim
	BOTH_DEATH3,			//# Third Death anim
	BOTH_DEATH4,			//# Fourth Death anim
	BOTH_DEATH5,			//# Fifth Death anim
	BOTH_DEATH6,			//# Sixth Death anim
	BOTH_DEATH7,			//# Seventh Death anim

	BOTH_DEATHFORWARD1,		//# First Death in which they get thrown forward
	BOTH_DEATHFORWARD2,		//# Second Death in which they get thrown forward
	BOTH_DEATHBACKWARD1,	//# First Death in which they get thrown backward
	BOTH_DEATHBACKWARD2,	//# Second Death in which they get thrown backward

	BOTH_DEATH1IDLE,		//# Idle while close to death
	BOTH_LYINGDEATH1,		//# Death to play when killed lying down
	BOTH_STUMBLEDEATH1,		//# Stumble forward and fall face first death
	BOTH_FALLDEATH1,		//# Fall forward off a high cliff and splat death - start
	BOTH_FALLDEATH1INAIR,	//# Fall forward off a high cliff and splat death - loop
	BOTH_FALLDEATH1LAND,	//# Fall forward off a high cliff and splat death - hit bottom
	//# #sep BOTH_ DEAD POSES # Should be last frame of corresponding previous anims
	BOTH_DEAD1,				//# First Death finished pose
	BOTH_DEAD2,				//# Second Death finished pose
	BOTH_DEAD3,				//# Third Death finished pose
	BOTH_DEAD4,				//# Fourth Death finished pose
	BOTH_DEAD5,				//# Fifth Death finished pose
	BOTH_DEAD6,				//# Sixth Death finished pose
	BOTH_DEAD7,				//# Seventh Death finished pose
	BOTH_DEADFORWARD1,		//# First thrown forward death finished pose
	BOTH_DEADFORWARD2,		//# Second thrown forward death finished pose
	BOTH_DEADBACKWARD1,		//# First thrown backward death finished pose
	BOTH_DEADBACKWARD2,		//# Second thrown backward death finished pose
	BOTH_LYINGDEAD1,		//# Killed lying down death finished pose
	BOTH_STUMBLEDEAD1,		//# Stumble forward death finished pose
	BOTH_FALLDEAD1LAND,		//# Fall forward and splat death finished pose
	//# #sep BOTH_ DEAD TWITCH/FLOP # React to being shot from death poses
	BOTH_DEAD1_FLOP,		//# React to being shot from First Death finished pose
	BOTH_DEAD2_FLOP,		//# React to being shot from Second Death finished pose
	BOTH_DEAD3_FLOP,		//# React to being shot from Third Death finished pose
	BOTH_DEAD4_FLOP,		//# React to being shot from Fourth Death finished pose
	BOTH_DEAD5_FLOP,		//# React to being shot from Fifth Death finished pose 
	BOTH_DEADFORWARD1_FLOP,		//# React to being shot First thrown forward death finished pose
	BOTH_DEADFORWARD2_FLOP,		//# React to being shot Second thrown forward death finished pose
	BOTH_DEADBACKWARD1_FLOP,	//# React to being shot First thrown backward death finished pose
	BOTH_DEADBACKWARD2_FLOP,	//# React to being shot Second thrown backward death finished pose
	BOTH_LYINGDEAD1_FLOP,		//# React to being shot Killed lying down death finished pose
	BOTH_STUMBLEDEAD1_FLOP,		//# React to being shot Stumble forward death finished pose
	BOTH_FALLDEAD1_FLOP,	//# React to being shot Fall forward and splat death finished pose
	//# #sep BOTH_ PAINS
	BOTH_PAIN1,				//# First take pain anim
	BOTH_PAIN2,				//# Second take pain anim
	BOTH_PAIN3,				//# Third take pain anim
	BOTH_PAIN4,				//# Fourth take pain anim
	BOTH_PAIN5,				//# Fifth take pain anim - from behind
	BOTH_PAIN6,				//# Sixth take pain anim - from behind
	BOTH_PAIN7,				//# Seventh take pain anim - from behind
	BOTH_PAIN8,				//# Eigth take pain anim - from behind
	//# #sep BOTH_ ATTACKS
	BOTH_ATTACK1,			//# Attack with generic 1-handed weapon
	BOTH_ATTACK2,			//# Attack with generic 2-handed weapon
	BOTH_ATTACK3,			//# Attack with heavy 2-handed weapon
	BOTH_ATTACK4,			//# Attack with ???
	BOTH_ATTACK5,			//# Attack with rocket launcher
	BOTH_MELEE1,			//# First melee attack
	BOTH_MELEE2,			//# Second melee attack
	BOTH_MELEE3,			//# Third melee attack
	BOTH_MELEE4,			//# Fourth melee attack
	BOTH_MELEE5,			//# Fifth melee attack
	BOTH_MELEE6,			//# Sixth melee attack
	//# #sep BOTH_ STANDING
	BOTH_STAND1,			//# Standing idle, no weapon, hands down
	BOTH_STAND1_RANDOM1,	//# Random standing idle
	BOTH_STAND1_RANDOM2,	//# Random standing idle
	BOTH_STAND1_RANDOM3,	//# Random standing idle
	BOTH_STAND1_RANDOM4,	//# Random standing idle
	BOTH_STAND1_RANDOM5,	//# Random standing idle
	BOTH_STAND1_RANDOM6,	//# Random standing idle
	BOTH_STAND1_RANDOM7,	//# Random standing idle
	BOTH_STAND1_RANDOM8,	//# Random standing idle
	BOTH_STAND1_RANDOM9,	//# Random standing idle
	BOTH_STAND1_RANDOM10,	//# Random standing idle
	BOTH_STAND1_RANDOM11,	//# Random standing idle
	BOTH_STAND1_RANDOM12,	//# Random standing idle
	BOTH_STAND1_RANDOM13,	//# Random standing idle
	BOTH_STAND1_RANDOM14,	//# Random standing idle
	BOTH_STAND2,			//# Standing idle with a weapon
	BOTH_STAND2_RANDOM1,	//# Random standing idle
	BOTH_STAND2_RANDOM2,	//# Random standing idle
	BOTH_STAND2_RANDOM3,	//# Random standing idle
	BOTH_STAND2_RANDOM4,	//# Random standing idle
	BOTH_STAND3,			//# Standing hands behind back, at ease, etc.
	BOTH_STAND4,			//# two handed, gun down, relaxed stand
	BOTH_STAND5,			//# two handed, gun up, relaxed stand
	BOTH_STAND6,			//# one handed, gun at side, relaxed stand
	BOTH_STAND7,			//# (Chell) timid stance while looking around slightly and breathing
	BOTH_STAND8,			//# breathing after exherting oneself one handed
	BOTH_STAND9,			//# breathing after exherting oneself two handed
	BOTH_STAND1TO3,			//# Transition from stand1 to stand3
	BOTH_STAND3TO1,			//# Transition from stand3 to stand1

	BOTH_STAND2TO4,			//# Transition from stand2 to stand4
	BOTH_STAND4TO2,			//# Transition from stand4 to stand2
	BOTH_STANDTOWALK1,		//# Transition from stand1 to walk1
	BOTH_STANDTOCONSOLE1,	//# a transition from stand animations to console animations
	BOTH_STANDUP1,			//# standing up and stumbling
	BOTH_TALKGESTURE1,		//# standing up and talking
	BOTH_TALKGESTURE2,		//# standing up and talking
	BOTH_TALKGESTURE3,		//# standing up and talking

	BOTH_HELP1,				//# helping hold injured4 man.

	BOTH_LEAN1,				//# leaning on a railing
	BOTH_LEAN1TODROPHELM,	//# transition from LEAN1 to DROPHELM

	BOTH_CONSOLE1,			//# Using a waist-high console with both hands
	BOTH_CONSOLE1IDLE,		//# Idle of CONSOLE1
	BOTH_CONSOLE1RIGHT,		//# Reach right from CONSOLE1
	BOTH_CONSOLE1LEFT,		//# Reach left from CONSOLE1
	BOTH_CONSOLE2,			//# Using a head-high wall console with the right hand
	BOTH_CONSOLE3,			//# arms parallel to ground and typing similar to con.1
	BOTH_CONSOLE3IDLE,		//# arms parallel to ground and typing similar to con.1
	BOTH_CONSOLE3RIGHT,		//# arms parallel to ground and typing similar to con.1
	BOTH_CONSOLE3LEFT,		//# arms parallel to ground and typing similar to con.1
	BOTH_CONSOLETOSTAND1,	//# a transition from console animations to stand animations

	BOTH_GUARD_LOOKAROUND1,	//# Cradling weapon and looking around
	BOTH_GUARD_IDLE1,		//# Cradling weapon and standing
	BOTH_GUARD_LKRT1,		//# cin17, quick glance right to sound of door slamming
	BOTH_ALERT1,			//# Startled by something while on guard
	BOTH_GESTURE1,			//# Generic gesture, non-specific
	BOTH_GESTURE2,			//# Generic gesture, non-specific
	BOTH_GESTURE3,			//# Generic gesture, non-specific
	BOTH_CROWDLOOK1,		//# Person staring out into space 1
	BOTH_CROWDLOOK2,		//# Person staring out into space 2
	BOTH_CROWDLOOK3,		//# Person staring out into space 3
	BOTH_CROWDLOOK4,		//# Person staring out into space 4
	BOTH_GRAB1,				//# Grabbing something from table
	BOTH_GRAB2,				//# Grabbing something from table
	BOTH_GRAB3,				//# Grabbing something from table
	BOTH_GRABBED1,			//# cin9.3 chell being grabbed 180 from munro, 28 pixels away
	BOTH_GRABBED2,			//# cin9.3 idle grabbed 180 from munro, 28 pixels away
	BOTH_SURPRISED1,		//# Surprised reaction 1
	BOTH_SURPRISED2,		//# Surprised reaction 2
	BOTH_SURPRISED3,		//# Surprised reaction 3
	BOTH_SURPRISED4,		//# Surprised reaction 4
	BOTH_SURPRISED5,		//# Surprised reaction 5
	BOTH_SCARED1,			//# Scared reaction 1
	BOTH_SCARED2,			//# Scared reaction 2
	BOTH_CATCH1,			//# Reaching to catch something falling
	BOTH_POSSESSED1,		//# 7 of 9 possessed
	BOTH_POSSESSED2,		//# 7 of 9 possessed with hand out 
	BOTH_SNAPTO1,			//# cin.23, 7o9 coming to from borg possession
	BOTH_SNAPTO2,			//# cin.23, 7o9 coming to from borg possession2
	BOTH_DROPANGERWEAP2,		//# cin.23, Nelson lowering weapon in anger
	BOTH_SHOCK1,			//# telsia being zapped by electricity cinematic 9.2
	BOTH_PSYCHICSHOCK1,		//# having visions of the boss
	BOTH_PSYCHICSHOCK2,		//# having visions of the boss
	BOTH_ASSIMILATED1,		//# Cin.18, Foster being assimilated by borg
	BOTH_FALSEJUMP1,		//# Biessman pretending to jump down on Chell
	BOTH_LAUGH1,			//# squat pose of Biessman laughing at Chell
	BOTH_LAUGH2,			//# standing laugh of mocking Biessman
	BOTH_ACTIVATEBELT1,		//# activating transport buffer on belt

	BOTH_GROUNDSHAKE1,		//#	Bracing self when ground shakes beneath him
	BOTH_GROUNDSHAKE2,		//#	Falling to knees and shileding self, then standing

	BOTH_READYWEAPON1,		//# cin17, comes from greeting, just before fighting

	BOTH_SPAWN1,			//# Spawning in to the world
	BOTH_TALK1,				//# Generic talk anim

	BOTH_COVERUP1_LOOP,		//# animation of getting in line of friendly fire
	BOTH_COVERUP1_START,	//# transitions from stand to coverup1_loop
	BOTH_COVERUP1_END,		//# transitions from coverup1_loop to stand
	BOTH_HEROSTANCE1,		//# Biessman in the final shootout
	BOTH_GUILT1,			//# Player has a guilty conscience after shooting a teammate.

	BOTH_INJURED4,			//# Injured pose 4
	BOTH_INJURED4TO5,		//# Transition from INJURED4 to INJURED5
	BOTH_INJURED5,			//# Injured pose 5

	//# #sep BOTH_ SITTING/CROUCHING
	BOTH_SIT1STAND,			//# Stand up from First sitting anim
	BOTH_SIT1TO2,			//# Trans from sit1 to sit2?
	BOTH_SIT1TO3,			//# Trans from sit1 to sit3?
	BOTH_SIT2TO1,			//# Trans from sit2 to sit1?
	BOTH_SIT2TO3,			//# Trans from sit2 to sit3?
	BOTH_SIT3TO1,			//# Trans from sit3 to sit1?
	BOTH_SIT3TO2,			//# Trans from sit3 to sit2?

	BOTH_SIT4TO5,			//# Trans from sit4 to sit5
	BOTH_SIT4TO6,			//# Trans from sit4 to sit6
	BOTH_SIT5TO4,			//# Trans from sit5 to sit4
	BOTH_SIT5TO6,			//# Trans from sit5 to sit6
	BOTH_SIT6TO4,			//# Trans from sit6 to sit4
	BOTH_SIT6TO5,			//# Trans from sit6 to sit5
	BOTH_SIT7,				//# sitting with arms over knees, no weapon
	BOTH_SIT7TOSTAND1,		//# getting up from sit7 into stand1

	BOTH_TABLE_EAT1,		//# Sitting at a table eating
	BOTH_TABLE_CHEW1,		//# Sitting at a table chewing
	BOTH_TABLE_WIPE1,		//# Sitting at a table wiping mouth
	BOTH_TABLE_DRINK1,		//# Sitting at a table drinking
	BOTH_TABLE_GETUP1,		//# Getting up from table
	BOTH_TABLE_DEATH1,		//# Dying while sitting at a table
	BOTH_TABLE_IDLE1,		//# Sitting at table breathing
	BOTH_TABLE_TALKGESTURE1,//# Sitting at table gesturing while talking
	BOTH_TABLE_GESTURE1,	//# Sitting at table gesturing
	BOTH_TABLE_GESTURE2,	//# Sitting at table gesturing

	BOTH_CROUCH1,			//# Transition from standing to crouch
	BOTH_CROUCH1IDLE,		//# Crouching idle
	BOTH_CROUCH1WALK,		//# Walking while crouched
	BOTH_UNCROUCH1,			//# Transition from crouch to standing
	BOTH_CROUCH2IDLE,		//# crouch and resting on back righ heel, no weapon
	BOTH_CROUCH2TOSTAND1,	//# going from crouch2 to stand1
	BOTH_GET_UP1,			//# Get up from the ground, face down
	BOTH_GET_UP2,			//# Get up from the ground, face up

	BOTH_BENCHSIT1_IDLE,	//# sitting on haz-locker room benches
	BOTH_BENCHSIT1TO2,		//# Trans from benchsit1 to benchsit2
	BOTH_BENCHSIT2TO1,		//# Trans from benchsit2 to benchsit1
	BOTH_BENCHSIT2STAND,	//# Trans from benchsit to standing
	BOTH_BENCHSIT2_IDLE,	//# sitting on haz-locker room benches
	BOTH_BENCHSIT1_2STAND,	//# getting up to stand from sitting on haz-benches
	BOTH_BENCHSIT1_FIXBOOT,	//# sitting on bench - pulling on/adjusting boot top
	BOTH_BENCHSTAND1TO2,	//# transition from stand to benchstand2
	BOTH_BENCHSTAND2,		//# standing with right foot up on bench
	BOTH_BENCHSTAND2TO1,	//# transition from benchstand2 to stand

	BOTH_COUCHSIT1_IDLE,	//# sitting in couch - haz lounge area
	BOTH_COUCHSIT1_TO2,	//# sitting in couch - lean back to 2nd position
	BOTH_COUCHSIT1_2STAND1,	//# getting up from couchsit1 to stand1
	BOTH_COUCHSIT1_TALKGESTURE,	//# sitting in couch - talking with hands
	BOTH_COUCHSIT1_GESTURELEFT,	//# sitting in couch - talk gesture to the left
	BOTH_COUCHSIT1_GESTURERIGHT,//# sitting in couch - talk gesture to the right

	BOTH_KNEELHAND1,			//# Jurot puts hand to Munro's face, then pulls away

	//# #sep BOTH_ MOVING
	BOTH_WALK1,				//# Normal walk
	BOTH_WALK2,				//# Normal walk
	BOTH_WALK3,				//# Goes with stand3
	BOTH_WALK4,				//# Walk cycle goes to a stand4
	BOTH_WALKTORUN1,		//# transition from walk to run
	BOTH_RUN1,				//# Full run
	BOTH_RUN1START,			//# Start into full run1
	BOTH_RUN1STOP,			//# Stop from full run1
	BOTH_RUN2,				//# Full run
	BOTH_RUNINJURED1,		//# Run with injured left leg
	BOTH_STRAFE_LEFT1,		//# Sidestep left, should loop
	BOTH_STRAFE_RIGHT1,		//# Sidestep right, should loop
	BOTH_TURN_LEFT1,		//# Turn left, should loop
	BOTH_TURN_RIGHT1,		//# Turn right, should loop
	BOTH_RUNAWAY1,			//# Running scared
	BOTH_SWIM1,				//# Swimming
	BOTH_JUMP1,				//# Jump - wind-up and leave ground
	BOTH_INAIR1,			//# In air loop (from jump)
	BOTH_LAND1,				//# Landing (from in air loop)
	BOTH_LAND2,				//# Landing Hard (from a great height)
	BOTH_JUMPBACK1,			//# Jump backwards - wind-up and leave ground
	BOTH_INAIRBACK1,		//# In air loop (from jump back)
	BOTH_LANDBACK1,			//# Landing backwards(from in air loop)
	BOTH_DIVE1,				//# Dive!
	BOTH_ROLL1_LEFT,		//# Roll to left side
	BOTH_ROLL1_RIGHT,		//# Roll to right side
	BOTH_LADDER_UP1,		//# Climbing up a ladder with rungs at 16 unit intervals
	BOTH_LADDER_DWN1,		//# Climbing down a ladder with rungs at 16 unit intervals
	BOTH_LADDER_IDLE,		//#	Just sitting on the ladder
	BOTH_ONLADDER_BOT1,		//# Getting on the ladder at the bottom
	BOTH_OFFLADDER_BOT1,	//# Getting off the ladder at the bottom
	BOTH_ONLADDER_TOP1,		//# Getting on the ladder at the top
	BOTH_OFFLADDER_TOP1,	//# Getting off the ladder at the top
	BOTH_LIFT1,				//# Lifting someone/thing over their shoulder
	BOTH_STEP1,				//# telsia checking out lake cinematic9.2
	BOTH_HITWALL1,			//# cin.18, Kenn hit by borg into wall 56 units away
	BOTH_AMBUSHLAND1,		//# landing from fall on victim
	BOTH_BIRTH1,			//# birth from jumping through walls

	BOTH_SHIELD1,			//# cin.6, munro's initial reaction to explosion
	BOTH_SHIELD2,			//# cin.6, munro in shielding position looping
	BOTH_WALKPUSH1,			//# man pushing crate
	BOTH_PUSHTOSTAND1,		//# man coming from pushing crate to stand1
	BOTH_HALT1,					//# munro being grabbed by telsia before going in core room

	//# #sep BOTH_ FLYING IDLE
	BOTH_FLY_IDLE1,		//# Flying Idle 1
	BOTH_FLY_IDLE2,		//# Flying Idle 2


	//# #sep BOTH_ FLYING MOVING
	BOTH_FLY_START1,		//# Start flying
	BOTH_FLY_STOP1,			//# Stop flying
	BOTH_FLY_LOOP1,			//# Normal flying, should loop
	BOTH_FLOAT1,			//# Crew floating through space 1
	BOTH_FLOAT2,			//# Crew floating through space 2
	BOTH_FLOATCONSOLE1,		//# Crew floating and working on console

	//# #sep BOTH_ LYING
	BOTH_LIE_DOWN1,			//# From a stand position, get down on ground, face down
	BOTH_LIE_DOWN2,			//# From a stand position, get down on ground, face up
	BOTH_LIE_DOWN3,			//# reaction to local disnode being destroyed
	BOTH_PAIN2WRITHE1,		//# Transition from upright position to writhing on ground anim
	BOTH_PRONE2RLEG,		//# Lying on ground reach to grab right leg
	BOTH_PRONE2LLEG,		//# Lying on ground reach to grab left leg
	BOTH_WRITHING1,			//# Lying on ground on back writhing in pain
	BOTH_WRITHING1RLEG,		//# Lying on ground writhing in pain, holding right leg
	BOTH_WRITHING1LLEG,		//# Lying on ground writhing in pain, holding left leg
	BOTH_WRITHING2,			//# Lying on ground on front writhing in pain
	BOTH_INJURED1,			//# Lying down, against wall - can also be sleeping against wall
	BOTH_INJURED2,			//# Injured pose 2
	BOTH_INJURED3,			//# Injured pose 3
	BOTH_INJURED6,			//# Injured pose 6
	BOTH_INJURED6ATTACKSTART,	//# Start attack while in injured 6 pose 
	BOTH_INJURED6ATTACKSTOP,	//# End attack while in injured 6 pose
	BOTH_INJURED6COMBADGE,	//# Hit combadge while in injured 6 pose
	BOTH_INJURED6POINT,		//# Chang points to door while in injured state
	BOTH_INJUREDTOSTAND1,	//# Runinjured to stand1

	BOTH_CRAWLBACK1,		//# Lying on back, crawling backwards with elbows
	BOTH_SITWALL1,			//# Sitting against a wall
	BOTH_SLEEP1,			//# laying on back-rknee up-rhand on torso
	BOTH_SLEEP2,			//# on floor-back against wall-arms crossed
	BOTH_SLEEP3,			//# Sleeping in a chair
	BOTH_SLEEP4,			//# Sleeping slumped over table
	BOTH_SLEEP5,			//# Laying on side sleeping on flat sufrace
	BOTH_SLEEP1GETUP,		//# alarmed and getting up out of sleep1 pose to stand
	BOTH_SLEEP1GETUP2,		//# 
	BOTH_SLEEP2GETUP,		//# alarmed and getting up out of sleep2 pose to stand
	BOTH_SLEEP3GETUP,		//# alarmed and getting up out of sleep3 pose to stand
	BOTH_SLEEP3DEATH,		//# death in chair, from sleep3 idle
	BOTH_SLEEP3DEAD,		//# death in chair, from sleep3 idle

	BOTH_SLEEP_IDLE1,		//# rub face and nose while asleep from sleep pose 1
	BOTH_SLEEP_IDLE2,		//# shift position while asleep - stays in sleep2
	BOTH_SLEEP_IDLE3,		//# Idle anim from sleep pose 3
	BOTH_SLEEP_IDLE4,		//# Idle anim from sleep pose 4
	BOTH_SLEEP1_NOSE,		//# Scratch nose from SLEEP1 pose
	BOTH_SLEEP2_SHIFT,		//# Shift in sleep from SLEEP2 pose
	BOTH_RESTRAINED1,		//# Telsia tied to medical table
	BOTH_RESTRAINED1POINT,	//# Telsia tied to medical table pointing at Munro
	BOTH_LIFTED1,			//# Fits with BOTH_LIFT1, lifted on shoulder
	BOTH_CARRIED1,			//# Fits with TORSO_CARRY1, carried over shoulder
	BOTH_CARRIED2,			//# Laying over object

	//# #sep BOTH_ BORG-SPECIFIC
	BOTH_PLUGIN1,			//# Borg plugs self in to alcove
	BOTH_PLUGGEDIN1,		//# Last frame of Borg plug in sequence
	BOTH_PLUGOUT1,			//# Borg unplugs self from alcove

	//# #sep BOTH_ HUNTER-SEEKER BOT-SPECIFIC
	BOTH_POWERUP1,			//# Wakes up


	//=================================================
	//ANIMS IN WHICH ONLY THE UPPER OBJECTS ARE IN MD3
	//=================================================
	//# #sep TORSO_ WEAPON-RELATED
	TORSO_DROPWEAP1,		//# Put weapon away
	TORSO_DROPWEAP2,		//# Put weapon away
	TORSO_DROPWEAP3,		//# Put weapon away
	TORSO_RAISEWEAP1,		//# Draw Weapon
	TORSO_RAISEWEAP2,		//# Draw Weapon
	TORSO_RAISEWEAP3,		//# Draw Weapon
	TORSO_WEAPONREADY1,		//# Ready to fire 1 handed weapon
	TORSO_WEAPONREADY2,		//# Ready to fire 2 handed weapon
	TORSO_WEAPONREADY3,		//# Ready to fire heavy 2 handed weapon
	TORSO_WEAPONIDLE1,		//# Holding 1 handed weapon
	TORSO_WEAPONIDLE2,		//# Holding 2 handed weapon
	TORSO_WEAPONIDLE3,		//# Holding heavy 2 handed weapon

	//# #sep TORSO_ USING NON-WEAPON OBJECTS
	TORSO_TRICORDER1,		//# Using a tricorder
	TORSO_MEDICORDER1,		//# Using a Medical Tricorder
	TORSO_PADD1,			//# Using a PADD
	TORSO_EQUIPMENT1,		//# Twisting pipe with both hands
	TORSO_EQUIPMENT2,		//# Fidgiting with cylinder with both hands
	TORSO_EQUIPMENT3,		//# Using equipment one handed
	TORSO_WRIST1,			//# cin.24, Chang detonating bomb with wrist device

	//# #sep TORSO_ MISC
	TORSO_COMBADGE1,		//# Touch right hand to left breast
	TORSO_COMBADGE2,		//# Touch left hand to left breast
	TORSO_COMBADGE3,		//# Combadge touch from stand4
	TORSO_REDALERT1,		//# Hitting comm button on wall with hand (Kirk-like)
	TORSO_HANDGESTURE1,		//# gestures to left one hand
	TORSO_HANDGESTURE2,		//# gestures to right one hand
	TORSO_HANDGESTURE3,		//# gestures to the left both hands
	TORSO_HANDGESTURE4,		//# gestures to the right both hands
	TORSO_HANDGESTURE5,		//# ?
	TORSO_HANDGESTURE6,		//# pointing (flank right) while talking & holding a weapon
	TORSO_HANDGESTURE7,		//# pointing (forward) while talking & holding a weapon
	TORSO_HANDGESTURE8,		//# pointing (flank left) while talking & holding a weapon
	TORSO_HANDGESTURE9,		//# quick point right from stand 4
	TORSO_HANDGESTURE10,	//# quick point forward from stand 4
	TORSO_HANDGESTURE11,	//# quick point left from stand 4
	TORSO_HANDGESTURE12,	//# gesturing with both hands forward
	TORSO_HANDGESTURE13,	//# gesturing a shrug as if not knowing answer

	TORSO_HEADNOD1,			//# nod in affirmation
	TORSO_HEADSHAKE1,		//# head goes down while shaking left and right in dissapointment
	TORSO_HYPOSPRAY1,		//# man giving hypo to people
	TORSO_HYPOSPRAY4,		//# using hypospray on telsia in scav5

	TORSO_HANDEXTEND1,		//# doctor reaching for hypospray in scav5
	TORSO_HANDRETRACT1,		//# doctor taking hypospray from player in scav5

	TORSO_DROPHELMET1,		//# Drop the helmet to the waist
	TORSO_RAISEHELMET1,		//# Bring the helmet to the head
	TORSO_REACHHELMET1,		//# reaching for helmet off of 60 tall cabinet
	TORSO_GRABLBACKL,		//# reach to lower back with left hand
	TORSO_GRABUBACKL,		//# reach to upper back with left hand
	TORSO_GRABLBACKR,		//# reach to lower back with right hand
	TORSO_GRABUBACKR,		//# reach to upper back with right hand

	TORSO_STAND2TOWEAPONREADY2,	//# cin.23, Nelson raising weapon in alarm and ready to fire

	TORSO_HAND1,			//# Exchanging items - giver
	TORSO_HAND2,			//#  Exchanging items - receiver

	TORSO_POKERIDLE1,		//# holding cards
	TORSO_POKERIDLE2,		//# re-arranging cards
	TORSO_POKERIDLE3,		//# put card on table

	TORSO_SPEECHLESS1,		//# hanging head in grief 1
	TORSO_SPEECHLESS2,		//# hanging head in grief 2

	TORSO_SHOUT1,			//# left hand to mouth
	TORSO_CARRY1,			//#	Carrying someone/thing over their shoulder (can go from BOTH_LIFT1)



	//=================================================
	//ANIMS IN WHICH ONLY THE LOWER OBJECTS ARE IN MD3
	//=================================================
	//# #sep Legs-only anims
	LEGS_WALKBACK1,			//# Walk1 backwards
	LEGS_WALKBACK2,			//# Walk2 backwards
	LEGS_RUNBACK1,			//# Run1 backwards
	LEGS_RUNBACK2,			//# Run2 backwards
	LEGS_TURN1,				//# What legs do when you turn your lower body to match your upper body facing
	LEGS_TURN2,				//# Leg turning from stand2
	LEGS_LEAN_LEFT1,		//# Lean left
	LEGS_LEAN_RIGHT1,		//# Lean Right
	LEGS_KNEELDOWN1,		//# Get down on one knee?
	LEGS_KNEELUP1,			//# Get up from one knee?
	LEGS_CRLEAN_LEFT1,		//# Crouch Lean left
	LEGS_CRLEAN_RIGHT1,		//# Crouch Lean Right

	//# #eol
	MAX_ANIMATIONS,
} animNumber_t;

#define MAX_ANIM_SOUNDS 64

#ifndef CG_PLAYERS_CPP

extern stringID_table_t animTable [MAX_ANIMATIONS+1];

#else

stringID_table_t animTable [MAX_ANIMATIONS+1] =
{
	//=================================================
	//ANIMS IN WHICH UPPER AND LOWER OBJECTS ARE IN MD3
	//=================================================
	//# DEATHS
	ENUM2STRING(BOTH_DEATH1),			//# First Death anim
	ENUM2STRING(BOTH_DEATH2),			//# Second Death anim
	ENUM2STRING(BOTH_DEATH3),			//# Third Death anim
	ENUM2STRING(BOTH_DEATH4),			//# Fourth Death anim
	ENUM2STRING(BOTH_DEATH5),			//# Fifth Death anim
	ENUM2STRING(BOTH_DEATH6),			//# Sixth Death anim
	ENUM2STRING(BOTH_DEATH7),			//# Seventh Death anim

	ENUM2STRING(BOTH_DEATH1IDLE),		//# Idle while close to death

	ENUM2STRING(BOTH_DEATHFORWARD1),	//# First Death in which they get thrown forward
	ENUM2STRING(BOTH_DEATHFORWARD2),	//# Second Death in which they get thrown forward
	ENUM2STRING(BOTH_DEATHBACKWARD1),	//# First Death in which they get thrown backward
	ENUM2STRING(BOTH_DEATHBACKWARD2),	//# Second Death in which they get thrown backward
	ENUM2STRING(BOTH_LYINGDEATH1),		//# Death to play when killed lying down
	ENUM2STRING(BOTH_STUMBLEDEATH1),		//# Stumble forward and fall face first death
	ENUM2STRING(BOTH_FALLDEATH1),		//# Fall forward off a high cliff and splat death - start
	ENUM2STRING(BOTH_FALLDEATH1INAIR),	//# Fall forward off a high cliff and splat death - loop
	ENUM2STRING(BOTH_FALLDEATH1LAND),	//# Fall forward off a high cliff and splat death - hit bottom
	//# DEAD POSES # Should be last frame of corresponding previous anims
	ENUM2STRING(BOTH_DEAD1),			//# First Death finished pose
	ENUM2STRING(BOTH_DEAD2),			//# Second Death finished pose
	ENUM2STRING(BOTH_DEAD3),			//# Third Death finished pose
	ENUM2STRING(BOTH_DEAD4),			//# Fourth Death finished pose
	ENUM2STRING(BOTH_DEAD5),			//# Fifth Death finished pose
	ENUM2STRING(BOTH_DEAD6),			//# Sixth Death finished pose
	ENUM2STRING(BOTH_DEAD7),			//# Seventh Death finished pose
	ENUM2STRING(BOTH_DEADFORWARD1),		//# First thrown forward death finished pose
	ENUM2STRING(BOTH_DEADFORWARD2),		//# Second thrown forward death finished pose
	ENUM2STRING(BOTH_DEADBACKWARD1),	//# First thrown backward death finished pose
	ENUM2STRING(BOTH_DEADBACKWARD2),	//# Second thrown backward death finished pose
	ENUM2STRING(BOTH_LYINGDEAD1),		//# Killed lying down death finished pose
	ENUM2STRING(BOTH_STUMBLEDEAD1),		//# Stumble forward death finished pose
	ENUM2STRING(BOTH_FALLDEAD1LAND),	//# Fall forward and splat death finished pose
	//# #sep BOTH_ DEAD TWITCH/FLOP # React to being shot from death poses
	ENUM2STRING(BOTH_DEAD1_FLOP),		//# React to being shot from First Death finished pose
	ENUM2STRING(BOTH_DEAD2_FLOP),		//# React to being shot from Second Death finished pose
	ENUM2STRING(BOTH_DEAD3_FLOP),		//# React to being shot from Third Death finished pose
	ENUM2STRING(BOTH_DEAD4_FLOP),		//# React to being shot from Fourth Death finished pose
	ENUM2STRING(BOTH_DEAD5_FLOP),		//# React to being shot from Fifth Death finished pose 
	ENUM2STRING(BOTH_DEADFORWARD1_FLOP),		//# React to being shot First thrown forward death finished pose
	ENUM2STRING(BOTH_DEADFORWARD2_FLOP),		//# React to being shot Second thrown forward death finished pose
	ENUM2STRING(BOTH_DEADBACKWARD1_FLOP),	//# React to being shot First thrown backward death finished pose
	ENUM2STRING(BOTH_DEADBACKWARD2_FLOP),	//# React to being shot Second thrown backward death finished pose
	ENUM2STRING(BOTH_LYINGDEAD1_FLOP),		//# React to being shot Killed lying down death finished pose
	ENUM2STRING(BOTH_STUMBLEDEAD1_FLOP),		//# React to being shot Stumble forward death finished pose
	ENUM2STRING(BOTH_FALLDEAD1_FLOP),	//# React to being shot Fall forward and splat death finished pose

	//# PAINS
	ENUM2STRING(BOTH_PAIN1),			//# First take pain anim
	ENUM2STRING(BOTH_PAIN2),			//# Second take pain anim
	ENUM2STRING(BOTH_PAIN3),			//# Third take pain anim
	ENUM2STRING(BOTH_PAIN4),			//# First take pain anim
	ENUM2STRING(BOTH_PAIN5),			//# Second take pain anim
	ENUM2STRING(BOTH_PAIN6),			//# Third take pain anim
	ENUM2STRING(BOTH_PAIN7),			//# First take pain anim
	ENUM2STRING(BOTH_PAIN8),			//# Second take pain anim

	//# ATTACKS
	ENUM2STRING(BOTH_ATTACK1),			//# Attack with generic 1-handed weapon
	ENUM2STRING(BOTH_ATTACK2),			//# Attack with generic 2-handed weapon
	ENUM2STRING(BOTH_ATTACK3),			//# 
	ENUM2STRING(BOTH_ATTACK4),			//# Attack with ???
	ENUM2STRING(BOTH_ATTACK5),			//# Attack with rocket launcher

	ENUM2STRING(BOTH_MELEE1),			//# First melee attack
	ENUM2STRING(BOTH_MELEE2),			//# Second melee attack
	ENUM2STRING(BOTH_MELEE3),			//# Third melee attack
	ENUM2STRING(BOTH_MELEE4),			//# Fourth melee attack
	ENUM2STRING(BOTH_MELEE5),			//# Fifth melee attack
	ENUM2STRING(BOTH_MELEE6),			//# Sixth melee attack

	//# STANDING
	ENUM2STRING(BOTH_STAND1),			//# Standing idle 1
	ENUM2STRING(BOTH_STAND1_RANDOM1),	//# Random standing idle
	ENUM2STRING(BOTH_STAND1_RANDOM2),	//# Random standing idle
	ENUM2STRING(BOTH_STAND1_RANDOM3),	//# Random standing idle
	ENUM2STRING(BOTH_STAND1_RANDOM4),	//# Random standing idle
	ENUM2STRING(BOTH_STAND1_RANDOM5),	//# Random standing idle
	ENUM2STRING(BOTH_STAND1_RANDOM6),	//# Random standing idle
	ENUM2STRING(BOTH_STAND1_RANDOM7),	//# Random standing idle
	ENUM2STRING(BOTH_STAND1_RANDOM8),	//# Random standing idle
	ENUM2STRING(BOTH_STAND1_RANDOM9),	//# Random standing idle
	ENUM2STRING(BOTH_STAND1_RANDOM10),	//# Random standing idle
	ENUM2STRING(BOTH_STAND1_RANDOM11),	//# Random standing idle
	ENUM2STRING(BOTH_STAND1_RANDOM12),	//# Random standing idle
	ENUM2STRING(BOTH_STAND1_RANDOM13),	//# Random standing idle
	ENUM2STRING(BOTH_STAND1_RANDOM14),	//# Random standing idle
	ENUM2STRING(BOTH_STAND2),			//# Standing idle 2
	ENUM2STRING(BOTH_STAND2_RANDOM1),	//# Random standing idle
	ENUM2STRING(BOTH_STAND2_RANDOM2),	//# Random standing idle
	ENUM2STRING(BOTH_STAND2_RANDOM3),	//# Random standing idle
	ENUM2STRING(BOTH_STAND2_RANDOM4),	//# Random standing idle

	ENUM2STRING(BOTH_STAND3),			//# Standing idle 3
	ENUM2STRING(BOTH_STAND4),			//# two handed, gun down, relaxed stand
	ENUM2STRING(BOTH_STAND5),			//# two handed, gun up, relaxed stand
	ENUM2STRING(BOTH_STAND6),			//# one handed, gun at side, relaxed stand
	ENUM2STRING(BOTH_STAND7),			//# (Chell) timid stance while looking around slightly and breathing
	ENUM2STRING(BOTH_STAND8),			//# breathing after exherting oneself one handed
	ENUM2STRING(BOTH_STAND9),			//# breathing after exherting oneself two handed
	ENUM2STRING(BOTH_STAND1TO3),		//# Transition from stand1 to stand3
	ENUM2STRING(BOTH_STAND3TO1),		//# Transition from stand3 to stand1

	ENUM2STRING(BOTH_STAND2TO4),		//# Transition from stand2 to stand4
	ENUM2STRING(BOTH_STAND4TO2),		//# Transition from stand4 to stand2
	ENUM2STRING(BOTH_STANDTOWALK1),		//# Transition from stand1 to walk1
	ENUM2STRING(BOTH_STANDTOCONSOLE1),	//# a transition from stand animations to console animations
	ENUM2STRING(BOTH_STANDUP1),			//# standing up and stumbling
	ENUM2STRING(BOTH_TALKGESTURE1),		//# standing up and talking
	ENUM2STRING(BOTH_TALKGESTURE2),		//# standing up and talking
	ENUM2STRING(BOTH_TALKGESTURE3),		//# standing up and talking

	ENUM2STRING(BOTH_HELP1),			//# helping hold injured4 man.

	ENUM2STRING(BOTH_LEAN1),			//# leaning on a railing
	ENUM2STRING(BOTH_LEAN1TODROPHELM),	//# transition from LEAN1 to DROPHELM

	ENUM2STRING(BOTH_CONSOLE1),			//# Using a waist-high console with both hands
	ENUM2STRING(BOTH_CONSOLE1IDLE),		//# Idle of CONSOLE1
	ENUM2STRING(BOTH_CONSOLE1RIGHT),	//# Reach right from CONSOLE1
	ENUM2STRING(BOTH_CONSOLE1LEFT),		//# Reach left from CONSOLE1
	ENUM2STRING(BOTH_CONSOLE2),			//# Using a head-high wall console with the right hand
	ENUM2STRING(BOTH_CONSOLE3),			//# arms parallel to ground and typing similar to con.1
	ENUM2STRING(BOTH_CONSOLE3IDLE),		//# arms parallel to ground and typing similar to con.1
	ENUM2STRING(BOTH_CONSOLE3RIGHT),	//# arms parallel to ground and typing similar to con.1
	ENUM2STRING(BOTH_CONSOLE3LEFT),		//# arms parallel to ground and typing similar to con.1
	ENUM2STRING(BOTH_CONSOLETOSTAND1),	//# a transition from console animations to stand animations

	ENUM2STRING(BOTH_GUARD_LOOKAROUND1),//# Cradling weapon and looking around
	ENUM2STRING(BOTH_GUARD_IDLE1),		//# Cradling weapon and standing
	ENUM2STRING(BOTH_GUARD_LKRT1),		//# cin17, quick glance right to sound of door slamming

	ENUM2STRING(BOTH_ALERT1),			//# Startled by something when on guard
	ENUM2STRING(BOTH_GESTURE1),			//# Generic gesture, non-specific
	ENUM2STRING(BOTH_GESTURE2),			//# Generic gesture, non-specific
	ENUM2STRING(BOTH_GESTURE3),			//# Generic gesture, non-specific

	ENUM2STRING(BOTH_CROWDLOOK1),		//# Person staring out into space 1
	ENUM2STRING(BOTH_CROWDLOOK2),		//# Person staring out into space 2
	ENUM2STRING(BOTH_CROWDLOOK3),		//# Person staring out into space 3
	ENUM2STRING(BOTH_CROWDLOOK4),		//# Person staring out into space 4
	ENUM2STRING(BOTH_GRAB1),			//# Grabbing something from table
	ENUM2STRING(BOTH_GRAB2),			//# Grabbing something 
	ENUM2STRING(BOTH_GRAB3),			//# Grabbing something 

	ENUM2STRING(BOTH_GRABBED1),			//# cin9.3 chell being grabbed 180 from munro, 28 pixels away
	ENUM2STRING(BOTH_GRABBED2),			//# cin9.3 idle grabbed 180 from munro, 28 pixels away

	ENUM2STRING(BOTH_SURPRISED1),		//# Surprised reaction 1
	ENUM2STRING(BOTH_SURPRISED2),		//# Surprised reaction 2
	ENUM2STRING(BOTH_SURPRISED3),		//# Surprised reaction 3
	ENUM2STRING(BOTH_SURPRISED4),		//# Surprised reaction 4
	ENUM2STRING(BOTH_SURPRISED5),		//# Surprised reaction 5

	ENUM2STRING(BOTH_SCARED1),			//# Scared reaction 1
	ENUM2STRING(BOTH_SCARED2),			//# Scared reaction 2
	ENUM2STRING(BOTH_CATCH1),			//# Reaching to catch something falling
	ENUM2STRING(BOTH_POSSESSED1),		//# 7 of 9 possessed
	ENUM2STRING(BOTH_POSSESSED2),		//# 7 of 9 possessed with hand out 

	ENUM2STRING(BOTH_SNAPTO1),			//# cin.23, 7o9 coming to from borg possession
	ENUM2STRING(BOTH_SNAPTO2),			//# cin.23, 7o9 coming to from borg possession2
	ENUM2STRING(BOTH_DROPANGERWEAP2),	//# cin.23, Nelson lowering weapon in anger
	ENUM2STRING(BOTH_SHOCK1),			//# telsia being zapped by electricity cinematic 9.2

	ENUM2STRING(BOTH_PSYCHICSHOCK1),	//# having visions of the boss
	ENUM2STRING(BOTH_PSYCHICSHOCK2),	//# having visions of the boss
	ENUM2STRING(BOTH_ASSIMILATED1),		//# Cin.18, Foster being assimilated by borg
	ENUM2STRING(BOTH_FALSEJUMP1),		//# Biessman pretending to jump down on Chell
	ENUM2STRING(BOTH_LAUGH1),			//# squat pose of Biessman laughing at Chell
	ENUM2STRING(BOTH_LAUGH2),			//# standing laugh of mocking Biessman
	ENUM2STRING(BOTH_ACTIVATEBELT1),	//# activating transport buffer on belt

	ENUM2STRING(BOTH_GROUNDSHAKE1),		//#	Bracing self when ground shakes beneath him
	ENUM2STRING(BOTH_GROUNDSHAKE2),		//#	Falling to knees and shileding self, then standing

	ENUM2STRING(BOTH_READYWEAPON1),		//# cin17, comes from greeting, just before fighting

	ENUM2STRING(BOTH_SPAWN1),			//# Spawning in to the world
	ENUM2STRING(BOTH_TALK1),			//# Generic talk anim

	ENUM2STRING(BOTH_COVERUP1_LOOP),	//# animation of getting in line of friendly fire
	ENUM2STRING(BOTH_COVERUP1_START),	//# transitions from stand to coverup1_loop
	ENUM2STRING(BOTH_COVERUP1_END),		//# transitions from coverup1_loop to stand
	ENUM2STRING(BOTH_HEROSTANCE1),		//# Biessman in the final shootout
	ENUM2STRING(BOTH_GUILT1),			//# Player has a guilty conscience after shooting a teammate.

	//# SITTING/CROUCHING
	ENUM2STRING(BOTH_SIT1STAND),		//# Stand up from First sitting anim
	ENUM2STRING(BOTH_SIT1TO2),			//# Trans from sit1 to sit2?
	ENUM2STRING(BOTH_SIT1TO3),			//# Trans from sit1 to sit3?
	ENUM2STRING(BOTH_SIT2TO1),			//# Trans from sit2 to sit1?
	ENUM2STRING(BOTH_SIT2TO3),			//# Trans from sit2 to sit3?
	ENUM2STRING(BOTH_SIT3TO1),			//# Trans from sit3 to sit1?
	ENUM2STRING(BOTH_SIT3TO2),			//# Trans from sit3 to sit2?

	ENUM2STRING(BOTH_SIT4TO5),			//# Trans from sit4 to sit5
	ENUM2STRING(BOTH_SIT4TO6),			//# Trans from sit4 to sit6
	ENUM2STRING(BOTH_SIT5TO4),			//# Trans from sit5 to sit4
	ENUM2STRING(BOTH_SIT5TO6),			//# Trans from sit5 to sit6
	ENUM2STRING(BOTH_SIT6TO4),			//# Trans from sit6 to sit4
	ENUM2STRING(BOTH_SIT6TO5),			//# Trans from sit6 to sit5

	ENUM2STRING(BOTH_SIT7),				//# sitting with arms over knees, no weapon
	ENUM2STRING(BOTH_SIT7TOSTAND1),		//# getting up from sit7 into stand1

	ENUM2STRING(BOTH_TABLE_EAT1),		//# Sitting at a table eating
	ENUM2STRING(BOTH_TABLE_CHEW1),		//# Sitting at a table chewing
	ENUM2STRING(BOTH_TABLE_WIPE1),		//# Sitting at a table wiping mouth
	ENUM2STRING(BOTH_TABLE_DRINK1),		//# Sitting at a table drinking
	ENUM2STRING(BOTH_TABLE_GETUP1),		//# Getting up from table
	ENUM2STRING(BOTH_TABLE_DEATH1),		//# Dying while sitting at a table
	ENUM2STRING(BOTH_TABLE_IDLE1),		//# Sitting at table breathing
	ENUM2STRING(BOTH_TABLE_TALKGESTURE1),	//# Sitting at table gesturing while talking
	ENUM2STRING(BOTH_TABLE_GESTURE1),	//# Sitting at table gesturing
	ENUM2STRING(BOTH_TABLE_GESTURE2),	//# Sitting at table gesturing

	ENUM2STRING(BOTH_CROUCH1),			//# Transition from standing to crouch
	ENUM2STRING(BOTH_CROUCH1IDLE),		//# Crouching idle
	ENUM2STRING(BOTH_CROUCH1WALK),		//# Walking while crouched
	ENUM2STRING(BOTH_UNCROUCH1),		//# Transition from crouch to standing
	ENUM2STRING(BOTH_CROUCH2IDLE),		//# crouch and resting on back righ heel, no weapon
	ENUM2STRING(BOTH_CROUCH2TOSTAND1),	//# going from crouch2 to stand1
	ENUM2STRING(BOTH_GET_UP1),			//# Get up from ground, face down
	ENUM2STRING(BOTH_GET_UP2),			//# Get up from ground, face up

	ENUM2STRING(BOTH_BENCHSIT1_IDLE),	//# sitting on haz-locker room benches
	ENUM2STRING(BOTH_BENCHSIT1TO2),		//# Trans from benchsit1 to benchsit2
	ENUM2STRING(BOTH_BENCHSIT2TO1),		//# Trans from benchsit2 to benchsit1
	ENUM2STRING(BOTH_BENCHSIT2STAND),	//# Trans from benchsit to standing
	ENUM2STRING(BOTH_BENCHSIT2_IDLE),	//# sitting on haz-locker room benches
	ENUM2STRING(BOTH_BENCHSIT1_2STAND),	//# getting up to stand from sitting on haz-benches
	ENUM2STRING(BOTH_BENCHSIT1_FIXBOOT),//# sitting on bench - pulling on/adjusting boot top
	ENUM2STRING(BOTH_BENCHSTAND1TO2),	//# transition from stand to benchstand2
	ENUM2STRING(BOTH_BENCHSTAND2),		//# standing with right foot up on bench
	ENUM2STRING(BOTH_BENCHSTAND2TO1),	//# transition from benchstand2 to stand

	ENUM2STRING(BOTH_COUCHSIT1_IDLE),	//# sitting in couch - haz lounge area
	ENUM2STRING(BOTH_COUCHSIT1_TO2),	//# sitting in couch - lean back to 2nd position
	ENUM2STRING(BOTH_COUCHSIT1_2STAND1),//# getting up from couchsit1 to stand1
	ENUM2STRING(BOTH_COUCHSIT1_TALKGESTURE),//# sitting in couch - talking with hands
	ENUM2STRING(BOTH_COUCHSIT1_GESTURELEFT),//# sitting in couch - talk gesture to the left
	ENUM2STRING(BOTH_COUCHSIT1_GESTURERIGHT),//# sitting in couch - talk gesture to the right
	ENUM2STRING(BOTH_KNEELHAND1),			//# Jurot puts hand to Munro's face, then pulls away
	ENUM2STRING(BOTH_HALT1),			//# munro being grabbed by telsia before going in core room

	//# MOVING
	ENUM2STRING(BOTH_WALK1),			//# Normal walk
	ENUM2STRING(BOTH_WALK2),			//# Normal walk
	ENUM2STRING(BOTH_WALK3),			//# Goes with stand 3
	ENUM2STRING(BOTH_WALK4),			//# Walk cycle goes to a stand4
	ENUM2STRING(BOTH_WALKTORUN1),		//# transition from walk to run

	ENUM2STRING(BOTH_RUN1),				//# Full run
	ENUM2STRING(BOTH_RUN1START),		//# Start into full run1
	ENUM2STRING(BOTH_RUN1STOP),			//# Stop from full run1
	ENUM2STRING(BOTH_RUN2),				//# Full run
	ENUM2STRING(BOTH_RUNINJURED1),		//# Run with injured left leg
	ENUM2STRING(BOTH_STRAFE_LEFT1),		//# Sidestep left, should loop
	ENUM2STRING(BOTH_STRAFE_RIGHT1),	//# Sidestep right, should loop
	ENUM2STRING(BOTH_TURN_LEFT1),		//# Turn left, should loop
	ENUM2STRING(BOTH_TURN_RIGHT1),		//# Turn right, should loop
	ENUM2STRING(BOTH_RUNAWAY1),			//# Runningf scared
	ENUM2STRING(BOTH_SWIM1),			//# Swimming
	ENUM2STRING(BOTH_JUMP1),			//# Jump - wind-up and leave ground
	ENUM2STRING(BOTH_INAIR1),			//# In air loop (from jump)
	ENUM2STRING(BOTH_LAND1),			//# Landing (from in air loop)
	ENUM2STRING(BOTH_LAND2),			//# Landing Hard (from a great height)

	ENUM2STRING(BOTH_JUMPBACK1),		//# Jump - wind-up and leave ground
	ENUM2STRING(BOTH_INAIRBACK1),		//# In air loop (from jump)
	ENUM2STRING(BOTH_LANDBACK1),		//# Landing (from in air loop)
	ENUM2STRING(BOTH_DIVE1),			//# Dive!
	ENUM2STRING(BOTH_ROLL1_LEFT),		//# Roll to left side
	ENUM2STRING(BOTH_ROLL1_RIGHT),		//# Roll to right side
	ENUM2STRING(BOTH_LADDER_UP1),		//# Climbing up a ladder with rungs at 16 unit intervals
	ENUM2STRING(BOTH_LADDER_DWN1),		//# Climbing down a ladder with rungs at 16 unit intervals
	ENUM2STRING(BOTH_LADDER_IDLE),		//# Holding onto ladder
	ENUM2STRING(BOTH_ONLADDER_BOT1),		//# Getting on the ladder at the bottom
	ENUM2STRING(BOTH_OFFLADDER_BOT1),	//# Getting off the ladder at the bottom
	ENUM2STRING(BOTH_ONLADDER_TOP1),		//# Getting on the ladder at the top
	ENUM2STRING(BOTH_OFFLADDER_TOP1),	//# Getting off the ladder at the top
	ENUM2STRING(BOTH_LIFT1),				//# Lifting someone/thing over their shoulder
	ENUM2STRING(BOTH_STEP1),				//# telsia checking out lake cinematic9.2
	ENUM2STRING(BOTH_HITWALL1),				//# cin.18, Kenn hit by borg into wall 56 units away
	ENUM2STRING(BOTH_AMBUSHLAND1),		//# landing from fall on victim
	ENUM2STRING(BOTH_BIRTH1),			//# birth from jumping through walls

	ENUM2STRING(BOTH_SHIELD1),			//# cin.6, munro's initial reaction to explosion
	ENUM2STRING(BOTH_SHIELD2),			//# cin.6, munro in shielding position looping
	ENUM2STRING(BOTH_WALKPUSH1),		//# man pushing crate
	ENUM2STRING(BOTH_PUSHTOSTAND1),		//# man coming from pushing crate to stand1

	//# FLY - IDLE
	ENUM2STRING(BOTH_FLY_IDLE1),			//#  Idle while flying
	ENUM2STRING(BOTH_FLY_IDLE2),			//#  Idle while flying

	//# FLY - MOVING
	ENUM2STRING(BOTH_FLY_START1),			//#  Start flying
	ENUM2STRING(BOTH_FLY_STOP1),			//#  Stop flying
	ENUM2STRING(BOTH_FLY_LOOP1),			//#  Normal flying, should loop

	ENUM2STRING(BOTH_FLOAT1),				//#  Crew floating through space 1
	ENUM2STRING(BOTH_FLOAT2),				//#  Crew floating through space 2
	ENUM2STRING(BOTH_FLOATCONSOLE1),		//#  Crew floating and working on console

	//# LYING
	ENUM2STRING(BOTH_LIE_DOWN1),
	ENUM2STRING(BOTH_LIE_DOWN2),
	ENUM2STRING(BOTH_LIE_DOWN3),			//# reaction to local disnode being destroyed

	ENUM2STRING(BOTH_PAIN2WRITHE1),		//# Transition from upright position to writhing on ground anim
	ENUM2STRING(BOTH_PRONE2RLEG),		//# Lying on ground reach to grab right leg
	ENUM2STRING(BOTH_PRONE2LLEG),		//# Lying on ground reach to grab left leg
	ENUM2STRING(BOTH_WRITHING1),			//# Lying on ground writhing in pain
	ENUM2STRING(BOTH_WRITHING1RLEG),		//# Lying on ground writhing in pain, holding right leg
	ENUM2STRING(BOTH_WRITHING1LLEG),		//# Lying on ground writhing in pain, holding left leg
	ENUM2STRING(BOTH_WRITHING2),		//# Lying on ground writhing in pain in a different way

	ENUM2STRING(BOTH_INJURED1),			//# Lying down), against wall - can also be sleeping
	ENUM2STRING(BOTH_INJURED2),			//# Injured pose 2
	ENUM2STRING(BOTH_INJURED3),			//# Injured pose 3
	ENUM2STRING(BOTH_INJURED4),			//# Injured pose 4
	ENUM2STRING(BOTH_INJURED4TO5),		//# Transition from INJURED4 to INJURED5
	ENUM2STRING(BOTH_INJURED5),			//# Injured pose 5
	ENUM2STRING(BOTH_INJURED6),			//# Injured pose 5
	ENUM2STRING(BOTH_INJURED6ATTACKSTART),	//# Start attack while in injured 6 pose 
	ENUM2STRING(BOTH_INJURED6ATTACKSTOP),	//# End attack while in injured 6 pose

	ENUM2STRING(BOTH_INJURED6COMBADGE),	//# Hit combadge while in injured 6 pose

	ENUM2STRING(BOTH_INJURED6POINT),	//# Chang points to door while in injured state

	ENUM2STRING(BOTH_INJUREDTOSTAND1),	//# Runinjured to stand1

	ENUM2STRING(BOTH_CRAWLBACK1),		//# Lying on back), crawling backwards with elbows
	ENUM2STRING(BOTH_SITWALL1),			//# Sitting against a wall
	ENUM2STRING(BOTH_SLEEP1),			//# laying on back-rknee up-rhand on torso
	ENUM2STRING(BOTH_SLEEP2),			//# on floor-back against wall-arms crossed
	ENUM2STRING(BOTH_SLEEP3),			//# Sleeping in a chair
	ENUM2STRING(BOTH_SLEEP4),			//# Slumped over table
	ENUM2STRING(BOTH_SLEEP5),			//# Laying on side sleeping on flat sufrace
	ENUM2STRING(BOTH_SLEEP1GETUP),		//# alarmed and getting up out of sleep1 pose to stand
	ENUM2STRING(BOTH_SLEEP1GETUP2),		//# 
	ENUM2STRING(BOTH_SLEEP2GETUP),		//# alarmed and getting up out of sleep2 pose to stand
	ENUM2STRING(BOTH_SLEEP3GETUP),		//# alarmed and getting up out of sleep3 pose to stand
	ENUM2STRING(BOTH_SLEEP3DEATH),		//# death in chair, from sleep3 idle
	ENUM2STRING(BOTH_SLEEP3DEAD),		//# death in chair, from sleep3 idle

	ENUM2STRING(BOTH_SLEEP_IDLE1),		//# rub face and nose while asleep
	ENUM2STRING(BOTH_SLEEP_IDLE2),		//# shift position while asleep - stays in sleep2
	ENUM2STRING(BOTH_SLEEP_IDLE3),		//# Sleep idle 3
	ENUM2STRING(BOTH_SLEEP_IDLE4),		//# Sleep idle 4
	ENUM2STRING(BOTH_SLEEP1_NOSE),		//# Scratch nose from SLEEP1 pose
	ENUM2STRING(BOTH_SLEEP2_SHIFT),		//# Shift in sleep from SLEEP2 pose
	ENUM2STRING(BOTH_RESTRAINED1),		//# Telsia tied to medical table
	ENUM2STRING(BOTH_RESTRAINED1POINT),	//# Telsia tied to medical table pointing at Munro

	ENUM2STRING(BOTH_LIFTED1),			//#
	ENUM2STRING(BOTH_CARRIED1),			//# Fits with TORSO_CARRY1, carried over shoulder
	ENUM2STRING(BOTH_CARRIED2),			//# Laying over object


	//# BORG-SPECIFIC
	ENUM2STRING(BOTH_PLUGIN1),			//# Borg plugs self in to alcove
	ENUM2STRING(BOTH_PLUGGEDIN1),		//# Last frame of Borg plug in sequence
	ENUM2STRING(BOTH_PLUGOUT1),			//# Borg unplugs self from alcove
	//# HUNTER-SEEKER BOT-SPECIFIC
	ENUM2STRING(BOTH_POWERUP1),			//# Wakes up

	//=================================================
	//ANIMS IN WHICH ONLY THE UPPER OBJECTS ARE IN MD3
	//=================================================
	//# WEAPON-RELATED
	ENUM2STRING(TORSO_DROPWEAP1),		//# Put weapon away
	ENUM2STRING(TORSO_DROPWEAP2),		//# Put weapon away
	ENUM2STRING(TORSO_DROPWEAP3),		//# Put weapon away
	ENUM2STRING(TORSO_RAISEWEAP1),		//# Draw Weapon
	ENUM2STRING(TORSO_RAISEWEAP2),		//# Draw Weapon
	ENUM2STRING(TORSO_RAISEWEAP3),		//# Draw Weapon
	ENUM2STRING(TORSO_WEAPONREADY1),	//# Ready to fire 1 handed weapon
	ENUM2STRING(TORSO_WEAPONREADY2),	//# Ready to fire 2 handed weapon
	ENUM2STRING(TORSO_WEAPONREADY3),	//# Ready to fire 2 handed weapon
	ENUM2STRING(TORSO_WEAPONIDLE1),		//# Holding 1 handed weapon
	ENUM2STRING(TORSO_WEAPONIDLE2),		//# Holding 2 handed weapon
	ENUM2STRING(TORSO_WEAPONIDLE3),		//# Holding 2 handed weapon
	//# USING NON-WEAPON OBJECTS
	ENUM2STRING(TORSO_TRICORDER1),		//# Using a tricorder
	ENUM2STRING(TORSO_MEDICORDER1),		//# Using a Medical Tricorder
	ENUM2STRING(TORSO_PADD1),			//# Using a PADD

	ENUM2STRING(TORSO_EQUIPMENT1),		//# Twisting pipe with both hands
	ENUM2STRING(TORSO_EQUIPMENT2),		//# Fidgiting with cylinder with both hands
	ENUM2STRING(TORSO_EQUIPMENT3),		//# Using equipment one handed
	ENUM2STRING(TORSO_WRIST1),			//# cin.24, Chang detonating bomb with wrist device


	//# MISC
	ENUM2STRING(TORSO_COMBADGE1),			//# Right hand to left breast
	ENUM2STRING(TORSO_COMBADGE2),			//# Left hand to left breast
	ENUM2STRING(TORSO_COMBADGE3),			//# Combadge touch from stand4

	ENUM2STRING(TORSO_REDALERT1),			//# Hitting comm button on wall with hand (Kirk-like)
	ENUM2STRING(TORSO_HANDGESTURE1),	//# gestures to left one hand
	ENUM2STRING(TORSO_HANDGESTURE2),	//# gestures to right one hand
	ENUM2STRING(TORSO_HANDGESTURE3),	//# gestures to the left both hands
	ENUM2STRING(TORSO_HANDGESTURE4),	//# gestures to the right both hands
	ENUM2STRING(TORSO_HANDGESTURE5),	//# ?
	ENUM2STRING(TORSO_HANDGESTURE6),	//# pointing (flank right) while talking & holding a weapon
	ENUM2STRING(TORSO_HANDGESTURE7),	//# pointing (forward) while talking & holding a weapon
	ENUM2STRING(TORSO_HANDGESTURE8),	//# pointing (flank left) while talking & holding a weapon
	ENUM2STRING(TORSO_HANDGESTURE9),	//# quick point right from stand 4
	ENUM2STRING(TORSO_HANDGESTURE10),	//# quick point forward from stand 4
	ENUM2STRING(TORSO_HANDGESTURE11),	//# quick point left from stand 4
	ENUM2STRING(TORSO_HANDGESTURE12),	//# gesturing with both hands forward
	ENUM2STRING(TORSO_HANDGESTURE13),	//# gesturing a shrug as if not knowing answer

	ENUM2STRING(TORSO_HEADNOD1),	//# nod in affirmation
	ENUM2STRING(TORSO_HEADSHAKE1),	//# head goes down while shaking left and right in dissapointment

	ENUM2STRING(TORSO_HYPOSPRAY1),		//# man giving hypo to people
	ENUM2STRING(TORSO_HYPOSPRAY4),		//# using hypospray on telsia in scav5

	ENUM2STRING(TORSO_HANDEXTEND1),		//# doctor reaching for hypospray in scav5
	ENUM2STRING(TORSO_HANDRETRACT1),		//# doctor taking hypospray from player in scav5

	ENUM2STRING(TORSO_DROPHELMET1),		//# Drop the helmet to the waist
	ENUM2STRING(TORSO_RAISEHELMET1),	//# Bring the helmet to the head
	ENUM2STRING(TORSO_REACHHELMET1),	//# reaching for helmet off of 60 tall cabinet
	ENUM2STRING(TORSO_GRABLBACKL),		//# reach to lower back with left hand
	ENUM2STRING(TORSO_GRABUBACKL),		//# reach to upper back with left hand
	ENUM2STRING(TORSO_GRABLBACKR),		//# reach to lower back with right hand
	ENUM2STRING(TORSO_GRABUBACKR),		//# reach to upper back with right hand

	ENUM2STRING(TORSO_STAND2TOWEAPONREADY2),	//# cin.23, Nelson raising weapon in alarm and ready to fire

	ENUM2STRING(TORSO_HAND1),			//# Exchanging items - giver
	ENUM2STRING(TORSO_HAND2),			//# Exchanging items - receiver

	ENUM2STRING(TORSO_POKERIDLE1),		//# holding cards
	ENUM2STRING(TORSO_POKERIDLE2),		//# re-arranging cards
	ENUM2STRING(TORSO_POKERIDLE3),		//# put card on table

	ENUM2STRING(TORSO_SPEECHLESS1),		//# hanging head in grief 1
	ENUM2STRING(TORSO_SPEECHLESS2),		//# hanging head in grief 2
	ENUM2STRING(TORSO_SHOUT1),			//# left hand to mouth
	ENUM2STRING(TORSO_CARRY1),			//#	Carrying someone/thing over their shoulder (can go from BOTH_LIFT1)

	//=================================================
	//ANIMS IN WHICH ONLY THE LOWER OBJECTS ARE IN MD3
	//=================================================
	ENUM2STRING(LEGS_WALKBACK1),			//# Walk1 backwards
	ENUM2STRING(LEGS_WALKBACK2),			//# Walk2 backwards
	ENUM2STRING(LEGS_RUNBACK1),			//# Run1 backwards
	ENUM2STRING(LEGS_RUNBACK2),			//# Run2 backwards
	ENUM2STRING(LEGS_TURN1),				//# What legs do when you turn your lower body to match your upper body facing
	ENUM2STRING(LEGS_TURN2),				//# Leg turning from stand2
	ENUM2STRING(LEGS_LEAN_LEFT1),		//# Lean left
	ENUM2STRING(LEGS_LEAN_RIGHT1),		//# Lean Right
	ENUM2STRING(LEGS_KNEELDOWN1),		//# Get down on one knee?
	ENUM2STRING(LEGS_KNEELUP1),			//# Get up from one knee?

	ENUM2STRING(LEGS_CRLEAN_LEFT1),			//# Crouch Lean left
	ENUM2STRING(LEGS_CRLEAN_RIGHT1),		//# Crouch Lean Right
	//must be terminated
	NULL,-1
};

#endif// #ifndef CG_PLAYER_CPP

typedef struct
{
	char			filename[MAX_QPATH];
	animation_t		animations[MAX_ANIMATIONS];
	animsounds_t	torsoAnimSnds[MAX_ANIM_SOUNDS];
	animsounds_t	legsAnimSnds[MAX_ANIM_SOUNDS];
	qboolean		soundsCached;
} animFileSet_t;
#define MAX_ANIM_FILES	32
extern animFileSet_t	knownAnimFileSets[MAX_ANIM_FILES];
extern int				numKnownAnimFileSets;

#endif// #ifndef __ANIMS_H__

