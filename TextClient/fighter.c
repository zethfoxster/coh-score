#include "fighter.h"
#include "mover.h"
#include <float.h>
#include "trayClient.h"
#include "character_base.h"
#include "character_attribs.h"
#include "character_target.h"
#include "entclient.h"
#include "entVarUpdate.h"
#include "itemselect.h"
#include "pmotion.h"
#include "player.h"
#include "EArray.h"
#include "powers.h"
#include "stdtypes.h"
#include "stddef.h"
#include "clientcomm.h"
#include "entity_power_client.h"
#include "PowerInfo.h"
#include "memcheck.h"
#include "itemselect.h"
#include "stdtypes.h"
#include "gridcoll.h"
#include "group.h"
#include "ctri.h"
#include "camera.h"
#include "cmdgame.h"
#include "win_init.h"
#include "entclient.h"
#include "player.h"
#include "gfxwindow.h"
#include "gfxwindow.h"
#include "assert.h"
#include "motion.h"
#include "groupdynrecv.h"
#include "font.h"
#include "float.h"
#include "uiCursor.h"
#include "uiTarget.h"
#include "utils.h"
#include "genericlist.h"
#include "textClientInclude.h"
#include "sprite_text.h"
#include "entity.h"
#include "MessageStoreUtil.h"


extern Entity	*current_target;
//extern PipeClient pc;
extern int g_nobrawl;

static DWORD lastDifferentTarget=0;
static FighterMode fighter_mode=FM_CYCLE_POWERS;

void setFighterMode(FighterMode fm) {
	fighter_mode = fm;
}

int out_of_range=0;
extern bool g_verbose_client;

void InfoBox(int msg_type, char *text, ...) {
	va_list arg;

	va_start(arg, text);
	text = vaTranslate(text, arg);
	va_end(arg);

	if (g_verbose_client)
	{
		printf("[info]: %s\n", text);
	}

	if (strstri(text, "You are dead and cannot")!=0)
		return;
	if (strstri(text, "too far away")!=0) {
		//Out of range
		out_of_range++;
		if (out_of_range>10) out_of_range=10;
	} else {
		out_of_range--;
		if (out_of_range<0) out_of_range=0;
		if (strstri(text, "you activated")!=0) {
			lastDifferentTarget = timeGetTime();
		}
	}
}


static float getAttackRating(const BasePower *base) {
	int i;
	float ret=0;

	if (base->eTargetType == kTargetType_Villain ||
			base->eTargetType == kTargetType_Foe) {
		if (stricmp(base->pchName, "Brawl")==0 && g_nobrawl) {
			return 0;
		}
		for( i = 0; i < eaSize( &base->ppTemplateIdx ); i++ )
		{
			const AttribModTemplate *mod = base->ppTemplateIdx[i];
			int iIdx;
			for (iIdx = 0; iIdx < eaiSize(&mod->piAttrib); iIdx++) {
				if (IS_HITPOINTS(mod->piAttrib[iIdx])) {
					ret += mod->fMagnitude;
				}
			}
		}
	}

	return ret;
}

// Used to cycle through powers
bool findNextTogglePower(int *ibuild, int *iset, int *ipow) {
	// ARM NOTE: Do we need to add ibuild_cursor?
	static int iset_cursor=-1;
	static int ipow_cursor=-1;

	int i, j;
	Entity *e = playerPtr();
	bool found_first=false;

	*iset=-1;

	for( i = 0; i < eaSize( &e->pchar->ppPowerSets ); i++ )
	{

		if( !e->pchar->ppPowerSets[i]->ppPowers ) // no powers, go to next power set
			continue;

		for( j = 0; j < eaSize( &e->pchar->ppPowerSets[i]->ppPowers ); j++ )
		{
			if(e->pchar->ppPowerSets[i]->ppPowers[j])
			{
				const BasePower *base = e->pchar->ppPowerSets[i]->ppPowers[j]->ppowBase;
				PowerRef ppowRef;
				PowerRechargeTimer * prt;

				if (base->eType != kPowerType_Toggle)
					continue;
				ppowRef.buildNum = e->pchar->iCurBuild;
				ppowRef.powerSet = i;
				ppowRef.power	= j;
				prt = powerInfo_PowerGetRechargeTimer(e->powerInfo, ppowRef);
				if (prt==NULL) { // It's ready!
					if (!found_first) {
						*ibuild = e->pchar->iCurBuild;
						*iset = i;
						*ipow = j;
						found_first=true;
					}
					if (i>iset_cursor || (i==iset_cursor && j>ipow_cursor)) {
						iset_cursor = i;
						ipow_cursor = j;
						*ibuild = e->pchar->iCurBuild;
						*iset = i;
						*ipow = j;
						return true;
					}
				}
			}
		}
	}
	if (found_first) {
		iset_cursor=*iset;
		ipow_cursor=*ipow;
	}
	return *iset!=-1;
}

