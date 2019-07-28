#include "UI.h"
#include "ArrayOld.h"
#include "HashTableStack.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>




//#define createHandler(handlerAlias, handlerID, handlerName, returnType, functionSignature) \
//	HandlerDescriptor handlerAlias = {handlerID, #handlerName}; \
//	typedef returnType (*Func##handlerAlias) functionSignature;
//
//typedef struct UIElementImp UIElementImp;
//
//createHandler(OnCreate,		1, ON_CREATE_HANDLER,	void, (UIElementImp* element));
//createHandler(OnDestroy,	2, ON_DESTROY_HANDLER,	void, (UIElementImp* element));
//createHandler(OnDraw,		3, ON_DRAW_HANDLER,		void, (UIElementImp* element));
//createHandler(OnSetRect,	4, ON_SET_RECT_HANDLER,	void, (UIElementImp* element, UIRect rect));
//createHandler(OnClick,		5, ON_CLICK_HANDLER,	void, (UIElementImp* element));
//
//
//
///**************************************************************************
// * Generic User Interface Element
// */
// 
//struct UIElementImp{
//	HashTableStack classVFtable;
//	StashTable instaceVFTable;
//
//	// Element location
//	UIRect rect;
//	
//	// Composite info
//	// Contains a list of child UIElementImp*
//	Array* children;
//
//	// User data
//	void* userData;
//
//	// Element display function.
//	FuncOnDraw onDraw;	// cached to speed drawing.  same value as classVFTable->OnDraw
//};
//
//void UIEDestroy(UIElementImp* element){
//	FuncOnDestroy instanceOnDestroy;
//	FuncOnDestroy classOnDestroy;
//	FuncOnDestroy superClassOnDestroy;
//	int i;
//
//	for(i = 0; i < element->children->size; i++){
//		UIEDestroy((UIElement)element->children->storage[i]);
//	}
//	instanceOnDestroy = (FuncOnDestroy)UIEGetInstanceHandler(element, &OnDestroy);
//	classOnDestroy = (FuncOnDestroy)UIEGetClassHandler(element, &OnDestroy);
//	superClassOnDestroy = (FuncOnDestroy)UIEGetSuperHandler(element, &OnDestroy);
//
//	if(instanceOnDestroy)
//		instanceOnDestroy(element);
//
//	if(superClassOnDestroy)
//		superClassOnDestroy(element);
//
//	if(classOnDestroy)
//		classOnDestroy(element);
//}
//
//UIRect UIEGetRect(UIElementImp* element){
//	return element->rect;
//}
//
///* UIESetRectBasic()
// *	For elements that do not have any sort of custom set rectangle function, this
// *	function is called to set the rectangle directly.
// *
// */
//void UIESetRectBasic(UIElementImp* element, UIRect rect){
//	element->rect = rect;
//}
//
///* UIESetRect()
// *	This function dispatches the a SetRect call to either the element's custom
// *	handler, or the default handler, 
// *
// */
//void UIESetRect(UIElementImp* element, UIRect rect){
//	FuncOnSetRect onSetRect = (FuncOnSetRect) hashStackFindValue(element->classVFtable, OnSetRect.handlerName);
//	assert(onSetRect);
//	onSetRect(element, rect);
//}
//
//void UIEDraw(UIElementImp* element){
//	// Ask the element to draw itself first.
//	//	Make sure the drawing handler is present.
//	if(!element->onDraw)
//		element->onDraw = hashStackFindValue(element->classVFtable, OnDraw.handlerName);
//
//	//	Call the drawing handler to have the element draw itself.
//	if(element->onDraw)
//		element->onDraw(element);
//
//	// Ask all the element's children to draw themselves.
//	if(element->children){
//		int i;
//		for(i = 0; i < element->children->size; i++){
//			UIEDraw((UIElement)element->children->storage[i]);
//		}
//	}
//}
//
//int UIEAddElement(UIElementImp* parent, UIElement child){
//	int addChildToList = 1;
//
//	// If the child is already being held by the parent, ignore the request.
//	if(parent->children){
//		if(-1 == arrayFindElement(parent->children, (void*)child))
//			return 0;
//	}
//	
//	// Otherwise, add the child to the parent's list of children.
//	pushBack(parent->children, (void*)child);
//	return 1;
//}
//
//int UIERemoveElement(UIElementImp* parent, UIElement child){
//	int childIndex;
//	childIndex = arrayFindElement(parent->children, (void*)child);
//
//	// The given child cannot be removed if it is not really a child
//	// of the given parent.
//	if(-1 == childIndex)
//		return 0;
//
//	// This is not the ideal behavior since it disturbs the existing order
//	// of the children.
//	arrayRemoveAndFill(parent->children, childIndex);
//	return 1;
//}
//
//int UIEClearElementList(UIElementImp* element){
//	if(element->children)
//		clearArray(element->children);
//	return 1;
//}
//
//void* UIESetUserData(UIElementImp* element, void* data){
//	void* oldData = element->userData;
//	element->userData = data;
//	return oldData;
//}
//
//void* UIEGetUserData(UIElementImp* element){
//	return element->userData;
//}
//
///* Function UIESetInstanceHandler()
// *	
// *
// *
// *
// */
//int UIESetInstanceHandler(UIElementImp* element, HandlerDescriptor* descriptor, void* func){
//	// Does element have a instance specific handler table?
//	// If it does, retrieve it and put in the new handler.
//	if(!element->instaceVFTable){
//		element->instaceVFTable = createHashTable();
//		element->instaceVFTable = stashTableCreateWithStringKeys(STASHSIZEPLEASE,  StashDeepCopyKeys | StashCaseSensitive );
//		initHashTable(element->instaceVFTable, 4);
//	}
//
//	{
//		//NOTE: It might be a good idea to introduce new modes the hash table to deal with
//		//situations where an existing entry already exists.  This section of code is merely
//		//try to make sure that new inserts are retained in the hash table instead of the existing
//		//behavior where old inserts are retained.
//		StashElement StashElement = hashFindElement(element->instaceVFTable, descriptor->handlerName);
//
//		// If the handler already exists,
//		if(StashElement){
//			// Replace the existing handler.
//			stashElementSetPointer(StashElement, func);
//			return 1;
//		}else{
//			// The handler doesn't exist yet.  Put in the handler.
//			return stashAddPointer(element->instaceVFTable, descriptor->handlerName, func, false);
//		}
//	}
//	
//}
//
//void* UIEGetInstanceHandler(UIElementImp* element, HandlerDescriptor* descriptor){
//	return stashFindPointerReturnPointer(element->instaceVFTable, descriptor->handlerName);
//}
//
//
//void* UIEGetClassHandler(UIElementImp* element, HandlerDescriptor* descriptor){
//	void* func;
//
//	func = stashFindPointerReturnPointer(element->classVFtable, descriptor->handlerName);
//	return func;
//}
//
///* Function UIEGetSuperHandler()
// *	Attempts to retrieve the handler of the parent class.
// *
// *	Try not using this function for now.  The meaning of "super" is still in flux.
// *
// *
// */
//void* UIEGetSuperHandler(UIElementImp* element, HandlerDescriptor* descriptor){
//	void* func;
//	StashTable table;
//
//	table = hashStackPopTable(element->classVFtable);
//	func = stashFindPointerReturnPointer(element->classVFtable, descriptor->handlerName);
//	hashStackPushTable(element->classVFtable, table);
//	return func;
//}
//
///*************************************************
// * Class Specific Handler map construction
// */
//static HashTableStack UIEVTable;
//static HashTableStack UIEGetVTable(){
//	StashTable fTable;
//
//	if(UIEVTable)
//		return UIEVTable;
//
//	
//	fTable = createHashTable();
//	initHashTable(fTable, 8);
//	fTable = stashTableCreateWithStringKeys(STASHSIZEPLEASE,  StashDeepCopyKeys | StashCaseSensitive );
//
//	stashAddPointer(fTable, OnSetRect.handlerName, UIESetRectBasic, false);
//
//	hashStackPushTable(UIEVTable, fTable);
//	return UIEVTable;
//}
///*
// *
// *************************************************/
//
///*
// * Generic User Interface Element
// **************************************************************************/
//
//
//
//
//
///**************************************************************************
// * ScrollBox
// *	A scroll box is an element that displays its elements in a linear in a
// *	linear list that runs up and down.
// */
//typedef struct{
//	UIElementImp element;
//
//	// Current view rectangle
//	UIRect viewRect;
//
//	unsigned short maxWidth;
//	unsigned short maxHeight;
//} UIScrollBoxImp;
//
//UIScrollBox createUIScrollBox(){
//
//}
//
//void destroyUIScrollBox(UIScrollBox scrollbox){
//
//}
//
//// Scrolling
//int UISBScroll(int scrollDistance){
//	
//}
///*
// * ScrollBox
// **************************************************************************/
//
///**************************************************************************
// * TextBox
// *	Holds static text.
// */
//typedef struct {
//	UIElementImp element;
//	int textLength;
//	char* textBuffer;
//}UITextBoxImp;
//
//void destroyUITextBox(UITextBoxImp* textbox){
//	if(textbox->textLength)
//		free(textbox->textBuffer);
//	free(textbox);
//}
//
//
//UITextBox createUITextBox(){
//	UITextBoxImp* textbox = calloc(1, sizeof(UITextBoxImp));
////	textbox->element.onDestroy = destroyUITextBox;
//}
//
//void UITBSetRect(UITextBoxImp* textbox, UIRect rect){
//	rect.height = 8;
//	rect.width = strlen(textbox->textBuffer) * 8;
//	textbox->element.rect = rect;
//}
//
//char* UITBGetText(UITextBoxImp* textbox){
//	return textbox->textBuffer;
//}
//
//int UITBSetText(UITextBoxImp* textbox, char* text){
//	int newTextLength = strlen(text);
//	if(newTextLength > textbox->textLength){
//		textbox->textBuffer = realloc(textbox->textBuffer, newTextLength);
//		assert(textbox->textBuffer);	// Yep... no propery error handling...
//	}
//
//	textbox->textLength = newTextLength;
//	strcpy(textbox->textBuffer, text);
//	return 1;
//}
//
//
///*************************************************
// * Class Specific Handler map construction
// */
//static HashTableStack UITBVTable;
//static HashTableStack UITBGetVTable(){
//	StashTable fTable;
//
//	if(UIEVTable)
//		return UIEVTable;
//
//	
//	fTable = createHashTable();
//	initHashTable(fTable, 8);
//	fTable = stashTableCreateWithStringKeys(STASHSIZEPLEASE,  StashDeepCopyKeys | StashCaseSensitive );
//
//	stashAddPointer(fTable, OnSetRect.handlerName, UITBSetRect, false);
//	
//
//	hashStackPushTable(UIEVTable, fTable);
//	return UIEVTable;
//}
///*
// *
// *************************************************/
///*
// * TextBox
// **************************************************************************/


