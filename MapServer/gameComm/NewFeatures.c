#include "NewFeatures.h"
#include "textparser.h"
#include "LoadDefCommon.h"
#include "earray.h"
#include "cmdserver.h"
#include "gametypes.h"

SHARED_MEMORY NewFeatureList g_NewFeatureList;

TokenizerParseInfo ParseNewFeature[]=
{
	{ "",   TOK_STRUCTPARAM|TOK_INT(NewFeature, id, 0 ) }, //ID
	{ "",   TOK_STRUCTPARAM|TOK_STRING(NewFeature, pchDescription, 0 ) },	//Description
	{ "",   TOK_STRUCTPARAM|TOK_STRING(NewFeature, pchCommand, 0 ) },		//Command
	{ "\n",   TOK_STRUCTPARAM|TOK_END,  0 },
	{ "", 0, 0 }
};

TokenizerParseInfo ParseNewFeatureList[]=
{
	{ "NewFeature",   TOK_STRUCT(NewFeatureList, list, ParseNewFeature),  0 },
	{ "", 0, 0 }
};

int load_NewFeatures(SHARED_MEMORY_PARAM NewFeatureList * pNewFeatureList, const char * pchDefName)
{
	const char * pchBinName = MakeBinFilename(pchDefName);
	if (!ParserLoadFilesShared(MakeSharedMemoryName(pchBinName), NULL, pchDefName, pchBinName, PARSER_SERVERONLY, ParseNewFeatureList, pNewFeatureList, sizeof(ParseNewFeatureList), NULL, NULL, NULL, NULL, NULL))
	{
		return 0;
	}
	return 1;
}

const char * runNewFeatureCommand(ClientLink * client, int id)
{
	int i = eaSize(&g_NewFeatureList.list)-1;
	for (; i >= 0; --i)
	{
		if (g_NewFeatureList.list[i]->id == id)
		{
			serverParseClientEx(g_NewFeatureList.list[i]->pchCommand, client, ACCESS_INTERNAL);
			break;
		}
	}
	return NULL;
}
