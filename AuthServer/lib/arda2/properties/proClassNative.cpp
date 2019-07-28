#include "arda2/core/corFirst.h"

#include "proFirst.h"

#include "arda2/properties/proClassNative.h"
#include "arda2/properties/proProperty.h"
#include "arda2/properties/proObject.h"

using namespace std;

PRO_REGISTER_CLASS(proClassNative, proClass)

proClassNative::proClassNative() :
    proClass(),
    m_newObjFunc(NULL)
{
}

proClassNative::proClassNative(const string &name, pConstructFn *newObjFunc) :
    proClass(name),
    m_newObjFunc(newObjFunc)
{
}

proClassNative::~proClassNative()
{
}

proObject *proClassNative::NewInstance()
{
    if( m_newObjFunc != NULL )
    {
        proObject *obj = (*m_newObjFunc)();
        return obj;
    }
    else
    {
        return NULL;
    }
}

int proClassNative::GetPropertyCount() const
{
    int value = (int)m_properties.size();
    if (GetParent())
        value += GetParent()->GetPropertyCount();

    return value;
}

proProperty* proClassNative::GetPropertyAt(int index) const
{
    if (index < (int)m_properties.size())
    {
        return m_properties[index];
    }
    else
    {
        if (GetParent())
        {
            index -= (int)m_properties.size();
            return GetParent()->GetPropertyAt(index);
        }
    }
    return NULL;
}

proProperty* proClassNative::GetPropertyByName(const string &name) const
{
    proProperty* rtn = NULL;

    for( PropertyList::const_iterator it = m_properties.begin(); it != m_properties.end(); it++ )
    {
        if( (*it)->GetName() == name)
        {
            rtn = *it;
            break;
        }
    }
    if (!rtn)
    {
        proClass *parent = GetParent();
        if (parent)
            rtn = parent->GetPropertyByName(name);
    }
    return rtn;
}

void proClassNative::AddProperty(proProperty *property)
{
    //ERR_REPORTV(ES_Info, ("  Registering property <%s> for <%s>", property->GetName().c_str(), GetName().c_str()));
    m_properties.push_back(property);
}


errResult proClassNative::Assign( const proObject* srcObj, proObject* destObj ) const
{   
    // if the class names aren't identical, return error
    if( strcmp( GetName().c_str(), destObj->GetClass()->GetName().c_str() ) != 0  )
    {
        ERR_RETURN( ES_Error, "attempt to assign disparate types" );
    }
  
    for ( int i = 0; i < GetPropertyCount(); ++i)
    {
        proProperty *p = GetPropertyAt( i );
        p->Clone( srcObj, destObj );
    }
    return ER_Success;
}
