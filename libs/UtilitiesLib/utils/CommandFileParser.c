#pragma warning(push)
#pragma warning(disable:4028) // parameter differs from declaration
#pragma warning(disable:4047)

#include "CommandFileParser.h"
#include "ArrayOld.h"
#include "MemoryPool.h"
#include "utils.h"
#include <string.h>
//#include <stdio.h>
//#include <stdlib.h>
#include <assert.h>
#include "SimpleParser.h"
#include "file.h"
#include "strings_opt.h"

/* Struture ParseContext
 *	Holds some state information so that some info can be retained
 *	from one parse interation to the next.
 *
 *	Fields:
 *		buffer - contains the string to be parsed for extractCommand.
 *		bufferCursor - points to beginning of unparsed portion of the string in buffer.
 *		lastCommand - points to the last command (if any) that extractCommand extracted.
 *
 *		commentDepth - current comment block depth.
 *		structureStack - contains pointers to structures that are currently being filled out.
 *		filledStructure - points to the structure that the parser finished filling last
 */

typedef struct ParseContextImp {
	#define buffersize 1024
	char buffer[buffersize];
	char bufferCopy[buffersize];

	// state info for operators create/store structure
	Array structureStack;
	void* filledStructure;

	// state info for operators push/pop commandwords
	Array commandWordStack;
	PCommandWord* initialCommandWord;

	// state info for extractCommand()
	char* bufferCursor;
	char* lastCommand;

	// state info for skipComments()
	int commentDepth;
	
	// general parser state info
	int lineno;
	char* filename;
	FILE* commandFile;

	int resumeParsing;
} ParseContextImp;

/********************************************************************************************
 * Begin Include file handling
 *	Command files have the ability to include other files.  The include statements must occur
 *	before any other file-specific parsing happens.
 *
 *	To do this, the command file parser always starts parsing using the CommandFileContents
 *	command words defined here.  This allows CommandFile_include_handler to be run when
 *	include statements are found.
 *
 *	Once the parser finds something that is not an include statement, it then switches over
 *	to the command words that the user had specified, stored in 
 *	ParseContextImp::initialCommandWords.
 *
 * 
 *	Note that this is also the place to add command words should be common to all command
 *	files.
 */
static int CommandFile_include_handler(ParseContextImp* context, char* param){
	char* filename;
	int result = 0;
	char buf[MAX_PATH];

	beginParse(param);
	result |= getString(&filename);
	endParse();

	// Make sure we got the name of the file to be included.
	if(!result){
		printf("Found bad \"include\" statement, %s:%i, no include filename\n", parseGetFilename(context), parseGetLineno(context));
		return PCH_STOP_PARSING;
	}

	// Make sure the file exists.
	if(!fileLocateRead(filename, buf)){
		char curFilename[512];	// A copy of the name of the file being parsed.  The string will be altered destructively.
		char newFilename[512];	// The name of the file to be parsed by the include statement.  This string will be constructed.
		char* readCursor;		// index into curFilename
		char* writeCursor=newFilename;		// index into newFilename

		newFilename[0] = '\0';

		// Maybe the path is a relative path.
		// Construct a relative path and then try again.
		strcpy(curFilename, context->filename);
		forwardSlashes(curFilename);

		//	If the filename consists of more than just the filename, start constructing the new relative
		//	path to the file now.
		if(getDirectoryName(curFilename)){
			readCursor = curFilename;
			writeCursor = newFilename;


			while(*readCursor != '\0'){
				//	Take care of the ".." operator that may appear in the path
				if(0 == strncmp(readCursor, "../", 3)){
					readCursor += 3;

					// If it is possible to go back up the directory tree, do it.
					if(curFilename[0])
						getDirectoryName(curFilename);
					else{
						printf("Found bad \"include\" statement, %s:%i, bad include filename\n", parseGetFilename(context), parseGetLineno(context));
						return PCH_STOP_PARSING;
					}
				}

				{
					// Grab the next directory name and concatenate it into the constructed filename.
					int spanlen = (int)strcspn(readCursor, "/\\");
					strncpy_s(writeCursor, ARRAY_SIZE_CHECKED(newFilename) - (writeCursor - newFilename), readCursor, spanlen);

					// Update both the read and write cursor.
					//  Be sure not to move the read cursor beyond the null terminating character.  Otherwise,
					//  the loop would start processing the actual filename also.
					readCursor += spanlen;
					if(*readCursor != '\0')
						readCursor++;
					writeCursor += spanlen;
				}
			}

			// Move the read cursor to the filename part of the path.
			readCursor++;

			*writeCursor++ = '/';
		}

		// Either the path consists of only the filename, or the path consists
		// of a relative path and the filename.  In either cases, when we reach
		// this point, the read cursor is still pointing at the filename part of the
		// path.  Add this to our constructed path now.
		strcpy_unsafe(writeCursor, filename); // JE: writeCursor might not even be initialized here!  What's this code doing?
		

		if(!fileLocateRead(newFilename, buf)){
			printf("Found bad \"include\" statement, %s:%i, include file does not exist\n", parseGetFilename(context), parseGetLineno(context));
			return PCH_STOP_PARSING;
		}

		parseCommandFile(newFilename, context->initialCommandWord);
	}

	parseCommandFile(filename, context->initialCommandWord);
	return 1;
}

