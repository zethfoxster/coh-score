//-----------------------------------------------------------------------------
/**
 *  property validation
 *
 *  @author Allen Jackson
 *
 *  @date 1/9/2006
 *
 *  @file
**/
//-----------------------------------------------------------------------------

#include "arda2/core/corFirst.h"
#include "proValidator.h"
#include "arda2/properties/proClassNative.h"
#include "arda2/properties/proStringUtils.h"
#include "arda2/math/matUtils.h"

using namespace std;

PRO_REGISTER_ABSTRACT_CLASS( proValidator, proObject )
PRO_REGISTER_CLASS( proValidatorProperty, proValidator )
PRO_REGISTER_CLASS( proValidatorClass, proValidator )
PRO_REGISTER_CLASS( proValidatorRangeFloat, proValidator )
PRO_REGISTER_CLASS( proValidatorRangeInt, proValidator )
PRO_REGISTER_CLASS( proValidatorRangeUint, proValidator )
PRO_REGISTER_CLASS( proValidatorString, proValidator )
PRO_REGISTER_CLASS( proValidatorWstring, proValidator )
PRO_REGISTER_CLASS( proValidatorObject, proValidator )
PRO_REGISTER_CLASS( proValidatorEnum, proValidator )
PRO_REGISTER_CLASS( proValidatorBitfield, proValidator )

//-----------------------------------------------------------------------------
// proValidatorObject

/**
 *  Validates a proObject property.
 *
 *  @param anObject a proObject that holds the value
 *  @param aPropertyName a property name found in the object 
 *
 *  @return ER_Success if the object's property value is valid;
 *  @return ER_Failure if the object's property value is invalid and a ValueInvalid message will be sent
**/
errResult proValidatorObject::Validate( proObject* anObject, const string& aPropertyName )
{
    // validate the proObject* property signaling observers on failure(s)
    if( Validate(anObject, anObject->GetPropertyByName(aPropertyName)) != ER_Success )
    {
        anObject->PropertyValueInvalid( aPropertyName );
        return ER_Failure;
    }

    return ER_Success;
}

/**
 *  There really is no way to make an invalid proObject* valid, so it only validates the object
 *
 *  @param anObject An object pointer to get the property from
 *  @param aPropertyName A property of the object's value to make valid validate
 *
 *  @return ER_Success if the object's property value is valid;
 *  @return ER_Failure if the object's property value is invalid
**/
errResult proValidatorObject::MakeValid( proObject* anObject, const string& aPropertyName )
{
    // there really is no way to make an invalid proObject* valid
    return Validate( anObject, anObject->GetPropertyByName(aPropertyName) );
}

/**
 *  Validates a proObject property.
 *
 *  @param anObject a proObject that holds the value
 *  @param aProperty a property pointer found in the object 
 *
 *  @return ER_Success if the object's property value is valid;
 *  @return ER_Success if the object's property value is NULL;
 *  @return ER_Failure if the object's property value is invalid;
 *  @return ER_Failure if the object's property data type is invalid;
 *  @return ER_Failure if the object's property has no 'ObjectType' annotation
**/
errResult proValidatorObject::Validate( proObject* anObject, proProperty* aProperty )
{
    // property exists & is right data type?
    if( !aProperty || aProperty->GetDataType() != DT_object )
    {
        return ER_Failure;
    }

    proProperty* propObjectType = aProperty->GetPropertyByName( proAnnoStrObjectType );

    // does this property have the "object type" annotation?
    if( !propObjectType )
    {
        // what the ?!
        return ER_Failure;
    }

    // first get value
    proObject* proObjVal = verify_cast<proPropertyObject*>(aProperty)->GetValue( anObject );

    if( proObjVal == NULL )
    {
        // technically, a NULL value is valid
        return ER_Success;
    }

    // alright, then make sure the value is of this instance type
    string strObjectType = proConvUtils::AsString( aProperty, propObjectType );

    if( proClassRegistry::InstanceOf(proObjVal->GetClass(), strObjectType) )
    {
        // all is good
        return ER_Success;
    }

    // ugh, object class type is not an "instance of" that it should be
    return ER_Failure;
}

//-----------------------------------------------------------------------------
// proValidatorEnum

