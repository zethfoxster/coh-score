#ifdef _WIN32
# include <windows.h>
#else
#include <stdarg.h>
#define strnicmp strncasecmp
#endif

#include <stdio.h>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/glu.h>

#include <string>
#include <vector>

#include "nvparse_errors.h"
#include "nvparse.h"


//void yyinit(char*);
//int yyparse(void);


// RC1.0  -- register combiners 1.0 configuration
bool rc10_init(char *);
int  rc10_parse();
bool is_rc10(const char *);

// TS1.0  -- texture shader 1.0 configuration
bool ts10_init(char *);
int  ts10_parse();
bool is_ts10(const char *);

// VP1.0  -- vertex program
bool vp10_init(char *);
int  vp10_parse();
bool is_vp10(const char *);

// VSP1.0  -- vertex state program
bool vsp10_init(char *);
int  vsp10_parse(int vspid);
bool is_vsp10(const char *);

// VCP1.0  -- vertex constant program
bool vcp10_init(char *);
int  vcp10_parse();
bool is_vcp10(const char *);

// DX8 stuff

// PS1.0  -- DX8 Pixel Shader 1.0 configuration
bool ps10_init(char *);
int  ps10_parse();
bool ps10_set_map(const std::vector<int>& argv);
bool is_ps10(const char *);
const int* ps10_get_info(int* pcount);

// VS1.0  -- DX8 Vertex Shader 1.0
bool vs10_init(char *);
int  vs10_parse();
bool is_vs10(const char *);
void vs10_load_program();

nvparse_errors errors;
int line_number;
char * myin = 0;

void nvparse(const char * input_string, int argc /* = 0 */,...)
{
    if (NULL == input_string)
    {
        errors.set("NULL string passed to nvparse");
        return;
    }
    
    char * instring = strdup(input_string);

    // vertex constant program
	if(is_vcp10(instring))
	{
		if(vcp10_init(instring))
		{
			vcp10_parse();
		}
	}

	// vertex state program
	else if(is_vsp10(instring))
	{
		if(vsp10_init(instring))
		{
			vsp10_parse(argc);
		}
	}

	// vertex program
    else if(is_vp10(instring))
	{
		if(vp10_init(instring))
		{
			vp10_parse();
		}
	}

	// register combiners (1 and 2)
	else if(is_rc10(instring))
	{
		if(rc10_init(instring))
		{
			rc10_parse();
		}
	}

	// texture shader
	else if(is_ts10(instring))
	{
		if(ts10_init(instring))
		{
			ts10_parse();
		}
	}

	// DX8 vertex shader
	else if ( is_vs10(instring) )
	{
		if(vs10_init(instring))
		{
			vs10_parse();
	        vs10_load_program();
		}

	}
	else if (is_ps10(instring))
	{
		if(ps10_init(instring))
		{
			va_list ap;
			std::vector<int> argv;
			va_start(ap,argc);
			for (int i=0;i<argc;++i)
			{
				int arg = va_arg(ap,int);
				argv.push_back(arg);
			}
			va_end(ap);
			if (!ps10_set_map(argv))
				return;
			ps10_parse();
		}
	}
    else
    {
        errors.set("invalid string.\n \
                    first characters must be: !!VP1.0 or !!VSP1.0 or !!RC1.0 or !!TS1.0\n \
                    or it must be a valid DirectX 8.0 Vertex Shader");
    }
    free(instring);
}

char * const * const nvparse_get_errors()
{
    return errors.get_errors();
}

char * const * const nvparse_print_errors(FILE * errfp)
{
    for (char * const *  ep = nvparse_get_errors(); *ep; ep++)
    {
        const char * errstr = *ep;
        fprintf(errfp, "%s\n", errstr);
    }
    return nvparse_get_errors();
}

const int* nvparse_get_info(const char* input_string, int* pcount)
{
	if (NULL == input_string)
    {
        errors.set("NULL string passed to nvparse_get_info");
        return 0;
    }
	if (is_ps10(input_string))
	{
		return ps10_get_info(pcount);
	}
	return 0;
}
