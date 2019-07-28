/***************************************************************************
*     Copyright (c) 2002, Cryptic Studios
*     All Rights Reserved
*     Confidential Property of Cryptic Studios
*
* $Workfile:$
* $Revision: 29213 $
* $Date: 2006-05-10 15:23:39 -0700 (Wed, 10 May 2006) $
*
* Module Description:
*
* Revision History:
*
* $Log:$
* 
***************************************************************************/
#ifndef PARSE_H__
#define PARSE_H__

#include "h\so_types.h"

#define APPEND_NULL         ( 1 << 0 )
#define SKIP_TO_SPACE       ( 1 << 1 )
#define SKIP_TO_NON_FLOAT   ( 1 << 2 )
#define SKIP_TO_QUOTE       ( 1 << 3 )
#define SKIP_TO_CURLY       ( 1 << 4 )
#define SKIP_TO_CR          ( 1 << 5 )
#define SKIP_WHITESPACE     ( 1 << 6 )
#define SKIP_TO_COLON       ( 1 << 7 )
#define SKIP_TO_COMMA       ( 1 << 8 )
#define SKIP_TO_ALPHA       ( 1 << 9 )
#define SKIP_TO_NULL        ( 1 << 10 )
#define SKIP_TO_COMMENT     ( 1 << 11 )
#define SKIP_TO_UNDERSCORE  ( 1 << 12 )

typedef enum case_style
{
	CASE_MATTERS = 0,
	CASE_DOESNT_MATTER
} CASE_STYLE;

extern void set_case_style(CASE_STYLE style);

extern SO_BOOL eat_str(SO_UINT8 **mem, SO_INT *len, char  *name );
extern SO_BOOL get_sint8(SO_UINT8 **mem, SO_INT *len, SO_SINT8 *num);
extern SO_BOOL get_sint16(SO_UINT8 **mem, SO_INT *len, SO_SINT16 *num);
extern SO_BOOL get_sint32(SO_UINT8 **mem, SO_INT *len, SO_SINT32 *num);
extern SO_BOOL get_float(SO_UINT8 **mem, SO_INT *len, SO_FLOAT *num);
extern SO_BOOL suck_float(SO_UINT8 **mem, SO_INT *len, SO_FLOAT *num);

extern int get_int(SO_UINT8 **mem, SO_INT *len);
extern int get_int_no_alpha(SO_UINT8 **mem, SO_INT *len,int *ans);

extern SO_INT skip_to_match(SO_UINT8 **mem, SO_INT *len, char  *list_root, ...);
extern SO_INT skip_to_match_then_eat(SO_UINT8 **mem, SO_INT *len, char  **list);

extern SO_INT suck_str(
	SO_UINT8 *src, 
	SO_INT src_len, 
	SO_UINT8 *dest, 
	SO_INT max_bytes, 
	SO_INT mods);

extern SO_INT suck_str1(
	SO_UINT8 **src,
	SO_INT *len,
	SO_UINT8 *dest,
	SO_INT max_bytes,
	SO_INT mods);

extern int suckSimpleInt(SO_UINT8 **mem, SO_INT *len);

extern SO_BOOL get_name_before_curlies(SO_UINT8 **mem, SO_INT *len, char  *name);

extern SO_BOOL skip_past_cr(SO_UINT8 **src, SO_INT *len);
extern int skip_whitespace(SO_UINT8 **mem, SO_INT *len);

extern unsigned int suckU32(SO_UINT8 **mem, SO_INT *len);

extern int get_a_string(char *dest, SO_UINT8 **mem, SO_INT *len);
extern void get_string(char *eat_me, char *dest, SO_UINT8 **mem, SO_INT *len);

extern int get_line_float(float *dest, SO_UINT8 **mem, SO_INT *len);
extern int get_line_int(int *dest, SO_UINT8 **mem, SO_INT *len);

extern float get_floatX(SO_UINT8 **mem, SO_INT *len);

extern int nocase_string_compare(char *a, char *b);
extern int nocase_strstr(char *str1, char *str2);

// poz Madness! Madness!
#define EAT_BYTE *mem = *mem + 1; *len = *len - 1;

#endif /* #ifndef PARSE_H__ */

/* End of File */



