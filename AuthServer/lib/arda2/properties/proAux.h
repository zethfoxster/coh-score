
#ifndef   INCLUDED_proAux_h
#define   INCLUDED_proAux_h
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "proFirst.h"

#if BUILD_PROPERTY_SYSTEM

enum
{
    DATATYPE_VOID   =0,
    DATATYPE_INT    =1,
    DATATYPE_UINT   =2,
    DATATYPE_FLOAT  =3,
    DATATYPE_BOOL   =4,
    DATATYPE_STRING =5,
    DATATYPE_ENUM   =6,
    DATATYPE_OBJECT =7,
    DATATYPE_DYNAMIC_OBJECT= 8,
    DATATYPE_WSTRING=10,
    DATATYPE_CLASS=11,
    DATATYPE_DYNAMIC_CLASS=12,
    DATATYPE_PANDORA_BEGIN=20000,
    DATATYPE_AUTOASSAULT_BEGIN=40000,
    DATATYPE_MISCPROJECT_BEGIN=60000,
    DATATYPE_CORE_BEGIN= 100000
};

inline int32 GetClassType()
{
    return DATATYPE_CLASS;
}

inline int32 GetDynamicClassType()
{
    return DATATYPE_DYNAMIC_CLASS;
}


inline int32 GetVoidType()
{
    return DATATYPE_VOID;
}

inline int32 GetIntType()
{
    return DATATYPE_INT;
}

inline int32 GetUintType()
{
    return DATATYPE_UINT;
}

inline int32 GetFloatType()
{
    return DATATYPE_FLOAT;
}

inline int32 GetBoolType()
{
    return DATATYPE_BOOL;
}

inline int32 GetStringType()
{
    return DATATYPE_STRING;
}

inline int32 GetEnumType()
{
    return DATATYPE_ENUM;
}

inline int32 GetObjectType()
{
    return DATATYPE_OBJECT;
}

inline int32 GetWStringType()
{
    return DATATYPE_WSTRING;
}

#endif // #if BUILD_PROPERTY_SYSTEM

#endif // INCLUDED_proAux_h

