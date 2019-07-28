#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "stdtypes.h"
//
#include "network\netio.h"
#include "netcomp.h"
#include "error.h"
#include "utils.h"
#include "strings_opt.h"
#include "file.h"
//
#include "language/MessageStore.h"
#include "textparser.h"
#include "StashTable.h"
//
#include "language/MessageStoreUtil.h"
//
#include "earray.h"
#include "DebugState.h"
#include "cmdoldparse.h"

// These functions are for storing variables to the registry for use in debugging

typedef struct CmdVarTable {
	char *var[10];
} CmdVarTable;
static TokenizerParseInfo CmdVarTableInfo[] = {
#define ENTRY(i) { "var"#i,			TOK_STRING(CmdVarTable, var[i], 0)},
	ENTRY(0)ENTRY(1)ENTRY(2)ENTRY(3)ENTRY(4)ENTRY(5)ENTRY(6)ENTRY(7)ENTRY(8)ENTRY(9)
	{ "", 0, 0 }
};
CmdVarTable cmd_var_table;

char *cmdOldVarsLookup(unsigned char *s)
{
	if (s[0]=='%' && isdigit(s[1]) && s[2]=='\0') {
		ParserLoadFromRegistry("COH", CmdVarTableInfo, &cmd_var_table);
		if (cmd_var_table.var[s[1]-'0'] && cmd_var_table.var[s[1]-'0'][0])
			return cmd_var_table.var[s[1]-'0'];
	}
	return s;
}

void cmdOldVarsAdd(int i, char *s)
{
	if (cmd_var_table.var[i]) {
		ParserFreeString(cmd_var_table.var[i]);
		cmd_var_table.var[i] = NULL;
	}
	if (s)
		cmd_var_table.var[i] = ParserAllocString(s);
	ParserSaveToRegistry("COH", CmdVarTableInfo, &cmd_var_table, 0, 0);
}

// Shortcuts associated a certain cmd name with a number, and are used to send 
// user visible commands back and forth

typedef struct
{
	char name[32];
	int	idx;
} CmdShortcut;

CmdShortcut *shortcuts;
int			shortcut_count,shortcut_max;

void cmdOldAddShortcut(char *name)
{
	CmdShortcut	*sc = dynArrayAdd(&shortcuts,sizeof(shortcuts[0]),&shortcut_count,&shortcut_max,1);
	Strncpyt(sc->name,name);
	sc->idx = shortcut_count;
}

void cmdOldSetShortcut(char *name,int idx)
{
	CmdShortcut	*sc = dynArrayFit(&shortcuts,sizeof(shortcuts[0]),&shortcut_max,idx-1);
	Strncpyt(sc->name,name);
	sc->idx = idx;
	if (idx > shortcut_count)
		shortcut_count = idx;
}

char *cmdOldShortcutName(int idx)
{
	if (idx < 1 || idx > shortcut_count)
		return 0;
	return shortcuts[idx-1].name;
}

int cmdOldShortcutIdx(char *name)
{
	int		i;

	for(i=0;i<shortcut_count;i++)
	{
		if (striucmp(shortcuts[i].name,name)==0)
			return shortcuts[i].idx;
	}
	return -1;
}

// Data swaps are used to change the base location of which parameter to modify, on the server
void cmdOldAddDataSwap(CmdContext *con, void *src, int size, void *dest)
{
	CmdDataSwap *swap;
	int i;

	for (i = 0; i < con->numDataSwaps; i++)
	{
		CmdDataSwap *temp = &con->data_swap[i];
		if (src == temp->source)
		{
			temp->size = size;
			temp->dest = dest;
			return;
		}
	}

	assert(con->numDataSwaps < CMDMAXDATASWAPS); // make sure haven't reached end of fixed array
	swap = &con->data_swap[con->numDataSwaps++];
	swap->source = src;
	swap->size = size;
	swap->dest = dest;
}

void cmdOldClearDataSwap(CmdContext *con)
{
	con->numDataSwaps = 0; // we rely on this to ignore stale data left in data_swap entries
}

// Check to see if this is covered by any data swaps, and if it is, do the swap
void *doDataSwap(CmdContext *con, void *iptr)
{
	int i;
	char *ptr = (char *)iptr;
	if (!con || !con->numDataSwaps) 
		return ptr;
	for (i = 0; i < con->numDataSwaps; i++)
	{
		CmdDataSwap *temp = &con->data_swap[i];
		if (ptr > (char *)temp->source && ptr < (char *)temp->source + temp->size)
		{
			ptr = (char *)temp->dest + (ptr - (char *)temp->source);
			return ptr;
		}
	}
	return ptr;
}

void cmdOldPrint(Cmd *cmd,char *buf,int buf_size,Cmd *cmds, CmdContext *cmd_context)
{
	char	buf2[4000];
	int		j,idx;
	void	*ptr;
	F32		*fptr;

	sprintf_s(buf, buf_size, "%s ",cmd->name);
	for(j=0,idx=0;idx<cmd->num_args;j++)
	{
		buf2[0] = 0;
		ptr = doDataSwap(cmd_context,cmd->data[j].ptr);
		switch(cmd->data[j].type)
		{
		case PARSETYPE_S32:
			sprintf_s(SAFESTR(buf2)," %d",*((S32 *)ptr));
			idx++;
		xcase PARSETYPE_U32:
			sprintf_s(SAFESTR(buf2)," %u",*((U32 *)ptr));
			idx++;
		xcase PARSETYPE_FLOAT:
		case PARSETYPE_EULER:
			sprintf_s(SAFESTR(buf2)," %f",*((F32 *)ptr));
			idx++;
		xcase PARSETYPE_SENTENCE:
		case PARSETYPE_STR:
			sprintf_s(SAFESTR(buf2)," \"%s\"",(char *)ptr);
			idx++;
		xcase PARSETYPE_VEC3:
			fptr = ptr;
			sprintf_s(SAFESTR(buf2)," %f %f %f",fptr[0],fptr[1],fptr[2]);
			idx += 3;
		xcase PARSETYPE_MAT4:
			fptr = ptr;
			sprintf_s(SAFESTR(buf2)," %f %f %f, %f %f %f, %f %f %f, %f %f %f",fptr[9],fptr[10],fptr[11],fptr[0],fptr[1],fptr[2],fptr[3],fptr[4],fptr[5],fptr[6],fptr[7],fptr[8]);
			idx += 12;
		}
		strcat_s(buf,buf_size,buf2);
	}
}

