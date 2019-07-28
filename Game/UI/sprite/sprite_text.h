/***************************************************************************
 *     Copyright (c) 2000-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef SPRITE_TEXT_H__
#define SPRITE_TEXT_H__

#include "wdwbase.h"
#include "stdtypes.h"
#include <stdarg.h>

typedef struct TTDrawContext TTDrawContext;

void prnt_raw(float x, float y, float z, float xsc, float ysc, const char * str);
void prnt(float x, float y, float z, float xsc, float ysc, const char * str, ...);
void prntEx(float x, float y, float z, float xsc, float ysc, int flags, const char * str, ...);
void cprnt(float x, float y, float z, float xsc, float ysc, const char * str, ...);
void cprntEx(float x, float y, float z, float xsc, float ysc, int flags, const char * str, ...);
void cprntFittedEx(float x, float y, float z, float xsc, float ysc, float wd, int flags, const char *str, ...);
int commandParameterStrcmp( char * command_key, char * string );
int commandParameterStrstr( char * command_key, char * string );

void font_ital( int on );
void font_bold( int on );
void font_outl( int on );
void font_ds( int on );
void font(TTDrawContext * f);
TTDrawContext* font_get();
void font_color(int clr0, int clr1);
int font_color_get_top();
int font_color_get_bottom();
void font_set( TTDrawContext * font, int ital, int bold, int outline, int dropShadow, int clr0, int clr1 );


int str_wd(TTDrawContext * font, float xscale, float yscale, const char * str, ...);
int str_wd_notranslate(TTDrawContext * font, float xscale, float yscale, const char  * str);
int str_layoutWidth(TTDrawContext* font, float xScale, float yScale, const char* str, ...);
float str_sc( TTDrawContext * font, float avail_wd, const char  * str, ...);
void vstr_dims(TTDrawContext * font, float xscale, float yscale, int center_text, CBox * box, const char *str, int noTranslate, va_list va);
void str_dims_notranslate(TTDrawContext * font, float xscale, float yscale, int center_text, CBox * box, const char  * str, ...);
void str_dims(TTDrawContext * font, float xscale, float yscale, int center_text, CBox * box, const char * str, ...);
float str_height(TTDrawContext * font, float xscale, float yscale, int center_text, const char * str, ...);

void determineTextColor(int rgba[4]);

// poor client locale functions looking for a home..
char* timerFriendlyDateFromSS2(char *datestr, int datestrLength, U32 seconds);
char* timerFriendlyHourFromSS2(char* datestr, int datestrLength, U32 seconds, int rounded, int showMin);	// rounded adds a few seconds to try to get to even hour
char* timerFriendlyHour(char* datestr, int datestrLength, int hour, int min, int showMin);
int timerHourFromSS2Value(U32 seconds, int rounded);


#endif /* #ifndef SPRITE_TEXT_H__ */

/* End of File */
