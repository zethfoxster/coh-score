/***************************************************************************
 *     Copyright (c) 2000-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef SPRITE_FONT_H__
#define SPRITE_FONT_H__

typedef struct TTDrawContext TTDrawContext;
typedef struct TTSimpleFont TTSimpleFont;

void loadFonts(void);

// Available true type draw contexts
// Each draw context is hooked up to their corresponding TTFont.
extern TTDrawContext *font_grp;

// ab: testing
extern TTDrawContext tinyfont;

// Century Gothic
extern TTDrawContext hybrid_9;
extern TTDrawContext hybrid_9_dynamic;
extern TTDrawContext hybrid_12;
extern TTDrawContext hybridbold_12;
extern TTDrawContext hybrid_14;
extern TTDrawContext hybridbold_14;
extern TTDrawContext hybrid_18;
extern TTDrawContext hybridbold_18;

// mont demi-bold
extern TTDrawContext game_9;
extern TTDrawContext game_9_dynamic;
extern TTDrawContext game_10;
extern TTDrawContext game_10_dynamic;
extern TTDrawContext game_12;
extern TTDrawContext game_12_dynamic; 
extern TTDrawContext game_14;
extern TTDrawContext game_18;

// mont heavy-bold
extern TTDrawContext gamebold_9;
extern TTDrawContext gamebold_12;
extern TTDrawContext gamebold_14;
extern TTDrawContext gamebold_18;

// red circle
extern TTDrawContext title_9;
extern TTDrawContext title_12;
extern TTDrawContext title_12_no_outline;
extern TTDrawContext title_14;
extern TTDrawContext title_18;
extern TTDrawContext title_18_thick;
extern TTDrawContext title_24;

// Chat font
extern TTDrawContext chatThin;
extern TTDrawContext chatFont;
extern TTDrawContext chatBold;

// oft-used SMF fonts
extern TTDrawContext smfLarge;
extern TTDrawContext smfDefault;
extern TTDrawContext smfSmall;
extern TTDrawContext smfVerySmall;
extern TTDrawContext smfMono;
extern TTDrawContext smfComputer;

extern TTDrawContext profileFont;
extern TTDrawContext profileNameFont;
extern TTDrawContext entityNameText;
extern TTDrawContext referenceFont;

// Internal sub-fonts (referenced by TexWords system)
extern TTSimpleFont* ttRedCircle;
extern TTSimpleFont* ttMontBold;
extern TTSimpleFont* ttMont;
extern TTSimpleFont* ttVerdana;
extern TTSimpleFont* ttVerdanab;
extern TTSimpleFont* ttMingLiu;
extern TTSimpleFont* ttGulim;
extern TTSimpleFont* ttTahomaBD;
extern TTSimpleFont* ttCourier;
extern TTSimpleFont* ttDigitalb;
extern TTSimpleFont* ttTiny;

#endif /* #ifndef SPRITE_FONT_H__ */

/* End of File */