Cmd *cmdOldFind(Cmd *cmds,char *name)
{
	int		i;

	for(i=0;cmds[i].name;i++)
	{
		if (striucmp(cmds[i].name,name)==0)
			return &cmds[i];
	}
	return 0;
}

int cmdOldCount(Cmd *cmds)
{
	int		i;

	for(i=0;cmds[i].name;i++)
		;
	return i;
}

#define MAXNAME 128

void cmdOldListAddToHashes(CmdList *cmdlist, int index)
{
	int j;
	char	fullname[MAXNAME];
	CmdGroup *list = cmdlist->groups;
	Cmd *cmds = list[index].cmds;
	for(j=0;cmds[j].name;j++)
	{
		if (list[index].header[0])
		{
			sprintf_s(SAFESTR(fullname),"%s.%s",list[index].header,cmds[j].name);
		}
		else
		{
			sprintf_s(SAFESTR(fullname),"%s",cmds[j].name);
		}
		stashAddInt(cmdlist->name_hashes,stripUnderscores(fullname),(index << 16) + j, false);
	}
}

Cmd *cmdOldListFind(CmdList *cmdlist, const char *name, const char *header, CmdHandler *handler)
{
	int		i,j,idx;
	Cmd		*cmd,*cmds;
	CmdGroup *list = cmdlist->groups;
	char	fullname[MAXNAME];

	if(!name || strlen(name) >= ARRAY_SIZE(fullname))
		return NULL;
	
	if (!cmdlist->name_hashes)
	{
		cmdlist->name_hashes = stashTableCreateWithStringKeys(1024,StashDeepCopyKeys);
		for(i=0;list[i].cmds;i++)
		{
			cmdOldListAddToHashes(cmdlist, i);
		}
	}

	if (header && header[0])
	{
		sprintf_s(SAFESTR(fullname),"%s.%s",header,name);
	}
	else
	{
		sprintf_s(SAFESTR(fullname),"%s",name);
	}

#if !TEST_CLIENT
	if(!stashFindInt(cmdlist->name_hashes,stripUnderscores(cmdTranslated(fullname)),&idx)) // cmdTranslate will attempt to get a foreign command name
	{																				 // if it fails, it will return the english string so we can check that too
		return 0;
	}
#else
	if(!stashFindInt(cmdlist->name_hashes,stripUnderscores(fullname),&idx)) 
	{
		return 0;
	}
#endif

	i = idx >> 16;
	j = idx & ((1<<16)-1);
	cmds = list[i].cmds;

	if (handler)
	{
		*handler = list[i].handler;
		if (!*handler)
		{
			*handler = cmdlist->handler;
		}
	}

	cmd = &cmds[j];

	return cmd;
}