static int CommandFile_catchall_handler(ParseContextImp* context, char* param){
	// We've found something that is not specified by the CommandFileContexts
	// command words.  This means we've reached some file specific content.
	// Start using the user supplied command words now.
	parseRestoreLine(context);
	parsePushComandWords(context, context->initialCommandWord);
	return 1;
}

static PCommandWord CommandFileContents[] = {
	{"include",			{{EXECUTE_HANDLER, CommandFile_include_handler}}},
	{"Include",			{{EXECUTE_HANDLER, CommandFile_include_handler}}},
	//{"*",				{{EXECUTE_HANDLER, CommandFile_catchall_handler}}},
	{0}
};

/*
 * 
 *
 *
 * End Include file handling
 ********************************************************************************************/

/********************************************************************************************
 * Begin ParseContext related stuff
 *
 *
 *
 */

ParseContext createParseContext(){
	return (ParseContext)calloc(1, sizeof(ParseContextImp));
}

void destroyParseContext(ParseContextImp* context){
	if(context->commandWordStack.storage)
		destroyArrayPartial(&context->commandWordStack);

	if(context->structureStack.storage)
		destroyArrayPartial(&context->structureStack);

	if(context->filename)
		free(context->filename);
	
	if(context->commandFile)
		fclose(context->commandFile);
	
	free(context);
}

int initParseContext(ParseContextImp* context, char* filename, PCommandWord* commandWords){
	assert(context && filename && commandWords);

	// Try to open the specified file.
	context->commandFile = fileOpen(filename, "rt");
	if(!context->commandFile)
		return 0;

	// Initialize required fields in the parse context.
	context->filename = strdup(filename);

	initArray(&context->commandWordStack, 4);
	initArray(&context->structureStack, 4);

	context->initialCommandWord = commandWords;
	arrayPushBack(&context->commandWordStack, commandWords);
	return 1;
}

/* Function setContextBuffer
 *	Given a parse context and a string to be used for parsing, the function
 *	resets some parameters used by extractCommand() so that function can
 *	process the string properly.
 *
 */
/*
static void setContextBuffer(ParseContextImp* context, char* buffer){
	context->buffer = buffer;
	context->bufferCursor = buffer;
	context->lastCommand = NULL;
}
*/
/* Function parseAdvanceLine()
 *	Advances the parseContext to the next line in the file currently being
 *	parsed.
 *
 *	Returns:
 *		1 - Line advanced successfully
 *		0 - Line advance failed.  There are no more lines in the file.
 */
