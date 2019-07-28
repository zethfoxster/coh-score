
#include "arda2/core/corFirst.h"
#include "arda2/util/utlProperty.h"

#include "arda2/properties/proObjectWriter.h"
#include "arda2/properties/proObjectReader.h"


#include "arda2/properties/proClassNative.h"

using namespace std;
PRO_REGISTER_ABSTRACT_CLASS(utlPropertyRect, proProperty)
PRO_REGISTER_ABSTRACT_CLASS(utlPropertyPoint, proProperty)

PRO_REGISTER_ABSTRACT_CLASS(utlPropertyRectNative, utlPropertyRect)
PRO_REGISTER_ABSTRACT_CLASS(utlPropertyPointNative, utlPropertyPoint)

PRO_REGISTER_CLASS(utlPropertyRectOwner, utlPropertyRect)
PRO_REGISTER_CLASS(utlPropertyPointOwner, utlPropertyPoint)


void proRegisterUtilTypes()
{
    IMPORT_PROPERTY_CLASS(utlPropertyRect);
    IMPORT_PROPERTY_CLASS(utlPropertyPoint);

    IMPORT_PROPERTY_CLASS(utlPropertyRectOwner);
    IMPORT_PROPERTY_CLASS(utlPropertyPointOwner);
}

// write the object to a writer (serialize)
void utlPropertyRect::Write( proObjectWriter* out, const proObject* obj ) const 
{ 
    out->PutPropValue(GetName(), ToString(obj) ); 
}

// read the object from a reader (unserialize)
void utlPropertyRect::Read( proObjectReader* in, proObject* obj ) 
{ 
    string str; 
    in->GetPropValue(GetName(), str); 
    FromString(obj, str); 
}


// write the object to a writer (serialize)
void utlPropertyPoint::Write( proObjectWriter* out, const proObject* obj ) const 
{ 
    out->PutPropValue(GetName(), ToString(obj) ); 
}

// read the object from a reader (unserialize)
void utlPropertyPoint::Read( proObjectReader* in, proObject* obj ) 
{ 
    string str; 
    in->GetPropValue(GetName(), str); 
    FromString(obj, str); 
}



/**
 * custom annotations for Rect properties
 */
void utlPropertyRect::SetAnnotation(const string &name, const string &value)
{
    if (_stricmp(name.c_str(), proAnnoStrDefault) == 0)
    {
        utlPropertyRectOwner *newProp = new utlPropertyRectOwner();
        newProp->SetName(name);
        AddInstanceProperty(newProp);
        newProp->SetValue(this, utlStringUtils::AsRect(value));
    }
    else
    {
        proProperty::SetAnnotation(name, value);
    }
}

utlRect utlPropertyRect::GetDefault() const 
{ 
    proProperty *prop = GetPropertyByName(proAnnoStrDefault);
    if (prop && proClassRegistry::InstanceOf(prop->GetClass(), "utlPropertyRect"))
    {
        utlPropertyRect *matProp = (utlPropertyRect*)prop;
        return matProp->GetValue(this);
    }
    return utlRect::Zero; 
}

/**
 * custom annotations for vector properties
 */
void utlPropertyPoint::SetAnnotation(const string &name, const string &value)
{
    if (_stricmp(name.c_str(), proAnnoStrDefault) == 0)
    {
        utlPropertyPointOwner *newProp = new utlPropertyPointOwner();
        newProp->SetName(name);
        AddInstanceProperty(newProp);
        newProp->SetValue(this, utlStringUtils::AsPoint(value));
    }
    else
    {
        proProperty::SetAnnotation(name, value);
    }
}

utlPoint utlPropertyPoint::GetDefault() const 
{ 
    proProperty *prop = GetPropertyByName(proAnnoStrDefault);
    if (prop && proClassRegistry::InstanceOf(prop->GetClass(), "utlPropertyPoint"))
    {
        utlPropertyPoint *matProp = (utlPropertyPoint*)prop;
        return matProp->GetValue(this);
    }
    return utlPoint::Zero; 
}

