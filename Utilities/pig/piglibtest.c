#include "file.h"
#include "fileutil.h"
#include "piglib.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "mathutil.h"
#include <ctype.h>
#include <string.h>
#include "rand.h"

#include <conio.h>
#include "FolderCache.h"
#include "utils.h"
#include <process.h>
#include "hoglib.h"
#include "crypt.h"

static int promptRanged(char *str, int max) { // Assumes min of 1, max can't be more than 9
	int ret=-1;
	do {
		printf("%s: [1-%d] ", str, max);
		ret = getch() - '1';
		printf("\n");
	} while (ret<0 || ret>=max);
	return ret;
}

char *files[] = {
// 	"/texture_library/ENEMIES/Tsoo/Chest_Skin_Tsoo_Vest.texture",
// 	"/texture_library/WORLD/nature/foliage/bushes/cat_tails.texture", 
// 	"/player_library/zombie.seq", 
// 	"/player_library/fem_head.anm",
	"fonts/Tahoma.ttf",
	"texture_library/system/random_test_01.wtex",
	"bin/kb.bin",
};

void readerfunc(void *junk) {
	intptr_t threadnum = (intptr_t)junk;
	int j=0;
	FILE *f;
	int len;
	char *data;
	//while (true) {
	while(true) {
		int i = rand()*ARRAY_SIZE(files)/RAND_MAX;
		int openstyle = rand()*2/RAND_MAX;
		//printf("%d: (%d) %s\n", threadnum, openstyle, files[i]);
		switch(openstyle) {
		case 0:
			f = fileOpen(files[i], "r");
			fileClose(f);
			break;
		case 1:
			data = fileAlloc(files[i], &len);
			fileFree(data);
			break;
		}
	}
}

extern int is_pigged_path(const char *path);

// Test to find bug when two threads are loading files at the same time
// this never actually found anything, because the bug only crops
// up when we're waiting on the filesystem at an inopportune time
void testThreadedPigLoading() {
	int count[2]={0,0};
	int i;
	char fn[MAX_PATH];
	// init
	for (i=0; i<ARRAY_SIZE(files); i++) {
		fileLocateRead(files[i], fn);
		printf("%s:", files[i]);
		if (is_pigged_path(fn)) {
			printf("\tPIGGED\n");
			count[0]++;
		} else {
			printf("\tNOT pigged\n");
			count[1]++;
		}
	}
	if (count[0]<2) {
		printf("Error: expected at least 2 pigged files in the list.  Repigg some up\n");
		gets(fn);
		return;
	} else if (count[1]<2) {
		printf("Error: expected at least 2 files not pigged in the list.  Touch some\n");
		gets(fn);
		return;
	}
	_beginthread(readerfunc, 0, (void*)1);
	readerfunc(0);

}

void testFileDeleted(void *junk, const char* path)
{
	printf("File deleted: %s\n", path);
}

void testFileNewUpdated(void *junk, const char* path, U32 filesize, U32 timestamp)
{
	printf("File new updated: %s  size:%d  timestamp:%d\n", path, filesize, timestamp);
}

void multiProcessHogLockTest(void)
{
	while (true) {
		int i = randomIntRange(0, ARRAY_SIZE(files)-1);
		if (fileExists(files[i])) {
			if (strStartsWith(files[i], "bin")) {
				fileUpdateHoggAfterWrite(files[i], NULL, 0);
			} else {
				fileFree(fileAlloc(files[i], NULL));
			}
			printf(".");
		}
	}
}

void multiProcessHogModTest(void)
{
	HogFile *hog_file;
	int ret;
	int i=0;
	hog_file = hogFileRead("./pigtest.hogg", NULL, PIGERR_ASSERT, NULL, HOG_DEFAULT);
	assert(hog_file);
	hogFileSetCallbacks(hog_file, NULL, testFileDeleted, testFileNewUpdated, testFileNewUpdated);
	while (true) {
		int choice=0;
		printf("Operation:\n");
		printf("  1. Add\n");
		printf("  2. Update\n");
		printf("  3. Delete\n");
		printf("  4. Query\n");
		printf("  5. Read\n");
		printf("  6. Flush\n");
		choice = promptRanged("Do what", 6);
		if (choice==0) {
			if (hogFileFind(hog_file, "test.txt")==HOG_INVALID_INDEX) {
				NewPigEntry entry={0};
				entry.data = malloc(16);
				sprintf_s(entry.data, 16, "Mod:%d", ++i);
				printf("Adding test.txt with data: \"%s\"\n", entry.data);
				entry.size = 16;
				entry.fname = "test.txt";
				entry.timestamp = i;
				ret = hogFileModifyUpdateNamed2(hog_file, &entry);
				assert(ret==0 || ret==-1);
			} else {
				printf("Already there!\n");
			}
		}
		if (choice==1) {
			HogFileIndex index;
			if ((index = hogFileFind(hog_file, "test.txt"))==HOG_INVALID_INDEX) {
				printf("Not found!\n");
			} else {
				NewPigEntry entry={0};
				entry.data = malloc(16);
				sprintf_s(entry.data, 16, "Mod:%d", ++i);
				printf("Updating test.txt with data: \"%s\"\n", entry.data);
				entry.size = 16;
				entry.fname = "test.txt";
				entry.timestamp = i;
				ret = hogFileModifyUpdate(hog_file, index, &entry);
				assert(ret==0 || ret==-1);
			}
		}
		if (choice==2) {
			ret = hogFileModifyDelete(hog_file, hogFileFind(hog_file, "test.txt"));
			assert(ret==0 || ret==-1);
		}
		if (choice==3) {
			HogFileIndex index = hogFileFind(hog_file, "test.txt");
			if (index == HOG_INVALID_INDEX) {
				printf("Not found.\n");
			} else {
				printf("Found, size : %d timestamp : %d\n", hogFileGetFileSize(hog_file, index), hogFileGetFileTimestamp(hog_file, index));
			}
		}
		if (choice==4) {
			HogFileIndex index = hogFileFind(hog_file, "test.txt");
			if (index == HOG_INVALID_INDEX) {
				printf("Not found.\n");
			} else {
				U32 count=0;
				U8 *data = hogFileExtract(hog_file, index, &count, NULL);
				printf("Data: \"%s\" size: %d\n", data, count);
			}
		}
		if (choice==5) {
			ret = hogFileModifyFlush(hog_file);
			assert(ret==0);
		}
	}
}


