#include <stdio.h>
#include <stdlib.h>
#include "coh_net.h"
#include <conio.h>
#include "windows.h"

void main(int argc,char **argv)
{
	char	buf[10000],*s,*server = "localhost";

	if (argc < 3)
	{
		printf("Usage: chatclient <username> <password>\n");
		exit(0);
	}
	sockStart();
	for(;;)
	{
		while(!cohConnect(server))
		{
			printf("connecting to %s..\n",server);
			Sleep(1000);
		}
		if (!cohLogin(argv[1],argv[2]))
			break;
		for(;;)
		{
			s = cohGetMsg();
			if (s)
				printf("%s\n",s);
			Sleep(1);
			if (_kbhit())
			{
				gets(buf);
				cohSendMsg(buf);
			}
			if (!cohConnected())
			{
				printf("lost connection.\n");
				break;
			}
		}
	}
}
