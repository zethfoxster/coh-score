#include "ArenaFighter.h"
#include "clientcomm.h"
#include "stdio.h"
#include "stdlib.h"
#include "testClientInclude.h"
#include "arenastruct.h"
#include "earray.h"
#include "entity.h"
#include "uiNet.h"
#include "character_base.h"
#include "gameData/randomName.h"
#include "Supergroup.h"
#include "utils.h"

typedef enum {
	ARENA_FIGHTER_MAPMOVE,
	ARENA_FIGHTER_WAIT_MAP,
	ARENA_FIGHTER_CHECK_MY_EVENTS,
	ARENA_FIGHTER_KIOSK,
	ARENA_FIGHTER_WAIT_KIOSK,
	ARENA_FIGHTER_JOIN,
	ARENA_FIGHTER_WAIT_PARTICIPANTS,
	ARENA_FIGHTER_READY,
	ARENA_FIGHTER_WAIT_ARENA,
	ARENA_FIGHTER_WAIT_ENDROUND,
	ARENA_FIGHTER_DESTROY,
	ARENA_FIGHTER_LASTSTATE,


	EVENT_CREATOR_MAPMOVE=ARENA_FIGHTER_LASTSTATE,
	EVENT_CREATOR_WAIT_MAP,
	EVENT_CREATOR_CREATE,
	EVENT_CREATOR_WAIT_PARTICIPANTS,
	EVENT_CREATOR_READY,
	EVENT_CREATOR_WAIT_ARENA,
	EVENT_CREATOR_WAIT_ENDROUND,
	EVENT_CREATOR_DESTROY
} ArenaFighterState;

char * stateNames[] = {
	"Map Move",
	"Wait Map",
	"Check My Events",
	"Kiosk",
	"Wait Kiosk",
	"Join",
	"Wait Participants",
	"Ready",
	"Wait Arena",
	"Wait Endround",
	"Destroy",
	"Creator Mapmove",
	"Creator Waitmap",
	"Creator Create",
	"Creator Wait Participants",
	"Creator Ready",
	"Creator Wait Arena",
	"Creator Wait Endround",
	"Creator Destroy"
};

char * phaseNames[] = {
	"Scheduled",
	"Negotiating",
	"Start Round",
	"Run Round",
	"End Round",
	"Prize Payout",
	"Ended",
	"Cancel Refund",
	"Cancelled"
};

ArenaFighterState myState;
extern ArenaEventList g_kiosk;
extern ArenaEvent * g_event;
ArenaEvent myEvent;
ArenaParticipant myParticipant;
int myTimer;
double myThrashRate;	//what percentage of commands will be random
int startFighting;
int exiting;
int gotToArenaMap=0;
int locationSet;

extern Entity * playerPtr();
extern void requestArenaKiosk(int,int);
extern int amOnArenaMap();
extern Entity * current_target;
extern char * g_arenastring;

//supergroup stuff

void quitSupergroup() {
	char cmd[64];
	sprintf(cmd,"teamleave 6");
	commAddInput(cmd);
}

void createArenaSupergroup() {
	Supergroup sg;
	if (!(g_testMode & TEST_ARENA_SUPERGROUP))
		return;
	strncpyt(sg.name, randomName(), ARRAY_SIZE(sg.name));
	strncpyt(sg.description, randomName(), ARRAY_SIZE(sg.description));
	strncpyt(sg.motto, randomName(), ARRAY_SIZE(sg.motto));
	strncpyt(sg.msg, randomName(), ARRAY_SIZE(sg.msg));
	strncpyt(sg.rankList[0].name, randomName(), ARRAY_SIZE(sg.rankList[0].name));
	strncpyt(sg.rankList[1].name, randomName(), ARRAY_SIZE(sg.rankList[1].name));
	strncpyt(sg.rankList[2].name, randomName(), ARRAY_SIZE(sg.rankList[2].name));
	strncpyt(sg.rankList[3].name, randomName(), ARRAY_SIZE(sg.rankList[3].name));
	strncpyt(sg.rankList[4].name, randomName(), ARRAY_SIZE(sg.rankList[4].name));
	sg.rankList[0].permissionInt[0] = rand();
	sg.rankList[1].permissionInt[0] = rand();
	sg.rankList[2].permissionInt[0] = rand();
	sg.rankList[3].permissionInt[0] = rand();
	sg.rankList[4].permissionInt[0] = rand();
	strncpyt(sg.emblem, "Atom_02", ARRAY_SIZE(sg.emblem));
	sgroup_sendCreate(&sg );
}

