/***************************************************************************
 *     Copyright (c) 2000-2003, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 *
 * $Workfile: code_table.h $
 * $Revision: 28748 $
 * $Date: 2006-04-17 15:13:20 -0700 (Mon, 17 Apr 2006) $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $Log: /dev/Common/code_table.h $
 * 
 * 41    3/24/03 6:36p Jonathan
 * Moved Vars and Attack out of read_menu
 * 
 * 40    3/19/03 10:38a Cw
 * Added CodeGroupList
 * 
 * 39    3/12/03 6:10p Cw
 * Created new code commands deleted old ones
 * 
 * 37    3/03/03 7:35p Cw
 * 
 * 36    2/24/03 5:15p Cw
 * 
 * 35    2/20/03 12:54p Cw
 * Changes to Scroll box and minor UI fixes
 * 
 * 34    2/13/03 1:52p Poz
 * Added hack so the server can figure out which buttons have strings that
 * I want to be attribs.
 * 
 * 33    2/05/03 5:21p Poz
 * Removed unused CodeFlipCostumeColors entry.
 * Added header.
 * 
 ***************************************************************************/
#ifndef CODE_TABLE_H__
#define CODE_TABLE_H__

#if CLIENT
	#define CODE_B(X)       #X, ( int )&(X), 0,
	#define CODE_B_ATTR(X)   #X, ( int )&(X), 0,
#else
	#define CODE_B(X)       #X, 0, 0,
	// HACK: CODE_B_STR is a hack to let the server know which buttons
	//       have strings which need to be output to the attrib file.
	//       The CodeType.code field is used by the game code as an 
	//       identifier designating which kind of items are contained
	//       within it. (This is not my fault, it's inherited.)
	//       Anyway, on the server, these functions don't exist, and
	//       we needed a way to tell the apart. Thus I gave them a different
	//       value for code to fidn them
	#define CODE_B_ATTR(X)   #X, 1, 0,
#endif

typedef struct code_type
{
	char    *str;
	int     code;
	int     hid;
}CodeType;

extern CodeType code_table[];

int code_idx(char* name);
int codeTableSize();
void codeTableInit();

#endif /* #ifndef CODE_TABLE_H__ */

/* End of File */

