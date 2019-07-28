#include "arda2/core/corFirst.h"
#include "arda2/properties/proFirst.h"


#include "arda2/properties/proClass.h"
#include "arda2/properties/proPropertyOwner.h"
#include "arda2/properties/proClassRegistry.h"
#include "arda2/properties/proPropertyNative.h"

#include "arda2/properties/proClassAdapter.h"

using namespace std;

PRO_REGISTER_ADAPTER_CLASS(proClassAdapter, proClass)
REG_PROPERTY_STRING(proClassAdapter, Classname)
REG_PROPERTY_STRING(proClassAdapter, ParentClassname)

void proClassAdapter::SetClassname(const string &)
{
}

string proClassAdapter::GetClassname() const
{
    if (m_target)
        return m_target->GetName();
    else
        return "empty";
}

void proClassAdapter::SetParentClassname(const string &)
{
}

string proClassAdapter::GetParentClassname() const
{
    if (m_target)
    {
        if (m_target->GetParent())
        {
            return m_target->GetParent()->GetName();
        }
        else
        {
            return "NULL";
        }
    }
    else
    {
        return "empty";
    }
}
void proClassAdapter::Initialize()
{
    if ( !IsInitialized() )
    {
        MyType::Initialize();

        if (!m_target)
            return;

        vector<proClass*> vals;
        proClassRegistry::GetClassesOfType(m_target, vals, false);

        vector<proClass*>::iterator it = vals.begin();
        for (; it != vals.end(); it++)
        {
            proClass *cls = (*it);
            proClassAdapter *adapter = new proClassAdapter();
            adapter->SetTarget(cls);
            AddChild(adapter, cls->GetName());
        }

        for (int i=0; i < m_target->GetPropertyCount(); i++)
        {
            proProperty *prop = m_target->GetPropertyAt(i);
            proPropertyStringOwner *newProp = new proPropertyStringOwner();
            newProp->SetName(prop->GetName());
            AddInstanceProperty(newProp);

            if (proClassRegistry::InstanceOf(prop->GetClass(), "proProperty"))
            {
                if (prop->GetDataType() != DT_other && prop->GetDataType() != DT_object)
                {
                    newProp->SetValue(this, proGetNameForType(prop->GetDataType()));
                }
                else if (prop->GetIsNative())
                {
                    // use the parent class for native classes
                     newProp->SetValue(this, prop->GetClass()->GetParent()->GetName());
                }
                else
                {
                    newProp->SetValue(this, prop->GetClass()->GetName());
                }
            }
            else
            {
                newProp->SetValue(this, prop->GetClass()->GetName());
            }
        }
    }
}

