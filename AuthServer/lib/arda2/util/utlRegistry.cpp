/**
 *  System registry manipulation helper classes
 * 
 *   Created: 2005/10/04
 * Copyright: 2005, NCSoft. All Rights Reserved
 *    Author: Todd Hayes
 *
 *  @par Last modified
 *      $DateTime: 2005/11/17 10:07:52 $
 *      $Author: cthurow $
 *      $Change: 178508 $
 *  @file
 *
 */

#include "arda2/core/corFirst.h"
#include "arda2/util/utlRegistry.h"

#if CORE_SYSTEM_WINAPI && !(CORE_SYSTEM_XENON)


/** 
 * Helper function for generating a null terminated string of spaces
 *
 * @param pStr Pointer to the string buffer to write into
 * @param indentLevel Number of spaces to place in the string
 *
 * @note Make sure the size of your buffer is greater than the indent level you pass
 */
void IndentString( LPTSTR pStr, int indentLevel)
{
    int index = 0;
    for (;index < indentLevel;++index)
    {
        pStr[index] = ' ';
    }
    // Terminate the string
    pStr[index] = 0;
}

//////////////////////////////////////////////////////////////////////////
// RegistryValue Implementations
//////////////////////////////////////////////////////////////////////////

/**
 * Destructor
 */
utlRegistryValue::~utlRegistryValue()
{
    // Note that the memory that is allocated for the name always comes
    // from _tcsdup, which requires the standard delete, not the array delete
    if (m_pName)
        delete m_pName;

    if (m_pValue)
        delete [] m_pValue;
}

/** 
 * Sets the name (key) of the registry entry
 *
 * @param pName Pointer to the TSTR representation of the key's name
 *
 * @note The registry value will contain a copy of the name, so the original should
 *       be managed by the caller and may be deleted after this function returns.
 */
void utlRegistryValue::SetName(const LPTSTR pName)
{
    // Delete any previously set name
    // Note that the memory that is allocated for the name always comes
    // from _tcsdup, which requires the standard delete, not the array delete
    if (m_pName)
        delete m_pName;

    if (pName)
        m_pName = _tcsdup(pName);
    else
        m_pName = NULL;
}

/** 
 * Sets the value of the registry entry
 *
 * @param name Pointer to the data to store in the value
 * @param size Size in bytes of the data pointe to by pValue
 *
 * @note The registry value will contain a copy of the data, so the original should
 *       be managed by the caller and may be deleted after this function returns.
 */
void utlRegistryValue::SetValue(const BYTE* pValue, DWORD size)
{
    if (m_pValue)
        delete [] m_pValue;

    if (pValue && (size > 0))
    {
        m_pValue = new BYTE[size];
        memcpy(m_pValue, pValue, size);
        m_valueSize = size;
    }
    else
    {
        m_pValue = NULL;
        m_valueSize = 0;
    }
}

/** 
 * Debugging aid to have the key name and associated value printed out at 
 * an appropriate indent level to present a reasonable treeview of the settings
 *
 * @param path Pointer to the string to represent the path of the key in the 
 *        registry.  This will be used to determine the indent depth of the key.
 */
void utlRegistryValue::Report(const LPTSTR pPath) const
{
    // Need a string to represent the default value if there's no name
    LPTSTR pValueName;
    if (m_pName)
        pValueName = m_pName;
    else
        pValueName = _T("(default)");

    // Indent the output string...
    int indentLevel = (int)(_tcslen(pPath));

    TCHAR outBuff[MAX_PATH];
    IndentString(outBuff, indentLevel);

    // ... and append the appropriate representation of the key/value pair
    switch (m_type)
    {
    case REG_SZ:
        _tcscat(outBuff, pValueName);
        _tcscat(outBuff, _T(" = "));
        _tcscat(outBuff, (const LPTSTR)m_pValue);
        _tcscat(outBuff, _T("\n"));
        break;
    case REG_NONE:
        _tcscat(outBuff, pValueName);
        _tcscat(outBuff, _T(" = <NONE>\n"));
        break;
    case REG_DWORD:
        _tcscat(outBuff, pValueName);
        _tcscat(outBuff, _T(" = "));
        _ltot(force_cast<DWORD>(m_pValue), outBuff + _tcslen(outBuff), 10);
        _tcscat(outBuff, _T("\n"));
        break;
    default:
        _tcscat(outBuff, pValueName);
        _tcscat(outBuff, _T(" = <Unsupported type = "));
        _ltot((DWORD)(m_type), outBuff + _tcslen(outBuff), 10);
        _tcscat(outBuff, _T(" >\n"));
        break;
    }
    corOutputDebugString(outBuff);
}

