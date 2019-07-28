#include "arda2/core/corFirst.h"

#include "proFirst.h"

#include "arda2/properties/proObject.h"
#include "arda2/properties/proClass.h"
#include "arda2/properties/proObserver.h"

#include "arda2/properties/proPropertyOwner.h"
#include "arda2/properties/proClassOwner.h"
#include "arda2/properties/proObjectWriter.h"
#include "arda2/properties/proObjectReader.h"

PRO_REGISTER_CLASS(proObject, NULL)

using namespace std;

/**
 * constructor
**/
proObject::proObject() :
    m_impl(NULL)
{
}

/**
 * destructor - this informs observers that the object is being deleted
**/
proObject::~proObject() 
{
    FlushObservers();
    SafeDelete(m_impl);
}

proClass *proObject::GetClass() const
{
    if (m_impl)
        return m_impl->GetClass();
    return s_class;
}

/**
 * assuming cls is an owner class that knows the right native class for this object
 */
void proObject::SetClass(proClassOwner *cls)
{
    if (m_impl)
    {
        m_impl->SetClass(cls);
    }
    else
    {
        m_impl = new proObjectImpl(cls);
    }
}


/** 
 * make an impl if we dont have one
 */
proObjectImpl *proObject::ConfirmImpl()
{
    if (!m_impl)
    {
        proClass *cls = NULL;
        cls = GetClass();
        m_impl = new proObjectImpl(cls);
    }

    return m_impl;
}

/**
 * base implementation for writing a property object
 * to a serialization stream.
 *
**/
void proObject::WriteObject(proObjectWriter* out) const
{
    if (!out)
        return;

    bool ref = out->CheckReference(this);
    if (ref)
        return;

    const proClass *cls =GetClass();
    errAssert( cls );

    // write header
    out->BeginObject( cls->GetName(), 0 );

    // static attributes
    for (int i = 0; i < cls->GetPropertyCount(); ++i)
    {
        proProperty *prop = cls->GetPropertyAt(i);
        proProperty *transient = prop->GetPropertyByName(proAnnoStrTransient);
        if (transient)
        {
            proPropertyBool *transientBool = (proPropertyBool*)transient;
            if (transientBool->GetValue(prop) == true)
            {
                // do not serialize this property
                continue;
            }
        }
        prop->Write( out, this );
    }

    // dynamic attributes
    if ( m_impl )
    {
        for (int i = 0; i < m_impl->GetInstancePropertyCount(); ++i)
        {
            proProperty *prop = m_impl->GetInstancePropertyAt(i);
            proProperty *transient = prop->GetPropertyByName(proAnnoStrTransient);
            if (transient)
            {
                proPropertyBool *transientBool = (proPropertyBool*)transient;
                if (transientBool->GetValue(prop) == true)
                {
                    // do not serialize this property
                    continue;
                }
            }
            prop->Write( out, this );
        }
    }

    out->EndObject();

}

/**
 * base implementation for reading a property object from a serialization stream.
**/
void proObject::ReadObject(proObjectReader* in)
{
    if (!in)
        return;

    proClass *cls = this->GetClass();

    // read header
    int version;
    if ( !ISOK(in->BeginObject(cls->GetName(), version)) )
    {
        return;
    }

    // loads non-dynamic properties only.
    for (int i = 0; i < GetPropertyCount(); ++i)
    {
        proProperty *prop = GetPropertyAt(i);
        if (prop)
        {
            proProperty *transient = prop->GetPropertyByName(proAnnoStrTransient);
            if (transient)
            {
                proPropertyBool *transientBool = (proPropertyBool*)transient;
                if (transientBool->GetValue(prop) == true)
                {
                    // do not serialize this property
                    continue;
                }
            }
            prop->Read( in, this );
        }
    }

    // load dynamic properties
    proProperty *newProp = NULL;
    while ( (newProp = in->ReadDynamicProperty(this)) != NULL)
    {
        AddInstanceProperty(newProp);
    }

}


/**
 * base implementation for cloning an existing property object
 *
**/
proObject* proObject::Clone( ) const
{
    proObject *clone = GetClass()->NewInstance();

    clone->Assign( this );

    if (m_impl)
    {
        for ( int i = 0; i < m_impl->GetInstancePropertyCount(); ++i)
        {
            // clone instance properties
            proProperty *oldProp = m_impl->GetInstancePropertyAt(i);
            oldProp->Clone(this, clone);
        }
    }
    return clone;
}


/**
 * populate this from the data in rhs
**/
errResult proObject::Assign( const proObject *rhs )
{
    return GetClass()->Assign( rhs, this );

}

/// ------ General Property Implementation ------

/**
 * use the impl or the static class pointer
 */
int proObject::GetPropertyCount() const
{
    if (m_impl && m_impl->GetClass())
        return m_impl->GetPropertyCount();

    return GetClass()->GetPropertyCount();
}

/**
 * use the impl or the static class pointer
 */
proProperty *proObject::GetPropertyAt(int index) const
{
    if (m_impl)
        return m_impl->GetPropertyAt(index);

    return GetClass()->GetPropertyAt(index);
}

/**
 * use the impl or the static class pointer
 */
proProperty *proObject::GetPropertyByName(const string &propName) const
{
    if (m_impl)
        return m_impl->GetPropertyByName(propName);

    return GetClass()->GetPropertyByName(propName);
}


// called by owner properties to access this object's storage
proValue *proObject::GetOwnerValue(const string &propName) const
{
    if (m_impl)
        return m_impl->GetOwnerValue(propName);
    else
        return NULL;
}

// called by owner properties to access this object's storage
void proObject::AddOwnerValue(proValue *value)
{
    ConfirmImpl()->AddOwnerValue(value);
}

// called by owner properties to access this object's storage
void proObject::RemoveOwnerValue(const string &propName)
{
    if (m_impl)
        m_impl->RemoveOwnerValue(propName);
    else
        ERR_REPORTV(ES_Warning, ("Trying to remove property <%s> when no impl present.", propName.c_str()));
}

/// ------ Instance Property Implementation ------

/**
 * add an owner instance property to this object.
 * always uses the impl.
 */
void proObject::AddInstanceProperty(proProperty *property)
{
    ConfirmImpl()->AddInstanceProperty(property);
    PropertyAdded(property->GetName());
}

/**
 * remove an owner instance property from this object.
  * always uses the impl.
 */
bool proObject::RemoveInstanceProperty(proProperty *property)
{
    string name = property->GetName();
    bool result = ConfirmImpl()->RemoveInstanceProperty(property);
    PropertyRemoved(name);
    return result;
}

/**
 * remove all instance properties, dont allocate an impl.
 */
