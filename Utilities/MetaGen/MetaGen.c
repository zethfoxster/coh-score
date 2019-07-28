#include "parse.h"
#include "fileio.h"
#include "unittest/ncunittest.h"

#include "timer.h"
#include "Array.h"
#include "Str.h"

void metagen_unittest(char *dir);

int getProjectFiles(char ***files, const char *project)
{
	// hack for now
	char *s;
	char *filetext = Str_temp();
	char fn[MAX_PATH];

	sprintf(fn, "%s/%s.vcproj", project, project);
	if(!file_alloc(&filetext, fn))
		return 0;

	for(s = filetext; (s = strstrAdvance(s, "RelativePath=\"")) != NULL; )
	{
		char *quote = strchr(s, '\"');
		if(quote)
		{
			char *file = NULL;
			*quote = '\0';
			if(	strEnds(s, ".c") ||
				strEnds(s, ".h") )
			{
				if(strBegins(s, ".\\"))
					s += 2;
				Str_catf(&file, "%s\\%s", project, s);
				ap_push(files, file);
			}
			s = quote+1;
		}
	}
	Str_destroy(&filetext);
	return 1;
}

static void s_printms_humanreadable(S64 ms)
{
	int printed = 0;
	if(ms > MS_PER_DAY)
	{
		printf("%dd ", (int)(ms/MS_PER_DAY));
		ms %= MS_PER_DAY;
		printed = 1;
	}
	if(printed || ms > MS_PER_HR)
	{
		printf("%dh ", (int)(ms/MS_PER_HR));
		ms %= MS_PER_HR;
		printed = 1;
	}
	if(printed || ms > MS_PER_MIN)
	{
		printf("%dm ", (int)(ms/MS_PER_MIN));
		ms %= MS_PER_MIN;
	}

	printf("%.3fs", (float)ms / MS_PER_SEC);
}

int main(int argc, char* argv[]) 
{
	int i;
	S64 t = time_mss2k();

	for( i = 0; i < argc; ++i ) 
	{
		if(streq(argv[i],"-unittest"))
		{
			char *dir = NULL;
			if(i+1<argc)
				dir = argv[++i];
			metagen_unittest(dir);
			return g_testsuite.num_failed;
		}

		if(	streq(argv[i], "-project") &&
			argc > i+1 )
		{
			// TODO: figure out a better directory layout

			// expects to find project/project.vcproj
			// output goes into MetaGen
			const char *project = argv[i+1];
			char **files = NULL;

			getProjectFiles(&files, project);
			if(ap_size(&files))
			{
				int j;
				ParseRun *r = parserun_create();
				for(j = 0; j < ap_size(&files); j++)
				{
					parserun_processfile(r, files[j]);
					Str_destroy(&files[j]);
				}
				parserun_close(r, project);
				parserun_destroy(r);
			}
			ap_destroy(&files, NULL);
			printf("%s: ", project), s_printms_humanreadable(time_mss2k() - t), printf("\n");
		}
	}

	printf("Total: "), s_printms_humanreadable(time_mss2k() - t), printf("\n");
	return 0;
}
