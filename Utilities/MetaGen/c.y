%{
// PROLOGUE
#include "ncstd.h"
#include "Array.h"
#include <stdio.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "parse.h"
#include "str.h"

#pragma warning(push)
#pragma warning(disable:4053)
#pragma warning(disable:4244)
#pragma warning(disable:4702)
#pragma warning(disable:4706)
#pragma warning(disable:6262)
#pragma warning(disable:6385)

// #define YYLTYPE int
#define YYLLOC_DEFAULT(Current, Rhs, N) do { \
        Current = YYRHSLOC(Rhs,N);                \
} while(0)

%}

%debug
%union
{
	char tok[MAX_IDENTIFIER_LEN];
	MetaToken mtok;
	char *str;							// Str

	Arg arg;
	Arg *args;							// aarg
	char **strs;						// ap of Str
	char **callnames[METATOKEN_COUNT];	// array of ap of Str
}


%{
#define __STDC__ // ab: do this last, only needed by output of this
#define YYMALLOC malloc
#define YYFREE free

int yylex (YYSTYPE *lvalp, YYLTYPE *line, ParseRun *run);
void yyerror (YYLTYPE *line, ParseRun *run, char const *s);
char *tok_append(char*a,char *b);

static void print_token_value (FILE *, int, YYSTYPE);
#define YYPRINT(file, type, value) print_token_value (file, type, value)

%}

%pure-parser //          re-entrant
%error-verbose
%verbose
%parse-param {ParseRun *run}
%lex-param   {ParseRun *run}
%defines        //       make a .h file
%locations

%token<tok>		TYPEDEF EXTERN STATIC CONST_TOK VOID_TOK
%token<tok>		META_TOK
%token<tok>		TOK CONSTANT
%token<mtok>	COMMANDLINE CONSOLE REMOTE ACCESSLEVEL CONSOLE_DESC CONSOLE_REQPARAMS
%token<str>		STR
// %destructor		{ Str_destroy(&$$); } STR

%type<arg>		type_decl		// piggybacking on the type part of an argument
%type<arg>		function_arg
%type<args>		function_args
%type<mtok>		meta_token
%type<strs>		meta_names
%type<callnames> meta_args

%token TYPEDEF EXTERN STATIC 

%start translation_unit

%%

translation_unit:	/* empty */
		|			translation_unit meta_function_definition
		|			translation_unit ignored_opt
					;

meta_function_definition:	META_TOK '(' meta_args ')' static_opt type_decl TOK '(' function_args ')' '{'
							{
								parse_addfunc(	run,
												$3, // acall
												&$6, // return type
												$7, // func name
												$9); // func args
							}
		|					META_TOK '(' meta_args ')' ';'
							{
								parse_addfunc(	run,
												$3,
												NULL,
												NULL,
												NULL);
							}
							;

// log_line :  WARN_TOK '(' STR ',' args ')'

type_decl:		TOK				{ ClearStruct(&$$); strcpy($$.type_name, $1); }
		|		VOID_TOK		{ ClearStruct(&$$); strcpy($$.type_name, $1); }
		|		CONST_TOK TOK	{ ClearStruct(&$$); strcpy($$.type_name, $2); $$.is_const = 1; }
		|		type_decl '*'	{ $$ = $1; $$.n_indirections++; }
				;

function_arg:	type_decl TOK	{ $$ = $1; strcpy($$.name,$2); }
				;

function_args:	VOID_TOK						{ $$ = NULL; }
		|		function_arg					{ $$ = NULL;	*aarg_push(&$$) = $1; }
		|		function_args ',' function_arg	{ $$ = $1;		*aarg_push(&$$) = $3; }
				;

// storage_class_specifier_opt
static_opt:		/* empty */
		|		TYPEDEF
		|		EXTERN
		|		STATIC
				;

// could put the valid args here, but might as well let the func
meta_token:		COMMANDLINE
		|		CONSOLE
		|		REMOTE
		|		ACCESSLEVEL
		|		CONSOLE_DESC
		|		CONSOLE_REQPARAMS
				;

meta_names:		/* empty */			{ $$ = NULL; }
		|		meta_names STR		{ ap_push(&$1, Str_dup($2)); $$ = $1; }
				;

meta_args:		/* empty */								{ memset(&$$, 0, sizeof($$)); }
		|		meta_args meta_token '(' meta_names ')'	{ ap_size(&$4) ? ap_append(&$1[$2], $4, ap_size(&$4)) : ap_push(&$1[$2], NULL); memcpy($$, $1, sizeof($$)); }
		|		meta_args meta_token					{ ap_push(&$1[$2], NULL); memcpy($$, $1, sizeof($$)); }
				;

