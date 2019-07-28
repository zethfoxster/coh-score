
#ifndef BASECLIENTRECV
#define BASECLIENTRECV

typedef struct Packet Packet;
typedef struct Base Base;

void baseReceive(Packet *pak, int raidIndex, Vec3 offset, int isStitch);
void setupInfoPointers(void);

void baseUpdateEntityLighting(void);
int checkBaseCorrupted(Base *base);
bool baseAfterLoad(int raidIndex, Vec3 offset, int isStitch);

#endif
