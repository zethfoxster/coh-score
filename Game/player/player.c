#include "assert.h"
#include "player.h"
#include "entity.h"
#include "cmdgame.h"
#include "cmdcommon.h"
#include "cmdcontrols.h"
#include "camera.h"
#include "entrecv.h"
#include "motion.h"
#include "input.h"
#include "pmotion.h"
#include "netcomp.h"
#include "clientcomm.h"
#include "uiConsole.h"
#include "memcheck.h"
#include "sun.h" 
#include "file.h" 
#include "entVarUpdate.h" // for ClientLink
#include "entclient.h"
#include "uiCursor.h"
#include "uiTray.h"
#include "Position.h"	
#include "utils.h"
#include "character_base.h"
#include "playerSticky.h"
#include <float.h>
#include "dooranimclient.h"
#include "font.h"
#include "uiGame.h"
#include "uiNet.h"
#include "uiCompass.h"
#include "varutils.h"
#include "edit_cmd.h"
#include "costume_client.h"
#include "entDebug.h"
#include "timing.h"
#include "seqstate.h"
#include "seq.h"
#include "bases.h"
#include "Quat.h"
#include "uiOptions.h"
#if !TEST_CLIENT
	#include "baseedit.h"	
#endif

typedef struct PlayerPrivateData {
	Entity*	player;
	Entity* mySlave;
	Entity* playerForShell;
	Entity* PCCCritter;
	int		playerIsSlave;
	int		slaveControl;
	int		pccEditingMode;
} PlayerPrivateData;

static PlayerPrivateData ppd;

static int forcedTurnHoldPYR;

int		glob_have_camera_pos;
Vec3    glob_pos;
Vec3    glob_vel;
int     glob_falling,glob_plrstate_valid;
int     glob_client_pos_id;

static ServerControlState server_control_state;

struct
{
	ControlStateChange* csc;
	F32					acc;
	MotionState			next_motion_state;
	F32					total_time;
	F32					used_time;
	Vec3				last_offset;
} pushCatchup;


Entity *playerPtrForShell(int alloc)
{
	//return playerPtr();
	if (!ppd.playerForShell && alloc) {
		ppd.playerForShell = entCreate("");
		//playerSetEnt(ppd.playerForShell);
		//entSetSeq(ppd.playerForShell, seqLoadInst( "male", 0, SEQ_LOAD_FULL, ppd.playerForShell->randSeed, ppd.playerForShell ));
		SET_ENTTYPE(ppd.playerForShell) = ENTTYPE_PLAYER;
		playerVarAlloc( ppd.playerForShell, ENTTYPE_PLAYER );    //must be after playerSetEnt?
		setGender( ppd.playerForShell, "male" );
		if (ppd.pccEditingMode > 0)
		{
			ppd.playerForShell->costume = ppd.PCCCritter->costume;
		}
		else
		{
			ppd.playerForShell->costume = ppd.player->costume;
		}		
	}
	// Set this so that the shell is on the same team but unganged, and thus does not have perception radius problems
	if (ppd.pccEditingMode > 0)
	{
		if (ppd.PCCCritter
			&& ppd.PCCCritter->pchar 
			&& ppd.PCCCritter->pchar->iAllyID 
			&& ppd.playerForShell
			&& ppd.playerForShell->pchar) 
		{
			ppd.playerForShell->pchar->iAllyID = ppd.PCCCritter->pchar->iAllyID;
			ppd.playerForShell->pchar->iGangID = 0;
		}
	}
	else
	{
		if (ppd.player 
			&& ppd.player->pchar 
			&& ppd.player->pchar->iAllyID 
			&& ppd.playerForShell
			&& ppd.playerForShell->pchar) 
		{
			ppd.playerForShell->pchar->iAllyID = ppd.player->pchar->iAllyID;
			ppd.playerForShell->pchar->iGangID = 0;
		}
	}
	return ppd.playerForShell;
}

void setPlayerPtrForShell(Entity *e)
{
	ppd.playerForShell = e;
}

void setMySlave(Entity* slave, Packet* pak, int control)
{
	Entity* master = ownedPlayerPtr();
	
	if(!master)
		return;
		
	if(slave)
	{
		// Check if there is already a slave set.
		
		if(master != playerPtr())
		{
			setMySlave(NULL, NULL, 0);
		}

		assert(pak);
		
		receiveCharacterFromServer(pak, slave);
		
		ppd.mySlave = slave;
		ppd.slaveControl = control;
	}
	else
	{
		slave = playerPtr();
		
		if(master != slave)
		{
			ppd.mySlave = NULL;
			ppd.slaveControl = 0;
		}
	}
}

Entity* getMySlave()
{
	return ppd.mySlave;
}

void setPlayerIsSlave(int set)
{
	ppd.playerIsSlave = set;
}

int playerIsSlave()
{
	return ppd.playerIsSlave;
}

//
// MS: Returns the entity that I'm viewing.
//
	
Entity *playerPtr()
{
	if(ppd.mySlave)
	{
		return ppd.mySlave;
	}
	
	if (ppd.pccEditingMode > 0)
	{	
		return ppd.PCCCritter;
	}
	return ppd.player;
}

//
// MS: Returns entity that I'm in control of.
//
	
Entity* controlledPlayerPtr()
{
	if(playerIsSlave())
		return NULL;
		
	if(ppd.mySlave)
		return ppd.slaveControl ? ppd.mySlave : NULL;

	return ppd.player;
}

void playerSetEnt(Entity *e)
{
	if (ppd.pccEditingMode > 0)
	{
		ppd.PCCCritter = e;
	}
	else
	{
		ppd.player = e;
	}
}

//
// MS: Returns player entity that I own regardless of whether I'm a slave.
//
	
Entity* ownedPlayerPtr()
{
	return ppd.player;
}

void playerSetPos(Vec3 pos)
{
	if (!playerPtr())
		return;
	entUpdatePosInstantaneous(playerPtr(),pos);
}

void playerSetMat3(Mat3 mat)
{
	if (!playerPtr())
		return;
	entSetMat3(playerPtr(), mat);
}

void playerHide(int hide)
{
	if (!playerPtr())
		return;
	SET_ENTHIDE(playerPtr()) = hide;
}

void playerGetPos(Vec3 pos)
{
	if (!playerPtr())
		return;
	copyVec3(ENTPOS(playerPtr()),pos);
}

void playerNoColl(int nocoll)
{
	Entity* e = playerPtr();
	if (!e)
		return;
	if (nocoll)
		e->move_type |= MOVETYPE_NOCOLL;
	else
		e->move_type &= ~MOVETYPE_NOCOLL;
}

/* setPlayerFacing:
 *	turn toward your target over time
 *
 */

void setPlayerFacing(Entity* target, F32 time_to_arrive_at_correct_facing)
{
	if(target)
	{
		control_state.forcedTurn.erEntityToFace = erGetRef(target);
		if(target)
		{
			copyVec3(ENTPOS(target), control_state.forcedTurn.locToFace);
		}
		control_state.forcedTurn.timeRemaining = time_to_arrive_at_correct_facing;
		if(!control_state.canlook && !control_state.cam_rotate)
		{
			control_state.forcedTurn.turnCamera = 1;
			cam_info.rotate_scale = 0.02;
		}
		else
		{
			control_state.forcedTurn.turnCamera = 0;
		}
	}
}

/* setPlayerFacingLocation:
 *	turn toward your target over time
 *
 */

void setPlayerFacingLocation(Vec3 vecTarget, F32 time_to_arrive_at_correct_facing)
{
	control_state.forcedTurn.erEntityToFace = 0;
	copyVec3(vecTarget, control_state.forcedTurn.locToFace);
	control_state.forcedTurn.timeRemaining = time_to_arrive_at_correct_facing;

	if(!control_state.canlook && !control_state.cam_rotate)
	{
		// Turn the camera if no camera-turn buttons are held down.
		
		control_state.forcedTurn.turnCamera = 1;
		cam_info.rotate_scale = 0.02;
	}
	else
	{
		control_state.forcedTurn.turnCamera = 0;
	}
}

// -20 to -2, linear
static F32 BestPitchAngle(F32 camdist)
{
	return RAD(-2.0 + camdist / 50 * -18.0);
}

// just set aside info for playerProcessMouseInput
static Vec3 g_last_inp_vel = {0};
static F32 g_last_max_speed = 0.0;
void MouseSpringInputVel(Vec3 vel, F32 max_speed)
{
	copyVec3(vel, g_last_inp_vel);
	g_last_max_speed = max_speed;
}

/* Function playerProcessMouseInput
 *	Updates the given pyr given the control state and the current mouse inputs.
 *
 *
 */
