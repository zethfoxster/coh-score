#ifndef UIEDIT_H
#define UIEDIT_H

#include "uiListView.h"

typedef struct TTDrawContext TTDrawContext;
typedef struct TTTextWrapper TTTextWrapper;
typedef struct KeyInput KeyInput;

typedef enum uiEditAction
{
	kuiEditAction_ToEOL = -2,
	kuiEditAction_ToCursor = -1,
	kuiEditAction_Max = 0x7fffffff
} uiEditAction;

typedef struct UIUnderline
{
	bool active;
	int iStart;
	int diLen;
} UIUnderline;

void uiunderline_Clear(UIUnderline *underline);

typedef struct
{
	unsigned short* lineText;			// What is the text that should go on this line?
	unsigned int hasNewLine : 1;		// reformatting does not proceed beyond these type of lines.
	unsigned int dirty		: 1;		// requires re-formatting.
	UIUnderline underline;
} UIEditLine;


typedef struct UIEdit UIEdit;
typedef void (*UIEditInputHandler)(UIEdit* edit);

typedef struct UIEditCursor
{
	unsigned int lineIndex;	// Which line is the cursor on?
	int characterIndex;		// Which character is the cursor on in the line?
	int lastXPos;			// Last X position the cursor was drawn at. 
							// ab: not quite. the last rendered x relative to the adjusted 
							// left margin, which may be -100, for example.
							// @deprecated: remove this when possible

	int xDrawn;    // the last rendered X position in screen coords
} UIEditCursor;

typedef enum UIEditContentLimit
{
	UIECL_NONE,
	UIECL_WIDE,
	UIECL_UTF8,
	UIECL_UTF8_ESCAPED,
} UIEditContentLimit;
 
typedef struct UIEdit
{
	UIEditCursor cursor;				// Where is the cursor?
	UIEditLine** lines;

	unsigned int inEditMode		: 1;
	unsigned int canEdit		: 1;
	unsigned int dirty			: 1;	// Any reformat required?
	unsigned int boundsChanged	: 1;	// Bounds changed. All lines should be reformatted.
	unsigned int cursorMoved	: 1;
	unsigned int password		: 1;	// Display *s
	unsigned int disallowReturn	: 1;	// Don't let CRs into the text
	unsigned int respectWhiteSpace : 1; // allow tabs and carraige returns
	unsigned int rightAlign		: 1;	// align text to the right
	unsigned int noClip			: 1;	// Don't clip.
    unsigned int dobox          : 1;    // do a box around the edit and do mouse focus.
	bool nonAsciiBlocked;
		// ASCII characters only allowed

	UIBox boundsPage; // the parent to the bounds for the text
	UIBox boundsText;
	struct BoundsLayout
	{
		F32 leftIndent;
	} boundsLayout;

	float z;
	float minWidth;

	TTDrawContext* font;
	int textColor;
	float textScale;
	int fontHeight;

	TTTextWrapper* wrapper;
	UIEditInputHandler inputHandler;

	float cursorInterval;
	float cursorTimer;
	int drawCursor;

	float vertScroll;

	int contentUTF8ByteCount;
	int contentCharacterCount;

	UIEditContentLimit limitType;
	int limitCount;

	char *keyClick;
	char *fullClick;
} UIEdit;

//UIEditLine* uiEditLineCreate();
//void uiEditLineDestroy(UIEditLine* line);
//void uiEditLinePrepareDestroy(UIEdit* edit, UIEditLine* line);

/*ok*/ UIEdit* uiEditCreate(); 
/*ok*/ void uiEditDestroy(UIEdit* edit);
/*ok*/ void uiEditClear(UIEdit* edit);
/*ok*/ void uiEditSetCursorPos(UIEdit* edit, int idx);

void uiEditSetBounds(UIEdit* edit, UIBox bounds);
void uiEditSetLeftIndent(UIEdit *edit, F32 dxIndent);

void uiEditSetMinWidth(UIEdit* edit, float minWidth);
//int uiEditCanReceiveInput(UIEdit* edit);
// void uiEditDisplay(UIEdit* edit);
void uiedit_Backspace(UIEdit *edit, U32 amount);
void uiEditDefaultKeyboardHandler(UIEdit* edit, KeyInput* input);
void uiEditCommaNumericKeyboardHandler(UIEdit* edit, KeyInput* input);
void uiEditDefaultInputHandler(UIEdit* edit);
void uiEditDefaultMouseHandler(UIEdit* edit);
void uiEditCommaNumericInputHandler(UIEdit* edit);
void uiEditCommaNumericUpdate(UIEdit *edit);
//void uiEditHandleInput(UIEdit* edit);
void uiEditProcess(UIEdit* edit); // the handler for uiEdit

// void uiEditRecalcContentCount(UIEdit* edit);

// returns an estring.
// unsigned short* uiEditGetWideText(UIEdit* edit);
char* uiEditGetUTF8Text(UIEdit* edit);
char* uiEditGetUTF8TextBuf(char **res, UIEdit* edit);
char* uiEditGetUTF8TextStatic(UIEdit* edit);

// void uiEditPasteWideText(UIEdit* edit, unsigned short* text);

void uiEditSetWideText(UIEdit* edit, unsigned short* text);
void uiEditSetUTF8Text(UIEdit* edit, char* text);

void uiEditSetFont(UIEdit* edit, TTDrawContext* font);
void uiEditSetFontScale(UIEdit* edit, float scale);
unsigned int uiEditGetFullDrawHeight(UIEdit* edit);
void uiEditSetZ(UIEdit *edit, float z);

int uiEditOnLostFocus(UIEdit* edit, void* requester);

// A simpler interface for replacing display_edit_string easier.
UIEdit *uiEditCreateSimple(char *pch, int size, TTDrawContext *font, int color, UIEditInputHandler handler);
bool uiEditTakeFocus(UIEdit *edit);
void uiEditReturnFocus(UIEdit *edit);
bool uiEditHasFocus( UIEdit *edit );
void uiEditEditSimple(UIEdit *edit, float x, float y, float z, float w, float h, TTDrawContext *font, int type, int password);
void uiEditSetClicks(UIEdit *edit, char *keyClick, char *fullClick);
void uiEditDisallowReturns(UIEdit *edit, int disallow);
void editInputHandler(UIEdit* edit);
void uiEditShowCursor(UIEdit* edit);

void uiEditInsertUnicodeStr(UIEdit* edit, unsigned short* str, unsigned int charCount);
void uiEditRemoveCharacters(UIEdit* edit, int iCharacter, int dCharCount );

void uiEditSetUnderlineStateCurLine(UIEdit *edit, bool active, int iStart, int diLen);
#endif
