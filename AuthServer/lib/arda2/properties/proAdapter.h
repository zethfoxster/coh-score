#ifndef _cpAdapter_INCLUDED_
#define _cpAdapter_INCLUDED_

#include "proFirst.h"

#include "arda2/core/corStdString.h"
#include "arda2/properties/proObject.h"
#include "arda2/properties/proProperty.h"
#include "arda2/properties/proClassRegistry.h"
#include "arda2/properties/proClassNative.h"
#include "arda2/properties/proStringUtils.h"

/**
 * shared adapter base class to allow clients to call Initialize on all adapters
 *
 * @note all implementations of adapters should handle multiple calls to Initialize
 *
**/
class proAdapterShared : public proObject
{
    PRO_DECLARE_OBJECT
public:

    virtual ~proAdapterShared(){}

    virtual void Initialize() = 0;
    virtual void FullInitialize() = 0;
    virtual bool IsInitialized() = 0;

    /**
     * Cloning adapters cannot be implemented in a general way. The adapter
     * does not know how to construct the adapted target so cannot clone it.
     * The normal property assignment that occurs in proObject::Clone cannot
     * be done as the newly cloned adapter does not have a valid target.
     *
     * If your adapter can clone itself correctly, override this method.
     */
    virtual proObject *Clone() const
    {
        return GetClass()->NewInstance();
    }

};

enum eAdapterInit
{
    PA_LAZY,
    PA_INIT,
    PA_FULL_INIT //< cyclic structures will run forever on this setting
};

enum eAdapterOwn
{
    PA_OWN,
    PA_REF
};

/**
 * property adapter template class. Use this to expose
 * existing classes to the property system.
 *
 * @param TARGET_CLASS the class that is being exposed to the property system
**/
template <typename TARGET_CLASS> 
class proAdapter : public proAdapterShared
{
public:
    proAdapter() : m_target(NULL), m_owner(false), m_init(false){}
    virtual ~proAdapter();
    typedef proAdapter<TARGET_CLASS> MyType;
    virtual void Initialize()   { m_init = true; }
    virtual bool IsInitialized()        { return m_init; }

    virtual void FullInitialize();

    virtual void SetTarget(TARGET_CLASS *target, eAdapterInit init = PA_FULL_INIT, eAdapterOwn owner = PA_REF);
    
    virtual TARGET_CLASS *GetTarget() const        { return m_target; }

protected:


    TARGET_CLASS *m_target;
    bool m_owner;
    bool m_init;

};


template <typename TARGET_CLASS> 
proAdapter<TARGET_CLASS>::~proAdapter()
{
    if (m_owner)
        SafeDelete(m_target);
}

template <typename TARGET_CLASS> 
void proAdapter<TARGET_CLASS>::FullInitialize()
{
    Initialize();
    
    proClass *cls = GetClass();
    for( int i = 0; i < cls->GetPropertyCount(); ++i )
    {
        proProperty *p = cls->GetPropertyAt(i);

        if ( p->GetDataType() == DT_object )
        {
            proObject* po = proConvUtils::AsObject(this, p);
            if( po && proClassRegistry::InstanceOf(po->GetClass(), "proAdapterShared") )
            {
                proAdapterShared * as = verify_cast<proAdapterShared*>( po );
                as->FullInitialize();
            }
        }
    }
}
template <typename TARGET_CLASS> 
void proAdapter<TARGET_CLASS>::SetTarget(TARGET_CLASS *target, eAdapterInit init, eAdapterOwn owner )   
{ 
    m_target = target; 

    if ( owner == PA_OWN )
        m_owner = true;
    else
        m_owner = false;

    if ( init == PA_INIT ) 
        Initialize(); 
    else if( init == PA_FULL_INIT )
        FullInitialize();
}

//-----------------------------------------------------------------------------

template <typename ADAPTER_CLASS> 
proObject* createAdapterClass() { return new ADAPTER_CLASS; }

//-----------------------------------------------------------------------------

#define PRO_REGISTER_ADAPTER_CLASS( adapterClass, targetClass ) \
struct _register_class_proAdapter_##adapterClass##_##targetClass \
{ \
    _register_class_proAdapter_##adapterClass##_##targetClass() \
    { \
        proClassNative *theClass = new proClassNative( "proAdapter<"#targetClass">", NULL);\
        proClassRegistry::GetSingleton().RegisterClass(theClass); \
        proClassRegistry::GetSingleton().RegisterInheritance( "proAdapter<"#targetClass">", "proAdapterShared");\
    } \
}; \
    _register_class_proAdapter_##adapterClass##_##targetClass _init_proAdapter_##adapterClass##_##targetClass; \
proClass* adapterClass::s_class = NULL;\
struct _register_adapter_class_##adapterClass \
{ \
    _register_adapter_class_##adapterClass() \
    { \
        proClassNative *theClass = new proClassNative(#adapterClass, (proClassNative::pConstructFn*)&createAdapterClass<adapterClass>);\
        proClassRegistry::RegisterClass(theClass);\
        proClassRegistry::RegisterInheritance( #adapterClass, "proAdapter<"#targetClass">"); \
        proClassNative::SetStaticClassPointer< adapterClass >(theClass);\
    } \
}; \
_register_adapter_class_##adapterClass _init_adapter_class_##adapterClass;\
void *_symbol_adapter_class_##adapterClass = &_init_adapter_class_##adapterClass;

#if CORE_COMPILER_MSVC
#pragma deprecated ("REGISTER_ADAPTER_CLASS")
#endif //CORE_COMPILER_MSVC
#define REGISTER_ADAPTER_CLASS PRO_REGISTER_ADAPTER_CLASS

//-----------------------------------------------------------------------------

#define PRO_IMPORT_ADAPTER_CLASS(classname) \
    extern void * _symbol_adapter_class_##classname; \
    void *___init_##classname = &_symbol_adapter_class_##classname;\
    ___init_##classname = NULL;

#if CORE_COMPILER_MSVC
#pragma deprecated ("IMPORT_ADAPTER_CLASS")
#endif // CORE_COMPILER_MSVC
#define IMPORT_ADAPTER_CLASS PRO_IMPORT_ADAPTER_CLASS

#endif
