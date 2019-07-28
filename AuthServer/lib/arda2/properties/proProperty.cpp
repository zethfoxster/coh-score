#include "arda2/core/corFirst.h"
#include "arda2/core/corStlAlgorithm.h"

#include "arda2/properties/proFirst.h"
#include "arda2/properties/proProperty.h"
#include "arda2/properties/proClass.h"
#include "arda2/properties/proClassNative.h"
#include "arda2/properties/proPropertyNative.h"
#include "arda2/properties/proPropertyOwner.h"
#include "arda2/properties/proStringUtils.h"
#include "arda2/properties/proObjectWriter.h"
#include "arda2/properties/proObjectReader.h"

#include "arda2/util/utlTokenizer.h"

using namespace std;

PRO_REGISTER_ABSTRACT_CLASS(proProperty, proObject)
REG_PROPERTY_STRING(proProperty, Name)
REG_PROPERTY_BOOL(proProperty, IsNative)

PRO_REGISTER_ABSTRACT_CLASS(proPropertyInt, proProperty)
PRO_REGISTER_ABSTRACT_CLASS(proPropertyUint, proProperty)
PRO_REGISTER_ABSTRACT_CLASS(proPropertyFloat, proProperty)
PRO_REGISTER_ABSTRACT_CLASS(proPropertyBool, proProperty)
PRO_REGISTER_ABSTRACT_CLASS(proPropertyString, proProperty)
PRO_REGISTER_ABSTRACT_CLASS(proPropertyWstring, proProperty)
PRO_REGISTER_ABSTRACT_CLASS(proPropertyObject, proProperty)
PRO_REGISTER_ABSTRACT_CLASS(proPropertyEnum, proProperty)
PRO_REGISTER_ABSTRACT_CLASS(proPropertyBitfield, proProperty)

typedef proDefaultAnnotation<proPropertyIntNative, int, const int> proDefaultAnnotationInt;
typedef proDefaultAnnotation<proPropertyUintNative, uint, const uint> proDefaultAnnotationUint;
typedef proDefaultAnnotation<proPropertyFloatNative, float, const float> proDefaultAnnotationFloat;
typedef proDefaultAnnotation<proPropertyBoolNative, bool, const bool> proDefaultAnnotationBool;
typedef proDefaultAnnotation<proPropertyStringNative, string, const string&> proDefaultAnnotationString;

const char *proAnnoStrDisplay = "Display";
const char* proAnnoStrInlineDisplay = "DisplayInline";
const char *proAnnoStrDefault = "Default";
const char *proAnnoStrMin = "Min";
const char *proAnnoStrMax = "Max";
const char *proAnnoStrReadonly = "Readonly";
const char *proAnnoStrTransient = "Transient";
const char *proAnnoStrValidator = "Validator";
const char *proAnnoStrObjectType = "ObjectType";

/**
 * utility function for parsing comma delimited strings of pairs
 */
void ParseStringPairs(const char *input, vector<string> &items, const char *delimiters)
{
   if (input == NULL || *input == 0)
        return;

    string data;
    // strip quotes if requried
    if (input[0] == '"')
    {
        char buf[4096];
        strcpy(buf, &input[1]);
        char *p = strrchr(buf, '"');
        if (p)
            *p = '\0';
        data = buf;
    }
    else
    {
        data = input;
    }

    utlTokenizer tokenizer;
    tokenizer.SetDroppedDelimiters(delimiters);
    tokenizer.Tokenize(data);

    int len = (int)(tokenizer.GetTokenCount() / 2);
    for (int i=0; i != len; i++)
    {
        const string *nameStr = tokenizer.GetNextToken();
        const string *valueStr = tokenizer.GetNextToken();

        items.push_back( *nameStr );
        items.push_back( *valueStr );
    }
}

/**
 * called by property registration macros to set the annotations
 * for properties. uses the internal SetAnnotation() method.
 * @param annoString string of the form "name|value|name|value"
 */
void proProperty::SetAnnotations(const char *annoString)
{
    if (annoString == NULL)
        return;

    vector<string> items;
    ParseStringPairs(annoString, items, "|");
    if (items.size() == 0 )
        return;

    for (int i=0; i != (int)items.size(); i+=2)
    {
        string &name = items[i];
        string &value = items[i+1];
        SetAnnotation(name, value);
    }
}

/**
 * handler for adding individual property annotations. used in derived
 * property classes to implement custom annotations.
 */
