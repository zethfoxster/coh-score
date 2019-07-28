#ifndef _CONTAINERDUMP_H
#define _CONTAINERDUMP_H

void lineToRefLine(StuffBuff *sb,StructDesc* structDesc, char* table, char* field, char* val, int idx);
int dumpContainers(char *fname, int* ids);
int processDump(char *fnamein, char *fnameout);

#endif