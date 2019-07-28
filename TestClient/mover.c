#include "wininclude.h"
#include "mover.h"
#include "cmdcommon.h"
#include "player.h"
#include "utils.h"
#include "mathutil.h"
#include "memcheck.h"
#include "cmdcontrols.h"
#include "assert.h"
#include "mathutil.h"
#include "entity.h"

static char *state_names[] = {
	"MS_NOT_MOVING", // Moving is disabled
	"MS_AT_DESTINATION", // We've made it to our target
	"MS_HEAD_TOWARDS_TARGET", // We're on our way, as the crow flies
	"MS_HEAD_TOWARDS_TARGET_TEMP_RANDOM", // We still want this target, but we're gonna move randomly for a bit
	"MS_RANDOM", // We're moving randomly for a bit, then we'll head back to the target
};

int display_move_messages=0;

int cmdGetTimeStamp()
{
	return timeGetTime();
}

#define updateControlStateM(id) updateControlState(id, MOVE_INPUT_CMD, control_state.move_input[MOVE_INPUT_CMD][id])

void updateControlState(ControlId id, MovementInputSet set, int state, int time_stamp)
{
	ControlStateChange* csc;
	int i;
	int on = 0;

	if(state)
		control_state.move_input[set] |= (1 << id);
	else
		control_state.move_input[set] &= ~(1 << id);

	for(i = 0; i < MOVE_INPUT_COUNT; i++)
		on |= (control_state.move_input[i] >> id) & 1;

	if(on == ((control_state.dir_key.down_now >> id) & 1))
		return;
		
	if(on)
		control_state.dir_key.down_now |= (1 << id);
	else
		control_state.dir_key.down_now &= ~(1 << id);

	csc = createControlStateChange();

	csc->control_id = id;
	csc->time_stamp_ms = time_stamp;
	csc->state = on;

	addControlStateChange(&control_state.csc_processed_list, csc, 1);
}


MoverState mover_state=MS_NOT_MOVING;
TurnType need_to_turn=TT_NO;
static int temp_random=0;
static int temp_random_length=200; // Total number of ticks to go randomly, divided by 10
static int jumping=0;
static int stuck_count = 0;
static Entity *destEnt=NULL;
#define TOL 0.5
#define DEST_TOL 10.0
static int running=0;

void startRunning(void) {
	if (!(control_state.dir_key.down_now & (1 << CONTROLID_FORWARD))) {
		if (display_move_messages) {
			printf("MOVER.C: Starting running\n");
		}
		updateControlState(CONTROLID_FORWARD, MOVE_INPUT_CMD, 1, timeGetTime());
	}
	running=1;
}

void stopRunning(void) {
	if (control_state.dir_key.down_now && (1 << CONTROLID_FORWARD)) {
		if (display_move_messages) {
			printf("MOVER.C: Stopping running\n");
		}
		updateControlState(CONTROLID_FORWARD, MOVE_INPUT_CMD, 0, timeGetTime());
	}
	running=0;
}

static void switchState(MoverState newstate) {
	switch (newstate) {
		case MS_AT_DESTINATION:
			if (running)
				stopRunning();
			break;
		case MS_HEAD_TOWARDS_TARGET:
			assert(destEnt);
			if (!running)
				startRunning();
			need_to_turn = TT_TOWARDS_DEST;
			break;
		case MS_HEAD_TOWARDS_TARGET_TEMP_RANDOM:
			if (!running)
				startRunning();
			need_to_turn = TT_RANDOM;
			temp_random = temp_random_length / 10;
			if (display_move_messages)
				printf("MOVER.C: Moving randomly for %d TCTicks\n", temp_random);
			temp_random_length += 200; // Make the length longer next time
			break;
		case MS_RANDOM:
			if (!running)
				startRunning();
			need_to_turn = TT_RANDOM;
			break;
	}
	if (mover_state != newstate && display_move_messages) {
		printf("New state of %s\n", state_names[newstate]);
	}
	mover_state = newstate;
}

