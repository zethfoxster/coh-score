#include "arda2/math/matFirst.h"
#include "arda2/math/matIntVector3.h"

const matIntVector3 matIntVector3::Zero(matZero);
const matIntVector3 matIntVector3::Max(matMaximum);
const matIntVector3 matIntVector3::Min(matMinimum);

void matIntVector3::StoreComponentClamp(	const matIntVector3& min,
										const matIntVector3& max)
{
	// componentwise clamp	
	x = matClamp(x, min.x, max.x);
	y = matClamp(y, min.y, max.y);
	z = matClamp(z, min.z, max.z);
}