//////////////////////////////////////////////////////////////////////////
// RegistryKey Implementations
//////////////////////////////////////////////////////////////////////////

/** 
 * Constructor providing the name and default string value
 *
 * @param name Pointer to the name of the key
 * @param defaultValue Pointer to a string to use as the default value for the key
 */
utlRegistryKey::utlRegistryKey(const LPTSTR pName, const LPTSTR pDefaultValue)
{
    if (pName)
        m_pKeyName = _tcsdup(pName);
    else
        m_pKeyName = NULL;

    // Only string values can be used as default values with this constructor.
    if (pDefaultValue)
    {
        utlRegistryValue* pRegValue = NewValue();
        pRegValue->SetStringValue(pDefaultValue);
    }
}

/** 
 * Destructor
 */
utlRegistryKey::~utlRegistryKey() 
{ 
    // Note that the memory that is allocated for the name always comes
    // from _tcsdup, which requires the standard delete, not the array delete
    if (m_pKeyName) 
        delete m_pKeyName; 

    // Delete subkeys
    RegKeyVec::iterator keyIt = m_subKeys.begin();
    RegKeyVec::const_iterator keyEnd = m_subKeys.end();
    while (keyIt != keyEnd)
    {
        const utlRegistryKey* pKey = (*keyIt);
        delete pKey;
        ++keyIt;
    }

    // Delete subvalues
    RegValueVec::iterator valueIt = m_values.begin();
    RegValueVec::const_iterator valueEnd = m_values.end();
    while (valueIt != valueEnd)
    {
        const utlRegistryValue* pValue = (*valueIt);
        delete pValue;
        ++valueIt;
    }
}

/** 
 * Sets the name of the key
 *
 * @param name Pointer to the string representing the name of the registry key
 */
void utlRegistryKey::SetName(const LPTSTR pName)
{
    // Note that the memory that is allocated for the name always comes
    // from _tcsdup, which requires the standard delete, not the array delete
    if (m_pKeyName)
        delete m_pKeyName;

    if (pName)
        m_pKeyName = _tcsdup(pName);
    else
        m_pKeyName = NULL;
}

/** 
 * Recursively add the key and all of its children to the registry
 * as a subtree of the provided key
 *
 * @param parentHK Parent key in the registry to place this key under
 *
 * @return True if the key and its children could be added to the registry,
 *         false if an error occurred adding any of them to the registry.
 */
bool utlRegistryKey::AddToRegistry(HKEY parentHK) const
{
    HKEY hk;
    DWORD disposition;
    bool result = true;

    // Create the key itself as a child of the parent
    if (ERROR_SUCCESS != RegCreateKeyEx(parentHK, m_pKeyName, NULL, NULL, 
        REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hk, &disposition)) 
        return false;

    // Create all of the sub-keys
    RegKeyVec::const_iterator keyIt = m_subKeys.begin();
    RegKeyVec::const_iterator keyEnd = m_subKeys.end();
    while (keyIt != keyEnd)
    {
        const utlRegistryKey* pKey = (*keyIt);
        result = result && pKey->AddToRegistry(hk);
        ++keyIt;
    }

    // Create all of the values (including the default value)
    RegValueVec::const_iterator valueIt = m_values.begin();
    RegValueVec::const_iterator valueEnd = m_values.end();
    while (valueIt != valueEnd)
    {
        const utlRegistryValue* pValue = (*valueIt);
        result = result && pValue->AddToRegistry(hk);
        ++valueIt;
    }

    // Close this key before returning back up the stack
    if (ERROR_SUCCESS != RegCloseKey(hk))   
        return false;

    return result;
}

/** 
 * Recursively remove the key and all of its children from the registry
 *
 * @param parentHK Parent key in the registry that this key is under
 *
 * @return True if the key and its children could be removed from the registry,
 *         false if an error occurred removing any of them from the registry. 
 */
