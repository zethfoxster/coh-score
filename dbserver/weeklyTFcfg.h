#ifndef _WEEKLYTFCFG_H
#define _WEEKLYTFCFG_H

typedef struct DateTime
{
	char *dateStr;
	char *timeStr;
} DateTime;

typedef struct WeeklyTFCfg
{
	DateTime *dateTime;
	unsigned int epochStartTime;
	char **weeklyTFTokens;
}WeeklyTFCfg;

typedef struct WeeklyTFCfgList
{
	WeeklyTFCfg **weeklyTF_cfg;
}WeeklyTFCfgList;

void WeeklyTF_GenerateTokenString(const WeeklyTFCfg* weeklyTF_cfg, char **buff);
void WeeklyTFCfgLoad(void);
void WeeklyTFCfgCheckReload(void);
void WeeklyTF_AddTFToken(WeeklyTFCfg* weeklyTF_cfg,char *s);
void WeeklyTF_RemoveTFToken(WeeklyTFCfg* weeklyTF_cfg,char *s);
void WeeklyTF_SetEpochTime(WeeklyTFCfg* weeklyTF_cfg,char *date, char *time);
WeeklyTFCfg* WeeklyTFCfg_getCurrentWeek(int reload);
#endif