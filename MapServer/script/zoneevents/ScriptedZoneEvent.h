/***************************************************************************
*     Copyright (c) 2009-2009, NCSoft
*     All Rights Reserved
*     Confidential Property of NCSoft
***************************************************************************/

#ifndef SCRIPTEDZONEEVENT_H
#define SCRIPTEDZONEEVENT_H

//////////////////////////////////////////////////////////////////////////////////////////////////
// Functions
int ScriptedZoneEventStartFile(char *file, int isMission);
void ScriptedZoneEventStopEvent(char *event);
void ScriptedZoneEventGotoStage(char *event, char *stage);
void ScriptedZoneEventKillDebug(int enable);
void ScriptedZoneEventDisableEvent(char *eventName);
void ScriptGotoMarker(Entity *e, char *marker);

void ScriptedZoneEvent_SetScriptValue(char **eval);
void ScriptedZoneEventEval_StoreAttribInfo(float fMagnitude, float fDuration);
void ScriptedZoneEvent_SetCurrentBGMusic(const char *musicName, float volumeLevel);


typedef enum SZE_LogType
{
	SZE_Log_None	= 0,
	SZE_Log_Reward	= 1,
}SZE_LogType;

void ScriptedZoneEventSendDebugPrintf(int logFlags, char *format, ...);

#endif //SCRIPTEDZONEEVENT_H