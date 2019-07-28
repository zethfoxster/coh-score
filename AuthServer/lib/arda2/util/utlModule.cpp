
#include "arda2/core/corFirst.h"
#include "arda2/util/utlModule.h"

#if !CORE_SYSTEM_PS3
// Not available on PS3

#if CORE_SYSTEM_WINAPI
#else
#include <dlfcn.h>
#endif


utlModule::utlModule()
{
    m_handle = NULL;
}

utlModule::~utlModule()
{
    if(m_handle)
    {
#if CORE_SYSTEM_WINAPI
        FreeLibrary( m_handle );
        m_handle = NULL;
#else
        dlclose( m_handle );
        m_handle = NULL;
#endif
    }
}

utlModule::utlModule( const utlModule& module )
{
    // hopefully this will cause the OS to increment some kind
    // of reference count on the module??
    BindModule( module.GetModuleName() );
}

utlModule& utlModule::operator=( const utlModule& module )
{
    // hopefully this will cause the OS to increment some kind
    // of reference count on the module??
    BindModule( module.GetModuleName() );   
    return *this;
}

bool utlModule::operator==( const utlModule& module )
{
    if ( strcmp(GetModuleName(), module.GetModuleName()) == 0 &&
         m_handle == module.m_handle)
    {
        return true;
    }
    return false;
}


bool utlModule::BindModule(const char* moduleName)
{
#if CORE_SYSTEM_WINAPI
    if ( moduleName == NULL || strcmp(moduleName, "\0") == 0 )
    {
        m_handle = GetModuleHandle( NULL );
    }
    else
    {
        m_handle = LoadLibrary( moduleName );
    }

#else
    if ( moduleName == NULL || strcmp(moduleName, "\0") == 0 )
    {
        errAssert(0); // get handle to "main" module make sure this works on linux
        //m_handle = dlopen( NULL );
    }
    else
    {
        m_handle = dlopen( moduleName, RTLD_NOW );
    }
#endif 

    if(m_handle != NULL)
    {
        if( moduleName == NULL )
            m_moduleName = "\0";
        else
            m_moduleName = moduleName;
        return true;
    }
    return false;
}

const char* utlModule::GetModuleName() const
{
    if(m_moduleName.size() == 0)
        return NULL;
    return m_moduleName.c_str();
}


void* utlModule::GetFuncAddressVoid(const char* funcName)
{
    void* funAddy = NULL;

#if CORE_SYSTEM_WINAPI
        funAddy = GetProcAddress( m_handle, funcName );
#else
        funAddy = dlsym(m_handle, funcName);
#endif 

    return funAddy;
}


#endif // !CORE_SYSTEM_PS3

