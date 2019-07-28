#include "arda2/core/corFirst.h"
#include "proFirst.h"

#include "proObjectReaderXML.h"

#include "arda2/properties/proClass.h"
#include "arda2/properties/proProperty.h"
#include "arda2/properties/proStream.h"
#include "arda2/properties/proStringUtils.h"
#include "arda2/properties/proPropertyNative.h"


PRO_REGISTER_CLASS(proObjectReaderXML, proObjectReader)


#if CORE_SYSTEM_WINAPI
#include <mbstring.h>
#endif

#include "proXmlParser.h"

using namespace std;

// constants for string comparisons

//static const char *dataLabel        = "PropertyData";
//static const char *objectLabel      = "PropertyObject";
static const char *classLabel       = "class";
static const char *versionLabel     = "version";
//static const char *attributeLabel   = "attribute";
//static const char *nameLabel        = "name";
//static const char *valueLabel       = "value";

/**
 * constructor
**/
proObjectReaderXML::proObjectReaderXML(proStreamInput *source) :
    proObjectReader(source),
    m_document(NULL),
    m_node(NULL),
    m_lastRef(0)
{

}

/**
 * desstructor
**/
proObjectReaderXML::~proObjectReaderXML()
{
    SafeDelete(m_document);
}

errResult proObjectReaderXML::LoadDocument()
{
    if (!m_source)
        return ER_Failure;

    const int chunkSize = 1024 * 10;
    uint position = 0;
    uint bytesRead = 0;
    char *buffer = new char[chunkSize];

    while ( true )
    {
        m_source->Read(&buffer[position], chunkSize, bytesRead);

        if (bytesRead == 0)
            break;

        position += bytesRead;

        char *tmp = new char[position + chunkSize + 1];
        memset(tmp, 0, position + chunkSize + 1);
        memcpy(tmp, buffer, position);
        delete [] buffer;
        buffer = tmp;
    }

    // parse the XML
    m_document = new XMLDocument();
    m_document->ParseDocument( buffer );

    m_node = m_document->Node();

    delete [] buffer;
    return ER_Success;
}

XMLNode *GetNextObject(XMLNode *begin)
{
    XMLNode *node = begin;
    while (node)
    {
        if (!node->GetLoaded())
        {
            if (node->Value() == "PropertyObject")
                return node;
        }
        node = node->NextSibling();
    }
    return NULL;
}

/**
 * reads a property object from the stream
 *
 * @param owner out param for passing back the owner flag for this object
**/
proObject *proObjectReaderXML::ReadObject(OUT DP_OWN_VALUE &owner)
{
    XMLNode *oldNode = NULL;
    proObject *obj = NULL;

    if (!m_document)
        if (!ISOK(LoadDocument()))
            return NULL;

    XMLNode *objectNode = GetNextObject(m_node);
    if (!objectNode)
        return NULL;

    objectNode->SetLoaded(true);

    // check if this object is a reference
    const char *refObjString = objectNode->Attribute("refObj");
    if (refObjString[0] != '\0')
    {
        // this is a reference!
        int refObj = atoi(refObjString);
        RefMap::iterator it = m_references.find(refObj);
        if (it != m_references.end())
        {
            obj = (*it).second;
        }
        owner = DP_refValue;
        m_node = objectNode;
        return obj;
    }

    // this is a new non-ref object
    const char *className = objectNode->Attribute(classLabel);
    if (!className || strlen(className) == 0 || strcmp(className, "null") == 0)
        return NULL;

    m_workingClassName = className;

    // create the new property instance
    obj = proClassRegistry::NewInstance(className);

    // add the new object to the ref map - BEFORE unserializing.
    const char *newObjString = objectNode->Attribute("newObj");
    uint newObj = atoi(newObjString);
    m_references.insert( RefMap::value_type(newObj, obj) );

    // populate the new object
    oldNode = m_node;
    m_node = objectNode;

    // friendship is not inherited, so call the base class function
    proObjectReader::InternalReadObject(obj);

    owner = DP_ownValue;
    
    m_node = objectNode;
    return obj;

}


errResult proObjectReaderXML::BeginObject(const string &className, int &version)
{
    if (className != m_workingClassName)
    {
        return ER_Failure;
    }

    const char *versionString = m_node->Attribute(versionLabel);
    if (versionString)
    {
        version = proStringUtils::AsInt(versionString);
    }

    m_node = m_node->FirstChild();
    return ER_Success;
}

void proObjectReaderXML::GetPropValue(const string &name, int &value)
{
    XMLNode *attr = FindSiblingByName(name);
    if (attr)
    {
        value = proStringUtils::AsInt( attr->Attribute("value") );
        attr->SetLoaded(true);
    }
}

