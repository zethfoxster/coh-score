/**
 *  property system
 *
 *  @author Sean Riley
 *
 *  @date 1/13/2003
 *
 *  @file
**/

#ifndef __OBJECT_WRITER_H__
#define __OBJECT_WRITER_H__

#include "arda2/core/corStdString.h"
#include "arda2/properties/proObject.h"

class proProperty;
class proStreamOutput;

/**
 * ObjectWriter is for serializing property objects.
 *
 * Includes support for writing references to objects that
 * are included in the stream multiple times.
**/
class proObjectWriter : public proObject
{
    PRO_DECLARE_OBJECT
public:
    proObjectWriter() : m_output(NULL) {}
    proObjectWriter(proStreamOutput *output) : m_output(output) {}
    virtual ~proObjectWriter() {}

    void SetOutput(proStreamOutput *output) { m_output = output; }

    // invoked externally to write a object out
    virtual void WriteObject(const proObject *obj) = 0;

    // invoked from within proObject or derived classes
    virtual bool CheckReference(const proObject *obj) = 0; 

    // invoked when NOT a reference
    virtual void BeginObject(const std::string &className, const int version) = 0;
    virtual void PutPropValue(const std::string &name, const int value) = 0;
    virtual void PutPropValue(const std::string &name, const bool value) = 0;
    virtual void PutPropValue(const std::string &name, const std::string &value) = 0;
    virtual void PutPropValue(const std::string &name, const std::wstring &value) = 0;
    virtual void PutPropValue(const std::string &name, const float value) = 0;
    virtual void PutPropValue(const std::string &name, const uint value) = 0;
    virtual void PutPropValue(const std::string &name, const proObject *obj) = 0;

    // used to write out additional types as strings
    virtual void PutPropTyped(const std::string &name, const std::string &value, const std::string &typeName) = 0;

    virtual void EndObject() = 0;

protected:

    proStreamOutput *m_output;

    // this enables derived classes to access the friend-ed proObject::WriteObject method
    void InternalWriteObject(const proObject* obj)
    {
        obj->WriteObject(this);
    }
};



#endif

