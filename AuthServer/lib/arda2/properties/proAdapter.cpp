#include "arda2/core/corFirst.h"
#include "proFirst.h"

#include "proAdapter.h"
#include "proClassNative.h"

PRO_REGISTER_ABSTRACT_CLASS( proAdapterShared, proObject)

using namespace std;

/*
 *  UNIT TESTS
 */
#if BUILD_UNIT_TESTS

#include "arda2/test/tstUnit.h"
#include "arda2/properties/proClassNative.h"
#include "arda2/properties/proPropertyNative.h"

static const int defaultInt       = 7711;
static const uint defaultUint       = 11234;
static const float defaultFloat   = 0.55113f;
static const string defaultString = "default";
static const bool defaultBool     = true;

static const string newName("foobar");
static const int newX = 51;
static const uint newSize = 99999;
static const float newRatio = 0.4f;

/**
 * test classes for proAdapter tests
*/

class proNonObject
{
public:
    proNonObject() : m_int(defaultInt), m_string(defaultString)
    {
    }

    int GetInt() const  { return m_int; }
    string GetString() const { return m_string; }

    void SetInt(int value) { m_int = value; }
    void SetString(const string &value) { m_string = value; }

protected:
    int m_int;
    string m_string;
};


class proNonObjectAdapter : public proAdapter<proNonObject>
{
    PRO_DECLARE_OBJECT
public:
    proNonObjectAdapter()
    {
    }

    void Initialize()
    {
        if ( !IsInitialized() )
        {
            MyType::Initialize();

            proObject *child = new proObject();
            AddChild(child, "Child");
        }
    }

    int GetInt() const { return m_target->GetInt(); }
    string GetString() const { return m_target->GetString(); }

    void SetInt(int value) { m_target->SetInt(value); }
    void SetString(string value) { m_target->SetString(value); }
};

PRO_REGISTER_ADAPTER_CLASS(proNonObjectAdapter, proNonObject)
REG_PROPERTY_INT(proNonObjectAdapter, Int)
REG_PROPERTY_STRING(proNonObjectAdapter, String)


class proAdapterTests : public tstUnit
{
private:
    proNonObject *m_object;
    proNonObjectAdapter *m_adapter;

public:
    proAdapterTests()
    {
    }

    virtual void Register()
    {
        SetName("proAdapterTests");

        proClassRegistry::Init();

        AddTestCase("testBasicTypes", &proAdapterTests::testBasicTypes);
        AddTestCase("testRefs", &proAdapterTests::testRefs);
        AddTestCase("testInitialization", &proAdapterTests::testInitialization);
        AddTestCase("testClone", &proAdapterTests::testClone);

    };

    virtual void TestCaseSetup()
    {
        m_object = new proNonObject();
        m_adapter = new proNonObjectAdapter();
        m_adapter->SetTarget(m_object);
    }

    virtual void TestCaseTeardown()
    {
        SafeDelete(m_object);
        SafeDelete(m_adapter);
    }

    void testBasicTypes() const
    {
        TESTASSERT( m_adapter->GetInt() == defaultInt);
        TESTASSERT( m_adapter->GetString() == defaultString);
    }

    void testRefs() const
    {
        proNonObject *nonObject = new proNonObject();

        proNonObjectAdapter *ownerAdapter = new proNonObjectAdapter();
        ownerAdapter->SetTarget(nonObject, PA_FULL_INIT, PA_OWN);

        proNonObjectAdapter *refAdapter = new proNonObjectAdapter();
        refAdapter->SetTarget(nonObject, PA_FULL_INIT, PA_REF);

        // object should NOT be deleted
        SafeDelete(refAdapter);

        // confirm object still works
        TESTASSERT(nonObject->GetInt() != 0);

        // should cause the object to be deleted too
        SafeDelete(ownerAdapter);

    }

    void testInitialization() const
    {
        vector<proObject*> children;

        proNonObject *nonObject = new proNonObject();
        proNonObjectAdapter *adapter = new proNonObjectAdapter();

        TESTASSERT(adapter->IsInitialized() == false);

        // test we have no children
        adapter->GetChildren(children);
        TESTASSERT(children.size() == 0);

        // make adapter initialize
        adapter->SetTarget(nonObject, PA_FULL_INIT, PA_OWN);
        TESTASSERT(adapter->IsInitialized() == true);

        // test for one child
        adapter->GetChildren(children);
        TESTASSERT(children.size() == 1);

        SafeDelete(adapter);
    }

    void testClone() const
    {
        proNonObject *nonObject = new proNonObject();
        proNonObjectAdapter *adapter = new proNonObjectAdapter();
        adapter->SetTarget(nonObject, PA_FULL_INIT, PA_OWN);

        proNonObjectAdapter *adapterClone = (proNonObjectAdapter*)adapter->Clone();
        TESTASSERT(adapterClone != NULL);
        TESTASSERT(adapterClone->GetTarget() == NULL);

        SafeDelete(adapter);
        SafeDelete(adapterClone);
    }

};

EXPORTUNITTESTOBJECT(proAdapterTests);


#endif
