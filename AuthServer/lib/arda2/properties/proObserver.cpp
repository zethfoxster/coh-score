#include "arda2/core/corFirst.h"

#include "proFirst.h"

#include "proObject.h"
#include "proPropertyOwner.h"
#include "proClassNative.h"
#include "proPropertyNative.h"
#include "proClass.h"
#include "proProperty.h"
#include "proObserver.h"

using namespace std;

int proObserver::Observe(proObject *obj)
{
    int token = m_nextToken++;

    obj->AddObserver(this, token);
    return token;
}

void proObserver::Unobserve(proObject *obj, int token)
{
    obj->RemoveObserver(this, token);
}


proObjectReference::proObjectReference() :
    m_object( 0 ) 
{
}

proObjectReference::proObjectReference( proObject* inObject )
{
    SetObject( inObject );
}

proObjectReference::proObjectReference( const proObjectReference& inSource ) :
  proObserver(inSource)
{
    SetObject( inSource.m_object );
}

proObjectReference& proObjectReference::operator=( const proObjectReference& inSource )
{
    if ( &inSource != this )
    {
        ReleaseObject();

        SetObject( inSource.m_object );
    }

    return *this;
}

proObject* proObjectReference::operator=( proObject* inObject )
{
    if ( inObject != m_object )
    {
        ReleaseObject();
        SetObject( inObject );
    }

    return m_object;
}

proObject* proObjectReference::operator*( void )
{
    return m_object;
}

void proObjectReference::ReleaseObject( void )
{
    if ( m_object )
    {
        Unobserve( m_object, m_token );
    }

    m_object = 0;
    m_token = proTokenUndefined;
}

void proObjectReference::SetObject( proObject* inObject )
{
    if ( inObject )
    {
        m_token = Observe( inObject );
    }

    m_object = inObject;
}

void proObjectReference::Signal( proObject* inObject, const int inToken,
    const int inEventType, const string& inPropertyName )
{
    UNREF( inToken );
    UNREF( inPropertyName );

    if ( inEventType == kDeleted && inObject == m_object )
    {
        Unobserve( inObject, m_token );

        m_object = 0;
        m_token = proTokenUndefined;
    }
}



#if BUILD_UNIT_TESTS

#include "arda2/test/tstUnit.h"

typedef pair<proObject*,int> proObjectToken;

static const int defaultInt       = 7711;
static const uint defaultUint       = 11234;
static const float defaultFloat   = 0.55113f;
static const string defaultString = "default";
static const bool defaultBool     = true;

/**
 * test class for proObject tests
*/
class proObserverTargetTest : public proObject
{
PRO_DECLARE_OBJECT

public:
    proObserverTargetTest() : 
        m_x(defaultInt), 
        m_size(defaultUint), 
        m_ratio(defaultFloat),
        m_name(defaultString), 
        m_flag(defaultBool),
        m_number(0)
     {
     }

    virtual ~proObserverTargetTest() {}

    int GetX() const { return m_x;}
    uint GetSize() const { return m_size;}
    float GetRatio() const { return m_ratio;}
    string GetName() const { return m_name;}
    bool GetFlag() const { return m_flag;}
    uint GetNumber() const { return m_number;}

    void SetX(int value) { m_x = value; }
    void SetSize(int value) { m_size = value; }
    void SetRatio(float value) { m_ratio = value; }
    void SetName(const string &value) { m_name = value; }
    void SetFlag(bool value) { m_flag = value; }
    void SetNumber(uint value) { m_number = value; }

protected:
    int m_x;
    uint m_size;
    float m_ratio;
    string m_name;
    bool m_flag;
    uint m_number;

};

PRO_REGISTER_CLASS(proObserverTargetTest, proObject)
REG_PROPERTY_INT(proObserverTargetTest, X)
REG_PROPERTY_UINT(proObserverTargetTest, Size)
REG_PROPERTY_FLOAT(proObserverTargetTest, Ratio)
REG_PROPERTY_STRING(proObserverTargetTest, Name)
REG_PROPERTY_BOOL(proObserverTargetTest, Flag)
REG_PROPERTY_ENUM(proObserverTargetTest, Number, "zero,0,one,1,two,2,three,3,four,4,five,5")


/*
 * test observer class
*/
class proTestObserver : public proObserver
{
public:
    proTestObserver() : m_changeCount(0), m_addCount(0), m_removeCount(0) {}
    ~proTestObserver() {}

