#include "coh_net.h"
#include "coh_string.h"
#include "md5.h"
#include <stdio.h>
#include <stdlib.h>
#ifdef WIN32
#include "windows.h"
#include <winsock2.h>
#else
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#define closesocket(x) close(x)
#define ioctlsocket(x,y,z) ioctl(x,y,(unsigned long)z)
#define Sleep(x) usleep(x*1000)
#endif

#define DEFAULT_SHARDCOMM_PUB_PORT	6987
static 	int		client_sock;

void    sockStart()
{
#ifdef WIN32
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	wVersionRequested = MAKEWORD(2, 0);

    err = WSAStartup(wVersionRequested, &wsaData);

    if (err) fprintf(stderr,"Winsock error..");
#endif
}

void sockSetAddr(struct sockaddr_in *addr,unsigned int ip,int port)
{
    memset(addr,0,sizeof(struct sockaddr_in));
    addr->sin_family=AF_INET;
    addr->sin_addr.s_addr=ip;
    addr->sin_port = htons((u_short)port);
}

void sockBind(int sock,const struct sockaddr_in *name)
{
    if (bind (sock,(struct sockaddr *) name, sizeof(struct sockaddr_in)) >= 0) return;

	printf("cant bind socket!\n");
	exit(1);
}

void sockSetBlocking(int fd,int block)
{
	int  noblock;

    noblock = !block;
    ioctlsocket (fd, FIONBIO, &noblock);
}

static void disconnect()
{
	if (client_sock)
		closesocket(client_sock);
	client_sock = 0;
}

void cohSendMsg(char *msg)
{
	char	buf[1000];
	int		amt;

	sprintf(buf,"%s\n",msg);
	amt = send(client_sock,buf,(int)strlen(buf),0);
	if (amt < 0)
		disconnect();
}

void cohSendCmd(char const *fmt, ...)
{
	va_list ap;
	char	*s;

	va_start(ap, fmt);
	s = printEscaped(fmt,ap);
	va_end(ap);
	cohSendMsg(s);
}


char *cohGetMsg()
{
	static	char	buf[10000];
	int		amt;

	amt = recv(client_sock,buf,sizeof(buf)-1,0);
	if (amt > 0)
	{
		buf[amt-1] = 0;
		return buf;
	}
	if (amt == 0)
		disconnect();
	return 0;
}

static int waitForData(int socket,float timeout)
{
	FD_SET readSet;
	struct timeval interval;
	int selectResult;

	FD_ZERO(&readSet);
	FD_SET(socket, &readSet);
	interval.tv_sec = (int)timeout;
	interval.tv_usec = (int)((timeout - interval.tv_sec) * 1000000);

	selectResult = select(0, &readSet, 0, 0, &interval);
	return selectResult;
}

int cohGetCmd(char **args,float timeout)
{
	char	*s;
	int		i,count;

	if (timeout)
		waitForData(client_sock,timeout);
	s = cohGetMsg();
	if (!s)
		return 0;
	count = tokenize_line(s,args,10,0);
	for(i=0;i<count;i++)
		strcpy(args[i],unescapeString(args[i]));
	return count;
}

int cohConnected()
{
	if (client_sock)
		return 1;
	return 0;
}

unsigned int ipFromString(char *s)
{
	struct hostent *hostent=0;
	char	ip_str[100];

	strcpy(ip_str,s);
	if (!isdigit((unsigned char)s[0]))
	{
		hostent = gethostbyname(s);
		if (hostent)
		{
			sprintf(ip_str,"%d.%d.%d.%d",(char)hostent->h_addr_list[0][0],(char)hostent->h_addr_list[0][1],
										(char)hostent->h_addr_list[0][2],(char)hostent->h_addr_list[0][3]);
		}
	}
	return inet_addr(ip_str);
}

int cohConnect(char *server_name)
{
	struct  sockaddr_in addr;
	int     ret,addr_len = sizeof(struct sockaddr_in),one=1;

    client_sock = (int)socket(AF_INET,SOCK_STREAM,0);
    sockSetAddr(&addr,ipFromString(server_name),DEFAULT_SHARDCOMM_PUB_PORT);
	ret = connect(client_sock,(struct sockaddr *)&addr,sizeof(addr));
	if (ret != 0)
	{
		closesocket(client_sock);
		return 0;
	}
	setsockopt(client_sock,IPPROTO_TCP,TCP_NODELAY,(void *)&one,sizeof(one));
    sockSetBlocking((int)client_sock,0);
	return 1;
}

int cohLogin(char *username,char *password)
{
	char	*args[10];
	int		count;
	char	pass_hash[16];
	int		pass_md5[4];
	MD5_CTX	ctx;

	count = cohGetCmd(args,5);
	if (count != 2 || stricmp(args[0],"SessionId")!=0)
		return 0;

	quickHash(password,pass_hash,16);
	MD5Init(&ctx);
	MD5Update(&ctx,pass_hash,16);
	MD5Update(&ctx,args[1],strlen(args[1]));
	MD5Final(&ctx);
	memcpy(pass_md5,ctx.digest,16);

	cohSendCmd("PubLogin %s %d %d %d %d",username,pass_md5[0],pass_md5[1],pass_md5[2],pass_md5[3]);
	return 1;
}