/**
 *  Validates an enumeration property.
 *
 *  @param anObject a proObject that holds the value
 *  @param aPropertyName a property name found in the object 
 *
 *  @return ER_Success if the object's property value is valid;
 *  @return ER_Failure if the object's property value is invalid and a ValueInvalid message will be sent
**/
errResult proValidatorEnum::Validate( proObject* anObject, const string& aPropertyName )
{
    // is the value a valid enumerated value?
    if( Validate(anObject, anObject->GetPropertyByName(aPropertyName)) != ER_Success )
    {
        // make sure observers know about this
        anObject->PropertyValueInvalid( aPropertyName );
        return ER_Failure;
    }

    return ER_Success;
}

/**
 *  This makes an invalid enumeration value equal the first enumerated value 
 *  from the table.
 *
 *  @param anObject An object pointer to get the property from
 *  @param aPropertyName A property of the object's value to make valid validate
 *
 *  @return ER_Success if the value is already valid;
 *  @return ER_Success if the value has been set to a valid value;
 *  @return ER_Failure if the property's data type is invalid;
 *  @return ER_Failure if the enum table is empty
**/
errResult proValidatorEnum::MakeValid( proObject* anObject, const string& aPropertyName )
{
    // make sure value is invalid?
    if( Validate(anObject,aPropertyName) == ER_Success )
    {
        return ER_Success;
    }

    proProperty* prop = anObject->GetPropertyByName( aPropertyName );

    // property exists & is right data type?
    if( !prop || prop->GetDataType() != DT_enum )
    {
        return ER_Failure;
    }

    // get the property enumerated type
    proPropertyEnum* propEnum = verify_cast<proPropertyEnum*>(prop);

    // get all the enumerated strings
    vector<string> enumValues;
    propEnum->GetEnumeration( enumValues );

    // to make this invalid enumeration value valid, 
    // take the first valid enumerated value and set it
    if( enumValues.size() != 0 )
    {
        // set value
        propEnum->SetValue( anObject, propEnum->GetEnumerationValue(enumValues.front()) );

        // all done
        return ER_Success;
    }

    // dog gone it! not even one enum value!
    return ER_Failure;
}

/**
 *  Validates an enumeration property.
 *
 *  @param anObject a proObject that holds the value
 *  @param aPropertyName a property name found in the object 
 *
 *  @return ER_Success if the object's property value is valid;
 *  @return ER_Failure if the object's property enum table is empty;
 *  @return ER_Failure if the object's property value is invalid;
 *  @return ER_Failure if the object's property data type is invalid;
 *  @return ER_Failure if none of the enum values equal the object value
**/
errResult proValidatorEnum::Validate( proObject* anObject, proProperty* aProperty )
{
    // property exists & is right data type?
    if( !aProperty || aProperty->GetDataType() != DT_enum )
    {
        return ER_Failure;
    }

    // get the property enumerated type
    proPropertyEnum* propEnum = verify_cast<proPropertyEnum*>(aProperty);

    // get all the enumerated strings
    vector<string> enumValues;
    propEnum->GetEnumeration( enumValues );

    // are there ANY enum values?
    if( enumValues.size() == 0 )
    {
        // okay, no enum values so any value would be valid
        return ER_Failure;
    }

    uint val = propEnum->GetValue(anObject);
    for( vector<string>::iterator it = enumValues.begin(); it != enumValues.end(); ++it )
    {
        // does this stored value equal this string enum value?
        if( val == propEnum->GetEnumerationValue(*it) )
        {
            return ER_Success;
        }
    }

    // none of the values equal an enumerated value
    return ER_Failure;
}

//-----------------------------------------------------------------------------
// proValidatorBitfield

/**
 *  Validates a bitfield propertiy.
 *
 *  @param anObject a proObject that holds the value
 *  @param aPropertyName a property name found in the object 
 *
 *  @return ER_Success if the object's property value is valid;
 *  @return ER_Failure if the object's property value is invalid and a ValueInvalid message will be sent
**/
errResult proValidatorBitfield::Validate( proObject* anObject, const string& aPropertyName )
{
    // is the value a valid enumerated value?
    if( Validate(anObject, anObject->GetPropertyByName(aPropertyName)) != ER_Success )
    {
        // make sure observers know about this
        anObject->PropertyValueInvalid( aPropertyName );
        return ER_Failure;
    }

    return ER_Success;
}

