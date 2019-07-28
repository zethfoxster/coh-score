/***************************************************************************
 *     Copyright (c) 2005-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/

// Compile this file using ArtTools.sln.  Or from the command line:
// cl /Ic:\src\coh\libs\utilitieslib\components /Ic:\src\coh\libs\utilitieslib\utils /Ic:\src\coh\libs\utilitieslib /D "USEZLIB" /D "_DEBUG" /D "_CRTDBG_MAP_ALLOC=1" /W4 /MTd /Ot /Gd /ZI kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib "C:\src\coh\libs\UtilitiesLib\Win32\Full Debug\UtilitiesLib.lib" logfilter.c /link /NODEFAULTLIB:"LIBCMT.lib" /LIBPATH:C:\src\coh\3rdparty\bin

#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <utils/file.h>
#include <utils/utils.h>
#include <components/earray.h>

#define RECORD_SEP "\n"
#define FIELD_SEP "|"

#define BUFFER_IN_SIZE  (1*1024*1024)
#define BUFFER_OUT_SIZE (1*1024*1024)

#define MAX_LINE_SIZE  (4096)

char *pchIn;
char *pchOut;

int g_iInSize = BUFFER_IN_SIZE;
int g_iOutSize = BUFFER_OUT_SIZE;

char *g_pchRecordSep = RECORD_SEP;
char *g_pchFieldSep = FIELD_SEP;
char *g_pchSqlCsvPrefix = "";
char *g_pchSqlCsvSuffix = "";

int *g_piKeeperOps;
char **g_ppchKeeperTags;
char **g_ppchKeeperRests;
int *g_piKeeperLengths;
int *g_piKeeperRestLengths;

typedef enum KeeperOp
{
	kReject = 0,
	kAccept = 1,
} KeeperOp;

typedef enum Format
{
	kRecord,
	kCryptic,
	kSqlCsv,
} Format;

bool g_eFormat = kSqlCsv;

/**********************************************************************func*
 * strappend
 *
 */
static char *strappend(char *dest, char *src)
{
	while((*dest++ = *src++)!=0);

	return dest-1;
}

/**********************************************************************func*
 * strappendpad
 *
 */
static char *strappendpad(char *dest, char *src, int iWidth)
{
	while((*dest++ = *src++)!=0)
		iWidth--;

	dest--;
	for(;iWidth>0;iWidth--)
		*dest++ = ' ';

	*dest = '\0';

	return dest;
}

/**********************************************************************func*
 * strappend_csvstr
 *
 */
static char *strappend_csvstr(char *dest, char *src)
{
	*dest++ = '"';
	while(*src)
	{
		switch(*src)
		{
			case '"':
				*dest++ = '"';
				*dest++ = '"';
				break;
			case ',':
				*dest++ = '\\';
				*dest++ = 'c';
				break;
			case '\n':
				*dest++ = '\\';
				*dest++ = 'n';
				break;
			case '\r':
				break;
			default:
				*dest++ = *src;
		}
		src++;
	}
	*dest++ = '"';
	*dest = '\0';
	return dest;
}

/**********************************************************************func*
 * LoadRuleFile
 *
 */
