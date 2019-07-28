/**
 *  property class
 *
 *  @author Sean Riley
 *
 *  @date 1/13/2003
 *
 *  @file
**/


#ifndef __proClassNative_h__
#define __proClassNative_h__

#include "proFirst.h"

#include "arda2/core/corStdString.h"
#include "arda2/core/corStlVector.h"

#include "arda2/properties/proClass.h"


/** 
 * static class - represents a "native" C++ class
 */
class proClassNative : public proClass
{

  // Dont use DECALRE_OBJECT as it makes this class a friend of itself
    public:
    virtual proClass *GetClass() const
    {
        if (m_impl)
            return m_impl->GetClass();
        return s_class;
    }
   protected:
    static proClass *s_class;\

public:

    typedef proObject* (pConstructFn)();

    proClassNative();
    proClassNative(const std::string &name, pConstructFn *newObjFunc);

    ~proClassNative();

    // creates a new instance of this class
    virtual proObject *     NewInstance();

    // property interface
    virtual int             GetPropertyCount() const;
    virtual proProperty*    GetPropertyAt(int anIndex) const;
    virtual proProperty*    GetPropertyByName(const std::string &aName) const;
    virtual void            AddProperty(proProperty *property);
    virtual void            RemoveProperty(proProperty *) {}

    virtual errResult       Assign(const proObject* srcObj, proObject* destObj) const;

    // ONLY called during class registration in PRO_REGISTER_CLASS macro
    template <typename T> static void SetStaticClassPointer(proClass *newClass)
    {
        T::s_class = newClass;
    }

    virtual bool IsNative() const { return true; }

protected:

    PropertyList    m_properties;
    pConstructFn   *m_newObjFunc;

};



//
// registers a native C++ class with the property system
//
#define PRO_REGISTER_CLASS( aClass, aBaseClass ) \
proObject* create##aClass() { return new aClass(); } \
proClass *aClass::s_class = NULL;\
struct _register_class_##aClass \
{ \
    _register_class_##aClass() \
    { \
       proClassNative *theClass = new proClassNative( #aClass, (proClassNative::pConstructFn*)&create##aClass);\
       proClassRegistry::GetSingleton().RegisterClass(theClass); \
       proClassRegistry::GetSingleton().RegisterInheritance( #aClass, #aBaseClass);\
       proClassNative::SetStaticClassPointer<aClass>(theClass);\
    } \
}; \
_register_class_##aClass _init_##aClass; 

#if CORE_COMPILER_MSVC
#pragma deprecated ("REGISTER_CLASS")
#endif // CORE_COMPILER_MSVC
#define REGISTER_CLASS PRO_REGISTER_CLASS

//
// registers an abstract native C++ class with the property system
//
#define PRO_REGISTER_ABSTRACT_CLASS( aClass, aBaseClass ) \
proClass *aClass::s_class = NULL;\
struct _register_class_##aClass \
{ \
    _register_class_##aClass() \
    { \
       proClassNative *theClass = new proClassNative( #aClass, NULL);\
       proClassRegistry::GetSingleton().RegisterClass(theClass); \
       proClassRegistry::GetSingleton().RegisterInheritance( #aClass, #aBaseClass);\
       proClassNative::SetStaticClassPointer<aClass>(theClass);\
    } \
}; \
_register_class_##aClass _init_##aClass; 

#if CORE_COMPILER_MSVC
#pragma deprecated ("REGISTER_ABSTRACT_CLASS")
#endif // CORE_COMPILER_MSVC
#define REGISTER_ABSTRACT_CLASS PRO_REGISTER_ABSTRACT_CLASS

#endif //proClassNative
