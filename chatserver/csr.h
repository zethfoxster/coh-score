#ifndef _CSR_H
#define _CSR_H

void userCsrSilence(User *user,char *target_name,char *minutes);
void userCsrRenameable(User *user, char *target_name);
void csrSilenceAll(User *user);
void csrUnsilenceAll(User *user);
void csrSendAll(User *user,char *msg);
void csrSendAllAnon(char *msg); 
void csrSysMsgSendAll(User *user,char *msg);
void csrRenameAll(User *user);
void csrStatus(User * user, char *target_name);
void csrRemoveAll(User *user);
void csrCheckMailSent(User * user, char *target_name);
void csrBounceMailSent(User * user, char *target_name, char * id );
void csrCheckMailReceived(User * user, char *target_name);
void csrBounceMailReceived(User * user, char *target_name, char * id );

#endif