void proProperty::SetAnnotation(const string &name, const string &value)
{
    if (_stricmp(name.c_str(), proAnnoStrReadonly) == 0  || _stricmp(name.c_str(), proAnnoStrTransient) == 0)
    {
        proPropertyBoolOwner *newProp = new proPropertyBoolOwner();
        newProp->SetName(name);
        newProp->SetValue(this, proStringUtils::AsBool(value));
        AddInstanceProperty(newProp);
    }
    else
    {
        // get rid of any old annotations that match the new value
        proProperty* foundIt = GetPropertyByName( name );
        if ( foundIt )
        {
            ERR_REPORTV(ES_Warning, ("Duplicate annotation %s for %s", name.c_str(), GetClass()->GetName().c_str()));

            bool succeeded = RemoveInstanceProperty( foundIt );
            ERR_CHECK( succeeded, "Unable to remove annotation." );
        }

        // add the new value
        proPropertyStringOwner *newProp = new proPropertyStringOwner();
        newProp->SetName(name);
        newProp->SetValue(this, value);
        AddInstanceProperty(newProp);
    }
}

/**
  * returns the computed display string, or ""
  * if there is no display string
  *
  * inputs:
  * @param inObject - to work from
  *
  * outputs:
  * @return processed display string, or ""
  *
  */
string proProperty::GetDisplayString( proObject* inObject )
{
    return ParsePropertyString( inObject, proAnnoStrDisplay );
}

/**
  * contains the shared code of the GetDisplayString() and
  * GetPropertySummaryString() functions.
  *
  * inputs:
  * @param inObject - to parse from
  * @param inKeyValue - name of the display string annotation to search for
  *
  * outputs:
  * @return contents of the property as a omatted string, or ""
  * @return if the value cannot be computed
  *
  */
