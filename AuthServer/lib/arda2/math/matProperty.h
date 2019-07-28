

#ifndef   INCLUDED_matProperties_h
#define   INCLUDED_matProperties_h
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "arda2/math/matVector3.h"
#include "arda2/math/matMatrix4x4.h"
#include "arda2/math/matQuaternion.h"
#include "arda2/math/matColor.h"
#include "arda2/math/matColorInt.h"
#include "arda2/core/corStdString.h"

#include "arda2/properties/proFirst.h"
#include "arda2/properties/proPropertyNative.h"
#include "arda2/properties/proPropertyOwner.h"


void proRegisterMathTypes();

namespace matStringUtils
{
    matVector3      AsVector3(const std::string& value);
    matMatrix4x4    AsMatrix(const std::string& value);
    matVector3      AsVector3(const std::wstring& value);
    matMatrix4x4    AsMatrix(const std::wstring& value);
    matVector3      AsVector3(const std::wstring& value);
    matMatrix4x4    AsMatrix(const std::wstring& value);
    matQuaternion   AsQuaternion(const std::string& value);
    matColor        AsColor(const std::string& value);
    matColorInt     AsColorInt(const std::string& value);

    std::string          AsString(const matQuaternion& value);
    std::string          AsString(const matVector3& value);
    std::string          AsString(const matMatrix4x4& value);
    std::string          AsString(const matColor& value);
    std::string          AsString(const matColorInt& value);
    std::wstring         AsWstring(const matVector3& value);
    std::wstring         AsWstring(const matMatrix4x4& value);
};

#include "arda2/properties/proClass.h"
#include "arda2/properties/proProperty.h"
#include "arda2/properties/proPropertyOwner.h"


////// ------------------ specializations of proValue for math properties ----------------------

template <> 
class proValueTyped<matMatrix4x4> : public proValue
{
public:
    proValueTyped(const std::string &name) : proValue(name), m_value() {}
    matMatrix4x4 m_value;
};

template <> 
class proValueTyped<matVector3> : public proValue
{
public:
    proValueTyped(const std::string &name) : proValue(name), m_value() {}
    matVector3 m_value;
};

template <> 
class proValueTyped<matColor> : public proValue
{
public:
    proValueTyped(const std::string &name) : proValue(name), m_value() {}
    matColor m_value;
};

template <> 
class proValueTyped<matColorInt> : public proValue
{
public:
    proValueTyped(const std::string &name) : proValue(name), m_value() {}
    matColorInt m_value;
};

template <> 
class proValueTyped<matQuaternion> : public proValue
{
public:
    proValueTyped(const std::string &name) : proValue(name), m_value() {}
    matQuaternion m_value;
};

////// ------------------ base classes for math properties ----------------------

/**
 * base class for matrix properties
*/
class matPropertyMatrix : public proProperty
{
    PRO_DECLARE_OBJECT
public:
    matPropertyMatrix() : proProperty() {}
    virtual ~matPropertyMatrix() {}
    
    virtual matMatrix4x4  GetValue(const proObject *target) const = 0;
    virtual void          SetValue(proObject *target, const matMatrix4x4 &value) = 0;
    
    virtual std::string        ToString(const proObject* o) const { return matStringUtils::AsString(GetValue(o)); }
    virtual void          FromString(proObject* o, const std::string& aValue) { SetValue(o, matStringUtils::AsMatrix(aValue));  }
    
    virtual proDataType   GetDataType() const { return DT_other; }
    virtual void          Clone(const proObject* source, proObject* dest) { SetValue(dest, GetValue(source)); }
    
    virtual void          Write(proObjectWriter* out, const proObject* obj ) const;
    virtual void          Read(proObjectReader* in, proObject* obj );

    matMatrix4x4 GetDefault() const;
    void SetAnnotation(const std::string &name, const std::string &value);
};

/**
 * base class for Vector properties
*/
class matPropertyVector3 : public proProperty
{
    PRO_DECLARE_OBJECT
public:
    matPropertyVector3() : proProperty() {}
    virtual ~matPropertyVector3() {}
    
