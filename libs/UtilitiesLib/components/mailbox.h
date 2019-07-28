#ifndef _MAILBOX_H
#define _MAILBOX_H


typedef struct MailboxMessage MailboxMessage;
typedef struct Mailbox
{
	MailboxMessage *msg;
	int msgcount;
	int msgmax;
} Mailbox;

/*checks all messages in mailbox for msg, returns 1 if found*/
int lookForThisMsgInMailbox( Mailbox * mailbox, int sender, int msg  );
int putMsgInMailbox(Mailbox * mailbox, int sender, int msg );
int clearMailbox(Mailbox * mailbox);

#endif