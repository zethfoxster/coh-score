void checkTarget(void);
void doAttacks(void);

typedef enum {
	FM_BEST_ATTACK_POWER,
	FM_CYCLE_POWERS,
} FighterMode;

void setFighterMode(FighterMode fm);