/**
 *  This will clear all the invalid bits that were set in the value.
 *
 *  @param anObject a proObject that holds the value
 *  @param aPropertyName a property name found in the object 
 *
 *  @return ER_Success if the value is valid;
 *  @return ER_Success if the value has been set to a valid number;
 *  @return ER_Failure if the property data type is invalid;
**/
errResult proValidatorBitfield::MakeValid( proObject* anObject, const string& aPropertyName )
{
    // is the value a valid enumerated value?
    if( Validate(anObject, anObject->GetPropertyByName(aPropertyName)) == ER_Success )
    {
        return ER_Success;
    }

    proProperty* prop = anObject->GetPropertyByName( aPropertyName );

    // property exists & is right data type?
    if( !prop || prop->GetDataType() != DT_bitfield )
    {
        return ER_Failure;
    }

    // get the property bitfield type
    proPropertyBitfield* propBits = verify_cast<proPropertyBitfield*>(prop);

    // get all the bitfield strings
    vector<string> bitValues;
    propBits->GetBitfield( bitValues );

    // find total bit mask
    uint bitMask = 0;
    for( vector<string>::iterator it = bitValues.begin(); it != bitValues.end(); ++it )
    {
        bitMask |= propBits->GetBitfieldValue(*it);
    }

    // mask off all bad bit values & set it
    uint val = propBits->GetValue(anObject) & bitMask;
    propBits->SetValue( anObject, val );

    return ER_Success;
}

/**
 *  Validates a bitfield propertiy.
 *
 *  @param anObject a proObject that holds the value
 *  @param aPropertyName a property name found in the object 
 *
 *  @return ER_Success if the object's property value is valid;
 *  @return ER_Success if the value is ZERO;
 *  @return ER_Success if the bitfield is empty;
 *  @return ER_Failure if the object's property value is invalid;
 *  @return ER_Failure if the property's data type is invalid;
**/
errResult proValidatorBitfield::Validate( proObject* anObject, proProperty* aProperty )
{
    // property exists & is right data type?
    if( !aProperty || aProperty->GetDataType() != DT_bitfield )
    {
        return ER_Failure;
    }

    // get the property bitfield type
    proPropertyBitfield* propBits = verify_cast<proPropertyBitfield*>(aProperty);

    uint val = propBits->GetValue(anObject);

    // any bits set?
    if( val == 0 )
    {
        // if no bits are set, then this is a valid value
        return ER_Success;
    }

    // get all the bitfield strings
    vector<string> bitValues;
    propBits->GetBitfield( bitValues );

    // are there ANY bitfield values?
    if( bitValues.size() == 0 )
    {
        // okay, no bitfield values so any value would be valid
        return ER_Success;
    }

    // find total bit mask
    uint bitMask = 0;
    for( vector<string>::iterator it = bitValues.begin(); it != bitValues.end(); ++it )
    {
        bitMask |= propBits->GetBitfieldValue(*it);
    }

    // eliminate all known bits
    val &= (~bitMask);

    // any bits left?
    if( !val )
    {
        return ER_Success;
    }

    // extra bits are set that are NOT in bitfield values
    return ER_Failure;
}

//-----------------------------------------------------------------------------
// proValidatorProperty

/**
 *  This is a generic wrapper to validate any proProperty object. This will 
 *  create the appropriate proValidator object to validate this property.
 *
 *  @param anObject a proObject that holds the value
 *  @param aPropertyName a property name found in the object 
 *
 *  @return ER_Sucess if the property is valid;
 *  @return ER_Failure if the property is invalid
**/
errResult proValidatorProperty::Validate( proObject* anObject, const string& aPropertyName )
{
    if( anObject )
    {
        proProperty* prop = anObject->GetPropertyByName( aPropertyName );
        if( prop )
        {
            return Validate( anObject, *prop );
        }
    }
    return ER_Failure;
}

/**
 *  This is a generic wrapper to make a proProperty object valid. This will 
 *  create the appropriate proValidator object to make this property validate.
 *
 *  @param anObject a proObject that holds the value
 *  @param aPropertyName a property name found in the object 
 *
 *  @return ER_Sucess if the property's value has been made valid;
 *  @return ER_Failure if the property's value has not been made valid
**/
errResult proValidatorProperty::MakeValid( proObject* anObject, const string& aPropertyName )
{
    if( anObject )
    {
        proProperty* prop = anObject->GetPropertyByName( aPropertyName );
        if( prop )
        {
            return MakeValid( anObject, *prop );
        }
    }
    return ER_Failure;
}

