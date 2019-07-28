#include "arda2/core/corFirst.h"

#include "proFirst.h"

#include "arda2/util/utlTokenizer.h"
#include "arda2/properties/proProperty.h"
#include "arda2/properties/proStringUtils.h"

using namespace std;

namespace proStringUtils
{

static char buf[4096];

int AsInt(const string &value)
{
    return atoi(value.c_str());
}

int AsInt(const wstring &value)
{
#if CORE_SYSTEM_WINAPI
    return _wtoi(value.c_str());
#else
    UNREF(value);
    ERR_UNIMPLEMENTED();
    return 0;
#endif
}


bool  AsBool(const string &value)
{
    if (_strnicmp(value.c_str(), "true", 4) == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool  AsBool(const wstring &value)
{
#if CORE_SYSTEM_WINAPI
    if (_wcsnicmp(value.c_str(), L"true", 4) == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
#else
    UNREF(value);
    ERR_UNIMPLEMENTED();
    return false;
#endif
}

uint AsUint(const string &value)
{
     return atoi(value.c_str());
}

uint AsUint(const wstring &value)
{
#if CORE_SYSTEM_WINAPI
    return _wtoi(value.c_str());
#else
    UNREF(value);
    ERR_UNIMPLEMENTED();
    return 0;
#endif
}

float AsFloat(const string &value)
{
    return (float)atof(value.c_str());
}

float AsFloat(const wstring &value)
{
#if CORE_SYSTEM_WINAPI
    return (float)_wtof(value.c_str());
#else
    UNREF(value);
    ERR_UNIMPLEMENTED();
    return 0;
#endif
}



string AsString(int value)
{
    sprintf(buf, "%d", value);
    return buf;
}

wstring AsWstring(int value)
{
#if CORE_SYSTEM_WINAPI
    swprintf((wchar_t*)buf, L"%d", value);
    return (wchar_t*)buf;
#else
    UNREF(value);
    ERR_UNIMPLEMENTED();
    return wstring();
#endif
}

string AsString(bool value)
{
    if (value)
        return "true";
    else
        return "false";
}

wstring AsWstring(bool value)
{
    if (value)
        return L"true";
    else
        return L"false";
}

string AsString(uint value)
{
    sprintf(buf, "%d", value);
    return buf;
}

wstring AsWstring(uint value)
{
#if CORE_SYSTEM_WINAPI
    swprintf((wchar_t*)buf, L"%d", value);
    return (wchar_t*)buf;
#else
    UNREF(value);
    ERR_UNIMPLEMENTED();
    return wstring();
#endif
}

string AsString(float value)
{
    sprintf(buf, "%f", value);
    return buf;
}

wstring AsWstring(float value)
{
#if CORE_SYSTEM_WINAPI
    swprintf((wchar_t*)buf, L"%f", value);
    return (wchar_t*)buf;
#else
    UNREF(value);
    ERR_UNIMPLEMENTED();
    return wstring();
#endif
}


const char* FindNextDigit( const char* cur )
{
    if( !cur || cur[0] == 0 )
        return NULL;

    if( isdigit(cur[0]) )
    {
        return cur;
    }
    else
    {
        return FindNextDigit( cur + 1 );
    }
}

const wchar_t* FindNextDigit( const wchar_t* cur )
{
#if CORE_SYSTEM_WINAPI
    if( cur[0] == NULL && cur[1] == NULL )
        return NULL;

    if( iswdigit(cur[0]) )
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

void AsArray( const string& value, vector<int>& v )
{
    const char* cur = value.c_str();

    while( cur != NULL && cur != (value.c_str() + value.length()) )
    {
        cur = FindNextDigit(cur);
        
        if(*cur == 0)
            break;

        v.push_back(atoi(cur));
        cur = strstr(cur, ",");
    }
}

void AsArray( const wstring& value, vector<int>& v )
{
#if CORE_SYSTEM_WINAPI
    const wchar_t* cur = value.c_str();

    while( cur != (value.c_str() + value.length()) )
    {
        cur = FindNextDigit(cur);
        
        if(cur[0] == 0 && cur[1] == 0)
            break;

        v.push_back(_wtoi(cur));
        cur = wcsstr(cur, L",");
    }
#else
    UNREF(value);
    UNREF(v);
    ERR_UNIMPLEMENTED();
#endif
}

void AsArray(const string& value, vector<uint>& v)
{
    const char* cur = value.c_str();

    while( cur != NULL && cur != (value.c_str() + value.length()) )
    {
        cur = FindNextDigit(cur);
        
        if(*cur == 0)
            break;

        v.push_back(atol(cur));
        cur = strstr(cur, ",");
    }
}

void AsArray(const wstring& value, vector<uint>& v)
{
#if CORE_SYSTEM_WINAPI
    const wchar_t* cur = value.c_str();

    while( cur != (value.c_str() + value.length()) )
    {
        cur = FindNextDigit(cur);
        
        if(cur[0] == 0  && cur[1] == 0)
            break;

        v.push_back(_wtol(cur));
        cur = wcsstr(cur, L",");
    }
#else
    UNREF(value);
    UNREF(v);
    ERR_UNIMPLEMENTED();
#endif
}

void AsArray( const string& value, vector<float>& v )
{
    const char* cur = value.c_str();

    while( cur != NULL && cur != (value.c_str() + value.length()) )
    {
        cur = FindNextDigit(cur);
        
        if(*cur == 0)
            break;

        v.push_back((float)atof(cur));
        cur = strstr(cur, ",");
    }
}

void AsArray( const wstring& value, vector<float>& v )
{
#if CORE_SYSTEM_WINAPI
    const wchar_t* cur = value.c_str();

    while( cur != (value.c_str() + value.length()) )
    {
        cur = FindNextDigit(cur);
        
        if(cur[0] == NULL && cur[1] == NULL)
            break;

        v.push_back((float)_wtof(cur));
        cur = wcsstr(cur, L",");
    }
#else
    UNREF(value);
    UNREF(v);
    ERR_UNIMPLEMENTED();
#endif
}

void AsArray ( const string& value, vector<bool>& v )
{
    const char* cur = value.c_str();

    while( cur != NULL && cur != (value.c_str() + value.length()) )
    {
        //cur = FindNextDigit(cur);
        while ( !isalpha( *cur ) )
            cur++;

        if(*cur == 0)
            break;

        if ( strcasecmp(cur, "true") == 0 )
            v.push_back( true );
        else 
            v.push_back( false );

        cur = strstr(cur, ",");
        cur++;
    }
}

void AsArray ( const wstring& value, vector<bool>& v )
{
#if CORE_SYSTEM_WINAPI
    const wchar_t* cur = value.c_str();

    while( cur != NULL && cur != (value.c_str() + value.length()) )
    {
        //cur = FindNextDigit(cur);
        while ( !isalpha( *cur ) )
            cur++;

        if(*cur == NULL)
            break;

        if (  _wcsicmp(cur, L"true") == 0 )
            v.push_back( true );
        else 
            v.push_back( false );

        cur = wcsstr(cur, L",");
        cur++;
    }
#else
    UNREF(value);
    UNREF(v);
    ERR_UNIMPLEMENTED();
#endif
}


string AsString( const int* values, int numVals )
{
    string str;
    for( int i = 0; i<numVals; ++i)
    {
        if( i == numVals - 1) 
            sprintf(buf, "%d", values[i]);
        else
            sprintf(buf, "%d, ", values[i]);
        str += buf;
    }
    return str;
}

wstring AsWstring( const int* values, int numVals )
{
#if CORE_SYSTEM_WINAPI
    wstring str;
    for( int i = 0; i<numVals; ++i)
    {
        if( i == numVals - 1) 
            swprintf((wchar_t*)buf, L"%d", values[i]);
        else
            swprintf((wchar_t*)buf, L"%d, ", values[i]);
        str += (wchar_t*)buf;
    }
    return str;
#else
    UNREF(values);
    UNREF(numVals);
    ERR_UNIMPLEMENTED();
    return wstring();
#endif
}

string  AsString(const float* values, int numVals)
{
    string str;
    for( int i = 0; i<numVals; ++i)
    {
        if( i == numVals - 1) 
            sprintf(buf, "%f", values[i]);
        else
            sprintf(buf, "%f, ", values[i]);
        str += buf;
    }
    return str;
}

wstring AsWstring(const float* values, int numVals)
{
#if CORE_SYSTEM_WINAPI
    wstring str;
    for( int i = 0; i<numVals; ++i)
    {
        if( i == numVals - 1) 
            swprintf((wchar_t*)buf, L"%f", values[i]);
        else
            swprintf((wchar_t*)buf, L"%f, ", values[i]);
        str += (wchar_t*)buf;
    }
    return str;
#else
    UNREF(values);
    UNREF(numVals);
    ERR_UNIMPLEMENTED();
    return wstring();
#endif
}

string  AsString(const uint* values, int numVals)
{
    string str;
    for( int i = 0; i<numVals; ++i)
    {
        if( i == numVals - 1) 
            sprintf(buf, "%u", values[i]);
        else
            sprintf(buf, "%u, ", values[i]);
        str += buf;
    }
    return str;
}

wstring AsWstring(const uint* values, int numVals)
{
#if CORE_SYSTEM_WINAPI
    wstring str;
    for( int i = 0; i<numVals; ++i)
    {
        if( i == numVals - 1) 
            swprintf((wchar_t*)buf, L"%u", values[i]);
        else
            swprintf((wchar_t*)buf, L"%u, ", values[i]);
        str += (wchar_t*)buf;
    }
    return str;
#else
    UNREF(values);
    UNREF(numVals);
    ERR_UNIMPLEMENTED();
    return wstring();
#endif
}

string  AsString(const bool* values, int numVals)
{
    string str;
    for( int i = 0; i<numVals; ++i)
    {
        if( i == numVals - 1) 
            sprintf(buf, "%s", values[i]==true ? "true" : "false");
        else
            sprintf(buf, "%s, ", values[i]==true ? "true" : "false");
        str += buf;
    }
    return str;
}

wstring AsWstring(const bool* values, int numVals)
{
#if CORE_SYSTEM_WINAPI
    wstring str;
    for( int i = 0; i<numVals; ++i)
    {
        if( i == numVals - 1) 
            swprintf((wchar_t*)buf, L"%s", values[i]==true? L"true" : L"false");
        else
            swprintf((wchar_t*)buf, L"%s, ", values[i]==true? L"true" : L"false");
        str += (wchar_t*)buf;
    }
    return str;
#else
    UNREF(values);
    UNREF(numVals);
    ERR_UNIMPLEMENTED();
    return wstring();
#endif
}

}; // namespace proStringUtils


//#if BUILD_PROPERTY_SYSTEM

namespace proConvUtils
{

int AsInt( proObject* o, proProperty* p )
{
    const int32 dt = p->GetDataType();
    if ( dt == DT_int )
    {
        return verify_cast<proPropertyInt*>(p)->GetValue(o);
    }
    else if ( dt == DT_float )
    {
        return (int)( verify_cast<proPropertyFloat*>(p)->GetValue(o) );
    }
    else if ( dt == DT_bool )
    {
        return verify_cast<proPropertyBool*>(p)->GetValue(o) != 0;
    }
    else if ( dt == DT_uint )
    {
        return (int)(verify_cast<proPropertyUint*>(p)->GetValue(o));
    }
    else if ( dt == DT_object )
    {
        return 0;
    }
    else if ( dt == DT_string )
    {
        return proStringUtils::AsInt( verify_cast<proPropertyString*>(p)->GetValue(o) );
    }
    else if( dt == DT_wstring )
    {
        return proStringUtils::AsInt( verify_cast<proPropertyWstring*>(p)->GetValue(o) );
    }
    else if ( dt == DT_enum )
    {
        return (int)( verify_cast<proPropertyInt*>(p)->GetValue(o) );
    }

    ERR_UNIMPLEMENTED(); // something's wrong!
    return 0;
}

bool AsBool( proObject* o, proProperty* p )
{
    const int32 dt = p->GetDataType();
    if ( dt == DT_int )
    {
        return verify_cast<proPropertyInt*>(p)->GetValue(o) != 0;
    }
    else if ( dt == DT_float )
    {
        return verify_cast<proPropertyFloat*>(p)->GetValue(o) != 0;
    }
    else if ( dt == DT_bool )
    {
        return verify_cast<proPropertyBool*>(p)->GetValue(o);
    }
    else if (   dt == DT_uint ||
                dt == DT_object ||
                dt == DT_string ||
                dt == DT_wstring ||
                dt == DT_enum )
    {
        return false;
    }

    ERR_UNIMPLEMENTED(); // something's wrong!
    return 0;
}

uint AsUint( proObject* o, proProperty* p )
{
    const int32 dt = p->GetDataType();
    if ( dt == DT_int )
    {
        return (uint)( verify_cast<proPropertyInt*>(p)->GetValue(o) );
    }
    else if ( dt == DT_float )
    {
        return (uint)( verify_cast<proPropertyFloat*>(p)->GetValue(o) );
    }
    else if ( dt == DT_bool )
    {
        return (uint)( verify_cast<proPropertyBool*>(p)->GetValue(o) );
    }
    else if ( dt == DT_uint )
    {
        return (uint)( verify_cast<proPropertyUint*>(p)->GetValue(o) );
    }
    else if ( dt == DT_object )
    {
        return 0;
    }
    else if ( dt == DT_string )
    {
        return proStringUtils::AsUint( verify_cast<proPropertyString*>(p)->GetValue(o) );
    }
    else if( dt == DT_wstring )
    {
        return proStringUtils::AsUint( verify_cast<proPropertyWstring*>(p)->GetValue(o) );
    }
    else if ( dt == DT_enum )
    {
        return (uint)( verify_cast<proPropertyInt*>(p)->GetValue(o) );
    }

    ERR_UNIMPLEMENTED(); // something's wrong!
    return 0;
}

uint AsDWORD( proObject* o, proProperty* p )
{
    return AsUint(o, p);
}

float AsFloat( proObject* o, proProperty* p )
{
    const int32 dt = p->GetDataType();
    if ( dt == DT_int )
    {
        return (float)( verify_cast<proPropertyInt*>(p)->GetValue(o) );
    }
    else if ( dt == DT_float )
    {
        return (float)( verify_cast<proPropertyFloat*>(p)->GetValue(o) );
    }
    else if ( dt == DT_bool )
    {
        return (float)( verify_cast<proPropertyBool*>(p)->GetValue(o) );
    }
    else if ( dt == DT_uint )
    {
        return 0.0f; // ?
    }
    else if ( dt == DT_object )
    {
        return 0.0f;
    }
    else if ( dt == DT_string )
    {
        return proStringUtils::AsFloat( verify_cast<proPropertyString*>(p)->GetValue(o) );
    }
    else if( dt == DT_wstring )
    {
        return proStringUtils::AsFloat( verify_cast<proPropertyWstring*>(p)->GetValue(o) );
    }
    else if ( dt == DT_enum )
    {
        return (float)( verify_cast<proPropertyInt*>(p)->GetValue(o) );
    }

    ERR_UNIMPLEMENTED(); // something's wrong!
    return 0;
}

string AsString( proObject* o, proProperty* p )
{
    const int32 dt = p->GetDataType();
    if ( dt == DT_int )
    {
        return proStringUtils::AsString( verify_cast<proPropertyInt*>(p)->GetValue(o) );
    }
    else if ( dt == DT_float )
    {
        return proStringUtils::AsString( verify_cast<proPropertyFloat*>(p)->GetValue(o) );
    }
    else if ( dt == DT_bool )
    {
        return proStringUtils::AsString( verify_cast<proPropertyBool*>(p)->GetValue(o) );
    }
    else if ( dt == DT_uint )
    {
        return proStringUtils::AsString( verify_cast<proPropertyUint*>(p)->GetValue(o) );
    }
    else if ( dt == DT_object )
    {
        return "";
    }
    else if ( dt == DT_string )
    {
        return verify_cast<proPropertyString*>(p)->GetValue(o);
    }
    else if ( dt == DT_wstring )
    {
        return "";
    }
    else if ( dt == DT_enum )
    {
        return verify_cast<proPropertyEnum*>(p)->ToString(o);
    }

    ERR_UNIMPLEMENTED(); // something's wrong!
    return 0;
}

wstring AsWstring( proObject* o, proProperty* p )
{
    const int32 dt = p->GetDataType();
    if ( dt == DT_int )
    {
        return proStringUtils::AsWstring( verify_cast<proPropertyInt*>(p)->GetValue(o) );
    }
    else if ( dt == DT_float )
    {
        return proStringUtils::AsWstring( verify_cast<proPropertyFloat*>(p)->GetValue(o) );
    }
    else if ( dt == DT_bool )
    {
        return proStringUtils::AsWstring( verify_cast<proPropertyBool*>(p)->GetValue(o) );
    }
    else if ( dt == DT_uint )
    {
        return proStringUtils::AsWstring( verify_cast<proPropertyUint*>(p)->GetValue(o) );
    }
    else if ( dt == DT_object )
    {
        return L"";
    }
    else if ( dt == DT_string )
    {
        return L"";
    }
    else if ( dt == DT_wstring )
    {
        return verify_cast<proPropertyWstring*>(p)->GetValue(o);
    }
    else if ( dt == DT_enum )
    {
        return L"";
    }

    ERR_UNIMPLEMENTED(); // something's wrong!
    return 0;
}

proObject* AsObject( proObject* o, proProperty* p )
{
    if (!o || !p)
        return NULL;

    const int32 dt = p->GetDataType();
    
    if ( dt == DT_object )
    {
        return verify_cast<proPropertyObject*>(p)->GetValue(o);
    }
    return NULL;
}

void SetValue( proObject* o, proProperty* p, int i )
{
    const int32 dt = p->GetDataType();
    if ( dt == DT_int )
    {
        verify_cast<proPropertyInt*>(p)->SetValue(o, i);
        return;
    }
    else if ( dt == DT_float )
    {
        verify_cast<proPropertyFloat*>(p)->SetValue(o, (float)i);
        return;
    }
    else if ( dt == DT_bool )
    {
        verify_cast<proPropertyBool*>(p)->SetValue(o, i != 0);
        return;
    }
    else if ( dt == DT_uint )
    {
        verify_cast<proPropertyUint*>(p)->SetValue(o, (uint)i);
        return;
    }
    else if ( dt == DT_object )
    {
        return;
    }
    else if ( dt == DT_string )
    {
        verify_cast<proPropertyString*>(p)->SetValue(o, proStringUtils::AsString(i) );
        return;
    }
    else if( dt == DT_wstring )
    {
        verify_cast<proPropertyWstring*>(p)->SetValue(o, proStringUtils::AsWstring(i) );
        return;
    }
    else if ( dt == DT_enum )
    {
        verify_cast<proPropertyEnum*>(p)->SetValue(o, i);
        return;
    }

    ERR_UNIMPLEMENTED(); // something's wrong!
}

void SetValue( proObject* o, proProperty* p, uint d )
{
    const int32 dt = p->GetDataType();
    if ( dt == DT_int )
    {
        verify_cast<proPropertyInt*>(p)->SetValue(o, (int)d);
        return;
    }
    else if ( dt == DT_float )
    {
        verify_cast<proPropertyFloat*>(p)->SetValue(o, (float)d);
        return;
    }
    else if ( dt == DT_bool )
    {
        verify_cast<proPropertyBool*>(p)->SetValue(o, d != 0);
        return;
    }
    else if ( dt == DT_uint )
    {
        verify_cast<proPropertyUint*>(p)->SetValue(o, d);
        return;
    }
    else if ( dt == DT_object )
    {
        return;
    }
    else if ( dt == DT_string )
    {
        verify_cast<proPropertyString*>(p)->SetValue(o, proStringUtils::AsString(d) );
        return;
    }
    else if( dt == DT_wstring )
    {
        verify_cast<proPropertyWstring*>(p)->SetValue(o, proStringUtils::AsWstring(d) );
        return;
    }
    else if ( dt == DT_enum )
    {
        verify_cast<proPropertyEnum*>(p)->SetValue(o, (uint)d);
        return;
    }

    ERR_UNIMPLEMENTED(); // something's wrong!
}

void SetValue( proObject* o, proProperty* p, float f )
{
    const int32 dt = p->GetDataType();
    if ( dt == DT_int )
    {
        verify_cast<proPropertyInt*>(p)->SetValue(o, (int)f);
        return;
    }
    else if ( dt == DT_float )
    {
        verify_cast<proPropertyFloat*>(p)->SetValue(o, f);
        return;
    }
    else if ( dt == DT_bool )
    {
        verify_cast<proPropertyBool*>(p)->SetValue(o, f != 0);
        return;
    }
    else if ( dt == DT_uint )
    {
        return; // ?
    }
    else if ( dt == DT_object )
    {
        return;
    }
    else if ( dt == DT_string )
    {
        verify_cast<proPropertyString*>(p)->SetValue(o, proStringUtils::AsString(f) );
        return;
    }
    else if( dt == DT_wstring )
    {
        verify_cast<proPropertyWstring*>(p)->SetValue(o, proStringUtils::AsWstring(f) );
        return;
    }
    else if ( dt == DT_enum )
    {
        verify_cast<proPropertyEnum*>(p)->SetValue(o, (uint)f);
        return;
    }

    ERR_UNIMPLEMENTED(); // something's wrong!
}

void SetValue( proObject* o, proProperty* p, bool b )
{
    const int32 dt = p->GetDataType();
    if ( dt == DT_int )
    {
        verify_cast<proPropertyInt*>(p)->SetValue(o, (int)b);
        return;
    }
    else if ( dt == DT_float )
    {
        verify_cast<proPropertyFloat*>(p)->SetValue(o,(float)b);
        return;
    }
    else if ( dt == DT_bool )
    {
        verify_cast<proPropertyBool*>(p)->SetValue(o, b);
        return;
    }
    else if ( dt == DT_uint )
    {
        return; // ?
    }
    else if ( dt == DT_object )
    {
        return;
    }
    else if ( dt == DT_string )
    {
        verify_cast<proPropertyString*>(p)->SetValue(o, proStringUtils::AsString(b) );
        return;
    }
    else if( dt == DT_wstring )
    {
        verify_cast<proPropertyWstring*>(p)->SetValue(o, proStringUtils::AsWstring(b) );
        return;
    }
    else if ( dt == DT_enum )
    {
        return; // ?
    }

    ERR_UNIMPLEMENTED(); // something's wrong!
}

void SetValue( proObject* o, proProperty* p, const string &aValue )
{
    const int32 dt = p->GetDataType();
    if ( dt == DT_int )
    {
        p->FromString(o, aValue);
        return;
    }
    else if ( dt == DT_float )
    {
        p->FromString(o, aValue);
        return;
    }
    else if ( dt == DT_bool )
    {
        p->FromString(o, aValue);
        return;
    }
    else if ( dt == DT_uint )
    {
        p->FromString(o, aValue);
        return;
    }
    else if ( dt == DT_object )
    {
        return;
    }
    else if ( dt == DT_string )
    {
        p->FromString(o, aValue);
        return;
    }
    else if( dt == DT_wstring )
    {
        p->FromString(o, aValue);
        return;
    }
    else if ( dt == DT_enum )
    {
        p->FromString(o, aValue);
        return;
    }

    ERR_UNIMPLEMENTED(); // something's wrong!
}

void SetValue( proObject* o, proProperty* p, const wstring &aValue )
{
    const int32 dt = p->GetDataType();
    
    if ( dt == DT_wstring )
    {
        verify_cast<proPropertyWstring*>(p)->SetValue(o, aValue);
        return;
    }
    else if ( dt != DT_other )
    {
        return;
    }

    ERR_UNIMPLEMENTED(); // something's wrong!
}

void SetValue( proObject* o, proProperty* p, proObject *value )
{
    const int32 dt = p->GetDataType();
    
    if ( dt == DT_object )
    {
        verify_cast<proPropertyObject*>(p)->SetValue(o, (proObject*)value);
        return;
    }
    else if (   dt == DT_int ||
                dt == DT_float ||
                dt == DT_bool || 
                dt == DT_uint ||
                dt == DT_string ||
                dt == DT_wstring ||
                dt == DT_enum ||
                dt == DT_bitfield ||
                dt == DT_function )
    {
        return;
    }

    ERR_UNIMPLEMENTED(); // something's wrong!
}

void GetValue( proObject* o, proProperty* p, int& i )
{
    i = AsInt(o,p);
}

void GetValue( proObject* o, proProperty* p, uint& d )
{
    d = AsUint(o,p);
}

void GetValue( proObject* o, proProperty* p, float& f )
{
    f = AsFloat(o,p);
}

void GetValue( proObject* o, proProperty* p, bool& b )
{
    b = AsBool(o,p);
}

void GetValue( proObject* o, proProperty* p, string& aValue )
{
    aValue = AsString(o,p);
}

void GetValue( proObject* o, proProperty* p, wstring& aValue )
{
    aValue = AsWstring(o,p);
}

void GetValue( proObject* o, proProperty* p, proObject** value )
{
    *value = AsObject(o,p);
}


/**
 *  get the value of a property at a speficied address
**/
proObject *GetAddressValue(proObject *root, const string &address)
{
    proObject *result = NULL;

    utlTokenizer tokenizer;
    tokenizer.SetDroppedDelimiters(".");
    tokenizer.Tokenize(address);

    proObject *obj = root;
    proProperty *pro = NULL;

    const string *token = NULL;
    while ( (token = tokenizer.GetNextToken()) != NULL)
    {
        pro = obj->GetClass()->GetPropertyByName(*token);
        if (pro)
        {
            proObject *inner = AsObject(obj, pro);
            if (!inner)
            {
                break;
            }
            else
            {
                obj = inner;
            }
        }
    }
    if (token)
    {
        result = AsObject(obj, pro);
    }
    else
    {
        result = obj;
    }

    return result;
}

/**
 *  change the value of a property at a speficied address from a root object
**/
errResult SetAddressValue(proObject *root, const string &address, const string &value)
{
    utlTokenizer tokenizer;
    tokenizer.SetDroppedDelimiters(".");
    tokenizer.Tokenize(address);

    proObject *obj = root;
    proProperty *pro = NULL;

    const string *token = NULL;
    while ( (token = tokenizer.GetNextToken()) != NULL)
    {
        pro = obj->GetClass()->GetPropertyByName(*token);
        if (pro)
        {
            proObject *inner = AsObject(obj, pro);
            if (!inner)
            {
                break;
            }
            else
            {
                obj = inner;
            }
        }
        else
        {
            return ER_Failure;
        }
    }
    if (token)
    {
        pro->FromString(obj, value);
        return ER_Success;
    }
    return ER_Failure;
}

// analyzes a format specifier and returns a count of all
// the variable specifiers in the string
//
// inputs:
// inString - to parse
//
// outputs:
// a count of the variable specifiers in the string
unsigned int proCountVariableSpecifiers( const string& inString )
{
    unsigned int variablesFound = 0;

    for ( unsigned int i = 0; i < inString.length(); ++i )
    {
        if ( inString[ i ] == '%' )
        {
            ++variablesFound;
        }
    }

    return variablesFound;
}

// returns true if a string format specifier was found, false if otherwise.
// This function will modify the inOut string, trimming off portions that
// are analyzed as it determines whether or not the string is really a proper
// format specifier
//
// returns:
// -1 if not string
// 0 if unable to conclude what type of string
// 1 if string
enum proStringFormatType
{
    eString,
    eNonString,
    eUncertain,
    eInvalid,
};
static proStringFormatType proIsStringFormatSpecifierHelperFunction( const char** inOutString )
{
    // initialize the output
    proStringFormatType result = eUncertain;

    // find the first %
    const char* currentCharacter = *inOutString;
    while ( *currentCharacter && *currentCharacter != '%' )
    {
        ++currentCharacter;
    }
    if ( *currentCharacter )
    {
        // skip the first %
        ++currentCharacter;

        // find the letter specifier after it (remember, there may be some numbers
        // and .'s in between)
        bool hitAPeriod = false;
        while
        (
            *currentCharacter
            &&
            (
                *currentCharacter == '.'
                ||
                (
                    *currentCharacter >= '0'
                    &&
                    *currentCharacter <= '9'
                )
            )
        )
        {
            if ( *currentCharacter == '.' )
            {
                // note when a period is found (a valid string
                // specifier never has a period)
                hitAPeriod = true;
            }

            ++currentCharacter;
        }

        // a string specifier always has a s character, and no periods
        if ( !hitAPeriod && *currentCharacter == 's' )
        {
            result = eString;
        }
        else if ( *currentCharacter == 'i' || *currentCharacter == 'd' ||
            *currentCharacter == 'f' || *currentCharacter == 'x' ||
            *currentCharacter == 'o' || *currentCharacter == 'u' || 
            *currentCharacter == 'e' ||  *currentCharacter == 'g' )
        {
            // any of the specifiers above is safe for a non-string type
            result = eNonString;
        }
        else
        {
            // we've been handed junk
            result = eInvalid;
        }
        ++currentCharacter; // skip the type
        *inOutString = currentCharacter; // update the inOut parameter
    }
    else
    {
        // let the caller know we've hit the end of the string
        *inOutString = currentCharacter;
    }

    return result;
}

// trying to use a string format specifier for a non-string
// variable will cause a crash.  This function will analyze
// the specified format specifier string and return true if
// all of the format specifiers found are for string literals
//
// inputs:
// inString - the format specifier to analyze
//
// outputs:
// true if the first specifier is for a string, false
// if otherwise
bool proIsStringFormatSpecifier( const string& inString )
{
    const char* currentCharacter = inString.c_str();
    proStringFormatType isString = eString;
    while ( *currentCharacter && isString == eString )
    {
        // find the next string specifier
         proStringFormatType result =
             proIsStringFormatSpecifierHelperFunction( &currentCharacter );
         if ( *currentCharacter || ( !*currentCharacter && result != eUncertain ) )
         {
             isString = result;
         }
    }

    return isString == eString;
}

// confirms if the entire format specifier contains
// non-string parameters
//
// inputs:
// inString - the format specifier to analyze
//
// outputs:
// true if the first specifier is for a string, false
// if otherwise
bool proIsNonStringFormatSpecifier( const string& inString )
{
    const char* currentCharacter = inString.c_str();
    proStringFormatType isString = eNonString;
    while ( *currentCharacter && isString == eNonString )
    {
        // find the next string specifier
         proStringFormatType result =
             proIsStringFormatSpecifierHelperFunction( &currentCharacter );
         if ( *currentCharacter || ( !*currentCharacter && result != eUncertain ) )
         {
             isString = result;
         }
    }

    return isString == eNonString;
}

// returns true if the format specifier string is a valid
// string for the specified data type
//
// inputs:
// inString - to analyze
// inDataType - to compare against
//
// outputs:
// true if the format string will produce valid output
// for the specified data type, false if otherwise
bool proFormatStringMatchesDataType(
    const string& inString, proDataType inDataType )
{
    // don't allow strings to be printed in non-strings,
    // and vice versa.  Also, objects, etc. will always
    // be submitted as strings.
    bool isStringFormat =    proIsStringFormatSpecifier( inString );
    bool isNonStringFormat = proIsNonStringFormatSpecifier( inString );
    bool needsStringFormat = inDataType == DT_string ||
        inDataType == DT_wstring || inDataType == DT_object ||
        inDataType == DT_enum || inDataType == DT_function ||
        inDataType == DT_other;

    return ( ( isStringFormat && needsStringFormat ) ||
        ( isNonStringFormat && !needsStringFormat ) );
}

}; // namespace proConvUtils

//#endif // #if BUILD_PROPERTY_SYSTEM
