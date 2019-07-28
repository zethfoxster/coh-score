/***************************************************************************
 *     Copyright (c) 2008-2008, NCSoft
 *     All Rights Reserved
 *     Confidential Property of NCSoft
 *
 * Module Description:
 *
 *
 ***************************************************************************/
#include "ncstd.h"
#include "net.h"
#include "http.h"
#include "ncHash.h"
#include "Array.h"
#include "ncunittest.h"

static BOOL connected = FALSE;
static int disconnected = 0;
static BOOL req_recvd = FALSE;
static BOOL req_can_respond = FALSE;

char *foo = "foo";

static void connect_cb(HTTPServer *s, HTTPLink *l)
{
    l;
    s;
    connected = TRUE;
}

static void disco_cb(HTTPServer *s, HTTPLink *l,BOOL error)
{
    s;
    l;
    disconnected = error?2:1;
}

static BOOL req_handler(HTTPServer *svr, HTTPLink *client, HTTPReq *req)
{
    svr;
    UTASSERT(0==strcmp("foo",req->uri)||0==strcmp("bar",req->uri));

    if(!req_can_respond)
        return TRUE;

    if(0==strcmp("foo",req->uri))
        http_sendstr(client,foo);
    else
    {
        http_sendbadreq(client);
        http_shutdownlink(client,TRUE);
    }

    req->done = TRUE;
    return TRUE;        // handled
}

static BOOL recv_foo_response(SOCKET client)
{
    char rb[4096];
    int nr;
    nr = recv(client,rb,ARRAY_SIZE(rb),0);
    if(nr < 0)
	{
        UTERR("recv failed.\n");
		return FALSE;
	}
    else if(nr == 0)
	{
        UTERR("shouldn't get a close from the server.");
		return FALSE;
	}
    else
    {
        char *s = rb;
        int len;
        rb[nr] = 0;
        UTASSERT(STRSTR_ADVANCE(s,"HTTP/1.1 200"));
        UTASSERT(STRSTR_ADVANCE(s,"Content-Length: "));
        len = atoi(s);
        UTASSERT(len == (int)strlen(foo));
        UTASSERT(STRSTR_ADVANCE(s,"\r\n\r\n"));
        UTASSERT(0==strcmp(s,foo));
        return TRUE;
    }
}


void nchttp_unittest(void)
{
    int i;
    TEST_START("nchttp_unittest");
    for(i = 0; i < 10; ++i)
    {
        SOCKET client = 0;
        HTTPServer *httpserver = NULL;
        BOOL done = FALSE;
        int state = 0; // 0,1 = req,recv foo, 2,3 = req,recv bar, 4 = client close
        
        httpserver = http_server_create(80,req_handler,connect_cb,disco_cb);
        while(!done)
        {
            char rb[4096];
            int nr;
            // sleep to flush IO.
            // @todo -AB: use a flush instead, e.g. select or waitforsingleobject :09/25/08
            Sleep(1);
            http_server_tick(httpserver);
            // ----------
            // an example of a self-communicating non blocking socket

            if(!client)
            {
                struct addrinfo *aiList = NULL;
                struct addrinfo aiHints = {0};
                client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
                memset(&aiHints, 0, sizeof(aiHints));
                
                aiHints.ai_family = AF_INET;
                aiHints.ai_socktype = SOCK_STREAM;
                aiHints.ai_protocol = IPPROTO_TCP;
                getaddrinfo("127.0.0.1", "80", &aiHints, &aiList);
                if(connect(client, aiList->ai_addr, (int)aiList->ai_addrlen) < 0)
                    printf("failed to connect to local webserver\n");
                freeaddrinfo(aiList);
//                sock_set_noblocking(client);
                req_can_respond = TRUE;
                connected = 0;
            }
            else if(!connected)
            {
                // wait until server socket via accept
                // ab: for some reason this happens some time. it would be like calling connect on
                //     one line and then accept on the next line and not having it work. very strange
                //     I suppose it goes through hardware even on localhost maybe?
            }
            else if(state==0)
            {
                char *buf = "GET foo HTTP/1.1\r\nHost: localhost\r\n\r\n";
                int n = strlen(buf);
                int ns = send(client,buf,n,0);
                UTASSERT(ns == n);
                if(ns>0)
                {
                    UTASSERT(ap_size(&httpserver->clients)==1);
//                     LOGF("sent %s",buf);
                    state++;
                }
            }
            else if(state == 1)         // get 'foo' back
            {
                UTASSERT(ap_size(&httpserver->clients[0]->reqs)==0); // req has been served
                if(recv_foo_response(client))
                    state++;
            }
            else if(state == 2)         // request 'bar'
            {
                char *buf = "GET bar HTTP/1.1\r\nHost: localhost\r\n\r\n";
                int n = strlen(buf);
                int ns = send(client,buf,n,0);
                UTASSERT(ns == n);
                UTASSERT(ap_size(&httpserver->clients)==1);
//                 LOGF("sent %s",buf);
                state++;
            }
            else if(state == 3) // req for 'bar' fails with 400, followed by server-side shutdown 
            {
                HTTPLink *c = ap_get(&httpserver->clients,0);
                nr = recv(client,rb,ARRAY_SIZE(rb),0);
                UTASSERT(nr>0);
                UTASSERT(strstr(rb,"HTTP/1.1 400"));
                UTASSERT(c);
                UTASSERT(c->shutdown);         // link should be closed on server side data
                nr = recv(client,rb,ARRAY_SIZE(rb),0);
                UTASSERT(nr == 0);             // recv shutdown from server 
                shutdown(client,SD_SEND);       // send shutdown to server
                state++;
            }
            else if(state == 4) // server should have seen shutdown from client. close socket
            {
                UTASSERT(!ap_get(&httpserver->clients,0)); // client should be cleaned up
                closesocket(client); // kick off a reconnect
                client = 0;
                state++;
            }
            else if(state == 5) // do client-side shutdown (only get here if connected)
            {
                char *buf = "GET foo HTTP/1.1\r\nHost: localhost\r\n\r\n";
                int n = strlen(buf);
                int ns = send(client,buf,n,0);
                UTASSERT(ns > 0);
                disconnected = FALSE;
                shutdown(client,SD_SEND);
                req_can_respond = FALSE;
                state++;
            }
            else if( state == 6 ) // make sure you still get responses on client closed sockets
            {
                // just run a couple of ticks
                Sleep(1);
                http_server_tick(httpserver);
                UTASSERT(disconnected==0);

                req_can_respond = TRUE;
                Sleep(1);
                http_server_tick(httpserver);
                UTASSERT(disconnected==1);
                if(recv_foo_response(client))           // response
                {
                    nr = recv(client,rb,ARRAY_SIZE(rb),0);  // server shutdown
                    UTASSERT(nr == 0);
                    closesocket(client);
                    client = INVALID_SOCKET;
                    done = TRUE;
                }
            }
        }
        UTASSERT(ap_size(&httpserver->clients)==0);
        http_server_destroy(&httpserver);
    }
    // let's try multiple sockets
    {
    }
    TEST_END;
}