// Used to cycle through powers
bool findNextPower(int *ibuild, int *iset, int *ipow) {
	// ARM NOTE: Do we need to add ibuild_cursor?
	static int iset_cursor=-1;
	static int ipow_cursor=-1;

	int i, j;
	Entity *e = playerPtr();
	bool found_first=false;

	*iset=-1;

	for( i = 0; i < eaSize( &e->pchar->ppPowerSets ); i++ )
	{

		if( !e->pchar->ppPowerSets[i]->ppPowers ) // no powers, go to next power set
			continue;

		for( j = 0; j < eaSize( &e->pchar->ppPowerSets[i]->ppPowers ); j++ )
		if (e->pchar->ppPowerSets[i]->ppPowers[j])
		{
			const BasePower *base = e->pchar->ppPowerSets[i]->ppPowers[j]->ppowBase;
			PowerRef ppowRef;
			PowerRechargeTimer * prt;

			ppowRef.buildNum = e->pchar->iCurBuild;
			ppowRef.powerSet = i;
			ppowRef.power	= j;
			prt = powerInfo_PowerGetRechargeTimer(e->powerInfo, ppowRef);
			if (prt==NULL) { // It's ready!
				if (!found_first) {
					*ibuild = e->pchar->iCurBuild;
					*iset = i;
					*ipow = j;
					found_first=true;
				}
				if (i>iset_cursor || (i==iset_cursor && j>ipow_cursor)) {
					iset_cursor = i;
					ipow_cursor = j;
					*ibuild = e->pchar->iCurBuild;
					*iset = i;
					*ipow = j;
					return true;
				}
			}
		}
	}
	if (found_first) {
		iset_cursor=*iset;
		ipow_cursor=*ipow;
	}
	return *iset!=-1;
}


bool findBestPower(int *ibuild, int *iset, int *ipow) {
	int i, j;
	Entity *e = playerPtr();
	float best=0;

	*ibuild = -1;
	*iset=-1;
	*ipow=-1;

	for( i = 0; i < eaSize( &e->pchar->ppPowerSets ); i++ )
	{

		if( !e->pchar->ppPowerSets[i]->ppPowers ) // no powers, go to next power set
			continue;

		for( j = 0; j < eaSize( &e->pchar->ppPowerSets[i]->ppPowers ); j++ )
		{
			const BasePower *base = e->pchar->ppPowerSets[i]->ppPowers[j]->ppowBase;
			float rating;
			PowerRef power;
			PowerRechargeTimer * prt;
			rating = getAttackRating(base);
			power.buildNum = e->pchar->iCurBuild;
			power.powerSet = i;
			power.power	= j;
			prt = powerInfo_PowerGetRechargeTimer(e->powerInfo, power);
			if (prt) { // Not ready yet!
				rating = 0;
			}

			if (rating > best) {
				*ibuild = e->pchar->iCurBuild;
				*iset = i;
				*ipow = j;
				best = rating;
			}
		}
	}
	return *iset!=-1;
}

#define TIME_TO_IGNORE 60000 // The number of ms to keep the same target before finally ignoring it
#define TIME_TO_FORGET (TIME_TO_IGNORE*15) // The number of ms before an entity is removed from the ignore list (must be much larger than time_to_ignore, otherwise we'll just cycle!)
typedef struct EntLinkedList {
	struct EntLinkedList *next;
	struct EntLinkedList *prev;
	Entity *data;
	DWORD expiration;
} EntLinkedList;

