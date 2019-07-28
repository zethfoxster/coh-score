/**
 *  property class
 *
 *  @author Sean Riley
 *
 *  @date 1/13/2003
 *
 *  @file
**/


#ifndef __proClassOwner_h__
#define __proClassOwner_h__

#include "proFirst.h"

#include "arda2/core/corStdString.h"
#include "arda2/core/corStlVector.h"

#include "arda2/properties/proProperty.h"
#include "arda2/properties/proClassNative.h"

/** 
 * owner class - is able to have properties added dynamically.
 * has a native class that it uses internally.
 *
 * NOTE: since the classOwner IS a proObject, it could have instance properties for static class data
 */
class proClassOwner : public proClass
{
    PRO_DECLARE_OBJECT
public:

    proClassOwner();
    proClassOwner(const std::string &name, proClass *parent);

    virtual ~proClassOwner();

    // creates a new instance of this class
    virtual proObject *     NewInstance();

    // property interface
    virtual int             GetPropertyCount() const;
    virtual proProperty*    GetPropertyAt(int anIndex) const;
    virtual proProperty*    GetPropertyByName(const std::string &aName) const;

    virtual void            AddProperty(proProperty *property);
    virtual void            RemoveProperty(proProperty *);

    virtual errResult       Assign(const proObject* srcObj, proObject* destObj) const;

    // add an owner instance property to this object
    virtual void AddInstanceProperty(proProperty *property);

    // check if another class is an instance of this class
    virtual bool Instanceof( const proClass* aClass ) const;


    // helpers for adding owner properties
    virtual proProperty *CreateProperty(const std::string &name, const proDataType dt, const std::string &def="");

    virtual void SetParent(proClassOwner* parent);

    // serialization
    virtual void WriteObject(proObjectWriter* out) const;
    virtual void ReadObject(proObjectReader* in);

    virtual bool IsNative() const { return false; }

protected:
    PropertyList    m_properties;
};


#endif //__proClassOwner_h__