void proObject::FlushInstanceProperties()
{
    if (m_impl)
    {
        while (m_impl->m_instanceProperties.size() > 0)
        {
            PropertyList::iterator it = m_impl->m_instanceProperties.begin();
            proProperty *property = (*it);
            RemoveInstanceProperty(property);
        }
    }
}


/// ------ Observer Implementation ------

/**
 * Add an observer to this object. uses the impl
 *
 * @propName is observation of a particular property (NOT IMPLEMENTED YET)
**/
void proObject::AddObserver(proObserver *observer, const int token, const string &propName)
{
    ConfirmImpl()->AddObserver(observer, token, propName);
}

/**
 * Remove an observer from this object. uses the impl
**/
void proObject::RemoveObserver(const proObserver *observer, const int token)
{
    ConfirmImpl()->RemoveObserver(observer, token);
}

/**
  * generic method for signalling all of the observers
  * (designed to cope with observers that delete themselves
  * during the signal event)
  *
  * inputs:
  * @param inSignal - an ePropertyObserverEvents enum value
  * @param inName - of the property being modified
  *
  * outputs:
  * -- none --
 **/
void proObject::SignalObservers( int inSignal, const string& inName )
{
    // making a copy of the observers list might seem like a more efficient
    // technique for iterating across a list whose elements can delete themselves,
    // but it will cause a crash bug in the undo list of the particle editor.
    //
    // Instead, we will walk the observers list directly, and not increment the
    // iterator if the array size decreases after a Signal() operation.

    for ( vector< proObserverToken >::iterator it = ConfirmImpl()->m_observers.begin();
        it != ConfirmImpl()->m_observers.end(); )
    {
        // record the array size before the Signal() operation
        size_t arraySize = ConfirmImpl()->m_observers.size();

        // signal the observer
        (*it).first->Signal( this, (*it).second, inSignal, inName );

        // proceed if the array size has not decreased
        if ( arraySize <= ConfirmImpl()->m_observers.size() )
        {
            ++it;
        }
    }
}

/**
 * Call this when a property changes. It signals the attached observers.
**/
void proObject::PropertyChanged(const string &propName)
{
    SignalObservers( kPropertyChanged, propName );
}

/**
 * Call this when a property name changes. It signals the attached observers.
**/
void proObject::PropertyNameChanged(const string &oldName, const string &newName)
{
    SignalObservers( kPropertyNameChanged, oldName + "." + newName );
}


/**
 * call this when a dynamic property is added
**/
void proObject::PropertyAdded(const string &propName)
{
    SignalObservers( kPropertyAdded, propName );
}

/**
 * call this when a dynamic property is removed
**/
void proObject::PropertyRemoved(const string &propName)
{
    SignalObservers( kPropertyRemoved, propName );
}

/**
 * call this when the dynamic properties change their ordering
**/
void proObject::PropertiesReordered( void )
{
    SignalObservers( kPropertiesReordered, string( "" ) );
}

/**
 * call this to notify observers that a property's value has been detected as invalid
**/
void proObject::PropertyValueInvalid(const string &propName)
{
    SignalObservers( kPropertyValueInvalid, propName );
}

/**
 * clears the observers
**/
void proObject::FlushObservers()
{
    if (m_impl)
    {
        for (int numObservers = (int)m_impl->m_observers.size(); 
            numObservers > 0; 
            numObservers = (int)m_impl->m_observers.size() )
        {
            proObserverToken &token = m_impl->m_observers.back();
            proObserver *observer = token.first;
            int t = token.second;

            // this MUST call RemoveObserver on the observer!!!!
            observer->Signal(this, t ,kDeleted, "");

            int newNumObservers = (int)m_impl->m_observers.size();
            if (numObservers == newNumObservers)
            {
                ERR_REPORTV(ES_Info, ("Observer did not remove itself...."));
                m_impl->m_observers.pop_back();
            }
        }
        m_impl->m_observers.clear();
    }
}

/// ------- Children Implementation ------

/**
 * add a proObject as a object owner property. This invokes
 * the PropertyAdded() helper function.
 * uses the impl
**/
int proObject::AddChild(proObject *object, const string &name, DP_OWN_VALUE own)
{
    ConfirmImpl();

    static int nextObj = 0;
    string realName = name;
    if (name.size() == 0)
    {
        char buf[128];
        sprintf(buf, "Child_%d", nextObj++ );
        realName = buf;
    }
    // ensure we don't add children with duplicate names.
    proProperty* pProperty = GetPropertyByName( realName );
    if ( pProperty == NULL )
    {
        proPropertyObjectOwner *pro = PRO_NEW_INSTANCE(proPropertyObjectOwner);
        if (pro)
        {
            pro->SetName(realName);
            pro->SetValue(this, object);
            pro->SetOwner(this, own);
            AddInstanceProperty(pro);
            return GetPropertyCount();
        }
        else
        {
            errAssertV(false,("ProObject: Unable to create new instance of proPropertyObjectOwner."));
        }
    }
    else
    {
        errAssertV(false,("ProObject: Unable to add child \"%s\". Name already exists.",realName.c_str()));
    }

    return -1;
}

/**
 * Remove a child that is an object owner property.
 * uses the impl
**/
bool proObject::RemoveChild(proObject *object)
{
    // optimization to prevent string lookup of proPropertyObjectOwner each time
    static const proClass *oo = proClassRegistry::ClassForName<proClass*>("proPropertyObjectOwner");

    ConfirmImpl();

    int count = 0;

    PropertyList::iterator it = m_impl->m_instanceProperties.begin();
    for (; it != m_impl->m_instanceProperties.end(); it++)
    {
        proProperty *pro = (*it);
        if (pro->GetClass()->Instanceof(oo))
        {
            proPropertyObjectOwner *proObj = static_cast<proPropertyObjectOwner*>(pro);
            proObject *foundObject = proObj->GetValue(this);
            if (foundObject == object)
            {
                this->RemoveInstanceProperty(pro);
                return true;
            }
        }
        count++;
    }
    return false;
}

/**
 * Remove all children.
 * does not allocate an impl
**/
void proObject::FlushChildren()
{
    // DON'T allocate the impl

    // build a list of all the owner properties
    vector< proObject* > toRemove;
    GetChildren( toRemove );

    for ( vector< proObject* >::iterator
        currentTarget  = toRemove.begin(), stopTarget = toRemove.end();
        currentTarget != stopTarget; ++currentTarget )
    {
        // nuke each one
        RemoveChild( *currentTarget );
    }
}

/**
  * builds a vector of all children of this object
  *
  * @param outList - the list to fill with pointers to the properties
  *
  * @return true if list filled correctly, false if otherwise
  */
