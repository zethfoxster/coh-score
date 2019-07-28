
#include "arda2/math/matFirst.h"
#include "arda2/math/matProperty.h"
#include "arda2/math/matAdapters.h"

#include "arda2/properties/proObjectWriter.h"
#include "arda2/properties/proObjectReader.h"

//#if BUILD_PROPERTY_SYSTEM

#include "arda2/properties/proClassNative.h"

using namespace std;

#if CORE_SYSTEM_LINUX
#include "ctype.h"
#endif

static const char* matDisplayKeyValue = "Display";
static const char* matPropertySummaryKeyValue = "DisplayInline";
PRO_REGISTER_ABSTRACT_CLASS(matPropertyMatrix, proProperty)
PRO_REGISTER_ABSTRACT_CLASS(matPropertyVector3, proProperty)
PRO_REGISTER_ABSTRACT_CLASS(matPropertyQuaternion, proProperty)
PRO_REGISTER_ABSTRACT_CLASS(matPropertyColor, proProperty)
PRO_REGISTER_ABSTRACT_CLASS(matPropertyColorInt, proProperty)

PRO_REGISTER_ABSTRACT_CLASS(matPropertyMatrixNative, matPropertyMatrix)
PRO_REGISTER_ABSTRACT_CLASS(matPropertyVector3Native, matPropertyVector3)
PRO_REGISTER_ABSTRACT_CLASS(matPropertyQuaternionNative, matPropertyQuaternion)
PRO_REGISTER_ABSTRACT_CLASS(matPropertyColorNative, matPropertyColor)
PRO_REGISTER_ABSTRACT_CLASS(matPropertyColorIntNative, matPropertyColorInt)

PRO_REGISTER_CLASS(matPropertyMatrixOwner, matPropertyMatrix)
PRO_REGISTER_CLASS(matPropertyVector3Owner, matPropertyVector3)
PRO_REGISTER_CLASS(matPropertyQuaternionOwner, matPropertyQuaternion)
PRO_REGISTER_CLASS(matPropertyColorOwner, matPropertyColor)
PRO_REGISTER_CLASS(matPropertyColorIntOwner, matPropertyColorInt)


void proRegisterMathTypes()
{
    IMPORT_PROPERTY_CLASS(matPropertyMatrix);
    IMPORT_PROPERTY_CLASS(matPropertyVector3);
    IMPORT_PROPERTY_CLASS(matPropertyColor);
    IMPORT_PROPERTY_CLASS(matPropertyColorInt);
    IMPORT_PROPERTY_CLASS(matPropertyQuaternion);

    IMPORT_PROPERTY_CLASS(matPropertyMatrixOwner);
    IMPORT_PROPERTY_CLASS(matPropertyVector3Owner);
    IMPORT_PROPERTY_CLASS(matPropertyColorOwner);
    IMPORT_PROPERTY_CLASS(matPropertyColorIntOwner);
    IMPORT_PROPERTY_CLASS(matPropertyQuaternionOwner);


    PRO_IMPORT_ADAPTER_CLASS(matTransformAdapter);
}

// write the object to a writer (serialize)
void matPropertyMatrix::Write( proObjectWriter* out, const proObject* obj ) const 
{ 
    out->PutPropValue(GetName(), ToString(obj) ); 
}

// read the object from a reader (unserialize)
void matPropertyMatrix::Read( proObjectReader* in, proObject* obj ) 
{ 
    string str; 
    in->GetPropValue(GetName(), str); 
    FromString(obj, str); 
}


// write the object to a writer (serialize)
void matPropertyVector3::Write( proObjectWriter* out, const proObject* obj ) const 
{ 
    out->PutPropValue(GetName(), ToString(obj) ); 
}

// read the object from a reader (unserialize)
void matPropertyVector3::Read( proObjectReader* in, proObject* obj ) 
{ 
    string str; 
    in->GetPropValue(GetName(), str); 
    FromString(obj, str); 
}

/**
  * overridden:  builds a proper string for a vector
  *
  * inputs:
  * @param inObject - to work from
  *
  * outputs:
  * @return processed display string, or ""
  *
  */
