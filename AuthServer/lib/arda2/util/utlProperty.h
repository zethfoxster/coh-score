#ifndef   INCLUDED_utlProperties_h
#define   INCLUDED_utlProperties_h

#include "arda2/util/utlRect.h"
#include "arda2/util/utlPoint.h"
#include "arda2/core/corStdString.h"

#include "arda2/properties/proFirst.h"
#include "arda2/properties/proPropertyNative.h"
#include "arda2/properties/proPropertyOwner.h"


void proRegisterUtilTypes();

namespace utlStringUtils
{
    utlRect      AsRect(const std::string& value);
    utlPoint     AsPoint(const std::string& value);

    std::string       AsString(const utlRect& value);
    std::string       AsString(const utlPoint& value);
};

#include "arda2/properties/proClass.h"
#include "arda2/properties/proProperty.h"
#include "arda2/properties/proPropertyOwner.h"


////// ------------------ specializations of proValue for utl properties ----------------------

template <> 
class proValueTyped<utlRect> : public proValue
{
public:
    proValueTyped(const std::string &name) : proValue(name), m_value() {}
    utlRect m_value;
};

template <> 
class proValueTyped<utlPoint> : public proValue
{
public:
    proValueTyped(const std::string &name) : proValue(name), m_value() {}
    utlPoint m_value;
};


////// ------------------ base classes for util properties ----------------------

/**
 * base class for rect properties
*/
class utlPropertyRect : public proProperty
{
    PRO_DECLARE_OBJECT
public:
    utlPropertyRect() : proProperty() {}
    virtual ~utlPropertyRect() {}
    
    virtual utlRect       GetValue(const proObject *target) const = 0;
    virtual void          SetValue(proObject *target, const utlRect &value) = 0;
    
    virtual std::string        ToString(const proObject* o) const { return utlStringUtils::AsString(GetValue(o)); }
    virtual void          FromString(proObject* o, const std::string& aValue) { SetValue(o, utlStringUtils::AsRect(aValue));  }
    
    virtual proDataType   GetDataType() const { return DT_other; }
    virtual void          Clone(const proObject* source, proObject* dest) { SetValue(dest, GetValue(source)); }
    
    virtual void          Write(proObjectWriter* out, const proObject* obj ) const;
    virtual void          Read(proObjectReader* in, proObject* obj );

    utlRect GetDefault() const;
    void SetAnnotation(const std::string &name, const std::string &value);
};

/**
 * base class for Point properties
*/
class utlPropertyPoint : public proProperty
{
    PRO_DECLARE_OBJECT
public:
    utlPropertyPoint() : proProperty() {}
    virtual ~utlPropertyPoint() {}
    
    virtual utlPoint    GetValue(const proObject *target) const = 0;
    virtual void          SetValue(proObject *target, const utlPoint &value) = 0;
    
    virtual std::string        ToString(const proObject* o) const { return utlStringUtils::AsString(GetValue(o)); }
    virtual void          FromString(proObject* o, const std::string& aValue) { SetValue(o, utlStringUtils::AsPoint(aValue));  }
    
    virtual proDataType   GetDataType() const { return DT_other; }
    virtual void          Clone(const proObject* source, proObject* dest) { SetValue(dest, GetValue(source)); }
    
    virtual void          Write(proObjectWriter* out, const proObject* obj ) const;
    virtual void          Read(proObjectReader* in, proObject* obj );

    utlPoint GetDefault() const;
    void SetAnnotation(const std::string &name, const std::string &value);
};


////// ------------------ classes for native util properties ----------------------

/*
*/
class utlPropertyRectNative : public utlPropertyRect
{
    PRO_DECLARE_OBJECT
public:
    typedef utlRect   MyType;
    virtual utlRect   GetValue(const proObject *target) const = 0;
    virtual void      SetValue(proObject *target, const utlRect &value) = 0;
    virtual ~utlPropertyRectNative() {};
};

/*
*/
class utlPropertyPointNative : public utlPropertyPoint
{
    PRO_DECLARE_OBJECT
public:
    typedef utlPoint   MyType;
    virtual utlPoint   GetValue(const proObject *target) const = 0;
    virtual void       SetValue(proObject *target, const utlPoint &value) = 0;
    virtual ~utlPropertyPointNative() {};
};

// macros to register native math properties
#define REG_PROPERTY_RECT(c, p)    REG_PROPERTY_INTERNAL(utlRect, const utlRect &, utl, Rect, c, p, GET_CODE, SET_CODE, NULL)
#define REG_PROPERTY_POINT(c,p)  REG_PROPERTY_INTERNAL(utlPoint, const utlPoint &, utl, Point, c, p, GET_CODE, SET_CODE, NULL)

#define REG_PROPERTY_RECT_ANNO(c, p, annotations) \
        REG_PROPERTY_INTERNAL(utlRect, const utlRect &, utl, Rect, c, p, GET_CODE, SET_CODE, annotations)

#define REG_PROPERTY_POINT_ANNO(c,p, annotations) \
        REG_PROPERTY_INTERNAL(utlPoint, const utlPoint &, utl, Point, c, p, GET_CODE, SET_CODE, annotations)


////// ------------------ classes for owner util properties ----------------------


/*
*/
class utlPropertyRectOwner : public proPropertyOwner<utlRect, const utlRect &, utlPropertyRect>
{
    PRO_DECLARE_OBJECT
public:
    typedef utlRect MyType;

    utlPropertyRectOwner() {}
    virtual ~utlPropertyRectOwner() {};
};


/*
*/
class utlPropertyPointOwner : public proPropertyOwner<utlPoint, const utlPoint&, utlPropertyPoint>
{
    PRO_DECLARE_OBJECT
public:
    typedef utlPoint MyType;

    utlPropertyPointOwner() {}
    virtual ~utlPropertyPointOwner() {};
};



#endif // INCLUDED_utlProperties_h
