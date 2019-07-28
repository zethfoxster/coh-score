/***************************************************************************
 *     Copyright (c) 2000-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 *
 * A simple mathematical expression evaluator in C
 *
 * operators supported: Operator               Precedence
 *
 *                        (                     Lowest
 *                        )                     Highest
 *                        +   (addition)        Low
 *                        -   (subtraction)     Low
 *                        *   (multiplication)  Medium
 *                        /   (division)        Medium
 *                        \   (modulus)         High
 *                        ^   (exponentiation)  High
 *                        sin(                  Lowest
 *                        cos(                  Lowest
 *                        atan(                 Lowest
 *                        abs(                  Lowest
 *                        sqrt(                 Lowest
 *                        ln(                   Lowest
 *                        exp(                  Lowest
 *
 * constants supported: pi
 *
 * Original Copyright 1991-93 by Robert B. Stout as part of
 * the MicroFirm Function Library (MFL)
 *
 * Further modifications Copyright (c) 2000-2003 by Cryptic Studios
 *
 * The user is granted a free limited license to use this source file
 * to create royalty-free programs, subject to the terms of the
 * license restrictions specified in the LICENSE.MFL file.
 *
 ***************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "strings_opt.h"

#define EVAL_ERROR       (-1)
#define EVAL_ERROR_RANGE (-2)
#define EVAL_SUCCESS     (0)

#define NUL '\0'
#if 0
struct operator {
	char        token;
	char       *tag;
	size_t      taglen;
	int         precedence;
};

static struct operator verbs[] = {
	{'+',  "+",       1, 2 },
	{'-',  "-",       1, 3 },
	{'*',  "*",       1, 4 },
	{'/',  "/",       1, 5 },
	{'%',  "%",       1, 5 },
	{'^',  "^",       1, 6 },
	{'(',  "(",       1, 0 },
	{')',  ")",       1, 99},
	{'S',  "SIN(",    4, 0 },
	{'C',  "COS(",    4, 0 },
	{'A',  "ABS(",    4, 0 },
	{'L',  "LN(",     3, 0 },
	{'E',  "EXP(",    4, 0 },
	{'t',  "ATAN(",   5, 0 },
	{'s',  "SQRT(",   5, 0 },
	{NUL,  NULL,      0, 0 }
};

static char   op_stack[256];   /* Operator stack       */
static double arg_stack[256];  /* Argument stack       */
static char   token[256];      /* Token buffer         */
static int    op_sptr;         /* op_stack pointer     */
static int    arg_sptr;        /* arg_stack pointer    */
static int    parens;          /* Nesting level        */

static int    state;           /* 0 = Awaiting expression */
                               /* 1 = Awaiting operator   */

const double Pi = 3.14159265358979323846;

static int              do_op(void);
static int              do_paren(void);
static void             push_op(char);
static void             push_arg(double);
static int              pop_arg(double *);
static int              pop_op(int *);
static char            *get_exp(char *);
static struct operator *get_op(char *);
static int              getprec(char);
static int              getTOSprec(void);


char *rmallws(char *str)
{
	char *obuf, *nbuf;

	if (str)
	{
		for (obuf = str, nbuf = str; *obuf; ++obuf)
		{
			if (!isspace((unsigned char)*obuf))
				*nbuf++ = *obuf;
		}
		*nbuf = NUL;
	}
	return str;
}


/************************************************************************/
/*                                                                      */
/*  evaluate()                                                          */
/*                                                                      */
/*  Evaluates an ASCII mathematical expression.                         */
/*                                                                      */
/*  Arguments: 1 - String to evaluate                                   */
/*             2 - Storage to receive double result                     */
/*                                                                      */
/*  Returns: EVAL_SUCCESS if successful                                 */
/*           EVAL_ERROR if syntax error                                 */
/*           EVAL_ERROR_RANGE if range error                            */
/*                                                                      */
/*  Side effects: Removes all whitespace from the string and converts   */
/*                it to U.C.                                            */
/*                                                                      */
/************************************************************************/

