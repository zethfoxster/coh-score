#include "shardcommtest.h"
#include "shardcomm.h"
#include "earray.h"
#include "stdtypes.h"
#include "utils.h"
#include "assert.h"
#include "error.h"
#include "timing.h"
#include "comm_backend.h"
#include "svr_base.h"
#include "comm_game.h"
#include "timing.h"
#include "entity.h"

// from shardcomm.c
int checkSpecialCommand(Entity *e,char *str,U32 repeat);
Entity *entFromAuthId(int auth_id);

int gChatServerTestClients = 0;

static bool gInitialLogin = true;
static bool gPrint = false;
static int gMinUser;
static int gMaxUser;
static int *gChannels = 0;
static int gOnlineCount = 0;

#define MAX_USERS		(400000)
 
#define MIN_CHANNELS	(3) // 3
#define MIN_FRIENDS		(4) 
#define MAX_CHANNELS	(20000) // 30000
#define LOGINS_PER_SEC  (10000)		// calibrated for for 50,000 players
#define MAX_ONLINE_PLAYERS (50000) //(50000)

#define	SEC_PER_MSG		(5)		// how often the average user sends a message
#define MSGS_PER_SEC	(1. / SEC_PER_MSG)	


void setShardCommTestParams(int clients, int startIdx)
{
	gChatServerTestClients = clients;
	gMinUser = startIdx;
	gMaxUser = clients + startIdx - 1;
}


typedef enum 
{
	CHAT_HANDLE = 1,
	ONLINE_USER,
	CURRENT_FRIEND,
	NON_FRIEND,
	MY_HANDLE,
	AUTH_ID,
	DB_ID,
	CURRENT_CHANNEL,
	EXISTING_CHANNEL,
	FREE_CHANNEL,
	MESSAGE,
	FRIENDSTATUS,
	USER_MODE,
	ACCESSLEVEL,
	SILENCE_TIME,
	NULL_STRING,
} TestParam;

typedef struct ChatTestClient
{
	int * watching;
	int * friends;
	bool online;
	bool waitingForLogin;
} ChatTestClient;


static ChatTestClient* gTestClients;

typedef struct{
	char	*cmdname;
	int		weight;
	TestParam params[10];
	int		num_params;
	int 	rouletteWeight;		
} TestCmd;


static TestCmd cmds[] = 
{
//	{ "Login",				1,			{MY_HANDLE, AUTH_ID, DB_ID}},
	{ "Logout",				20,			{MY_HANDLE}},

//	{ "SendUser",			40,		{CURRENT_FRIEND, MESSAGE} }, 
	{ "SendUser",			300,		{CURRENT_FRIEND, MESSAGE} }, 

//	{ "Name",				1, 			{CHAT_HANDLE} }, 
//	{ "Save",				1,			}, 

//	{ "CsrName",			1, 			{CHAT_HANDLE,	CHAT_HANDLE} }, 
//	{ "CsrSilence",			1,			{CHAT_HANDLE,	SILENCE_TIME} }, 
//	{ "CsrChanKill",		1,			{CHANNEL} }, 
//	{ "CsrSilenceAll",		1,			}, 
//	{ "CsrUnsilenceAll",	1,			}, 
//	{ "CsrSendAll",			2,			{MESSAGE} }, 

	{ "Friend",				20,			{NON_FRIEND} }, 
	{ "UnFriend",			5,			{CURRENT_FRIEND} }, 
	{ "Friends",			5,			}, 
	{ "Status",				50,			{FRIENDSTATUS} }, 
	{ "Invisible",			1,			}, 
	{ "Visible",			1,			}, 

	{ "Ignore",				1,			{CURRENT_FRIEND} }, 
	{ "Unignore",			1,			{CURRENT_FRIEND} }, 
	{ "Ignoring",			1,			}, 

	{ "Join",				30,			{EXISTING_CHANNEL, NULL_STRING} }, 
	{ "Join",				10,			{FREE_CHANNEL, NULL_STRING} },		// might not know about some channels that actually exist...
	{ "Create",				1,			{FREE_CHANNEL, NULL_STRING} },
	{ "Leave",				40,			{CURRENT_CHANNEL} }, 
	{ "ReserveTest",		10000,			},		// might not know about some channels that actually exist...

	{ "Send",				1200,		{CURRENT_CHANNEL, MESSAGE} }, 
//	{ "Send",				50,		{CURRENT_CHANNEL, MESSAGE} }, 


	{ "UserMode",			1,			{CURRENT_CHANNEL, CHAT_HANDLE, USER_MODE} }, 
	{ "ChanMode",			1,			{CURRENT_CHANNEL, USER_MODE} }, 
	{ "Invite",				1,		{CURRENT_CHANNEL, CURRENT_FRIEND} }, 
	{ "Watching",			1,			}, 
	{ "ChanList",			1,			{CURRENT_CHANNEL} }, 
	{ "ChanMotd",			1,			{CURRENT_CHANNEL, MESSAGE} }, 
//	{ "ChanFind",			1,			{CHANNEL} } 
};

