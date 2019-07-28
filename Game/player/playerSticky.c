#include <float.h>
#include "playerSticky.h"
#include "cmdcommon.h"
#include "entityRef.h"
#include "uiCursor.h"
#include "player.h"
#include "camera.h"
#include "position.h"
#include "edit_cmd.h"
#include "cmdgame.h"
#include "font.h"
#include "motion.h"
#include "entity.h"
#include "fx.h"
#include "pmotion.h"
#include "clientcomm.h"
#include "entVarUpdate.h"
#include "doorAnimClient.h"
#include "uiOptions.h"
#include "comm_game.h"

int useClickToMove();

static struct {
	Vec3		targetPos;
	F32			lastDistToTarget;
	int			failedMoveCount;
	int			activateTarget;
	GroupDef	*defTarget;
} follow_state;

static int playerHasValidFollowTarget()
{
	Entity* ent;
	ent = erGetEnt(control_state.followTarget);
	if(!ent 
		|| (ENTTYPE(ent) != ENTTYPE_PLAYER && ent->state.mode & ( 1 << ENT_DEAD ))
		|| (ent->seq && ent->seq->notPerceptible))
	{
		return 0;
	}
	return 1;
}

void playerToggleFollowMode(){
	// If the current target is not the same as the previous follow target,
	// assmue that "toggling" the follow mode means to switch over to following the
	// new target.
	if(control_state.followTarget != erGetRef(current_target)){
		playerStopFollowMode();
		playerSetFollowMode(current_target, NULL, NULL, 1, 0);	// Start following the selected target.
	}else{
		// Otherwise, if the selected target is the same as the current follow target or if there isn't a follow
		// target at all, really toggle the follow mode.

		// Do not engage follow mode unless there is a valid target to follow.
		if(!control_state.follow && !playerHasValidFollowTarget()){
			return;
		}
		
		playerSetFollowMode(current_target, NULL, NULL, !control_state.follow, 0);
	}
}

void playerStartForcedFollow(Packet *pak)
{
	// ARM NOTE: Oh god, how bad is this hack?
	Mat4 temp;
	temp[3][0] = pktGetF32(pak);
	temp[3][1] = pktGetF32(pak);
	temp[3][2] = pktGetF32(pak);

	playerSetFollowMode(NULL, temp, NULL, 1, 0);
	control_state.followForced = true;
}

void playerStopForcedFollow(void)
{
	Packet *pak = pktCreateEx(&comm_link, CLIENT_STOP_FORCED_MOVEMENT);
	pktSend(&pak,&comm_link);	

	control_state.followForced = false;
}

static void setTargetPosFX(int on, Mat4 matTarget){
	static int id;

	FxParams fxp;

	fxDelete(id, on ? HARD_KILL : SOFT_KILL);
	
	if(on)
	{
		Mat4 fxMat;
		Vec3 fxOffset;

		fxInitFxParams(&fxp);
		copyMat4(unitmat, fxp.keys[0].offset);
		fxp.keycount++;
		fxp.power = 10; //# from 1 to 10
		fxp.fxtype = FX_ENTITYFX; // We don't want this FX to go away if there are a lot onscreen

		id = fxCreate( "GENERIC/ClickSelection.fx", &fxp );
		
		copyMat4(matTarget, fxMat);
		
		if (distance3(cam_info.cammat[3], fxMat[3]) > 150)
		{
			F32 scale = 10 * (0.1 + fabs(dotVec3(fxMat[1], cam_info.cammat[2])));
			scaleVec3(fxMat[1], scale, fxOffset);
			addVec3(fxMat[3], fxOffset, fxMat[3]);
		}

		fxChangeWorldPosition(id, fxMat);
	}
}

int playerIsFollowing(Entity *entity)
{
	return control_state.follow && entity && (entity == erGetEnt(control_state.followTarget));
}

static Entity *lastEntFollowed = 0;
static GroupDef *lastDefFollowed = 0;


