/**
 *  System registry manipulation helper classes
 * 
 *   Created: 2005/10/04
 * Copyright: 2005, NCSoft. All Rights Reserved
 *    Author: Todd Hayes
 *
 *  @par Last modified
 *      $DateTime: 2007/12/12 10:43:51 $
 *      $Author: pflesher $
 *      $Change: 700168 $
 *  @file
 *
 */

#ifndef INCLUDED_UTLREGISTRY_H
#define INCLUDED_UTLREGISTRY_H

#if CORE_SYSTEM_WINAPI && !(CORE_SYSTEM_XENON)

#include "arda2/core/corStlVector.h"
#include <tchar.h>


class utlRegistryValue
{
public:
    utlRegistryValue() : m_pName(NULL), m_type(0), m_pValue(NULL), m_valueSize(0) {}
    ~utlRegistryValue();
    void SetName(const LPTSTR pName);
    const LPTSTR GetName() const { return m_pName; }

    void SetType(DWORD type) { m_type = type; }
    DWORD GetType() const { return m_type; }

    void SetValue(const BYTE* pValue, DWORD size);
    const BYTE* GetValue() const { return m_pValue; }
    DWORD GetValueSize() const { return m_valueSize; }

    void SetStringValue(LPTSTR pStrValue)
    { 
        SetValue((BYTE*)pStrValue, (DWORD)((_tcslen(pStrValue)+1)*sizeof(TCHAR))); SetType(REG_SZ);
    }

    bool AddToRegistry(HKEY parentKey) const
    { 
        return (ERROR_SUCCESS == RegSetValueEx(parentKey, m_pName, 0, m_type, m_pValue, m_valueSize)); 
    }

    bool RemoveFromRegistry(HKEY parentKey) const
    { 
        return (ERROR_SUCCESS == RegDeleteKey(parentKey, m_pName)); 
    }

    // Debugging
    void Report(const LPTSTR pPath) const;

protected:
    LPTSTR  m_pName;
    DWORD   m_type;
    BYTE*   m_pValue;
    DWORD   m_valueSize;
};

class utlRegistryKey
{
public:
    utlRegistryKey(const LPTSTR pName = NULL, const LPTSTR defaultValue = NULL);
    ~utlRegistryKey();

    void SetName(const LPTSTR pName);
    const LPTSTR GetName() const { return m_pKeyName; }

    bool AddToRegistry(HKEY parentHK) const;
    bool RemoveFromRegistry(HKEY parentHK) const;

    utlRegistryKey* NewKey(const LPTSTR pName = NULL, const LPTSTR pDefaultValue = NULL);
    utlRegistryKey* GetKey(const LPTSTR pKeyName);

    utlRegistryValue* NewValue();
    utlRegistryValue* GetValue(const LPTSTR pValueName);

    // Debugging
    void Report(HKEY parentKey) const;
    void Report(const LPTSTR pPath) const;

protected:
    typedef std::vector<utlRegistryKey*> RegKeyVec;
    typedef std::vector<utlRegistryValue*> RegValueVec;

    LPTSTR      m_pKeyName;
    RegKeyVec   m_subKeys;
    RegValueVec m_values;
};

// Quick Test
void TestUtlRegistration();

#endif

#endif //INCLUDED_UTLREGISTRY_H