TestCmd * GetCommand(const char * name)
{
	int k;
     	for(k=0;k<ARRAY_SIZE(cmds);k++)
	{
		if(!stricmp(cmds[k].cmdname, name))
			return &cmds[k];
	}

	assert(!"Bad command name");
	return 0;
}

static char * gUserModes[] = 
{
		"+join",
		"-join",
		"+send",
		"-send",
		"+operator",
		"-operator",
		// try to break chat server
		"BAD MODE1",	
		"-BADMODE2",
		"-REALLY REALLY EXTREMELY NASTY AND BAD MODE3",
		""
};



int GetAuthID(int i)
{
  	return i + 10000;
}

int AuthID2Name(int i)
{
	if(i >= 10000)
		return i - 10000;
	else
		return -1;
}

char * makeStatusString(char * buf, int userID)
{
 	char* maps[] = {"Atlas Park", "Kings' Row", "Shadow Shard", "Rikti Crash Site", "Really Sorta Long Zone Name"};
	char* archetypes[] = {"Blaster", "Controller", "Defender", "Scrapper", "Tanker"};
	char* origins[] = {"Natural", "Magic", "Science", "Technology", "Mutant"};

	sprintf(buf, "\"player%d\" %d \"%s\" \"%s\" \"%s\" %d %d", 
		userID,
		GetAuthID(userID),
		maps[userID % ARRAY_SIZE(maps)],
		archetypes[userID % ARRAY_SIZE(archetypes)],
		origins[userID % ARRAY_SIZE(origins)],
		(rand() % 7) + 1,	// team size
		userID % 50		// level
		);

	return buf;

}

void Login(int userID)
{
	char name[100];
	char buf[2000];
	char buf2[1000];

 	shardCommSendInternal(GetAuthID(userID), "LinkEnt");
	sprintf(name, "username%d", userID);
 	sprintf(buf,"Login \"%s\" \"%s\" %d %d %i", escapeString(name), "0123456789012345", 0, userID, 0); // no access for now, generic password md5 hash
	shardCommSendInternal(GetAuthID(userID), buf);
	sprintf(buf, "Status \"%s\"", escapeString(makeStatusString(buf2, userID)));
	shardCommSendInternal(GetAuthID(userID), buf);
	gTestClients[userID].waitingForLogin = timerSecondsSince2000();
	gOnlineCount++;
}


void Logout(int userID)
{
	shardCommSendInternal(GetAuthID(userID), "UnlinkEnt");
	shardCommSendInternal(GetAuthID(userID), "Logout");

	gTestClients[userID].online = false;
	gTestClients[userID].waitingForLogin = 0;
	gOnlineCount--;
	assert(gOnlineCount >= 0);
}

// test the "reserve" channel feature, used to allow channel options menu to queue up channel join/create commands
void ReserveTest(int userID)
{
	char chanName[100];
	char cmd[100];
	char buf[1000];

	sprintf(chanName, "rsvChan%d", (rand() % 300));
	
	if(rand() % 2)	// 50% chance
		strcpy(cmd, "create");
	else
		strcpy(cmd, "join");

	sprintf(buf, "%s %s reserve", cmd, chanName);
	shardCommSendInternal(GetAuthID(userID), buf);
	

 	if(rand() % 4) // 75% chance
		sprintf(buf, "leave %s", chanName);
	else
 		sprintf(buf, "join %s 0", chanName);

	shardCommSendInternal(GetAuthID(userID), buf);
}

void shardCommTestInit()
{
	int i,k;
	int sum = 0;

	printf("\n(Chat Server Test Client Mode)\n");

	// init commands
	for(i=0;i<ARRAY_SIZE(cmds);i++)
	{
		// params
		for(k=0;cmds[i].params[k];k++)
			cmds[i].num_params++;

		// weights
		sum += cmds[i].weight;
		cmds[i].rouletteWeight = sum;
	}

	gTestClients = calloc(sizeof(ChatTestClient), MAX_USERS);

	eaiCreate(&gChannels);
	eaiSetSize(&gChannels, MAX_CHANNELS);	// no than 1:1 channel to user ratio
	memset(gChannels, 0, sizeof(int) * eaiSize(&gChannels));

	for(i=0;i<MAX_USERS;i++)
	{
		eaiCreate(&gTestClients->watching);
		eaiCreate(&gTestClients->friends);
	}
}


