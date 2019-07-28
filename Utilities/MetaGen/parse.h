/***************************************************************************
 *     Copyright (c) 2008-2008, NCSoft
 *     All Rights Reserved
 *     Confidential Property of NCSoft
 *
 * Module Description:
 *
 *
 ***************************************************************************/
#ifndef METAGEN_PARSE_H
#define METAGEN_PARSE_H

#include "ncstd.h"
#include "array.h"
NCHEADER_BEGIN

#define MAX_IDENTIFIER_LEN 64
#define MAX_STRING_LITERAL_LEN 2048

typedef enum ParseType
{
    ParseType_Void,
    ParseType_Func = MAKE_TYPE_CODE('F','U','N','C'),
} ParseType;

typedef enum MetaToken
{
	METATOKEN_COMMANDLINE,	// create a command line parameter, use COMMANDLINE(name) to specify a name
	METATOKEN_CONSOLE,		// create a in-game console command, use CONSOLE(name) to specify a 
	METATOKEN_REMOTE,		// create a remote procedure call
//	METATOKEN_LOADER,		// create a background loader command

	METATOKEN_ACCESSLEVEL,	// set accesslevel for client-initiated commands
	METATOKEN_CONSOLE_DESC,	// usage summary for console command listings
	METATOKEN_CONSOLE_REQPARAMS, // hack until I get per-parameter markup (and allow optional parameters)

	METATOKEN_COUNT
} MetaToken;
// update toks and meta_kws in c.y as well. see RPC

typedef enum GenType
{
    // (loop for i in '(CHAR SHORT INT LONG SIGNED UNSIGNED FLOAT DOUBLE CONST VOLATILE VOID) do (insert (format "GenType_%s\n" (symbol-name i))))
    GENTYPE_ERROR,
    GENTYPE_CHAR,
    GENTYPE_SHORT,
    GENTYPE_INT,
    GENTYPE_LONG,
    GENTYPE_UNSIGNED,
    GENTYPE_FLOAT,
    GENTYPE_DOUBLE,
    GENTYPE_VOID,
    GENTYPE_S8,
    GENTYPE_U8,
    GENTYPE_S16,
    GENTYPE_U16,
    GENTYPE_S32,
    GENTYPE_U32,
    GENTYPE_S64,
    GENTYPE_U64,
    GENTYPE_F32,
    GENTYPE_UNKNOWN,
    GENTYPE_COUNT,
} GenType;

typedef enum StorageSpecifier
{
     STORAGESPECIFIER_NONE     = 0,
     STORAGESPECIFIER_EXTERN   = 1,
     STORAGESPECIFIER_STATIC   = 2,
     STORAGESPECIFIER_AUTO     = 4,
     STORAGESPECIFIER_REGISTER = 8,
     STORAGESPECIFIER_TYPEDEF  = 16,
} StorageSpecifier;

typedef enum TypeQualifier
{
    TYPEQUALIFIER_NONE,
    TYPEQUALIFIER_CONST    = 1,
    TYPEQUALIFIER_RESTRICT = 2,
    TYPEQUALIFIER_VOLATILE = 4,
} TypeQualifier;




typedef struct Arg
{
//     GenType type;
    char name[MAX_IDENTIFIER_LEN];
    char type_name[MAX_IDENTIFIER_LEN];
    enum yytokentype type;
    // specifiers 
    BOOL ellipses; // this arg is ...
    union
    {
        U32 storage_specifiers;
        struct
        {
            U32 is_extern : 1;
            U32 is_static : 1;
            U32 is_auto   : 1;
            U32 is_register:1;
            U32 is_typedef :1; 
        };
    };
    // qualifiers
    union
    {
        U32 type_qualifiers;
        struct
        {
            U32 is_const    : 1;
            U32 is_restrict : 1;
            U32 is_volatile : 1;
        };
    };
    int n_indirections; // void *** = 3
} Arg;

#define TYPE_T           Arg
#define TYPE_FUNC_PREFIX aarg
#include "Array_def.h"
#undef TYPE_T
#undef TYPE_FUNC_PREFIX