int parseAdvanceLine(ParseContextImp* context){
	// Grab a line from the command file.
	// If there are no more lines from the command file, there is no more
	// commands to execute.
	if(!fgets(context->buffer, buffersize, context->commandFile))
		return 0;

	context->lineno++;
	context->bufferCursor = context->buffer;

	// Make a backup copy of the line, just in case some handler wants to use it.
	strcpy(context->bufferCopy, context->buffer);
	context->lastCommand = NULL;
	return 1;
}


// Returns a copy of the original line without any alterations
char* parseGetLine(ParseContextImp* context){
	return context->bufferCopy;
}

int parseGetLineno(ParseContextImp* context){
	return context->lineno;
}

char* parseGetFilename(ParseContextImp* context){
	return context->filename;
}

void* parseGetCurrentStructure(ParseContextImp* context){
	return arrayGetBack(&context->structureStack);
}

void* parseGetPreviousStructure(ParseContextImp* context){
	// Is it possible to get a "previous" structure?
	if(context->structureStack.size >= 2)
		// If so, return it.
		return context->structureStack.storage[context->structureStack.size - 2];
	else
		// Otherwise, return an invalid value to indicate it is not possible.
		return NULL;
}

void parsePushStructure(ParseContextImp* context, void* structure){
	arrayPushBack(&context->structureStack, structure);
}

void* parsePopStructure(ParseContextImp* context){
	return arrayPopBack(&context->structureStack);
}

void parseRestoreLine(ParseContextImp* context){
	strcpy(context->buffer, context->bufferCopy);
	context->lastCommand = NULL;
	context->bufferCursor = context->buffer;
}

void parsePushComandWords(ParseContextImp* context, PCommandWord* commandWords){
	assert(commandWords);
	arrayPushBack(&context->commandWordStack, commandWords);
}	

void parsePopComandWords(ParseContextImp* context){
	arrayPopBack(&context->commandWordStack);
}

void parseTransferControl(ParseContextImp* context){
	context->resumeParsing = 1;
}
/*
 * 
 *
 *
 * End ParseContext related stuff
 ********************************************************************************************/


/********************************************************************************************
 * Begin Command File Parser functions
 *
 *
 *
 */

/* Function extractCommand
 *	Given a single line in the command file, this function extracts
 *	the next command and the corresponding parameters.
 *
 *	Note that this function does not know anything about keywords
 *	in the command file being parsed.  It simply extracts the
 *	command and param using the command file format.
 *
 *	Params:
 *		context[in] - ParseContext to use to extract the next command and params.
 *		command[out] - The command word found.  Will be NULL if nothing can be extracted.
 *		params[out] - The parameters found.  Will be NULL if nothing can be extracted.
 *
 *	Returns:
 *		0 - Did not get anything useful from context->buffer.  Update required.
 *		1 - Got something useful from the context->buffer.
 *	
 */
static int extractCommand(ParseContextImp* context, char** command, char** params){
	int extractedCommand = 0;  // Was a command extracted during this time?

	*command = NULL;
	*params = NULL;

	// Remove leading whitespaces from bufferCursor
	context->bufferCursor += strspn(context->bufferCursor, " \t\n");

	// Extract command word
	//	This operation is performed differently depending on if we've already extracted
	//	a command word from this line previously.
	if(!context->lastCommand){
		// If no commands have been extracted from this line yet...

		// Extract command
		//	The first token that available in the buffer must be the command word.
		//	Extract it.
		*command = strsep(&context->bufferCursor, " \n\t");

		//	Did we actually get a token?
		if(*command && (*command)[0]){
			// Save the extracted command to be used in case there are more than one
			// param list in this line.
			context->lastCommand = *command;
			extractedCommand = 1;
		}else{
			// Could not extract any tokens at all.
			return 0;
		}
		
		// Advance bufferCursor past the processed command token
		//context->bufferCursor = *command + strlen(*command) + 1;
	}else{
		// If a command has already been extracted from this line...

		// Return previously encountered command
		*command = context->lastCommand;
	}

	// Remove leading whitespaces from bufferCursor
	context->bufferCursor += strspn(context->bufferCursor, " \t\n");

	// Extract parameter
	*params = strsep(&context->bufferCursor, ",\t\n");

	// If we got some parameters this time, update the bufferCursor to point to
	// the beginning of the unparsed portion of the buffer.
	if(*params){
		return 1;
	}

	// If we didn't get any parameters this time, but got some command word.
	if(extractedCommand)
		return 1;
	else
	// Otherwise, we didn't get any parameters, and we didn't get any command word.
		return 0;
	
}