static enum ChannelSize_Type
{
	Small = 0,	// 5-10 users
	Medium, // 40-60 users
	Large,	// 
};

// for now, split evenly between small, medium & large
int chooseChatSize()
{
	int r = rand() % 3;
	return r;
}

int availableChannelCount()
{
 	return min( (MAX_USERS / 10), eaiSize(&gChannels));
}


int getFreeChannel()
{
	int size = availableChannelCount();
	int r = rand() % size;
	int i = r;
	while(gChannels[i] != 0)
	{
		i++;

		if(i >= size)
			i = 0;

		if(i == r)
			return 0; // wraparound (failure)
	}

	return i;
}


int getExistingChannel()
{
	int size = availableChannelCount();
	int r = rand() % size;
	int i = r;
	while(gChannels[i] == 0)
	 {
		 i++;

		 if(i >= size)
			 i = 0;		 
		 
		 if(i == r)
			 return 0; // wraparound (failure)
	 }

	return i;
}


void shardCommTestReconnect()
{
	int i;

	if(gTestClients)
	{
		for(i=gMinUser;i<=gMaxUser;i++)
		{
			gTestClients[i].online = 0;
			gTestClients[i].waitingForLogin = 0;
			eaiSetSize(&gTestClients->watching, 0);
			eaiSetSize(&gTestClients->friends, 0);
		}
	}

	gInitialLogin = true;
	gPrint = false;
	gOnlineCount = 0;
}


bool LoginUsers(float time)
{
	int i;

	// see if clients have been waiting too long for login command
	int expireTime = timerSecondsSince2000() - (1*60);
	for(i=gMinUser;i<=gMaxUser;i++)
	{
		if(gTestClients[i].waitingForLogin 
			&& expireTime > gTestClients[i].waitingForLogin)
		{
			gTestClients[i].waitingForLogin = 0;
		}
	}

	if(gInitialLogin || gOnlineCount < gChatServerTestClients)
	{
		int count=0;
		int i;
		static int next = -1;

		if(gInitialLogin)
		{
			for(i=gMinUser;i<=gMaxUser;i++)
			{
				if(!gTestClients[i].online && !gTestClients[i].waitingForLogin)
					count++;
			}
			count = min(count, gChatServerTestClients);
		}
		else
			count = gChatServerTestClients - gOnlineCount;

		count = min(count, ((time * LOGINS_PER_SEC * gChatServerTestClients / MAX_ONLINE_PLAYERS) + 0.5f));

		if(!gPrint)
		{
			loadstart_printf("Logging in %d clients (%d - %d)...", gChatServerTestClients, gMinUser, gMaxUser);
			gPrint = true;
		}

		if(next == -1)
			next = gMinUser; // init

		for (i=0;i<count;i++)
		{
			// login a random offline user
			while(gTestClients[next].online || gTestClients[next].waitingForLogin)
			{
				next++;
				if(next > gMaxUser)
					next = gMinUser;
			}
			Login(next);
		}

		// are we done w/ initial login?
		if(gInitialLogin)
		{
			count = 0;
			for(i=gMinUser;i<=gMaxUser;i++)
			{
				if(gTestClients[i].online)
					count++;
			}
			if(	   count >= gChatServerTestClients)
				//				|| gOnlineCount >= gChatServerTestClients * 1.1)
			{
				loadend_printf("done");
				gInitialLogin = false;
			}
			else
			{
				return true;
			}
		}
	}

	return false;
}
	