void arenaFighterDialog(const char * s) {
	if (strstr(s,"join a supergroup")!=NULL) {
		sendSuperGroupAccept(0);
	}
}

//end supergroup stuff

#define SWITCH_STATE(x)	switchFighterState(x, __LINE__)
void switchFighterState(ArenaFighterState nextstate, int line_number)
{
	printf("Switched to %s (on line %i)\n", stateNames[nextstate], line_number);
	myState = nextstate;
}

void exitArenaMap() {
	printf("Requesting return to static map.\n");
	if (myState>=ARENA_FIGHTER_LASTSTATE)
		SWITCH_STATE(EVENT_CREATOR_WAIT_MAP);
	else
		SWITCH_STATE(ARENA_FIGHTER_WAIT_MAP);
	myTimer=50;
	exiting=0;
}

void prepareToExitArenaMap() {	
	if (myState!=ARENA_FIGHTER_DESTROY && myState!=EVENT_CREATOR_DESTROY)
		return;
	printf("Game Over!!!\n");
//	memset(&myEvent,-1,sizeof(ArenaEvent));
	myTimer=1;
	startFighting=0;
	exiting=1;
	exitArenaMap();
}

void arenaFighterHandleFloatingText(char * pch) {
	if (stricmp(pch,"ArenaFight")==0) {
		startFighting=1;
	} else
	if (stricmp(pch,"HaveWinner")==0) {
		if (myState==ARENA_FIGHTER_DESTROY || myState==EVENT_CREATOR_DESTROY) {
			prepareToExitArenaMap();
		}
	}
}



ArenaEvent randomArenaEvent() {
	ArenaEvent ae;
	ae.listed=rand()%100-50;

	ae.teamType=rand()%100-50;
	ae.teamStyle=rand()%100-50;

	ae.no_setbonus=rand()%100-50;
	ae.no_travel=rand()%100-50;
	ae.no_stealth=false;//rand()%100-50;
	ae.no_observe=rand()%100-50;
	ae.no_chat=rand()%100-50;

	ae.numsides=rand()%100-50;
	ae.maxplayers=rand()%100-50;

	return ae;
}

ArenaParticipant randomArenaParticipant() {
	ArenaParticipant ap;
	int i;
	ap.dbid=rand()%100-50;
	ap.desired_team=rand()%100-50;
	ap.dropped=rand()%100-50;
	ap.entryfee=rand()%100-50;
	ap.id=rand()%100-50;
	for (i=0;i<ARENA_MAX_ROUNDS;i++)
		ap.kills[i]=rand()%100-50;
	ap.level=rand()%1000-500;
	ap.rating=rand()%10000-5000;
	ap.side=rand()%100-50;

	ap.archetype=NULL;
	ap.desired_archetype=NULL;
	ap.name=NULL;

	return ap;
}

