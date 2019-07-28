/**
 *  property system
 *
 *  @author Sean Riley
 *
 *  @date 1/13/2003
 *
 *  @file
**/

#ifndef __proProperty_h__
#define __proProperty_h__

#include "arda2/properties/proFirst.h"
#include "arda2/core/corStdString.h"
#include "arda2/core/corStlVector.h"
#include "arda2/core/corStlMap.h"
#include "arda2/properties/proObject.h"

class proClass;
class proObjectWriter;
class proObjectReader;
class proObject;

extern const char *proAnnoStrDisplay;
extern const char *proAnnoStrDefault;
extern const char *proAnnoStrMin;
extern const char *proAnnoStrMax;
extern const char *proAnnoStrReadonly;
extern const char *proAnnoStrTransient;
extern const char *proAnnoStrValidator;
extern const char *proAnnoStrObjectType;

/**
 *  core property class
 *
**/
class proProperty : public proObject
{
    PRO_DECLARE_OBJECT
public:

    proProperty() : 
      proObject(),
      m_isNative(false)
      {
      }

    virtual ~proProperty() {}

    // string conversion
    virtual std::string  ToString( const proObject* o ) const = 0;
    virtual void         FromString( proObject* o, const std::string& str ) = 0;

    // the type of basic data represented by this property
    virtual proDataType GetDataType() const = 0;

    // type interface defined in derived base type properties
    //virtual TYPE_GET GetValue(proObject *target) const = 0;
    //virtual void     SetValue(proObject *target, TYPE_SET value) = 0;

    // ------  Annotation Interface ------
    virtual void SetName(const std::string &) {};
    virtual void SetIsNative(bool value)       { m_isNative = value; }
    virtual bool GetIsNative() const         { return m_isNative; }
    // GetName() is implemented in property registration macros for native properties,
    // and in the proPropertyOwner template for owner properties.
    virtual const std::string &GetName() const = 0;

    virtual void SetDeclaringClass(proClass *) {};
    virtual proClass* GetDeclaringClass() const = 0;

    virtual void Clone( const proObject* source, proObject* dest ) = 0;

    // ------ serialization - implemented by typed properties ---------
    virtual void Write( proObjectWriter* out, const proObject* obj ) const  = 0;
    virtual void Read( proObjectReader* in, proObject* obj ) = 0;

    // setup annotations for this property
    virtual void SetAnnotations(const char *annoString);

    // builds a display string for this property value
    virtual std::string GetDisplayString( proObject* );

    // builds an inline display string for this property value
    virtual std::string GetPropertySummaryString( proObject* );

protected:
    // used to parse display and property summary strings
    std::string ParsePropertyString( proObject*, const std::string& inKeyValue );

    // used for custom individual annotations on derived property classes
    virtual void SetAnnotation(const std::string &name, const std::string &value);

    bool   m_isNative;

};

//*****************************************************************************
// macro for declaring base-type properties

#define PROPERTY_BASETYPE(PREFIX, NAME, TYPE_GET, TYPE_SET, DEFAULT) \
\
class PREFIX##Property##NAME : public proProperty\
{\
public:\
    PREFIX##Property##NAME() : proProperty(), m_default(DEFAULT) {}\
    virtual ~PREFIX##Property##NAME() {}\
    \
    virtual TYPE_GET      GetValue(const proObject *target) const = 0;\
    virtual void          SetValue(proObject *target, TYPE_SET value) = 0;\
    \
    virtual std::string        ToString(const proObject* o ) const = 0;\
    virtual void          FromString(proObject* o, const std::string& str ) = 0;\
    \
    virtual void          Clone(const proObject* source, proObject* dest) { SetValue(dest, GetValue(source)); }\
    \
    virtual void          Write(proObjectWriter* out, const proObject* obj ) const;\
    virtual void          Read(proObjectReader* in, proObject* obj );\
    \
    virtual void          SetDefault(TYPE_GET value) { m_default = value; }\
    virtual TYPE_GET      GetDefault() const          { return m_default; }\
    PRO_DECLARE_OBJECT\
    \
protected:\
    virtual void SetAnnotation(const std::string &name, const std::string &value);\
    TYPE_GET m_default;\
};

