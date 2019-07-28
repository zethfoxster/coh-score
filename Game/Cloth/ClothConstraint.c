#include "Cloth.h"
#include "ClothPrivate.h"

// This determines how significantly a collision affects the effective mass
// of a particle. 
#define COL_MASS_FACTOR 1.0f

//============================================================================
// CLOTH CONSTRAINT CODE
//============================================================================
// This code profoundly affects the behavior of the cloth.
// Currently the only constraints supported are length constraints.
// There are two types of length constraints supported:
//  1. MinMax constraints (RestLength >= 0).
//     These constraints push the particles towards a specific distance apart
//  2. Min constraints (RestLength < 0)
//     These constraints only activate when the particles are less than
//     -RestLength units apart.
// Angular constraints are implemented by using a Min constraint between
//   particles that share a comon horizontal or vertical neighbor.
//   e.g. if (A,B) are constrained with RestLength = X
//        and (B,C) are constrained with RestLength = Y
//        constrain (A,C) with RestLength = (X+Y)*.707
//        The angle between (A,B) and (B,C) will be limited to (roughly) 90 degrees.

// Setup a constraint between two particles.
// Use frac = 1.0 to create a MinMax (i.e. fixed distance) constraint
// Use frac < 0 to create an angular constraint, where -frac ~= cos((180 - minang)*.5)
//  minang = minimum angle between the segments connecting the particles
//   e.g. frac = -.866 would create a minimum angle between segments at about 120 degrees.
void ClothLengthConstraintInitialize(ClothLengthConstraint *constraint, Cloth *cloth, S32 p1, S32 p2, F32 frac)
{
	Vec3 dvec;
	F32 dlen;
	constraint->P1 = (S16)p1;
	constraint->P2 = (S16)p2;
	subVec3(cloth->CurPos[p2], cloth->CurPos[p1], dvec);
	dlen = lengthVec3(dvec);
	constraint->RestLength = dlen * frac;
	constraint->InvLength = (dlen > 0.0f) ? 1.0f / dlen : 0.0f;
	constraint->MassTot2 = 2.0f / (cloth->InvMasses[p1] + cloth->InvMasses[p2]);
}

// Process the constraint.
// Each constraint update moves the partices towards or away from each other
//   as dictated by the constraint.
// The more often the Update is called, the closer to their correct position
//   the particles will be.
// More massive particles are moved less than less massive particles.
// A particle that collided will have a greater effective mass by up to 2X.
//
// Cloth->SubLOD affects the speed and accuracy of this code.
// Currently at SubLOD 0,1 the code is fully accurate,
//   at SubLOD 2 a sqrt approximation is used and collision amount is ignored
//   at SubLod 3 ClothLengthConstraintFastUpdate() is called which also igores masses

