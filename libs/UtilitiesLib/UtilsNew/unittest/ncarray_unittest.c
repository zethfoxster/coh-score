#include "ncstd.h"
#include "ncHash.h"
#include "ncunittest.h"
#include "array.h"

typedef struct Foo
{
    char b[5];
    int a;
} Foo;

static int foo_acc;
static int foo_acc;
void Foo_Destroy(Foo *helt)
{
    foo_acc += helt->a;
}

#define TYPE_T Foo
#define TYPE_FUNC_PREFIX foo
#include "array_def.h"
#undef TYPE_T
#undef TYPE_T_CODE

#define foo_create(capacity) foo_create_dbg(capacity  DBG_PARMS_INIT)
#define foo_destroy(ha, fp) foo_destroy_dbg(ha, fp DBG_PARMS_INIT)
#define foo_size(ha) foo_size_dbg(ha  DBG_PARMS_INIT)
#define foo_setsize(ha,size) foo_setsize_dbg(ha, size  DBG_PARMS_INIT)
#define foo_push(ha) foo_push_dbg(ha  DBG_PARMS_INIT)
#define foo_pushn(ha,n) foo_pushn_dbg(ha,(n)  DBG_PARMS_INIT)
#define foo_pop(ha) foo_pop_dbg(ha  DBG_PARMS_INIT)
#define foo_top(ha) foo_top_dbg(ha  DBG_PARMS_INIT)
#define foo_cp(hdest, hsrc, n) foo_cp_dbg(hdest, hsrc, n  DBG_PARMS_INIT)
#define foo_append(hdest, src, n) foo_append_dbg(hdest, src, n  DBG_PARMS_INIT)
#define foo_rm(ha, offset, n) foo_rm_dbg(ha, offset, n  DBG_PARMS_INIT)
#define foo_find(ha, b, cmp_fp, ctxt) foo_find_dbg(ha, b, cmp_fp, ctxt DBG_PARMS_INIT)
#define foo_foreach_ctxt(ha, cmp_fp, ctxt) foo_foreach_ctxt_dbg(ha, cmp_fp, ctxt DBG_PARMS_INIT)
#define foo_foreach(ha, cmp_fp) foo_foreach_dbg(ha, cmp_fp DBG_PARMS_INIT)
#define foo_pushfront(ha) foo_pushfront_dbg(ha  DBG_PARMS_INIT) // expensive!!!

int foo_cmp(Foo *a, Foo *b, void *ctxt)
{
    intptr_t i = (intptr_t)ctxt;
    if(!i)
        return strcmp(a->b,b->b);
    return a->a - b->a;
}

void foo_foreach_ctxt_func(Foo *a, void *ctxt)
{
    int *pi = (int*)ctxt;
    *pi += a->a;
}

void foo_foreach_func(Foo *a)
{
    foo_acc += a->a;
}

