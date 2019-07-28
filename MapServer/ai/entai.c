
#include "entaiBehaviorCoh.h"
#include "entaiprivate.h"
#include "groupnetsend.h"
#include "entserver.h"
#include "beacon.h"
#include "beaconPath.h"
#include "beaconPrivate.h"
#include "encounter.h"
#include "motion.h"
#include "character_base.h"
#include "character_target.h"
#include "storyarcinterface.h"
#include "entgen.h"
#include "seq.h"
#include "entity.h"
#include "cmdcommon.h"
#include "character_combat.h"
#include "character_animfx.h"
#include "gridcoll.h"
#include "cmdserver.h"
#include "entaiLog.h"
#include "StringCache.h"
#include "VillainDef.h"
#include "earray.h"
#include "timing.h"
#include "rand.h"

AIState ai_state;

void aiCritterDeathHandler(Entity* e){
	AIVars* ai = ENTAI(e);
	MotionState* motion = e->motion;
	AIPowerInfo* powInfo;

	if(ENTTYPE_PLAYER == ENTTYPE(e))
	{
		aiProximityRemoveEntitysStatusEntries(e, ai, 1);
		return;
	}

	powInfo = aiChoosePower(e, ai, AI_POWERGROUP_POSTDEATH);

	if(powInfo){
		Power* ppow = powInfo->power;

		if(e->pchar->ppowQueued){
			character_KillQueuedPower(e->pchar);
		}
		if(!ppow->bActive && ppow->fTimer == 0){
			aiUsePower(e, powInfo, NULL);
		}
	}

	e->move_type = MOVETYPE_WALK;
	motion->input.flying = 0;
	motion->falling = 1;

	aiMotionSwitchToNone(e, ai, "I died.");

	removeEntFromSpawnArea(e);

	aiDiscardFollowTarget(e, "I died.", false);
	aiDiscardAttackTarget(e, "I died.");

	if (ai->power.toggle.curBuff)
	{
		aiClearCurrentBuff(&ai->power.toggle.curBuff);
	}
	if (ai->power.toggle.curDebuff)
	{
		aiClearCurrentBuff(&ai->power.toggle.curDebuff);
	}
	if (ai->power.toggle.curPaced)
	{
		aiClearCurrentBuff(&ai->power.toggle.curPaced);
	}

	//if(!ai->doNotChangePowers){
	//	aiDiscardCurrentPower(e);
	//}

	if(e->pchar->erLastDamage != 0 && erGetEnt(e->pchar->erLastDamage)){
		AI_LOG(	AI_LOG_COMBAT,
				(e,
				"Oh my god, %s ^0killed %s^0!  You bastard!\n",
				AI_LOG_ENT_NAME(erGetEnt(e->pchar->erLastDamage)),
				AI_LOG_ENT_NAME(e)));
	}

	// Destroy the avoid information.
	
	aiRemoveAvoidNodes(e, ai);
	
	ZeroStruct(&ai->avoid.placed);
}

void aiClearBeaconReferences()
{
	int i;
	for(i = 0; i < MAX_ENTITIES_PRIVATE; i++){
		Entity* e = validEntFromId(i);
		if(e){
			AIVars* ai = ENTAI_BY_ID(i);

			e->nearestBeacon = NULL;

			entMotionFreeColl(e);

			if(ai){
				if(	ai->followTarget.type == AI_FOLLOWTARGET_BEACON){
					ai->followTarget.type = AI_FOLLOWTARGET_NONE;
					ai->followTarget.beacon = NULL;
					ai->followTarget.pos = NULL;
				}

				if(ai->navSearch){
					clearNavSearch(ai->navSearch);
				}

				switch(ENTTYPE(e)){
					case ENTTYPE_CAR:
						ai->car.curveArray = NULL;
						break;
					case ENTTYPE_NPC:
						ai->npc.lastWireBeacon = NULL;
						break;
				}
			}
		}
	}
}

void aiSetActivity(Entity* e, AIVars* ai, AIActivity activity){
	if(aiBehaviorGetActivity(e) != activity){
		EntType entType = ENTTYPE(e);
		AIActivity oldActivity = aiBehaviorGetActivity(e);

		aiBehaviorSetActivity(e, activity);
		ai->time.setActivity = ABS_TIME;

		switch(entType){
			case ENTTYPE_CRITTER:{
				aiCritterActivityChanged(e, ai, oldActivity);
				break;
			}

			case ENTTYPE_NPC:{
				aiNPCActivityChanged(e, ai, oldActivity);
				break;
			}
		}
	}
}

void aiSetShoutRadius(Entity* e, float radius){
	if(e && ENTAI(e)){
		ENTAI(e)->shoutRadius = radius;
	}
}

void aiSetShoutChance(Entity* e, int chance){
	if(e && ENTAI(e)){
		ENTAI(e)->shoutChance = chance;
	}
}

float aiGetJumpHeight(Entity* e){
	AIVars* ai = e ? ENTAI(e) : NULL;
	
	if(ai && ai->followTarget.jumpHeightOverride > 0.0f){
		return ai->followTarget.jumpHeightOverride;
	}
	else if(e->pchar){
		if(e->pchar->attrCur.fStunned > 0.0f){
			return 0.0f;
		}else{
			return 10.0f + e->pchar->attrCur.fJumpHeight * 10.0f;
		}
	}
	else{
		return 0.0f;
	}
}

