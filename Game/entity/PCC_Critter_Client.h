#ifndef PCC_CRITTER_CLIENT_H
#define PCC_CRITTER_CLIENT_H

typedef struct PCC_Critter PCC_Critter;
typedef struct Entity Entity;

typedef enum PCC_EDITING_MODE_TYPE{
	PCC_EDITING_MODE_NONE = 0,
	PCC_EDITING_MODE_CREATION = 1,
	PCC_EDITING_MODE_EDIT = 2,
}PCC_EDITING_MODE_TYPE;
typedef enum PCC_SETUP_FLAGS {
	PCC_SETUP_FLAGS_ISCONTACT = 1,
	PCC_SETUP_FLAGS_SEED_MINION = 1 << 1,
	PCC_SETUP_FLAGS_SEED_LT = 1 << 2,
};

void initPCCFolderCache();
void loadPCCFromFiles();
Entity *createPCC_Entity();
void destroyPCC_Entity(Entity *e);
char *getPCCRankText(int rank);
char *getPCCRankSelectionTitleText(int rank);
char *getPCCDifficultySelectionTitleText(int difficulty);
char * getCustomCritterDir(void);
PCC_Critter * addCustomCritter(PCC_Critter *pCritter, Entity *entityPtr);
void removeStoryArcCritters();
void removeCustomCritter(char *referenceFile, int deleteSourceFile);
PCC_Critter *getCustomCritterByReferenceFile( char * referenceFile);
void PCC_generatePCCHTMLSelectionText(char **str, PCC_Critter *pcc, int reasonForInvalid, int ContactIsInvalid, char *defaultColor);
void PCC_reinsertMissing(PCC_Critter *pcc);
#endif

