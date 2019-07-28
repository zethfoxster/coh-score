#ifndef UIHELPBUTTON_H
#define UIHELPBUTTON_H

typedef struct SMFBlock SMFBlock;
typedef struct CBox CBox;

typedef struct HelpButton
{
	int state;
	SMFBlock * pBlock;

}HelpButton;
typedef enum helpButtonFlags
{
	HELP_BUTTON_USE_BOX_WIDTH = 1 << 0,
};
void freeHelpButton(HelpButton *pHelp);
float helpButtonEx( HelpButton **ppHelpButton, F32 x, F32 y, F32 z, F32 sc, char * txt, CBox * box, int flags );
#define helpButton( ppHelpButton, x, y, z, sc, txt, box )	helpButtonEx( ppHelpButton, x, y, z, sc, txt, box, 0 );
void helpButtonFrame( F32 x, F32 y, F32 z, F32 wd, F32 ht, F32 sc, int color, int back_col, int back_col2 );
float helpButtonFullyConfigurable( HelpButton **ppHelpButton, F32 x, F32 y, F32 z, F32 sc, char * txt, CBox * bounds, int flags,
									int color, int back_col, int back_col2, int icon_col, int icon_col2);

#endif