/**
 *  property system
 *
 *  @author Sean Riley
 *
 *  @date 1/13/2003
 *
 *  @file
**/

#ifndef __proPropertyOwner_h__
#define __proPropertyOwner_h__

#include "proFirst.h"

#include "arda2/core/corStdString.h"
#include "arda2/core/corStlVector.h"
#include "arda2/core/corStlMap.h"

#include "proStringUtils.h"

class proObjectReader;
class proObjectWriter;
class proObject;

#include "arda2/properties/proProperty.h"
#include "arda2/properties/proClassRegistry.h"

/**
 * value types for storing owner property values.
 * these live in the associated proObject, not in the property itself
 */
template <typename T> class proValueTyped : public proValue
{
public:
    proValueTyped(const std::string &name) : proValue(name), m_value(0) {}
    T m_value;
};

template <> class proValueTyped<std::string> : public proValue
{
public:
    proValueTyped(const std::string &name) : proValue(name), m_value("") {}
    std::string m_value;
};

template <> class proValueTyped<proObject*> : public proValue
{
public:
    proValueTyped(const std::string &name) : proValue(name), m_value(NULL), m_owner(DP_ownValue) {}
    ~proValueTyped()
    {
        if (m_owner == DP_ownValue)
            SafeDelete(m_value);
    }
    proObject*      m_value;
    DP_OWN_VALUE    m_owner;
};

/**
 *  template for owner properties - implements the typed get and set functions
**/
template <typename TYPE_GET, typename TYPE_SET, typename T>
class proPropertyOwner : public T
{
public:

    proPropertyOwner() :
        m_declaringClass(NULL)
        {
        }

    /**
     * Gets a value object from an object and returns its typed value.
     * For un-instantiated values, the default is returned.
     */
    virtual TYPE_GET GetValue(const proObject *o) const
    {
        proValueTyped<TYPE_GET> *v = GetOwnerValue(o);
        if (!v)
        {
            // never been set, return a default value
            return T::GetDefault();
        }
        return v->m_value;
    }

    // sets a value object on an object. creates value of object if not found
    virtual void SetValue(proObject *o, TYPE_SET value)
    {
        proValueTyped<TYPE_GET> *v = GetOwnerValue(o);
        if (v)
        {
            v->m_value = value;
        }
        else
        {
            v = new proValueTyped<TYPE_GET>(m_name);
            v->m_value = value;
            AddOwnerValue(o, v);
        }
        if (o)
            o->PropertyChanged(GetName());
    }
    virtual const std::string &GetName() const { return m_name; }
    virtual void SetName(const std::string &name) { m_name = name; }

    /**
     * use if the property is already applied to an object.
     * makes sure the associated proValue gets updated.
     */
    virtual void Rename(const std::string &name, proObject *o = NULL ) 
    {
        if (o)
        {
            proValueTyped<TYPE_GET> *v = GetOwnerValue(o);
            if (v)
            {
                v->SetName(name);
            }
        }
        m_name = name; 
    }

    virtual proClass *GetDeclaringClass() const { return m_declaringClass; }
    virtual void SetDeclaringClass(proClass *cls) { m_declaringClass = cls; }

    virtual void Clone(const proObject* source, proObject* dest)
    {
        proPropertyOwner *newProp = (proPropertyOwner*) this->GetClass()->NewInstance();
        newProp->SetName(GetName());
        dest->AddInstanceProperty(newProp);

        proValueTyped<TYPE_GET> *v = GetOwnerValue(source);
        if (v)
            newProp->SetValue(dest, v->m_value);
    }
    
protected:

    proValueTyped<TYPE_GET> *GetOwnerValue(const proObject *o) const
    {
        if (m_declaringClass)
            return static_cast< proValueTyped<TYPE_GET>*>(m_declaringClass->GetOwnerValue(m_name));
        else 
            return static_cast< proValueTyped<TYPE_GET>*>(o->GetOwnerValue(m_name));
    }