static PCommandWord* findCommandActions(char* command, PCommandWord* commandWords){
	int i;

	// loop through all of the command words
	for(i = 0; commandWords[i].commandWord != NULL; i++){
		// Compare the given command against all known command words
		if(0 == strcmp(command, commandWords[i].commandWord)){
			// return the CommandWord structure if the command words match perfectly
			return &commandWords[i];
		}

		// Is the command word the catch-all command, "*"?
		if(*commandWords[i].commandWord == '\0*')
			return &commandWords[i];
	}

	return NULL;
}

/* Function clearCommentBlocks()
 *	Given a parse context with a full buffer, this function empties the parts
 *	that belongs to a comment block.
 *
 *	This function clears C-style comment blocks.  Unlike C-style comment blocks,
 *	multi-level comments are processed correctly.
 *
 *	This function may be somewhat slow since it needs to look at all the characters
 *	in the given context buffer.  Fortunately, it only needs to examine all characters
 *	once.
 */
static void clearCommentBlocks(ParseContextImp* context){
	enum{
		NoMatch,
		MatchingOpenCommentBlock,	// Got partial match on open comment block token
		MatchingCloseCommentBlock,	// Got partial match on close comment block token
	} state = NoMatch;

	int clearCharacter=0;		// controls whether the current character gets wiped or not.
	char* cursor = context->buffer;
	

	// Examine all characters in the context buffer.
	while(1){
		switch(state){
		default:
		case NoMatch:
		{
			switch(*cursor){
			case '/':
				// Found possible beginning to a comment block token.
				state = MatchingOpenCommentBlock;

				// Leave current character in the input stream because
				// we do not know if it will complete a valid open comment
				// token yet.
				clearCharacter = 0;	
				break;

			case '*':
				// Found possible ending to a comment block token.
				state = MatchingCloseCommentBlock; 

				// Leave current character in the input stream because
				// we do not know if it will complete a valid close comment
				// token yet.
				clearCharacter = 0;
				break;

			default:

				// There is no reason to keep the current character.
				clearCharacter = 1;
				break;
			}
		}
		break;

		case MatchingOpenCommentBlock:
		{
			switch(*cursor){
			case '*':
				// Completed matching open comment block token.
				// Increment comment depth.
				context->commentDepth++;
				
				// Clear out the previous '/' character that was left on the input stream.
				*(cursor - 1) = ' ';
				clearCharacter = 1;

				state = NoMatch;
				break;

			default:

				// There is still a '/' left on the input stream.  If the current character
				// did not complete a open comment token and the previous character was
				// inside a comment block, clear out the previous character now.
				if(context->commentDepth){
					// Clear the '/' character left in the context buffer.
					*(cursor - 1) = ' ';
					clearCharacter = 1;		// The current character is must also be inside a comment.
				}
				state = NoMatch;
				break;
			}
		}
		break;

		case MatchingCloseCommentBlock:
		{
			switch(*cursor){
			
			case '/':
				// Completed matching close comment block token.
				// Decrement comment depth if possible.
				if(context->commentDepth > 0)
					context->commentDepth--;
				else
					printf("Found unmatched comment block closing token on line %i\n", context->lineno);

				// Clear out the previous character
				*(cursor - 1) = ' ';
				*cursor = ' ';
				
				// Return to no match state
				state = NoMatch;
				break;
				
			case '*':
				// Found possible ending to a comment block token.
				state = MatchingCloseCommentBlock;
				*(cursor - 1) = ' ';
				clearCharacter = 1;
				break;

			default:
				if(context->commentDepth){
					// Clear the '*' character left in the context buffer.
					*(cursor - 1) = ' ';
					clearCharacter = 1;
				}
				state = NoMatch;
				break;
				
			}
		}
		break;
		
		}

		// Clear any characters that are inside comment blocks.
		//	Don't clear 
		if(context->commentDepth && 1 == clearCharacter)
			*cursor = ' ';

		cursor++;
		if('\0' == *cursor)
			return;
	}
}

