#include "builderizer.h"
#include "builderizerclient.h"
#include "utils.h"
#include "timing.h"
#include "builderizercomm.h"
#include "builderizerconfig.h"
#include "buildstatus.h"
#include "windows.h"
#include <process.h>

NetComm * net_comm;
NetListen * net_listen;
BuildStatus lastBuildStatus = {0};
HANDLE buildThread = 0;
int sentLastStatus = 0;
int buildPaused = 0;

typedef struct
{
	NetLink *link;
}BuildClientLink;

int buildClientCreateCb(NetLink* link,void *user_data)
{
	BuildClientLink	*client;

	client = user_data;
	client->link = link;
	linkSendRecvSize(link, 64*1024, 64*1024); // Max send buffer size of 64K/link
	linkSetTimeout(link, 60*60);
	return 1;
}

int buildClientDeleteCb(NetLink* link,void *user_data)
{
	BuildClientLink	*client = user_data;
	return 1;
}

void buildserverSendConfigs(NetLink *link)
{
	int i;
	Packet *pak = pktCreate(link, BUILDCOMM_SENDCONFIGS);
	// send whether or not a build is currently in progress
	pktSendBitsPack(pak, 1, buildThread == 0 ? 0 : 1);
	pktSendBitsPack(pak, 1, eaSize(&loadedConfigs));
	for ( i = 0; i < eaSize(&loadedConfigs); ++i )
	{
		ParserSend(tpiBuildConfig, pak, NULL, loadedConfigs[i], 1, 0, 0, 0, NULL);
	}
	pktSend(&pak);
	linkFlush(link);
}

void buildserverSendStatus(NetLink *link)
{
	Packet *pak = pktCreate(link, BUILDCOMM_SENDSTATUS);

	buildStatusGetStatus(&lastBuildStatus);

	// send whether or not a build is currently in progress
	pktSendBitsPack(pak, 1, buildThread == 0 && sentLastStatus ? 0 : 1);
	ParserSend(tpiBuildStatus, pak, NULL, &lastBuildStatus, 1, 0, 0, 0, NULL);
	pktSend(&pak);
	linkFlush(link);
	if (!sentLastStatus)
		sentLastStatus = 1;
}

int buildserverSetVariablesFromConfig(BuilderizerConfig *bcfg)
{
	clearAllBuildStateLists();
	if ( bcfg->vars )
	{
		int i, j;
		for ( i = 0; i < eaSize(&bcfg->vars); ++i )
		{
			if ( !bcfg->vars[i]->value && !bcfg->vars[i]->notRequired)
				return 0;
			if ( bcfg->vars[i]->isActuallyList )
			{
				if ( bcfg->vars[i]->values )
				{
					if ( isBuildStateList(bcfg->vars[i]->name) )
						clearBuildStateList(bcfg->vars[i]->name);
					for ( j = 0; j < eaSize(&bcfg->vars[i]->values); ++j )
					{
						if ( !bcfg->vars[i]->values[j] )
							return 0;
						addToBuildStateList(bcfg->vars[i]->name, bcfg->vars[i]->values[j]);
					}
				}
				else if ( !bcfg->vars[i]->notRequired )
					return 0;
			}
			else
				setBuildStateVar(bcfg->vars[i]->name, bcfg->vars[i]->value);
		}
		//for ( i = 0; i < eaSize(&bcfg->lists); ++i )
		//{
		//	if ( bcfg->lists[i]->values )
		//	{
		//		if ( isBuildStateList(bcfg->lists[i]->name) )
		//			clearBuildStateList(bcfg->lists[i]->name);
		//		for ( j = 0; j < eaSize(&bcfg->lists[i]->values); ++j )
		//		{
		//			if ( !bcfg->lists[i]->values[j] )
		//				return 0;
		//			addToBuildStateList(bcfg->lists[i]->name, bcfg->lists[i]->values[j]);
		//		}
		//	}
		//}
	}
	return 1;
}

