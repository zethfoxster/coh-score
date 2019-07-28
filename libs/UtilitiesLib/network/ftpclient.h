/***************************************************************************
 *     Copyright (c) 2008-2008, NCSoft
 *     All Rights Reserved
 *     Confidential Property of NCSoft
 *
 * Module Description:
 *
 *
 ***************************************************************************/
#ifndef FTPCLIENT_H
#define FTPCLIENT_H

#include "winsock2.h" // for SOCKET

#define FTPCLIENT_STARTING_DATA_SIZE (1024*16)

typedef struct FtpClient
{
    SOCKET s;
    char reply[128];
    void *data;               // malloc'd
    int datalen;              // how full
    int datacapacity;         // buf capacity
} FtpClient;

FtpClient *ftpLogin(char *ip, char *name, char *pass);
void FtpClient_Destroy( FtpClient *hItem );
BOOL FtpClient_SendCmd(FtpClient *c, FORMAT args, ...);
BOOL FtpClient_SendDataCmd(FtpClient *c, FORMAT args, ...);

// ----------
// helper functions

BOOL FtpClient_CD(FtpClient *c, char *path);
BOOL FtpClient_LS(FtpClient *c, char *dir);
BOOL FtpClient_FileGet(FtpClient *c, char *file_path);
BOOL FtpClient_RM(FtpClient *c, char *file_path);

#endif //FTPCLIENT_H
