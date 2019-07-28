/**
 *  copyright:  (c) 2004, NCsoft. All Rights Reserved
 *  author(s):  Peter Freese
 *  purpose:    
 *  modified:   $DateTime: 2005/11/17 10:07:52 $
 *              $Author: cthurow $
 *              $Change: 178508 $
 *              @file
 */

#include "arda2/core/corFirst.h"
#include "arda2/test/tstUnit.h"
#include <stdio.h>

tstUnit* tstUnit::sm_listHead = NULL;

utlStringTable  tstUnit::sm_stringTable;


void tstUnit::SetName( const char * szName )
{
    // TODO: strip path info from name if added (via macro usage)
    m_nameId = sm_stringTable.AddString(szName);
}


void tstUnit::AddDependency( const char * szName )
{
    // TODO: strip path info from name if added (via macro usage)
    utlStringId id = sm_stringTable.AddString(szName);
    m_dependencyIds.push_back(id);
}


/*
void tstUnit::AddTestCase( const char *szName, TestCasePointerType pmfTestCase )
{
    TestCase testCase;
    testCase.memberFunctionPointer = pmfTestCase;
    testCase.szName = szName;
    m_testCases.push_back(testCase);
}
*/