static void playerProcessMouseInput(ControlState *controls, Vec3 playerPyr, Vec3 cameraPyr)
{
	#define PITCH_SNAP RAD(10)
	#define MPITCH_SNAP RAD(90)
	extern int mouse_dx,mouse_dy;
	F32		d,maxsnap=MPITCH_SNAP,y_sign = 1;
	F32		pitch_snap;

	if (!optionGet(kUO_MouseInvert))
		y_sign = -1;

	// time adjust snap values
	pitch_snap = PITCH_SNAP / 2 * TIMESTEP;

	if( optionGetf(kUO_SpeedTurn) < 0 )
		optionSetf(kUO_SpeedTurn,0,0);
	if( optionGetf(kUO_SpeedTurn) > 30 )
		optionSetf(kUO_SpeedTurn,30,0);

	if( optionGetf(kUO_MouseSpeed) < 0 )
		optionSetf(kUO_MouseSpeed,0,0); 
	if( optionGetf(kUO_MouseSpeed) > 6 )
		optionSetf(kUO_MouseSpeed,6,0); 

	if (optionGet(kUO_CamFree) && control_state.canlook)
	{
		control_state.pyr.cur[1] = addAngle(control_state.pyr.cur[1],control_state.cam_pyr_offset[1]);
		control_state.cam_pyr_offset[1] = 0;
	}


	// Make player model orientation adjustments.
	if (controls->turnleft)
	{			
		playerPyr[1] = addAngle(playerPyr[1], -RAD(optionGetf(kUO_SpeedTurn)) * TIMESTEP);
		if (optionGet(kUO_CamFree) && !controls->canlook && !controls->first && !controls->follow)
			controls->cam_pyr_offset[1] = 0;
	}
	if (controls->turnright)
	{
		playerPyr[1] = addAngle(playerPyr[1], RAD(optionGetf(kUO_SpeedTurn)) * TIMESTEP);
		if (optionGet(kUO_CamFree) && !controls->canlook && !controls->first && !controls->follow)
			controls->cam_pyr_offset[1] = 0;
	}

 	if (controls->lookup) 
		cameraPyr[0] = addAngle(cameraPyr[0], RAD(optionGetf(kUO_SpeedTurn)) * TIMESTEP);
	if (controls->lookdown)
		cameraPyr[0] = addAngle(cameraPyr[0], -RAD(optionGetf(kUO_SpeedTurn)) * TIMESTEP);

	if( controls->zoomin )
	{
		game_state.camdist -= 3.0; 

		if(game_state.camdist < 0.0) 
		{
			game_state.camdist = 0.0;
			if( !control_state.follow )
			{
				control_state.first = 1;
				sendThirdPerson();
			}
		}
	}
	if( controls->zoomout )
	{
		if(control_state.first)
		{
			game_state.camdist = 0.0;
			control_state.first = 0;
			sendThirdPerson();
		}
		game_state.camdist += 3.0;

		#if !TEST_CLIENT
			if( baseedit_Mode() )
			{
				int max = 800;
				if( game_state.edit_base==kBaseEdit_Plot )
					max = 1500;

				if(game_state.camdist >= max)
					game_state.camdist = max;
			}
			else if(game_state.camdist >= 80.f)
				game_state.camdist = 80.f;
		#endif
	}

	// Make camera orientation adjustments.
   	if ( game_state.ortho && inpIsMouseLocked() )
	{
  		Entity *e = playerPtr();
   		cmdParse( "setpospyr 0 5000 0 0.0 0.0 0.0 " );
		cmdParse( "vis_scale 10" );
		cmdParse( "nocoll 1" );
		cameraPyr[0] = -PI/2;
		cameraPyr[1] = 0;
		cameraPyr[2] = 0;
		game_state.shadowvol = 2;
		game_state.nearz = 10.f;
		game_state.farz = 50000.0f;
	}
	else if ((controls->canlook || controls->cam_rotate) && inpIsMouseLocked())
	{	
		float *pitch;

		if (control_state.follow)
			pitch = control_state.cam_pyr_offset;
		else
			pitch = cameraPyr;

		// Adjust player yaw according to player input.
		cameraPyr[1] = addAngle(cameraPyr[1], optionGetf(kUO_MouseSpeed) * mouse_dx / 500.0F);
		
		// If the player is in mouselook mode...
		if(controls->mlook)
		{
			if (optionGet(kUO_MousePitchOption) == MOUSEPITCH_FIXED)
			{
				d = subAngle(BestPitchAngle(game_state.camdist),pitch[0]);
				if (d > pitch_snap) d = pitch_snap;
				if (d <-pitch_snap) d = -pitch_snap;
				pitch[0] = addAngle(d,pitch[0]);
			}
			else // free
			{
				// Adjust the pitch of the camera according to the mouse's up/down motion.
				pitch[0] += optionGetf(kUO_MouseSpeed) * y_sign * -mouse_dy / 500.0F;
			}
		}
		else
		{
			// The player is not in mouselook mode.
			// Lock the into to looking 2 degrees downwards.
			d = subAngle(RAD(-2),cameraPyr[0]);
			if (d > pitch_snap) d = pitch_snap;
			if (d <-pitch_snap) d = -pitch_snap;
			cameraPyr[0] = addAngle(d,cameraPyr[0]);
		}
		// Make sure the pyr does not go beyond certain pitch limits.
		if (pitch[0] < -MPITCH_SNAP) 
			pitch[0] = -MPITCH_SNAP;
		if (pitch[0] >  maxsnap) 
			pitch[0] =  maxsnap;

	}
	else if (optionGet(kUO_MousePitchOption)  == MOUSEPITCH_FIXED) // not canlook, but pitch is fixed
	{
		d = subAngle(BestPitchAngle(game_state.camdist),cameraPyr[0]);
		if (d > pitch_snap) d = pitch_snap;
		if (d <-pitch_snap) d = -pitch_snap;
		cameraPyr[0] = addAngle(d,cameraPyr[0]);
	}

	// If the player is using the 3rd person camera...
	if (!control_state.first)
	{
		if(control_state.server_state->fly || control_state.nocoll)
		{
			maxsnap = RAD(90);
		}
		else
		{
			// Restrict the camera movement so it does not cut into the player model when walking.
			maxsnap = RAD(70);
		}
	}

	// Make sure the pyr does not go beyond certain pitch limits.
	if (cameraPyr[0] < -MPITCH_SNAP) cameraPyr[0] = -MPITCH_SNAP;
	if (cameraPyr[0] >  maxsnap) cameraPyr[0] =  maxsnap;

	// do mouse spring according to last input from pmotionSetVel
	if (optionGet(kUO_MousePitchOption)  == MOUSEPITCH_SPRING && g_last_max_speed)
	{
		#define PITCH_SPRING RAD(7.5)
		#define PITCH_BREAK_THRESHOLD RAD(0.05) // if player is rotating this fast, spring doesn't happen
		F32 xzspeed2 = g_last_inp_vel[0] * g_last_inp_vel[0] + g_last_inp_vel[2] * g_last_inp_vel[2];
		F32 maxspeed2 = g_last_max_speed * g_last_max_speed;
		F32 pitch_spring;
		F32 player_move;
		F32 break_ratio;

		// scale spring according to input speed
		pitch_spring = PITCH_SPRING / 2 * TIMESTEP;
		if (xzspeed2 < 0.0) xzspeed2 = 0.0;
		if (xzspeed2 > maxspeed2) xzspeed2 = maxspeed2;
		pitch_spring *= xzspeed2 / maxspeed2;

		// if player is moving mouse, spring doesn't happen
		if (inpIsMouseLocked())
		{
			player_move = fabs(optionGetf(kUO_MouseSpeed) * mouse_dy / 500.0F);
			if (player_move > PITCH_BREAK_THRESHOLD) player_move = PITCH_BREAK_THRESHOLD;
			break_ratio = player_move / PITCH_BREAK_THRESHOLD; 
			pitch_spring *= 1.0 - break_ratio;
		}

		// adjust the camera
		d = subAngle(BestPitchAngle(game_state.camdist),cameraPyr[0]);
		if (d > pitch_spring) d = pitch_spring;
		if (d <-pitch_spring) d = -pitch_spring;
		cameraPyr[0] = addAngle(d,cameraPyr[0]);
	}
}

/* Function playerLook
 *	Adjust the player model and camera orientation based on user input.
 *
 *
 */
static void playerLook(ControlState *controls)
{
	#define MPITCH_SNAP RAD(90)

	F32		y_sign = 1;
	Vec3	pyr;
	Entity* e = controlledPlayerPtr();

	if (editMode())
		return;

	if (!optionGet(kUO_MouseInvert))
		y_sign = -1;

	// Decide which pyr to update.
	
	if(e)
	{
		if((control_state.cam_rotate || control_state.follow || (optionGet(kUO_CamFree) && !control_state.canlook))
			 && !control_state.first)
		{
			float old_yaw = controls->cam_pyr_offset[1];

			if(!control_state.follow)
				controls->cam_pyr_offset[0] = controls->pyr.cur[0];

			if (optionGet(kUO_CamFree))
			{			
				playerProcessMouseInput(controls, controls->pyr.cur, controls->cam_pyr_offset);
			}
			else
			{
				playerProcessMouseInput(controls, controls->cam_pyr_offset, controls->cam_pyr_offset);
			}

			if(controls->forcedTurn.turnCamera && !control_state.follow && old_yaw != controls->cam_pyr_offset[1])
			{
				controls->forcedTurn.turnCamera = 0;
				cam_info.rotate_scale = 1;
			}

			if(!control_state.follow)
			{
				controls->pyr.cur[0] = controls->cam_pyr_offset[0];
				controls->cam_pyr_offset[0] = 0;
			}			
		}
		else
		{
			float old_yaw = controls->pyr.cur[1];
			
			playerProcessMouseInput(controls, controls->pyr.cur, controls->pyr.cur);
			
			if(controls->forcedTurn.turnCamera && old_yaw != controls->pyr.cur[1])
			{
				controls->pyr.cur[1] = subAngle(addAngle(cam_info.pyr[1], RAD(180)), controls->cam_pyr_offset[1]);
				controls->forcedTurn.turnCamera = 0;
				cam_info.rotate_scale = 1;
			}
		}
	}

	// Update camera orientation.
	
	copyVec3(controls->pyr.cur, pyr);
	pyr[1] = addAngle(pyr[1], RAD(180));

	// Find the camera pyr based on the player's current pyr.
	//	Allow cam offset pitch adjustment when in follow mode.
	if(control_state.follow)
	{
		pyr[0] = controls->cam_pyr_offset[0];
	}

	pyr[1] = addAngle(controls->cam_pyr_offset[1], pyr[1]);
	pyr[0] = -pyr[0];
	
	camSetPyr(pyr);
}

static void playerQueueControlAngleChange(ControlState* controls, int index)
{
	ControlId id = CONTROLID_PITCH + index;
	U32 angleU32 = CSC_ANGLE_F32_TO_U32(controls->pyr.cur[index]);

	if(angleU32 != controls->pyr.prev[index])
	{
		ControlStateChange* csc = createControlStateChange();
		
		csc->control_id = id;
		csc->time_stamp_ms = controls->time.last_send_ms;
		csc->angleU32 = angleU32;

		addControlStateChange(&control_state.csc_processed_list, csc, 1);
		
		controls->pyr.prev[index] = angleU32;
	}
}

static F32 clampF32abs(F32 value, F32 max_abs)
{
	if(value > max_abs)
		return max_abs;
	else if(value < -max_abs)
		return -max_abs;
	else
		return value;		
}