void setDestRandom() {
	if (mover_state!=MS_NOT_MOVING) {
		switchState(MS_RANDOM);
	}
}

void setDest(Vec3 pos) {
	static Entity temp;
	if (temp_random > 0)
		return;
/*	if (destEnt == &temp
		&& pos[0]==temp.mat[3][0]
		&& pos[1]==temp.mat[3][1]
		&& pos[2]==temp.mat[3][2])
	{
		return;
	}*/
	temp.owner = 0xbadf00d;
	copyVec3(pos, temp.fork[3]); // owner is invalid so no ENTPOS
	setDestEnt(&temp);
}

void setDestEnt(Entity *target) {
	if (destEnt == target && mover_state!=MS_NOT_MOVING && mover_state!=MS_RANDOM)
		return;
	destEnt = target;
	if (mover_state!=MS_NOT_MOVING) {
		if (mover_state==MS_HEAD_TOWARDS_TARGET_TEMP_RANDOM && temp_random>0) {
			// ignore
		} else {
			switchState(MS_HEAD_TOWARDS_TARGET);
			need_to_turn = TT_TOWARDS_DEST;
		}
	} else {
		need_to_turn = TT_TOWARDS_DEST;
	}
}

void doJump(void) {
	if (!jumping) {
		jumping = 3;
		updateControlState(CONTROLID_UP, MOVE_INPUT_CMD, 1, timeGetTime());
	}
}

void stopJump(void) {
	if (jumping) {
		jumping--;
		if (!jumping) {
			updateControlState(CONTROLID_UP, MOVE_INPUT_CMD, 0, timeGetTime());
		}
	}
}

int atDest() {
	return mover_state == MS_AT_DESTINATION;
}

static void checkTurnToFace(Entity *e) { // e == player_ent
	/*
	// this broke, is it needed?
	if( e->guy_im_turning_to_face )
	{
		Vec3 delta;
		if (display_move_messages)
			printf("MOVER.C: Turning to face Entity %s\n", e->name);
		//#### who am I facing? check this ent, turn off facing if bad...
		if(e->guy_im_turning_to_face->svr_idx != e->guy_im_turning_to_face_svr_idx)
		{
			e->guy_im_turning_to_face = 0;
			e->guy_im_turning_to_face_svr_idx = 0;
		} else {
			// Face the sky... err... this guy
			subVec3(e->guy_im_turning_to_face->mat[3], e->mat[3], delta);
			control_state.pyr[1] = getVec3Yaw(delta);
			e->guy_im_turning_to_face = 0;
			e->guy_im_turning_to_face_svr_idx = 0;
		}
	}
	*/
}

static void checkTurning(Entity *e) { // e == player_ent
	Vec3 delta;
	switch (need_to_turn) {
		case TT_RANDOM:
			if (display_move_messages)
				printf("MOVER.C: Turning randomly\n");
			control_state.pyr.cur[1] = addAngle(control_state.pyr.cur[1], 2 * PI * rand()/RAND_MAX);
			//printf("random yaw: %f\n", DEG(control_state.pyr.cur[1]));
			break;
		case TT_TOWARDS_DEST:
			if (!destEnt || destEnt->owner<=0)
				return;
			//if (display_move_messages)
			//	printf("MOVER.C: Turning towards destination entity\n");
			subVec3(ENTPOS(destEnt), ENTPOS(e), delta);
			control_state.pyr.cur[1] = getVec3Yaw(delta);
			//printf("yaw to %s: %f\n", destEnt->name, DEG(control_state.pyr.cur[1]));
			break;
		case TT_NO:
		default:
			break;
	}
	need_to_turn = TT_NO;
}

