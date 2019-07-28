#include "wininclude.h"

void wglMakeCurrentSafeDirect( HDC hdc, HGLRC glRC, F32 timeout);
void makeCurrentCBDirect(void);
void windowInitDisplayContextsDirect(void);
void windowUpdateDirect(void);
void winMakeCurrentDirect(void);
void rdrSetGammaDirect(float Gamma);
void rdrNeedGammaResetDirect(void);
void rdrPreserveGammaRampDirect(void);
void rdrRestoreGammaRampDirect(void);
void windowDestroyDisplayContextsDirect(void);
void windowReleaseDisplayContextsDirect(void);
void windowReInitDisplayContextsDirect(void);
void rdrSetVSyncDirect(int enable);

