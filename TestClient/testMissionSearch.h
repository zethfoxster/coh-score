#ifndef TEST_MISSION_SERVER
#define TEST_MISSION_SERVER

//functions for testing the mission server.

void testMissionSearchTestPublish();
void testMissionSearch_testSearch();
void testMissionSearch_testVoteRandom(int nVotes);
void testMissionSearch_testPlayRandom();
void testMissionSearch_receiveArc(int arcId, char *headerStr, int rating);
void testMissionSearchDelete();
void testMissionSearch_testPlayRandom();
void testMissionSearch_registerHeader(char *header);
void testMissionSearch_registerPages(int page, int pages);

//for stress testing--no validation
void testMissionSearch_forceSearch();
void testMissionSearch_forcePubUnpubRepub();
void testMissionSearch_forceVote();
void testMissionSearch_forcePlay();
void testMissionSearch_registerData(char *arc);



#endif