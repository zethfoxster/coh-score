#include "arda2/core/corFirst.h"

#include "proFirst.h"

#include "arda2/properties/proClassOwner.h"
#include "arda2/properties/proPropertyOwner.h"
#include "arda2/properties/proObject.h"


#include "arda2/properties/proObjectReader.h"
#include "arda2/properties/proObjectWriter.h"

using namespace std;

PRO_REGISTER_CLASS(proClassOwner, proClass)

proClassOwner::proClassOwner() :
    proClass()
{
}

proClassOwner::proClassOwner(const string &name, proClass *parent) :
    proClass(name)
{
    m_parent = parent;
}

proClassOwner::~proClassOwner()
{
    // delete class properties
    PropertyList::iterator pit = m_properties.begin();
    for (;pit != m_properties.end(); pit++)
    {
        delete *pit;
    }
}

/**
 * creates a new instance proObject of this class's type.
 * this object's impl must point to this owner class.
 * properties values are lazily added to the instance, so none are added here.
 * This chains NewInstance calls until a native class is reached
 */
proObject *proClassOwner::NewInstance()
{
    if( m_parent != NULL )
    {
        proObject *newObject = m_parent->NewInstance();
        newObject->SetClass(this);
        return newObject;
    }
    else
    {
        return NULL;
    }
}

int proClassOwner::GetPropertyCount() const
{
    int numInstanceProps = 0;
    if (m_impl)
        numInstanceProps += m_impl->GetInstancePropertyCount();

    if (!m_parent)
    {
        return numInstanceProps + proObject::GetPropertyCount() + (int)m_properties.size();
    }
    else
    {
        return numInstanceProps + m_parent->GetPropertyCount() + (int)m_properties.size();
    }
}

proProperty* proClassOwner::GetPropertyAt(int anIndex) const
{
    if (m_impl)
    {
        if (anIndex < m_impl->GetInstancePropertyCount())
        {
            return m_impl->GetInstancePropertyAt(anIndex);
        }
        anIndex -= m_impl->GetInstancePropertyCount();
    }

    if (!m_parent)
    {
        return m_properties[anIndex];
    }
    else
    {
        if (anIndex < m_parent->GetPropertyCount())
        {
            return m_parent->GetPropertyAt(anIndex);
        }
        else
        {
            anIndex -= m_parent->GetPropertyCount();
            if (anIndex < 0 || anIndex >= (int)m_properties.size())
            {
                return NULL;
            }
            return m_properties[anIndex];
        }
    }
    return NULL;
}

proProperty* proClassOwner::GetPropertyByName(const string &propName) const
{
    proProperty* rtn = NULL;
    if (m_impl)
    {
        rtn = m_impl->GetPropertyByName(propName);
    }

    if (!rtn)
    {
        for( PropertyList::const_iterator it = m_properties.begin(); it != m_properties.end(); it++ )
        {
            proProperty *p = (*it);
            if (p->GetName() == propName)
            {
                rtn = p;
                break;
            }
        }
    }

    if (!rtn && m_parent)
    {
        rtn = m_parent->GetPropertyByName(propName);
    }
    return rtn;
}

void proClassOwner::AddProperty(proProperty *property)
{
    //TODO - observation?
    m_properties.push_back(property);
}

void proClassOwner::RemoveProperty(proProperty *property)
{
    //TODO - observation?
    PropertyList::iterator it = m_properties.begin();
    for (; it != m_properties.end(); it++)
    {
        proProperty *found = (*it);
        if (found == property)
        {
            m_properties.erase(it);
            return;
        }
    }
}


errResult proClassOwner::Assign( const proObject* srcObj, proObject* destObj ) const
{   
    // if the class names aren't identical, return error
    if( strcmp( GetName().c_str(), destObj->GetClass()->GetName().c_str() ) != 0  )
    {
        ERR_RETURN( ES_Error, "attempt to assign disparate types" );
    }
  
    for ( int i = 0; i < (int)m_properties.size(); ++i)
    {
        proProperty *p = m_properties[i];
        p->Clone( srcObj, destObj );
    }
    return ER_Success;
}

