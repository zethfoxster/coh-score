#pragma once

typedef struct PersistInfo PersistInfo;

// FIXME: move these two into persist.c, especially since persist.c uses related functionality for panicking
int persist_loadTextDb(PersistInfo *info, void(*cleanup)(PersistInfo*));
int persist_flushTextDb(PersistInfo *info, void(*cleanup)(PersistInfo*));

int loader_flat(PersistInfo *info);
int flusher_flat(PersistInfo *info);
