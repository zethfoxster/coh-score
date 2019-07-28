/**
 *  property system
 *
 *  @author Sean Riley
 *
 *  @date 1/13/2003
 *
 *  @file
**/

#ifndef __proPropertyNative_h__
#define __proPropertyNative_h__

#include "proFirst.h"

#include "arda2/core/corStdString.h"
#include "arda2/core/corStlVector.h"
#include "arda2/core/corStlMap.h"

#include "proStringUtils.h"

class proObjectReader;
class proObjectWriter;
class proObject;

#include "arda2/properties/proProperty.h"
#include "arda2/properties/proClassNative.h"

/*
    the basic implementation for the "int" property
*/
class proPropertyIntNative : public proPropertyInt
{
    PRO_DECLARE_OBJECT
public:
    typedef int MyType;

    virtual std::string  ToString(const proObject* o) const { return proStringUtils::AsString(GetValue(o)); }
    virtual void    FromString(proObject* o, const std::string& aValue ) { SetValue(o, proStringUtils::AsInt(aValue)); }

    virtual int     GetValue(const proObject *target) const = 0;
    virtual void    SetValue(proObject *target, const int value) = 0;

    virtual proDataType GetDataType() const;
    virtual void Clone(const proObject* source, proObject* dest );
    virtual     ~proPropertyIntNative() {};
};

/*
    the basic implementation for the "uint" property
*/
class proPropertyUintNative : public proPropertyUint
{
    PRO_DECLARE_OBJECT
public:
    typedef uint MyType;

    virtual std::string  ToString(const proObject* o) const { return proStringUtils::AsString(GetValue(o)); }
    virtual void    FromString(proObject* o, const std::string& aValue ) { SetValue(o, proStringUtils::AsUint(aValue)); }

    virtual uint     GetValue(const proObject *target) const = 0;
    virtual void    SetValue(proObject *target, const uint value) = 0;

    virtual proDataType GetDataType() const;
    virtual void Clone(const proObject* source, proObject* dest );
    virtual ~proPropertyUintNative() {};
};

/*
    the basic implementation for the "float" property
*/
class proPropertyFloatNative : public proPropertyFloat
{
    PRO_DECLARE_OBJECT
public:
    typedef float MyType;
    
    virtual std::string  ToString(const proObject* o) const { return proStringUtils::AsString(GetValue(o)); }
    virtual void    FromString(proObject* o, const std::string& aValue ) { SetValue(o, proStringUtils::AsFloat(aValue)); }

    virtual float   GetValue(const proObject *target) const = 0;
    virtual void    SetValue(proObject *target, const float value) = 0;

    virtual proDataType GetDataType() const;
    virtual void Clone(const proObject* source, proObject* dest );
    virtual ~proPropertyFloatNative() {};
};

/*
    the basic implementation for the "bool" property
*/
class proPropertyBoolNative : public proPropertyBool
{
    PRO_DECLARE_OBJECT
public:
    typedef bool MyType;

    virtual std::string  ToString(const proObject* o) const { return proStringUtils::AsString(GetValue(o)); }
    virtual void    FromString(proObject* o, const std::string& aValue ) { SetValue(o, (const bool&)proStringUtils::AsBool(aValue)); }

    virtual bool    GetValue(const proObject *target) const = 0;
    virtual void    SetValue(proObject *target, const bool value) = 0;

    virtual proDataType GetDataType() const;
    virtual void Clone(const proObject* source, proObject* dest );
    virtual ~proPropertyBoolNative() {};
};

/*
    the basic implementation for the "string" property
*/
class proPropertyStringNative : public proPropertyString
{
    PRO_DECLARE_OBJECT
public:
    typedef std::string MyType;

    virtual std::string  ToString(const proObject* o) const { return GetValue(o); }
    virtual void    FromString(proObject* o, const std::string& aValue ) { SetValue(o, aValue); }

    virtual std::string  GetValue(const proObject *target) const = 0;
    virtual void    SetValue(proObject *target, const std::string &value) = 0;

    virtual proDataType GetDataType() const;
    virtual void Clone(const proObject* source, proObject* dest );
    virtual ~proPropertyStringNative() {};

};

