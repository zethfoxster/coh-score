%{
// PROLOGUE
#include "ncstd.h"
#include "Array.h"
#include <stdio.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "parse.h"


// #define YYLTYPE int
#define CS ClearStruct 
#define MV(A,B) (A) = (B); ClearStruct(&B)
#define YYLLOC_DEFAULT(Current, Rhs, N) do { \
        Current = YYRHSLOC(Rhs,N);                \
} while(0)

%}

%debug
%union
{
   char tok[MAX_IDENTIFIER_LEN];
   U32 flag;
   Arg *args; //         aarg
   Arg arg;
   Parse parse;
   char *str; //         achr
   int num;
   U32 u32;                        
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

%token<tok> META_TOK
%token<flag> RPC TST1 TST2

//@todo -AB: oh man, there is so much more to do than this :11/06/08
%destructor { achr_destroy(&$$)  ; } STRING_LITERAL

%type<flag>   meta_flag meta_args

// C standard tokens:

%type<u32> storage_class_specifier type_qualifier 
%type<arg>  declaration_specifiers type_specifier
%type<arg> parameter_declaration // inline typedef extern static void* foo
%type<num>   pointer
%type<parse> declarator direct_declarator
%type<args>  parameter_type_list parameter_list

%token<str> STRING_LITERAL
%token<tok> IDENTIFIER CONSTANT SIZEOF
%token PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP
%token AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token XOR_ASSIGN OR_ASSIGN 
%token<tok> TYPE_NAME

%token<tok> TYPEDEF EXTERN STATIC AUTO REGISTER INLINE RESTRICT
%token<tok> CHAR_TOK SHORT_TOK INT_TOK LONG_TOK SIGNED UNSIGNED FLOAT_TOK DOUBLE CONST_TOK VOLATILE VOID_TOK
%token<tok> BOOL_TOK COMPLEX IMAGINARY
%token<tok> STRUCT UNION ENUM ELLIPSIS

%token CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN

%start translation_unit

%%

//translation_unit: /* empty */
//        |       meta_function_definition translation_unit
//        |       ignored_opt translation_unit
//                ;

// meta_function_definition: */
//                 META_TOK '(' meta_args ')' static_opt type_decl IDENTIFIER '(' function_args_opt ')' '{'  */
//  {  */
//     parse_addfunc(run, */
//                 $3, /\/ meta flags */
//                 &$6, /\/ return type */
//                 $7, /\/ func name */
//                 $9); /\/ func args */
//  } */
//                 ; */

// function_arg: */
//                 type_decl IDENTIFIER { */
//                     $$ = $1; */
//                     strcpy_asafe($$.name,$2); */
//                 } */
//                 ; */

// function_args_opt: */
//                 /*empty*\/ { $$ = NULL; } */
//         |       function_arg ',' function_args_opt  */
//                 {  */
//                     *aarg_pushfront(&$3) = $1;  */
//                     $$ = $3; */
//                 } */
//         |       function_arg  */
//                 {  */
//                     $$ = NULL; *aarg_pushfront(&$$) = $1;  */
//                 }; */

// type_decl: */
//                 IDENTIFIER            {CS(&$$);strcpy_asafe($$.type_name,$1);} */
//         |       type_decl  '*' { $$ = $1; $$.n_indirections++; } */
//                 ; */

// storage_class_specifier_opt
// static_opt: */
//                 /*empty*\/ */
//         |       TYPEDEF */
//         |       EXTERN */
//         |       STATIC */
//                 ;

// could put the valid args here, but might as well let the func
meta_flag:      RPC
        |       TST1
        |       TST2
                ;

meta_args: /*   empty*/   { $$ = 0; }
        |       meta_flag { $$ = $1; }
        |       meta_flag ',' meta_args { $$ = $1 | $3; }
                ;

//meta_func_decl: META_TOK '(' meta_args ')' { $$ = parse_init(run,ParseType_Func,$3); }
                ;

// things we don't care about
//ignored_opt:    '{'
//        |       '('
//        |       ')'
//        |       '*'
//        |       ','
//        |       IDENTIFIER
//        |       STRING_LITERAL
//        ;

// *************************************************************************
//  Big C grammar
// *************************************************************************

translation_unit
	: external_declaration
	| translation_unit external_declaration
	;

primary_expression
	: IDENTIFIER
	| CONSTANT
	| STRING_LITERAL
	| '(' expression ')'
	;

postfix_expression
	: primary_expression
	| postfix_expression '[' expression ']'
	| postfix_expression '(' ')'
	| postfix_expression '(' argument_expression_list ')'
	| postfix_expression '.' IDENTIFIER
	| postfix_expression PTR_OP IDENTIFIER
	| postfix_expression INC_OP
	| postfix_expression DEC_OP
	| '(' type_name ')' '{' initializer_list '}'
	| '(' type_name ')' '{' initializer_list ',' '}'
	;

argument_expression_list
	: assignment_expression
	| argument_expression_list ',' assignment_expression
	;

unary_expression
	: postfix_expression
	| INC_OP unary_expression
	| DEC_OP unary_expression
	| unary_operator cast_expression
	| SIZEOF unary_expression
	| SIZEOF '(' type_name ')'
	;

unary_operator
	: '&'
	| '*'
	| '+'
	| '-'
	| '~'
	| '!'
	;

cast_expression
	: unary_expression
	| '(' type_name ')' cast_expression
	;

multiplicative_expression
	: cast_expression
	| multiplicative_expression '*' cast_expression
	| multiplicative_expression '/' cast_expression
	| multiplicative_expression '%' cast_expression
	;

additive_expression
	: multiplicative_expression
	| additive_expression '+' multiplicative_expression
	| additive_expression '-' multiplicative_expression
	;

shift_expression
	: additive_expression
	| shift_expression LEFT_OP additive_expression
	| shift_expression RIGHT_OP additive_expression
	;

relational_expression
	: shift_expression
	| relational_expression '<' shift_expression
	| relational_expression '>' shift_expression
	| relational_expression LE_OP shift_expression
	| relational_expression GE_OP shift_expression
	;

equality_expression
	: relational_expression
	| equality_expression EQ_OP relational_expression
	| equality_expression NE_OP relational_expression
	;

and_expression
	: equality_expression
	| and_expression '&' equality_expression
	;

exclusive_or_expression
	: and_expression
	| exclusive_or_expression '^' and_expression
	;

inclusive_or_expression
	: exclusive_or_expression
	| inclusive_or_expression '|' exclusive_or_expression
	;

logical_and_expression
	: inclusive_or_expression
	| logical_and_expression AND_OP inclusive_or_expression
	;

logical_or_expression
	: logical_and_expression
	| logical_or_expression OR_OP logical_and_expression
	;

conditional_expression
	: logical_or_expression
	| logical_or_expression '?' expression ':' conditional_expression
	;

assignment_expression
	: conditional_expression
	| unary_expression assignment_operator assignment_expression
	;

assignment_operator
	: '='
	| MUL_ASSIGN
	| DIV_ASSIGN
	| MOD_ASSIGN
	| ADD_ASSIGN
	| SUB_ASSIGN
	| LEFT_ASSIGN
	| RIGHT_ASSIGN
	| AND_ASSIGN
	| XOR_ASSIGN
	| OR_ASSIGN
	;

expression
	: assignment_expression
	| expression ',' assignment_expression
	;

constant_expression
	: conditional_expression
	;

declaration
	: declaration_specifiers ';'
	| declaration_specifiers init_declarator_list ';'
	;

declaration_specifiers // typedef/extern/static, void/int/char, inline
	: storage_class_specifier   /*X*/               { CS(&$$); $$.storage_specifiers = $1; } // X typedef/extern/static
	| storage_class_specifier declaration_specifiers {$$.storage_specifiers |= $1 }
    | type_specifier            /*X*/               { CS(&$$); $$.type = $1.type; strcpy_asafe($$.type_name,$1.type_name); }
	| type_specifier declaration_specifiers         { $$.type = $1.type; strcpy_asafe($$.type_name,$1.type_name); } // ab: see TYPE_NAME notes elsewhere for this reasoning. I'm making it so you can't say, e.g. void extern
	| type_qualifier            /*X*/               { CS(&$$); $$.type_qualifiers = $1; } 
	| type_qualifier declaration_specifiers         { $$.type_qualifiers |= $1 }
	| function_specifier        /*X*/               { CS(&$$); }            // inline, who cares
    | function_specifier declaration_specifiers     { MV($$,$2); } 
	;

init_declarator_list
	: init_declarator
	| init_declarator_list ',' init_declarator
	;

init_declarator
	: declarator
	| declarator '=' initializer
	;

storage_class_specifier
	: TYPEDEF  { $$ = STORAGESPECIFIER_TYPEDEF; }
	| EXTERN   { $$ = STORAGESPECIFIER_EXTERN; }
	| STATIC   { $$ = STORAGESPECIFIER_STATIC; }
	| AUTO     { $$ = STORAGESPECIFIER_AUTO; }
	| REGISTER { $$ = STORAGESPECIFIER_REGISTER; }
	;

type_specifier
	: VOID_TOK   { $$.type = VOID_TOK; strcpy_asafe($$.type_name,$1); } 
	| CHAR_TOK   { $$.type = CHAR_TOK; strcpy_asafe($$.type_name,$1); } 
	| SHORT_TOK  { $$.type = SHORT_TOK; strcpy_asafe($$.type_name,$1); } 
	| INT_TOK    { $$.type = INT_TOK; strcpy_asafe($$.type_name,$1); } 
	| LONG_TOK   { $$.type = LONG_TOK; strcpy_asafe($$.type_name,$1); } 
	| FLOAT_TOK  { $$.type = FLOAT_TOK; strcpy_asafe($$.type_name,$1); } 
	| DOUBLE     { $$.type = DOUBLE; strcpy_asafe($$.type_name,$1); } 
	| SIGNED     { $$.type = SIGNED; strcpy_asafe($$.type_name,$1); } 
	| UNSIGNED   { $$.type = UNSIGNED; strcpy_asafe($$.type_name,$1); } 
	| BOOL_TOK   { $$.type = BOOL_TOK; strcpy_asafe($$.type_name,$1); } 
	| COMPLEX    { $$.type = COMPLEX; strcpy_asafe($$.type_name,$1); } 
	| IMAGINARY  { $$.type = IMAGINARY; strcpy_asafe($$.type_name,$1); } 
	| struct_or_union_specifier { CS(&$$); }  // AB: these two lines are probably a bug
	| enum_specifier            { CS(&$$); }
	| TYPE_NAME { $$.type = TYPE_NAME; strcpy_asafe($$.type_name,$1); }  
                // ab: changing this to IDENTIFIER causes a non-reducable rule conflict with parameter_declaration because, apparently, an identifier_list can be just an identifier (e.g. foo(a,b,c)). I removed that line from parameter_declaration but I think it will still parse validly. we'll see. it might not actually matter if I left it. (SEE parameter_declaration for the conflict)
	;

struct_or_union_specifier
	: struct_or_union IDENTIFIER '{' struct_declaration_list '}'
	| struct_or_union '{' struct_declaration_list '}'
	| struct_or_union IDENTIFIER
	;

struct_or_union
	: STRUCT
	| UNION
	;

struct_declaration_list
	: struct_declaration
	| struct_declaration_list struct_declaration
	;

struct_declaration
	: specifier_qualifier_list struct_declarator_list ';'
	;

specifier_qualifier_list
	: type_specifier specifier_qualifier_list
	| type_specifier
	| type_qualifier specifier_qualifier_list
	| type_qualifier
	;

struct_declarator_list
	: struct_declarator
	| struct_declarator_list ',' struct_declarator
	;

struct_declarator
	: declarator
	| ':' constant_expression
	| declarator ':' constant_expression
	;

enum_specifier
	: ENUM '{' enumerator_list '}'
	| ENUM IDENTIFIER '{' enumerator_list '}'
	| ENUM '{' enumerator_list ',' '}'
	| ENUM IDENTIFIER '{' enumerator_list ',' '}'
	| ENUM IDENTIFIER
	;

enumerator_list
	: enumerator
	| enumerator_list ',' enumerator
	;

enumerator
	: IDENTIFIER
	| IDENTIFIER '=' constant_expression
	;

type_qualifier
	: CONST_TOK { $$ = TYPEQUALIFIER_CONST; }
	| RESTRICT  { $$ = TYPEQUALIFIER_RESTRICT; }
	| VOLATILE  { $$ = TYPEQUALIFIER_VOLATILE; }
	;

function_specifier
	: INLINE
	;

declarator
        : pointer direct_declarator { MV($$,$2); 

                $$.ret_type.n_indirections
                = $1; }
        | direct_declarator         { MV($$,$1); }
	;


direct_declarator  // foo(int a, int b,...)
    : IDENTIFIER                                    { CS(&$$); strcpy_asafe($$.name,$1); }
    | '('   declarator ')'                          { CS(&$$); MV($$.args,$2.args); $2.args = NULL; }
	| direct_declarator '[' type_qualifier_list assignment_expression ']'
	| direct_declarator '[' type_qualifier_list ']'
	| direct_declarator '[' assignment_expression ']'
	| direct_declarator '[' STATIC type_qualifier_list assignment_expression ']'
	| direct_declarator '[' type_qualifier_list STATIC assignment_expression ']'
	| direct_declarator '[' type_qualifier_list '*' ']'
	| direct_declarator '[' '*' ']'
	| direct_declarator '[' ']'
	| direct_declarator '(' parameter_type_list ')'  { MV($$,$1); MV($$.args,$3); }
	| direct_declarator '(' identifier_list ')'      { MV($$,$1); }
	| direct_declarator '(' ')'
	;

pointer
	: '*'                             { $$ = 1; }
	| '*' type_qualifier_list         { $$ = 1; }
	| '*' pointer                     { $$ += 1; }
	| '*' type_qualifier_list pointer { $$ += 1; }
	;

type_qualifier_list
	: type_qualifier
	| type_qualifier_list type_qualifier
	;

parameter_type_list
	: parameter_list
        |       parameter_list ',' ELLIPSIS { Arg *a = aarg_push(&$1); a->ellipses = TRUE; $$ = $1;  }
	;

parameter_list
	: parameter_declaration { $$ = NULL; *aarg_push(&$$) = $1; } // push param 
|       parameter_list ',' parameter_declaration { MV($$,$1); *aarg_push(&$$) = $3; }
	;

parameter_declaration
    : declaration_specifiers declarator          { $$ = $1; strcpy_asafe($$.name,$1.name); CS(&$1); } // void *foo,
	| declaration_specifiers abstract_declarator { $$ = $1; } // void *
	| declaration_specifiers                     { $$ = $1;} // void  ab: see TYPE_NAME comment later in this doc
	;

identifier_list
	: IDENTIFIER
	| identifier_list ',' IDENTIFIER
	;

type_name
	: specifier_qualifier_list
	| specifier_qualifier_list abstract_declarator
	;

abstract_declarator
	: pointer
	| direct_abstract_declarator
	| pointer direct_abstract_declarator
	;

direct_abstract_declarator // e.g. void *, no param name.
	: '(' abstract_declarator ')'
	| '[' ']'
	| '[' assignment_expression ']'
	| direct_abstract_declarator '[' ']'
	| direct_abstract_declarator '[' assignment_expression ']'
	| '[' '*' ']'
	| direct_abstract_declarator '[' '*' ']'
	| '(' ')'
	| '(' parameter_type_list ')'
	| direct_abstract_declarator '(' ')'
	| direct_abstract_declarator '(' parameter_type_list ')'
	;

initializer
	: assignment_expression
	| '{' initializer_list '}'
	| '{' initializer_list ',' '}'
	;

initializer_list
	: initializer
	| designation initializer
	| initializer_list ',' initializer
	| initializer_list ',' designation initializer
	;

designation
	: designator_list '='
	;

designator_list
	: designator
	| designator_list designator
	;

designator
	: '[' constant_expression ']'
	| '.' IDENTIFIER
	;

statement
	: labeled_statement
	| compound_statement
	| expression_statement
	| selection_statement
	| iteration_statement
	| jump_statement
	;

labeled_statement
	: IDENTIFIER ':' statement
	| CASE constant_expression ':' statement
	| DEFAULT ':' statement
	;

compound_statement
	: '{' '}'
	| '{' block_item_list '}'
	;

block_item_list
	: block_item
	| block_item_list block_item
	;

block_item
	: declaration
	| statement
	;

expression_statement
	: ';'
	| expression ';'
	;

selection_statement
	: IF '(' expression ')' statement
	| IF '(' expression ')' statement ELSE statement
	| SWITCH '(' expression ')' statement
	;

iteration_statement
	: WHILE '(' expression ')' statement
	| DO statement WHILE '(' expression ')' ';'
	| FOR '(' expression_statement expression_statement ')' statement
	| FOR '(' expression_statement expression_statement expression ')' statement
	| FOR '(' declaration expression_statement ')' statement
	| FOR '(' declaration expression_statement expression ')' statement
	;

jump_statement
	: GOTO IDENTIFIER ';'
	| CONTINUE ';'
	| BREAK ';'
	| RETURN ';'
	| RETURN expression ';'
	;

external_declaration
                : function_definition
        |       declaration
	;

function_definition
                : declaration_specifiers declarator declaration_list compound_statement
        |       declaration_specifiers declarator compound_statement
        |       META_TOK '(' meta_args ')' 
                declaration_specifiers                  // <arg> extern/static,void/int/char,inline => return type
                declarator                              // <parse> *foo() => ret_type indirection, fname, args
                compound_statement                      // ignored
                {
                   $5.n_indirections = $6.ret_type.n_indirections;
                   parse_addfunc(run, $3, &$5, $6.name, $6.args );
                   CS(&$6);
                }
	;

declaration_list
	: declaration
	| declaration_list declaration
	;

%%

// return these as their value
static char non_tok_chars[] = ";{},:=()[].&!~-+*/%<>^|?";
static struct StrKws { char *kw; int tok; } tok_kws[] = {
    // my tokens
    { "META",              META_TOK },
    // --------------------
    // C lang
    { "...", ELLIPSIS },
    { ">>=", RIGHT_ASSIGN },
    { "<<=", LEFT_ASSIGN },
    { "+=", ADD_ASSIGN },
    { "-=", SUB_ASSIGN },
    { "*=", MUL_ASSIGN },
    { "/=", DIV_ASSIGN },
    { "%=", MOD_ASSIGN },
    { "&=", AND_ASSIGN },
    { "^=", XOR_ASSIGN },
    { "|=", OR_ASSIGN },
    { ">>", RIGHT_OP },
    { "<<", LEFT_OP },
    { "++", INC_OP },
    { "--", DEC_OP },
    { "->", PTR_OP },
    { "&&", AND_OP },
    { "||", OR_OP },
    { "<=", LE_OP },
    { ">=", GE_OP },
    { "==", EQ_OP },
    { "!=", NE_OP },
    // section 2
    { "auto", AUTO },
    { "_Bool", BOOL_TOK },
    { "break", BREAK },
    { "case", CASE },
    { "char", CHAR_TOK },
    { "_Complex", COMPLEX },
    { "const", CONST_TOK },
    { "continue", CONTINUE },
    { "default", DEFAULT },
    { "do", DO },
    { "double", DOUBLE },
    { "else", ELSE },
    { "enum", ENUM },
    { "extern", EXTERN },
    { "float", FLOAT_TOK },
    { "for", FOR },
    { "goto", GOTO },
    { "if", IF },
    { "_Imaginary", IMAGINARY },
    { "inline", INLINE },
    { "int", INT_TOK },
    { "long", LONG_TOK },
    { "register", REGISTER },
    { "restrict", RESTRICT },
    { "return", RETURN },
    { "short", SHORT_TOK },
    { "signed", SIGNED },
    { "sizeof", SIZEOF },
    { "static", STATIC },
    { "struct", STRUCT },
    { "switch", SWITCH },
    { "typedef", TYPEDEF },
    { "union", UNION },
    { "unsigned", UNSIGNED },
    { "void", VOID_TOK },
    { "volatile", VOLATILE },
    { "while", WHILE },

};

static struct MetaKws { char *kw; int tok; int flag; } meta_kws[] = {
   { "RPC",    RPC,    METAFLAG_RPC },
   //@todo -AB: remove, testing only :10/30/08 
   { "TST1",   TST1,   METAFLAG_RPC<<1 },
   { "TST2",   TST2,   METAFLAG_RPC<<2 },
};

int yylex_stdin(YYSTYPE *lval, YYLTYPE *line, ParseRun *run)
{
//    static struct TypeKws
    int c;
    char tok[128];
    char *i = tok;
    int j;

    yydebug = 1;

    while ((c = getchar ()) == ' ' || c == '\t' || c == '\n' || c == '\r')
    {
        if(c == '\n') { 
        }
    }
    ungetc (c, stdin);
    while(isalnum(c = getchar ()) || c == '_')
        *i++ = c;
    *i = 0;
    if(!*tok) // no characters grabbed. return
        return c == EOF?0:c;
    ungetc(c,stdin);
    for(j = 0; j < sizeof(tok_kws)/sizeof(*tok_kws); ++j)
    {
        if(0 == strcmp(tok_kws[j].kw,tok))
        {
            strcpy_asafe(lval->tok,tok);
            return tok_kws[j].tok;
        }
     }
    for(j = 0; j < sizeof(meta_kws)/sizeof(*meta_kws); ++j)
    {
        if(0 == strcmp(meta_kws[j].kw,tok))
        {
            lval->flag = meta_kws[j].flag;
            return meta_kws[j].tok;
        }
     }
    strcpy_asafe(lval->tok,tok);
    return IDENTIFIER;
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


   if(beginning_of_line && *(*ht) == '#') {                     // #include, etc.
       ADVANCE_TO_NEWLINE;
       return yylex(lval, line, run); // recurse
   }
   if(*(*ht) == '/' && (*ht)[1] == '/') {             // '//' comment
       ADVANCE_TO_NEWLINE;
       return yylex(lval, line, run); // recurse
   }
   else if(*(*ht) == '/' && (*ht)[1] == '*') {        // /*...*/ comment
       prev = 0;
       (*ht) +=2;
       while(*(*ht) && *(*ht) != '/' && prev != '*') {
           prev = *(*ht)++;
       }
       (*ht)++;
       return yylex(lval, line, run); // recurse
   }
   else if(*(*ht) == '"') {                          // string literal
       lval->str = 0;
       (*ht)++;
       for(;*(*ht);(*ht)++) {
           if(*(*ht) == '"' && (*ht)[-1] != '\\')
               break;
           if(achr_size(&lval->str) > MAX_STRING_LITERAL_LEN) {
               WARN("string literal too long line %i",run->loc.first_line);
               return -1;
           }
           achr_push(&lval->str,*(*ht));
       }
       if(*(*ht))
           (*ht)++;
       res = STRING_LITERAL;
   }
   else if(*(*ht) == '0' && strchr("xX",(*ht)[1])) { // hex
       *i = *(*ht)++;
       *i = *(*ht)++;
       while(*(*ht) && isxdigit(*(*ht)))
           *i = *(*ht)++;
       // apparently floats and ints and hex ar constants
//        if(1 != sscanf(tok,"%h",&lval->num)) */
//            WARN("couldn't scan %s to hex",tok); */
       res = CONSTANT;
   }
   else if(isdigit(*(*ht))) { // numeric constant
       *i = *(*ht)++;
       while(*(*ht) && isdigit(*(*ht)))
           *i = *(*ht)++;
       res = CONSTANT;
   }
   else while(*ht && (isalnum(*(*ht)) || *(*ht) == '_')) { // IDENTIFIER
        *i++ = *(*ht)++;
        if(i - lval->tok >= ARRAY_SIZE(lval->tok)) {
            *i = 0;
            WARN("token %s too long",lval->tok);
            return -1;
        }
   }
   *i = 0;

   run->loc.last_column = *ht - run->lex_line_start;
   line->first_line = run->loc.first_line;
   line->first_column = run->loc.first_column;
   line->last_line = run->loc.last_line;
   line->last_column = run->loc.last_column;

   // already set, e.g. string literal
   if(res)
       return res;

   if(!*lval->tok) { // no characters grabbed. '+','=', etc.
       char c = *(*ht)++;
       if(strchr(non_tok_chars,c))
           return c;
       // we don't do anything with this yet, return as IDENTIFIER
       // otherwise we get a parse error when a non IDENTIFIER is
       // returned in ignored_tok rule
       lval->tok[0] = c;
       lval->tok[1] = 0;
       return IDENTIFIER;
   }

   for(j = 0; j < sizeof(tok_kws)/sizeof(*tok_kws); ++j) {
       if(0 == strcmp(tok_kws[j].kw,lval->tok)) {
           return tok_kws[j].tok;
       }
   }
   for(j = 0; j < sizeof(meta_kws)/sizeof(*meta_kws); ++j) {
       if(0 == strcmp(meta_kws[j].kw,lval->tok)) {
           lval->flag = meta_kws[j].flag;
           return meta_kws[j].tok;
       }
   }
   return IDENTIFIER;
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
