#include "serverAutoStart.h"
#include "loadBalancing.h"
#include "earray.h"
#include "container.h"
#include "comm_backend.h"
#include "timing.h"
#include "utils.h"
#include "launchercomm.h"
#include "servercfg.h"
#include "dbinit.h"
#include "log.h"

RunServer **eaRunServers;

static char *getShortName(const char *command)
{
	static char buf[128];
	char *s;
	while (strchr("./\\", command[0]))
		command++;
	Strncpyt(buf, command);
	s = strchr(buf, '.');
	if (s) {
		*s = 0;
	}
	return buf;
}

bool assembleAutoStartList(void)
{
	int i, j;
	bool ret=true;
	eaSetSize(&eaRunServers, 0);
	for (i=0; i<eaSize(&load_balance_config.serverRoles); i++)
	{
		ServerRoleDescription *role = load_balance_config.serverRoles[i];
		for (j=0; j<eaSize(&role->runServers); j++) 
		{
			RunServer* rs = role->runServers[j];
			rs->ip = role->iprange[0];
			Strncpyt(rs->appname, rs->appname_cfg ? rs->appname_cfg : getShortName(rs->command));
			eaPush(&eaRunServers, rs);
		}
	}

	// Verification
	{
		// These names should be for the regular, 32-bit versions. All our
		// 64-bit versions are the same name with '64' appended so we'll check
		// that variant below.
		char *required_servers[] = {"LogServer", "StatServer", "ArenaServer", "RaidServer"};
		int found[ARRAY_SIZE(required_servers)]={0};
		int badcount;
		if (!server_cfg.log_server[0]) {
			required_servers[0]="";
			found[0]=1;
		}
		for (i=0; i<eaSize(&eaRunServers); i++) {
			for (j=0; j<ARRAY_SIZE(required_servers); j++) {
				char required_name[500];

				// Don't use the whole buffer because we need room to append
				strncpy_s(required_name, 490, required_servers[j], strlen(required_servers[j]));

				if (stricmp(required_name, eaRunServers[i]->appname)==0) {
					found[j] = 1;
				} else {
					// No first match so try the 64-bit version instead
					strcat_s(required_name, 500, "64");

					if (stricmp(required_name, eaRunServers[i]->appname) == 0) {
						found[j] = 1;
					}
				}
			}
		}
		badcount=0;
		for (j=0; j<ARRAY_SIZE(required_servers); j++) {
			if (!found[j]) {
				if (!badcount)
					printf("Warning: No launcher specified to start the following server applications:\n     ");
				else
					printf(", ");
				printf("%s", required_servers[j]);
				badcount++;
				ret = false;
			}
		}
		if (badcount)
		{
			printf("\n");
			//printf("\n   Please edit server/db/loadBalanceShardSpecific.cfg and fix this.\n");
		}
	}
	return ret;
}


void serverAppStatusCb(ServerAppCon *container,char *buf)
{
	RemoteProcessInfo	*rpi = &container->remote_process_info;

	sprintf(buf,"%d %s Ip:%-13s Mem:%4dM Load:%.2f %.2f",
		container->id, container->appname, makeHostNameStr(container->ip),
		rpi->mem_used_phys / 1024,rpi->cpu_usage,rpi->cpu_usage60);
}

bool serverAutoStartInit(void)
{
	int i;
	bool ret = true;
	assert(serverapp_list);

	// Verify each server has it's launcher connected
	for (i=0; i<eaSize(&eaRunServers); i++)
	{
		RunServer *run_server = eaRunServers[i];
		// Find a launcher for this server
		if (launcherFindByIp(run_server->ip)==-1)
		{
			ret = false;
			LOG(LOG_ERROR, LOG_LEVEL_IMPORTANT, LOG_CONSOLE | LOG_ONCE, "You MUST edit loadBalanceShardSpecific.cfg or start the appropriate launcher.\n")
			LOG(LOG_ERROR, LOG_LEVEL_IMPORTANT, LOG_CONSOLE, "LoadBalanceConfig error: Server application \"%s\" is set to run on %s, but no launcher is connected from that IP.\n", run_server->appname, makeIpStr(run_server->ip));
		}
	}
	
	// Verify that the Master-BeaconServer has a launcher.
	
	if(	server_cfg.master_beacon_server[0] &&
		!server_cfg.do_not_launch_master_beacon_server)
	{
		U32 ip = ipFromString(server_cfg.master_beacon_server);
		
		if(ip == ~0)
		{
			ret = false;
			
			LOG(LOG_ERROR, LOG_LEVEL_IMPORTANT, LOG_CONSOLE, "ServerCfg error: Can't resolve name of Master-BeaconServer: %s\n", server_cfg.master_beacon_server);
		}
		else if(launcherFindByIp(ip) == -1)
		{
			ret = false;
			
			LOG(LOG_ERROR, LOG_LEVEL_IMPORTANT, LOG_CONSOLE,
						"ServerCfg error: Can't find a launcher for Master-BeaconServer: %s (%s)\n",server_cfg.master_beacon_server,makeIpStr(ip));
		}
	}

	// Launch initial servers
	checkServerAutoStart();
	return ret;
}

