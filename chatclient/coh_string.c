#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

const char *escapeString(const char *instring) {
	static char *ret=NULL;
	static int retlen=0;
	const char *c;
	char *out;
	int deslen;
	
	if(!instring)
		return NULL;
	deslen = strlen(instring)*2+1;
	if (retlen < deslen) { // We need a bigger buffer to return the data
		if (ret)
			free(ret);
		if (deslen<256) deslen = 256; // Allocate at least 256 bytes the first time
		ret = malloc(deslen);
		retlen = deslen;
	}
	for (c=instring, out=ret; ; c++) {
		switch (*c) {
			case '\\':
				*out++='\\';
				*out++='\\';
				break;
			case '\n':
				*out++='\\';
				*out++='n';
				break;
			case '\r':
				*out++='\\';
				*out++='r';
				break;
			case '\t':
				*out++='\\';
				*out++='t';
				break;
			case '\"':
				*out++='\\';
				*out++='q';
				break;
			case '\'':
				*out++='\\';
				*out++='s';
				break;
			case '%':
				*out++='\\';
				*out++='p';
				break;
			default:
					*out++=*c;
		}
		if (!*c) break;
	}
	return ret;
}

const char *unescapeString(const char *instring)
{
	static char *ret=NULL;
	static int retlen=0;
	const char *c;
	char *out;
	int deslen;

	if(!instring)
		return NULL;
	deslen = strlen(instring)+1;
	if (retlen < deslen) { // We need a bigger buffer to return the data
		if (ret)
			free(ret);
		if (deslen<256) deslen = 256; // Allocate at least 256 bytes the first time
		ret = malloc(deslen);
		retlen = deslen;
	}
	for (c=instring, out=ret; ; c++) {
		switch (*c) {
			case '\\':
				c++;
				switch(*c){
					case 'n':
						*out++='\n';
						break;
					case 'r':
						*out++='\r';
						break;
					case 't':
						*out++='\t';
						break;
					case 'q':
						*out++='\"';
						break;
					case 's':
						*out++='\'';
						break;
					case '\\':
						*out++='\\';
						break;
					case 'p':
						*out++='%';
						break;
					case '\0':
						// End of string in an escape!  Probably bad, maybe truncation happened, add nothing
						*out++=*c;
						break;
					default:
						// just copy verbatim
						*out++='\\';
						*out++=*c;
				}
				break;
			default:
				*out++=*c;
		}
		if (!*c) break;
	}
	return ret;
}

int tokenize_line(char *buf,char *args[],int max_args,char **next_line_ptr)
{
	char	*s,*next_line;
	int		i,idx;

	s = buf;
	for(i=0;;)
	{
		if (i < max_args)
			args[i] = 0;
		if (*s == ' ' || *s == '\t')
			s++;
		else if (*s == 0)
		{
			next_line = 0;
			break;
		}
		else if (*s == '"')
		{
			if (i < max_args)
				args[i] = s+1;
			i++;
			s = strchr(s+1,'"');
			if (!s) // bad input string
			{
				if (next_line_ptr)
					*next_line_ptr = 0;
				return 0;
			}
			*s++ = 0;
		}
		else
		{
			if (*s != '\r' && *s != '\n')
			{
				if (i < max_args)
					args[i] = s;
				i++;
			}
			idx = (int)strcspn(s,"\n \t");
			s += idx;
			if (*s == ' ' || *s == '\t')
				*s++ = 0;
			else
			{
				if (*s == 0)
					next_line = 0;
				else
				{
					*s = 0;
					if (s[-1] == '\r')
						s[-1] = 0;
					next_line = s+1;
				}
				if (i < max_args)
					args[i] = 0;
				break;
			}
		}
	}
	if (next_line_ptr)
		*next_line_ptr = next_line;
	return i;
}

char *printEscaped(const char* fmt, va_list ap)
{
	static char	curr_msg[10000];
	const char	*sc;
	char		*tail = curr_msg;

	for(sc=fmt;*sc;sc++)
	{
		if (sc[0] == '%' && sc[1] != '%' && sc[1])
		{
			switch(sc[1])
			{
				case 's':
				{
					char *s = va_arg(ap,char *);
					*tail++ = '"';
					strcpy(tail,escapeString(s));
					tail += strlen(tail);
					*tail++ = '"';
				}
				break;case 'S':
				{
					char *s = va_arg(ap,char *);
					strcpy(tail,escapeString(s));
					tail += strlen(tail);
				}
				break;case 'd':
				{
					int val = va_arg(ap,int);
					itoa(val,tail,10);
					tail += strlen(tail);
				}
			}
			sc++;
		}
		else
			*tail++ = *sc;
	}
	*tail = 0;
	return curr_msg;
}

void quickHash(const char * original, char * encoded, unsigned int encodeBlock)
{
    char passKey[16],pad=0;
    unsigned int len = strlen(original);
    unsigned int i;

    strcpy(encoded, original);
    for(i = len; i < encodeBlock; ++i)
    {
        encoded[i] = '\0';
    }

    *((int *)passKey) = *((int *)encoded)*213119 + 2529077;
    *(((int *)passKey) + 1) = *(((int *)encoded) + 1)*213247 + 2529089;
    *(((int *)passKey) + 2) = *(((int *)encoded) + 2)*213203 + 2529589;
    *(((int *)passKey) + 3) = *(((int *)encoded) + 3)*213821 + 2529997;

    // the actual encryption
    for(i = 0; i < encodeBlock; ++i)
    {
        encoded[i] = encoded[i] ^ pad ^ passKey[i & 15];
        pad = encoded[i];
        if(encoded[i] == 0)
        {
            encoded[i] = 0x66;
        }
    }
}