static void playerAdjustBodyPYR(Entity* e, ControlState* controls)
{
	static F32 time_to_realign = 0;

	Vec3	body_pyr;
	int		usePYR = 0;
	Vec3	pyr_start;
	int		use_pitch = 0;

	Mat3	newmat;
	
	// Don't turn me if in the shell.
	
	if(shell_menu())
	{
		return;
	}

	// Figure out if I should use my PYR.
	
	if(	!editMode() &&
		(	controls->nocoll ||
			controls->alwaysmobile ||
			(	!controls->server_state->controls_input_ignored &&
				!controls->recover_from_landing_timer)))
	{
		usePYR = 1;
	}

	if(!usePYR && controls->forcedTurn.timeRemaining <= 0)
	{
		time_to_realign = 30.0 * 0.5;
	}
	
	if (!usePYR || !controls->server_state->fly && !controls->nocoll)
	{
		body_pyr[0] = 0;
	}
	else
	{
		if(	controls->server_state->fly &&
			abs(controls->dir_key.total_ms[CONTROLID_FORWARD] - controls->dir_key.total_ms[CONTROLID_BACKWARD]) < 50 &&
			abs(controls->dir_key.total_ms[CONTROLID_UP] - controls->dir_key.total_ms[CONTROLID_DOWN]) < 50)
		{
			body_pyr[0] = 0;
		}
		else
		{
			body_pyr[0] = controls->pyr.cur[0];
			use_pitch = 1;
		}
	}

	if(use_pitch != controls->using_pitch)
	{
		controls->using_pitch = use_pitch;
		
		if(!use_pitch)
		{
			Vec3 temp_vec;

			getMat3YPR(ENTMAT(e), temp_vec);
			
			controls->pitch_offset = temp_vec[0];
		}
		else
		{
			controls->pitch_offset = controls->pitch_offset * (1.0 - controls->pitch_scale);
		}

		controls->pitch_scale = 0;
	}

	if(controls->pitch_scale < 1)
	{
		controls->pitch_scale += (1.0 - controls->pitch_scale) * 0.1 * TIMESTEP;
	
		if(controls->pitch_scale > 1)
		{
			controls->pitch_scale = 1;
		}
	}

	if(controls->pitch_offset)
	{
		body_pyr[0] = addAngle(body_pyr[0], subAngle(controls->pitch_offset, body_pyr[0]) * (1.0 - controls->pitch_scale));
	}
	else
	{
		body_pyr[0] = body_pyr[0] * controls->pitch_scale;
	}
		
	getMat3YPR(ENTMAT(e), pyr_start);
	
	if(controls->server_state->fly)
	{
		F32 scale = 0.3;
		
		//printf("pitch: [%1.3f,%1.3f] (%1.3f - %1.3f) * %1.3f", controls->pitch_offset, controls->pitch_scale, body_pyr[0], pyr_start[0], scale * min(TIMESTEP, 0.6 / scale));
		
		body_pyr[0] = addAngle(pyr_start[0], scale * min(TIMESTEP, 0.6 / scale) * subAngle(body_pyr[0], pyr_start[0]));
		
		//printf(" ==> %1.3f\n", body_pyr[0]);
	}
	
	if(controls->forcedTurn.timeRemaining > 0 || forcedTurnHoldPYR)
	{
		Entity* faceEnt = erGetEnt(controls->forcedTurn.erEntityToFace);
		Vec3 pos;
		Vec3 diff;
		float yaw, pitch;
		float yaw_diff;
		float ratio;
		
		getMat3YPR(ENTMAT(e), body_pyr);

		if(faceEnt)
		{
			copyVec3(ENTPOS(faceEnt), pos);
			copyVec3(pos, controls->forcedTurn.locToFace);
		}
		else
		{
			copyVec3(controls->forcedTurn.locToFace, pos);
		}
			
		subVec3(pos, ENTPOS(e), diff);
		
		if(lengthVec3SquaredXZ(diff))
		{
			getVec3YP(diff, &yaw, &pitch);
		}
		else
		{
			getVec3YP(ENTMAT(e)[2], &yaw, &pitch);
		}
		
		ratio = TIMESTEP / controls->forcedTurn.timeRemaining;
		
		if(ratio > 1)
			ratio = 1;
			
		yaw_diff = subAngle(yaw, body_pyr[1]) * ratio;
		
		body_pyr[1] = addAngle(body_pyr[1], yaw_diff);
		
		if(controls->server_state->fly)
		{
			if(pitch < -RAD(45))
				pitch += RAD(45);
			else if(pitch > RAD(45))
				pitch -= RAD(45);
			else
				pitch = 0;
				
			body_pyr[0] = addAngle(body_pyr[0], subAngle(pitch, body_pyr[0]) * ratio);
		}

		if(controls->forcedTurn.turnCamera)
		{
			controls->pyr.cur[1] = addAngle(controls->pyr.cur[1], yaw_diff);
			if (optionGet(kUO_CamFree))
				controls->cam_pyr_offset[1] = addAngle(controls->cam_pyr_offset[1], -yaw_diff);
		}
		
		controls->forcedTurn.timeRemaining -= TIMESTEP;
		
		if(controls->forcedTurn.timeRemaining <= 0)
		{
			time_to_realign = 30.0 * 0.5;
			
			controls->last_vel_yaw = 0;

			controls->forcedTurn.timeRemaining = 0;
			
			forcedTurnHoldPYR = getMoveFlags(e->seq, SEQMOVE_CANTMOVE) ? 1 : 0;
		}
	}
	else if(usePYR)
	{
		F32 yaw_diff;
		
		// Subtract out the current input velocity yaw.

		pyr_start[1] = subAngle(pyr_start[1], controls->last_vel_yaw);

		if(controls->no_strafe || e->seq->type->noStrafe)
		{
			F32 air_scale = e->motion->input.flying ?
								0.05 :
								e->motion->falling || e->motion->jumping ?
									0.02 :
									vec3IsZero(controls->last_inp_vel) ?
										0.2 :
										0.4;
			F32 swap_threshold;
			F32 vel_yaw;
			
			// Calculate the yaw caused by the input velocity.

			vel_yaw = getVec3Yaw(controls->last_inp_vel);
			
			if(controls->last_inp_vel[2] < 0)
			{
				vel_yaw = addAngle(vel_yaw, RAD(180));
				swap_threshold = RAD(130);
			}
			else
			{
				if(!controls->yaw_swap_backwards && controls->last_vel_yaw >= -RAD(90) && controls->last_vel_yaw <= RAD(90))
				{
					swap_threshold = RAD(175);
				}
				else
				{
					swap_threshold = RAD(130);
				}
			}
			
			yaw_diff = fabs(subAngle(vel_yaw, controls->last_vel_yaw));

			if(yaw_diff > swap_threshold)
			{
				if(controls->yaw_swap_timer > 0)
				{
					controls->yaw_swap_timer -= TIMESTEP;
					
					if(controls->yaw_swap_timer < 1)
						controls->yaw_swap_timer = 1;
				}
				else
				{
					if(controls->last_inp_vel[2] < 0)
					{
						controls->yaw_swap_timer = 15;
						controls->yaw_swap_backwards = 1;
					}
					else
					{
						controls->yaw_swap_timer = 7;
						controls->yaw_swap_backwards = 0;
					}
				}
				
				if(controls->yaw_swap_timer > 1)
				{
					vel_yaw = addAngle(vel_yaw, RAD(180));
				}
			}
			else
			{
				controls->yaw_swap_timer = 0;
			}
			
			yaw_diff = clampF32abs(air_scale * subAngle(vel_yaw, controls->last_vel_yaw), TIMESTEP * RAD(10));
				
			controls->last_vel_yaw = addAngle(controls->last_vel_yaw, yaw_diff);
		}
		else
		{
			controls->last_vel_yaw = 0;
		}
				
		// Calculate the yaw to the control direction.
		
		yaw_diff = subAngle(controls->pyr.cur[1], pyr_start[1]);
		
		if(time_to_realign > 0)
		{
			if(yaw_diff)
			{
				F32 new_yaw_diff = clampF32abs(yaw_diff, TIMESTEP * RAD(25));
				
				if(new_yaw_diff == yaw_diff)
				{
					time_to_realign = 0;
				}
				else
				{
					yaw_diff = new_yaw_diff;
					
					time_to_realign -= TIMESTEP;
				}
			}
			else
			{
				time_to_realign = 0;
			}
		}
		else
		{
			F32 scale = 0.4;

			yaw_diff = yaw_diff * scale * min(TIMESTEP, 0.6 / scale);
		}
				
		yaw_diff = addAngle(yaw_diff, controls->last_vel_yaw);
		
		body_pyr[1] = addAngle(pyr_start[1], yaw_diff);
		
		//printf("pyr: %1.3f\t%1.3f\t%1.3f (%1.3f + %1.3f + %1.3f)\n", body_pyr[0], body_pyr[1], controls->pyr.cur[2], pyr_start[1], controls->last_vel_yaw, yaw_diff);
	}
	else
	{
		body_pyr[1] = pyr_start[1];
	}
		
	body_pyr[2] = controls->pyr.cur[2];
	
	createMat3YPR(newmat,body_pyr);
	entSetMat3(e, newmat);
}

void playerResetControlState()
{
	ControlState old_cs;

	destroyControlStateChangeList(&control_state.csc_to_process_list);
	destroyControlStateChangeList(&control_state.csc_processed_list);
	destroyFuturePushList(&control_state);

	old_cs = control_state;

	// Clear and then restore some old values.
	
 	ZeroStruct(&control_state);
 	ZeroStruct(&server_control_state);
	ZeroStruct(&pushCatchup);
 	
 	control_state.server_state			= &server_control_state;
	control_state.packet_ids			= old_cs.packet_ids;
	control_state.notimeout				= old_cs.notimeout;
	control_state.no_strafe				= old_cs.no_strafe;
	control_state.net_error_correction	= old_cs.net_error_correction;

	control_state.first_packet_sent = 0;
	control_state.speed_scale = 1;
	control_state.predict = 1;
	//control_state.third = old_cs.third;
	control_state.inp_vel_scale = 1;

	control_state.mlook = 1;
	control_state.editor_mouse_invert = old_cs.editor_mouse_invert;
}

#if !TEST_CLIENT
static void handleBinaryStateChange(Entity* e, ControlState* controls, U8 use_prev)
{
	ControlStateChange* csc = controls->csc_to_process_list.head;
	int destroy_csc = !control_state.mapserver_responding;
	int id = csc->control_id;
	
	// This is a direction key being toggled.
	
	controls->csc_to_process_list.head = csc->next;
	controls->csc_to_process_list.count--;
	
	assert(controls->csc_to_process_list.count || !controls->csc_to_process_list.head);

	if(!destroy_csc)
	{
		addControlStateChange(&controls->csc_processed_list, csc, 1);
	}

	if(csc->state != ((controls->dir_key.down_now >> id) & 1))
	{
		// State changed.
		
		int bit = 1 << id;
		
		if(csc->state)
		{
			// Key being pressed.
			
			controls->dir_key.down_now |= bit;
			
			// If changed from previous step, enable for this step.
			
			if(!(use_prev & bit))
			{
				controls->dir_key.use_now |= bit;
				controls->dir_key.affected_last_frame |= bit;
			}

			// Store the time that the control changed.
			
			controls->dir_key.start_ms[id] = csc->time_stamp_ms;
			
			if(controls->controldebug & 1)
			{
				printf("\nKeyDown: %s @ %dms\n", pmotionGetBinaryControlName(id), csc->time_stamp_ms);
			}
		}
		else
		{
			// Key being released.
			
			int add_ms;
			
			controls->dir_key.down_now &= ~bit;

			// If changed from previous step, disable for this step.
			
			if(use_prev & bit)
			{
				controls->dir_key.use_now &= ~bit;
				controls->dir_key.affected_last_frame |= bit;
			}

			if(id == CONTROLID_UP)
			{
				e->motion->input.jump_released = 1;
			}

			if(controls->dir_key.down_now & (1 << opposite_control_id[id]))
			{
				// Add nothing if the opposite key is held down on release.
				
				add_ms = 0;
			}
			else
			{
				int add_ms = csc->time_stamp_ms - controls->dir_key.start_ms[id];
				
				if(add_ms > 0)
				{
					controls->dir_key.total_ms[id] += add_ms;
					
					if(controls->dir_key.total_ms[id] > 1000)
					{
						controls->dir_key.total_ms[id] = 1000;
					}
				}
				else if(!controls->dir_key.total_ms[id])
				{
					// Must add at least 1ms, so that the keypress isn't ignored..

					controls->dir_key.total_ms[id] = 1;
				}
			}
			
			if(controls->controldebug & 1)
			{
				printf(	"\nKeyUp: %s @ %dms, %dms added, %dms total\n",
						pmotionGetBinaryControlName(id),
						csc->time_stamp_ms,
						add_ms,
						controls->dir_key.total_ms[id]);
			}
		}
	}

	if(destroy_csc)
	{
		destroyControlStateChange(csc);
	}
}

