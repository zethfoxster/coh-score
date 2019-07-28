/*\
 *
 *	scripthookmission.h - Copyright 2004 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	Provides all the functions CoH script files should be referencing -
 *		essentially a list of hooks into game systems.
 *
 *	This file contains all functions related to missions
 *
 */

#ifndef SCRIPTHOOKMISSION_H
#define SCRIPTHOOKMISSION_H

typedef enum TrialStatus TrialStatus;

// Missions
void SetMissionStatus(STRING localizedStatus);
void SetMissionStatusWithCount(NUMBER count, STRING localizedStatus);
void SetMissionComplete(NUMBER success);
void SetMissionFailingTimer(NUMBER time);
void SetMissionNonFailingTimer(NUMBER timeLimit, NUMBER timerType);
void ClearMissionTimer();
void SendTrialStatus(TEAM team);
int MissionIsComplete(void);
int MissionIsSuccessful(void);
ENTITY MissionOwner( void );
int MissionOwnerDBID(void);
int ObjectiveIsComplete(STRING objective);
int ObjectivePartialComplete(STRING objective, int numForCompletion);
int ObjectiveHasFailed(STRING objective);
NUMBER CheckMissionObjectiveExpression(STRING expression);
void CreateMissionObjective(STRING objectiveName, STRING state, STRING singularState);
void SetObjectiveString(STRING state, STRING singularState); // only works on encounter scripts
void SetNamedObjectiveString(STRING objectiveName, STRING state, STRING singularState);
void ScriptKickFromMap(ENTITY player);

STRING GetShortMapName(void);
STRING GetMapName(void);
NUMBER GetMapID(void);
NUMBER GetBaseMapID(void);
NUMBER GetMapAllowsHeroes(void);
NUMBER GetMapAllowsVillains(void);
NUMBER GetMapAllowsPraetorians(void);
int MissionLevel(void);		// overall level of mission, does not consider pacing

void SetTrialStatus(TrialStatus status);
TrialStatus GetTrialStatus();
int IncarnateTrialComplete();


#define OBJECTIVE_SUCCESS 1
#define OBJECTIVE_FAILED 0
void SetMissionObjectiveComplete( int status, const char * ObjectiveName );

int HasSouvenirClue(ENTITY entity, STRING clueName);
int HasClue(ENTITY entity, STRING clueName);

//////////////////////////////////////////////////////////////////
// From mission.c

int MissionNumHeroes(void);	// current # on map or team size
int OnMissionMap(void);


#endif