int evaluate(char *line, double *val)
{
	double  arg;
	char   *ptr = line, *str, *endptr;
	int     ercode,num;
	struct operator *op;

	strupr(line);
	rmallws(line);
	state = op_sptr = arg_sptr = parens = 0;

	while (*ptr)
	{
		switch (state)
		{
			case 0:
				if (NULL != (str = get_exp(ptr)))
				{
					if (NULL != (op = get_op(str)) &&
						strlen(str) == op->taglen)
					{
						push_op(op->token);
						ptr += op->taglen;
						break;
					}

					if (EVAL_SUCCESS == strcmp(str, "-"))
					{
						push_op(*str);
						++ptr;
						break;
					}

					// Support symbols in Cryptic symbol table
					if((*str >'9' || *str <'0' ) && bind_str( symbols, cnt_symbols, str, &num ))
					{
						push_arg( (double)num );
					}
					else if (EVAL_SUCCESS == strcmp(str, "PI"))
					{
						push_arg(Pi);
					}
					else
					{
						if (0.0 == (arg = strtod(str, &endptr)) && NULL == strchr(str, '0'))
						{
							return EVAL_ERROR;
						}
						push_arg(arg);
					}

					ptr += strlen(str);
				}
				else  return EVAL_ERROR;

				state = 1;
				break;

			case 1:
				if (NULL != (op = get_op(ptr)))
				{
					if (')' == *ptr)
					{
						if (EVAL_SUCCESS > (ercode = do_paren()))
							return ercode;
					}
					else
					{
						while (op_sptr &&
							op->precedence <= getTOSprec())
						{
							do_op();
						}
						push_op(op->token);
						state = 0;
					}

					ptr += op->taglen;
				}
				else  return EVAL_ERROR;

				break;
		}
	}

	while (1 < arg_sptr)
	{
		if (EVAL_SUCCESS > (ercode = do_op()))
			return ercode;
	}

	if (!op_sptr)
		return pop_arg(val);
	else
		return EVAL_ERROR;
}

/*
**  Evaluate stacked arguments and operands
*/

static int do_op(void)
{
	double arg1, arg2;
	int op;

	if (EVAL_ERROR == pop_op(&op))
		return EVAL_ERROR;

	pop_arg(&arg1);
	pop_arg(&arg2);

	switch (op)
	{
		case '+':
			push_arg(arg2 + arg1);
			break;

		case '-':
			push_arg(arg2 - arg1);
			break;

		case '*':
			push_arg(arg2 * arg1);
			break;

		case '/':
			if (0.0 == arg1)
				return EVAL_ERROR_RANGE;
			push_arg(arg2 / arg1);
			break;

		case '%':
			if (0.0 == arg1)
				return EVAL_ERROR_RANGE;
			push_arg(fmod(arg2, arg1));
			break;

		case '^':
			push_arg(pow(arg2, arg1));
			break;

		case 't':
			++arg_sptr;
			push_arg(atan(arg1));
			break;

		case 'S':
			++arg_sptr;
			push_arg(sin(arg1));
			break;

		case 's':
			if (0.0 > arg2)
				return EVAL_ERROR_RANGE;
			++arg_sptr;
			push_arg(sqrt(arg1));
			break;

		case 'C':
			++arg_sptr;
			push_arg(cos(arg1));
			break;

		case 'A':
			++arg_sptr;
			push_arg(fabs(arg1));
			break;

		case 'L':
			if (0.0 < arg1)
			{
				++arg_sptr;
				push_arg(log(arg1));
				break;
			}
			else
				return EVAL_ERROR_RANGE;

		case 'E':
			++arg_sptr;
			push_arg(exp(arg1));
			break;

		case '(':
			arg_sptr += 2;
			break;

		default:
			return EVAL_ERROR;
	}

	if (1 > arg_sptr)
		return EVAL_ERROR;
	else
		return op;
}

/*
**  Evaluate one level
*/

static int do_paren(void)
{
	int op;

	if (1 > parens--)
		return EVAL_ERROR;
	do
	{
		if (EVAL_SUCCESS > (op = do_op()))
			break;
	} while (getprec((char)op));
	return op;
}

/*
**  Stack operations
*/