void aiSetFlying(Entity* e, AIVars* ai, int isFlying){
	if(ai->isFlying != isFlying)
	{
		if(!isFlying || ai->canFly){
			AIPowerInfo* flyEffect = aiChooseTogglePower(e, ai, AI_POWERGROUP_FLY_EFFECT);
			MotionState* motion = e->motion;

			if(flyEffect){
				aiSetTogglePower(e, flyEffect, isFlying);

				if(isFlying){
					AI_LOG(	AI_LOG_PATH,
							(e,
							"^2Enabling fly effect.\n"));
				}else{
					AI_LOG(	AI_LOG_PATH,
							(e,
							"^1Disabling fly effect.\n"));
				}
			}

			motion->falling = !isFlying;
			motion->input.flying = isFlying;

			ai->isFlying = isFlying;

			e->move_type = isFlying ? 0 : MOVETYPE_WALK;
		}
	}
}

void aiSetTimeslice(Entity* e, int timeslice){
	AIVars* ai = ENTAI(e);
	if(ai){
		if(timeslice < 0)
			timeslice = 0;
		else if(timeslice > 100)
			timeslice = 100;

		if(ENTTYPE(e) == ENTTYPE_CRITTER){
			if(ai->timeslice && !timeslice){
				ai_state.activeCritters--;
			}
			else if(!ai->timeslice && timeslice){
				ai_state.activeCritters++;
			}
		}

		ai_state.totalTimeslice += timeslice - ai->timeslice;

		ai->timeslice = timeslice;
	}
}

