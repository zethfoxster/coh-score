#ifndef UIMISSIONREVIEW_H
#define UIMISSIONREVIEW_H
#include "uiScrollBar.h"


int missionReviewWindow(void);

void missionReview_Set( int id, int test, char * pchName,  char * pchAuthor, char * pchDesc, int rating, int euro );
void missionReview_Complete( int id );
int onArchitectTestMap();
void missionReviewLoadKeywordList();
char * missionReviewGetKeywordKeyFromNumber( int i );
F32 missionReviewDrawKeywords(F32 x, F32 y, F32 z, F32 sc, F32 wd, F32 ht, int wdw, U32 *bitfield, int limitSelection, int backcolor);
void missionReviewResetKeywordsTooltips();
#endif