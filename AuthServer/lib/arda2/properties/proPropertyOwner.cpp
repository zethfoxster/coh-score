#include "arda2/core/corFirst.h"

#include "proFirst.h"


#include "arda2/util/utlTokenizer.h"

#include "arda2/properties/proPropertyOwner.h"
#include "arda2/properties/proClassOwner.h"

#include "arda2/properties/proObjectWriter.h"
#include "arda2/properties/proObjectReader.h"

using namespace std;

/********************* int Implementation **************/

PRO_REGISTER_CLASS(proPropertyIntOwner, proPropertyInt)

proDataType proPropertyIntOwner::GetDataType() const
{
    return DT_int;
}

/********************* uint Implementation **************/

PRO_REGISTER_CLASS(proPropertyUintOwner, proPropertyUint)

proDataType proPropertyUintOwner::GetDataType() const
{
    return DT_uint;
}

/********************* float Implementation **************/

PRO_REGISTER_CLASS(proPropertyFloatOwner, proPropertyFloat)

proDataType proPropertyFloatOwner::GetDataType() const
{
    return DT_float;
}

/********************* bool Implementation **************/

PRO_REGISTER_CLASS(proPropertyBoolOwner, proPropertyBool)

proDataType proPropertyBoolOwner::GetDataType() const
{
    return DT_bool;
}

/********************* string Implementation **************/

PRO_REGISTER_CLASS(proPropertyStringOwner, proPropertyString)

proDataType proPropertyStringOwner::GetDataType() const
{
    return DT_string;
}

/********************* std::wstring Implementation **************/

PRO_REGISTER_CLASS(proPropertyWstringOwner, proPropertyWstring)

proDataType proPropertyWstringOwner::GetDataType() const
{
    return DT_string;
}

/********************* object Implementation **************/

PRO_REGISTER_CLASS(proPropertyObjectOwner, proPropertyObject)

proDataType proPropertyObjectOwner::GetDataType() const
{
    return DT_object;
}

string proPropertyObjectOwner::ToString( const proObject* o ) const 
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

// read the object from a reader (unserialize)
void proPropertyObjectOwner::Read(proObjectReader* in, proObject* obj) 
{
    DP_OWN_VALUE owner;
    proObject* o = NULL;
    in->GetPropValue(GetName(), o, owner);
    SetValue(obj, o);
    SetOwner(obj, owner);
}

/**
 * clone an object property - includes cloning children...
 */
void proPropertyObjectOwner::Clone(const proObject* source, proObject* dest)
{
    if (dest->GetPropertyByName(GetName()) )
    {
        ERR_REPORTV(ES_Warning, ("property name collision when attempting to clone a child"));
        return;
    }

    proPropertyObjectOwner *newProp = (proPropertyObjectOwner *)GetClass()->NewInstance();

    proObject *oldValue = GetValue(source);
    proObject *newValue = oldValue->Clone();

    newProp->SetName(GetName());
    dest->AddInstanceProperty(newProp);
    newProp->SetValue(dest, newValue);
    newProp->SetOwner(dest, GetOwner(source));

}

/********************* enum Implementation **************/


PRO_REGISTER_CLASS(proPropertyEnumOwner, proPropertyEnum)

proDataType proPropertyEnumOwner::GetDataType() const
{
    return DT_enum;
}

/********************* bitfield Implementation **************/

PRO_REGISTER_CLASS(proPropertyBitfieldOwner, proPropertyBitfield)

proDataType proPropertyBitfieldOwner::GetDataType() const
{
    return DT_bitfield;
}

void ImportOwnerProperties()
{
IMPORT_PROPERTY_CLASS(proPropertyIntOwner);
}
