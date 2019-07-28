// ============================================================
// META generated file. do not edit!
// Generated: Fri Sep 11 22:56:21 2009
// ============================================================

#include "serverlib_meta.h"

#include "netio.h"

// FIXME: thread-safety

#include "UtilsNew/Array.h"
#include "UtilsNew/lock.h"

#if PROJECT_SERVERLIB
void serverlib_plMerge(const char *typeName);
#endif // PROJECT_SERVERLIB

void serverlib_meta_register(void)
{
	static int registered = 0;
	static LazyLock lock = {0};

	ObjectInfo *object = NULL;
	TypeParameter *parameter = NULL;

	if(registered)
		return;

	lazyLock(&lock);
	if(registered)
	{
		lazyUnlock(&lock);
		return;
	}

#if PROJECT_SERVERLIB
	object = NULL;

	#if PROJECT_SERVERLIB
		meta_registerCall(CALLTYPE_COMMANDLINE, object, "quit");
	#endif
#endif // PROJECT_SERVERLIB

#if PROJECT_SERVERLIB
	object = calloc(1, sizeof(*object));
	object->name = "serverlib_plMerge";
	object->typeinfo.type = TYPE_FUNCTION;
	object->console_reqparams = 1;
	object->location = serverlib_plMerge;
	object->filename = "serverlib_main.c";
	object->fileline = 260;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "typeName";
	parameter->typeinfo.type = TYPE_STRING;

	#if PROJECT_SERVERLIB
		meta_registerCall(CALLTYPE_COMMANDLINE, object, "plMerge");
	#endif
#endif // PROJECT_SERVERLIB

	registered = 1;
	lazyUnlock(&lock);
}
