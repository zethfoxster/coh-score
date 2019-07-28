/*****************************************************************************
	created:	2001/08/14
	copyright:	2001, NCSoft. All Rights Reserved
	author(s):	Peter M. Freese
	
	purpose:	Integer Point class which wraps the windows POINT
				structure. This is based loosely on the code in
				afxwin.h and afxwin1.inline; see the MFC docs for
				instructions on using these.
*****************************************************************************/

#include "arda2/core/corFirst.h"

#include "arda2/util/utlPoint.h"

const utlPoint utlPoint::Zero = utlPoint(0,0);