static int LoadRuleFile(char *pchKeepFile)
{
	FILE *fp;
	char pch[256];

	fp = fopen(pchKeepFile, "rt");
	if(!fp)
	{
		printf("Error: Unable to open rule file: \"%s\"\n", pchKeepFile);
		return 0;
	}

	eaCreate(&g_ppchKeeperTags);
	eaCreate(&g_ppchKeeperRests);
	eaiCreate(&g_piKeeperLengths);
	eaiCreate(&g_piKeeperRestLengths);

	while(fgets(pch, 256, fp))
	{
		KeeperOp op;

		if(*pch == '#' || (*pch == '/' && pch[1] == '/'))
			continue;

		if(*pch)
		{
			if(pch[strlen(pch)-1] == '\n') pch[strlen(pch)-1] = '\0';
			if(pch[strlen(pch)-1] == '\r') pch[strlen(pch)-1] = '\0';
		}

		if(*pch=='+')
		{
			strcpy(pch, pch+1);
			op = kAccept;
		}
		else if(*pch=='-')
		{
			strcpy(pch, pch+1);
			op = kReject;
		}
		else
		{
			op = kAccept;
		}

		if(*pch)
		{
			int iLenTag = 0;
			char *pchTag = NULL;
			int iLenRest = 0;
			char *pchRest = NULL;

			char *pos = strchr(pch, ',');
			if(pos)
			{
				*pos = '\0';
				pos++;

				if(pos[strlen(pos)-1] == '*')
				{
					pos[strlen(pos)-1] = '\0';
					iLenRest = (int) strlen(pos);
				}

				pchRest = strdup(pos);
			}

			if(pch[strlen(pch)-1] == '*')
			{
				pch[strlen(pch)-1] = '\0';
				iLenTag = (int) strlen(pch);
			}

			if(*pch)
			{
				pchTag = strdup(pch);
			}

			eaiPush(&g_piKeeperLengths, iLenTag);
			eaPush(&g_ppchKeeperTags, pchTag);
			eaiPush(&g_piKeeperRestLengths, iLenRest);
			eaPush(&g_ppchKeeperRests, pchRest);

			eaiPush(&g_piKeeperOps, op);
		}
	}

	printf("Loaded %d rules.\n", eaSize(&g_ppchKeeperTags));

	fclose(fp);
	return 1;
}

/**********************************************************************func*
 * Match
 *
 */
int Match(char *pch, char *pchTest, int iLen)
{
	if(pchTest==NULL)
		return 1;

	if(iLen==0)
	{
		if(stricmp(pch, pchTest)==0)
		{
			return 1;
		}
	}
	else
	{
		if(strnicmp(pch, pchTest, iLen)==0)
		{
			return 1;
		}
	}

	return 0;
}

/**********************************************************************func*
 * IsKeeper
 *
 */
static int IsKeeper(char *pchTag, char *pchRest)
{
	int n = eaSize(&g_ppchKeeperTags);
	int i;
	for(i=0; i<n; i++)
	{
		if(Match(pchTag, g_ppchKeeperTags[i], g_piKeeperLengths[i])
			&& Match(pchRest, g_ppchKeeperRests[i], g_piKeeperRestLengths[i]))
		{
			return g_piKeeperOps[i];
		}
	}

	return 0;
}

/**********************************************************************func*
 * FillBuff
 *
 */
static int FillBuff(char *pchIn, char **ppchInCur, char **ppchInEnd, FILE *fp)
{
	char *pos = strchr(*ppchInCur, '\n');
	if(!pos)
	{
		int iLeftover;

		iLeftover = *ppchInEnd-*ppchInCur;
		memmove(pchIn, *ppchInCur, iLeftover);
		*ppchInEnd = pchIn+iLeftover;
		*ppchInCur = pchIn;
		*ppchInEnd += fread(*ppchInEnd, 1, g_iInSize-iLeftover, fp);
		*(*ppchInEnd) = '\0';

		if(!*pchIn)
		{
			// All done.
			return 0;
		}
	}
	return 1;
}

/**********************************************************************func*
 * SkipToEol
 *
 */
void SkipToEol(char **ppchInCur)
{
	char *pos = strchr(*ppchInCur, '\n');
	if(!pos)
		pos = *ppchInCur+strlen(*ppchInCur);
	else
		pos++;

	*ppchInCur=pos;
}

#define CHECK(pos)  if(!(pos))  { printf("Error: Malformed log file at line %d. Attempting to recover... (code %d)\n   <%20.20s>\n", iLine, __LINE__, pchInCur); SkipToEol(&pchInCur); continue; }

/**********************************************************************func*
 * DoIt
 *
 */