void aiSetFOVAngle(Entity* e, AIVars* ai, F32 fovAngle){
	if(fovAngle < 0)
		fovAngle = 0;
	else if(fovAngle > RAD(180))
		fovAngle = RAD(180);

	if(ai->fov.angle != fovAngle){
		ai->fov.angle = fovAngle;
		ai->fov.angleCosine = cos(fovAngle);
	}
}
static F32 aiAOEModifier(Entity *e, AIVars *ai, AIPowerInfo *aiPower)
{
	const BasePower *bPower = aiPower->power->ppowBase;
	switch (bPower->eEffectArea)
	{
	case kEffectArea_Character:
		aiPower->debugTargetCount = 1.0f;
		return ai->AOEUsage.fSingleTargetPreference;
	case kEffectArea_Location:
		{
			int count = 0;
			if (ai->attackTarget.entity && e && e->pchar)
			{
				AIProxEntStatus *status;
				for (status = ai->proxEntStatusTable->statusHead; status; status = status->tableList.next) {
					//	valid entity
					//	either the power is an attack and the target is not its ally, or the power is not an attack and the target is its ally
					//	and in range
					if (status->entity && (status->entity->pchar && (!aiPower->isAttack ^ (e->pchar->iAllyID != status->entity->pchar->iAllyID)) ) &&
						distance3Squared(ENTPOS(status->entity), ENTPOS(ai->attackTarget.entity)) < SQR(bPower->fRadius) )
					{
						count++;
					}
				}
			}
			aiPower->debugTargetCount = count;
			return MAX(count,1)*ai->AOEUsage.fAOEPreference;
		}
	case kEffectArea_Cone:
		{
			int count = 0;
			if (e && e->pchar)
			{
				AIProxEntStatus *status;
				for (status = ai->proxEntStatusTable->statusHead; status; status = status->tableList.next) {
					//	valid entity
					//	either the power is an attack and the target is not its ally, or the power is not an attack and the target is its ally
					//	and in range
					if (status->entity && (status->entity->pchar && (!aiPower->isAttack ^ (e->pchar->iAllyID != status->entity->pchar->iAllyID))) && 
						character_CharacterIsReachable(e, status->entity) &&
						distance3Squared(ENTPOS(status->entity), ENTPOS(e)) < SQR(bPower->fRadius) )
					{
						count++;
					}
				}
			}	
			{
				//	This formula was taken from the designer spreadsheet on expected dmg on arc based attacks
				//	We could be more accurate by checking if the target is in the cone 
				//	but that stuff is a bit iffy since it can change anyways by the time the power is fired, 
				//	so why worry about being so accurate at decision time. just estimate how many would be effected

				F32 expectedRatioOfHits = ((1 + (bPower->fRange*0.15))- ((((bPower->fRange/6)*0.011)/5)*(2*PI-bPower->fArc))) / (1+(bPower->fRange*0.15));
				aiPower->debugTargetCount = MAX(expectedRatioOfHits * count, 1.0f);
				return aiPower->debugTargetCount * ai->AOEUsage.fAOEPreference;
			}
		}
	default:
		{
			int count = 0;
			if (e && e->pchar)
			{
				AIProxEntStatus *status;
				for (status = ai->proxEntStatusTable->statusHead; status; status = status->tableList.next) {
					//	valid entity
					//	either the power is an attack and the target is not its ally, or the power is not an attack and the target is its ally
					//	and in range
					if (status->entity && (status->entity->pchar && (!aiPower->isAttack ^ (e->pchar->iAllyID != status->entity->pchar->iAllyID))) && 
						character_CharacterIsReachable(e, status->entity) &&
						distance3Squared(ENTPOS(status->entity), ENTPOS(e)) < SQR(bPower->fRadius) )
					{

						count++;
					}
				}
			}
			aiPower->debugTargetCount = count;
			return MAX(count,1)*ai->AOEUsage.fAOEPreference;
		}
	}
}
AIPowerInfo* aiChooseAttackPower(	Entity* e,
									AIVars* ai,
									F32 minRange,
									F32 maxRange,
									int requireRange,
									int groupFlags,
									int stanceID)
{
	AIPowerInfo* info;
	int foundInRange = 0;
	float bestOffset = FLT_MAX;
	float preferenceList[100];
	float preferenceRange = 0.0f;
	AIPowerInfo* bestArray[100];
	int bestCount = 0;
	Character* pchar = e->pchar;

	if( ai->doNotUsePowers )
		return 0;

	for(info = ai->power.list; info; info = info->next)
	{
		Power* power = info->power;
		float range;

		info->debugPreference = 0.0f;
		info->debugTargetCount = 0.0f;
		if(	!info->isAttack ||
			power->fTimer > 2.0f ||
			groupFlags && !(info->groupFlags & groupFlags) ||
			stanceID && info->stanceID && stanceID != info->stanceID ||
			info->doNotChoose.duration && ABS_TIME_SINCE(info->doNotChoose.startTime) < info->doNotChoose.duration)
		{
			continue;
		}

		//If I am a pet and this is a pet power I only used when commanded, don't use it
		if( e->erOwner && info->isSpecialPetPower && !ai->wasToldToUseSpecialPetPower )
			continue;
		if( info->neverChoose )
			continue;
		if(info->isNearGround && ai->isFlying)
			continue;

		range = aiGetPowerRange(pchar, info);

		if(range >= minRange)
		{
			if(range <= maxRange || ai->power.useRangeAsMaximum)
			{
				float preference = aiAOEModifier(e, ai, info) * ((info->critterOverridePreferenceMultiplier >= 0.0f) ? info->critterOverridePreferenceMultiplier : info->power->ppowBase->fPreferenceMultiplier);
				//	the only way preference can be negative is if their is no override multiplier, and the original power def multiplier is 0
				preference *= 1.f/(1+power->fTimer*4);
				if (preference > 0.0f)
				{
					preferenceList[bestCount] = preference;
					preferenceRange += preference;
					bestArray[bestCount++] = info;
					if (!foundInRange)
						foundInRange = 1;
				}
				info->debugPreference = preference;
			}
			else if(!foundInRange && !requireRange)
			{
				if(bestCount < ARRAY_SIZE(bestArray))
				{
					float preference = aiAOEModifier(e, ai, info) * ((info->critterOverridePreferenceMultiplier >= 0.0f) ? info->critterOverridePreferenceMultiplier : info->power->ppowBase->fPreferenceMultiplier);
					//	the only way preference can be negative is if their is no override multiplier, and the original power def multiplier is 0
					preference *= 1.f/(1+power->fTimer*4);
					if (preference > 0.0f)
					{
						preferenceList[bestCount] = preference;
						preferenceRange += preference;
						bestArray[bestCount++] = info;
					}
					info->debugPreference = preference;
				}
				else
				{
					Errorf("AI Best Array overflowed.");
				}
			}
		}
	}

	if(!bestCount) // this time ignore ranges
	{
		for(info = ai->power.list; info; info = info->next)
		{
			Power* power = info->power;

			if(	!info->isAttack ||
				power->fTimer > 2.0f ||
				groupFlags && !(info->groupFlags & groupFlags) ||
				stanceID && info->stanceID && stanceID != info->stanceID ||
				info->doNotChoose.duration && ABS_TIME_SINCE(info->doNotChoose.startTime) < info->doNotChoose.duration)
			{
				continue;
			}

			//If I am a pet and this is a pet power I only used when commanded, don't use it
			if( e->erOwner && info->isSpecialPetPower && !ai->wasToldToUseSpecialPetPower )
			{
				continue;
			}
			if( info->neverChoose )
				continue;
			if(info->isNearGround && ai->isFlying)
				continue;

			if(bestCount < ARRAY_SIZE(bestArray))
			{
				float preference = aiAOEModifier(e, ai, info) * ((info->critterOverridePreferenceMultiplier >= 0.0f) ? info->critterOverridePreferenceMultiplier : info->power->ppowBase->fPreferenceMultiplier);
				//	the only way preference can be negative is if their is no override multiplier, and the original power def multiplier is 0

				preference *= 1.f/(1+power->fTimer*4);

				if (preference > 0.0f)
				{
					preferenceList[bestCount] = preference;
					preferenceRange += preference;
					bestArray[bestCount++] = info; 
				}
				info->debugPreference = preference;
			}
			else
			{
				Errorf("AI Best Array overflowed.");
			}
		}
	}

	if(bestCount)
	{
		//	walk along the preference list;
		int itr = 0;
		float randomPower = randomPositiveF32() * preferenceRange;
		while ((randomPower > preferenceList[itr]) && (itr < (bestCount-1)))
		{
			randomPower -=preferenceList[itr];
			itr++;
		}
		return bestArray[itr];
	}

	return NULL;
}

