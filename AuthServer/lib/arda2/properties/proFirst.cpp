#include "arda2/core/corFirst.h"
#include "arda2/properties/proFirst.h"

static const char *proTypeNames[] =
{
    "int",
    "uint",
    "float",
    "bool",
    "string",
    "wstring",
    "object",
    "enum",
    "bitfield",
    "function",
    "other"
};

const char *proGetNameForType(proDataType const t)
{
    if (t<0 || t >= NUM_DTs)
        return "null";

    return proTypeNames[t];
}


proDataType proGetTypeForName(const char *name)
{
    for (int i=0; i != NUM_DTs; i++)
    {
        if (_stricmp(name, proTypeNames[i]) == 0)
        {
            return (proDataType)i;
        }
    }
    if (strstr(name, "proObject"))
        return DT_object;
    
    if (strstr(name, "char"))
        return DT_string;

    return DT_other;
}