string matPropertyVector3::GetDisplayString( proObject* inObject )
{
    return BuildDisplayString( inObject, matDisplayKeyValue );
}

/**
  * overridden:  builds a proper string for a vector
  *
  * inputs:
  * @param inObject - to work from
  *
  * outputs:
  * @return processed display string, or ""
  *
  */
string matPropertyVector3::GetPropertySummaryString( proObject* inObject )
{
    return BuildDisplayString( inObject, matPropertySummaryKeyValue );
}

/**
  * builds a string description of this vector, using the display settings
  *
  * inputs:
  * @param inObject - to work from
  * @param inKeyValue - the type of annotation display string to look for
  *
  * outputs:
  * @return processed display string, or ""
  *
  */
string matPropertyVector3::BuildDisplayString(
    proObject* inObject, const string& inKeyValue )
{
    char buffer[ 1024 ] = { 0 };

    // find the child property that is a proPropertyStringOwner
    // with the name Display
    for ( int i = 0; i < GetPropertyCount(); ++i )
    {
        proProperty* currentChild = GetPropertyAt( i );

        if ( currentChild->GetName() == inKeyValue && proClassRegistry::InstanceOf(
            currentChild->GetClass(), "proPropertyStringOwner" ) )
        {
            // compute the data format specifier, and verify that
            // it will work for the data type of this property.  Also
            // make sure there is the property number of variable
            // specifiers.
            string formatSpecifier( currentChild->ToString( this ) );
            if
            (
                proConvUtils::proFormatStringMatchesDataType( formatSpecifier, DT_float )
                &&
                proConvUtils::proCountVariableSpecifiers( formatSpecifier ) == 3
            )
            {
                // build the string up based on the actual type of property
                matVector3 myValue( GetValue( inObject ) );
                sprintf( buffer, formatSpecifier.c_str(),
                    myValue.x, myValue.y, myValue.z );
            }
            else
            {
                sprintf( buffer, "##BADFORMAT##" );
            }

            // stop looping, since we've found the
            // relevant data member
            break;
        }
    }

    // else....
    return string( buffer );
}

// write the object to a writer (serialize)
void matPropertyColor::Write( proObjectWriter* out, const proObject* obj ) const 
{ 
    out->PutPropValue(GetName(), ToString(obj) ); 
}

// read the object from a reader (unserialize)
void matPropertyColor::Read( proObjectReader* in, proObject* obj ) 
{ 
    string str; 
    in->GetPropValue(GetName(), str); 
    FromString(obj, str); 
}

/**
  * overridden:  builds a proper string for a color
  *
  * inputs:
  * @param inObject - to work from
  *
  * outputs:
  * @return processed display string, or ""
  *
  */
string matPropertyColor::GetDisplayString( proObject* inObject )
{
    return BuildDisplayString( inObject, matDisplayKeyValue );
}

/**
  * overridden:  builds a proper string for a color
  *
  * @param inObject - to work from
  * @return processed display string, or ""
  */
string matPropertyColorInt::GetDisplayString( proObject* inObject )
{
    return BuildDisplayString( inObject, matDisplayKeyValue );
}

// write the object to a writer (serialize)
void matPropertyColorInt::Write( proObjectWriter* out, const proObject* obj ) const 
{ 
    out->PutPropValue(GetName(), ToString(obj) ); 
}

// read the object from a reader (unserialize)
void matPropertyColorInt::Read( proObjectReader* in, proObject* obj ) 
{ 
    string str; 
    in->GetPropValue(GetName(), str); 
    FromString(obj, str); 
}


/**
  * overridden:  builds a proper string for a color
  *
  * inputs:
  * @param inObject - to work from
  *
  * outputs:
  * @return processed display string, or ""
  *
  */
string matPropertyColor::GetPropertySummaryString( proObject* inObject )
{
    return BuildDisplayString( inObject, matPropertySummaryKeyValue );
}


/**
  * overridden:  builds a proper string for a color
  *
  * @param inObject - to work from
  * @return processed display string, or ""
  */
