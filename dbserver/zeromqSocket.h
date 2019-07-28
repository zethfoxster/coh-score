#ifndef _ZEROMQSOCKET_H
#define _ZEROMQSOCKET_H

#define IP_ADD_MAX 16000 //enough space for really, really long hostnames

typedef struct
{
	// ZeroMQ context
	void * context;

	// ZeroMQ router connection for message data
	void * outgoing;

	int portNumber;
	int zmqType;

	int connectError;

	unsigned __int64 highWaterMark;
	char ipAddress[IP_ADD_MAX];
} ZmqSocket;

void zmqInit(ZmqSocket* zmqConn, int zmqType);
void zmqCreateSocket(ZmqSocket* zmqConn);
void zmqCloseSocket(ZmqSocket* zmqConn);
void zmqConnect(ZmqSocket* zmqConn);
void zmqSend(ZmqSocket* zmqConn, char* data, U32 dataSize);
void zmqShutdown(ZmqSocket* zmqConn);

void zmqResetConnection(ZmqSocket* zmqConn);
void zmqSetNewIPAddress(ZmqSocket* zmqConn, char* address);
void zmqSetNewPortNumber(ZmqSocket* zmqConn, int portNumber);
void zmqSetHighWaterMark(ZmqSocket* zmqConn, unsigned __int64 hwm_value);
#endif