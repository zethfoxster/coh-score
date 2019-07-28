#ifndef UILISTVIEW_H
#define UILISTVIEW_H

#include "uiScrollBar.h"
#include "uiBox.h"
#include "stdtypes.h"

//--------------------------------------------------------------------------------------------------------------------
// Widet3Part
//--------------------------------------------------------------------------------------------------------------------
typedef enum
{
	WPM_NONE,
	WPM_HORIZONTAL,
	WPM_VERTICAL,
} WidgetPartMode;

typedef struct 
{
	union
	{
		struct 
		{
			char* top;
			char* mid;
			char* bottom;
		};
		struct 
		{
			char* left;
			char* center;
			char* right;
		};
		struct 
		{
			char* part1;
			char* part2;
			char* part3;
		};
	};

	WidgetPartMode mode;
} Widget3Part;

void Widget3PartDraw(Widget3Part* widget, float x, float y, float z, float scale, unsigned int size, int color);
int Widget3PartHeight(Widget3Part* widget);
void Widget3PartGetWidths(Widget3Part* widget, unsigned int* part1Width, unsigned int* part2Width, unsigned int* part3Width);

//--------------------------------------------------------------------------------------------------------------------
// UIColumnHeader 
//--------------------------------------------------------------------------------------------------------------------

typedef struct 
{
	char* name;
	char* caption;
	float minWidth;
	float maxWidth;
	float curWidth;
	float relWD;

	bool resizable;
	bool dragging;
	bool hidden;

} UIColumnHeader;

extern int gDivSuperName;
extern int gDivSuperMap;
extern int gDivSuperTitle;
extern int gDivEmailFrom;
extern int gDivEmailSubject;
extern int gDivFriendName;
extern int gDivLfgName;
extern int gDivLfgMap;

UIColumnHeader* uiCHCreate(void);
void uiCHDestroy(UIColumnHeader* column);

//--------------------------------------------------------------------------------------------------------------------
// UIListViewHeader
//--------------------------------------------------------------------------------------------------------------------
typedef struct 
{
	UIColumnHeader** columns;
	int selectedColumn;
	unsigned int width;
	unsigned int minWidth;
	int drawColor;
} UIListViewHeader;

UIListViewHeader* uiLVHCreate(void);
void uiLVHDestroy(UIListViewHeader* header);

int uiLVHAddColumn(UIListViewHeader* header, char* name, char* caption, float minWidth);
int uiLVHAddColumnEx(UIListViewHeader* header, char* name, char* caption, float minWidth, float maxWidth, int resizable);
int uiLVHFindColumnIndex(UIListViewHeader* header, char* name);
void uiLVHSetColumnWidth(UIListViewHeader* header, char* name, int width);
void uiLVHSetColumnCaption(UIListViewHeader* header, char* name, char * caption);

void setSavedDividerSettings(void);
void saveDividerSettings(void);

int uiLVHFinalizeSettings(UIListViewHeader* header);
int uiLVHSetWidth(UIListViewHeader* header, unsigned int width);
int uiLVHSetMinWidth(UIListViewHeader* header, unsigned int width);
unsigned int uiLVHGetMinDrawWidth(UIListViewHeader* header);
unsigned int uiLVHGetHeight(UIListViewHeader* header);

void uiLVHSetDrawColor(UIListViewHeader* header, int color);

typedef struct 
{
	// All fields are read-only.
	UIListViewHeader* header;		// Iterating through columns in this header.

	int columnIndex;				// The index ofd the column the iterator is on.
	UIColumnHeader* column;			// The column the iterator is on.

	int columnStartOffset;			// Where should this column start on the row?
	int currentWidth;				// The width of this column.
	int extraColumnSpacing;			// The amount of space added to column::minWidth to get currentWidth.

	int headerPart1Width;			// Width of the the "rounded" part" that sticks off of the end of the headers.
	int headerPart3Width;
	
	int separatorWidth;				// How wide is the seperator?
	UIBox clipBox;					// ClipBox for
} UIColumnHeaderIterator;