void randomArenaCommand() {
	int n=rand()%8;
	char cmd[64];
	ArenaEvent ae=randomArenaEvent();
	ArenaParticipant ap=randomArenaParticipant();
	switch(n) {
		case 0:
			sprintf(cmd,"arenacreate %s","thrashed");
			commAddInput(cmd);
			printf("Thrash: %s\n",cmd);
		xcase 1:
			sprintf(cmd,"%s %d %d","arenajoin",rand()%1000,rand()%1000);
			commAddInput(cmd);
			printf("Thrash: %s\n",cmd);
		xcase 2:
			printf("Thrash: requestReturnToStatic()\n");
		xcase 3:
			requestArenaKiosk(ARENA_FILTER_ALL,ARENA_TAB_ALL);
			printf("Thrash: requestArenaKiosk()\n");
		xcase 4:
			arenaSendCreatorUpdate(&ae);
			printf("Thrash: arenaSendCreatorUpdate()\n");
		xcase 5:
			arenaSendPlayerUpdate(&ae,&ap);
			printf("Thrash: arenaSendPlayerUpdate()\n");
		xcase 6:
			ae.creatorid=playerPtr()->db_id;	
			arenaSendCreatorUpdate(&ae);
			printf("Thrash: arenaSendCreatorUpdate()\n");
		xcase 7:
			ae.creatorid=playerPtr()->db_id;
			ap.dbid=playerPtr()->db_id;
			arenaSendPlayerUpdate(&ae,&ap);
			printf("Thrash: arenaSendPlayerUpdate()\n");
	}
}

void createNewEvent() {
	char cmd[64];
	sprintf(cmd,"arenacreate %s","thunderforce");
	commAddInput(cmd);
}

void checkForRandomEvent() {
	int i;
	int joined=0;
	char cmd[64];
	if (eaSize(&g_kiosk.events)==0) 
		return;
	i=rand()%eaSize(&g_kiosk.events);
	memcpy(&myEvent,g_kiosk.events[i],sizeof(ArenaEvent));
	sprintf(cmd,"%s %d %d","arenajoin",myEvent.eventid,myEvent.uniqueid);
	commAddInput(cmd);
	printf("Thrash: %s",cmd);
	myTimer=50;
	SWITCH_STATE(myState+1);
}

void checkForEvent() {
	int i;
	int joined=0;
	
	if (g_testMode&TEST_ARENA_THRASHER) {
		checkForRandomEvent();
		return;
	}
	for(i = 0; i < eaSize(&g_kiosk.events); i++)
	{
		if(myEvent.eventid == g_kiosk.events[i]->eventid &&
			myEvent.uniqueid == g_kiosk.events[i]->uniqueid)
		{
			// our event is still valid, don't leave yet
			myTimer = 1000;
			SWITCH_STATE(ARENA_FIGHTER_WAIT_ARENA);
			return;
		}
	}
	for (i=0;i<eaSize(&g_kiosk.events);i++) {
		if (TEST_ARENA_SCHEDULED || strcmp(g_kiosk.events[i]->creatorname,"System")!=0) {
			memcpy(&myEvent,g_kiosk.events[i],sizeof(ArenaEvent));
			arenaSendJoinRequest(myEvent.eventid,myEvent.uniqueid,0);
			joined=1;
			break;
		}
	}
	myTimer=5;
	if (!joined) {
		SWITCH_STATE(ARENA_FIGHTER_KIOSK);
	} else {
		SWITCH_STATE(myState+1);
	}
}

/*
void checkForEndRound() {
	int i;
	int requested=0;
	return;
	myTimer=50;
	for (i=0;i<eaSize(&g_kiosk.events);i++) {
		if (myEvent.eventid==g_kiosk.events[i]->eventid &&
			myEvent.uniqueid==g_kiosk.events[i]->uniqueid) {
				if (g_kiosk.events[i]->phase>=ARENA_ENDROUND) {
					requestReturnToStatic();
					if (myState>=ARENA_FIGHTER_LASTSTATE)
						SWITCH_STATE(EVENT_CREATOR_WAIT_MAP);
					else
						SWITCH_STATE(ARENA_FIGHTER_WAIT_MAP);
					requested=1;
				} else {
					return;
				}
			}
	}
	if (myState>=ARENA_FIGHTER_LASTSTATE)
		SWITCH_STATE(EVENT_CREATOR_WAIT_MAP);
	else
		SWITCH_STATE(ARENA_FIGHTER_WAIT_MAP);
	requestReturnToStatic();
}
*/