// check if another class is an instance of this class
bool proClassOwner::Instanceof( const proClass* aClass ) const
{
    if (m_parent->Instanceof(aClass))
        return true;
    else
        return proClass::Instanceof(aClass);
}
void proClassOwner::SetParent(proClassOwner* parent)
{
    if (m_parent)
    {
        ERR_REPORTV(ES_Warning, ("Class %s already has a parent!", m_parent->GetName().c_str()));
        return;
    }
    m_parent = parent;
}

// add an owner instance property to this object
void proClassOwner::AddInstanceProperty(proProperty *property)
{
    property->SetDeclaringClass(this);

    // add annotation for this property
    proPropertyBoolOwner *classProp = new proPropertyBoolOwner();
    classProp->SetName("IsClassProperty");
    property->AddInstanceProperty(classProp);
    classProp->SetValue(property, true);

    proObject::AddInstanceProperty(property);
}


void proClassOwner::WriteObject(proObjectWriter* out) const
{
    if (!out)
        return;

    bool ref = out->CheckReference(this);
    if (ref)
        return;

    // write header
    out->BeginObject( GetClass()->GetName(), 0 );

    out->PutPropValue("ClassName", GetName());
    out->PutPropValue("NumProperties", GetPropertyCount());
    out->PutPropValue("ParentClass", m_parent->GetName());

    // attributes
    for (int i = 0; i < GetPropertyCount(); ++i)
    {
        proProperty *property = GetPropertyAt(i);
        out->WriteObject(property);
    }
    out->EndObject();
}


void proClassOwner::ReadObject(proObjectReader* in)
{
    if (!in)
        return;

    // read header
    int version;
    if ( !ISOK(in->BeginObject(GetClass()->GetName(), version)) )
        return;

    int count =0;
    in->GetPropValue("NumProperties", count);

    string parentClassName;
    in->GetPropValue("ParentClass", parentClassName);

    string className;
    in->GetPropValue("ClassName", className);

    SetName(className);

    m_parent = proClassRegistry::ClassForName<proClassNative*>(parentClassName);
    if (!m_parent)
        return;

    for (int i=0; i != count; i++)
    {
        DP_OWN_VALUE owner;
        proProperty *prop = (proProperty*)in->ReadObject(owner);
        
        // post-processing to handle special types of annotations

        // Default special annotation
        proProperty *defaultProperty = prop->GetPropertyByName("Default");
        if (defaultProperty)
        {
            string value("Default|");
            value += defaultProperty->ToString(prop);
            prop->RemoveInstanceProperty(defaultProperty);
            prop->SetAnnotations(value.c_str());
        }

        // special IsClassProperty annotation
        proProperty *classProp = prop->GetPropertyByName("IsClassProperty");
        if (classProp)
        {
            AddInstanceProperty(prop);
            prop->RemoveInstanceProperty(classProp); // dupe of this is added by the line above
        }
        else
        {
            AddProperty(prop);
        }
    }

    /*  // does not automatically register with class registry.
    proClassOwner *foundClass = proClassRegistry::ClassForName<proClassOwner*>(GetName());
    if (foundClass)
    {
        ERR_REPORTV(ES_Warning, ("Owner Class <%s> already exists in class registry...", GetName().c_str()));
    }
    else
    {
        proClassRegistry::RegisterClass(this);
    }*/

}


