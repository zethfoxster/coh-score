#include <stdio.h>
#include <string.h>
#include <time.h>
#include "file.h"
#include "stdtypes.h"
#include "cmdoldparse.h"
#include "cmdcommon.h"
#include "MemoryPool.h"
#include "assert.h"
#include "cmdcontrols.h"
#include "timing.h"
#include "cmdcommon_enum.h"
#include "motion.h"
#include "utils.h"
#include "MessageStore.h"
#include "MessageStoreUtil.h"
#include "strings_opt.h"

#if CLIENT
#include "uiConsole.h"
#include "sprite_text.h"
#include "language/langClientUtil.h"
#else
#include "sendToClient.h"
#include "cmdserver.h"
#include "language/langServerUtil.h"
#endif


ControlState control_state;

ControlId opposite_control_id[CONTROLID_BINARY_MAX] =
{
	CONTROLID_BACKWARD,
	CONTROLID_FORWARD,
	CONTROLID_RIGHT,
	CONTROLID_LEFT,
	CONTROLID_DOWN,
	CONTROLID_UP,
};

#if CLIENT


	Cmd client_control_cmds[] = 
	{
		{ 0, "canlook", 0, {{ PARSETYPE_S32, &control_state.canlook }},0,
							"Whether the player can use the mouse to look around." },
		{ 0, "camrotate", 0, {{ PARSETYPE_S32, &control_state.cam_rotate }},0,
							"Camrotate is bound to the PageUp key to allow controlled camera rotation around the player.\n"
							"This command should not be invoked through the console." },
		{ 0, "forward_mouse",	CONCMD_FORWARD_MOUSECTRL, {{ PARSETYPE_S32, &control_state.move_input_state }}, 0,
							"Move forward.  Enable autorun after 2 seconds." },
		{ 0, "forward",	CONCMD_FORWARD, {{ PARSETYPE_S32, &control_state.move_input_state }}, 0,
							"Move forward." },
		{ 0, "backward", CONCMD_BACKWARD, {{ PARSETYPE_S32, &control_state.move_input_state }}, 0,
							"Move backwards." },
		{ 0, "left", CONCMD_LEFT, {{ PARSETYPE_S32, &control_state.move_input_state }}, 0,
							"Strafe left." },
		{ 0, "right", CONCMD_RIGHT, {{ PARSETYPE_S32, &control_state.move_input_state }}, 0,
							"Strafe right." },
		{ 0, "up", CONCMD_UP, {{ PARSETYPE_S32, &control_state.move_input_state }}, 0,
							"Jump or fly up." },
		{ 0, "down", CONCMD_DOWN, {{ PARSETYPE_S32, &control_state.move_input_state }}, 0,
							"Move down (if flying)." },
		{ 1, "nocoll", CONCMD_NOCOLL, {{ PARSETYPE_S32, &control_state.nocoll }},0,
							"turns off collisions for player" },
		{ 9, "nosync", 0, {{ PARSETYPE_S32, &control_state.nosync }},0,
							"disables syncing player position with the server" },
		{ 9, "lag", 0, {{ PARSETYPE_S32, &control_state.snc_client.lag }},0,
							"sets artificial lag for testing" },
		{ 9, "lag_vary", 0, {{ PARSETYPE_S32, &control_state.snc_client.lag_vary }},0,
							"sets artificial lag variance for testing" },
		{ 9, "pl", 0, {{ PARSETYPE_S32, &control_state.snc_client.pl }},0,
							"sets artificial packet loss for testing" },
		{ 9, "oo_packets", 0, {{ PARSETYPE_S32, &control_state.snc_client.oo_packets }},0,
							"sets artificial out of order packets for testing" },
		{ 0, "turnleft", 0, {{ PARSETYPE_S32, &control_state.turnleft }},0,
							"Rotate left a fixed number of degrees." },
		{ 0, "turnright", 0, {{ PARSETYPE_S32, &control_state.turnright }},0,
							"Rotate right a fixed number of degrees." },
		{ 0, "zoomin", 0, {{ PARSETYPE_S32, &control_state.zoomin }},0,
							"Zoom camera in." },
		{ 0, "zoomout", 0, {{ PARSETYPE_S32, &control_state.zoomout }},0,
							"Zoom camera out." },
		{ 0, "lookup", 0, {{ PARSETYPE_S32, &control_state.lookup }},0,
							"Pitch camera up." },
		{ 0, "lookdown", 0, {{ PARSETYPE_S32, &control_state.lookdown }},0,
							"Pitch camera down." },
		{ 0, "third", CONCMD_THIRD, {{ PARSETYPE_S32, &control_state.first }},0,
							"Toggles between first and third person camera." },
		{ 0, "first", CONCMD_FIRST, {{ PARSETYPE_S32, &control_state.first }},0,
							"Toggles between first and third person camera." },
		{ 9, "velscale", 0, {{ PARSETYPE_FLOAT, &control_state.inp_vel_scale}},0,
							"scales the speed at which the player runs"},
		{ 9, "pitch",		0, {{ PARSETYPE_FLOAT, &control_state.pyr.cur[0]}},0,
							"pitch"},
		{ 9, "yaw",		0, {{ PARSETYPE_FLOAT, &control_state.pyr.cur[1]}},0,
							"yaw"},
		{ 0, "autorun", 0, {{ PARSETYPE_S32, &control_state.autorun }},0,
							"Toggles autorun." },
		{ 9, "no_ragdoll", 0, {{ PARSETYPE_S32, &control_state.no_ragdoll }},0,
							"disables client ragdoll sim" },
		{ 0 },
	};
