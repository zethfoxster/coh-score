
#ifndef UITEXTLINK_H
#define UITEXTLINK_H

void init_TextLinkStash(void);

bool isValidLinkToken( char * pchName );
void textLink(char *pchName );

typedef struct DetailRecipe DetailRecipe;
typedef struct BasePower BasePower;
typedef struct SalvageItem SalvageItem;

char *getTextLinkForRecipe(const DetailRecipe *pRecipe);
char *getTextLinkForPower(const BasePower *ppow, int iLevel);
char *getTextLinkForSalvage(SalvageItem *pSalvage);

#endif