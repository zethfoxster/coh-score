/***************************************************************************
 *     Copyright (c) 2005-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 *
 * Notes: A hacked up version of the regular logfilter.c that is used to extract auction house
 * info for restoring players who have lost their items.
 *
 ***************************************************************************/

#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "file.h"
#include "utils.h"
#include "earray.h"
#include "StashTable.h"

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


char g_apchs[][256]=
{
	"AuctionClient:Put,xid\r\n",
	"AuctionClient:Change,inf fee\r\n",
	"AuctionClient:Get,xid\r\n"
};


/**********************************************************************func*
 * LoadRuleFile
 *
 */
static int LoadRuleFile(char *pchKeepFile)
{
	int i = 0;
//	FILE *fp;
//	char pch[256];
//
//	fp = fopen(pchKeepFile, "rt");
//	if(!fp)
//	{
//		printf("Error: Unable to open rule file: \"%s\"\n", pchKeepFile);
//		return 0;
//	}

	EArrayCreate(&g_ppchKeeperTags);
	EArrayCreate(&g_ppchKeeperRests);
	EArrayIntCreate(&g_piKeeperLengths);
	EArrayIntCreate(&g_piKeeperRestLengths);


	for(i=0; i<2; i++)
	{
		KeeperOp op;
		char *pch = g_apchs[i];

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
					iLenRest = strlen(pos);
				}

				pchRest = strdup(pos);
			}

			if(pch[strlen(pch)-1] == '*')
			{
				pch[strlen(pch)-1] = '\0';
				iLenTag = strlen(pch);
			}

			if(*pch)
			{
				pchTag = strdup(pch);
			}

			EArrayIntPush(&g_piKeeperLengths, iLenTag);
			EArrayPush(&g_ppchKeeperTags, pchTag);
			EArrayIntPush(&g_piKeeperRestLengths, iLenRest);
			EArrayPush(&g_ppchKeeperRests, pchRest);

			EArrayIntPush(&g_piKeeperOps, op);
		}
	}

	printf("Loaded %d rules.\n", EArrayGetSize(&g_ppchKeeperTags));

//	fclose(fp);
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
		if(strstri(pch, pchTest)!=0)
		{
			return 1;
		}
	}
	else
	{
		char ch = pch[iLen];
		pch[iLen] = '\0';
		if(strstri(pch, pchTest)!=0)
		{
			pch[iLen] = ch;
			return 1;
		}
		pch[iLen] = ch;
	}

	return 0;
}

/**********************************************************************func*
 * IsKeeper
 *
 */
