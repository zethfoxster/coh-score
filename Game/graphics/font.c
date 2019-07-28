#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "font.h"
#include "model.h"
#include "memcheck.h"
#include "win_init.h"
#include "tex.h"
#include "render.h"
#include "input.h"
#include "cmdgame.h"
#include "sprite.h"
#include "renderfont.h"
#include "edit_cmd.h"
#include "assert.h"
#include "file.h"
#include "timing.h"


#define FONT_MAX		16
#define FONT_TEXTBUFMAX	100000
#define FONT_MESSAGEMAX	5000

static Font			*fonts[FONT_MAX];
static S32			font_count,fonts_just_removed;

static char			textbuf[FONT_TEXTBUFMAX];
static S32			textbuf_count;

static FontMessage	messages[FONT_MESSAGEMAX];
static S32			message_count;

static CRITICAL_SECTION state_stack_critical_section;
static int			stack_depth;
static FontState	state_stack[16];
static FontState	curr_state = { {255,255,255,255} };
static FontState	default_state = 
{
	{ 255,255,255,255},
	1,1,
	0
};

void fontInitCriticalSection()
{
	InitializeCriticalSection(&state_stack_critical_section);
}

static void fontPushState()
{
	if (++stack_depth >= ARRAY_SIZE(state_stack))
		return;
	state_stack[stack_depth-1] = curr_state;
}

static void fontPopState()
{
	if (--stack_depth >= ARRAY_SIZE(state_stack))
		return;
	curr_state = state_stack[stack_depth];
}

void fontSaveText(char *buf,int size)
{
int		*mem,msg_size;

	msg_size = sizeof(messages[0]) * message_count;
	mem = (int *) buf;
	if (msg_size + textbuf_count + 8 >= size)
	{
		mem[0] = mem[1] = 0;
		return;
	}
	mem[0] = textbuf_count;
	mem[1] = message_count;
	memcpy(&mem[2],messages,msg_size);
	memcpy(&mem[3 + msg_size/4],textbuf,textbuf_count);
}

void fontRestoreText(char *buf)
{
int		*mem,msg_size;

	mem = (int *)buf;
	textbuf_count = mem[0];
	message_count = mem[1];
	msg_size = sizeof(messages[0]) * message_count;
	memcpy(messages,&mem[2],msg_size);
	memcpy(textbuf,&mem[3 + msg_size/4],textbuf_count);
}

void fontInit()
{
	font_count = 0;
	textbuf_count = message_count = 0;
	curr_state.font = 0;
	fonts_just_removed = 1;
	curr_state.scaleX = curr_state.scaleY = 1;
}

int fontNew(FontInfo *font_info)
{
	Font		*font;
	S32			i;
	FontLetter	*l_info;

	assert(font_count < FONT_MAX);
	font = memAlloc(sizeof(Font));
	memset(font,0,sizeof(Font));

	font->font_bind	= texLoadBasic(font_info->Name,TEX_LOAD_NOW_CALLED_FROM_MAIN_THREAD, TEX_FOR_UI);
	font->texid		= texDemandLoad(font->font_bind);
	font->Letters = font_info->Letters;
	for(i = 0,l_info = font_info->Letters;i<FONT_MAXCHARS && l_info->Letter;i++,l_info++)
	{
		font->Indices[l_info->Letter] = i;

		if (!l_info->Width)
			l_info->Width	= font_info->Width;
		if (!l_info->Height)
			l_info->Height	= font_info->Height;
	}

	if (!curr_state.font) curr_state.font = font;
	fonts[font_count++] = font;
	return font_count-1;
}

void fontText(F32 x,F32 y,const char *str)
{
	FontMessage	*msg;
	FontState	*fstate;
	S32			len;

	if (message_count >= FONT_MESSAGEMAX-1)
		return;
	len = strlen(str) + 1;
	if (textbuf_count + len >= FONT_TEXTBUFMAX-1)
		return;

	msg				= &messages[message_count++];
	fstate			= &msg->state;
	msg->X			= x;
	msg->Y			= y;
	fstate->font	= curr_state.font;
	fstate->scaleX	= curr_state.scaleX;
	fstate->scaleY	= curr_state.scaleY;
	msg->StringIdx	= textbuf_count;
	memcpy(msg->state.rgba,curr_state.rgba,4);
	strcpy(textbuf+textbuf_count,str);
	textbuf_count += len;
}

void fontTextf(F32 x,F32 y,char const *fmt, ...)
{
char str[200];
va_list ap;

	va_start(ap, fmt);
	vsprintf(str, fmt, ap);
	va_end(ap);
	fontText(x,y,str);
}


