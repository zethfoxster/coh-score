#ifndef _PATCHUTILS_H
#define _PATCHUTILS_H

typedef struct
{
	char	mode[1000];
	char	thread_name[1000];
	S64		base_bytes;
	S64		total_bytes;
	S64		curr_bytes;
	int		timer;
	U32		force_print : 1;
	U32		stats_init : 1;
	U32		no_gui_mode : 1;

	char	bytespersec_str[200],bytesleft_str[200],timeleft_str[200];
} DataXferInfo;


void updateUiStats(DataXferInfo *info);
void printTextStats(DataXferInfo *info);
F32 xferStatsElapsed(void);
void xferStatsFinish(void);
void xferStatsUpdate(S64 curr);
void xferStatsInit(char *mode,char *thread_name,S64 base,S64 total);
void xferStatsSetMode(char *mode);

#endif
