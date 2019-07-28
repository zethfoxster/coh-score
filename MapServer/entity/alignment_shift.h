#ifndef ALIGNMENT_SHIFT_H__
#define ALIGNMENT_SHIFT_H__

#include "gametypes.h"
#include "entity.h"
#include "RewardToken.h"

#define ALIGNMENT_TIPS_MAX 3

U32 alignmentshift_secondsUntilCanEarnAlignmentPoint(Entity *e);
void alignmentshift_determineSecondsUntilCanEarnAlignmentPoints(Entity *e, int seconds[], int alignments[]);
void alignmentshift_resetPoints(Entity *e);
void alignmentshift_resetTimers(Entity *e);
void alignmentshift_resetSingleTimer(Entity *e, int timerIndex);
RewardToken *alignmentshift_getTip(Entity *e, int contactHandle);
void alignmentshift_awardTip(Entity *e);
void tipContacts_awardDesignerTip(Entity *e, int type);
void alignmentshift_revokeTip(Entity *e, int contactHandle);
void alignmentshift_revokeAllBadTips(Entity *e);
void alignmentshift_revokeAllTips(Entity *e, int revokeEverythingReally);
void alignmentshift_resetAllTips(Entity *e);
void alignmentshift_UpdateAlignment(Entity *e);
void alignmentshift_updatePlayerTypeByLocation(Entity *e);
void alignmentshift_printRogueStats(Entity *e, char **buf);
int alignmentshift_countPointsEarnedRecently(Entity *e);

#endif