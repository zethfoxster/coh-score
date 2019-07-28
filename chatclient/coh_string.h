#ifndef _COH_STRING_H
#define _COH_STRING_H

#include <stdarg.h>

const char *escapeString(const char *instring);
const char *unescapeString(const char *instring);
int tokenize_line(char *buf,char *args[],int max_args,char **next_line_ptr);
char *printEscaped(const char* fmt, va_list ap);
void quickHash(const char * original, char * encoded, unsigned int encodeBlock);

#endif