void playerSetFollowMode(Entity* entity, Mat4 matTarget, GroupDef *defTarget, int follow, int activateTarget){
	follow = follow ? 1 : 0;
	control_state.follow = control_state.follow ? 1 : 0;

	// Don't do anything if the current control states is the same as the requested state.

	if(	!follow && !control_state.follow ||
		follow && !entity && !matTarget && !defTarget)
	{
		return;
	}

	// Don't let them change anything if they are being force moved.
	if (control_state.followForced)
		return;

	lastEntFollowed = erGetEnt(control_state.followTarget);
	lastDefFollowed = follow_state.defTarget;

	follow_state.activateTarget = 0;
	follow_state.defTarget = 0;
	control_state.followTarget = 0;

	if(follow)
	{
		if(!control_state.follow)
		{
			control_state.cam_pyr_offset[0] = control_state.pyr.cur[0];
			control_state.pyr.cur[0] = 0;			
		}
		
		if (!optionGet(kUO_CamFree))
		{
			control_state.cam_pyr_offset[1] = 0;			
		}		

		cam_info.rotate_scale = 0.1;

		// Turn following mode on.
		if(entity)
		{
			control_state.followTarget = erGetRef(entity);
			control_state.autorun = 0;

			setTargetPosFX(0, NULL);
		}
		else
		{
			copyVec3(matTarget[3], follow_state.targetPos);
			follow_state.lastDistToTarget = FLT_MAX;
			follow_state.failedMoveCount = 0;
			
			if (!defTarget)
				setTargetPosFX(1, matTarget);
			else if (activateTarget)
				follow_state.defTarget = defTarget;
		}

		follow_state.activateTarget = activateTarget;
		
		control_state.follow = 1;
		control_state.followMovementCount = control_state.movementControlUpdateCount;
	}
	else
	{
		control_state.pyr.cur[0] = control_state.cam_pyr_offset[0];
		control_state.cam_pyr_offset[0] = 0;
		if (!optionGet(kUO_CamFree))
		{
			control_state.pyr.cur[1] += control_state.cam_pyr_offset[1];
			control_state.cam_pyr_offset[1] = 0;
		}

		cam_info.rotate_scale = 0.1;

		// Turn following mode off.
		control_state.follow = 0;
		updateControlState(CONTROLID_FORWARD, MOVE_INPUT_OTHER, 0, control_state.time.last_send_ms);
		control_state.inp_vel_scale = 1.0;
		
		setTargetPosFX(0, NULL);
	}
}

void playerStopFollowMode()
{
	playerSetFollowMode(NULL, NULL, NULL, 0, 0);
}


// Gets the average velocity calculated over the past given sample count.
float entGetAverageVel(Entity* e, int samples)
{
	float distanceTraveled = 0;
	U32 timeElapsed = 0;
	int i;
	MotionHist* hist = e->motion_hist;

	if(samples >= ARRAY_SIZE(e->motion_hist))
		samples = ARRAY_SIZE(e->motion_hist) - 1;
	
	for(i = 0; i < samples; i++)
	{
		Vec3 diff;
		int cur = (e->motion_hist_latest - i + ARRAY_SIZE(e->motion_hist)) % ARRAY_SIZE(e->motion_hist);
		int prev = (e->motion_hist_latest - i - 1 + ARRAY_SIZE(e->motion_hist)) % ARRAY_SIZE(e->motion_hist);
		subVec3(hist[cur].pos, hist[prev].pos, diff);
		distanceTraveled += lengthVec3(diff);
		timeElapsed += hist[cur].abs_time - hist[prev].abs_time;
	}

	return distanceTraveled / (ABS_TO_SEC((float)timeElapsed)) / 30;
}

#define PSR_MELEE_RANGE		3	// Try to get within this distance to the badguy.
#define PSR_KEEPOUT_ZONE	10	// Never get closer to the followed entity than this distance.
#define PSR_IN_TOW_ZONE		12	// Match the follow target's speed at this distance.
#define PSR_FALL_IN_ZONE	30	// Stop slowing down to the followed entity's speed at this distance.

