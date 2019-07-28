#ifndef UINPCCOSTUME_H
#define UINPCCOSTUME_H

typedef struct Costume Costume;
typedef struct cCostume cCostume;
typedef struct CustomNPCCostume CustomNPCCostume;
typedef struct VillainDef VillainDef;
typedef struct NPCDef NPCDef;
void CustomNPCEdit_setup(const NPCDef *contactDef, const VillainDef *vDef, int index, CustomNPCCostume **npcCostume, int npcCostumeEditMode, char **displayNamePtr, char **descriptionPtr, char **vg_name, int vg_name_locked);
void CustomNPCEdit_exit(int saveChanges);
void NPCCostumeMenu();
int getNPCEditCostumeMode();
int NPCCostume_canEditCostume(const cCostume *costume);

#endif