//this is called when the arena kiosk comes up, so we know to move on to the next step
void arenaRebuildListView() {
	if (!(g_testMode&TEST_ARENA_FIGHTER))
		return;

	if (myState<=ARENA_FIGHTER_WAIT_KIOSK) {
		if (g_testMode&TEST_ARENA_CREATOR)
			createNewEvent();
		else
			checkForEvent();
	}
/*
	else {
			if (myState>=ARENA_FIGHTER_WAIT_ENDROUND && myState<ARENA_FIGHTER_LASTSTATE)
				checkForEndRound();
			if (myState>=EVENT_CREATOR_WAIT_ENDROUND)
				checkForEndRound();
	}
*/	
}


void setSelfReady() {
	int i;
	for (i=0;i<eaSize(&g_event->participants);i++) {
		if (playerPtr()->db_id==g_event->participants[i]->dbid) {
			//we are in the event, move on to the next step
			memcpy(&myParticipant,g_event->participants[i],sizeof(ArenaParticipant));
//			if (!myParticipant.ready) {
//				myState=ARENA_FIGHTER_READY;
//			}
		}
	}
}

void updateWithReady() {
	myParticipant.dbid=playerPtr()->db_id;
//	myParticipant.ready=1;
	arenaSendPlayerUpdate(&myEvent,&myParticipant);
}

//this gets called if we won an event, set myEvent to nothing, go back to asking for a kiosk
void arenaRebuildResultView(ArenaRankingTable * ranking) {
	if (myState==ARENA_FIGHTER_DESTROY) {
		prepareToExitArenaMap();
	}
}


void checkForDrop() {
	int i;
	for (i=0;i<eaSize(&g_event->participants);i++)
		if (strcmp(g_event->participants[i]->name,g_event->creatorname)==0)
			return;
	arenaSendPlayerDrop(&myEvent,playerPtr()->db_id);
	SWITCH_STATE(ARENA_FIGHTER_KIOSK);
}

void waitForPlayers() {
	if (!g_event->listed) {
		g_event->listed=1;

		if (g_testMode & TEST_ARENA_SUPERGROUP) {
			g_event->teamType = ARENA_SUPERGROUP;// ARENA_TEAM;// rand()%2;	//either single or team, no supergroups
			g_event->teamStyle = ARENA_FREEFORM;
			g_event->victoryType = ARENA_TIMED;
			g_event->victoryValue = 20;
			g_event->numsides = 2;
			g_event->maxplayers = 12;
		} else {
			g_event->teamType = ARENA_SINGLE;// ARENA_TEAM;// rand()%2;	//either single or team, no supergroups
			g_event->teamStyle = ARENA_FREEFORM;
			g_event->victoryType = ARENA_LASTMAN;
			g_event->victoryValue = 1;
			g_event->numsides = 0;//rand()%3+1;
			g_event->maxplayers = rand()%5+2;
		}


		g_event->no_setbonus = rand()%2;
		g_event->no_travel = rand()%2;
		g_event->no_stealth = false;//rand()%2;
		g_event->no_observe = 0;//rand()%2;
		g_event->no_chat = 0;

		arenaSendCreatorUpdate(g_event);
	} else {
		if (g_event->numplayers==g_event->maxplayers) {
			SWITCH_STATE(EVENT_CREATOR_READY);
		}
	}
}

