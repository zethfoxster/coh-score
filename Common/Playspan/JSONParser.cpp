#include "JSONParser.h"

#include "utils/SuperAssert.h"

#ifdef _DEBUG
	#ifdef _M_AMD64
		#pragma comment(lib,"../3rdparty/yajl/x64/yajl-2.0.3/lib/debug/yajl_s.lib")
	#else
		#pragma comment(lib,"../3rdparty/yajl/win32/yajl-2.0.3/lib/debug/yajl_s.lib")
	#endif	
#else
	#ifdef _M_AMD64
		#pragma comment(lib,"../3rdparty/yajl/x64/yajl-2.0.3/lib/relwithdebinfo/yajl_s.lib")
	#else
		#pragma comment(lib,"../3rdparty/yajl/win32/yajl-2.0.3/lib/relwithdebinfo/yajl_s.lib")
	#endif	
#endif

bool yajl_get_string(yajl_val parent, const char ** path, const char ** value) {
	yajl_val val = yajl_tree_get(parent, path, yajl_t_string);

	if (!devassert(YAJL_IS_STRING(val)))
		return false;

	*value = YAJL_GET_STRING(val);
	return true;
}

bool yajl_get_int(yajl_val parent, const char ** path, int * value) {
	yajl_val val = yajl_tree_get(parent, path, yajl_t_number);

	if (!devassert(YAJL_IS_INTEGER(val)))
		return false;

	*value = YAJL_GET_INTEGER(val);
	return true;
}

bool yajl_get_string_as_int(yajl_val parent, const char ** path, int * value) {
	const char * str_value;
	if (!yajl_get_string(parent, path, &str_value))
		return false;

	char * end = NULL;
	*value = strtol(str_value, &end, 10);
	if (!devassert(end && !*end))
		return false;

	return true;
}

yajl_val parse_json(void * data, size_t size) {
	static char method[64];

	yajl_val tree = yajl_tree_parse(reinterpret_cast<char*>(data), size, NULL, 0);
	if (!devassertmsg(tree, "Could not parse the JSON:\n%s", tree))
		return NULL;

	return tree;
}