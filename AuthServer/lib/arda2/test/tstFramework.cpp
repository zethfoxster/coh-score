/**
 *  copyright:  (c) 2004, NCsoft. All Rights Reserved
 *  author(s):  Peter Freese
 *  purpose:    
 *  modified:   $DateTime: 2007/12/12 10:43:51 $
 *              $Author: pflesher $
 *              $Change: 700168 $
 *              @file
 */


#include "arda2/test/tstFramework.h"
#include "arda2/test/tstUnit.h"
#include <stdio.h>
#include <stdarg.h>

using namespace std;

void tstFramework::SetFormat( UnitTestFormat format )
{
    UNREF(format);
}


void tstFramework::WriteText( const char *szText )
{
    printf("%s", szText);
}


void tstFramework::FormatText( UnitTestFormat format, const char* fmt, ...)
{
    SetFormat(format);

    char buffer[1024];

    // get varargs stuff
    va_list args;
    va_start(args, fmt);

    // format to stdout
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    buffer[sizeof(buffer) - 1] = 0;

    // end varargs
    va_end(args);

    WriteText(buffer);
}


void tstFramework::EnumerateUnits()
{
    tstUnit *pTest = tstUnit::sm_listHead;
    while ( pTest != NULL )
    {
        pTest->Register();

        utlStringId id = pTest->GetNameId();

        if ( m_unitTests.find(id) != m_unitTests.end() )
        {
            ERR_REPORTV(ES_Fatal, ("Unit test name collision: %s", pTest->GetNameString()));
        }
        m_unitTests[id] = pTest;
        pTest = pTest->m_next;
    }
}


void tstFramework::AddUnitToTestList( tstUnit *pUnit )
{
    // if unit already added, ignore
    if ( m_unitsAdded.find(pUnit) != m_unitsAdded.end() )
        return;

    // if unit is in the process of being added, circular dependency check
    if ( m_unitsBeingAdded.find(pUnit) != m_unitsBeingAdded.end() )
    {
        ERR_REPORTV(ES_Fatal, ("Circular dependency detected: %s", pUnit->GetNameString()));
    }

    m_unitsBeingAdded.insert(pUnit);

    // add this units dependencies
    const tstUnit::NameIdVec& dependencyIds = pUnit->GetDependencies();
    for ( tstUnit::NameIdVec::const_iterator it = dependencyIds.begin(); it != dependencyIds.end(); ++it )
    {
        utlStringId id = *it;

        // look up test in map
        IdTestMap::iterator mapIt = m_unitTests.find(id);
        if ( mapIt == m_unitTests.end())
            ERR_REPORTV(ES_Fatal, ("Could not find id for unit test %s in map", tstUnit::sm_stringTable.GetString(id) ));

        tstUnit *pDependency = mapIt->second;

        AddUnitToTestList(pDependency);
    }

    // remove from the circular dependency guard set
    m_unitsBeingAdded.erase(pUnit);

    // add it to the ordered vector
    m_orderUnitList.push_back(pUnit);

    // add it to the completed set
    m_unitsAdded.insert(pUnit);
}


/**
*  Build an order list of unit tests to run, based on dependency information
*/
void tstFramework::BuildDependencyList( const char * szUnit )
{
    if ( szUnit != NULL )
    {
        utlStringId id = tstUnit::sm_stringTable.GetId(szUnit);
        if ( !id.IsValid() )
            return;

        // look up test in map
        IdTestMap::iterator mapIt = m_unitTests.find(id);
        if ( mapIt == m_unitTests.end())
        {
            ERR_REPORTV(ES_Fatal, ("Could not find id for unit test %s in map", tstUnit::sm_stringTable.GetString(id) ));
            return;
        }

        AddUnitToTestList(mapIt->second);
    }
    else
    {
        // add all units
        for ( IdTestMap::iterator it = m_unitTests.begin(); it != m_unitTests.end(); ++it )
        {
            AddUnitToTestList(it->second);
        }
    }
}


void tstFramework::DumpUnits()
{
    for ( vector<tstUnit*>::iterator it = m_orderUnitList.begin(); it != m_orderUnitList.end(); ++it )
    {
        tstUnit *pUnit = *it;
        puts(pUnit->GetNameString());
    }
}


