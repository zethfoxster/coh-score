/* A Bison parser, made by GNU Bison 2.1.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     TYPEDEF = 258,
     EXTERN = 259,
     STATIC = 260,
     CONST_TOK = 261,
     VOID_TOK = 262,
     META_TOK = 263,
     TOK = 264,
     CONSTANT = 265,
     COMMANDLINE = 266,
     CONSOLE = 267,
     REMOTE = 268,
     ACCESSLEVEL = 269,
     CONSOLE_DESC = 270,
     CONSOLE_REQPARAMS = 271,
     STR = 272
   };
#endif
/* Tokens.  */
#define TYPEDEF 258
#define EXTERN 259
#define STATIC 260
#define CONST_TOK 261
#define VOID_TOK 262
#define META_TOK 263
#define TOK 264
#define CONSTANT 265
#define COMMANDLINE 266
#define CONSOLE 267
#define REMOTE 268
#define ACCESSLEVEL 269
#define CONSOLE_DESC 270
#define CONSOLE_REQPARAMS 271
#define STR 272




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 29 "c.y"
typedef union YYSTYPE {
	char tok[MAX_IDENTIFIER_LEN];
	MetaToken mtok;
	char *str;							// Str

	Arg arg;
	Arg *args;							// aarg
	char **strs;						// ap of Str
	char **callnames[METATOKEN_COUNT];	// array of ap of Str
} YYSTYPE;
/* Line 1447 of yacc.c.  */
#line 83 "c.tab.h"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



#if ! defined (YYLTYPE) && ! defined (YYLTYPE_IS_DECLARED)
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} YYLTYPE;
# define yyltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif




