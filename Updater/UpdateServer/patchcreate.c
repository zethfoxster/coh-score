#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "patchcreate.h"
#include "patchfileutils.h"
#include "filechecksum.h"
#include "timing.h"
#include "utils.h"
#include "bindiff.h"
#include "zlib.h"
#include "crypt.h"
#include "mathutil.h"
#include "patchutils.h"
#include "StashTable.h"
#include "wininclude.h"
#include "cpu_count.h"

#define PATCHFILE_VERSION 1

extern int num_threads;

static U32 zipAndWrite(FILE *file,void *data,U32 len)
{
	void	*zip_data;
	U32		zip_len;

	zip_data = zipData(data,len,&zip_len);
	safeWriteBytes(file,ftell(file),zip_data,zip_len);
	free(zip_data);
	return zip_len;
}

static __declspec(thread) int patch_timer;

static void startPatchTimer(void)
{
	if (!patch_timer)
		patch_timer = timerAlloc();
	timerStart(patch_timer);
}

static void printPatchStats(const char *msg,const char *from,U32 raw_size,U32 zip_size)
{
	char	buf1[100],buf2[100];

	printf("%s  From %-14s  ",msg,from);
	printf("%7s Raw  %7s Zip  %0.1f sec\n",printUnit(buf1,raw_size),printUnit(buf2,zip_size),timerElapsed(patch_timer));
}

static void checkPigFileTime(StuffBuff *sb,const ImageCheck *dst,const ImageCheck *src,char *name,StashTable visited)
{
	const FileCheck *dst_pig;
	const FileCheck *src_pig;
	int dst_visited = 0;

	stashFindPointerConst(dst->filenames,name,&dst_pig);
	stashFindPointerConst(src->filenames,name,&src_pig);
	stashAddressFindInt(visited,dst_pig,&dst_visited);
	if (!dst_visited && (!src_pig || dst_pig->timestamp != src_pig->timestamp))
		addStringToStuffBuff(sb,"TIME \"FILE\" \"%s\" %u %u\n",dst_pig->name, 0, dst_pig->timestamp);
	stashAddressAddInt(visited,dst_pig,1,1);
}

static int openPatchGzData(GzStream *gz,char *patch_name,char *temp_name)
{
	FILE	*temp_file;

	sprintf(temp_name,"%s.working",patch_name);
	mkdirtree(temp_name);
	temp_file = fopen(temp_name,"wb");
	gzStreamCompressInit(gz,temp_file);
	return 1;
}

static FileCheck *findSrcFile(const ImageCheck *src,const FileCheck *dst_file)
{
	FileCheck	*src_file;

	stashFindPointer(src->filenames,dst_file->name,&src_file);
	// Currently, files cannot jump to another pig, so force them to get added/deleted
	if (!src_file || (!src_file->pig_name && !dst_file->pig_name))
		return src_file;
	if (!src_file->pig_name || !dst_file->pig_name)
		return 0;
	else if (stricmp(src_file->pig_name,dst_file->pig_name)!=0)
		return 0;
	return src_file;
}

U32 getPackSize(PigCache *cache,ImageCheck *image,FileCheck *file)
{
	PigFileHeader *pfh;

	if (!file->is_compressed)
		return 0;
	pfh = getPfh(cache,image->fname,file->name,file->pig_name);
	if (!pfh)
		return 0;
	return pfh->pack_size;
}

static U32 addFileToPatch(PigCache *cache,StuffBuff *sb,GzStream *gz,const ImageCheck *dst,const FileCheck *dst_file)
{
	U32		unpack_size,pack_size=0,patch_len;
	U8		*dst_mem;

	dst_mem = loadFileData(cache,dst->fname,dst_file->name,dst_file->pig_name,&patch_len,dst_file->is_compressed);
	if (dst_file->is_compressed)
	{
		PigFileHeader *pfh = getPfh(cache,dst->fname,dst_file->name,dst_file->pig_name);
		unpack_size = pfh->size;
		pack_size = patch_len;
	}
	else
		unpack_size = patch_len;
	if (!dst_file->is_compressed && !dst_file->pig_name)
		pack_size = zipAndWrite(gz->file,dst_mem,patch_len);
	else
		safeWriteBytes(gz->file,ftell(gz->file),dst_mem,patch_len);
	addStringToStuffBuff(sb,"ADD \"%s\" \"%s\" %u %u %u %u\n",dst_file->pig_name ? dst_file->pig_name : "FILE",dst_file->name,pack_size,dst_file->timestamp,unpack_size,dst_file->is_compressed);
	free(dst_mem);
	return patch_len;
}

