#include "PigDir.h"
#include "memcheck.h"
#include <string.h>
#include "MemoryPool.h"
#include "file.h"
#include "utils.h"
#include "earray.h"
#include "fileutil.h"
#include "SharedMemory.h"
#include "network/crypt.h"
#include "memlog.h"
#include "StashTable.h"
#include "piglib_internal.h"

#define PIG_FILE_SIZE (10*1024*1024)

static PigDir *g_pig_dir;

PigFile *pigDirFindRightPigFile(PigDir *pigdir, const char *relpath)
{
	int ret;
	if (stashFindInt(pigdir->htNameToIndex, relpath, &ret)) {
		return pigdir->pig_files[ret];
	}
	return NULL;
}

void pigDirNextFilename(PigDir *pigdir, char *buf, size_t buf_size)
{
	int i=0;
	bool exists;
	do {
		int j;
		exists = false;
		sprintf_s(buf, buf_size, "/%03d.pigg", i);
		for (j=0; j<eaSize(&pigdir->pig_files) && !exists; j++) {
			if (strEndsWith(pigdir->pig_files[j]->filename, buf))
				exists = true;
		}
		sprintf_s(buf, buf_size, "%s/%03d.pigg", pigdir->dirname, i);
		i++;
	} while (exists);
}

void pigDirReIndex(PigDir *pigdir)
{
	int i;
	unsigned int j;
	if (pigdir->htNameToIndex) {
		stashTableClear(pigdir->htNameToIndex);
	} else {
		pigdir->htNameToIndex = stashTableCreateWithStringKeys(1024, StashDefault);
	}

	for (i=0; i<eaSize(&pigdir->pig_files); i++) {
		PigFile *pf = pigdir->pig_files[i];
		U32 numfiles = PigFileGetNumFiles(pf);
		for (j=0; j<numfiles; j++) {
			const char *filename = PigFileGetFileName(pf, i);
			if (!filename || PigFileIsSpecialFile(pf, i))
				continue;
			stashAddInt(pigdir->htNameToIndex, filename, i, false);
		}
	}
}

void pigDirLoadPig(PigDir *pigdir, const char *filename)
{
	PigFile *pf;
	pf = malloc(sizeof(*pf));
	PigFileCreate(pf);
	if (0!=PigFileRead(pf, filename)) {
		printf("Error reading pig file: %s\n", filename);
	} else {
		eaPush(&pigdir->pig_files, pf);
	}
}

FileScanAction pigDirLoadProcessor(char* dir, struct _finddata32_t* data)
{
	char filename[MAX_PATH];
	sprintf_s(SAFESTR(filename), "%s/%s", dir, data->name);
	if (!strEndsWith(filename, ".pigg"))
		return FSA_EXPLORE_DIRECTORY;
	pigDirLoadPig(g_pig_dir, filename);
	return FSA_EXPLORE_DIRECTORY;
}

MP_DEFINE(PigDir);
PigDir *createPigDir(const char *dir)
{
	PigDir *pigdir;
	MP_CREATE(PigDir, 4);
	pigdir = MP_ALLOC(PigDir);

	sharedMemorySetMode(SMM_DISABLED);

	strcpy(pigdir->dirname, dir);
	forwardSlashes(pigdir->dirname);
	while (strEndsWith(pigdir->dirname, "/")) {
		pigdir->dirname[strlen(pigdir->dirname)-1]=0;
	}

	// Find all piggs
	// Load them and make file index<->pig mapping
	g_pig_dir = pigdir;
	fileScanAllDataDirs(pigdir->dirname, pigDirLoadProcessor);
	g_pig_dir = NULL;

	pigDirReIndex(pigdir);
	pigdir->htDeletedNames = stashTableCreateWithStringKeys(1024, StashDefault);
	pigdir->htNewNames = stashTableCreateWithStringKeys(1024, StashDefault);

	return pigdir;
}

void destroyPigDir(PigDir *pigdir) // Calls flush
{
	int i;
	pigDirFlush(pigdir);

	for (i=eaSize(&pigdir->pig_files) - 1; i>=0; i--) {
		PigFileDestroy(pigdir->pig_files[i]);
	}
	eaDestroyEx(&pigdir->pig_files, NULL);
	eaDestroy(&pigdir->new_files);
	eaDestroy(&pigdir->deleted_files);
	stashTableDestroy(pigdir->htDeletedNames);
	stashTableDestroy(pigdir->htNewNames);
	stashTableDestroy(pigdir->htNameToIndex);

	MP_FREE(PigDir, pigdir);
}

bool pigDirIsDeletedFile(PigDir *pigdir, const char *filename)
{
	int dummy;
	return stashFindInt(pigdir->htDeletedNames, filename, &dummy);
}

bool pigDirIsNewFile(PigDir *pigdir, const char *filename)
{
	int dummy;
	return stashFindInt(pigdir->htNewNames, filename, &dummy);
}

void pigDirDependentFlush(PigDir *pigdir, const char *relpath)
{
	if (pigDirIsNewFile(pigdir, relpath) || pigDirIsDeletedFile(pigdir, relpath))
		pigDirFlush(pigdir);
}

