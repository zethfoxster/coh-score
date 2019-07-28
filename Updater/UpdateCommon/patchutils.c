#include <stdio.h>
#include <string.h>
#include "stdtypes.h"
#include "timing.h"
#include "patchutils.h"
#if PATCHCLIENT
#include "../CohUpdater/patchui.h"
#endif
#include "utils.h"

THREADSAFE_STATIC DataXferInfo xfer_info;

#define LOGID "CohUpdater"


static void calcTextStats(DataXferInfo *info)
{
	F32		elapsed = timerElapsed(info->timer) + 0.00001;
	S64		bytespersec = (info->curr_bytes - info->base_bytes) / elapsed;
	S64		bytesleft = info->total_bytes - info->curr_bytes;
	U32		timeleft = (U32)(bytesleft / (bytespersec+1));
	char	buf[200],buf2[200];

	sprintf(info->bytespersec_str,"%s%s",printUnit(buf,bytespersec),"PerSec");
	sprintf(info->bytesleft_str,"%s/%s",printUnit(buf,info->curr_bytes),printUnit(buf2,info->total_bytes));
	sprintf(info->timeleft_str,"%s %s",printTimeUnit(buf,timeleft),"TimeRemaining");
}

void printTextStats(DataXferInfo *info)
{
	printf("   %s: %s %s %s %s      \r",info->mode,info->thread_name,info->bytespersec_str,info->bytesleft_str,info->timeleft_str);
	fflush(stdout);
}

void printNoGuiStats(DataXferInfo *info)
{
	printf("   %s: %s %s %s %s\n",info->mode,info->thread_name,info->bytespersec_str,info->bytesleft_str,info->timeleft_str);
	fflush(stdout);
}

THREADSAFE_STATIC int xfer_stats_timer;
void updateXferStats(DataXferInfo *info)
{
	float updateRate = info->no_gui_mode ? 10.0f : 0.25f;

	if (!xfer_stats_timer)
		xfer_stats_timer = timerAlloc();
	if (timerElapsed(xfer_stats_timer) < updateRate && !info->force_print)
		return;
	timerStart(xfer_stats_timer);
	info->force_print = 0;

	calcTextStats(info);
	if(info->no_gui_mode)
		printNoGuiStats(info);
	else
		#if PATCHCLIENT
		updateUiStats(info);
		#else
		printTextStats(info);
		#endif
}

void xferStatsInit(char *mode,char *thread_name,S64 base,S64 total)
{
#if PATCHCLIENT
	filelog_printf( LOGID, __FUNCTION__ ": mode=%s,base=%i,total=%i",mode,base,total);
#endif

	xfer_info.stats_init = 1;
	xfer_info.no_gui_mode = thread_name ? 1 : (isGuiDisabled() ? 1 : 0);
	xfer_info.force_print = 1;
	if (!xfer_info.timer)
		xfer_info.timer = timerAlloc();
	timerStart(xfer_info.timer);
	xfer_info.base_bytes = xfer_info.curr_bytes = base;
	xfer_info.thread_name[0] = 0;
#if PATCHCLIENT
	//xlatePrint(SAFESTR(xfer_info.mode),mode);
	strcpy_s(SAFESTR(xfer_info.mode),xlateQuick(mode));
	if(thread_name) strcpy_s(SAFESTR(xfer_info.thread_name),xlateQuick(thread_name));
#else
	sprintf(xfer_info.mode,mode);
	if(thread_name) sprintf(xfer_info.thread_name,thread_name);
#endif
	xfer_info.total_bytes = total;
	updateXferStats(&xfer_info);
}

void xferStatsSetMode(char *mode)
{
	xfer_info.force_print = 1;
	strcpy(xfer_info.mode,mode);
	updateXferStats(&xfer_info);
}

void xferStatsUpdate(S64 curr)
{
	xfer_info.curr_bytes += curr;
	updateXferStats(&xfer_info);
}

void xferStatsFinish()
{
	char	oldmode[256];

	if (!xfer_info.stats_init)
		return;

	strncpy(oldmode, xfer_info.mode, 256);
	strcpy(xfer_info.mode, "");
	xfer_info.stats_init = 0;
	xfer_info.force_print = 1;
	updateXferStats(&xfer_info);
	xfer_info.force_print = 0;
	printf("%-78s\r","");
	fflush(stdout);
}

F32 xferStatsElapsed()
{
	return timerElapsed(xfer_info.timer);
}