void shardCommTestTick()
{
	static bool sInit = false;
	int n,k,m;
	char str[2000];

	static int sTimer;
	static float msgCount = 0;
	int count;
	F32 time;
	TestCmd * pCmd;

    if(!sInit)
	{
		shardCommTestInit();
		sTimer = timerAlloc();
		sInit = true;
		return;
	}

	time = timerElapsed(sTimer);
	timerStart(sTimer);

	// login more users if below set number
	if(LoginUsers(time))
		return;


    msgCount += (time * MSGS_PER_SEC * gChatServerTestClients); // round
	count = (int)msgCount;
	msgCount -= count;	// keep remainder


   	for(n=0;n<count;n++)
	{
		bool skip = false;

		// choose random command
		static int userID = -1;

		int size = chooseChatSize();

 		do {
			userID++;
 			if(userID > gMaxUser)
				userID = gMinUser;

		} while(! gTestClients[userID].online);

 		if(eaiSize(&gTestClients[userID].watching) < MIN_CHANNELS)
		{
			if(rand() % 10)
				pCmd = GetCommand("Join");
			else
				pCmd = GetCommand("Create");
		}
		else
		{
			// random command
	
			int r = rand() % cmds[ ARRAY_SIZE(cmds)-1 ].rouletteWeight;
			for(k=0;k<ARRAY_SIZE(cmds);k++)
			{
				if(r < cmds[k].rouletteWeight)
					break;
			}
			assert(k<ARRAY_SIZE(cmds));

			pCmd = &cmds[k];

			if(gTestClients[userID].online &&  ! stricmp(pCmd->cmdname, "Login"))
				continue;
		}

		strcpy(str, pCmd->cmdname);


		if( ! stricmp(pCmd->cmdname, "Logout"))
		{
			if(gTestClients[userID].online)
				Logout(userID);
			continue;
		}
		else if( ! stricmp(pCmd->cmdname, "ReserveTest"))
		{
			if(gTestClients[userID].online)
				ReserveTest(userID);
			continue;
		}
 
		// choose parameters

		for(m=0;m<pCmd->num_params;m++)
		{
			char buf[2000];

  			switch(pCmd->params[m])
			{
				case CHAT_HANDLE:
				{	
					// pick user names relative to each name, this helps simulate groups of people. 
					int r = rand();
					int num = max( (userID - 5 + r % 10), gMinUser);
					num = min(num, gChatServerTestClients-1);
					if(num >= userID) // don't choose yourself (shift everything up
						num++;
					assert(num >= 0);
					sprintf(buf, "username%d", num); 
				}
				xcase ONLINE_USER:
				{
					int r;
					do {
						r = gMinUser + (rand() % gChatServerTestClients);
					} while(!gTestClients[r].online);
					sprintf(buf, "username%d", r); 
				}
				xcase MY_HANDLE:
				{
					sprintf(buf, "username%d", userID); 
				}
				xcase CURRENT_FRIEND:
				{
					int id=0, size = eaiSize(&gTestClients[userID].friends);
					if(size)
						id = gTestClients[userID].friends[ rand() % size ];
					sprintf(buf, "username%d", id); 
				}
				xcase NON_FRIEND:
				{
					int i, size = eaiSize(&gTestClients[userID].friends);
					int id = rand() % MAX_USERS;
					if(size)
					{
						
						for(i=0;i<MAX_USERS;i++,id++)
						{
							if(id >= MAX_USERS)
								id= 0;
							if(-1 == eaiFind(&gTestClients[userID].friends, (void*) id))
								break;
						}
					}
					sprintf(buf, "username%d", id); 
				}
				xcase AUTH_ID:
				{
					sprintf(buf, "%d", GetAuthID(userID)); 
				}
				xcase DB_ID:
				{
					sprintf(buf, "%d", userID); 
				}
				xcase FREE_CHANNEL:
				{
					sprintf(buf, "channel%d", getFreeChannel());
				}
				xcase EXISTING_CHANNEL:
				{
					sprintf(buf, "channel%d", getExistingChannel());
				}
				xcase CURRENT_CHANNEL:
				{
					int size = eaiSize(&gTestClients[userID].watching);
					if(size)
					{
						int num = gTestClients[userID].watching[rand() % size];
						sprintf(buf, "channel%d", num);
					}
					else
						skip = true; // skip this message

				}
				xcase MESSAGE:
				{
					// random message
					int i, size=(4 + rand() % 30);
					for(i=0;i<size;i++)
					{
						buf[i] = (rand() % 26) + 'a';
					}
					buf[i] = 0;
				//	sprintf(buf, "The current time is %s", timerGetTimeString());
				}
				xcase FRIENDSTATUS:
				{
					char buf2[1000];
 					sprintf(buf, "%s", makeStatusString(buf2, userID));
				}
				xcase USER_MODE:
				{
					sprintf(buf, "%s", gUserModes[ rand () % ARRAY_SIZE(gUserModes)]);
				}
				xcase ACCESSLEVEL:
				{
					sprintf(buf, "%d", rand() % 10);
				}
				xcase SILENCE_TIME:
				{
					sprintf(buf, "%d", 1 + (rand() % 4));	// right now, 1-5 minutes
				}
				xcase NULL_STRING:
				{
					strcpy(buf, "0");
				}
				break;
				default:
					assert(!"Bad type");
			}

			strcatf(str, " \"%s\"", escapeString(buf));
 		}

 		if(!skip)
		{
			// do it --- skip commands unless they're online or attempting to login
			shardCommSendInternal(GetAuthID(userID), str);	
		}
	}
}