void ncarray_unittest(void)
{
#pragma warning(disable:4312) // int to pointer
    TEST_START("ncarray_unittest");
    {
        int i;
        int n_tests = 100;
        Foo *ap = NULL;
        Foo *acp = NULL;
        foo_setsize(&ap,0);
        for( i = 0; i < n_tests; ++i )
        {
            Foo *fp = foo_push(&ap);
            sprintf_s(ASTR(fp->b),"%i",i);
            fp->a = i;
            UTASSERT(fp - ap == i);
        }
        UTASSERT(foo_size(&ap) == i);
        foo_setsize(&ap,0);  // force re-use of allocated memory
        {
            Foo *fp = foo_pushn(&ap,n_tests);
            Foo tst = {0};            
            for( i = 0; i < n_tests; ++i )
            {
                UTASSERT(0 == memcmp(fp,&tst,sizeof(tst)));
                sprintf_s(ASTR(fp->b),"%i",i);
                fp->a = i;
                fp++;
            }
        }        
        UTASSERT(foo_size(&ap) == n_tests);
        foo_setsize(&ap,n_tests);
        
        // copy
        foo_cp(&acp,&ap,0);
        UTASSERT(foo_size(&acp) == i);
        for( i = 0; i < n_tests; ++i )
            UTASSERT(0==memcmp(&acp[i],&ap[i],sizeof(Foo)));

        UTASSERT(foo_find(&acp,&acp[1],NULL,NULL) == 1);
        UTASSERT(foo_find(&acp,&acp[1],&foo_cmp,(void*)0) == 1);
        UTASSERT(foo_find(&acp,&acp[1],&foo_cmp,(void*)1) == 1);
        
        {
            int acc = 0;
            foo_foreach_ctxt(&acp,&foo_foreach_ctxt_func,&acc);
            UTASSERT(acc == 99*50);
            foo_acc = 0;
            foo_foreach(&acp,&foo_foreach_func);
            UTASSERT(foo_acc == 99*50);
        }
            
        // pop
        for( i--; i >= 0; --i)
        {
            Foo *ft = foo_top(&ap);
            Foo *f =  foo_pop(&ap);
            UTASSERT(ft == f);
            UTASSERT(f->a == i);
            UTASSERT(atoi(f->b) == i);
        }
        UTASSERT(foo_size(&ap) == 0);
        foo_destroy(&ap,Foo_Destroy);
        foo_acc = 0;
        foo_destroy(&acp,Foo_Destroy);
        UTASSERT(foo_acc == 99*50);
    }
    {
        int i;
        int n_tests = 100;
        void **ap = NULL;
        void **acp = NULL;
        ap_setsize(&ap,0);
        for( i = 0; i < n_tests; ++i )
            UTASSERT(i == ap_push(&ap,(void*)i));
        UTASSERT(ap_size(&ap) == i);

        // copy
        ap_cp(&acp,&ap);
        UTASSERT(ap_size(&acp) == i);
        for( i = 0; i < n_tests; ++i )
            UTASSERT(acp[i] == (void*)i);

        for( i--; i >= 0; --i)
            UTASSERT((void*)i == *ap_pop(&ap));
    }
    // achr
    {
        char *a=0,*b=0;
        int i;
        for( i = 0; i < 100; ++i ) 
            achr_push(&a,(char)i);
        UTASSERT(achr_size(&a)==100);
        for( i = 0; i < 100; ++i ) 
            UTASSERT(a[i] == (char)i);
        achr_ncp(&b,a,10);
        UTASSERT(achr_size(&b) == 10);
        for( i = 0; i < achr_size(&b); ++i ) 
            UTASSERT(a[i] == b[i]);
        achr_cp(&b,&a,0);
        for( i = 0; i < achr_size(&b); ++i ) 
            UTASSERT(a[i] == b[i]);

        // remove
        achr_rm(&b,10,40);
        UTASSERT(achr_size(&b)==60);
        for( i = 0; i < 10; ++i ) 
            UTASSERT(b[i] == (char)i);
        for(; i < 60; ++i ) 
            UTASSERT(b[i] == (char)i+40);
        achr_cp(&b,&a,0);

        achr_rm(&b,10,1000); // should truncate to length
        UTASSERT(achr_size(&b)==10);
        for( i = 0; i < 10; ++i ) 
            UTASSERT(b[i] == (char)i);
        achr_cp(&b,&a,0);
        achr_rm(&b,10000,10); // fail
        UTASSERT(achr_size(&b)==100);
        achr_rm(&b,0,-10); // fail
        UTASSERT(achr_size(&b)==100);
    }
    {
        char src[] = {
            '0','1','2','3','4'
        };
        char s2[] = {
            '5','6','7','8','9'
        };
        char *d = 0;
        int n;
        int n2;
        achr_ncp(&d,src,ARRAY_SIZE(src));
        UTASSERT(0==memcmp(src,d,ARRAY_SIZE(src)));
        UTASSERT(ARRAY_SIZE(src) == achr_size(&d));
        achr_append(&d,s2,ARRAY_SIZE(s2));
        n = ARRAY_SIZE(src);
        n2 = ARRAY_SIZE(src) + ARRAY_SIZE(s2);
        UTASSERT(n2 == achr_size(&d));
        UTASSERT(0==memcmp(src,d,ARRAY_SIZE(src)));        
        UTASSERT(0==memcmp(s2,d+n,ARRAY_SIZE(s2)));
        achr_destroy(&d); 
    }
    {
        char *d = 0;
        achr_insert(&d,"0",0,1);
        UTASSERT(*d == '0');
        achr_insert(&d,"1",0,1);
        UTASSERT(*d == '1');
        achr_insert(&d,"2",0,1);
        UTASSERT(*d == '2');
        achr_insert(&d,"3",1,1);
        UTASSERT(d[1] == '3');
		achr_push(&d,0);
        UTASSERT(0==strcmp(d,"2310"));
        achr_destroy(&d);
    }
    {
        char *d = 0;
        *achr_pushfront(&d) = '0';        
        UTASSERT(*d == '0');
        *achr_pushfront(&d) = '1';
        UTASSERT(*d == '1');
        *achr_pushfront(&d) = '2'; 
        UTASSERT(*d == '2');
        achr_push(&d,0);
        UTASSERT(0==strcmp(d,"210"));
        achr_destroy(&d);
    }
    TEST_END;
}


#define TYPE_T Foo
#define TYPE_FUNC_PREFIX foo
#include "array_def.c"
#undef TYPE_T
#undef TYPE_T_CODE