proProperty *proClassOwner::CreateProperty(const string &name, const proDataType dt, const string &def)
{
    proProperty *newProp = NULL;

    switch (dt)
    {
    case DT_int:
        {
        proPropertyIntOwner *p = (proPropertyIntOwner*)proClassRegistry::NewInstance("proPropertyIntOwner");
        p->SetDefault( proStringUtils::AsInt(def) );
        newProp = (proProperty*)p;
        }
        break;

    case DT_uint:
        {
        proPropertyUintOwner *p = (proPropertyUintOwner*)proClassRegistry::NewInstance("proPropertyUintOwner");
        p->SetDefault( proStringUtils::AsUint(def) );
        newProp = (proProperty*)p;
        }
        break;

    case DT_float:
        {
        proPropertyFloatOwner *p = (proPropertyFloatOwner*)proClassRegistry::NewInstance("proPropertyFloatOwner");
        p->SetDefault( proStringUtils::AsFloat(def) );
        newProp = (proProperty*)p;
        }
        break;

    case DT_bool:
        {
        proPropertyBoolOwner *p = (proPropertyBoolOwner*)proClassRegistry::NewInstance("proPropertyBoolOwner");
        p->SetDefault( proStringUtils::AsBool(def) );
        newProp = (proProperty*)p;
        }
        break;

    case DT_string:
        {
        proPropertyStringOwner *p = (proPropertyStringOwner*)proClassRegistry::NewInstance("proPropertyStringOwner");
        p->SetDefault(def);
        newProp = (proProperty*)p;
        }
        break;

    case DT_wstring:
        {
        proPropertyWstringOwner *p = (proPropertyWstringOwner*)proClassRegistry::NewInstance("proPropertyWstringOwner");
        //p->SetDefault( L"");
        newProp = (proProperty*)p;
        }
        break;

    case DT_object:
        {
        proPropertyObjectOwner *p = (proPropertyObjectOwner*)proClassRegistry::NewInstance("proPropertyObjectOwner");
        p->SetDefault(NULL);
        newProp = (proProperty*)p;
        }

        break;

    case DT_enum:
        {
        //proPropertyEnumOwner *p = (proPropertyEnumOwner*)proClassRegistry::NewInstance("proPropertyEnumOwner");
        //p->SetDefault( proStringUtils::AsUint(def) );
        //newProp = (proProperty*)p;
        }
        break;

    default:
        break;
    }

    if (newProp)
    {
        newProp->SetName(name);
        AddProperty(newProp);
    }
    return newProp;
}

#if BUILD_UNIT_TESTS

#include "arda2/test/tstUnit.h"
#include "arda2/properties/proClassNative.h"
#include "arda2/properties/proPropertyNative.h"

#include "arda2/properties/proObjectWriterXML.h"
#include "arda2/properties/proObjectReaderXML.h"
#include "arda2/storage/stoFileOSFile.h"
#include "arda2/storage/stoPropertyStream.h"

static const int defaultInt       = 7711;
static const uint defaultUint       = 11234;
static const float defaultFloat   = 0.55113f;
static const string defaultString = "default";
static const bool defaultBool     = true;

static const string newName("foobar");
static const int newX = 51;
static const uint newSize = 99999;
static const float newRatio = 0.4f;


class proTestDynamicClass : public proObject
{
PRO_DECLARE_OBJECT
public:
    proTestDynamicClass()
    {
    }
    virtual ~proTestDynamicClass()
    {
    }
};

PRO_REGISTER_CLASS(proTestDynamicClass, proObject)


class proTestDynamicClassDerived : public proTestDynamicClass
{
PRO_DECLARE_OBJECT
public:
    proTestDynamicClassDerived()
    {
    }
    virtual ~proTestDynamicClassDerived()
    {
    }
};

PRO_REGISTER_CLASS(proTestDynamicClassDerived, proTestDynamicClass)


class proObjectDynamicTests : public tstUnit
{
private:
    proTestDynamicClass *m_object;
    proClassOwner       *m_ownerClass;

public:
    proObjectDynamicTests()
    {
    }

    virtual void Register()
    {
        ImportOwnerProperties();
        proClassRegistry::Init();

        SetName("proObjectDynamic");
        AddDependency("proObject");

        AddTestCase("testBasicTypes", &proObjectDynamicTests::testBasicTypes);
        AddTestCase("testClone", &proObjectDynamicTests::testClone);
        AddTestCase("testClasses", &proObjectDynamicTests::testClasses);
        AddTestCase("testObjectProperties", &proObjectDynamicTests::testObjectProperties);
        AddTestCase("testEnum", &proObjectDynamicTests::testEnum);
        AddTestCase("testClassProperties", &proObjectDynamicTests::testClassProperties);
        AddTestCase("testSerialization", &proObjectDynamicTests::testSerialization);
        AddTestCase("testInheritance", &proObjectDynamicTests::testInheritance);

    };