#endif

Cmd control_cmds[] = 
{
	{ 9, "controldebug",0, {{ PARSETYPE_S32, &control_state.controldebug }},0,
						"shows control information" },
	{ 9, "nostrafe",	0, {{ PARSETYPE_S32, &control_state.no_strafe }},0,
						"enables experimental no-strafe mode" },
	{ 9, "alwaysmobile",0, {{ PARSETYPE_S32, &control_state.alwaysmobile }},0,
						"player is always mobile (can't be immobilized by powers)" },
	{ 9, "repredict",	0, {{ PARSETYPE_S32, &control_state.repredict }},0,
						"client is out of sync with server, asking for new physics state info." },
	{ 0, "neterrorcorrection",	0, {{ PARSETYPE_S32, &control_state.net_error_correction }},CMDF_HIDEPRINT,
						"sets the level of error correction to use for input packets." },
	{ 1, "speed_scale",	0, {{ PARSETYPE_FLOAT, &control_state.speed_scale }},0,
						"debug option to make player move faster / slower" },
	{ 9, "svr_lag",		0, {{ PARSETYPE_S32, &control_state.snc_server.lag }},0,
						"sets artificial lag for testing" },
	{ 9, "svr_lag_vary",0, {{ PARSETYPE_S32, &control_state.snc_server.lag_vary }},0,
						"sets artificial lag variance for testing" },
	{ 9, "svr_pl",		0, {{ PARSETYPE_S32, &control_state.snc_server.pl }},0,
						"sets artificial packet loss for testing" },
	{ 9, "svr_oo_packets",	0, {{ PARSETYPE_S32, &control_state.snc_server.oo_packets }},0,
						"sets artificial out of order packets for testing" },
	{ 0, "client_pos_id",	0, {{ PARSETYPE_S32, &control_state.client_pos_id }},CMDF_HIDEPRINT,
						"for letting the server know we got his position update command." },
	{ 9, "atest0", 0, {{ PARSETYPE_S32, &control_state.anim_test[0] }},0,
						"anim test flag" },
	{ 9, "atest1", 0, {{ PARSETYPE_S32, &control_state.anim_test[1] }},0,
						"anim test flag" },
	{ 9, "atest2", 0, {{ PARSETYPE_S32, &control_state.anim_test[2] }},0,
						"anim test flag" },
	{ 9, "atest3", 0, {{ PARSETYPE_S32, &control_state.anim_test[3] }},0,
						"anim test flag" },
	{ 9, "atest4", 0, {{ PARSETYPE_S32, &control_state.anim_test[4] }},0,
						"anim test flag" },
	{ 9, "atest5", 0, {{ PARSETYPE_S32, &control_state.anim_test[5] }},0,
						"anim test flag" },
	{ 9, "atest6", 0, {{ PARSETYPE_S32, &control_state.anim_test[6] }},0,
						"anim test flag" },
	{ 9, "atest7", 0, {{ PARSETYPE_S32, &control_state.anim_test[7] }},0,
						"anim test flag" },
	{ 9, "atest8", 0, {{ PARSETYPE_S32, &control_state.anim_test[8] }},0,
						"anim test flag" },
	{ 9, "atest9", 0, {{ PARSETYPE_S32, &control_state.anim_test[9] }},0,
						"anim test flag" },
	{ 1, "predict", 0, {{ PARSETYPE_S32, &control_state.predict }},0,
						"turn client side prediction on/off" },
	{ 9, "notimeout", CONCMD_NOTIMEOUT, {{ PARSETYPE_S32, &control_state.notimeout }}, 0,
						"tells server not to disconnect this client if he times out. used for debugging" },
	{ 0, "selected_ent_server_index", 0, {{ PARSETYPE_S32, &control_state.selected_ent_server_index }},CMDF_HIDEPRINT,
						"send selected entity" },
	{ 9, "record_motion", 0, {{ PARSETYPE_S32, &control_state.record_motion }}, 0,
						"records physics, for posterity" },
	{ 0 },
};


