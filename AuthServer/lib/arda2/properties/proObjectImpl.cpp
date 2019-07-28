#include "arda2/core/corFirst.h"

#include "proFirst.h"

#include "arda2/properties/proObjectImpl.h"
#include "arda2/properties/proClass.h"
#include "arda2/properties/proObserver.h"

#include "arda2/properties/proPropertyOwner.h"
#include "arda2/properties/proClassOwner.h"
#include "arda2/properties/proChildSort.h"

using namespace std;

/**
 * constructor
**/
proObjectImpl::proObjectImpl(proClass *cls) :
    m_class(cls)
{
}

/**
 * destructor - doesnt delete it's class
**/
proObjectImpl::~proObjectImpl() 
{
    // delete owner values
    proValueList::iterator it = m_propertyStorage.begin();
    for (; it != m_propertyStorage.end(); it++)
    {
        proValue *v = (*it);

#if CORE_DEBUG
        // check for values without properties - this should not happen!
        proProperty *testProp = GetPropertyByName(v->GetName());
        errAssert(testProp != NULL);
#endif
        SafeDelete(v);
    }

    // delete instance properties
    PropertyList::iterator pit = m_instanceProperties.begin();
    for (;pit != m_instanceProperties.end(); pit++)
    {
        proProperty *pro = (*pit);
        SafeDelete(pro);
    }
}

proClass *proObjectImpl::GetClass()
{
    return m_class;
}


/**
 * used for setting the owner class of an object.
 * by default the impl's m_class points to a native class,
 * this replaces that with a pointer to a owner class
 */
void proObjectImpl::SetClass(proClassOwner *cls)
{
    m_class = cls;
}


/// ------ General Property Implementation ------

int proObjectImpl::GetPropertyCount() const
{
    return m_class->GetPropertyCount() + (int)m_instanceProperties.size();
}

/**
 * order of properties is:
 *    instance owner properties
 *    class owner properties
 *    native properties
 */
proProperty *proObjectImpl::GetPropertyAt(int index) const
{
    if (index < (int)m_instanceProperties.size())
    {
        return m_instanceProperties[index];
    }
    index -= (int)m_instanceProperties.size();
    return m_class->GetPropertyAt(index);
}

proProperty *proObjectImpl::GetPropertyByName(const string &propName) const
{
    PropertyList::const_iterator it = m_instanceProperties.begin();
    for (; it != m_instanceProperties.end(); it++)
    {
        if ((*it)->GetName() == propName)
            return (*it);
    }
    return m_class->GetPropertyByName(propName);
}

/// ------ Instance Property Implementation ------


// add an owner instance property to this object
void proObjectImpl::AddInstanceProperty(proProperty *property)
{
    m_instanceProperties.push_back(property);
}

// remove an owner instance property from this object
bool proObjectImpl::RemoveInstanceProperty(proProperty *property)
{
    if (property->GetIsNative())
    {
        ERR_REPORTV(ES_Warning, ("Cannot remove native property <%s>", property->GetName().c_str()));
        return false;
    }

    bool result = RemoveOwnerValue(property->GetName());
    if (!result)
    {
        return false;
    }

    PropertyList::iterator it = m_instanceProperties.begin();
    for (; it != m_instanceProperties.end(); it++)
    {
        if ( (*it) == property)
        {
            m_instanceProperties.erase(it);
            SafeDelete(property);
            return true;
        }
    }
    return false;
}

/// ------ Observer Implementation ------ ///
/**
 * Add an observer to this object. 
 *
 * @propName is observation of a particular property (NOT IMPLEMENTED YET)
**/
void proObjectImpl::AddObserver(proObserver *observer, const int token, const string &propName)
{
    UNREF(propName);

    if (!observer)
        return;

    //ERR_REPORTV(ES_Info, ("Add Observer for %s (ty: %s) : %x | obs %x", propName.c_str(), GetClass()->GetName().c_str(), this, observer));


    m_observers.push_back( make_pair(observer, token) );
}

/**
 * Remove an observer from this object.
**/
void proObjectImpl::RemoveObserver(const proObserver *observer, const int token)
{
    //ERR_REPORTV(ES_Info, ("Remove Observer from (%s) %x | obs %x", GetClass()->GetName().c_str(), this, observer));

    if (!observer)
        return;

    vector<proObserverToken>::iterator it = m_observers.begin();
    for (; it != m_observers.end(); it++)
    {
        if ( (*it).first == observer && (*it).second == token)
        {
            m_observers.erase(it);
            break;
        }
    }
}


/**
 * called by owner properties to access this object's storage.
 * gets a proValue pointer.
 */
proValue *proObjectImpl::GetOwnerValue(const string &propName) const
{
    proValueList::const_iterator it = m_propertyStorage.begin();
    for (; it != m_propertyStorage.end(); it++)
    {
        proValue *value = (*it);
        if (value->GetName() == propName)
        {
            return value;
        }
    }
    return NULL;
}

/**
 * called by owner properties to access this object's storage.
 * adds a new value object for this object
 */
void proObjectImpl::AddOwnerValue(proValue *value)
{
    m_propertyStorage.push_back(value);
}


/**
 * called by owner properties to access this object's storage.
 * removes a value object for this object
 */
bool proObjectImpl::RemoveOwnerValue(const string &propName)
{
    proValueList::iterator it = m_propertyStorage.begin();
    for (; it != m_propertyStorage.end(); it++)
    {
        proValue *value = (*it);
        if (value->GetName() == propName)
        {
            m_propertyStorage.erase(it);
            SafeDelete(value);
            return true;
        }
    }
    return false;
}

int proObjectImpl::GetInstancePropertyCount()
{
    return (int)m_instanceProperties.size();
}

proProperty *proObjectImpl::GetInstancePropertyAt(int index)
{
    if (index < 0 || index >= (int)m_instanceProperties.size())
        return NULL;
    return m_instanceProperties[index];
}

/**
  * sorts the instance properties based on the specified functor
  *
  * inputs:
  * @param inObject - that holds the properties being compared
  * @param inSort - the sorting comparison to use
  *
 **/
void proObjectImpl::SortInstanceProperties(
    proObject* inObject, proChildSortFunctor& inSort )
{
    // can't sort nothing.
    if ( m_instanceProperties.size() == 0 )
    {
        return;
    }

    // just bubble sort the instance properties
    for ( size_t i = m_instanceProperties.size() - 1; i > 0; --i )
    {
        for ( size_t j = 0; j < i; ++j )
        {
            if ( inSort.ShouldSwap( inObject, m_instanceProperties[ j ],
                m_instanceProperties[ j + 1 ] ) )
            {
                // swap the instance property pointers
                proProperty* temp = m_instanceProperties[ j ];
                m_instanceProperties[ j ] =
                    m_instanceProperties[ j + 1 ];
                m_instanceProperties[ j + 1 ] = temp;

                // swap the property pointers (a parallel
                // array to m_instanceProperties)
                proValue* temp2 = m_propertyStorage[ j ];
                m_propertyStorage[ j ] = m_propertyStorage[ j + 1 ];
                m_propertyStorage[ j + 1 ] = temp2;
            }
        }
    }
}