string proProperty::ParsePropertyString( proObject* inObject, const string& inKeyValue )
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
                proConvUtils::proFormatStringMatchesDataType( formatSpecifier, GetDataType() )
                &&
                proConvUtils::proCountVariableSpecifiers( formatSpecifier ) == 1
            )
            {
                // build the string up based on the actual type of property
                // that this class represents
                switch ( GetDataType() )
                {
                case DT_int:
                    sprintf( buffer, formatSpecifier.c_str(),
                        ( ( proPropertyInt* )this )->GetValue( inObject ) );
                    break;
                case DT_uint:
                    sprintf( buffer, formatSpecifier.c_str(),
                        ( ( proPropertyUint* )this )->GetValue( inObject ) );
                    break;
                case DT_float:
                    sprintf( buffer, formatSpecifier.c_str(),
                        ( ( proPropertyFloat* )this )->GetValue( inObject ) );
                    break;
                case DT_bool:
                    sprintf( buffer, formatSpecifier.c_str(),
                        ( ( proPropertyBool* )this )->GetValue( inObject ) );
                    break;
                case DT_string:
                    sprintf( buffer, formatSpecifier.c_str(),
                        ( ( proPropertyString* )this )->GetValue( inObject ).c_str() );
                    break;
                case DT_wstring:
                    sprintf( buffer, formatSpecifier.c_str(),
                        ( ( proPropertyWstring* )this )->GetValue( inObject ).c_str() );
                    break;
                case DT_object:
                case DT_enum:
                case DT_function:
                case DT_other:
                default:
                    sprintf( buffer, formatSpecifier.c_str(),
                        ToString( inObject ).c_str() );
                    break;
                }
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

/**
  * returns the computed property summary string, or ""
  * if there is no property summary string
  *
  * inputs:
  * @param inObject - to work from
  *
  * outputs:
  * @return processed summary string, or ""
  *
  */
string proProperty::GetPropertySummaryString( proObject* inObject )
{
    return ParsePropertyString( inObject, proAnnoStrInlineDisplay );
}

/**
  * returns true if this property is read-only
  *
  * inputs:
  * inProperty - the property to test
  *
  * outputs:
  * true if the property is read-only, false
  * if otherwise
  *
  **/
bool proIsReadOnly( const proProperty* inProperty )
{
    // look for a read only annotation, and return that value
    for ( int i = 0; i < inProperty->GetPropertyCount(); ++i )
    {
        proProperty* testProperty = inProperty->GetPropertyAt( i );
        if ( testProperty->GetDataType() == DT_bool &&
            testProperty->GetName() == proAnnoStrReadonly )
        {
            proPropertyBoolOwner* targetProperty =
                static_cast< proPropertyBoolOwner* >( testProperty );
            return targetProperty->GetValue( inProperty );
        }
    }

    // if none is found, it must not be read-only
    return false;
}


// write the object to a writer (serialize)
void proPropertyString::Write(proObjectWriter* out, const proObject* obj) const 
{ 
    out->PutPropValue(GetName(), GetValue(obj)); 
}

// read the object from a reader (unserialize)
void proPropertyString::Read(proObjectReader* in, proObject* obj) 
{ 
    string v = m_default;
    in->GetPropValue(GetName(), v); 
    SetValue(obj, v); 
}


// write the object to a writer (serialize)
void proPropertyInt::Write(proObjectWriter* out, const proObject* obj) const 
{ 
    out->PutPropValue(GetName(), GetValue(obj)); 
}

// read the object from a reader (unserialize)
void proPropertyInt::Read(proObjectReader* in, proObject* obj) 
{ 
    int v = m_default;
    in->GetPropValue(GetName(), v); 
    SetValue(obj, v); 
}


// write the object to a writer (serialize)
void proPropertyUint::Write(proObjectWriter* out, const proObject* obj) const 
{ 
    out->PutPropValue(GetName(), GetValue(obj)); 
}

// read the object from a reader (unserialize)
void proPropertyUint::Read(proObjectReader* in, proObject* obj) 
{ 
    uint v = m_default; 
    in->GetPropValue(GetName(), v); 
    SetValue(obj, v); 
}

// write the object to a writer (serialize)
void proPropertyFloat::Write(proObjectWriter* out, const proObject* obj) const 
{ 
    out->PutPropValue(GetName(), GetValue(obj)); 
}

// read the object from a reader (unserialize)
void proPropertyFloat::Read(proObjectReader* in, proObject* obj) 
{ 
    float v = m_default;
    in->GetPropValue(GetName(), v); 
    SetValue(obj, v); 
}


// write the object to a writer (serialize)
void proPropertyBool::Write(proObjectWriter* out, const proObject* obj) const 
{ 
    out->PutPropValue(GetName(), GetValue(obj)); 
}

// read the object from a reader (unserialize)
void proPropertyBool::Read(proObjectReader* in, proObject* obj) 
{ 
    bool v = m_default;
    in->GetPropValue(GetName(), v); 
    SetValue(obj, v); 
}


// write the object to a writer (serialize)
void proPropertyWstring::Write(proObjectWriter* out, const proObject* obj) const 
{ 
    out->PutPropValue(GetName(), GetValue(obj)); 
}

// read the object from a reader (unserialize)
void proPropertyWstring::Read(proObjectReader* in, proObject* obj) 
{ 
    wstring v = m_default;;
    in->GetPropValue(GetName(), v); 
    SetValue(obj, v); 
}

// write the object to a writer (serialize)
void proPropertyObject::Write(proObjectWriter* out, const proObject* obj) const 
{ 
    out->PutPropValue(GetName(), GetValue(obj)); 
}

// read the object from a reader (unserialize)
void proPropertyObject::Read(proObjectReader* in, proObject* obj) 
{
    DP_OWN_VALUE owner;
    proObject* o = 0;
    in->GetPropValue(GetName(), o, owner);
    SetValue(obj, o);
}

//-----------------------------------------------------------------------------
// proPropertyEnum

// write the object to a writer (serialize)
void proPropertyEnum::Write(proObjectWriter* out, const proObject* obj) const 
{ 
    out->PutPropValue(GetName(), GetValue(obj)); 
}

// read the object from a reader (unserialize)
void proPropertyEnum::Read(proObjectReader* in, proObject* obj) 
{ 
    uint v = m_default;
    in->GetPropValue(GetName(), v); 
    SetValue(obj, v); 
}


void proPropertyEnum::SetEnumString(const string &anEnum)
{
    vector<string> items;
    ParseStringPairs(anEnum.c_str(), items, ",");
    if (items.size() == 0 )
        return;

    for (int i=0; i != (int)items.size(); i+=2)
    {
        string &name = items[i];
        string &value = items[i+1];

        m_enumeration.insert( pair<string,uint>(name, proStringUtils::AsUint(value)) );        
    }
}

void proPropertyEnum::SetEnumTable(const EnumTable *aTable, uint aTableSize)
{
	// this function is note meant to be used per frame or for any performance

	string enumString;
	string comma = ",";
	char number[64];

	for( uint i = 0; i < aTableSize; ++i )
	{
		enumString += aTable[i].key;
		enumString += ",";

        // can use itoa?
        sprintf(number, "%ul", aTable[i].value);
		enumString += number;
		enumString += comma;
	}

	SetEnumString( enumString );
}

uint proPropertyEnum::GetEnumerationValue(const string &astring) const 
{ 
    map<string, uint>::const_iterator i = m_enumeration.find(astring);
    if (i != m_enumeration.end())
    {
        return (*i).second;
    }
    return 0;
}

void proPropertyEnum::GetEnumeration(std::vector<string> &value)  const
{ 
    for(map<string, uint>::const_iterator e = m_enumeration.begin(); e != m_enumeration.end(); e++ )
    {
        // insert into value in sorted order (sort by numeric value associated with the string)
        bool found = false;
        for (std::vector<string>::iterator it = value.begin(); it != value.end(); it++)
        {
            string &name = (*it);
            uint number = GetEnumerationValue(name);
            if (e->second < number)
            {
                value.insert(it, e->first);
                found = true;
                break;
            }
        }
        if (!found)
            value.push_back(e->first);
    }
}

string proPropertyEnum::GetEnumerationValue(uint value) const
{
    for(map<string, uint>::const_iterator e = m_enumeration.begin(); e != m_enumeration.end(); e++ )
    {
        if ((*e).second == value)
        {
            return (*e).first;
        }
    }
    return "no found";
}

//-----------------------------------------------------------------------------
// proPropertyBitfield

// write the object to a writer (serialize)
void proPropertyBitfield::Write(proObjectWriter* out, const proObject* obj) const 
{ 
    out->PutPropValue(GetName(), GetValue(obj)); 
}

// read the object from a reader (unserialize)
void proPropertyBitfield::Read(proObjectReader* in, proObject* obj) 
{ 
    uint v = m_default;
    in->GetPropValue(GetName(), v); 
    SetValue(obj, v); 
}


void proPropertyBitfield::SetBitfieldString(const string &anEnum)
{
    m_bitfields.clear();

    vector<string> items;
    ParseStringPairs(anEnum.c_str(), items, ",");
    if (items.size() == 0 )
        return;

    for (int i=0; i != (int)items.size(); i+=2)
    {
        string &name = items[i];
        string &value = items[i+1];

        uint bit = (1 << proStringUtils::AsUint(value));
        m_bitfields.insert(pair<string, uint>(name, bit));
    }
}

uint proPropertyBitfield::GetBitfieldValue(const string &astring) const 
{ 
    // Separate bitfield string into individuals parts
    utlTokenizer tokenizer;
    tokenizer.SetDroppedDelimiters("|");
    tokenizer.Tokenize(astring);

    // Accumulate bits
    uint bits = 0;
    const int numTokens = (int)tokenizer.GetTokenCount();

    for (int i = 0; i < numTokens; ++i)
    {
        const string* token = tokenizer.GetNextToken();
        map<string, uint>::const_iterator iter = m_bitfields.find(*token);

        if (iter != m_bitfields.end())
        {
            bits |= (*iter).second;
        }
    }

    return bits;
}

void proPropertyBitfield::GetBitfield(std::vector<string> &value)  const
{ 
    for (map<string, uint>::const_iterator e = m_bitfields.begin(); e != m_bitfields.end(); e++ )
        value.push_back(e->first);
}

string proPropertyBitfield::GetBitfieldValue(uint value) const
{
    // Build bitfield string from flags
    string astring = "";

    for (map<string, uint>::const_iterator e = m_bitfields.begin(); e != m_bitfields.end(); ++e)
    {
        if ((*e).second & value)
        {
            astring += "|" + (*e).first;
        }
    }

    // Remove initial separator
    if (!astring.empty())
    {
        astring.erase(0, 1);
    }

    return astring;
}

//-----------------------------------------------------------------------------
// SetAnnotation implementations

/**
 * custom annotations for Int properties
 */
void proPropertyInt::SetAnnotation(const string &name, const string &value)
{
  if (_stricmp(name.c_str(), proAnnoStrMin) == 0 || _stricmp(name.c_str(), proAnnoStrMax) == 0)
    {
        proPropertyIntOwner *newProp = new proPropertyIntOwner();
        newProp->SetName(name);
        newProp->SetValue(this, proStringUtils::AsInt(value));
        AddInstanceProperty(newProp);
    }
    else if (_stricmp(name.c_str(), proAnnoStrDefault) == 0)
    {
        proDefaultAnnotationInt *newProp = new proDefaultAnnotationInt();
        m_default = proStringUtils::AsInt(value);
        AddInstanceProperty(newProp);
    }
    else
    {
        proProperty::SetAnnotation(name, value);
    }
}

/**
 * custom annotations for Uint properties
 */
void proPropertyUint::SetAnnotation(const string &name, const string &value)
{
    if (_stricmp(name.c_str(), proAnnoStrMin) == 0  || _stricmp(name.c_str(), proAnnoStrMax) == 0)
    {
        proPropertyUintOwner *newProp = new proPropertyUintOwner();
        newProp->SetName(name);
        newProp->SetValue(this, proStringUtils::AsUint(value));
        AddInstanceProperty(newProp);
    }
    else if (_stricmp(name.c_str(), proAnnoStrDefault) == 0)
    {
        proDefaultAnnotationUint *newProp = new proDefaultAnnotationUint();
        m_default = proStringUtils::AsUint(value);
        AddInstanceProperty(newProp);
    }
    else
    {
        proProperty::SetAnnotation(name, value);
    }
}

/**
 * custom annotations for Float properties
 */
void proPropertyFloat::SetAnnotation(const string &name, const string &value)
{
    if (_stricmp(name.c_str(), proAnnoStrMin) == 0  || _stricmp(name.c_str(), proAnnoStrMax) == 0)
    {
        proPropertyFloatOwner *newProp = new proPropertyFloatOwner();
        newProp->SetName(name);
        newProp->SetValue(this, proStringUtils::AsFloat(value));
        AddInstanceProperty(newProp);
    }
    else if (_stricmp(name.c_str(), proAnnoStrDefault) == 0)
    {
        proDefaultAnnotationFloat *newProp = new proDefaultAnnotationFloat();
        m_default = proStringUtils::AsFloat(value);
        AddInstanceProperty(newProp);
    }
    else
    {
        proProperty::SetAnnotation(name, value);
    }
}

/**
 * custom annotations for Bool properties
 */
void proPropertyBool::SetAnnotation(const string &name, const string &value)
{
    if (_stricmp(name.c_str(), proAnnoStrDefault) == 0)
    {
        proDefaultAnnotationBool *newProp = new proDefaultAnnotationBool();
        m_default = proStringUtils::AsBool(value);
        AddInstanceProperty(newProp);
    }
    else
    {
        proProperty::SetAnnotation(name, value);
    }
}

/**
 * custom annotations for String properties
 */
void proPropertyString::SetAnnotation(const string &name, const string &value)
{
    if (_stricmp(name.c_str(), proAnnoStrDefault) == 0)
    {
        proDefaultAnnotationString *newProp = new proDefaultAnnotationString();
        m_default = value;
        AddInstanceProperty(newProp);
    }
    else
    {
        proProperty::SetAnnotation(name, value);
    }
}
/**
 * custom annotations for object properties
 */
void proPropertyObject::SetAnnotation(const string &name, const string &value)
{
    proProperty::SetAnnotation(name, value);
}

/**
 * custom annotations for Wstring properties
 */
void proPropertyWstring::SetAnnotation(const string &name, const string &value)
{
    proProperty::SetAnnotation(name, value);
}

/**
 * custom annotations for enum properties
 */
void proPropertyEnum::SetAnnotation(const string &name, const string &value)
{
    if (_stricmp(name.c_str(), proAnnoStrDefault) == 0)
    {
        bool isNumber = false;

        // see if all the chars in value are all digits
        for( string::const_iterator it = value.begin(); it != value.end(); it++ )
        {
            if( !isdigit(*it) )
            {
                isNumber = false;
                break;
            }
        }

        // if this is a non-number (i.e. a string) try to turn this into a UINT value
        if( !isNumber )
        {
            proDefaultAnnotationUint *newProp = new proDefaultAnnotationUint();
            m_default = proPropertyEnum::GetEnumerationValue( value );
            AddInstanceProperty(newProp);
            return;
        }
    }

    // default
    proPropertyUint::SetAnnotation( name, value );
}
