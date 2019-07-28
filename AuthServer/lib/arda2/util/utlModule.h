/*
 *  @par Last modified
 *      $DateTime: 2007/12/12 10:43:51 $
 *      $Author: pflesher $
 *      $Change: 700168 $
 *  @file
 */

#if CORE_SYSTEM_PS3
// Not available on PS3
#define   INCLUDED_utlModule_h
#endif

#ifndef   INCLUDED_utlModule_h
#define   INCLUDED_utlModule_h
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "arda2/core/corFirst.h"
#include "arda2/core/corStdString.h"


/**
 * encapsulate dynamic modules in various environments (dll/so)
**/
class utlModule
{
public:
    utlModule();
    utlModule( const utlModule& module );
    utlModule& operator=( const utlModule& module );
    virtual ~utlModule();

    bool                        operator== ( const utlModule& module );
    bool                        BindModule(const char* moduleName);
    const char*                 GetModuleName() const;
    
    void*                       GetFuncAddressVoid(const char* funcName);

    // experimental..
    template <typename T> T     GetFuncAddress(const char* funcName)
    {
        return (T)GetFuncAddressVoid(funcName);
    }

private:

#if CORE_SYSTEM_WINAPI
        typedef HMODULE ModuleHandleType;
#else
        typedef void*   ModuleHandleType;
#endif

    ModuleHandleType m_handle;
    std::string           m_moduleName;
};


#endif // INCLUDED_utlModule_h