// things we don't care about
ignored_opt:	'{'
		|		'('
		|		')'
		|		'*'
		|		','
		|		';'
		|		TOK
		|		STR			{ Str_destroy(&$1); }
		|		CONSTANT
		|		CONST_TOK
		|		VOID_TOK
		;
%%

#pragma warning(pop)

static char non_tok_chars[] = ",*(){;"; // if here, return as is, otherwise a TOK

static struct StrKws { char *kw; int tok; } tok_kws[] =
{
	{ "META",		META_TOK	},
	{ "const",		CONST_TOK	},
	{ "void",		VOID_TOK	},
//	{ "typedef",	TYPEDEF		},
//	{ "extern",		EXTERN		},
//	{ "static",		STATIC		},
};

static struct MetaKws { char *kw; int tok; MetaToken mtok; } meta_kws[] =
{
#define METAKWS(word) { #word, word, METATOKEN_##word }
	METAKWS(COMMANDLINE),
	METAKWS(CONSOLE),
	METAKWS(REMOTE),

	METAKWS(ACCESSLEVEL),
	METAKWS(CONSOLE_DESC),
	METAKWS(CONSOLE_REQPARAMS),
};
assert_static(ARRAY_SIZE(meta_kws) == METATOKEN_COUNT);

int yylex_stdin(YYSTYPE *lval, YYLTYPE *line, ParseRun *run)
{
//    static struct TypeKws
    int c;
    char *tok = Str_temp();
    int j;

    yydebug = 1;

    while ((c = getchar ()) == ' ' || c == '\t' || c == '\n' || c == '\r')
    {
        if(c == '\n') { 
        }
    }
    ungetc (c, stdin);

	achr_pop(&tok);
    while(isalnum(c = getchar()) || c == '_')
		achr_push(&tok, (U8)c);
	achr_push(&tok, (U8)c);
    if(!*tok) // no characters grabbed. return
        return c == EOF?0:c;
    ungetc(c,stdin);

	for(j = 0; j < ARRAY_SIZE(tok_kws); ++j)
    {
        if(0 == strcmp(tok_kws[j].kw,tok))
        {
            strcpy(lval->tok,tok);
            return tok_kws[j].tok;
        }
    }
    for(j = 0; j < ARRAY_SIZE(meta_kws); ++j)
    {
        if(0 == strcmp(meta_kws[j].kw,tok))
        {
            lval->mtok = meta_kws[j].mtok;
            return meta_kws[j].tok;
        }
    }

	strcpy(lval->tok,tok);
	Str_destroy(&tok);

	return TOK;
}

