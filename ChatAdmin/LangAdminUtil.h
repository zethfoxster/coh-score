#ifndef __LANG_ADMIN_UTIL_H__
#define __LANG_ADMIN_UTIL_H__

void conTransf( int type, char *str, ... );
char* localizedPrintf( char *str, ... );

void reloadAdminMessageStores(int localeID);

#endif //__LANG_ADMIN_UTIL_H__