Cmd *cmdOldReadInternal(CmdList *cmd_list, const char *cmdstr,CmdContext* cmd_context, const char * header, bool no_exec, bool syntax_check)
{
static char *startdelim_quoted = " \n,\t",
			*enddelim_quoted = " \n,\t",
			*startdelim_sentence = " \n",
			*enddelim_sentence = "\n";
char	*startdelim,*enddelim,*argv[128],*s,buf[CMDMAXLENGTH], *tokbuf;
int		j,argc=0,toggle=0,idx=1,k,invert=0;
void	*ptr;
F32		*fptr;
Cmd		*cmd;
int access_level;
CmdHandler handler = NULL;

	if (cmd_context)
	{
		cmd_context->op = CMD_OPERATION_NONE;
		cmd_context->msg[0] = 0;
		access_level = cmd_context->access_level;
	}
	else
	{
		access_level = 0;
	}

	strncpy_s(buf,sizeof(buf),cmdstr,_TRUNCATE);
	buf[sizeof(buf)-1] = 0;
	s = strtok_quoted_r(buf,startdelim_quoted, enddelim_quoted,&tokbuf);
	if (!s)
		return 0;
	argv[argc++] = s;
	if (header && (header[0] == '+' || header[0] == '-'))
	{
		if (header[1] == '+')
			toggle = 1;

		argv[argc++] = (header[0] == '+') ? "1" : "0";
		header += 1 + toggle;
	}
	else if (header && header[0] == '&')
	{
		cmd_context->op = CMD_OPERATION_AND;
		header += 1;
	}
	else if(header && header[0] == '|')
	{
		cmd_context->op = CMD_OPERATION_OR;
		header += 1;
	}
	else if (argv[0][0] == '+' || argv[0][0] == '-')
	{
		if (argv[0][1] == '+')
			toggle = 1;

		argv[argc++] = (argv[0][0] == '+') ? "1" : "0";
		argv[0] += 1 + toggle;
	}
	else if(argv[0][0] == '&')
	{
		cmd_context->op = CMD_OPERATION_AND;
		argv[0] += 1;
	}
	else if(argv[0][0] == '|')
	{
		cmd_context->op = CMD_OPERATION_OR;
		argv[0] += 1;
	}
	cmd = cmdOldListFind(cmd_list,argv[0],header,&handler);
	if (!cmd || cmd->access_level > access_level)
	{
		if (cmd_context)
			strcpy_s(SAFESTR(cmd_context->msg), textStd("UnknownCommandString", cmdstr));
		return 0;
	}
	cmd->usage_count++;
	if (cmd_context) {
		cmd_context->found_cmd = true;
	}
	cmd->send = 1;
	for(;;)
	{
		j = argc-1;
		startdelim = startdelim_quoted;
		enddelim = enddelim_quoted;
		if (j < cmd->num_args && cmd->data[j].type == PARSETYPE_SENTENCE)
		{
			int len;
			s=strtok_delims_r(0,startdelim_sentence,enddelim_sentence,&tokbuf);
			if (s)
			{			
				len = (int)strlen(s);
				while (s[len - 1] == ' ')
				{
					// ignore trailing whitespace
					len--;
				}
				if (s[0] == '"' && s[len - 1] == '"')
				{
					// chop off the outer quotes
					s[len-1] = '\0';
					s++;
				}
			}
		}
		else if (j < cmd->num_args && cmd->data[j].type == PARSETYPE_TEXTPARSER)
		{
			char *tempstr;
			s = tokbuf;
			tempstr = strstr(s,"&>");
			if (!tempstr)
				s = NULL;
			tempstr++;tempstr++;
			*tempstr = 0;
			tokbuf = tempstr+1;
		}
		else
		{
			s=strtok_quoted_r(0,startdelim,enddelim,&tokbuf);
		}
		
		if (!s)
			break;
		if (strcmp(s,"=") == 0)
			continue;
		if (argc==ARRAY_SIZE(argv))
			break;
		argv[argc++] = s;
	}
	if (argc-1 != cmd->num_args)
	{
		if (argc == 1 && (!(cmd->flags & CMDF_HIDEVARS)))
		{
			if (cmd_context && cmd->data[0].ptr && cmd->data[0].type != PARSETYPE_TEXTPARSER)
			{
				// Print out the parameter, if it's the right kind of command
				cmdOldPrint(cmd,buf,10000,0,0);
				Strncpyt(cmd_context->msg,buf);
			}
		}
		else
		{
			if (cmd_context && cmd->name[0] && !(cmd->flags & CMDF_RETURNONERROR))
			{
				int		j;

				msPrintf(menuMessages, SAFESTR(cmd_context->msg), "IncorrectUsage", argv[0],cmd->num_args,argc-1,cmd->comment);
				strcat(cmd_context->msg,"\n ");	
				//Add a space before the command to prevent localtime (and poss. others) from being translated, 
				//  needing an argument, and then crashing the client.
				strcat(cmd_context->msg,argv[0]);
				for(j=0;j<cmd->num_args;j++)
				{
					switch(cmd->data[j].type)
					{
						case PARSETYPE_S32:
							strcat(cmd_context->msg," <int>");
						xcase PARSETYPE_U32:
							strcat(cmd_context->msg," <unsignedint>");
						xcase PARSETYPE_FLOAT:
						case PARSETYPE_EULER:
							strcat(cmd_context->msg," <float>");
						xcase PARSETYPE_SENTENCE:
						case PARSETYPE_STR:
							strcat(cmd_context->msg," <string>");
						xcase PARSETYPE_VEC3:
							strcat(cmd_context->msg," <vec3>");
						xcase PARSETYPE_MAT4:
							strcat(cmd_context->msg," <mat4>");
						xcase PARSETYPE_TEXTPARSER:
							strcat(cmd_context->msg," <tpblock>");
					}
				}
			}
		}
		if (cmd->flags & CMDF_RETURNONERROR)
			return cmd;
		else
			return 0;
	}
	if (syntax_check)
	{
		return cmd;
	}
	for(j=0,idx=1;idx<=cmd->num_args;j++)
	{
		ptr = doDataSwap(cmd_context,cmd->data[j].ptr);
		switch(cmd->data[j].type)
		{
			case PARSETYPE_S32:
				if(argc && argv[1][0] == '!')
				{
					invert = 1;
				}
				
				if (toggle)
				{
					int		t;
					if (ptr) {					
						t = *((S32 *)ptr);
						if (cmd->data[j].maxval)
						{
							if (++t > cmd->data[j].maxval)
								t = 0;
						}
						else
						{
							t = !t;
						}



						*((S32 *)ptr) = t;

						idx++;
						if (cmd_context && !no_exec)
						{					
							cmd_context->args[j].iArg = t;
						}
					}
				}
				else
				{
					char* str = argv[idx++];
					int value;
					
					if(str[0] == '!'){
						str++;
						invert = 1;
					}
						
					value = atoi(str);

					if(invert)
						value = ~value;
						
					if(cmd_context && cmd_context->op && ptr)
					{
						cmd_context->orig_value = *((S32 *)ptr);

						// Apply the operation if the flags say to.
						
						if(cmd->flags & CMDF_APPLYOPERATION){
							switch(cmd_context->op){
								case CMD_OPERATION_AND:
									value &= *((S32 *)ptr);
									break;
								case CMD_OPERATION_OR:
									value |= *((S32 *)ptr);
									break;
							}
						}
					}

					if (ptr)
					{					
						*((S32 *)ptr) = value;
					}
					if (cmd_context && !no_exec)
					{					
						cmd_context->args[j].iArg = value;
					}
				}
			xcase PARSETYPE_U32:
				if(argc && argv[1][0] == '!')
				{
					invert = 1;
				}

				if (toggle)
				{
					int		t;
					if (ptr)
					{					
						t = *((U32 *)ptr);
						if (cmd->data[j].maxval)
						{
							if (++t > cmd->data[j].maxval)
								t = 0;
						}
						else
						{
							t = !t;
						}
					
						*((U32 *)ptr) = t;
						idx++;
						if (cmd_context && !no_exec)
						{					
							cmd_context->args[j].iArg = t;
						}
					}
				}
				else
				{
					char* str = argv[idx++];
					int value;

					if(str[0] == '!'){
						str++;
						invert = 1;
					}

					value = atoi(str);

					if(invert)
						value = ~value;

					if(cmd_context && cmd_context->op && ptr)
					{
						cmd_context->orig_value = *((U32 *)ptr);

						// Apply the operation if the flags say to.

						if(cmd->flags & CMDF_APPLYOPERATION){
							switch(cmd_context->op){
								case CMD_OPERATION_AND:
									value &= *((U32 *)ptr);
									break;
								case CMD_OPERATION_OR:
									value |= *((U32 *)ptr);
									break;
							}
						}
					}

					if (ptr)
					{					
						*((U32 *)ptr) = value;
					}
					if (cmd_context && !no_exec)
					{					
						cmd_context->args[j].uiArg = value;
					}
				}
			xcase PARSETYPE_FLOAT:
			case PARSETYPE_EULER:
				if (toggle)
				{
					F32 t;
					if (ptr)
					{					
						t = *((F32 *)ptr);
						if (cmd->data[j].maxval)
						{
							if (++t > cmd->data[j].maxval)
								t = 0;
						}
						else if (t)
						{
							t = 0.0f;
						}
						else
						{
							t = 1.0f;
						}				
						*((F32 *)ptr) = t;
						idx++;
						if (cmd_context && !no_exec)
						{					
							cmd_context->args[j].fArg = t;
						}
					}
				}
				else
				{
					F32 t = (F32)atof(argv[idx++]);

					if (ptr)
					{					
						*((F32 *)ptr) = t;
					}
					if (cmd_context && !no_exec)
					{					
						cmd_context->args[j].fArg = t;
					}
				}
			xcase PARSETYPE_STR:
			case PARSETYPE_SENTENCE:
				{
					char *str = (char *)ptr;
					if (!str)
					{
						str = _alloca(cmd->data[j].maxval);
//							ParserAllocStruct(cmd->data[j].maxval);	
					}
					strncpy_s(str,cmd->data[j].maxval,cmdOldVarsLookup(argv[idx]),_TRUNCATE);
					str[cmd->data[j].maxval-1] = 0;
					idx++;
					if (cmd_context && !no_exec)
					{					
						cmd_context->args[j].pArg = str;
					}
				}
			xcase PARSETYPE_TEXTPARSER:			
				{	
					void	*struct_ptr;
					char *argtemp = argv[idx];

					if (!ptr || !cmd_context)
					{
						idx++;
						break;
					}

					struct_ptr = ParserAllocStruct(cmd->data[j].maxval);
					
					ParserExtractFromText(&argtemp,cmd->data[j].ptr,struct_ptr);

					idx++;
					if (cmd_context && !no_exec)
					{					
						cmd_context->args[j].pArg = struct_ptr;
					}
				}
			xcase PARSETYPE_VEC3:
				fptr = ptr;
				if (!fptr)
				{
					fptr = _alloca(sizeof(F32)*3);
//						ParserAllocStruct(sizeof(Vec3));
				}
				for(k=0;k<3;k++)
					fptr[k] = (F32)atof(argv[idx++]);
				if (cmd_context && !no_exec)
				{					
					cmd_context->args[j].pArg = fptr;
				}
			xcase PARSETYPE_MAT4:
				fptr = ptr;
				if (!fptr)
				{
					fptr = _alloca(sizeof(F32)*12 ); 
						//ParserAllocStruct(sizeof(Vec3)*4);
				}
				for(k=0;k<3;k++)
					fptr[k+9] = (F32)atof(argv[idx++]);
				for(k=0;k<9;k++)
					fptr[k] = (F32)atof(argv[idx++]);
				if (cmd_context && !no_exec)
				{					
					cmd_context->args[j].pArg = fptr;
				}
		}
	}
	cmd_context->cmdstr = cmdstr;
	if (cmd->handler && !no_exec)
	{
		// If there's a function pointer handler, run it now
		cmd->handler(cmd,cmd_context);
	}
	else if (handler && !no_exec)
	{
		// If this table has a global handler, run it now
		handler(cmd,cmd_context);
	}
	cmd_context->cmdstr = NULL;

	cmdOldCleanup(cmd,cmd_context);

	return cmd;
}

