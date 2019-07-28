#include "stdtypes.h"
#include "piglib.h"

typedef struct NewPigEntry NewPigEntry;
typedef struct StashTableImp *StashTable;
typedef const struct StashTableImp *cStashTable;

typedef struct PigDir {
	char dirname[1024];
	PigFile **pig_files;
	NewPigEntry **new_files;
	char **deleted_files;
	U32 new_data;
	StashTable htNameToIndex;
	StashTable htDeletedNames;
	StashTable htNewNames;
	bool disable_auto_flush; // Does not auto-matically flush after writing X MB
} PigDir;

PigDir *createPigDir(const char *dir);
void destroyPigDir(PigDir *pigdir); // Calls flush
void pigDirFlush(PigDir *pigdir);

char *pigDirFileAlloc(PigDir *pigdir, const char *filename, int *lenp);
const char *pigDirHeaderAlloc(PigDir *pigdir, const char *filename, int *lenp);

void pigDirDeleteFiles(PigDir *pigdir, char **files);
void pigDirWriteFile(PigDir *pigdir, const char *filename, const char *data, int len, U32 timestamp); // Use time(NULL) for timestamp

typedef void (*PigDirScanProcessor)(PigDir *pigdir, const char* filename);
void pigDirScanAllFiles(PigDir *pigdir, PigDirScanProcessor processor);
