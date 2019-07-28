#include "testMissions.h"
#include "testClientInclude.h"
#include "testUtil.h"
#include "testContact.h"
#include "assert.h"
#include "mover.h"
#include "clientcomm.h"
#include "chatter.h"
#include "player.h"
#include "mathutil.h"
#include "entity.h"
#include "dooranimclient.h"

typedef enum TestMissionState {
	TMS_NEED_A_MISSION,
	TMS_GO_TO_MISSION,
	TMS_ENTER_MISSION,
	TMS_WAITING_FOR_MAP_XFER,
	TMS_ON_MISSION,
	TMS_EXIT_MISSION,
	TMS_RETURN_TO_CONTACT,
} TestMissionState;

char *TMS_Names[] = {
	"TMS_NEED_A_MISSION",
	"TMS_GO_TO_MISSION",
	"TMS_ENTER_MISSION",
	"TMS_WAITING_FOR_MAP_XFER",
	"TMS_ON_MISSION",
	"TMS_EXIT_MISSION",
	"TMS_RETURN_TO_CONTACT",
};

static TestMissionState tms = TMS_NEED_A_MISSION;
static bool in_ignore_state = false;
static int talked_to_contact = 0;
bool cheat_to_get_places = true;
bool cheat_to_pass_missions = false; // Turn this on and TestClients will cycle through missions at one every 10-20 seconds
bool cheat_if_not_on_map = false;

void startIgnoring(void)
{
	if (in_ignore_state)
		return;
	modePush();
	in_ignore_state = true;
	g_testMode &= ~(TEST_FIGHT | TEST_FOLLOW);
	g_testMode |= TEST_MOVE;
}

void stopIgnoring(void)
{
	if (!in_ignore_state)
		return;
	in_ignore_state = 0;
	modePop();
}

#define commAddInput(s) printf("commAddInput(\"%s\");\n", s); commAddInput(s);

bool atThisDest(Destination *dest) {
	Entity *e = playerPtr();
	Vec3 distToDest;
	F32 length;

	if (!dest)
		return false;

	subVec3(ENTPOS(e), dest->world_location[3], distToDest);
	distToDest[1]=0;
	length = lengthVec3(distToDest);
	if (length < 10.0)
		return true;
	return false;
}

int was_near_dest=0;
bool just_transfered_map=0;

static bool g_mission_hop_transfered_map;

void checkMissionMapXferCallback(void) {
	g_mission_hop_transfered_map = true;
	if (was_near_dest) {
		just_transfered_map = true;
	}
}

