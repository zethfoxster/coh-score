/***************************************************************************
 *     Copyright (c) 2003-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#if (!defined(ATTRIB_NAMES_H__)) || (defined(ATTRIB_NAMES_PARSE_INFO_DEFINITIONS)&&!defined(ATTRIB_NAMES_PARSE_INFO_DEFINED))
#define ATTRIB_NAMES_H__


/***************************************************************************/
/***************************************************************************/

typedef struct AttribName
{
	char *pchName;
	char *pchDisplayName;
	char *pchIconName;
} AttribName;

#ifdef ATTRIB_NAMES_PARSE_INFO_DEFINITIONS
TokenizerParseInfo ParseAttribName[] =
{
	{ "Name",         TOK_STRUCTPARAM|TOK_STRING(AttribName, pchName, 0) },
	{ "DisplayName",  TOK_STRUCTPARAM|TOK_STRING(AttribName, pchDisplayName, 0) },
	{ "IconName",     TOK_STRUCTPARAM|TOK_STRING(AttribName, pchIconName, 0) },
	{ "\n",           TOK_END,                      0 },
	{ 0 }
};
#endif

/***************************************************************************/
/***************************************************************************/

typedef struct AttribNames
{
	const AttribName **ppDamage;
	const AttribName **ppDefense;
	const AttribName **ppBoost;
	const AttribName **ppGroup;
	const AttribName **ppMode;
	const AttribName **ppElusivity;
	const AttribName **ppStackKey;
} AttribNames;

#ifdef ATTRIB_NAMES_PARSE_INFO_DEFINITIONS
TokenizerParseInfo ParseAttribNames[] =
{
	{ "{",				TOK_START,       0 },
	{ "Damage",			TOK_STRUCT(AttribNames, ppDamage, ParseAttribName) },
	{ "Defense",		TOK_STRUCT(AttribNames, ppDefense, ParseAttribName) },
	{ "Boost",			TOK_STRUCT(AttribNames, ppBoost, ParseAttribName) },
	{ "Group",			TOK_STRUCT(AttribNames, ppGroup, ParseAttribName) },
	{ "Mode",			TOK_STRUCT(AttribNames, ppMode, ParseAttribName) },
	{ "Elusivity",		TOK_STRUCT(AttribNames, ppElusivity, ParseAttribName) },
	{ "StackKey",		TOK_STRUCT(AttribNames, ppStackKey, ParseAttribName) },
	{ "}",				TOK_END,         0 },
	{ 0 }
};
#endif

/***************************************************************************/
/***************************************************************************/

extern SHARED_MEMORY AttribNames g_AttribNames;
extern int g_offHealAttrib;

/***************************************************************************/
/***************************************************************************/

char *dbg_AttribName(int offset, char *pchOrig);
int attrib_Offset(char *pch);

/***************************************************************************/
/***************************************************************************/

#ifdef ATTRIB_NAMES_PARSE_INFO_DEFINITIONS
#define ATTRIB_NAMES_PARSE_INFO_DEFINED
#endif

#endif /* #ifndef ATTRIB_NAMES_H__ */

/* End of File */

