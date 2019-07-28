#include <zmq.h>
#include "zeromqSocket.h"

#ifdef _FULLDEBUG
#ifdef _M_AMD64
#pragma comment(lib,"libzmqd64.lib")
#else
#pragma comment(lib,"libzmqd.lib")
#endif	
#else
#ifdef _M_AMD64
#pragma comment(lib,"libzmqr64.lib")
#else
#pragma comment(lib,"libzmqr.lib")
#endif	
#endif

void zmqInit(ZmqSocket* zmqConn, int zmqType)
{
	if (zmqConn)
	{
		zmqConn->context = zmq_init (1);
		zmqConn->zmqType = zmqType;
		//default set to 1 so that attempts to call zmqSend without calling zmqConnect first will fail
		zmqConn->connectError = 1;

		zmqCreateSocket(zmqConn);		
	}
}

void zmqCreateSocket(ZmqSocket* zmqConn)
{
	if (zmqConn && zmqConn->context && !zmqConn->outgoing)
	{
		zmqConn->outgoing = zmq_socket(zmqConn->context, zmqConn->zmqType);
	}
}

void zmqCloseSocket(ZmqSocket* zmqConn)
{
	if (zmqConn && zmqConn->outgoing)
	{
		zmq_close(zmqConn->outgoing);
		zmqConn->outgoing = NULL;
	}
}

void zmqConnect(ZmqSocket* zmqConn)
{
	if (zmqConn && zmqConn->outgoing && zmqConn->ipAddress && zmqConn->portNumber)
	{
		char connect_str[16500];
		snprintf(connect_str, sizeof(connect_str), "tcp://%s:%d", zmqConn->ipAddress, zmqConn->portNumber);
		 //connectError will be set to 0 if zmq_connect succeeded
		zmqConn->connectError = zmq_connect(zmqConn->outgoing, connect_str);
	}
}


void zmqSend(ZmqSocket* zmqConn, char* data, U32 dataSize)
{
	if (zmqConn && zmqConn->context && zmqConn->outgoing && !zmqConn->connectError && data)
	{
		zmq_msg_t message;
		zmq_msg_init_size(&message, dataSize);
		memcpy(zmq_msg_data(&message), data, dataSize);
		zmq_send(zmqConn->outgoing, &message, 0);
		zmq_msg_close(&message);
	}
}

void zmqShutdown(ZmqSocket* zmqConn)
{
	if (zmqConn && zmqConn->context)
	{
		zmq_term(zmqConn->context);
		zmqConn->context = NULL;
	}
}

void zmqResetConnection(ZmqSocket* zmqConn)
{
	zmqCloseSocket(zmqConn);
	zmqCreateSocket(zmqConn);
	zmqConnect(zmqConn);
}

void zmqSetNewIPAddress(ZmqSocket* zmqConn, char* address)
{
	if (zmqConn && address)
	{
		strcpy(zmqConn->ipAddress, address);
	}
}

void zmqSetNewPortNumber(ZmqSocket* zmqConn, int portNumber)
{
	if (zmqConn)
	{
		zmqConn->portNumber = portNumber;
	}
}

void zmqSetHighWaterMark(ZmqSocket* zmqConn, unsigned __int64 hwm_value)
{
	if (zmqConn && zmqConn->outgoing)
	{
		zmqConn->highWaterMark = hwm_value;
		zmq_setsockopt (zmqConn->outgoing, ZMQ_HWM, &hwm_value, sizeof(hwm_value));
	}
}

