#ifndef CUSTOM_VILLAIN_GROUP_CLIENT_H
#define CUSTOM_VILLAIN_GROUP_CLIENT_H

typedef struct CustomVG CustomVG;

char * getCustomVillainGroupDir();
int CVG_removeCustomVillainGroup(char *displayName, int removeSource);
void loadCVGFromFiles();
void initCVGFolderCache();
int CVG_removeAllEmptyCVG(CustomVG **vgList);
#endif