#include "font.h"
#include "input.h"
#include "edit_cmd.h"
#include "cmdcommon.h"
#include "mathutil.h"
#include "edit_uiscroll.h"
#include "win_init.h"

static char* uiScroll2SimpleName(char* name)
{
	/*
	char	*s;
	
	s = strrchr(name,'/');
	if (!s)
		s = name;
	else
		s++;
	return s;
	*/
	return name;
}

void uiScroll2AddNameSimple(ScrollInfo2* scroll, char* name){
	NameInfo2* ni;
	ni = &scroll->names[scroll->count++];

	ni->depth = 0;
	ni->expanded = 0;
	ni->group = 0;
	strcpy(ni->name, name);
}

void uiScroll2AddName(ScrollInfo2* scroll, char* name, int depth, int cangroup)
{
	char base[128],*s;
	NameInfo2* ni;
	int	add_base = 1,i;
	
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
	if (s && s != base)
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
		ni->group = cangroup;
		ni->expanded = 0;
	}
	else
	{
		strcpy(ni->name,name);
		ni->group = 0;
	}
}

static void uiScroll2Expand(ScrollInfo2* scroll, int idx)
{
	int		i,orig_count,count;
	
	if (!scroll->names[idx].group || scroll->names[idx].expanded)
		return;
	scroll->names[idx].expanded = 1;
	orig_count = scroll->count;
	for(i=0;i<scroll->list_count;i++)
	{
		uiScroll2AddName(scroll, &scroll->list_names[i * scroll->list_step], scroll->names[idx].depth+1, 1);
	}
	count = scroll->count - orig_count;
	memmove(&scroll->names[idx + 1 + count],&scroll->names[idx + 1],(scroll->count - idx - 1) * sizeof(NameInfo));
	memcpy(&scroll->names[idx + 1],&scroll->names[scroll->count],count * sizeof(NameInfo));
}