AIPowerInfo* aiChooseTogglePower(Entity* e, AIVars* ai, int groupFlags)
{
	return aiChoosePowerEx(e, ai, groupFlags, CHOOSEPOWER_TOGGLEONLY);
}

AIPowerInfo* aiChoosePower(Entity* e, AIVars* ai, int groupFlags)
{
	assert(groupFlags);
	return aiChoosePowerEx(e, ai, groupFlags, CHOOSEPOWER_ANY);
}

AIPowerInfo* aiChoosePowerEx(Entity* e, AIVars* ai, int groupFlags, ChoosePowerChoiceFlags flags){
	AIPowerInfo* info;
	AIPowerInfo* bestArray[100];
	float preferenceRange = 0.0f;
	float preferenceList[100];
	int bestCount = 0;

	if( ai->doNotUsePowers )
		return 0;

	for(info = ai->power.list; info && bestCount < ARRAY_SIZE(bestArray); info = info->next)
	{
		if(!groupFlags || info->groupFlags & groupFlags)
		{
			// Check if this power has been temporarily disabled.
			float preference;
			if(	info->doNotChoose.duration &&
				ABS_TIME_SINCE(info->doNotChoose.startTime) < info->doNotChoose.duration)
			{
				continue;
			}

			//If I am a pet and this is a pet power I only used when commanded, don't use it
			if( e->erOwner && info->isSpecialPetPower && !ai->wasToldToUseSpecialPetPower )
			{
				continue;
			}
			if( info->neverChoose )
				continue;

			// Find the correct toggle type.
			if(	flags == CHOOSEPOWER_NOTTOGGLE && info->isToggle ||
				flags == CHOOSEPOWER_TOGGLEONLY && !info->isToggle)
			{
				continue;
			}
			preference = aiAOEModifier(e, ai, info) * ((info->critterOverridePreferenceMultiplier >= 0.0f) ? info->critterOverridePreferenceMultiplier : info->power->ppowBase->fPreferenceMultiplier);
			//	the only way preference can be negative is if their is no override multiplier, and the original power def multiplier is 0
			if (preference > 0.0f)
			{
				preferenceList[bestCount] = preference;
				preferenceRange += preference;
				bestArray[bestCount++] = info; 
			}
			info->debugPreference = preference;
		}
	}

	if(bestCount)
	{
		int itr = 0;
		float randomPower = randomPositiveF32()*preferenceRange;
		while ((randomPower > preferenceList[itr]) && (itr < (bestCount-1)))
		{
			randomPower -=preferenceList[itr];
			itr++;
		}
		return bestArray[itr];
	}

	return NULL;
}

int aiIsLegalAttackTarget(Entity* e, Entity* target){
	if(!target || e == target){
		return 0;
	}

	if(!(entity_state[target->owner] & ENTITY_IN_USE))
		return 0;

	if(	ENTAI(target) &&
		character_TargetMatchesType(e->pchar, target->pchar, kTargetType_Foe, false) &&
		!ENTAI(target)->doNotAggroCritters &&
		!target->untargetable)
	{
		return 1;
	}

	return 0;
}

int aiIsMyFriend(Entity* me, Entity* notMe){
	if(!me || !notMe || !me->pchar || !notMe->pchar)
		return 0;

	return	character_TargetIsFriend(me->pchar,notMe->pchar);
}

int aiIsMyEnemy(Entity* me, Entity* notMe){
	if(!me || !notMe || !me->pchar || !notMe->pchar)
		return 0;

	return	character_TargetIsFoe(me->pchar,notMe->pchar);
}

// This should always be called to set the attack target.