string matPropertyColorInt::GetPropertySummaryString( proObject* inObject )
{
    return BuildDisplayString( inObject, matPropertySummaryKeyValue );
}

string matPropertyColor::BuildDisplayString(
    proObject* inObject, const string& inKeyValue )
{
    char buffer[ 1024 ] = { 0 };

    // find the child property that is a proPropertyStringOwner
    // with the name Display
    for ( int i = 0; i < GetPropertyCount(); ++i )
    {
        proProperty* currentChild = GetPropertyAt( i );

        if ( currentChild->GetName() == inKeyValue && proClassRegistry::InstanceOf(
            currentChild->GetClass(), "proPropertyStringOwner" ) )
        {
            // compute the data format specifier, and verify that
            // it will work for the data type of this property.  Also
            // make sure there is the property number of variable
            // specifiers.
            string formatSpecifier( currentChild->ToString( this ) );
            if
            (
                proConvUtils::proFormatStringMatchesDataType( formatSpecifier, DT_float )
                &&
                proConvUtils::proCountVariableSpecifiers( formatSpecifier ) == 4
            )
            {
                // build the string up based on the actual type of property
                matColor myValue( GetValue( inObject ) );
                sprintf( buffer, formatSpecifier.c_str(),
                    myValue.GetR(), myValue.GetG(),
                    myValue.GetB(), myValue.GetA() );
            }
            else
            {
                sprintf( buffer, "##BADFORMAT##" );
            }

            // stop looping, since we've found the
            // relevant data member
            break;
        }
    }

    // else....
    return string( buffer );
}


string matPropertyColorInt::BuildDisplayString(
    proObject* inObject, const string& inKeyValue )
{
    char buffer[ 1024 ] = { 0 };

    // find the child property that is a proPropertyStringOwner
    // with the name Display
    for ( int i = 0; i < GetPropertyCount(); ++i )
    {
        proProperty* currentChild = GetPropertyAt( i );

        if ( currentChild->GetName() == inKeyValue && proClassRegistry::InstanceOf(
            currentChild->GetClass(), "proPropertyStringOwner" ) )
        {
            // compute the data format specifier, and verify that
            // it will work for the data type of this property.  Also
            // make sure there is the property number of variable
            // specifiers.
            string formatSpecifier( currentChild->ToString( this ) );
            if
            (
                proConvUtils::proFormatStringMatchesDataType( formatSpecifier, DT_int )
                &&
                proConvUtils::proCountVariableSpecifiers( formatSpecifier ) == 4
            )
            {
                // build the string up based on the actual type of property
                matColor myValue( GetValue( inObject ) );
                sprintf( buffer, formatSpecifier.c_str(),
                    myValue.GetR(), myValue.GetG(),
                    myValue.GetB(), myValue.GetA() );
            }
            else
            {
                sprintf( buffer, "##BADFORMAT##" );
            }

            // stop looping, since we've found the
            // relevant data member
            break;
        }
    }

    // else....
    return string( buffer );
}


// write the object to a writer (serialize)
void matPropertyQuaternion::Write( proObjectWriter* out, const proObject* obj ) const 
{ 
    out->PutPropValue(GetName(), ToString(obj) ); 
}

// read the object from a reader (unserialize)
void matPropertyQuaternion::Read( proObjectReader* in, proObject* obj ) 
{ 
    string str; in->GetPropValue(GetName(), str); FromString(obj, str); 
}



/**
 * custom annotations for Color properties
 */
void matPropertyColor::SetAnnotation(const string &name, const string &value)
{
    if (_stricmp(name.c_str(), proAnnoStrDefault) == 0)
    {
        matPropertyColorOwner *newProp = new matPropertyColorOwner();
        newProp->SetName(name);
        AddInstanceProperty(newProp);
        newProp->SetValue(this, matStringUtils::AsColor(value));
    }
    else
    {
        proProperty::SetAnnotation(name, value);
    }
}