bool proObject::GetChildren(std::vector<proObject*>& outList, bool recursive) const
{
    if (!m_impl)
        return true;

    size_t len = m_impl->m_instanceProperties.size();
    for (size_t i=0; i < len; i++)
    {
        proProperty *prop = m_impl->m_instanceProperties[i];
        if (prop->GetDataType() == DT_object)
        {
            proValueTyped<proObject*> *value = (proValueTyped<proObject*>*)&(*m_impl->m_propertyStorage[i]);
            outList.push_back(value->m_value);

            if (recursive)
                value->m_value->GetChildren(outList, recursive);
        }
    }
    proClass *cls = GetClass();
    len = cls->GetPropertyCount();
    for (size_t j=0; j != len; j++)
    {
        proProperty *prop = cls->GetPropertyAt((int)j);
        if (prop->GetDataType() == DT_object)
        {
            proObject *child = ((proPropertyObject*)prop)->GetValue(this);
            if (child)
            {
                outList.push_back(child);
                if (recursive)
                    child->GetChildren(outList, recursive);
            }
        }
    }

    return true;
}

/**
  * will provide a descriptive string for this class,
  * or "" if no data is relevant or available.
  *
  * inputs:
  * @param inLabel - some extra data to append to the summary
  *
  * outputs:
  * @return description string ("" if no relevant data to report)
  *
  */
string proObject::GetPropertySummary( const string& inLabel )
{
    // walk all of the properties, building a string up
    // from their data, if the properties are descriptive
    string propertyString( "" );
    for ( int i = 0; i < GetPropertyCount(); ++i )
    {
        proProperty* currentProperty = GetPropertyAt( i );
        if ( currentProperty )
        {
            string newPropertyString( currentProperty->GetPropertySummaryString( this ) );
            if ( newPropertyString != "" )
            {
                propertyString += newPropertyString;
            }
        }
    }

    // then, build the output string
    string output( inLabel );
    if ( propertyString != "" )
    {
        output += " = ";
        output += propertyString;
    }

    // done!
    return output;
}


string proObject::GetValue(const char *name) const
{
    proProperty *p = GetPropertyByName(name);
    if (p)
    {
        return p->ToString(this);
    }
    return "";
}

void proObject::SetValue(const char *name, const char *value)
{
    proProperty *p = GetPropertyByName(name);
    if (p)
    {
        p->FromString(this, value);
    }
}

/**
  * sorts the children of this object using the provided functor
  *
  * inputs:
  * @param inSort - the sort comparison algorithm to use
  *
 **/
void proObject::SortChildren( proChildSortFunctor& inSort )
{
    if ( m_impl )
    {
        m_impl->SortInstanceProperties( this, inSort );

        PropertiesReordered();
    }
}

proProperty *AddInstanceProperty(proObject *obj, const string &name, int value)
{
    static proClass *cls = proClassRegistry::ClassForName<proClass*>("proPropertyIntOwner");
    proPropertyIntOwner *pro = static_cast<proPropertyIntOwner*>(cls->NewInstance());
    pro->SetName(name);
    pro->SetValue(obj, value);
    obj->AddInstanceProperty(pro);
    return pro;
}

proProperty *AddInstanceProperty(proObject *obj, const string &name, uint value)
{
    static proClass *cls = proClassRegistry::ClassForName<proClass*>("proPropertyUintOwner");
    proPropertyUintOwner *pro = static_cast<proPropertyUintOwner*>(cls->NewInstance());
    pro->SetName(name);
    pro->SetValue(obj, value);
    obj->AddInstanceProperty(pro);
    return pro;
}

proProperty *AddInstanceProperty(proObject *obj, const string &name, float value)
{
    static proClass *cls = proClassRegistry::ClassForName<proClass*>("proPropertyFloatOwner");
    proPropertyFloatOwner *pro = static_cast<proPropertyFloatOwner*>(cls->NewInstance());
    pro->SetName(name);
    pro->SetValue(obj, value);
    obj->AddInstanceProperty(pro);
    return pro;
}

proProperty *AddInstanceProperty(proObject *obj, const string &name, bool value)
{
    static proClass *cls = proClassRegistry::ClassForName<proClass*>("proPropertyBoolOwner");
    proPropertyBoolOwner *pro = static_cast<proPropertyBoolOwner*>(cls->NewInstance());
    pro->SetName(name);
    pro->SetValue(obj, value);
    obj->AddInstanceProperty(pro);
    return pro;
}

proProperty *AddInstanceProperty(proObject *obj, const string &name, const string &value)
{
    static proClass *cls = proClassRegistry::ClassForName<proClass*>("proPropertyStringOwner");
    proPropertyStringOwner *pro = static_cast<proPropertyStringOwner*>(cls->NewInstance());
    pro->SetName(name);
    pro->SetValue(obj, value);
    obj->AddInstanceProperty(pro);
    return pro;
}


//*****************************************************************************
/*
 *  UNIT TESTS
 */
//*****************************************************************************

#if BUILD_UNIT_TESTS

#include "arda2/test/tstUnit.h"
#include "arda2/properties/proPropertyDelegate.h"
#include "arda2/properties/proClassNative.h"
#include "arda2/properties/proPropertyNative.h"
#include "arda2/properties/proChildSort.h"

//-----------------------------------------------------------------------------

static const int defaultInt         = 7711;
static const uint defaultUint       = 11234;
static const float defaultFloat     = 0.55113f;
static const string defaultString   = "default";
static const bool defaultBool       = true;
static const int defaultFunky       = 1234;
static const uint defaultBits       = ( (1<<1) | (1<<3) );
static const wstring defaultWString = L"wdefault";

static const string newName = "foobar";
static const int newX       = 51;
static const uint newSize   = 99999;
static const float newRatio = 0.4f;
static const int newFunky   = 4321; 

/**
 * test class for proObject tests
*/
class proTestClass : public proObject
{
    PRO_DECLARE_OBJECT
public:
    proTestClass() : 
        m_Funky(defaultFunky),
        m_x(defaultInt), 
        m_size(defaultUint), 
        m_ratio(defaultFloat),
        m_name(defaultString), 
        m_flag(defaultBool),
        m_number(0),
		m_enumNumber(0),
        m_objectPtr(NULL),
        m_objectPtrFunky(0),
        m_bits(defaultBits)
     {}

    virtual ~proTestClass() 
    {
        SafeDelete(m_objectPtr);
    }

    int GetX() const { return m_x;}
    uint GetSize() const { return m_size;}
    float GetRatio() const { return m_ratio;}
    string GetName() const { return m_name;}
    bool GetFlag() const { return m_flag;}
    uint GetNumber() const { return m_number;}
    proObject *GetObject() const { return m_objectPtr; }
    uint GetEnumTableNumber() const { return m_enumNumber;}
    int GetAroundAndFunky() const { return m_Funky; }
    proObject *GetFunkyObject() const { return m_objectPtrFunky; }
    uint GetBits() const { return m_bits; }

