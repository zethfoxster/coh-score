/* File CommandFileParser
 *	Provides a fast way to create a parser for files compatible with the command file format.
 *	A command file conists
 *	
 *	What does a command file look like?
 *		A "command file" is a file that consists of a series of commands, one after another, 
 *		arranged in some hierarchical fashion.  A command consists of a single command word, 
 *		and then a series of parameters, seperated by white spaces.
 *
 *		Each line can contain only one command.  However, it is possible to invoke the same 
 *		command multiple times by using a comma to separate separate sets of parameters for 
 *		the command.
 *
 *		Commands can be used to open or close a block.
 *
 *		Single line comments can start with either "//" or "#".  Multi-line comments are enclosed
 *		by C-style comments.  Multi-level comments are allowed.
 *
 *	What is this parser used for?
 *		Currently, this parser is mostly intended to extract and load fields into structures of 
 *		unknown size and layout.  The main purpose was to provide a way to read in structures
 *		that may have complex connections.  However, the format can also be used to execute 
 *		simple scripts.  It is a "Command File" after all.
 *
 *	How does this parser work?
 *		While parsing, the parser really has no idea what keywords your specific command file
 *		contains.  It simply follows the command file's format to figure out which tokens
 *		are the commands and which tokens are the parameters.
 *		
 *		After it finds a complete command, a command word + a set of parameters, it then looks 
 *		up the command word in the list of user defined command words.  There can be multiple 
 *		actions (operations) associated with a command word.
 *	
 *	What is an action?
 *		An action is just an operator + operand pair.  The operator is an integer while the
 *		operand is anything that can fit within a 4 byte space.  Mostly, it is either an
 *		integer or a pointer.
 *
 *		Obviously, each operator requires a different type of operand.  Please see documentation
 *		associated with each action for the exact operand type that is expected.  Since there
 *		is no easy way to perform compile-time or run-time type checking, the parser just assumes
 *		that an operand of the correct type has been specified.  The resulting behavior is undefined
 *		if an operand of the incorrect type was supplied.  (Bad thing *will* happen.)
 *
 *	What actions can parser perform?
 *		All the actions that the parser knows how to perform are enumerated in PCommandActionType.
 *		The most cruicial ones are the first 6 actions listed.  
 *
 *	  -	Doing nothing...
 *		This operator is only here so that uninitialized actions will be skipped.
 *
 *		NO_ACTION:
 *			Expects no parameters.
 *
 *
 *	  -	Automatic structure creation + connection
 *		These following two operators gives the users the ability to connect up structures in a strict
 *		hierarchical fashion.  Memory Pools and Arrays are used so that the parser does not need to 
 *		know the details of how to allocate and store the structures.  The result is a simplified 
 *		parser that is capable of creating dynamic connections between structures.
 *
 *		CREATE_STRUCTURE:
 *			Expects the address of a properly initialized Memory Pool.  
 *
 *			When the parser comes across this action, it grabs a piece of memory and pushes it
 *			into a stack.  All subsequent actions will be applied to this new structure.
 *
 *		STORE_STRUCTURE:
 *			Expects the address of an Array.  If no address is supplied, the structure is not stored.
 *			This might be useful if the structure is already connected.
 *
 *			The parser pops the most recently created structure and pushes it into the supplied array.  
 *			The array will be initialized to a small size if it is not initialized already.
 *
 *		Note that there may be a significant memory overhead if there is a large number of Arrays since 
 *		the storage space is dynamically allocated.  There are some memory accounting overheads associated 
 *		with each piece of dynamically allocated memory.  If the structure's connections will remain static 
 *		for the duration of the program, a possible fix is to have the Array's allocate their storage space 
 *		from a StringTable.
 *
 *
 *	  -	Swapping command words
 *		The following two operators gives the user the ability to temporarily change the command words used
 *		to parse the command file.  This is often needed when processing a new block within a command file.
 *
 *		PUSH_COMMANDWORDS:
 *			Expects the address of an array of PCommandWord.
 *
 *		POP_COMMANDWORDS:
 *			Expects no parameters.  Supplied parameter will be ignored if present.
 *
 *		The command words are simply pushed and popped from a command word stack.  Only the topmost command
 *		words are used for command execution at any time.
 *		
 *
 *	  - Performing non-trivial/non-generic tasks
 *		Generic functionalities (such as primitive extraction) should be added to the list of predefined
 *		parser actions.  Complex commands that are usually specific to some command files should have
 *		their behaviors defined via this command handling mechanism so as not to overload the generic
 *		version of the parser.
 *
 *		EXECUTE_HANDLER:
 *			Expects the address of a ParseCommandHandler.
 *
 *			The parser executes the supplied handler.  See ParseCommandHandler documentation for details.
 *		
 *		
 *	How do I use this stupid thing anyway?
 *		Create several command word arrays to completely define the format of your command file.  Call
 *		parseCommandFile with a filename and the initial command file.  The file with the given filename
 *		will be magically parsed.
 *
 *		Note that the parser is currently functional, but it is not done.  The most notable thing is that 
 *		the parser does not understand comments at all.  bla bla bla.  This thing will be revised later.
 *
 *	What new features can be expected?
 *		- comment handling.
 *		- able to optionally "compile" command words into StashTable form for faster lookups.
 *		- able to optionally make use of StringTable to conserve memory during string extraction.
 *		- add interface to ParseContext let the user avoid setup costs by holding onto the ParseContexts.
 *		- better error handling.
 *
 *	Where can I find an example of how to use this thing?
 *		Although having a command file parser simplifies parsing files, it still takes a bit of work to
 *		set everything up properly.  Therefore, there is currently no simple "standalone" example.  For 
 *		a somewhat complex example of command file parser usage, see the SpawnArea file parser section 
 *		in entgen.h/.c
 */

