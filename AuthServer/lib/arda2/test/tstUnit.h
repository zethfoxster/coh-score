/**
*  copyright:  (c) 2004, NCsoft. All Rights Reserved
*  author(s):  Peter Freese
*  purpose:    
*  modified:   $DateTime: 2007/12/12 10:43:51 $
*              $Author: pflesher $
*              $Change: 700168 $
*              @file
*/

#ifndef   INCLUDED_tstUnit
#define   INCLUDED_tstUnit
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "arda2/core/corFirst.h"
#include "arda2/util/utlStringTable.h"

#pragma deprecated("DECLAREUNITTESTOBJECT")	// this macro should no longer be used
#define DECLAREUNITTESTOBJECT(classname)	// use EXPORTUNITTESTOBJECT() instead

#define EXPORTUNITTESTOBJECT(classname) \
    classname classname##_Object; \
    void * classname##_ObjSym = &classname##_Object;

#define IMPORTUNITTESTOBJECT(classname) \
    extern void *classname##_ObjSym; \
    void *classname##_ObjSym_ = classname##_ObjSym;

#define ADDTESTCASE(n) AddTestCase(#n, n)

#define TEST_SOURCEANNOTATION

#ifdef TEST_SOURCEANNOTATION

#define TESTASSERTMSG(condition, msg)\
    (this->AssertImplementation ((condition), (#condition " : " msg), \
    __LINE__, __FILE__))

#define TESTASSERT(condition)\
    (this->AssertImplementation ((condition), (#condition), \
    __LINE__, __FILE__))

#else

#define TESTASSERTMSG(condition, msg)\
    (this->AssertImplementation ((condition), (msg), \
    __LINE__, __FILE__))

#define TESTASSERT(condition)\
    (this->AssertImplementation ((condition),"",\
    __LINE__, __FILE__))

#endif


class tstUnit
{
public:
    tstUnit() : m_bFailed(false)
    {
        // insert test case into list
        m_next = sm_listHead;
        sm_listHead = this;
    };

    virtual ~tstUnit() {};
    utlStringId GetNameId() const { return m_nameId; }
    const char *GetNameString() const { return sm_stringTable.GetString(m_nameId); }

    typedef std::vector<utlStringId> NameIdVec;
    const NameIdVec& GetDependencies() const { return m_dependencyIds; }

    typedef void (tstUnit::*TestCasePointerType)() const;
    struct TestCase
    {
        TestCasePointerType memberFunctionPointer;
        const char *szName;
    };
    typedef std::vector<TestCase> TestCaseVec;

    const TestCaseVec& GetTestCases() const { return m_testCases; }

    // Unit tests should override these virtual functions
    virtual void Register() {};
    virtual void UnitTestSetup() {};
    virtual void UnitTestTeardown() {};
    virtual void TestCaseSetup() {};
    virtual void TestCaseTeardown() {};

protected:
    void SetName( const char *szName );
    void AddDependency( const char *szName );

    template <class T>
        void AddTestCase( const char *szName, void (T::* pmfTestCase)() const )
    {
        TestCase testCase;
        testCase.memberFunctionPointer = (TestCasePointerType)pmfTestCase;
        testCase.szName = szName;
        m_testCases.push_back(testCase);
    }

    void AssertImplementation( bool bCondition, const char* msg, const int iLineNumber, const char* szFileName ) const
    {
        if ( bCondition )
            return;

        FailureRecord record;
        record.szMsg = msg;
        record.iLineNumber = iLineNumber;
        record.szFileName = szFileName;
        m_failureRecords.push_back(record);
    }

private:
    tstUnit*            m_next;
    static tstUnit*     sm_listHead;

    utlStringId         m_nameId;
    NameIdVec           m_dependencyIds;

    TestCaseVec         m_testCases;

    static utlStringTable  sm_stringTable;

    struct FailureRecord
    {
        const char*         szMsg;
        int                 iLineNumber;
        const char*         szFileName;
    };
    typedef std::vector<FailureRecord> FailureVec;
    mutable FailureVec  m_failureRecords;
    mutable bool        m_bFailed;

    friend class tstFramework;
};


#endif // INCLUDED_tstUnit
