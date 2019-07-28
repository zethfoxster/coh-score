#ifndef _DIALOG_H
#define	_DIALOG_H

#include "uiInclude.h"
#include "stdtypes.h"

typedef enum
{
	DIALOG_YES_NO,
	DIALOG_OK,
	DIALOG_SINGLE_RESPONSE,
	DIALOG_TWO_RESPONSE,
	DIALOG_TWO_RESPONSE_CANCEL,
	DIALOG_THREE_RESPONSE,
	DIALOG_NO_RESPONSE,
	DIALOG_OK_CANCEL_TEXT_ENTRY,
	DIALOG_NAME_TEXT_ENTRY,
	DIALOG_YES_NO_CHECKBOXES,
	DIALOG_ACCEPT_CANCEL,
	DIALOG_SMF,
	DIALOG_NAME_ACCEPT,
	DIALOG_ARCHITECT_OPTIONS,
	DIALOG_MORAL_CHOICE,
}DialogStyles;


typedef enum
{
	DLGFLAG_GAME_ONLY = 1,
	DLGFLAG_ALLOW_HANDLE = (1 << 1),
	DLGFLAG_NAG = (1 << 2),
	DLGFLAG_NO_TRANSLATE = (1 << 3),
	DLGFLAG_SECRET_INPUT = (1 << 4),
	DLGFLAG_NAMES_ONLY = (1 << 5),
	DLGFLAG_ARCHITECT = (1 << 6),
	DLGFLAG_LIMIT_MOUSE_TO_DIALOG = (1 << 7),
}DialogFlags;

typedef struct DialogCheckbox
{
	char	txt[128];
	bool	*variable;
	int		optionID;
}DialogCheckbox;

void dialog( int type, float x, float y, float wd, float ht, const char * txt, const char * option1, void (*code1)(void *data), char * option2, void (*code2)(void *data),
			 int flags, void (*checkboxCallback)(void *data), DialogCheckbox *dcb, int dcbCount, int timer, int maxChars, void *data );

void dialogStd( int type, const char * txt, const char * response1, const char * response2, void(*code1)(void *data), void(*code2)(void *data), int flags);
void dialogStd3( int type, const char * txt, const char * response1, const char * response2, const char *response3, void(*code1)(void *data), void(*code2)(void *data), void(*code3)(void *data), int flags);
void dialogStdCB( int type, const char * txt, const char * response1, const char * response2, void(*code1)(void*data), void(*code2)(void*data), int flags, void *data );

void dialogStdWidth( int type, const char * txt, const char * response1, const char * response2, void(*code1)(void *data), void(*code2)(void *data), int flags, float width);

void dialogNameEdit(const char * txt, const char * response1, const char * response2, void(*code1)(void *data), void(*code2)(void *data), int maxChars, int flags );

void dialogTimed( int type, const char * txt, const char * response1, const char * response2, void(*code1)(void *data), void(*code2)(void *data), int flags, int time);

void dialogArchitect( int type, int count );

void dialogMoralChoice( const char *ltext, const char * rtext, void(*code1)(void*data), void(*code2)(void*data), const char *lwatermark, const char *rwatermark );

void dialogChooseNameImmediate(int first_time);


void dialogRemove(const char *pch, void (*code1)(void *data), void (*code2)(void *data) );
	// Removes first dialog which matches give text, code1 and/or code2.

bool dialogInQueue(const char *pch, void (*code1)(void *data), void (*code2)(void *data) );
	// Pass NULL to match anything.

void dialogClearQueue( int game );

void displayDialogQueue();

char *dialogGetTextEntry(void);
void dialogClearTextEntry(void);
void dialogSetTextEntry( char * str );
void dialogYesNo( int yesno ); // 1 - yes, 0 - no
void dialogAnswer( const char * text );
int dialogMoveLocked(int wdw);
#endif