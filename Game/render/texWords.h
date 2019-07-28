#ifndef TEXWORDS_H
#define TEXWORDS_H

#include "texEnums.h"

typedef struct TexThreadPackage TexThreadPackage;

typedef struct TexWord TexWord;
typedef struct TexWordParams {
	const char *layoutName; // call allocAddString() when you set this!
	char **parameters; // EArray of strings to replace "TEMPLATE1", "TEMPLATE2", etc
} TexWordParams;

TexWord *texWordFind(const char *texName, int search); // Use search=1 if this is for a dynamic texture that wants to search the Base locale for a layout file as well
BasicTexture *texWordGetBaseImage(TexWord *texWord);
void texWordsLoad(char *localeName);
void texWordsReloadText(char *localName);
void texWordsAllowLoading(bool allow); // Any load calls before this just set a locale override
void texWordsCheckReload(void);
void texWordLoadDeps(TexWord *texWord, TexLoadHow mode, TexUsage use_category, BasicTexture *texBindParent);
void texWordDoFinalComposition(TexWord *texWord, BasicTexture *texBindParent);
int texWordDoThreadedWork(TexThreadPackage *pkg, bool yield);

TexWordParams *createTexWordParams(void);
void destroyTexWordParams(TexWordParams *params);

int texWordGetPixelsPerSecond(void);
int texWordGetTotalPixels(void);
int texWordGetLoadsPending(void);

void initBackgroundTexWordRenderer(void);
void texWordsFlush(void); // Asynchronously cause the background renderer to finish quickly

// void texWordsSetThreadPriority(U32 priority);

// In texWordsEdit.c
void texWordsEditor(int fromEditor);

#endif