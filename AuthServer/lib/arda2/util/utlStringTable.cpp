/**
    copyright:  (c) 2003, NCsoft. All Rights Reserved
    author(s):  Peter Freese
    purpose:    Map global strings to dynamic identifiers for fast comparison.
    modified:   $DateTime: 2007/12/12 10:43:51 $
                $Author: pflesher $
                $Change: 700168 $
                @file
**/

#include "arda2/core/corFirst.h"
#include "arda2/core/corStlAlgorithm.h"
#include "arda2/util/utlStringTable.h"

const utlStringId utlStringId::ID_NONE;
char *utlStringTable::s_nullString = "";

utlStringTable g_stringTable;

using namespace std;

inline int CharStringCompare(const char *lhs, const char *rhs)
{
    // changing this function will change the behavior of the string table
    // ideally, this should be some sort of functor template parameter
    return _stricmp(lhs, rhs);  
}


inline bool CharStringCompareLess(const char *lhs, const char *rhs)
{
    return CharStringCompare(lhs, rhs) < 0;  
}


// Size of initial heap in bytes (will allocate more blocks of this size as needed)
static const int iInitialHeapBytes = 1024*4;

utlStringTable::utlStringTable()
{
    m_heap.AllocateHeap(iInitialHeapBytes, false);
}


utlStringTable::~utlStringTable()
{
    m_strings.clear(); // no delete! heap owns memory
    m_heap.Release();
}


/**
 *  Get the utlStringId for a particular string. If the string
 *  is not found in the table, it is added
 *  
 *  @param  pString: the string to look for
 *   
 *  @return the id of the found string
 */
utlStringId utlStringTable::AddString( const char *pString )
{
    if (pString == NULL || *pString == '\0' )
        return utlStringId::ID_NONE;

    StringVec::iterator it;
    if ( !LookupString(pString, it) )
    {
        // insert a new copy into the vector at the appropriate location
        const char *pNewString = m_heap.AddString(pString);
        it = m_strings.insert(it, pNewString);
    }
    return utlStringId(*it);
}


/**
 *  Get the utlStringId for a particular string. If the string
 *  is not found in the table, it will be equivalent to utlStringId::ID_NONE
 *  
 *  @param  pString: the string to look for
 *   
 *  @return the id of the found string
 */
utlStringId utlStringTable::GetId( const char *pString ) const
{
    if (pString == NULL || *pString == '\0' )
        return utlStringId::ID_NONE;

    StringVec::iterator it;
    if ( !LookupString(pString, it) )
        return utlStringId(NULL);
    return utlStringId(*it);
}


/**
 *  Lookup a string in the table, returning true and an iterator to the table
 *  entry if found. If the string is not found, the function returns false, and
 *  the iterator will be set to the correction insertion point.
 *
 *  @param pString:     String to find/add. If the string is added, it is copied to a new allocation
 *  @return it:         lower_bound result for string
 */
bool utlStringTable::LookupString( IN const char *pString, OUT StringVec::iterator& it ) const
{
    errAssert(pString != NULL && *pString != '\0' );

    // first search for the string in the table
    it  = force_cast<StringVec::iterator>(lower_bound(m_strings.begin(), m_strings.end(), pString, CharStringCompareLess));

    // test whether the string was actually found
    return ( it != m_strings.end() && CharStringCompare(*it, pString) == 0 );
}


void utlStringTable::Clear()
{
    m_strings.clear(); // no delete! heap owns memory

    // Dump string heap all at once...
    m_heap.Release();
    m_heap.AllocateHeap(iInitialHeapBytes, false);
}


#if BUILD_UNIT_TESTS

#include "arda2/test/tstUnit.h"

class utlStringTableTests : public tstUnit
{
private:
	utlStringTable	m_table;
	utlStringId		m_id0;
	utlStringId		m_id1;
	utlStringId		m_id2;
	utlStringId		m_id3;
	utlStringId		m_id4;
public:
	utlStringTableTests()
	{
	}

	virtual void Register()
	{
		SetName("utlStringTable");

		AddTestCase("utlStringTable::AddString()", &utlStringTableTests::TestAddString);
		AddTestCase("utlStringTable::GetId()", &utlStringTableTests::TestGetId);
		AddTestCase("utlStringTable::GetString()", &utlStringTableTests::TestGetString);
		AddTestCase("utlStringTable::Clear()", &utlStringTableTests::TestClear);
	};

	virtual void TestCaseSetup()
	{
		m_id0 = m_table.AddString("test0");
		m_id1 = m_table.AddString("test1");
		m_id2 = m_table.AddString("test2");
		m_id3 = m_table.AddString("test3");
		m_id4 = m_table.AddString("test4");
	}

	virtual void TestCaseTearDown()
	{
		m_table.Clear();
	}

	void TestAddString() const
	{
		utlStringTable table;

		utlStringId	id0 = table.AddString("test0");
		TESTASSERT(id0.IsValid());

		utlStringId	id1 = table.AddString("test1");
		TESTASSERT(id1.IsValid());

		TESTASSERT(id0 != id1);

		utlStringId id2 = table.AddString("test0");
		TESTASSERT(id2 == id0);

		utlStringId id3 = table.AddString("TEST1");
		TESTASSERT(id3 == id1);

		utlStringId id4 = table.AddString("");
		TESTASSERT(id4 == utlStringId::ID_NONE);

		utlStringId id5 = table.AddString(NULL);
		TESTASSERT(id5 == utlStringId::ID_NONE);
	}

	void TestGetId() const
	{
		TESTASSERT(m_table.GetId("test0") == m_id0);
		TESTASSERT(m_table.GetId("test1") == m_id1);
		TESTASSERT(m_table.GetId("test2") == m_id2);
		TESTASSERT(m_table.GetId("tEsT0") == m_id0);
		TESTASSERT(m_table.GetId("TEST10") == utlStringId::ID_NONE);
		TESTASSERT(m_table.GetId("") == utlStringId::ID_NONE);
		TESTASSERT(m_table.GetId(NULL) == utlStringId::ID_NONE);
	}

	void TestGetString() const
	{
		TESTASSERT(strcmp(m_table.GetString(m_id0), "test0") == 0);
		TESTASSERT(strcmp(m_table.GetString(m_id1), "test1") == 0);
		TESTASSERT(strcmp(m_table.GetString(m_id2), "test2") == 0);
	}

	void TestClear() const
	{
		utlStringTable table;

		table.AddString("test0");
		table.AddString("test1");
		table.AddString("test0");

		table.Clear();
		utlStringId id0 = table.GetId("test0");
		TESTASSERT(id0 == utlStringId::ID_NONE);
	}
};

EXPORTUNITTESTOBJECT(utlStringTableTests);

#endif // BUILD_UNIT_TESTS

