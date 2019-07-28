/**
 *  string utilities for the property system
 *
 *  @author Sean Riley
 *
 *  @date 1/13/2003
 *
 *  @file
**/

#ifndef __PROSTRING_UTILS__
#define __PROSTRING_UTILS__

#include "proFirst.h"

#include "arda2/core/corStdString.h"
#include "arda2/core/corStlVector.h"
#include "arda2/properties/proClass.h"
#include "arda2/properties/proProperty.h"

#if CORE_SYSTEM_LINUX
#include "ctype.h"
#endif

namespace proStringUtils
{
    int     AsInt(const std::string &value);
    bool    AsBool(const std::string &value);
    uint    AsUint(const std::string &value);
    float   AsFloat(const std::string &value);

    int     AsInt(const std::wstring &value);
    bool    AsBool(const std::wstring &value);
    uint    AsUint(const std::wstring &value);
    float   AsFloat(const std::wstring &value);

    void    AsArray(const std::string& value, std::vector<int>& v);
    void    AsArray(const std::wstring& value, std::vector<int>& v);
    void    AsArray(const std::string& value, std::vector<uint>& v);
    void    AsArray(const std::wstring& value, std::vector<uint>& v);
    void    AsArray(const std::string& value, std::vector<float>& v);
    void    AsArray(const std::wstring& value, std::vector<float>& v);
    void    AsArray(const std::string& value, std::vector<bool>& v);
    void    AsArray(const std::wstring& value, std::vector<bool>& v);

    std::string  AsString(int value);
    std::string  AsString(bool value);
    std::string  AsString(uint value);
    std::string  AsString(float value);

    std::wstring AsWstring(int value);
    std::wstring AsWstring(bool value);
    std::wstring AsWstring(uint value);
    std::wstring AsWstring(float value);

    std::string  AsString(const int* values, int numVals);
    std::wstring AsWstring(const int* values, int numVals);
    std::string  AsString(const uint* values, int numVals);
    std::wstring AsWstring(const uint* values, int numVals);
    std::string  AsString(const float* values, int numVals);
    std::wstring AsWstring(const float* values, int numVals);
    std::string  AsString(const bool* values, int numVals);
    std::wstring AsWstring(const bool* values, int numVals);
};


class proObject;
class proProperty;

// this set of functions currently follows certain idioms of conversion
// that the pandora project's legacy code uses. Make sure you agree with them
// if you use these.
namespace proConvUtils
{
    int         AsInt( proObject* o, proProperty* p );
    bool        AsBool( proObject* o, proProperty* p );
    uint        AsUint( proObject* o, proProperty* p );
    uint        AsDWORD( proObject* o, proProperty* p );
    float       AsFloat( proObject* o, proProperty* p );
    std::string      AsString( proObject* o, proProperty* p );
    std::wstring     AsWstring( proObject* o, proProperty* p );
    proObject*  AsObject( proObject* o, proProperty* p );

    void        SetValue( proObject* o, proProperty* p, int i );
    void        SetValue( proObject* o, proProperty* p, uint d );
    void        SetValue( proObject* o, proProperty* p, float f );
    void        SetValue( proObject* o, proProperty* p, bool b );
    void        SetValue( proObject* o, proProperty* p, const std::string &aValue );
    void        SetValue( proObject* o, proProperty* p, const std::wstring &aValue );
    void        SetValue( proObject* o, proProperty* p, proObject *value );

    void        GetValue( proObject* o, proProperty* p, int& i );
    void        GetValue( proObject* o, proProperty* p, uint& d );
    void        GetValue( proObject* o, proProperty* p, float& f );
    void        GetValue( proObject* o, proProperty* p, bool& b );
    void        GetValue( proObject* o, proProperty* p, std::string& aValue );
    void        GetValue( proObject* o, proProperty* p, std::wstring& aValue );
    void        GetValue( proObject* o, proProperty* p, proObject** value );

    // set the value of a property at an address (by string)
    errResult   SetAddressValue(proObject *root, const std::string &address, const std::string &value);