    virtual void TestCaseSetup()
    {
        proProperty *pro = NULL;

        // build an owner class
        m_ownerClass = new proClassOwner("MyOwner", proClassRegistry::ClassForName<proClassNative*>("proTestDynamicClass"));
        m_object = static_cast<proTestDynamicClass*>(m_ownerClass->NewInstance());

        pro = m_ownerClass->CreateProperty("XXX", DT_int); 
        pro->SetAnnotations("Min|100|Max|1000|Default|30");
        pro->FromString(m_object, "7711");
        pro = m_ownerClass->CreateProperty("Size", DT_uint);
        pro->FromString(m_object, "11234");
        pro = m_ownerClass->CreateProperty("Name", DT_string);
        pro->FromString(m_object, "default");
        pro = m_ownerClass->CreateProperty("Ratio", DT_float);
        pro->FromString(m_object, "0.55113");
        pro = m_ownerClass->CreateProperty("Flag", DT_bool);
        pro->SetAnnotations("Readonly|true|Description|A Flag|Default|false");
        pro->FromString(m_object, "true");
     }
    virtual void TestCaseTeardown()
    {
        SafeDelete(m_object);
        SafeDelete(m_ownerClass);
    }

    void testBasicTypes() const
    {
        proProperty *pro = NULL;

        TESTASSERT( m_object->GetPropertyCount() == 5);

        // Test integer property type
        pro = m_object->GetPropertyByName("XXX");
        TESTASSERT( pro->GetDataType() == DT_int );
        proPropertyInt *proInt = static_cast<proPropertyInt*>(pro);
        int valueInt = proInt->GetValue(m_object);
        TESTASSERT( valueInt == defaultInt);

        // Test unsigned integer property type
        pro = m_object->GetPropertyByName("Size");
        TESTASSERT( pro->GetDataType() == DT_uint);
        proPropertyUint *proUint = static_cast<proPropertyUint*>(pro);
        uint valueUint = proUint->GetValue(m_object);
        TESTASSERT( valueUint == defaultUint);

        // Test float property type
        pro = m_object->GetPropertyByName("Ratio");
        TESTASSERT( pro->GetDataType() == DT_float );
        proPropertyFloat *proFlt = static_cast<proPropertyFloat*>(pro);
        float valueFlt = proFlt->GetValue(m_object);
        TESTASSERT( valueFlt == defaultFloat);

        // Test string property type
        pro = m_object->GetPropertyByName("Name");
        TESTASSERT( pro->GetDataType() == DT_string );
        proPropertyString *proStr = static_cast<proPropertyString*>(pro);
        string valueStr = proStr->GetValue(m_object);
        TESTASSERT( valueStr == defaultString);

        // Test bool property type
        pro = m_object->GetPropertyByName("Flag");
        TESTASSERT( pro->GetDataType() == DT_bool );
        proPropertyBool *proBool = static_cast<proPropertyBool*>(pro);
        bool valueBool = proBool->GetValue(m_object);
        TESTASSERT( valueBool == defaultBool);
        
        // this removes the object's values.. but not the classes properties
        m_object->FlushInstanceProperties();
        TESTASSERT( m_object->GetPropertyCount() == 5);

    }

    void testClasses() const
    {
        // test factory construction
        proClass *cls1 = proClassRegistry::ClassForName<proClass*>("proTestDynamicClass");
        proObject *same= cls1->NewInstance();
        TESTASSERT( same->GetClass()->Instanceof(cls1) );

        // test inheritance
        proClass *cls2 = proClassRegistry::ClassForName<proClass*>("proTestDynamicClassDerived");
        proObject *derived = cls2->NewInstance();
        TESTASSERT( derived->GetClass()->Instanceof(cls1) );

        SafeDelete(same);
        SafeDelete(derived);
    }

