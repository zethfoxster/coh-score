#include "textClientInclude.h"
#include "testEmail.h"
#include "testTeam.h"
#include "testUtil.h"
#include "testConnections.h"
#include "testClient/testLevelup.h"
#include "testNetworkStats.h"
#include "testContact.h"
#include "cmdgame.h"
#include "seq.h"
#include "initClient.h"
#include "clientcomm.h"
#include "load_def.h"
#include "sock.h"
#include "uiLogin.h"
#include "uiEditText.h"
#include "gameData/costume_data.h"
#include "gameData/BodyPart.h"
#include "gameData/randomCharCreate.h"
#include "Npc.h"
#include "cmdCommon.h"
#include "entrecv.h"
#include "timing.h"
#include "error.h"
#include "player.h"
#include "utils.h"
#include "sysutil.h"
#include "textparser.h"
#include <conio.h>
#include <time.h>
#include <process.h>
#include "character_base.h"
#include "entclient.h"
#include "entVarUpdate.h"
#include "itemselect.h"
#include "pmotion.h"
#include "fighter.h"
#include "mover.h"
#include "MemoryMonitor.h"
#include "memcheck.h"
#include "dbclient.h"
#include "authclient.h"
#include "testclient/TestClient.h"
#include "error.h"
#include "FolderCache.h"
#include "net_version.h"
#include "character_level.h"
#include "SharedMemory.h"
#include "SharedHeap.h"
#include "StashTable.h"
#include "AppVersion.h"
#include "entPlayer.h"
#include "dooranimcommon.h"
#include "Stackdump.h"
#include "Version/AppVersion.h"
#include "Version/AppRegCache.h"
#include "direct.h"
#include "netfx.h"
#include "entity.h"
#include "seqstate.h"
#include "authUserData.h"
#include "chatter.h"
#include "crypt.h"
#include "tchar.h"
#include "estring.h"
#include "StringCache.h"
#include "textclientcmdparse.h"
#include "comm_backend.h"
#include "log.h"
#include "inventory_client.h"
#include "applocale.h"
#include "language/langClientUtil.h"
#include "character_eval.h"
#include "character_combat_eval.h"
#include "file.h"

char *display_server_names[] = {
		"Freedom",
		"Justice",
		"Pinnacle",
		"Virtue",
		"Liberty",
		"Guardian",
		"Infinity",
		"Protector",
		"Victory",
		"Champion",
		"Triumph",
		"Defiant",
		"Zukunft",
		"Vigilance",
		"Union",
		"Exalted",
};

void mainloop(void);

extern TokenizerParseInfo ParseCostumeSet[];
extern Entity	*current_target;
extern int do_map_xfer;
extern char glob_map_name[MAX_PATH];
extern int glob_plrstate_valid;

int test_client_num = 0;
int g_nobrawl = 0;
char connserver[256] = "";
char server[256] = "";
char character_name[256] = {0};
int err=0;
int reconnect=0;
const char *launcherVersion = NULL;
int login_retry_count = 2;
int server_num=-1; // Which server in the list the AuthServer returns to connect to 
int sleep_time = 200;
bool g_verbose_client;
char g_achPasswordTC[32];
bool dontpause = false;

void gameStateInit() {
	strcpy(game_state.cs_address, connserver);
	game_state.no_version_check = 1;
	strcpy(game_state.address, server);
	cmdOldSetSends(control_cmds,1);
	//
}

void genLoginInfo() {
	static char name[256] = "";
	static char password[256] = "11111111";
	sprintf(name, "Dummy%05d", 1);
	printf("Username [%s]: ", name);
	gets(name);
	printf("Password [11111111]: ");
	gets(password);
	if (stricmp(password, "")==0) {
		strcpy(password, "11111111");
	}
	if (!name[0]) {
		sprintf(name, "Dummy%05d", 1);
	}
	strcpy(g_achPasswordTC, password);
	strcpy(g_achAccountName, name);
}

void printLocation() {
	const F32 *pos = ENTPOS(playerPtr());
	printf("Location: %1.3f %1.3f %1.3f\n", pos[0], pos[1], pos[2]);
}

