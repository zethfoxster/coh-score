#ifndef STORYARCCLIENT_H
#define STORYARCCLIENT_H

#include "storyarc/storyarcCommon.h"

//------------------------------------------------------------------
//------------------------------------------------------------------
// StoryClue
//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct Packet Packet;
void ClueReceive(StoryClue* clue, Packet* pak);
void ClueListReceive(Packet* pak);

extern StoryClue** storyClues;
extern StoryClue** keyClues;

// key clues
void KeyClueListReceive(Packet* pak);


//------------------------------------------------------------------
//------------------------------------------------------------------
// SouvenirClue
//------------------------------------------------------------------
//------------------------------------------------------------------

void scReceiveAllHeaders(Packet* pak);
void scReceiveHeader(SouvenirClue* clue, Packet* pak);
void scRequestDetails(SouvenirClue* clue);
void scReceiveDetails(Packet* pak);

#endif