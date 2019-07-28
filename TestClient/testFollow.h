typedef enum {
	FM_RETURNING, // Currently heading towards leader
	FM_FREEDOM, // Currently free-ranging
} FollowMode;

void checkFollow(void);