    void SetX(int value) { m_x = value; }
    void SetSize(int value) { m_size = value; }
    void SetRatio(float value) { m_ratio = value; }
    void SetName(const string &value) { m_name = value; }
    void SetFlag(bool value) { m_flag = value; }
    void SetNumber(uint value) { m_number = value; }
    void SetObject(proObject *value) { m_objectPtr = value; }
    void SetEnumTableNumber(uint value) { m_enumNumber = value; }
    void SetDownAndFunky(int value) { m_Funky = value; }
    void SetFunkyObject(proObject *value) { m_objectPtrFunky = value; }
    void SetBits( uint aBits ) { m_bits = aBits; }

	enum Values
	{
		EVT_A, EVT_B, EVT_C, EVT_D,
	};

    static proPropertyEnum::EnumTable s_EnumTable[4];

    int m_Funky;

protected:
    int m_x;
    uint m_size;
    float m_ratio;
    string m_name;
    bool m_flag;
    uint m_number;
	uint m_enumNumber;
    proObject *m_objectPtr;
    proObject *m_objectPtrFunky;
    uint m_bits;
};

proPropertyEnum::EnumTable proTestClass::s_EnumTable[4] =
{
	"enumA", proTestClass::EVT_A,
	"enumB", proTestClass::EVT_B,
	"enumC", proTestClass::EVT_C,
	"enumD", proTestClass::EVT_D,
};

//----------------------------------------------------------------------------
// this section corresponds to the tests performed on the properties below
// basic types (with out annotations)
//

PRO_REGISTER_CLASS(proTestClass, proObject)
REG_PROPERTY_INT(proTestClass, X)
REG_PROPERTY_UINT(proTestClass, Size)
REG_PROPERTY_FLOAT(proTestClass, Ratio)
REG_PROPERTY_STRING(proTestClass, Name)
REG_PROPERTY_BOOL(proTestClass, Flag)
REG_PROPERTY_ENUM(proTestClass, Number, "zero,0,one,1,two,2,three,3,four,4,five,5")
REG_PROPERTY_ENUM_TABLE(proTestClass, EnumTableNumber, proTestClass::s_EnumTable, ARRAY_LENGTH(proTestClass::s_EnumTable) )
REG_PROPERTY_OBJECT(proTestClass, Object, proObject)
REG_PROPERTY_BITFIELD(proTestClass, Bits, "one,1,two,2,three,3,four,4" )
REG_PROPERTY_INT_DELEGATE(proTestClass, Funky, GetAroundAndFunky, SetDownAndFunky )
REG_PROPERTY_ENUM_DELEGATE(proTestClass, FunkyNumber, "zero,0,one,1,two,2,three,3,four,4,five,5", GetNumber, SetNumber)
REG_PROPERTY_ENUM_TABLE_DELEGATE(proTestClass, FunkyEnumTableNumber, proTestClass::s_EnumTable, ARRAY_LENGTH(proTestClass::s_EnumTable), GetEnumTableNumber, SetEnumTableNumber )
REG_PROPERTY_OBJECT_DELEGATE(proTestClass, FunkyPtr, proObject, GetFunkyObject, SetFunkyObject )

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

/*
* these are for compile tests
*/

// basic types (with annotations)
class proTestClassWithAnnotations : public proObject
{
    PRO_DECLARE_OBJECT
public:
    proTestClassWithAnnotations() : 
        m_x(0), 
        m_size(0), 
        m_ratio(0),
        m_name(""), 
        m_flag(false),
        m_number(0),
		m_enumNumber(0),
        m_objectPtr(0),
        m_bits(0)
     {}

    virtual ~proTestClassWithAnnotations() 
    {
        SafeDelete(m_objectPtr);
    }

    int GetX() const { return m_x;}
    uint GetSize() const { return m_size;}
    float GetRatio() const { return m_ratio;}
    string GetName() const { return m_name;}
    bool GetFlag() const { return m_flag;}
    uint GetNumber() const { return m_number;}
    proObject *GetObject() const { return m_objectPtr; }
    uint GetEnumTableNumber() const { return m_enumNumber;}
    uint GetBits() const { return m_bits; }

    void SetX(int value) { m_x = value; }
    void SetSize(int value) { m_size = value; }
    void SetRatio(float value) { m_ratio = value; }
    void SetName(const string &value) { m_name = value; }
    void SetFlag(bool value) { m_flag = value; }
    void SetNumber(uint value) { m_number = value; }
    void SetEnumTableNumber(uint value) { m_enumNumber = value; }
    void SetObject(proObject *value) { m_objectPtr = value; }
    void SetBits( uint aBits ) { m_bits = aBits; }

	enum Values
	{
		EVT_A, EVT_B, EVT_C, EVT_D,
	};

    static proPropertyEnum::EnumTable s_EnumTable[4];

protected:
    int m_x;
    uint m_size;
    float m_ratio;
    string m_name;
    bool m_flag;
    uint m_number;
	uint m_enumNumber;
    proObject *m_objectPtr;
    uint m_bits;
};

proPropertyEnum::EnumTable proTestClassWithAnnotations::s_EnumTable[] =
{
	"enumA2", proTestClassWithAnnotations::EVT_A,
	"enumB2", proTestClassWithAnnotations::EVT_B,
	"enumC2", proTestClassWithAnnotations::EVT_C,
	"enumD2", proTestClassWithAnnotations::EVT_D,
};

PRO_REGISTER_CLASS(proTestClassWithAnnotations, proObject)
REG_PROPERTY_INT_ANNO(proTestClassWithAnnotations, X, "Min|20|Max|50|Default|-101" )
REG_PROPERTY_UINT_ANNO(proTestClassWithAnnotations, Size, "Default|101")
REG_PROPERTY_FLOAT_ANNO(proTestClassWithAnnotations, Ratio, "Default|1.01")
REG_PROPERTY_STRING_ANNO(proTestClassWithAnnotations, Name, "Default|sz101")
REG_PROPERTY_BOOL_ANNO(proTestClassWithAnnotations, Flag, "Default|true")
REG_PROPERTY_ENUM_ANNO(proTestClassWithAnnotations, Number, "zero,0,one,1,two,2,three,3,four,4,five,5", "Default|three")
REG_PROPERTY_ENUM_TABLE_ANNO(proTestClassWithAnnotations, EnumTableNumber, proTestClassWithAnnotations::s_EnumTable, (uint)ARRAY_LENGTH(proTestClassWithAnnotations::s_EnumTable), "Default|enumD2")
REG_PROPERTY_OBJECT_ANNO(proTestClassWithAnnotations, Object, proObject, "TestAnno|testval")
REG_PROPERTY_BITFIELD_ANNO(proTestClassWithAnnotations, Bits, "one,1,two,2,three,3,four,4" , "Default|four")