#ifndef COMMANDFILEPARSER_H
#define COMMANDFILEPARSER_H


#define MAX_ACTION_PER_COMMAND 4

#include "stdtypes.h"	

typedef enum{
	NO_ACTION = 0,

	CREATE_STRUCTURE,
	CREATE_STRUCTURE_FROM_PARENT,
	STORE_STRUCTURE,

	PUSH_COMMANDWORDS,
	POP_COMMANDWORDS,

	// Generic handler
	EXECUTE_HANDLER,

	// Primitive extraction
	EXTRACT_INT,
	EXTRACT_FLOAT,
	EXTRACT_STRING,
	EXTRACT_BOOLEAN,

	// Extended primitive extraction
	EXTRACT_FLOAT3,
	EXTRACT_INT3,
	EXTRACT_PERCENTAGE,
} PCommandActionType;


typedef struct ParseContextImp *ParseContext;

/* Typedef ParseCommandHandler
 *	Parse command handlers are used by the parser to process commands to
 *	implement special behavior for that are too complex for the generic
 *	parser to handle.
 *
 *	When configured to do so, the parser will invoke the command handler
 *	associated with particular command words.
 *
 *	Parameters:
 *		ParseContext - current parser context
 *		params - the param string that follows the command word
 *	
 *	Returns:
 *		0 - continue parsing as usual
 *		~0 - abandon all fruther processing because some unrecoverable 
 *			 condition has been encountered.  Value is user defined error code.
 *
 */
typedef int (*ParseCommandHandler)(ParseContext context, char* params);

// Handler return values
#define PCH_STOP_PARSING 0xffffffff

typedef struct MemoryPoolImp *MemoryPool;

/* Structure PCommandAction
 *	Defines an action for the command file parser to execute.
 *
 */
typedef struct{
	PCommandActionType actionType;
	union{
		ParseCommandHandler handler;
		intptr_t operandOffset;
		MemoryPool *memoryPool;
		struct PCommandWord* commandWords;
	};
} PCommandAction;


typedef struct PCommandWord {
	char* commandWord;
	PCommandAction commandAction[MAX_ACTION_PER_COMMAND];
} PCommandWord;

int parseCommandFile(char* filename, PCommandWord* commandWords);




/********************************************************************
 * Begin Parse Context information stuff
 */

ParseContext createParseContext();
int initParseContext(ParseContext context, char* filename, PCommandWord* commandWords);
void destroyParseContext(ParseContext context);

int parseGetLineno(ParseContext context);
char* parseGetFilename(ParseContext context);
void* parseGetCurrentStructure(ParseContext context);
void* parseGetPreviousStructure(ParseContext context);
void parsePushStructure(ParseContext context, void* structure);
void* parsePopStructure(ParseContext context);
void parsePushComandWords(ParseContext context, PCommandWord* commandWords);
void parsePopComandWords(ParseContext context);
char* parseGetLine(ParseContext context);
void parseBeginParse(ParseContext context);
void parseRestoreLine(ParseContext context);
void parseTransferControl(ParseContext context);
/*
 * Begin Parse Context information stuff
 ********************************************************************/


#endif