void pigDirFlush(PigDir *pigdir)
{
	char pig_file_name[MAX_PATH];
	// Process deletions first, then add new files
	int i;
	int j;
	bool saved_auto_flush = pigdir->disable_auto_flush;
	bool loaded_new_pigg=false;

	if (eaSize(&pigdir->deleted_files) == 0 &&
		eaSize(&pigdir->new_files) == 0)
	{
		return;
	}

	pigdir->disable_auto_flush = true;
	// Process deletions, and load all files from .piggs that are going to be re-written
	for (i=eaSize(&pigdir->pig_files)-1; i>=0; i--)
	{
		PigFile *pf = pigdir->pig_files[i];
		bool has_changed_file=false;
		for (j=0; j<eaSize(&pigdir->deleted_files) && !has_changed_file; j++) {
			if (PigFileFindIndexQuick(pf, pigdir->deleted_files[j])!=-1)
				has_changed_file = true;
		}
		for (j=0; j<eaSize(&pigdir->new_files) && !has_changed_file; j++) {
			if (PigFileFindIndexQuick(pf, pigdir->new_files[j]->fname)!=-1)
				has_changed_file = true;
		}
		if (has_changed_file)
		{
			int num_files = PigFileGetNumFiles(pf);
			// Read pig into memory, one file at a time
			// Add any non-deleted, non-new files to new_files list
			for (j=0; j<num_files; j++) {
				const char *filename = PigFileGetFileName(pf, j);
				if (!filename || PigFileIsSpecialFile(pf, j))
					continue;
				if (pigDirIsNewFile(pigdir, filename) || pigDirIsDeletedFile(pigdir, filename)) {
					// Skip it!
					memlog_printf(NULL, "Dropping file %s\n", filename);
				} else {
					// Read it in, add to new files
					int len;
					char *data = PigFileExtract(pf, filename, &len);
					memlog_printf(NULL, "Keeping file %s\n", filename);
					if (!data) {
						assertmsg(0, "Failed to read from pig?");
					} else {
						assert((U32)len == PigFileGetFileSize(pf, j));
						pigDirWriteFile(pigdir, filename, data, len, PigFileGetFileTimestamp(pf, j));
						free(data);
					}
				}
			}
			strcpy(pig_file_name, pf->filename);
			// close pig handle
			PigFileDestroy(pf);
			free(pf);
			eaRemove(&pigdir->pig_files, i);
			// delete pig file
			memlog_printf(NULL, "Removing pigg file %s\n", pig_file_name);
			fileRenameToBak(pig_file_name);
			// write new pig file (happens in writing of New data)
		}
	}
	while (eaSize(&pigdir->new_files)!=0)
	{
		NewPigEntry *new_entries;
		int size=0;
		int ret;
		char buf[MAX_PATH];
		pigDirNextFilename(pigdir, SAFESTR(pig_file_name));
		memlog_printf(NULL, "New .pigg file named %s\n", pig_file_name);

		// Find how many files to go in first pigg
		for (i=0; i<eaSize(&pigdir->new_files); i++) {
			size += pigdir->new_files[i]->size;
			pigdir->new_data -= pigdir->new_files[i]->size;
			if (size >= PIG_FILE_SIZE) {
				i++;
				break;
			}
		}

		// Make entries, free old data, compact list
		new_entries = calloc(sizeof(NewPigEntry), i);
		for (j=0; j<i; j++) {
			new_entries[j] = *pigdir->new_files[j];
			SAFE_FREE(pigdir->new_files[j]);
		}
		memmove(&pigdir->new_files[0], &pigdir->new_files[i], (eaSize(&pigdir->new_files) - i)*sizeof(pigdir->new_files[0]));
		eaSetSize(&pigdir->new_files, eaSize(&pigdir->new_files) - i);

		fileLocateWrite(pig_file_name, buf);
		ret = pigCreateFromMem(new_entries, i, buf, PIG_VERSION);
		assert(ret);

		// Free memory
		for (j=0; j<i; j++) {
			SAFE_FREE(new_entries[j].fname);
			SAFE_FREE(new_entries[j].data);
		}

		// Load new pig
		pigDirLoadPig(pigdir, pig_file_name);
		loaded_new_pigg = true;
	}
	pigDirReIndex(pigdir);
	stashTableClear(pigdir->htNewNames);
	stashTableClear(pigdir->htDeletedNames);
	eaClearEx(&pigdir->deleted_files, NULL);
	pigdir->disable_auto_flush = saved_auto_flush;

	assert(pigdir->new_data == 0);
}

static char *pigDirFixupPath(PigDir *pigdir, const char *filename, char *relpath, size_t relpath_size)
{
	if (strStartsWith(filename, pigdir->dirname)) {
		strcpy_s(relpath, relpath_size, filename + strlen(pigdir->dirname));
	} else {
		// assume relative path into pigdir
		strcpy_s(relpath, relpath_size, filename);
	}
	forwardSlashes(relpath);
	while (relpath[0]=='/')
		strcpy_s(relpath, relpath_size, relpath+1);
	return relpath;
}

