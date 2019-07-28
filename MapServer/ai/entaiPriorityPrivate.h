
#ifndef ENTAIPRIORITYPRIVATE_H
#define ENTAIPRIORITYPRIVATE_H

#include "entaiPriority.h"

typedef struct AIPriorityCondition {
	int** simple;
	
	struct {
		int		op;
		char*	param1;
		char*	param2;
		int		not;
	} complx;
} AIPriorityCondition;

typedef struct AIPriorityEvent {
	char*					name;
	float					atTime;
	AIPriorityCondition**	conditions;
	int**					sets;
	AIPriorityAction**		actions;
	AIPriorityAction**		elseActions;
} AIPriorityEvent;

typedef struct AIPriority {
	char*						name;
	int							holdPriority;
	AIPriorityCondition**		conditions;
	//int**						exitConditions;
	int**						sets;
	TokenizerParams**			IAmA;
	TokenizerParams**			IReactTo;
	AIPriorityAction**			actions;
	AIPriorityEvent**			events;
} AIPriority;

typedef struct AIPriorityList {
	char*					name;
	AIPriority**			priorities;
} AIPriorityList;

typedef struct AIAllPriorityLists {
	AIPriorityList** lists;
} AIAllPriorityLists;

typedef struct AIPriorityState {
	unsigned int		disabled : 1;
} AIPriorityState;

typedef struct AIPriorityManager {
	AIBehavior*			myBehavior;
	AIPriorityList*		list;
	int					priority;
	int*				flags;
	AIPriorityList*		nextList;
	int					nextPriority;
	int*				nextFlags;
	float				priorityTime;
	AIPriorityState*	stateArray;
	int					stateArraySize;
	unsigned int		stateArrayIsValid	: 1;
	unsigned int		holdCurrentList		: 1;
} AIPriorityManager;

enum{
	AI_PRIORITY_PRIVATE_ENUM_BEGIN = AI_PRIORITY_PUBLIC_ENUM_END + 1,

	// Comparator

	CONDCOMP_LOW,
	CONDCOMP_EQUAL,
	CONDCOMP_NOTEQUAL,
	CONDCOMP_LESS,
	CONDCOMP_LESS_EQUAL,
	CONDCOMP_GREATER,
	CONDCOMP_GREATER_EQUAL,
	CONDCOMP_HIGH,

	// Var

	CONDVAR_LOW,

	CONDVAR_ME_ATTACKER_COUNT,
	CONDVAR_ME_DISTANCE_TO_SAVIOR,
	CONDVAR_ME_DISTANCE_TO_TARGET,
	CONDVAR_ME_ENDURANCE_PERCENT,
	CONDVAR_ME_HAS_ATTACK_TARGET,
	CONDVAR_ME_HAS_FOLLOW_TARGET,
	CONDVAR_ME_HAS_REACHED_TARGET,
	CONDVAR_ME_HEALTH_PERCENT,
	CONDVAR_ME_IS_LEADER,
	CONDVAR_ME_OUT_OF_DOOR,
	CONDVAR_ME_TIME_SINCE_COMBAT,
	CONDVAR_ME_TIME_SINCE_WAS_ATTACKED,

	CONDVAR_TEAM_LEADER_HAS_REACHED_TARGET,
	CONDVAR_TEAM_TIME_SINCE_WAS_ATTACKED,

	CONDVAR_GLOBAL_TIME24,

	CONDVAR_RANDOM_PERCENT,

	// Set-able vars.

	CONDVAR_SET_LOW,

	CONDVAR_LIST_HOLD,

	CONDVAR_ME_ALWAYS_GET_HIT,
	CONDVAR_ME_DESTROY,
	CONDVAR_ME_DO_NOT_DRAW_ON_CLIENT,
	CONDVAR_ME_DO_NOT_FACE_TARGET,
	CONDVAR_ME_DO_NOT_GO_TO_SLEEP,
	CONDVAR_ME_DO_NOT_MOVE,
	CONDVAR_ME_FEELING_BOTHERED,
	CONDVAR_ME_FEELING_CONFIDENCE,
	CONDVAR_ME_FEELING_FEAR,
	CONDVAR_ME_FEELING_LOYALTY,
	CONDVAR_ME_FIND_TARGET,
	CONDVAR_ME_FLYING,
	CONDVAR_ME_INVINCIBLE,
	CONDVAR_ME_INVISIBLE,
	CONDVAR_ME_LIMITED_SPEED,
	CONDVAR_ME_ON_TEAM_MONSTER,
	CONDVAR_ME_ON_TEAM_HERO,
	CONDVAR_ME_ON_TEAM_VILLAIN,
	CONDVAR_ME_OVERRIDE_PERCEPTION_RADIUS,
	CONDVAR_ME_SPAWN_POS_IS_SELF,
	CONDVAR_ME_TARGET_INSTIGATOR,
	CONDVAR_ME_UNTARGETABLE,
	CONDVAR_ME_HAS_PATROL_POINT,

	CONDVAR_SET_HIGH,

	CONDVAR_HIGH,

	// Set

	SETOPERATOR_LOW,
	SETOPERATOR_EQUAL,
	SETOPERATOR_HIGH,
};

enum{
	AI_PL_FLAG_UNKNOWN = -1,
	AI_PL_FLAG_FLY,
	AI_PL_FLAG_RUN,
	AI_PL_FLAG_WALK,
	AI_PL_FLAG_WIRE,
	AI_PL_FLAG_DOORENTRY_ONLY,
};

enum{
	PRIORITY_NONE = -1,
	PRIORITY_NOT_SET = -2,
};

AIPriorityList* aiGetPriorityListByName(const char* name);

#endif