void checkPlayer(void) {
	static int doneit=0;
	if (!doneit && playerPtr()) {
		char buf[256];
		sprintf(buf, "Running: %s (%s)", playerPtr()->name, g_achAccountName);
		setConsoleTitle(buf);
		doneit=1;
	}
	if (playerPtr()) {
/*		static float oldhp = -1;
		static int countdown = -1;
		static int setpos_countdown = -1;
		static int report_countdown = -1;
		char buf[512];
        float hp = playerPtr()->pchar->attrCur.fHitPoints;
		const F32 *pos = ENTPOS(playerPtr());

		report_countdown--;
		countdown--;

		if (hp!=oldhp && report_countdown <=0 && report_to_launcher) {
			sprintf(buf, "HP: %1.1f/%1.1f", hp, playerPtr()->pchar->attrMax.fHitPoints);
			if (show_hp_changes)
				printf("%s\n", buf);
			oldhp = hp;
			report_countdown = 10*5; // About once a 10 seconds
			sendMessageToLauncher("%s", buf);
			if (hp<=0) {
				//statusUpdate("Dead");
				countdown = 45;
				stats.deathcount++;
			} else if (countdown) {
				countdown=-1;
//				statusUpdate("Running");
			}
		}
		if (countdown<=0 && (g_testMode & TEST_AUTOREZ)) {
//			statusUpdate("Running");
			if (hp<=0) {
//				char buff[1024];
				EMPTY_INPUT_PACKET(CLIENTINP_DEAD_NOGURNEY_RESPONSE);
//				sprintf(buff, "entcontrol me sethealth %1.3f\nentcontrol me setendurance %1.3f",
//					playerPtr()->pchar->attrMax.fHitPoints, playerPtr()->pchar->attrMax.fEndurance);
//				commAddInput(buff);
				countdown = 45;
			}
		}
		if (stricmp(playerPtr()->name, character_name)!=0) {
			printf("Old (expected) name: %s\n", character_name);
			printf("New name: %s\n", playerPtr()->name);
			strcpy(character_name, playerPtr()->name);
		}
#define OUT_OF_RANGE 16000
		if (setpos_countdown<=0 && (pos[0] <-OUT_OF_RANGE || pos[0] >OUT_OF_RANGE ||
			pos[1] <-OUT_OF_RANGE || pos[1] >OUT_OF_RANGE ||
			pos[2] <-OUT_OF_RANGE || pos[2] >OUT_OF_RANGE))
		{
			printf("I seem to be out of range, running STUCK (%1.3f %1.3f %1.3f)\n", pos[0], pos[1], pos[2]);
			commAddInput("stuck");
			setpos_countdown = 10;
		}
		if (setpos_countdown>0)
			setpos_countdown--; */
	}
}

bool isNumeric(const unsigned char *s) {
	const unsigned char *c;
	for (c=s; *c; c++) {
		if (!isdigit(*c))
			return false;
	}
	return true;
}

