
#include "entaiprivate.h"
#include "cmdcommon.h"
#include "cmdserver.h"
#include "entaiLog.h"
#include "earray.h"


// AIPowerInfo ----------------------------------------------------

MP_DEFINE(AIPowerInfo);

AIPowerInfo* createAIPowerInfo(){
	MP_CREATE(AIPowerInfo, 100);
	
	return MP_ALLOC(AIPowerInfo);
}

void destroyAIPowerInfo(AIPowerInfo* info){
	eaDestroy(&info->allowedTargets);	// these strings are all allocAddStrings so just an eaDestroy is enough
	MP_FREE(AIPowerInfo, info);
}

// AIBasePowerInfo ------------------------------------------------

MP_DEFINE(AIBasePowerInfo);

AIBasePowerInfo* createAIBasePowerInfo(){
	MP_CREATE(AIBasePowerInfo, 100);
	
	return MP_ALLOC(AIBasePowerInfo);
}

void destroyAIBasePowerInfo(AIBasePowerInfo* info){
	MP_FREE(AIBasePowerInfo, info);
}

void aiSetCurrentBuff(AIPowerInfo **pbuff, AIPowerInfo *buff)
{
	assert(*pbuff==NULL);
	*pbuff = buff;
	if (buff->teamInfo)
		buff->teamInfo->inuseCount++;
}

void aiClearCurrentBuff(AIPowerInfo **pbuff)
{
	assert(*pbuff!=NULL);
	if ((*pbuff)->teamInfo) {
		(*pbuff)->teamInfo->inuseCount--;
		(*pbuff)->teamInfo->time.lastStoppedUsing = ABS_TIME;
	}
	*pbuff = NULL;
}

//---------------------------------------------------------------------------------------------

MP_DEFINE(AIAvoidInstance);

AIAvoidInstance* createAIAvoidInstance(){
	MP_CREATE(AIAvoidInstance, 100);
	
	return MP_ALLOC(AIAvoidInstance);
}

void destroyAIAvoidInstance(AIAvoidInstance* avoid){
	MP_FREE(AIAvoidInstance, avoid);
}

void aiAddAvoidInstance(Entity* e, AIVars* ai, int maxLevelDiff, F32 radius, U32 uid){
	AIAvoidInstance* avoid = createAIAvoidInstance();
	
	avoid->maxLevelDiff = maxLevelDiff;
	avoid->radius = radius;
	avoid->uid = uid;
	
	avoid->next = ai->avoid.list;
	ai->avoid.list = avoid;

	AI_LOG(	AI_LOG_COMBAT,
			(e,
			"^2ADDED ^0avoid instance ^4%d^0: ^4%d ^0lvl diff at ^4%1.1f^0ft.\n",
			avoid->uid,
			avoid->maxLevelDiff,
			avoid->radius));
			
	if(radius > ai->avoid.maxRadius){
		ai->avoid.maxRadius = radius;
	}
}

void aiRemoveAvoidInstance(Entity* e, AIVars* ai, U32 uid){
	AIAvoidInstance* avoid;
	AIAvoidInstance* prev;
	F32 maxRadius = 0;
	
	for(avoid = ai->avoid.list, prev = NULL; avoid;){
		if(avoid->uid == uid){
			AIAvoidInstance* next;
			
			if(prev){
				next = prev->next = avoid->next;
			}else{
				next = ai->avoid.list = avoid->next;
			}
			
			AI_LOG(	AI_LOG_COMBAT,
					(e,
					"^1REMOVED ^0avoid instance ^4%d^0: ^4%d ^0lvl diff at ^4%1.1f^0ft.\n",
					avoid->uid,
					avoid->maxLevelDiff,
					avoid->radius));
			
			destroyAIAvoidInstance(avoid);
			
			avoid = next;
		}else{
			if(avoid->radius > maxRadius){
				maxRadius = avoid->radius;
			}
			
			prev = avoid;
			avoid = avoid->next;
		}
	}

	ai->avoid.maxRadius = maxRadius;
}

void aiRemoveAllAvoidInstances(Entity* e, AIVars* ai){
	while(ai->avoid.list){
		AIAvoidInstance* next = ai->avoid.list->next;
		destroyAIAvoidInstance(ai->avoid.list);
		ai->avoid.list = next;
	}
	
	aiRemoveAvoidNodes(e, ai);
	
	ai->avoid.maxRadius = 0;
}

//TO DO I'm not sure where this should live
#include "entity.h"
#include "powers.h"
AIPowerInfo * aiFindAIPowerInfoByPowerName( Entity * e, const char * powerName )
{
	AIPowerInfo * powerInfo = 0;
	AIVars* ai = ENTAI(e);

	if( powerName && powerName[0] )
	{
		AIPowerInfo * info;
		for(info = ai->power.list; info; info = info->next)
		{
			if(!stricmp(powerName, info->power->ppowBase->pchName))
			{
				powerInfo = info;
				break;
			}
		}
	}

	return powerInfo;
}

