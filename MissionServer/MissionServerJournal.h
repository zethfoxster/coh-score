#pragma once

#include "MissionServer.h"

typedef struct PersistConfig PersistConfig;
int missionserver_registerJournaledArcPersist(PersistConfig *config); // SORTED!
int missionserver_registerJournaledUserPersist(PersistConfig *config);

void missionserver_journalArc(MissionServerArc *arc); // journals the entire arc
void missionserver_journalArcUnpublish(MissionServerArc *arc);
void missionserver_journalArcInvalidate(MissionServerArc *arc);
void missionserver_journalArcPlayableInvalidate(MissionServerArc *arc);
void missionserver_journalArcHonors(MissionServerArc *arc);
void missionserver_journalArcBanPulled(MissionServerArc *arc);
void missionserver_journalArcVotesRating(MissionServerArc *arc);
void missionserver_journalArcKeywords(MissionServerArc *arc);
void missionserver_journalArcTotalCompletions(MissionServerArc *arc);
void missionserver_journalArcTotalStarts(MissionServerArc *arc);

void missionserver_journalUser(MissionServerUser *user); // journals the entire user
void missionserver_journalUserComplaint(MissionServerUser *user, int arcid);
void missionserver_journalUserBan(MissionServerUser *user);
void missionserver_journalUserArcVote(MissionServerUser *user, int arcid, MissionRating vote);
void missionserver_journalUserArcKeywordVote(MissionServerUser *voter, int arcid, int keywords);
void missionserver_journalUserArcBestVote(MissionServerUser *user, int arcid, MissionRating bestvote);
void missionserver_journalUserInventory(MissionServerUser *user);
void missionserver_journalUserRecentUserVote(MissionServerUser *user, int authid, AuthRegion authregion, U32 timestamp);
void missionserver_journalUserTotalVotesEarned(MissionServerUser *user);
void missionserver_journalUserArcCompleted(MissionServerUser *user, int arcid, U32 timestamp);
void missionserver_journalUserArcCompletedUpdate(MissionServerUser *user, int arcid, U32 timestamp);
void missionserver_journalUserArcStarted(MissionServerUser *user, int arcid, U32 timestamp);
void missionserver_journalUserArcStartedUpdate(MissionServerUser *user, int arcid, U32 timestamp);

void missionserver_journalUserDeleteArcVote(MissionServerUser *user, int arcid);
void missionserver_journalUserDeleteArcBestVote(MissionServerUser *user, int arcid);
void missionserver_journalUserDeleteUserVote(MissionServerUser *user, int authid, AuthRegion authregion);