EntLinkedList *ignoreList=NULL; // Head
EntLinkedList *tail=NULL; // Tail

void addToIgnoreList(Entity *e) {
	ignoreList = listAddNewMember(&ignoreList, sizeof(EntLinkedList));
	ignoreList->data = e;
	ignoreList->expiration = timeGetTime() + TIME_TO_FORGET;
	if (!tail) {
		tail = ignoreList;
	}
	printf("FIGHER.C: Adding Entity %s (%d) to target ignore list\n", e->name, e->svr_idx);
}

void flushOldIgnores(void) {
	if (ignoreList && timeGetTime() > ignoreList->expiration) {
		EntLinkedList *newtail=tail->prev;
		listFreeMember(tail, &ignoreList);
		tail = newtail;
	}
}

bool isIgnored(Entity *e) {
	EntLinkedList *walk = ignoreList;
	while (walk) {
		if (walk->data==e)
			return true;
		walk = walk->next;
	}
	return false;
}

void checkIgnores(void) {
	static Entity *current_target_critter=NULL;
	static Entity *current_target_other=NULL;
	static DWORD lastTargetSwitch=0;
	if (!current_target) return;
	if (current_target != current_target_critter) {
		if (ENTTYPE(current_target)==ENTTYPE_CRITTER) {
			current_target_critter = current_target;
			lastTargetSwitch = timeGetTime();
		} else {
			current_target_other = current_target;
		}
	} else {
		// We're still targetting the same guy
		if (!atDest()) { // And we're out of range still
			DWORD timeSinceTargetting = timeGetTime() - lastTargetSwitch;
			if (timeSinceTargetting > TIME_TO_IGNORE) {
				addToIgnoreList(current_target);
				current_target = NULL;
			}
		} else {
			lastTargetSwitch = timeGetTime();
		}
	}
	flushOldIgnores();
}

void checkTarget(void) {
	static DWORD lastNewTarget=0;
	checkIgnores();
	if (!current_target || entMode( current_target, ENT_DEAD ) || lastNewTarget < timeGetTime() - 10000) {
		int old = current_target?current_target->owner:0;
		lastNewTarget = timeGetTime();
		selectNearestTarget(ENTTYPE_CRITTER, false);
		if (current_target) {
			if (atDest() && out_of_range==10) {
				startRunning();
				//printf("FIGHTER.C: atDest(), but out of range, moving!\n");
			} else if (current_target->owner==old) {
				setDestEnt(current_target);
				if (lastDifferentTarget < timeGetTime() - 60000 && atDest()) {
					// 60 seconds on the same target is too long!  Start moving randomly
					//setDestRandom();
					startMoving();
					lastDifferentTarget = timeGetTime();
					//printf("FIGHTER.C: atDest(), nothing happened in 60 seconds, moving randomly!\n");
				}
			} else {
				lastDifferentTarget = timeGetTime();
				// Don't care: PipeClientSendMessage(pc, "NewTarget: %d", current_target->owner);
				setDestEnt(current_target);
				//printf("FIGHTER.C: Got a new target, targetting him!\n");
			}
		} else if (atDest()) {
			// At the target position, but he's not there (dead)
			//setDestRandom();
			startMoving();
			//printf("FIGHTER.C: No target, but atDest(), moving randomly!\n");
		}
	}

}