//----------------------------------------------------------------------------
// basic types using the delegate macros
//
class proBasicTypesWithDelegates : public proObject
{
    PRO_DECLARE_OBJECT
public:
    proBasicTypesWithDelegates() : 
        m_x(0), 
        m_size(0), 
        m_ratio(0),
        m_name(""), 
        m_flag(false),
        m_number(0),
		m_enumNumber(0),
        m_objectPtr(0),
        m_bits(0),
        m_FunkyPtr(0),
        m_FunkyFloat(0),
        m_FunkyEnum(0),
        m_FunkyBits(0)
     {}

    virtual ~proBasicTypesWithDelegates() 
    {
        SafeDelete(m_objectPtr);
    }

    int GetX() const { return m_x;}
    uint GetSize() const { return m_size;}
    float GetRatio() const { return m_ratio;}
    string GetName() const { return m_name;}
    bool GetFlag() const { return m_flag;}
    uint GetNumber() const { return m_number;}
    proObject *GetObject() const { return m_objectPtr; }
    uint GetEnumTableNumber() const { return m_enumNumber;}
    uint GetBits() const { return m_bits; }
    proObject *GetFunkyPtr() const { return m_FunkyPtr; }
    uint GetFunkyEnum() const { return m_FunkyEnum; }
    uint GetFunkyBits() const { return m_FunkyBits; }
    float FloatGetFunkyDude() const { return m_FunkyFloat; }

    void SetX(int value) { m_x = value; }
    void SetSize(int value) { m_size = value; }
    void SetRatio(float value) { m_ratio = value; }
    void SetName(const string &value) { m_name = value; }
    void SetFlag(bool value) { m_flag = value; }
    void SetNumber(uint value) { m_number = value; }
    void SetEnumTableNumber(uint value) { m_enumNumber = value; }
    void SetObject(proObject *value) { m_objectPtr = value; }
    void SetBits( uint aBits ) { m_bits = aBits; }
    void SetFunkyPtr( proObject* aPtr ) { m_FunkyPtr = aPtr; }
    void SetFunkyEnum( uint anEnum ) { m_FunkyEnum = anEnum; }
    void SetFunkyBits( uint aBits ) { m_FunkyBits = aBits; }
    void FloatSetFunkyAndStuff( float aFloat ) { m_FunkyFloat = aFloat; }

    static proPropertyEnum::EnumTable s_EnumTable[4];

protected:

	enum Values
	{
		EVT_A, EVT_B, EVT_C, EVT_D,
	};

protected:
    int m_x;
    uint m_size;
    float m_ratio;
    string m_name;
    bool m_flag;
    uint m_number;
	uint m_enumNumber;
    proObject *m_objectPtr;
    uint m_bits;
    proObject* m_FunkyPtr;
    float m_FunkyFloat;
    uint m_FunkyEnum;
    uint m_FunkyBits;
};

proPropertyEnum::EnumTable proBasicTypesWithDelegates::s_EnumTable[] =
{
	"pbtwdA", proBasicTypesWithDelegates::EVT_A,
	"pbtwdB", proBasicTypesWithDelegates::EVT_B,
	"pbtwdC", proBasicTypesWithDelegates::EVT_C,
	"pbtwdD", proBasicTypesWithDelegates::EVT_D,
};

typedef proBasicTypesWithDelegates PBTWD;

PRO_REGISTER_CLASS(proBasicTypesWithDelegates, proObject)
REG_PROPERTY_INT_DELEGATE(proBasicTypesWithDelegates, X, GetX, SetX )
REG_PROPERTY_UINT_DELEGATE(proBasicTypesWithDelegates, Number, GetNumber, SetNumber)
REG_PROPERTY_FLOAT_DELEGATE(proBasicTypesWithDelegates, Ratio, GetRatio, SetRatio)
REG_PROPERTY_STRING_DELEGATE(proBasicTypesWithDelegates, Name, GetName, SetName)
REG_PROPERTY_BOOL_DELEGATE(proBasicTypesWithDelegates, Flag, GetFlag, SetFlag)
REG_PROPERTY_OBJECT_DELEGATE(proBasicTypesWithDelegates, FunkyPtr, proObject, GetFunkyPtr, SetFunkyPtr )
REG_PROPERTY_ENUM_DELEGATE(proBasicTypesWithDelegates, FunkyNumber, "zero,0,one,1,two,2,three,3,four,4,five,5", GetNumber, SetNumber)
REG_PROPERTY_ENUM_TABLE_DELEGATE(proBasicTypesWithDelegates, FunkyEnumTableNumber, PBTWD::s_EnumTable, (uint)ARRAY_LENGTH(PBTWD::s_EnumTable), GetFunkyEnum, SetFunkyEnum )
REG_PROPERTY_BITFIELD_DELEGATE(proBasicTypesWithDelegates, FunkyBits, "one,1,two,2,three,3,four,4", GetFunkyBits, SetFunkyBits )
REG_PROPERTY_FLOAT_DELEGATE(proBasicTypesWithDelegates, FloatNum, FloatGetFunkyDude, FloatSetFunkyAndStuff)

//----------------------------------------------------------------------------
// basic types (with annotations) using the delegate macros

class proBasicTypesWithAnnotationsAndDelegates : public proObject
{
    PRO_DECLARE_OBJECT
public:
    proBasicTypesWithAnnotationsAndDelegates() : 
        m_x(0), 
        m_size(0), 
        m_ratio(0),
        m_name(""), 
        m_flag(false),
        m_number(0),
		m_enumNumber(0),
        m_objectPtr(0),
        m_bits(0),
        m_FunkyPtr(0),
        m_FunkyFloat(0),
        m_FunkyEnum(0),
        m_FunkyBits(0)

     {}

    virtual ~proBasicTypesWithAnnotationsAndDelegates() 
    {
        SafeDelete(m_objectPtr);
    }

    int GetX() const { return m_x;}
    uint GetSize() const { return m_size;}
    float GetRatio() const { return m_ratio;}
    string GetName() const { return m_name;}
    bool GetFlag() const { return m_flag;}
    uint GetNumber() const { return m_number;}
    proObject *GetObject() const { return m_objectPtr; }
    uint GetEnumTableNumber() const { return m_enumNumber;}
    uint GetBits() const { return m_bits; }
    proObject *GetFunkyPtr() const { return m_FunkyPtr; }
    uint GetFunkyEnum() const { return m_FunkyEnum; }
    uint GetFunkyBits() const { return m_FunkyBits; }
    float FloatGetFunkyDude() const { return m_FunkyFloat; }

