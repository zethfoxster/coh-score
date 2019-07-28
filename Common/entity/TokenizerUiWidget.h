/***************************************************************************
 *     Copyright (c) 2005-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef TOKENIZERUIWIDGET_H
#define TOKENIZERUIWIDGET_H

//------------------------------------------------------------
// ui related salvage info.
// separate because i'm interested in having all ui objects
// share a struct of this type eventually.
// see TOKENIZERUIWIDGET_INLINEPARSEINFO if making changes to this
// -AB: created :2005 Feb 10 05:44 PM
// -AB: moved to its own file :2005 Mar 08 05:01 PM
//----------------------------------------------------------
typedef struct TokenizerUiWidget
{
	const char *pchDisplayName;
	const char *pchDisplayHelp;
	const char *pchDisplayShortHelp;
	const char *pchIcon;
} TokenizerUiWidget;


//------------------------------------------------------------
// helper macro for an __forceinline ui token
// -AB: created :2005 Feb 15 12:21 PM
//----------------------------------------------------------
#define TOKENIZERUIWIDGET_INLINEPARSEINFO(structname) \
{ "DisplayName",		TOK_STRUCTPARAM | TOK_STRING(structname,ui.pchDisplayName, 0) }, \
{ "DisplayHelp",		TOK_STRUCTPARAM | TOK_STRING(structname,ui.pchDisplayHelp, 0) }, \
{ "DisplayShortHelp",	TOK_STRUCTPARAM | TOK_STRING(structname,ui.pchDisplayShortHelp, 0) }, \
{ "Icon",				TOK_STRUCTPARAM | TOK_STRING(structname,ui.pchIcon, 0) }


#endif //TOKENIZERUIWIDGET_H