void fontSysText(F32 x,F32 y,const char *str, U8 r, U8 g, U8 b)
{
	EnterCriticalSection(&state_stack_critical_section);
	fontPushState();
	curr_state = default_state;

	curr_state.rgba[0] = r;
	curr_state.rgba[1] = g;
	curr_state.rgba[2] = b;

	curr_state.font = fonts[0];
	fontText(x,y,str);
	fontPopState();
	LeaveCriticalSection(&state_stack_critical_section);
}

void xyprintf(int x,int y,char const *fmt, ...)
{
	#define bufferSize 1024
	va_list ap;
	int j1=0,j2=0,len;
	char str[bufferSize] = "";

	PERFINFO_AUTO_START("xyprintf", 1);
		va_start(ap, fmt);
		len = _vsnprintf(str, bufferSize, fmt, ap);
		str[bufferSize-1]='\0';
		assert(len < bufferSize);
		va_end(ap);
		if (x > TEXT_JUSTIFY/2)
			x = ((j1=x) - TEXT_JUSTIFY);
		if (y > TEXT_JUSTIFY/2)
			y = ((j2=y) - TEXT_JUSTIFY);
		x <<= 3;
		y <<= 3;
		if (j1)
			x += TEXT_JUSTIFY;
		if (j2)
			y += TEXT_JUSTIFY;
		fontSysText(x,y,str, 255,255,255 );
		#undef bufferSize
	PERFINFO_AUTO_STOP();
}

void xyprintfcolor(int x,int y,U8 r, U8 g, U8 b, char const *fmt, ...)
{
	#define bufferSize 1024
	va_list ap;
	int j1=0,j2=0,len;
	char str[bufferSize];

	PERFINFO_AUTO_START("xyprintfcolor", 1);
		va_start(ap, fmt);
		len = _vsnprintf(str, bufferSize, fmt, ap);
		str[bufferSize-1]='\0';
		assert(len < bufferSize);
		va_end(ap);
		if (x > TEXT_JUSTIFY/2)
			x = ((j1=x) - TEXT_JUSTIFY);
		if (y > TEXT_JUSTIFY/2)
			y = ((j2=y) - TEXT_JUSTIFY);
		x <<= 3;
		y <<= 3;
		if (j1)
			x += TEXT_JUSTIFY;
		if (j2)
			y += TEXT_JUSTIFY;
		fontSysText(x,y,str, r, g, b );

		#undef bufferSize
	PERFINFO_AUTO_STOP();
}

void fontDefaults()
{
Font	*t;

	t = curr_state.font;
	curr_state = default_state;
	curr_state.font = t;
}

void fontScale(F32 scale_x,F32 scale_y)
{
	curr_state.scaleX = scale_x;
	curr_state.scaleY = scale_y;
}

void fontColor(int rgb)
{
	curr_state.rgba[0] = ((rgb >> 16) & 255);
	curr_state.rgba[1] = ((rgb >>  8) & 255);
	curr_state.rgba[2] = ((rgb >>  0) & 255);
}

void fontAlpha(int a)
{
	curr_state.rgba[3] = a & 255;
}

void fontSet(S32 font_index)
{
	if (font_index < 0 || font_index >= font_count)
		return;
	if (fonts[font_index])
		curr_state.font = fonts[font_index];
}

Font* fontPtr(S32 font_index)
{
	if (font_index < 0 || font_index >= font_count)
		return NULL;
	return fonts[font_index];
}


F32 fontStringWidth(char *s)
{
F32 width = 0;

	for(;*s;s++)
		width += curr_state.font->Letters[curr_state.font->Indices[(int)*s]].Width * curr_state.scaleX;

	return width;
}

F32 fontStringWidthf(char *s, ...)
{
	F32 width = 0;
	char str[2048] = {0};
	va_list ap;

	va_start(ap, s);
	quick_vsprintf(SAFESTR(str), s, ap);
	va_end(ap);

	s = str;

	for(;*s;s++)
		width += curr_state.font->Letters[curr_state.font->Indices[(int)*s]].Width * curr_state.scaleX;

	return width;
}

F32	fontLocX(F32 x)
{
int		h,v;

	if (x > TEXT_JUSTIFY/2)
	{
		windowSize(&h,&v);
		x = h - (640.f - (x - TEXT_JUSTIFY));
	}
	return x;
}

F32	fontLocY(F32 y)
{
int		h,v;

	if (y > TEXT_JUSTIFY/2)
	{
		windowSize(&h,&v);
		y = v - (480.f - (y - TEXT_JUSTIFY));
	}
	return y;
}




extern int shell_menu();