void proObjectReaderXML::GetPropValue(const string &name, bool &value)
{
    XMLNode *attr = FindSiblingByName(name);
    if (attr)
    {
        value = proStringUtils::AsBool( attr->Attribute("value") );
        attr->SetLoaded(true);
    }
}

void proObjectReaderXML::GetPropValue(const string &name, string &value)
{
    XMLNode *attr = FindSiblingByName(name);
    if (attr)
    {
        value = attr->Attribute("value");
        attr->SetLoaded(true);
    }
}

void proObjectReaderXML::GetPropValue(const string &name, wstring &value)
{
#if CORE_SYSTEM_WINAPI
    XMLNode *attr = FindSiblingByName(name);

    if(attr)
    {
        const char* str = attr->Attribute("value");

        value.resize( strlen(str) );
        for( uint i = 0; i< value.size(); ++i)
        {
            value[i] = (wchar_t)_mbbtombc( str[i] );
        }
        attr->SetLoaded(true);
    }
#else
    UNREF(name);
    UNREF(value);
    ERR_UNIMPLEMENTED();
#endif
}

void proObjectReaderXML::GetPropValue(const string &name, float &value)
{
    XMLNode *attr = FindSiblingByName(name);
    if (attr)
    {
        value = proStringUtils::AsFloat( attr->Attribute("value") );
        attr->SetLoaded(true);
    }
}

void proObjectReaderXML::GetPropValue(const string &name, uint &value)
{
    XMLNode *attr = FindSiblingByName(name);
    if (attr)
    {
        value = proStringUtils::AsUint( attr->Attribute("value") );
        attr->SetLoaded(true);
    }
}

void proObjectReaderXML::GetPropValue(const string &name, proObject *&value, OUT DP_OWN_VALUE &owner)
{
    XMLNode *attr = FindSiblingByName(name);
    if (attr)
    {
        attr->SetLoaded(true);
        XMLNode *oldNode = m_node;
        m_node = attr->FirstChild();
        value = ReadObject(owner);
        m_node = oldNode;
        
    }

}

errResult proObjectReaderXML::EndObject()
{
    m_node = m_node->Parent();
    return ER_Success;
}

XMLNode *proObjectReaderXML::FindChildByName(const string &name)
{
    XMLNode *attr = m_node->FirstChild();
    while (attr)
    {
        if ( strcmp(attr->Attribute("name"), name.c_str()) == 0)
        {
            return attr;
        }
        attr = attr->NextSibling();
    }
    return NULL;
}


XMLNode *proObjectReaderXML::FindSiblingByName(const string &name)
{
    XMLNode *attr = m_node;
    while (attr)
    {
        if ( strcmp(attr->Attribute("name"), name.c_str()) == 0)
        {
            return attr;
        }
        attr = attr->NextSibling();
    }
    return NULL;
}


static const pair<const char*,const char*> TypeStrings[] = 
{
    make_pair("int",        "proPropertyIntOwner"),
    make_pair("uint",       "proPropertyUintOwner"),
    make_pair("float",      "proPropertyFloatOwner"),
    make_pair("bool",       "proPropertyBoolOwner"),
    make_pair("string",     "proPropertyStringOwner"),
    make_pair("wstring",    "proPropertyWstringOwner"),
    make_pair("object",     "proPropertyObjectOwner"),
    make_pair((const char*)NULL, (const char*)NULL)
};

proProperty *proObjectReaderXML::ReadDynamicProperty(proObject *object)
{
    // create a property for the next not-loaded node
    XMLNode *node = m_node;
    while (node)
    {
        if (node->GetLoaded())
        {
            node = node->NextSibling();
            continue;
        }

        const char *typeName = node->Attribute("type");
        const char *propertyName = node->Attribute("name");

        int i=0;
        const char *propertyClassName = NULL;
        while (1)
        {
            if (TypeStrings[i].first == NULL)
                break;

            if (strcmp(typeName, TypeStrings[i].first) == 0)
            {
                propertyClassName = TypeStrings[i].second;
            }
            i++;
        }
        
        // found a basic type
        if (propertyClassName)
        {
            proProperty *prop =  proClassRegistry::NewInstance<proProperty*>(propertyClassName);
            prop->SetName(propertyName);
            node->SetLoaded(true);

            prop->Read(this, object);
            return prop;
        }

        // may be a custom type
        proProperty *prop = proClassRegistry::NewInstance<proProperty*>(typeName);
        if (prop)
        {
            prop->SetName(propertyName);
            node->SetLoaded(true);
            prop->Read(this, object);
            return prop;
        }
        else 
        {
            return NULL;
        }
    }
    return NULL;
}



