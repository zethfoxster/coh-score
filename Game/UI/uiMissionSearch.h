#pragma once

#include "MissionSearch.h"

// uiNet interface
void missionsearch_TabClear(MissionSearchPage category, const char *context, int page, int pages);
void missionsearch_TabAddLine(	MissionSearchPage category, int arcid, int mine, int played,
								int myVote, int ratingi, int total_votes, int keyword_votes, U32 date, const char *header ); 
void missionsearch_TabAddSection(MissionSearchPage category, const char *section);
void missionsearch_SetArcData(int arcid, const char *arcdata);
void missionsearch_ArcDeleted(int arcid);
void missionsearch_IntroChoice(void);

void missionsearch_deleteArc(const char *filename);
void missionsearch_myArcsChanged(const char *relpath, int when);

void missionsearch_MakeMission(void *notused);
void missionsearch_PlayMission(void *notused);
void missionsearch_playPublishedArc(int arcid, int dev_choice);
void missionsearch_SortResults(MissionSearchPage category);

void MissionSearch_requestPublish(int arcid, char * publishArcstr);
void MissionSearch_publishRequestUpdate();
int MissionSearch_publishWait();
void missionsearch_clearAllPages();

int missionsearch_Window(void);


void missionsearch_InvalidWarning(int invalidArcs, int playableErrors);