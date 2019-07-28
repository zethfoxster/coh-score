#ifndef NVPARSE_H
#define NVPARSE_H

#define NVPARSE 1

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif



void nvparse(const char * input_string, ...);
char * const * const nvparse_get_errors();
char * const * const nvparse_print_errors(FILE *fp);

#ifdef __cplusplus
}
#endif

#endif