#define CMDEQ(s) (stricmp(argv[i], s)==0)
void checkArgs(int argc, char **argv) {
	int i;
	for (i=1; i<argc; i++) {
		if (CMDEQ("-hideconsole")) {
			hideConsoleWindow();
		} else if (CMDEQ("-authname")) {
			i++;
			strcpy(g_achAccountName,argv[i]);
		} else if (CMDEQ("-password")) {
			i++;
			strcpy(g_achPasswordTC,argv[i]);
		} else if (CMDEQ("-character")) {
			strncpy(character_name,argv[++i], sizeof(character_name));
			printf("Will attempt to resume character %s...\n",character_name);
		} else if (CMDEQ("-version")) {
			launcherVersion = argv[i] + ARRAY_SIZE("-version");
			printf("cmd line launcherVersion=%s\n",launcherVersion);
		} else if (CMDEQ("-retry")) {
			login_retry_count = -1;
		} else if (CMDEQ("-dontpause")) {
			dontpause = true;
		} else if (CMDEQ("-cs") || CMDEQ("-db")) {
			i++;
			strcpy(connserver, argv[i]);
		} else if (CMDEQ("-auth")) {
			i++;
			strcpy(game_state.auth_address, argv[i]);
		} else if (CMDEQ("-serverchoice")) {
			i++;
			server_num = atoi(argv[i]);
		} else if (CMDEQ("-localmapserver")) {
			game_state.local_map_server = 1;
		} else if (CMDEQ("-server")) {
			char noquote[1024];
			i++;
			if (!strstr(argv[i],"\"")) {
				// Doesn't start with a ", so just copy it
				strcpy(server,argv[i]);
			}
			else {
				strcpy(noquote,&((argv[i])[1]));
				while (strstr(noquote,"\"")==NULL) {
					strcat(noquote," ");
					strcat(noquote,argv[++i]);
				}
				strncpy(server,noquote,strlen(noquote)-1);
			}
		} else if (CMDEQ("-patchDir")) {
			argv[i][0] = 0;
			if (i+1 < argc && argv[i+1])
			{
				forwardSlashes(argv[i+1]);
				PigSetAddPatchDir(unquote(argv[i+1]));

				argv[i+1][0] = 0;
				++i;
			}
		} else if (CMDEQ("-patchVersion")) {
			argv[i][0] = 0;
			if (i+1 < argc && argv[i+1])
			{
				setExecutablePatchVersionOverride(argv[i+1]);
				argv[i+1][0] = 0;
				++i;
			}
		} else if (CMDEQ("-notimeout")) {
			control_state.notimeout = 1;
		} else if (CMDEQ("-packetdebug")) {
			pktSetDebugInfo();
			bsAssertOnErrors(true);
		} else if (CMDEQ("-cov")) {
			game_state.skin = UISKIN_VILLAINS;
		} else if (CMDEQ("-cod")) {
			game_state.cod = 1;
			printf("Using development data (d_).\n");
			FolderCacheRemoveIgnorePrefix("d_");
		}
	}
}

// handle shared memory command line flags before the first use
static bool process_cmdline_for_sharedmemory(int argc, char **argv) {
	int i;
	for (i=1; i<argc; i++) {
		if (CMDEQ("-nosharedmemory")) {
			printf(" ** WARN **: Not using shared memory due to -nosharedmemory\n");
			sharedMemorySetMode(SMM_DISABLED);
			sharedHeapTurnOff("disabled");
			return false;
		} else if (CMDEQ("-testsharedmemory")) {
			testSharedMemory();
			error_exit(0);
			return false;
		} else if (CMDEQ("-printsharedmemory")) {
			printSharedMemoryUsage();
			error_exit(0);
			return false;
		}
	}
	return true;	// use shared memory
}

int error_exit(int allow_debug) {
	if (!dontpause) {
		if (allow_debug)
			printf("\nPress 'd' to debug, any other key to exit.\n");
		while (!kbhit()) {
			Sleep(500);
		}
		if (allow_debug && kbhit() && getch()=='d')
			return 1;
	}
	exit(-1);
	return 0;
}

void errorCallback(char *errMsg) {
	if (!testClientSafeError(errMsg)) {
		printf("Errorf: %s\n", errMsg);
	}
}

void fatalErrorCallback(char *errMsg) {
	printf("Fatal Error: %s\n", errMsg);
	setConsoleTitle("ERROR");
	error_exit(0);
}

char errorBuff[1024];
char stackdump[1024*10];

static int CrtReportingFunction( int reportType, char *userMessage, int *retVal )
{
	if (reportType == _CRT_WARN) {
		return FALSE;
	}
	printf("\nCRT Error: %s\n", userMessage);
	sdDumpStackToBuffer(stackdump, ARRAY_SIZE(stackdump)-1, NULL);
	printf("Call stack:\n%s\n", stackdump);
	setConsoleTitle("CRASH");
	if (error_exit(1)) {
		*retVal = 1;
		return 1;
	} else {
		*retVal = 0;
		return 0;
	}
}

extern char* GetExceptionName(DWORD code);
int tcAssertExcept(unsigned int code, PEXCEPTION_POINTERS info)
{
	printf("\nException caught: %s\n", GetExceptionName(code));
	sdDumpStackToBuffer(stackdump, ARRAY_SIZE(stackdump)-1, info->ContextRecord);
	printf("Call stack:\n%s\n", stackdump);
	setConsoleTitle("CRASH");
	if (error_exit(1)) {
		return EXCEPTION_CONTINUE_SEARCH;
	} else {
		return EXCEPTION_EXECUTE_HANDLER;
	}
}

