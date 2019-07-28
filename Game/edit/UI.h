/* File UI_H
 *	This file is intended to be used to construct UI for the editor.
 *
 *	Each element has two event handler maps, class and instance.  The class handler map 
 *	describes how elements of a particular class handles certain events.  These functions 
 *	dictate how the elements interact with one another.  This map will inherit/override 
 *	parent class handlers.  The instance handler map, as the name implies, is specific to 
 *	a particular element instance.  It can have all the same types of handlers that is 
 *	present in the class handler map.  However, these handlers are intended to perform
 *	functions that specializes an element to a particular task.  For example,
 *	two textbox elements can behave in the selection highlight rules using the class handler 
 *	map.  However, when the textboxes are clicked, they can behave differently using the
 *	instance handler.
 *
 *
 */

#ifndef UI_H
#define UI_H


//typedef long UIElement;
 //// Event Handler signatures
 //typedef void (*onClickHandler)(UIElement element);
 //
 //typedef struct{
 //	unsigned short x;
 //	unsigned short y;
 //	unsigned short width;
 //	unsigned short height;
 //} UIRect;
 //
 //typedef struct{
 //	unsigned int handlerID;
 //	char* handlerName;
 //} HandlerDescriptor;
 //
 ///**************************************************************************
 // * Generic User Interface Element
 // *	The entire user interface is made up of User Interface Elements.  They
 // *	can form complex composites to be drawn on the screen on request.
 // */
 //void UIEDestroy(UIElement element);
 //
 //// Element properties
 //UIRect UIEGetRect(UIElement element);
 //void UIESetRect(UIElement element, UIRect rect);
 //
 //// Element display
 //void UIEDraw(UIElement element);// Request the element to draw itself.
 //
 //// Composite management
 //int UIEAddElement(UIElement parent, UIElement child);
 //int UIERemoveElement(UIElement parent, UIElement child);
 //int UIEClearElementList(UIElement element);
 //
 //// User data
 //void* UIESetUserData(UIElement element, void* data);
 //void* UIEGetUserData(UIElement element);
 //
 //int UIESetInstanceHandler(UIElement element, HandlerDescriptor* descriptor, void* func);
 //void* UIEGetInstanceHandler(UIElement element, HandlerDescriptor* descriptor);
 //
 //void* UIEGetClassHandler(UIElement element, HandlerDescriptor* descriptor);
 //void* UIEGetSuperHandler(UIElement element, HandlerDescriptor* descriptor);
 ///*
 // * Generic User Interface Element
 // **************************************************************************/
 //
 ///**************************************************************************
 // * ScrollBox
 // *	A scroll box is an element that displays its elements in a linear in a
 // *	linear list that runs up and down.
 // */
 //typedef long UIScrollBox;
 //
 //UIScrollBox createUIScrollBox();
 //void destroyUIScrollBox(UIScrollBox scrollbox);
 //
 //// Scrolling
 //int UISBScroll(int scrollDistance);
 ///*
 // * ScrollBox
 // **************************************************************************/
 //
 ///**************************************************************************
 // * TextBox
 // *	Holds text!
 // */
 //typedef long UITextBox;
 //
 //UITextBox createUITextBox();
 //void destroyUITextBox();
 //
 //void UITBSetRect(UITextBox textbox, UIRect rect);
 //char* UITBGetText(UITextBox textbox);
 //int UITBSetText(UITextBox textbox, char* text);
 ///*
 // * TextBox
 // **************************************************************************/
 
#include "stdtypes.h"

typedef struct
{
	U8	depth;
	U8	group;
	U8	expanded;
	char	name[128];
} NameInfo2;

typedef int cb(char *,int);

typedef struct ScrollInfo2 ScrollInfo2;

struct ScrollInfo2{
	NameInfo2	*names;
	int			count;
	char		*list_names;
	int			list_step;
	int			list_count;
	int			xpos,ypos;
	int			maxlines;
	int			maxcols;
	char		header[128];
	int			offset;
	int			got_focus;
	int			grow_upwards;
	cb			*callback;

	// The linked scroll scrolls, expands, compacts along with this one.
	ScrollInfo2* linkedScroll;

	// Don't respond to anything other than to invoke the callback function.
	unsigned int			callbackOnly	: 1;

	// Always show the header, no matter where the window has scrolled to.
	unsigned int			retainHeader	: 1;
};

void uiScroll2AddNameSimple(ScrollInfo2* scroll, char* name);
void uiScroll2AddName(ScrollInfo2* scroll, char* name, int depth, int cangroup);
int uiScroll2Update(ScrollInfo2* scroll, int cant_focus);
void uiScroll2Init(ScrollInfo2* scroll);
void uiScroll2Set(ScrollInfo2* scroll);

#endif