void fontRenderEditor()
{
	if (fonts_just_removed)
	{
		fonts_just_removed = 0;
	}
	else
	{
		fontTouchTextures();
		if(1 != game_state.disable2Dgraphics || shell_menu())
			rdrRenderEditor( messages, message_count, textbuf_count, textbuf);
	}
	textbuf_count = message_count = 0;
}

void fontRenderGame()
{
	if (fonts_just_removed)
	{
		fonts_just_removed	= 0;
		textbuf_count		= message_count = 0;
	}
	else
	{
		if(!game_state.disable2Dgraphics || shell_menu())
			rdrRenderGame();
		else
			init_display_list();
	}
}

void fontRender()
{
	if ( shell_menu() )
	{
		fontRenderGame();
	}
	else
	{
		PERFINFO_AUTO_START("fontRenderGame", 1);
			fontRenderGame();
		PERFINFO_AUTO_STOP();
			
		init_display_list();	//ensure that all frames reinit my display list to fix overflow bug.
									//lf, 03/26/01
	}
}

int fontLoadImage(char *texname)
{
	FontLetter *letters;
	BasicTexture *bind;
	static FontInfo	info;

	bind = texLoadBasic(texname,TEX_LOAD_NOW_CALLED_FROM_MAIN_THREAD, TEX_FOR_UI);
	if (!bind)
		return 0;
	letters = calloc(sizeof(FontLetter),2);
	letters[0].Letter = 'X';
	letters[0].Width = bind->width;

	info.Name = texname;
	info.Height = bind->height;
	info.Width = bind->width;
	info.Letters = letters;

	return fontNew(&info);
}

void fontReplaceImage(int font_id,char *texname)
{
	fonts[font_id]->font_bind = texLoadBasic(texname,TEX_LOAD_NOW_CALLED_FROM_MAIN_THREAD, TEX_FOR_UI);
}

#define LOG_SIZE 5000
char mylog[LOG_SIZE];
int myloglength = 0;
int  numlogentries = 0;
int loginit = 0;
void displayLog()
{
	char * s;
	int i = 0;
	char temp[LOG_SIZE];

	while(numlogentries > 20)
	{
		s = strchr( mylog, '\n' );
		s++;
		strncpy(temp, s, LOG_SIZE);
		strncpy( mylog, temp, LOG_SIZE );
		numlogentries--;
	}

	if( game_state.fxdebug & ( FX_DEBUG_BASIC | FX_PRINT_VERBOSE ) )
	{
		char * t;
	
		if(numlogentries)
		{
			s = mylog;
			xyprintf( 1, 24 + i + TEXT_JUSTIFY, "'.' to clear, '/' to disable" );
			do
			{
				t = strchr(s, '\n');
				if( t )
				{
					*t = '\0';
					xyprintf( 1, 25 + i + TEXT_JUSTIFY, s );
					*t = '\n';
				}
				s = (t + 1);
				i++;
			} 
			while ( t );
		}

	}

}

void clearLog()
{
	memset( mylog, 0, sizeof(char) * LOG_SIZE );
	numlogentries = 0;
	myloglength = 0;

}

void printToScreenLog(int message_priority, char const *fmt, ...)
{
	char str[2000];
	va_list ap;

	if( !(game_state.fxdebug & ( FX_DEBUG_BASIC | FX_PRINT_VERBOSE ) ) )
		return;

	if( message_priority < 1 && !(game_state.fxdebug & FX_PRINT_VERBOSE) ) //only print high priority messages
		return ;

	if(!loginit)
	{ 
		clearLog();
		loginit = 1;
	}
	va_start(ap, fmt);
	vsprintf(str, fmt, ap);
	va_end(ap);

	assert( strlen(str) < 2000 );
	strcat(str, "\n");

	printf("%s",str);
	if( strlen(str) >= 300 )
	{
		str[299] = 0;
	}

	numlogentries++;
	if( numlogentries > 30 || (myloglength + strlen(str) >= LOG_SIZE ))
	{
		clearLog();
		printToScreenLog(0, "Log Overflow, Cleared Log");
	}
	
	myloglength += strlen(str);

	strcat( mylog, str );
	{
		static	FILE	*file;

		if (!file)
			file = fopen("c:\\client_warnings.txt","wt");
		if (file)
		{
			fprintf(file,"%s",str);
			fflush(file);
		}
	}
}

void fontTouchTextures()
{
	int		i;

	for(i=0;i<font_count;i++)
	{
		fonts[i]->texid = texDemandLoad(fonts[i]->font_bind);
		fonts[i]->tex_width = fonts[i]->font_bind->width;
		fonts[i]->tex_height = fonts[i]->font_bind->height;
	}
}

