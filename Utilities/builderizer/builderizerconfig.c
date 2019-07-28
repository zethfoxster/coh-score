#include "stdtypes.h"
#include "textparser.h"
#include "builderizerconfig.h"
#include "builderizer.h"

ParseTable tpiBuildConfigRoot[] =
{
	{"Config", TOK_INDIRECT | TOK_EARRAY | TOK_STRUCT_X, 0, sizeof(BuilderizerConfig), tpiBuildConfig },
	{0}
};


BuilderizerConfig **loadedConfigs = NULL;

void bcfgInit()
{
	ParserSetTableInfo(tpiBuildConfigRoot, sizeof(loadedConfigs), "BuildConfigRoot", NULL, __FILE__);
}

void bcfgLoad(char *fname)
{
	//int i, j;
	ParserLoadFiles(NULL, fname, NULL, 0, tpiBuildConfigRoot, &loadedConfigs);

	//// duplicate the required lists as vars, so as not to confuse the client
	//for ( i = 0; i < eaSize(&loadedConfigs); ++i )
	//{
	//	for ( j = 0; j < eaSize(&loadedConfigs[i]->lists); ++j )
	//	{
	//		RequiredVariable *rv = (RequiredVariable*)malloc(sizeof(RequiredVariable));
	//		rv->name = loadedConfigs[i]->lists[j]->name;
	//		rv->text = loadedConfigs[i]->lists[j]->text;
	//		rv->value = loadedConfigs[i]->lists[j]->values ? loadedConfigs[i]->lists[j]->values[0] : 0;
	//		rv->isActuallyList = 1;
	//		eaPush(&loadedConfigs[i]->vars, rv);
	//	}
	//}
}

void bcfgFreeConfigs()
{
	if ( loadedConfigs )
	{
		StructDeInit(tpiBuildConfigRoot, &loadedConfigs);
		loadedConfigs = NULL;
	}
}