void aiSetAttackTarget(Entity* e, Entity* target, AIProxEntStatus* status, const char* reason, int doNotAlertEncounter, int doNotForgetTarget ){
	AIVars* ai = ENTAI(e);


	if(target != ai->attackTarget.entity){
		// Remove self from the old target's list if there is one.

		if(ai->attackTarget.entity){
			AIVars* aiTarget = ENTAI(ai->attackTarget.entity);

			if(aiTarget){
				AI_LOG(	AI_LOG_COMBAT,
						(e,
						reason ? "^1DISENGAGING ^0target: %s ^0(^1%s^0)\n" : "^1DISENGAGING ^0target: %s\n",
						AI_LOG_ENT_NAME(ai->attackTarget.entity),
						reason));

				if(ai->attackerInfo.prev){
					ai->attackerInfo.prev->next = ai->attackerInfo.next;
				}else{
					assert(aiTarget->attackerList.head == &ai->attackerInfo);

					aiTarget->attackerList.head = ai->attackerInfo.next;
				}

				if(ai->attackerInfo.next){
					ai->attackerInfo.next->prev = ai->attackerInfo.prev;
				}else{
					assert(aiTarget->attackerList.tail == &ai->attackerInfo);

					aiTarget->attackerList.tail = ai->attackerInfo.prev;
				}

				assert(aiTarget->attackerList.count > 0);

				aiTarget->attackerList.count--;
			}
		}

		if(target && !ENTAI(target)){
			target = NULL;
			status = NULL;
		}

		ai->attackTarget.entity = target;

		if(target && !status){
			status = aiGetProxEntStatus(ai->proxEntStatusTable, target, 1);
		}
		else if(status){
			if(target){
				if(!devassert(status->entity == target && status->table->owner.entity == e)){
					status = aiGetProxEntStatus(ai->proxEntStatusTable, target, 1);
				}
			}else{
				status = NULL;
			}
		}
		
		if (status && doNotForgetTarget)
			status->neverForgetMe = 1;

		ai->attackTarget.status = status;

		ai->attackTarget.hitCount = 0;

		ai->attackTarget.time.set = ABS_TIME;

		ai->canSeeTarget = 0;
		ai->canHitTarget = 0;

		// Add self to the new target's list, if there is one.

		if(target){
			AIVars* aiTarget = ENTAI(target);

			// Reset the LOS check time so that it will get updated ASAP.

			ai->time.checkedLOS = 0;

			if(aiTarget->attackerList.tail){
				aiTarget->attackerList.tail->next = &ai->attackerInfo;
			}else{
				aiTarget->attackerList.head = &ai->attackerInfo;
			}

			ai->attackerInfo.next = NULL;
			ai->attackerInfo.prev = aiTarget->attackerList.tail;

			aiTarget->attackerList.tail = &ai->attackerInfo;
			aiTarget->attackerList.count++;

			AI_LOG(	AI_LOG_COMBAT,
					(e,
					"^2ENGAGING ^0target: %s\n",
					AI_LOG_ENT_NAME(target)));

			// Tell encounter system that I saw someone.

			if(!doNotAlertEncounter && !ai->teamMemberInfo.alerted){
				AI_LOG(	AI_LOG_ENCOUNTER,
						(e,
						"^2ALERTING ^dself to presence of %s\n",
						AI_LOG_ENT_NAME(target)));

    			EncounterAISignal(e, ENCOUNTER_ALERTED);

				ai->teamMemberInfo.alerted = 1;
			}
		}
		else if(!e->erCreator){
			// Only non-created entities (anything except pets) should become unalerted.
			
			EncounterAISignal(e, ENCOUNTER_NOTALERTED);

			ai->teamMemberInfo.alerted = 0;
		}

		// Let the combat and client know about the current target
		// (OK if target is NULL).
		if(e->pchar){
			e->pchar->erTargetInstantaneous = erGetRef(target);
			e->target_update = true;
		}
	}
}

void aiDiscardAttackTarget(Entity* e, const char* reason){
	AIVars* ai = ENTAI(e);

	if(	ai->followTarget.type == AI_FOLLOWTARGET_ENTITY &&
		ai->followTarget.reason == AI_FOLLOWTARGETREASON_ATTACK_TARGET)
	{
		aiDiscardFollowTarget(e, "Was attack target, not anymore.", false);
	}

	if(!ai->attackTarget.entity)
		return;

	if (ai->brain.type == AI_BRAIN_COT_BEHEMOTH) {
		if (ai->brain.cotbehemoth.statusPrimary &&
			ai->brain.cotbehemoth.statusPrimary->entity == ai->attackTarget.entity)
		{
			ai->brain.cotbehemoth.statusPrimary = NULL;
		}
	}

	aiSetAttackTarget(e, NULL, NULL, reason, 0, 0);
}

// This should always be called to get the attacker count.

int aiGetAttackerCount(Entity* e){
	if(e){
		AIVars* ai = ENTAI(e);

		if(ai){
			return ai->attackerList.count;
		}
	}

	return 0;
}

// Sets the current power.

bool aiSetCurrentPower(Entity* e, AIPowerInfo* info, int enterStance, int overrideDoNotUsePowersFlag)
{
	AIVars* ai = ENTAI(e);

	if (ai->doNotUsePowers && !overrideDoNotUsePowersFlag)
		return false;

	if(ai->power.current && (ai->power.current == info))
		return false;

	if(ai->power.current){
		AI_LOG(	AI_LOG_COMBAT,
				(e, "^1Disarming power: ^5%s^0. ",
				ai->power.current->power->ppowBase->pchName));

		if(enterStance){
			character_EnterStance(e->pchar, NULL, true);
		}
	}

	ai->power.current = info;
	ai->time.setCurrentPower = ABS_TIME;

	if(info){
		AI_LOG(	AI_LOG_COMBAT,
				(e, "^2Arming power: ^5%s^0.",
				info->power->ppowBase->pchName));

		info->time.lastCurrent = ABS_TIME;

		if(info->teamInfo){
			info->teamInfo->time.lastCurrent = ABS_TIME;
		}

		if(enterStance){
			character_EnterStance(e->pchar, info->power, true);
		}

		if(info->stanceID > 0 && (ai->power.lastStanceID != info->stanceID) && !info->power->bDontSetStance){
			ai->time.setCurrentStance = ABS_TIME;

			ai->power.lastStanceID = info->stanceID;
		}
	}else{
		if(enterStance){
			character_EnterStance(e->pchar, NULL, true);
		}
		ai->power.lastStanceID = 0;
	}

	AI_LOG(AI_LOG_COMBAT, (e, "\n"));

	return true;
}

