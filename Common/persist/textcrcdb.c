/***************************************************************************
 *     Copyright (c) 2006-2007, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/

#include "textcrcdb.h"
#include "textparser.h"
#include "assert.h"
#include "file.h"
#include "EString.h"
#include "error.h"
#include "utils.h"
#include "timing.h"
#include "fileutil.h"
#include "wininclude.h" // Sleep
#include "log.h"

// TODO: some of these should be fatal on failure

static void s_TextCRCDb_SetCRCs(ParseTable *pti, U32 *ver, U32 *crc, void *structptr)
{
	*ver = *crc = 0; // clear these out to make sure StructCRC is consistent

	*crc = StructCRC(pti,structptr);
	*ver = ParseTableCRC(pti,NULL);
}

bool TextCRCDb_WriteNewFile(char *fn, ParseTable *pti, U32 *ver, U32 *crc, void *structptr)
{
	char fn_new[MAX_PATH];
	bool res;

	sprintf(fn_new,"%s.new",fn);

	loadstart_printf("writing %s...\n",fn_new);
	s_TextCRCDb_SetCRCs(pti,ver,crc,structptr);
	res = ParserWriteTextFile(fn_new,pti,structptr,0,0);
	loadend_printf("");
	return res;
}

bool TextCRCDb_WriteEString(char **estr_dst, ParseTable *pti, U32 *ver, U32 *crc, void *structptr)
{
	if(!estr_dst)
		return false;

	estrClear(estr_dst); // currently do not allow appending, to keep the data trustworthy

	s_TextCRCDb_SetCRCs(pti,ver,crc,structptr);
	return ParserWriteText(estr_dst,pti,structptr,0,0);
}

bool TextCRCDb_WriteNewFileFromEString(char *fn, char *estr_src)
{
	char fn_new[MAX_PATH];
	FILE *fout;
	int bytes_to_write = estrLength(&estr_src);
	int bytes_written;

	if(!fn || !estr_src)
		return false;

	sprintf(fn_new,"%s.new",fn);

	fout = fileOpen(fn_new,"wb");
	if(!fout)
	{
		LOG( LOG_TEXT_JOURNAL, LOG_LEVEL_VERBOSE, LOG_CONSOLE_ALWAYS, "Error: %s could not be opened for writing.",fn_new);
		return false;  // should this be fatal?
	}

	bytes_written = fwrite(estr_src,sizeof(char),bytes_to_write,fout);
	if(bytes_written != bytes_to_write)
	{
		LOG( LOG_TEXT_JOURNAL, LOG_LEVEL_VERBOSE, LOG_CONSOLE_ALWAYS, "Error: Tried to write %d bytes to %s, but only wrote %d.",bytes_to_write,fn_new,bytes_written);
		fclose(fout);
		if(unlink(fn_new) != 0)
			LOG( LOG_TEXT_JOURNAL, LOG_LEVEL_VERBOSE, LOG_CONSOLE_ALWAYS, "Warning: %s could not be deleted.", fn_new);
		return false; // should this be fatal?
	}
	fclose(fout);
	return true;
}

static bool TextCRCDb_LoadFileInternal(char *fn, ParseTable *pti, U32 *ver, U32 *crc, void *structptr, const char *action)
{
	U32 ver_read;
	U32 crc_read;

	loadstart_printf("%s %s...\n",action,fn);

	if(!fileExists(fn))
	{
		loadend_printf("");
		LOG( LOG_TEXT_JOURNAL, LOG_LEVEL_VERBOSE, LOG_CONSOLE_ALWAYS,"Warning: %s does not exist, assuming new database",fn);
		return true; 
	}

	if(!ParserReadTextFile(fn,pti,structptr))
	{
		loadend_printf("");
		LOG( LOG_TEXT_JOURNAL, LOG_LEVEL_VERBOSE, LOG_CONSOLE_ALWAYS,"Error: %s could not be parsed, data is corrupted!",fn);
		return false; // should this be fatal?
	}

	ver_read = *ver;
	crc_read = *crc;
	*ver = *crc = 0;

	if(ver_read != (U32)ParseTableCRC(pti,NULL))
	{
		loadend_printf("");
		LOG( LOG_TEXT_JOURNAL, LOG_LEVEL_VERBOSE, LOG_CONSOLE_ALWAYS,"Warning: %s had its parse table changed, ignoring crc.  This is normal for a new publish.",fn);
		return true;
	}

	if(crc_read != StructCRC(pti,structptr))
	{
		loadend_printf("");
		LOG( LOG_TEXT_JOURNAL, LOG_LEVEL_VERBOSE, LOG_CONSOLE_ALWAYS,"Error: %s failed its crc check, data is corrupted!",fn);
		return false;
	}

	*ver = ver_read;
	*crc = crc_read;
	loadend_printf("");
	return true;
}

bool TextCRCDb_ValidateNewFile(char *fn, ParseTable *pti, U32 *ver, U32 *crc, void *structptr)
{
	char fn_new[MAX_PATH];
	sprintf(fn_new,"%s.new",fn);

	LOG( LOG_TEXT_JOURNAL, LOG_LEVEL_VERBOSE, LOG_CONSOLE_ALWAYS,"verifying %s",fn_new);

	if(!fileExists(fn_new))
	{
		LOG( LOG_TEXT_JOURNAL, LOG_LEVEL_VERBOSE, LOG_CONSOLE_ALWAYS,"%s does not exist to be verified",fn_new);
		return false;
	}
	
	if(TextCRCDb_LoadFileInternal(fn_new, pti, ver, crc, structptr, "verifying"))
	{
		StructClear(pti,structptr);
		return true;
	}

	return false;
}

bool TextCRCDb_LoadFile(char *fn, ParseTable *pti, U32 *ver, U32 *crc, void *structptr)
{
	LOG( LOG_TEXT_JOURNAL, LOG_LEVEL_VERBOSE, LOG_CONSOLE_ALWAYS,"loading %s",fn);
	return TextCRCDb_LoadFileInternal(fn,pti,ver,crc,structptr,"loading");
}

static int s_TextCRCDb_ExponentialArchiveHelper(char *dir, char *fn, int delete_id)
{
	// this function scans the file's directory,
	// looking for either the last archive or a specific archive to delete
	int res = 0;
	int i, file_count;
	char **files = fileScanDir(dir,&file_count);
	char *match_expr = NULL;
	char *dot = strrchr(fn,'.');

	estrPrintCharString(&match_expr,fn);
	if(dot)
		estrSetLength(&match_expr,dot-fn);
	estrConcatStaticCharArray(&match_expr, "_*.zip");

	for( i = 0; i < file_count; ++i ) 
	{
		char *file = files[i];
		char *last_match;
		int version;

		if(!file || !simpleMatch(match_expr,file))
			continue;

		last_match = getLastMatch();
		if(!last_match)
			continue;

		version = atoi(last_match);

		if(delete_id)
		{
			// we want to delete a file, there should only ever be one
			if(version == delete_id)
			{
				if(fileForceRemove(file))
					printf("Warning: Could not delete archive %s\n",file);
				break;
			}
		}
		else if(version > res)
		{
			// we are just looking for the highest version
			res = version;
		}
	}
	if(delete_id && !devassert(i < file_count))
		LOG( LOG_TEXT_JOURNAL, LOG_LEVEL_VERBOSE, LOG_CONSOLE_ALWAYS,"Warning: Could not find archive %i of %s for deletion",delete_id,fn);

	estrDestroy(&match_expr);
	fileScanDirFreeNames(files,file_count);
	return res;
}

static bool s_forceRename(const char *src, const char *dst, bool force)
{
	int retry = 0;
	while(rename(src, dst))
	{
		char buf[1024];
		strerror_s(SAFESTR(buf), errno);
		if(force)
		{
			Sleep(1000);
			LOG( LOG_TEXT_JOURNAL, LOG_LEVEL_VERBOSE, LOG_CONSOLE_ALWAYS,"moving %s to %s (%s) (retry %i).", src, dst, buf, ++retry);
		}
		else
		{
			LOG( LOG_TEXT_JOURNAL, LOG_LEVEL_VERBOSE, LOG_CONSOLE_ALWAYS,"Error: Could not move %s to %s. (%s)", src, dst, buf);
			return false;
		}
	}
	return true;
}

static bool s_TextCRCDb_ExponentialArchive(char *fn_cur, bool force)
{
	char dir[MAX_PATH];
	char fn_bak[MAX_PATH];
	char fn_zip[MAX_PATH];
	int backup_id, delete_id, low_bit;
	char *dot;
	struct tm time = {0};

	strcpy(dir,fn_cur);
	if(dot = strrchr(dir,'/'))
		*(dot+1) = '\0';
	else if(dot = strrchr(dir,'\\'))
		*(dot+1) = '\0';
	
	backup_id = s_TextCRCDb_ExponentialArchiveHelper(dir,fn_cur,0)+1;

	timerMakeTimeStructFromSecondsSince2000(timerSecondsSince2000(),&time);

	// create the archive name "filename_iteration_date.zip"
	strcpy(fn_bak,fn_cur);
	if(dot = strrchr(fn_bak,'.'))
		*dot = '\0';
	else
		dot = fn_bak + strlen(fn_bak);

	sprintf_s(dot,MAX_PATH-(dot-fn_bak),"_%.5i_%.4i-%.2i-%.2i-%.2i-%.2i-%.2i.zip",backup_id,
				time.tm_year+1900,time.tm_mon+1,time.tm_mday,time.tm_hour,time.tm_min,time.tm_sec);

	sprintf(fn_zip,"%s.zip",fn_cur);

	fileZip(fn_cur); // zips "filename.ext" to "filename.ext.zip"

	if(!fileExists(fn_zip))
	{
		LOG( LOG_TEXT_JOURNAL, LOG_LEVEL_VERBOSE, LOG_CONSOLE_ALWAYS,"Error: Could not zip %s for archival.",fn_cur);
		return false;
	}
	
	if(!s_forceRename(fn_zip, fn_bak, force)) // moves the archive into position
		return false; // logged above

	// this scheme has the following properties:
	// the number of archives kept at iteration n is log2(n)
	// there is only one deletion per iteration, except when the iteration is a power of 2 (when there are none)
	// archives are never altered except when they are first put into place and when they are deleted
	// each archive kept is approximately twice as old as the next (where age is in iterations)
	//
	// here are the archives kept for each of the first sixteen iterations:
	// (the archive number is the iteration it was created, 1 is the first archive, 16 is the newest)
	//  1				no deletion
	//  1, 2			no deletion
	//  2, 3			delete 1  ( 3-2)
	//  2, 3, 4			no deletion
	//  2, 4, 5			delete 3  ( 5-2)
	//  4, 5, 6			delete 2  ( 6-4)
	//  4, 6, 7			delete 5  ( 7-2)
	//  4, 6, 7, 8		no deletion
	//  4, 6, 8, 9		delete 7  ( 9-2)
	//  4, 8, 9,10		delete 6  (10-4)
	//  4, 8,10,11		delete 9  (11-2)
	//  8,10,11,12		delete 4  (12-8)
	//  8,10,12,13		delete 11 (13-2)
	//  8,12,13,14		delete 10 (14-4)
	//  8,12,14,15		delete 13 (15-2)
	//  8,12,14,15,16	no deletion
	//
	// this algorithm was determined empirically
	// there's likely a more elegant version, but i'll leave that as an exercise to the reader

	for(low_bit = 0; !(backup_id & (1<<low_bit)); ++low_bit)
		;

	delete_id = backup_id - (1<<(low_bit+1));

	if(delete_id > 0) // if delete_id is negative, that means backup_id was a power of 2
		s_TextCRCDb_ExponentialArchiveHelper(dir,fn_cur,delete_id);

	return true;
}

bool TextCRCDb_MoveNewFileToCur(char *fn, bool exp_archive, bool force)
{
	char fn_new[MAX_PATH];
	sprintf(fn_new,"%s.new",fn);

	LOG( LOG_TEXT_JOURNAL, LOG_LEVEL_VERBOSE, LOG_CONSOLE_ALWAYS,"moving %s to %s.",fn_new,fn);

	if ( fileExists(fn) )
	{
		int retryCount = 0;

		// this should zip up the current file and remove any old backups - sometimes, though it fails to remove the original current file
		if (exp_archive && !s_TextCRCDb_ExponentialArchive(fn, force))  
			return false;

		// if exp_archive is not set or if the original current files was not removed by the archive, move it to a .bak file
		while (fileExists(fn) && fileRenameToBak(fn) && retryCount < 50 && force)
		{
			retryCount++;
			log_printf(0,"Unable to move %s to %s.  Retry in 5 seconds.", fn_new, fn);
			Sleep(5000);
		}

		if (fileExists(fn))
			return false;
	}

	if(!s_forceRename(fn_new, fn, force))
		return false; // error logged above

	return true;
}
