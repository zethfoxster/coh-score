/**
 *  XML streams for property objects
 *
 *  @author Sean Riley
 *
 *  @date 1/13/2003
 *
 *  @file
**/

#ifndef __OBJECT_XML_READER_H__
#define __OBJECT_XML_READER_H__

#include "proFirst.h"


#include "proObjectReader.h"
#include "arda2/core/corStlHashmap.h"

class proObject;
class proClass;

class XMLDocument;
class XMLNode;

/**
 *  for unserializing property objects from XML data
 *  TODO: object reader needs array support, when added, make containers use it
**/
class proObjectReaderXML : public proObjectReader
{
    PRO_DECLARE_OBJECT
public:
    proObjectReaderXML() : proObjectReader(), m_document(NULL), m_node(NULL), m_lastRef(0) {}
    proObjectReaderXML(proStreamInput *source);
    virtual ~proObjectReaderXML();

    virtual proObject *ReadObject(OUT DP_OWN_VALUE &owner);

    virtual proProperty *ReadDynamicProperty(proObject *object);

    virtual errResult BeginObject(const std::string &className, int &version);
    virtual void GetPropValue(const std::string &name, int &value);
    virtual void GetPropValue(const std::string &name, bool &value);
    virtual void GetPropValue(const std::string &name, std::string &value);
    virtual void GetPropValue(const std::string &name, std::wstring &value);
    virtual void GetPropValue(const std::string &name, float &value);
    virtual void GetPropValue(const std::string &name, uint &value);
    virtual void GetPropValue(const std::string &name, proObject *&value, OUT DP_OWN_VALUE &owner);
    virtual errResult EndObject();


protected:

    errResult LoadDocument();

    XMLDocument *m_document;
    XMLNode     *m_node;

    XMLNode *FindSiblingByName(const std::string &name);
    XMLNode *FindChildByName(const std::string &name);

    std::string m_workingClassName;


    typedef HashMap<uint,proObject*> RefMap;
    RefMap m_references;
    uint   m_lastRef;

};

#endif

