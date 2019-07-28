#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "container.h"
#include "utils.h"
#include "file.h"
#include "error.h"
#include "log.h"

static void stripCrs(char *data)
{
	char	*src,*dst;

	for(src = dst = data;*src;src++)
	{
		if (*src != '\r')
			*dst++ = *src;
	}
	*dst = 0;
}

int containerListLoad(DbList *list,char *mem,int *idxs)
{
	char		*last=0,*data_str,*str=0,*delim="\r\n",*cmd_delim="\n\r ",
				*next_str,at_end=0,*start_name = "Container", *cnt_end = "\nContainerEnd\n";
	int			start_len,id,count=0,end_len;
	DbContainer *container;

	start_len = strlen(start_name);
	end_len = strlen(cnt_end);
	stripCrs(mem);
	str = strtok_r(mem,delim,&last);
	next_str = str;
	for(;;)
	{
		str = strtok_r(next_str,delim,&last);
		if (!str)
			break;
		if (strncmp(str,start_name,start_len) == 0)
		{
			data_str = str + strlen(str) + 1;
			next_str = strstr(data_str,cnt_end);
			str = strtok_r(str,cmd_delim,&last);
			str = strtok_r(0,cmd_delim,&last);
			if (!str)
				continue;
			id = atoi(str);
			if (!next_str)
				break;
			next_str += strlen(cnt_end) + 1;
			if (!next_str[-1])
				at_end = 1;
			next_str[-end_len] = 0;

			container = containerAlloc(list,id);
			containerUpdate(list,container->id,data_str,1);
			if (idxs)
				idxs[count] = container->id;
			count++;

			for(;;)
			{
				str = strtok_r(0,cmd_delim,&last);
				if (!str)
					break;
			}
			if (at_end)
				break;
		}
	}
	return count;
}

int containerListLoadFile(DbList *list)
{
	char	*mem;
	int		count;

	mem = fileAlloc(list->fname,0);
	if (!mem)
	{
		LOG_OLD_ERR( "containerListLoad Failed to alloc memory to load %s\n",list->fname);
		return 0;
	}
	count = containerListLoad(list,mem,0);
	free(mem);
	return 1;
}

int containerListSave(DbList *list)
{
	char		*data;
	int			i,count=0;
	FILE		*file;
	DbContainer *container;

	//printf("Saving: %s\n",list->fname);
	file = fileOpen(list->fname,"wb");
	if (!file)
	{
		printf("cant open %s for writing!\n",list->fname);
		return 0;
	}
	for(i=0;i<list->num_alloced;i++)
	{
		container = list->containers[i];
		fprintf(file,"Container %d",container->id);
		fprintf(file,"\n");
		data = containerGetText(container);
		if (data)
		{
			fwrite(data,1,strlen(data),file);
		}
		else
			fprintf(file,"\n");
		fprintf(file,"ContainerEnd\n\n");
		count++;
	}
	fclose(file);
	return 1;
}