    virtual matVector3    GetValue(const proObject *target) const = 0;
    virtual void          SetValue(proObject *target, const matVector3 &value) = 0;
    
    virtual std::string        ToString(const proObject* o) const { return matStringUtils::AsString(GetValue(o)); }
    virtual void          FromString(proObject* o, const std::string& aValue) { SetValue(o, matStringUtils::AsVector3(aValue));  }
    
    virtual proDataType   GetDataType() const { return DT_other; }
    virtual void          Clone(const proObject* source, proObject* dest) { SetValue(dest, GetValue(source)); }
    
    virtual void          Write(proObjectWriter* out, const proObject* obj ) const;
    virtual void          Read(proObjectReader* in, proObject* obj );

    matVector3 GetDefault() const;
    void SetAnnotation(const std::string &name, const std::string &value);

    virtual std::string GetDisplayString( proObject* );
    virtual std::string GetPropertySummaryString( proObject* );
protected:
    std::string BuildDisplayString( proObject*, const std::string& );
};

/**
 * base class for Color properties
*/
class matPropertyColor : public proProperty
{
    PRO_DECLARE_OBJECT
public:
    matPropertyColor() : proProperty() {}
    virtual ~matPropertyColor() {}
    
    virtual matColor      GetValue(const proObject *target) const = 0;
    virtual void          SetValue(proObject *target, const matColor &value) = 0;
    
    virtual std::string        ToString(const proObject* o) const { return matStringUtils::AsString(GetValue(o)); }
    virtual void          FromString(proObject* o, const std::string& aValue) { SetValue(o, matStringUtils::AsColor(aValue));  }
    
    virtual proDataType   GetDataType() const { return DT_other; }
    virtual void          Clone(const proObject* source, proObject* dest) { SetValue(dest, GetValue(source)); }
    
    virtual void          Write(proObjectWriter* out, const proObject* obj ) const;
    virtual void          Read(proObjectReader* in, proObject* obj );

    matColor GetDefault() const;
    void SetAnnotation(const std::string &name, const std::string &value);

    virtual std::string GetDisplayString( proObject* );
    virtual std::string GetPropertySummaryString( proObject* );

protected:
    std::string BuildDisplayString( proObject*, const std::string& );
};

/**
 * base class for Color Int properties
*/
class matPropertyColorInt : public proProperty
{
    PRO_DECLARE_OBJECT
public:
    matPropertyColorInt() : proProperty() {}
    virtual ~matPropertyColorInt() {}
    
    virtual matColorInt   GetValue(const proObject *target) const = 0;
    virtual void          SetValue(proObject *target, const matColorInt &value) = 0;
    
    virtual std::string        ToString(const proObject* o) const { return matStringUtils::AsString(GetValue(o)); }
    virtual void          FromString(proObject* o, const std::string& aValue) { SetValue(o, matStringUtils::AsColorInt(aValue));  }
    
    virtual proDataType   GetDataType() const { return DT_other; }
    virtual void          Clone(const proObject* source, proObject* dest) { SetValue(dest, GetValue(source)); }
    
    virtual void          Write(proObjectWriter* out, const proObject* obj ) const;
    virtual void          Read(proObjectReader* in, proObject* obj );

    matColorInt GetDefault() const;
    void SetAnnotation(const std::string &name, const std::string &value);

    virtual std::string GetDisplayString( proObject* );
    virtual std::string GetPropertySummaryString( proObject* );

protected:
    std::string BuildDisplayString( proObject*, const std::string& );};

/**
 * base class for Quatenrnion properties
*/
class matPropertyQuaternion : public proProperty
{
    PRO_DECLARE_OBJECT
public:
    matPropertyQuaternion() : proProperty() {}
    virtual ~matPropertyQuaternion() {}
    
    virtual matQuaternion GetValue(const proObject *target) const = 0;
    virtual void          SetValue(proObject *target, const matQuaternion &value) = 0;
    
