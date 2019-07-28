// JE: I'm using this file in CrashRpt.dll as well (via SourceSafe branch)
#include "xlate.h"
#include "wininclude.h"
#include "AppLocale.h"
#include "stdtypes.h"
#include "locale.h"
#include "ConvertUtf.h"
#include <mbctype.h>
#include "utils.h"
#include "stringutil.h"

MessageStore	*g_msg_store = NULL;

#if PATCHCLIENT|CRASHRPT
#include "resource.h"

static LCID current_locale_id = LOCALE_ENGLISH;
int xlateGetLoadedLocale()
{
	return current_locale_id;
}

char *xlateLoad(void *hInstance)
{
	int		i,idx=-1,ret;
	HRSRC	rsrc;
	HGLOBAL	handle;
	char	*text,*types,*str;
	char	buf[1000];
	LCID    langid = GetUserDefaultLCID();
	DWORD   primarylang = langid & 0x03ff;

	struct { LCID lang; int resource_id; char *name; char *webname; } langs[] = 
	{
		{ LOCALE_ENGLISH, IDR_ENGLISH, "English", "English" },
		{ LOCALE_TEST, IDR_1337, "English", "English" },
		{ LOCALE_CHINESE_TRADITIONAL, IDR_CHINESE_TRADITIONAL, "Chinese (Traditional)", "ChineseTraditional" },
		{ LOCALE_CHINESE_SIMPLIFIED, IDR_CHINESE_SIMPLIFIED, "Chinese (Simplified)", "ChineseSimplified" },
		{ LOCALE_CHINESE_HONGKONG, IDR_CHINESE_HONGKONG, "Chinese (Hong Kong)", "ChineseHongKong" },
		{ LOCALE_KOREAN, IDR_KOREAN, "Korean", "Korean" },
		{ LOCALE_JAPANESE, IDR_JAPANESE, "Japanese", "Japanese" },
		{ LOCALE_GERMAN, IDR_GERMAN, "German", "German" },
		{ LOCALE_FRENCH, IDR_FRENCH, "French", "French" },
		{ LOCALE_SPANISH, IDR_SPANISH, "Spanish", "Spanish" },
	};

	for(i=0;i<ARRAY_SIZE(langs);i++)
	{
		if (langs[i].lang == langid)
			idx = i;
	}

	if (idx < 0)
	{
		if (primarylang == LANG_GERMAN)
			langid = LOCALE_GERMAN;
		else if (primarylang == LANG_FRENCH)
			langid = LOCALE_FRENCH;
		else if (primarylang == LANG_CHINESE)
			langid = LOCALE_CHINESE_SIMPLIFIED;
		else if (primarylang == LANG_JAPANESE)
			langid = LOCALE_JAPANESE;
		else if (primarylang == LANG_KOREAN)
			langid = LOCALE_KOREAN;
		else if (primarylang == LANG_SPANISH)
			langid = LOCALE_SPANISH;
		else
			langid = LOCALE_ENGLISH;

		idx = 0;

		for(i=0;i<ARRAY_SIZE(langs);i++)
		{
			if (langs[i].lang == langid)
				idx = i;
		}
	}

	if (!locIsImplemented(locGetIDByWindowsLocale(langs[idx].lang)))
	{
		idx = 0;
		langid = langs[idx].lang;
	}

	str = setlocale ( LC_ALL, langs[idx].name);
	_setmbcp(_MB_CP_LOCALE);
	ret = _getmbcp();

	rsrc = FindResource(hInstance,MAKEINTRESOURCE(langs[idx].resource_id),RT_RCDATA);
	handle = LoadResource(hInstance,rsrc);
	text = LockResource(handle);
	rsrc = FindResource(hInstance,MAKEINTRESOURCE(IDR_LANGTYPES),RT_RCDATA);
	handle = LoadResource(hInstance,rsrc);
	types = LockResource(handle);

	LoadMessageStoreFromMem(&g_msg_store,text,types);
	msPrintf(g_msg_store,SAFESTR(buf),"eSInvalidCreditsCard");
	printf("");

	current_locale_id = langid;

	return langs[idx].webname;
}
#endif

int xlatePrint(char *buffer, int bufferLength, const char *msg)
{
	int		ret;
	char	*s;
	wchar_t *wcBuf;

	ret = msPrintf(g_msg_store,buffer,bufferLength,msg);
	wcBuf = UTF8ToWideStrConvertStatic(buffer);
	if (strcmp(buffer,msg)==0)
	{
		char	xlateword[1024];

		strcpy(xlateword,msg);
		if (s=strrchr(xlateword,' '))
		{
			*s = 0;
			ret = msPrintf(g_msg_store,buffer,bufferLength,xlateword);
			*s = ' ';
			strcat_s(buffer,bufferLength,s);
		}
	}
	//utf8ToUtf16(buffer);
	utf8ToMbcs(buffer,bufferLength);
	return ret;
}

char *xlateQuick(char *msg)
{
	static	char	buf[1000];

	buf[0] = 0;
	msPrintf(g_msg_store,SAFESTR(buf),msg);
	
	if (strcmp(buf,msg)==0)
	{
		char	xlateword[1024];
		char	*s;

		strcpy(xlateword,msg);
		if (s=strrchr(xlateword,' '))
		{
			*s = 0;
			msPrintf(g_msg_store,SAFESTR(buf),xlateword);
			*s = ' ';
			strcat_s(SAFESTR(buf),s);
		}
	}

	//utf8ToMbcs(SAFESTR(buf));
	{
		static wchar_t wcbuf[1000];
		UTF8ToWideStrConvert(buf, SAFESTR(wcbuf));
		wcstombs(buf, wcbuf, ARRAY_SIZE_CHECKED(buf) );
	}
	return buf;
}

char *xlateQuickUTF8(char *msg)
{
	static	char	buf[1000];

	buf[0] = 0;
	msPrintf(g_msg_store,SAFESTR(buf),msg);
	return buf;
}

int xlatePrintf(char* outputBuffer, int bufferLength, const char* messageID, ...)
{
	int result;

	VA_START(arg, messageID);
	result = msvaPrintfInternal(g_msg_store, outputBuffer, bufferLength, messageID, NULL, NULL, arg);
	VA_END();

	//utf8ToMbcs(outputBuffer,100000);
	{
		static wchar_t buf[2000];
		UTF8ToWideStrConvert(outputBuffer, SAFESTR(buf));
		wcstombs(outputBuffer, buf, bufferLength);
	}
	return result;
}

wchar_t *xlateToUnicode(char *mbcs)
{
	static wchar_t buf[2000];
	mbstowcs(buf, mbcs, 2000);
	return buf;
}

wchar_t *xlateToUnicodeBuffer(const char *mbcs, wchar_t *buf, size_t buflen)
{
	mbstowcs(buf, mbcs, buflen);
	//buf[_mbstrlen(mbcs)] = 0;
	//ConvertUTF8toUTF16(&mbcs, &mbcs[buflen], &buf, &buf[buflen], 0);
	//wcsncpy( buf, UTF8ToWideStrConvertStatic(mbcs), buflen );
	return buf;
}