    // get the value of a property at an address (object)
    proObject *GetAddressValue(proObject *root, const std::string &address);


    // find an object of type T in the tree 
    template <typename T> std::string FindTypedObject(
                            IN const char *className, 
                            IN proObject *root, 
                            IN T *target,
                            OUT proObject *&containerObj,
                            OUT proProperty *&containerProp)
    {
        for (int i=0; i != root->GetPropertyCount(); i++)
        {
            proProperty *pro = root->GetPropertyAt(i);
            proObject *obj = AsObject(root, pro);
            if (obj && proClassRegistry::InstanceOf(target->GetClass(), className))
            {
                if (obj == target)
                {
                    containerObj = root;
                    containerProp = pro;
                    return pro->GetName();
                }
            }

            if (pro->GetDataType() == DT_object )
            {
                proObject *innerObject = AsObject(root, pro);

                std::string address = FindTypedObject<T>(className, innerObject, target, containerObj, containerProp);
                if (address.size() != 0)
                {
                    return pro->GetName() + "." + address;
                }
            }
        }
        return "";
    }
    /** finds all instances of the template class type
      * within the specified object tree
      *
      * @param inRoot - to search
      * @param outFoundList - found objects will be stored here
      */
    template< class TargetClass > void FindClassObjectsInTree(
                    const char *className,
                    proObject* inRoot, 
                    std::vector<TargetClass*>& outFoundList)
    {
        // don't empty the incoming list (assume the caller has done that already)

        if (!inRoot)
            return;

        for ( int i = 0; i != inRoot->GetPropertyCount(); ++i )
        {
            // compute the property to target
            proProperty* targetProperty = inRoot->GetPropertyAt( i );

            // add all matching objects to the list
            proObject *target = proConvUtils::AsObject(inRoot, targetProperty);
            if (target && proClassRegistry::InstanceOf(target->GetClass(), className))
            {
                TargetClass* nextObject = static_cast<TargetClass*>(target);
                outFoundList.push_back( nextObject );
            }

            // when a subparent is found, parse it
            if ( targetProperty->GetDataType() == DT_object)
            {
                FindClassObjectsInTree< TargetClass >(
                    className,
                    AsObject( inRoot, targetProperty ), 
                    outFoundList );
            }
        }

        // stop at the end of the tree
    }


    template <typename T>
    T *proAsTypedObjectFunc(proObject *object, proProperty *property, const char *className)
    {
        proObject *target = proConvUtils::AsObject(object, property);
        if (target && proClassRegistry::InstanceOf(target->GetClass(), className))
        {
            return static_cast<T*>(target);
        }
        return NULL;
    }

    // API to analyze printf-style format strings
    unsigned int proCountVariableSpecifiers( const std::string& );
    bool proIsStringFormatSpecifier( const std::string& );
    bool proIsNonStringFormatSpecifier( const std::string& );
    bool proFormatStringMatchesDataType( const std::string&, proDataType );
};


// macro to help calling proAsTypedObjectFunc without duplicating the class name
#define PRO_AS_TYPED_OBJECT(CLASS, OBJECT, PROPERTY) \
    proConvUtils::proAsTypedObjectFunc<CLASS>(OBJECT, PROPERTY, #CLASS);

#if CORE_COMPILER_MSVC
#pragma deprecated ("proAsTypedObject")
#endif // CORE_COMPILER_MSVC
#define proAsTypedObject PRO_AS_TYPED_OBJECT

// (aaj - gone for good, please use PRO_NEW_INSTANCE instead)
// macro to create a typed new instance of a class
//#define proNewInstance(CLASS) proClassRegistry::NewInstance<CLASS*>(#CLASS);

#if CORE_COMPILER_MSVC
#pragma deprecated ("proNewInstance")
#endif // CORE_COMPILER_MSVC
#define proNewInstance PRO_NEW_INSTANCE;



#endif //__PROSTRING_UTILS__

