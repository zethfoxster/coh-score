#include "utilitiesLib.h"
#include "rand.h"
#include "mathutil.h"
#include "referencesystem.h"
#include "wininclude.h"

static bool bInit = false;
int gBuildVersion = 0;

void utilitiesLibPreAutoRunStuff(void)
{
	static int once = 0;

	if (!once)
	{			
		once = 1;
		memCheckInit();
// 		ScratchStackInitSystem();
	}
}

bool utilitiesLibStartup()
{
	if ( bInit )
		return false;

	initRand();
	initQuickTrig();
	RefSystem_Init();

	return true;
}

void DebuggerPrint(const char * msg) {
	OutputDebugStringA(msg);
}
