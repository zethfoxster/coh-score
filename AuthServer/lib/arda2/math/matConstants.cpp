/*****************************************************************************
created:	2001/09/20
copyright:	2001, NCSoft. All Rights Reserved
author(s):	Peter M. Freese

purpose:	
*****************************************************************************/

#include "arda2/math/matFirst.h"
#include "arda2/math/matConstants.h"

// Math constants are initialized using #defines rather than interdependent definitions
// This eliminates any problems arising from static initialization order. If you add new
// constants here, follow the same pattern of non-dependent constant definitions.

#define PI 3.1415926535897932384626433832795
#define ROOT2 1.4142135623730950488016887242097
#define LOG2E 0.693147180559945309417

const float	matPi		= (float)PI;
const float	mat2Pi		= (float)(2.0 * PI);
const float	matPiDiv2	= (float)(0.5 * PI);
const float	matPiDiv4	= (float)(0.25 * PI);
const float	matInvPi	= (float)(1.0 / PI);
const float	matInv2Pi	= (float)(0.5 / PI);
const float	matDegToRad	= (float)(PI / 180.0);
const float	matRadToDeg	= (float)(180.0 / PI);
const float	matRoot2	= (float)(ROOT2);
const float matInvRoot2 = (float)(1.0 / ROOT2);
const float matLog2E	= (float)(LOG2E);