namespace utlStringUtils
{
char buf[4096];

string AsString(const utlRect& value)
{
    sprintf(buf, "%ld,%ld,%ld,%ld", value.left, value.top, value.right, value.bottom);
    return buf;
}

string AsString(const utlPoint& value)
{
    sprintf(buf, "%ld,%ld", value.x, value.y);
    return buf;
}

utlRect AsRect(const string &value)
{
    int l,t,r,b;
    int num = sscanf(value.c_str(), "%d,%d,%d,%d", &l, &t, &r, &b);
    if (num == 4)
        return utlRect(l,t,r,b);
    else
        return utlRect::Zero;
}

utlPoint AsPoint(const string &value)
{
    int x,y;
    int num = sscanf(value.c_str(), "%d,%d", &x, &y);
    if (num == 2)
        return utlPoint(x,y);
    else
        return utlPoint::Zero;
}


}; // end namespace utlStringUtils






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

static utlRect testRect(10,20,100,200);
static utlPoint testPoint(33,55);

/**
 * test class for proObject tests
*/
class utlTestClass : public proObject
{
    PRO_DECLARE_OBJECT
public:
    utlTestClass() :
      m_rect(0,0,0,0),
      m_point(7,8)
     {}

    virtual ~utlTestClass() {}

    utlRect GetMyRect() const { return m_rect;}
    utlPoint GetMyPoint() const { return m_point;}

    void SetMyRect(const utlRect &value) { m_rect = value; }
    void SetMyPoint(const utlPoint &value) { m_point = value; }

protected:
    utlRect m_rect;
    utlPoint m_point;
};

PRO_REGISTER_CLASS(utlTestClass, proObject)
REG_PROPERTY_RECT(utlTestClass, MyRect)
REG_PROPERTY_POINT(utlTestClass, MyPoint)



class utlPropertyTests : public tstUnit
{
private:
    utlTestClass *m_object;

public:
    utlPropertyTests()
    {
    }

    virtual void Register()
    {
        SetName("utlProperty");

        proClassRegistry::Init();

        AddTestCase("testBasicTypes", &utlPropertyTests::testBasicTypes);
        AddTestCase("testOwnerProperties", &utlPropertyTests::testOwnerProperties);

    };

    virtual void TestCaseSetup()
    {
        m_object = new utlTestClass();
    }

    virtual void TestCaseTeardown()
    {
        SafeDelete(m_object);
    }

    void testBasicTypes() const
    {
        proProperty *prop = NULL;
        
        // test get & set rect values
        prop = m_object->GetPropertyByName("MyRect");
        TESTASSERT(prop->ToString(m_object) == "0,0,0,0");

        utlPropertyRect *propRect = (utlPropertyRect*)prop;
        propRect->SetValue(m_object, testRect);
        TESTASSERT(propRect->GetValue(m_object) == testRect);

        // test get & set point values
        prop = m_object->GetPropertyByName("MyPoint");
        TESTASSERT(prop->ToString(m_object)== "7,8");

        utlPropertyPoint *propPoint = (utlPropertyPoint*)prop;
        propPoint->SetValue(m_object, testPoint);
        TESTASSERT(propPoint->GetValue(m_object) == testPoint);
    }

    void testOwnerProperties() const
    {
        utlPropertyRectOwner *propRect = new utlPropertyRectOwner();
        propRect->SetName("MyRect");
        m_object->AddInstanceProperty(propRect);
        propRect->SetValue(m_object, testRect);

        utlPropertyPointOwner *propPoint = new utlPropertyPointOwner();
        propPoint->SetName("MyPoint");
        m_object->AddInstanceProperty(propPoint);
        propPoint->SetValue(m_object, testPoint);

        TESTASSERT( m_object->GetValue("MyRect") == "10,20,100,200");
        TESTASSERT( m_object->GetValue("MyPoint") == "33,55");
    }
};

EXPORTUNITTESTOBJECT(utlPropertyTests);


#endif