/**
 *  This creates a proValidator object to validate the property. It can use two methods to 
 *  do this. First it tries to find a 'Validator' annotation in the property to create the 
 *  proValidator object. If there is no annotation, it tries to create a proValidator based 
 *  on the property's data type. This method creates the proValidator on the stack, does 
 *  the validity check and returns the result.
 *
 *  @param anObject a proObject that holds the value
 *  @param aProperty a property reference found in the object 
 *
 *  @return ER_Success if the proValidator validates the property;
 *  @return ER_Success if no validator could be created;
 *  @return ER_Failure if the proValidator does not validate the property
**/
errResult proValidatorProperty::Validate( proObject* anObject, proProperty& aProperty )
{
    // is there a property annotation validator class name?
    proProperty* validatorAnno = aProperty.GetPropertyByName(proAnnoStrValidator);
    if( validatorAnno )
    {
        proObject* inst = proClassRegistry::NewInstance( aProperty.GetValue(proAnnoStrValidator) );
        if( inst )
        {
            proValidator* validator = verify_cast<proValidator*>( inst );
            errResult result = validator->Validate( anObject, aProperty.GetName() );
            SafeDelete(validator);
            return result;
        }
    }

    // try the basic data types
    const int32 dt = aProperty.GetDataType();

    if ( dt == DT_int )
    {
        proValidatorRangeInt validator;
        return validator.Validate( anObject, aProperty.GetName() );
    }
    else if ( dt == DT_uint )
    {
        proValidatorRangeUint validator;
        return validator.Validate( anObject, aProperty.GetName() );
    }
    else if ( dt == DT_float )
    {
        proValidatorRangeFloat validator;
        return validator.Validate( anObject, aProperty.GetName() );
    }
    else if ( dt == DT_bool )
    {
        // no proper method for validating boolean values since GetValue() 
        // will always either return either a 1 or 0
        return ER_Success;
    }
    else if ( dt == DT_string )
    {
        proValidatorString validator;
        return validator.Validate( anObject, aProperty.GetName() );
    }
    else if( dt == DT_wstring )
    {
        proValidatorWstring validator;
        return validator.Validate( anObject, aProperty.GetName() );
    }
    else if ( dt == DT_object )
    {
        proValidatorObject validator;
        return validator.Validate( anObject, aProperty.GetName() );
    }
    else if ( dt == DT_enum )
    {
        proValidatorEnum validator;
        return validator.Validate( anObject, aProperty.GetName() );
    }
    else if ( dt == DT_bitfield )
    {        
        proValidatorBitfield validator;
        return validator.Validate( anObject, aProperty.GetName() );
    }

    // if there is no validator for this property type, then the 
    // default behavior is to leave it and signal success
    return ER_Success;
}

/**
 *  This creates a proValidator object to make the property valid. It can use two methods to 
 *  do this. First it tries to find a 'Validator' annotation in the property to create the 
 *  proValidator object. If there is no annotation, it tries to create a proValidator based 
 *  on the property's data type. This method creates the proValidator on the stack.
 *
 *  @param anObject a proObject that holds the value
 *  @param aProperty a property reference found in the object 
 *
 *  @return ER_Success if the proValidator makes the property valid;
 *  @return ER_Success if no validator could be created;
 *  @return ER_Failure if the proValidator does not makes the property valid
**/
errResult proValidatorProperty::MakeValid( proObject* anObject, proProperty& aProperty )
{
    // is there a property annotation validator class name?
    proProperty* validatorAnno = aProperty.GetPropertyByName(proAnnoStrValidator);
    if( validatorAnno )
    {
        proObject* inst = proClassRegistry::NewInstance( aProperty.GetValue(proAnnoStrValidator) );
        if( inst )
        {
            proValidator* validator = verify_cast<proValidator*>( inst );
            errResult result = validator->MakeValid( anObject, aProperty.GetName() );
            SafeDelete(validator);
            return result;
        }
    }

    // try the basic data types
    const int32 dt = aProperty.GetDataType();

    if ( dt == DT_int )
    {
        proValidatorRangeInt validator;
        return validator.MakeValid( anObject, aProperty.GetName() );
    }
    else if ( dt == DT_uint )
    {
        proValidatorRangeUint validator;
        return validator.MakeValid( anObject, aProperty.GetName() );
    }
    else if ( dt == DT_float )
    {
        proValidatorRangeFloat validator;
        return validator.MakeValid( anObject, aProperty.GetName() );
    }
    else if ( dt == DT_bool )
    {
        // no proper method for validating boolean values since GetValue() 
        // will always either return either a 1 or 0
        return ER_Success;
    }
    else if ( dt == DT_string )
    {
        proValidatorString validator;
        return validator.MakeValid( anObject, aProperty.GetName() );
    }
    else if( dt == DT_wstring )
    {
        proValidatorWstring validator;
        return validator.MakeValid( anObject, aProperty.GetName() );
    }
    else if ( dt == DT_object )
    {
        proValidatorObject validator;
        return validator.MakeValid( anObject, aProperty.GetName() );
    }
    else if ( dt == DT_enum )
    {
        proValidatorEnum validator;
        return validator.MakeValid( anObject, aProperty.GetName() );
    }
    else if ( dt == DT_bitfield )
    {
        proValidatorBitfield validator;
        return validator.MakeValid( anObject, aProperty.GetName() );
    }

    // if there is no validator for this property type, then the 
    // default behavior is to leave it and signal success
    return ER_Success;
}

