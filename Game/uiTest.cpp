

#include "textparser.h"
#include "uiTest.h"



TokenizerParseInfo ParseWidget[] = {
	{ "Widget",					TOKEN_STRUCT,	offsetof(Widget, children),		sizeof(Widget),		ParseWidget}, 
	{ "value",					TOKEN_INT,		offsetof(Widget, value)			        },	
	{ "End",					TOKEN_END,		0										},
	{ "", 0, 0 }
};

Widget samplewidget;

void MakeWidget()
{
	//int ParserLoadFiles(char* dir, char* filemask, char* persistfile, int trypersistfirst, TokenizerParseInfo pti[], void* structptr);
	if (!ParserLoadFiles("menu/widgets", ".widge", "widgets.bin", 0, ParseWidget, &samplewidget, NULL))
	{
		printf("MakeWidget: couldn't load widgets!\n");
	}
	printf("yeah!");
}