static void uiScroll2Compact(ScrollInfo2* scroll, int idx)
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

int uiScroll2Update(ScrollInfo2* scroll, int cant_focus)
{
	int		i,j,idx,lost_focus=0,maxlines;
	int		x,xp,y,ypos;
	extern int mouse_dy;
	char	buf[128];
	U32		widthOfDarkenedBox;
	U32		nameCount;
#define PTSZX 8
#define PTSZY 9
	
	maxlines = windowScaleY(scroll->maxlines); 
	if (scroll->maxlines < 20)
		maxlines += (maxlines - scroll->maxlines) * 1.15;
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
		if(scroll->linkedScroll)
			scroll->linkedScroll->offset += -mouse_dy;
	}
	if (scroll->offset > (scroll->count - maxlines) * PTSZY)
		scroll->offset = (scroll->count - maxlines) * PTSZY;
	if (scroll->offset < 0)
		scroll->offset = 0;
	
	//Resize the text box to fix the longest string in it
	widthOfDarkenedBox = scroll->maxcols;
	nameCount = 0;							//raises the box so no properties fall off the bottom
	for(i=0;i<maxlines;i++)     
	{
		idx = scroll->offset/PTSZY + i;
		if (idx >= scroll->count)
			break;
		if( scroll->names[idx].name )
		{
			nameCount++;
			widthOfDarkenedBox = MINMAX( widthOfDarkenedBox, strlen( scroll->names[idx].name )+1, 128-1 ); //128 = buf size
		}
	}
	//End resize


	fontDefaults(); 
	fontSet(0);
	fontScale(PTSZX + widthOfDarkenedBox * PTSZX,PTSZY/2 + PTSZY + MIN(scroll->count,maxlines) * PTSZY);
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
		if (!lost_focus && i == y && ( 0 <= xp && xp < (int)widthOfDarkenedBox ) )
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
					// Don't expand or compat if only callbacks are supposed to be working.
					// This effectively turns the scroll box into a flat list.
					if(!scroll->callbackOnly){						
						if(!scroll->names[idx].expanded){
							uiScroll2Expand(scroll,idx);
							if(scroll->linkedScroll)
								uiScroll2Expand(scroll->linkedScroll,idx);
						}else{
							uiScroll2Compact(scroll,idx);
							if(scroll->linkedScroll)
								uiScroll2Compact(scroll->linkedScroll,idx);
						}
					}
				}
			}
		}
		else
		{
			int		len;
			
			len = strlen(scroll->names[idx].name)-1;
			if (scroll->names[idx].name[len] == '~')
				fontColor(0x7f7f7f);
			else if (scroll->names[idx].name[len] == '*')
				fontColor(0xffffff);
			else if (scroll->names[idx].group)
				fontColor(0xffff00);
			else
				fontColor(0x00ff00);
		}
		buf[0] = 0; 
		for(j=0;j<scroll->names[idx].depth;j++)
			strcat(buf," ");

		//Removed this from properties and values, didn't do anything
		if(0 && !scroll->callbackOnly){
			if (scroll->names[idx].group)
			{
				if (scroll->names[idx].expanded)
					strcat(buf,"-");
				else
					strcat(buf,"+");
			}
			else
				strcat(buf," ");	// indent got disabled also... doesn't really matter though...
		}
		//ENd removed

		// Extract the string after the last "/"
		strcat(buf,uiScroll2SimpleName(scroll->names[idx].name));

		// Index 0 gets the header string.
		if (!idx) 
			strcat(buf,scroll->header);
		
		buf[widthOfDarkenedBox] = 0;
		fontTextf(scroll->xpos,ypos + i * PTSZY,buf);
	}
	//cmdOldSetSends(control_cmds,1);
	return scroll->got_focus;
}

void uiScroll2Init(ScrollInfo2* scroll)
{
	scroll->names = calloc(sizeof(ScrollInfo),1000);
	scroll->count = 0;
	
	memset(&scroll->names[0],0,sizeof(scroll->names[0]));
	scroll->names[0].group = 1;
	scroll->count = 1;
	scroll->offset = 0;
}

void uiScroll2Set(ScrollInfo2* scroll)
{
	int		i;

	scroll->count = scroll->list_count + 1;
	for(i=0;i<scroll->list_count;i++)
	{
		strcpy(scroll->names[i+1].name,scroll->list_names + i * scroll->list_step);
	}
}