    void testClone() const
    {
        proObject *obj = m_object->Clone();

        TESTASSERT( proClassRegistry::InstanceOf(obj->GetClass(), m_object->GetClass()) );

        // check for cloned properties
        TESTASSERT( obj->GetPropertyCount() == 10);

        // Test integer property type
        proProperty *pro = obj->GetPropertyByName("XXX");
        TESTASSERT( pro->GetDataType() == DT_int );
        proPropertyInt *proInt = static_cast<proPropertyInt*>(pro);
        int valueInt = proInt->GetValue(obj);
        TESTASSERT( valueInt == defaultInt);

        SafeDelete(obj);
    }

    void testObjectProperties() const 
    {
        proTestDynamicClass *child1 = static_cast<proTestDynamicClass*>(m_ownerClass->NewInstance());;
        proTestDynamicClass *child2 = static_cast<proTestDynamicClass*>(m_ownerClass->NewInstance());;

        m_object->AddChild(child1, "Child1");
        m_object->AddChild(child2, "Child2", DP_refValue);
        TESTASSERT( m_object->GetPropertyCount() == 5 + 2);

        m_object->RemoveChild(child1);
        m_object->RemoveChild(child2);
        TESTASSERT( m_object->GetPropertyCount() == 5);

        // - was deleted when removeSafeDelete(child1);
        SafeDelete(child2);
    }

    void testEnum() const 
    {
        proPropertyEnumOwner *enumProp = new proPropertyEnumOwner();
        enumProp->SetEnumString("zero,0,one,1,two,2,three,3,four,4");
        enumProp->SetName("Numbers");
        m_object->AddInstanceProperty(enumProp);
        
        enumProp->SetValue(m_object, "three");
        uint val = enumProp->GetValue(m_object);
        TESTASSERT(val == 3);

        enumProp->SetValue(m_object, (uint)1);
        uint val2 = enumProp->GetValue(m_object);
        string strValue = enumProp->GetEnumerationValue(val2);
        TESTASSERT( strValue == "one");
    }

    void testClassProperties() const
    {
        proPropertyIntOwner *prop = new proPropertyIntOwner();
        prop->SetName("FOO");
        m_ownerClass->AddInstanceProperty(prop);

        // test setting value on class directly
        prop->SetValue(NULL, 55);
        int result = prop->GetValue(NULL);
        TESTASSERT(result == 55);

        // test setting values on an instance
        m_object->SetValue("FOO", "100");
        int fooValue = prop->GetValue(m_object);
        TESTASSERT(fooValue == 100);
        TESTASSERT(m_object->GetPropertyCount() == 6);

        // test getting the value on another instance
        proObject *newObject = m_ownerClass->NewInstance();
        string fooStr = newObject->GetValue("FOO");
        TESTASSERT(fooStr == "100");
        TESTASSERT(newObject->GetPropertyCount() == 6);

        // test removing
        m_ownerClass->RemoveInstanceProperty(prop);
        string fooStr2 = newObject->GetValue("FOO");
        TESTASSERT(fooStr2 == "");
        TESTASSERT(newObject->GetPropertyCount() == 5);

        SafeDelete(newObject);
    }

