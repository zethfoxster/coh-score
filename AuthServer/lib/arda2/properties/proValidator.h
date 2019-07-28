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

#ifndef _proValidator_INCLUDED_
#define _proValidator_INCLUDED_

#include "proFirst.h"

#include "arda2/core/corStdString.h"
#include "arda2/core/corStlVector.h"
#include "arda2/math/matUtils.h"
#include "arda2/properties/proClassRegistry.h"
#include "arda2/properties/proObject.h"
#include "arda2/properties/proProperty.h"
#include "arda2/properties/proStringUtils.h"
#include <float.h>

//-----------------------------------------------------------------------------
/*! The base validator interface.  
/**
 *
 * There is no "bool" type validator object, since the compiler will make 
 * the proPropertyBool object will always return "bool" value which can only 
 * be a 1 or 0.
 * 
 */
class proValidator : public proObject
{
    PRO_DECLARE_OBJECT
public:

    proValidator() : proObject() {}

    virtual ~proValidator(){}

    virtual errResult Validate( proObject* anObject, const std::string& aPropertyName ) = 0;

    virtual errResult MakeValid( proObject* anObject, const std::string& aPropertyName ) = 0;
};

//-----------------------------------------------------------------------------
/**
 * 
 * The template range checking validator.
 * 
 */
template <typename TYPE>
class proValidatorRangeT : public proValidator
{
public:

    proValidatorRangeT() {}

    virtual ~proValidatorRangeT() {}

    /**
     *  Validates a number property in a certain range.
     *
     *  @param anObject a proObject that holds the value
     *  @param aPropertyName a property name found in the object 
     *
     *  @return ER_Success if the property value is with in range
     *  @return ER_Failure if the property object does not exist
     *  @return ER_Failure if the property object was the wrong data type
     *  @return ER_Failure if the property value is not with in range, and will send Property Value invalid event
     *
     *  @see GetMinMax
     */
    virtual errResult Validate( proObject* anObject, const std::string& aPropertyName )
    {
        // has this property?
        proProperty* prop = 0;
        errResult ret = FillOutProperty(anObject,&prop,aPropertyName);
        if( ret != ER_Success )
        {
            return ret;
        }

        // get min & max, and return the result of the range logic
        TYPE min, max;
        GetMinMax( min, max, prop );
        return ValidateInRange( anObject, prop, min, max );
    }

    /**
     *  Clamps the property's number value with in the range.
     *
     *  @param anObject a proObject that holds the value
     *  @param aPropertyName a property name found in the object 
     *
     *  @return ER_Success if the property value is now with in range
     *  @return ER_Failure if the property object does not exist
     *  @return ER_Failure if the property object was the wrong data type
     *
     *  @see GetMinMax
     */
    virtual errResult MakeValid( proObject* anObject, const std::string& aPropertyName )
    {
        // has this property?
        proProperty* prop = 0;
        errResult ret = FillOutProperty(anObject,&prop,aPropertyName);
        if( ret != ER_Success )
        {
            return ret;
        }

        // find the range
        TYPE min, max;
        GetMinMax( min, max, prop );

        // get current value & and save a copy
        TYPE val;
        proConvUtils::GetValue(anObject, prop, val);
        TYPE oldVal = val;

        // clamp value with in range
        val = matClamp( val, min, max );

        // has the value been changed?
        if( val != oldVal )
        {
            // set that new value
            proConvUtils::SetValue( anObject, prop, val );
        }
        return ER_Success;
    }

    /**
     *  Validates that a property in an object is in a certain 
     *  passed in range. This is a public interface to over ride 
     *  the Min and Max annotations.
     *
     *  @param anObject a proObject that holds the value
     *  @param aPropertyName a property name found in the object 
     *  @param aMin A minimum value of the range
     *  @param aMax A maximum value of the range
     *
     *  @return ER_Success if the property value is with in range
     *  @return ER_Failure if the property object does not exist
     *  @return ER_Failure if the property object was the wrong data type
     *  @return ER_Failure if the property value is not with in range, and will send Property Value invalid event
     */
    errResult ValidateInRange( proObject* anObject, const std::string& aPropertyName, TYPE aMin, TYPE aMax )
    {
        // has this property?
        proProperty* prop = 0;
        errResult ret = FillOutProperty(anObject,&prop,aPropertyName);
        if( ret != ER_Success )
        {
            return ret;
        }

        return ValidateInRange( anObject, prop, aMin, aMax );
    }

protected:

