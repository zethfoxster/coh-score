#include "arda2/core/corFirst.h"

#include "proFirst.h"

#include "arda2/util/utlTokenizer.h"

#include "arda2/properties/proPropertyNative.h"
#include "arda2/properties/proClassNative.h"

#include "arda2/properties/proObjectWriter.h"
#include "arda2/properties/proObjectReader.h"

using namespace std;

/********************* int Implementation **************/

PRO_REGISTER_ABSTRACT_CLASS(proPropertyIntNative, proPropertyInt)

proDataType proPropertyIntNative::GetDataType() const
{
    return DT_int;
}

void proPropertyIntNative::Clone( const proObject* source, proObject* dest )
{
    SetValue( dest, GetValue( source ) );
}


/********************* uint Implementation **************/

PRO_REGISTER_ABSTRACT_CLASS(proPropertyUintNative, proPropertyUint)

proDataType proPropertyUintNative::GetDataType() const
{
    return DT_uint;
}

void proPropertyUintNative::Clone( const proObject* source, proObject* dest )
{
    SetValue( dest, GetValue( source ) );
}


/********************* float Implementation **************/

PRO_REGISTER_ABSTRACT_CLASS(proPropertyFloatNative, proPropertyFloat)

proDataType proPropertyFloatNative::GetDataType() const
{
    return DT_float;
}

void proPropertyFloatNative::Clone( const proObject* source, proObject* dest )
{
    SetValue( dest, GetValue( source ) );
}


/********************* bool Implementation **************/

PRO_REGISTER_ABSTRACT_CLASS(proPropertyBoolNative, proPropertyBool)

proDataType proPropertyBoolNative::GetDataType() const
{
    return DT_bool;
}

void proPropertyBoolNative::Clone( const proObject* source, proObject* dest )
{
    SetValue( dest, GetValue( source ) );
}


/********************* string Implementation **************/

PRO_REGISTER_ABSTRACT_CLASS(proPropertyStringNative, proPropertyString)

proDataType proPropertyStringNative::GetDataType() const
{
    return DT_string;
}

void proPropertyStringNative::Clone( const proObject* source, proObject* dest )
{
    SetValue( dest, GetValue( source ) );
}


/********************* wstring Implementation **************/

PRO_REGISTER_ABSTRACT_CLASS(proPropertyWstringNative, proPropertyWstring)

proDataType proPropertyWstringNative::GetDataType() const
{
    return DT_wstring;
}

void proPropertyWstringNative::Clone( const proObject* source, proObject* dest )
{
    SetValue( dest, GetValue( source ) );
}


/********************* object Implementation **************/

PRO_REGISTER_ABSTRACT_CLASS(proPropertyObjectNative, proPropertyObject)

proDataType proPropertyObjectNative::GetDataType() const
{
    return DT_object;
}

void proPropertyObjectNative::Clone( const proObject* source, proObject* dest )
{
    proProperty *targetProp = dest->GetPropertyByName(GetName());
    if (!targetProp)
    {
        ERR_REPORTV(ES_Warning, ("no native proprety to clone to"));
        return;
    }
    proObject *oldValue = GetValue(source);
    proObject *newValue = NULL;
    if (oldValue)
    {
        newValue = oldValue->Clone();
    }
    SetValue( dest, newValue );
}

string proPropertyObjectNative::ToString( const proObject* o ) const 
{ 
    proObject *value = GetValue(o);
    if (value)
    {
        return value->GetClass()->GetName(); 
    }
    else
    {
        return "null";
    }
}

/********************* enum Implementation **************/

PRO_REGISTER_ABSTRACT_CLASS(proPropertyEnumNative, proPropertyEnum)

proDataType proPropertyEnumNative::GetDataType() const
{
    return DT_enum;
}

void proPropertyEnumNative::Clone( const proObject* source, proObject* dest )
{
    SetValue( dest, GetValue( source ) );
}



/********************* bitfield Implementation **************/

PRO_REGISTER_ABSTRACT_CLASS(proPropertyBitfieldNative, proPropertyBitfield)

proDataType proPropertyBitfieldNative::GetDataType() const
{
    return DT_bitfield;
}

void proPropertyBitfieldNative::Clone( const proObject* source, proObject* dest )
{
    SetValue( dest, GetValue( source ) );
}