    virtual std::string        ToString(const proObject* o) const { return matStringUtils::AsString(GetValue(o)); }
    virtual void          FromString(proObject* o, const std::string& aValue) { SetValue(o, matStringUtils::AsQuaternion(aValue));  }
    
    virtual proDataType   GetDataType() const { return DT_other; }
    virtual void          Clone(const proObject* source, proObject* dest) { SetValue(dest, GetValue(source)); }
    
    virtual void          Write(proObjectWriter* out, const proObject* obj ) const;
    virtual void          Read(proObjectReader* in, proObject* obj );

    matQuaternion GetDefault() const;
    void SetAnnotation(const std::string &name, const std::string &value);
};


////// ------------------ classes for native math properties ----------------------

/*
*/
class matPropertyMatrixNative : public matPropertyMatrix
{
    PRO_DECLARE_OBJECT
public:
    typedef matVector3   MyType;
    virtual matMatrix4x4 GetValue(const proObject *target) const = 0;
    virtual void         SetValue(proObject *target, const matMatrix4x4 &value) = 0;
    virtual ~matPropertyMatrixNative() {};
};

/*
*/
class matPropertyVector3Native : public matPropertyVector3
{
    PRO_DECLARE_OBJECT
public:
    typedef matVector3 MyType;
    virtual matVector3 GetValue(const proObject *target) const = 0;
    virtual void       SetValue(proObject *target, const matVector3 &value) = 0;
    virtual ~matPropertyVector3Native() {};
};

/*
*/
class matPropertyColorNative : public matPropertyColor
{
    PRO_DECLARE_OBJECT
public:
    typedef matColor MyType;
    virtual matColor GetValue(const proObject *target) const = 0;
    virtual void     SetValue(proObject *target, const matColor &value) = 0;
    virtual ~matPropertyColorNative() {};
};

/*
*/
class matPropertyColorIntNative : public matPropertyColorInt
{
    PRO_DECLARE_OBJECT
public:
    typedef matColorInt MyType;
    virtual matColorInt GetValue(const proObject *target) const = 0;
    virtual void        SetValue(proObject *target, const matColorInt &value) = 0;
    virtual ~matPropertyColorIntNative() {};
};

/*
 */
class matPropertyQuaternionNative : public matPropertyQuaternion
{
    PRO_DECLARE_OBJECT
public:
    typedef matQuaternion MyType;
    virtual matQuaternion GetValue(const proObject *target) const = 0;
    virtual void          SetValue(proObject *target, const matQuaternion &value) = 0;
    virtual ~matPropertyQuaternionNative() {};
};

// macros to register native math properties
#define REG_PROPERTY_MATRIX(c, p) REG_PROPERTY_INTERNAL(matMatrix4x4, const matMatrix4x4 &, mat, Matrix, c, p, GET_CODE, SET_CODE, NULL)
#define REG_PROPERTY_VECTOR3(c,p)  REG_PROPERTY_INTERNAL(matVector3, const matVector3 &, mat, Vector3, c, p, GET_CODE, SET_CODE, NULL)
#define REG_PROPERTY_QUAT(c,p)     REG_PROPERTY_INTERNAL(matQuaternion, const matQuaternion &, mat, Quaternion, c, p, GET_CODE, SET_CODE, NULL)
#define REG_PROPERTY_COLOR(c,p)    REG_PROPERTY_INTERNAL(matColor, const matColor &, mat, Color, c, p, GET_CODE, SET_CODE, NULL)
#define REG_PROPERTY_COLORINT(c,p) REG_PROPERTY_INTERNAL(matColorInt, const matColorInt &, mat, ColorInt, c, p, GET_CODE, SET_CODE, NULL)

#define REG_PROPERTY_MATRIX_ANNO(c, p, annotations) \
        REG_PROPERTY_INTERNAL(matMatrix4x4, const matMatrix4x4 &, mat, Matrix, c, p, GET_CODE, SET_CODE, annotations)

