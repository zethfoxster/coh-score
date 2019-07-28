#ifndef MESSAGESTOREUTIL_H
#define	MESSAGESTOREUTIL_H

typedef struct MessageStore MessageStore;

extern MessageStore* menuMessages; // Global message store for the client and libraries to use

char* vaTranslateEx(int flags, const char *fmt, va_list args );
char* vaTranslate(const char *fmt, va_list args );
char* textStdEx(int flags, const char *fmt, ... );
char* textStd(const char *fmt, ... );

const char* textStdUnformattedConst(const char *pstring);

#endif