static int IsKeeper(char *pchTag, char *pchRest)
{
	int n = EArrayGetSize(&g_ppchKeeperTags);
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


int	InfProc(void *pv, StashElement element)
{
	char val[1000];
	char **ppchOutCur = (char **)pv;

	sprintf(val, "/csr_offline \"%s\" influence_add %d\r\n", stashElementGetStringKey(element), stashElementGetInt(element));
	*ppchOutCur = strappend(*ppchOutCur, val);

	return 1;
}

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

	int inf = 0;
	int newinf = 0;

	
	StashTable hashInf = stashTableCreateWithStringKeys(1024, StashDeepCopyKeys);

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

	fpOut = fopen(pchOutFile, "wb");
	if(!fpIn)
	{
		printf("Error: Unable to open output file \"%s\"\n", pchOutFile);
		return 0;
	}

	pchOutCur = pchOut;
	pchInEnd = pchInCur = pchIn;
	*pchInEnd = '\0';

	pchOutCur = strappend(pchOutCur, "# From logfile ");
	pchOutCur = strappend(pchOutCur, pchInFile);
	*pchOutCur++ = '\r';
	*pchOutCur++ = '\n';

	while(FillBuff(pchIn, &pchInCur, &pchInEnd, fpIn))
	{
		bool bHadQuotes = false;

		iLine++;

		if(*pchInCur<'0' || *pchInCur>'9')
		{
			if(stricmp(tag,"chat")!=0)
			{
				printf("Error: Malformed log file at line %d. Attempting to recover... (code %d)\n   <%20.20s>\n", iLine, __LINE__, pchInCur);
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

		while(*pos==' ' && *pos)
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

		while(*pos==' ' && *pos)
			pos++;
		CHECK(*pos);

		pchInCur=pos;
		pos = strchr(pchInCur, ' ');
		CHECK(pos);
		strncpyt(team, pchInCur, pos-pchInCur+1);
		pos++;
		pchInCur=pos;

		while(*pos!='[' && *pos)
			pos++;
		CHECK(*pos);
		pos++;
		pchInCur=pos;

		pos = strchr(pchInCur, ']');
		CHECK(pos);
		strncpyt(tag, pchInCur, pos-pchInCur+1);
		pos+=2;
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
//					pchOutCur = strappend(pchOutCur, g_pchSqlCsvPrefix);
//					*pchOutCur++ = '2';
//					*pchOutCur++ = '0';
//					*pchOutCur++ = date[0];
//					*pchOutCur++ = date[1];
//					*pchOutCur++ = '-';
//					*pchOutCur++ = date[2];
//					*pchOutCur++ = date[3];
//					*pchOutCur++ = '-';
//					*pchOutCur++ = date[4];
//					*pchOutCur++ = date[5];
//					*pchOutCur++ = ' ';
//					pchOutCur = strappend(pchOutCur, times);
//					*pchOutCur++ = ',';
//
//					pchOutCur = strappend_csvstr(pchOutCur, zone);
//					*pchOutCur++ = ',';
//
//					pchOutCur = strappend_csvstr(pchOutCur, ent);
//					pchOutCur = strappend(pchOutCur, ent);
//					*pchOutCur++ = '@';

//					pchOutCur = strappend(pchOutCur, team);
//					*pchOutCur++ = ',';

//					*pchOutCur++ = '[';
					if(strstri(tag,"Put")!=0)
					{
						char *pos;

						if(strstri(rest,")Failed."))
						{
							// ignore. failed case.
						}
						else if(pos=strstri(rest,"bid placed. removed "))
						{
							pos+=strlen("bid placed. removed ");
							*strchr(pos, ' ') = '\0';
							newinf = atoi(pos);
							if(stashFindInt(hashInf, ent, &inf))
							{
								newinf += inf;
							}
							stashAddInt(hashInf, ent, newinf, true);
							
						}
						else if(pos=strstri(rest, "Item is '"))
						{
							int type;
							char *pos2;
							char *start;
							int nolevel = 0;
							int cnt;
										
							start = pos;
							pos+=strlen("Item is '");
							type = atoi(pos);
							pos=strchr(pos, ' ');
							pos++;

							switch(type)
							{
								//case 0: //	kTrayItemType_None,
								//case 1: //	kTrayItemType_Power,
								case 2: //	kTrayItemType_Inspiration,
									pchOutCur = strappend(pchOutCur, "/csr_offline \"");
									pchOutCur = strappend(pchOutCur, ent);
									pchOutCur = strappend(pchOutCur, "\" ");

									pchOutCur = strappend(pchOutCur, "inspire ");
									pos=strstri(pos, "Inspirations.");
									pos+=strlen("Inspirations.");
									*(pos2=strchr(pos, '.'))='\0';
									pos2++;
									pchOutCur = strappend(pchOutCur, pos); //set
									pchOutCur = strappend(pchOutCur, " ");
									*(pos=strchr(pos2, '\''))='\0';
									pos++;
									pchOutCur = strappend(pchOutCur, pos2);//power

									*pchOutCur++ = '\r';
									*pchOutCur++ = '\n';

									break;

								//case 3: //	kTrayItemType_BodyItem,
								//case 4: //	kTrayItemType_SpecializationPower,
								case 5: //	kTrayItemType_SpecializationInventory,
									pchOutCur = strappend(pchOutCur, "/csr_offline \"");
									pchOutCur = strappend(pchOutCur, ent);
									pchOutCur = strappend(pchOutCur, "\" ");

									pchOutCur = strappend(pchOutCur, "boost ");
									pos=strstri(pos, "Boosts.");
									pos+=strlen("Boosts.");
									*(pos2=strchr(pos, '.'))='\0';
									pos2++;
									pchOutCur = strappend(pchOutCur, pos); //set
									pchOutCur = strappend(pchOutCur, " ");
									*strchr(pos2, '\'')='\0';
									pos=strchr(pos2, ' ');
									if(!pos)
									{
										nolevel=1;
									}
									else
									{
										*pos='\0';
										pos++;
									}
									pchOutCur = strappend(pchOutCur, pos2);//power
									pchOutCur = strappend(pchOutCur, " ");
									if(nolevel)
									{
										pchOutCur = strappend(pchOutCur, "1");
									}
									else
									{
										int lvl;
										lvl=atoi(pos);
										sprintf(pos, "%d", lvl+1);
										pchOutCur = strappend(pchOutCur, pos);
									}
									*pchOutCur++ = '\r';
									*pchOutCur++ = '\n';
									break;

								//case 6: //	kTrayItemType_Macro,
								//case 7: //	kTrayItemType_RespecPile,
								//case 8: //	kTrayItemType_Tab,
								//case 9: //	kTrayItemType_ConceptInvItem,
								//case 10: //	kTrayItemType_PetCommand,
								case 11: //	kTrayItemType_Salvage,
									*start='\0';
									pos2 = strrchr(start-10, ',');
									pos2++;

									*strchr(pos, '\'')='\0';

									pchOutCur = strappend(pchOutCur, "/csr_offline \"");
									pchOutCur = strappend(pchOutCur, ent);
									pchOutCur = strappend(pchOutCur, "\" ");

									pchOutCur = strappend(pchOutCur, "salvage_grant ");
									pchOutCur = strappend(pchOutCur, pos);
									pchOutCur = strappend(pchOutCur, " ");
									pchOutCur = strappend(pchOutCur, pos2);
									*pchOutCur++ = '\r';
									*pchOutCur++ = '\n';

									break;

								case 12: //	kTrayItemType_Recipe,
									*start='\0';
									pos2 = strrchr(start-10, ',');
									pos2++;
									cnt = atoi(pos2);

									pos2 = strchr(pos, ' ');
									if(!pos2) pos2=strchr(pos, '\'');
									*pos2='\0';

									while(cnt>0)
									{
										pchOutCur = strappend(pchOutCur, "/csr_offline \"");
										pchOutCur = strappend(pchOutCur, ent);
										pchOutCur = strappend(pchOutCur, "\" ");

										pchOutCur = strappend(pchOutCur, "rw_recipe ");
										pchOutCur = strappend(pchOutCur, pos);
										*pchOutCur++ = '\r';
										*pchOutCur++ = '\n';

										cnt--;
									}
									break;

									//case 13: //	kTrayItemType_StoredInspiration,
									//case 14: //	kTrayItemType_StoredEnhancement,
									//case 15: //	kTrayItemType_StoredSalvage,
									//case 16: //	kTrayItemType_StoredRecipe,
									//case 17: //	kTrayItemType_MacroHideName,
									//case 18: //	kTrayItemType_PersonalStorageSalvage,
									
								default:
									pchOutCur = strappend(pchOutCur, "**ERROR** Unknown type: ");
									pchOutCur = strappend(pchOutCur, rest);
									*pchOutCur++ = '\r';
									*pchOutCur++ = '\n';

							}
						}
					}
					else
					{
						if(strstri(rest,")Get Failed."))
						{
							// ignore. failed case.
						}
						else
						{
							pos=strstri(rest, "selling item. ");
							pos+=strlen("selling item. ");
							*strchr(pos, ' ')='\0';
							
							newinf = atoi(pos);
							if(stashFindInt(hashInf, ent, &inf))
							{
								newinf += inf;
							}
							stashAddInt(hashInf, ent, newinf, true);	
						}
					}
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

	stashForEachElementEx(hashInf, InfProc, &pchOutCur);

	fwrite(pchOut, 1, pchOutCur-pchOut, fpOut);

	fclose(fpIn);
	fclose(fpOut);

	free(pchOut);
	free(pchIn);

	time(&clock);
	printf("End: %s", asctime(localtime(&clock)));

	return 1;
}

/**********************************************************************func*
 * Usage
 *
 */
void Usage(int verbose)
{
	printf("\n");
	printf("Logfilter - %s\n\n", __DATE__);
	printf("Filters Cryptic Studios Logfiles and makes a command file for\ngranting auction items to players.\n");
	printf("\n");
	printf("Usage ------------------------------------------------------------\n");
	printf("\n");
	printf("logfilter infile outfile\n");
	if(verbose)
	{
		printf("          [-insize size] [-outsize size]\n");
	}
	printf("\n");
	printf("   -help            verbose help\n");
	printf("   -?               verbose help\n");
	printf("   infile           name of gzipped file to filter\n");
	printf("   outfile          name of output file\n");
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

//	if(!pchKeepFile)
//	{
//		printf("Error: No keep file specified.\n");
//		bAbort = true;
//	}
	if(!LoadRuleFile(pchKeepFile))
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
