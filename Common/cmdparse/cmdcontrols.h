#ifndef _CMDCONTROLS_H
#define _CMDCONTROLS_H

#include "cmdcommon.h"
#include "motion.h"

typedef struct ControlStateChange	ControlStateChange;
typedef struct ServerControlState	ServerControlState;
typedef struct FuturePush			FuturePush;

#define CSC_ANGLE_BITS 11
#define CSC_ANGLE_F32_TO_U32(x) ((U32)(0.5 + ((x) + PI) * (1 << CSC_ANGLE_BITS) / (2 * PI)))
#define CSC_ANGLE_U32_TO_F32(x) ((F32)(x) * (2 * PI) / (1 << CSC_ANGLE_BITS) - PI)

typedef struct ControlStateChange
{
	ControlStateChange*				next;

	ControlId						control_id;
	
	U16								net_id;
	
	#if SERVER
		ControlStateChange*			prev;
		
		U16							time_diff_ms;
		U32							pak_id;
		U16							pak_id_index;
		U8							last_key_state;
		F32							last_pyr[2];
		U32							initializer : 1;
	#else
		U32							time_stamp_ms;
		
		U32							last_send_ms;
		U32							net_packet_order;

		FuturePush*					future_push_list;
		
		U32							sent : 1;
	#endif

	union{
		// CONTROLID_UP/DOWN/LEFT/RIGHT/FORWARD/BACKWARD.
		
		U32							state : 1;

		// CONTROLID_NOCOLL
		U32							state2 : 2;

		// CONTROLID_YAW/PITCH.
		
		#if SERVER
			F32						angleF32;
		#else
			U32						angleU32 : CSC_ANGLE_BITS;
		#endif
		
		// CONTROLID_RUN_PHYSICS.
		
		struct {
			U32						client_abs;
			U32						client_abs_slow;
			U8						inp_vel_scale_U8;
			
			#if CLIENT
				// The state of things immediately BEFORE this physics step is run.
				
				struct {
					MotionState		motion_state;
					Vec3			pos;
				} before;
				
				// The state of things immediately AFTER this physics step is run.
				
				struct {
					Vec3			pos;
					Vec3			vel;
				} after;
				
				// Ack data, if it exists.
				
				struct {
					Vec3			pos;
					Vec3			vel;
				} ack_info;
				
				U32					has_ack : 1;
			#endif

			U32						no_ack : 1;
			U32						controls_input_ignored : 1;
		};

		// CONTROLID_APPLY_SERVER_STATE.
		
		U8							applied_server_state_id;
	};
} ControlStateChange;

typedef struct ServerControlState
{
	#if SERVER
		ServerControlState*			next;
		U8							id;
		U32							abs_time_sent;
		U32							physics_disabled : 1;
	#endif

	float							speed[3];
	float							speed_back;
	float							jump_height;

	SurfaceParams					surf_mods[SURFPARAM_MAX];

	U32								controls_input_ignored	: 1;
	U32								shell_disabled			: 1;
	U32								no_ent_collision		: 1;
	U32								fly						: 1;
	U32								stun					: 1;
	U32								jumppack				: 1;
	U32								no_jump_repeat			: 1;
	U32								glide					: 1;
	U32								no_physics				: 1;
	U32								hit_stumble				: 1;
	U32								ninja_run				: 1;
	U32								walk				    : 1;
    U32								beast_run				: 1;
	U32								steam_jump				: 1;
	U32								hover_board				: 1;
	U32								magic_carpet			: 1;
	U32								parkour_run				: 1;
} ServerControlState;


typedef struct FuturePush
{
	FuturePush*				controls_next;
	U16						net_id_start;
	U16						phys_tick_remaining;
	U32						abs_time;
	Vec3					add_vel;
	
	#if SERVER
		U16					phys_tick_wait;
	#else
		FuturePush*			csc_next;
		ControlStateChange*	csc;
	#endif
	
	U32						net_id_used			: 1;
	U32						do_knockback_anim	: 1;

	#if SERVER
		U32					was_sent			: 1;
	#else
		U32					abs_time_used		: 1;
	#endif
} FuturePush;

#endif
