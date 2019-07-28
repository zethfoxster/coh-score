#include "wininclude.h"

C_DECLARATIONS_BEGIN

// Helper function to align all of the elements in a dialog.
// Call once with the initial width and heigh, and then after that
//  call it with the new width/height (from WM_SIZE), and an ID of two controls:
//		idAlignMe:	Everything to the right of the left of this control will translate horizontally upon resize
//					Everything below the top of this control will translate vertically upon resize
//		idUpperLeft:	Everything whose top aligns with the top of this control will stretch vertically upon resize
//						Everything whose left aligns with the left of this control will stretch horizontally upon resize
void doDialogOnResize(HWND hDlg, WORD w, WORD h, int idAlignMe, int idUpperLeft);
void setDialogMinSize(HWND hDlg, WORD minw, WORD minh);

int NumLines(LPTSTR text);
int LongestWord(LPTSTR text); 
void OffsetWindow(HWND hDlg, HWND hWnd, int xdelta, int ydelta);

void errorDialog(HWND hwnd, char *str, char* title, char* fault, int highlight); // title & fault optional
void msgAlert(HWND hwnd, char *str);

HICON getIconColoredLetter(char letter, U32 colorRGB);
void setWindowIconColoredLetter(HWND hwnd, char letter, U32 colorRGB);
char* getIconColoredLetterBytes(int letter, U32 colorRGB); // returns an achr array

void winRegisterMe(const char *command, const char *extension); // Registers the current executable to handle files of the given extension
char *winGetFileName_s(HWND hwnd, const char *fileMask, char *fileName, size_t fileName_size, bool save);

void winSetHInstance(HINSTANCE hInstance);
HINSTANCE winGetHInstance(void);

void winAddToPath(const char *path, int prefix);
bool winExistsInRegPath(const char *path);
bool winExistsInEnvPath(const char *path);

char *winGetLastErrorStr();
bool winCreateProcess(char *command_line, PROCESS_INFORMATION *res_pi);
bool winProcessRunning(PROCESS_INFORMATION *pi);
bool winProcessExitCode(PROCESS_INFORMATION *pi, U32 *res_exit);

C_DECLARATIONS_END