void aiDiscardCurrentPower(Entity* e)
{
	aiSetCurrentPower(e, NULL, 1, 1);
}

int aiPowerTargetAllowed(Entity* e, AIPowerInfo* info, Entity* target, bool buffCheck)
{
	AIVars* ai = ENTAI(e);

	if (ai->doNotUsePowers || (target && target->untargetable))
		return 0;

	if(target)
	{
		if (buffCheck && target->pchar) // don't waste buffs on the untouchable, but attacks are fine
		{
			if (e != target)	//	don't check for self casting powers
			{
				if (target->pchar->attrCur.fUntouchable)
				{
					//	can't shoot through untouchable
					if (!(info->power && info->power->ppowBase && info->power->ppowBase->bShootThroughUntouchable))
						return 0;
				}
			}
		}
		if (eaSize(&info->allowedTargets))
		{
			int i;
			const char* targetType;

			if(target->villainDef)
				targetType = allocFindString(target->villainDef->name);
			else if(target->db_id != -1)
				targetType = allocFindString("Player");

			if(targetType)
			{
				for(i = eaSize(&info->allowedTargets)-1; i >= 0; i--)
				{
					if(info->allowedTargets[i] == targetType)
						return 1;
				}
			}
			return 0;
		}
	}

	return 1;
}

int aiUsePower(Entity* e, AIPowerInfo* info, Entity* target)
{
	AIVars* ai = ENTAI(e);
	if( ai->doNotUsePowers )
		return 0;

	if(ENTAI(e)->power.maxPets && info->groupFlags & AI_POWERGROUP_PET && ENTAI(e)->power.numPets >= ENTAI(e)->power.maxPets)
		return 0;

	if(!aiPowerTargetAllowed(e, info, target, false))
		return 0;



	info->time.lastUsed = ABS_TIME;
	info->usedCount++;

	if(info->teamInfo){
		info->teamInfo->time.lastUsed = ABS_TIME;
		info->teamInfo->usedCount++;
	}

	return character_ActivatePowerOnCharacter(e->pchar, target ? target->pchar : NULL, info->power);
}

int aiUsePowerOnLocation(Entity* e, AIPowerInfo* info, const Vec3 target)
{
	AIVars* ai = ENTAI(e);
	info->time.lastUsed = ABS_TIME;
	info->usedCount++;

	if(info->teamInfo){
		info->teamInfo->time.lastUsed = ABS_TIME;
		info->teamInfo->usedCount++;
	}

	if( ai->doNotUsePowers )
		return 0;

	if(ENTAI(e)->power.maxPets && info->groupFlags & AI_POWERGROUP_PET && ENTAI(e)->power.numPets >= ENTAI(e)->power.maxPets)
		return 0;

	character_ActivatePower(e->pchar, erGetRef(e), target, info->power);

	return 1;
}

void aiSetTogglePower(Entity* e, AIPowerInfo* info, int set)
{
	AIVars* ai = ENTAI(e);
	Character* pchar = e->pchar;

	if( ai->doNotUsePowers )
		return;

	info->time.lastUsed = ABS_TIME;

	if(set){
		info->usedCount++;
	}

	if(info->teamInfo){
		info->teamInfo->time.lastUsed = ABS_TIME;

		if(set){
			info->teamInfo->usedCount++;
		}
	}

	character_ShutOffAndRemoveTogglePower(pchar, info->power);

	if(set){
		character_ActivatePower(pchar, erGetRef(e), ENTPOS(e), info->power);

		AI_LOG(	AI_LOG_COMBAT,
				(e,
				"^2TOGGLED ON^0: ^5%s\n",
				info->power->ppowBase->pchName));
	}else{
		AI_LOG(	AI_LOG_COMBAT,
				(e,
				"^1TOGGLED OFF^0: ^5%s\n",
				info->power->ppowBase->pchName));
	}
}

float aiGetPowerRange(Character* pchar, AIPowerInfo* info){
	float range = character_PowerRange(pchar, info->power);

	if(info->power->ppowBase->eEffectArea == kEffectArea_Cone)
		range *= 0.5f;

	if(range == 0)
		range = power_GetRadius(info->power) * 0.5f;

	return range + info->sourceRadius;
}


float aiGetPreferredAttackRange(Entity * e, AIVars * ai)
{
	F32 dist;
	AIVars * aiTarget = 0;

	if(ai->attackTarget.entity)
		aiTarget = ENTAI(ai->attackTarget.entity);

	// determine how far to be from target -- at max range if there is a preferred power
	if(ai->power.preferred)
		dist = aiGetPowerRange(e->pchar, ai->power.preferred);
	else
		dist = 10;

	if(aiTarget)
		dist += aiTarget->collisionRadius;
	else
		dist = 0;

	return dist;
}

// Visibility functions.

