#ifndef _UIEDITTEXT_H
#define _UIEDITTEXT_H

#include "uiInclude.h"

typedef struct TTDrawContext TTDrawContext;

typedef enum ProfileNames
{
	EDIT_ACCOUNT_NAME,
	EDIT_ACCOUNT_PASSWORD,
	EDIT_TOTAL_BLOCKS,
}ProfileNames;


#define MAX_CLIP_CHARS  ( 512 )
typedef struct clip
{
	char    buf[ MAX_CLIP_CHARS ];
	int     len;
}Clip;


typedef struct edit_text_block
{
	int     single_line;
	int     selected;
	int     sel1;       //where drag started
	int     sel2;       //curr drag part.
	int     sel_start;  //lesser of the ends
	int     sel_end;    //greater of the ends.  This part is non-inclusive. ie if start = 0,
	int     cursor_idx; //end = 1 then only char 0 will be in select region.
	int     len;        //How many characters are there in the string?
	int     realLength; //How many bytes are there in the string?
	int     total;      //How many bytes can the string buffer hold?
	char    *str;       //String buffer.
	float   cblink_timer;
}EditTextBlock;

extern EditTextBlock edit_block[ EDIT_TOTAL_BLOCKS ];
extern EditTextBlock textblock;

extern Clip clip;

void initEditTextBlock(	EditTextBlock *e, int single_line, char *storage, int storage_len );
void ebAddCharacter(EditTextBlock* block, char character);
void ebAddWideCharacter(EditTextBlock* block, unsigned short character);
int display_edit_string( EditTextBlock *e, int xstart, int y, int z, 
						 int width, int height, float sc, 
						 TTDrawContext *fnt, int type, int password );

extern int editting_on;

#define TEXT_RETURN	(1<<0)
#define TEXT_ESCAPE (1<<1)
#define TEXT_UP		(1<<2)
#define TEXT_DOWN	(1<<3)

int enter_text( EditTextBlock *e, int filter );

int drawCursorText( int xp, int yp, int zp, float sc, int width, int height, char **text, 
				    int *line_style, int drawCursor, int cursorLocation );

typedef struct Array Array;
void drawTextStd( int x, int y, int z, float scale, int height, float line_ht, Array* text, int*color, int align_bot_ht );

void dumpFormattedTextArray( int xp, int yp, int zp, float xsc, float ysc, int fitHeight, float font_height,
							Array* textArray, int *paragraph_style, int style_is_color, int center, int drawCursor, int cursorLocation, int align_bot_ht );

void wrappedText( int x, int y, int z, int wd, char* txt, int type );

#define	FONT_HEIGHT			( 15.f )

int editTextOnLostFocus(EditTextBlock* block, void* requester);
#endif