static void handleControlsUntilTime(Entity* e, ControlState* controls, U32 timestamp)
{
	ControlStateChange* csc;
	U8 use_prev = controls->dir_key.use_now;
	
	for(csc = controls->csc_to_process_list.head; csc; csc = controls->csc_to_process_list.head)
	{
		S32 time_diff_ms = timestamp - csc->time_stamp_ms;
		
		// Process all timestamps that are before or equal to this point in time.
		
		if(time_diff_ms > -3000 && time_diff_ms < 0)
		{
			break;
		}
		
		// Only direction keys should be in the process list.
		
		assert(csc->control_id < CONTROLID_BINARY_MAX);

		handleBinaryStateChange(e, controls, use_prev);
		
		// Update the tail if the end of the list was hit.

		if(!controls->csc_to_process_list.head)
		{
			controls->csc_to_process_list.tail = NULL;
		}
	}
}

static void createKeyString(char* key_string, ControlState* controls)
{
	char* cur = key_string;
	int i;

	key_string[0] = 0;

	for(i = 0; i < CONTROLID_BINARY_MAX; i++)
	{
		if(controls->dir_key.total_ms[i] || controls->dir_key.down_now & (1 << i))
		{
			cur += sprintf(cur,	"%s%s (%dms), ",
								controls->dir_key.down_now & (1 << i) ? "+" : "-",
								pmotionGetBinaryControlName(i),
								controls->dir_key.total_ms[i]);
		}
	}
}

static struct {
	Vec3	start_vel;
	float	gravity;
	int		jumping;
} controldebug_data;

static void printControlDebugText(	Entity* e,
									ControlState* controls,
									int cur_physics_step,
									const char* key_string,
									U32 cur_time_ms,
									U32 current_timestamp_ms,
									F32 start_timestep_acc,
									Vec3 new_vel,
									F32 inp_vel_scale_F32)
{
	static int count = 0;

	if(key_string[0] || !sameVec3(controls->start_pos, controls->end_pos))
	{
		count++;
		
		if(count == 0)
		{
			printf(	"\n\n\n\n"
					"----------------------------------------------------------------------\n"
					"------- BEGIN PHYSICS STEPS ------------------------------------------\n"
					"----------------------------------------------------------------------\n"
					"\n");
		}
		else if(count % 10 == 0)
		{
			count = count % 1000;
			
			printf("\n------ %3d -----------------------------------------------------------------\n", count);
		}
		
		#define TRIPLET "%1.8f, %1.8f, %1.8f"
		
		printf(	"\n"
				"%4d. [%d/%dx]:\tid=%5d, cur_time=%dms, runTime=%dms (%dms)\n"
				"        keys:      %s\n"
				"        pos:       ("TRIPLET")\n"
				"      + vel:       ("TRIPLET")\n"
				"      + inpvel:    ("TRIPLET") @ %f\n"
				"      + pyr:       ("TRIPLET")\n"
				"      + misc:      grav=%1.3f, %s%s\n"
				"      + move_time: %1.3f\n"
				"      = newpos:    ("TRIPLET")\n"
				"        newvel:    ("TRIPLET")\n"
				"%s"
				"%s",
				count,
				cur_physics_step + 1,
				(int)start_timestep_acc,
				0,//controls->net_id.cur - 1,
				cur_time_ms,
				current_timestamp_ms,
				current_timestamp_ms - cur_time_ms,
				key_string,
				vecParamsXYZ(controls->start_pos),
				vecParamsXYZ(controldebug_data.start_vel),
				vecParamsXYZ(new_vel),
				inp_vel_scale_F32,
				CSC_ANGLE_U32_TO_F32(CSC_ANGLE_F32_TO_U32(controls->pyr.cur[0])),
				CSC_ANGLE_U32_TO_F32(CSC_ANGLE_F32_TO_U32(controls->pyr.cur[1])),
				0.0,
				controldebug_data.gravity,
				controldebug_data.jumping ? "Jumping, " : "",
				controls->server_state->controls_input_ignored ? "ControlsInputIgnored, " : "",
				e->motion->move_time,
				vecParamsXYZ(controls->end_pos),
				vecParamsXYZ(e->motion->vel),
				controls->server_state->no_ent_collision ? "    ** NO ENT COLL **\n" : "",
				controls->server_state->fly ? "    ** FLYING **\n" : "");

		#undef TRIPLET
	}
	else
	{
		if(count != -1)
		{
			count = -1;

			printf(	"\n"
					"----------------------------------------------------------------------\n"
					"-------- END PHYSICS STEPS -------------------------------------------\n"
					"----------------------------------------------------------------------\n"
					"\n");
		}
	}
}

static ControlStateChange* removeFuturePushFromCSC(FuturePush* fp)
{
	ControlStateChange* csc = fp->csc;
	FuturePush* cur;
	FuturePush* prev = NULL;
	
	if(csc)
	{
		for(cur = csc->future_push_list; cur; prev = cur, cur = cur->csc_next)
		{
			if(cur == fp)
			{
				if(prev)
				{
					prev->csc_next = cur->csc_next;
				}
				else
				{
					csc->future_push_list = cur->csc_next;
				}
				
				fp->csc = NULL;
				
				return csc;
			}
		}
		
		assert(0);
	}
		
	return NULL;
}

static ControlStateChange* getNextPhysicsCSC(ControlStateChange* csc)
{
	for(; csc && csc->control_id != CONTROLID_RUN_PHYSICS; csc = csc->next);

	return csc;
}

static void playerForwardFuturePushes(ControlState* controls)
{
	FuturePush* fp;
	
	for(fp = controls->future_push_list; fp; fp = fp->controls_next)
	{
		ControlStateChange* csc = removeFuturePushFromCSC(fp);
		
		if(!csc)
		{
			// Packets were probably out of order.

			continue;
		}

		// Move the FuturePush forward until it's on the CSC that it should apply to.
		
		while(csc && fp->phys_tick_remaining)
		{
			ControlStateChange* next = getNextPhysicsCSC(csc->next);
			
			if(next)
			{
				fp->phys_tick_remaining--;
				csc = next;
			}
			else
			{
				break;
			}
		}
		
		assert(csc);
		
		fp->csc = csc;
		fp->csc_next = csc->future_push_list;
		csc->future_push_list = fp;
	}
}

static int tractionNonZero(MotionState* motion)
{
	return 1;
	return motion->input.surf_mods[0].traction > 0;
}

static int getPhysicsCSCCount(ControlStateChange* csc)
{
	int count = 0;
	for(; csc; csc = csc->next)
	{
		if(csc->control_id == CONTROLID_RUN_PHYSICS)
		{
			count++;
		}
	}
	return count;
}

static void hexDump(void* dataParam, U32 length)
{
	U8* data = dataParam;
	int odd = 1;
	
	printf("---------------------\n");
	while(length)
	{
		int writeLength = min(length, 16);
		int sum = 0;
		int i;
		
		odd = !odd;
		
		printf("%8.8x:   ", data - (U8*)dataParam);
		
		for(i = 0; i < writeLength; i++)
		{
			if(i % 4 == 0)
			{
				consoleSetColor((odd?COLOR_BRIGHT:0)|(i%8?COLOR_GREEN:0)|COLOR_BLUE, i%8?COLOR_BLUE:0);
			}
			
			sum += data[i];
			printf("%2.2x ", data[i]);
		}
		
		for(; i < 16; i++)
		{
			printf("   ");
		}
		
		consoleSetColor(1 + sum % 15, 0);
		printf(" = %8.8x\n", sum);
		consoleSetDefaultColor();
		
		data += writeLength;
		length -= writeLength;
	}
	printf("---------------------\n\n");
}

static void setEntityPositions(Entity* entExclude, ControlStateChange* csc)
{
	int i;
	
	for(i = 1; i < entities_max; i++)
	{
		Entity* e = validEntFromId(i);

		if(e && e != entExclude)
		{
			Mat4 newmat;
			copyMat4(ENTMAT(e), newmat);
			entCalcInterp(e, newmat, ENTINFO(e).send_on_odd_send ? csc->client_abs_slow : csc->client_abs, e->cur_interp_qrot);
			entSetMat3(e, newmat);
			entUpdatePosInterpolated(e, newmat[3]);
		}
	}
}

#define PUSH_CATCHUP_SCALE (1.7)

