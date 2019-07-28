/*****************************************************************************
	created:	2002/09/29
	copyright:	2002, NCSoft. All Rights Reserved
	author(s):	Peter M. Freese

	purpose:	4x4 Matrix Math functions
*****************************************************************************/

#include "arda2/math/matFirst.h"
#include "arda2/math/matMatrix4x4.h"


const matMatrix4x4 matMatrix4x4::Identity(matIdentity);


matMatrix4x4& matMatrix4x4::StoreLookDir( const matVector3 &vForward, const matVector3 &vGoalUp )
{
	StoreIdentity();

	matVector3 vRight;

	// calculate a new right vector which will be orthogonal to up and forward
	vRight.StoreCrossProduct(vGoalUp, vForward);

	// handle colinear cases
	if ( vRight.MagnitudeSquared() < 0.00001f )
	{
		vRight.StoreCrossProduct(vForward, matVector3(1,0,0));
		if ( vRight.MagnitudeSquared() < 0.00001f )
		{
			vRight.StoreCrossProduct(matVector3(0,0,-1), vForward);
		}
	}

	// normalize right
	vRight.Normalize();

	// calculate a new up vector which will be orthogonal to right and forward
	matVector3 vUp;
	vUp.StoreCrossProduct(vForward, vRight);
	vUp.Normalize();

	GetRow0() = vRight;
	GetRow1() = vUp;
	GetRow2() = vForward;

	return *this;
}


// Get the Euler angles
// This is only guaranteed to work if the matrix has no scaling
void matMatrix4x4::GetEulerAngles( float &fYaw, float &fPitch, float &fRoll ) const
{
	// Roll matrix (rotate around local z)
	// | Cz  Sz  0 |
	// |-Sz  Cz  0 |
	// | 0   0   1 |

	// Pitch matrix (rotate around local x)
	// | 1   0   0 |
	// | 0   Cx  Sx|
	// | 0  -Sx  Cx|

	// Yaw matrix (rotate around local y)
	// | Cy  0  -Sy|
	// | 0   1   0 |
	// | Sy  0   Cy|

	// Roll x Pitch
	// | Cz  Sz  0 |   | 1   0   0 |   | Cz  SzCx SzSx |
	// |-Sz  Cz  0 | X | 0   Cx  Sx| = |-Sz  CzCx CzSx |   
	// | 0   0   1 |   | 0  -Sx  Cx|   | 0  -Sx   Cx   |

	// Roll x Pitch x Yaw
	// | Cz  SzCx SzSx |   | Cy  0  -Sy|   |  CzCy+SzSxSy   SzCx  -CzSy+SzSxCy |
	// |-Sz  CzCx CzSx | X | 0   1   0 | = | -SzCy+CzSxSy   CzCx   SzSy+CzSxCy |   
	// | 0  -Sx   Cx   |   | Sy  0   Cy|   | CxSy          -Sx     CxCy        |

	const float fEpsilon = 0.00001f;

	float Cx = sqrtf(_31 * _31 + _33 * _33);
	if ( Cx > fEpsilon )
	{
		fYaw  = atan2f(_31, _33);
		fRoll = atan2f(_12, _22);
		fPitch = atan2f(-_32, Cx);
	}
	else
	{
		// pitch is +/-90, therefore Cx=0, Sx=1
		// |  CzCy+SzSy  0  -CzSy+SzCy |
		// | -SzCy+CzSy  0   SzSy+CzCy |   
		// | 0          -1   0         |

		// yaw and roll are gimbal-locked; arbitrarily pick a roll of 0, and solve for yaw and pitch
		fRoll = 0.0f;
		GetEulerAnglesNoRoll(fYaw, fPitch);
	}
}


// Get Euler angles, assuming no Z rotation
// This is only guaranteed to work if the matrix has no scaling
void matMatrix4x4::GetEulerAnglesNoRoll( float &fYaw, float &fPitch ) const
{
	// Pitch x Yaw
	// |  Cy   0     -Sy  |
	// | SxSy  Cx    SxCy |   
	// | CxSy -Sx    CxCy |

	fYaw    = atan2f(-_13, _11);
	fPitch  = atan2f(-_32, _22);
}


