#ifndef UISALVAGEOPEN_H
#define UISALVAGEOPEN_H

int superPackDescriptionsLoad(void);
void setSalvageOpenResultsPackage(char *package);
void setSalvageOpenResultsDesc(char *product, int quantity, const char *rarity);

int salvageOpenWindow(void);

#endif