// This is called by the old command handlers
Cmd *cmdOldRead(CmdList *cmd_list, const char *cmdstr,CmdContext* cmd_context)
{
	CmdContext tempcon = {0};
	if (!cmd_context)
		cmd_context = &tempcon;
	return cmdOldReadInternal(cmd_list,cmdstr,cmd_context,NULL,1,0);
}

// Clear out anything allocated by cmdRead
void cmdOldCleanup(Cmd *cmd, CmdContext* cmd_context) 
{
	int j;
	void *ptr;
	if (!cmd)
		return;
	for(j=0;j<cmd->num_args;j++)
	{
		ptr = cmd_context->args[j].pArg;
		switch(cmd->data[j].type)
		{
		// These are allocated with alloca for now
/*		xcase PARSETYPE_STR:
		case PARSETYPE_SENTENCE:
		case PARSETYPE_VEC3:
		case PARSETYPE_MAT4:
			{
				char *str = (char *)ptr;
				if (ptr && ptr != cmd->data[j].ptr) // If we were allocated by read
				{ 
					ParserFreeStruct(str);
				}
			}*/
		case PARSETYPE_TEXTPARSER:			
			{	
				if (!ptr || !cmd_context || !cmd->data[j].ptr)
				{					
					break;
				}

				ParserDestroyStruct(cmd->data[j].ptr,ptr);
				ParserFreeStruct(ptr);
			}
		}
	}
}

