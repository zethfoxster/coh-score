#include "builderizercomm.h"


extern NetLink * buildServerLink;

int lostServerLink()
{
	if ( linkDisconnected(buildServerLink) )
		printf( "Lost connection to server...\n" );
	return linkDisconnected(buildServerLink);
}

static Packet *next_pak = 0;
static int next_cmd = 0;

void handleNetMsg(Packet *pak,int cmd,NetLink* link,void *user_data)
{
	assert(!next_pak);
	assert(!next_cmd);

	next_pak = pktDup(pak, link);
	next_cmd = cmd;
}

int waitForServerMsg(Packet **pak_p)
{
	int cmd;

	*pak_p = 0;

	while (!next_pak)
	{
		if (lostServerLink())
			return BUILDCOMM_DISCONNECTED;
		linkWaitForPacket(buildServerLink,0, 1.0f);
	}

	cmd = next_cmd;
	*pak_p = next_pak;

	next_pak = 0;
	next_cmd = 0;

	return cmd;
}