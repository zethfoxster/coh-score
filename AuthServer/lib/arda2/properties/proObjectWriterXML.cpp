#include "arda2/core/corFirst.h"

#include "proFirst.h"


#include "proObjectWriterXML.h"

#include "arda2/properties/proClass.h"
#include "arda2/properties/proProperty.h"
#include "arda2/properties/proStream.h"

#include "arda2/properties/proStringUtils.h"
#include "arda2/properties/proPropertyNative.h"

using namespace std;

// constants for string comparisons

//static const char *dataLabel        = "PropertyData";
static const char *objectLabel      = "PropertyObject";
static const char *classLabel       = "class";
static const char *versionLabel     = "version";
static const char *attributeLabel   = "property";
static const char *nameLabel        = "name";
static const char *valueLabel       = "value";
static const char *typeLabel        = "type";

PRO_REGISTER_CLASS(proObjectWriterXML, proObjectWriter)

/**
 * constructor
**/
proObjectWriterXML::proObjectWriterXML(proStreamOutput *output) :
    proObjectWriter(output),
    m_indentation(0),
    m_lastRef(0)
{
}

/**
 * destructor
**/
proObjectWriterXML::~proObjectWriterXML()
{
}

/**
 * writes a property object to the stream
**/
void proObjectWriterXML::WriteObject(const proObject *obj)
{
    if (!obj || !m_output)
        return;

    // friendship is no inherited, call the base class method.
    proObjectWriter::InternalWriteObject(obj);
}

bool proObjectWriterXML::CheckReference(const proObject *obj)
{
  RefMap::const_iterator it = m_references.find((proObject*)obj);
    if (it == m_references.end())
    {
        // never seen this object before - write the full objects with a new ref id.
        m_lastRef += 1;
        m_references.insert( RefMap::value_type((proObject*)obj, m_lastRef) );
        return false;
    }
    else
    {
        // have seen it before. just write out a reference
        uint ref = (*it).second;

        string output;

        Indent(output);
        output += "<";
        output += objectLabel;
        output += " ";
        char buf[64];
        sprintf(buf, "refObj=\"%u\"", ref);
        output += buf;
        output += "/>\n";

        m_output->Write(output.c_str(), (uint)output.size());

        return true;
    }
}

void proObjectWriterXML::BeginObject(const string &className, int version)
{
    string output;

    Indent(output);
    output += "<";
    output += objectLabel;
    output += " ";
    output += classLabel;
    output += "=\"";
    output += className;
    output += "\" ";
    output += versionLabel;
    output += "=\"";
    output += proStringUtils::AsString(version);
    output += "\" ";

    char buf[64];
    sprintf(buf, "newObj=\"%u\"", m_lastRef);
    output += buf;
    output += ">\n";

    //ERR_REPORT(ES_Info, output.c_str());
    m_output->Write(output.c_str(), (uint)output.size());

    m_indentation++;

}
void proObjectWriterXML::PutPropValue(const string &name, const string &value)
{
    PutTypedStringValue(name, value, "string");
}

void proObjectWriterXML::PutPropValue(const string &name, const wstring &value)
{
    UNREF(value); 
    PutPropValue(name, string("PutPropValue( wstring ) unimplemented!!"));
    // todo: make AsString(wstring)
    //PutPropValue(name, proStringUtils::AsString(value));
}

void proObjectWriterXML::PutPropValue(const string &name, const int value)
{
    PutTypedStringValue(name, proStringUtils::AsString(value), "int");}

void proObjectWriterXML::PutPropValue(const string &name, const bool value)
{
    PutTypedStringValue(name, proStringUtils::AsString(value), "bool");
}

void proObjectWriterXML::PutPropValue(const string &name, const float value)
{
    PutTypedStringValue(name, proStringUtils::AsString(value), "float");
}

void proObjectWriterXML::PutPropValue(const string &name, const uint value)
{
    PutTypedStringValue(name, proStringUtils::AsString(value), "uint");
}

void proObjectWriterXML::PutPropValue(const string &name, const proObject *obj)
{
    string output1;
    string output2;

    Indent(output1);
    output1 += "<";
    output1 += attributeLabel;
    output1 += " ";
    output1 += nameLabel;
    output1 += "=\"";
    output1 += name;
    output1 += "\" ";
    output1 += typeLabel;
    output1 += "=\"object\">\n";

    m_output->Write(output1.c_str(), (uint)output1.size());

    if (obj)
    {
         proObjectWriter::InternalWriteObject(obj);
    }
    else
    {
        // write nothing if the object's value is NULL
        string out;
        Indent(out);
        out +=  "<";
        out += objectLabel;
        out += " ";
        out += classLabel;
        out += "=\"null\"></";
        out += objectLabel;
        out += ">\n";
        m_output->Write(out.c_str(), (uint)out.size());
    }


    Indent(output2);
    output2 += "</";
    output2 += attributeLabel;
    output2 += ">\n";

    //ERR_REPORT(ES_Info, output.c_str());
    m_output->Write(output2.c_str(), (uint)output2.size());

}