void doAttacks(void) {
	int ibuild, iset, ipow;
	static int do_toggles_countdown = 0;

	do_toggles_countdown--;
	if (do_toggles_countdown <= 0) {
		do_toggles_countdown = 5;
		if (findNextTogglePower(&ibuild, &iset, &ipow)) {
			START_INPUT_PACKET(pak, CLIENTINP_ACTIVATE_POWER);
			entity_ActivatePowerSend(playerPtr(), pak, ibuild, iset, ipow, current_target);
			END_INPUT_PACKET
			return;
		}
	}

	if (fighter_mode==FM_BEST_ATTACK_POWER) {
		if (current_target && findBestPower(&ibuild, &iset, &ipow)) {
			//printf("Using power %s!\n", playerPtr()->pchar->ppPowerSets[iset]->ppPowers[ipow]->ppowBase->pchName);
			START_INPUT_PACKET(pak, CLIENTINP_ACTIVATE_POWER);
			entity_ActivatePowerSend(playerPtr(), pak, ibuild, iset, ipow, current_target);
			END_INPUT_PACKET
		}
	} else if (fighter_mode==FM_CYCLE_POWERS) {
		if (findNextPower(&ibuild, &iset, &ipow)) {
			selectNearestTargetForPower(iset, ipow);
			if (current_target) {
				START_INPUT_PACKET(pak, CLIENTINP_ACTIVATE_POWER);
				entity_ActivatePowerSend(playerPtr(), pak, ibuild, iset, ipow, current_target);
				END_INPUT_PACKET
			}
		}
	}
}

// From itemSelect.c

extern int gSelectedDBID;
static int selectEntityVisible(Entity *e)
{
	return 1;
}

static int entityOnSamePlane(Entity *e)
{
	F32 ydiff = ABS(ENTPOSY(playerPtr()) - ENTPOSY(e));
	return ydiff < 18;
}

void selectNearestTarget( int type, int dead )
{
	selectNearestTargetForPower(1,0);
}

int isEntitySelectable(Entity *e)
{
	if (!e || !game_state.select_any_entity && !game_state.place_entity &&
		(e->fadedirection == ENT_FADING_OUT))
	{
		return 0;
	}
	return 1;
}

void selectNearestTargetForPower(int iset, int ipow)
{
	Entity *pSrc = playerPtr();
	Power *ppow;
	int			i,idx = -1;
	Entity		*e;
	float		smallestDist = FLT_MAX;

	if(!pSrc || !pSrc->pchar || !eaGet(&pSrc->pchar->ppPowerSets, iset) || !eaGet(&pSrc->pchar->ppPowerSets[iset]->ppPowers, ipow))
		return;

	ppow = pSrc->pchar->ppPowerSets[iset]->ppPowers[ipow];

	for( i=1; i<=entities_max; i++)
	{
		float dist;

		if (!(entity_state[i] & ENTITY_IN_USE ) )
			continue;

		e = entities[i];

		if (!selectEntityVisible(e))
			continue;

		if (!entityOnSamePlane(e))
			continue;

		if (!character_PowerAffectsTarget(pSrc->pchar, e->pchar, ppow) || e == playerPtr())
			continue;

		if (isIgnored(e))
			continue;

		if (!isEntitySelectable(e))
			continue;

		if (e->erOwner) // Don't attack pets, they might be untargetable puddles.  Don't even try.
			continue;

		dist = distance3Squared( ENTPOS(playerPtr()), ENTPOS(e) );
		if ( dist < smallestDist )
		{
			idx = i;
			smallestDist = dist;
		}
	}

	if ( idx < 0 )
		current_target = 0;
	else
		current_target = entities[idx];

	gSelectedDBID = 0;
}

void targetByName(char *name)
{
	Entity *pSrc = playerPtr();

	int			i,idx = -1;
	Entity		*e;
	float		smallestDist = FLT_MAX;

	if (!name && !name[0]) {
		current_target = 0;
		return;
	}

	for( i=1; i<=entities_max; i++)
	{
		float dist;

		if (!(entity_state[i] & ENTITY_IN_USE ) )
			continue;

		e = entities[i];

		if (strnicmp(name, e->name, strlen(name)))
			continue;

		if (!isEntitySelectable(e))
			continue;

		dist = distance3Squared( ENTPOS(playerPtr()), ENTPOS(e) );
		if ( dist < smallestDist )
		{
			idx = i;
			smallestDist = dist;
		}
	}

	if ( idx < 0 )
		current_target = 0;
	else
		current_target = entities[idx];

	gSelectedDBID = 0;
	gSelectedHandle[0] = 0;
}