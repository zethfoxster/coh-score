/**
    purpose:    Allows virtual unserialization of derived classes. This is 
                intended to be used as a mix-in template class. Each derived 
                class must supply a kTag enum which maps to a TAG4 for chunk
                serialization. They must also call Register<DerivedType> on
                the base class. For example usage, see the unit test.
    author(s):  Peter Freese
    copyright:  (c) 2003, NCsoft. All Rights Reserved
    modified:   $DateTime: 2008/01/15 15:13:29 $
                $Author: mbreitkreutz $
                $Change: 711839 $
                @file
**/

#ifndef   INCLUDED_stoAbstractUnserializationFactory
#define   INCLUDED_stoAbstractUnserializationFactory
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "arda2/core/corStlHashmap.h"

class stoChunkFileReader;

template<class T>
class stoAbstractUnserializationFactory
{
public:
    virtual ~stoAbstractUnserializationFactory() {};

    typedef T* (*CreationFunc)();

    static void RegisterUnserializationTag( TAG4 tag, CreationFunc func )
    {
        if ( GetMap().find(tag) != GetMap().end() )
            ERR_REPORT(ES_Warning, "Inserting duplicate creator tag to factory");

        GetMap()[tag] = func;
    }

    template <class U>
    static void RegisterForUnserialization()
    {
        RegisterUnserializationTag((TAG4)U::kTag, Create<U>);
    }

    template <class U>
    DEPRECATED static void Register()
    {
        RegisterUnserializationTag((TAG4)U::kTag, Create<U>);
    }

    template <class U>
    static T * Create()
    {
        return new U;
    }

    static T* CreateFromTag( TAG4 tag )
    {
        typename CreatorMap::iterator i = GetMap().find(tag);
        return i != GetMap().end() ? i->second() : 0;
    }

    static T* AbstractCreate(stoChunkFileReader &reader)
    {
        TAG4 tag = reader.PeekNextChunkTag();
        return CreateFromTag(tag);
    }

    static errResult AbstractUnserialize( stoChunkFileReader &reader, T*& pT )
    {
        TAG4 tag = reader.PeekNextChunkTag();
        pT = CreateFromTag(tag);
        return pT ? pT->Unserialize(reader) : ER_Failure;
    }

    virtual errResult Unserialize( stoChunkFileReader &reader ) = 0;

private:
    typedef HashMap<TAG4, CreationFunc> CreatorMap;

    static CreatorMap& GetMap()
    {
        static CreatorMap map;
        return map;
    }
};


#endif // INCLUDED_stoAbstractUnserializationFactory

