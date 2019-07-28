/**
 *  property class
 *
 *  @author Sean Riley
 *
 *  @date 1/13/2003
 *
 *  @file
**/

#ifndef _proClass_h__
#define _proClass_h__

#include "arda2/properties/proFirst.h"

#include "arda2/core/corStdString.h"
#include "arda2/core/corStlVector.h"

class proProperty;

#include "arda2/properties/proClassRegistry.h"
#include "arda2/properties/proObject.h"

/** 
 * abstract base class and interface for different types of classes
 *
 */
class proClass : public proObject
{
    friend class proClassRegistry;
    PRO_DECLARE_OBJECT

public:

    // constructor
    proClass();
    proClass(const std::string &name);

    // destructor
    virtual ~proClass();

    // creates a new instance of this class
    virtual proObject *NewInstance() = 0;

    // check if another class is an instance of this class
    virtual bool Instanceof( const proClass* aClass ) const;

    // property interface
    virtual int GetPropertyCount( ) const = 0;
    virtual proProperty *GetPropertyAt( int anIndex ) const = 0;
    virtual proProperty *GetPropertyByName( const std::string &aName ) const = 0;

    virtual void AddProperty(proProperty *property) = 0;
    virtual void RemoveProperty(proProperty *property) = 0;

    // gets the name of the class
    virtual const std::string &GetName() const { return m_name; }

    // gets the parent class of this class
    virtual proClass *GetParent() const { return m_parent; }

    // assign two objects of this class
    virtual errResult Assign( const proObject* srcObj, proObject* destObj ) const = 0;

    virtual void SetName(const std::string &name) { m_name = name; }

    virtual bool IsNative() const = 0;
protected:

    std::string          m_name;
    proClass       *m_parent;
};


#endif // _proClass_h__