    /**
     *  Validates a property object using passed in min & max values. 
     *
     *  @param anObject a proObject that holds the value
     *  @param aProperty a property pointer found in the object 
     *  @param aMin A minimum value of the range
     *  @param aMax A maximum value of the range
     *
     *  @return ER_Success if the value is with in range
     *  @return ER_Failure if the value is out of range, plus it sends the "Property Value Invalid"
     */
    errResult ValidateInRange( proObject* anObject, proProperty* aProperty, TYPE aMin, TYPE aMax )
    {
        // the final range check
        TYPE val;
        proConvUtils::GetValue( anObject, aProperty, val );
        if( (val >= aMin) && (val <= aMax) )
        {
            return ER_Success;
        }

        // signal the value check failed
        anObject->PropertyValueInvalid( aProperty->GetName() );
        return ER_Failure;
    }

    /**
     *  Determines that the property exists and is the correct data type.
     *
     *  @param anObject a proObject that holds the value
     *  @param aPropName a property name found in the object 
     *
     *  @return ER_Success if the property exists (by name) and is the correct data type
     *  @return ER_Failure if either the property does not exist or the wrong data type
     */
    errResult FillOutProperty( proObject* anObject, proProperty** aProperty, const std::string& aPropName )
    {
        *aProperty = anObject->GetPropertyByName( aPropName );

        if( *aProperty == 0 || !IsCorrectDataType(*aProperty) )
        {
            return ER_Failure;
        }
        return ER_Success;
    }

    /**
     *  Gets the Min and Max values for this property. If the Min 
     *  or Max annotations do not exist, then the default Min or 
     *  Max value will be used.
     *
     *  @param aProperty A property pointer found in the object 
     *  @param aMin A reference min value to be filled out
     *  @param aMax A reference max value to be filled out
     *
     */
    void GetMinMax( TYPE& aMin, TYPE &aMax, proProperty* aProperty )
    {
        SetDefaultMinMax( aMin, aMax );

        proProperty* propMin = aProperty->GetPropertyByName( proAnnoStrMin );
        proProperty* propMax = aProperty->GetPropertyByName( proAnnoStrMax );

        if( propMin && propMax )
        {
            proConvUtils::GetValue( aProperty, propMin, aMin );
            proConvUtils::GetValue( aProperty, propMax, aMax );
        }
        else if( propMin )
        {
            proConvUtils::GetValue( aProperty, propMin, aMin );
        }
        else if( propMax )
        {
            proConvUtils::GetValue( aProperty, propMax, aMax );
        }

        // last minute sanity, make sure "min is min" and "max is max"
        aMin = Min(aMin,aMax);
        aMax = Max(aMax,aMin);
    }

protected:

    /**
     *  To be implemented. Should determine the TYPE's min and max default values.
     *
     *  @param aMin A reference to be filled out with a min value
     *  @param aMax A reference to be filled out with a max value
     *
     */
    virtual void SetDefaultMinMax( TYPE& aMin, TYPE& aMax ) = 0;

    /**
     *  To be implemented. Should determine if the property is the correct data type.
     *
     *  @param aProperty A property pointer found in the object 
     *
     *  @return TRUE if the property is the correct data type
     *  @return FALSE if the property is not the correct data type
     */
    virtual bool IsCorrectDataType( proProperty* aProperty ) = 0;
};

//-----------------------------------------------------------------------------
/**
 * 
 * The float range validator
 * 
 */
class proValidatorRangeFloat : public proValidatorRangeT<float>
{
    PRO_DECLARE_OBJECT

protected:

    /**
     *  Fills out float min and max.
     *
     *  @param aMin A reference to be filled out with float min
     *  @param aMax A reference to be filled out with float max
     *
     */
    virtual void SetDefaultMinMax( float& aMin, float& aMax )
    {
        aMin = FLT_MIN;
        aMax = FLT_MAX;
    }

    /**
     *  Determines if a property object data type is a DT_float
     *
     *  @param aProperty A property pointer found in the object 
     *
     *  @return TRUE if the property is the correct data type
     *  @return FALSE if the property is not the correct data type
     */
    virtual bool IsCorrectDataType( proProperty* aProperty )
    {
        return aProperty->GetDataType() == DT_float;
    }
};

//-----------------------------------------------------------------------------
/**
 * 
 * The integer range validator
 * 
 */
class proValidatorRangeInt : public proValidatorRangeT<int>
{
    PRO_DECLARE_OBJECT

protected:

    /**
     *  Fills out integer min and max.
     *
     *  @param aMin A reference to be filled out with integer min
     *  @param aMax A reference to be filled out with integer max
     *
     */
    virtual void SetDefaultMinMax( int& aMin, int& aMax )
    {
        aMin = INT_MIN;
        aMax = INT_MAX;
    }