matColor matPropertyColor::GetDefault() const 
{ 
    proProperty *prop = GetPropertyByName(proAnnoStrDefault);
    if (prop && proClassRegistry::InstanceOf(prop->GetClass(), "matPropertyColor"))
    {
        matPropertyColor *matProp = (matPropertyColor*)prop;
        return matProp->GetValue(this);
    }
    return matColor::Black; 
}



/**
 * custom annotations for Color properties
 */
void matPropertyColorInt::SetAnnotation(const string &name, const string &value)
{
    if (_stricmp(name.c_str(), proAnnoStrDefault) == 0)
    {
        matPropertyColorIntOwner *newProp = new matPropertyColorIntOwner();
        newProp->SetName(name);
        AddInstanceProperty(newProp);
        newProp->SetValue(this, matStringUtils::AsColorInt(value));
    }
    else
    {
        proProperty::SetAnnotation(name, value);
    }
}


matColorInt matPropertyColorInt::GetDefault() const 
{ 
    proProperty *prop = GetPropertyByName(proAnnoStrDefault);
    if (prop && proClassRegistry::InstanceOf(prop->GetClass(), "matPropertyColorInt"))
    {
        matPropertyColorInt *matProp = (matPropertyColorInt*)prop;
        return matProp->GetValue(this);
    }
    return matColorInt::Black; 
}

/**
 * custom annotations for matrix properties
 */
void matPropertyMatrix::SetAnnotation(const string &name, const string &value)
{
    if (_stricmp(name.c_str(), proAnnoStrDefault) == 0)
    {
        matPropertyMatrixOwner *newProp = new matPropertyMatrixOwner();
        newProp->SetName(name);
        AddInstanceProperty(newProp);
        newProp->SetValue(this, matStringUtils::AsMatrix(value));
    }
    else
    {
        proProperty::SetAnnotation(name, value);
    }
}

matMatrix4x4 matPropertyMatrix::GetDefault() const 
{ 
    proProperty *prop = GetPropertyByName(proAnnoStrDefault);
    if (prop && proClassRegistry::InstanceOf(prop->GetClass(), "matPropertyMatrix"))
    {
        matPropertyMatrix *matProp = (matPropertyMatrix*)prop;
        return matProp->GetValue(this);
    }
    return matMatrix4x4::Identity; 
}

/**
 * custom annotations for vector properties
 */
void matPropertyVector3::SetAnnotation(const string &name, const string &value)
{
    if (_stricmp(name.c_str(), proAnnoStrDefault) == 0)
    {
        matPropertyVector3Owner *newProp = new matPropertyVector3Owner();
        newProp->SetName(name);
        AddInstanceProperty(newProp);
        newProp->SetValue(this, matStringUtils::AsVector3(value));
    }
    else
    {
        proProperty::SetAnnotation(name, value);
    }
}

matVector3 matPropertyVector3::GetDefault() const 
{ 
    proProperty *prop = GetPropertyByName(proAnnoStrDefault);
    if (prop && proClassRegistry::InstanceOf(prop->GetClass(), "matPropertyVector3"))
    {
        matPropertyVector3 *matProp = (matPropertyVector3*)prop;
        return matProp->GetValue(this);
    }
    return matVector3::Zero; 
}


/**
 * custom annotations for quaternion properties
 */
void matPropertyQuaternion::SetAnnotation(const string &name, const string &value)
{
    if (_stricmp(name.c_str(), proAnnoStrDefault) == 0)
    {
        matPropertyQuaternionOwner *newProp = new matPropertyQuaternionOwner();
        newProp->SetName(name);
        AddInstanceProperty(newProp);
        newProp->SetValue(this, matStringUtils::AsQuaternion(value));
    }
    else
    {
        proProperty::SetAnnotation(name, value);
    }
}

matQuaternion matPropertyQuaternion::GetDefault() const 
{ 
    proProperty *prop = GetPropertyByName(proAnnoStrDefault);
    if (prop && proClassRegistry::InstanceOf(prop->GetClass(), "matPropertyQuaternion"))
    {
        matPropertyQuaternion *matProp = (matPropertyQuaternion*)prop;
        return matProp->GetValue(this);
    }
    return matQuaternion::Identity; 
}
//#endif // #if BUILD_PROPERTY_SYSTEM

