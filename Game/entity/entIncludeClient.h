
#if ENT_INCLUDE_PART == 1
	/* Start client-only members ***********************************************/

	// client net vars

	Quat				net_qrot;            // last qrot received rom server
	Quat				cur_interp_qrot;

	// Index of entity in the ent_xlat array on the client. 
	// This value corresponds to Entity::owner on the server.
	// This is not the same as Entity::owner on the client.
	int                     svr_idx;

	int						activeFxTrackerCount; //for perf
	int						maxTrackerIdxPlusOne; //for perf
	ClientNetFxTracker *	fxtrackers;  // fx this ent owns
	int						fxtrackersAllocedCount; //for dynArrayAdd
	NetFx *					outOfOrderFxDestroy;
	int						outOfOrderFxDestroyCount;
	int						outOfOrderFxDestroyMax;

	MotionHist				motion_hist[MOTION_HIST_MAX];
	int						motion_hist_latest;
	Vec3					ragdoll_offset_pos;
	Vec3					ragdollHipBonePos;
	Quat*				nonRagdollBoneRecord;
//	Quat				ragdoll_offset_qrot;

	NwEmissaryDataGuid		nxCapsuleKinematic;

	Vec3					posLastFrame;
	Vec3					vel_last_frame;		//for the splatter fx
	Mat4    				vol_collision;      // world space coords of the collision with the volume surface. Only valid if you are actually in a volume
	Mat4					prev_vol_collision; // needed for nested volumes
	U8						currfade;           //client's


	int						next_move_idx;		// change to this when move_change_timer expires.
	U16						next_move_bits;		// hack to set seq state bits
	U16						move_bits;
#if(NEXTMOVEBIT_COUNT > 16)
#error "Please make next_move_bits and move_bits bigger."
#endif
	U8						move_updated;

	int						demo_id;			// used by demo playback system (is non-zero when entity was created by demo)

	ClientEntityDebugInfo*	debugInfo;
	EntityDemoRecordInfo*	demoRecordInfo;

	int						onFlashback;
	int						onArchitect;
	int						architect_invincible;
	int						architect_untargetable;
	int						iRank;
	char*					pchVillGroupName;
	char*					pchDisplayClassNameOverride; //If classDisplayName is defined in villaindef the server sends it down from the villain def instead of the class->displayName
	char*					villainDefFilename;			 // Used for click to source, not sent in live game
	char*					villainInternalName;		 // Used for click to source, villain internal name

	F32						curr_xlucency;
	EntFadeDirection		fadedirection;		//ENT_FADING_IN or ENT_FADING_OUT

	U32						waitingForFadeOut : 1;			// This entity will be deleted when it is faded out entirely when (currfade == 0).
	int						fadeState[STATE_ARRAY_SIZE];
	Vec3					fadeVel;

	Floater					aFloaters[FLOATER_MAX_COUNT]; // Dynamic things that appear over the entities head
	int						iNumFloaters;

	unsigned long			pvpUpdateTime;	// When was info about pvp last sent to this client
	int						pvpClientZoneTimer;	// How many seconds after the update time this target becomes pvp active

	U32						confuseRand;	// Saved when this character becomes confused, used for confused reticles

	U32						ulStatusEffects;	// Saves the flags for various crowd control effects

	// Packet ids for the individual streams of data on the entity
	U32						pkt_id_friendslist;
	U32						pkt_id_seq_move;
	U32						pkt_id_seq_triggermove;
	U32						pkt_id_logout;
	U32						pkt_id_supergroup;
	U32						pkt_id_costume;
	U32						pkt_id_powerCust;
	U32						pkt_id_xlucency;
	U32						pkt_id_titles;
	U32						pkt_id_character_stats;
	U32						pkt_id_targetting;
	U32						pkt_id_status_effects;
	U32						pkt_id_buffs;
	U32						pkt_id_debug_info;
	U32						pkt_id_create;
	U32						pkt_id_rare_bits;
	U32						pkt_id_state_mode;
	U32						pkt_id_qrot[3];				// 3 packet ids for each qrot component.
	U32						pkt_id_pos[24];				// That's right, a packet id for each position bit.  Oh yeah baby!

	// Packet IDs of when a given attribute was last updated (only 8 bytes of each of these is used on entities other than the player)
	void **					pkt_id_fullAttrDef; // Only used on the player
	void **					pkt_id_basicAttrDef; // Used on all others

	char					supergroup_name[64];    // used for everyone else's supergroup name
	char					supergroup_emblem[128]; // used for everyone else's supergroup
	U32						supergroup_color[2];    // used for everyone else's supergroup

	PowerBuff				**ppowBuffs;
	bool					showInMenu;				// Entities specifically for menus

	const PNPCDef*			persistentNPCDef;		// info for persistent npc's
	int						contactHandle;

	/* End client-only members *************************************************/
#elif ENT_INCLUDE_PART == 2

#elif ENT_INCLUDE_PART == 3

	#define MOTION_HIST_MAX			(64)
	#define FLOATER_MAX_STRING_LEN  (512)
	#define FLOATER_MAX_COUNT       (10)

	#define ENTPOS(e)				((const F32*)ENTMAT(e)[3])
	#define ENTPOSX(e)				ENTPOS((e))[0]
	#define ENTPOSY(e)				ENTPOS((e))[1]
	#define ENTPOSZ(e)				ENTPOS((e))[2]
	#define ENTPOSPARAMS(e)			ENTPOSX(e), ENTPOSY(e), ENTPOSZ(e)

	typedef struct Floater
	{
		EFloaterStyle eStyle;
		char    ach[FLOATER_MAX_STRING_LEN];
		U32     colorFG;
		U32     colorBG;
		U32     colorBorder;
		int     iXLast;
		int     iYLast;
		float   fScale;
		float   fSpeed;
		float   fTimer;
		float   fMaxTimer;
		bool	bUseLoc;
		bool	bScreenLoc;
		Vec3	vecLocation;
	} Floater;

	typedef struct MotionHist
	{
		Vec3		pos;
		Quat	qrot;
		U32			abs_time;
		void*		debug;
	} MotionHist;

	typedef enum EntFadeDirection
	{
		ENT_FADING_IN  = 0,
		ENT_FADING_OUT = 1,
		ENT_NOT_FADING = 2,
	} EntFadeDirection;
	
	typedef struct EntityDemoRecordInfo		EntityDemoRecordInfo;
	typedef struct ClientEntityDebugInfo	ClientEntityDebugInfo;

	/*Header info client entity needs to track netFx currently attached to it (one shots aren't tracked)*/
	typedef struct ClientNetFxTracker		ClientNetFxTracker;

	typedef struct PowerBuff PowerBuff;


#endif