/**
 *  Used for creating a validator for this property (the client will be own this object). 
 *  It can use two methods to do this. First it tries to find a 'Validator' annotation 
 *  in the property to create the proValidator object. If there is no annotation, it tries 
 *  to create a proValidator based on the property's data type. The caller must call
 *  delete on the validator object.
 *
 *  @param anObject a proObject that holds the value
 *  @param aProperty a property reference found in the object 
 *
 *  @return "NULL" if the proValidator could not be created;
 *  @return "a valid proValidator object" if the proValidator could be created
**/
proValidator* proValidatorProperty::CreateValidator( proObject* anObject, const string& aPropertyName )
{
    proValidator* pValidator = 0;

    // make sure the property exists
    proProperty* property = anObject->GetPropertyByName( aPropertyName );
    if( !property )
    {
        return pValidator;
    }

    // is there a property annotation validator class name?
    proProperty* validatorAnno = property->GetPropertyByName(proAnnoStrValidator);
    if( validatorAnno )
    {
        // this is an over-ride
        proObject* inst = proClassRegistry::NewInstance( property->GetValue(proAnnoStrValidator) );
        if( inst )
        {
            pValidator = verify_cast<proValidator*>( inst );
            return pValidator;
        }

        // even though this was an invalid validator class name, fall though. 
        // Maybe the a validator can be made using the data type.
    }

    // try the basic data types
    const int32 dt = property->GetDataType();

    if ( dt == DT_int )
    {
        pValidator = new proValidatorRangeInt;
    }
    else if ( dt == DT_uint )
    {
        pValidator = new proValidatorRangeUint;
    }
    else if ( dt == DT_float )
    {
        pValidator = new proValidatorRangeFloat;
    }
    else if ( dt == DT_bool )
    {
        // no proper method for validating boolean values since GetValue() 
        // will always either return either a 1 or 0
        pValidator = 0;
        return pValidator;
    }
    else if ( dt == DT_string )
    {
        return new proValidatorString;
    }
    else if( dt == DT_wstring )
    {
        return new proValidatorWstring;
    }
    else if ( dt == DT_object )
    {
        return new proValidatorObject;
    }
    else if ( dt == DT_enum )
    {
        return new proValidatorEnum;
    }
    else if ( dt == DT_bitfield )
    {
        return new proValidatorBitfield;
    }

    return pValidator;
}

//-----------------------------------------------------------------------------
// proValidatorClass

/**
 *  This validates all the proClass properties for an object. Both dynamic and 
 *  static properties are validated. Further, all the inherited proClass 
 *  properties are validated. This goes through each property even if one of 
 *  them comes back as "invalid".
 *
 *  @param anObject a proObject that holds the value
 *  @param aPropertyName (Is not used. Pass in "" normally.)
 *
 *  @return ER_Success if all property are reported as valid;
 *  @return ER_Failure if any property comes back as invalid
**/
errResult proValidatorClass::Validate( proObject* anObject, const string& aPropertyName )
{
    UNREF(aPropertyName);

    proValidatorProperty validator;

    errResult result = ER_Success;
    for( int i = 0; i < anObject->GetPropertyCount(); ++i )
    {
        if( validator.Validate( anObject, *anObject->GetPropertyAt(i) ) != ER_Success )
        {
            result = ER_Failure;
        }
    }
    return result;
}