    void AddOwnerValue(proObject *o, proValueTyped<TYPE_GET> *value)
    {
        if (m_declaringClass)
            m_declaringClass->AddOwnerValue(value);
        else
            o->AddOwnerValue(value);
    }
    std::string        m_name;
    proClass     *m_declaringClass;
};


/*
*/
class proPropertyIntOwner : public proPropertyOwner<int,const int,proPropertyInt>
{
    PRO_DECLARE_OBJECT
public:
    typedef int MyType;

    proPropertyIntOwner() {}

    virtual std::string  ToString(const proObject*o ) const { return proStringUtils::AsString(GetValue(o)); }
    virtual void    FromString(proObject* o, const std::string& aValue ) 
    { 
        SetValue( o, (const int)proStringUtils::AsInt(aValue)); 
    }

    virtual proDataType GetDataType() const;
    virtual ~proPropertyIntOwner() {};
};

/*
*/
class proPropertyUintOwner : public proPropertyOwner<uint,const uint,proPropertyUint>
{
    PRO_DECLARE_OBJECT
public:
    typedef uint MyType;

    proPropertyUintOwner() {}

    virtual std::string  ToString(const proObject *o ) const { return proStringUtils::AsString(GetValue(o)); }
    virtual void    FromString(proObject* o, const std::string& aValue ) { SetValue( o, proStringUtils::AsUint(aValue)); }

    virtual proDataType GetDataType() const;
    virtual ~proPropertyUintOwner() {};
};

/*
*/
class proPropertyFloatOwner : public proPropertyOwner<float,const float,proPropertyFloat>
{
    PRO_DECLARE_OBJECT
public:
    typedef float MyType;
    
    proPropertyFloatOwner() {}

    virtual std::string  ToString(const proObject *o) const { return proStringUtils::AsString(GetValue(o)); }
    virtual void    FromString(proObject* o, const std::string& aValue ) { SetValue( o, proStringUtils::AsFloat(aValue)); }

    virtual proDataType GetDataType() const;
    virtual ~proPropertyFloatOwner() {};
};

/*
*/
class proPropertyBoolOwner : public proPropertyOwner<bool,const bool,proPropertyBool>
{
    PRO_DECLARE_OBJECT
public:
    typedef bool MyType;

    proPropertyBoolOwner() {}

    virtual std::string  ToString(const proObject *o ) const { return proStringUtils::AsString(GetValue(o)); }
    virtual void    FromString(proObject* o, const std::string& aValue ) { SetValue( o, (const bool&)proStringUtils::AsBool(aValue)); }

    virtual proDataType GetDataType() const;
    virtual ~proPropertyBoolOwner() {};
};

/*
*/
class proPropertyStringOwner : public proPropertyOwner<std::string, const std::string &, proPropertyString>
{
    PRO_DECLARE_OBJECT
public:
    typedef std::string MyType;

    proPropertyStringOwner() {}

    virtual std::string  ToString(const proObject *o ) const { return GetValue(o); }
    virtual void    FromString(proObject* o, const std::string& aValue ) { SetValue( o, aValue); }

    virtual proDataType GetDataType() const;
    virtual ~proPropertyStringOwner() {};
};

/*
*/
class proPropertyWstringOwner : public proPropertyOwner<std::wstring, const std::wstring &,proPropertyWstring>
{
    PRO_DECLARE_OBJECT
public:
    typedef std::wstring MyType;

    proPropertyWstringOwner() {}

    virtual std::string  ToString(const proObject*  ) const {return std::string("Wstring is not convertible to std::string"); }
    virtual void    FromString(proObject* o, const std::string& aValue ) { UNREF(o); UNREF(aValue); }

    virtual proDataType GetDataType() const;
    virtual ~proPropertyWstringOwner() {};
};


class proPropertyObjectOwner : public proPropertyOwner<proObject*,proObject*,proPropertyObject>
{
    PRO_DECLARE_OBJECT
public:
    typedef proObject* MyType;

    proPropertyObjectOwner(DP_OWN_VALUE own = DP_refValue)  { m_ownValue = own;}