namespace matStringUtils
{
static char buf[4096];

const char* FindNextDigit( const char* inStart )
{
    const char* cur = inStart;
    while ( cur && *cur )
    {
        if( isdigit(cur[0]) || ( cur[0] =='-' && isdigit(cur[1])) )
        {
            return cur;
        }
        else
        {
            ++cur;
        }
    }

    return NULL;
}

const char* FindNextSeparator( const char* inStart )
{
    // stop when you find white space or a comma
    const char* current = inStart;
    for ( ; current && *current && ( *current != ' ' &&
        *current != '\t' && *current != ',' ); ++current )
    {
        // all work is done inside the loop control condition
    }

    return current;
}

const wchar_t* FindNextDigit( const wchar_t* cur )
{
#if CORE_SYSTEM_WINAPI
    if( cur[0] == NULL && cur[1] == NULL )
        return NULL;

    if( iswdigit(cur[0]) || ( cur[0] =='-' && isdigit(cur[1])) )
    {
        return cur;
    }
    else
    {
        return FindNextDigit( cur + 1 );
    }
#else
    UNREF(cur);
    ERR_UNIMPLEMENTED();
    return NULL;
#endif
}

template <typename OUT_TYPE>
OUT_TYPE ConvertFloatArray(const int size, const string& value)
{
    OUT_TYPE outvar;
    float *curPos = (float*)&outvar;
    const char* cur = value.c_str();

    for(int pos=0; pos < size; ++pos)
    {   
        cur = FindNextDigit(cur);
            if(cur == NULL)
            return outvar;
        *curPos = (float)atof(cur);
        cur = FindNextSeparator( cur );
        ++curPos;
    }
    return outvar;
}


template <typename OUT_TYPE>
OUT_TYPE ConvertIntArray(const int size, const string& value)
{
    OUT_TYPE outvar;
    uint8 *curPos = (uint8*)&outvar;
    const char* cur = value.c_str();

    for(int pos=0; pos < size; ++pos)
    {   
        cur = FindNextDigit(cur);
        if(cur == NULL)
            return outvar;
        *curPos = (uint8)atoi(cur);
        cur = FindNextSeparator( cur );
        ++curPos;
    }
    return outvar;
}

matVector3 AsVector3(const string& value)
{
    return ConvertFloatArray<matVector3>(sizeof(matVector3)/sizeof(matVector3().x), value);
}

matVector3 AsVector3(const wstring& value)
{
    matVector3 retVal;

#if CORE_SYSTEM_WINAPI
    const wchar_t *cur = value.c_str();

    retVal.x = (float)_wtof(cur);

    cur = wcsstr(cur, L",");
    cur = FindNextDigit(cur);
    retVal.y = (float)_wtof(cur);

    cur = wcsstr(cur, L",");
    cur = FindNextDigit(cur);
    retVal.z = (float)_wtof(cur);
#else
    UNREF(value);
    ERR_UNIMPLEMENTED();
#endif

    return retVal;
}

matMatrix4x4 AsMatrix(const string& value)
{
    return ConvertFloatArray<matMatrix4x4>(sizeof(matMatrix4x4)/sizeof(matMatrix4x4()._11), value);
}

matMatrix4x4 AsMatrix(const wstring& value)
{
    matMatrix4x4 mat;
#if CORE_SYSTEM_WINAPI
    const wchar_t* cur = value.c_str();

    for(int row=0; row < 4; ++row)
    {   
        for(int col=0; col < 4; ++col)
        {
            cur = FindNextDigit(cur);
            mat.m[col][row] = (float)_wtof(cur);
            cur = wcsstr(cur, L",");
        }
    }
#else
    UNREF(value);
    ERR_UNIMPLEMENTED();
#endif
    return mat;
}

matQuaternion AsQuaternion(const string& value)
{
    return ConvertFloatArray<matQuaternion>(sizeof(matQuaternion)/sizeof(matQuaternion().x), value);
}

matColor AsColor(const string& value)
{
    return ConvertFloatArray<matColor>(sizeof(matColor)/sizeof(matColor().GetR()), value);
}

template <typename IN_TYPE>
string ConvertFloatArray ( const IN_TYPE& val, const int size )
{
    string ret;
    const float* curPos = (float*)&val;
    for( int i=0; i<size; ++i, ++curPos)
    {
        if ( i != size-1 )
            sprintf(buf, "%f, ", *curPos);
        else
            sprintf(buf, "%f", *curPos);
        ret += buf;
    }
    return ret;
}

template <typename IN_TYPE>
string ConvertIntArray ( const IN_TYPE& val, const int size )
{
    string ret;
    const uint8* curPos = (uint8*)&val;
    for( int i=0; i<size; ++i, ++curPos)
    {
        if ( i != size-1 )
            sprintf(buf, "%d, ", *curPos);
        else
            sprintf(buf, "%d", *curPos);
        ret += buf;
    }
    return ret;
}

matColorInt AsColorInt(const string& value)
{
    return ConvertIntArray<matColorInt>(4, value);
}


string AsString(const matVector3& value)
{
    return ConvertFloatArray<matVector3>(value, sizeof(matVector3)/sizeof(matVector3().x));
}

wstring AsWstring(const matVector3& value)
{
#if CORE_SYSTEM_WINAPI
    swprintf((wchar_t*)buf, L"%f, %f, %f", value.x, value.y, value.z);
    return (wchar_t*)buf;
#else
    UNREF(value);
    ERR_UNIMPLEMENTED();
    return wstring();
#endif
}

string AsString(const matMatrix4x4& value)
{
    return ConvertFloatArray<matMatrix4x4>(value, sizeof(matMatrix4x4)/sizeof(matMatrix4x4()._11));
}

string AsString(const matQuaternion& value)
{
    return ConvertFloatArray<matQuaternion>(value, sizeof(matQuaternion)/sizeof(matQuaternion().x));
}

string AsString(const matColor& value)
{
    return ConvertFloatArray<matColor>(value, sizeof(matColor)/sizeof(matColor().GetR()));
}

string AsString(const matColorInt& value)
{
    return ConvertIntArray<matColorInt>(value, 4);
}

wstring AsWstring(const matMatrix4x4& value)
{
#if CORE_SYSTEM_WINAPI
    swprintf((wchar_t*)buf, L"%f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f ", 
        value._11, value._12, value._13, value._14, 
        value._21, value._22, value._23, value._24, 
        value._31, value._32, value._33, value._34, 
        value._41, value._42, value._43, value._44 );
    return (wchar_t*)buf;
#else
    UNREF(value);
    ERR_UNIMPLEMENTED();
    return wstring();
#endif
}

}; // end namespace matStringUtils


