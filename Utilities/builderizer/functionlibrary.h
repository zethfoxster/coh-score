
typedef enum
{
	PARAMTYPE_NULL,
	PARAMTYPE_INT,
	PARAMTYPE_FLOAT,
	PARAMTYPE_STRING,
	//
	PARAMTYPE_MAX
};

typedef struct
{
	int type;
	union
	{
		int	iParam;
		float fParam;
		char *sParam;
	};
} Parameter;

// all functions registered to the function library must be in this format.
// the functions will be passed an earray of Parameter pointers that function should 
// cast to the expected types (by checking the type variable and using the appropriate
// type in the union).
typedef int (*FuncLibType) (Parameter **);

typedef struct
{
	char *name;
	int numParams;
	int *paramTypes;
	FuncLibType func;
} Function;

// returned by flibCallFunc when a call is made to an invalid functions
#define INVALID_FUNCTION_CALL -1

// For the variable argument list, past a list of parameter types 
// in the order they should be passed to the function.
// This list should be the same length as numParams.
int flibRegisterFunc( char *name, FuncLibType func, int numParams, ... );
int flibCallFunc( char *name, char **args );