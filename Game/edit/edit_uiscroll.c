#include "font.h"
#include "input.h"
#include "edit_cmd.h"
#include "cmdcommon.h"
#include "mathutil.h"
#include "edit_uiscroll.h"
#include "win_init.h"
#include "edit_cmd_group.h"
#include "uiInput.h"
#include "Menu.h"

static char *simpleName(char *name)
{
char	*s;

	s = strrchr(name,'/');
	if (!s)
		s = name;
	else
		s++;
	return s;
}

static void scrollAddName(ScrollInfo *scroll,char *name,int depth)
{
char	base[128],*s;
NameInfo	*ni;
int		add_base = 1,i,cangroup=1;

	strcpy(base,name);
	s = base;
	for(i=0;i<depth;i++)
	{
		s = strchr(s,'/');
		if (s && s[1] == '/')
			s = 0;
		if (!s)
			break;
		s++;
	}
	if (s)
		s[-1] = 0;
	else
		cangroup = 0;
	for(i=0;i<scroll->count;i++)
	{
		ni = &scroll->names[i];
		if (stricmp(ni->name,base)==0)
			return;
		if (strnicmp(ni->name,base,strlen(ni->name))==0 && base[strlen(ni->name)] == '/')
		{
			if (ni->group)
			{
				if (!ni->expanded)
					return;
				if (!cangroup)
					add_base = 0;
			}
		}
	}
	ni = &scroll->names[scroll->count++];
	ni->depth = depth;
	if (add_base)
	{
		strcpy(ni->name,base);
		ni->group = 1;
		ni->expanded = 0;
	}
	else
	{
		strcpy(ni->name,name);
		ni->group = 0;
	}
}

static void scrollExpand(ScrollInfo *scroll,int idx)
{
int		i,orig_count,count;

	if (!scroll->names[idx].group || scroll->names[idx].expanded)
		return;
	scroll->names[idx].expanded = 1;
	orig_count = scroll->count;
	for(i=0;i<scroll->list_count;i++)
	{
		scrollAddName(scroll,&scroll->list_names[i * scroll->list_step],scroll->names[idx].depth+1);
	}
	count = scroll->count - orig_count;
	memmove(&scroll->names[idx + 1 + count],&scroll->names[idx + 1],(scroll->count - idx - 1) * sizeof(NameInfo));
	memcpy(&scroll->names[idx + 1],&scroll->names[scroll->count],count * sizeof(NameInfo));
}

static void scrollCompact(ScrollInfo *scroll,int idx)
{
int		i,count,len;
char	*base;

	if (!scroll->names[idx].group || !scroll->names[idx].expanded)
		return;
	scroll->names[idx].expanded = 0;
	base = scroll->names[idx].name;
	len = strlen(base);
	for(i=idx;i<scroll->count;i++)
	{
		if (strnicmp(scroll->names[i].name,base,len)!=0 || (len && scroll->names[i].name[len] && scroll->names[i].name[len] != '/'))
			break;
	}
	count = i - idx - 1;
	memmove(&scroll->names[idx + 1],&scroll->names[idx + 1 + count],(scroll->count - idx - count) * sizeof(NameInfo));
	scroll->count -= count;
}

static int getMaxLines(ScrollInfo *scroll)
{
	int		maxlines;

	maxlines = windowScaleY(scroll->maxlines);
	if (scroll->maxlines < 20)
		maxlines += (maxlines - scroll->maxlines) * 1.15;

	return maxlines;
}

#define PTSZX 8
#define PTSZY 9

static void makeScrollOffsetLegal(ScrollInfo *scroll)
{
	int		maxlines = getMaxLines(scroll);
	if (scroll->offset > (scroll->count - maxlines) * PTSZY)
		scroll->offset = (scroll->count - maxlines) * PTSZY;
	if (scroll->offset < 0)
		scroll->offset = 0;
}