char *pigDirFileAlloc(PigDir *pigdir, const char *filename, int *lenp)
{
	char relpath[MAX_PATH];
	PigFile *pf;
	pigDirFixupPath(pigdir, filename, SAFESTR(relpath));
	pigDirDependentFlush(pigdir, relpath);
	pf = pigDirFindRightPigFile(pigdir, relpath);
	if (pf)
	{
		char *data = PigFileExtract(pf, relpath, lenp);
		assert(data);
		if (data)
			return data;
	}
	return NULL;
}

void pigDirDeleteFiles(PigDir *pigdir, char **files)
{
	int i;
	int j;
	// Because we process deletions then adds in a flush, we have to flush here!
	if (eaSize(&pigdir->new_files))
		pigDirFlush(pigdir);
	for (i=0; i<eaSize(&files); i++) {
		char relpath[MAX_PATH];
		pigDirFixupPath(pigdir, files[i], SAFESTR(relpath));
		j = eaPush(&pigdir->deleted_files, strdup(relpath));
		stashAddInt(pigdir->htDeletedNames, pigdir->deleted_files[j], 1, false);
	}
}

void pigDirWriteFile(PigDir *pigdir, const char *filename, const char *data, int len, U32 timestamp)
{
	int j;
	NewPigEntry *npe = calloc(sizeof(*npe), 1);
	char relpath[MAX_PATH];
	pigDirFixupPath(pigdir, filename, SAFESTR(relpath));

	npe->fname = strdup(relpath);
	npe->timestamp = timestamp;
	npe->data = malloc(len);
	memcpy(npe->data, data, len);
	npe->size = len;
	j = eaPush(&pigdir->new_files, npe);
	pigdir->new_data += len;

	stashAddInt(pigdir->htNewNames, pigdir->new_files[j]->fname, 1, false);

	if (!pigdir->disable_auto_flush && pigdir->new_data > 2 * PIG_FILE_SIZE) // Flush every X MB
		pigDirFlush(pigdir);
}

const char *pigDirHeaderAlloc(PigDir *pigdir, const char *filename, int *lenp)
{
	char relpath[MAX_PATH];
	PigFile *pf;
	pigDirFixupPath(pigdir, filename, SAFESTR(relpath));
	pigDirDependentFlush(pigdir, relpath);
	pf = pigDirFindRightPigFile(pigdir, relpath);
	if (pf) {
		S32 index = PigFileFindIndexQuick(pf, relpath);
		assert(index!=-1);
		if (index!=-1) {
			const U8 * data = PigFileGetHeaderData(pf, index, lenp);
			if (data==NULL) {
				memlog_printf(NULL, "Warning: requested header on file that does not have a header : %s", filename);
				return NULL;
			}
			return data;
		}
	}
	return NULL;
}

void pigDirScanAllFiles(PigDir *pigdir, PigDirScanProcessor processor)
{
	int i;
	pigDirFlush(pigdir);
	for (i=0; i<eaSize(&pigdir->pig_files); i++)
	{
		U32 j;
		PigFile *pf = pigdir->pig_files[i];
		U32 num_files = PigFileGetNumFiles(pf);
		for (j=0; j<num_files; j++) {
			const char *filename = PigFileGetFileName(pf, j);
			if (!filename || PigFileIsSpecialFile(pf, j))
				continue;
			processor(pigdir, filename);
		}
	}
}


static void printFilename(PigDir *pd, const char *fn)
{
	int len;
	char *data = pigDirFileAlloc(pd, fn, &len);
	printf("%s\n", fn);
	free(data);
}

void pigDirTest()
{
	PigDir *pd = createPigDir("lightmaps/_jimb/test_0000");
	char *s;
	int len;
	char *dummy_data = "abcdefghijklmnopqrstuvwxyz";
	char **dummy_list=NULL;
	assert(pd);
	cryptMD5Init();
	pigDirScanAllFiles(pd, printFilename);
	s = pigDirFileAlloc(pd, "dummy.txt", &len);
	SAFE_FREE(s);
	pigDirWriteFile(pd, "dummy.txt", dummy_data, (int)strlen(dummy_data), _time32(NULL));
	s = pigDirFileAlloc(pd, "dummy.txt", &len);
	SAFE_FREE(s);
	pigDirFlush(pd);
	s = pigDirFileAlloc(pd, "dummy.txt", &len);
	SAFE_FREE(s);
	eaPush(&dummy_list, "dummy3.txt");
	pigDirDeleteFiles(pd, dummy_list);
	pigDirWriteFile(pd, "dummy3.txt", dummy_data, (int)strlen(dummy_data), _time32(NULL));
	pigDirFlush(pd);
	pigDirWriteFile(pd, "dummy4.txt", dummy_data, (int)strlen(dummy_data), _time32(NULL));
	pigDirFlush(pd);
	destroyPigDir(pd);
	assert(heapValidateAll());  
}
