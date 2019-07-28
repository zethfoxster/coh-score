#include "packetFlood.h"
#include "sock.h"
#include <sys/stat.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include <conio.h>

extern int randInt2(int max);

char data[2048] = {0x40, 0x00, 0x00, 0x00, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18};

SOCKET connectToTcp(U32 destip, int port) {
	struct sockaddr_in addr;
	int result;
	SOCKET s;

	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s==INVALID_SOCKET) {
		printf("Couldn't create socket");
		return INVALID_SOCKET;
	}

	sockSetAddr(&addr,destip,port);
	result = connect(s,(struct sockaddr *)&addr,sizeof(addr));
	if (result != 0) {
		printf("Could not connect to %s:%d\n", makeIpStr(destip), port);
		return INVALID_SOCKET;
	}
	return s;
}


void sendToTCP(U32 destip, int port, char *data, int len) {
	SOCKET s = connectToTcp(destip, port);

	if (s==INVALID_SOCKET) return;

	sockSetDelay(s, 0);
	send(s, data, len, 0);
}

void sendToTCPStream(U32 destip, int port, char *data, int len, int loops) {
	SOCKET s = connectToTcp(destip, port);
	int i;

	if (s==INVALID_SOCKET) return;

	sockSetDelay(s, 0);
	for (i=0; i!=loops; i++) {
		int ret = send(s, data, len, 0);
		if (ret ==SOCKET_ERROR)
			return;
		if (i%100) {
			printf("Sent %d packets (%d bytes)\r", i, len*i);
		}
		Sleep(1);
	}
	printf("Done.                                          \n");
}

void sendToUDP(U32 destip, int port, char *data, int len) {
	struct sockaddr_in addr;
	int result;
	SOCKET s;

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s==INVALID_SOCKET) {
		printf("Couldn't create socket");
		return;
	}

	sockSetAddr(&addr,destip,port);
	result = sendto(s, data, len, 0, (struct sockaddr*)&addr, sizeof(addr));
	if (result != len) {
		printf("Could not send to %s:%d\n", makeIpStr(destip), port);
		return;
	}
}

void sendToUDPStream(U32 destip, int port, char *data, int len, int loops) {
	struct sockaddr_in addr;
	int result;
	SOCKET s;
	int i;

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s==INVALID_SOCKET) {
		printf("Couldn't create socket");
		return;
	}

	sockSetAddr(&addr,destip,port);
	for (i=0; i<loops; i++) {
		result = sendto(s, data, len, 0, (struct sockaddr*)&addr, sizeof(addr));
		if (result != len) {
			printf("Could not send to %s:%d\n", makeIpStr(destip), port);
			return;
		}
		if (i%100) {
			printf("Sent %d packets (%d bytes)\r", i, len*i);
		}
	}
	printf("Done.                                          \n");
}


void doFlood(U32 destip, int port, FloodMode mode) {
	switch (mode) {
		case FLOOD_TCPCONNECT:
			connectToTcp(destip, port);
			break;
		case FLOOD_TCPLARGE:
			*(int*)(data)=1200*8;
			sendToTCP(destip, port, data, 1200);
			break;
		case FLOOD_TCPSMALL:
			data[0]=8*8; // Tell it there's an 8-byte packet coming
			data[1]=data[2]=data[3]=0;
			sendToTCP(destip, port, data, 4); // only send 4 bytes
			break;
		case FLOOD_TCPGARBAGE:
			{
				int i;
				for (i=0; i<sizeof(data); i++) 
					data[i] = randInt2(256);
				sendToTCP(destip, port, data, sizeof(data));
			}
			break;
		case FLOOD_TCPCHECKSUMMED:
			data[0]=16*8; // Tell it there's an 16-byte packet coming
			data[1]=data[2]=data[3]=0;
			// Fake the checksum
			*(int*)&data[4] = 2130751490;
			sendToTCP(destip, port, data, 20); // blowfish pads it to 20
			break;
		case FLOOD_TCPSTREAM:
			// Doesn't actually work, as our networking code will drop you on the first packet!
			sendToTCPStream(destip, port, data, ARRAY_SIZE(data), -1);
			break;
		case FLOOD_UDPLARGE:
			*(int*)(data)=1200*8;
			sendToUDP(destip, port, data, 1200);
			break;
		case FLOOD_UDPSMALL:
			data[0]=8*8; // Tell it there's an 8-byte packet coming
			data[1]=data[2]=data[3]=0;
			sendToUDP(destip, port, data, 4); // only send 4 bytes
			break;
		case FLOOD_UDPGARBAGE:
			{
				int i;
				for (i=0; i<sizeof(data); i++) 
					data[i] = randInt2(256);
				sendToUDP(destip, port, data, sizeof(data));
			}
			break;
		case FLOOD_UDPCHECKSUMMED:
			data[0]=16*8; // Tell it there's an 16-byte packet coming
			data[1]=data[2]=data[3]=0;
			// Fake the checksum
			*(int*)&data[4] = 1157632001;
			sendToUDP(destip, port, data, 16);
			break;
		case FLOOD_UDPSTREAM:
			sendToUDPStream(destip, port, data, ARRAY_SIZE(data), -1);
			break;
	}
}

void packetFlood(char *dest, int port, FloodMode mode, int num)
{
	int count;
	U32 destip;
	bool pause=false;
	sockStart();

	if (mode >= 100) {
		mode -=100;
		pause=true;
	}

	destip = ipFromString(dest);

	printf("Flooding %s (%s):%d\n", dest, makeIpStr(destip), port);
	for (count=1; !num || count<=num; count++) {
		doFlood(destip, port, mode);
		printf("\rCount: %d", count);
	}

	 if (pause) {
		printf("Press any key to exit...\n");
		_getch();
	}
}