#if BUILD_UNIT_TESTS

#include "arda2/test/tstUnit.h"

// class to test the color property macros
class gfxColorTest : public proObject
{
    PRO_DECLARE_OBJECT
    DECLARE_RGBA_PROPERTY( Color )
};
PRO_REGISTER_CLASS( gfxColorTest, proObject )
REG_PROPERTY_RGBA( gfxColorTest, Color )

// class to test the wrapped color property macros
class gfxWrappedColorTest : public proObject
{
    PRO_DECLARE_OBJECT
    DECLARE_RGBA_PROPERTY_WRAPPED( Color, m_color )
private:
    matColor m_color;
};
PRO_REGISTER_CLASS( gfxWrappedColorTest, proObject )
REG_PROPERTY_RGBA_WRAPPED( gfxWrappedColorTest, Color, m_color )

// testing a classes with two of each property type
class gfx2ColorTest : public proObject
{
    PRO_DECLARE_OBJECT
    DECLARE_RGBA_PROPERTY( Color1 )
    DECLARE_RGBA_PROPERTY( Color2 )
};
PRO_REGISTER_CLASS( gfx2ColorTest, proObject )
REG_PROPERTY_RGBA( gfx2ColorTest, Color1 )
REG_PROPERTY_RGBA( gfx2ColorTest, Color2 )