/*
    the basic implementation for the "wide string" property
*/
class proPropertyWstringNative : public proPropertyWstring
{
    PRO_DECLARE_OBJECT
public:
    typedef std::wstring MyType;

    virtual std::string  ToString(const proObject* o) const { UNREF(o); return std::string("Wstring is not convertible to std::string"); }
    virtual void    FromString(proObject* o, const std::string& aValue ) { UNREF(o); UNREF(aValue);  /*SetValue(o, proStringUtils::AsInt(aValue));*/ }

    virtual std::wstring GetValue(const proObject *target) const = 0;
    virtual void    SetValue(proObject *target, const std::wstring &value) = 0;

    virtual proDataType GetDataType() const;
    virtual void Clone(const proObject* source, proObject* dest );
    virtual ~proPropertyWstringNative() {};

};

/*
    the basic implementation for the "proObject*" property
*/
class proPropertyObjectNative : public proPropertyObject
{
    PRO_DECLARE_OBJECT
public:
    typedef proObject* MyType;

    virtual std::string  ToString(const proObject* o) const;
    virtual void    FromString(proObject* o, const std::string& aValue ) { UNREF(o); UNREF(aValue); /*SetValue(o, proStringUtils::AsInt(aValue));*/ }

    virtual proObject *GetValue(const proObject *target) const = 0;
    virtual void    SetValue(proObject *target, proObject *value) = 0;

    virtual proDataType GetDataType() const;
    virtual void Clone(const proObject* source, proObject* dest );
    virtual ~proPropertyObjectNative() {};

};

/*
    the basic implementation for the "enum uint" property
*/
class proPropertyEnumNative : public proPropertyEnum
{
    PRO_DECLARE_OBJECT
public:
    typedef uint MyType;

    virtual std::string  ToString(const proObject* o) const { return GetEnumerationValue( GetValue(o) ); }
    virtual void    FromString(proObject* o, const std::string& aValue ) { SetValue(o, aValue); }

    virtual uint GetValue(const proObject *target) const  = 0;
    virtual void SetValue(proObject *target, const uint value) = 0;

    virtual void SetValue(proObject* o, const std::string &aValue ) 
    {
        std::map<std::string,uint>::iterator it = m_enumeration.find(aValue);
        if (it == m_enumeration.end())
        {
            SetValue(o, proStringUtils::AsUint(aValue) );
        }
        else
        {
            SetValue(o, (*it).second);
        }
        o->PropertyChanged(GetName());
    }

    virtual proDataType GetDataType() const;
    virtual void Clone(const proObject* source, proObject* dest );

    virtual ~proPropertyEnumNative() {};

};

/*
    the basic implementation for the "bit field" property
*/
class proPropertyBitfieldNative : public proPropertyBitfield
{
    PRO_DECLARE_OBJECT
public:
    typedef uint MyType;

    virtual std::string  ToString(const proObject* o) const { return GetBitfieldValue( GetValue(o) ); }
    virtual void    FromString(proObject* o, const std::string& aValue ) { SetValue(o, aValue); }

    virtual uint GetValue(const proObject *target) const  = 0;
    virtual void SetValue(proObject *target, const uint value) = 0;

    virtual void SetValue(proObject* o, const std::string &aValue ) 
    {
        uint bits = GetBitfieldValue(aValue);
        SetValue(o, bits);
        o->PropertyChanged(GetName());
    }

    virtual proDataType GetDataType() const;
    virtual void Clone(const proObject* source, proObject* dest );

    virtual ~proPropertyBitfieldNative() {};

};

//*****************************************************************************
// macros for class getter/setter methods

#define SET_CODE(cls, obj, prop)  ((cls*)obj)->Set##prop(val);
#define SET_CODE_NULL(cls, obj, prop) UNREF(obj); UNREF(val);

#define GET_CODE(cls, obj, prop)  return ((cls*)obj)->Get##prop();
#define GET_CODE_NULL(cls, obj, prop) UNREF(obj); UNREF(val);

//*****************************************************************************
// This macro is to be used by the REG_ macros below. To register properties 
// please use the REG_PROPERTY_XXX macros. Annotations are optional.