int aiPointCanSeePoint(Vec3 src, float srcHeight, Vec3 dst, float dstHeight, float radius){
	CollInfo coll;
	Vec3 srcPos;
	Vec3 dstPos;
	int canSee;

	copyVec3(src, srcPos);
	copyVec3(dst, dstPos);

	srcPos[1] += srcHeight;
	dstPos[1] += dstHeight;

	PERFINFO_AUTO_START("los", 1);

	canSee = !collGrid(NULL, srcPos, dstPos, &coll, radius, COLL_NOTSELECTABLE | COLL_HITANY | COLL_BOTHSIDES);

	PERFINFO_AUTO_STOP();

	return canSee;
}

int aiGetPointGroundOffset(const Vec3 src, float offset, float max, Vec3 out){
	CollInfo coll;
	Vec3 srcPos;
	Vec3 dstPos;

	copyVec3(src, srcPos);
	copyVec3(src, dstPos);

	srcPos[1] += offset;
	dstPos[1] -= max;

	PERFINFO_AUTO_START("groundOffset", 1);

	if(collGrid(NULL, srcPos, dstPos, &coll, 0, COLL_NOTSELECTABLE | COLL_DISTFROMSTART)){
		copyVec3(coll.mat[3], out);

		PERFINFO_AUTO_STOP();

		return 1;
	}

	PERFINFO_AUTO_STOP();

	return 0;
}

int aiEntityCanSeeEntity(Entity* src, Entity* dst, float distance){
	if(distance3Squared(ENTPOS(src), ENTPOS(dst)) < SQR(distance)){
		return character_CharacterIsReachable(src, dst);
		//return aiPointCanSeePoint(posPoint(src), 5, posPoint(dst), 5, 0);
	}

	return 0;
}

float aiYawBetweenPoints(Vec3 p1, Vec3 p2){
	Vec3 vec;

	subVec3(p2, p1, vec);

	return getVec3Yaw(vec);
}

int aiPointInViewCone(const Mat4 mat, const Vec3 pos, F32 maxCosine){
	float cosBetween;
	Vec3 toMe;

	if(maxCosine == -1)
		return 1;

	subVec3(pos, mat[3], toMe);

	cosBetween = dotVec3(mat[2], toMe);

	// Assume the mat[2] vector has length 1, so only divide by the length of the vector to me.

	cosBetween /= lengthVec3(toMe);

	// Check if the cosine is greater than the cosine of 120 degrees.

	return cosBetween > maxCosine;
}

#include "seqstate.h" // GGFIXME: this probably belongs somewhere else

void aiTurnBodyByPYR(Entity* e, AIVars* ai, Vec3 pyr)
{
	if(!sameVec3(pyr, ai->pyr)) 
	{
		Mat3 newmat;

		if(SAFE_MEMBER(e->seq, type->turnSpeed) > 0.f)
		{
			F32 maxDiff = RAD(e->seq->type->turnSpeed) * e->timestep;
			F32 diff = subAngle(pyr[1], ai->pyr[1]);

			if(diff < 0.f)
			{
				ai->pyr[1] = addAngle(ai->pyr[1], MAX(-maxDiff, diff));
				SETB(e->seq->state, STATE_TURNRIGHT);
			}
			else if(diff > 0.f)
			{
				ai->pyr[1] = addAngle(ai->pyr[1], MIN(maxDiff, diff));
				SETB(e->seq->state, STATE_TURNLEFT);
			}

			ai->pyr[0] = pyr[0];
			ai->pyr[2] = pyr[2];
		}
		else if (ai->turnSpeed > 0.0)
		{
			F32 maxDiff = RAD(ai->turnSpeed) * e->timestep; 
			int i; 

			for(i = 0; i < 3; i++)
			{
				F32 diff = subAngle(pyr[i], ai->pyr[i]);

				if(diff < -maxDiff){
					diff = -maxDiff;
				}
				else if(diff > maxDiff){
					diff = maxDiff;
				}

				ai->pyr[i] = addAngle(ai->pyr[i], diff);
			}
		}
		else
		{
			copyVec3(pyr, ai->pyr);
		}

		createMat3YPR(newmat, ai->pyr); 
		entSetMat3(e, newmat);

		//printf("setting yaw: %f\n", ai->pyr[1]);
	}
}

void aiTurnBodyByDirection(Entity* e, AIVars* ai, Vec3 direction){
	Vec3 pyr;

	getVec3YP(direction, &pyr[1], &pyr[0]);

	pyr[2] = 0;

	aiTurnBodyByPYR(e, ai, pyr);
}

static struct {
	Vec3 pos;
	int maxCount;
	int curCount;
	float rxz;
	float ry;
	float minRangeSQR;
	float maxRangeSQR;
	Beacon** array;
} staticBeaconSearchData;

static void aiFindBeaconInRangeHelper(Array* beaconArray, AIFindBeaconApproveFunction approveFunc){
	int i;

	for(i = 0; i < beaconArray->size; i++){
		Beacon* b = beaconArray->storage[i];

		if(b->gbConns.size || b->rbConns.size){
			float distSQR = distance3Squared(posPoint(b), staticBeaconSearchData.pos);

			if(	staticBeaconSearchData.curCount < staticBeaconSearchData.maxCount &&
				distSQR >= staticBeaconSearchData.minRangeSQR &&
				distSQR <= staticBeaconSearchData.maxRangeSQR)
			{
				if(!approveFunc || approveFunc(b)){
					staticBeaconSearchData.array[staticBeaconSearchData.curCount++] = b;
				}
			}
		}
	}
}


