#ifndef JSONPARSER_H
#define JSONPARSER_H

#include <yajl/yajl_tree.h>

C_DECLARATIONS_BEGIN

bool yajl_get_string(yajl_val parent, const char ** path, const char ** value);
bool yajl_get_int(yajl_val parent, const char ** path, int * value);
bool yajl_get_string_as_int(yajl_val parent, const char ** path, int * value);
yajl_val parse_json(void * data, size_t size);

C_DECLARATIONS_END

#endif