extern int dbWaitForStartOrQueue(F32 timeout);
int loginWrapper(void) {
	int ret;
	if (!authLogin(g_achAccountName,g_achPasswordTC))
		return 0;

	if (game_state.auth_address[0]) {
		if (server_num == -1) {
			int i;
			printf("Available Servers:\n");
			for (i=0; i<auth_info.server_count; i++) {
				printf("  %d %s (%s) (%d/%d) %s\n", 1 + i, auth_info.servers[i].name, makeIpStr(auth_info.servers[i].ip), auth_info.servers[i].curr_user_count, auth_info.servers[i].max_user_count, auth_info.servers[i].server_status?"up":"DOWN");
			}
			server_num = promptRanged("Which server: ", auth_info.server_count);
		}
		acSendAboutToPlay(auth_info.servers[server_num].id);
		if (!authWaitFor(AC_PLAY_OK))
			return 0;
		authDisconnect();
	} else {
		server_num = 0;
	}
	ret = dbConnect(makeIpStr(auth_info.servers[server_num].ip),auth_info.servers[server_num].port,
		auth_info.uid,auth_info.game_key,g_achAccountName,
		1,NO_TIMEOUT,true);

	if(ret == DBGAMESERVER_QUEUE_POSITION) {
		setConsoleTitle("Queued...");
		while((ret=dbWaitForStartOrQueue(control_state.notimeout?NO_TIMEOUT:30.0f)) == DBGAMESERVER_QUEUE_POSITION) {
			int pos, count;
			if(dbclient_getQueueStatus(&pos, &count)) {
				printf("Queued (%d of %d)\n", pos, count);
			}
			Sleep(1000);
		}
	}
	return ret;
}

void processCmd(char *buf, int quiet) {
	if (!textClientCmdParse(buf))
		commAddInput(buf);
}

int handleKey() {
	static char *buf = NULL;
	int ch;

	if (!buf) {
		buf = malloc(1024*1024);
	}

	ch = getch();
	switch (ch) {
	case 27:
		return 1;
	xcase 'e':
		checkEmail(1);
	xcase ' ':
		doJump();
	xcase 'm':
		memMonitorDisplayStats();
		memCheckDumpAllocs();
	xcase 'n':
		enableNetworkStats(-1);
	xcase 'i':
		testChatNPC();
	xcase 'c':
		printContactInfo();
	xcase 'l':
		printLocation();
	xcase 'r':
		reconnect=1;
		return 1;
	xcase 'h':
		if (current_target) {
			printf("Currently targeting: ");
			if ( current_target->name[0] )
				printf("%s\n", current_target->name );
			else
				printf("%s\n", npcDefList.npcDefs[current_target->npcIndex]->displayName );
		} else {
			printf("No current target\n");
		}
	xcase 's':
		printf("Stopping.\n");
		stopMoving();
	xcase 'f':
		if (current_target) {
			printf("Now following: ");
			if ( current_target->name[0] )
				printf("%s\n", current_target->name );
			else
				printf("%s\n", npcDefList.npcDefs[current_target->npcIndex]->displayName );
			setDestEnt(current_target);
		} else {
			printf("No current target\n");
		}
	xcase 'v':
		g_verbose_client = !g_verbose_client;
		printf("Verbose mode: %s\n", g_verbose_client ? "ON" : "OFF");
	xcase 't':
		printf("Enter target: ");
		gets(buf);
		targetByName(buf);
		if (!current_target)
			printf("Target not found!\n");
	xcase 'u':
		current_target = 0;
	xcase '/':
		printf("/");
		gets(buf);
		processCmd(buf,0);
	xcase '!':
		system("cmd");
	}
	return 0;
}

void calcTimeStep(void);

void fixUpEntities(void)
{
	int i;
	for(i=1;i<entities_max;i++)
	{
		if (entity_state[i] & ENTITY_IN_USE ) {
			Entity *e = entities[i];
			memset( e->netfx_list, 0, sizeof( NetFx ) * e->netfx_count );
			e->netfx_count = 0; 
		}
	}
}

