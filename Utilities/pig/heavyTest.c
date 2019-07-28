#include "heavyTest.h"
#include "hoglib.h"
#include "piglib.h"
#include "timing.h"
#include "utils.h"
#include "file.h"
#include "StringCache.h"
#include "mathutil.h"

AUTO_RUN_FIRST;
void initStringCache(void)
{
	stringCachePreAlloc(50000);
}

void *memdup(const void *data, size_t size)
{
	void *ret = malloc(size);
	memcpy(ret, data, size);
	return ret;
}

void doHeavyTest(void)
{
#if 1
	printf("doHeavyTest disabled\n");
#else
	// Run two of these in parallel to test this
	char filenames[][MAX_PATH] = {
		"1111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111.txt",
		"222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222.txt",
		"33333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333.txt",
	};
	char *data[10];
	int data_size[ARRAY_SIZE(data)];
	int i;
	int timestamp =1;
	int timer=timerAlloc();
	int fgTimer=timerAlloc();
	int bgTimer=timerAlloc();
	int totalTimer=timerAlloc();
	F32 fgElapsed;
	F32 bgElapsed;
	int count=0;
	int totalCount=0;
	HogFile *hog_file;
	int j;
	enum {
		ModeRepeatUpdate,
		ModeIncrementalAdd,
	} mode = ModeRepeatUpdate;
	F32 maxSize = 1024*1024;

	if (mode == ModeRepeatUpdate) {
		maxSize = 1024;
	} else {
		maxSize = 1024;
	}
//#define DO_HEADERS
#ifdef DO_HEADERS
	for (i=0; i<ARRAY_SIZE(filenames); i++) {
		changeFileExt(filenames[i], ".wtex", filenames[i]);
	}
#endif

	hogSetGlobalOpenMode(HogSafeAgainstAppCrash);
	hogSetMaxBufferSize(128*1024*1024);

	for (j=0; j<1; j++) {

	hog_file = hogFileRead("./testing.hogg", NULL, PIGERR_ASSERT, NULL, HOG_DEFAULT);
	hogFileSetSingleAppMode(hog_file, true);
	for (i=0; i<ARRAY_SIZE(data); i++) {
#ifdef DO_HEADERS
		char fn[MAX_PATH];
		int newsize;
		sprintf(fn, "./headertest%d.wtex", i);
		data[i] = fileAlloc(fn, &data_size[i]);
		assert(data[i]);
		newsize = rand()*maxSize/(F32)(RAND_MAX+1);
		MIN1(data_size[i], newsize);
#else
		data_size[i] = rand()*maxSize/(F32)(RAND_MAX+1);
		data[i] = malloc(data_size[i]);
		memset(data[i], 0x77, data_size[i]);
#endif
	}
	while (!_kbhit()) {
		F32 elapsed;
		int index = rand()*ARRAY_SIZE(data)/(RAND_MAX+1);
		if (mode == ModeRepeatUpdate) {
			hogFileModifyUpdateNamed(hog_file, filenames[rand()*ARRAY_SIZE(filenames)/(RAND_MAX+1)], 
				memdup(data[index], data_size[index]), data_size[index], ++timestamp, NULL);
		} else {
			char name[MAX_PATH];
			sprintf(name, "file_%d", totalCount);
			hogFileModifyUpdateNamed(hog_file, name, 
				memdup(data[index], data_size[index]), data_size[index], ++timestamp, NULL);
		}
		//if ((rand()%7)==1) // To allow multi-process to have a chance
		//	hogFileModifyFlush(hog_file);
		count++;
		totalCount++;
		if ((elapsed = timerElapsed(timer)) > 1) {
			timerStart(timer);
			printf("%1.1f queued updates/s (%ld queued bytes)\n", count / elapsed, hogFileGetQueuedModSize(hog_file));
			count = 0;
		}
	};
	fgElapsed = timerElapsed(fgTimer);
	printf("Closing...");
	hogFileDestroy(hog_file);
	printf(" done.\n");
	bgElapsed = timerElapsed(bgTimer);
	printf("FG: %1.1f u/s  BG: %1.1f u/s\n", totalCount / fgElapsed, totalCount / bgElapsed);
	}
	printf("Total time: %1.1fs\n", timerElapsed(totalTimer));
	{
		int k = _getch();
	}
#endif
}
