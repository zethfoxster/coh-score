/**
    copyright:  (c) 2003, NCsoft. All Rights Reserved
    author(s):  Peter Freese
    purpose:    Map global strings to dynamic identifiers for fast comparison.
    
                This is designed with the following performance requirements:
                    (1) Id comparison speed is of primary importance
                    (2) String to id conversion is of secondary importance
                    (3) Id to string conversion exists for debugging and 
                        serialization, so not a perf concern

                The class implementation is case-insensitive but case-preserving.
                At some point in the future, we might want to include a template 
                parameter to specify the behavior.

                Anticipated usage:

                    class SomeClass
                    {
                        ...

                    private:
                        static utlStringTable s_stringTable;
                    }

                or use the global string table g_stringTable;

    modified:   $DateTime: 2007/12/12 10:43:51 $
                $Author: pflesher $
                $Change: 700168 $
                @file
**/

#ifndef   INCLUDED_utlStringTable
#define   INCLUDED_utlStringTable
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "arda2/core/corStlVector.h"
#include "arda2/core/corStlHashTraits.h"
#include "arda2/util/utlStringHeap.h"

class utlStringId
{
public:
    utlStringId() : value(NULL) {};

    bool operator==( const utlStringId &rhs ) const
    {
        return value == rhs.value;
    }
    bool operator!=( const utlStringId &rhs ) const
    {
        return value != rhs.value;
    }
    bool operator<( const utlStringId &rhs ) const
    {
        return value < rhs.value;
    }

    bool IsValid() const
    {
        return value != NULL;
    }

    static const utlStringId ID_NONE;

private:
    // default copy constructor and assignment okay
    explicit utlStringId( const char *pString ) : value(pString) {};

    const char *value;
    friend class utlStringTable;
    friend struct utlStringIdHashFunc;
};

// hash functor
struct utlStringIdHashFunc
{
    inline size_t operator()(const utlStringId& id) const
    {
        return reinterpret_cast<size_t>(id.value);
    }
};

typedef corHashTraits<utlStringId, utlStringIdHashFunc> utlStringIdHashTraits;

class utlStringTable
{
public:
    utlStringTable();
    ~utlStringTable();

    utlStringId AddString( const char *pString );
    utlStringId GetId( const char *pString ) const;
    const char* GetString( utlStringId id ) const
    {
        if ( id.IsValid() )
            return id.value;
        else
            return s_nullString;
    }
    void Clear();

protected:
    
private:
    typedef std::vector<const char *> StringVec;

    bool LookupString( IN const char* pString, OUT StringVec::iterator& it ) const;

    StringVec       m_strings;
    static char *   s_nullString;

utlStringHeap       m_heap;
};

extern utlStringTable g_stringTable;

#endif // INCLUDED_utlStringTable