int tstFramework::RunTests()
{
    int iUnitRun = 0, iUnitFail = 0;
    for ( vector<tstUnit*>::iterator it = m_orderUnitList.begin(); it != m_orderUnitList.end(); ++it )
    {
        tstUnit *pUnit = *it;

        const tstUnit::TestCaseVec& testCaseVec = pUnit->GetTestCases();
        FormatText(UTF_Heading1, "Running unit test %s (%i test case%s)\n", 
            pUnit->GetNameString(),
            testCaseVec.size(), 
            testCaseVec.size() != 1 ? "s" : "");

        // verify this units dependencies have succeeded
        const tstUnit::NameIdVec& dependencyIds = pUnit->GetDependencies();
        for ( tstUnit::NameIdVec::const_iterator it = dependencyIds.begin(); it != dependencyIds.end(); ++it )
        {
            utlStringId id = *it;

            // look up test in map
            IdTestMap::iterator mapIt = m_unitTests.find(id);
            if ( mapIt == m_unitTests.end())
                ERR_REPORTV(ES_Fatal, ("Could not find id for unit test %s in map", tstUnit::sm_stringTable.GetString(id) ));

            tstUnit *pDependency = mapIt->second;

            if ( pDependency->m_bFailed )
            {
                FormatText(UTF_Warn, "Unit test dependency %s failed, results unreliable\n", pDependency->GetNameString());
            }
        }

        try
        {
            pUnit->UnitTestSetup();
        }
        catch ( ... )
        {
            // bail on the test
            FormatText(UTF_Fail, "Unit test setup failed on %s, skipping\n", pUnit->GetNameString());
            continue;
        }

        int iTestRun = 0, iTestFail = 0;
        for ( tstUnit::TestCaseVec::const_iterator it = testCaseVec.begin(); it != testCaseVec.end(); ++it )
        {
            const tstUnit::TestCase& testCase = *it;

            try
            {
                pUnit->TestCaseSetup();
            }
            catch ( ... )
            {
                // bail on the test
                FormatText(UTF_Fail, "Test case setup failed on unit %s, test cases unreliable\n", pUnit->GetNameString());
            }

            FormatText(UTF_Normal, "  %2i: %s, ", iTestRun, testCase.szName);

            bool bException = false;
            try
            {
                (pUnit->*testCase.memberFunctionPointer)();
            }
            catch ( ... )
            {
                bException = true;
            }

            if ( bException || !pUnit->m_failureRecords.empty() )
            {
                FormatText(UTF_Fail, "FAILED\n");
                ++iTestFail;
            }
            else
            {
                FormatText(UTF_Pass, "SUCCEEDED\n");
            }

            if ( !pUnit->m_failureRecords.empty() )
            {
                for ( tstUnit::FailureVec::iterator it = pUnit->m_failureRecords.begin();
                    it != pUnit->m_failureRecords.end();
                    ++it )
                {
                    tstUnit::FailureRecord& record = *it;
                    FormatText(UTF_Fail, "    %s(%i): %s\n", 
                        record.szFileName, record.iLineNumber, record.szMsg);
                }
                pUnit->m_failureRecords.clear();
                pUnit->m_bFailed = true;
            }

            if ( bException )
            {
                FormatText(UTF_Fail, "    Exception thrown.\n");
            }

            try
            {
                pUnit->TestCaseTeardown();
            }
            catch ( ... )
            {
                // bail on the test
                FormatText(UTF_Fail, "Test case teardown failed on unit %s, subsequent test cases unreliable\n", pUnit->GetNameString());
            }

            ++iTestRun;
        }

        try
        {
            pUnit->UnitTestTeardown();
        }
        catch ( ... )
        {
            FormatText(UTF_Fail, "Unit test teardown failed on %s\n", pUnit->GetNameString());
        }

        if ( iTestFail > 0)
        {
            FormatText(UTF_Fail, "%i tests run; %i failures\n\n", iTestRun, iTestFail);
        }
        else
        {
            FormatText(UTF_Pass, "%i tests run; all passed\n\n", iTestRun);
        }

        ++iUnitRun;
        if ( iTestFail )
            ++iUnitFail;
    }

    // output total test results
    FormatText(UTF_Heading1, "Complete results:\n");
    if ( iUnitFail > 0)
    {
        FormatText(UTF_Fail, "%i unit tests run; %i had failures\n", iUnitRun, iUnitFail);
    }
    else
    {
        FormatText(UTF_Pass, "%i units successfully run\n", iUnitRun);
    }

    // return color to default and skip a line
    FormatText(UTF_Normal, "\n");

    return iUnitFail;
}