bool utlRegistryKey::RemoveFromRegistry(HKEY parentHK) const
{
    if ((m_subKeys.size() + m_values.size()) > 0)
    {
        HKEY hk;
        bool result = true;

        // Create the key itself as a child of the parent
        if (ERROR_SUCCESS != RegOpenKeyEx(parentHK, m_pKeyName, 0, KEY_ALL_ACCESS, &hk)) 
            return false;

        // Recursively remove all sub keys
        RegKeyVec::const_iterator keyIt = m_subKeys.begin();
        RegKeyVec::const_iterator keyEnd = m_subKeys.end();
        while (keyIt != keyEnd)
        {
            const utlRegistryKey* pKey = (*keyIt);
            result = result && pKey->RemoveFromRegistry(hk);
            ++keyIt;
        }

        // remove all values (including the default value)
        RegValueVec::const_iterator valueIt = m_values.begin();
        RegValueVec::const_iterator valueEnd = m_values.end();
        while (valueIt != valueEnd)
        {
            const utlRegistryValue* pValue = (*valueIt);
            result = result && pValue->RemoveFromRegistry(hk);
            ++valueIt;
        }
        RegCloseKey(hk);
    }
    return (RegDeleteKey(parentHK, m_pKeyName) == ERROR_SUCCESS);
}

/** 
 * Adds a new key as a child of this key
 *
 * @param pName Pointer to the name to use for the new key
 * @param pDefaultValue Pointer to a string to use as the default value of the new key
 * 
 * @return Pointer to the new key
 */
utlRegistryKey* utlRegistryKey::NewKey(const LPTSTR pName, const LPTSTR pDefaultValue) 
{ 
    utlRegistryKey* pNewKey = new utlRegistryKey( pName, pDefaultValue);
    m_subKeys.push_back(pNewKey);
    return pNewKey;
}

/** 
 * Retrieves the child key with the given name
 *
 * @param pKeyName Pointer to the string representing the name of the key to 
 *                 retrieve
 *
 * @return Pointer to the key of the given name if one exists, NULL if no child
 *         key of that name exists
 */
utlRegistryKey* utlRegistryKey::GetKey(const LPTSTR pKeyName)
{
    RegKeyVec::iterator keyIt = m_subKeys.begin();
    RegKeyVec::const_iterator keyEnd = m_subKeys.end();
    while (keyIt != keyEnd)
    {
        const utlRegistryKey* pKey = (*keyIt);
        if (_tcsicmp(pKey->GetName(), pKeyName) == 0)
        {
            // Found it
            return (*keyIt);
        }
        ++keyIt;
    }
    return NULL;
}

/** 
 * Create a new registry value as a child of this key
 *
 * @return Pointer to the new, uninitialized value
 */
utlRegistryValue* utlRegistryKey::NewValue() 
{ 
    utlRegistryValue* pNewValue = new utlRegistryValue;
    m_values.push_back(pNewValue);
    return pNewValue;
}

/**
 * Retrieve the value with the given name
 * 
 * @param pValueName Pointer to the string representing the name of the value
 *
 * @return Pointer to the value with the given name, NULL if no child value
 *         with the given name could be found in this key
 */
utlRegistryValue* utlRegistryKey::GetValue(const LPTSTR pValueName)
{
    RegValueVec::iterator valueIt = m_values.begin();
    RegValueVec::const_iterator valueEnd = m_values.end();
    while (valueIt != valueEnd)
    {
        utlRegistryValue* pValue = (*valueIt);
        if (_tcsicmp(pValue->GetName(), pValueName) == 0)
        {
            // Found it
            return (*valueIt);
        }
        ++valueIt;
    }
    return NULL;
}

/** 
 * Debugging aid to have the key name and associated value printed out at 
 * an appropriate indent level to present a reasonable treeview of the settings
 *
 * @param parentKey System wide key that acts as the parent to the key
 */
