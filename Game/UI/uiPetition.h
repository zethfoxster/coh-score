/***************************************************************************
 *     Copyright (c) 2004-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef UIPETITION_H__
#define UIPETITION_H__

typedef enum PetitionWindowMode
{
	PETITION_NORMAL,		// opened from Menu
	PETITION_COMMAND_LINE,	// opened from console command
	PETITION_BUG,			// /bug console command
} PetitionWindowMode;

#define TEXTAREA_SUCCESS     0
#define TEXTAREA_LOSE_FOCUS  1
#define TEXTAREA_WANTS_FOCUS 2

void setPetitionMode(PetitionWindowMode mode);
void setPetitionSummary(const char * summary);

int petitionWindow(void);

typedef struct UIEdit UIEdit;

int textarea_Edit(UIEdit *pedit, float x, float y, float z, float wd, float ht, float sc);
int RadioButtons(float x, float y, float z, float sc, int iCurrentChoice, char *pchTitle, int iCntChoices, ...);

#endif /* #ifndef UIPETITION_H__ */

/* End of File */

