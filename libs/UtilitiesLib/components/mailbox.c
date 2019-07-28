#include "memory.h"
#include "mailbox.h"
#include "error.h"
#include "utils.h"
#include "timing.h"

typedef struct MailboxMessage
{
	int sender;
	int flag;  
} MailboxMessage;



//TO DO was this a good idea?  Think about it...

/*checks all messages in mailbox for MailboxMessage, returns 1 if found*/
int lookForThisMsgInMailbox( Mailbox * mailbox, int sender, int flag  )
{
	int i;
	if(mailbox && mailbox->msgcount && sender && flag)
	{
		for(i = 0 ; i < mailbox->msgcount ; i++)
		{	
			if(mailbox->msg[i].sender == sender && (mailbox->msg[i].flag & flag) )
			{
				return 1;
			}
		}
	}
	return 0;
}

int putMsgInMailbox(Mailbox * mailbox, int sender, int flag )
{
	MailboxMessage *msg;
	
	PERFINFO_AUTO_START("putMsgInMailbox", 1);
		msg = dynArrayAdd(&mailbox->msg, sizeof(MailboxMessage), &mailbox->msgcount, &mailbox->msgmax, 1);

		msg->flag = flag;
		msg->sender = sender;
	PERFINFO_AUTO_STOP();

	return 1;
}


int clearMailbox(Mailbox * mailbox)
{
	if(mailbox)
	{
		mailbox->msgcount = 0;
		return 1;
	}
	return 0;
}