    void SetX(int value) { m_x = value; }
    void SetSize(int value) { m_size = value; }
    void SetRatio(float value) { m_ratio = value; }
    void SetName(const string &value) { m_name = value; }
    void SetFlag(bool value) { m_flag = value; }
    void SetNumber(uint value) { m_number = value; }
    void SetEnumTableNumber(uint value) { m_enumNumber = value; }
    void SetObject(proObject *value) { m_objectPtr = value; }
    void SetBits( uint aBits ) { m_bits = aBits; }
    void SetFunkyPtr( proObject* aPtr ) { m_FunkyPtr = aPtr; }
    void SetFunkyEnum( uint anEnum ) { m_FunkyEnum = anEnum; }
    void SetFunkyBits( uint aBits ) { m_FunkyBits = aBits; }
    void FloatSetFunkyAndStuff( float aFloat ) { m_FunkyFloat = aFloat; }

    static proPropertyEnum::EnumTable s_EnumTable[4];

protected:

	enum Values
	{
		EVT_A, EVT_B, EVT_C, EVT_D,
	};

protected:
    int m_x;
    uint m_size;
    float m_ratio;
    string m_name;
    bool m_flag;
    uint m_number;
	uint m_enumNumber;
    proObject *m_objectPtr;
    uint m_bits;
    proObject* m_FunkyPtr;
    float m_FunkyFloat;
    uint m_FunkyEnum;
    uint m_FunkyBits;
};

proPropertyEnum::EnumTable proBasicTypesWithAnnotationsAndDelegates::s_EnumTable[] =
{
	"pbtwaadA", proBasicTypesWithAnnotationsAndDelegates::EVT_A,
	"pbtwaadB", proBasicTypesWithAnnotationsAndDelegates::EVT_B,
	"pbtwaadC", proBasicTypesWithAnnotationsAndDelegates::EVT_C,
	"pbtwaadD", proBasicTypesWithAnnotationsAndDelegates::EVT_D,
};

PRO_REGISTER_CLASS(proBasicTypesWithAnnotationsAndDelegates, proObject)
REG_PROPERTY_INT_DELEGATE_ANNO(proBasicTypesWithAnnotationsAndDelegates, X, GetX, SetX, "Min|-10|Max|10|Default|1")
REG_PROPERTY_UINT_DELEGATE_ANNO(proBasicTypesWithAnnotationsAndDelegates, Number, GetNumber, SetNumber, "Default|101")
REG_PROPERTY_FLOAT_DELEGATE_ANNO(proBasicTypesWithAnnotationsAndDelegates, Ratio, GetRatio, SetRatio, "Default|1.01")
REG_PROPERTY_STRING_DELEGATE_ANNO(proBasicTypesWithAnnotationsAndDelegates, Name, GetName, SetName, "Default|name")
REG_PROPERTY_BOOL_DELEGATE_ANNO(proBasicTypesWithAnnotationsAndDelegates, Flag, GetFlag, SetFlag, "Default|false")
REG_PROPERTY_OBJECT_DELEGATE_ANNO(proBasicTypesWithAnnotationsAndDelegates, FunkyPtr, proObject, GetFunkyPtr, SetFunkyPtr, "Note|note")
REG_PROPERTY_ENUM_DELEGATE_ANNO(proBasicTypesWithAnnotationsAndDelegates, FunkyNumber, "zero,0,one,1,two,2,three,3,four,4,five,5", GetNumber, SetNumber, "Default|three")
REG_PROPERTY_ENUM_TABLE_DELEGATE_ANNO(proBasicTypesWithAnnotationsAndDelegates, FunkyEnumTableNumber, proBasicTypesWithAnnotationsAndDelegates::s_EnumTable, (uint)ARRAY_LENGTH(proBasicTypesWithAnnotationsAndDelegates::s_EnumTable), GetFunkyEnum, SetFunkyEnum, "Default|pbtwaadB")
REG_PROPERTY_BITFIELD_DELEGATE_ANNO(proBasicTypesWithAnnotationsAndDelegates, FunkyBits, "one,1,two,2,three,3,four,4", GetFunkyBits, SetFunkyBits, "Default|four")
REG_PROPERTY_FLOAT_DELEGATE_ANNO(proBasicTypesWithAnnotationsAndDelegates, FloatNum, FloatGetFunkyDude, FloatSetFunkyAndStuff, "Default|1.234")

//-----------------------------------------------------------------------------
// a derived class from a proObject test

const uint defaultDerivedInt = 0xDEADBEEF;

class proTestClassDerived : public proTestClass
{
    PRO_DECLARE_OBJECT
public:
    proTestClassDerived() : m_X(defaultDerivedInt)
    {
    }

    ~proTestClassDerived() {}

    uint m_X;

    uint GetX( void ) const 
    { 
        return m_X; 
    }

    void SetX( const uint anX ) 
    { 
        m_X = anX;
    }
};
PRO_REGISTER_CLASS(proTestClassDerived, proTestClass)
REG_PROPERTY_UINT(proTestClassDerived, X)

//-----------------------------------------------------------------------------
// the test unit object

class proObjectTests : public tstUnit
{
private:
    proTestClass *m_object;

public:
    proObjectTests()
    {
    }

    virtual void Register()
    {
        SetName("proObject");

        proClassRegistry::Init();

        AddTestCase("testBasicTypes", &proObjectTests::testBasicTypes);
        AddTestCase("testStringConversions", &proObjectTests::testStringConversions);
        AddTestCase("testClasses", &proObjectTests::testClasses);
        AddTestCase("testClone", &proObjectTests::testClone);
        AddTestCase("testAssign", &proObjectTests::testAssign);
        AddTestCase("testObjectProperties", &proObjectTests::testAssign);
        AddTestCase("testCloneChildren", &proObjectTests::testCloneChildren);
        AddTestCase("testSortChildren", &proObjectTests::testSortChildren );
        AddTestCase("testEnumTable", &proObjectTests::testEnumTable );

        // test throws an assert.  Known to fail.
        //AddTestCase( "testDuplicateNames", &proObjectTests::testDuplicateNames);

    };

    virtual void TestCaseSetup()
    {
        m_object = new proTestClass();
    }

    virtual void TestCaseTeardown()
    {
        SafeDelete(m_object);
    }

