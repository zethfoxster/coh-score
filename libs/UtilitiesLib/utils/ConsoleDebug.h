/*\
 *
 *	ConsoleDebug.h/c - Copyright 2004, 2005 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	A keyboard handler for console server apps that want to have a 
 *  simple interactive debug menu.
 *
 */

C_DECLARATIONS_BEGIN

// take a look at an example to see how to use this struct
typedef struct ConsoleDebugMenu {
	char	cmd;						// if empty, print only helptext
	char*	helptext;					// if both empty, end list
	void	(*singlekeyfunc)(void);		// function to execute on a single keypress
	void	(*paramfunc)(char*);		// function to execute with a typed parameter
} ConsoleDebugMenu;

void DoConsoleDebugMenu(ConsoleDebugMenu* menu);	// run the menu
void ConsoleDebugPause(void);						// often attached to 'p' in menu

C_DECLARATIONS_END
