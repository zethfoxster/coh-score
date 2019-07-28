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
#include "array.h"
#include "Str.h"
#include "ncHash.h"
#include "ncunittest.h"
void str_unittest(void)
{
    TEST_START("str_unittest"); 
    {
        char tmp[1024];
        char *base = "12345";
        char *str = NULL;
        int i,n;
        // NULL checks
        UTASSERT(0==Str_len(&str));
        Str_destroy(&str); // shouldn't crash

        UTASSERT(5==Str_printf(&str,base));
        UTASSERT(5 == Str_len(&str));
        UTASSERT(0 == strcmp(base,str));
        n = Str_printf(&str,"%s",base);
        UTASSERT(5 == n);
        UTASSERT(5 == Str_len(&str));
        UTASSERT(0 == strcmp(base,str));
        *tmp = 0;
        Str_clear(&str);
        for( i = 0; i < 100; ++i ) 
        {
            strcat_s(tmp,ARRAY_SIZE(tmp)-strlen(tmp),base);
            n = Str_catf(&str,"%s", base);
            UTASSERT(5 == n);
            UTASSERT(5*(i+1) ==  Str_len(&str));
            UTASSERT(0 == strcmp(tmp,str));
        }
    }
    // Str_strtok
    {
        char str[] ="- This, a sample string.";
        char *res[] = {
            "This",
            "a",
            "sample",
            "string",
        };
        char *iter = str;
        char * pch = NULL;
        int i;
        for(i = 0;Str_tok(&pch,&iter," ,.-");i++)
        {
            UTASSERT(0==strcmp(pch,res[i]));
        }
    }
    // Str_rm
    {
        char *t = NULL;
        Str_printf(&t,"0123456789");
        Str_destroy(&t);
        UTASSERT(t == NULL);
        Str_printf(&t,"0123456789");
        Str_rm(&t,4,3); 
        UTASSERT(0==strcmp(t,"0123789"));

        Str_printf(&t,"0123456789");
        Str_rm(&t,400000,1);
        UTASSERT(0==strcmp(t,"0123456789"));

        Str_printf(&t,"0123456789");
        Str_rm(&t,4,30000);
        UTASSERT(0==strcmp(t,"0123"));

        Str_printf(&t,"0123456789");
        Str_rm(&t,4,-3);
        UTASSERT(0==strcmp(t,"0123456789"));

        Str_printf(&t,"0123456789");
        Str_rm(&t,-4,3);
        UTASSERT(0==strcmp(t,"0123456789"));

        Str_destroy(&t);
    }
    // Str_cp
    {
        char *t;
        char src[] = {'a','b','c',0,'d'}; 
        Str_cp_raw(&t,src);
        UTASSERT(0==strcmp(t,"abc"));
        Str_destroy(&t);
    }
    TEST_END;
}
