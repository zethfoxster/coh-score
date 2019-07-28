/*\
 *
 *	structTokenizer.h - Copyright 2004 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	Provides a single-pass, destructive tokenizer for textparser.  The tokenizer
 *  is independent and can be used independently, although it is heavily
 *  optimized for speed over convenience.
 *
 */

#ifndef STRUCTTOKENIZER_H
#define STRUCTTOKENIZER_H

C_DECLARATIONS_BEGIN

// limitations on tokens
#define MAX_TOKEN_LENGTH 300	// need to allow space for a full filename after #include
#define MAX_STRING_LENGTH (MAX_TOKEN_LENGTH * 40) // 12k seems like enough, but this can certainly be changed
#define TOKEN_BUFFER_LENGTH (MAX_STRING_LENGTH * 2)

// basic tokenizer struct.  You can create the tokenizer from a file or in-memory string.
typedef void* TokenizerHandle;
typedef struct StaticDefine StaticDefine;
typedef struct DefineContext DefineContext;

TokenizerHandle TokenizerCreateEx(const char* filename, int ignore_empty);	// opens up file and prepares for tokenizing
static INLINEDBG TokenizerHandle TokenizerCreate(const char* filename) { return TokenizerCreateEx(filename, 0); }
TokenizerHandle TokenizerCreateString(const char* string, int length);// sets up parsing for a string, length can be -1 to use strlen() instead
void TokenizerDestroy(TokenizerHandle tokenizer);	// closes all file handles and memory
char* TokenizerGetFileName(TokenizerHandle tokenizer); // returns file name of current context
int TokenizerGetCurLine(TokenizerHandle tokenizer);
char* TokenizerGetFileAndLine(TokenizerHandle tokenizer);
void TokenizerSetGlobalDefines(TokenizerHandle tokenizer, DefineContext* context);

int IsEol(const char* token); // ! only way to distinguish between an actual eol returned by the Tokenizer and a string containing only \n
int TokenizerEscape(char* target, const char* source);
int TokenizerUnescape(char* target, const char* source);

// the key peek and get functions
const char* TokenizerPeek(TokenizerHandle tokenizer, int ignorelinebreak, int ignorecomma);
	// returns next token or NULL at end of file, VOLATILE RESULT: you must copy result between calls
const char* TokenizerGet(TokenizerHandle tokenizer, int ignorelinebreak, int ignorecomma);
	// as PeekToken but eats token, VOLATILE RESULT: you must copy result between calls

// The Ex functions let you break down tokens further by seperating along any characters in the <seps> parameter.
// Return values are NOT broken with NULL's like Peek/Get.  You must use the length return value to determine correct
// length of string.  They are compatible with Peek/Get and only return incrementally smaller tokens.
const char* TokenizerPeekEx(TokenizerHandle tokenizer, int ignorelinebreak, int ignorecomma, char* seps, int* length);
const char* TokenizerGetEx(TokenizerHandle tokenizer, int ignorelinebreak, int ignorecomma, char* seps, int* length);

void TokenizerSetGlobalDefines(TokenizerHandle tokenizer, DefineContext* context);
DefineContext* TokenizerGetGlobalDefines(TokenizerHandle tokenizer);
void TokenizerSetLocalDefines(TokenizerHandle tokenizer, DefineContext* context);
DefineContext* TokenizerGetLocalDefines(TokenizerHandle tokenizer);
void TokenizerSetStaticDefines(TokenizerHandle tokenizer, StaticDefine* list);

C_DECLARATIONS_END

#endif // STRUCTTOKENIZER_H