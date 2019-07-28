#ifndef TASKRANDOM_H
#define TASKRANDOM_H

typedef enum {
	VGT_ANY,
	VGT_ARCANE,
	VGT_TECH,
} VillainGroupType;

typedef struct Entity Entity;
typedef struct StoryTask StoryTask;
typedef struct ScriptVarsScope ScriptVarsScope;

void loadRandomDefFiles();
VillainGroupEnum getVillainGroupFromTask(const StoryTask * task);
VillainGroupEnum getVillainGroupTypeFromTask(const StoryTask * task);
MapSetEnum getMapSetFromTask(const StoryTask * task);
void getRandomVillainGroup(const Entity * player,VillainGroupEnum * vge,int level,VillainGroupType type,int seed);
void getRandomMapSet(const Entity * player,MapSetEnum * mse,char ** filename,int level,int seed);
void getRandomMapSetForVillainGroup(const Entity * player,MapSetEnum * mse,char ** filename,int level,VillainGroupEnum vge,int seed);
void getRandomVillainGroupAndMapSet(const Entity * player,VillainGroupEnum * vge,MapSetEnum * mse,char ** filename,int level,VillainGroupType type,int seed);
void getRandomFields(const Entity * player,VillainGroupEnum * vge,MapSetEnum * mse,char ** filename,int level,const StoryTask * st,int seed);
void randomAddVarsScopeFromStoryTaskInfo(ScriptVarsTable * svt, const Entity * ent, const StoryTaskInfo * sti);
void randomAddVarsScopeFromContactTaskPickInfo(ScriptVarsTable * svt, const Entity * ent, const ContactTaskPickInfo * pickInfo);
void randomAddMapVarsScope(ScriptVarsTable * svt, const Entity * player, MapSetEnum mse);
void randomAddGroupVarsScope(ScriptVarsTable * svt, const Entity * player, VillainGroupEnum vge);

int isVillainGroupAcceptableGivenMapSet(const Entity * player,VillainGroupEnum vge,MapSetEnum mse,int level);
int isVillainGroupAcceptableGivenLevel(const Entity * player,VillainGroupEnum vge,int level);
int isVillainGroupAcceptableGivenVillainGroupType(const Entity * player,VillainGroupEnum vge,VillainGroupType vgt);
#endif