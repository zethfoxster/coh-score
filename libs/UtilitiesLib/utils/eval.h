/***************************************************************************
 *     Copyright (c) 2004-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 *
 * This is a fairly simple stack-based postfix evaluator which also supports
 * variables and callbacks to C functions. It can handle strings and numbers
 * on the stack (and in variables). The callbacks have full access to the
 * stack and variables.
 *
 * Before using the evaluator, you need to set up a context to do the
 * evaluation in via eval_Create. The stack, variables, and function
 * registrations all live within this context, which is available for
 * examination after doing an actual evaluation. There is no need to
 * re-create an eval context each time you want to do an evaluation; you can
 * re-use a single one that you create at startup time. You may wish to clear
 * the stack between invocations, but that's up to you.
 *
 * When you're done with the evaluator, you should destroy it with
 * eval_Destroy. This will free any memory and data structures allocated by
 * the system.
 *
 * The evaluator takes a previously tokenized EArray of char *s. Each entry in
 * the EArray should be a single string token.
 *
 * Intrinsic Functions:
 *
 * The following intrinsics are built-in. You may redefine them via
 * eval_RegisterFunc.
 *
 *   &&       LogicalAnd
 *   ||       LogicalOr
 *   !        LogicalNot
 *   >        NumericGreater
 *   <        NumericLess
 *   >=       NumericGreaterEqual
 *   <=       NumericLessEqual
 *   ==       NumericEqual
 *
 *   eq       StringEqual
 *
 *   +        NumericAdd
 *   *        NumericMultiply
 *   /        NumericDivide
 *   %        NumericModulo
 *   -        NumericSubtract
 *
 *   negate   numeric negation (same as "-1.0 *")
 *   rand     Random value [0.0, 1.0]
 *   minmax   Forces a value to be between two values (inclusive)
 *
 *   >var     Store variable
 *   var>     Fetch variable
 *   auto>    Fetch variable (usually overriden if used)
 *   forget   Forget variable
 *
 *   dup      Duplicates the top item on the stack
 *   drop     Removes the top item from the stack
 *
 *   if       Noop. Syntactic sugar for then
 *   then     If the top of the stack is true, then continues evaluation.
 *            If the top of the stack is false, scans ahead looking for
 *            "endif" and then continues evaluation.
 *
 *   label    Makes a label useful for goto and gotoif
 *   :        Makes a label useful for goto and gotoif
 *   goto     Scans the expression from the beginning for the label at the
 *            top of the stack and continues evaluating from the spot.
 *   gotoif   If the value at the top of the stack is true, scans the
 *            expression from the beginning for the label next on the stack
 *            and continues evaluating from the spot.
 *
 * Shortcuts:
 *
 *   @varname Fetches a variable. Equivalent to "varname var>"
 *   $varname Fetches an auto variable. Equivalent to "varname auto>". It
 *            is expected that auto> will be overridden with a function which
 *            understands how to fetch a special variable. For example,
 *            the eval might be for a particular entity. $level might
 *            automatically return the level of the entity. Basically, the
 *            $ means that this is some property of a structure intrinsic
 *            to this context.
 *
 * Special Functions:
 *
 * These functions are implicitly called by the evaluator. You may redefine
 * them to use your own via eval_RegisterFunc.
 *
 *   num>     If a number is expected at the top of the stack, but the current
 *            item is not one, then this function is called. It must not
 *            change the overall number of items on the stack; i.e. if it
 *            pops something off (say a string), it must push something on
 *            (say the int which the string represents).
 *
 *   int>     DEPRECATED: Use a num> which pushes an int.
 *            If an int is expected at the top of the stack, but the current
 *            item is non-numeric, then this function is called. It must not
 *            change the overall number of items on the stack; i.e. if it
 *            pops something off (say a string), it must push something on
 *            (say the int which the string represents).
 *
 *   float>   DEPRECATED: Use a num> which pushes a float.
 *            If a float is expected at the top of the stack, but the current
 *            item is non-numeric, then this function is called. It must not
 *            change the overall number of items on the stack; i.e. if it
 *            pops something off (say a string), it must push something on
 *            (say the int which the string represents).
 *
 *   auto>    Used by the $varname syntax for fetching a special property of a
 *            structure, like the level (the property) of an entity (the
 *            structure).
 *
 * SPECIAL NOTE ON NUMBERS:
 *
 * Numbers are stored as either ints or floats, but are always promoted to
 * floats for the built-in conditional and numeric operations. The results
 * of these operations are pushed on the stack as floats as well. (The 
 * exception are the modulo and logical operations, which promote to ints
 * and pushed on to the stack as ints.)
 *
 * I would not expect this to be an issue due to the numeric ranges we're 
 * dealing with, but there is a chance of precision loss because of this
 * promotion.
 *
 * SPECIAL NOTE ON STRINGS:
 *
 * Strings are not duplicated by the evaluator (except for variable
 * and function names). It is up to the caller to make sure that the strings
 * are valid while the context is being used.
 *
 ***************************************************************************/
#ifndef EVAL_H__
#define EVAL_H__

C_DECLARATIONS_BEGIN

#include "stdtypes.h"

typedef struct StashTableImp *StashTable;
typedef const struct StashTableImp *cStashTable;

