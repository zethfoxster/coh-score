typedef struct Entity Entity;
typedef struct Power Power;
typedef struct CharacterClass CharacterClass;
typedef struct BasePowerSet BasePowerSet;

int simulateCharacterCreate(int askuser, int connectionless);
int randInt2(int max);
int promptRanged(char *str, int max);
Power *testChooseAPowerSet(Entity *e, int askuser, int category);
int isAccountAbleToSelectClass(Entity *e, const CharacterClass *pClass);
int isAccountAbleToSelectPowerSet(Entity *e, const BasePowerSet *pPowerSet);