/**
 *  This makes all the proClass properties valid for an object. Both dynamic and 
 *  static properties are validated. Further, all the inherited proClass 
 *  properties are validated. This goes through each property even if one of 
 *  the properties could not be made valid.
 *
 *  @param anObject a proObject that holds the value
 *  @param aPropertyName (Is not used. Pass in "" normally.)
 *
 *  @return ER_Success if all property are reported as made valid;
 *  @return ER_Failure if any property comes back as not valid
**/
errResult proValidatorClass::MakeValid( proObject* anObject, const string& aPropertyName )
{
    UNREF(aPropertyName);

    proValidatorProperty validator;

    errResult result = ER_Success;
    for( int i = 0; i < anObject->GetPropertyCount(); ++i )
    {
        if( validator.MakeValid( anObject, *anObject->GetPropertyAt(i) ) != ER_Success )
        {
            result = ER_Failure;
        }
    }
    return result;
}

//-----------------------------------------------------------------------------
/*
 *  UNIT TESTS
 */
#if BUILD_UNIT_TESTS

#include "arda2/test/tstUnit.h"
#include "arda2/properties/proClassNative.h"
#include "arda2/properties/proPropertyNative.h"
#include "arda2/math/matTypes.h"

static const int defaultInt       = 1;
static const uint defaultUint     = 11234;
static const float defaultFloat   = 0.55113f;
static const string defaultString = "default";
static const bool defaultBool     = true;

static const string newName("foobar");
static const int newX = 51;
static const uint newSize = 99999;
static const float newRatio = 0.4f;

/**
 * test custom proValidator
*/

class myValidatorPowerOf2 : public proValidatorRangeInt
{
    PRO_DECLARE_OBJECT

public:
    virtual errResult Validate( proObject* anObject, const string& aPropertyName )
    {
        const int pow2min = 2;
        const int pow2max = 256;

        if( ValidateInRange(anObject,aPropertyName,pow2min,pow2max) == ER_Success )
        {
            int val = proConvUtils::AsInt( anObject, anObject->GetPropertyByName(aPropertyName) );

            if( IsPower2(val) )
            {
                return ER_Success;
            }
        }

        return ER_Failure;
    }

    virtual errResult MakeValid( proObject* anObject, const string& aPropertyName )
    {
        const int pow2min = 2;
        const int pow2max = 256;

        proProperty* prop = anObject->GetPropertyByName(aPropertyName);
        if( !prop )
        {
            return ER_Failure;
        }

        int val = proConvUtils::AsInt( anObject, prop );

        if( IsPower2(val) )
        {
            return ER_Success;
        }

        val = matClamp( val, pow2min, pow2max );

        if( IsPower2(val) )
        {
            proConvUtils::SetValue( anObject, prop, val );
        }
        else
        {
            proConvUtils::SetValue( anObject, prop, MakePower2Down(val) );
        }

        return ER_Success;
    }
};
PRO_REGISTER_CLASS( myValidatorPowerOf2, proValidatorRangeInt );

/**
 * test classes for proAdapter tests
*/

class proTestValidatorObject : public proObject
{
    PRO_DECLARE_OBJECT
public:
    proTestValidatorObject() : 
        m_Int(defaultInt), 
        m_Enum(1), 
        m_Enum2(0), 
        m_Pow2(128),
        m_Float(defaultFloat),
        m_Object(0),
        m_Bits(0)
    {
    }

    int m_Int;
    int GetInt( void ) const { return m_Int; }
    void SetInt( const int aValue ) { m_Int = aValue; }

    uint m_Enum;
    uint GetEnum( void ) const { return m_Enum; }
    void SetEnum( const uint aValue ) { m_Enum = aValue; }

    uint m_Enum2;
    uint GetEnum2( void ) const { return m_Enum2; }
    void SetEnum2( const uint aValue ) { m_Enum2 = aValue; }