// Variables the mapserver can update on the client, not specific to an entity
ServerVisibleState server_visible_state;
Cmd server_visible_cmds[] = 
{
	{ 9, "time", SVCMD_TIME, {{ PARSETYPE_FLOAT, &server_visible_state.time }},0,
						"sets current time of day for this map" },
	{ 9, "timescale", SVCMD_TIMESCALE, {{ PARSETYPE_FLOAT, &server_visible_state.timescale }},0,
						"sets rate time passes" },
	{ 9, "timestepscale",	SVCMD_TIMESTEPSCALE, {{ PARSETYPE_FLOAT, &server_visible_state.timestepscale }}, 0,
							"Runs simulation at this time scale."},
	{ 9, "pause", 0, {{PARSETYPE_S32, &server_visible_state.pause}},0,
						"<0/1> pauses game logic on server" },
	{ 9, "disablegurneys", 0, {{PARSETYPE_S32, &server_visible_state.disablegurneys}},0,
						"allows player to resurrect in place" },
	{ 9, "nodynamiccollisions", 0, {{ PARSETYPE_S32, &global_motion_state.noDynamicCollisions }},0,
						"disables placing dynamic collisions" },
	{ 9, "noentcollisions", 0, {{ PARSETYPE_S32, &global_motion_state.noEntCollisions }},0,
						"disables entity-entity collisions" },
	{ 9, "pvpmap", SVCMD_PVPMAP, {{ PARSETYPE_S32, &server_visible_state.isPvP }},0,
						"flags this map as pvp enabled" },
	{ 9, "svr_fog_dist", SVCMD_FOG_DIST, {{ PARSETYPE_FLOAT, &server_visible_state.fog_dist }},0,
						"Sets the fog on the server, overrides scene file" },
	{ 9, "svr_fog_color", SVCMD_FOG_COLOR, {{ PARSETYPE_FLOAT, &server_visible_state.fog_color[0]},{ PARSETYPE_FLOAT, &server_visible_state.fog_color[1]},{ PARSETYPE_FLOAT, &server_visible_state.fog_color[2]}},0,
						"Sets the fog on the server, overrides scene file" },
	{ 0 },
};

GlobalState global_state;

void cmdOldCommonInit()
{
	cmdOldInit(server_visible_cmds);
	#if CLIENT
		cmdOldInit(client_control_cmds);
	#endif
	cmdOldInit(control_cmds);
}

MP_DEFINE(ControlStateChange);

ControlStateChange* createControlStateChange()
{
	#if SERVER
		MP_CREATE(ControlStateChange, 1000);
	#else
		MP_CREATE(ControlStateChange, 100);
	#endif
	
	return MP_ALLOC(ControlStateChange);
}

void destroyControlStateChange(ControlStateChange* csc)
{
	#if CLIENT
		while(csc->future_push_list)
		{
			FuturePush* next = csc->future_push_list->csc_next;
			csc->future_push_list->csc = NULL;
			csc->future_push_list = next;
		}
	#endif
		
	MP_FREE(ControlStateChange, csc);
}

void addControlStateChange(ControlStateChangeList* list, ControlStateChange* csc, int adjust_time)
{
	list->count++;
	
	if(!list->head)
	{
		#if CLIENT
			if(adjust_time)
			{
				S32 diff_ms = csc->time_stamp_ms - control_state.time.last_send_ms;
				
				if(diff_ms < 0)
				{
					csc->time_stamp_ms = control_state.time.last_send_ms;
				}
			}
		#endif
		
		list->head = list->tail = csc;
		csc->next = NULL;

		#if SERVER
			csc->prev = NULL;
		#endif
	}
	else
	{
		#if CLIENT
			if(adjust_time)
			{
				S32 diff_ms = csc->time_stamp_ms - list->tail->time_stamp_ms;

				if(diff_ms < 0)
				{
					csc->time_stamp_ms = list->tail->time_stamp_ms;
				}		
			}
		#endif
		
		list->tail->next = csc;
		csc->next = NULL;
		
		#if SERVER
			csc->prev = list->tail;
		#endif

		list->tail = csc;
	}
}