    /**
     *  Determines if a property object data type is a DT_int
     *
     *  @param aProperty A property pointer found in the object 
     *
     *  @return TRUE if the property is the correct data type
     *  @return FALSE if the property is not the correct data type
     */
    virtual bool IsCorrectDataType( proProperty* aProperty )
    {
        return aProperty->GetDataType() == DT_int;
    }
};

//-----------------------------------------------------------------------------
/**
 * 
 * The unsigned integer range validator
 * 
 */
class proValidatorRangeUint : public proValidatorRangeT<uint>
{
    PRO_DECLARE_OBJECT

protected:

    /**
     *  Fills out unsigned integer min and max.
     *
     *  @param aMin A reference to be filled out with unsigned integer min
     *  @param aMax A reference to be filled out with unsigned integer max
     *
     */
    virtual void SetDefaultMinMax( uint& aMin, uint& aMax )
    {
        aMin = 0;
        aMax = UINT_MAX;
    }

    /**
     *  Determines if a property object data type is a DT_uint
     *
     *  @param aProperty A property pointer found in the object 
     *
     *  @return TRUE if the property is the correct data type
     *  @return FALSE if the property is not the correct data type
     */
    virtual bool IsCorrectDataType( proProperty* aProperty )
    {
        return aProperty->GetDataType() == DT_uint;
    }
};

//-----------------------------------------------------------------------------
/**
 * 
 * The basic string validator template
 * 
 */
template <typename TYPE>
class proValidatorBasicStringT : public proValidator
{
public:

    proValidatorBasicStringT() : proValidator() {}

    virtual ~proValidatorBasicStringT(){}

    /**
     *  Using the "Max" annotation, this tests the string array is smaller 
     *  than this value. If no Max annotation exists, then any 
     *  size is valid.
     *
     *  @param anObject a proObject that holds the value
     *  @param aPropertyName a property name found in the object 
     *
     *  @return ER_Success if there is no Max annotation
     *  @return ER_Success if the string could be resized
     *  @return ER_Failure if the data type is incorrect
     *  @return ER_Failure if string size greater than Max, also send the Property Value Invalid event
    **/
    virtual errResult Validate( proObject* anObject, const std::string& aPropertyName )
    {
        proProperty* prop = anObject->GetPropertyByName( aPropertyName );

        // has this property & is right data type?
        if( !prop || !IsCorrectDataType(prop) )
        {
            return ER_Failure;
        }

        proProperty* propMax = prop->GetPropertyByName( proAnnoStrMax );
        if( !propMax )
        {
            // no maximum specified, thus all string sizes will work
            return ER_Success;
        }

        uint valueMax = proConvUtils::AsUint(prop,propMax);

        TYPE value;
        proConvUtils::GetValue( anObject, prop, value );

        if( value.size() > valueMax )
        {
            anObject->PropertyValueInvalid( aPropertyName );
            return ER_Failure;
        }
        return ER_Success;
    }

    /**
     *  Using the "Max" annotation, this caps any string array to that 
     *  value. If no Max annotation exists, then any size is valid.
     *
     *  @param anObject a proObject that holds the value
     *  @param aPropertyName a property name found in the object 
     *
     *  @return ER_Success if there is no Max annotation
     *  @return ER_Success if the string could be resized
     *  @return ER_Failure if the data type is incorrect
    **/
    virtual errResult MakeValid( proObject* anObject, const std::string& aPropertyName )
    {
        proProperty* prop = anObject->GetPropertyByName( aPropertyName );

        // has this property & is right data type?
        if( !prop || !IsCorrectDataType(prop) )
        {
            return ER_Failure;
        }

        proProperty* propMax = prop->GetPropertyByName( proAnnoStrMax );
        if( !propMax )
        {
            // no maximum specified, thus all string sizes will work
            return ER_Success;
        }

        uint valueMax = proConvUtils::AsUint(prop,propMax);

        TYPE value;
        proConvUtils::GetValue( anObject, prop, value );

        if( value.size() > valueMax )
        {
            value.resize( valueMax );
            proConvUtils::SetValue( anObject, prop, value );
        }

        // all done
        return ER_Success;
    }

protected:

    /**
    *  The template implementation tests if the property is the right data type.
    *
    *  @param aProperty A property to test its data type.
    *
    *  @return Should return "true" if the data type is correct.
    **/
    virtual bool IsCorrectDataType( proProperty* aProperty ) = 0;
};

//-----------------------------------------------------------------------------
/**
 * 
 * The wide string validator
 * 
 */
class proValidatorWstring: public proValidatorBasicStringT<std::wstring>
{
    PRO_DECLARE_OBJECT

protected:

