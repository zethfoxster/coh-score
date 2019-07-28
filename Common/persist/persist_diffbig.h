#pragma once
#include "persist_internal.h"

int initializer_diffbig(PersistInfo *info);
int loader_diffbig(PersistInfo *info);
int merger_diffbig(PersistInfo *info, int write);
DirtyType adder_diffbig(PersistInfo *info, void *structptr);
DirtyType remover_diffbig(PersistInfo *info, void *structptr);
int flusher_diffbig(PersistInfo *info);
int syncer_diffbig(PersistInfo *info);
