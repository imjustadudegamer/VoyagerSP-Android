// define GAME_INCLUDE so that g_public.h does not define the
// short, server-visible gclient_t and gentity_t structures,
// because we define the full size ones in this file
#define GAME_INCLUDE
#include "q_shared.h"
#include "g_shared.h"
#include "bg_local.h"
#include "anims.h"

extern pmove_t		*pm;
extern pml_t		pml;

extern void PM_SetAnim(pmove_t	*pm,int setAnimParts,int anim,int setAnimFlags);
void BG_CalculateOffsetAngles( gentity_t *ent, usercmd_t *ucmd )
{
	if ( !ent->client )
		return;

	//These are the values we can set that will lock the legs and torso in place
	if(ucmd->forwardmove || ucmd->rightmove)
	{//When moving, always face all parts toward angles
		//FIXME: What about strafing?
		ent->client->renderInfo.torsoYawLockMode = LM_NONE;	// center to catch up with head
		ent->client->renderInfo.torsoPitchLockMode = LM_NONE;	// center to catch up with head
		ent->client->renderInfo.legsYawLockMode = LM_NONE;	// center to catch up with head
	}
	else
	{
		// if standing still, only point all in the same direction if go beyond swing tolerance
		// allow yaw to drift a bit
		ent->client->renderInfo.torsoYawLockMode = LM_SWING;	
		ent->client->renderInfo.torsoPitchLockMode = LM_SWING;	
		ent->client->renderInfo.legsYawLockMode = LM_SWING;	
	}

	if(!ent->NPC)
	{
		return;
	}

	//Additional mods to angles based on lookMode, if any
	switch( ent->NPC->lookMode )
	{
	case LT_FULLFACE:
		//Turn every part to full face the angles
		ent->client->renderInfo.torsoYawLockMode = LM_NONE;	// center to catch up with head
		ent->client->renderInfo.torsoPitchLockMode = LM_NONE;	// center to catch up with head
		ent->client->renderInfo.legsYawLockMode = LM_NONE;	// center to catch up with head
		return;
		break;

	case LT_AIMSWING:
		//Turn torso and head to point, allow legs to swing
		ent->client->renderInfo.torsoYawLockMode = LM_NONE;	// center to catch up with head
		ent->client->renderInfo.torsoPitchLockMode = LM_NONE;	// center to catch up with head
		ent->client->renderInfo.legsYawLockMode = LM_SWING;	// center to catch up with head
		return;
		break;

	case LT_AIM:
		//Turn torso and head to point, clamp legs
		ent->client->renderInfo.torsoYawLockMode = LM_NONE;	// center to catch up with head
		ent->client->renderInfo.torsoPitchLockMode = LM_NONE;	// center to catch up with head
		ent->client->renderInfo.legsYawLockMode = LM_CLAMP;	// center to catch up with head
		return;
		break;

	case LT_NORMAL:
		//Turn all toward direction, allow soft swing
		ent->client->renderInfo.torsoYawLockMode = LM_SWING;	// center to catch up with head
		ent->client->renderInfo.torsoPitchLockMode = LM_SWING;	// center to catch up with head
		ent->client->renderInfo.legsYawLockMode = LM_SWING;	// center to catch up with head
		return;
		break;

	case LT_AIMSOFT:
		//lock legs, turn tosro to general head angles
		ent->client->renderInfo.torsoYawLockMode = LM_SWING;	// center to catch up with head
		ent->client->renderInfo.torsoPitchLockMode = LM_SWING;	// center to catch up with head
		ent->client->renderInfo.legsYawLockMode = LM_CLAMP;	// center to catch up with head
		return;
		break;

	case LT_GLANCE://Turn head only if possible
		ent->client->renderInfo.torsoYawLockMode = LM_CLAMP;	// center to catch up with head
		ent->client->renderInfo.torsoPitchLockMode = LM_CLAMP;	// center to catch up with head
		ent->client->renderInfo.legsYawLockMode = LM_CLAMP;	// center to catch up with head
		return;
		break;

	case LT_NONE:
	default:
		//use the bstate below
		break;
	}

	switch((int)ent->NPC->behaviorState)
	{
	case BS_FACE://Completely Face a new direction
	case BS_POINT_AND_SHOOT://Aim at enemy and fire
	case BS_FACE_ENEMY://Aim at enemy, don't fire
	case BS_RUN_AND_SHOOT:
	case BS_HUNT_AND_KILL:
		ent->client->renderInfo.torsoYawLockMode = LM_NONE;	// center to catch up with head
		ent->client->renderInfo.torsoPitchLockMode = LM_NONE;	// center to catch up with head
		ent->client->renderInfo.legsYawLockMode = LM_NONE;	// center to catch up with head
		break;

	case BS_STAND_AND_SHOOT://Stay in one spot and shoot- duck when neccesary
	case BS_STAND_GUARD://Wait around for an enemy
	case BS_AIM:
	//case BS_COVER://35: Watch your coverTarg and shoot any enemy around him, laying down supressing fire
		//Only aim torso/head at facing
		ent->client->renderInfo.legsYawLockMode = LM_CLAMP;	//Keep legs facing if possible
		break;

	case BS_LOOK:
	case BS_SAY://36: Turn to sayTarg, use talk anim, say your sayString (look up string in your sounds table), exit tempBState when sound finished (anim of mouth should be timed to length of sound as well)
		//Only look at facing with head
		ent->client->renderInfo.torsoYawLockMode = LM_CLAMP;	//Keep torso facing if possible
		ent->client->renderInfo.torsoPitchLockMode = LM_CLAMP;	//Keep torso facing if possible
		ent->client->renderInfo.legsYawLockMode = LM_CLAMP;	//Keep legs facing if possible
		break;

	case BS_FORMATION://Maintain a formation
		if(!ucmd->forwardmove && !ucmd->rightmove)
		{//Just standing around
			ent->client->renderInfo.legsYawLockMode = LM_CLAMP;	//Keep legs facing if possible
		}
		
		if(!(ucmd->buttons & BUTTON_ATTACK))
		{//Looking around, not shooting
			ent->client->renderInfo.torsoYawLockMode = LM_SWING;	//Keep torso facing if possible
			ent->client->renderInfo.torsoPitchLockMode = LM_SWING;	//Keep torso facing if possible
		}
		break;

	case BS_EVADE_AND_SHOOT://Run from enemies, shoot them if they hit you
	case BS_MOVE://move in one dir, face another...
	//case BS_RETREAT://34: Back away while still engaging enemy
		//Does this mean shoot in one dir and move in another?
		//???
		break;

	default:
		//No changes
		break;
	}
}