static void checkStuckCount(Entity *e) {
	static Vec3 hist[4] = {{0,0,0},{0,0,0},{0,0,0},{0,0,0}};
	Vec3 delta;

	subVec3(ENTPOS(e), hist[0], delta);
	memcpy(hist[0], hist[1], sizeof(hist[0][0])*3*3);
	copyVec3(ENTPOS(e), hist[3]);
	delta[1]=0; // Ignore vertical
	//printf("Delta: %1.3f\n", lengthVec3(delta));

	if (lengthVec3(delta) < TOL) {
		stuck_count++;
	} else {
		stuck_count--;
		if (stuck_count<=1) stuck_count=0;
	}
}

void doMoverLoop(void) {
	ControlStateChange* csc;
	int i;
	Entity *e = playerPtr();

	stopJump();
	for (i=0; i<6; i++) {
		csc = createControlStateChange();

		csc->control_id = CONTROLID_RUN_PHYSICS;
		csc->time_stamp_ms = timeGetTime();
		csc->controls_input_ignored = 0;
		csc->client_abs = global_state.client_abs;
		csc->client_abs_slow = global_state.client_abs_slow;
		csc->inp_vel_scale_U8 = 255;

		addControlStateChange(&control_state.csc_processed_list, csc, 1);
	}

	if (!e)
		return;

	if (mover_state==MS_HEAD_TOWARDS_TARGET || mover_state==MS_AT_DESTINATION) {
		need_to_turn = TT_TOWARDS_DEST;
	}
	checkTurning(e);

	if (mover_state==MS_NOT_MOVING)
		return;


	if (mover_state==MS_HEAD_TOWARDS_TARGET || mover_state==MS_AT_DESTINATION)
	{
		// Calc distance to the destination
		Vec3 distToDest;
		F32 length;
		int at_dest;

		if (!destEnt || destEnt->owner<=0) {
			destEnt=NULL;
			switchState(MS_RANDOM);
		} else {
			subVec3(ENTPOS(e), ENTPOS(destEnt), distToDest);
			distToDest[1]=0;
			length = lengthVec3(distToDest);
			at_dest = (length < DEST_TOL);

			//if (display_move_messages)
			//	printf("dist to dest %1.3f\n", length);

			if (!at_dest && mover_state==MS_AT_DESTINATION) {
				switchState(MS_HEAD_TOWARDS_TARGET);
			}
			if (at_dest && mover_state==MS_HEAD_TOWARDS_TARGET) {
				switchState(MS_AT_DESTINATION);
			}
		}
	}

	// Check for temp_random expiring
	temp_random_length--;
	if (temp_random_length < 100) temp_random_length=100;
	if (mover_state==MS_HEAD_TOWARDS_TARGET_TEMP_RANDOM) {
		temp_random--;
		if (temp_random<=0)
			switchState(MS_HEAD_TOWARDS_TARGET);
	}


	// Check for getting stuck
	if (mover_state!=MS_AT_DESTINATION) {
		checkStuckCount(e);
		if (stuck_count==1) {
			doJump();
		}

		if (stuck_count > 10) { // We're pretty stuck
			switch (mover_state) {
				case MS_RANDOM:
					need_to_turn = TT_RANDOM;
					checkTurning(e);
					stuck_count=0;
					break;
				case MS_HEAD_TOWARDS_TARGET:
					switchState(MS_HEAD_TOWARDS_TARGET_TEMP_RANDOM);
					stuck_count=0;
					break;
				case MS_HEAD_TOWARDS_TARGET_TEMP_RANDOM:
					switchState(MS_HEAD_TOWARDS_TARGET);
					stuck_count=0;
					temp_random_length -= 100;
					break;
			}
		}
	}
	checkTurnToFace(e);
}

void startMoving() {
	if (mover_state==MS_NOT_MOVING) {
		mover_state=MS_RANDOM;
		setDestRandom();
	}
}

void stopMoving() {
	stopRunning();
	mover_state=MS_NOT_MOVING;
}

void setDestMap(const char *mapname)
{
	//TODO: implementme
}

