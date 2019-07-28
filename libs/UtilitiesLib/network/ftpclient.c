/***************************************************************************
 *     Copyright (c) 2008-2008, NCSoft
 *     All Rights Reserved
 *     Confidential Property of NCSoft
 *
 * Module Description:
 *
 *
 ***************************************************************************/
#include "ftpclient.h"
#include "EString.h"
#include "sock.h"
#include "utils.h"
#include "assert.h"
#include "error.h"
#include "mathutil.h"
#include "earray.h"
#include "MemoryPool.h"
#include "StashTable.h"
#include "structNet.h"

MP_DEFINE(FtpClient);
static FtpClient* FtpClient_Create( SOCKET s )
{
	FtpClient *res = NULL;

	// create the pool, arbitrary number
	MP_CREATE(FtpClient, 64);
	res = MP_ALLOC( FtpClient );
	if( devassert( res ))
	{
		res->s = s;
	}
	return res;
}

void FtpClient_Destroy( FtpClient *hItem )
{
	if(hItem)
	{
        closesocket(hItem->s);
		MP_FREE(FtpClient, hItem);
	}
}


FtpClient *ftpLogin(char *ip, char *name, char *pass)
{
    int n;
    char tmp[1024];
    FtpClient *c = NULL;
    SOCKET s = INVALID_SOCKET;
    struct sockaddr_in	addr_in;
    int res;
    
    if(!ip)
        return NULL;
    if(!name)
        name = "anonymous";
    if(!pass)
        pass = "anonymous@nowhere.com";
    
    sockStart();  
    s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockSetAddr(&addr_in,ipFromString(ip),21); // 21 = default ftp listen port
    
    res = connect(s,(struct sockaddr *)&addr_in,sizeof(addr_in));
    if(res != 0)
        goto error;

    // get preamble (ignored)
    n = recv(s,tmp,sizeof(tmp),0);

    // send user, wait for response, send pass
    n = sprintf(tmp,"USER %s\r\n",name);
    send(s,tmp,n,0);
    n = recv(s,tmp,sizeof(tmp),0);

    n = sprintf(tmp,"PASS %s\r\n",pass);
    send(s,tmp,n,0);
    n = recv(s,tmp,sizeof(tmp),0);
    tmp[n] = 0;
    if(strstr(tmp,"230") != tmp)
        goto error;        
    
    c = FtpClient_Create(s);

    // default behavior: binary
    FtpClient_SendCmd(c,"TYPE I"); // 'image' type, i.e. binary

    return c;
    
error:
    closesocket(s);
    return NULL;
}



BOOL FtpClient_SendCmd(FtpClient *c, const char *cmd, ...)
{
    char *str = NULL;
    int n;
    int res;
    
    if(!c || !cmd)
        return FALSE;
    
	VA_START(args, cmd);
    n = estrConcatfv(&str, cmd, args);
	VA_END(); 
    
    if(!strEndsWith(str,"\r\n"))
        estrConcatStaticCharArray(&str,"\r\n");
    
    res = send(c->s,str,estrLength(&str),0);
    estrDestroy(&str);    
    if(res<0)
        return FALSE;
    
    // get the response, success is of form \r\n2XX
    *c->reply = 0;
    n = recv(c->s,c->reply,sizeof(c->reply),0);
    c->reply[n] = 0;
    
    // 5 values for first digit:
    // 1yz   Positive Preliminary reply - further info coming from server.
    // 2yz   Positive Completion reply - success, and done
    // 3yz   Positive Intermediate reply - further info needed from client
    // 4yz   Transient Negative Completion reply
    // 5yz   Permanent Negative Completion reply
    if(!INRANGE(c->reply[0],'1','4')) // all correct responses start with 2
        return FALSE;
    return TRUE;
}

BOOL FtpClient_SendDataCmd(FtpClient *c, const char *cmd, ...)
{
    BOOL cmdres;
    int n;
    int cursor = 0;
    SOCKET data_socket = INVALID_SOCKET;
    struct sockaddr_in addr = {0};    
    int r;
    BOOL res = FALSE;
    char *str = NULL;
    int portlow, porthi;
    u_short port;
    
    if(!cmd)
        return FALSE;
   
    if(!FtpClient_SendCmd(c,"PASV"))
        return FALSE;

    // scan the reply: 
    // 227 Entering Passive Mode (206,127,155,235,193,198) (port hi/low)
    addr.sin_family = AF_INET;
    r = sscanf(strstr(c->reply,"("),"(%u,%u,%u,%u,%u,%u",&addr.sin_addr.S_un.S_un_b.s_b1,&addr.sin_addr.S_un.S_un_b.s_b2,&addr.sin_addr.S_un.S_un_b.s_b3,&addr.sin_addr.S_un.S_un_b.s_b4,&porthi,&portlow);
    port = porthi<<8 | portlow;
    addr.sin_port = htons(port);
    
    data_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    r = connect(data_socket,(struct sockaddr *)&addr,sizeof(addr));
    
    if(0 != r)
        goto done;

    FtpClient_SendCmd(c,"TYPE I"); // 'image' type, i.e. binary

	VA_START(args, cmd);
    n = estrConcatfv(&str, cmd, args);
	VA_END();

    cmdres = FtpClient_SendCmd(c,str);
    estrDestroy(&str);

    if(!cmdres)
        goto done;


    // slurp in all the data
    if(!c->data)
    {
        c->datacapacity = FTPCLIENT_STARTING_DATA_SIZE;
        c->data = malloc(c->datacapacity);
    }
    
    c->datalen = 0;
    while(n = recv(data_socket,((char*)c->data)+c->datalen,c->datacapacity-c->datalen,0))
    {
        if(n == -1)
            goto done;        
        c->datalen += n;
        if(c->datalen+1 >= c->datacapacity) // give 1 byte room for terminating char for string commands
        {
            c->datacapacity *= 2;
            c->data = realloc(c->data,c->datacapacity);
        }
    }
    closesocket(data_socket);
    data_socket = INVALID_SOCKET;
    
    // in some cases there is pending data (if 1 in reply), grab it
    if(c->reply[0] == '1')
    {
        n = recv(c->s,c->reply,sizeof(c->reply),0);
        if(n>=0)
            c->reply[n] = 0;
    }

    res = TRUE;
done:
    if(data_socket != INVALID_SOCKET)
        closesocket(data_socket);
    return res;
}

BOOL FtpClient_CD(FtpClient *c, char *path)
{
    return FtpClient_SendCmd(c,"CWD %s", path);
}


BOOL FtpClient_LS(FtpClient *c, char *dir)
{
    if(!FtpClient_SendDataCmd(c,"NLST %s", dir?dir:""))
        return FALSE;
    ((char*)c->data)[c->datalen] = 0;
    return TRUE;
}

BOOL FtpClient_FileGet(FtpClient *c, char *file_path)
{
    return file_path
        && FtpClient_SendDataCmd(c,"RETR %s", file_path);
}

BOOL FtpClient_RM(FtpClient *c, char *file_path)
{
    return file_path && FtpClient_SendCmd(c, "DELE %s", file_path);
}