#define aarg_create(capacity) aarg_create_dbg(capacity  DBG_PARMS_INIT)
#define aarg_destroy(ha) aarg_destroy_dbg(ha, NULL DBG_PARMS_INIT)
#define aarg_size(ha) aarg_size_dbg(ha  DBG_PARMS_INIT)
#define aarg_setsize(ha,size) aarg_setsize_dbg(ha, size  DBG_PARMS_INIT)
#define aarg_push(ha) aarg_push_dbg(ha  DBG_PARMS_INIT)
#define aarg_pushn(ha,n) aarg_pushn_dbg(ha,(n)  DBG_PARMS_INIT)
#define aarg_pop(ha) aarg_pop_dbg(ha  DBG_PARMS_INIT)
#define aarg_top(ha) aarg_top_dbg(ha  DBG_PARMS_INIT)
#define aarg_cp(hdest, hsrc, n) aarg_cp_dbg(hdest, hsrc, n  DBG_PARMS_INIT)
#define aarg_insert(hdest, src, i, n) aarg_insert_dbg(hdest, src, i, n  DBG_PARMS_INIT)
#define aarg_append(hdest, src, n) aarg_append_dbg(hdest, src, n  DBG_PARMS_INIT)
#define aarg_happend(hdest, hsrc) aarg_append_dbg(hdest, *hsrc, aarg_size(hsrc)  DBG_PARMS_INIT)
#define aarg_rm(ha, offset, n) aarg_rm_dbg(ha, offset, n  DBG_PARMS_INIT)
#define aarg_find(ha, b, cmp_fp, ctxt) aarg_find_dbg(ha, b, cmp_fp, ctxt DBG_PARMS_INIT)
#define aarg_foreach_ctxt(ha, cmp_fp, ctxt) aarg_foreach_ctxt_dbg(ha, cmp_fp, ctxt DBG_PARMS_INIT)
#define aarg_foreach(ha, cmp_fp) aarg_foreach_dbg(ha, cmp_fp DBG_PARMS_INIT)
#define aarg_pushfront(ha) aarg_pushfront_dbg(ha  DBG_PARMS_INIT) // expensive!!!


typedef struct ParseRun ParseRun;

typedef struct Parse
{
    ParseType type;
	char **callnames[METATOKEN_COUNT];
    Arg ret_type;
    char name[MAX_IDENTIFIER_LEN];
	char filename[MAX_PATH];
	int fileline;
    Arg *args;  // arg_
    ParseRun *run; // parent

	// used  after parsing
	U32 printed_header	: 1;
	U32 internal_only	: 1;
} Parse;

#define TYPE_T           Parse
#define TYPE_FUNC_PREFIX aparse
#include "Array_def.h"
#undef  TYPE_T
#undef  TYPE_FUNC_PREFIX

#define aparse_push(ha) aparse_push_dbg(ha  DBG_PARMS_INIT)
#define aparse_top(ha) aparse_top_dbg(ha  DBG_PARMS_INIT)
#define aparse_destroy(ha, fp) aparse_destroy_dbg(ha, fp DBG_PARMS_INIT)
#define aparse_size(ha) aparse_size_dbg(ha  DBG_PARMS_INIT)

typedef struct lex_line_info
{
    int first_line;
    int first_column;
    int last_line;
    int last_column;
} lex_line_info;

typedef struct ParseRun
{
    char fn[MAX_PATH];
    char *input;        // achr
    char *lex_ctxt;
    char *lex_line_start;
    lex_line_info loc;
    
    BOOL debug;         //output debug info

	// results 
    Parse *funcs;		// aparse_

    DBG_PARMS_MBR;
} ParseRun;

#include "c.tab.h" // has to be included here after the types

// *************************************************************************
// internal parsing functions
// *************************************************************************
Parse*      parse_init(ParseRun *run, ParseType type, U32 meta_flags);
void        parse_cleanup(Parse *p);

void	parse_addfunc(ParseRun *run, char **calls[METATOKEN_COUNT], Arg *return_type, char *func_name, Arg *args /*arg_*/);

void        parse_addmetaarg(Parse *p,char *arg, int line);
void        parse_addargtype(Arg *p, char *type_name);
void        parse_addargname(Parse *p, char *name);

// *************************************************************************
// 
// *************************************************************************

#define aparse_size(ha) aparse_size_dbg(ha  DBG_PARMS_INIT)
#define parserun_create() parserun_create_dbg(DBG_PARMS_INIT_SOLO)
#define parserun_processfile(run, fn) parserun_processfile_dbg(run, fn DBG_PARMS_INIT)
#define parserun_processfiles(run, files) parserun_processfile_dbg(run, files DBG_PARMS_INIT)
#define parserun_close(run, fn) parserun_close_dbg(run, fn DBG_PARMS_INIT)

ParseRun*   parserun_create_dbg(DBG_PARMS_SOLO);
void        parserun_destroy(ParseRun *r);
BOOL        parserun_init(ParseRun *pr, char *fn);
void        parserun_cleanup(ParseRun *pr);
BOOL        parserun_processfile_dbg(ParseRun *run, char *fn DBG_PARMS);
BOOL        parserun_processfiles_dbg(ParseRun *run, char ***files DBG_PARMS);
BOOL        parserun_close_dbg(ParseRun *run, const char *fn DBG_PARMS);
BOOL        parse_metafn_from_fn(char *dst, size_t n, char *nodir, int n_nodir, const char *fn, char *ext);
NCHEADER_END
#endif //METAGEN_PARSE_H
