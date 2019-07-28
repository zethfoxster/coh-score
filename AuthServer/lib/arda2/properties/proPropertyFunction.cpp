#include "arda2/core/corFirst.h"
#include "arda2/math/matFirst.h"

#include "arda2/properties/proPropertyNative.h"
#include "arda2/properties/proPropertyOwner.h"
#include "arda2/properties/proPropertyFunction.h"

#if CORE_COMPILER_GNU
#include <stdarg.h>
#endif

using namespace std;

// helper function used to invoke delegate functions with arbitrary arguements
int proCallFunction(void* func, size_t numArgs, proObject *obj, va_list ap, bool returnFloat );

PRO_REGISTER_ABSTRACT_CLASS(proPropertyFunction, proProperty)
PRO_REGISTER_ABSTRACT_CLASS(proPropertyFunctionNative, proPropertyFunction)


proDataType proPropertyFunction::GetDataType() const
{
    return DT_function;
}

proDataType proPropertyFunction::GetArgDataType(uint index) const
{ 
    if (index >= m_args.size())
        return DT_other;
    else
        return m_args[index];
}

/**
 *  invoke the wrapper function using the proCallFunction helper function
 */
int proPropertyFunction::Invoke(proObject *target, ...)
{
    if (!target || !m_func)
        return -1;

    va_list ap;
    va_start(ap, target);
    return proCallFunction(m_func, m_args.size(), target, ap, m_returnFloat);
}

/**
 * custom annotations for Function properties
 */
void proPropertyFunction::SetAnnotation(const string &name, const string &value)
{
    if (_stricmp(name.c_str(), "NumArgs") == 0 )
    {
        proPropertyIntOwner *newProp = new proPropertyIntOwner();
        newProp->SetName(name);
        AddInstanceProperty(newProp);
        newProp->SetValue(this, proStringUtils::AsInt(value));
    }
    else if (strncmp(name.c_str(), "Arg", 3) == 0 && name.size() == 4)
    {
        m_args.push_back( proGetTypeForName(value.c_str()) );
    }
    else
    {
        proProperty::SetAnnotation(name, value);
    }
}


string proPropertyFunction::ToString( const proObject*  ) const 
{ 
    string result = GetValue("ReturnType");
    result += " ";
    result += GetName() + "(";
    size_t end =  m_args.size();
    for (size_t i=0; i != end; i++)
    {
        result += proGetNameForType( GetArgDataType( (uint)i) );
        if (i <  end-1)
            result += ", ";
    }
    result += ")";
    return result;
}


/**
 * Dangerous function to call any C function with any number of args.
 * This is NOT type safe - it uses the non-C, assembly stack to pass 
 * arguments to the function - regardless of their type.
 *
 * (only works on 32-bit windows for now)
 *
 * @param func the function to be called - a delegate create by the REG_PROPERTY_FUNCTION macros
 * @param numArmgs the number of args to be passed to the function
 * @param obj the target object that the eventual member function will be invoked on
 * @param ap the list of arguments - MUST match numArgs.
 */
int proCallFunction(void* func, size_t numArgs, proObject *obj, va_list ap, bool returnFloat)
{
    int returnVal = 0;

#if CORE_SYSTEM_WIN32

    void *tstFloat = (void*)0x40000000;
    void *args[32]; // calling convention is right to left for __cdecl
    double fargs[32];
    int fcounter = 0;

    // extract arguments
    for (size_t i=0; i != numArgs; i++)
    {
        args[i] = va_arg(ap, void*);
        if (args[i] == tstFloat)
        {
            // this is a float value - go back and read it as the right type
            ap--; ap--; ap--; ap--;
            fargs[fcounter++] = va_arg(ap, double);
        }
    }

    fcounter = 0;
    // push on stack in right to left order
    for (size_t i=numArgs; i != 0; i--)
    {
        void *arg = args[i-1];
        if (arg == tstFloat)
        {
            // pass floating point values on FP registers
            double tmp = fargs[fcounter++];
            __asm fld   qword ptr [tmp] ;
            __asm fst   dword ptr [ebp-50h] ;
            __asm push  ecx ;
            __asm fstp  dword ptr [esp] ;
        }
        else
        {
            // pass integers on the stack
            __asm mov eax, arg ;
            __asm push eax ;
        }
    }

    // push object onto stack
    __asm mov eax, obj ;
    __asm push eax ;

    // call function
    __asm call func ;

    // get return value
    if (returnFloat)
    {
        // grab from FP register
        __asm fst returnVal ;
    }
    else
    {
        __asm mov returnVal, eax ;
    }

    // clean up stack
    for (size_t j=0; j != numArgs; j++)
    {
        __asm pop eax ;
    }
#else
    UNREF(func);
    UNREF(numArgs);
    UNREF(obj);
    UNREF(ap);
    UNREF(returnFloat);

    ERR_UNIMPLEMENTED();
#endif
    return returnVal;
}


/*
 *  UNIT TESTS
 */
#if BUILD_UNIT_TESTS & ! CORE_COMPILER_GNU

#include "arda2/test/tstUnit.h"
#include "arda2/properties/proClassNative.h"
#include "arda2/properties/proPropertyNative.h"