void proObjectWriterXML::PutTypedStringValue(const string &name, const string &value, const string &type)
{
    string output;
    Indent(output);
    output += "<";
    output += attributeLabel;
    output += " ";
    output += nameLabel;
    output += "=\"";
    output += name;
    output += "\" ";
    output += valueLabel;
    output += "=\"";
    output += value;
    output += "\" ";
    output += typeLabel;
    output += "=\"";
    output += type;
    output += "\"/>\n";

    m_output->Write(output.c_str(), (uint)output.size());
}

void proObjectWriterXML::EndObject()
{
    string output;

    m_indentation--;

    Indent(output);
    output += "</";
    output += objectLabel;
    output += ">\n";


    //ERR_REPORT(ES_Info, output.c_str());
    m_output->Write(output.c_str(), (uint)output.size());
}

void proObjectWriterXML::Indent(string &output)
{
    for (int i=0; i!=m_indentation; i++)
    {
        output += "    ";
    }
}


/**
 *  used to write out owner properties
**/
void proObjectWriterXML::PutPropTyped(const string &name, const string &value, const string &typeName)
{
    string output;
    Indent(output);
    output += "<";
    output += attributeLabel;
    output += " ";
    output += nameLabel;
    output += "=\"";
    output += name;
    output += "\" ";
    output += valueLabel;
    output += "=\"";
    output += value;
    output += "\" ";
    output += typeLabel;
    output += "=\"";
    output += typeName;
    output += "\"/>\n";

    m_output->Write(output.c_str(), (uint)output.size());
}




/*
 *  UNIT TESTS
 */
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

/**
 * test class for proObject tests
*/
class proTestClassForWriter : public proObject
{
    PRO_DECLARE_OBJECT
public:
    proTestClassForWriter() : 
        m_x(defaultInt), 
        m_size(defaultUint), 
        m_ratio(defaultFloat),
        m_name(defaultString), 
        m_flag(defaultBool),
        m_number(0),
        m_theObject1(NULL),
        m_theObject2(NULL),
        m_theObject3(NULL)
     {}

    virtual ~proTestClassForWriter() 
    {
    }

    int GetX() const { return m_x;}
    uint GetSize() const { return m_size;}
    float GetRatio() const { return m_ratio;}
    string GetName() const { return m_name;}
    bool GetFlag() const { return m_flag;}
    uint GetNumber() const { return m_number;}
    proObject *GetTheObject1() const { return m_theObject1; }
    proObject *GetTheObject2() const { return m_theObject2; }
    proObject *GetTheObject3() const { return m_theObject3; }

    void SetX(int value) { m_x = value; }
    void SetSize(int value) { m_size = value; }
    void SetRatio(float value) { m_ratio = value; }
    void SetName(const string &value) { m_name = value; }
    void SetFlag(bool value) { m_flag = value; }
    void SetNumber(uint value) { m_number = value; }
    void SetTheObject1(proObject *value) { m_theObject1 = value; }
    void SetTheObject2(proObject *value) { m_theObject2 = value; }
    void SetTheObject3(proObject *value) { m_theObject3 = value; }

protected:
    int m_x;
    uint m_size;
    float m_ratio;
    string m_name;
    bool m_flag;
    uint m_number;
    proObject *m_theObject1;
    proObject *m_theObject2;
    proObject *m_theObject3;

};

PRO_REGISTER_CLASS(proTestClassForWriter, proObject)
REG_PROPERTY_INT(proTestClassForWriter, X)
REG_PROPERTY_UINT(proTestClassForWriter, Size)
REG_PROPERTY_FLOAT(proTestClassForWriter, Ratio)
REG_PROPERTY_STRING(proTestClassForWriter, Name)
REG_PROPERTY_BOOL(proTestClassForWriter, Flag)
REG_PROPERTY_ENUM(proTestClassForWriter, Number, "zero,0,one,1,two,2,three,3,four,4,five,5")
REG_PROPERTY_OBJECT(proTestClassForWriter, TheObject1, proTestClassForWriter);
REG_PROPERTY_OBJECT(proTestClassForWriter, TheObject2, proTestClassForWriter);
REG_PROPERTY_OBJECT(proTestClassForWriter, TheObject3, proTestClassForWriter);

class proTestClassForWriterDerived : public proTestClassForWriter
{
    PRO_DECLARE_OBJECT
public:
    proTestClassForWriterDerived() {}
    ~proTestClassForWriterDerived() {}

};
PRO_REGISTER_CLASS(proTestClassForWriterDerived, proTestClass)