//this should be called when the even is updated with our request to join, now we can
//set ourselves as ready
void arenaApplyEventToUI(void) {
	if (!(g_testMode&TEST_ARENA_FIGHTER))
		return;
	if (myState==ARENA_FIGHTER_WAIT_MAP || myState==EVENT_CREATOR_WAIT_MAP)
		return;
	if (g_event->phase>ARENA_RUNROUND)
		return;
	memcpy(&myEvent,g_event,sizeof(ArenaEvent));
	if (myEvent.phase>=0 && myEvent.phase<=ARENA_CANCELLED) {
		printf("%s ",phaseNames[myEvent.phase]);
		printf("eventid:%d uniqueid:%d \n",myEvent.eventid,myEvent.uniqueid);
	}
	if ((!(g_testMode&TEST_ARENA_CREATOR)))// && myState==ARENA_FIGHTER_JOIN)
		setSelfReady();
//	else
	if (myState==EVENT_CREATOR_WAIT_PARTICIPANTS)
		waitForPlayers();
	if (myEvent.phase==ARENA_RUNROUND){
		if (myState>ARENA_FIGHTER_LASTSTATE)
			SWITCH_STATE(EVENT_CREATOR_WAIT_ENDROUND);
		else
			SWITCH_STATE(ARENA_FIGHTER_WAIT_ENDROUND);
	}
/*		else
			if (myState==ARENA_FIGHTER_WAIT_ARENA)
				checkForDrop();
*/
}

void killingMachine() {
	static int killerTimer=50;
	char cmd[64];
	killerTimer--;
	if (playerPtr()->pchar->attrCur.fHitPoints==0)
		return;
	if (killerTimer==5) {
		commAddInput("nextplayer");
	}
	if (killerTimer<5 && current_target) {
		if (current_target->pchar->iAllyID!=kAllyID_Good && 
			current_target->pchar->iAllyID!=playerPtr()->pchar->iAllyID &&
			current_target->pchar->attrCur.fHitPoints>=1.0) {
				sprintf(cmd,"ec %d sethealth 1",current_target->id);
				commAddInput(cmd);
				killerTimer+=50;
			}
	}
	if (killerTimer==0)
		killerTimer=50;
}

void arenaFighterDestroy() {
	if (g_arenastring && strstr(g_arenastring,"ArenaFight")!=0)
		startFighting=1;
	if (g_arenastring && strstr(g_arenastring,"MatchLivesRemaining")!=0)
		startFighting=1;
	if (g_arenastring && strstr(g_arenastring,"TeamLivesRemaining")!=0)
		startFighting=1;
	if (g_arenastring && strstr(g_arenastring,"MatchTimeRemaining")!=0)
		startFighting=1;
	if (startFighting && g_testMode&TEST_ARENA_KILLBOT) {
		g_testMode|=TEST_FIGHT;
		killingMachine();
	}
	if (amOnArenaMap())
		gotToArenaMap=1;
	if (exiting)
		--myTimer;
	if (myTimer==0) {
		exitArenaMap();
	}
	if (!amOnArenaMap()) {
		myTimer=50;
		if(myState > ARENA_FIGHTER_LASTSTATE)
			SWITCH_STATE(EVENT_CREATOR_WAIT_MAP);
		else
			SWITCH_STATE(ARENA_FIGHTER_WAIT_MAP);
		gotToArenaMap=0;
		startFighting=0;
		exiting=0;
	}
}

void arenaFighterListen(const char * s) { 
	if (s && strstr(s,"you're already on map")) {
		if (myState==ARENA_FIGHTER_MAPMOVE)
			SWITCH_STATE(ARENA_FIGHTER_WAIT_MAP);
		if (myState==EVENT_CREATOR_MAPMOVE)
			SWITCH_STATE(EVENT_CREATOR_WAIT_MAP);
	}
}

void inviteNearbyFolks(double d) {
	int i;
	for (i=1; i<entities_max; i++) {
		if (!(entity_state[i] & ENTITY_IN_USE ) )
			continue;
		if ( ENTTYPE_BY_ID(i) != ENTTYPE_PLAYER )
			continue;
		if (((double)rand()/(double)RAND_MAX)<d) {
			char buf[1024];
			sprintf(buf, "sginvite %s", entities[i]->name);
			commAddInput(buf);
		}
	}
}