/* Function clearCommentBlocks()
 *	Given a parse context with a full buffer, this function empties the parts
 *	that belongs to a comment line.
 *
 *	It is possible roll this function into the clearCommentBlocks function so
 *	that all comments can be cleared with a single examination of the line.
 */
static void clearCommentLines(ParseContextImp* context){
	char* cursor = context->buffer;
	cursor = strstr(context->buffer, "//");
	if(cursor){
		*cursor = '\0';
		*(cursor+1) = '\0';
	}

	cursor = strchr(context->buffer, '#');
	if(cursor){
		*cursor = '\0';
		*(cursor+1) = '\0';
	}
}

// Erases all comments from ParseContextImp::buffer
static void clearComments(ParseContextImp* context){
	clearCommentBlocks(context);
	clearCommentLines(context);
}


// Add exit error checks... Did not exit some structure completely?
/* Function parseCommandFile()
 *	Parse the given command file using the given command words as the words to use
 *	for parsing initially.  For now, this is the only way to ask the Command File 
 *	parser to do anything.
 *
 *	The function opens the given command file and continously extracts a command
 *	and a parameter from the file.  Note that this extraction process only follows
 *	the basic format of a command file and not the user defined command words.
 *	After a command is extracted, the looks for the actions associated with the
 *	extracted command word.  If the command word is not defined by the user, it 
 *	will be ignored.  If the command word is defined, the parser attempts to
 *	execute all actions associated with the command word.  Unknown actions
 *	will be ignored.  The parser stops when EOF has been reached.
 *	
 *	Parameters:
 *		filename - absolute path to the command file
 *		commandWords - initial command word array to be used for parsing.
 */
int parseCommandFile(char* filename, PCommandWord* commandWords){
	ParseContext context;
	int result = 0;
	if(!filename)
		return 0;
	context = createParseContext();
	if(initParseContext(context, filename, commandWords))
	{
		parseBeginParse(context);
		result = 1;
	}
	destroyParseContext(context);
	return result;
}