    void testBasicTypes() const
    {
        proProperty *pro = NULL;

        // Test integer property type
        pro = m_object->GetPropertyByName("X");
        TESTASSERT( pro->GetDataType() == DT_int );
        proPropertyInt *proInt = static_cast<proPropertyInt*>(pro);
        int valueInt = proInt->GetValue(m_object);
        TESTASSERT( valueInt == defaultInt);

        // Test unsigned integer property type
        pro = m_object->GetPropertyByName("Size");
        TESTASSERT( pro->GetDataType() == DT_uint );
        proPropertyUint *proUint = static_cast<proPropertyUint*>(pro);
        uint valueUint = proUint->GetValue(m_object);
        TESTASSERT( valueUint == defaultUint);

        // Test float property type
        pro = m_object->GetPropertyByName("Ratio");
        TESTASSERT( pro->GetDataType() == DT_float );
        proPropertyFloat *proFlt = static_cast<proPropertyFloat*>(pro);
        float valueFlt = proFlt->GetValue(m_object);
        TESTASSERT( valueFlt == defaultFloat);

        // Test string property type
        pro = m_object->GetPropertyByName("Name");
        TESTASSERT( pro->GetDataType() == DT_string );
        proPropertyString *proStr = static_cast<proPropertyString*>(pro);
        string valueStr = proStr->GetValue(m_object);
        TESTASSERT( valueStr == defaultString);

        // Test bool property type
        pro = m_object->GetPropertyByName("Flag");
        TESTASSERT( pro->GetDataType() == DT_bool );
        proPropertyBool *proBool = static_cast<proPropertyBool*>(pro);
        bool valueBool = proBool->GetValue(m_object);
        TESTASSERT( valueBool == defaultBool);

        // Test the bit field type
        pro = m_object->GetPropertyByName("Bits");
        TESTASSERT( pro->GetDataType() == DT_bitfield );
        proPropertyBitfield *proBits = static_cast<proPropertyBitfield*>(pro);
        uint valueBits = proBits->GetValue(m_object);
        TESTASSERT( valueBits == defaultBits );
        m_object->SetValue( "Bits", "one|two|three" );        
        valueBits = proBits->GetValue(m_object);
        TESTASSERT( valueBits == ( (1<<1) | (1<<2) | (1<<3) ) );

        // Test enum property type
        pro = m_object->GetPropertyByName("Number");
        TESTASSERT( pro->GetDataType() == DT_enum );
        proPropertyEnum *proEnum = static_cast<proPropertyEnum*>(pro);
        uint valueEnum = proEnum->GetValue(m_object);
        TESTASSERT( valueEnum == 0);
		proEnum->SetValue( m_object, proEnum->GetEnumerationValue("one") );
        uint valueEnum1 = proEnum->GetValue(m_object);
        TESTASSERT( valueEnum1 == 1);

		// testing the delegate property macros
        pro = m_object->GetPropertyByName("Funky");
        TESTASSERT( pro->GetDataType() == DT_int );
        proInt = static_cast<proPropertyInt*>(pro);
        TESTASSERT( m_object->GetAroundAndFunky() == proInt->GetValue(m_object) );
        proInt->SetValue( m_object, 4321 );
        int valueEnumInt = proInt->GetValue( m_object );
        TESTASSERT( valueEnumInt == 4321 );
        TESTASSERT( m_object->GetAroundAndFunky() == 4321 );

        // testing delegate macro with object pointers
        proObject* tempObj = new proObject();
        pro = m_object->GetPropertyByName("FunkyPtr");
        TESTASSERT( pro->GetDataType() == DT_object );
        proPropertyObject* proObj = static_cast<proPropertyObject*>(pro);
        TESTASSERT( m_object->GetFunkyObject() == proObj->GetValue(m_object) );
        proObj->SetValue( m_object, tempObj );
        proObject* testObjPtr = proObj->GetValue(m_object);
        TESTASSERT( testObjPtr == tempObj );
        proObj->SetValue( m_object, 0 );
        delete tempObj;
    }

    void testStringConversions() const
    {
        proProperty *pro = NULL;
        string result;
        char buf[512];

        // test int conversion
        pro = m_object->GetPropertyByName("X");
        result = pro->ToString(m_object);
        sprintf(buf, "%d", defaultInt);
        TESTASSERT( result == buf);
        pro->FromString(m_object, "44");
        result = pro->ToString(m_object);
        TESTASSERT( result == "44");

        // test uint conversion
        pro = m_object->GetPropertyByName("Size");
        result = pro->ToString(m_object);
        sprintf(buf, "%u", defaultUint);
        TESTASSERT( result == buf);
        pro->FromString(m_object, "44");
        result = pro->ToString(m_object);
        TESTASSERT( result == "44");

        // test bool conversion
        pro = m_object->GetPropertyByName("Flag");
        result = pro->ToString(m_object);
        TESTASSERT( result == "true");
        pro->FromString(m_object, "false");
        result = pro->ToString(m_object);
        TESTASSERT( result == "false");


        // test float conversion
        pro = m_object->GetPropertyByName("Ratio");
        result = pro->ToString(m_object);
        sprintf(buf, "%f", defaultFloat);
        TESTASSERT( result == buf);
        float f = 0.33f;
        sprintf(buf, "%f", f);
        pro->FromString(m_object, buf);
        result = pro->ToString(m_object);
        //TESTASSERT( Approx( (float)atof(result.c_str()), f) );

    }

    void testClasses() const
    {
        // test factory construction
        proClass *cls1 = proClassRegistry::ClassForName<proClass*>("proTestClass");
        proObject *same= cls1->NewInstance();
        TESTASSERT( same->GetClass() == m_object->GetClass());

        // test inheritance
        proClass *cls2 = proClassRegistry::ClassForName<proClass*>("proTestClassDerived");
        proObject *derived = cls2->NewInstance();
        TESTASSERT( derived->GetClass()->Instanceof( m_object->GetClass() ) );

        // test inheritance of properties
        proProperty *pro = derived->GetPropertyByName("Name");
        string value = pro->ToString(derived);
        TESTASSERT( value == defaultString);

        // test named "X" property at derived level
        pro = derived->GetPropertyByName("X");
        TESTASSERT( pro->GetDataType() == DT_uint );
        proPropertyUint *proUint = static_cast<proPropertyUint*>(pro);
        uint valueUint = proUint->GetValue(derived);
        TESTASSERT( valueUint == defaultDerivedInt);

        // test named "X" property at parent level (skipping derived level)
        pro = derived->GetClass()->GetParent()->GetPropertyByName("X");
        TESTASSERT( pro->GetDataType() == DT_int );
        proPropertyInt *proInt = static_cast<proPropertyInt*>(pro);
        int valueInt = proInt->GetValue(derived);
        TESTASSERT( valueInt == defaultInt);

        SafeDelete(same);
        SafeDelete(derived);
    }


