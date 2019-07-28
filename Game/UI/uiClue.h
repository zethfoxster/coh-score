#ifndef UICLUE_H
#define UICLUE_H

typedef struct SouvenirClue SouvenirClue;

int clueWindow();
void souvenirCluePrepareUpdate();
void souvinerClueAdd( SouvenirClue* clue );
void updateSouvenirClue( int uid, char * desc );
SouvenirClue * getClueByID( int uid );
void scPrintAll(void);

void addPlayerCreatedSouvenir(int uid, char * title, char * description );

#endif