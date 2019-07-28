#include "msgsend.h"
#include "comm_backend.h"
#include "utils.h"
#include "shardnet.h"
#include "admin.h"
#include "earray.h"

static char g_resend_msg[10000];

void msgFlushPacket(NetLink *link)
{
	ClientLink	*client = link->userData;

	if (client->aggregate_pak)
		pktSend(&client->aggregate_pak,link);
}

void sendMsg(NetLink *link,int auth_id,char *msg)
{
	ClientLink	*client = link->userData;
	Packet		*aggregate_pak;

	if (!link->connected)
		return;
	if (!client->aggregate_pak)
	{
		link->compressionAllowed = 1;
		link->compressionDefault = 1;
		client->aggregate_pak = pktCreateEx(link,SHARDCOMM_SVR_CMD);
	}
	aggregate_pak = client->aggregate_pak;
	pktSendBitsPack(aggregate_pak,31,auth_id);
	if (strcmp(msg,client->last_msg)==0)
	{
		pktSendBits(aggregate_pak,1,1);
	}
	else
	{
		strncpy(client->last_msg,msg,ARRAY_SIZE(client->last_msg)-1);
		pktSendBits(aggregate_pak,1,0);
		pktSendString(aggregate_pak,msg);
	}
	if (client->aggregate_pak->stream.size > 500000)
		msgFlushPacket(link);
	g_stats.sendmsg_count++;
}

void resendMsg(User *user)
{
	ClientLink	*client;

	if (!user->link || !user->link->connected)
		return;
	client = user->link->userData;
	if (user->publink_id)
		sendMsg(user->link,user->publink_id,g_resend_msg);
	else
		sendMsg(user->link,user->auth_id,g_resend_msg);
}

static void tailcat(char **ptail, int *ptail_len, const char *str)
{
	int len;
	strncpy_s(*ptail, *ptail_len, str, _TRUNCATE);
	len = strlen(*ptail);
	*ptail += len;
	*ptail_len -= len;
}

static bool printAndSend(User *user,char *pre,const char* fmt, va_list ap)
{
	const		char *sc;
	char		*tail;
	int			tail_len;
	ClientLink	*client;

  	if (!user->link || !user->link->connected)
		return 0;

	client = user->link->userData;

	tail = g_resend_msg;
	tail_len = ARRAY_SIZE_CHECKED(g_resend_msg);

	if (pre)
		tailcat(&tail, &tail_len, pre);

	for(sc = fmt; *sc && tail_len > 1; sc++)
	{
		if (sc[0] == '%' && sc[1] != '%' && sc[1])
		{
			switch(sc[1])
			{
				xcase 's':
				{
					char *s = va_arg(ap,char *);
					tailcat(&tail, &tail_len, "\"");
					tailcat(&tail, &tail_len, escapeString(s));
					tailcat(&tail, &tail_len, "\"");
				}
				xcase 'S':
				{
					char *s = va_arg(ap,char *);
					tailcat(&tail, &tail_len, escapeString(s));
				}
				xcase 'd':
				case 'i':
				{
					char buf[12];
					int val = va_arg(ap,int);
					itoa(val, buf, 10);
					tailcat(&tail, &tail_len, buf);
				}
			}
			sc++;
		}
		else
		{
			*tail++ = *sc;
			tail_len--;
		}
	}
	*tail = 0;
	resendMsg(user);
	return 1;
}

bool sendUserMsg(User *user, char const *fmt, ...)
{
	va_list ap;
	bool	res;

	va_start(ap, fmt);
	res = printAndSend(user,0,fmt,ap);
	va_end(ap);

	return res;
}

bool sendSysMsg(User *user,char *channel_name, int is_error, char const *fmt, ...)
{
	va_list	ap;
	char	pre[1000];
	bool	res;

	if (channel_name)
		sprintf(pre,"ChanSysMsg \"%s\" ",escapeString(channel_name));
	else
		strcpy(pre,"SysMsg ");
	va_start(ap, fmt);
	res = printAndSend(user,pre,fmt,ap);
	va_end(ap);

	if(is_error)
		g_stats.error_msgs_sent++;

	return res;
}

void sendAdminMsg(char * fmt, ...)
{
	int i;
	bool resend = 0;
	for(i=eaSize(&admin_clients)-1; i>=0; --i)
	{
		User* user = admin_clients[i]->adminUser;
		if(resend)
			resendMsg(user);
		else
		{
			va_list ap;
			va_start(ap, fmt);
			resend = printAndSend(user,0,fmt,ap);
			va_end(ap);
		}
	}
}
