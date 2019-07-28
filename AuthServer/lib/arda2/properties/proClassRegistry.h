/**
 *  property object
 *
 *  @author Sean Riley
 *
 *  @date 1/13/2003
 *
 *  @file
**/

#ifndef __proClassRegistry_h__
#define __proClassRegistry_h__

#include "arda2/properties/proFirst.h"

#include "arda2/core/corStdString.h"
#include "arda2/core/corStlMap.h"

#ifdef RegisterClass 
#undef RegisterClass
#endif 

class proClass;
class proObject;

#define PRO_NEW_INSTANCE(T) proClassRegistry::NewInstance<T*>(#T)

#if CORE_COMPILER_MSVC
#pragma deprecated ("proNEW_INSTANCE")
#endif // CORE_COMPILER_MSVC
#define proNEW_INSTANCE PRO_NEW_INSTANCE

/**
 * repository of class objects
 */
class proClassRegistry
{
public:
    proClassRegistry();
    ~proClassRegistry();

    // one-time initialization
    static void Init();

    // register a class
    static void RegisterClass(proClass *cls);

    // register inheritance - this is actually deferred until Init()
    static void RegisterInheritance(const std::string &derivedClassName, const std::string& parentClassName); 

    // register property annotations - deferred until Init()
    static void RegisterAnnotation(proProperty *prop, const std::string &annoString);

    // look up a class instance by its name
    template<typename T> static T ClassForName( const std::string &name )
    {
        proClassRegistry &reg = GetSingleton();

        proClass *cls = NULL;
        ClassMap::const_iterator it = reg.m_classMap.find(name);
        if( it != reg.m_classMap.end() )
        {
            cls = it->second;
        }
        return static_cast<T>(cls);
    }

    // get all of the classes derived fom a class
    static void GetClassesOfType( proClass* cls, std::vector<proClass*>& vals, bool recurse = true);

    // check is another class is an instance of thisClass   
    static bool InstanceOf(const proClass *thisCls, const proClass *otherCls);
    static bool InstanceOf(const proClass *thisCls, const std::string &otherClassName);

    // create an instance of an object of a type
    static proObject *NewInstance(const std::string &className);

    // typed version of NewInstance
    template <typename T> static T NewInstance(const std::string &className)
    {
        return static_cast<T>(NewInstance(className));
    }

    static proClassRegistry &GetSingleton();

protected:

    typedef std::map<std::string, proClass*> ClassMap;
    ClassMap        m_classMap;

    typedef std::map<std::string,std::string> InherMap;
    InherMap        m_inherMap;
    bool            m_initialized;

    typedef std::map<proProperty *,std::string> AnnoMap;
    AnnoMap m_annoMap;

};



#endif //__proClassRegistry_h__
