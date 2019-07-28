#ifndef _UISCRIPT_H
#define	_UISCRIPT_H

#include "uiInclude.h"
#include "stdtypes.h"
#include "scriptuienum.h"
#include "uiCompass.h" // For ScriptUIClientWidget

extern int scriptUIIsDetached;

int scriptUIWindow();
void detachScriptUIWindow(int detach);
void scriptUIReceiveDetach(Packet *pak);

#endif