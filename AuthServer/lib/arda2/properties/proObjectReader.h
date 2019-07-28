/**
 *  property system
 *
 *  @author Sean Riley
 *
 *  @date 1/13/2003
 *
 *  @file
**/

#ifndef __OBJECT_READER_H__
#define __OBJECT_READER_H__

#include "arda2/core/corStdString.h"
#include "arda2/properties/proObject.h"

class proProperty;
class proStreamInput;

/**
 * ObjectReader is for unserializing property objects.
**/
class proObjectReader : public proObject
{
    PRO_DECLARE_OBJECT
public:
    proObjectReader() : m_source(NULL) {}
    proObjectReader(proStreamInput *source) : m_source(source) {}
    virtual ~proObjectReader() {}

    void SetSource(proStreamInput *source) { m_source = source; }

    virtual proObject *ReadObject(OUT DP_OWN_VALUE &owner) = 0;

    virtual proProperty *ReadDynamicProperty(proObject *object) = 0;

    // invoked from within proObject or derived classes
    virtual errResult BeginObject(const std::string &className, int &version) = 0;
    virtual void GetPropValue(const std::string &name, int &value) = 0;
    virtual void GetPropValue(const std::string &name, bool &value) = 0;
    virtual void GetPropValue(const std::string &name, std::string &value) = 0;
    virtual void GetPropValue(const std::string &name, std::wstring &value) = 0;
    virtual void GetPropValue(const std::string &name, float &value) = 0;
    virtual void GetPropValue(const std::string &name, uint &value) = 0;
    virtual void GetPropValue(const std::string &name, proObject *&value, OUT DP_OWN_VALUE &owner) = 0;
    virtual errResult EndObject() = 0;

protected:
    proStreamInput *m_source;

    // this enables derived classes to access the friend-ed proObject::ReadObject method
    void InternalReadObject(proObject* obj)
    {
        obj->ReadObject(this);
    }

};

#endif