static void playerApplyFuturePushes(Entity* e, ControlState* controls)
{
	MotionState* motion = e->motion;
	FuturePush* fp = controls->future_push_list;
	FuturePush* prev = NULL;
	FuturePush* next;
	
	playerForwardFuturePushes(controls);
	
	for(; fp; fp = next)
	{
		S32 client_abs_diff = global_state.client_abs - fp->abs_time;
		
		next = fp->controls_next;

		if(!fp->abs_time_used && client_abs_diff > 0)
		{
			ControlStateChange* csc;
			
			fp->abs_time_used = 1;
		
			// Apply the push.
		
			if(fp->do_knockback_anim)
			{
				SETB(e->seq->state, STATE_KNOCKBACK);
				SETB(e->seq->state, STATE_HIT);
			}
			
			pmotionApplyFuturePush(motion, fp);
				
			// Destroy if the net_id was already used.
			
			csc = removeFuturePushFromCSC(fp);
			
			if(csc && !fp->phys_tick_remaining)
			{
				int remainingCount = getPhysicsCSCCount(csc);
				
				if(pushCatchup.csc)
				{
					MotionState oldMotion;
					
					addVec3(controls->start_pos, pushCatchup.last_offset, controls->start_pos);
					addVec3(controls->end_pos, pushCatchup.last_offset, controls->end_pos);
					copyVec3(controls->end_pos, motion->last_pos);
					
					oldMotion = *motion;
					
					// Run the rest of the physics up to the new starting point.
					
					global_motion_state.doNotSetAnimBits = 1;
					
					*motion = pushCatchup.csc->before.motion_state;
					entUpdatePosInterpolated(e, pushCatchup.csc->before.pos);
					copyVec3(pushCatchup.csc->before.pos, motion->last_pos);
					
					for(; pushCatchup.csc; pushCatchup.csc = pushCatchup.csc->next)
					{
						motion->input = pushCatchup.csc->before.motion_state.input;
						
						pushCatchup.csc->before.motion_state = *motion;
						copyVec3(ENTPOS(e), pushCatchup.csc->before.pos);
						
						if(pushCatchup.csc->control_id != CONTROLID_RUN_PHYSICS)
						{
							continue;
						}
						
						if(pushCatchup.csc == csc)
						{
							break;
						}
						
						// Move all the entities to this point in time.
						
						setEntityPositions(e, pushCatchup.csc);

						// Record motion BEFORE physics.
						
						if(controls->record_motion)
						{
							pmotionRecordStateBeforePhysics(e, "pushClear", pushCatchup.csc);
						}
						
						// Run the physics.
						
						e->timestep = 1;
						entMotion(e, unitmat);
						
						// Record motion AFTER physics.
						
						if(controls->record_motion)
						{
							pmotionRecordStateAfterPhysics(e, "pushClear");
						}
						
						copyVec3(ENTPOS(e), pushCatchup.csc->after.pos);
						copyVec3(motion->vel, pushCatchup.csc->after.vel);
					}

					global_motion_state.doNotSetAnimBits = 0;
					
					*motion = oldMotion;
				}

				pushCatchup.csc = csc;
				pushCatchup.acc = -1;
				zeroVec3(pushCatchup.last_offset);
				
				pushCatchup.total_time = (F32)remainingCount / (F32)(PUSH_CATCHUP_SCALE - 1.0);
				pushCatchup.used_time = 0;
				
				//printf("total time: %1.2f\n", pushCatchup.total_time);

				pmotionApplyFuturePush(&csc->before.motion_state, fp);

				assert(tractionNonZero(&csc->before.motion_state));
			}
			else
			{
				//printf("destroying future push!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
			}
		
			destroyFuturePush(fp);
			
			if(prev)
			{
				prev->controls_next = next;
			}
			else
			{
				controls->future_push_list = next;
			}
			
			fp = NULL;
		}
		
		if(fp)
		{
			prev = fp;
		}
	}
}

static void playerRunPhysicsStep(Entity* e, ControlState* controls, Vec3 new_vel, ControlStateChange* csc)
{
	static const char* trackName = "client";
	
	extern int dump_grid_coll_info;
	
	MotionState* motion = e->motion;

	// Initialize physics variables to what they were last time.

	copyVec3(controls->end_pos, controls->start_pos);
	entUpdatePosInterpolated(e, controls->end_pos);
	e->timestep = 1;
	copyVec3(new_vel, motion->input.vel);

	if(controls->recover_from_landing_timer)
	{
		controls->recover_from_landing_timer--;
	}
	
	// Set stuff from the controls.

	motion->input.max_speed_scale	= controls->max_speed_scale;
	motion->input.max_jump_height	= pmotionGetMaxJumpHeight(e, controls);
	motion->input.stunned			= controls->server_state->stun;

	// Copy the BEFORE data.

	csc->before.motion_state = *motion;
	assert(tractionNonZero(motion));
	copyVec3(ENTPOS(e), csc->before.pos);

	// Enable gridcoll info for control debugging.
	
	dump_grid_coll_info = controls->controldebug;
	
	// Apply future pushes.
	
	playerApplyFuturePushes(e, controls);
	
	// Record motion BEFORE physics.
	
	if(controls->record_motion)
	{
		pmotionRecordStateBeforePhysics(e, trackName, csc);
	}
	
	// Run the physics.

	entMotion(e, unitmat);
	
	// Record motion AFTER physics.
	
	if(controls->record_motion)
	{
		pmotionRecordStateAfterPhysics(e, trackName);
	}

	// Disable gridcoll info.

	dump_grid_coll_info = 0;

	// Copy the position that was reached into the current end_pos.

	copyVec3(ENTPOS(e), controls->end_pos);

	// Add a debug line.
	
	if(game_state.ent_debug_client & 128)
	{
		entDebugAddLine(controls->start_pos, 0xffffffff, controls->end_pos, 0xffffff00);
	}
}

