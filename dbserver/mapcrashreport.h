#ifndef _MAPCRASHREPORT_H
#define _MAPCRASHREPORT_H

#define CRASHED_MAP_BASE_ID 90000 // If we ever have more than this many maps at once, we need to increase this number!
typedef struct MapCon MapCon;
void startMapCrashReportThread(void);
void delinkCrashedMaps(void);
void addCrashedMap(MapCon *map_con, char * message, char *status, int ipaddr);
void handleCrashedMapReport(int id, int process_id, int cookie, char *message, int ipaddr);

#endif
