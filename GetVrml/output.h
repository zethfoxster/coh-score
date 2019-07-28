#ifndef _OUTPUT_H
#define _OUTPUT_H

#include "tree.h"

// Generated by mkproto
void outputPackAllNodes(char *name, Node **list, int count);
void outputGroupInfo(char *fname);
void outputResetVars(void);
void outputData(char *fname);
int foundDuplicateNames(char *fname);
char *stripGeoName(char *s,char *buf);
// End mkproto

#endif