// Returns 1 if the command was executed. 0 if there was an error. context->found_cmd will
// be true if a command was found, and just failed to execute.
int cmdOldExecute(CmdList *cmdlist, const char* str, CmdContext *context)
{
	int period = -1, space = -1;
	int i;
	char header[CMDMODULELEN];
	Cmd *cmd;

	context->found_cmd = false;

	for (i = 0; str[i]; i++)
	{
		if (str[i] == '.' && period < 0)
		{
			period = i;
			break;
		}
		if ((str[i] == '\n' || str[i] == ',' || str[i] == '\t' || str[i] == ' ') && space < 0)
		{
			space = i;
			break;
		}
	}
	if (period > 0) //we found a header
	{
		strncpy_s(header,CMDMODULELEN,str,period);
		header[period] = '\0';
		str += (period + 1); //chop off the header
	}
	else
	{
		header[0] = '\0';
	}

	cmd = cmdOldReadInternal(cmdlist,str,context,header,0,0);

	if (context->msg[0] || !cmd)
	{	
		return 0;
	}

	return 1;
	
}

// Does most of what cmdExecute does, but just checks to see if it is valid
int cmdOldCheckSyntax(CmdList *cmdlist, const char* str, CmdContext *context )
{
	int period = -1, space = -1;
	int i;
	char header[CMDMODULELEN];
	Cmd *cmd;

	context->found_cmd = false;

	for (i = 0; str[i]; i++)
	{
		if (str[i] == '.' && period < 0)
		{
			period = i;
			break;
		}
		if ((str[i] == '\n' || str[i] == ',' || str[i] == '\t' || str[i] == ' ') && space < 0)
		{
			space = i;
			break;
		}
	}
	if (period > 0) //we found a header
	{
		strncpy_s(header,CMDMODULELEN,str,period);
		header[period] = '\0';
		str += (period + 1); //chop off the header
	}
	else
	{
		header[0] = '\0';
	}

	cmd = cmdOldReadInternal(cmdlist,str,context,header,1,1); //only do a syntax check

	if (context->msg[0] || !cmd)
	{	
		return 0;
	}

	return 1;

}

// These functions are used for the old-style reflecting of status data between the server
// and client
int cmdOldSendPacked(Packet *pak,Cmd *cmd,Cmd *cmds,CmdContext *cmd_context)
{
int		j,k,idx;
void	*ptr;
F32		*fptr;

	idx = cmdOldShortcutIdx(cmd->name);
	if (idx < 0)
		return 0;

	pktSendBitsPack(pak,1,idx);

	for(j=0;j<cmd->num_args;j++)
	{
		ptr = doDataSwap(cmd_context,cmd->data[j].ptr);
		switch(cmd->data[j].type)
		{
			case PARSETYPE_S32:
			case PARSETYPE_U32:
				pktSendBitsPack(pak,1,*((int *)ptr));
			xcase PARSETYPE_FLOAT:
				pktSendF32(pak,*((F32 *)ptr));
			xcase PARSETYPE_EULER:
				pktSendBits(pak,14,packEuler(*((F32 *)ptr),14));
			xcase PARSETYPE_STR:
			case PARSETYPE_SENTENCE:
				pktSendString(pak,(char *)ptr);
			xcase PARSETYPE_VEC3:
				fptr = ptr;
				for(k=0;k<3;k++)
					pktSendF32(pak,fptr[k]);
		}
	}
	
	return 1;
}

int cmdOldSend(Packet *pak,Cmd *cmds,CmdContext *cmd_context,int all)
{
int		i,need_reliable = 0;

	for(i=0;cmds[i].name;i++)
	{
		if (all || cmds[i].send)
		{
			need_reliable |= cmdOldSendPacked(pak,&cmds[i],cmds,cmd_context);
		}
	}
	pktSendBitsPack(pak,1,0);
	return need_reliable;
}

static int cmdOldReceivePacked(Packet *pak,CmdList *cmd_list,CmdContext *cmd_context)
{
	int		j,k,idx,update=0;
	void	*ptr;
	F32		*fptr,fval;
	char	*s,*sval;
	Cmd		*cmd;
	U32		ival;
	int		access_level;

	if (cmd_context)
	{
		access_level = cmd_context->access_level;
	}
	else
	{
		access_level = 0;
	}

	//printf("start pos: %d\n", pak->stream.cursor.byte);

	idx = pktGetBitsPack(pak,1);
	s = cmdOldShortcutName(idx);
	
	if(!s)
		return 0;

	cmd = cmdOldListFind(cmd_list,s,NULL,NULL);

	//printf("%d: got packet size %d(%d bytes, cur=%d) - %s\n", 0, pak->id, pak->stream.size, pak->stream.cursor.byte, cmd->name);

	update = 1;

	if (cmd->access_level > access_level)
		update = 0;
	for(j=0;j<cmd->num_args;j++)
	{
		ptr = doDataSwap(cmd_context,cmd->data[j].ptr);
		switch(cmd->data[j].type)
		{
			case PARSETYPE_S32:
			case PARSETYPE_U32:
				ival = pktGetBitsPack(pak,1);
				if (update)
					*((int *)ptr) = ival;
			xcase PARSETYPE_EULER:
				fval = unpackEuler(pktGetBits(pak,14),14);
				if (update)
					*((F32 *)ptr) = fval;
			xcase PARSETYPE_FLOAT:
				fval = pktGetF32(pak);
				if (update)
					*((F32 *)ptr) = fval;
			xcase PARSETYPE_STR:
			case PARSETYPE_SENTENCE:
				sval = pktGetString(pak);
				if (update)
				{
					strncpy_s((char *)ptr,cmd->data[j].maxval, cmdOldVarsLookup(sval),cmd->data[j].maxval-1);
					((char *)ptr)[cmd->data[j].maxval-1] = 0;
				}
			xcase PARSETYPE_VEC3:
				fptr = ptr;
				for(k=0;k<3;k++)
				{
					fval = pktGetF32(pak);
					if (update)
						fptr[k] = fval;
				}
		}
	}
	return 1;
}

