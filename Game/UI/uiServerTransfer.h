#ifndef UISERVERTRANSFER_H_
#define UISERVERTRANSFER_H_

typedef void (*StoreExitCallback)(void);

extern void serverTransferStartRedeem(StoreExitCallback exitCB);

extern bool characterTransferReceiveError(char* errorMsg);

extern bool characterTransferAddCharacterCount(char* destShard, int unlockedCount, int totalCount, int accountSlotCount, int serverSlotCount);
extern bool characterTransferReceiveCharacterCountsDone(void);

// Used by the character select screen to figure out whether to draw the
// character slots.
extern bool characterTransferAreSlotsHidden(void);

extern void characterTransferHandleProgress(float screenScaleX, float screenScaleY);

#endif