// declare base-type property classes
PROPERTY_BASETYPE(pro, Int, int, const int, 0)
PROPERTY_BASETYPE(pro, Uint, uint, const uint, 0)
PROPERTY_BASETYPE(pro, Float, float, const float, 0.0f)
PROPERTY_BASETYPE(pro, Bool, bool, const bool, false)
PROPERTY_BASETYPE(pro, String, std::string, const std::string &, "")
PROPERTY_BASETYPE(pro, Wstring, std::wstring, const std::wstring &, L"")
PROPERTY_BASETYPE(pro, Object, proObject *, proObject *, NULL)

/**
 * enum property - derived from Uint property and has additional interface
**/
class proPropertyEnum : public proPropertyUint
{
    PRO_DECLARE_OBJECT
public:
	struct EnumTable
	{
		std::string key;
		uint   value;
	};

    proPropertyEnum() : proPropertyUint() {}
    virtual ~proPropertyEnum() {}

    virtual uint        GetValue(const proObject *target) const = 0;
    virtual void        SetValue(proObject *target, const uint value) = 0;

    virtual std::string      ToString( const proObject* o ) const = 0;
    virtual void        FromString( proObject* o, const std::string& str ) = 0;

    virtual const std::string &GetName() const = 0;
    virtual proClass* GetDeclaringClass() const = 0;

    virtual void        Write( proObjectWriter* out, const proObject* obj ) const;
    virtual void        Read( proObjectReader* in, proObject* obj );

    // --- enumeration interface ---
    void    SetEnumString(const std::string &anEnum);
	void    SetEnumTable(const EnumTable *aTable, uint aTableSize);
    void    GetEnumeration( std::vector<std::string> &value) const;
    uint    GetEnumerationValue( const std::string &astring ) const;
    std::string  GetEnumerationValue(uint value) const;
    
protected:
    std::map<std::string,uint> m_enumeration;
    virtual void SetAnnotation(const std::string &name, const std::string &value);
};

/**
* bitfield property - derived from Uint property and has additional interface
**/
class proPropertyBitfield : public proPropertyUint
{
    PRO_DECLARE_OBJECT
public:
    proPropertyBitfield() : proPropertyUint() {}
    virtual ~proPropertyBitfield() {}

    virtual uint        GetValue(const proObject *target) const = 0;
    virtual void        SetValue(proObject *target, const uint value) = 0;

    virtual std::string      ToString( const proObject* o ) const = 0;
    virtual void        FromString( proObject* o, const std::string& str ) = 0;

    virtual const std::string &GetName() const = 0;
    virtual proClass* GetDeclaringClass() const = 0;

    virtual void        Write( proObjectWriter* out, const proObject* obj ) const;
    virtual void        Read( proObjectReader* in, proObject* obj );

    // --- enumeration interface ---
    void    SetBitfieldString(const std::string &anEnum);
    void    GetBitfield( std::vector<std::string> &value) const;
    uint    GetBitfieldValue( const std::string &astring ) const;
    std::string  GetBitfieldValue(uint value) const;

protected:
    std::map<std::string,uint> m_bitfields;
};


/**
 * the default annotation of properties is a special class. It is a native
 * property that is added dynamically. It is typed to match the built-in 
 * property types. 
 *
 */
template <typename BaseClass, typename GET, typename SET>
class proDefaultAnnotation : public BaseClass    
{                                                           
public:                                                     
    proDefaultAnnotation()                               
    {                                                       
        BaseClass::m_isNative = true;
    }                                                       
    virtual const std::string &GetName() const 
    { 
        static std::string s_name(proAnnoStrDefault);
        return s_name; 
    }
    virtual proClass* GetDeclaringClass() const { return NULL;}
                                                            
    void SetValue(proObject* o, SET val)               
    { 
        static_cast<BaseClass*>(o)->SetDefault(val);
        o->PropertyChanged(proAnnoStrDefault);                         
    } 
    
    GET GetValue(const proObject* o) const 
    { 
        return static_cast<const BaseClass*>(o)->GetDefault();
    } 
}; 

// returns true if the property is read-only
bool proIsReadOnly( const proProperty* );

#endif //__proProperty_h__

