#include "arda2/core/corFirst.h"

#include "proFirst.h"

#include "arda2/properties/proObject.h"
#include "arda2/properties/proProperty.h"
#include "arda2/properties/proClass.h"
#include "arda2/properties/proClassRegistry.h"

using namespace std;

proClassRegistry::proClassRegistry() :
    m_initialized(false)
{
}

proClassRegistry::~proClassRegistry()
{
    ClassMap::iterator it = m_classMap.begin();
    for (; it != m_classMap.end(); ++it)
    {
        proClass *cls = (*it).second;
        SafeDelete(cls);
    }
    m_classMap.clear();
}

/**
 * create an instance of an object of a type.
 * this sets the class member of the object.
 */
proObject *proClassRegistry::NewInstance(const string &className)
{
    proClass *cls = ClassForName<proClass*>(className);
    if (cls)
    {
        proObject *obj = cls->NewInstance();
        if (obj)
        {
            return obj;
        }
    }
    return NULL;
}

// register a class
void proClassRegistry::RegisterClass(proClass *cls)
{
    //ERR_REPORTV(ES_Info, ("Registering class <%s>", cls->GetName().c_str()));
    proClassRegistry &reg = GetSingleton();
    ClassMap::iterator it = reg.m_classMap.find(cls->GetName());
    if (it != reg.m_classMap.end())
    {
        // it is ok if a class is registered twice. this can happen when two adapters
        // target the same class. handle it here without leaking the first registered class object.
        proClass *oldClass = (*it).second;
        SafeDelete(oldClass);
        reg.m_classMap.erase(it);
    }
    reg.m_classMap.insert( make_pair(cls->GetName(), cls) );
}

void proClassRegistry::RegisterInheritance(const string &derivedClassName, const string& parentClassName)
{
    proClassRegistry &reg = GetSingleton();

    if (reg.m_initialized == false)
    {
        // store for creationg during Init()
        reg.m_inherMap[derivedClassName] = parentClassName;
    }
    else
    {
        // register now
        proClass *derivedClass = ClassForName<proClass*>(derivedClassName);
        proClass *parentClass = ClassForName<proClass*>(parentClassName);

        if (!derivedClass || !parentClass)
        {
            if (derivedClass->GetName() != "proObject")
                ERR_REPORTV(ES_Error, ("Error initializing class: %s/%s", derivedClassName.c_str(), parentClassName.c_str()));
        }
        else
        {
            derivedClass->m_parent = parentClass;
        }
    }
}

void proClassRegistry::Init()
{
    proClassRegistry &reg = GetSingleton();

    if (reg.m_initialized)
        return;

    // register inheritance now that all classes exist
    InherMap::iterator it = reg.m_inherMap.begin();
    for (; it != reg.m_inherMap.end(); ++it)
    {
        proClass *derivedClass = ClassForName<proClass*>((*it).first);
        proClass *parentClass = ClassForName<proClass*>((*it).second);
        if (!derivedClass || !parentClass)
        {
            if (derivedClass->GetName() != "proObject")
                ERR_REPORTV(ES_Error, ("Error initializing class: %s/%s", (*it).first.c_str(), (*it).second.c_str()));
        }
        else
        {
            derivedClass->m_parent = parentClass;
        }
    }

    // register annotations now that all properties are registered
    AnnoMap::iterator annoIt = reg.m_annoMap.begin();
    for (; annoIt != reg.m_annoMap.end(); annoIt++)
    {
        proProperty *prop = (*annoIt).first;
        prop->SetAnnotations( (*annoIt).second.c_str());
    }

    reg.m_initialized = true;
    reg.m_inherMap.clear();
    reg.m_annoMap.clear();
}

bool IsChildOf( proClass* parent, proClass* child )
{
    if ( child == NULL || parent == NULL )
        return false;

    if ( child->GetParent() == parent )
        return true;
    else
        return IsChildOf( parent, child->GetParent() );
}


// get all of the classes derived fom a class
void proClassRegistry::GetClassesOfType(proClass* cls, vector<proClass*>& vals, bool recurse )
{
    proClassRegistry &reg = GetSingleton();

    ClassMap::const_iterator it = reg.m_classMap.begin();
    for(; it != reg.m_classMap.end(); ++it)
    {
        if (recurse)
        {
            if( IsChildOf(cls, it->second) )
                vals.push_back( it->second );
        }
        else
        {
            if (it->second->GetParent() == cls)
                vals.push_back( it->second );
        }
    }
}

/**
 * check is another class is an instance of thisClass 
 *
 * @param thisCls the base class
 * @param target the other class that may be an instance of thisClss
 */
bool proClassRegistry::InstanceOf(const proClass *thisCls, const proClass *otherClass)
{
    return thisCls->Instanceof(otherClass);
}

/**
 * check is another class is an instance of thisClass 
 *
 * @param thisCls the base class
 * @param target the name of the other class that may be an instance of thisClss
 */
bool proClassRegistry::InstanceOf(const proClass *thisCls, const string &otherClassName)
{
    proClass *otherClass = ClassForName<proClass*>(otherClassName);
    return thisCls->Instanceof(otherClass);
}

proClassRegistry &proClassRegistry::GetSingleton()
{
    static proClassRegistry s_singleton;
    return s_singleton;
}

/**
 * register an annotation string for a property.
 */
void proClassRegistry::RegisterAnnotation(proProperty *prop, const string &annoString)
{
    proClassRegistry &reg = GetSingleton();
    if (reg.m_initialized == false)
    {
        reg.m_annoMap[prop] = annoString;
    }
    else
    {
        prop->SetAnnotations(annoString.c_str());
    }
}