static void push_op(char op)
{
	if (!getprec(op))
		++parens;
	op_stack[op_sptr++] = op;
}

static void push_arg(double arg)
{
	arg_stack[arg_sptr++] = arg;
}

static int pop_arg(double *arg)
{
	*arg = arg_stack[--arg_sptr];
	if (0 > arg_sptr)
		return EVAL_ERROR;
	else  return EVAL_SUCCESS;
}

static int pop_op(int *op)
{
	if (!op_sptr)
		return EVAL_ERROR;
	*op = op_stack[--op_sptr];
	return EVAL_SUCCESS;
}

/*
**  Get an expression
*/

static char * get_exp(char *str)
{
	unsigned char *ptr = str, *tptr = token;
	struct operator *op;

	if (EVAL_SUCCESS == strncmp(str, "PI", 2))
		return strcpy(token, "PI");

	while (*ptr)
	{
		if (NULL != (op = get_op(ptr)))
		{
			if ('-' == *ptr)
			{
				if (str != ptr && 'E' != ptr[-1])
					break;
				if (str == ptr && !isdigit((unsigned char)ptr[1]) && '.' != ptr[1])
				{
					push_arg(0.0);
					strcpy(token, op->tag);
					return token;
				}
			}

			else if (str == ptr)
			{
				strcpy(token, op->tag);
				return token;
			}

			else break;
		}

		*tptr++ = *ptr++;
	}
	*tptr = NUL;

	return token;
}

/*
**  Get an operator
*/

static struct operator * get_op(char *str)
{
	struct operator *op;

	for (op = verbs; op->token; ++op)
	{
		if (EVAL_SUCCESS == strncmp(str, op->tag, op->taglen))
			return op;
	}
	return NULL;
}

/*
**  Get precedence of a token
*/

static int getprec(char token)
{
	struct operator *op;

	for (op = verbs; op->token; ++op)
	{
		if (token == op->token)
			break;
	}
	if (op->token)
		return op->precedence;
	else  return 0;
}

/*
**  Get precedence of TOS token
*/

static int getTOSprec(void)
{
	if (!op_sptr)
		return 0;
	return getprec(op_stack[op_sptr - 1]);
}



/*
  Basically, EVAL.C converts infix notation to postfix notation. If you're
not familiar with these terms, infix notation is standard human-readable
equations such as you write in a C program. Postfix notation is most familiar
as the "reverse Polish" notation used in Hewlett Packard calculators and in
the Forth language. Internally, all languages work by converting to postfix
evaluation. Only Forth makes it obvious to the programmer.

  EVAL.C performs this conversion by maintaining 2 stacks, an operand stack
and an operator stack. As it scans the input, each time it comes across a
numerical value, it pushes it onto the operand stack. Whenever it comes
across an operator, it pushes it onto the operator stack. Once the input
expression has been scanned, it evaluates it by popping operands off the
operand stack and applying the operators to them.

 For example the simple expression "2+3-7" would push the values 2, 3, and 7
onto the operand stack and the operators "+" and "-" onto the operator stack.
When evaluating the expression, it would first pop 3 and 7 off the operand
stack and then pop the "-" operator off the operator stack and apply it. This
would leave a value of -4 on the stack. Next the value of 2 would be popped
from the operand stack and the remaining operator off of the operator stack.
Applying the "+" operator to the values 2 and -4 would leave the result of
the evaluation, -2, on the stack to be returned.

  The only complication of this in EVAL.C is that instead of raw operators
(which would all have to be 1 character long), I use operator tokens which
allow multicharacter operators and precedence specification. What I push on
the operator stack is still a single character token, but its the operator
token which is defined in the 'verbs' array of valid tokens. Multicharacter
tokens are always assumed to include any leading parentheses. For example, in
the expression "SQRT(37)", the token is "SQRT(".

  Using parentheses forces evaluation to be performed out of the normal
sequence. I use the same sort of mechanism to implement precedence rules.
Unary negation is yet another feature which takes some explicit exception
processing to process. Other than these exceptions, it's pretty
straightforward stuff.
*/

/* End of File */
#endif