// class to test a vector property
class testVectorProperty : public proObject
{
    PRO_DECLARE_OBJECT
    DECLARE_XYZ_PROPERTY( Value )
};
PRO_REGISTER_CLASS( testVectorProperty, proObject )
REG_PROPERTY_XYZ( testVectorProperty, Value )

// class to test a wrapped vector property
class testWrappedVectorProperty : public proObject
{
    PRO_DECLARE_OBJECT
    DECLARE_XYZ_PROPERTY_WRAPPED( Value, m_value )
private:
    matVector3 m_value;
};
PRO_REGISTER_CLASS( testWrappedVectorProperty, proObject )
REG_PROPERTY_XYZ_WRAPPED( testWrappedVectorProperty, Value, m_value )

// stuff to test a class with a callback
static int counter = 0;
static void CounterPlusPlus( void )
{
    ++counter;
}
class testCallbackVectorProperty : public proObject
{
    PRO_DECLARE_OBJECT
    DECLARE_XYZ_PROPERTY( Value )
};
PRO_REGISTER_CLASS( testCallbackVectorProperty, proObject )
REG_PROPERTY_XYZ_WITH_CALLBACK( testCallbackVectorProperty, Value, CounterPlusPlus )

class matPropertyTests : public tstUnit
{
public:
    matPropertyTests() { };
    virtual ~matPropertyTests() { };

	virtual void Register()
	{
        SetName("matProperty");

        AddTestCase( "matPropertyTests::TestIndividualGetAndSet",
            &matPropertyTests::TestIndividualGetAndSet );
        AddTestCase( "matPropertyTests::TestRawGetAndSet",
            &matPropertyTests::TestRawGetAndSet );
        AddTestCase( "matPropertyTests::TestTwoProperties",
            &matPropertyTests::TestTwoProperties );
        AddTestCase( "matPropertyTests::testVector",
            &matPropertyTests::testVector );
        AddTestCase( "matPropertyTests::testWrappedVector",
            &matPropertyTests::testWrappedVector );
        AddTestCase( "matPropertyTests::testVectorCallback",
            &matPropertyTests::testWrappedVector );
    }

    void TestIndividualGetAndSet( void ) const
    {
        const float testAlpha = .33f;
        const float testRed =   .44f;
        const float testBlue =  .55f;
        const float testGreen = .66f;

        // test a regular property
        gfxColorTest testColor;

        testColor.SetColorA( testAlpha );
        testColor.SetColorB( testBlue );
        testColor.SetColorG( testGreen );
        testColor.SetColorR( testRed );

        TESTASSERT( testColor.GetColorA() == testAlpha );
        TESTASSERT( testColor.GetColorB() == testBlue );
        TESTASSERT( testColor.GetColorG() == testGreen );
        TESTASSERT( testColor.GetColorR() == testRed );

        // test a wrapped property
        gfxWrappedColorTest testColor2;

        testColor2.SetColorA( testAlpha );
        testColor2.SetColorB( testBlue );
        testColor2.SetColorG( testGreen );
        testColor2.SetColorR( testRed );

        TESTASSERT( testColor2.GetColorA() == testAlpha );
        TESTASSERT( testColor2.GetColorB() == testBlue );
        TESTASSERT( testColor2.GetColorG() == testGreen );
        TESTASSERT( testColor2.GetColorR() == testRed );
    }