class proObjectWriterTests : public tstUnit
{
private:
    proTestClassForWriter *m_object;

public:
    proObjectWriterTests()
    {
    }

    virtual void Register()
    {
        SetName("proObjectWriter");

        proClassRegistry::Init();

        AddTestCase("testBasicTypes", &proObjectWriterTests::testBasicTypes);
        AddTestCase("testChildren", &proObjectWriterTests::testChildren);
        //AddTestCase("testMultipleObjects", &proObjectWriterTests::testMultipleObjects);

    };

    virtual void TestCaseSetup()
    {
        m_object = new proTestClassForWriter();
    }

    virtual void TestCaseTeardown()
    {
        SafeDelete(m_object);
    }

    void testBasicTypes() const
    {
        m_object->SetName("aName");
        m_object->SetNumber(2);

        proTestClassForWriter *theObject1 = new proTestClassForWriter();
        theObject1->SetName("THEOBJECT1");
        m_object->SetTheObject1(theObject1);
        m_object->SetTheObject2(theObject1);

        stoFileOSFile file;
        if (ISOK(file.Open("test.xml", stoFileOSFile::kAccessCreate)) )
        {
            stoStreamOutputFile stream;
            stream.Init(&file);

            proObjectWriterXML writer(&stream);
            writer.WriteObject(m_object);
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

            TESTASSERT(proClassRegistry::InstanceOf(obj->GetClass(), "proTestClassForWriter"));

            proTestClassForWriter *testObject = (proTestClassForWriter*)obj;

            TESTASSERT(testObject->GetName() == "aName");
            TESTASSERT(testObject->GetNumber() == 2);
            TESTASSERT(testObject->GetTheObject1() != NULL);

            proTestClassForWriter *theObject1 = (proTestClassForWriter*)testObject->GetTheObject1();
            TESTASSERT(theObject1->GetName() == "THEOBJECT1");

            TESTASSERT(testObject->GetTheObject1() == testObject->GetTheObject2());
            TESTASSERT(testObject->GetTheObject3() == NULL);

            proObject * child = (proObject*)testObject->GetTheObject1();
            SafeDelete(child);
            SafeDelete(testObject);

        }
        SafeDelete(theObject1);
    }

    void testChildren() const
    {
        proTestClassForWriter *child1 = new proTestClassForWriter();
        child1->SetName("aChild1");
        child1->SetRatio(0.55f);
        m_object->AddChild(child1, "aChild1");

        proTestClassForWriter *child2 = new proTestClassForWriter();
        child2->SetName("aChild2");
        child2->SetRatio(0.55f);
        m_object->AddChild(child2, "aChild2");

        stoFileOSFile file;
        if (ISOK(file.Open("test.xml", stoFileOSFile::kAccessCreate)) )
        {
            stoStreamOutputFile stream;
            stream.Init(&file);

            proObjectWriterXML writer(&stream);
            writer.WriteObject(m_object);
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

            vector<proObject*> children;
            obj->GetChildren(children);

            TESTASSERT(children.size() == 2);

            proObject *newChild = children[0];
            TESTASSERT(newChild != NULL);
            TESTASSERT(proClassRegistry::InstanceOf(newChild->GetClass(), "proTestClassForWriter"));

            proTestClassForWriter *testChild = (proTestClassForWriter*)newChild;
            TESTASSERT(testChild->GetName() == "aChild1");
            TESTASSERT(testChild->GetRatio() == 0.55f);

            SafeDelete(obj);
        }
    }

    /* // no support for multiple objects due to limitations of XML code

    void testMultipleObjects() const
    {
        stoFileOSFile file;
        if (ISOK(file.Open("test.xml", stoFileOSFile::kAccessCreate)) )
        {
            stoStreamOutputFile stream;
            stream.Init(&file);
            proObjectWriterXML writer(&stream);

            vector<proTestClassForWriter*> items;
            for (int i=0; i != 10; i++)
            {
                proTestClassForWriter *item = new proTestClassForWriter();
                items.push_back(item);
                writer.WriteObject(item);
            }
            file.Close();
        }

        if (ISOK(file.Open("test.xml", stoFileOSFile::kAccessRead)) )
        {
            stoStreamInputFile stream;
            stream.Init(&file);

            DP_OWN_VALUE owner;
            proObjectReaderXML reader(&stream);

            vector<proTestClassForWriter*> items;

            proObject *obj = NULL;
            while ((obj=reader.ReadObject(owner)) )
            {
                items.push_back( (proTestClassForWriter*)obj);
            }
            file.Close();

            TESTASSERT(items.size() == 10);
        }
    }*/
};

EXPORTUNITTESTOBJECT(proObjectWriterTests);


#endif

