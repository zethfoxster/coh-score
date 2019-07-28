#include "arda2/math/matFirst.h"
#include "arda2/math/matIntVector4.h"

const matIntVector4 matIntVector4::Zero(matZero);
const matIntVector4 matIntVector4::Max(matMaximum);
const matIntVector4 matIntVector4::Min(matMinimum);

void matIntVector4::StoreComponentClamp(	const matIntVector4& min,
										const matIntVector4& max)
{
	// componentwise clamp	
	x = matClamp(x, min.x, max.x);
	y = matClamp(y, min.y, max.y);
	z = matClamp(z, min.z, max.z);
    w = matClamp(w, min.w, max.w);
}