    void testClone() const
    {
        m_object->SetName(newName);
        m_object->SetX(newX);
        m_object->SetSize(newSize);
        m_object->SetRatio(newRatio);
        m_object->SetDownAndFunky(newFunky);

        proObject *obj = new proObject();
        m_object->SetObject(obj);

        // check the cloned object is of the correct type
        proObject *newObject = m_object->Clone();
        TESTASSERT( newObject->GetClass() == m_object->GetClass());

        // check that all the values are correctly set
        proTestClass *clone = static_cast<proTestClass*>(newObject);
        TESTASSERT( clone->GetName() == newName);
        TESTASSERT( clone->GetX() == newX);
        TESTASSERT( clone->GetSize() == newSize);
        TESTASSERT( clone->GetRatio() == newRatio);
        TESTASSERT( clone->GetObject() != NULL);
        TESTASSERT( clone->GetObject() != obj);
        TESTASSERT( clone->GetAroundAndFunky() == newFunky );

        SafeDelete(newObject);
    }

    void testAssign() const
    {
        m_object->SetName(newName);
        m_object->SetX(newX);
        m_object->SetSize(newSize);
        m_object->SetRatio(newRatio);

        // create new object to assign
        proTestClass *newObject = proClassRegistry::NewInstance<proTestClass*>("proTestClass");
        
        // perform assignment
        newObject->Assign(m_object);

        // check that all the values are correctly set
        TESTASSERT( newObject->GetName() == newName);
        TESTASSERT( newObject->GetX() == newX);
        TESTASSERT( newObject->GetSize() == newSize);
        TESTASSERT( newObject->GetRatio() == newRatio);

        SafeDelete(newObject);
    }

    void testObjectProperties() const
    {
        proTestClass *newObject1 = proClassRegistry::NewInstance<proTestClass*>("proTestClass");
        proTestClass *newObject2 = proClassRegistry::NewInstance<proTestClass*>("proTestClass");

        newObject1->AddChild(newObject2);
        newObject1->AddChild(newObject2);

        SafeDelete(newObject1);
    }

    void testCloneChildren() const
    {
        proTestClass *newObject1 = proClassRegistry::NewInstance<proTestClass*>("proTestClass");
        proTestClass *newObject2 = proClassRegistry::NewInstance<proTestClass*>("proTestClass");

        newObject1->SetName("obj1");
        newObject2->SetName("obj2");

        m_object->AddChild(newObject1, "One");
        m_object->AddChild(newObject2, "Two");

        proTestClass *clone = (proTestClass*)m_object->Clone();

        TESTASSERT(clone != NULL);

        proProperty *propOne = clone->GetPropertyByName("One");
        TESTASSERT(propOne != NULL);
        proTestClass *cloneOne = PRO_AS_TYPED_OBJECT(proTestClass, clone, propOne);
        TESTASSERT(cloneOne != NULL);
        TESTASSERT(cloneOne->GetName() == "obj1");

        proProperty *propTwo = clone->GetPropertyByName("Two");
        TESTASSERT(propTwo != NULL);
        proTestClass *cloneTwo = PRO_AS_TYPED_OBJECT(proTestClass, clone, propTwo);
        TESTASSERT(cloneTwo != NULL);
        TESTASSERT(cloneTwo->GetName() == "obj2");

        SafeDelete(clone);
    }

    void testSortChildren( void ) const
    {
        const string child1Name( "Z" );
        const string child2Name( "Q" );
        const string child3Name( "R" );

        // just use the name sorter
        proChildSortByName sorter;

        // build an object
        proObject testObject;

        // sort 0 children
        testObject.SortChildren( sorter );

        // add first child
        proObject child1;
        testObject.AddChild( &child1, child1Name, DP_refValue );

        testObject.SortChildren( sorter );
        TESTASSERT( testObject.GetPropertyAt( 0 )->GetName() == child1Name );

        // add another child
        proObject child2;
        testObject.AddChild( &child2, child2Name, DP_refValue );

        testObject.SortChildren( sorter );
        TESTASSERT( testObject.GetPropertyAt( 0 )->GetName() == child2Name );
        TESTASSERT( testObject.GetPropertyAt( 1 )->GetName() == child1Name );

        // add the third child
        proObject child3;
        testObject.AddChild( &child2, child3Name, DP_refValue );

        testObject.SortChildren( sorter );
        TESTASSERT( testObject.GetPropertyAt( 0 )->GetName() == child2Name );
        TESTASSERT( testObject.GetPropertyAt( 1 )->GetName() == child3Name );
        TESTASSERT( testObject.GetPropertyAt( 2 )->GetName() == child1Name );
    }

    void testDuplicateNames() const
    {
        proObject *child1 = new proObject();
        proObject *child2 = new proObject();

        m_object->AddChild(child1, "dupe");
        m_object->AddChild(child2, "dupe");

        m_object->RemoveChild(child2);

        proPropertyObjectOwner* prop = (proPropertyObjectOwner*)m_object->GetPropertyByName("dupe");
        proObject *lookup = prop->GetValue(m_object);
        TESTASSERT(lookup == child1);
       
    }

    void testEnumTable() const
    {
        // Test new enum property initializer
		proProperty* pro = m_object->GetPropertyByName("EnumTableNumber");
        TESTASSERT( pro->GetDataType() == DT_enum );
		proPropertyEnum *proEnum2 = static_cast<proPropertyEnum*>(pro);
		proEnum2->SetValue( m_object, proEnum2->GetEnumerationValue(proTestClass::s_EnumTable[2].key) );
		uint valueEnum2 = proEnum2->GetValue(m_object);
        TESTASSERT( valueEnum2 == proTestClass::s_EnumTable[2].value );

		// inserting some dynamic fields to the current enum table
		{
			vector< string > stringList;
			proEnum2->GetEnumeration( stringList );

			proPropertyEnum::EnumTable entry;

			vector< proPropertyEnum::EnumTable > table;
			for( vector<string>::iterator sit = stringList.begin(); sit != stringList.end(); sit++ )
			{
				entry.key   = *(sit);
				entry.value = proEnum2->GetEnumerationValue(entry.key);
				table.push_back( entry );
			}

			entry.key   = "dynA";
			entry.value = 101;
			table.push_back( entry );

			entry.key   = "dynB";
			entry.value = 102;
			table.push_back( entry );

			proEnum2->SetEnumTable( &table.front(), (uint)table.size() );
		}

        // verifying the new table has been set
		proEnum2->SetValue( m_object, proEnum2->GetEnumerationValue("dynA") );
		valueEnum2 = proEnum2->GetValue(m_object);
		TESTASSERT( valueEnum2 == 101 );
    }
};

EXPORTUNITTESTOBJECT(proObjectTests);
#endif

