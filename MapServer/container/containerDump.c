#include <string.h>

#include "container/dbcontainerpack.h"
#include "cmdoldparse.h"
#include "timing.h"
#include "utils.h"
#include "float.h"
#include "StringCache.h"
#include <assert.h>
#include "strings_opt.h"
#include "earray.h"
#include "container/container_util.h"
#include "dbcontainer.h"
#include "containerloadsave.h"
#include "comm_backend.h"
#include "dbcomm.h"
#include "file.h"
#include "memcheck.h"

#define MAX_BUF 1000000
#define DESC_STRUCTADDR(desc) ((LineDesc *)(desc->indirection[0].offset))
#define MAX_FILE 100000000

static FILE *dump_file;
static char outname[1024];

// This converts any lines with container references into LISTID:CONTAINERID form
void lineToRefLine(StuffBuff *sb,StructDesc* structDesc, char* table, char* field, char* val, int idx)
{
	LineDesc *desc;
	static char	buf[MAX_BUF];
	int i;

	for(i = 0; structDesc->lineDescs[i].name; i++)
	{
		desc = &structDesc->lineDescs[i];
		if (stricmp(desc->name,table)==0)
		{			
			switch (desc->type)
			{
				xcase PACKTYPE_INT:
				case PACKTYPE_FLOAT:
				{
					addSingleStringToStuffBuff( sb, desc->name);
					addSingleStringToStuffBuff( sb, " ");
					addSingleStringToStuffBuff( sb, val);
					addSingleStringToStuffBuff( sb, "\n");
				}
				xcase PACKTYPE_EARRAY:
				case PACKTYPE_SUB:
				{				
					STR_COMBINE_BEGIN(buf);
					STR_COMBINE_CAT(table);
					STR_COMBINE_CAT("[");
					itoa(idx, c, 10);
					c += strlen(c);
					STR_COMBINE_CAT("].");
					STR_COMBINE_END();
					addSingleStringToStuffBuff( sb, buf);
					lineToRefLine(sb, (StructDesc*)DESC_STRUCTADDR(desc), field, 0, val, idx);
				}
				xcase PACKTYPE_CONREF:
				{
					addSingleStringToStuffBuff( sb, desc->name);
					addSingleStringToStuffBuff( sb, " ");
					addStringToStuffBuff( sb, "CONREF%d:%s", desc->size,val);
					addSingleStringToStuffBuff( sb, "\n");
				}
				xdefault:
				{
					addSingleStringToStuffBuff( sb, desc->name);
					addSingleStringToStuffBuff( sb, " \"");
					addSingleStringToStuffBuff( sb, val);
					addSingleStringToStuffBuff( sb, "\"\n");
				}
			}		
		}
	}
}

// Write the dump to the file, opening a new one if we're past the max size
static void printDump(char *container)
{
	static int writeout = 0;
	static int suffix = 1;
	writeout += fprintf(dump_file,"%s\n",container);

	if (writeout > MAX_FILE)
	{
		char tempfile[1024];

		fflush(dump_file);
		fclose(dump_file);
		sprintf(tempfile,"%s-%d",outname,suffix++);
		dump_file = fopen(tempfile,"wb");

		if (!dump_file)
		{
			printf("Bad output file: %s", tempfile);
			exit(1);
		}
		writeout = 0;
	}
}

static void processContainer(int list_id, int con_id, char *condata)
{
	char * buff = condata;
	char * lastline = buff;
	char table[100],field[200],idxstr[MAX_BUF],*val;
	static StuffBuff sb;
	int idx;

	if (!sb.buff)
		initStuffBuff( &sb, MAX_BUF );
	else
		clearStuffBuff(&sb);

	addStringToStuffBuff(&sb,"CONID%d:%d\n",list_id,con_id);
	while(decodeLine(&buff,table,field,&val,&idx,idxstr))
	{
		StructDesc *desc = getStructDesc(list_id,table);
		if (!desc)
		{
			// For things like badges and inventory without proper descs, just copy it
			char temp = *buff;
			*buff = '\0';
			addStringToStuffBuff(&sb,"%s",lastline);
			*buff = temp;
		}
		else
		{				
			lineToRefLine(&sb,desc,table,field,val,idx);
		}
		lastline = buff;
	}
	printDump(sb.buff);
}


