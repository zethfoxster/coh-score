/**
 *  XML streams for property objects
 *
 *  @author Sean Riley
 *
 *  @date 1/13/2003
 *
 *  @file
**/


#ifndef __OBJECT_XML_WRITER_H__
#define __OBJECT_XML_WRITER_H__

#include "proFirst.h"

#include "proObjectWriter.h"
#include "arda2/core/corStlMap.h"
#include "arda2/properties/proObjectWriter.h"

class proObject;
class proClass;

/**
 * for serializing property objects in XML format
 * TODO: object reader needs array support, when added, make containers use it
**/
class proObjectWriterXML : public proObjectWriter
{
    PRO_DECLARE_OBJECT
public:
    proObjectWriterXML() : proObjectWriter(), m_indentation(0), m_lastRef(0){}
    proObjectWriterXML(proStreamOutput *output);
    virtual ~proObjectWriterXML();

    virtual void WriteObject(const proObject *obj);

    virtual bool CheckReference(const proObject *obj); 

    virtual void BeginObject(const std::string &className, const int version);
    virtual void PutPropValue(const std::string &name, const int value);
    virtual void PutPropValue(const std::string &name, const bool value);
    virtual void PutPropValue(const std::string &name, const std::string &value);
    virtual void PutPropValue(const std::string &name, const std::wstring &value);
    virtual void PutPropValue(const std::string &name, const float value);
    virtual void PutPropValue(const std::string &name, const uint value);
    virtual void PutPropValue(const std::string &name, const proObject *obj);

    // used to write out additional types as strings
    virtual void PutPropTyped(const std::string &name, const std::string &value, const std::string &typeName);

    virtual void EndObject();

protected:


    void PutTypedStringValue(const std::string &name, const std::string &value, const std::string &type);
    void Indent(std::string &output);
    int m_indentation;

    typedef std::map<proObject*,uint> RefMap;
    RefMap m_references;
    uint   m_lastRef;
};


#endif
