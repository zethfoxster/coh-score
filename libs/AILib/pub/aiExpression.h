#ifndef AIEXPRESSION_H
#define AIEXPRESSION_H

#include "stdtypes.h"

typedef enum
{
	// fundamental types that operations will work on
	MULTI_INT,
	MULTI_FLOAT,
	MULTI_STRING,
	MULTI_IDENTIFIER,
	NUM_MULTI_BASIC_TYPES,

	// operators produced by tokenizer
	MULTIOP_ADD,
	MULTIOP_SUBTRACT,
	MULTIOP_NEGATE,
	MULTIOP_MULTIPLY,
	MULTIOP_DIVIDE,
	MULTIOP_EXPONENT,
	MULTIOP_FIELDACCESS,
	MULTIOP_PAREN_OPEN,
	MULTIOP_PAREN_CLOSE,
	MULTIOP_EQUALITY,
	MULTIOP_LESSTHAN,
	MULTIOP_LESSTHANEQUALS,
	MULTIOP_GREATERTHAN,
	MULTIOP_GREATERTHANEQUALS,

	MULTIOP_WORDOP_START,
	MULTIOP_AND = MULTIOP_WORDOP_START,
	MULTIOP_OR,
	MULTIOP_NOT,
	MULTIOP_WORDOP_END,
} MultiValType;


typedef struct MultiVal
{
	union {
		const void* ptr;
		S64 intval;
		F64 floatval;
	};
	MultiValType type;
} MultiVal;

typedef MultiVal** MultiValArray;

void aiExprTokenize(char* str, MultiValArray* result);
void MultiValArrayDestroy(MultiValArray* array);

#endif