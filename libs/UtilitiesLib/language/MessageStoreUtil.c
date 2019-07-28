#include "MessageStoreUtil.h"
#include "MessageStore.h"
#include "EString.h"
#include "mathutil.h"

MessageStore*			menuMessages;


//------------------------------------------------------------
//  Translate the passed string and store it in a static buffer.
// NOTE: this function contains a ring of static buffers so that multiple
// calls will result in valid results, however the returned string is
// still owned by this function and may change.
//----------------------------------------------------------
char* vaTranslateEx(int flags, const char *fmt, va_list args)
{
	static char* bufs[15] = {0};
	static U32 bufRotor = 0;	
	U32 bufCur = bufRotor++%ARRAY_SIZE( bufs );
	int requiredBufferSize;

	if (!fmt)
		return NULL;

	requiredBufferSize = MAX(msvaPrintfEx(flags, menuMessages, NULL, 0, fmt, args), (int)strlen(fmt));
	estrSetLength( &bufs[bufCur], requiredBufferSize + 10 ); // making these bigger than nessecary to keep off by one cases from crashing

	// not sure why this copy is here...
	estrPrintCharString( &bufs[bufCur], fmt );

	// print to the string
	msvaPrintfEx(flags, menuMessages, bufs[bufCur], requiredBufferSize + 10, fmt, args);

	return bufs[bufCur];
}

char* vaTranslate(const char *fmt, va_list args)
{
	return vaTranslateEx(0, fmt, args);
}

char* textStdEx(int flags, const char *fmt, ...)
{
	va_list va;
	char  *buffer;

	va_start(va, fmt);
	buffer = vaTranslateEx(flags, fmt, va);
	va_end(va);

	return buffer;
}

// I'd like textStd to just call textStdEX, but I have to process the varargs stuff here.  So this will have to be a duplicate.
// It should, however, be the only one.
char* textStd(const char *fmt, ...)
{
	va_list va;
	char  *buffer;

	va_start(va, fmt);
	buffer = vaTranslateEx(0, fmt, va);
	va_end(va);

	return buffer;
}

const char *textStdUnformattedConst(const char *pstring)
{
	return pstring ? msGetUnformattedMessageConst(menuMessages,pstring) : NULL;
}
