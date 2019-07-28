

void zowie_Load(void);
void zowie_SetupEntities();
int zowie_ObjectivesRemaining(StoryTaskInfo *task);
bool zowie_ReceiveReadZowie(Packet *pak, Entity *e);
int zowie_setup(StoryTaskInfo* task, int fullInit);
int ZowieComplete(Entity *player, MissionObjectiveInfo *target);