#define REG_PROPERTY_VECTOR3_ANNO(c,p, annotations) \
        REG_PROPERTY_INTERNAL(matVector3, const matVector3 &, mat, Vector3, c, p, GET_CODE, SET_CODE, annotations)

#define REG_PROPERTY_QUAT_ANNO(c,p, annotations) \
        REG_PROPERTY_INTERNAL(matQuaternion, const matQuaternion &, mat, Quaternion, c, p, GET_CODE, SET_CODE, annotations)

#define REG_PROPERTY_COLOR_ANNO(c,p, annotations) \
        REG_PROPERTY_INTERNAL(matColor, const matColor &, mat, Color, c, p, GET_CODE, SET_CODE, annotations)

#define REG_PROPERTY_COLORINT_ANNO(c,p, annotations) \
        REG_PROPERTY_INTERNAL(matColorInt, const matColorInt &, mat, Color, c, p, GET_CODE, SET_CODE, annotations)


////// ------------------ classes for owner math properties ----------------------


/*
*/
class matPropertyMatrixOwner : public proPropertyOwner<matMatrix4x4, const matMatrix4x4 &, matPropertyMatrix>
{
    PRO_DECLARE_OBJECT
public:
    typedef matMatrix4x4 MyType;

    matPropertyMatrixOwner() {}
    virtual ~matPropertyMatrixOwner() {};
};


/*
*/
class matPropertyVector3Owner : public proPropertyOwner<matVector3, const matVector3&, matPropertyVector3>
{
    PRO_DECLARE_OBJECT
public:
    typedef matVector3 MyType;

    matPropertyVector3Owner() {}
    virtual ~matPropertyVector3Owner() {};
};


/*
*/
class matPropertyColorOwner : public proPropertyOwner<matColor, const matColor&, matPropertyColor>
{
    PRO_DECLARE_OBJECT
public:
    typedef matColor MyType;

    matPropertyColorOwner() { }
    virtual ~matPropertyColorOwner() {};
};

/*
*/
class matPropertyColorIntOwner : public proPropertyOwner<matColorInt, const matColorInt&, matPropertyColorInt>
{
    PRO_DECLARE_OBJECT
public:
    typedef matColorInt MyType;

    matPropertyColorIntOwner() { }
    virtual ~matPropertyColorIntOwner() {};
};

/*
*/
class matPropertyQuaternionOwner : public proPropertyOwner<matQuaternion, const matQuaternion&, matPropertyQuaternion>
{
    PRO_DECLARE_OBJECT
public:
    typedef matQuaternion MyType;

    matPropertyQuaternionOwner() { }
    virtual ~matPropertyQuaternionOwner() {};
};


/**
  * A set of macros to include "easy to use" (for the user)
  * color properties inside a class that will operate
  * inside of the wxWidgets interface
  *
  * Note:  This property can only be added to classes
  * that derive from proObject
  *
  */
