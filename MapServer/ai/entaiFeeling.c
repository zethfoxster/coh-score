
#include "entaiprivate.h"
#include "cmdcommon.h"
#include "cmdserver.h"
#include "entaiLog.h"


const char* aiFeelingName[AI_FEELING_COUNT] = {
	"FEAR",
	"CONFIDENCE",
	"LOYALTY",
	"BOTHERED",
};

// AIFeelingModifier ---------------------------------------------------------------------------

MP_DEFINE(AIFeelingModifier);

AIFeelingModifier* createAIFeelingModifier(){
	AIFeelingModifier* modifier;
	
	MP_CREATE(AIFeelingModifier, 1000);
	
	modifier = MP_ALLOC(AIFeelingModifier);
	
	return modifier;
}

void destroyAIFeelingModifier(AIFeelingModifier* modifier){
	if(!modifier)
		return;

	MP_FREE(AIFeelingModifier, modifier);
}

void aiFeelingDestroy(Entity* e, AIVars* ai){
	AIFeelingModifier* cur = ai->feeling.modifiers;
	
	while(cur){
		AIFeelingModifier* next = cur->next;

		destroyAIFeelingModifier(cur);
		cur = next;
	}
	
	ai->feeling.modifiers = NULL;
}

void aiFeelingUpdateTotals(Entity* e, AIVars* ai){
	int i, j;
	AIFeelingModifier* modifier;
	AIFeelingModifier* prev;
	AIFeelingLevels mods;
	
	// Only update modifiers table once per second.
	
	if(ABS_TIME_SINCE(ai->feeling.modifierCheckTime) < SEC_TO_ABS(1))
		return;
		
	ai->feeling.modifierCheckTime = ABS_TIME;

	ZeroStruct(&mods);
	
	for(modifier = ai->feeling.modifiers, prev = NULL; modifier;){
		AIFeelingModifier* next = modifier->next;
		U32 range = modifier->time.end - modifier->time.begin;
		U32 diff = modifier->time.end - ABS_TIME;

		if(diff <= range){
			F32 begin_factor = (float)diff / (float)range;
			F32 total = begin_factor * modifier->level.begin + (1.0 - begin_factor) * modifier->level.end;

			mods.level[modifier->feelingID] += total;
			
			prev = modifier;
		}else{
			S32 diff = ABS_TIME - modifier->time.end;
			
			if(diff > 0){
				destroyAIFeelingModifier(modifier);
				
				if(prev){
					prev->next = next;
				}else{
					ai->feeling.modifiers = next;
				}
			}
		}
		
		modifier = next;
	}
	
	ai->feeling.mods = mods;
	
	for(i = 0; i < AI_FEELINGSET_COUNT; i++){
		for(j = 0; j < AI_FEELING_COUNT; j++){
			mods.level[j] += ai->feeling.sets[i].level[j];
		}
	}

	for(j = 0; j < AI_FEELING_COUNT; j++){
		F32 level = mods.level[j];
		
		if(level < 0)
			level = 0;
		else if(level > 1)
			level = 1;
		
		ai->feeling.total.level[j] = level;
	}
}

void aiFeelingAddValue(Entity* e, AIVars* ai, AIFeelingSet feelingSet, AIFeelingID feelingID, float amount){
	float* value = &ai->feeling.sets[feelingSet].level[feelingID];
	
	*value += amount;
	
	if(*value < 0)
		*value = 0;
	else if(*value > 1)
		*value = 1;
}

void aiFeelingSetValue(Entity* e, AIVars* ai, AIFeelingSet feelingSet, AIFeelingID feelingID, float amount){
	float* value = &ai->feeling.sets[feelingSet].level[feelingID];
	
	*value = amount;
	
	if(*value < 0)
		*value = 0;
	else if(*value > 1)
		*value = 1;
}

void aiFeelingAddModifier(Entity* e, AIVars* ai, AIFeelingModifier* modifier){
	modifier->next = ai->feeling.modifiers;

	ai->feeling.modifiers = modifier;
	
	AI_LOG(	AI_LOG_FEELING,
			(e,
			"Added ^8%s ^0[^4%1.2f ^0- ^4%1.2f^0] from [^4%1.2f^0s - ^4%1.2f^0s]\n",
			aiFeelingName[modifier->feelingID],
			modifier->level.begin,
			modifier->level.end,
			(float)ABS_TO_SEC(modifier->time.begin - ABS_TIME),
			(float)ABS_TO_SEC(modifier->time.end - ABS_TIME)));
}

void aiFeelingAddModifierEx(	Entity* e,
								AIVars* ai,
								AIFeelingID feelingID,
								float levelBegin,
								float levelEnd,
								unsigned int timeBegin,
								unsigned int timeEnd)
{
	AIFeelingModifier* modifier;
	
	if(	!ai->feeling.trackFeelings ||
		!levelBegin && !levelEnd ||
		timeBegin == timeEnd)
	{
		return;
	}

	modifier = createAIFeelingModifier();

	modifier->feelingID = feelingID;
	modifier->level.begin = levelBegin;
	modifier->level.end = levelEnd;
	modifier->time.begin = timeBegin;
	modifier->time.end = timeEnd;
	
	aiFeelingAddModifier(e, ai, modifier);
}