void cmdOldResetPacketIds(CmdList *cmd_list)
{

}

void cmdOldReceive(Packet *pak,CmdList *cmd_list,CmdContext *cmd_context)
{
int		idx;

	for(;;)
	{
		idx = cmdOldReceivePacked(pak,cmd_list,cmd_context);
		if (!idx)
			break;
	}
}

void cmdOldSetSends(Cmd *cmds,int val)
{
int		i;

	for(i=0;cmds[i].name;i++)
		cmds[i].send = val;
}

void cmdOldListInit(CmdList *list)
{
	int i;
	for (i = 0; list->groups[i].cmds; i++)
	{
		cmdOldInit(list->groups[i].cmds);
	}
}

void cmdOldInit(Cmd *cmds)
{
int		i,j,count;

	for(i=0;cmds[i].name;i++)
	{
		count = 0;
		for(j=0;cmds[i].data[j].type && j<ARRAY_SIZE(cmds[0].data);j++)
		{
			if (cmds[i].data[j].type == PARSETYPE_STR || cmds[i].data[j].type == PARSETYPE_SENTENCE)
			{
				assertmsg(cmds[i].data[j].maxval,"strings must use CMDSTR or CMDSENTENCE now");
			}
			if (cmds[i].data[j].type == PARSETYPE_MAT4)
				count += 12;
			else if (cmds[i].data[j].type == PARSETYPE_VEC3)
				count += 3;
			else
				count++;
		}
		cmds[i].num_args = count;
	}
}

void cmdOldSaveMessageStore( CmdList *cmdlist, char * filename )
{
	FILE *file;
	char filepath[MAX_PATH];
	int i,j, level = 0;

	if(isProductionMode())
		return;

	sprintf_s(SAFESTR(filepath), "texts\\English\\%s", filename );

	file = fileOpen( filepath, "wt" );
	if (!file)
		return;

	fprintf( file, "%s",	"#	This file contains translations for in-game commands and their help text\n"
							"#\n"
							"#	General file format:\n"
							"#		\"TranslatedLaunguageCommand\", \"EnglishCommand\", \"Help Text Message\"\n"
							"#\n"
							"#	TranslatedLanguageCommand is the translated command, it CAN be the same as the english command.\n"
							"#	EnglishCommand is the hardcoded command that will always work. DO NOT CHANGE THIS!\n"
							"#	Help Text Message is a brief description of the command.\n"
							"#\n"
							"#	Comments:\n"
							"#		Comment lines must begin with a '#' character.\n"
							"#		Comments must begin at the start of the line. No trailing comments.\n"
							"#		Do *NOT* comment out a ID + message pair if it is going to be replaced\n"
							"#		by a pair with an identical ID.\n"
			);
	
	for( level = 0; level < 9; level++ )
	{
		fprintf( file, "\n\n# Access Level %d\n#------------------------------\n\n", level );
  		for( i=0;cmdlist->groups[i].cmds;i++)
		{
			for(j=0;cmdlist->groups[i].cmds[j].name;j++)
			{
				Cmd *cmd = &cmdlist->groups[i].cmds[j];
				char * comment = cmd->comment; // comment is for debugging purposes
				char * emptystring = "";

				UNUSED(comment);

  				if(cmd->access_level != level || cmd->access_level >= 9) // leave debug stuff alone
					continue;

				if( cmd->name[0] == 0 )
					continue;

				if( !cmd->comment )
					cmd->comment = emptystring;

     			else if( !strchr(cmd->comment, '\n') )
				{
 					if( strlen(cmd->name) >= 22 )
						fprintf( file, "\"%s\"\t\"%s\"\t\"%s\"\n", cmd->name, cmd->name, cmd->comment );
					else if( strlen(cmd->name) >= 14 )
						fprintf( file, "\"%s\"\t\t\"%s\"\t\t\"%s\"\n", cmd->name, cmd->name, cmd->comment );
					else if ( strlen(cmd->name) >= 6 )
						fprintf( file, "\"%s\"\t\t\t\"%s\"\t\t\t\"%s\"\n", cmd->name, cmd->name, cmd->comment );
					else
						fprintf( file, "\"%s\"\t\t\t\t\"%s\"\t\t\t\t\"%s\"\n", cmd->name, cmd->name, cmd->comment );
				}
				else
				{
					char * dup = strdup(cmd->comment);
 					strchrReplace(dup, '<', '[' );
					strchrReplace(dup, '>', ']' );

					if( strlen(cmd->name) >= 22 )
						fprintf( file, "\"%s\"\t\"%s\"\t<<%s>>\n", cmd->name, cmd->name, strchrInsert(dup, "\t\t\t\t\t\t\t\t", '\n' )  );
					else if( strlen(cmd->name) >= 14 )
						fprintf( file, "\"%s\"\t\t\"%s\"\t\t<<%s>>\n", cmd->name, cmd->name, strchrInsert(dup, "\t\t\t\t\t\t\t\t", '\n' ) );
					else if ( strlen(cmd->name) >= 6 )
						fprintf( file, "\"%s\"\t\t\t\"%s\"\t\t\t<<%s>>\n", cmd->name, cmd->name, strchrInsert(dup, "\t\t\t\t\t\t\t\t", '\n' ) );
					else
						fprintf( file, "\"%s\"\t\t\t\t\"%s\"\t\t\t\t<<%s>>\n", cmd->name, cmd->name, strchrInsert(dup, "\t\t\t\t\t\t\t\t", '\n' )  );
				}
			}
		}
	}
	fclose(file);
}

