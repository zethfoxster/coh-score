#ifndef UINPC_PROFILE_H
#define UINPC_PROFILE_H

typedef struct VillainDef VillainDef;
void NPCProfileMenu();
void setupNPCProfileTextBlocks(char **defaultName, char **defaultDesc, char **vg_name, int vg_locked);
void destroyNPCProfileTextBlocks();
void NPCProfileMenu_setCurrentVDef(const VillainDef *vDef);
#endif