// The callback function that actually dumps the container contents, with references
static int dbDumpContainers(Packet *pak,int cmd,NetLink *link)
{
	int				i;
	ContainerInfo	ci;
	int		list_id,count;

	list_id	= pktGetBitsPack(pak,1);
	count	= pktGetBitsPack(pak,1);

	for(i=0;i<count;i++)
	{
		if (!dbReadContainerUncached(pak,&ci,list_id))
		{
			continue;
		}
		else
		{			
			processContainer(list_id,ci.id,ci.data);
		}
		
	}
	return 1;
}

// Dumps out all the containers we are told to, to a dump file
int dumpContainers(char *fname, int* ids)
{
	int i;
	int timeout = 100000000;
	static PerformanceInfo* perfInfo;
	int list_id = 0;

	strcpy(outname,fname);
	dump_file = fopen(fname,"wb");
	
	if (!dump_file)
	{
		printf("Bad output file: %s", fname);
		return 1;
	}
	fprintf(dump_file,"// Created:%s\n",timerGetTimeString());
	
	if (dbTryConnect(15.0, DEFAULT_DB_PORT)) {

		for (i = 0; i < eaiSize(&ids); i++)
		{
			int container_id = ids[i];
			if (container_id < 0)
			{
				list_id = -container_id;
				continue;
			}
			if (list_id == 0)
			{
				printf("A list id in form -listid must be specified");
				fclose(dump_file);
				return 1;
			}
			if (container_id == 0)
			{
				dbAsyncContainerRequest(list_id,container_id,CONTAINER_CMD_LOAD_ALL,dbDumpContainers);
			}
			else
			{			
				dbAsyncContainerRequest(list_id,container_id,CONTAINER_CMD_TEMPLOAD,dbDumpContainers);				
			}
			if (!dbMessageScanUntilTimeout("CONTAINER_CMD_TEMPLOAD_2", &perfInfo, timeout))
			{
				printf("Dump timed out\n");
				fclose(dump_file);
				return 1;
			}
		}
		
	} else {
		printf("Failed to connect to DbServer");
		fclose(dump_file);
		return 1;
	}

	fclose(dump_file);
	memCheckDumpAllocs();
	return 0;
}

int processDump(char *fnamein, char *fnameout)
{
	char filenamein[1024];
	char basefilenamein[1024];
	int suffix = 1;
	static StuffBuff sb;
	int len;
	char *mem;
	initStuffBuff(&sb,1000);

	if (fnamein[0] == '.' || strstr(fnamein,":")) //do we look like an absolute path?
		strcpy(basefilenamein,fnamein);
	else
		sprintf(basefilenamein,"./%s",fnamein); //to keep filealloc from trying to read it from data
	strcpy(filenamein, basefilenamein);

	strcpy(outname,fnameout);
	dump_file = fopen(outname,"wb");

	if (!dump_file)
	{
		printf("Bad output file: %s", outname);
		return 1;
	}

	fprintf(dump_file,"// Created:%s\n",timerGetTimeString());

	while (mem = fileAlloc(filenamein, &len))
	{	
		char *temp = mem;
		char *line;

		while (line = strsep(&temp,"\n\r"))
		{			
			if (!line[0] || strncmp(line, "//",2) == 0)
			{
				// skip comments and blanks
				continue;
			}
			if (strncmp(line,"CONID",5) == 0)
			{
				int list_id,con_id;
				char conid[512], *tempnum;
				clearStuffBuff(&sb);				
				strncpyt(conid, line+strlen("CONID"),strlen(line) - strlen("CONID") + 1);
				tempnum = strstr(conid,":");
				*tempnum = '\0';
				list_id = atoi(conid);
				con_id = atoi(tempnum+1);
				*tempnum = ':';

				while (line = strsep(&temp,"\n"))
				{
					if (!line[0])
					{
						// if it's blank, this is the end of the container
						break;
					}
					if (strncmp(line, "//",2) == 0)
					{
						// skip comments and blanks
						continue;
					}
					else
					{
						addStringToStuffBuff(&sb,"%s\n",line);
					}
				}
				processContainer(list_id,con_id,sb.buff);
			}
		}		
		fileFree(mem);
		sprintf(filenamein,"%s-%d",basefilenamein,suffix++);	
	}

	fclose(dump_file);	
	return 0;

}



