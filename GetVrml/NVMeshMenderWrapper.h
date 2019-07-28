
enum NormalCalcOption{ DONT_CALCULATE_NORMALS , CALCULATE_NORMALS };
enum ExistingSplitOption{ DONT_RESPECT_SPLITS , RESPECT_SPLITS };
enum CylindricalFixOption{ DONT_FIX_CYLINDRICAL , FIX_CYLINDRICAL };


bool MeshMenderMend(Model *model);
//		  const float minNormalsCreaseCosAngle = 0.0f,
//		  const float minTangentsCreaseCosAngle = 0.0f ,
//		  const float minBinormalsCreaseCosAngle = 0.0f,
//		  const float weightNormalsByArea = 1.0f,
//		  const NormalCalcOption computeNormals = CALCULATE_NORMALS,
//		  const ExistingSplitOption respectExistingSplits = DONT_RESPECT_SPLITS,
//		  const CylindricalFixOption fixCylindricalWrapping = DONT_FIX_CYLINDRICAL);	

#ifndef CLIENT
bool MeshMenderMendGMesh(GMesh *mesh, char *name);
#endif
