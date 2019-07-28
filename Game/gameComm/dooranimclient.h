/*\
 *  dooranimclient.h - client functions to run door animations for player
 *
 *  Copyright 2003 Cryptic Studios, Inc.
 */

#ifndef __DOORANIMCLIENT_H
#define __DOORANIMCLIENT_H

#include "mathutil.h"
#include "dooranimcommon.h"

typedef struct Packet Packet;

extern int dooranim_doorfade; // whether the client will do fading on door entry/exit
extern F32 dooranim_fadePercent; // current fade level

// tell the server we want to enter this door, pos may be an entity or group def pos
void enterDoorClient(const Vec3 pos, char* map_id);
void enterDoorFromSgid( int idSgrp, char* map_id );

// old client door functions - this wait mode determines whether the client is 
// locked with hourglass cursor and no input.  This is a seperate state from 
// DoorAnimXxx when the client is actually animating and/or fading out
void setWaitForDoorMode();
int waitingForDoor();
void doneWaitForDoorMode();

// mission kick timer functions - let the server show a countdown on client while waiting to be
// kicked from a mission map
void MissionKickReceive(Packet *pak);
void MissionKickClear(void);
void CheckMissionKickProgress(int disconnecting); // pass whether player is disconnecting as well
int MissionKickSeconds(void); // when player will be disconnected from mission, -1 if not being disconnected

// animation of running through door
void DoorAnimReceiveStart(Packet *pak);
void DoorAnimReceiveExit(Packet *pak);
void DoorAnimClientCheckMove(void); // async call to process animation
int DoorAnimIgnorePositionUpdates(void); // returns 1 if player should ignore position updates
int DoorAnimFrozenCam(void); // returns 1 if player should currently have camera frozen

// fade-in/out
void DoorAnimCheckFade();
void DoorAnimFadeIn(F32 fadeTime);
void DoorAnimFadeOut(F32 fadeTime);
void DoorAnimSetFade(F32 percent);
int DoorAnimIsFadedOut(void);
void DoorAnimCheckForcedMove(void);

#endif // __DOORANIMCLIENT_H
