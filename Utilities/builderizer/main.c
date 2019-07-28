#include "builderizer.h"
#include "utils.h"
#include <windows.h>


//S32 main(S32 argc, char** argv)
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	char	*argv[1000],buf[1000] = "";
	int		argc;

	argv[0] = "builderizer.exe";
	strcat(buf,lpCmdLine);
	argc = 1 + tokenize_line_quoted_safe(buf,&argv[1],ARRAY_SIZE(argv)-1,0);

	EXCEPTION_HANDLER_BEGIN
	
	runBuilderizer(argc, argv);

	EXCEPTION_HANDLER_END

	return 0;
}