void destroyControlStateChangeList(ControlStateChangeList* list)
{
	ControlStateChange* csc = list->head;
	
	while(csc)
	{
		ControlStateChange* next = csc->next;
		destroyControlStateChange(csc);
		csc = next;
	}
	
	ZeroStruct(list);
}

void destroyControlStateChangeListHead(ControlStateChangeList* list)
{
	if(list->head)
	{
		ControlStateChange* next = list->head->next;
		destroyControlStateChange(list->head);
		list->head = next;
		assert(list->count > 0);
		if(!--list->count)
		{
			list->tail = NULL;
		}
	}
}

#if SERVER

MP_DEFINE(ServerControlState);

ServerControlState* createServerControlState()
{
	MP_CREATE(ServerControlState, 10);
	
	return MP_ALLOC(ServerControlState);
}

void destroyServerControlState(ServerControlState* scs)
{
	MP_FREE(ServerControlState, scs);
}

void destroyServerControlStateList(ServerControlStateList* list)
{
	ServerControlState* scs = list->head;
	
	while(scs)
	{
		ServerControlState* next = scs->next;
		destroyServerControlState(scs);
		scs = next;
	}
	
	ZeroStruct(list);
}

void addServerControlState(ServerControlStateList* list, ServerControlState* scs)
{
	list->count++;
	
	if(!list->head)
	{
		list->head = list->tail = scs;
		scs->next = NULL;
	}
	else
	{
		list->tail->next = scs;
		list->tail = scs;
	}
}

ServerControlState* getQueuedServerControlState(ControlState* controls)
{
	ServerControlState* scs;
	ServerControlStateList* list = &controls->queued_server_states;

	if(controls->cur_server_state_sent || !list->head)
	{
		// Create a new state because the list is empty or the current state was already sent.
		
		controls->cur_server_state_sent = 0;
		
		controls->sent_update_id++;
			
		scs = createServerControlState();

		if(list->tail)
			*scs = *list->tail;
		else
			*scs = *controls->server_state;

		scs->id = controls->sent_update_id;
		scs->hit_stumble = 0;  //This was just a ping, and should always get cleared after being used
		
		addServerControlState(list, scs);
	}
	else
	{
		scs = list->tail;
	}

	return scs;
}

const ServerControlState* getLatestServerControlState(ControlState* controls)
{
	ServerControlStateList* list = &controls->queued_server_states;

	if(list->tail)
		return list->tail;
	else
		return controls->server_state;
}

void destroyControlState(ControlState* controls) {
	if (controls->packet_ids) {
		free(controls->packet_ids);
		controls->packet_ids=NULL;
	}
	
	destroyServerControlState(controls->server_state);
	controls->server_state = NULL;
	
	destroyControlStateChangeList(&controls->csc_list);
	destroyServerControlStateList(&controls->queued_server_states);
	destroyFuturePushList(controls);
}
#endif

#if CLIENT
void addControlLogEntry(ControlState* controls)
{
	if(controls->controlLog.enabled)
	{
		time_t curTime = time(NULL);
		
		if(curTime - controls->controlLog.start_time > controls->controlLog.duration)
		{
			controls->controlLog.enabled = 0;
		}
		else if(controls->controlLog.cur < ARRAY_SIZE(controls->controlLog.entries))
		{
			ControlLogEntry* entry = controls->controlLog.entries + controls->controlLog.cur;
			
			controls->controlLog.cur++;
			
			entry->actualTime = curTime;
			entry->mmTime = timeGetTime();
			entry->timestep = TIMESTEP;
			entry->timestep_noscale = TIMESTEP_NOSCALE;
			QueryPerformanceCounter((LARGE_INTEGER *)&entry->qpc);
			QueryPerformanceFrequency((LARGE_INTEGER *)&entry->qpcFreq);
		}
	}
}
#endif


MP_DEFINE(FuturePush);

static FuturePush* createFuturePush()
{
	#if SERVER
		MP_CREATE(FuturePush, 1000);
	#else
		MP_CREATE(FuturePush, 100);
	#endif
	
	return MP_ALLOC(FuturePush);
}

void destroyFuturePush(FuturePush* fp)
{
	#if CLIENT
		assert(!fp->csc);
	#endif
	
	MP_FREE(FuturePush, fp);
}

