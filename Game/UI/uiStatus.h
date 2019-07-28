/***************************************************************************
 *     Copyright (c) 2004-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef UISTATUS_H__
#define UISTATUS_H__

#include "uiInclude.h"

enum
{
	HEALTHBAR_HEALTH,
	HEALTHBAR_ENDURANCE,
	HEALTHBAR_INSIGHT,
	HEALTHBAR_RAGE,
	HEALTHBAR_EXPERIENCE,
	HEALTHBAR_EXPERIENCE_DEBT,
	HEALTHBAR_TARGET_HEALTH,
	HEALTHBAR_TARGET_ENDURANCE,
	HEALTHBAR_TARGET_EXPERIENCE,
	HEALTHBAR_LOADING,
	HEALTHBAR_ABSORB,
};

int  status_addNotification( AtlasTex *icon, void (*code)(), char *tooltip );
void status_removeNotification( AtlasTex * icon );

int statusGetEmblemWidth();

#define MAX_HEARTBEAT_HEALTH_PERCENTAGE .2f
F32 getHeartBeatIntensity(float health_percentage, bool reset);

void drawHealthBar( float x, float y, float z, float width, float curr, float total, float scale, int frameColor, int barColor, int type );
int getHealthColor(bool inPhase, float percentage);
int getAbsorbColor(float percentage);
int statusWindow();

// This needs a better home.
char* calcTimeRemainingString(int seconds, bool pad_minutes);

#endif /* #ifndef UISTATUS_H__ */

/* End of File */