int uiCHIGetIterator(UIListViewHeader* header, UIColumnHeaderIterator* iterator);
int uiCHIGetNextColumn(UIColumnHeaderIterator* iterator, float scale);


void uiLVHDisplay(UIListViewHeader* header, float x, float y, float z, float scale);

//--------------------------------------------------------------------------------------------------------------------
// UIListView
//--------------------------------------------------------------------------------------------------------------------
typedef struct UIListView UIListView;

typedef enum
{
	UILVSS_FORWARD,
	UILVSS_BACKWARD,
	UILVSS_MAX,
} UIListViewSortDirection;

/* UILIstViewDisplayItemFunc
 *	Each list view may deal with display its items differently.
 *	When a list view is displayed, the list view first draws the header, then it calls the display 
 *	callback on each of the items.  The function is expected to return an UIBox which specifies where
 *	the item was drawn.
 */
typedef UIBox (*UIListViewDisplayItemFunc)(UIListView* list, PointFloatXYZ rowOrigin, void* userSettings, void* item, int itemIndex);
typedef int (*UIListViewItemCompare)(const void* item1, const void* item2);

struct UIListView
{
	UIListViewHeader* header;
	void** items;				// EArray of items to be displayed by the list view.
	void* userSettings;			// Extra user data to be store with the list view
	UIBox** itemWindows;		// Used to determine if a user has clicked on an item.

	void* selectedItem;			// Which item is selected in the list view?
	void* mouseOverItem;		// Which item is the mouse over?

	UIListViewDisplayItemFunc	displayItem;

	UIListViewItemCompare*		compareItem;	// Sort function for each column in the header.
	UIListViewSortDirection		sortDirection;
	unsigned int minHeight;
	unsigned int height;		// How tall is the table right now?
	unsigned int rowHeight;		// How tall is each row?

	unsigned int scrollOffset;	// How much to scroll the list down?
	ScrollBar verticalScrollBar;

	unsigned int enableSelection : 1;
	unsigned int enableMouseOver : 1;
	unsigned int newlySelectedItem : 1;
	unsigned int itemsAreClickable : 1;

	float scale;

	int drawColor;
};

UIListView* uiLVCreate(void);
void uiLVDestroy(UIListView* list);


void uiLVHandleInput(UIListView* list, PointFloatXYZ origin);
void uiLVDisplay(UIListView* list, PointFloatXYZ origin);
void uiLVSetClickable(UIListView* list,int clickable);

int uiLVSetWidth(UIListView* list, unsigned int width);
int uiLVSetHeight(UIListView* list, unsigned int height);
unsigned int uiLVGetHeight(UIListView* list);
void uiLVSetScale(UIListView* list, float scale);

int uiLVSetMinHeight(UIListView* list, unsigned int height);
unsigned int uiLVGetMinDrawHeight(UIListView* list);
unsigned int uiLVGetFullDrawHeight(UIListView* list);
unsigned int uiLVSetRowHeight(UIListView* list, unsigned int height);
int uiLVFinalizeSettings(UIListView* list);

void uiLVSetDrawColor(UIListView* list, int color);

void uiLVEnableSelection(UIListView* list, int enable);
void uiLVEnableMouseOver(UIListView* list, int enable);

int uiLVAddItem(UIListView* list, void* item);
int uiLVFindItem(UIListView* list, void* item);
int uiLVRemoveItem(UIListView* list, void* item);
int uiLVSelectItem(UIListView* list, unsigned int itemIndex);
int uiLVClear(UIListView* list );
int uiLVClearEx(UIListView* list, void (*destructor)(void*) );
int uiLVSortList(UIListView* list, UIListViewSortDirection sortDirection, UIListViewItemCompare itemCompare);
int uiLVSortBySelectedColumn(UIListView* list);

int uiLVBindCompareFunction(UIListView* list, char* columnName, UIListViewItemCompare itemCompare);

#endif