typedef struct 
{
	int keepOutRange;	// Never get closer to the followed entity than this distance.
	int inTowRange;		// Match the follow target's speed at this distance.
	int fallInRange;	// Stop slowing down to the followed entity's speed at this distance.
} FollowRange;

typedef enum
{
	FRT_NORMAL,
	FRT_CRITTER,
	FRT_FLYING_CRITTER,
	FRT_MAX,
} FollowRangeType;

FollowRange followRanges[FRT_MAX] =
{
	// FRT_NORMAL
	{
		PSR_KEEPOUT_ZONE,
		PSR_IN_TOW_ZONE,
		PSR_FALL_IN_ZONE,
	},

	// FRT_CRITTER
	{
		PSR_MELEE_RANGE,
		PSR_MELEE_RANGE + 2,
		PSR_MELEE_RANGE + 22,
	},

	// FRT_CRITTER
	{
		10,
		10 + 2,
		10 + 22,
	},
};

static F32 playerGetMaxSpeed(Entity* ent)
{
	if(ent->motion->input.flying)
		return ent->motion->input.surf_mods[SURFPARAM_AIR].max_speed;
	else
		return ent->motion->input.surf_mods[SURFPARAM_GROUND].max_speed;
}

void playerDoFollowMode()
{
	static int lastFollowState = 0;
	static int showDebug = 0;

	float	distToTarget;
	int		reachedTarget = 0;
	const F32*	targetPos;
	Entity* followTarget;
	Entity* ent = playerPtr();
	int		overrideFollowMode = 0;
	Vec3	movementDirection;

	float	instantaneousTargetVelocity;
	float	projectedTargetVelocity;
	float	horizontalTargetVelocity;
	Vec3	targetMovementVector;

	Entity *controlledPlayer = controlledPlayerPtr();

	// Don't do anything if the player is not trying to follow any entities.
	if(!control_state.follow)
		return;

	if (control_state.followMovementCount != control_state.movementControlUpdateCount
		&& !control_state.followForced)
	{
		playerStopFollowMode();
		return;
	}

	// Don't do anything if:
	//	1) the entity the player is following is no longer available on the game client.
	//	2) the non-player entity the player is following dies.
	
	if(control_state.followTarget){
		followTarget = erGetEnt(control_state.followTarget);
		
		if(!playerHasValidFollowTarget())
		{
			playerStopForcedFollow();
			playerStopFollowMode();
			return;
		}

		targetPos = ENTPOS(followTarget);
	}else{
		followTarget = NULL;
		targetPos = follow_state.targetPos;
	}

	// Which direction do I want to move in?
	subVec3(targetPos, ENTPOS(ent), movementDirection);
	if(!ent->motion->input.flying)
		movementDirection[1] = 0;
	normalVec3(movementDirection);

	
	// How fast do I want to move?
	//	Derived the velocity of the followed entity projected onto the player movement
	//	vector.  This tells us how fast the player should move along his movement vector
	//	to catch up to the followed entity.
	if(followTarget){
		instantaneousTargetVelocity = entGetAverageVel(followTarget, 1);
		copyVec3(ENTMAT(followTarget)[2], targetMovementVector);
		scaleVec3(targetMovementVector, instantaneousTargetVelocity, targetMovementVector);

		// Project the target movement vector onto the player's movement vector.
		projectedTargetVelocity = dotVec3(targetMovementVector, movementDirection);

		{
			Vec3 temp;
			copyVec3(targetMovementVector, temp);
			
			if(!ent->motion->input.flying)
				temp[1] = 0;
			horizontalTargetVelocity = lengthVec3(temp);
		}
	}else{
		projectedTargetVelocity = 0;
		horizontalTargetVelocity = 0;
	}

	if(showDebug){
		xyprintf(10, 10, "targetvel %.2f", instantaneousTargetVelocity);
		xyprintf(10, 11, "ptargetvel %.2f", projectedTargetVelocity);
	}
	
	
	// Calculate the distance between the two entities
	{
		Vec3 playerCoord;
		Vec3 followTargetCoord;
		Vec3 diff;
		EntityCapsule playerCap;
		EntityCapsule targetCap;

		copyVec3(ENTPOS(ent), playerCoord);
		copyVec3(targetPos, followTargetCoord);
		subVec3(playerCoord, followTargetCoord, diff);
		if(!ent->motion->input.flying)
			diff[1] = 0;

		getCapsule(ent->seq, &playerCap );
		
		if(followTarget){
			getCapsule(followTarget->seq, &targetCap );
		}else{
			ZeroStruct(&targetCap);
		}

		distToTarget = lengthVec3(diff) - playerCap.radius - targetCap.radius;
		if(showDebug)
			xyprintf(10, 20, "dist: %f", distToTarget);
	}
	
	// Does the target seem to be moving?
	if(!followTarget || projectedTargetVelocity < 0)
	{
		control_state.inp_vel_scale = 1.0;
		
		if(!followTarget){
			if(distToTarget < follow_state.lastDistToTarget){
				follow_state.lastDistToTarget = distToTarget;
			}else{
				if(++follow_state.failedMoveCount > 40){
					distToTarget = 0;
				}
			}
		
			if(distToTarget < 0.5){
//				playerStopFollowMode();
				reachedTarget = 1;
				copyVec3(ENTPOS(ent), follow_state.targetPos);
//				setTargetPosFX(0, NULL);
//				return;
			}
		}
	}
	else
	{
		FollowRange* range;
		
		if(ENTTYPE(followTarget) == ENTTYPE_CRITTER)
		{
			if(ent->motion->input.flying)
				range = &followRanges[FRT_FLYING_CRITTER];
			else
				range = &followRanges[FRT_CRITTER];
		}
		else
		{
			range = &followRanges[FRT_NORMAL];
		}

		if(showDebug)
		{
			xyprintf(10, 17, "%i, %i, %i", range->keepOutRange, range->inTowRange, range->fallInRange);
		}

		if(horizontalTargetVelocity <= 0.1)
		{
			// Stop when the player has reached the "keep out zone".
			if(distToTarget < range->keepOutRange)
			{
				// Reached safety zone.  Stop completely.
				control_state.inp_vel_scale = 0.0;
				reachedTarget = 1;
				if(showDebug)
					xyprintf(10, 9, "keepout zone");
			}
			else
			{
				control_state.inp_vel_scale = 1.0;
			}
		}
		else
		{
			// Stop when the player has reached the "keep out zone".
			if(distToTarget < range->keepOutRange)
			{
				reachedTarget = 1;
				if(showDebug)
					xyprintf(10, 9, "keepout zone");
			}
			// Match target's movement speed in "in tow zone"
			// Don't do this in forced movement.
			else if (distToTarget < range->inTowRange && !control_state.followForced)
			{
				float maxVelocity;

				maxVelocity = playerGetMaxSpeed(ent);

				maxVelocity = ent->motion->input.surf_mods[ent->motion->input.flying].max_speed;
				control_state.inp_vel_scale = projectedTargetVelocity / maxVelocity;
				if(showDebug)
					xyprintf(10, 9, "in tow zone");
			}
			// Slowly slow down to match the target's movement speed in "fall-in zone".
			// Don't do this in forced movement.
			else if (distToTarget < range->fallInRange && !control_state.followForced)
			{
				float maxVelocity;
				float vel;

				// Match target velocity.
				maxVelocity = playerGetMaxSpeed(ent);

				vel = (maxVelocity - projectedTargetVelocity)/(PSR_FALL_IN_ZONE - PSR_KEEPOUT_ZONE)*(distToTarget-PSR_KEEPOUT_ZONE) + projectedTargetVelocity;
				control_state.inp_vel_scale = vel / maxVelocity;
				control_state.inp_vel_scale = MAX(0, MIN(control_state.inp_vel_scale, 1.0));

				if(showDebug)
					xyprintf(10, 9, "fallin zone");
			}
			// Move at max speed to catch up to target in the "catch up zone". 
			else
			{
				
				control_state.inp_vel_scale = 1.0;

				if(showDebug)
					xyprintf(10, 9, "catchup zone");
			}
		}
	}

	if(showDebug)
	{
		xyprintf(10, 12, "vel scale %.2f", control_state.inp_vel_scale);
		xyprintf(10, 13, "max speed %.2f", ent->motion->input.surf_mods[SURFPARAM_GROUND].max_speed);
		xyprintf(10, 14, "abs speed %.2f", control_state.inp_vel_scale * ent->motion->input.surf_mods[SURFPARAM_GROUND].max_speed);
		xyprintf(10, 15, "cur vel %.2f", lengthVec3(ent->motion->vel));
		xyprintf(10, 16, "dist %.2f", distToTarget);
	}

	updateControlState(CONTROLID_FORWARD, MOVE_INPUT_OTHER, (reachedTarget ? 0 : 1), control_state.time.last_send_ms);

	// Ripped from turnEntityBodyTowardDirection()
	if(followTarget || !reachedTarget){
		Vec3 directionPYR;
		Vec3 oldEntPYR;
		Vec3 newEntPYR;

		// What is the entity's current pyr?
		copyVec3(control_state.pyr.cur, oldEntPYR);
		copyVec3(oldEntPYR, newEntPYR);

		
		// Find out what orientation the entity wants to be in.
		getVec3YP(movementDirection, &directionPYR[1], &directionPYR[0]);

		// Adjust entity orientation according to the orientation dictated by the given direction
		// Only pitch in the direction of the follow target if the target is flying.
		if(ent->motion->input.flying)
			newEntPYR[0] = directionPYR[0];
		else
			newEntPYR[0] = 0;

		// Always yaw in the direction of the follow target
		newEntPYR[1] = directionPYR[1];

		// Was the entity's orientation really adjusted?
		//if(!sameVec3(oldEntPYR, newEntPYR)){
		//	// If the orientation was really adjusted, update the entity's matrix
		//	createMat3YPR(ent->mat, newEntPYR);
		//}
		if(!editMode() && !game_state.camera_follow_entity)
		{
			float yawdiff = newEntPYR[1] - control_state.pyr.cur[1];			
			copyVec3(newEntPYR, control_state.pyr.cur);
			if (optionGet(kUO_CamFree))
			{
				control_state.cam_pyr_offset[1] = subAngle(control_state.cam_pyr_offset[1],yawdiff);
			}
		}
	}

	// CLICKTOMOVE
	if (follow_state.activateTarget && controlledPlayer && !entMode(controlledPlayer, ENT_DEAD))
	{
		if (followTarget)
		{
			if (followTarget && distance3Squared(ENTPOS(controlledPlayer), ENTPOS(followTarget)) < SQR(INTERACT_DISTANCE - 3))
			{
				if(ENTTYPE(followTarget) == ENTTYPE_NPC || ENTTYPE(followTarget) == ENTTYPE_MISSION_ITEM)
				{
					START_INPUT_PACKET(pak, CLIENTINP_TOUCH_NPC);
					pktSendBitsPack( pak, PKT_BITS_TO_REP_SVR_IDX, followTarget->svr_idx);
					END_INPUT_PACKET
					playerStopFollowMode();
				}
				else if(ENTTYPE(followTarget) == ENTTYPE_DOOR)
				{
					enterDoorClient(ENTPOS(followTarget), 0);
					playerStopFollowMode();
				}
			}
		}
		else if (follow_state.defTarget)
		{
			interactWithInterestingGroupDef( controlledPlayer, follow_state.defTarget, follow_state.targetPos, NULL ); /* TODO: This should have a tracker */
		}
		else
		{
			follow_state.activateTarget = 0;
		}
	}

	if (reachedTarget)
		playerStopForcedFollow();

	if (!followTarget && reachedTarget && !follow_state.activateTarget)
		playerStopFollowMode();
}

/*
* Sticky player functions
*********************************************************************************/
