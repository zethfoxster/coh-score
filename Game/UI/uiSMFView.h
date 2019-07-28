/***************************************************************************
 *     Copyright (c) 2003-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef UISMFVIEW_H__
#define UISMFVIEW_H__

typedef void (*uiTabActionFunc)(void*);	
typedef struct uiTabControl uiTabControl;
typedef struct SMFView SMFView;
typedef struct TextAttribs TextAttribs;

SMFView *smfview_CreateInPlace(SMFView *pview, int wnd);
SMFView *smfview_Create(int wnd);
	// Creates a scrollable SMF view for the given window. The scrollbar
	// (if needed) will be placed on the right border of the window.
	// Pass 0 (zero) if you don't have a window to hang off of, then use
	// SetBaseLocation to set the other stuff.

void smfview_Destroy(SMFView *pview);
	// Destroys the SMFView created by smfview_Create.

void smfview_SetBaseLocation(SMFView *pview, int x, int y, int z, int wd, int ht);
	// If there is no window to attach to (by passing 0 to ...Create), you
	//  can use this function to specify the base coords the relative
	// location is based from.

void smfview_SetLocation(SMFView *pview, int x, int y, int z);
	// Sets the location within the window of the text region. This is the
	// relative location with the upper-left corner of the window being
	// (0, 0). This usually doesn't need to change after initialization
	// unless you are making some kind of fancy resizing window.

void smfview_SetSize(SMFView *pview, int wd, int ht);
	// Sets the width and height of the text region in the window. The
	// text will be drawn slightly within this region. Windows whose
	// text expands when the window resizes will call this often. Changing
	// the width causes a reparse. Calling this with the same values is safe,
	// though, since the new values are checked against the old ones first.

void smfview_SetWindowRadius(SMFView *pview, int radius);
	// Windows with a radii which isn't R10 should call this at creation time
	// to set radius to the right size.

void smfview_SetText(SMFView *pview, char *pch);
	// Sets the text to be parsed, formatted, and drawn. Changing the pointer
	// causes a reparse automatically. Calling with the same pointer all the
	// time is safe, though, since the new value is checked against the
	// old one first. If you change the contents of the pointer and therefore
	// need to get it reformatted, you need to call ...Reparse to flag the
	// data as changed.

void smfview_SetAttribs(SMFView *pview, TextAttribs *pattrs);
	// Sets basic attributes for the displayed text (until modified by
	// the markup itself).
void smfview_SetScale(SMFView *pview, float scale);

void smfview_Reparse(SMFView *pview);
	// Forces the text to be reparsed and reformatted on the next ...Draw.

void smfview_Reformat(SMFView *pview);
	// Forces the text to be reformatted on the next ...Draw.

int smfview_GetHeight(SMFView *pview);
	// Determines how tall the formatted text will be. Parses and formats
	// the text (if it needs to), but doesn't draw it.

void smfview_setAlignment( SMFView *pview, int alignment );
	// cram in alignment

void smfview_Draw(SMFView *pview);
	// Draws the text with scroll bar as specified above.

void smfview_DrawWithCallback(SMFView *pview, int (*callback)(char *));
	// Draws the text with scroll bar as specified above.

void smfview_AddTabs(SMFView *pview, char** tabNames, uiTabActionFunc func);
	// Add tabs from a list of names with a change callback

bool smfview_HasFocus(SMFView *pview);

void smfview_SetScrollBar(SMFView *pview, bool hasScrollBar);

void smfview_ResetScrollBarOffset(SMFView *pview);
struct SMFBlock *smfview_GetBlock(SMFView *view);

#ifdef SMFVIEW_PRIVATE
	typedef struct SMFBlock SMFBlock;
	typedef struct ScrollBar ScrollBar;

	typedef struct SMFView
	{
		char *pch;
		int x;
		int y;
		int z;
		int wd;
		int ht;

		int xbase;
		int ybase;
		int zbase;
		int wdbase;
		int htbase;

		int radius;
		SMFBlock *block;
		TextAttribs *pattrs;
		bool bReparse;
		bool bReformat;
		ScrollBar sb;
		bool hasScrollBar;
		uiTabControl* tabs;
	} SMFView;
#endif

#endif /* #ifndef UISMFVIEW_H__ */

/* End of File */