#define REG_PROPERTY_RGBA_WRAPPED_DEFINITIONS( hostingClass, propertyName, wrappedVariableName ) \
    float hostingClass::Get##propertyName##A( void ) const \
    { \
        return wrappedVariableName.GetA(); \
    } \
    \
    float hostingClass::Get##propertyName##B( void ) const \
    { \
        return wrappedVariableName.GetB(); \
    } \
    \
    float hostingClass::Get##propertyName##G( void ) const \
    { \
        return wrappedVariableName.GetG(); \
    } \
    \
    float hostingClass::Get##propertyName##R( void ) const \
    { \
        return wrappedVariableName.GetR(); \
    } \
    \
    matColor hostingClass::Get##propertyName##Raw( void ) const \
    { \
        return wrappedVariableName; \
    } \
    \
    void hostingClass::Set##propertyName##A( float inA ) \
    { \
        wrappedVariableName.SetA( inA ); \
        PropertyChanged( #propertyName "Raw" ); \
    } \
    \
    void hostingClass::Set##propertyName##B( float inB ) \
    { \
        wrappedVariableName.SetB( inB ); \
        PropertyChanged( #propertyName "Raw" ); \
    } \
    \
    void hostingClass::Set##propertyName##G( float inG ) \
    { \
        wrappedVariableName.SetG( inG ); \
        PropertyChanged( #propertyName "Raw" ); \
    } \
    \
    void hostingClass::Set##propertyName##R( float inR ) \
    { \
        wrappedVariableName.SetR( inR ); \
        PropertyChanged( #propertyName "Raw" ); \
    } \
    \
    void hostingClass::Set##propertyName##Raw( const matColor& inColor ) \
    { \
        wrappedVariableName = inColor; \
    } 

// use the macro header to add the easier to use interface
// of an RGBA color property to a variable that was previously
// defined
//
// propertyName - the name that the property system users will see
// wrappedVariableName - the name of the value that is being wrapped
//
// Note:  this macro contains private/public declarations,
// so please add these macros to parts of your class where
// changes in scope type will not matter.
#define DECLARE_RGBA_PROPERTY_WRAPPED( propertyName, wrappedVariableName ) \
    public: \
        float Get##propertyName##A( void ) const; \
        float Get##propertyName##B( void ) const; \
        float Get##propertyName##G( void ) const; \
        float Get##propertyName##R( void ) const; \
        matColor Get##propertyName##Raw( void ) const; \
        \
        void Set##propertyName##A( float ); \
        void Set##propertyName##B( float ); \
        void Set##propertyName##G( float ); \
        void Set##propertyName##R( float ); \
        void Set##propertyName##Raw( const matColor& );

// add this to your .cpp to define your new wrapped color property
//
// hostingClass - the class type that owns the property
// propertyName - name of the property to implement
// wrappedVariableName - the name of the value that is being wrapped
#define REG_PROPERTY_RGBA_WRAPPED( hostingClass, propertyName, wrappedVariableName ) \
    REG_PROPERTY_FLOAT( hostingClass, propertyName##R ) \
    REG_PROPERTY_FLOAT( hostingClass, propertyName##G ) \
    REG_PROPERTY_FLOAT( hostingClass, propertyName##B ) \
    REG_PROPERTY_FLOAT( hostingClass, propertyName##A ) \
    REG_PROPERTY_COLOR( hostingClass, propertyName##Raw ) \
    REG_PROPERTY_RGBA_WRAPPED_DEFINITIONS( hostingClass, propertyName, wrappedVariableName )
#define REG_PROPERTY_RGBA_WRAPPED_ANNO( \
    hostingClass, propertyName, wrappedVariableName, annotations ) \
    REG_PROPERTY_FLOAT( hostingClass, propertyName##R ) \
    REG_PROPERTY_FLOAT( hostingClass, propertyName##G ) \
    REG_PROPERTY_FLOAT( hostingClass, propertyName##B ) \
    REG_PROPERTY_FLOAT( hostingClass, propertyName##A ) \
    REG_PROPERTY_COLOR_ANNO( hostingClass, propertyName##Raw, annotations ) \
    REG_PROPERTY_RGBA_WRAPPED_DEFINITIONS( hostingClass, propertyName, wrappedVariableName )

// add this to a header to declare your new color property
//
// Note:  this macro contains private/public declarations,
// so please add these macros to parts of your class where
// changes in scope type will not matter.
#define DECLARE_RGBA_PROPERTY( propertyName ) \
    DECLARE_RGBA_PROPERTY_WRAPPED( propertyName, m_##propertyName ) \
    private: \
        matColor m_##propertyName;

// add this to your .cpp to define your new color property
//
// hostingClass - the class type that owns the property
// propertyName - name of the property to implement
#define REG_PROPERTY_RGBA( hostingClass, propertyName ) \
    REG_PROPERTY_RGBA_WRAPPED( hostingClass, propertyName, m_##propertyName )
#define REG_PROPERTY_RGBA_ANNO( hostingClass, propertyName, annotations ) \
    REG_PROPERTY_RGBA_WRAPPED_ANNO( hostingClass, propertyName, m_##propertyName, annotations )


#define REG_PROPERTY_XYZ_WRAPPED_GET_DEFINITIONS( \
    hostingClass, propertyName, wrappedVariableName ) \
    float hostingClass::Get##propertyName##X( void ) const \
    { \
        return wrappedVariableName.x; \
    } \
    float hostingClass::Get##propertyName##Y( void ) const \
    { \
        return wrappedVariableName.y; \
    } \
    float hostingClass::Get##propertyName##Z( void ) const \
    { \
        return wrappedVariableName.z; \
    } \
    matVector3 hostingClass::Get##propertyName( void ) const \
    { \
        return wrappedVariableName; \
    }

#define REG_PROPERTY_XYZ_WRAPPED_SET_DEFINITIONS( \
    hostingClass, propertyName, wrappedVariableName ) \
    void hostingClass::Set##propertyName##X( float inX ) \
    { \
        wrappedVariableName.x = inX; \
        PropertyChanged( #propertyName ); \
    } \
    void hostingClass::Set##propertyName##Y( float inY ) \
    { \
        wrappedVariableName.y = inY; \
        PropertyChanged( #propertyName ); \
    } \
    void hostingClass::Set##propertyName##Z( float inZ ) \
    { \
        wrappedVariableName.z = inZ; \
        PropertyChanged( #propertyName ); \
    } \
    void hostingClass::Set##propertyName( const matVector3& inVector ) \
    { \
        wrappedVariableName = inVector; \
        PropertyChanged( #propertyName "X" ); \
        PropertyChanged( #propertyName "Y" ); \
        PropertyChanged( #propertyName "Z" ); \
    }

#define REG_PROPERTY_XYZ_WITH_CALLBACK_SET_DEFINITIONS( hostingClass, propertyName, callback ) \
    void hostingClass::Set##propertyName##X( float inX ) \
    { \
        m_##propertyName.x = inX; \
        PropertyChanged( #propertyName ); \
        callback(); \
    } \
    void hostingClass::Set##propertyName##Y( float inY ) \
    { \
        m_##propertyName.y = inY; \
        PropertyChanged( #propertyName ); \
        callback(); \
    } \
    void hostingClass::Set##propertyName##Z( float inZ ) \
    { \
        m_##propertyName.z = inZ; \
        PropertyChanged( #propertyName ); \
        callback(); \
    } \
    void hostingClass::Set##propertyName( const matVector3& inVector ) \
    { \
        m_##propertyName = inVector; \
        PropertyChanged( #propertyName "X" ); \
        PropertyChanged( #propertyName "Y" ); \
        PropertyChanged( #propertyName "Z" ); \
        callback(); \
    }

// use this macro to add easier to edit property
// settings for a matVector3 value
//
// propertyName - the name of the property as the user will read it
// wrappedVariableName - the name of the variable being wrapped
//
// Note:  this macro contains private/public declarations,
// so please add these macros to parts of your class where
// changes in scope type will not matter.
#define DECLARE_XYZ_PROPERTY_WRAPPED( propertyName, wrappedVariableName ) \
    public: \
        float Get##propertyName##X( void ) const; \
        float Get##propertyName##Y( void ) const; \
        float Get##propertyName##Z( void ) const; \
        matVector3 Get##propertyName( void ) const; \
    \
        void Set##propertyName##X( float ); \
        void Set##propertyName##Y( float ); \
        void Set##propertyName##Z( float ); \
        void Set##propertyName( const matVector3& );

// add this macro to your .cpp to define the new wrapped property
//
// hostingClass - class that holds the wrapped property
// propertyName - the name of the property being added
// wrappedVariableName - the name of the variable wrapped by the property
#define REG_PROPERTY_XYZ_WRAPPED( hostingClass, propertyName, wrappedVariableName ) \
    REG_PROPERTY_FLOAT( hostingClass, propertyName##X ) \
    REG_PROPERTY_FLOAT( hostingClass, propertyName##Y ) \
    REG_PROPERTY_FLOAT( hostingClass, propertyName##Z ) \
    REG_PROPERTY_VECTOR3( hostingClass, propertyName ) \
    REG_PROPERTY_XYZ_WRAPPED_GET_DEFINITIONS( hostingClass, propertyName, wrappedVariableName ) \
    REG_PROPERTY_XYZ_WRAPPED_SET_DEFINITIONS( hostingClass, propertyName, wrappedVariableName )
#define REG_PROPERTY_XYZ_WRAPPED_ANNO( hostingClass, propertyName, wrappedVariableName, annotations ) \
    REG_PROPERTY_FLOAT( hostingClass, propertyName##X ) \
    REG_PROPERTY_FLOAT( hostingClass, propertyName##Y ) \
    REG_PROPERTY_FLOAT( hostingClass, propertyName##Z ) \
    REG_PROPERTY_VECTOR3_ANNO( hostingClass, propertyName, annotations ) \
    REG_PROPERTY_XYZ_WRAPPED_GET_DEFINITIONS( hostingClass, propertyName, wrappedVariableName ) \
    REG_PROPERTY_XYZ_WRAPPED_SET_DEFINITIONS( hostingClass, propertyName, wrappedVariableName )

// use this macro to define an easy to edit matVector3 class property
//
// propertyName - the name of the property as the user will read it
//
// Note:  this macro contains private/public declarations,
// so please add these macros to parts of your class where
// changes in scope type will not matter.
#define DECLARE_XYZ_PROPERTY( propertyName ) \
        DECLARE_XYZ_PROPERTY_WRAPPED( propertyName, m_##propertyName ) \
    private: \
        matVector3 m_##propertyName;

// add this macro to your .cpp to define the new vector3 property
//
// hostingClass - class that holds the wrapped property
// propertyName - the name of the property being added
#define REG_PROPERTY_XYZ( hostingClass, propertyName ) \
    REG_PROPERTY_XYZ_WRAPPED( hostingClass, propertyName, m_##propertyName )
#define REG_PROPERTY_XYZ_ANNO( hostingClass, propertyName, annotations ) \
    REG_PROPERTY_XYZ_WRAPPED_ANNO( hostingClass, propertyName, m_##propertyName, annotations )

// use the macro in your .cpp if your class has a special
// operation it must perform one it sets the property values
//
// hostingClass - class that holds the wrapped property
// propertyName - the name of the property being added
// callback - a callback function of the form void callback( void ) that
// must be called each time the property is set
#define REG_PROPERTY_XYZ_WITH_CALLBACK( hostingClass, propertyName, callback ) \
    REG_PROPERTY_FLOAT( hostingClass, propertyName##X ) \
    REG_PROPERTY_FLOAT( hostingClass, propertyName##Y ) \
    REG_PROPERTY_FLOAT( hostingClass, propertyName##Z ) \
    REG_PROPERTY_VECTOR3( hostingClass, propertyName ) \
    REG_PROPERTY_XYZ_WRAPPED_GET_DEFINITIONS( hostingClass, propertyName, m_##propertyName ) \
    REG_PROPERTY_XYZ_WITH_CALLBACK_SET_DEFINITIONS( hostingClass, propertyName, callback )
#define REG_PROPERTY_XYZ_WITH_CALLBACK_ANNO( hostingClass, propertyName, callback, annotations ) \
    REG_PROPERTY_FLOAT( hostingClass, propertyName##X ) \
    REG_PROPERTY_FLOAT( hostingClass, propertyName##Y ) \
    REG_PROPERTY_FLOAT( hostingClass, propertyName##Z ) \
    REG_PROPERTY_VECTOR3_ANNO( hostingClass, propertyName, annotations ) \
    REG_PROPERTY_XYZ_WRAPPED_GET_DEFINITIONS( hostingClass, propertyName, m_##propertyName ) \
    REG_PROPERTY_XYZ_WITH_CALLBACK_SET_DEFINITIONS( hostingClass, propertyName, callback )

#endif // INCLUDED_matProperties_h