    void Signal(proObject *obj, const int token, const int eventType, const string &propName)
    {
        UNREF(token);
        switch (eventType)
        {
        case kPropertyChanged:
            {
                m_changeCount++;
                proProperty *pro = obj->GetPropertyByName(propName);
                pro->Clone(obj, &m_copy);
            }
            break;

        case kPropertyAdded:
            {
                proProperty *pro = obj->GetPropertyByName(propName);
                pro->Clone(obj, &m_copy);
            }
            break;

        case kPropertyRemoved:
            break;

        case kPropertyNameChanged:
            break;

        case kDeleted:
            {
                for (vector<proObjectToken>::iterator it =m_objects.begin(); it != m_objects.end(); it++)
                {
                    proObjectToken &t = (*it);
                    if ( t.first == obj && t.second == token)
                    {
                        Unobserve(obj, token);
                        m_objects.erase(it);
                        break;
                    }
                }
            }
            break;
        }
    }


    int m_changeCount;
    int m_addCount;
    int m_removeCount;
    proObserverTargetTest m_copy;

    vector<proObjectToken>  m_objects;
};

class proTestChildObserver : public proTestObserver, public proObject
{
    PRO_DECLARE_OBJECT

public:
    proTestChildObserver() { };
    virtual ~proTestChildObserver()
    {
        // walk through the list of kids and unobserve them
        for ( vector< proObjectToken >::iterator current = m_objects.begin(),
            stop = m_objects.end(); current != stop; ++current )
        {
            Unobserve( current->first, current->second );
        }
    }

    virtual int Observe( proObject* inObject )
    {
        // if the object is real, push it into the
        // m_observers list and then observe it
        if ( inObject )
        {
            int newToken = proObserver::Observe( inObject );
            m_objects.push_back( make_pair( inObject, newToken) );

            return newToken;
        }

        return -1;
    }

protected:

private:
};

PRO_REGISTER_CLASS( proTestChildObserver, proObject )

class proObserverTests : public tstUnit
{
private:
    proObserverTargetTest *m_object;
    proTestObserver *m_observer;

public:
    proObserverTests()
    {
    }

    virtual void Register()
    {
        SetName("proObserver");
        AddDependency("proObject");

        AddTestCase("testPropChanged", &proObserverTests::testPropChanged);
        AddTestCase("testPropAddRemove", &proObserverTests::testPropAddRemove);
        AddTestCase("testObserveChildren", &proObserverTests::testObserveChildren);

    };

    virtual void TestCaseSetup()
    {
        m_object = new proObserverTargetTest();
        m_observer = new proTestObserver();

        // is removed on deletion
        int t = m_observer->Observe(m_object);
        m_observer->m_objects.push_back( make_pair(m_object, t) );

    }
    virtual void TestCaseTeardown()
    {
        SafeDelete(m_object);
        SafeDelete(m_observer);
    }

    void testPropChanged() const
    {
        proProperty *pro = NULL;
        
        pro = m_object->GetPropertyByName("X");
        pro->FromString(m_object, "1000");
        TESTASSERT(m_observer->m_changeCount == 1);
        TESTASSERT(pro->ToString( &(m_observer->m_copy)) == "1000");

        pro = m_object->GetPropertyByName("Name");
        pro->FromString(m_object, "test");
        TESTASSERT(m_observer->m_changeCount == 2);
        TESTASSERT(pro->ToString( &(m_observer->m_copy)) == "test");

    }

    //TODO - write a multi-observer test

    /**
     * TODO - observation does not work for adding / removing non-object properties!!
     *
     */
    void testPropAddRemove() const
    {
        proTestObserver *observer = new proTestObserver;

        proObserverTargetTest *obj1 = new proObserverTargetTest;

        int t1 = observer->Observe(obj1);
        observer->m_objects.push_back( make_pair(obj1, t1) );

        int t2 = observer->Observe(obj1);
        observer->m_objects.push_back( make_pair(obj1, t2) );


        SafeDelete(obj1);

        TESTASSERT(observer->m_objects.size() == 0);
        SafeDelete(observer);

    }

    void testObserveChildren() const
    {
        // build the parent
        proTestChildObserver* observer = new proTestChildObserver;

        // build some kids to look at
        proObserverTargetTest *kid1 =    new proObserverTargetTest;
        proObserverTargetTest *kid2 =    new proObserverTargetTest;

        // test 1:  observe the kids, kill the parent, will not crash
        observer->Observe( kid1 );
        observer->Observe( kid2 );
        SafeDelete( observer );
        SafeDelete( kid1 );
        SafeDelete( kid2 );

        // test 2:  add kids as kids, observe kids, then delete the parent
        observer = new proTestChildObserver;
        kid1 =     new proObserverTargetTest;
        kid2 =     new proObserverTargetTest;
        observer->AddChild( kid1, "kid1" );
        observer->AddChild( kid2, "kid2" );
        observer->Observe( kid1 );
        observer->Observe( kid2 );
        SafeDelete( observer );
    }
};

EXPORTUNITTESTOBJECT(proObserverTests);


#endif
