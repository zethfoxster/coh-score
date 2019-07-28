/**
 *  copyright:  (c) 2004, NCsoft. All Rights Reserved
 *  author(s):  Peter Freese
 *  purpose:    
 *  modified:   $DateTime: 2007/12/12 10:43:51 $
 *              $Author: pflesher $
 *              $Change: 700168 $
 *              @file
 */


#include "arda2/core/corFirst.h"
#include "arda2/util/utlStringTable.h"
#include "arda2/core/corStlSet.h"
#include "arda2/core/corStlMap.h"

class tstUnit;

class tstFramework
{
public:
    tstFramework() {};
    virtual ~tstFramework() {};

    void EnumerateUnits();
    void BuildDependencyList( const char *szUnit = NULL );
    void DumpUnits();
    int RunTests();

    enum UnitTestFormat
    {
        UTF_Normal,
        UTF_Pass,
        UTF_Fail,
        UTF_Warn,
        UTF_Heading1,
    };

    virtual void SetFormat( UnitTestFormat format );
    virtual void WriteText( const char * );

    void FormatText( UnitTestFormat format, const char* fmt, ... );

protected:
    void AddUnitToTestList( tstUnit * );

private:
    typedef std::map<utlStringId, tstUnit*> IdTestMap;
    IdTestMap               m_unitTests;

    std::vector<tstUnit*>        m_orderUnitList;
    std::set<const tstUnit*>     m_unitsAdded;
    std::set<const tstUnit*>     m_unitsBeingAdded;
};
