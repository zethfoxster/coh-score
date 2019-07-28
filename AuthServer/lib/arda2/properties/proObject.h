/**
 *  property object
 *
 *  @author Sean Riley
 *
 *  @date 1/13/2003
 *
 *  @file
**/

#ifndef __proObject_h__
#define __proObject_h__

#include "proFirst.h"

#include "arda2/core/corStdString.h"
#include "arda2/core/corStlVector.h"

#include "proObjectImpl.h"

class proChildSortFunctor;
class proClass;
class proClassOwner;
class proClassRegistry;
class proObjectWriter;
class proObjectReader;
class proObserver;
class proValue;

// each C++ class have a native class pointer - NOTE: this is protected!
// this static's value is set with the PRO_REGISTER_CLASS macro
#define PRO_DECLARE_OBJECT\
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
    public:


#if CORE_COMPILER_MSVC
#pragma deprecated ("DECLARE_OBJECT")
#endif // CORE_COMPILER_MSVC
#define DECLARE_OBJECT PRO_DECLARE_OBJECT

/**
 * base object class for the property system.
 * implements observation.
 *
 */
class proObject
{
public:

    proObject();

    virtual ~proObject();

    // set the associated class instance
    virtual void SetClass(proClassOwner *cls);

    // create a new instance with the same attribute values
    virtual proObject *Clone() const;

    // assign rhs's attribute values to this's attributes
    virtual errResult Assign(const proObject *rhs);

    // ------ property interface ------

    virtual int GetPropertyCount() const;
    virtual proProperty *GetPropertyAt(int index) const;
    virtual proProperty *GetPropertyByName(const std::string &propName) const;

    // add an owner instance property to this object
    virtual void AddInstanceProperty(proProperty *property);

    // remove an owner instance property from this object
    virtual bool RemoveInstanceProperty(proProperty *property);

    // remove all instance properties
    virtual void FlushInstanceProperties();

    // ------ observation interface ------

    // call this to notify observers that a property has changed
    virtual void PropertyChanged(const std::string &propName);

    // call this to notify observers that a property name has changed
    virtual void PropertyNameChanged(const std::string &oldName, const std::string &newName);

    // call this when a child object is added to the object
    void PropertyAdded(const std::string &proName);

    // call this when a child object is removed from the object
    void PropertyRemoved(const std::string &proName);

    // call this when you have sorted the children
    void PropertiesReordered( void );

    // call this to notify observers that a property's value has been detected as invalid
    virtual void PropertyValueInvalid(const std::string &propName);

    virtual void FlushObservers();

    // ------ Containment interface -------

    // add another property object as a child
    virtual int AddChild(proObject *object, const std::string &name = "", DP_OWN_VALUE own = DP_ownValue);

    // remove an existsing child by pointer
    virtual bool RemoveChild(proObject *child);

    // remove all children
    virtual void FlushChildren();

    friend class proClass;
    friend class proClassRegistry;

    virtual proClass *GetClass() const;

    // ------ convenience value interface TODO: templatize these? ------
    std::string GetValue(const char *name) const;
    void SetValue(const char *name, const char *value);

    // builds a vector of all the owned objects assigned to this object
    virtual bool GetChildren(std::vector<proObject*>& outList, bool recursive = false) const;

    // will provide a descriptive string for this class,
    // or "" if no data is relevant or available.
    virtual std::string GetPropertySummary( const std::string& inLabel );

    // sorts the children of this object using the provided functor
    virtual void SortChildren( proChildSortFunctor& );

protected:

    // generic method for signalling all of the observers
    // (designed to cope with observers that delete themselves
    // during the signal event)
    void SignalObservers( int inSignal, const std::string& inName );

    // write the object to a writer (serialize)
    virtual void WriteObject(proObjectWriter* out) const;

    // read the object from a reader (unserialize)
    virtual void ReadObject(proObjectReader* in);

    // add an observer to this object
    virtual void AddObserver(proObserver *observer, const int token, const std::string &propName = "");

    // remove an observer from this object
    virtual void RemoveObserver(const proObserver *observer, const int token);

    // called by owner properties  to access this object's storage
    virtual proValue *GetOwnerValue(const std::string &propName) const;

    // called by owner properties  to access this object's storage
    virtual void AddOwnerValue(proValue *value);

    // called by owner properties  to access this object's storage
    virtual void RemoveOwnerValue(const std::string &propName);

    // needed to allow owner properties as access owner values
    template <typename TYPE_GET, typename TYPE_SET, typename T> friend class proPropertyOwner;

    friend class proObserver;
    friend class proObjectReader;
    friend class proObjectWriter;

    proObjectImpl *ConfirmImpl();
    proObjectImpl *m_impl;

    static proClass *s_class;

};


// helper functions for adding instance owner properties
proProperty *AddInstanceProperty(proObject *obj, const std::string &name, int value);
proProperty *AddInstanceProperty(proObject *obj, const std::string &name, uint value);
proProperty *AddInstanceProperty(proObject *obj, const std::string &name, float value);
proProperty *AddInstanceProperty(proObject *obj, const std::string &name, bool value);
proProperty *AddInstanceProperty(proObject *obj, const std::string &name, const std::string &value);


#endif // __proObject_h__