    void TestRawGetAndSet( void ) const
    {
        const float testAlpha = .22f;
        const float testRed =   .33f;
        const float testBlue =  .44f;
        const float testGreen = .55f;
        const matColor testColorValue( testRed, testGreen, testBlue, testAlpha );

        // test a regular property
        gfxColorTest testColor;

        testColor.SetColorRaw( testColorValue );

        TESTASSERT( testColorValue == testColor.GetColorRaw() );
        TESTASSERT( testAlpha == testColor.GetColorA() );
        TESTASSERT( testBlue == testColor.GetColorB() );
        TESTASSERT( testGreen == testColor.GetColorG() );
        TESTASSERT( testRed == testColor.GetColorR() );

        // test a wrapped property
        gfxColorTest testColor2;

        testColor2.SetColorRaw( testColorValue );

        TESTASSERT( testColorValue == testColor2.GetColorRaw() );
        TESTASSERT( testAlpha == testColor2.GetColorA() );
        TESTASSERT( testBlue == testColor2.GetColorB() );
        TESTASSERT( testGreen == testColor2.GetColorG() );
        TESTASSERT( testRed == testColor2.GetColorR() );
    }

    void TestTwoProperties( void ) const
    {
        const float testAlpha =  .11f;
        const float testRed =    .22f;
        const float testBlue =   .33f;
        const float testGreen =  .44f;

        const float testAlpha2 = .55f;
        const float testRed2 =   .66f;
        const float testBlue2 =  .77f;
        const float testGreen2 = .88f;

        gfx2ColorTest testColor;
        testColor.SetColor1A( testAlpha );
        testColor.SetColor1B( testBlue );
        testColor.SetColor1G( testGreen );
        testColor.SetColor1R( testRed );
        testColor.SetColor2A( testAlpha2 );
        testColor.SetColor2B( testBlue2 );
        testColor.SetColor2G( testGreen2 );
        testColor.SetColor2R( testRed2 );

        TESTASSERT( testAlpha == testColor.GetColor1A() );
        TESTASSERT( testAlpha2 == testColor.GetColor2A() );
        TESTASSERT( testBlue == testColor.GetColor1B() );
        TESTASSERT( testBlue2 == testColor.GetColor2B() );
        TESTASSERT( testGreen == testColor.GetColor1G() );
        TESTASSERT( testGreen2 == testColor.GetColor2G() );
        TESTASSERT( testRed == testColor.GetColor1R() );
        TESTASSERT( testRed2 == testColor.GetColor2R() );
    }

    void testVector( void ) const
    {
        const float x = 1.2f;
        const float y = 2.3f;
        const float z = 3.4f;
        const float x2 = 2.2f;
        const float y2 = 3.3f;
        const float z2 = 4.4f;
        const matVector3 testVector( x2, y2, z2 );

        // test individual gets and sets
        testVectorProperty test;
        test.SetValueX( x );
        test.SetValueY( y );
        test.SetValueZ( z );
        TESTASSERT( x == test.GetValueX() );
        TESTASSERT( y == test.GetValueY() );
        TESTASSERT( z == test.GetValueZ() );
    }

    void testWrappedVector( void ) const
    {
        const float x = 1.2f;
        const float y = 2.3f;
        const float z = 3.4f;
        const float x2 = 2.2f;
        const float y2 = 3.3f;
        const float z2 = 4.4f;
        const matVector3 testVector( x2, y2, z2 );

        // test individual gets and sets
        testWrappedVectorProperty test;
        test.SetValueX( x );
        test.SetValueY( y );
        test.SetValueZ( z );
        TESTASSERT( x == test.GetValueX() );
        TESTASSERT( y == test.GetValueY() );
        TESTASSERT( z == test.GetValueZ() );
    }

    void testVectorCallback( void ) const
    {
        const float x = 1.2f;
        const float y = 2.3f;
        const float z = 3.4f;
        const float x2 = 2.2f;
        const float y2 = 3.3f;
        const float z2 = 4.4f;
        const matVector3 testVector( x2, y2, z2 );

        // test individual gets and sets
        testCallbackVectorProperty test;
        counter = 0;
        test.SetValueX( x );
        TESTASSERT( counter == 1 );
        test.SetValueY( y );
        TESTASSERT( counter == 2 );
        test.SetValueZ( z );
        TESTASSERT( counter == 3 );
        TESTASSERT( x == test.GetValueX() );
        TESTASSERT( y == test.GetValueY() );
        TESTASSERT( z == test.GetValueZ() );
    }

protected:

private:
};

EXPORTUNITTESTOBJECT(matPropertyTests)

#endif
