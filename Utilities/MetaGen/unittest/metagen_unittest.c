/***************************************************************************
 *     Copyright (c) 2008-2008, NCSoft
 *     All Rights Reserved
 *     Confidential Property of NCSoft
 *
 * Module Description:
 *
 *
 ***************************************************************************/
#include "ncstd.h"
#include "fileio.h"
#include "str.h"
#include "../parse.h"
#include "ncHash.h"
#include "Array.h"
#include "unittest/ncunittest.h"
TestSuite g_testsuite; // hack, so i don't have to include so much in UtilitiesLib

#define foo "bar baz \"sanq\"" // trip up parser META(FOO) {}
META(RPC)
void*** voidpppFoo(int a, char *b)
{
    return NULL;
}

#define simple_func "META(REMOTE) \r\n void Foo(int a){}"
#define more_complex_func "META(REMOTE) void*** Foo(int a, char **b,Bar ***c){}"
#define broken_func "META(REMOTE ARGLEBARGLE) void*** Foo(int a, char **b,Bar ***c){}"

int yyparse (ParseRun *run); 

#include "meta/metagen_unittest_meta.h"

void metagen_unittest(char *dir)
{
    int i;
    char fn[MAX_PATH];
    if(!dir)
        sprintf(fn, "%s", __FILE__);
    else
        sprintf(fn, "%s/%s", dir, "metgen_unittest.c");
    TEST_START("meta_unittest");
    {
        ParseRun *r = parserun_create();
        Parse *p;
        char *strs[] = {
            "// this shouldn't do anything \"\" META(\nint a;",
            "int /* this shouldn't do anything \"\" META(*/ a;",
            "void foo();",
            "#define F 0x12345\nint a;",
            "extern void bar();",
            "static int baz(){ int a; int b; }",
        }; 
//        r->debug = 1;
        for( i = 0; i < ARRAY_SIZE(strs); ++i ) {
            // parse tests 
            parserun_cleanup(r);
            Str_cp_raw(&r->input,strs[i]); 
            UTASSERT(0==yyparse(r));
            UTASSERT(aparse_size(&r->funcs) == 0);
        }
        
        // simple
        parserun_cleanup(r);
        Str_cp_raw(&r->input,simple_func);
        r->lex_ctxt = NULL;
        UTASSERT(0==yyparse(r));
        UTASSERT(aparse_size(&r->funcs) == 1);
        p = &r->funcs[0];
        UTASSERT(aarg_size(&p->args) == 1);
        UTASSERT(0==strcmp(p->ret_type.type_name,"void"));
        UTASSERT(0==strcmp(p->name,"Foo"));
        UTASSERT(0==_stricmp(p->args[0].name,"a"));
        UTASSERT(0==_stricmp(p->args[0].type_name,"int"));
        UTASSERT(p->args[0].n_indirections == 0);

        // complex
        parserun_cleanup(r);
        Str_printf(&r->input,"%s\n%s\n%s\n",more_complex_func,more_complex_func,more_complex_func);
        r->lex_ctxt = NULL;
        UTASSERT(0==yyparse(r));
        UTASSERT(aparse_size(&r->funcs) == 3);
        for(i = 0; i < aparse_size(&r->funcs);++i)
        {
            p = &r->funcs[i];
            UTASSERT(ap_size(&p->callnames[METATOKEN_REMOTE]));
            UTASSERT(0==strcmp(p->ret_type.type_name,"void"));
            UTASSERT(p->ret_type.n_indirections == 3);
            UTASSERT(0==strcmp(p->name,"Foo"));
            UTASSERT(3 == aarg_size(&p->args));
            
            UTASSERT(0==_stricmp(p->args[0].name,"a"));
            UTASSERT(0==_stricmp(p->args[0].type_name,"int"));
            UTASSERT(p->args[0].n_indirections == 0);
            
            UTASSERT(0==_stricmp(p->args[1].name,"b"));
            UTASSERT(0==_stricmp(p->args[1].type_name,"char"));
            UTASSERT(p->args[1].n_indirections == 2);
            
            UTASSERT(0==_stricmp(p->args[2].name,"c"));
            UTASSERT(0==_stricmp(p->args[2].type_name,"Bar"));
            UTASSERT(p->args[2].n_indirections == 3);
        }
        
        // error case
        parserun_cleanup(r);
        UTASSERT(aparse_size(&r->funcs) == 0);
        Str_cp_raw(&r->input,broken_func);
        r->lex_ctxt = NULL;
        UTASSERT(0!=yyparse(r));
        UTASSERT(aparse_size(&r->funcs) == 0);

        parserun_cleanup(r);
        UTASSERT(file_exists(fn)); 
        UTASSERT(file_alloc(&r->input,fn));
        UTASSERT(0==yyparse(r));
        parserun_destroy(r);
    }
    TEST_END;
    
    TEST_START("meta from a file");
    {
        char metafn[MAX_PATH];
        ParseRun *r = parserun_create();
//        strcpy_asafe(fn,"c:/src_aaron/Utilities/MetaGen/unittest/example.c");
//        r->debug = TRUE;
        UTASSERT(parserun_processfile(r,fn));
        UTASSERT(parse_metafn_from_fn(ASTR(metafn),NULL,0,fn,"h"));
        UTASSERT(file_exists(metafn));
        UTASSERT(parse_metafn_from_fn(ASTR(metafn),NULL,0,fn,"c"));
        UTASSERT(file_exists(metafn));
        parserun_destroy(r);
    }
    TEST_END;

	UTSUMMARY();
}