int yylex(YYSTYPE *lval, YYLTYPE *line, ParseRun *run)
{
    BOOL beginning_of_line = FALSE;
    char prev;
    int res = 0;
//    static struct TypeKws
//    char tok_scratch[MAX_IDENTIFIER_LEN]; */
   char *i = lval->tok;
   char **ht;
   int j;

   if(run->debug)
       yydebug = 1;
   else
       yydebug = 0;

   if(!run->lex_ctxt) {
      run->lex_ctxt = run->input;
      beginning_of_line = TRUE;
   }
   if(!run->lex_ctxt)
   {
	  WARN("yylex: no input to lexer %s:%i",run->caller_fname,run->line);
      return 0; // 
   }
   else if(!INRANGE(run->lex_ctxt,run->input,run->input+Str_len(&run->input)+1))
   { 
       WARN("yylex: run->lex_ctxt(%p) is not within run->input and its length (%p,%i)", run->lex_ctxt, run->input, Str_len(&run->input));
       return 0;
   }
   ht = &run->lex_ctxt;

   if(!run->loc.first_line)
       run->loc.first_line = 1;

   while (*(*ht) && strchr(" \t\v\r\n",*(*ht)))
   {
       if(*(*ht) == '\n') {
           run->loc.first_line++;
           run->lex_line_start = *ht;
           beginning_of_line = TRUE;
       }
       prev = *(*ht);
      (*ht)++;
   }

   if(!INRANGE(run->lex_line_start,run->input,*ht))
       run->lex_line_start = run->input;

   run->loc.last_line = run->loc.first_line;
   run->loc.first_column = *ht - run->lex_line_start;

#define ADVANCE_TO_NEWLINE        while(*(*ht) && *(*ht) != '\n') (*ht)++


	if(beginning_of_line && *(*ht) == '#') // #include, etc.
	{
		ADVANCE_TO_NEWLINE;
		return yylex(lval, line, run); // recurse
	}
	else if(*(*ht) == '/' && (*ht)[1] == '/') // line comment
	{
		ADVANCE_TO_NEWLINE;
		return yylex(lval, line, run); // recurse
	}
	else if(*(*ht) == '/' && (*ht)[1] == '*') // block comment
	{
		prev = 0;
		(*ht) +=2;
		while(*(*ht) && *(*ht) != '/' && prev != '*')
			prev = *(*ht)++;
		(*ht)++;
		return yylex(lval, line, run); // recurse
	}
	else if(*(*ht) == '"') // string literal
	{
		lval->str = NULL;
		(*ht)++;
		for(;*(*ht);(*ht)++)
		{
			if(*(*ht) == '"' && (*ht)[-1] != '\\')
				break;

			if(achr_size(&lval->str) > MAX_STRING_LITERAL_LEN)
			{
				WARN("Warning: %s(%d): string literal too long", run->fn, run->loc.first_line);
				return -1;
			}
			achr_push(&lval->str,*(*ht));
		}
		achr_push(&lval->str, '\0');
		if(*(*ht))
			(*ht)++;
		res = STR;
	}
	else if(*(*ht) == '\'') // character constant
	{
		*i = *(*ht)++;
		while(*(*ht))
		{
			if(*(*ht) == '\'' && (*ht)[-1] != '\\')
				break;
			*i = *(*ht)++;
		}
		if(*(*ht))
			(*ht)++;
		*i = 0;
		res = CONSTANT;
	}
	else if(*(*ht) == '0' && strchr("xX",(*ht)[1])) // hex
	{
		*i = *(*ht)++;
		*i = *(*ht)++;
		while(*(*ht) && ch_isxdigit(*(*ht)))
			*i = *(*ht)++;
		// apparently floats and ints and hex ar constants
//		if(1 != sscanf(tok,"%h",&lval->num)) */
//			WARN("couldn't scan %s to hex",tok); */
		*i = 0;
		res = CONSTANT;
	}
	else if(ch_isdigit(*(*ht))) // numeric constant
	{
		*i = *(*ht)++;
		while(*(*ht) && ch_isdigit(*(*ht)))
			*i = *(*ht)++;
		*i = 0;
		res = CONSTANT;
	}
	else // TOK
	{
		while(*ht && (ch_isalnum(*(*ht)) || *(*ht) == '_'))
		{
			*i++ = *(*ht)++;
			if(i - lval->tok >= ARRAY_SIZE(lval->tok))
			{
				*i = 0;
				WARN("token %s too long",lval->tok);
				return -1;
			}
		}
		*i = 0;
	}
	run->loc.last_column = *ht - run->lex_line_start;

	line->first_line = run->loc.first_line;
	line->first_column = run->loc.first_column;
	line->last_line = run->loc.last_line;
	line->last_column = run->loc.last_column;

	// already set, e.g. string literal
	if(res)
		return res;

	if(!*lval->tok) // no characters grabbed. '+','=',';' etc.
	{
		char c = *(*ht)++;
		if(strchr(non_tok_chars,c))
			return c;
		// we don't do anything with this yet, return as TOK
		// otherwise we get a parse error when a non TOK is
		// returned in ignored_tok rule
		lval->tok[0] = c;
		lval->tok[1] = 0;
		return TOK;
	}

	for(j = 0; j < ARRAY_SIZE(tok_kws); ++j)
		if(0 == strcmp(tok_kws[j].kw,lval->tok))
			return tok_kws[j].tok;

	for(j = 0; j < ARRAY_SIZE(meta_kws); ++j)
	{
		if(0 == strcmp(meta_kws[j].kw,lval->tok))
		{
			lval->mtok = meta_kws[j].mtok;
			return meta_kws[j].tok;
		}
	}

	return TOK;
}

void yyerror (YYLTYPE *line, ParseRun *run, char const *s)
{
    fprintf (stderr, "%s(%i):%s\n", run->fn,line->first_line,s);
}

static void print_token_value (FILE *strm, int tok, YYSTYPE lval)
{
    int j;
    for(j = 0; j < sizeof(tok_kws)/sizeof(*tok_kws); ++j) {
        if(tok_kws[j].tok == tok) {
            fprintf(strm,"%s",lval.tok);
            return;
        }
    }
    for(j = 0; j < sizeof(meta_kws)/sizeof(*meta_kws); ++j) {
        if(meta_kws[j].tok == tok) {
            fprintf(strm,"%s",meta_kws[j].kw);
            return;
        }
    }
    fprintf(strm,"%s",lval.tok);
}

char *tok_append(char *a,char *b)
{
    char *res;
    int na = strlen(a);
    int nb = strlen(b);
    res = malloc(na+nb+1);
    sprintf_s(res,na+nb+1,"%s%s",a,b);
    return res;
}