void utlRegistryKey::Report(HKEY parentKey) const
{
    LPTSTR pPath;
    if (parentKey == HKEY_CLASSES_ROOT)
    {
        pPath = _T("HKEY_CLASSES_ROOT");
    }
    else if (parentKey == HKEY_CURRENT_CONFIG)
    {
        pPath = _T("HKEY_CURRENT_CONFIG");
    }
    else if (parentKey == HKEY_CURRENT_USER)
    {
        pPath = _T("HKEY_CURRENT_USER");
    }
    else if (parentKey == HKEY_DYN_DATA)
    {
        pPath = _T("HKEY_DYN_DATA");
    }
    else if (parentKey == HKEY_LOCAL_MACHINE)
    {
        pPath = _T("HKEY_LOCAL_MACHINE");
    }
    else if (parentKey == HKEY_PERFORMANCE_DATA)
    {
        pPath = _T("HKEY_PERFORMANCE_DATA");
    }
    else if (parentKey == HKEY_PERFORMANCE_NLSTEXT)
    {
        pPath = _T("HKEY_PERFORMANCE_NLSTEXT");
    }
    else if (parentKey == HKEY_PERFORMANCE_TEXT)
    {
        pPath = _T("HKEY_PERFORMANCE_TEXT");
    }
    else if (parentKey == HKEY_USERS)
    {
        pPath = _T("HKEY_USERS");
    }
    else
    {
        pPath = _T("\\\\");
    }

    TCHAR outBuff[MAX_PATH];
    outBuff[0] = 0;
    _tcscat(outBuff, pPath);
    _tcscat(outBuff, _T("\n"));

    corOutputDebugString(outBuff);

    Report(pPath);
}

/** 
 * Debugging aid to have the key name and associated value printed out at 
 * an appropriate indent level to present a reasonable treeview of the settings
 *
 * @param pPath Pointer to the string to represent the path of the key in the 
 *        registry.  This will be used to determine the indent depth of the key.
 */
void utlRegistryKey::Report(const LPTSTR pPath) const
{
    // Print out the indented name of this key
    TCHAR outBuff[MAX_PATH];
    IndentString(outBuff, (int)(_tcslen(pPath)));
    _tcscat(outBuff, m_pKeyName);
    _tcscat(outBuff, _T("\n"));
    corOutputDebugString(outBuff);

    // Build up the full path to pass on (is a TCHAR version of sprintf too much to ask?)
    TCHAR fullPath[MAX_PATH];
    fullPath[0] = 0;
    _tcscat(fullPath, pPath);
    _tcscat(fullPath, _T("\\"));
    _tcscat(fullPath, m_pKeyName);

    // Print out sub keys
    RegKeyVec::const_iterator keyIt = m_subKeys.begin();
    RegKeyVec::const_iterator keyEnd = m_subKeys.end();
    while (keyIt != keyEnd)
    {
        const utlRegistryKey* pKey = (*keyIt);
        pKey->Report(fullPath);
        ++keyIt;
    }

    // Print out values
    RegValueVec::const_iterator valueIt = m_values.begin();
    RegValueVec::const_iterator valueEnd = m_values.end();
    while (valueIt != valueEnd)
    {
        const utlRegistryValue* pValue = (*valueIt);
        pValue->Report(fullPath);
        ++valueIt;
    }
}

/** 
 * Quick little registration test to verify functionality
 */
void TestUtlRegistration()
{
    utlRegistryKey testKey;
    testKey.SetName(_T("NCTest"));

    utlRegistryValue* defaultTestValue = testKey.NewValue();
    defaultTestValue->SetType(REG_SZ);
    defaultTestValue->SetStringValue(_T("Test Key Default Value"));

    utlRegistryValue* testValue = testKey.NewValue();
    testValue->SetType(REG_SZ);
    testValue->SetName(_T("NamedValue"));
    testValue->SetStringValue(_T("Test Key Named Value"));


    utlRegistryKey* testSubKey = testKey.NewKey();
    testSubKey->SetName(_T("TestSubKey"));

    utlRegistryValue* defaultSubTestValue = testSubKey->NewValue();
    defaultSubTestValue->SetType(REG_SZ);
    defaultSubTestValue->SetStringValue(_T("Test Sub Key Default Value"));

    utlRegistryValue* testSubValue = testSubKey->NewValue();
    testSubValue->SetType(REG_SZ);
    testSubValue->SetName(_T("SubKeyNamedValue"));
    testSubValue->SetStringValue(_T("Test Sub Key Named Value"));

    testKey.Report(HKEY_CLASSES_ROOT);
    testKey.AddToRegistry(HKEY_CLASSES_ROOT);
    testKey.RemoveFromRegistry(HKEY_CLASSES_ROOT);
}

#endif