int uiScrollUpdate(ScrollInfo *scroll,int cant_focus)
{
	int		i,j,idx,lost_focus=0,maxlines;
	int		x,xp,y,ypos;
	extern int mouse_dy;
	char	buf[128];

	maxlines = getMaxLines(scroll);
	if (cant_focus || !scroll->got_focus && inpLevel(INP_RBUTTON) && !inpEdge(INP_RBUTTON) )
		lost_focus = 1;
	inpMousePos(&x,&y);
	x/=PTSZX;
	ypos = scroll->ypos;
	if (scroll->grow_upwards)
		ypos -= MIN(scroll->count,maxlines) * PTSZY;
	y = (y + PTSZY/2 - fontLocY(ypos)) / PTSZY;
	if (!inpLevel(INP_RBUTTON))
		scroll->got_focus = 0;
	else if (scroll->got_focus && inpLevel(INP_RBUTTON)){
		scroll->offset += -mouse_dy;
	}
	makeScrollOffsetLegal(scroll);    


	fontDefaults();
	fontSet(0);
	fontScale(PTSZX + scroll->maxcols * PTSZX,PTSZY/2 + PTSZY + MIN(scroll->count,maxlines) * PTSZY);
	fontColor(0x000000);
	fontAlpha(0x80);
	fontText(scroll->xpos-PTSZX,ypos-PTSZY,"\001");
	fontDefaults();

	xp = (x - fontLocX(scroll->xpos)/PTSZX) + 1; //+1 makes better selection 
	for(i=0;i<maxlines;i++)      
	{
		idx = scroll->offset/PTSZY + i;
		if (idx >= scroll->count)
			break;
		if (!lost_focus && i == y && ( 0 <= xp && xp < (int)scroll->maxcols ) )
		{
			scroll->got_focus = 1;
			control_state.ignore = 1;
			fontColor(0xff0000);
			if (edit_state.sel)
			{
				edit_state.sel = 0;
				if (!scroll->names[idx].group)
				{
					if (scroll->callback)
						scroll->callback(scroll->names[idx].name,idx-1);
				}
				else
				{
					//Little hack
					if( 0 == stricmp( scroll->header, "open groups" ) )
						editCmdUnsetActiveLayer();
					//end little hack
					if (!scroll->names[idx].expanded)
						scrollExpand(scroll,idx);
					else
						scrollCompact(scroll,idx);
				}
			}
			if (scroll->rollover)
				scroll->rollover(scroll->names[idx].name,idx-1);
		}
		else
		{
			int		len;

			fontColor(0x00ff00);   
			len = strlen(scroll->names[idx].name)-1; 
			if( len >=0 )     
			{   
				if( len > 12 && 0 == stricmp( &scroll->names[idx].name[len-11], "ACTIVE LAYER" ) )
					fontColor(0xff7000);
				else if( len > 5 && 0 == stricmp( &scroll->names[idx].name[len-4], "LAYER" ) )
					fontColor(0xafafff);
				else if (scroll->names[idx].name[len] == '~')
					fontColor(0x7f7f7f);
				else if (scroll->names[idx].name[len] == '*')
					fontColor(0xffffff);
				else if (scroll->names[idx].group)
					fontColor(0xffff00);
			}
		}
		buf[0] = 0;
		for(j=0;j<scroll->names[idx].depth;j++)
			strcat(buf," ");
		if (scroll->names[idx].group)
		{
			if (scroll->names[idx].expanded)
				strcat(buf,"-");
			else
				strcat(buf,"+");
		}
		else
			strcat(buf," ");
		strcat(buf,simpleName(scroll->names[idx].name));
		if (!idx)
			strcat(buf,scroll->header);
		buf[scroll->maxcols-1] = 0;
		fontTextf(scroll->xpos,ypos + i * PTSZY,buf);
	}
	//cmdOldSetSends(control_cmds,1);
	return scroll->got_focus;
}

void uiScrollInit(ScrollInfo *scroll)
{
	scroll->names = calloc(sizeof(ScrollInfo),1000);
	scroll->count = 0;

	memset(&scroll->names[0],0,sizeof(scroll->names[0]));
	scroll->names[0].group = 1;
	scroll->count = 1;
	scroll->offset = 0;
}

void uiScrollSet(ScrollInfo *scroll)
{
	int		i;

	scroll->count = scroll->list_count + 1;
	for(i=0;i<scroll->list_count;i++)
	{
		strcpy(scroll->names[i+1].name,scroll->list_names + i * scroll->list_step);
	}
}

void uiScrollSetOffset(ScrollInfo *scroll,int line_num)
{
	scroll->offset = line_num * PTSZY - (getMaxLines(scroll) * PTSZY) / 2;
	makeScrollOffsetLegal(scroll);
}