    /**
     *  Determines if a property object data type is a DT_wstring
     *
     *  @param aProperty A property pointer found in the object 
     *
     *  @return TRUE if the property is the correct data type
     *  @return FALSE if the property is not the correct data type
     */
    virtual bool IsCorrectDataType( proProperty* aProperty )
    {
        return( aProperty->GetDataType() == DT_wstring );
    }
};

//-----------------------------------------------------------------------------
/**
 * 
 * The string validator
 * 
 */
class proValidatorString: public proValidatorBasicStringT<std::string>
{
    PRO_DECLARE_OBJECT

protected:

    /**
     *  Determines if a property object data type is a DT_string
     *
     *  @param aProperty A property pointer found in the object 
     *
     *  @return TRUE if the property is the correct data type
     *  @return FALSE if the property is not the correct data type
     */
    virtual bool IsCorrectDataType( proProperty* aProperty )
    {
        return( aProperty->GetDataType() == DT_string );
    }
};

//-----------------------------------------------------------------------------
/**
 * 
 * The proObject pointer validator. Verifies the value stored in the object 
 * pointer is either NULL or the of the class that it is designed to hold. 
 * When a proPropertyObject is created it automatically inserts an annotation 
 * about the object pointer expected.
 * 
 */
class proValidatorObject : public proValidator
{
    PRO_DECLARE_OBJECT
public:

    proValidatorObject() : proValidator() {}

    virtual ~proValidatorObject(){}

    virtual errResult Validate( proObject* anObject, const std::string& aPropertyName );

    virtual errResult MakeValid( proObject* anObject, const std::string& aPropertyName );

    errResult Validate( proObject* anObject, proProperty* aProperty );
};

//-----------------------------------------------------------------------------
/**
 * 
 * The enumeration validator. Verifies the value stored in the object 
 * pointer is in the enum table. 
 * 
 */
class proValidatorEnum : public proValidator
{
    PRO_DECLARE_OBJECT
public:

    proValidatorEnum() : proValidator() {}

    virtual ~proValidatorEnum(){}

    virtual errResult Validate( proObject* anObject, const std::string& aPropertyName );

    virtual errResult MakeValid( proObject* anObject, const std::string& aPropertyName );

    errResult Validate( proObject* anObject, proProperty* aProperty );
};

//-----------------------------------------------------------------------------
/**
 * 
 * The bitfield validator. Verifies the value stored in the object 
 * pointer is in the bitfield table. 
 * 
 */
class proValidatorBitfield : public proValidator
{
    PRO_DECLARE_OBJECT
public:

    proValidatorBitfield() : proValidator() {}

    virtual ~proValidatorBitfield(){}

    virtual errResult Validate( proObject* anObject, const std::string& aPropertyName );

    virtual errResult MakeValid( proObject* anObject, const std::string& aPropertyName );

    errResult Validate( proObject* anObject, proProperty* aProperty );
};

//-----------------------------------------------------------------------------
/**
 * 
 *  This is the basic property validator. This is a generic wrapper that will 
 *  either find the data type of the property or the proAnnoStrValidator 
 *  annotation to try to validate this property. 
 * 
 */
class proValidatorProperty : public proValidator
{
    PRO_DECLARE_OBJECT
public:

    proValidatorProperty() {}

    virtual ~proValidatorProperty() {}

    // from proValidator interface

    virtual errResult Validate( proObject* anObject, const std::string& aPropertyName );

    virtual errResult MakeValid( proObject* anObject, const std::string& aPropertyName );

    // more straight forward interface

    errResult Validate( proObject* anObject, proProperty& aProperty );

    errResult MakeValid( proObject* anObject, proProperty& aProperty );

    // for creating a validator for this property (the client will be own this object) 

    proValidator* CreateValidator( proObject* anObject, const std::string& aPropertyName );
};


//-----------------------------------------------------------------------------
/**
 * 
 *  This goes through the object's properties (both dynamic and static) 
 *  and validates each one. This uses the proValidatorProperty object to 
 *  generically validate the properties. This will go through all the 
 *  derived classes as well.
 *
 *  The "aPropertyName" is not used on either Validate() or MakeValid()
 * 
 */
class proValidatorClass : public proValidator
{
    PRO_DECLARE_OBJECT
public:

    proValidatorClass() {}

    virtual ~proValidatorClass() {}

    // from the proValidator interface

    virtual errResult Validate( proObject* anObject, const std::string& aPropertyName );

    virtual errResult MakeValid( proObject* anObject, const std::string& aPropertyName );
};

#endif // _proValidator_INCLUDED_
