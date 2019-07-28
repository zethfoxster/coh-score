#include "structnet.h"

typedef struct
{
	char *name;
	char *value;
	char **values;
	char *text; // help text to let the person inputting the variable know what it is used for
	bool isActuallyList;  // used by the build client to differentiate between vars and lists
	bool notRequired;
} RequiredVariable;

typedef struct
{
	char *name;
	char *script;
	char *text; // help text to let the person using the config know what it does
	RequiredVariable **vars;

	bool disabled; // do not let them build this config
} BuilderizerConfig;

static ParseTable tpiReqVars[] =
{
	{"Name", TOK_STRING(RequiredVariable, name, 0)},
	{"DefaultValue", TOK_STRING(RequiredVariable, value, 0)},
	{"HelpText", TOK_STRING(RequiredVariable, text, 0)},
	{"IsActuallyList", TOK_BOOLFLAG(RequiredVariable, isActuallyList, 0)},
	{"NotRequired", TOK_BOOLFLAG(RequiredVariable, notRequired, 0)},
	{"{", TOK_START}, {"}", TOK_END},
	//{"\n", TOK_END},	
	{0}
};

static ParseTable tpiReqLists[] =
{
	{"Name", TOK_STRING(RequiredVariable, name, 0)},
	{"UNUSED", TOK_STRING(RequiredVariable, value, 0)},
	{"DefaultValues", TOK_STRINGARRAY(RequiredVariable, values)},
	{"HelpText", TOK_STRING(RequiredVariable, text, 0)},
	{"IsActuallyList", TOK_BOOLFLAG(RequiredVariable, isActuallyList, 1)},
	{"NotRequired", TOK_BOOLFLAG(RequiredVariable, notRequired, 0)},
	{"{", TOK_START}, {"}", TOK_END},
	//{"\n", TOK_END},	
	{0}
};

static ParseTable tpiBuildConfig[] =
{
	{"Name", TOK_STRING(BuilderizerConfig, name, 0), 0, TOK_FORMAT_LVWIDTH(100)},
	{"Script", TOK_STRING(BuilderizerConfig, script, 0)},
	{"HelpText", TOK_STRING(BuilderizerConfig, text, 0)},
	{"Var", TOK_STRUCT(BuilderizerConfig, vars, tpiReqVars)},
	{"List", TOK_STRUCT(BuilderizerConfig, vars, tpiReqLists)},
	{"Disable", TOK_BOOLFLAG(BuilderizerConfig, disabled, 0)},
	{"{", TOK_START}, {"}", TOK_END},
	//{"\n", TOK_END},	
	{0}
};

extern BuilderizerConfig **loadedConfigs;

//typedef struct ParseTable;
//extern ParseTable *tpiBuildConfig;

void bcfgInit(); // not necessary if you are calling bcfgLoad
void bcfgLoad(char *fname);
void bcfgFreeConfigs();
