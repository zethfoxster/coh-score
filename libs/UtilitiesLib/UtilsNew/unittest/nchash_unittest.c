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
#include "ncunittest.h"
#include "nchash.h"

void nchashstr_unittest(void)
{
    int i;
    HashStr *hs = HashStr_Create(0,TRUE);

    TEST_START("NcHashStr");
    for( i = 1; i < 10000; ++i ) 
    {
        char tmp[128];
        sprintf_s(ASTR(tmp),"%i",i);
#pragma warning(disable:4312)
        UTEXPECT(!HashStr_Find(&hs,tmp));								// doesn't exist
        UTEXPECT(((void*)i) == HashStr_Insert(&hs,tmp,(void*)i,FALSE));	// add successfullly
        UTEXPECT(HashStr_Find(&hs,tmp) == (void*)i);						// exists
        UTEXPECT(!HashStr_Insert(&hs,tmp,(void*)i,FALSE));				// fail to re-add
        UTEXPECT(HashStr_Find(&hs,tmp) == (void*)i);						// exists
        UTEXPECT(((void*)i) == HashStr_Insert(&hs,tmp,(void*)i,TRUE));	// overwrite
        UTEXPECT(HashStr_Find(&hs,tmp) == (void*)i);						// exists
        UTEXPECT(HashStr_Remove(&hs,tmp));                                // remove 
        UTEXPECT(!HashStr_Find(&hs,tmp));                         		// doesn't exists
        UTEXPECT(!HashStr_Remove(&hs,tmp));                               // remove fails
    }

	UTASSERT(g_testsuite.num_failed==0);
	HashStr_Destroy(&hs);
    TEST_END;
}

void nchashptr_unittest(void)
{
    int i;
    HashPtr *hs = NULL;
    TEST_START("NcHashPtr");
    for( i = 1; i < 10000; ++i ) 
    {
#pragma warning(disable:4312)
        void *tmp = (void*)i;
        UTEXPECT(!HashPtr_Find(&hs,tmp));								// doesn't exist
        UTEXPECT(((void*)i) == HashPtr_Insert(&hs,tmp,(void*)i,FALSE));	// add successfullly
        UTEXPECT(HashPtr_Find(&hs,tmp) == (void*)i);						// exists
        UTEXPECT(!HashPtr_Insert(&hs,tmp,(void*)i,FALSE));				// fail to re-add
        UTEXPECT(HashPtr_Find(&hs,tmp) == (void*)i);						// exists
        UTEXPECT(((void*)i) == HashPtr_Insert(&hs,tmp,(void*)i,TRUE));	// overwrite
        UTEXPECT(HashPtr_Find(&hs,tmp) == (void*)i);						// exists
        UTEXPECT(HashPtr_Remove(&hs,tmp));                                // remove 
        UTEXPECT(!HashPtr_Find(&hs,tmp));         						// doesn't exists
        UTEXPECT(!HashPtr_Remove(&hs,tmp));                               // remove fails
    }

	UTASSERT(g_testsuite.num_failed==0);
	HashPtr_Destroy(&hs);
    TEST_END;
}

void nchash_unittest(void)
{
    nchashstr_unittest();
    nchashptr_unittest();
}