    int m_Pow2;
    int GetPow2( void ) const { return m_Pow2; }
    void SetPow2( const int aValue ) { m_Pow2 = aValue; }

    float m_Float;
    float GetFloat( void ) const { return m_Float; }
    void SetFloat( const float aValue ) { m_Float = aValue; }

    proObject* m_Object;
    proObject* GetObjectPtr( void ) const { return m_Object; }
    void SetObjectPtr( proObject* aValue ) { m_Object = aValue; }

    uint m_Bits;
    uint GetBits( void ) const { return m_Bits; }
    void SetBits( const uint aValue ) { m_Bits = aValue; }
};

PRO_REGISTER_CLASS(proTestValidatorObject, proObject)
REG_PROPERTY_INT_ANNO(proTestValidatorObject, Int, "Default|1|Min|-10|Max|10" )
REG_PROPERTY_FLOAT_ANNO(proTestValidatorObject, Float, "Default|1|Min|-1.0|Max|1.0" )
REG_PROPERTY_INT_ANNO(proTestValidatorObject, Pow2, "Default|2|Validator|myValidatorPowerOf2" )
REG_PROPERTY_OBJECT(proTestValidatorObject, ObjectPtr, proTestValidatorObject )
REG_PROPERTY_ENUM(proTestValidatorObject, Enum, "one,1,two,2,four,4," )
REG_PROPERTY_ENUM(proTestValidatorObject, Enum2, "zero,0,eight,8" )
REG_PROPERTY_BITFIELD(proTestValidatorObject, Bits, "bit1,1,bit2,2,bit3,3,bit5,5" )

/**
 * used to test the multiple derived proObject classes
*/

class proTestValidatorObjectDerived : public proTestValidatorObject
{
    PRO_DECLARE_OBJECT
public:
    proTestValidatorObjectDerived() : m_NewInt(99)
    {
    }

    int m_NewInt;
    int GetInt( void ) const { return m_NewInt; }
    void SetInt( const int aValue ) { m_NewInt = aValue; }

    string m_TheString;
    string GetString( void ) const { return m_TheString; }
    void SetString( const string& aValue ) { m_TheString = aValue; }

    wstring m_TheWideString;
    wstring GetWideString( void ) const { return m_TheWideString; }
    void SetWideString( const wstring& aValue ) { m_TheWideString = aValue; }
};

PRO_REGISTER_CLASS(proTestValidatorObjectDerived, proTestValidatorObject)
REG_PROPERTY_INT_ANNO(proTestValidatorObjectDerived, Int, "Min|1|Max|100" )
REG_PROPERTY_STRING_ANNO(proTestValidatorObjectDerived, String, "Max|16" )
REG_PROPERTY_WSTRING_ANNO(proTestValidatorObjectDerived, WideString, "Max|16" )

/**
 * the validator tests
*/

class proValidatorTests : public tstUnit
{
private:
    proTestValidatorObject* m_object;

public:
    proValidatorTests() : m_object(0)
    {
    }

    virtual void Register()
    {
        SetName("proValidatorTests");

        proClassRegistry::Init();

        AddTestCase("testBasicTypes", &proValidatorTests::testBasicTypes);
        AddTestCase("testClassValidation", &proValidatorTests::testClassValidation);
        AddTestCase("testDerivedClassValidation", &proValidatorTests::testDerivedClassValidation);
        AddTestCase("testObjectPtrValidation", &proValidatorTests::testObjectPtrValidation);
        AddTestCase("testEnumValidation", &proValidatorTests::testEnumValidation);
        AddTestCase("testBitsValidation", &proValidatorTests::testBitsValidation);
    };

    virtual void TestCaseSetup()
    {
        m_object = new proTestValidatorObject();
    }

    virtual void TestCaseTeardown()
    {
        SafeDelete(m_object);
    }

    void testBasicTypes() const
    {
        proValidatorRangeInt validator;

        // make sure "it is valid"
        errResult ret = validator.Validate( m_object, "Int" );
        TESTASSERT( ret == ER_Success );

        // test explicit validation
        ret = validator.ValidateInRange( m_object, "Int", 0, 10 );
        TESTASSERT( ret == ER_Success );

        // determine the property as invalid
        m_object->SetValue( "Int", "11" );
        ret = validator.Validate( m_object, "Int" );
        TESTASSERT( ret == ER_Failure );
    }

