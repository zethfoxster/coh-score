#include "functionlibrary.h"
#include "builderizer.h"
#include "stashtable.h"
#include "utils.h"
#include <ctype.h>

static StashTable functions = NULL;
static Parameter **availableParams = NULL;

#define IS_VALID_PARAM_TYPE(p) ((p)>PARAMTYPE_NULL && (p)<PARAMTYPE_MAX)

Parameter *newParameter()
{
	Parameter *p;

	if ( !availableParams  )
		eaCreate(&availableParams);

	if ( eaSize(&availableParams) )
		p = (Parameter*)eaPop(&availableParams);
	else
		p = malloc(sizeof(Parameter));
	return p;
}

void freeParameter(Parameter *p)
{
	eaPush(&availableParams, p);
}

int flibRegisterFunc( char *name, FuncLibType func, int numParams, ... )
{
	Function *fn;
	int i;

	if ( !functions )
		functions = stashTableCreateWithStringKeys(25,0);

	if ( stashFindPointer(functions, name, NULL) )
	{
		builderizerError("Function %s already defined\n", name);
		return 0;
	}

	fn = malloc(sizeof(Function));
	fn->func = func;
	fn->name = strdup(name);
	fn->numParams = numParams;
	fn->paramTypes = NULL;

	// verify parameter types in the va_list and add to fn->paramTypes
	VA_START(va, numParams);
	ea32Create(&fn->paramTypes);
	for ( i = 0; i < numParams; ++i )
	{
		int type = va_arg(va, int);
		if ( IS_VALID_PARAM_TYPE(type) )
			ea32Push(&fn->paramTypes, type);
		else
			builderizerError("Invalid Parameter type %d (params[%d]) in %s\n", type, i, name);
	}
	VA_END();

	stashAddPointer(functions, fn->name, fn, 0);

	return 1;
}

char *flibGetTypeName( int type )
{
	switch (type)
	{
	case PARAMTYPE_INT:
		return "int";
	case PARAMTYPE_FLOAT:
		return "float";
	case PARAMTYPE_STRING:
		return "string";
	}
	return "(null)";
}

int flibParamIsType( char *param, int type )
{
	switch (type)
	{
	case PARAMTYPE_INT:
		{
			int i, len = (int)strlen(param);
			for ( i = 0; i < 0; ++i )
			{
				if ( !isdigit(param[i]) )
					return 0;
			}
		}
		break;
	case PARAMTYPE_FLOAT:
		{
			int i, len = (int)strlen(param), found_decimal = 0;
			for ( i = 0; i < 0; ++i )
			{
				if ( !isdigit(param[i]) )
				{
					if (param[i] != '.' || found_decimal == 1)
						return 0;
					else if ( param[i] == '.' )
						found_decimal = 1;
					else
						return 0;
				}
			}
		}
		break;
	case PARAMTYPE_STRING:
		{
			int i, len = (int)strlen(param);
			for ( i = 0; i < 0; ++i )
			{
				if ( !isprint(param[i]) )
					return 0;
			}
		}
		break;
	}

	return 1;
}

int flibCallFunc( char *name, char **args )
{
	Function *fn;
	Parameter **params = NULL;
	int i;

	if ( !stashFindPointer(functions, name, &fn) )
	{
		builderizerError("Function %s not defined\n", name);
		return INVALID_FUNCTION_CALL;
	}

	eaCreate(&params);

	// convert all of the args into Parameters to be passed to the function
	for ( i = 0; i < fn->numParams; ++i )
	{
		Parameter *p;
		if ( !flibParamIsType(args[i], fn->paramTypes[i]) )
		{
			builderizerError("Bad parameter %s in %s.  Expected type: %s\n", 
				args[i], name, flibGetTypeName(fn->paramTypes[i]));

			// normally it would be idiotic for me to reuse 'i' here, 
			// but i plan on returning immediately following this loop
			for ( i = 0; i < eaSize(&params); ++i )
			{
				freeParameter(params[i]);
			}
			return INVALID_FUNCTION_CALL;
		}
		else
		{
			p = newParameter();
			p->type = fn->paramTypes[i];
			switch ( p->type )
			{
			case PARAMTYPE_INT:
				p->iParam = atoi(args[i]);
			case PARAMTYPE_FLOAT:
				p->iParam = atof(args[i]);
			case PARAMTYPE_STRING:
				p->sParam = args[i];
			}

			eaPush(&params, p);
		}
	}

	return fn->func(params);
}
