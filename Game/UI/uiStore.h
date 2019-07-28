
#ifndef UISTORE_H
#define UISTORE_H

int storeWindow();
void storeClose(void);
void storeOpen(int id, int iCnt, char **ppchStores);
int isStoreOpen();
void storeDisableRestoreContact();

int store_willBuyInventoryEnhancement( int idx );
void store_SellEnhancement( int idx );

extern int g_IdStore;

#endif