static int DoIt(char *pchInFile, char *pchOutFile)
{
	char date[256];
	char times[256];
	char zone[256];
	char ent[256];
	char team[256];
	char tag[256];
	char rest[10000];
	char *pos;

	time_t clock;
	FILE *fpIn;
	FILE *fpOut;
	char *pchInEnd;
	char *pchInCur;
	char *pchOutCur;
	int iLine = 0;
	char *pchIn = malloc(g_iInSize+1);
	char *pchOut = malloc(g_iOutSize+1);
	int errorCount = 0;

	time(&clock);
	printf("Start: %s", asctime(localtime(&clock)));

	if(!pchIn || !pchOut)
	{
		printf("Error: Not enough memory to run. Use -insize and -outsize to reduce the memory needed.\n");
		return 0;
	}


	fpIn = fopen(pchInFile, "rbz");
	if(!fpIn)
	{
		printf("Error: Unable to open input file \"%s\"\n", pchInFile);
		return 0;
	}

	fpOut = fopen(pchOutFile, "wbz");
	if(!fpOut)
	{
		printf("Error: Unable to open output file \"%s\"\n", pchOutFile);
		return 0;
	}


	pchOutCur = pchOut;
	pchInEnd = pchInCur = pchIn;
	*pchInEnd = '\0';

	while(FillBuff(pchIn, &pchInCur, &pchInEnd, fpIn))
	{
		bool bHadQuotes = false;

		iLine++;

		// skip lines with leading whitespace.
		if ( (*pchInCur == ' ') || (*pchInCur == '\t') || (*pchInCur == '\n') || (*pchInCur == '\r') )
		{
			SkipToEol(&pchInCur);
			continue;
		}

		// Skip lines that are bracketed between '{' and '}'.
		if ( (*pchInCur == '{') )
		{
			SkipToEol(&pchInCur);
			while ( FillBuff(pchIn, &pchInCur, &pchInEnd, fpIn) && *pchInCur != '}' )
			{
				iLine++;
				SkipToEol(&pchInCur);
			}
			if (*pchInCur == '}')
			{
				iLine++;
				SkipToEol(&pchInCur);
			}
			continue;
		}

		if(*pchInCur<'0' || *pchInCur>'9')
		{
			if(stricmp(tag,"chat")!=0)
			{
				printf("Error: Malformed log file at line %d. Attempting to recover... (code %d)\n   <%20.20s>\n", iLine, __LINE__, pchInCur);
				errorCount++;
			}
			SkipToEol(&pchInCur);
			continue;
		}

		strncpyt(date, pchInCur, 7);
		pchInCur+=7;
		strncpyt(times, pchInCur, 9);
		pchInCur+=9;

		pos = strchr(pchInCur, ' ');
		CHECK(pos);

		strncpyt(zone, pchInCur, pos-pchInCur+1);
		pos++;
		pchInCur=pos;

		while(*pos==' ')
			pos++;
		CHECK(*pos);
		pchInCur=pos;

		if(*pos=='"')
		{
			pos++;
			pchInCur=pos;
			pos = strchr(pchInCur, '"');
			CHECK(pos);
			strncpyt(ent, pchInCur, pos-pchInCur+1);
			pos++;
			pchInCur=pos;
			bHadQuotes = true;
		}
		else
		{
			pchInCur=pos;
			pos = strchr(pchInCur, ' ');
			CHECK(pos);
			strncpyt(ent, pchInCur, pos-pchInCur+1);
			pos++;
			pchInCur=pos;
		}

		while(*pos==' ')
			pos++;
		CHECK(*pos);

		pchInCur=pos;
		pos = strchr(pchInCur, ' ');
		CHECK(pos);
		strncpyt(team, pchInCur, pos-pchInCur+1);
		pos++;
		pchInCur=pos;

		while(*pos==' ')
			pos++;
		CHECK(*pos);

		pchInCur=pos;

		pos = strchr(pchInCur, ' ');
		CHECK(pos);
		strncpyt(tag, pchInCur, pos-pchInCur+1);
		pos++;
		pchInCur=pos;

		pos = strchr(pchInCur, '\n');
		if(!pos) pos = pchInCur+strlen(pchInCur);
		if(*(pos-1)=='\r')
			strncpyt(rest, pchInCur, pos-pchInCur);
		else
			strncpyt(rest, pchInCur, pos-pchInCur+1);
		if(*pos) pos++;
		pchInCur=pos;

		if(IsKeeper(tag, rest))
		{
			if(pchOutCur > pchOut+g_iOutSize-MAX_LINE_SIZE)
			{
				fwrite(pchOut, 1, pchOutCur-pchOut, fpOut);
				pchOutCur = pchOut;
			}

			switch(g_eFormat)
			{
				case kCryptic:
				{
					char ach[256];
					char *name = ent;

					if(bHadQuotes)
					{
						name = ach;
						*name++ = '"';
						name = strappend(name, ent);
						*name++ = '"';
						*name++ = '\0';
						name = ach;
					}

					pchOutCur = strappend(pchOutCur, date);
					*pchOutCur++ = ' ';
					pchOutCur = strappend(pchOutCur, times);
					*pchOutCur++ = ' ';
					pchOutCur = strappendpad(pchOutCur, zone, 18);
					*pchOutCur++ = ' ';
					pchOutCur = strappendpad(pchOutCur, name, 18);
					*pchOutCur++ = ' ';
					pchOutCur = strappend(pchOutCur, team);
					*pchOutCur++ = ' ';
					*pchOutCur++ = '[';
					pchOutCur = strappend(pchOutCur, tag);
					*pchOutCur++ = ']';
					*pchOutCur++ = ' ';
					pchOutCur = strappend(pchOutCur, rest);
					*pchOutCur++ = '\r';
					*pchOutCur++ = '\n';
				}
				break;

				case kSqlCsv:
					pchOutCur = strappend(pchOutCur, g_pchSqlCsvPrefix);
					*pchOutCur++ = '2';
					*pchOutCur++ = '0';
					*pchOutCur++ = date[0];
					*pchOutCur++ = date[1];
					*pchOutCur++ = '-';
					*pchOutCur++ = date[2];
					*pchOutCur++ = date[3];
					*pchOutCur++ = '-';
					*pchOutCur++ = date[4];
					*pchOutCur++ = date[5];
					*pchOutCur++ = ' ';
					pchOutCur = strappend(pchOutCur, times);
					*pchOutCur++ = ',';

					pchOutCur = strappend_csvstr(pchOutCur, zone);
					*pchOutCur++ = ',';

					pchOutCur = strappend_csvstr(pchOutCur, ent);
					*pchOutCur++ = ',';

					pchOutCur = strappend(pchOutCur, team);
					*pchOutCur++ = ',';

					*pchOutCur++ = '[';
					pchOutCur = strappend(pchOutCur, tag);
					*pchOutCur++ = ']';
					*pchOutCur++ = ',';

					pchOutCur = strappend_csvstr(pchOutCur, rest);

					pchOutCur = strappend(pchOutCur, g_pchSqlCsvSuffix);

					*pchOutCur++ = '\r';
					*pchOutCur++ = '\n';
				break;

				default:
					pchOutCur = strappend(pchOutCur, date);
					pchOutCur = strappend(pchOutCur, g_pchFieldSep);
					pchOutCur = strappend(pchOutCur, times);
					pchOutCur = strappend(pchOutCur, g_pchFieldSep);
					pchOutCur = strappend(pchOutCur, zone);
					pchOutCur = strappend(pchOutCur, g_pchFieldSep);
					pchOutCur = strappend(pchOutCur, ent);
					pchOutCur = strappend(pchOutCur, g_pchFieldSep);
					pchOutCur = strappend(pchOutCur, team);
					pchOutCur = strappend(pchOutCur, g_pchFieldSep);
					pchOutCur = strappend(pchOutCur, tag);
					pchOutCur = strappend(pchOutCur, g_pchFieldSep);
					pchOutCur = strappend(pchOutCur, rest);
					pchOutCur = strappend(pchOutCur, g_pchRecordSep);
				break;
			}
		}
	}

	fwrite(pchOut, 1, pchOutCur-pchOut, fpOut);

	fclose(fpIn);
	fclose(fpOut);

	free(pchOut);
	free(pchIn);

	time(&clock);
	printf("End: %s", asctime(localtime(&clock)));

	if (errorCount > 0)
	{
		printf("Encountered %d errors.\n", errorCount);
	}

	return 1;
}