void ClothLengthConstraintUpdate(const ClothLengthConstraint *constraint, Cloth *cloth)
{
	F32 r = cloth->PointColRad;
	int P1 = constraint->P1;
	int P2 = constraint->P2;
	F32 imass1 = cloth->InvMasses[P1];
	F32 imass2 = cloth->InvMasses[P2];
	int SubLOD = cloth->commonData.SubLOD;
	F32 tot_imass;
	F32 x1,y1,z1,x2,y2,z2,dx,dy,dz,dp;
	F32 fdiff;
	F32 RestLength;

	tot_imass = imass1 + imass2;
	if (imass1 + imass2 == 0.0f)
		return; // both particles are immovable
	
#if 1
	if (SubLOD <= 1)
	{
		if (cloth->ColAmt[P1] > 0.0f)
			imass1 *= (1.f / (1.f + cloth->ColAmt[P1]*cloth->ColAmt[P1]*COL_MASS_FACTOR));
		if (cloth->ColAmt[P2] > 0.0f)
			imass2 *= (1.f / (1.f + cloth->ColAmt[P2]*cloth->ColAmt[P2]*COL_MASS_FACTOR));
		tot_imass = imass1 + imass2;
	}
#endif

	x1 = cloth->CurPos[P1][0];
	y1 = cloth->CurPos[P1][1];
	z1 = cloth->CurPos[P1][2];
	x2 = cloth->CurPos[P2][0];
	y2 = cloth->CurPos[P2][1];
	z2 = cloth->CurPos[P2][2];
	dx = x2 - x1;
	dy = y2 - y1;
	dz = z2 - z1;
	dp = (dx*dx + dy*dy + dz*dz);

	RestLength = constraint->RestLength;

	if (SubLOD <= 1) 
	{
		// Uses slow, accurate sqrt
		if (RestLength >= 0) {
			F32 deltalength = sqrtf(dp);
			fdiff = (deltalength - RestLength) / (deltalength * tot_imass);
		}else if (RestLength*RestLength > dp) {
			F32 deltalength = sqrtf(dp);
			fdiff = (deltalength + RestLength) / (deltalength * tot_imass);
		}else{
			return;
			//fdiff = 0.0f;
		}
	}
	else
	{
		// Uses Newton-Raphson method for sqrt(dp) using sqrt(dp ~= RestLength)
		//  deltalength ~= .5*(RestLength + dp/RestLength)
		F32 MassTot2 = constraint->MassTot2;
		F32 rl2 = RestLength*RestLength;
		if (RestLength >= 0)
			fdiff = (0.5f - rl2/(rl2 + dp))*MassTot2;
		else if (rl2 > dp)
			fdiff = (0.5f - rl2/(rl2 + dp))*MassTot2;
		else
			//fdiff = 0;
			return;
	}

	{
		F32 idiff1 = imass1*fdiff;
		F32 idiff2 = -imass2*fdiff;

		cloth->CurPos[P1][0] = x1 + dx*idiff1;
		cloth->CurPos[P1][1] = y1 + dy*idiff1;
		cloth->CurPos[P1][2] = z1 + dz*idiff1;

		cloth->CurPos[P2][0] = x2 + dx*idiff2;
		cloth->CurPos[P2][1] = y2 + dy*idiff2;
		cloth->CurPos[P2][2] = z2 + dz*idiff2;
	}
}

// Uses fast square root and ignores masses
void ClothLengthConstraintFastUpdate(const ClothLengthConstraint *constraint, Cloth *cloth)
{
	int P1 = constraint->P1;
	int P2 = constraint->P2;
	
	// Ignore non 0 masses
	F32 imass1 = cloth->InvMasses[P1];
	F32 imass2 = cloth->InvMasses[P2];
	if (imass1 + imass2 == 0.0f)
		return;

	{
	F32 RestLength = constraint->RestLength;
	
	F32 x1,y1,z1,x2,y2,z2,dx,dy,dz;
	x1 = cloth->CurPos[P1][0];
	y1 = cloth->CurPos[P1][1];
	z1 = cloth->CurPos[P1][2];
	x2 = cloth->CurPos[P2][0];
	y2 = cloth->CurPos[P2][1];
	z2 = cloth->CurPos[P2][2];
	dx = x2 - x1;
	dy = y2 - y1;
	dz = z2 - z1;
	// Uses Newton-Raphson method for sqrt(dp) using sqrt(dp ~= RestLength)
	//  deltalength ~= .5*(RestLength + dp/RestLength)
	{
	F32 dp = (dx*dx + dy*dy + dz*dz);
	F32 rl2 = RestLength*RestLength;
	F32 diff;
	if (RestLength >= 0)
		diff = (0.5f - rl2/(rl2 + dp));
	else if (rl2 > dp)
		diff = (0.5f - rl2/(rl2 + dp));
	else
		diff = 0;

	if (imass1 != 0)
	{
		cloth->CurPos[P1][0] = x1 + dx*diff;
		cloth->CurPos[P1][1] = y1 + dy*diff;
		cloth->CurPos[P1][2] = z1 + dz*diff;
	}
	if (imass2 != 0)
	{
		cloth->CurPos[P2][0] = x2 - dx*diff;
		cloth->CurPos[P2][1] = y2 - dy*diff;
		cloth->CurPos[P2][2] = z2 - dz*diff;
	}
	}
	}
}

//////////////////////////////////////////////////////////////////////////////