static int playerIsControlDisabled(Entity* e, ControlState* controls)
{
	if(	editMode()
		||
		!controls->nocoll &&
		!controls->alwaysmobile &&
		(	controls->server_state->controls_input_ignored
			||
			controls->recover_from_landing_timer
			||
			controls->ignore
			||
			getMoveFlags(e->seq, SEQMOVE_CANTMOVE)
			||
			waitingForDoor(0)))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

static void playerAdjustMotionPYR(Entity* e, ControlState* controls)
{
	int i;
	
	// Calculate the motion PYR by converting to a CSC angle and back to F32 (since that's what the server will do).
	
	for(i = 0; i < 2; i++)
	{
		U32 angleU32 = CSC_ANGLE_F32_TO_U32(controls->pyr.cur[i]) & ((1 << CSC_ANGLE_BITS) - 1);
		
		controls->pyr.motion[i] = CSC_ANGLE_U32_TO_F32(angleU32);
	}
	
	controls->pyr.motion[2] = 0;

	if (!controls->server_state->fly && !controls->nocoll)
	{
		controls->pyr.motion[0] = 0;
	}
}

static void playerRunPhysicsSteps(Entity* e, ControlState* controls)
{
	MotionState*		motion = e->motion;
	char				key_string[1000];
	ControlStateChange* csc;
	int					cur_physics_step;
	float				start_timestep_acc;
	U8					inp_vel_scale_U8;
	float				inp_vel_scale_F32;
	
	// Run physics once for every 1 timestep.

	controls->timestep_acc += min(TIMESTEP, 15);
	
	if(controls->timestep_acc < 1.0)
	{
		return;
	}

	controls->controls_input_ignored_this_csc = playerIsControlDisabled(e, controls);
	
	// Adjust motion PYR.

	controls->pyr.cur[2] = 0;

	// Calculate the input velocity scale.
	
	if(controls->inp_vel_scale > 1)
		controls->inp_vel_scale = 1;
	else if(controls->inp_vel_scale < 0)
		controls->inp_vel_scale = 0;
	
	inp_vel_scale_U8 = (U8)(255 * controls->inp_vel_scale);
	inp_vel_scale_F32 = (F32)inp_vel_scale_U8 / 255.0;
		
	// Run all the physics steps that have accumulated.

	cur_physics_step = 0;
	
	start_timestep_acc = controls->timestep_acc;
	
	for(;controls->timestep_acc >= 1.0; cur_physics_step++)
	{
		static S32 old_end_time_init = 0;
		static U32 old_end_time_ms;

		U32		current_timestamp_ms;
		S32		time_diff_ms;
		Vec3	new_vel;
		F32		diff_ms_scale;
		
		if(!old_end_time_init)
		{
			old_end_time_init = 1;
			old_end_time_ms = global_state.cur_timeGetTime;
		}
		
		// Calculate the ms timestamp that this physics step "happened" at.
		// Do this by scaling the ms time passed to match the timestep accumulator.
		
		time_diff_ms = global_state.cur_timeGetTime - old_end_time_ms;
		
		if(time_diff_ms > 1000)
			time_diff_ms = 1000;
			
		diff_ms_scale = (cur_physics_step + 1) / start_timestep_acc;
			
		current_timestamp_ms =	global_state.cur_timeGetTime - time_diff_ms + (S32)floor(time_diff_ms * diff_ms_scale + 0.5);
		
		//assert(current_timestamp_ms <= cur_time_ms);

		// Update timestep_acc, and old_end_time_ms if necessary.

		controls->timestep_acc -= 1.0;
		
		// Process controls that happened up to current_timestamp_ms.
		
		handleControlsUntilTime(e, controls, current_timestamp_ms);
		
		// Do stuff for follow-mode.

		playerDoFollowMode();

		// Check for PYR changes.
		
		playerAdjustMotionPYR(e, controls);
		playerQueueControlAngleChange(controls, 0);
		playerQueueControlAngleChange(controls, 1);
			
		// Update control times.
		
		pmotionUpdateControlsPrePhysics(e, controls, current_timestamp_ms);

		// Print out key states.
		
		if(controls->controldebug & 1 && !server_visible_state.pause)
		{
			createKeyString(key_string, controls);
		}
		
		// Do the physics and junk.

		pmotionSetVel(e, controls, inp_vel_scale_F32);
		
		// Do stuff that happens on the last physics step.

		if(controls->timestep_acc < 1.0)
		{
			// Store the starting point for next time.
		
			old_end_time_ms = current_timestamp_ms;
			
			// Output the last input velocity.
			
			if(!controls->controls_input_ignored_this_csc)
			{
				copyVec3(motion->input.vel, controls->last_inp_vel);
			}
			else
			{
				zeroVec3(controls->last_inp_vel);
			}
		}
		
		// Calculate the input velocity based on the control direction.
		
		if(!controls->controls_input_ignored_this_csc)
		{
			Mat3 control_mat;
			createMat3RYP(control_mat, controls->pyr.motion);
			mulVecMat3(motion->input.vel, control_mat, new_vel);
		}
		else
		{
			zeroVec3(new_vel);
		}
	
		//printf("pos %5d :\ttimes %8d\t%8d\n", controls->phys_position_cur_id, global_state.client_abs, global_state.client_abs_slow);
		
		// Store some stuff if in debug mode.

		if(controls->controldebug & 1)
		{
			copyVec3(motion->vel, controldebug_data.start_vel);
			controldebug_data.gravity = motion->input.surf_mods[motion->input.flying].gravity;
			controldebug_data.jumping = motion->jumping;
		}
		
		// Run the physics if prediction is on.

		csc = createControlStateChange();
		
		if(controls->predict)
		{
			if( !controls->server_state->no_physics ) 
				playerRunPhysicsStep(e, controls, new_vel, csc);
		}
		else
		{
			FuturePush* fp;

			csc->no_ack = 1;

			// Not predicting, so just copy current position to start_pos and end_pos
			//   so that transition is smoother when re-enabling prediction.
		
			copyVec3(ENTPOS(e), controls->start_pos);
			copyVec3(ENTPOS(e), controls->end_pos);
			
			for(fp = controls->future_push_list; fp; fp = fp->controls_next)
			{
				removeFuturePushFromCSC(fp);
			}

			destroyFuturePushList(controls);
		}
	
		//if(0){
		//	static F32 max_y = -9999999999.0;
		//	static int printing = 0;
		//	
		//	F32 cur_y = posY(e);
		//	
		//	if(cur_y > max_y){
		//		printf("max: %f\t%f\t%f\t%d\n", cur_y, new_vel[1], motion->vel[1], motion->jumping);
		//		printing = 1;
		//	}
		//	else if(printing){
		//		printf("--------------------------------------\n");
		//		printing = 0;
		//	}

		//	max_y = posY(e);
		//}
			
		// Store the AFTER data.
		
		copyVec3(ENTPOS(e),				csc->after.pos);
		copyVec3(motion->vel,			csc->after.vel);

		// Add a physics step.
		
		csc->control_id	=				CONTROLID_RUN_PHYSICS;
		csc->time_stamp_ms =			current_timestamp_ms;
		csc->controls_input_ignored =	controls->controls_input_ignored_this_csc;
		csc->client_abs =				global_state.client_abs;
		csc->client_abs_slow =			global_state.client_abs_slow;
		csc->inp_vel_scale_U8 =			inp_vel_scale_U8;
		
		if(pushCatchup.csc)
		{
			csc->no_ack = 1;
		}
		
		assert(abs((int)(current_timestamp_ms - global_state.cur_timeGetTime)) < 2000);
		
		if(control_state.mapserver_responding)
		{
			addControlStateChange(&controls->csc_processed_list, csc, 1);
		}
		else
		{
			destroyControlStateChange(csc);
		}
		
		//assert(current_timestamp_ms == csc->time_stamp);

		// Reset key states.
		
		pmotionUpdateControlsPostPhysics(controls, current_timestamp_ms);

		// Print debugging stuff.

		if(controls->controldebug & 1)
		{
			printControlDebugText(	e,
									controls,
									cur_physics_step,
									key_string,
									global_state.cur_timeGetTime,
									current_timestamp_ms,
									start_timestep_acc,
									new_vel,
									inp_vel_scale_F32);
		}
	}
}

static void playerRunPushCatchup(Entity* e, ControlState* controls)
{
	MotionState* motion = e->motion;
	ControlState old_control_state;
	MotionState old_motion_state;
	ControlStateChange cscTemp;
	
	if(!pushCatchup.csc)
	{
		return;
	}
	
	if(pushCatchup.acc < 0)
	{
		cscTemp.next = pushCatchup.csc;
		pushCatchup.csc = &cscTemp;
		pushCatchup.acc = 1;
		pushCatchup.next_motion_state = cscTemp.next->before.motion_state;
		assert(tractionNonZero(&pushCatchup.next_motion_state));
	}
	
	pushCatchup.acc += TIMESTEP * PUSH_CATCHUP_SCALE;
	
	old_control_state = control_state;
	old_motion_state = *motion;
	assert(tractionNonZero(motion));
	
	while(pushCatchup.csc && pushCatchup.acc >= 1.0)
	{
		ControlStateChange* next = getNextPhysicsCSC(pushCatchup.csc->next);
		ControlStateChange* csc;
		
		if(!next)
		{
			pushCatchup.csc->no_ack = 1;
			//entDebugAddLine(pushCatchup.csc->before.pos, 0xff00ff00, pushCatchup.csc->after.pos, 0xff00ffff);
			*motion = pushCatchup.next_motion_state;
			motion->input = old_motion_state.input;
			control_state = old_control_state;
			copyVec3(pushCatchup.csc->before.pos, controls->start_pos);
			copyVec3(pushCatchup.csc->after.pos, controls->end_pos);
			copyVec3(pushCatchup.csc->after.pos, motion->last_pos);
			copyVec3(pushCatchup.csc->after.vel, motion->vel);
			pushCatchup.csc = NULL;
			return;
		}

		pushCatchup.acc -= 1.0;
		
		assert(pushCatchup.acc >= 0);
	
		if(pushCatchup.csc != &cscTemp)
		{
			copyVec3(pushCatchup.csc->after.pos, next->before.pos);
			pushCatchup.next_motion_state.input = next->before.motion_state.input;
			next->before.motion_state = pushCatchup.next_motion_state;
			assert(tractionNonZero(&next->before.motion_state));
		}
		
		csc = pushCatchup.csc = next;
		
		csc->no_ack = 1;

		*motion = pushCatchup.next_motion_state;
		entUpdatePosInterpolated(e, csc->before.pos);
		copyVec3(csc->before.pos, motion->last_pos);
		copyVec3(csc->before.pos, control_state.end_pos);
		
		motion->input = csc->before.motion_state.input;
		
		// Move all the entities to this point in time.
		
		setEntityPositions(e, csc);
		
		// Record motion BEFORE physics.
		
		if(controls->record_motion)
		{
			pmotionRecordStateBeforePhysics(e, "pushCatchup", csc);
		}
		
		// Run the physics.
		
		e->timestep = 1;
		global_motion_state.doNotSetAnimBits = 1;
		entMotion(e, unitmat);
		global_motion_state.doNotSetAnimBits = 0;
		
		// Record motion AFTER physics.
		
		if(controls->record_motion)
		{
			pmotionRecordStateAfterPhysics(e, "pushCatchup");
		}
		
		copyVec3(ENTPOS(e), csc->after.pos);
		copyVec3(motion->vel, csc->after.vel);
		
		pushCatchup.next_motion_state = *motion;
		assert(tractionNonZero(&pushCatchup.next_motion_state));
	}

	*motion = old_motion_state;
	control_state = old_control_state;
}

static void playerSetInterpPosition(Entity* e, ControlState* controls)
{
	static Vec3 lastFramePos;
	Vec3 posDiff;
	Vec3 newPos;
	
	if(controls->record_motion)
	{
		entUpdatePosInterpolated(e, lastFramePos);
		pmotionRecordStateBeforePhysics(e, "framePos", controls->csc_processed_list.tail);
	}

	// Scale between start_pos and end_pos by how much timestep has accumulated.

	subVec3(controls->end_pos, controls->start_pos, posDiff);
	scaleVec3(posDiff, controls->timestep_acc, posDiff);
	addVec3(posDiff, controls->start_pos, newPos);
	entUpdatePosInterpolated(e, newPos);
	
	if(pushCatchup.csc && pushCatchup.acc >= 0 && pushCatchup.acc <= 1)
	{
		Vec3 pushPos;
		F32 scale;
		
		if(controls->record_motion)
		{
			pmotionRecordStateBeforePhysics(e, "pushInterp", pushCatchup.csc);
		}

		subVec3(pushCatchup.csc->after.pos, pushCatchup.csc->before.pos, pushPos);
		scaleVec3(pushPos, pushCatchup.acc, pushPos);
		addVec3(pushCatchup.csc->before.pos, pushPos, pushPos);
		
		pushCatchup.used_time += TIMESTEP;
		
		if(pushCatchup.used_time > pushCatchup.total_time)
		{
			pushCatchup.used_time = pushCatchup.total_time;
		}
		
		scale = pushCatchup.used_time / pushCatchup.total_time;
		
		//printf("scale: %1.2f\t%1.2f\t%1.2f\t%1.2f\n", scale, pushCatchup.acc, pushCatchup.used_time, pushCatchup.total_time);
		
		subVec3(pushPos, ENTPOS(e), pushCatchup.last_offset);
		scaleVec3(pushCatchup.last_offset, scale, pushCatchup.last_offset);
		addVec3(ENTPOS(e), pushCatchup.last_offset, newPos);
		entUpdatePosInterpolated(e, newPos);
		
		//entDebugAddLine(e->mat[3], 0xffffffff, pushPos, 0xffff0000);
		//copyVec3(e->mat[3], controls->start_pos);
		//copyVec3(e->mat[3], controls->end_pos);
		
		if(controls->record_motion)
		{
			pmotionRecordStateAfterPhysics(e, "pushInterp");
		}
	}

	if(controls->record_motion)
	{
		pmotionRecordStateAfterPhysics(e, "framePos");
	}

	copyVec3(ENTPOS(e), lastFramePos);
}

static void playerPredictMotion(Entity* e, ControlState* controls)
{
	// Run all the needed physics steps.
	
	playerRunPhysicsSteps(e, controls);

	// Run all the needed push catchup physics.
	
	playerRunPushCatchup(e, controls);

	// Adjust body PYR.
	
	playerAdjustBodyPYR(e, controls);
	
	// Move the player model to the appropriate position based on the timestep.
	
	playerSetInterpPosition(e, controls);

	// Set states that need setting every frame.
	
	pmotionSetState(e, controls);
}
#endif

static void playerCheckForcedMove(Entity *e, ControlState *controls)
{
	MotionState* motion = e->motion;
	
	if (glob_plrstate_valid && !control_state.nosync)
	{
		//Vec3 cam_pyr;
		Vec3 body_pyr;
		Mat3 newmat;
		
		// Cancel forced turn if it is toward a position.
		
		if(!controls->forcedTurn.erEntityToFace)
		{
			controls->forcedTurn.timeRemaining = 0;
			forcedTurnHoldPYR = 0;
		}
		
		pushCatchup.csc = NULL;
		
		if( e->seq )
			e->seq->moved_instantly = 1;
		
		entUpdatePosInterpolated(e, glob_pos);
		copyVec3(glob_pos,motion->last_pos);
		copyVec3(glob_vel,motion->vel);
		copyVec3(glob_pos,controls->start_pos);
		copyVec3(glob_pos,controls->end_pos);
		motion->falling = glob_falling;

		playerStopFollowMode();

		copyVec3(controls->pyr.cur, body_pyr);
		body_pyr[0] = 0;
		createMat3YPR(newmat, body_pyr);
		entSetMat3(e, newmat);

		// Set the camera position directly to avoid dragging the camera through
		// geometry when moving from map to map.
		camSetPos( ENTPOS(e), false );
		
		DoorAnimCheckForcedMove();
		
		compass_UpdateWaypoints(1);
	}

	glob_plrstate_valid = 0;
}

// Top level function that processes all player input.

#if !TEST_CLIENT
void playerGetInput()
{
	// MS: Add a control-log entry.  This is a debugging feature.
	
	addControlLogEntry(&control_state);

 	if(	(control_state.nocoll || control_state.first_packet_sent) &&
 		!DoorAnimIgnorePositionUpdates())
	{
		Entity* e = controlledPlayerPtr();
	
		if (e && !(game_state.spin360))
		{
			// Check if the server has placed me somewhere else.
			
			playerCheckForcedMove(e, &control_state);

			// Check for FOV changes.
			
			if(!control_state.detached_camera)
				playerLook(&control_state);

			if(control_state.mapserver_responding || control_state.nocoll || control_state.alwaysmobile)
			{
 				// Enable/disable autorun.
 				
 				if(!control_state.follow)
 				{
					updateControlState(	CONTROLID_FORWARD,
						MOVE_INPUT_OTHER,
						control_state.autorun ? 1 : 0,
						control_state.time.last_send_ms);
 				}
 				
 				// Do motion prediction.

	 			playerPredictMotion(e, &control_state);
			}
		}
		else
		{
			playerLook(&control_state);
		}
	}

}
#endif

static Vec3	plr_pyr = {RAD(0),RAD(200),RAD(0)};
/*turns a player in the shell the given number of degrees and remembers the player's 
current pyr in the shell for future turns.  If you give it zero degrees, it resets the 
player's position to the default*/
void playerTurn(F32 ticks)
{
	Entity  * player;

	player = playerPtrForShell(0);

	if( ticks == 0 )
	{
		plr_pyr[0] = RAD(0);
		plr_pyr[1] = RAD(20);
		plr_pyr[2] = RAD(0);
	}

	if(player)
	{
		Mat3 newmat;
		plr_pyr[1] += RAD(ticks);
		plr_pyr[1] = fmodf(plr_pyr[1], TWOPI);
		PYRToQuat(plr_pyr,player->net_qrot);
		createMat3YPR(newmat,plr_pyr);
		entSetMat3(player, newmat);
	}

}
F32 playerTurn_GetTicks()
{
	return DEG(plr_pyr[1]);
}

float playerHeight()
{
	Entity* e = playerPtr();
	float height = SEQ_DEFAULT_CAMERA_HEIGHT;
	SeqInst* seq = e ? e->seq : NULL;
	
	if(!seq || !seq->type->name)
		return height;		// Default player height.
		
	if(seq->type->cameraHeight) //Fems are 5'5"  
		height = seq->type->cameraHeight;

	// Take into consideration the scale of this character.
	height *= seq->currgeomscale[1];

	return height;
}


static void playerReceiveServerControlState(Packet *pak)
{
	int i;
	Entity* e = playerPtr();

	if (pktGetBits(pak,1))
	{
		// Get a new server state.

		int oo_packet = pak->id <= control_state.server_state_pkt_id;
		ServerControlState server_state;
		int received_update_id;

		received_update_id =					pktGetBits(pak, 8);

		for(i=0;i<3;i++)
		{
			server_state.speed[i] =				pktGetF32(pak);
		}

		server_state.speed_back =				pktGetF32(pak);

		pktGetBitsArray(pak, sizeof(server_state.surf_mods)*8, &server_state.surf_mods);

		server_state.jump_height =				pktGetF32(pak);

		server_state.fly =						pktGetBits(pak,1);
		server_state.stun =						pktGetBits(pak,1);
		server_state.jumppack =					pktGetBits(pak,1);
		server_state.glide =					pktGetBits(pak,1);
		server_state.ninja_run =				pktGetBits(pak,1);
		server_state.walk =						pktGetBits(pak,1);
		server_state.beast_run =				pktGetBits(pak,1);
		server_state.steam_jump =				pktGetBits(pak,1);
		server_state.hover_board =				pktGetBits(pak,1);
		server_state.magic_carpet =				pktGetBits(pak,1);
		server_state.parkour_run =				pktGetBits(pak, 1);
		server_state.no_physics =				pktGetBits(pak,1);
		server_state.hit_stumble =				pktGetBits(pak,1);

		server_state.controls_input_ignored =	pktGetBits(pak,1);
		server_state.shell_disabled =			pktGetBits(pak, 1);
		server_state.no_ent_collision =			pktGetBits(pak,1);

		server_state.no_jump_repeat =			pktGetBits(pak, 1);

		if(!oo_packet)
		{
			MotionState* motion = e->motion;
			ServerControlState* scs = control_state.server_state;

			control_state.server_state_pkt_id = pak->id;

			control_state.received_update_id = received_update_id;

			copyVec3(server_state.speed, scs->speed);

			scs->speed_back = server_state.speed_back;

			memcpy(scs->surf_mods, server_state.surf_mods, sizeof(server_state.surf_mods));

			memcpy(motion->input.surf_mods, server_state.surf_mods, sizeof(server_state.surf_mods));

			scs->jump_height = server_state.jump_height;

			if(0 && control_state.controldebug)
			{
				printf(	"received state %d: %f, %f, %f, %f\n",
						control_state.received_update_id,
						scs->surf_mods[SURFPARAM_AIR].bounce,
						scs->surf_mods[SURFPARAM_AIR].friction,
						scs->surf_mods[SURFPARAM_AIR].gravity,
						scs->surf_mods[SURFPARAM_AIR].traction);
			}

			scs->fly =							server_state.fly;
			scs->stun =							server_state.stun;
			scs->jumppack =						server_state.jumppack;
			scs->glide =						server_state.glide;
			scs->ninja_run =					server_state.ninja_run;
			scs->walk =							server_state.walk;
			scs->beast_run =					server_state.beast_run;
			scs->steam_jump =					server_state.steam_jump;
			scs->hover_board =					server_state.hover_board;
			scs->magic_carpet =					server_state.magic_carpet;
			scs->parkour_run =					server_state.parkour_run;
			scs->no_physics =					server_state.no_physics;
			scs->hit_stumble =					server_state.hit_stumble;

			scs->controls_input_ignored =		server_state.controls_input_ignored;
			scs->shell_disabled =				server_state.shell_disabled;
			motion->input.no_ent_collision =	server_state.no_ent_collision;
			scs->no_ent_collision =				server_state.no_ent_collision;
			scs->no_jump_repeat =				server_state.no_jump_repeat;

			if(motion->input.flying != scs->fly)
			{
				motion->input.flying = scs->fly;

				if(!motion->input.flying)
				{
					motion->falling = 1;
				}
				else
				{
					motion->falling = 0;
					motion->jumping = 0;
				}
			}

			if( scs->hit_stumble )  
			{
				motion->hit_stumble = 1;
				motion->hitStumbleTractionLoss = 1.0; //TO DO base on strength of hit
#if !TEST_CLIENT
				SETB( e->seq->state, STATE_STUMBLE );
#endif
			}

			if(control_state.first_packet_sent)
			{
				// Add an acknowledgement control update.

				ControlStateChange* csc = createControlStateChange();

				csc->control_id = CONTROLID_APPLY_SERVER_STATE;
				csc->applied_server_state_id = control_state.received_update_id;
				csc->time_stamp_ms = control_state.time.last_send_ms;

				addControlStateChange(&control_state.csc_processed_list, csc, 1);
			}
		}
	}

	if (pktGetBits(pak,1))
	{
		int oo_packet = pak->id <= control_state.forced_move_pkt_id;
		int pos_id;
		Vec3 pos;
		Vec3 vel;
		Vec3 pyr;
		int falling;

		pos_id = pktGetBitsPack(pak,1);

		for(i=0;i<3;i++)
			pos[i] = pktGetF32(pak);

		for(i=0;i<3;i++)
			vel[i] = pktGetIfSetF32(pak);

		pktGetIfSetF32(pak);	// ignore pitch
		pyr[1] = pktGetIfSetF32(pak);
		pyr[2] = pktGetIfSetF32(pak);

		falling = pktGetBitsPack(pak,1);

		if(!oo_packet)
		{
			control_state.forced_move_pkt_id = pak->id;

			glob_client_pos_id = pos_id;

			copyVec3(pos, glob_pos);

			//printf("got pos: (%1.1f, %1.1f, %1.1f)\n", vecParamsXYZ(pos));

			copyVec3(vel, glob_vel);

			//printf("got yaw: %1.3f @ (%1.1f, %1.1f, %1.1f)\n", pyr[1], vecParamsXYZ(pos));

			if(!control_state.nosync)
			{
				control_state.pyr.cur[1] = pyr[1];
				control_state.pyr.cur[2] = pyr[2];

				for(i = 0; i < 3; i++)
				{
					control_state.pyr.prev[i] = control_state.pyr.send[i] = CSC_ANGLE_F32_TO_U32(control_state.pyr.cur[i]);
				}
			}

			glob_falling = falling;

			glob_plrstate_valid = 1;
		}
	}

	if(glob_client_pos_id != control_state.client_pos_id)
	{
		Entity* e = playerPtr();
		Vec3 cam_pyr;

		copyVec3(control_state.pyr.cur, cam_pyr);
		cam_pyr[1] = addAngle(cam_pyr[1], RAD(180));
		cam_pyr[1] = addAngle(cam_pyr[1], control_state.cam_pyr_offset[1]);
		cam_pyr[0] = -cam_pyr[0];

		//printf("got:\t%1.2f\t%1.2f\t%1.2f\n", cam_pyr[0], cam_pyr[1], cam_pyr[2]);

		copyVec3(cam_pyr, cam_info.pyr);
		copyVec3(cam_pyr, cam_info.targetPYR);

		//printf("got PYR %d: %f, %f, %f\n", update_pyr, control_state.pyr[0], control_state.pyr[1], control_state.pyr[2]);
	}
}

#if !TEST_CLIENT
static void repredictPhysicsSteps(ControlState* controls, ControlStateChange* csc)
{
	Entity* e = playerPtr();
	MotionState* motion = e->motion;
	int physics_count = 0;

	for(; csc; csc = csc->next)
	{
		MotionState cscMotion;
		
		if(csc->control_id != CONTROLID_RUN_PHYSICS)
			continue;

		cscMotion = csc->before.motion_state;

		csc->no_ack = 1;

		if(!physics_count++)
		{
			// For the first physics step, copy the BEFORE data from the CSC.

			*motion = cscMotion;
			entUpdatePosInterpolated(e, csc->before.pos);
			copyVec3(csc->before.pos, motion->last_pos);
			copyVec3(csc->before.pos, control_state.end_pos);
		}
		else
		{
			// If not the first one, then copy input from the old MotionState.
			
			motion->input = cscMotion.input;

			// Copy the whole MotionState back to the CSC.
			
			copyVec3(ENTPOS(e), csc->before.pos);
			csc->before.motion_state = *motion;
		}
				
		// Move all the entities to this point in time.
		
		setEntityPositions(e, csc);
		
		copyVec3(control_state.end_pos, control_state.start_pos);

		// Record motion BEFORE physics.
		
		if(controls->record_motion)
		{
			pmotionRecordStateBeforePhysics(e, "repredict", csc);
		}
		
		e->timestep = 1;
		global_motion_state.doNotSetAnimBits = 1;
		entMotion(e, unitmat);
		global_motion_state.doNotSetAnimBits = 0;
		
		// Record motion AFTER physics.
		
		if(controls->record_motion)
		{
			pmotionRecordStateAfterPhysics(e, "repredict");
		}
		
		copyVec3(ENTPOS(e), control_state.end_pos);
	}
}
#endif

static void playerReceiveServerPhysicsPositions(Packet* pak)
{
	ControlStateChange* csc;
	int i;
	const S32 size = ARRAY_SIZE(control_state.svr_phys.step);
	Entity* e = playerPtr();
	S32		pos_changed;
	S32		new_id = -1;
	Vec3	new_pos;
	Vec3	new_vel;
	S32		last_server_net_id;
	Vec3	last_server_pos;
	Vec3	last_server_vel;
	F32		offset_distance;
	S32		found_push_list = 0;

	// Read all the positions from the server.

	pos_changed = pktGetBits(pak, 1);

	if(pos_changed || pktGetBits(pak, 1))
	{
		new_id = pktGetBits(pak, 16);
	}

	if(pos_changed)
	{
		for(i = 0; i < 3; i++)
			new_pos[i] = pktGetF32(pak);
		for(i = 0; i < 3; i++)
			new_vel[i] = pktGetIfSetF32(pak);
	}

	if(pktGetBits(pak, 1) == 1)
	{
		pmotionReceivePhysicsRecording(pak, e);
	}

	if(	new_id < 0 ||
		pak->id <= control_state.server_pos_pkt_id ||
		!pos_changed && !control_state.server_pos_pkt_id)
	{
		return;
	}
	
	// Store the new information if it's really new.
	
	control_state.svr_phys.step[control_state.svr_phys.end].net_id = new_id;

	if(pos_changed)
	{
		control_state.server_pos_pkt_id = pak->id;
		
		copyVec3(new_pos, control_state.svr_phys.step[control_state.svr_phys.end].pos);
		copyVec3(new_vel, control_state.svr_phys.step[control_state.svr_phys.end].vel);
	}
	else
	{
		int last_index = (control_state.svr_phys.end + size - 1) % size;

		copyVec3(	control_state.svr_phys.step[last_index].pos,
					control_state.svr_phys.step[control_state.svr_phys.end].pos);

		copyVec3(	control_state.svr_phys.step[last_index].vel,
					control_state.svr_phys.step[control_state.svr_phys.end].vel);
	}

	// Make a copy of the current state.
	
	last_server_net_id = control_state.svr_phys.step[control_state.svr_phys.end].net_id;
	copyVec3(control_state.svr_phys.step[control_state.svr_phys.end].pos, last_server_pos);
	copyVec3(control_state.svr_phys.step[control_state.svr_phys.end].vel, last_server_vel);

	// Increment to the next 
	
	if(!server_visible_state.pause)
	{
		control_state.svr_phys.end = (control_state.svr_phys.end + 1) % size;
	}

	// Find the CSC with the matching net_id.

	for(csc = control_state.csc_processed_list.head; csc && csc->sent;)
	{
		#if !TEST_CLIENT
			if(csc->future_push_list || pushCatchup.csc == csc)
			{
				found_push_list = 1;
			}
			
			// Stop if this CSC is the physics step we're looking for.
			
			if(csc->control_id == CONTROLID_RUN_PHYSICS && csc->net_id == last_server_net_id)
			{
				ControlStateChange* cscNextPhys = getNextPhysicsCSC(csc->next);
				
				csc->has_ack = 1;
				copyVec3(last_server_pos, csc->ack_info.pos);
				copyVec3(last_server_vel, csc->ack_info.vel);
				
				if(cscNextPhys)
				{
					copyVec3(last_server_pos, cscNextPhys->before.pos);
					copyVec3(last_server_vel, cscNextPhys->before.motion_state.vel);
				}
				
				break;
			}
		#endif
		
		// Ignore everything before the matching net_id.

		if(!found_push_list)
		{
			control_state.csc_processed_list.head = csc->next;

			destroyControlStateChange(csc);

			if(!--control_state.csc_processed_list.count)
			{
				assert(!control_state.csc_processed_list.head);
				control_state.csc_processed_list.tail = NULL;
			}

			csc = control_state.csc_processed_list.head;
		}
		else
		{
			csc = csc->next;
		}
	}

	if(	found_push_list ||
		DoorAnimIgnorePositionUpdates() ||
		control_state.nosync ||
		!csc ||
		!csc->sent ||
		csc->net_id != last_server_net_id ||
		csc->no_ack ||
		sameVec3(csc->after.pos, last_server_pos))
	{
		// No need to repredict physics.

		return;
	}
	
	#if !TEST_CLIENT
		// TEMP!!!
		if(0)
		{
			ControlStateChange* prev = control_state.csc_processed_list.head;
			Vec3 pos;
			copyVec3(last_server_pos, pos);
			pos[1] += 10;
			entDebugAddLine(last_server_pos, 0xffffffff, pos, 0xffff00ff);
			entDebugAddLine(last_server_pos, 0xffffffff, csc->after.pos, 0xffff0000);
			
			if(getNextPhysicsCSC(csc->next))
			{
				entDebugAddLine(csc->after.pos, 0xffffffff, getNextPhysicsCSC(csc->next)->after.pos, 0xff00ffff);
			}
			
			for(; prev; prev = prev->next)
			{
				if(prev->control_id == CONTROLID_RUN_PHYSICS)
				{
					ControlStateChange* next = getNextPhysicsCSC(prev->next);
					
					if(next == csc)
					{
						entDebugAddLine(prev->after.pos, 0xffffff00, csc->after.pos, 0xff00ffff);
						break;
					}
				}
			}
		}
	#endif
	
	if(csc)
	{
		// Set this for demo recorder.

		global_state.client_abs_latency = server_visible_state.abs_time - csc->client_abs;
	}

	if(control_state.controldebug)
	{
		char buffer[100];
		Vec3 pos2;
		copyVec3(last_server_pos, pos2);
		pos2[1] += 100;
		entDebugAddLine(last_server_pos, 0xffff3333, pos2, 0xffffffff);
		sprintf(buffer, "%d : %1.2f", last_server_net_id, distance3(last_server_pos, csc->after.pos));
		entDebugAddText(last_server_pos, buffer, 0);
	}
	
	//if(lengthVec3(last_server_pos) < 0.1){
	//	printf("last_server_pos[%d]: %1.1f, %1.1f, %1.1f\n", control_state.svr_phys.end, vecParamsXYZ(last_server_pos));
	//}

	offset_distance = distance3(last_server_pos, csc->after.pos);

	if(control_state.controldebug & 3)
	{
		consoleSetColor(COLOR_BRIGHT | COLOR_RED, 0);

		printf(	"*************************************************************************************\n"
				"*********************************** missed %5d ************************************\n"
				"*************************************************************************************\n"
				"* abs_time:   %d\n"
				"* server pos: %f, %f, %f\n"
				"* client pos: %f, %f, %f\n"
				"* diff:       %f\n"
				"*************************************************************************************\n",
				last_server_net_id,
				csc->client_abs,
				vecParamsXYZ(last_server_pos),
				vecParamsXYZ(csc->after.pos),
				offset_distance);

		consoleSetDefaultColor();
	}

	if(control_state.csc_processed_list.head == csc)
	{
		// Destroy this CSC now that it's not needed and start at the next one.

		control_state.csc_processed_list.head = csc->next;
		destroyControlStateChange(csc);
		csc = control_state.csc_processed_list.head;

		if(!--control_state.csc_processed_list.count)
		{
			assert(!control_state.csc_processed_list.head);
			control_state.csc_processed_list.tail = NULL;
		}
	}
	else
	{
		csc = csc->next;
	}

	#if !TEST_CLIENT
		// Rerun all the physics steps.

		PERFINFO_RUN(
			if(offset_distance <= 0.000001)
			{
				PERFINFO_AUTO_START("repredict <= 0.000001", 1);
			}
			else if(offset_distance <= 0.00001)
			{
				PERFINFO_AUTO_START("repredict <= 0.00001", 1);
			}
			else if(offset_distance <= 0.0001)
			{
				PERFINFO_AUTO_START("repredict <= 0.0001", 1);
			}
			else if(offset_distance <= 0.001)
			{
				PERFINFO_AUTO_START("repredict <= 0.001", 1);
			}
			else if(offset_distance <= 0.01)
			{
				PERFINFO_AUTO_START("repredict <= 0.01", 1);
			}
			else if(offset_distance <= 0.1)
			{
				PERFINFO_AUTO_START("repredict <= 0.1", 1);
			}
			else if(offset_distance <= 1)
			{
				PERFINFO_AUTO_START("repredict <= 1", 1);
			}
			else if(offset_distance <= 10)
			{
				PERFINFO_AUTO_START("repredict <= 10", 1);
			}
			else if(offset_distance <= 100)
			{
				PERFINFO_AUTO_START("repredict <= 100", 1);
			}
			else if(offset_distance <= 1000)
			{
				PERFINFO_AUTO_START("repredict <= 1000", 1);
			}
			else
			{
				PERFINFO_AUTO_START("repredict > 1000", 1);
			}
		);

			repredictPhysicsSteps(&control_state, csc);

		PERFINFO_AUTO_STOP();
	#endif
}

static void playerReceiveFuturePushList(Packet* pak)
{
	while(pktGetBits(pak, 1) == 1)
	{
		ControlStateChange* csc;
		FuturePush* fp = addFuturePush(&control_state);
		
		fp->net_id_start		= pktGetBits(pak, 16);
		fp->phys_tick_remaining = pktGetBitsPack(pak, 5);
		fp->abs_time			= pktGetBits(pak, 32);
		fp->add_vel[0] 			= pktGetF32(pak);
		fp->add_vel[1] 			= pktGetF32(pak);
		fp->add_vel[2] 			= pktGetF32(pak);
		fp->do_knockback_anim	= pktGetBits(pak, 1);

		// The starting net_id better be in the reprocess list or something is out of sync.
		
		for(csc = control_state.csc_processed_list.head; csc && csc->sent; csc = csc->next)
		{
			if(csc->net_id == fp->net_id_start)
			{
				if(csc->control_id == CONTROLID_RUN_PHYSICS)
				{
					fp->csc = csc;
					fp->csc_next = csc->future_push_list;
					csc->future_push_list = fp;
				}
				
				break;
			}
		}
	}
}

void playerReceiveControlState(Packet* pak)
{
	// Receive future push list.
	
	START_BIT_COUNT(pak, "receiveFuturePushList");
		playerReceiveFuturePushList(pak);
	STOP_BIT_COUNT(pak);
	
	#if !TEST_CLIENT
		// Push FuturePushes forward.

		START_BIT_COUNT(pak, "forwardFuturePushes");
			playerForwardFuturePushes(&control_state);
		STOP_BIT_COUNT(pak);
	#endif
	
	// Receive physics positions.

	START_BIT_COUNT(pak, "receiveServerPhysicsPositions");
		playerReceiveServerPhysicsPositions(pak);
	STOP_BIT_COUNT(pak);

	// Receive server control state.

	START_BIT_COUNT(pak, "receiveServerControlState");
		playerReceiveServerControlState(pak);
	STOP_BIT_COUNT(pak);
}

void setPCCEditingMode(int mode)
{
	ppd.pccEditingMode = mode;
}

int getPCCEditingMode()
{
	return ppd.pccEditingMode;
}