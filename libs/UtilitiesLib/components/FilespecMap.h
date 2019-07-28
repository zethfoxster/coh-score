
typedef struct FilespecMap FilespecMap;

FilespecMap *filespecMapCreate(void);
void filespecMapDestroy(FilespecMap *handle);

void filespecMapAdd(FilespecMap *handle, const char *spec, void *data);
bool filespecMapGet(FilespecMap *handle, const char *filename, void **result);
bool filespecMapGetExact(FilespecMap *handle, const char *spec, void **result);

void filespecMapAddInt(FilespecMap *handle, const char *spec, int data);
bool filespecMapGetInt(FilespecMap *handle, const char *filename, int *result);
bool filespecMapGetExactInt(FilespecMap *handle, const char *spec, int *result);

int filespecMapGetNumElements(FilespecMap *handle);