/**********************************************************************func*
 * Usage
 *
 */
void Usage(int verbose)
{
	printf("\n");
	printf("LogFilter - %s\n\n", __DATE__);
	printf("Filters Cryptic Studios Logfiles to keep certain tags.\n");
	printf("\n");
	printf("Usage ------------------------------------------------------------\n");
	printf("\n");
	printf("logfilter [-cryptic|-sqlcsv|-record] -rules rulefile infile outfile\n");
	printf("          [-fieldsep sep] [-recordsep sep]\n");
	printf("          [-prefix str] [-suffix str]\n");
	printf("          [-help] [-?]\n");
	if(verbose)
	{
		printf("          [-insize size] [-outsize size]\n");
	}
	printf("\n");
	printf("   -rules rulefile  specifies file with rulset. (See below)\n");
	printf("   -cryptic         outputs the new logfile using the standard Cryptic Studios\n");
	printf("                    format instead of using field and record separators.\n");
	printf("   -sqlcsv          outputs the new logfile in a form that can be used for\n");
	printf("                    MS SQL Bulk import. (default)\n");
	printf("   -record          outputs the new logfile with the given field and record\n");
	printf("                    separators\n");
	printf("   -help            verbose help\n");
	printf("   -?               verbose help\n");
	printf("   infile           name of gzipped file to filter\n");
	printf("   outfile          name of gzipped output file\n");
	printf("\n");
	printf(" For SqlCsv Format:\n");
	printf("   -prefix stuff    this exact string will be prepended to every output line.\n");
	printf("   -suffix stuff    this exact string will be appended to every output line.\n");
	printf("\n");
	printf(" For Record Format:\n");
	printf("   -fieldsep sep    specifies the string to put between fields. (Default |)\n");
	printf("   -fsep sep        same as -fieldsep\n");
	printf("   -recordsep sep   specifies the string to put between records.(Default \\n)\n");
	printf("   -rsep sep        same as -recordsep\n");
	printf("\n");
	printf(" sep may contain \\r\\n to make an end of line.\n");
	printf("\n");
	if(!verbose)
	{
		printf("Use \"logfilter -help\" for verbose help.\n");
	}
	if(verbose)
	{
		printf("Debug ------------------------------------------------------------\n");
		printf("\n");
		printf("   -insize size   specify the size of the input buffer. (%d by default)\n", BUFFER_IN_SIZE);
		printf("   -outsize size  specify the size of the output buffer. (%d by default)\n", BUFFER_OUT_SIZE);
		printf("\n");
		printf("Rule file format -------------------------------------------------\n");
		printf("\n");
		printf("The rule file lists the rules to be applied to a Cryptic Studios logfile\n");
		printf("to create a new logfile (ostensibly with a subset of the data from the\n");
		printf("original).\n");
		printf("\n");
		printf("If the rules system is too complicated for you, you can ignore the advanced \n");
		printf("information below and simply list the exact tags you want in the output\n");
		printf("file.\n");
		printf("\n");
		printf("Example: Only these tags will appear in the output file.\n");
		printf("\n");
		printf("   Power:Hit\n");
		printf("   Power:Activate\n");
		printf("   chat\n");
		printf("\n");
		printf("You can have more control if you use the advanced rules system.\n");
		printf("\n");
		printf("Rules are executed in order from top to bottom. The first rule which\n");
		printf("applied to a line is used to either reject or accept the line from the\n");
		printf("original. If no rule applies to a line, then that line is rejected.\n");
		printf("\n");
		printf("Each rule is set up like this:\n");
		printf("\n");
		printf("   (op)(string to match to tag)[,string to match to rest]\n");
		printf("\n");
		printf("op is either + or -. + means to put the line in the output file. - means to\n");
		printf("   exclude the line from the output file.\n");
		printf("\n");
		printf("(string to match to tag) is the tag you wish to reject or accept with the\n");
		printf("   rule. You can put a * at the end of a tag and it will match all tags \n");
		printf("   that start with the tag you give. For example -- Power:* -- will match \n");
		printf("   Power:Activate, Power:Hit, etc.\n");
		printf("\n");
		printf("[,string to match to rest] is an optional string which is matched against\n");
		printf("   the free-form string following the tag. It may be useful for taking a \n");
		printf("   subset of a particular tag's lines. You may also put a * at the end of\n");
		printf("   this string as a wildcard.\n");
		printf("\n");
		printf("These strings are case insensitive.\n");
		printf("\n");
		printf("Pound signs can be used for comments in this file.\n");
		printf("\n");
		printf("Example: This rejects all chat lines which have \"team_map_relay\" in them.\n");
		printf("         Then it accepts all other chat lines. Lines without the tag \"chat\"\n");
		printf("         are rejected.\n");
		printf("\n");
		printf("   # This is a comment\n");
		printf("   # Ignore chat followed by team_map_relay\n");
		printf("   -chat,team_map_relay*\n");
		printf("\n");
		printf("   # Accept everything else with the chat tag\n");
		printf("   +chat\n");
		printf("\n");
	}
}