typedef struct EvalContext
{
	char *ppchError;
	bool bDebug;
	const char **piVals;		// MK: switched this to 64-bit compatible earray
	int *piTypes;
	StashTable hashInts;
	StashTable hashPointers;
	StashTable hashStrings;
	StashTable hashFloats;
	StashTable hashFuncs;

	const char * const *ppchExpr;
	int iNumTokens;
	int iCurToken;
	bool bDone;
	bool bVerifyStackSize;

	const char *pchFileBlame;
} EvalContext;

typedef void (*EvalFunc)(EvalContext *pcontext);

typedef enum ClientEvalPrediction
{
	kClientEvalPrediction_None,
	kClientEvalPrediction_False,
	kClientEvalPrediction_True,

	kClientEvalPrediction_Count,
} ClientEvalPrediction;
	// When the client has predicted the outcome of the an eval, the server can
	//   examine the prediction and tweak its own eval process accordingly

EvalContext *eval_Create(void);
	// Allocates and initializes an evalation context.

void eval_Destroy(EvalContext *pcontext);
	// Frees a context created with eval_Create

void eval_Init(EvalContext *pcontext);
	// Initializes or clears an eval conetxt. All data stroed in the context
	//   is lost.

bool eval_Validate(EvalContext *pcontext, const char *pchDbgName, const char * const *ppchExpression, char **ppchOut);
	// Validates that the given expression is evaluatable by the context.

bool eval_EvaluateEx(EvalContext *pcontext, const char * const *ppchExpression, int iNumTokens);
bool eval_Evaluate(EvalContext *pcontext, const char * const *ppchExpression);
	// Evaluates the given expression, returning true if the item on the
	//   top of the stack is int> non-zero, false otherwise.

void eval_SetBlame(EvalContext *pcontext, const char *pchFileBlame);
	// allow eval to correctly blame eval errors.
	// NOTE: does not intern string name. assumes it persists.

void eval_ReRegisterFunc(EvalContext *pcontext, const char *pchName, EvalFunc pfn, int iNumIn, int iNumOut);
void eval_RegisterFunc(EvalContext *pcontext, const char *pchName, EvalFunc pfn, int iNumIn, int iNumOut);
	// Adds a function which will be called when the evaluator finds
	//   an operand which matches pchName.

void eval_UnregisterFunc(EvalContext *pcontext, const char *pchName);
	// Removes a function from the evaluator.

void eval_SetDebug(EvalContext *pcontext, bool bDebug);
	// Turns on and off debugging. When on, the entire stack is dumped
	//   after each operand is evaluated.

// Storing and retrieving named variables
//    Strings are not duplicated here. The caller must guarantee that the
//    string stays valid while it's in the table.
void eval_StorePointer(EvalContext *pcontext, const char *pchName, void *pVal);
bool eval_FetchPointer(EvalContext *pcontext, const char *pchName, void **ppVal);
void eval_ForgetPointer(EvalContext *pcontext, const char *pchName);
void eval_StoreString(EvalContext *pcontext, const char *pchName, const char *pchVal);
bool eval_FetchString(EvalContext *pcontext, const char *pchName, const char **pchVal);
void eval_ForgetString(EvalContext *pcontext, const char *pchName);
void eval_StoreInt(EvalContext *pcontext, const char *pchName, int iVal);
bool eval_FetchInt(EvalContext *pcontext, const char *pchName, int *piVal);
void eval_ForgetInt(EvalContext *pcontext, const char *pchName);
void eval_StoreFloat(EvalContext *pcontext, const char *pchName, float fVal);
bool eval_FetchFloat(EvalContext *pcontext, const char *pchName, float *pfVal);
void eval_ForgetFloat(EvalContext *pcontext, const char *pchName);

// Stack manipulation
//    Strings are not duplicated here. The caller must guarantee that the
//    string stays valid which it's in the stack.
const char *eval_StringPop(EvalContext *pcontext);
const char *eval_StringPeek(EvalContext *pcontext);
void eval_StringPush(EvalContext *pcontext, const char *pch);

int eval_IntPop(EvalContext *pcontext);
int eval_IntPeek(EvalContext *pcontext);
void eval_IntPush(EvalContext *pcontext, int i);

bool eval_BoolPop(EvalContext *pcontext);
bool eval_BoolPeek(EvalContext *pcontext);
void eval_BoolPush(EvalContext *pcontext, bool b);

float eval_FloatPop(EvalContext *pcontext);
float eval_FloatPeek(EvalContext *pcontext);
void eval_FloatPush(EvalContext *pcontext, float f);

void eval_Dup(EvalContext *pcontext);
	// Duplicates the top element on the stack.

bool eval_StackIsEmpty(EvalContext *pcontext);
	// Returns true when the stack is empty

void eval_ClearStack(EvalContext *pcontext);
	// Clears the stack on the given context. Variables and registered
	//   functions aren't affected.

void eval_AddErrorMessage(EvalContext *pcontext, char *message);
	// message MUST be an estring!  It will be destroyed later...

void eval_DumpStack(EvalContext *pcontext);
	// Dumps the stack for debugging.

C_DECLARATIONS_END

#endif /* #ifndef EVAL_H__ */

/* End of File */