// used to extract number from "userXX" & "channelXX"
int string2ID(const char * str, const char * prefix)
{
	const char * p;
	char buf[2000];
 	strcpy(buf, str);
	p = strstri(buf, prefix);
	if(p)
	{
		p += strlen(prefix);
		return atoi(p);
	}
	else
	{
		return -1;
	}
}



int shardTestMessageCallback(Packet *pak,int cmd,NetLink *link)
{
	if(cmd != LOGSERVER_SHARDCHAT)
		return 0;
 	while(!pktEnd(pak))
	{
		U32 auth_id = pktGetBits(pak,32);
		U32 repeat = pktGetBits(pak, 1);
		char * str = pktGetString(pak);

		if(gChatServerTestClients < 5)
			printf("RECV (%d)\t%s\n", auth_id, str);

		checkSpecialCommand(0,str,repeat);

		if (auth_id >= 10000)
		{
			char	*args[100],buf[1000];
			int		i,count;
			int id = AuthID2Name(auth_id);

 			strncpy(buf,str,sizeof(buf)-1);
			count = tokenize_line(buf,args,0);
			if (!count)
				return 1;
			for(i=0;i<count;i++)
				strcpy(args[i],unescapeString(args[i]));
			if (stricmp(args[0],"Join")==0 && args[2])
			{
				int channelID = string2ID(args[1], "channel");
				int userID = string2ID(args[2], "username");
				if(userID >= 0 && userID <= MAX_USERS)
				{
					if(-1 == eaiFind(&gTestClients[userID].watching, (void*) channelID))
					{
						eaiPush(&gTestClients[userID].watching, (void*) channelID);
						gChannels[channelID]++;
					}
				}
			}
			else if (stricmp(args[0],"Leave")==0 && args[2])
			{
				int channelID = string2ID(args[1], "channel");
				int userID = string2ID(args[2], "username");
				int i;
				if(userID >= 0 && userID <= MAX_USERS)
				{
					if(-1 != (i = eaiFind(&gTestClients[userID].watching, (void*) channelID)))
					{
						eaiRemove(&gTestClients[userID].watching, i);
						gChannels[channelID]--;
						assert(gChannels[channelID] >= 0);
					}
				}
			}
 			else if (stricmp(args[0],"Login")==0  && args[2])
			{
				int userID = string2ID(args[2], "username");
				if(userID >= 0 && userID <= MAX_USERS)
				{	
					gTestClients[userID].online = true;
					gTestClients[userID].waitingForLogin = 0;
				}
			}
 			else if (stricmp(args[0],"FriendReq")==0  && args[1])
			{
				// auto-accept
				char buf[1000];
  				sprintf(buf, "Friend \"%s\"", args[1]);
				shardCommSendInternal(auth_id, buf);
			}
			else if (stricmp(args[0],"Friend")==0 && args[1])
			{
				int userID = AuthID2Name(auth_id);
				int friendID = string2ID(args[1], "username");
				if(friendID >= 0 && friendID <= MAX_USERS)
				{
					if(-1 == eaiFind(&gTestClients[userID].friends, (void*) friendID))
					{
						eaiPush(&gTestClients[userID].friends, (void*) friendID);
					}
				}
			}
			else if (stricmp(args[0],"Unfriend")==0 && args[1])
			{
				int userID = AuthID2Name(auth_id);
				int i, friendID = string2ID(args[1], "username");
				if(friendID >= 0 && friendID <= MAX_USERS)
				{
					if(-1 != (i = eaiFind(&gTestClients[userID].friends, (void*) friendID)))
					{
						eaiRemove(&gTestClients[userID].watching, i);
					}
				}
			}
			else if (stricmp(args[0],"Invite")==0 && args[1])
			{
				// auto-accept
				char buf[1000];
				sprintf(buf, "join \"%s\" 0", args[1]);
				shardCommSendInternal(auth_id, buf);
			}
		}
		else
		{
			// route normally
			Entity * e = entFromAuthId(auth_id);
			if (e)
			{
				checkSpecialCommand(e,str,repeat);
				START_PACKET(pak, e, SERVER_SHARDCOMM);
				pktSendString(pak,str);
				END_PACKET
			}
		}
	}
	return 1;
}