class proFuncTest : public proObject
{
    PRO_DECLARE_OBJECT
public:
    proFuncTest() {};

    int Func0()
    {
        return 101;
    }

    int Func1(int v1)
    {
        UNREF(v1);
        return 102;
    }

    float Func2(int v1, char *v2)
    {
        UNREF(v1); UNREF(v2);
        return 55.32f;
    }

    int Func3(int v1, float v2, int v3)
    {
        UNREF(v1); UNREF(v2); UNREF(v3);
        return 103;
    }

    const char* FuncStrPtr(const string *str)
    {
        UNREF(str);
        return "A String!";
    }

    const char* FuncStrRef(const string& str)
    {
        UNREF(str);
        return "A String!";
    }

    const char* FuncObj(proObject *obj)
    {
        return obj->GetClass()->GetName().c_str();
    }


};


PRO_REGISTER_CLASS(proFuncTest, proObject)
REG_PROPERTY_FUNCTION0(proFuncTest, Func0, Func0, int)
REG_PROPERTY_FUNCTION1(proFuncTest, Func1, Func1, int, int)
REG_PROPERTY_FUNCTION2(proFuncTest, Func2, Func2, float, int, char*)
REG_PROPERTY_FUNCTION3(proFuncTest, Func3, Func3, int, int, float, int)
REG_PROPERTY_FUNCTION1(proFuncTest, FuncStrPtr, FuncStrPtr, const char*, const string*)
REG_PROPERTY_FUNCTION1(proFuncTest, FuncStrRef, FuncStrRef, const char*, const string& )
REG_PROPERTY_FUNCTION1(proFuncTest, FuncObj, FuncObj, const char*, proObject*)

class proPropertyFunctionTests : public tstUnit
{
private:

public:
    proPropertyFunctionTests()
    {
    }

    virtual void Register()
    {
        SetName("proPropertyFunction");

        proClassRegistry::Init();

        AddTestCase("testFunction0", &proPropertyFunctionTests::testFunction0);
        AddTestCase("testFunction1", &proPropertyFunctionTests::testFunction1);
        AddTestCase("testFunction2", &proPropertyFunctionTests::testFunction2);
        AddTestCase("testFunction3", &proPropertyFunctionTests::testFunction3);
        AddTestCase("testFuncStrPtr", &proPropertyFunctionTests::testFuncStrPtr);
        AddTestCase("testFuncStrRef", &proPropertyFunctionTests::testFuncStrRef);
        AddTestCase("testFunctObj", &proPropertyFunctionTests::testFunctObj);
    }

    virtual void TestCaseSetup()
    {
    }

    virtual void TestCaseTeardown()
    {
    }

    void testFunction0() const
    {
        proFuncTest ft;
        
        proPropertyFunction *pf = (proPropertyFunction*)ft.GetPropertyByName("Func0");
        int result = pf->Invoke(&ft);
        TESTASSERT(result == 101);
    }

    void testFunction1() const
    {
        proFuncTest ft;
        
        proPropertyFunction *pf = (proPropertyFunction*)ft.GetPropertyByName("Func1");
        int result = pf->Invoke(&ft, 43);
        TESTASSERT(result == 102);
    }

    void testFunction2() const
    {
        proFuncTest ft;
        
        proPropertyFunction *pf = (proPropertyFunction*)ft.GetPropertyByName("Func2");
        int result = pf->Invoke(&ft, 77, "Hello");
        float f = *(float*)(int*)&result;
        TESTASSERT( fabs(f-55.32f) < 0.0001);
    }

    void testFunction3() const
    {
        proFuncTest ft;
        
        proPropertyFunction *pf = (proPropertyFunction*)ft.GetPropertyByName("Func3");

        float f = 10.33333f;
        int result = pf->Invoke(&ft, 100, f, 200);
        TESTASSERT(result == 103);
    }

    void testFuncStrPtr() const
    {
        proFuncTest ft;
        string str("This is a long string. It is so long. So verrry long.");

        proPropertyFunction *pf = (proPropertyFunction*)ft.GetPropertyByName("FuncStrPtr");
        char *result = (char*)(size_t)pf->Invoke(&ft, &str);
        TESTASSERT( strcmp(result, "A String!") == 0);
    }

    void testFuncStrRef() const
    {
        proFuncTest ft;
        string str("This is a long string. It is so long. So verrry long.");

        //NOTE: varargs knows nothing of "pass-by-reference". If the target function
        //      requires a reference, the arg must be passed by address from the caller.
        proPropertyFunction *pf = (proPropertyFunction*)ft.GetPropertyByName("FuncStrRef");
        char *result = (char*)(size_t)pf->Invoke(
             &ft, 
             &str /* NOTE: address-of str - even though the target function takes a "string&" */
             );

        TESTASSERT( strcmp(result, "A String!") == 0);
    }

    void testFunctObj() const
    {
        proFuncTest ft;
        proObject o;

        proPropertyFunction *pf = (proPropertyFunction*)ft.GetPropertyByName("FuncObj");
        char *result = (char*)(size_t)pf->Invoke(&ft, &o);
        TESTASSERT( strcmp(result, "proObject") == 0);
    }

    //TODO: return strings by value from functions

};

EXPORTUNITTESTOBJECT(proPropertyFunctionTests);


#endif