void cmdOldApplyOperation(int* var, int value, CmdContext* cmd_context)
{
	switch(cmd_context->op)
	{
		case CMD_OPERATION_AND:
			*var &= value;
			break;
		case CMD_OPERATION_OR:
			*var |= value;
			break;
		default:
			*var = value;
			break;
	}
}

static int dummyGlobCmdParseFunc(const char *cmd)
{
	CmdContext cmd_context = {0};
	cmd_context.access_level = 10;
	cmdOldExecute(&globCmdOldList, cmd, &cmd_context);
	if (cmd_context.msg[0]) {
		printf("%s\n", cmd_context.msg);
	}
	return 1;
}

CmdParseFunc globCmdOldParse = dummyGlobCmdParseFunc;
void cmdOldSetGlobalCmdParseFunc(CmdParseFunc func)
{
	globCmdOldParse = func;
}

void globCmdOldParsef(const char *fmt, ...)
{
	char buf[1024];
	va_list args;

	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);
	globCmdOldParse(buf);
}


CmdList globCmdOldList;
static int globCmdOldList_count;
void cmdOldAddToGlobalCmdList(Cmd *cmds, CmdHandler handler, char *header)
{
	static bool inited=false;
	int i;
	int index=-1;

	if (!inited) {
		inited = true;
// 		cmdOldAddToGlobalCmdList(dbg_cmds, &dbgCommandParse, NULL);
	}

	for (i=0; i<globCmdOldList_count; i++) {
		if (globCmdOldList.groups[i].cmds == cmds) { // Overwrite existing ones
			index=i;
			break;
		}
	}
	if (index==-1) {
		assert(globCmdOldList_count < ARRAY_SIZE(globCmdOldList.groups)-1);
		index = globCmdOldList_count;
		globCmdOldList_count++;
		globCmdOldList.groups[globCmdOldList_count].cmds = NULL;
	}
	cmdOldInit(cmds);
	globCmdOldList.groups[index].cmds = cmds;
	globCmdOldList.groups[index].handler = handler;
	if (header) {
		Strcpy(globCmdOldList.groups[index].header, header);
	} else {
		globCmdOldList.groups[index].header[0] = '\0';
	}
	if (globCmdOldList.name_hashes) {
		// Re-add to hash table
		cmdOldListAddToHashes(&globCmdOldList, index);
	}
}

CmdList privCmdList;
static int privCmdList_count;
void cmdOldAddToPrivateCmdList(Cmd *cmds, CmdHandler handler, char *header)
{
	static bool inited=false;
	int i;
	int index=-1;

	if (!inited) {
		inited = true;
	}

	for (i=0; i<privCmdList_count; i++) {
		if (privCmdList.groups[i].cmds == cmds) { // Overwrite existing ones
			index=i;
			break;
		}
	}
	if (index==-1) {
		assert(privCmdList_count < ARRAY_SIZE(privCmdList.groups)-1);
		index = privCmdList_count;
		privCmdList_count++;
		privCmdList.groups[privCmdList_count].cmds = NULL;
	}
	cmdOldInit(cmds);
	privCmdList.groups[index].cmds = cmds;
	privCmdList.groups[index].handler = handler;
	if (header) {
		Strcpy(privCmdList.groups[index].header, header);
	} else {
		privCmdList.groups[index].header[0] = '\0';
	}
	if (privCmdList.name_hashes) {
		// Re-add to hash table
		cmdOldListAddToHashes(&privCmdList, index);
	}
}


// 0 means we couldn't find anything, otherwise, returns the "nth" result that matches the substring pcStart
int cmdOldTabCompleteComand(const char* pcStart, const char** pcResult, int iSearchID, bool bSearchBackwards)
{
	int i;
	int iResultIndex=0;
	int iSearchIndex = iSearchID + (bSearchBackwards?-1:1);
	for (i=0; i<globCmdOldList_count; ++i)
	{
		CmdGroup* pCG = &globCmdOldList.groups[i];
		int j;
		for(j=0;pCG->cmds[j].name;j++)
		{
			// See if the name matches
			if ( strStartsWith(pCG->cmds[j].name, pcStart))
			{
				iResultIndex++;
				if ( iSearchIndex == iResultIndex )
				{
					*pcResult = pCG->cmds[j].name;
					return iResultIndex;
				}
				else if ( iSearchIndex < 0)
				{
					// Return the last one
					*pcResult = pCG->cmds[j].name;
				}
			}
		}
	}
	if ( iSearchIndex < 0)
		return iResultIndex;
	return 0;
}

