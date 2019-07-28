/*\
 *
 *	iopdata.h/c - Copyright 2004, 2005 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	Bolted on to the raid server, this manages the items of power game
 *
 */

#ifndef IOPDATA_H
#define IOPDATA_H

typedef struct ItemOfPowerInfo
{
	char * name;				//Name is unique, and used to create tokenID for reward token
	bool	unique;
} ItemOfPowerInfo;

typedef struct ItemOfPowerInfoList
{
	ItemOfPowerInfo ** infos;
	int daysGameLasts;
	int daysCathedralIsOpen;
	int chanceOfGettingUnique;
	bool allowScheduledRaids;

} ItemOfPowerInfoList;

extern ItemOfPowerInfoList g_itemOfPowerInfoList;
extern StashTable	itemOfPowerInfo_ht;

void LoadItemOfPowerInfos(void);
ItemOfPowerInfo * getItemOfPowerInfoFromList( char * name );

#endif // IOPDATA_H