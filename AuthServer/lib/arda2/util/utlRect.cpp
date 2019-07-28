/*****************************************************************************
	created:	2001/08/14
	copyright:	2001, NCSoft. All Rights Reserved
	author(s):	Peter M. Freese
	
	purpose:	Integer Rect class which wraps the windows RECT
				structure. This is based loosely on the code in
				afxwin.h and afxwin1.inline; see the MFC docs for
				instructions on using these.
*****************************************************************************/

#include "arda2/core/corFirst.h"
#include "arda2/util/utlRect.h"

const utlRect utlRect::Zero = utlRect(0,0,0,0);


bool utlRect::ConstrainTo( const utlRect &bounds )
{
	// bail early if nothing to do
	if ( bounds.Contains(*this) )
		return FALSE;

	// check left-right
	if (left < bounds.left)
		OffsetX(bounds.left - left);
	else if (right > bounds.right)
		OffsetX(bounds.right - right);

	// check up-down
	if (top < bounds.top)
		OffsetY(bounds.top - top);
	else if (bottom > bounds.bottom)
		OffsetY(bounds.bottom - bottom);

	return TRUE;
}