    virtual std::string  ToString(const proObject*  ) const;
    virtual void    FromString(proObject* o, const std::string& aValue ) { UNREF(o); UNREF(aValue);  }

    virtual proDataType GetDataType() const;
    virtual ~proPropertyObjectOwner() {};

    // specialization that also sets the ownership of the proValue
    virtual void SetValue(proObject *o, proObject* value)
    {
         proValueTyped<proObject*> *v = GetOwnerValue(o);
        if (v)
        {
            v->m_value = value;
        }
        else
        {
            v = new proValueTyped<proObject*>(m_name);
            v->m_value = value;
            v->m_owner = m_ownValue;
            AddOwnerValue(o, v);
        }
        if (o)
            o->PropertyChanged(GetName());
    }

    
    void SetOwner(proObject *o, DP_OWN_VALUE own)
    {
        proValueTyped<proObject*> *value = GetOwnerValue(o);
        value->m_owner = own;
        m_ownValue = own;
    }

    DP_OWN_VALUE GetOwner(const proObject *o) const
    {
        proValueTyped<proObject*> *value = GetOwnerValue(o);
        return value->m_owner;
    }

    virtual void Read(proObjectReader* in, proObject* obj);

    virtual void Clone(const proObject* source, proObject* dest);

protected:
    DP_OWN_VALUE  m_ownValue;

};

class proPropertyEnumOwner : public proPropertyOwner<uint,const uint,proPropertyEnum>
{
    PRO_DECLARE_OBJECT
public:
    typedef uint MyType;

    proPropertyEnumOwner() {}

    virtual std::string  ToString(const proObject *o ) const { return proStringUtils::AsString(GetValue(o)); }
    virtual void    FromString(proObject* o, const std::string& aValue ) { proPropertyOwner<uint,const uint,proPropertyEnum>::SetValue( o, proStringUtils::AsUint(aValue)); }

    virtual void    SetValue(proObject *o, const uint value)
    {
        proPropertyOwner<uint,const uint,proPropertyEnum>::SetValue(o, value);
    }

    virtual void    SetValue(proObject* o, const std::string &aValue ) 
    {
        std::map<std::string,uint>::iterator it = m_enumeration.find(aValue);
        if (it == m_enumeration.end())
        {
            proPropertyOwner<uint,const uint,proPropertyEnum>::SetValue(o, proStringUtils::AsUint(aValue) );
        }
        else
        {
            proPropertyOwner<uint,const uint,proPropertyEnum>::SetValue(o, (*it).second);
        }
        o->PropertyChanged(GetName());
    }

    virtual proDataType GetDataType() const;
    virtual ~proPropertyEnumOwner() {};
};


class proPropertyBitfieldOwner : public proPropertyOwner<uint,const uint,proPropertyBitfield>
{
    PRO_DECLARE_OBJECT
public:
    typedef uint MyType;

    proPropertyBitfieldOwner() {}

    virtual std::string  ToString(const proObject *o ) const { return proStringUtils::AsString(GetValue(o)); }
    virtual void    FromString(proObject* o, const std::string& aValue ) { proPropertyOwner<uint,const uint,proPropertyBitfield>::SetValue( o, proStringUtils::AsUint(aValue)); }

    virtual void    SetValue(proObject *o, const uint value)
    {
        proPropertyOwner<uint,const uint,proPropertyBitfield>::SetValue(o, value);
    }

    virtual void    SetValue(proObject* o, const std::string &aValue ) 
    {
        uint bits = GetBitfieldValue(aValue);
        proPropertyOwner<uint,const uint,proPropertyBitfield>::SetValue(o, bits);
        o->PropertyChanged(GetName());
    }

    virtual proDataType GetDataType() const;
    virtual ~proPropertyBitfieldOwner() {};
};

#define IMPORT_PROPERTY_CLASS(classname) \
    extern _register_class_##classname _init_##classname; \
    void *___init_##classname = &_init_##classname;\
    ___init_##classname = NULL;



void ImportOwnerProperties();

#endif //__proPropertyOwner_h__
