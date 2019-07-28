/*\
 *
 *	uiSGRaidList.h/c - Copyright 2005 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	Shows the list of active supergroup raids for the player
 *
 */

#ifndef UISGRAIDLIST_H
#define UISGRAIDLIST_H

int sgRaidListWindow(void);
void sgRaidListToggle(void);
void sgRaidListRefresh(void);

void sgRaidTimeToggle(void);
int sgRaidTimeWindow(void);

void sgRaidStartTimeToggle(void);
int sgRaidStartTimeWindow(void);

void sgRaidSizePrompt(void);
int sgRaidSizeWindow(void);

#endif // UISGRAIDLIST_H