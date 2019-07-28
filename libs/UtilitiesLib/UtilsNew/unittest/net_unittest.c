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
#include "ncHash.h"
#include "Array.h"
#include "ncunittest.h"
#include "net.h"

#define alnk_size(ha) alnk_size_dbg(ha  DBG_PARMS_INIT)

#define TYPE_T NetLink*
#define TYPE_FUNC_PREFIX alnk
#include "Array_def.h"
#undef TYPE_T
#undef TYPE_FUNC_PREFIX

#define asvr_size(ha) asvr_size_dbg(ha  DBG_PARMS_INIT)


#define TYPE_T NetServer*
#define TYPE_FUNC_PREFIX asvr
#include "Array_def.h"
#undef TYPE_T
#undef TYPE_FUNC_PREFIX


typedef enum MsgId
{
    MsgId_None,
    MsgId_Test1,
    MsgId_Test1Res,
    MsgId_Test2,
    MsgId_Test2Res,
} MsgId;

static MsgId svr_msgid;
static BOOL svr_conn, svr_disco;
static BOOL svr_msg_wait;
static int svr_recv_parm;
static void SvrLinkConn(NetLink* link)
{
    link;
    svr_conn = TRUE;
}

static void SvrLinkDisco(NetLink* link)
{
    link;
    svr_disco = TRUE;
}

static BOOL SvrMessage(NetLink* link,Pak *pak,int msgid)
{
    Pak *p;
    pak; link;
    svr_msgid = msgid;
    if(svr_msg_wait)
        return FALSE;
    svr_recv_parm = pak_get_int(pak);

    p = pak_create(link,msgid+1);
    pak_send_int(p,(int)((intptr_t)(link->s)));
    pak_send(link,p);
    
    return TRUE; // msg handled, destroy packet
}

// *************************************************************************
//  client
// *************************************************************************

static BOOL clnt_conn;
static BOOL clnt_disco;
static MsgId clnt_msgid;
static int clnt_recv_parm;
static void ClntLinkConn(NetLink* link)
{
    link;
    clnt_conn = TRUE;
}

static void ClntLinkDisco(NetLink* link)
{
    link;
    clnt_disco = TRUE;
}

static BOOL ClntMessage(NetLink* link,Pak *pak,int cmd)
{
    link;
    clnt_msgid = cmd;
    clnt_recv_parm = pak_get_int(pak);
    return TRUE; // Pak handled
}

// static NetLink *clnt;
// unsigned clnt_threadmain(void *args)
// {
//     clnt = c;
//     for(;;)
//     {
//         netlink_tick(c);
//     }
// }

void net_unittest(void)
{
    // basic: 
    // - server create
    // - client connects
    // - handshake happens
    // - server discos client
    // - client recvs disco
    // - done
    TEST_START("net_unittest");
    {
        NetCtxt *ctxt = net_create();
        int i;
        NetServer *s = netserver_create(ctxt,5678,
                                         SvrLinkConn,
                                         SvrLinkDisco,
                                         SvrMessage);
        NetLink *c = net_connect(ctxt,"localhost",5678,ClntLinkConn,ClntLinkDisco,ClntMessage,FALSE);
        Pak *p;

        UTASSERT(ctxt);
        UTASSERT(s && c);
        UTASSERT(ctxt->n_lnks == 1);
        UTASSERT(ctxt->n_svrs == 1);
        UTASSERT(alnk_size(&ctxt->links) == 1);
        UTASSERT(asvr_size(&ctxt->servers) == 1);

        // ----------
        // first the handshake: both sides should
        // be connected.

        for( i = 0; i < 10; ++i)
        {
            net_tick(ctxt);
            if(c->handshaken && s->clients && s->clients[0] && s->clients[0]->handshaken)
                break;
            Sleep(1);
        }
        UTASSERT(c->handshaken);
        UTASSERT(alnk_size(&s->clients) == 1);
        UTASSERT(s->clients && s->clients[0] && s->clients[0]->handshaken);
        UTASSERT(clnt_conn);
        UTASSERT(svr_conn);

        // ----------
        // sequence of messages:
        // clnt: test1, s
        // srvr: recv test1, s. send test1+1, s
        // clnt: recv test1+1, s

        p = pak_create(c,MsgId_Test1);
        pak_send_int(p,(int)(intptr_t)s);
        pak_send(c,p);
        
        for( i = 0; i < 10 && svr_msgid != MsgId_Test1; ++i ) 
        {
            net_tick(ctxt);
            Sleep(1);
        }
        UTASSERT( svr_msgid == MsgId_Test1 );
        UTASSERT( clnt_msgid == MsgId_Test1Res );
        UTASSERT( clnt_recv_parm == (int)(intptr_t)s );
        UTASSERT( svr_recv_parm == (int)(intptr_t)s );

        {
            int tmp = (int)(intptr_t)s;
            printf("%i\n",tmp);
        }
        
        // ----------
        // shutdown
        // clnt: send msg, then shutdown
        // srvr: send msg+1, shutdown, close
        // clnt: recv/handle packet, close
        //net_shutdownlink
        // todo
        UTASSERT(clnt_disco && svr_disco);
        net_destroy(ctxt);
    }
    TEST_END;
}