    void testSerialization() const
    {
        proPropertyIntOwner *prop = new proPropertyIntOwner();
        prop->SetName("FOO");
        prop->SetAnnotations("Default|33");
        m_ownerClass->AddInstanceProperty(prop);

        stoFileOSFile file;
        if (ISOK(file.Open("test.xml", stoFileOSFile::kAccessCreate)) )
        {
            stoStreamOutputFile stream;
            stream.Init(&file);

            proObjectWriterXML writer(&stream);
            writer.WriteObject(m_ownerClass);
            file.Close();
        }


        if (ISOK(file.Open("test.xml", stoFileOSFile::kAccessRead)) )
        {
            stoStreamInputFile stream;
            stream.Init(&file);

            DP_OWN_VALUE owner;
            proObjectReaderXML reader(&stream);
            proObject *obj = reader.ReadObject(owner);
            file.Close();

            TESTASSERT(obj != NULL);

            if (obj && proClassRegistry::InstanceOf(obj->GetClass(), "proClassOwner"))
            {
                proClassRegistry::RegisterClass( (proClassOwner*)obj );

                proObject *newObject = proClassRegistry::NewInstance("MyOwner");
                TESTASSERT(newObject != NULL);

                // tests that the default value of a class property was set correctly
                TESTASSERT(newObject->GetValue("FOO") == "33");
                TESTASSERT(newObject->GetClass()->GetName() == "MyOwner");

                /*for (int i=0; i != newObject->GetPropertyCount(); i++)
                {
                    proProperty *prop = newObject->GetPropertyAt(i);
                    ERR_REPORTV(ES_Info, ("Property: %s / %s (%s)", prop->GetName().c_str(), prop->ToString(newObject).c_str(), prop->GetClass()->GetName().c_str()));

                    for (int j=0; j != prop->GetPropertyCount(); j++)
                    {
                        proProperty *anno= prop->GetPropertyAt(j);
                        ERR_REPORTV(ES_Info, ("   Anno: %s / %s (%s)", anno->GetName().c_str(), anno->ToString(prop).c_str(), anno->GetClass()->GetName().c_str()));
                    }
                }*/
                SafeDelete(newObject);
            }
        }
    }

    void testInheritance() const
    {
        // build a derived owner class
        proClassOwner *derivedClass = new proClassOwner("MyOwnerDerived", m_ownerClass);

        //setup some class properties
        proPropertyIntOwner *prop1 = new proPropertyIntOwner();
        prop1->SetName("FOO");
        prop1->SetAnnotations("Default|33");
        m_ownerClass->AddInstanceProperty(prop1);
        prop1->SetValue(NULL, 100);

        proPropertyIntOwner *prop2 = new proPropertyIntOwner();
        prop2->SetName("DERIVEDFOO");
        prop2->SetAnnotations("Default|66");
        m_ownerClass->AddInstanceProperty(prop2);
        

        // set some regular properties on the new class
        proPropertyIntOwner *newPropInt = new proPropertyIntOwner();
        newPropInt->SetName("derivedInt");
        newPropInt->SetAnnotations("Default|1000");
        derivedClass->AddProperty(newPropInt);

        proPropertyStringOwner *newPropStr = new proPropertyStringOwner();
        newPropStr->SetName("derivedStr");
        derivedClass->AddProperty(newPropStr);

        proObject *derivedObject = static_cast<proTestDynamicClass*>(derivedClass->NewInstance());
        TESTASSERT(derivedObject->GetClass()->GetName() == "MyOwnerDerived");

        // test properties from parent class
        proProperty *fromParent1 = derivedObject->GetPropertyByName("XXX");
        TESTASSERT(fromParent1 != NULL);
        TESTASSERT(fromParent1->ToString(derivedObject) == "30");

        proProperty *fromParent2 = derivedObject->GetPropertyByName("Name");
        TESTASSERT(fromParent2 != NULL);
        
        derivedObject->SetValue("Name", "testing");
        TESTASSERT(fromParent2->ToString(derivedObject) == "testing");

        // test properties on this class
        proProperty *fromThis = derivedObject->GetPropertyByName("derivedInt");
        TESTASSERT(fromThis != NULL);
        TESTASSERT(fromThis->ToString(derivedObject) == "1000");
        derivedObject->SetValue("derivedInt", "999");

        TESTASSERT(fromThis->ToString(derivedObject) == "999");

        TESTASSERT(derivedObject->GetValue("FOO") == "100");

        SafeDelete(derivedObject);
        SafeDelete(derivedClass);
    }
};

EXPORTUNITTESTOBJECT(proObjectDynamicTests);

#endif
