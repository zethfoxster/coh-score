#ifndef ENTDEBUGPRIVATE_H
#define ENTDEBUGPRIVATE_H

#include "stdtypes.h"
#include "group.h"

#define ENCOUNTER_HIST_LENGTH 250
#define MAX_ENCOUNTER_BEACONS 200

typedef struct DebugMenuItem DebugMenuItem;
typedef struct BasePower BasePower;

typedef struct DebugMenuItem {
	DebugMenuItem*			parent;
	char					displayText[200];
	char					commandString[200];
	char*					rolloverText;

	struct {
		DebugMenuItem*		head;
		DebugMenuItem*		tail;
	} subItems;

	DebugMenuItem*			nextSibling;

	int						flashStartTime;
	U32						usedCount;
	U32						bitsUsed;
	U32						open : 1;
	U32						flash : 1;
} DebugMenuItem;

typedef struct FileDebugMenu
{
	char*						name;
	char*						command;
	struct FileDebugMenu**		menus;
	int							open;
} FileDebugMenu;

typedef struct EntDebugLine
{
	Vec3 p1, p2;
	int argb1, argb2;
} EntDebugLine;

typedef struct EntDebugText
{
	Vec3	pos;
	char	text[50];
	int		flags;
} EntDebugText;


typedef struct AStarRecordingSet AStarRecordingSet;

typedef struct AStarPoint
{
	AStarRecordingSet* set;
	Vec3 pos;
	float flyHeight;
	float minHeight;
	float maxHeight;
} AStarPoint;

typedef struct AStarConnectionInfo 
{
	int				queueIndex;
	U32				costSoFar;
	U32				totalCost;
	Vec3			beaconPos;
	AStarPoint*		point;
	U32				flash : 1;
} AStarConnectionInfo;

typedef struct AStarRecordingSet 
{
	int						index;
	AStarPoint**			points;
	AStarPoint**			nextPoints;
	AStarPoint*				parent;
	int						checkedConnectionCount;
	int						costSoFar;
	int						totalCost;
	int						foundNextPoints;
	AStarConnectionInfo**	nextConns;
} AStarRecordingSet;

typedef struct AStarBeaconBlock 
{
	Vec3					pos;
	F32						searchY;
	AStarPoint**			beacons;
} AStarBeaconBlock;

typedef struct AStarRecording 
{
	AStarRecordingSet** sets;
	AStarBeaconBlock**	blocks;
	Vec3				target;
	int					frame;
} AStarRecording;

typedef struct DebugButton 
{
	struct DebugButton*	next;

	char				displayText[100];
	char				commandText[200];
} DebugButton;

typedef struct EncounterBeacon
{
	Vec3				pos;
	int					speed;
	U32					color;
	U32					time;
	char				filename[MAX_PATH];
} EncounterBeacon;

typedef struct DebugCamera
{
	Mat4				mat;
	int					relativeToThisEntity;
	int					trackThisEntity;
	DefTracker			*tracker;
} DebugCamera;

typedef struct DebugCameraInfo 
{
	int						editMode;
	int						on;
	int						curCamera;
	int						hideInEditor;
	
	struct 
	{
		DebugCamera*		camera;
		int					size;
		int					previousSize;
		int					maxSize;
	} cameras;

} DebugCameraInfo;

typedef struct DebugState 
{
	int						mouseOverUI;

	int						cursel;
	DebugMenuItem*			menuItem;
	FileDebugMenu			allFileMenus;
	StashTable				menuItemNameTable;
	int						loadedFileMenus;
	int						upHeld, downHeld;
	int						pageupHeld, pagedownHeld;
	int						keyDelayMsecStart;
	int						keyDelayMsecWait;
	int						closeMenu;
	int						fromMouse;
	int						firstLine;
	int						windowHeight;
	int						openingTime;
	int						lastTime;
	int						closingTime;
	int						old_mouse_x, old_mouse_y;
	int						mouseMustMove;
	int						moveMouseToSel;
	int						retainMenuState;
	int						motdVisible;
	int						rolloverTextVisible;
	int						waitingForBindKey;
	char*					motdText;
	struct 
	{
		int x, y;
		int menuWidth;
		int greenx, greeny, greenw, greenh; // Size of the area in the lower left (green area)
	} size;

	StashTable				perfInfoStateTable;
	int						resetPerfInfoTops;
	int						hidePerfInfo;
	int						hidePerfInfoText;
	int						showMaxCPUCycles;
	int						perfInfoScale;
	F32						perfInfoScaleF32;
	int						perfInfoPaused;
	int						perfInfoBreaks;
	PerformanceInfo*		perfInfoZoomed;
	int						perfInfoZoomedHilight;
	int						perfInfoZoomedAlpha;

	EntDebugLine*			lines;
	int						linesCount;
	int						curLine;

	EntDebugText			text[1000];
	int						textCount;
	int						curText;

	__int64					serverTimersTotal;
	__int64					serverCPUSpeed;
	__int64					serverTimePassed;
	__int64					serverTimeUsed;
	int						serverTimersLayoutID;
	int						serverTimersCount;
	int						serverTimersAllocated;
	PerformanceInfo*		serverTimers;
	char					(*serverTimerName)[100];

	AStarRecording			aStarRecording;

	DebugButton*			debugButtonList;
	int						debugButtonCount;

	int						encounterPlayerCount[ENCOUNTER_HIST_LENGTH];
	int						encounterRunning[ENCOUNTER_HIST_LENGTH];
	int						encounterActive[ENCOUNTER_HIST_LENGTH];
	F32						encounterWin[ENCOUNTER_HIST_LENGTH];
	F32						encounterPlayerRatio[ENCOUNTER_HIST_LENGTH];
	F32						encounterActiveRatio[ENCOUNTER_HIST_LENGTH];
	int						encounterPanicLevel[ENCOUNTER_HIST_LENGTH];
	int						encounterTail;
	int						encounterPlayerRadius;
	int						encounterVillainRadius;
	int						encounterAdjustProb;
	F32						encounterPanicLow;
	F32						encounterPanicHigh;
	int						encounterPanicMinPlayers;
	int						encounterMinCritters;
	int						encounterMaxCritters;
	int						encounterCurCritters;
	unsigned int			encounterWrap : 1;
	unsigned int			encounterReceivingStats : 1;
	unsigned int			encounterPlayerRadiusIsDefault : 1;
	unsigned int			encounterVillainRadiusIsDefault : 1;
	unsigned int			encounterAdjustProbIsDefault : 1;
	unsigned int			encounterBelowMinCritters : 1;
	unsigned int			encounterAboveMaxCritters : 1;

	int						numEncounterBeacons;
	EncounterBeacon*		encounterBeacons;		// only MAX_ENCOUNTER_BEACONS will be alloc'ed

	DebugCameraInfo			debugCamera;

	const BasePower			*debugPower;
} DebugState;

extern DebugState debug_state;
extern KeyBindProfile ent_debug_binds_profile;

DebugMenuItem* addDebugMenuItem(DebugMenuItem* parent, const char* displayText, const char* commandString, int open);
void freeDebugMenuItem(DebugMenuItem* item);
void displayDebugMenu(void);
void entDebugInitMenu(void);
void initEntDebug();

#endif