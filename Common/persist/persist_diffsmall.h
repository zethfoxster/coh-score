#pragma once
#include "persist_internal.h"

int initializer_diffsmall(PersistInfo *info);
int loader_diffsmall(PersistInfo *info);
int merger_diffsmall(PersistInfo *info, int write);
DirtyType dirtier_diffsmall(PersistInfo *info, void *structptr);
DirtyType journaler_diffsmall(PersistInfo *info, void *lineptr);
DirtyType remover_diffsmall(PersistInfo *info, void *structptr);
int ticker_diffsmall(PersistInfo *info);
int flusher_diffsmall(PersistInfo *info);