    void testClassValidation() const
    {
        //
        // simple object validation
        //
        proValidatorClass validator;
        errResult ret = validator.Validate( m_object, "" );
        TESTASSERT( ret == ER_Success );

        // make most of the properties invalid values
        m_object->SetValue( "Int", "11" );
        m_object->SetValue( "Float", "2.0" );
        m_object->SetValue( "Pow2", "127" );

        // class validation (expect failure)
        if( validator.Validate( m_object, "" ) == ER_Failure )
        {
            validator.MakeValid( m_object, "" );
        }
        ret = validator.Validate( m_object, "" );
        TESTASSERT( ret == ER_Success );
    }

    void testDerivedClassValidation() const
    {
        proValidatorClass validator;

        //
        // derived object test
        //
        proTestValidatorObjectDerived derivedObject;
        errResult ret = validator.Validate( &derivedObject, "" );
        TESTASSERT( ret == ER_Success );

        // set invalid values
        derivedObject.SetValue( "String", "12345678901234567890" );
        derivedObject.SetValue( "Int", "101" );
        proConvUtils::SetValue( &derivedObject, 
                                derivedObject.GetPropertyByName("WideString"), 
                                wstring(L"12345678901234567890") );

        // class validation (expect failure)
        if( validator.Validate( &derivedObject, "" ) == ER_Failure )
        {
            validator.MakeValid( &derivedObject, "" );
        }
        ret = validator.Validate( &derivedObject, "" );
        TESTASSERT( ret == ER_Success );
    }

    void testObjectPtrValidation() const
    {
        proValidatorObject validator;

        // make sure "it is valid"
        errResult ret = validator.Validate( m_object, "ObjectPtr" );
        TESTASSERT( ret == ER_Success );

        // test explicit validation
        verify_cast<proPropertyObject*>(m_object->GetPropertyByName("ObjectPtr"))->SetValue( m_object, m_object );
        ret = validator.Validate( m_object, "ObjectPtr" );
        TESTASSERT( ret == ER_Success );

        // test explicit validation (expected failure)
        verify_cast<proPropertyObject*>(m_object->GetPropertyByName("ObjectPtr"))->SetValue( m_object, &validator );
        ret = validator.Validate( m_object, "ObjectPtr" );
        TESTASSERT( ret == ER_Failure );
    }

    void testEnumValidation() const
    {
        proValidatorEnum validator;

        proPropertyEnum* propEnum = verify_cast<proPropertyEnum*>( m_object->GetPropertyByName("Enum") );

        // make sure "it is valid"
        errResult ret = validator.Validate( m_object, propEnum->GetName() );
        TESTASSERT( ret == ER_Success );

        // test explicit validation
        propEnum->SetValue( m_object, 4 );
        ret = validator.Validate( m_object, propEnum->GetName() );
        TESTASSERT( ret == ER_Success );

        // test invalid value (expected failure)
        propEnum->SetValue( m_object, 3 );
        ret = validator.Validate( m_object, propEnum->GetName() );
        TESTASSERT( ret == ER_Failure );

        // test make valid
        ret = validator.MakeValid( m_object, propEnum->GetName() );
        TESTASSERT( ret == ER_Success );

        // change enum table and re-check (expect success)
        ret = validator.Validate( m_object, "Enum2" );
        TESTASSERT( ret == ER_Success );
    }
    
    void testBitsValidation() const
    {
        proValidatorBitfield validator;

        proPropertyBitfield* propBits = verify_cast<proPropertyBitfield*>( m_object->GetPropertyByName("Bits") );

        // make sure "it is valid"
        errResult ret = validator.Validate( m_object, propBits->GetName() );
        TESTASSERT( ret == ER_Success );

        // test explicit validation
        propBits->FromString( m_object, "bit1|bit2|bit3" );
        ret = validator.Validate( m_object, propBits->GetName() );
        TESTASSERT( ret == ER_Success );

        // test invalid value (expected failure)
        propBits->SetValue( m_object, (1<<4) | (1<<3) );
        ret = validator.Validate( m_object, propBits->GetName() );
        TESTASSERT( ret == ER_Failure );

        // test make valid
        ret = validator.MakeValid( m_object, propBits->GetName() );
        TESTASSERT( ret == ER_Success );
    }
};

EXPORTUNITTESTOBJECT(proValidatorTests);
#endif
