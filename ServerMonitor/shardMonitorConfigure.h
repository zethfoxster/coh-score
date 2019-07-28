#include "wininclude.h"
#include "textparser.h"

void shardMonConfigure(HINSTANCE hinst, HWND hwndParent, char *configfile);

void shardMonLoadConfig(char *configfile);

typedef struct ShardMonitorConfigEntry {
	char name[128];
	U32 ip;
} ShardMonitorConfigEntry;

typedef struct ShardMonitorConfig {
	ShardMonitorConfigEntry **shardList;	
} ShardMonitorConfig;

extern TokenizerParseInfo shardMonitorConfigEntryDispInfo[];
extern TokenizerParseInfo shardMonitorConfigInfo[];

extern ShardMonitorConfig shmConfig;
