
typedef struct CharacterTransfer {
	char *rootPath;
	char *getMapserver;
	char *putMapserver;
} CharacterTransfer;

extern CharacterTransfer characterTransfer;
void characterTransferMonitor(void);