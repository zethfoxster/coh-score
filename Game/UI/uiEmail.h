#ifndef _UIEMAIL_H
#define _UIEMAIL_H

typedef enum EmailType
{
	kEmail_Local,
	kEmail_Global,
	kEmail_Certification,
	kEmail_Count,
}EmailType;

typedef enum MailViewMode
{
	MVM_MAIL,
	MVM_VOUCHER,
	MVM_COUNT,
}MailViewMode;

void emailHeaderListPrepareUpdate();
void emailResetHeaders(int quit_to_login);
void emailAddHeader(U64 message_id,int auth_id,char *sender,char *subject,int sent, int influence, char * attachment );
void emailCacheMessage(U64 message_id, EmailType type, char *recip_buf,char *msg);
int  emailWindow();
int  emailComposeWindow();
void emailSetNewMessageStatus(int status,char *msg);
int hasEmail(int type);
void emailAddHeaderGlobal(const char * sender, int sender_id, const char * subj, const char * msg, const char * attachment, int sent, U64 id, int cert, int claims, int subType );
void emailClearAttachments( U64 id );
int globalEmailIsFull();
char* emailBuildDateString(char* datestr, U32 seconds);

void updateCertifications(int fromMap);
void email_setView(char *view);
int email_getView();

#define EMAIL_FROM_COLUMN		"EmailFrom"
#define EMAIL_SUBJECT_COLUMN	"EmailSubject"
#define EMAIL_DATE_COLUMN		"EmailDate"
// End mkproto
#endif