void destroyFuturePushList(ControlState* controls)
{
	while(controls->future_push_list)
	{
		FuturePush* next = controls->future_push_list->controls_next;
		destroyFuturePush(controls->future_push_list);
		controls->future_push_list = next;
	}
}

#if SERVER
	FuturePush* addFuturePush(ControlState* controls, U32 abs_diff)
#else
	FuturePush* addFuturePush(ControlState* controls)
#endif	
{
	FuturePush* fp = createFuturePush();
	
	#if CLIENT
		// Client adds to the tail of the list.
		
		FuturePush** prev = &controls->future_push_list;
		while(*prev)
		{
			prev = &(*prev)->controls_next;
		}
		*prev = fp;
	#else
		// Server adds to the head of the list.
		
		fp->controls_next = controls->future_push_list;
		controls->future_push_list = fp;
		fp->abs_time = ABS_TIME + abs_diff;
		fp->net_id_start = controls->phys.latest.net_id;
		fp->phys_tick_wait = fp->phys_tick_remaining = max(1, (abs_diff + 50) / 100);
	#endif
		
	return fp;
}

// Moved from cmdparse.c
void cmdOldPrintList(Cmd *cmds,void *client,int access_level,char *match_str_src,int print_header,int show_usage)
{
	int		i;
	char	buf[2000];
	Cmd		*cmd;
	char	commentBuffer[2000];
	char*	comment;	
	char*	str;
	char	delim;
	int		firstLine;
	size_t	maxNameLen = 1;
	char	nameFormat[100];
	char	commentFormat[100];
	char	match_str[1024];

	if( match_str_src )
		Strncpyt(match_str, stripUnderscores(match_str_src));
	else
		match_str[0] = '\0';

	if (print_header) {
		Strncpyt( buf, textStd("LevelCommands",access_level) );
#if CLIENT
		conPrintf("\n%s\n", buf  );
#else
		conPrintf(client,"\n%s\n",buf);
#endif
		strcat( buf, "\n" );
	}
	for(i=0;cmds[i].name;i++)
	{
		cmd = &cmds[i];
		maxNameLen = max(maxNameLen, strlen(cmd->name));
	}

	if( access_level > 0 )
		sprintf_s(SAFESTR(nameFormat), "%%d %%-%d.%ds", maxNameLen, maxNameLen);
	else
		sprintf_s(SAFESTR(nameFormat), "%%-%d.%ds", maxNameLen+10, maxNameLen+10);
	sprintf_s(SAFESTR(commentFormat), "%%s%%-%d.%ds", maxNameLen + 3, maxNameLen + 3);

	for(i=0;cmds[i].name;i++)
	{
		const char * cmdname;

		cmd = &cmds[i];

#if !TEST_CLIENT
		cmdname = cmdRevTranslated(cmd->name);
#else
		cmdname = cmd->name;
#endif

		if (cmd->access_level > access_level || !cmd->comment || (match_str && !strstri(stripUnderscores(cmdname),match_str)))
			continue;
		if (show_usage && !cmd->usage_count)
			continue;
		if( cmd->flags & CMDF_HIDEPRINT )
			continue;

		if( access_level > 0 )
			sprintf_s(SAFESTR(buf),nameFormat,cmd->access_level,cmdname);
		else
			sprintf_s(SAFESTR(buf),nameFormat,cmdname);

		strcat(buf," ");

		if (show_usage) {
			sprintf_s(SAFESTR(buf), "%s -- %s", buf, textStd("UsesString",cmd->usage_count) );
		} else {

			if( stricmp(cmd->name, cmdname) == 0 )
			{
				assert(strlen(cmd->comment) < 2000);
				Strncpyt(commentBuffer, cmd->comment ? cmd->comment : "");
			}
			else
			{
#if !TEST_CLIENT
				Strncpyt(commentBuffer, cmdHelpTranslated(cmd->name));
#endif
			}

			comment = commentBuffer;
			firstLine = 1;
			for(str = strsep2(&comment, "\n", &delim); str; str = strsep2(&comment, "\n", &delim)){
				if(firstLine)
					firstLine = 0;
				else
					sprintf_s(SAFESTR(buf), commentFormat, buf, "");

				strcat(buf, str);
				strcat(buf, "\n");
				if(delim)
					comment[-1] = delim;
			}
		}
		//strcat(buf,cmd->comment);
#if CLIENT
		conPrintf("%s",buf);
#else
		conPrintf(client,"%s",buf);
#endif
	}
}



