

#ifndef DBBACKUP_H
#define DBBACKUP_H

typedef struct ClientLink ClientLink;
typedef struct Packet Packet;

void dbbackup( ClientLink * client, char * name, int type );
	// send tell dbserver to back this player up

void dbbackup_search( ClientLink * client, char * name, int type );
	// ask dbserver for list of all the saves available for this player

void dbbackup_view( ClientLink * client, char * name, int idx, int type );
	// ask dbserver to view entire text of one of the saves

void dbbackup_apply( ClientLink * client, char * name, int idx, int type );
	// tell dbserver to apply one of the saved backups


void handleDbBackupSearch(Packet *pak);
	// Recieve search data from dbserver

void handleDbBackupApply(Packet *pak);
	// recieve result of save apply

void handleDbBackupView(Packet *pak);
	// recieve text view of a given save



#endif