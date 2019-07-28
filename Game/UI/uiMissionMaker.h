#pragma once

typedef struct PlayerCreatedStoryArc PlayerCreatedStoryArc;
typedef struct MMScrollSet MMScrollSet;

void mmscrollsetViewList( char * indexPath, int toggle_region );

void missionMakerOpenWithArc(PlayerCreatedStoryArc *pArc, char *pchFileName, int arc_id, int owned); // NULL for new
void missionMakerTestStoryArc(PlayerCreatedStoryArc *pArc); // takes ownership of pArc

#define MM_TEST		(1<<0)
#define MM_SAVEAS	(1<<1)
#define MM_EXIT		(1<<2)
void missionMakerSave(int flags);

void missionMakerExitDialog(void *userdata);
void missionMakerDialogDummyFunction(void *unused);
void missionMakerFixErrors(int unused);
void missionMakerRepublish(void);

int showMissionMakerToolTips(void);
int missionMaker_drawDialogOptions( F32 x, F32 y, F32 z, F32 sc, F32 wd, int options );

extern MMScrollSet missionMaker;

int missionMakerWindow(void);
void toggleSaveCompressedCostumeArc(int mode);
void compressArcPCCCostumes(PlayerCreatedStoryArc *pArc);