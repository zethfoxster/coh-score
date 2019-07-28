/**
 *  property system
 *
 *  @author Allen Jackson
 *
 *  @date 1/3/2006
 *
 *  @file
**/

#ifndef __proPropertyDelegate_h__
#define __proPropertyDelegate_h__

#include "proFirst.h"

//*****************************************************************************

#define REG_PROPERTY_INTERNAL_DELEGATE( t_get, t_set, pre, tn, c, p, aGetter, aSetter, annotations)\
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
        static_cast<c*>(o)->aSetter(val);\
        o->PropertyChanged(s_name);\
    } \
    t_get GetValue(const proObject* o) const \
    { \
        return static_cast<const c*>(o)->aGetter();\
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
Property_##c##_##p##Ref regProperty_##c##_##p;\
const std::string Property_##c##_##p##Ref::s_name = "" #p ""; \
PRO_REGISTER_CLASS(Property_##c##_##p##Ref, pre##Property##tn##Native)

//*****************************************************************************
// This macro automatically sets "Object Properties" with certain annotations 
// to mark this property as a proObjet container.

#define REG_PROPERTY_INTERNAL_OBJ_DELEGATE( t_get, t_set, pre, tn, c, p, aGetter, aSetter, clas, annotations)\
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
            realAnnos = std::string(proAnnoStrObjectType) + "|" #clas;\
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
        static_cast< c* >(o)->aSetter( val );\
        o->PropertyChanged(s_name);\
    } \
    \
    t_get GetValue(const proObject* o) const \
    { \
        return static_cast< const c* >( o )->aGetter();\
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
// macros for defining a property enumeration, using delegated functions for
// getting and setting the value

#define REG_PROPERTY_ENUM_INTERNAL_DELEGATE( c, p, aGetter, aSetter, annotations)\
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
    Property_##c##_##p##Ref(const EnumTable* aTable, uint aTableSize)\
    {\
        proClass *cls = proClassRegistry::ClassForName<proClassNative*>(#c);\
        if (cls) { cls->AddProperty(this); }\
        m_isNative = true;\
        SetEnumTable(aTable,aTableSize);\
        proClassRegistry::RegisterAnnotation(this,#annotations);\
    }\
    \
    void SetValue(proObject* anObject, const uint val )\
    {\
        static_cast< c* >(anObject)->aSetter( val );\
        anObject->PropertyChanged(s_name);\
    }\
    \
    uint GetValue(const proObject* anObject) const \
    {\
        return static_cast< const c* >(anObject)->aGetter();\
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
const std::string Property_##c##_##p##Ref::s_name = "" #p "";

//*****************************************************************************

#define REG_PROPERTY_BITFIELD_INTERNAL_DELEGATE( c, p, enumData, aGetter, aSetter, annotations)\
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
    void SetValue(proObject* anObject, const uint val )\
    {\
        static_cast< c* >(anObject)->aSetter( val );\
        anObject->PropertyChanged(s_name);\
    }\
    \
    uint GetValue(const proObject* anObject) const \
    {\
        return static_cast< const c* >(anObject)->aGetter();\
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
Property_##c##_##p##Ref  regProperty_##c##_##p##Ref( #enumData );\
const std::string Property_##c##_##p##Ref::s_name = "" #p "";\
PRO_REGISTER_CLASS(Property_##c##_##p##Ref, proPropertyBitfieldNative)

//-----------------------------------------------------------------------------
// the external property registration macros

// --------------------
// with out annotations

#define REG_PROPERTY_INT_DELEGATE(c, p, get, set)\
    REG_PROPERTY_INTERNAL_DELEGATE(int, const int, pro, Int, c, p, get, set, NULL )

#define REG_PROPERTY_UINT_DELEGATE(c, p, get, set)\
    REG_PROPERTY_INTERNAL_DELEGATE(uint, const uint, pro, Uint, c, p, get, set, NULL )

#define REG_PROPERTY_FLOAT_DELEGATE(c, p, get, set)\
    REG_PROPERTY_INTERNAL_DELEGATE(float, const float, pro, Float, c, p, get, set, NULL )

#define REG_PROPERTY_BOOL_DELEGATE(c, p, get, set)\
    REG_PROPERTY_INTERNAL_DELEGATE(bool, const bool, pro, Bool, c, p, get, set, NULL )

#define REG_PROPERTY_STRING_DELEGATE(c, p, get, set)\
    REG_PROPERTY_INTERNAL_DELEGATE(std::string, const std::string &, pro, String, c, p, get, set, NULL )

#define REG_PROPERTY_WSTRING_DELEGATE(c, p, get, set)\
    REG_PROPERTY_INTERNAL_DELEGATE(std::wstring, const std::wstring &, pro, Wstring, c, p, get, set, NULL )

#define REG_PROPERTY_OBJECT_DELEGATE(c, p, clas, get, set)\
    REG_PROPERTY_INTERNAL_OBJ_DELEGATE(proObject*, proObject *, pro, Object, c, p, get, set, clas, NULL)

#define REG_PROPERTY_BITFIELD_DELEGATE(c, p, enumData, get, set)\
    REG_PROPERTY_BITFIELD_INTERNAL_DELEGATE(c, p, enumData, get, set, NULL )

#define REG_PROPERTY_ENUM_DELEGATE(c, p, enumData, get, set)\
    REG_PROPERTY_ENUM_INTERNAL_DELEGATE(c, p, get, set, NULL )\
    Property_##c##_##p##Ref regProperty_##c##_##p##Ref(#enumData);\
    PRO_REGISTER_CLASS(Property_##c##_##p##Ref, proPropertyEnumNative)

#define REG_PROPERTY_ENUM_TABLE_DELEGATE(c, p, enumTable, enumTableSize, get, set)\
    REG_PROPERTY_ENUM_INTERNAL_DELEGATE(c, p, get, set, NULL )\
    Property_##c##_##p##Ref regProperty_##c##_##p##Ref( enumTable, enumTableSize );\
    PRO_REGISTER_CLASS(Property_##c##_##p##Ref, proPropertyEnumNative)

// ----------------
// with annotations

#define REG_PROPERTY_INT_DELEGATE_ANNO(c, p, get, set, annotations) \
        REG_PROPERTY_INTERNAL_DELEGATE(int, const int, pro, Int, c, p, get, set, annotations )

#define REG_PROPERTY_UINT_DELEGATE_ANNO(c, p, get, set, annotations) \
        REG_PROPERTY_INTERNAL_DELEGATE(uint, const uint, pro, Uint, c, p, get, set, annotations )

#define REG_PROPERTY_FLOAT_DELEGATE_ANNO(c, p, get, set, annotations) \
        REG_PROPERTY_INTERNAL_DELEGATE(float, const float, pro, Float, c, p, get, set, annotations )

#define REG_PROPERTY_BOOL_DELEGATE_ANNO(c, p, get, set, annotations) \
        REG_PROPERTY_INTERNAL_DELEGATE(bool, const bool, pro, Bool, c, p, get, set, annotations )

#define REG_PROPERTY_STRING_DELEGATE_ANNO(c, p, get, set, annotations) \
        REG_PROPERTY_INTERNAL_DELEGATE(std::string, const std::string&, pro, String, c, p, get, set, annotations )

#define REG_PROPERTY_WSTRING_DELEGATE_ANNO(c, p, get, set, annotations) \
        REG_PROPERTY_INTERNAL_DELEGATE(std::wstring, const std::wstring&, pro, Wstring, c, p, get, set, annotations )

#define REG_PROPERTY_OBJECT_DELEGATE_ANNO(c, p, clas, get, set, annotations) \
        REG_PROPERTY_INTERNAL_OBJ_DELEGATE(proObject*, proObject*, pro, Object, c, p, get, set, clas, annotations )

#define REG_PROPERTY_BITFIELD_DELEGATE_ANNO(c, p, enumData, get, set, annotations) \
		REG_PROPERTY_BITFIELD_INTERNAL_DELEGATE(c, p, enumData, get, set, annotations )

#define REG_PROPERTY_ENUM_DELEGATE_ANNO(c, p, enumData, get, set, annotations)\
    REG_PROPERTY_ENUM_INTERNAL_DELEGATE(c, p, get, set, annotations )\
    Property_##c##_##p##Ref regProperty_##c##_##p##Ref(#enumData);\
    PRO_REGISTER_CLASS(Property_##c##_##p##Ref, proPropertyEnumNative)

#define REG_PROPERTY_ENUM_TABLE_DELEGATE_ANNO(c, p, enumTable, enumTableSize, get, set, annotations)\
    REG_PROPERTY_ENUM_INTERNAL_DELEGATE(c, p, get, set, annotations )\
    Property_##c##_##p##Ref regProperty_##c##_##p##Ref( enumTable,  enumTableSize);\
    PRO_REGISTER_CLASS(Property_##c##_##p##Ref, proPropertyEnumNative)

#endif //__proPropertyDelegate_h__
