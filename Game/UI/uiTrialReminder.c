#include "uiWindows.h"
#include "wdwbase.h"

int trialReminderWindow(void)
{
	//always hide this window
	window_setMode(WDW_TRIALREMINDER, WINDOW_SHRINKING);

	return 0;
}