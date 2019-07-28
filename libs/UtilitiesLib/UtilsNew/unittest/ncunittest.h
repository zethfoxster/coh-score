/***************************************************************************
 *     Copyright (c) 2008-2008, NCSoft
 *     All Rights Reserved
 *     Confidential Property of NCSoft
 *
 * Module Description:
 *
 *
 ***************************************************************************/
#ifndef NCUNITTEST_H
#define NCUNITTEST_H

#include "../ncstd.h"
NCHEADER_BEGIN

// @todo -AB: no unittest should leak memory. auto check :08/24/08
#define LOGF(...) printf(__VA_ARGS__)
#define TEST_START(TEST) LOGF("starting test %s\n", TEST);   \
    ClearStruct(&g_testsuite);                          \
    g_testsuite.test = TEST;
#define TEST_END 
//LOGF("finished with test %s\n",g_testsuite.test);

typedef struct TestSuite
{
    char *test;
    int num_tests;
    int num_failed;
} TestSuite;
extern TestSuite g_testsuite;

#define UTASSERT(cond)  do {                       \
        g_testsuite.num_tests++;                 \
if (!(cond)) {                                \
    LOGF("Test failed: " #cond "\n");         \
    g_testsuite.num_failed++;                   \
    _DbgBreak();                                \
  }                                             \
} while (0)

#define UTWAITFOR(cond,retries) do{ \
        int zz_tmpvar_zz; \
        for( zz_tmpvar_zz = 0;; ++zz_tmpvar_zz )    \
        {                                           \
            if(cond)                                \
                break;                              \
            if(zz_tmpvar_zz == retries) {           \
                UTASSERT(cond);                     \
                break;                              \
            }                                       \
            Sleep(1);                               \
        }                                           \
    } while(0)
        
#define UTEXPECT(cond)  do {                       \
        g_testsuite.num_tests++;                 \
  if (!(cond)) {                                \
    LOGF("Test failed: " #cond "\n");         \
    g_testsuite.num_failed++;                   \
  }                                             \
} while (0)

#define UTERR(...)  do {                         \
        g_testsuite.num_tests++;                 \
        LOGF(__VA_ARGS__);                       \
        g_testsuite.num_failed++;                \
        _DbgBreak();                                 \
    } while (0)

#define UTWARN(...)  do {                          \
        g_testsuite.num_tests++;                 \
        LOGF(__VA_ARGS__);                       \
        g_testsuite.num_failed++;                \
        exit(1);                                 \
    } while (0)

#define UTMSG(...)  do {                          \
        LOGF(__VA_ARGS__);                       \
    } while (0)

#define UTSUMMARY()  do {                     \
        LOGF("Tests Done. %i tests %i failed",g_testsuite.num_tests,g_testsuite.num_failed); \
    } while (0)

void ncunittest_run(void);


NCHEADER_END
#endif //NCUNITTEST_H