Beacon* aiFindBeaconInRange(Entity* e,
							AIVars* ai,
							const Vec3 centerPos,
							float rxz,
							float ry,
							float minRange,
							float maxRange,
							AIFindBeaconApproveFunction approveFunc)
{
	Beacon* array[1000];

	ai->time.foundPath = ABS_TIME;

	staticBeaconSearchData.array = array;
	staticBeaconSearchData.maxCount = ARRAY_SIZE(array);
	staticBeaconSearchData.curCount = 0;
	staticBeaconSearchData.rxz = rxz;
	staticBeaconSearchData.ry = ry;
	staticBeaconSearchData.minRangeSQR = SQR(minRange);
	staticBeaconSearchData.maxRangeSQR = SQR(maxRange);
	copyVec3(ENTPOS(e), staticBeaconSearchData.pos);

	beaconForEachBlock(centerPos, rxz, ry, rxz, aiFindBeaconInRangeHelper, approveFunc);

	if(staticBeaconSearchData.curCount){
		int i = rand() % staticBeaconSearchData.curCount;

		return array[i];
	}

	return NULL;
}

void aiRemoveAvoidNodes(Entity* e, AIVars* ai){
	BeaconAvoidNode* node;
	
	for(node = ai->avoid.placed.nodes; node;){
		BeaconAvoidNode* next = node->entityList.next;
		beaconDestroyAvoidNode(node);
		node = next;
	}
	
	ai->avoid.placed.nodes = NULL;
}

static void aiAddAvoidNode(Entity* e, AIVars* ai, Beacon* beacon){
	BeaconAvoidNode* node = beaconAddAvoidNode(beacon, e);
	
	node->entityList.next = ai->avoid.placed.nodes;
	ai->avoid.placed.nodes = node;
}

static void aiUpdateAvoidHelper(Array* beacons, Entity* e){
	AIVars* ai = ENTAI(e);
	F32 avoidRadius = max(70.0, ai->avoid.maxRadius);
	F32 radiusSQR = SQR(avoidRadius);
	int count = beacons->size;
	int i;
	
	for(i = 0; i < count; i++){
		Beacon* b = beacons->storage[i];
		
		if(distance3Squared(posPoint(b), ENTPOS(e)) <= radiusSQR){
			aiAddAvoidNode(e, ai, b);
		}
	}	
}

void aiUpdateAvoid(Entity* e, AIVars* ai){
	if(	*(int*)&ai->avoid.maxRadius != *(int*)&ai->avoid.placed.maxRadius
		||
		!sameVec3((int*)ENTPOS(e), (int*)ai->avoid.placed.pos))
	{
		aiRemoveAvoidNodes(e, ai);
		
		if(*(int*)&ai->avoid.maxRadius){
			beaconForEachBlock(ENTPOS(e), ai->avoid.maxRadius, ai->avoid.maxRadius, ai->avoid.maxRadius, aiUpdateAvoidHelper, e);
		}
		
		ai->avoid.placed.maxRadius = ai->avoid.maxRadius;
		copyVec3(ENTPOS(e), ai->avoid.placed.pos);
	}
}

int aiShouldAvoidEntity(Entity* e, AIVars* ai, Entity* target, AIVars* aiTarget){
	AIAvoidInstance* avoid;
	S32 entityLevel = (e->pchar->bIgnoreCombatMods ? (MAX_PLAYER_SECURITY_LEVEL+4) : e->pchar->iCombatLevel);
	S32 targetEntityLevel = target->pchar->bIgnoreCombatMods ? (MAX_PLAYER_SECURITY_LEVEL+4) : target->pchar->iCombatLevel;
	S32 levelDiff = entityLevel - targetEntityLevel;
	
	for(avoid = aiTarget->avoid.list; avoid; avoid = avoid->next){
		if(levelDiff <= avoid->maxLevelDiff){
			return 1;
		}
	}
	
	return 0;
}

int aiShouldAvoidBeacon(Entity* e, AIVars* ai, Beacon* beacon){
	BeaconAvoidNode* node;
	
	if(!e->pchar){
		return 0;
	}
	
	for(node = beacon->avoidNodes; node; node = node->beaconList.next){
		Entity* target = node->entity;
		AIVars* aiTarget = ENTAI(target);
		S32 entityLevel = (e->pchar->bIgnoreCombatMods ? (MAX_PLAYER_SECURITY_LEVEL+4) : e->pchar->iCombatLevel);
		S32 targetEntityLevel = target->pchar->bIgnoreCombatMods ? (MAX_PLAYER_SECURITY_LEVEL+4) : target->pchar->iCombatLevel;
		S32 levelDiff = entityLevel - targetEntityLevel;
		F32 dist = distance3(posPoint(beacon), ENTPOS(target));
		AIAvoidInstance* avoid;
		
		for(avoid = aiTarget->avoid.list; avoid; avoid = avoid->next){
			if(dist <= avoid->radius && levelDiff <= avoid->maxLevelDiff){
				return 1;
			}
		}
	}
	
	return 0;
}

