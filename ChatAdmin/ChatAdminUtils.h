#ifndef __CHAT_ADMIN_UTILS_H__
#define __CHAT_ADMIN_UTILS_H__

 
const char *regGetString(const char *key, const char *deflt);
void regPutString(const char *key, const char *value);
void regDeleteString(const char *key);
int regGetInt(const char *key, int deflt);
void regPutInt(const char *key, int value);
void getNameList(const char * category);
void addNameToList(const char * category, char *name);


extern int name_count;
extern char namelist[256][256];

#endif  // __CHAT_ADMIN_UTILS_H__