void checkMissions(void)
{
	static int delay=0; // Delays the execution of this function, used to wait for server data to get to the client
	Destination *dest;
	TaskType task_type;
	TestMissionState old_tms = tms;
	static int retry_count=0;

	if (delay) {
		delay--;
		return;
	}

	if (talked_to_contact)
		talked_to_contact--;
	if (was_near_dest)
		was_near_dest--;

	task_type = getCurrentTaskType();

	if (task_type==TASK_NOTYPE) {
		// We don't have a mission!
		tms = TMS_NEED_A_MISSION;
	}

	if (tms == TMS_NEED_A_MISSION) {
		if (task_type != TASK_NOTYPE && task_type != TASK_COMPLETED) {
			tms = TMS_GO_TO_MISSION;
		} else {
			// Need a mission
			// New hackery
			commAddInput("missionx");
			delay=5;
		}
	} else if (tms == TMS_NEED_A_MISSION || tms == TMS_RETURN_TO_CONTACT) {
		if (tms == TMS_NEED_A_MISSION && task_type != TASK_NOTYPE) {
			tms = TMS_GO_TO_MISSION;
		} else {
			// Try to get near the contact
			dest = findContactLocation();
			if (!dest) {
				//don't care  assert(!"no contact to go to!");
				if (tms == TMS_RETURN_TO_CONTACT) {
					tms = TMS_NEED_A_MISSION;
				}
			}
			if (isDestOnThisMap(dest)) {
				setDest(dest->world_location[3]);
			} else {
				setDestMap(dest->mapName);
			}
			startIgnoring();
			if (atThisDest(dest) && !talked_to_contact) {
				if (retry_count == 10) {
					// Not getting anywhere!
					tms = TMS_NEED_A_MISSION;
				}
				if (testChatNPC()) {
					delay=1;
					talked_to_contact = 5;
					retry_count++;
				}
				was_near_dest=1;
			} else if (!atThisDest(dest) && cheat_to_get_places && isDestOnThisMap(dest)) {
				// Not at destination and we can cheat
				char buf[256];
				sprintf(buf, "setpos %1.5f %1.5f %1.5f", dest->world_location[3][0], dest->world_location[3][1], dest->world_location[3][2]);
				commAddInput(buf);
				delay=1;
				return;
			} else if (!isDestOnThisMap(dest) && cheat_if_not_on_map) {
				commAddInput("mapmove 1");
				delay = 5;
				return;
			}
		}
	}
	if (tms==TMS_GO_TO_MISSION) {
		if (task_type==TASK_COMPLETED) {
			// tms = TMS_RETURN_TO_CONTACT;
			tms = TMS_NEED_A_MISSION;
			commAddInput("completemission"); // finished the task, finish the mission!
			delay=5;
		} else {
			// We have a task to do!
			dest = findTaskDest();
			if (!dest) {
				switch (task_type) {
					case TASK_VISITLOCATION:
						completeCurrentTask();
						delay = 5; // Don't want to send more than one!
						tms = TMS_RETURN_TO_CONTACT;
						return;
					case TASK_KILLX:
						// We're fine, just let the normal code run around
						tms = TMS_ON_MISSION;
						break;
					case TASK_COMPLETED:
						tms = TMS_RETURN_TO_CONTACT;
						break;
					default:
						printf("Task without a location, and we don't know what to do!\n");
						completeCurrentTask();
						delay = 5; // Don't want to send more than one!
						tms = TMS_RETURN_TO_CONTACT;
						break;
				}
			} else {
				// We have a destination to go to!  Go for it!
				// New hackery
				if (1) {
					if (just_transfered_map && task_type==TASK_MISSION) {
						tms = TMS_ON_MISSION;
					} else if (!atThisDest(dest)) {
						retry_count++;
						if (retry_count == 10) {
							completeCurrentTask();
							retry_count = 0;
						} else {
							commAddInput("gotomission");
						}
						delay = 5;
						return;
					}
				} else if (!isDestOnThisMap(dest)) {
					if (cheat_if_not_on_map) {
						completeCurrentTask();
						delay = 5; // Don't want to send more than one!
						return;
					}
					setDestMap(dest->mapName);
				} else {
					// Keep headin' that direction!
					setDest(dest->world_location[3]);
				}
				startIgnoring();
				if (task_type==TASK_MISSION) {
					// We are not on it until we zone!
					if (atThisDest(dest)) {
						was_near_dest=5;
					} else if (!atThisDest(dest) && cheat_to_get_places && isDestOnThisMap(dest)) {
						// Not at destination and we can cheat
						char buf[256];
						sprintf(buf, "setpos %1.5f %1.5f %1.5f", dest->world_location[3][0], dest->world_location[3][1], dest->world_location[3][2]);
						commAddInput(buf);
						delay=1;
						return;
					}
					stopIgnoring();
					if (testChatNPC())
						delay=5;
				} else {
					if (atThisDest(dest)) {
						tms = TMS_ON_MISSION;
						was_near_dest=5;
					} else if (!atThisDest(dest) && cheat_to_get_places && isDestOnThisMap(dest)) {
						// Not at destination and we can cheat
						char buf[256];
						sprintf(buf, "setpos %1.5f %1.5f %1.5f", dest->world_location[3][0], dest->world_location[3][1], dest->world_location[3][2]);
						commAddInput(buf);
						delay=1;
						return;
					}
				}
			}
		}
	}
	if (tms == TMS_ON_MISSION) {
		// Talk to NPCs nearbye, it might be part of the mission!
		if (task_type!=TASK_KILLX && task_type!=TASK_MISSION) // But not if we're on a mission, we'll just use the door to exit!
			if (testChatNPC())
				delay=1;
		if (task_type==TASK_COMPLETED)
			tms = TMS_RETURN_TO_CONTACT;
		else
			stopIgnoring();

		if (task_type==TASK_MISSION && cheat_to_pass_missions) {
			static int completemission=-1;
			if (completemission==-1) {
				completemission = 100 + rand()%40;
			} else if (completemission==0) {
				completeCurrentTask();
				delay=5;
				completemission=-1;
			} else {
				completemission--;
			}
		}
	}
	if (tms != old_tms) {
		talked_to_contact = 0;
		retry_count = 0;
		printf("testMission.c: TestMissionState changed to %s\n", TMS_Names[tms]);
	}
	just_transfered_map = false;
}

