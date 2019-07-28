typedef struct TCStats {
	int deathcount;
	int defeatcount;
	int xp;
	int influence;
} TCStats;

extern TCStats stats;

int testChatNPC(void); // Returns the number of NPCs interacted with
void chatClientInit();