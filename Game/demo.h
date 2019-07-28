#ifndef _DEMO_H
#define _DEMO_H

#include "stdtypes.h"

#define ABSTIME_SECOND (30*100)
#define ABSTIME_TO_MSECS(x) ((x)/3)
#define MSECS_TO_ABSTIME(x) ((x)*3)

typedef struct Entity Entity;
typedef struct NetFx NetFx;

typedef enum DemoChatType
{
	kDemoChatType_Normal,
	kDemoChatType_Floater, // must be 1
	kDemoChatType_InfoBox,
	kDemoChatType_Count,
} DemoChatType;


typedef struct DemoState
{
	int		demo_pause;
	int		demo_fps;
	int		demo_dump;
	int		demo_dump_tga;
	int		demo_loop;
	int		demo_framestats;
	F32		demo_speed_scale;
	char	demoname[MAX_PATH];
	int		demo_hide_names;
	int		demo_hide_damage;
	int		demo_hide_chat;
	int		demo_hide_all_entity_ui;
	int		detached_camera;
} DemoState;

extern DemoState demo_state;

void demoFreeEntityRecordInfo(Entity* e);
int demoIsRecording();
void demoRecord(char const *fmt, ...);
void demoSetEntIdx(int ent_idx);
void demoSetAbsTime(unsigned int time);
void demoCheckPlay();
void demoStartPlay(char *fname);
void demoStartRecord(char *fname);
void demoStop();
void demoRecordAppearance(Entity *e);
void demoRecordStatChanges(Entity* e);
void demoRecordFx(NetFx *netfx);
void demoRecordMove(Entity *e,int move_idx,int move_bits);
void demoRecordPos(const Vec3 pos);
void demoRecordPyr(Vec3 pyr);
void demoRecordDelete();
void demoRecordCreate(char *name);
void demoSetAbsTimeOffset(int abs_time_offset);
U32 demoGetAbsTime();
void demoPlay();
void demoRecordCamera(Mat4 mat);
void demoRecordClientInfo();
int demoIsPlaying();
void demoRecordDynGroups( void );
void demoRecordSkyFile(int sky1, int sky2, F32 weight);

#endif