// new 'mission hop' mode which is used to have test clients generate map load
// on servers. The older 'mission mode' only infrequently ends up spawning a map
// and appears to have logic problems in that clients that accept a mission which
// requires a map transfer just spend time around the door. That code appears to be
// a mix of a few different historical approaches and needs some repairs or a complete rewrite.

// accept a random mission which requires a map transfer and then exit the mission
// after the after time_on_mission expires. Small values for time_on_mission will
// cause the client to hop frequently from mission to mission and can be used
// to generate map instance load on a server

static TestMissionState s_mission_hop_state = TMS_NEED_A_MISSION;
static int s_server_delay;
static U32 s_state_begin_time;

static bool waiting_for_server()
{
	if (s_server_delay) {
		s_server_delay--;
		return true;
	}
	return false;
}

static void wait_for_server( int delay )
{
	s_server_delay = delay;
}

static void state_timer_reset(void)
{
	s_state_begin_time = 0;
}

static bool state_timer_expired( unsigned time_in_state )
{
	// we should be on mission, hang around for the requested mission time and then exit
	// do we need to make sure we've completed the map xfer?
	if (s_state_begin_time)
	{
		if (timerSecondsSince2000()-s_state_begin_time > time_in_state)
			return true;
	}
	else
	{
		// we just entered the state so seed the duration start time
		s_state_begin_time = timerSecondsSince2000();
	}
	return false;
}

void checkMissionHop( unsigned time_on_mission )
{
	if (waiting_for_server())
		return;

	switch (s_mission_hop_state)
	{
	case TMS_NEED_A_MISSION:
		{
			printf("mission: randomly selecting a mission task with an instance map...\n");
			// randomly select a contact and task that requires going to an instanced mission map
			commAddInput("missionxmap");
			s_mission_hop_state = TMS_GO_TO_MISSION;
			wait_for_server(5);
		}
		break;
	case TMS_GO_TO_MISSION:
		{
			// ask server to teleport us to the door for the mission, even if it is in a different static map zone
			commAddInput("gotomission");
			s_mission_hop_state = TMS_ENTER_MISSION;
			wait_for_server(5);
		}
		break;
	case TMS_ENTER_MISSION:
		{
			// enter the door, we will then need to wait for the map xfer to complete
			// can get door location from the selected task but we can also just use the
			// player position as 'gotomission' should have put us in range to 'click' the door
			Entity *e = playerPtr();
//			char s[256];
//			sprintf_s(s, sizeof(s), "enterdoor %f %f %f 0", ENTPOSX(e), ENTPOSY(e) ,ENTPOSZ(e));
//			commAddInput(s);
			enterDoorClient(ENTPOS(e), "0");
			s_mission_hop_state = TMS_WAITING_FOR_MAP_XFER;
			state_timer_reset();
			wait_for_server(5);
			g_mission_hop_transfered_map = false;
			printf("mission: entering mission door and waiting for map xfer...\n");
		}
		break;
	case TMS_WAITING_FOR_MAP_XFER:
		{
			// wait for map xfer

			// we have a timeout (2 minutes) so we don't get stuck here forever
			if (state_timer_expired(120))
			{
				s_mission_hop_state = TMS_EXIT_MISSION;
				state_timer_reset();
			}

			if (g_mission_hop_transfered_map)
			{
				s_mission_hop_state = TMS_ON_MISSION;
				state_timer_reset();
			}
		}
		break;

	case TMS_ON_MISSION:
		{
			// we should be on mission, hang around for the requested mission time and then exit
			// do we need to make sure we've completed the map xfer?
			if (state_timer_expired(time_on_mission))
			{
				s_mission_hop_state = TMS_EXIT_MISSION;
				state_timer_reset();
			}
		}
		break;
	case TMS_EXIT_MISSION:
		{
			// time to leave
			commAddInput("exitmission");
			s_mission_hop_state = TMS_NEED_A_MISSION;	// grab a new mission
			wait_for_server(5);
			printf("mission: exiting mission and selecting a new one...\n");
		}
		break;
	}
}
