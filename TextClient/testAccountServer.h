#pragma once

void testAccountServer(void);
void testAccountAddCharacterCount(char *shard, int count);
void testAccountReceiveCharacterCountsDone(void);
void testAccountCharacterCountsError(char *errmsg);
bool testAccountIsTransferPending(void);
char *testAccountGetOriginalCharName(void);
char *testAccountGetNewCharName(void);
void testAccountRevertCharName(void);
void testAccountLoginReconnect(void);
bool IsTestAccountServer_SuperPacksOptionActivated(void);