/**********************************************************************func*
 * main
 *
 */
int main(int argc, char *argv[])
{
	int i;
	char *pchInFile = NULL;
	char *pchOutFile = NULL;
	char *pchKeepFile = NULL;
	bool bAbort = false;

	for(i=1; i<argc; i++)
	{
		if(argv[i][0]=='-' || argv[i][0]=='/')
		{
			if(stricmp(&argv[i][1], "help")==0 || stricmp(&argv[i][1], "?")==0)
			{
				Usage(1);
				return -1;
			}
			else if(stricmp(&argv[i][1], "insize")==0 && (i+1)<argc)
			{
				i++;
				g_iInSize = atoi(argv[i]);
				if(g_iInSize<MAX_LINE_SIZE)
				{
					g_iInSize = MAX_LINE_SIZE;
					printf("Warning: Insize is too small. Set to %d\n", MAX_LINE_SIZE);
				}
			}
			else if(stricmp(&argv[i][1], "outsize")==0 && (i+1)<argc)
			{
				i++;
				g_iOutSize = atoi(argv[i]);
				if(g_iOutSize<MAX_LINE_SIZE)
				{
					g_iOutSize = MAX_LINE_SIZE;
					printf("Warning: Outsize is too small. Set to %d\n", MAX_LINE_SIZE);
				}
			}
			else if((stricmp(&argv[i][1], "fieldsep")==0
				|| stricmp(&argv[i][1], "fsep")==0) && (i+1)<argc)
			{
				i++;
				g_pchFieldSep = strdup(unescapeString(argv[i]));
			}
			else if((stricmp(&argv[i][1], "recordsep")==0
				|| stricmp(&argv[i][1], "rsep")==0) && (i+1)<argc)
			{
				i++;
				g_pchRecordSep = strdup(unescapeString(argv[i]));
			}
			else if((stricmp(&argv[i][1], "rule")==0 
				|| stricmp(&argv[i][1], "rules")==0) && (i+1)<argc)
			{
				i++;
				pchKeepFile = argv[i];
			}
			else if(stricmp(&argv[i][1], "in")==0 && (i+1)<argc)
			{
				i++;
				pchInFile = argv[i];
			}
			else if(stricmp(&argv[i][1], "out")==0 && (i+1)<argc)
			{
				i++;
				pchOutFile = argv[i];
			}
			else if(stricmp(&argv[i][1], "prefix")==0 && (i+1)<argc)
			{
				i++;
				g_pchSqlCsvPrefix = argv[i];
			}
			else if(stricmp(&argv[i][1], "suffix")==0 && (i+1)<argc)
			{
				i++;
				g_pchSqlCsvSuffix = argv[i];
			}
			else if(stricmp(&argv[i][1], "record")==0)
			{
				g_eFormat = kRecord;
			}
			else if(stricmp(&argv[i][1], "cryptic")==0)
			{
				g_eFormat = kCryptic;
			}
			else if(stricmp(&argv[i][1], "sqlcsv")==0)
			{
				g_eFormat = kSqlCsv;
			}
			else
			{
				printf("Error: Unrecognized command line switch: \"%s\".\n", argv[i]);
				bAbort = true;
			}
		}
		else
		{
			if(!pchInFile)
			{
				pchInFile = argv[i];
			}
			else if(!pchOutFile)
			{
				pchOutFile = argv[i];
			}
			else
			{
				printf("Error: Already have both input and output files: \"%s\"\n", argv[i]);
				bAbort = true;
			}
		}
	}

	if(!pchInFile)
	{
		printf("Error: No input file specified.\n");
		bAbort = true;
	}
	if(!pchOutFile)
	{
		printf("Error: No output file specified.\n");
		bAbort = true;
	}

	if(!pchKeepFile)
	{
		printf("Error: No keep file specified.\n");
		bAbort = true;
	}
	else if(!LoadRuleFile(pchKeepFile))
	{
		bAbort = true;
	}


	if(bAbort)
	{
		Usage(0);
		return -1;
	}

	if(!DoIt(pchInFile, pchOutFile))
	{
		printf("Filtering failed.\n");
		return -1;
	}

	return 0;
}

/* End of File */