void mainloop() {
	extern int glob_have_camera_pos;
	int framecount=0;
	for(;;) // the official infinite loop
    {
		//heapValidateAll();
		sharedHeapLazySync();
		framecount++;
		Sleep(sleep_time);
		glob_plrstate_valid = 0;
		glob_have_camera_pos = PLAYING_GAME;
		commSendInput();
		commCheck(0);
		entNetUpdate();
		entClientProcess();
		fixUpEntities();

		calcNetworkStats();

		calcTimeStep();

		// Test Client specific:
		if (playerPtr()) {
			stats.xp = playerPtr()->pchar->iExperiencePoints;
			stats.influence = playerPtr()->pchar->iInfluencePoints;
		}
		checkPlayer();
		checkTarget();
		checkEmail(0);

		doMoverLoop();

		if (kbhit() && handleKey())
			break;

		if (do_map_xfer)
		{
			do_map_xfer = 0;
			if (!doMapXfer()) {
				printf("Error connecting to MapServer on MapMove\n");
				setConsoleTitle("ERROR");
				error_exit(0);
				break;
			} else {
				printf("Map: %s\n", glob_map_name);
			}
		}
		{
			static int done=0;
			static int timer=0;
			if (playerPtr() && playerPtr()->pl->door_anim && (playerPtr()->pl->door_anim->state != DOORANIM_WAITDOOROPEN && playerPtr()->pl->door_anim->state != DOORANIM_ACK))
			{
				if (!timer && !done) {
					timer=20;
					done=1;
				}
				if (timer) {
					if (timer==1) {
						playerPtr()->pl->door_anim->state = DOORANIM_ACK;
						EMPTY_INPUT_PACKET(CLIENTINP_DOORANIM_DONE);
						done = 0;
					}
				}
			}
			if (timer) {
				timer--;
			}
		}

		mpCompactPools();
	}
}

