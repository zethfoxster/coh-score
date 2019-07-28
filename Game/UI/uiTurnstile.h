/*
 *
 *	uiTurnstile.h - Copyright 2010 NC Soft
 *		All Rights Reserved
 *		Confidential property of NC Soft
 *
 *	Turnstile system for matching players for large raid content
 *
 */

#ifndef UITURNSTILE_H
#define UITURNSTILE_H

#include "stdtypes.h"
#include "TurnstileCommon.h"

#define		CLIENT_RESPONSE_DELAY		EVENT_READY_BASE_DELAY		// Seconds to wait for player to respond,  This is slightly shorter than the 
																	// serverside value, so the client times out first

#define		MAX_GROUP					16							// DGNOTE 5/3/2010 - If the assert in uiTurnstile.c tripped that sent you here
																	// just increase this, rebuild and carry on.

// Clientside Mission definition.  Contains a few strategic fields from the serverside version, plus some client specific ones
typedef struct TurnstileMissionClient
{
	char *name;						// Event name
	float avgWait;					// Average wait time before getting into this mission
	int isWeeklyTF;
	int selected;					// Is this selected in the UI?
	int index;						// Index of this mission as sent from the mapserver: used when queueing for the event
	TurnstileCategory category;		// Which tab the event should be displayed under
	int eligible;
} TurnstileMissionClient;

int OnEndgameRaidMap();
int turnstileGameCM_EndgameRaidLeader(void *foo);
int turnstileGameCM_onEndGameRaid(void *foo);
void turnstile_votekickTarget(void *foo);

void turnstileGame_handleEventList(Packet *pak);
void turnstileGame_handleQueueStatus(Packet *pak);
void turnstileGame_handleEventReady(Packet *pak);
void turnstileGame_handleEventReadyAccept(Packet *pak);
void turnstileGame_HandleEventStartStatus(Packet *pak);
void turnstileGame_handleEventStatus(Packet *pak);
void turnstileGame_handleJoinLeader(Packet *pak);
void turnstileGame_handleTurnstileError(Packet *pak);
void turnstileGame_handleTurnstileGeneralStatus(Packet *pak);
void turnstileGame_handleVotekickRequest(Packet *pak);
void turnstileGame_handleForceQueue(Packet *pak);

void turnstileGame_generateRequestEventList();
void turnstileGame_generateQueueForEvents(int *events, int eventCount);
void turnstileGame_generateQueueForEventsByString(char *eventList);
void turnstileGame_generateRemoveFromQueue();
int turnstileGame_generateEventResponse(char *response);

int turnstileWindow();
int turnstileDialogWindow();
void turnstileDialogWindowActivate(char *title, char *text, char *footer, int seconds,
								   char *yesButton, char *noButton,
								   void (*yesFunc)(void *), void *yesData,
								   void (*noFunc)(void *), void *noData,
								   int overRide);

#endif // UITURNSTILE_H