int patchAssembleHeader(char *cmds,char *manifest,char *data_fname,U32 data_unpack_size,char *patch_name)
{
	U32			zip_cmd_size,hdr_size,zip_manifest_size,zip_data_size,data_size;
	U8			*zip_cmd,*zip_manifest;
	PatchHeader	patch;
	FILE		*file;

	file = fopen(data_fname,"r+b");
	fseek(file,0,2);
	data_size = ftell(file);
	assert(file);
	zip_data_size	= fileSize(data_fname);
	zip_cmd			= zipData(cmds,strlen(cmds),&zip_cmd_size);
	zip_manifest	= zipData(manifest,strlen(manifest),&zip_manifest_size);
	hdr_size		= zip_cmd_size + zip_manifest_size;

	patch.version				= PATCHFILE_VERSION;

	patch.cmd.size				= zip_cmd_size;
	patch.cmd.unpack_size		= strlen(cmds);
	patch.cmd.pos				= data_size;

	patch.manifest.size			= zip_manifest_size;
	patch.manifest.unpack_size	= strlen(manifest);
	patch.manifest.pos			= patch.cmd.pos + patch.cmd.size;

	patch.data.size				= zip_data_size;
	patch.data.unpack_size		= data_unpack_size;
	patch.data.pos				= sizeof(patch);

	safeWriteBytes(file,ftell(file),zip_cmd,patch.cmd.size);
	safeWriteBytes(file,ftell(file),zip_manifest,patch.manifest.size);
	free(zip_cmd);
	free(zip_manifest);
	data_size = ftell(file);
	safeWriteBytes(file,0,&patch,sizeof(patch));

	// Checksum the file
	{
		#define CHUNK_SIZE (8 << 20)
		U8 *chunk;
		U32	i,chunk_size,bytesread;

		patch.checksum.size = data_size - sizeof(Checksum);
		fseek(file,sizeof(Checksum),0);
		chunk = malloc(CHUNK_SIZE);
		xferStatsInit("Checksumming patch",NULL,0,data_size);
		for(i=0;i<patch.checksum.size;i+=CHUNK_SIZE)
		{
			chunk_size = MIN(CHUNK_SIZE,patch.checksum.size-i);
			bytesread = fread(chunk,1,chunk_size,file);
			if (bytesread != chunk_size)
				FatalErrorf("error reading from file %s (%d/%d)",data_fname);
			cryptMD5Update(chunk,chunk_size);
			xferStatsUpdate(chunk_size);
		}
		free(chunk);
		xferStatsFinish();
		cryptMD5Final(patch.checksum.values);
		safeWriteBytes(file,0,&patch,sizeof(Checksum));
	}
	fclose(file);
	safeRenameFile(data_fname,patch_name);
	return 1;
}