void buildserverStartBuild(NetLink *link, Packet *pak)
{
	if ( !buildThread )
	{
		unsigned int addr;
		BuilderizerConfig *bcfg = (BuilderizerConfig*)malloc(sizeof(BuilderizerConfig));
		ZeroStruct(bcfg);
		ParserRecv(tpiBuildConfig, pak, bcfg);
		//readScriptFiles(bcfg->script);
		if ( buildserverSetVariablesFromConfig(bcfg) )
		{
			// clear out old status
			buildStatusFreeAll();
			sentLastStatus = 0;
			buildThread = (HANDLE)_beginthreadex(NULL, 0, runBuildScriptThread, bcfg->script, 0, &addr);
			if ( buildThread == (HANDLE)1L )
			{
				// TODO: handle error
				printf("There was an error when creating the build thread...\n");
				buildThread = 0;
			}
			free(bcfg);
		}
		else
			printf("There was an error setting the build variables...\n");
	}
	else
	{
		// TODO: handle build already in progress
		printf("There is already a build in progress...\n");
	}
}

void buildserverStopBuild(NetLink *link)
{
	if ( buildThread && !buildPaused )
	{
		// just error out the build, so that we can do a restart build if we want
		incErrorLevel();
	}
}

void buildserverPauseBuild(NetLink *link)
{
	if ( buildThread && !buildPaused )
	{
		while ( SuspendThread(buildThread) != -1 );

		buildPaused = 1;
	}
}

void buildserverResumeBuild(NetLink *link)
{
	if ( buildThread && buildPaused )
	{
		int ret;
		
		while ( (ret = ResumeThread(buildThread)) != 0 );

		buildPaused = 0;
	}
}

void buildserverRestartBuild(NetLink *link)
{
	if ( !buildThread )
	{
		unsigned int addr;
		// clear out old status
		buildStatusFreeAll();
		buildThread = (HANDLE)_beginthreadex(NULL, 0, resumeBuildScriptThread, NULL, 0, &addr);
		if ( buildThread == (HANDLE)1L )
		{
			// TODO: handle error
			printf("There was an error when creating the build thread...\n");
			buildThread = 0;
		}
	}
	else
	{
		// TODO: handle build already in progress
		printf("There is already a build in progress...\n");
	}
}

void buildserverRunStep(NetLink *link, Packet *pak)
{
	if ( !buildThread )
	{
		unsigned int addr;
		BuildStep *step = (BuildStep*)malloc(sizeof(BuildStep));
		ZeroStruct(step);
		ParserRecv(tpiBuildStep, pak, step);
		//buildserverSetVariablesFromConfig(bcfg);
		// clear out old status
		buildStatusFreeAll();
		buildThread = (HANDLE)_beginthreadex(NULL, 0, runBuildStepThread, step, 0, &addr);
		if ( buildThread == (HANDLE)1L )
		{
			// TODO: handle error
			printf("There was an error when creating the build thread...\n");
			buildThread = 0;
		}
		//free(step);
	}
	else
	{
		// TODO: handle build already in progress
		printf("There is already a build in progress...\n");
	}
}

int buildserverHandleClientMsg(Packet *pak,int cmd,NetLink* link,void *user_data)
{
	BuildClientLink	*client = user_data;

	switch(cmd)
	{
		xcase BUILDCOMM_REQUESTCONFIGS:
			buildserverSendConfigs(link);
		xcase BUILDCOMM_REQUESTSTATUS:
			buildserverSendStatus(link);
		xcase BUILDCOMM_STARTBUILD:
			buildserverStartBuild(link, pak);
		xcase BUILDCOMM_STOPBUILD:
			buildserverStopBuild(link);
		xcase BUILDCOMM_PAUSEBUILD:
			buildserverPauseBuild(link);
		xcase BUILDCOMM_RESUMEBUILD:
			buildserverResumeBuild(link);
		xcase BUILDCOMM_RESTARTBUILD:
			buildserverRestartBuild(link);
		xcase BUILDCOMM_RUNSTEP:
			buildserverRunStep(link, pak);
			break;
		default:
			printf("Unknown command from client: %d\n",cmd);
			return 0;
	}
	return 1;
}

void buildserverInit(void)
{
	net_comm = commCreate(100, 1);
	net_listen = commListen(net_comm, BUILDERIZER_FLAGS, BUILDERIZER_PORT, buildserverHandleClientMsg, buildClientCreateCb, buildClientDeleteCb, sizeof(BuildClientLink));
}

void handleBuildThread()
{
	// see if the thread is finished
	if ( buildThread && WaitForSingleObject(buildThread, 10) == WAIT_OBJECT_0 )
	{
		CloseHandle(buildThread);
		buildThread = 0;
	}
}

void buildserverRun(void)
{
	S64 timer = 0;
	for(;;)
	{
		timer = timerCpuSeconds();
		commMonitor(net_comm);

		// check on the build thread to see if its finished
		handleBuildThread();
	}
}