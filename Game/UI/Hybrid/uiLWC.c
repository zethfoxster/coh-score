#include "uiLWC.h"
#include "uiHybridMenu.h"
#include "uiWindows.h"
#include "wdwbase.h"

int uiLWCWindow()
{
	float x, y, z, wd, ht, sc;

	// Do everything common windows are supposed to do.
	if ( !window_getDims( WDW_LWC_UI, &x, &y, &z, &wd, &ht, &sc, 0, 0 ))
		return 0;

	drawLWCUI(x+wd/2.f, y+ht/2.f, z, sc);
	return 0;
}