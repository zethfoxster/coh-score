#include "persist_internal.h"
#include "persist_flat.h"
#include "textparser.h"
#include "assert.h"
#include "earray.h"
#include "textcrcdb.h"
#include "utils.h"
#include "error.h"
#include "file.h"
#include "log.h"

// GGFIXME: there are a lot of return values (for file operations and whatnot) that aren't checked
// GGFIXME: s_parse_flat isn't thread safe

typedef struct TextCRCDb
{
	void **structptrs;
	U32 parse_crc;
	U32 data_crc;
	U32 last_key;
} TextCRCDb;

static ParseTable s_parse_flat[] =
{
	// filled in below
	{ "ParseCRC",	TOK_INT(TextCRCDb, parse_crc, 0)										},
	{ "DataCRC",	TOK_INT(TextCRCDb, data_crc, 0)											},
	{ "LastKey",	TOK_INT(TextCRCDb, last_key, 0)											},
	{ ".",			TOK_INDIRECT|TOK_EARRAY|TOK_STRUCT_X, offsetof(TextCRCDb, structptrs)	},
	{ "", 0 }
};

int persist_loadTextDb(PersistInfo *info, void(*cleanup)(PersistInfo*))
{
	TextCRCDb loaded = {0};
	char fname[MAX_PATH];
	char *structname = info->config->structname ? info->config->structname : info->typeName;
	int success = 1;

	s_parse_flat[0].type = s_parse_flat[1].type = (info->config->flags & PERSIST_CRCDB) ? TOK_INT_X : TOK_IGNORE;
	s_parse_flat[3].name = structname;
	s_parse_flat[3].param = info->settings.size;
	s_parse_flat[3].subtable = info->settings.tpi;

	assert(!info->structptrs);
	strcpy(fname, info->config->fname); // fileLocateRead(info->config->fname, fname);
	if(info->config->flags & PERSIST_CRCDB)
	{
		TextCRCDb totest = {0};
		if(TextCRCDb_ValidateNewFile(fname, s_parse_flat, &totest.parse_crc, &totest.data_crc, &totest))
		{
			if(cleanup)
				cleanup(info);
			if(!TextCRCDb_MoveNewFileToCur(fname, !!(info->config->fail_time & PERSIST_EXPONENTIAL), 1))
			{
				LOG(LOG_TEXT_JOURNAL, LOG_LEVEL_VERBOSE, LOG_CONSOLE_ALWAYS, "ERROR: Could not move new database into place!");
				success = 0;
			}
		}
		if(success)
			success = TextCRCDb_LoadFile(fname, s_parse_flat, &loaded.parse_crc, &loaded.data_crc, &loaded);
	}
	else
	{
		if(!fileExists(fname))
		{
			// check if we were in the middle of a swap
			char fname_new[MAX_PATH];
			sprintf(fname_new, "%s.new", fname);
			if(fileExists(fname_new))
			{
				if(cleanup)
					cleanup(info);
				// GGFIXME: there's a race condition here that could lose a journal (no race for CRCDB)
				if(!TextCRCDb_MoveNewFileToCur(fname, !!(info->config->fail_time & PERSIST_EXPONENTIAL), 1))
				{
					LOG(LOG_TEXT_JOURNAL, LOG_LEVEL_VERBOSE, LOG_CONSOLE_ALWAYS, "ERROR: Could not move new database into place!");
					success = 0;
				}
			}
		}
		if(success)
		{
			if(fileExists(fname))
				success = ParserReadTextFile(fname, s_parse_flat, &loaded.structptrs);
			else
				LOG(LOG_TEXT_JOURNAL, LOG_LEVEL_VERBOSE, LOG_CONSOLE_ALWAYS, "Warning: %s does not exist, assuming new database.", fname);
		}
	}
	info->structptrs = loaded.structptrs;
	info->key_last = loaded.last_key;

	return !!success;
}

int persist_flushTextDb(PersistInfo *info, void(*cleanup)(PersistInfo*))
{
	TextCRCDb towrite = {0};
	char fname[MAX_PATH];
	char *structname = info->config->structname ? info->config->structname : info->typeName;
	int success;

	s_parse_flat[0].type = s_parse_flat[1].type = (info->config->flags & PERSIST_CRCDB) ? TOK_INT_X : TOK_IGNORE;
	s_parse_flat[3].name = structname;
	s_parse_flat[3].param = info->settings.size;
	s_parse_flat[3].subtable = info->settings.tpi;

	strcpy(fname, info->config->fname); // fileLocateWrite(info->config->fname, fname);
	towrite.structptrs = info->structptrs;
	towrite.last_key = info->key_last;
	mkdirtree(fname);
	if(info->config->flags & PERSIST_CRCDB)
	{
		// TODO: integrate the textcrc stuff into the persist layer directly
		success = TextCRCDb_WriteNewFile(fname, s_parse_flat, &towrite.parse_crc, &towrite.data_crc, &towrite);
		if(success)
		{
			TextCRCDb totest = {0};
			success = TextCRCDb_ValidateNewFile(fname, s_parse_flat, &totest.parse_crc, &totest.data_crc, &totest);
		}
	}
	else
	{
		char fname_new[MAX_PATH];
		sprintf(fname_new, "%s.new", fname);
		success = ParserWriteTextFile(fname_new, s_parse_flat, &towrite, 0, 0);
	}

	if(success)
	{
		if(cleanup)
			cleanup(info);
		success = TextCRCDb_MoveNewFileToCur(fname, !!(info->config->flags & PERSIST_EXPONENTIAL), 1);
	}

	return !!success;
}

int loader_flat(PersistInfo *info)
{
	return persist_loadTextDb(info, NULL);
}

int flusher_flat(PersistInfo *info)
{
	return persist_flushTextDb(info, NULL);
}