/*
// The old one, for reference, since some features are still missing in the new one
int gclCompleteCommand(char *str, char *out, int searchID, int searchBackwards)
{
	static int num_cmds = sizeof(game_cmds)/sizeof(Cmd) - 1, 
		num_client_cmds = 0,
		num_control_cmds = 0,
		num_svr_cmds = 0;

	int i,
		cmd_offset = 0,
		server_cmd_offset = 0,
		client_cmd_offset = 0,
		control_cmd_offset = 0,
		total_cmds = 0;

	//static int server_searchID = 0;
	char searchStr[256] = {0}, foundStr[256] = {0};
	MRUList *consolemru = conGetMRUList();
	int num_mru_commands = 0;
	int effi;
	char uniquemrucommands[32][1024];

	if ( !str || str[0] == 0 )
	{
		out[0] = 0;
		return 0;
	}

	for (i=consolemru->count-1; i>=0; i--)
	{
		char cmd[1024];
		bool good = true;
		int j;
		Strncpyt(cmd, consolemru->values[i]);
		if (strchr(cmd, ' '))
			*strchr(cmd, ' ')=0;
		for (j=0; j<num_mru_commands; j++) {
			if (stricmp(uniquemrucommands[j], cmd)==0) {
				good = false;
			}
		}
		if (good) {
			Strcpy(uniquemrucommands[num_mru_commands], cmd);
			num_mru_commands++;
		}
	}

	if ( !num_svr_cmds )
		num_svr_cmds = eaSize(&serverCommands);

	if ( !num_client_cmds )
	{
		if ( client_control_cmds ) 
		{
			i = 0;

			for (;;++i )
			{
				if ( client_control_cmds[i].name )
					continue;
				break;
			}
		}
		num_client_cmds = i;
	}

	if ( !num_control_cmds )
	{
		if ( control_cmds )
		{
			i = 0;
			for (;;++i )
			{
				if ( control_cmds[i].name )
					continue;
				break;
			}
		}
		num_control_cmds = i;
	}

	// figure out command offsets
	cmd_offset = num_mru_commands,
		server_cmd_offset = cmd_offset + num_cmds,
		client_cmd_offset = server_cmd_offset + num_svr_cmds,
		control_cmd_offset = client_cmd_offset + num_client_cmds,
		total_cmds = control_cmd_offset + num_control_cmds;


	Strcpy( searchStr, stripUnderscores(str) );

	i = searchBackwards ? searchID - 1 : searchID + 1;

	//for ( ; i < num_cmds; ++i )
	while ( i != searchID )
	{
		if ( i >=1 && i < cmd_offset + 1) {
			effi = i - 1;
			Strcpy( foundStr, uniquemrucommands[effi]);
			if ( strStartsWith(foundStr, searchStr) )
			{
				strcpy(out, foundStr);
				return i;
			}
		}
		else if ( i >= cmd_offset + 1 && i < server_cmd_offset + 1 )
		{
			effi = i - cmd_offset - 1;
			Strcpy( foundStr, stripUnderscores(game_cmds[effi].name) );

			if ( !(game_cmds[effi].flags & CMDF_HIDEPRINT) && 
				game_cmds[effi].access_level <= GameClientAccessLevel() && strStartsWith(foundStr, searchStr) )
			{
				//strcpy(out, game_cmds[i].name);
				strcpy(out, foundStr);
				return i;
			}
		}
		else if ( i >= server_cmd_offset + 1 && i < client_cmd_offset + 1 )
		{
			effi = i - server_cmd_offset - 1;
			Strcpy( foundStr, stripUnderscores(serverCommands[effi]) );

			if ( strStartsWith(foundStr, searchStr) )
			{
				//strcpy(out, serverCommands[i - num_cmds]);
				strcpy(out, foundStr);
				return i;
			}
		}
		else if ( i >= client_cmd_offset + 1 && i < control_cmd_offset + 1 )
		{
			effi = i - client_cmd_offset - 1;
			Strcpy( foundStr, stripUnderscores(client_control_cmds[effi].name) );

			if ( !(client_control_cmds[effi].flags & CMDF_HIDEPRINT) && 
				client_control_cmds[effi].access_level <= GameClientAccessLevel() && 
				strStartsWith(foundStr, searchStr) )
			{
				//strcpy(out, game_cmds[i].name);
				strcpy(out, foundStr);
				return i;
			}
		}
		else if ( i >= control_cmd_offset + 1 && i < total_cmds + 1 )
		{
			effi = i - control_cmd_offset - 1;
			Strcpy( foundStr, stripUnderscores(control_cmds[effi].name) );

			if ( !(control_cmds[effi].flags & CMDF_HIDEPRINT) && 
				control_cmds[effi].access_level <= GameClientAccessLevel() && 
				strStartsWith(foundStr, searchStr) )
			{
				//strcpy(out, game_cmds[i].name);
				strcpy(out, foundStr);
				return i;
			}
		}

		searchBackwards ? --i : ++i;

		if ( i >= total_cmds + 1 ) i = 0;
		else if ( i < 0 ) i = total_cmds;
	}

	if ( searchID && searchID != -1 )
	{
		if ( searchID < num_mru_commands + 1 && searchID - 1 > 0 )
		{
			int idx = searchID - 1;
			strcpy(out, idx >= 0 ? uniquemrucommands[idx] : "");
		}
		if ( searchID < num_mru_commands + num_cmds + 1 )
		{
			int idx = searchID - num_mru_commands - 1;
			strcpy(out, idx >= 0 ? game_cmds[idx].name : "");
		}
		else if ( searchID < num_mru_commands + num_cmds + num_svr_cmds + 1 )
		{
			int idx = searchID - num_mru_commands - num_cmds - 1;
			strcpy(out, idx >= 0 ? serverCommands[idx] : "");
		}
		else
			out[0] = 0;
	}
	else
		out[0] = 0;
	return 0;
}
*/
