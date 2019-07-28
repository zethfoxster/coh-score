
#ifndef UIOPTIONS_H
#define UIOPTIONS_H


#include "uiOptions_Type.h"

int mapOptionGet(int id);
void mapOptionSet(int id, int val);
void mapOptionSetBitfield(int id, int val);
int optionGet(int id );
F32 optionGetf( int id );
void optionSet(int id, int val, bool save);
void optionSetf(int id, F32 fVal, bool save);
void optionToggle(int id, bool save);
UserOption * userOptionGet( int id );

void optionToggleByName( char * name );
void optionSetByName( char * name, char *val );
void optionList();

void optionSave(char * pchFile);
void optionLoad(char * pchFile);


typedef enum EOptionType
{
	kOptionType_Nop,
	kOptionType_Bool,
	kOptionType_Func,
	kOptionType_String,
	kOptionType_FloatSlider,
	kOptionType_IntSlider,
	kOptionType_Title,
	kOptionType_SnapSlider,
	kOptionType_SnapMinMaxSlider,
	kOptionType_MinMaxSlider,
	kOptionType_IntMinMaxSlider,
	kOptionType_TitleUltra,
	kOptionType_SnapSliderUltra,
	kOptionType_IntSnapSlider,
} EOptionType;

typedef enum BubbleColorType
{
	BUB_COLOR_TEXT,
	BUB_COLOR_BACK,
	BUB_COLOR_TOTAL,
}BubbleColorType;

typedef enum OptionDisplayType
{
	OPT_DISPLAY,
	OPT_DISABLE,
	OPT_CUSTOMIZED, // Same as Disable but the UI reports "Customized" instead of "N/A"
	OPT_HIDE
}OptionDisplayType;


typedef struct GameOptions GameOptions;
typedef struct ToolTip ToolTip;
typedef struct HelpButton HelpButton;

typedef struct GameOptions
{
	int			id;					// UserOption Id

	EOptionType eTypeDisplay;		// what kind of thing is being displayed
	void		*pvDisplay;			// what the display type expects
	char		*pchHelp;			// Help text explaining the option
	void		*pvDisplayParam;    // ditto
	EOptionType eTypeToggle;		// Action to perform
	void		*pvToggle;			// data to perform action
	void		*pvToggleParam;     // ditto
	char		*pchTrue;           // Strings to display when option type is bool
	char		*pchFalse;
	OptionDisplayType (*doDisplay)(GameOptions *); //callback function that determines display state
	void		(*getSliderMinMax)(float *, float*);	// call back to get slider min and max
	void        *pSave;             // Where to save the initial value
	ToolTip     *pToolTip;          // Tooltip if we have one
	HelpButton  *pHelpButton;
} GameOptions;

void setNewOptions();
int optionsWindow();

#endif

