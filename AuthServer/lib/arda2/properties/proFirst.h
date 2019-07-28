#ifndef   INCLUDED_proFirst_h
#define   INCLUDED_proFirst_h

#include "arda2/core/corStlVector.h"
#include "arda2/core/corStdString.h"

class proObject;
class proClass;
class proClassRegistry;
class proObjectWriter;
class proObjectReader;
class proObserver;
class proValue;
class proProperty;

typedef std::vector<proProperty*>  PropertyList;

enum DP_OWN_VALUE
{
    DP_ownValue,    // the property owns the value (will delete it)
    DP_refValue     // the property does not own the value
};


/** 
 * base class for values used for owner property storage
 */
class proValue
{
public:
    proValue(const std::string &name) : m_name(name) {}
    const std::string &GetName() const { return m_name; }
    void SetName(const std::string &name) { m_name = name; }
    virtual ~proValue() {};
protected:
    std::string m_name;
};

/**
 * the set of primitive types that properties can be.
 * this set is not extended by derived properties - additional
 * type information should be obtained through the property's
 * class object.
 */
enum proDataType
{
    DT_int = 0,
    DT_uint,
    DT_float,
    DT_bool,
    DT_string,
    DT_wstring,
    DT_object,
    DT_enum,
    DT_bitfield,
    DT_function,
    DT_other,
    NUM_DTs
};

const char *proGetNameForType(proDataType const t);

proDataType proGetTypeForName(const char *name);

#endif
