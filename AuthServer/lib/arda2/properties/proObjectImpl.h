/**
 *  property object impl
 *
 *  @author Sean Riley
 *
 *  @date 1/13/2003
 *
 *  @file
**/

#ifndef __proObjectImpl_h__
#define __proObjectImpl_h__

#include "proFirst.h"

#include "arda2/core/corStdString.h"
#include "arda2/core/corStlVector.h"
#include "arda2/core/corStlHashmap.h"

class proChildSortFunctor;
class proClass;
class proClassOwner;
class proClassRegistry;
class proObject;
class proObjectReader;
class proObjectWriter;
class proObserver;
class proProperty;
class proValue;

typedef std::pair<proObserver*,int> proObserverToken;

/**
* contains dynamically allocated portion of proObject.
*
*/
class proObjectImpl
{
    friend class proObject;

public:

    // construct with the object's class - called by proObject
    proObjectImpl(proClass *cls);

    virtual ~proObjectImpl();

    proClass *GetClass();

    // for changing a native class to an ownerclass
    void SetClass(proClassOwner *cls);

    // -- general property interface --
    virtual int GetPropertyCount() const;
    virtual proProperty *GetPropertyAt(int index) const;
    virtual proProperty *GetPropertyByName(const std::string &propName) const;

    // -- instance property interface --

    // add an owner instance property to this object
    void AddInstanceProperty(proProperty *property);

    // remove an owner instance property from this object
    bool RemoveInstanceProperty(proProperty *property);

    // return the number of instance properties
    int GetInstancePropertyCount();

    // get an instance property by index
    proProperty *GetInstancePropertyAt(int index);

    // -- observation interface --

    // add an observer to this object
    void AddObserver(proObserver *observer, const int token, const std::string &propName = "");

    // remove an observer from this object
    void RemoveObserver(const proObserver *observer, const int token);

    // sorts the instance properties based on the specified functor
    void SortInstanceProperties( proObject*, proChildSortFunctor& );

protected:

    proProperty *FindChild(proObject *object, proObject *child);

    // called by owner properties (via proObject) to access this object's storage
    virtual proValue *GetOwnerValue(const std::string &propName) const;

    // called by owner properties (via proObject) to access this object's storage
    virtual void AddOwnerValue(proValue *value);

    // called by owner properties (via proObject) to access this object's storage
    virtual bool RemoveOwnerValue(const std::string &propName);

    typedef std::vector<proValue*> proValueList;

    std::vector<proObserverToken> m_observers;             //< observers of this object
    PropertyList                  m_instanceProperties;    //< instance properties on this object
    proValueList                  m_propertyStorage;       //< storage for this object's owner properties (both class and instance)
    proClass*                     m_class;                 //< this can be a native class or an owner class

};



#endif

