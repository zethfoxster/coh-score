#ifndef _COH_NET_H
#define _COH_NET_H

void    sockStart();
int cohConnect(char *server_name);
int cohLogin(char *username,char *password);
void cohSendMsg(char *msg);
char *cohGetMsg();
int cohConnected();

#endif
