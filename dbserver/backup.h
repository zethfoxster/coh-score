
#ifndef BACKUP_H
#define BACKUP_H


typedef struct Packet Packet;
typedef struct NetLink NetLink;

void handleBackup(Packet *pak, NetLink *link);
void handleBackupSearch(Packet *pak, NetLink *link);
void handleBackupApply(Packet *pak, NetLink *link);
void handleBackupView(Packet *pak, NetLink *link);

bool backupSaveContainer( void * container, U32 timestamp );
void backupLoadIndexFiles(void);
void backupUnloadIndexFiles(void);

void setBackupPeriod(int period);

#endif