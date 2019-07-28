#include "stdtypes.h"
#include "wininclude.h"
#include "Color.h"

typedef struct CDXSurface // wrapper hack
{
	U8 * buffer;
	int pitch;
	RECT clipRect;
} CDXSurface;

int DrawBlkRotoZoom(CDXSurface *src, CDXSurface* dest, int midX, int midY, RECT* srcArea, double angle, double scalex, double scaley, Color rgba[], int numColors, int blend);

