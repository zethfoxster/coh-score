#include "fileutil.h"
#include "utils.h"
#include <string.h>

static char	*string_data,**name_list;
static	int string_count,string_max,name_count,name_max;

static loadGroupNames(char *fname)
{
	char	*mem,*str,*args[100],*s,buf[1000];
	int		i,count;
	FILE	*file,*root_file;

	str = mem = fileAlloc(fname,0);
	file = fopen(fname,"wt");
	strcpy(buf,fname);
	s = strrchr(buf,'.');
	strcpy(s,".rootnames");
	root_file = fopen(buf,"wt");
	while(str)
	{
		count = tokenize_line(str,args,&str);
		if (count == 2 && stricmp(args[0],"Def")==0)
		{
			char	*a2[100];
			int		c2;
			char	*a3[100];
			int		c3;

			c2 = tokenize_line(str,a2,&str);
			if (c2 == 2 && stricmp(a2[0],"Obj")==0)
			{
				c3 = tokenize_line(str,a3,&str);
				for(i=0;i<count;i++)
					fprintf(root_file,"%s ",args[i]);
				fprintf(root_file,"\n");
				for(i=0;i<c2;i++)
					fprintf(root_file,"%s ",a2[i]);
				fprintf(root_file,"\nDefEnd\n\n");

				if (stricmp(a3[0],"DefEnd")!=0)
				{
					fprintf(file,"RootMod ");
					for(i=1;i<count;i++)
						fprintf(file,"%s ",args[i]);
					fprintf(file,"\n");
					for(i=0;i<c3;i++)
						fprintf(file,"%s ",a3[i]);
					fprintf(file,"\n");
				}
			}
			else
			{
				for(i=0;i<count;i++)
					fprintf(file,"%s ",args[i]);
				fprintf(file,"\n");
				for(i=0;i<c2;i++)
					fprintf(file,"%s ",a2[i]);
				fprintf(file,"\n");
			}
		}
		else
		{
			for(i=0;i<count;i++)
				fprintf(file,"%s ",args[i]);
			fprintf(file,"\n");
		}



#if 0
		{
			sprintf(buf,"%s/%s",fname,args[1]);
			s = dynArrayAdd(&string_data,1,&string_count,&string_max,strlen(buf)+1);
			strcpy(s,buf);
		}
#endif
	}
	free(mem);
	fclose(file);
	fclose(root_file);
}

static FileScanAction loadNames(char* dir, struct _finddata_t* data)
{
	char buffer[512];
	int len;
	int istxt = 0;

	len = strlen(data->name);
	if( len > 4 && !stricmp(".txt", data->name+len-4) )
		istxt = 1;

	if(!(data->name[0] == '_') && istxt )
	{
		sprintf(buffer, "%s/%s", dir, data->name);
		//printf("%s\n",buffer);
		loadGroupNames(buffer);
	}
	return FSA_EXPLORE_DIRECTORY;
}


static int cmpNames(const char **a,const char **b)
{
	return strcmp(*a,*b);
}