/*
================
PM_UpdateViewAngles

This can be used as another entry point when only the viewangles
are being updated isntead of a full move

//FIXME: Now that they pmove twice per think, they snap-look really fast
================
*/
void PM_UpdateViewAngles( playerState_t *ps, usercmd_t *cmd, gentity_t *gent ) 
{
	short		temp, pitchClampMin, pitchClampMax;
	float		pitchMin=-75, pitchMax=75, yawMin=0, yawMax=0;	//just to shut up warnings
	int			i;
	vec3_t		start, end, tmins, tmaxs, right;
	trace_t		trace;
	qboolean	lockedYaw = qfalse;

	if ( ps->pm_type == PM_INTERMISSION ) 
	{
		return;		// no view changes at all
	}

	if ( ps->pm_type != PM_SPECTATOR && ps->stats[STAT_HEALTH] <= 0 ) 
	{
		return;		// no view changes at all
	}

	if ( ps->eFlags & EF_NPC && gent != NULL && gent->client != NULL )
	{
		if(gent->client->renderInfo.renderFlags & RF_LOCKEDANGLE)
		{
			pitchMin = 0 - gent->client->renderInfo.headPitchRangeUp - gent->client->renderInfo.torsoPitchRangeUp;
			pitchMax = gent->client->renderInfo.headPitchRangeDown + gent->client->renderInfo.torsoPitchRangeDown;
			
			yawMin = 0 - gent->client->renderInfo.headYawRangeLeft - gent->client->renderInfo.torsoYawRangeLeft;
			yawMax = gent->client->renderInfo.headYawRangeRight + gent->client->renderInfo.torsoYawRangeRight;

			lockedYaw = qtrue;
		}
		else
		{
			pitchMin = -gent->client->renderInfo.headPitchRangeUp;
			pitchMax = gent->client->renderInfo.headPitchRangeDown;
		}
	}

	pitchClampMin = ANGLE2SHORT(pitchMin);
	pitchClampMax = ANGLE2SHORT(pitchMax);

	// circularly clamp the angles with deltas
	for (i=0 ; i<3 ; i++) 
	{
		temp = cmd->angles[i] + ps->delta_angles[i];
		if ( i == PITCH ) 
		{
			//FIXME get this limit from the NPCs stats?
			// don't let the player look up or down more than 90 degrees
			if ( temp > pitchClampMax ) 
			{
				ps->delta_angles[i] = pitchClampMax - cmd->angles[i];
				temp = pitchClampMax;
			} 
			else if ( temp < pitchClampMin ) 
			{
				ps->delta_angles[i] = pitchClampMin - cmd->angles[i];
				temp = pitchClampMin;
			}
		}
		//FIXME: Are we losing precision here?  Is this why it jitters?
		ps->viewangles[i] = SHORT2ANGLE(temp);

		if ( i == YAW && lockedYaw) 
		{
			// don't let the player look left or right more than the clamp, if any
			if ( AngleSubtract(ps->viewangles[i], gent->client->renderInfo.lockYaw) > yawMax ) 
			{
				ps->viewangles[i] = yawMax;
			} 
			else if ( AngleSubtract(ps->viewangles[i], gent->client->renderInfo.lockYaw) < yawMin ) 
			{
				ps->viewangles[i] = yawMin;
			}
		}
	}

	if ( (cmd->buttons & BUTTON_USE) && cmd->rightmove != 0 && !cmd->forwardmove && cmd->upmove <= 0 )
	{//Only lean if holding use button, strafing and not moving forward or back and not jumping
		if ( gent )
		{
			int leanofs = 0;
			vec3_t	viewangles;

			if ( cmd->rightmove > 0 )
			{
				if((pm->ps->legsAnim&~ANIM_TOGGLEBIT) != LEGS_LEAN_RIGHT1)
				{
					PM_SetAnim(pm, SETANIM_LEGS, LEGS_LEAN_RIGHT1, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
				}
				pm->ps->legsAnimTimer = 500;//Force it to hold the anim for at least half a sec

				if ( ps->leanofs <= 28 )
				{
					leanofs = ps->leanofs + 4;
				}
				else
				{
					leanofs = 32;
				}
			}
			else
			{
				if ( (pm->ps->legsAnim&~ANIM_TOGGLEBIT) != LEGS_LEAN_LEFT1 )
				{
					PM_SetAnim(pm, SETANIM_LEGS, LEGS_LEAN_LEFT1, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
				}
				pm->ps->legsAnimTimer = 500;//Force it to hold the anim for at least half a sec

				if ( ps->leanofs >= -28 )
				{
					leanofs = ps->leanofs - 4;
				}
				else
				{
					leanofs = -32;
				}
			}

			VectorCopy( ps->origin, start );
			start[2] += ps->viewheight;
			VectorCopy( ps->viewangles, viewangles );
			viewangles[ROLL] = 0;
			AngleVectors( ps->viewangles, NULL, right, NULL );
			VectorNormalize( right );
			right[2] = (leanofs<0)?0.25:-0.25;
			VectorMA( start, leanofs, right, end );
			VectorSet( tmins, -8, -8, -4 );
			VectorSet( tmaxs, 8, 8, 4 );
			//if we don't trace EVERY frame, can TURN while leaning and
			//end up leaning into solid architecture (sigh)
			gi.trace( &trace, start, tmins, tmaxs, end, gent->s.number, MASK_PLAYERSOLID );

			ps->leanofs = floor((float)leanofs * trace.fraction);

			ps->leanStopDebounceTime = 20;
		}
	}
	else
	{
		if ( gent && (cmd->forwardmove || cmd->upmove > 0) )
		{
			if( ((pm->ps->legsAnim&~ANIM_TOGGLEBIT) == LEGS_LEAN_RIGHT1) ||
				((pm->ps->legsAnim&~ANIM_TOGGLEBIT) == LEGS_LEAN_LEFT1) )
			{
				pm->ps->legsAnimTimer = 0;//Force it to stop the anim
			}
		}

		if ( ps->leanofs > 0 )
		{
			//FIXME: play lean anim backwards?
			ps->leanofs-=4;
			if ( ps->leanofs < 0 )
			{
				ps->leanofs = 0;
			}
		}
		else if ( ps->leanofs < 0 )
		{
			//FIXME: play lean anim backwards?
			ps->leanofs+=4;
			if ( ps->leanofs > 0 )
			{
				ps->leanofs = 0;
			}
		}
	}

	if ( ps->leanStopDebounceTime )
	{
		ps->leanStopDebounceTime -= 1;
		cmd->rightmove = 0;
		cmd->buttons &= ~BUTTON_USE;
	}
}

