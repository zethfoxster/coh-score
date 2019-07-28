#include "arda2/core/corFirst.h"

#include "arda2/properties/proFirst.h"

#include "matAdapters.h"

#include "arda2/properties/proClass.h"

#include "matProperty.h"


/**
*   ------------- Transform Adapter ----------------
**/

PRO_REGISTER_ADAPTER_CLASS(matTransformAdapter, matTransform)
REG_PROPERTY_VECTOR3(matTransformAdapter,  Position)
REG_PROPERTY_FLOAT(matTransformAdapter,  Scale)
REG_PROPERTY_FLOAT(matTransformAdapter,  Yaw)
REG_PROPERTY_FLOAT(matTransformAdapter,  Pitch)
REG_PROPERTY_FLOAT(matTransformAdapter,  Roll)

matTransformAdapter::matTransformAdapter()
{
}





PRO_REGISTER_ADAPTER_CLASS(matVectorAdapter, matVector)
REG_PROPERTY_FLOAT(matVectorAdapter,  X)
REG_PROPERTY_FLOAT(matVectorAdapter,  Y)
REG_PROPERTY_FLOAT(matVectorAdapter,  Z)


matVectorAdapter::matVectorAdapter()
{
}




PRO_REGISTER_ADAPTER_CLASS(matColorAdapter, matVector)
REG_PROPERTY_FLOAT(matColorAdapter,  R)
REG_PROPERTY_FLOAT(matColorAdapter,  G)
REG_PROPERTY_FLOAT(matColorAdapter,  B)
REG_PROPERTY_FLOAT(matColorAdapter,  A)


matColorAdapter::matColorAdapter()
{
}

