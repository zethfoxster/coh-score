/**********************************************************************
 *<
	FILE: vrml.h

	DESCRIPTION:  Basic includes for modules in vrmlexp

	CREATED BY: greg finch

	HISTORY: created 9, May 1997

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/

#ifndef __VRML__H__
#define __VRML__H__

#define _CRT_SECURE_NO_WARNINGS
// disable LRESULT to int conversion warning because the 64 bit compilation currently has lots of them
// (this is also disabled in the maxsdk vrml project)
#pragma warning( disable : 4244 )	

#include <stdio.h>
#include "max.h"
#include "resource.h"
#include "iparamm.h"
#include "bmmlib.h"
#include "utilapi.h"
#include "decomp.h"

//#define DDECOMP
#ifdef DDECOMP
#include "cdecomp.h"
#endif

TCHAR *GetString(int id);

#endif