static bool isServerAppRunning(const char *appname, int *running, int *crashed, int *pid)
{
	int i;
	*running = *crashed = *pid = 0;
	for (i=0; i<serverapp_list->num_alloced; i++)
	{
		ServerAppCon* app_con = (ServerAppCon *)serverapp_list->containers[i];
		if (stricmp(app_con->appname, appname)==0) {
			*running = true;
			*crashed = app_con->crashed;
			*pid = app_con->remote_process_info.process_id;
			return true;
		}
	}
	return false;
}

void startServerApp(RunServer *server)
{
	ServerAppCon *container;
	container = containerAlloc(serverapp_list,-1);
	container->active = 1;
	container->ip = server->ip;
	container->monitor = server->monitor;
	strcpy(container->appname, server->appname);
	if (!launcherCommStartServerProcess(server->command, server->ip, container, !server->track_by_id, server->monitor))
	{
		containerDelete(container);
	} 
	else 
	{
		// It's been started!
		if (server->monitor) 
		{
			LOG_OLD("Watching \"%s\" on %s.\n", server->appname, makeIpStr(server->ip));
		} 
		else 
		{
			LOG_OLD("Started \"%s\" on %s.\n", server->appname, makeIpStr(server->ip));
		}
	}
	server->last_crash_time = server->last_kill_time = 0;
}

void watchServerApp(RunServer *server)
{
	ServerAppCon *container;
	int i;
	bool foundit=false;
	// Find existing container
	for (i=0; i<serverapp_list->num_alloced; i++)
	{
		container = (ServerAppCon *)serverapp_list->containers[i];
		if (stricmp(container->appname, server->appname)==0) {
			foundit=true;
			break;
		}
	}
	if (!foundit)
		return;

	if (!launcherCommStartServerProcess(server->command, server->ip, container, !server->track_by_id, server->monitor))
	{
		containerDelete(container);
	}
	else 
	{
		// It's been started!
		if (server->monitor) 
		{
			LOG_OLD("Watching \"%s\" on %s.\n", server->appname, makeIpStr(server->ip));
		}
		else 
		{
			LOG_OLD("Started \"%s\" on %s.\n", server->appname, makeIpStr(server->ip));
		}
	}
	server->last_crash_time = server->last_kill_time = 0;
}

void checkServerAutoStart(void)
{
	int i;
	U32 ss2k = timerSecondsSince2000();
	// Any server in the list that does not have a container needs to be started
	for (i=eaSize(&eaRunServers)-1; i>=0; i--)
	{
		int running, crashed, pid;
		if (!isServerAppRunning(eaRunServers[i]->appname, &running, &crashed, &pid) && ((ss2k - eaRunServers[i]->last_run_time) > 30))
		{
			// Start it!
			eaRunServers[i]->last_run_time = ss2k;
			startServerApp(eaRunServers[i]);
		}
		if (running && pid==0 && eaRunServers[i]->monitor && ((ss2k - eaRunServers[i]->last_run_time) > 10))
		{
			// Look for it again!
			eaRunServers[i]->last_run_time = ss2k;
			watchServerApp(eaRunServers[i]);
		}
		if (running && crashed)
		{
			if (!eaRunServers[i]->last_crash_time) 
			{
				if (eaRunServers[i]->kill_and_restart) 
				{
					printf("%s \"%s\" crashed (will be automatically restarted in %d seconds).\n", timerGetTimeString(), eaRunServers[i]->appname, eaRunServers[i]->kill_and_restart);
				} 
				else 
				{
					printf("%s \"%s\" crashed (will NOT be automatically restarted).\n", timerGetTimeString(), eaRunServers[i]->appname, makeIpStr(eaRunServers[i]->ip));
				}
				eaRunServers[i]->last_crash_time = ss2k;
			} 
			else 
			{
				if (eaRunServers[i]->kill_and_restart) 
				{
					if ((ss2k - eaRunServers[i]->last_crash_time) > eaRunServers[i]->kill_and_restart &&
						(ss2k - eaRunServers[i]->last_kill_time) > eaRunServers[i]->kill_and_restart)
					{
						char cmd[1024];
						// Crashed for long enough to need a restart
						eaRunServers[i]->last_kill_time = ss2k;
						sprintf(cmd, "taskkill /f /pid %d", pid);
						printf("%s Killing crashed \"%s\" on %s.\n", timerGetTimeString(), eaRunServers[i]->appname, makeIpStr(eaRunServers[i]->ip));
						launcherCommExec(cmd, eaRunServers[i]->ip);
					}
				}
			}
		}
	}
	loadBalanceCheckReload();
}