int main(int argc, char **argv) {
	char buf[MAX_PATH]={0};
	unsigned char *mem;
	int count;
	DO_AUTO_RUNS

	FolderCacheSetMode(FOLDER_CACHE_MODE_DEVELOPMENT_DYNAMIC);
	cryptMD5Init();

	//testThreadedPigLoading();

	fileLoadGameDataDirAndPiggs();

	printf("Do you want to to test multi-process locking of multiple hoggs? [y/N]");
	if (consoleYesNo()) {
		multiProcessHogLockTest();
		return;
	}

	printf("Do you want to to test multi-process hog modification? [y/N]");
	if (consoleYesNo()) {
		multiProcessHogModTest();
		return;
	}

	while(true)
	{
		char locatebuf[MAX_PATH];
		static bool first=true;
		FolderNode *node;
		if (first) {
			strcpy(buf, "test/test1.txt");
			first = false;
		} else {
			char oldbuf[MAX_PATH];
			strcpy(oldbuf, buf);
			printf("Enter a filename: ");
			gets(buf);
			if (buf[0]==0)
				strcpy(buf, oldbuf);
		}
		if (buf[0]==0 || stricmp(buf, "exit")==0) break;
		if (fileLocateRead(buf, locatebuf)) {
			printf("Located: %s\n", locatebuf);
		} else {
			printf("Not found by fileLocateRead()\n");
		}
		node = FolderCacheQuery(folder_cache, buf);
		if (!node) {
			printf("File not found by FolderCacheQuery()!\n");
		} else {
			printf("FolderNode says: %s: %d bytes timestamp: %s",
				(node->virtual_location>=0)?"FS":"Pigg",
				node->size, _ctime32(&node->timestamp));
		}
		{
			mem = fileAlloc(buf, &count);
			if (mem==NULL) {
				printf("Error extracting file!\n");
			} else {
				if (!mem[0]) {
					printf("0-length file\n");
				} else {
					int i;
					printf("First %d bytes of file:\n\t", MIN(64, count));
					for (i=0; i<64 && i<count; i++) {
						if (isprint(mem[i]) || mem[i]=='\t') {
							printf("%c", mem[i]);
						} else if (mem[i] && strchr("\r\n", mem[i])) {
							printf("%c\t", mem[i]);
						} else {
							printf("%02Xh ", mem[i]);
						}
					}
					printf("\n");
				}

				// Test functions:
				if (count>16) {
					FILE *f = fileOpen(buf, "rb");
					if (!f) {
						printf("Error opening file!\n");
					} else {
						U8 data[32];
						U8 data2[16];
						assert(ftell(f)==0);
						assert(15==fread(data, 1, 15, f));
						assert(ftell(f)==15);
						assert(data[0]==mem[0] && data[1]==mem[1] && data[2]==mem[2] && data[3]==mem[3] && data[13] == mem[13]); // Check for reading form cached header == reading the whole file
						assert(0==fseek(f, -4, SEEK_CUR));
						assert(ftell(f)==11);
						assert(3==fread(data2, 1, 3, f));
						assert(data2[0]==data[11] && data2[1]==data[12] && data2[2]==data[13]);
						assert(fgetc(f) == data[14]);
						assert(0==fseek(f, 0, SEEK_SET));
						assert(ftell(f)==0);
						assert(0==fseek(f, -2, SEEK_END));
						assert(ftell(f)==fileGetSize(f)-2);
						fclose(f);
					}
				}
				fileFree(mem);
				// Test fgets
				{
					FILE *f = fileOpen(buf, "r");
					if (!f) {
						printf("Error opening file!\n");
					} else {
						char data[128];
						fgets(data, 125, f);
						printf("Read: '%s'\n", data);
						fclose(f);
					}
				}
			}
		}
		// General file function tests
		printf("fileExists: %d\n", fileExists(buf));
		{
			char buf2[MAX_PATH];
			printf("fileLocateRead: %s\n", fileLocateRead(buf, buf2));
			printf("fileLocateWrite: %s\n", fileLocateWrite(buf, buf2));
		}
		printf("\n");


	}
	return 0;
}