int main(int argc, char **argv)
{
	int firstEmptySlot=0;
	int timer;
	int global_count=0;

	memCheckInit();

	timer = timerAlloc();

	//pktSetDebugInfo();
	//bsAssertOnErrors(true);
	cryptInit();

	setAssertMode( (!IsDebuggerPresent()? ASSERTMODE_THROWEXCEPTION : 0) | ASSERTMODE_DEBUGBUTTONS);
	_CrtSetReportHook(CrtReportingFunction);
	__try {
		if (!"stack corruption") {
			int a[4];
			a[-1] = -1;
			return;
		}
		if (!"heap corruption") {
			char *data = malloc(5);
			data[-1] = -1;
			free(data);
			return;
		}
		if (!"assert") {
			assert(0);
			assertmsg(0, "die!");
		}
		if (!"exception") {
			int a = 0;
			int b = 1/a;
		}
		if (!"debug break") {
			_asm { int 3 };
		}
		

		//print some info
		{
			TCHAR buffer[MAX_PATH];
			int i;

			printf("(%d) %s\n",_getpid(), getExecutableName());

			printf("cmdline args: ");
			for( i=1; i< argc; ++i)
			{
				printf("%s ", argv[i]);
			}
			printf("\n");

			GetCurrentDirectory(ARRAY_SIZE(buffer), buffer);
			_tprintf(_T("Working directory: %s\n"),buffer);

		}
		
		mpCompactPools();
		memset(&game_state, 0, sizeof(game_state));

		printf("Memory startup...\n");
		{
			bool bUseSharedMemory = process_cmdline_for_sharedmemory(argc, argv);
			if (bUseSharedMemory)
			{
				loadstart_printf("reserving shared heap space...");
				{
					// @todo this should be a different heap name than the server heap
					// with a smaller reservation size for test clients
					U32 uiReservedSize = reserveSharedHeapSpace(getSharedHeapReservationSize());
					if ( uiReservedSize )
					{
						loadend_printf(" reserved %d MB", uiReservedSize / (1024 * 1024));
					}
					else
					{
						loadend_printf(" failed: no shared heap");
					}
				}
				if (sharedHeapEnabled())
					sharedStringReserve(65536);
			}
		}


		loadstart_printf("Misc startup...\n");

		// cov and dev file ignore
		//FolderCacheAddIgnorePrefix("v_");
		FolderCacheAddIgnorePrefix("d_");

		authUserSetHeroAccess((U32*)db_info.auth_user_data,1); // for development debugging
		authUserSetVillainAccess((U32*)db_info.auth_user_data,1); // for development debugging
		authUserSetRogueAccess((U32*)db_info.auth_user_data,1);	// for development debugging

		checkArgs(argc, argv);
		fileInitSys();
		FolderCacheChooseMode();
		fileInitCache();
		FatalErrorfSetCallback(fatalErrorCallback);
		ErrorfSetCallback(errorCallback);
		disableRtlHeapChecking(NULL);
		loadend_printf("");

		loadstart_printf("Pigs/Foldercache...");
		logSetDir("TestClient");
		loadend_printf("");

		if (strcmp(g_achAccountName,"")==0)
			genLoginInfo();
		printf("Connecting as %s...\n", g_achAccountName);

		setConsoleTitle("Loading...");

		SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS);

		srand( timerCpuTicks64() );
		gameStateInit();

		// Test client init
		if (strcmp(server, "Training Room"))
			launcherVersion = getExecutablePatchVersion("CoH");
		else
			launcherVersion = getExecutablePatchVersion("CohTest");
		printf("getting version from registry: %s\n", launcherVersion);
		if (launcherVersion) {
			setExecutablePatchVersionOverride(launcherVersion);
		}

		// From game.main.c
		cmdOldCommonInit();
		loadend_printf("");
// 		if (launcherVersion) {
// 			printf("Using version specified by TCLauncher: %s\n", launcherVersion);
// 		}

		reloadClientMessageStores(LOCALE_ID_ENGLISH);
		chareval_Init();
		combateval_Init();
		chatClientInit();

		loadstart_printf("Networking startup...");
		packetStartup(0,1); // <--- All of the time is in here
		commNewInputPak();
		sockStart();
		commInit();
		//pktSetDebugInfo(); // ab: need to fix so this works!
		loadend_printf("");

		loadstart_printf("load_AllDefs()");
		seqLoadStateBits();
		load_AllDefs();
		loadend_printf("");
		//init_menus();
		//loadFonts();
		loadstart_printf("bpReadBodyPartFiles()");
		bpReadBodyPartFiles(); // Need this, at least, for character creation
		loadend_printf("");
		//loadstart_printf("npcReadDefFiles()");
		//npcReadDefFiles();
		//loadend_printf("");

		loadstart_printf("loadCostumes()");
		loadCostumes();
		loadPowerCustomizations();
		loadend_printf("");

		setConsoleTitle("Connecting...");

		global_count = 0;
		do {
			bool createNew = false;
			global_count++;
			err = 0;
			// Log on
			{
				int count=0;
				bool logged_in=false;
				for (;!logged_in && count!=login_retry_count; count++) {
					if(loginWrapper()) {
						logged_in = true;
					} else if (count != login_retry_count-1) {
						printf("Error logging on\nAuthServer Error:\n%s\nDBServer Error:\n%s\nTrying again...\n", authGetError(), dbGetError());
					}
					if (kbhit() && getch()==27) break;
				}
				if (!logged_in) {
					printf("Error logging on\nAuthServer Error:\n%s\nDBServer Error:\n%s\n", authGetError(), dbGetError());
					setConsoleTitle("ERROR");
					err=1;
				}
			}
			gPlayerNumber = 0;
			if (!err) {
				for (firstEmptySlot=0; firstEmptySlot<db_info.player_count; firstEmptySlot++) {
					if (stricmp(db_info.players[firstEmptySlot].name, "empty")==0)
						break;
				}
				if (strcmp(character_name,"") != 0)
				{
					printf("Searching for %s in character list...\n",character_name);
					for (gPlayerNumber=0; gPlayerNumber<db_info.player_count; gPlayerNumber++) {
						printf(" %d - %s\n",gPlayerNumber,db_info.players[gPlayerNumber].name);
						if (stricmp(db_info.players[gPlayerNumber].name, character_name)==0)
							break;
					}
					if (gPlayerNumber == db_info.player_count) {
						gPlayerNumber = -1;
						printf("Unable to locate character %s\n",character_name);
					}
					else {
                        printf("Found character %s in slot %d\n",character_name, gPlayerNumber);
					}
				}
				if (db_info.player_count && firstEmptySlot==db_info.player_count)
					firstEmptySlot = -1;
			}

			if (!err) {
				bool chooseExisting = false;
				if (db_info.player_count>0)	// account has some pre-existing characters, let user choose
				{
					printf("Do you wish to choose an existing character? [yN]");
					chooseExisting = consoleYesNo();
					if (chooseExisting) {
						int i;
						for (i=0; i<db_info.player_count; i++) {
							printf("\t%d : %s (dbid %d)\n", i+1, db_info.players[i].name, db_info.players[i].db_id);
						}
						gPlayerNumber=-1;
						while (gPlayerNumber<0 || gPlayerNumber>=db_info.player_count) {
							int k;
							printf("Choose a slot: [1-%d] ", db_info.player_count);
							k = getch();
							gPlayerNumber = k - '1';
							printf("\n");
						}
					}
				}
				else
				{
					printf("No existing characters to choose from.\n");
				}

				if (!chooseExisting)
				{
					// either nothing to choose or they don't want to, so we have to create one
					printf("Do you wish to create a character? [yN]");
					if (consoleYesNo()) {
						createNew = true;
					} else {
						printf("Exiting.\n");
						setConsoleTitle("ERROR");
						err = 1;
					}
				}
			}

			if (!err && !createNew && (firstEmptySlot<=gPlayerNumber) && (firstEmptySlot!=-1)) {
				printf("%s does not have a character in slot %d to log into.\n", g_achAccountName, gPlayerNumber);
				setConsoleTitle("ERROR");
				err = 1;
			}

			if (!err && !createNew && (firstEmptySlot>gPlayerNumber || firstEmptySlot==-1) && (gPlayerNumber>=0)) {
				// Resume a character
				loadstart_printf("Resuming character in slot %d...",gPlayerNumber);
				if(!choosePlayerWrapper( gPlayerNumber, 0 )) {
					printf("Error calling choosePlayerWrapper/resuming character:\n%s\n", dbGetError());
					setConsoleTitle("ERROR");
					dbDisconnect();
					err = 1;
				}
				if (!err) {
					strcpy(character_name, db_info.players[gPlayerNumber].name);
					dbDisconnect();           //Character being resumed, no longer need dbServer.
					player_being_created = 0;
					loadend_printf("");
					tryConnect();
					if( !commConnected() ) {
						printf("Error connecting to MapServer\n");
						setConsoleTitle("ERROR");
						err = 1;
					} else {
						// this is where we receive costume back
						loadstart_printf("commReqScene()");
						commReqScene(1);
						loadend_printf("");
					}
				}
			} else if (!err) {
				if (firstEmptySlot==-1) {
					printf("No free slots for %s\n", g_achAccountName);
					setConsoleTitle("ERROR");
					err=1;
				} else {
					player_slot = gPlayerNumber = firstEmptySlot;
					// Create a new character in firstEmptySlot
					printf("simulateCharacterCreate()\n");
					loadstart_printf("");
					if(db_info.players)
					{
						simulateCharacterCreate(1, 0);
						strcpy(character_name, playerPtr()->name);
						loadend_printf("commReqScene(1)?");
					}
					else
					{
						printf("Did not get players from dbserver.  Error: %s\n", db_info.error_msg);
						setConsoleTitle("ERROR");
						err=1;
					}
				}
			}

		} while(global_count != (login_retry_count+1) && err);

		if (glob_map_name[0]) {
			printf("Map: %s\n", glob_map_name);
		}

		if (!err) {
			checkPlayer();
		}
		loadstart_printf("");

		printf("Total time to launch TestClient : %1.3g\n", timerElapsed(timer));
		timerFree(timer);

		testClientRandomDisconnect(TCS_allDone);
		
		// Switch to normal-er priority
		SetPriorityClass(GetCurrentProcess(), BELOW_NORMAL_PRIORITY_CLASS);

		printf("Press Esc to exit...\n");
		if (err) {
			error_exit(0);
			return 0;
		}
		game_state.local_map_server = 0; // So that it gets booted back to login screen when it gets disconnected

		enableNetworkStats(1);
		calcNetworkStats();
		enableNetworkStats(0);

		if (!err)
			mainloop();

		if ((reconnect) && !err) {
			commDisconnect();
			setConsoleTitle("Reconnecting...");
			Sleep(5000);
			error_exit(0); // temp hack so testclients stick around so we can see the log better...
		}

	}
	__except (tcAssertExcept(GetExceptionCode(), GetExceptionInformation()))
	{
		return 0;
	}
	return 0;
}