static void patchCreateInternal(const ImageCheck *src,const ImageCheck *dst,char *patch_name,char *msg,int no_diff, int threaded)
{
	const FileCheck *src_file;
	const FileCheck *dst_file;
	int			i,has_piggs = 0;
	U32			data_unpack_size=0;
	StuffBuff	sb;
	PigCache	cache = {0};
	char		temp_name[MAX_PATH];
	GzStream	gz;
	PatchHeader	blank_header = {0};
	U32			bytecount=0,update_bytes;
	StashTable visited;

	startPatchTimer();
	assert(src->pigs_opened);
	assert(dst->pigs_opened);

	visited = stashTableCreateAddress(1000);
	for(i=0;i<dst->file_count;i++)
	{
		dst_file = dst->files[i];
		if (strEndsWith(dst_file->name,".pigg")) {
			has_piggs = 1;
			continue;
		}
		bytecount += dst->files[i]->full.size;
	}
	xferStatsInit("Creating patch", threaded ? msg : NULL,0,dst->bytecount);
	openPatchGzData(&gz,patch_name,temp_name);
	safeWriteBytes(gz.file,0,&blank_header,sizeof(PatchHeader));

	fflush(fileGetStdout());
	initStuffBuff(&sb,1024);

	if(has_piggs) {
		// Tell CohUpdater which patch version we are sending for newer CohUpdater clients
		// Older clients do not understand this so omit it from the file
		addStringToStuffBuff(&sb,"VERSION \"FILE\" \"FILE\" %u %u\n",0,0);
	}

	for(i=0;i<dst->file_count;i++)
	{
		int		dst_len,patch_len;
		U8		*dst_mem,*patch_mem,*pig_name;

		dst_file = dst->files[i];
		src_file = findSrcFile(src,dst_file);
		if (strEndsWith(dst_file->name,".pigg"))
		{
			continue;
		}
		if (dst_file->is_compressed)
		{
			PigFileHeader	*pfh;

			pfh = getPfh(&cache,dst->fname,dst_file->name,dst_file->pig_name);
			if (pfh)
				update_bytes = pfh->pack_size;
			else
				update_bytes = dst_file->full.size;
		}
		else
			update_bytes = dst_file->full.size;

		// if the files have the same data, timestamp, and filename capitalization, exclude from patch
		if (src_file && checksumMatch(&src_file->full,&dst_file->full) && src_file->timestamp == dst_file->timestamp && strcmp(src_file->name,dst_file->name)==0 && src_file->is_compressed == dst_file->is_compressed)
		{
			xferStatsUpdate(update_bytes);
			continue;
		}
		pig_name = dst_file->pig_name;
		if (!pig_name)
			pig_name = "FILE";
		else
			checkPigFileTime(&sb,dst,src,pig_name,visited);
		if (!src_file)
			patch_len = addFileToPatch(&cache,&sb,&gz,dst,dst_file);
		else
		{
			U8		*src_mem;
			U32		src_len,pack_size;

			dst_mem = loadFileData(&cache,dst->fname,dst_file->name,dst_file->pig_name,&dst_len,0);
			src_mem = loadFileData(&cache,src->fname,src_file->name,src_file->pig_name,&src_len,0);
			patch_len = bindiffCreatePatch(src_mem,src_len,dst_mem,dst_len,&patch_mem,no_diff ? 0 : 32);
			free(dst_mem);
			free(src_mem);
			pack_size = zipAndWrite(gz.file,patch_mem,patch_len);
			addStringToStuffBuff(&sb,"MOD \"%s\" \"%s\" %u %u %u %u\n",pig_name,dst_file->name,pack_size,dst_file->timestamp,patch_len,dst_file->is_compressed);
			free(patch_mem);
		}
		xferStatsUpdate(update_bytes);
		data_unpack_size += patch_len;
	}
	for(i=0;i<dst->file_count;i++)
	{
		dst_file = dst->files[i];
		if (strEndsWith(dst_file->name,".pigg"))
			checkPigFileTime(&sb,dst,src,dst_file->name,visited);
	}
	stashTableDestroy(visited);

	xferStatsFinish();
	for(i=0;i<src->file_count;i++)
	{
		if (!stashFindPointer(dst->filenames,src->files[i]->name, NULL))
			addStringToStuffBuff(&sb,"DEL \"%s\" \"%s\"\n",src->files[i]->pig_name ? src->files[i]->pig_name : "FILE", src->files[i]->name);
	}
	gzStreamFree(&gz);
	fclose(gz.file);

	{
		char		*s,patch_cmds[MAX_PATH];

		strcpy(patch_cmds,patch_name);
		getDirectoryName(patch_cmds);
		strcat(patch_cmds,"/patchcmds/");
		strcat(patch_cmds,getFileName(patch_name));
		s = strrchr(patch_cmds,'.');
		strcpy(s,".patchcmds");
		mkdirtree(patch_cmds);
		safeWriteFile(patch_cmds,"wt",sb.buff,strlen(sb.buff));

		patchAssembleHeader(sb.buff,dst->manifest_string,temp_name,data_unpack_size,patch_name);
		printPatchStats(msg,getFileName(src->fname),data_unpack_size + strlen(dst->manifest_string) + strlen(sb.buff),fileSize(patch_name));
		freeStuffBuff(&sb);
		closeOpenPigs(&cache);
	}
}