void setloc() {
	char buf[128];
	if (locationSet) return;
	locationSet=1;
	sprintf(buf,"setpos -768.267,-764.797,-816.324");
	commAddInput(buf);
}

void arenaFighterLoop() {
	double inviteRate=.025;
	if (g_arenastring && strcmp(g_arenastring,"MatchTimeRemaining")==0)
		startFighting=1;
	if (myThrashRate==0)
		myThrashRate=.07;
	if (g_testMode&TEST_ARENA_KILLBOT)
		g_testMode&=~TEST_FIGHT;
	if ((g_testMode&TEST_ARENA_THRASHER) && (double)rand()/(double)RAND_MAX<myThrashRate) {
		randomArenaCommand();
		return;
	}
//	printf("%c",(startFighting)?'*':' ');
//	printf("(%d)",playerPtr()->db_id);
//	printf("%s ",stateNames[myState]);
//	if (myEvent.phase>=0 && myEvent.phase<=ARENA_CANCELLED) {
//		printf("%s ",phaseNames[myEvent.phase]);
//		printf("eventid:%d uniqueid:%d ",myEvent.eventid,myEvent.uniqueid);
//	}

//	printf("\n");
	if (playerPtr() && playerPtr()->supergroup)
		printf("supergroup %s %d\n",playerPtr()->supergroup->name,playerPtr()->supergroup_id);

	if (g_testMode&TEST_ARENA_CREATOR && myState<ARENA_FIGHTER_LASTSTATE)
		SWITCH_STATE(EVENT_CREATOR_MAPMOVE);

	if (g_testMode&TEST_ARENA_CREATOR) {
		switch (myState) {
			case EVENT_CREATOR_MAPMOVE:
				myTimer=50;
				commAddInput("mapmove 29");
				SWITCH_STATE(myState+1);
				locationSet=0;

			xcase EVENT_CREATOR_WAIT_MAP:
				if (--myTimer==0) {
					setloc();
					quitSupergroup();
					if (g_testMode & TEST_ARENA_SUPERGROUP)
						createArenaSupergroup();
					myTimer=50;
					SWITCH_STATE(myState+1);
				}
				memset(&myEvent,-1,sizeof(ArenaEvent));

			xcase EVENT_CREATOR_CREATE:
				if ((g_testMode & TEST_ARENA_SUPERGROUP) && (myTimer>0)) {
					myTimer--;
					inviteNearbyFolks(inviteRate);
				} else {
					createNewEvent();
					myTimer=250;
					SWITCH_STATE(myState+1);
				}


			xcase EVENT_CREATOR_WAIT_PARTICIPANTS:
				if (--myTimer==0) {
					if (g_testMode & ARENA_SUPERGROUP) {
						SWITCH_STATE(EVENT_CREATOR_READY);
					} else {
						printf("Creator %d is leaving event: not enough players\n",playerPtr()->db_id);
						arenaSendPlayerDrop(&myEvent,playerPtr()->db_id);
						SWITCH_STATE(EVENT_CREATOR_CREATE);
					}
				}

			xcase EVENT_CREATOR_READY:
			{
				if (!g_event) {
					printf("Error, no event defined\n");
					return;
				}
				g_event->leader_ready = 1;
				arenaSendCreatorUpdate(g_event);
				SWITCH_STATE(EVENT_CREATOR_WAIT_ARENA);
				myTimer=1000;
			}

			xcase EVENT_CREATOR_WAIT_ARENA:
				if (--myTimer==0 && myEvent.phase<=ARENA_NEGOTIATING) {
					arenaSendPlayerDrop(&myEvent,playerPtr()->db_id);
					SWITCH_STATE(EVENT_CREATOR_CREATE);
				}
				if (myEvent.phase>ARENA_NEGOTIATING || (g_arenastring && !stricmp(g_arenastring, "ArenaFight")))
					SWITCH_STATE(myState+1);

			xcase EVENT_CREATOR_WAIT_ENDROUND:
				SWITCH_STATE(myState+1);
				startFighting=0;
				gotToArenaMap=0;

			xcase EVENT_CREATOR_DESTROY:
				arenaFighterDestroy();
		}
	}

	switch (myState) {
		case ARENA_FIGHTER_MAPMOVE:
			myTimer=50;
			commAddInput("mapmove 29");
			SWITCH_STATE(myState+1);
			locationSet=0;

		xcase ARENA_FIGHTER_WAIT_MAP:
			if (--myTimer==0) {
				setloc();
				quitSupergroup();
				if (g_testMode & TEST_ARENA_SUPERGROUP)
					createArenaSupergroup();
				myTimer=50;
				SWITCH_STATE(myState+1);
			}
//			memset(&myEvent,-1,sizeof(ArenaEvent));

		xcase ARENA_FIGHTER_CHECK_MY_EVENTS:
			if(!--myTimer)
			{
//			if((g_testMode & TEST_ARENA_SCHEDULED))
				requestArenaKiosk(ARENA_FILTER_MY_EVENTS,ARENA_TAB_ALL);
				SWITCH_STATE(ARENA_FIGHTER_WAIT_KIOSK);
				myTimer = 100;
//			else
//				myState = ARENA_FIGHTER_KIOSK;
			}

		xcase ARENA_FIGHTER_KIOSK:
			if ((g_testMode & TEST_ARENA_SUPERGROUP) && (myTimer>0)) {
				inviteNearbyFolks(inviteRate);
				myTimer--;
				//invite folks
			} else {
				requestArenaKiosk(ARENA_FILTER_ELIGABLE,ARENA_TAB_ALL);
				myTimer=5;
			}
	//		SWITCH_STATE(myState+1);

		xcase ARENA_FIGHTER_WAIT_KIOSK:
			if (--myTimer==0) {
				SWITCH_STATE(myState-1);
//				memset(&myEvent,-1,sizeof(ArenaEvent));
			}

		xcase ARENA_FIGHTER_JOIN:
			if (--myTimer==0)
				SWITCH_STATE(ARENA_FIGHTER_KIOSK);

		xcase ARENA_FIGHTER_WAIT_PARTICIPANTS:
			if (--myTimer==0) {
				printf("Creator %d is leaving event: not enough players\n",playerPtr()->db_id);
				arenaSendPlayerDrop(&myEvent,playerPtr()->db_id);
				memset(&myEvent, 0, sizeof(myEvent));
				SWITCH_STATE(ARENA_FIGHTER_KIOSK);
			}
			if (myEvent.phase>ARENA_NEGOTIATING)
				SWITCH_STATE(ARENA_FIGHTER_WAIT_ENDROUND);

		xcase ARENA_FIGHTER_READY:
			{
				if (!g_event)
					return;
				updateWithReady();
				SWITCH_STATE(myState+1);
				myTimer=500;
			}

		xcase ARENA_FIGHTER_WAIT_ARENA:
			if (--myTimer==0 && myEvent.phase<=ARENA_NEGOTIATING) {
				arenaSendPlayerDrop(&myEvent,playerPtr()->db_id);
				memset(&myEvent, 0, sizeof(myEvent));
				SWITCH_STATE(ARENA_FIGHTER_KIOSK);
			}
			if (myEvent.phase>ARENA_NEGOTIATING || (g_arenastring && !stricmp(g_arenastring, "ArenaFight")))
				SWITCH_STATE(myState+1);

		xcase ARENA_FIGHTER_WAIT_ENDROUND:
			SWITCH_STATE(myState+1);
			startFighting=0;
			gotToArenaMap=0;

		xcase ARENA_FIGHTER_DESTROY:
			arenaFighterDestroy();
	}
	playerPtr();
}
