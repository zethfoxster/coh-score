#include "wininclude.h"

DWORD promptGetValue(HINSTANCE hinst, HWND hwnd, char *sName, DWORD dwDefaultValue); // Calls a dialog, and asks for sName
char *promptGetString(HINSTANCE hinst, HWND hwnd, char *sName, const char *sDefaultValue); // Calls a dialog, and asks for sName