void parseBeginParse(ParseContextImp* context){
	char* command = NULL;
	char* params = NULL;
	PCommandWord* currentCommand;
	PCommandAction* currentAction;
	int i;
	int hasFurtherActions;
	
	assert(context);
	
	while(1){
		
		// If we are picking up where some other parser has left off, don't advance
		// to the next line.
		// 
		// Otherwise, try to get the next line in the command file.
		if(context->resumeParsing)
			context->resumeParsing = 0;
		else if(!parseAdvanceLine(context))
			break;
		
		clearComments(context);
		
		// Execute all commands on that line.
		//	Continually extract a command and a list of parameters 
		while(extractCommand(context, &command, &params)){
			// Can't do anything if there are no commands.
			// The only time this happens is when there is an emtpy line.
			if(!command)
				continue;
			
			// Find the corresponding command word structure
			// The command file contents commands can appear anywhere in the file.  Always check for it.
			currentCommand = findCommandActions(command, CommandFileContents);
			if(!currentCommand)
				currentCommand = findCommandActions(command, (PCommandWord*)arrayGetBack(&context->commandWordStack));
			if(!currentCommand){
				printf("Invalid command \"%s\" found %s:%i\n", command, context->filename, context->lineno);
				continue;
			}
			
			// Execute the specified action.
			for(i = 0, hasFurtherActions = 1; hasFurtherActions && i < MAX_ACTION_PER_COMMAND; i++){
				currentAction = &currentCommand->commandAction[i];
				
				switch(currentAction->actionType){
				case NO_ACTION:
					hasFurtherActions = 0;
					break;
					
				case CREATE_STRUCTURE:
					{
						// Attempt to grab a new structure to be filled out.
						void* structure = mpAlloc((MemoryPool)*(currentAction->memoryPool));
						
						// If such a structure cannot be created, 
						if(!structure){
							printf("Error executing the \"%s\" command at line %i in file \"%s\"\n", command, context->lineno, context->filename);
							printf("Could not allocate memory for new structure\n");
						}
						else
							arrayPushBack(&context->structureStack, structure);
						
						break;
					}
					
					
				case CREATE_STRUCTURE_FROM_PARENT:
					{
						MemoryPool pool;
						void* parentStructure = arrayGetBack(&context->structureStack);
						void* childStructure;
						
						pool = *(MemoryPool*)((char*)parentStructure + currentAction->operandOffset);
						childStructure = mpAlloc(pool);
						
						// If such a structure cannot be created, 
						if(!childStructure){
							printf("Error executing the \"%s\" command at line %i in file \"%s\"\n", command, context->lineno, context->filename);
							printf("Could not allocate memory for new structure\n");
						}
						else
							arrayPushBack(&context->structureStack, childStructure);
						
						break;
					}
				case STORE_STRUCTURE:
					{
						Array* targetArray;
						void* structure = arrayPopBack(&context->structureStack);
						
						
						// If some offset is available, assume that this is the offset
						// into the parent structure where an array structure resides.
						if(currentAction->operandOffset){
							// Assume that parent structure is the top of the unfilled structure stack.
							// Apply the specified offset into the parent structure to get the address
							// to the array to push the newly filled structure into.
							targetArray = (Array*)((char*)arrayGetBack(&context->structureStack) + currentAction->operandOffset);
							
							// Push the newly filled structure into the target array.
							arrayPushBack(targetArray, structure);
						}
						break;	
						
					}
					
				case PUSH_COMMANDWORDS:
					arrayPushBack(&context->commandWordStack, currentAction->commandWords);
					break;
					
				case POP_COMMANDWORDS:
					arrayPopBack(&context->commandWordStack);
					break;
					
				case EXECUTE_HANDLER:
					{
						int result;
						
						// Execute the specified command word handler, passing it the
						// extracted parameters.
						result = currentAction->handler(context, params);
						if(result == PCH_STOP_PARSING){
							goto exit;
						}
						break;
					}

				case EXTRACT_INT:
				case EXTRACT_PERCENTAGE:
					{
						int* addr;
						void* structure = arrayGetBack(&context->structureStack);
						
						addr = (int*)((char*)structure + currentAction->operandOffset);
						*addr = atoi((unsigned char*)params);
						break;
					}
					
				case EXTRACT_FLOAT:
					{
						float* addr;
						void* structure = arrayGetBack(&context->structureStack);
						
						addr = (float*)((char*)structure + currentAction->operandOffset);
						*addr = (float)atof(params);
						break;
					}
					
				case EXTRACT_STRING:
					{
						char** addr;
						void* structure = arrayGetBack(&context->structureStack);
						
						addr = (char**)((char*)structure + currentAction->operandOffset);
						if(*addr)
							free(*addr);
						params = (char*)removeLeadingWhiteSpaces(params);
						removeTrailingWhiteSpaces(&params);
						*addr = strdup(params);
						break;
					}
					
				case EXTRACT_BOOLEAN:
					{
						char* addr;
						void* structure = arrayGetBack(&context->structureStack);
						
						addr = (char*)((char*)structure + currentAction->operandOffset);
						
						if(params && (0 == stricmp("true", params) || 0 != atoi((unsigned char*)params)))
							*addr = 1;
						else
							*addr = 0;
						
					}
				case EXTRACT_FLOAT3:
					break;
					
				case EXTRACT_INT3:
					break;
				
				default:
					printf("Error executing the \"%s\" command at line %i in file \"%s\"\n", command, context->lineno, context->filename);
					printf("Unknown command action\n");
				}
			}
		}
	}
exit:;
}

/*
 * 
 *
 *
 * End Command File Parser functions
 ********************************************************************************************/


#pragma warning(pop)