#define REG_PROPERTY_INTERNAL( t_get, t_set, pre, tn, c, p, get, set, annotations)\
class Property_##c##_##p##Ref : public pre##Property##tn##Native    \
{\
public:\
    Property_##c##_##p##Ref()\
    {\
        proClass *cls = proClassRegistry::ClassForName<proClass*>(#c);\
        if (cls) { cls->AddProperty(this); }\
        m_isNative = true;\
        proClassRegistry::RegisterAnnotation(this,#annotations);\
    }\
    virtual const std::string &GetName() const { return s_name; }\
    virtual proClass* GetDeclaringClass() const { return proClassRegistry::ClassForName<proClass*>(#c); }\
    static const std::string s_name;\
    \
    void SetValue(proObject* o, t_set val)\
    { \
        set(c, o, p) \
        o->PropertyChanged(s_name);\
    } \
    \
    t_get GetValue(const proObject* o) const \
    { \
        get(c, o, p) \
    } \
 public:\
    virtual proClass *GetClass() const\
    {\
        if (m_impl)\
            return m_impl->GetClass();\
        return s_class;\
    }\
 protected:\
    static proClass *s_class;\
    friend class proClassNative;\
}; \
Property_##c##_##p##Ref  regProperty_##c##_##p; \
const std::string Property_##c##_##p##Ref::s_name = "" #p ""; \
PRO_REGISTER_CLASS(Property_##c##_##p##Ref, pre##Property##tn##Native)

//*****************************************************************************
// This macro automatically sets "Object Properties" with certain annotations 
// to mark this property as a proObjet container.

#define REG_PROPERTY_INTERNAL_OBJ( t_get, t_set, pre, tn, c, p, get, set, clas, annotations)\
class Property_##c##_##p##Ref : public pre##Property##tn##Native    \
{\
public:\
    Property_##c##_##p##Ref()\
    {\
        proClass *cls = proClassRegistry::ClassForName<proClass*>(#c);\
        if (cls) { cls->AddProperty(this); }\
        m_isNative = true;\
        std::string realAnnos;\
        if (strcmp(#annotations,"NULL")==0)\
            realAnnos = std::string(proAnnoStrObjectType) + "|" + #clas;\
        else\
            realAnnos = std::string(#annotations) + "|" + proAnnoStrObjectType + "|" + #clas;\
        proClassRegistry::RegisterAnnotation(this,realAnnos);\
    }\
    virtual const std::string &GetName() const { return s_name; }\
    virtual proClass* GetDeclaringClass() const { return proClassRegistry::ClassForName<proClass*>(#c); }\
    static const std::string s_name;\
    \
    void SetValue(proObject* o, t_set val)\
    { \
        set(c, o, p) \
        o->PropertyChanged(s_name);\
    } \
    \
    t_get GetValue(const proObject* o) const \
    { \
        get(c, o, p) \
    } \
 public:\
    virtual proClass *GetClass() const\
    {\
        if (m_impl)\
            return m_impl->GetClass();\
        return s_class;\
    }\
 protected:\
    static proClass *s_class;\
    friend class proClassNative;\
}; \
Property_##c##_##p##Ref  regProperty_##c##_##p; \
const std::string Property_##c##_##p##Ref::s_name = "" #p ""; \
PRO_REGISTER_CLASS(Property_##c##_##p##Ref, pre##Property##tn##Native)

//***********************************************************

#define REG_PROPERTY_ENUM_INTERNAL( c, p, enumData, get, set, annotations)\
class Property_##c##_##p##Ref : public proPropertyEnumNative\
{\
public:\
    Property_##c##_##p##Ref(const std::string &data="")\
    {\
        proClass *cls = proClassRegistry::ClassForName<proClassNative*>(#c);\
        if (cls) { cls->AddProperty(this); }\
        m_isNative = true;\
        SetEnumString(data);\
        proClassRegistry::RegisterAnnotation(this,#annotations);\
    }\
    \
    void SetValue(proObject* o, const uint val )\
    {\
        set(c, o, p)\
        o->PropertyChanged(s_name);\
    }\
    \
    uint GetValue(const proObject* o) const \
    {\
        get(c, o, p);\
    }\
    virtual const std::string &GetName() const { return s_name; }\
    virtual proClass* GetDeclaringClass() const { return proClassRegistry::ClassForName<proClass*>(#c); }\
    static const std::string s_name;\
 public:\
    virtual proClass *GetClass() const\
    {\
        if (m_impl)\
            return m_impl->GetClass();\
        return s_class;\
    }\
 protected:\
    static proClass *s_class;\
    friend class proClassNative;\
};\
Property_##c##_##p##Ref  regProperty_##c##_##p##Ref(#enumData);\
const std::string Property_##c##_##p##Ref::s_name = "" #p "";\
PRO_REGISTER_CLASS(Property_##c##_##p##Ref, proPropertyEnumNative)

//***********************************************************

#define REG_PROPERTY_ENUM_TABLE_INTERNAL( c, p, aEnumTable, aEnumTableSize, get, set, annotations)\
class Property_##c##_##p##Ref : public proPropertyEnumNative\
{\
public:\
    \
    Property_##c##_##p##Ref(const std::string &data="")\
    {\
        proClass *cls = proClassRegistry::ClassForName<proClassNative*>(#c);\
        if (cls) { cls->AddProperty(this); }\
        m_isNative = true;\
        SetEnumString(data);\
        proClassRegistry::RegisterAnnotation(this,#annotations);\
    }\
    \
    Property_##c##_##p##Ref(const EnumTable* aTable, uint aTableSize)\
    {\
        proClass *cls = proClassRegistry::ClassForName<proClassNative*>(#c);\
        if (cls) { cls->AddProperty(this); }\
        m_isNative = true;\
        SetEnumTable(aTable,aTableSize);\
        proClassRegistry::RegisterAnnotation(this,#annotations);\
    }\
    \
    void SetValue(proObject* o, const uint val )\
    {\
        set(c, o, p)\
        o->PropertyChanged(s_name);\
    }\
    \
    uint GetValue(const proObject* o) const \
    {\
        get(c, o, p);\
    }\
    virtual const std::string &GetName() const { return s_name; }\
    virtual proClass* GetDeclaringClass() const { return proClassRegistry::ClassForName<proClass*>(#c); }\
    static const std::string s_name;\
 public:\
    virtual proClass *GetClass() const\
    {\
        if (m_impl)\
            return m_impl->GetClass();\
        return s_class;\
    }\
 protected:\
    static proClass *s_class;\
    friend class proClassNative;\
};\
Property_##c##_##p##Ref regProperty_##c##_##p##Ref( aEnumTable, aEnumTableSize );\
const std::string Property_##c##_##p##Ref::s_name = "" #p "";\
PRO_REGISTER_CLASS(Property_##c##_##p##Ref, proPropertyEnumNative)

//***********************************************************

#define REG_PROPERTY_BITFIELD_INTERNAL( c, p, enumData, get, set, annotations)\
class Property_##c##_##p##Ref : public proPropertyBitfieldNative\
{\
public:\
    Property_##c##_##p##Ref(const std::string &data="")\
    {\
		proClass *cls = proClassRegistry::ClassForName<proClassNative*>(#c);\
		if (cls) { cls->AddProperty(this); }\
		m_isNative = true;\
		SetBitfieldString(data);\
		proClassRegistry::RegisterAnnotation(this,#annotations);\
    }\
    \
    void SetValue(proObject* o, const uint val )\
    {\
		set(c, o, p)\
		o->PropertyChanged(s_name);\
    }\
    \
    uint GetValue(const proObject* o) const\
    {\
		get(c, o, p);\
    }\
    virtual const std::string &GetName() const { return s_name; }\
    virtual proClass* GetDeclaringClass() const { return proClassRegistry::ClassForName<proClass*>(#c); }\
    static const std::string s_name;\
public:\
    virtual proClass *GetClass() const\
    {\
    if (m_impl)\
    return m_impl->GetClass();\
    return s_class;\
    }\
protected:\
    static proClass *s_class;\
    friend class proClassNative;\
};\
Property_##c##_##p##Ref  regProperty_##c##_##p##Ref(#enumData);\
const std::string Property_##c##_##p##Ref::s_name = "" #p "";\
PRO_REGISTER_CLASS(Property_##c##_##p##Ref, proPropertyBitfieldNative)

//***********************************************************

#define REG_PROPERTY_INT(c, p)          REG_PROPERTY_INTERNAL(int, const int, pro, Int, c, p, GET_CODE, SET_CODE, NULL )
#define REG_PROPERTY_UINT(c, p)         REG_PROPERTY_INTERNAL(uint, const uint, pro, Uint, c, p, GET_CODE, SET_CODE, NULL )
#define REG_PROPERTY_FLOAT(c, p)        REG_PROPERTY_INTERNAL(float, const float, pro, Float, c, p,GET_CODE, SET_CODE, NULL )
#define REG_PROPERTY_BOOL(c, p)         REG_PROPERTY_INTERNAL(bool, const bool, pro, Bool, c, p, GET_CODE, SET_CODE, NULL )
#define REG_PROPERTY_STRING(c, p)       REG_PROPERTY_INTERNAL(std::string, const std::string &, pro, String, c, p, GET_CODE, SET_CODE, NULL )
#define REG_PROPERTY_WSTRING(c, p)      REG_PROPERTY_INTERNAL(std::wstring, const std::wstring &, pro, Wstring, c, p, GET_CODE, SET_CODE, NULL )
#define REG_PROPERTY_OBJECT(c, p, clas) REG_PROPERTY_INTERNAL_OBJ(proObject*, proObject *, pro, Object, c, p, GET_CODE, SET_CODE, clas, NULL)
#define REG_PROPERTY_ENUM(c, p, enumData) REG_PROPERTY_ENUM_INTERNAL(c, p, enumData, GET_CODE, SET_CODE, NULL )
#define REG_PROPERTY_ENUM_TABLE(c, p, enumTable, enumTableSize) REG_PROPERTY_ENUM_TABLE_INTERNAL(c, p, enumTable, enumTableSize, GET_CODE, SET_CODE, NULL )
#define REG_PROPERTY_BITFIELD(c, p, enumData) REG_PROPERTY_BITFIELD_INTERNAL(c, p, enumData, GET_CODE, SET_CODE, NULL )

#define REG_PROPERTY_INT_ANNO(c, p, annotations) \
        REG_PROPERTY_INTERNAL(int, const int, pro, Int, c, p, GET_CODE, SET_CODE, annotations )

#define REG_PROPERTY_UINT_ANNO(c, p, annotations) \
        REG_PROPERTY_INTERNAL(uint, const uint, pro, Uint, c, p, GET_CODE, SET_CODE, annotations )

#define REG_PROPERTY_FLOAT_ANNO(c, p, annotations) \
        REG_PROPERTY_INTERNAL(float, const float, pro, Float, c, p, GET_CODE, SET_CODE, annotations )

#define REG_PROPERTY_BOOL_ANNO(c, p, annotations) \
        REG_PROPERTY_INTERNAL(bool, const bool, pro, Bool, c, p, GET_CODE, SET_CODE, annotations )

#define REG_PROPERTY_STRING_ANNO(c, p, annotations) \
        REG_PROPERTY_INTERNAL(std::string, const std::string&, pro, String, c, p, GET_CODE, SET_CODE, annotations )

#define REG_PROPERTY_WSTRING_ANNO(c, p, annotations) \
        REG_PROPERTY_INTERNAL(std::wstring, const std::wstring&, pro, Wstring, c, p, GET_CODE, SET_CODE, annotations )

#define REG_PROPERTY_OBJECT_ANNO(c, p, clas, annotations) \
        REG_PROPERTY_INTERNAL_OBJ(proObject*, proObject*, pro, Object, c, p, GET_CODE, SET_CODE, clas, annotations )

#define REG_PROPERTY_ENUM_ANNO(c, p, enumData, annotations) \
        REG_PROPERTY_ENUM_INTERNAL(c, p, enumData, GET_CODE, SET_CODE, annotations )

#define REG_PROPERTY_ENUM_TABLE_ANNO(c, p, enumTable, enumTableSize, annotations)\
        REG_PROPERTY_ENUM_TABLE_INTERNAL(c, p, enumTable, enumTableSize, GET_CODE, SET_CODE, annotations )

#define REG_PROPERTY_BITFIELD_ANNO(c, p, enumData, annotations) \
		REG_PROPERTY_BITFIELD_INTERNAL(c, p, enumData, GET_CODE, SET_CODE, annotations )

#endif //__proPropertyNative_h__
