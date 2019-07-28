#ifndef TEXUNLOAD_H
#define TEXUNLOAD_H

void texDynamicUnload(int enable);
int texDynamicUnloadEnabled(void);
void texUnloadTexturesToFitMemory(void);

#endif