void patchCreate(ImageCheck *src,ImageCheck *dst,char *patch_name,char *msg,int no_diff)
{
	checksumOpenPigs(src,src->fname,0);
	checksumOpenPigs(dst,dst->fname,0);
	patchCreateInternal(src, dst, patch_name, msg, no_diff, 0);
}

#define MAX_THREADS_ASYNC 8
STATIC_ASSERT(MAX_THREADS_ASYNC <= MAXIMUM_WAIT_OBJECTS);

typedef struct patchCreateAsyncParams {
	ImageCheck *src;
	ImageCheck *dst;
	char *patch_name;
	char *msg;
	int no_diff;
} patchCreateAsyncParams;

static int maxThreads = 0;
static HANDLE patchThreads[MAX_THREADS_ASYNC];
static patchCreateAsyncParams asyncPatches[MAX_THREADS_ASYNC];

unsigned __stdcall patchCreateThreaded(patchCreateAsyncParams * params)
{
	cryptInit();
	patchCreateInternal(params->src, params->dst, params->patch_name, params->msg, params->no_diff, 1);
	free(params->patch_name);
	free(params->msg);
	return 0;
}

void patchCreateAsync(ImageCheck *src,ImageCheck *dst,char *patch_name,char *msg,int no_diff)
{
	int i;
	patchCreateAsyncParams * params = NULL;	

	if(num_threads) {
		maxThreads = MIN(MAX_THREADS_ASYNC, num_threads);
	} else if(!maxThreads) {
		maxThreads = MIN(MAX_THREADS_ASYNC, getNumVirtualCpus());
	}

	// Not enough processors for multithreaded mode, run the synchronous version
	if(maxThreads <= 1) {
		patchCreate(src, dst, patch_name, msg, no_diff);
		return;
	}

	// Find an open slot to use
	for(i = 0; i < maxThreads; i++) {
		if(!patchThreads[i]) {
			params = asyncPatches + i;
			break;
		}
	}

	// Currently no slots open, wait for one to complete
	if(i == maxThreads) {
		DWORD found = WaitForMultipleObjects(maxThreads, patchThreads, FALSE, INFINITE);
		assert(found > WAIT_OBJECT_0 && found <= (WAIT_OBJECT_0 + maxThreads));
		i = found - WAIT_OBJECT_0;
		params = asyncPatches + i;
		patchThreads[i] = NULL;
	}

	// Open piggs from a single thread for the source because it may be shared
	checksumOpenPigs(src,src->fname,0);
	checksumOpenPigs(dst,dst->fname,0);

	params->src = src;
	params->dst = dst;
	params->patch_name = strdup(patch_name);
	params->msg = strdup(msg);
	params->no_diff = no_diff;

	patchThreads[i] = (HANDLE)_beginthreadex(0, 0, &patchCreateThreaded, params, 0, NULL);
}

void patchCreateWait()
{
	int i;
	int count = 0;
	HANDLE waitList[MAX_THREADS_ASYNC];

	// Build the list of handles to wait for
	for(i = 0; i < maxThreads; i++) {
		if(patchThreads[i]) {
			waitList[count++] = patchThreads[i];
		}
	}

	if(count) {	
		// Wait for all handles to complete
		verify(WaitForMultipleObjects( count, waitList, TRUE, INFINITE ) == WAIT_OBJECT_0);
	}
}


// ugly, hackish way to force the update server to a new version
//
// this function performs nothing useful
void FunctionWithNoPurposeOtherThanToForceALink()
{
	int junk = 0;
	// to force a link, i just up the num_iterations.  
	// i just hope we never get above 4 billion versions of the update server.
	unsigned int i, num_iterations = 4; 
	junk = patch_timer;

	if ( junk == 12 )
	{
		++junk;
		return;
	}

	for ( i = 0; i < num_iterations; ++i )
	{
		junk--;
	}
}
