#ifndef UICOMBOBOX_H
#define UICOMBOBOX_H

typedef struct ScrollBar ScrollBar;

typedef struct ComboSetting
{
	int id;
	char * text;
	char * title;
	char * icon;
}ComboSetting;

#define CCE_UNSELECTABLE	1
#define CCE_TEXT_NOMSPRINT	2
#define CCE_TITLE_NOMSPRINT	4

typedef struct ComboCheckboxElement
{
	int			selected;
	int			flags;
//	int			unselectable;
	int			id;
	int			color;
	int			icon_color;
	AtlasTex	*icon;
	char		*txt;
	char		*title;
	void		*data;
}ComboCheckboxElement;

typedef struct ComboBox
{
	int state;
	int currChoice;
	int type;
	int wdw;
	int reverse;
	int newlySelectedItem;  // regular combo box only
	int all;
	int unselectable;  //invisible
	int locked;
	int changed;
	int	architect;
	int color;
	int back_color;
	int highlight_color;

	float x;
	float y;
	float z;
	float sc;

	float wd;
	float dropWd;
	float ht;
	float expandHt;
	float scale;

	char				 *title;
	const char			 **strings;
	ComboCheckboxElement **elements;

	ScrollBar *sb;
}ComboBox;
#define COMBOBOX_ARCHITECT_UNSELECTED 1
#define COMBOBOX_ARCHITECT_SELECTED 2

// This is the default combobox, it displays in the window the current selected string.  Array of strings is passed in here
void combobox_init( ComboBox *cb, float x, float y, float z, float wd, float ht, float expandHt, const char **strings, int wdw );

// This isn't so much a combobox, but a drop down list of checkboxes.  The same title is always displayed.  Each element must be added with combocheckbox_add
void combocheckbox_init( ComboBox *cb, float x, float y, float z, float wd, float dropWd, float ht, float expandHt, char *title, int wdw, int reverse, int all );

// This is an expanded combobox, it can display a title that is different than a string in the list.  Elements are added via comboboxTitle_add
void comboboxTitle_init( ComboBox *cb, float x, float y, float z, float wd, float ht, float expandHt, int wdw );

// Add an element to the checkbox
void combocheckbox_add( ComboBox *cb, int selected, AtlasTex *icon, const char * txt, int id );
void comboboxTitle_add( ComboBox *cb, int selected, AtlasTex *icon, const char * txt, const char * title, int id, int color, int icon_color, void * data  );

// for repeated combo boxes
void comboboxSharedElement_add( ComboCheckboxElement ***elements, AtlasTex *icon, char *txt, char *title, int id, void * data );

// clear and free all of the elements
void comboboxRemoveAllElements( ComboBox * cb );

// clear out a combobox to be re-inited
void comboboxClear(ComboBox *cb);

// Returns true if an element with matching id exists
int combobox_hasID( ComboBox *cb, int id);

// Returns true if an element with matching id is selected
int combobox_idSelected( ComboBox *cb, int id);

// Returns the ID of the currChoice or the first selected element
int combobox_GetSelected( ComboBox *cb);

// sets a specific element to be selected
void combobox_setId( ComboBox *cb, int id, int selected );

// find the first idx that has passed id, starting at iStart
// -1 if none found
int combobox_mpIdxId( ComboBox *cb, int id, int iStart );

// set the visible item to the passed id
void combobox_setChoiceCur( ComboBox *cb, int id );

// set a specific element to be unselectable
void combobox_setSelectable( ComboBox *cb, int id, int selectable );

// force a specific element's flags to a given value
void combobox_setFlags( ComboBox *cb, int id, int flags );

// set the location of a combobox
void combobox_setLoc( ComboBox *cb, float x, float y, float z );

// Draws the combobox
const char *combobox_display( ComboBox *cb );

// The profile combobox is special, so this function is just for it.
int combobox_displayRegister( ComboBox *cb, int radius, int color, int back_color, int text_color, char *defaultText, int locked, float screenScaleX, float screenScaleY, int xOffset, int yOffset );
int combobox_displayHybridRegister( ComboBox *cb, int radius, int color, int back_color, int text_color );

int combobox_getBitfieldFromElements( ComboBox *cb );
void combobox_setElementsFromBitfield( ComboBox * cb, int bitfield  );

#endif
