
#include "entaiprivate.h"
#include "entity.h"
#include "cmdcommon.h"
#include "character_base.h"
#include "earray.h"
#include "MemoryPool.h"
#include "StashTable.h"

// AIProxEntStatus ---------------------------------------------------------------------------

MP_DEFINE(AIProxEntStatus);

static AIProxEntStatus* createAIProxEntStatus(AIProxEntStatusTable* table){
	AIProxEntStatus* status;
	
	MP_CREATE(AIProxEntStatus, 50);
	
	status = MP_ALLOC(AIProxEntStatus);
	
	status->table = table;
	
//	status->time.posKnown = 0;
//	status->time.attackMe = 0;
//	status->time.attackMyFriend = 0;
	
	return status;
}

static void removeAIProxEntStatusBackRef(AIVars* ai, AIProxEntStatus* status){
	if(status->entityList.prev){
		status->entityList.prev->entityList.next = status->entityList.next;
	}else{
		ai->proxEntStatusList = status->entityList.next;
	}
	
	if(status->entityList.next){
		status->entityList.next->entityList.prev = status->entityList.prev;
	}

	status->entityList.prev = NULL;
	status->entityList.next = NULL;
}

static void destroyAIProxEntStatus(AIProxEntStatus* status){
	if(!status)
		return;
		
	if(status->entity){
		removeAIProxEntStatusBackRef(ENTAI(status->entity), status);
	}else{
		assert(!status->entityList.prev && !status->entityList.next);
	}
		
	MP_FREE(AIProxEntStatus, status);
}

// AIProxEntStatusTable ----------------------------------------------------------------------

MP_DEFINE(AIProxEntStatusTable);

AIProxEntStatusTable* createAIProxEntStatusTable(void* owner, int teamTable){	  
	AIProxEntStatusTable* table;
	
	MP_CREATE(AIProxEntStatusTable, 1000);
	
	table = MP_ALLOC(AIProxEntStatusTable);
	
	table->isTeamTable = teamTable;
	
	if(teamTable){
		table->owner.team = owner;
	}else{
		table->owner.entity = owner;
	}
	

	table->dbIdToStatusTable = stashTableCreateInt(4);
	table->entToStatusTable = stashTableCreateAddress(4);


	return table;
}

void destroyAIProxEntStatusTable(AIProxEntStatusTable* table){
	AIProxEntStatus* status;
	AIProxEntStatus* next;
	
	if(!table)
		return;

	stashTableDestroy(table->dbIdToStatusTable);
	table->dbIdToStatusTable = 0;

	stashTableDestroy(table->entToStatusTable);
	table->entToStatusTable = 0;
	
	for(status = table->statusHead; status; status = next){
		next = status->tableList.next;
		destroyAIProxEntStatus(status);
	}
	
	MP_FREE(AIProxEntStatusTable, table);
}

static void addToProxEntStatusList(AIProxEntStatusTable* table, AIProxEntStatus* status){
	if(!status)
		return;
		
	if(table->statusHead){
		table->statusTail->tableList.next = status;
		status->tableList.prev = table->statusTail;
		status->tableList.next = NULL;
		table->statusTail = status;
	}else{
		table->statusHead = table->statusTail = status;
		status->tableList.next = status->tableList.prev = NULL;
	}
}

static void removeFromProxEntStatusList(AIProxEntStatusTable* table, AIProxEntStatus* status){
	if(!status)
		return;
		
	aiProximityRemoveFromAvoidList(status);

	if(status->tableList.prev)
		status->tableList.prev->tableList.next = status->tableList.next;
	else if(table->statusHead == status)
		table->statusHead = status->tableList.next;
	else
		assert(0);
	
	if(status->tableList.next)
		status->tableList.next->tableList.prev = status->tableList.prev;
	else if(table->statusTail == status)
		table->statusTail = status->tableList.prev;
	else
		assert(0);
}

AIProxEntStatus* aiGetProxEntStatus(AIProxEntStatusTable* table, Entity* proxEnt, int create){
	AIProxEntStatus* status = NULL;
	StashElement element;
	int initStatus = 0;
	int i;
	
	if(!table || !proxEnt){
		return NULL;
	}
		
	if(!verify(proxEnt->owner > 0)){
		return NULL;
	}
	
	if(proxEnt->db_id > 0){
		// Players are tracked by db_id.
		
		if(!stashIntFindElement(table->dbIdToStatusTable, proxEnt->db_id, &element)){
			if(!create){
				element = NULL;
			}else{
				status = createAIProxEntStatus(table);
			
				stashIntAddPointerAndGetElement(table->dbIdToStatusTable, proxEnt->db_id, status, false, &element);
				
				initStatus = 1;
			}
		}
	}else{
		// Any other entity is tracked by entity pointer.

		if(!stashAddressFindElement(table->entToStatusTable, proxEnt, &element)){
			if(!create){
				element = NULL;
			}else{
				status = createAIProxEntStatus(table);
			
				stashAddressAddPointerAndGetElement(table->entToStatusTable, proxEnt, status, false, &element);
				
				initStatus = 1;
			}
		}
	}
	
	if(!status){
		status = element ? stashElementGetPointer(element) : 0;
	}

	if(initStatus || (status && !status->entity)){
		AIVars* aiProx = ENTAI(proxEnt);
		
		assert(aiProx);
		
		if(initStatus){
			addToProxEntStatusList(table, status);

			// Target Preference processing
			if(!table->isTeamTable)
			{
				Entity* e = table->owner.entity;
				AIVars* ai = ENTAI(e);

				for(i = 0; i < eaSize(&ai->targetPref); i++)
				{
					if(!strcmp(proxEnt->pchar->pclass->pchName, ai->targetPref[i]->archetype))
						status->targetPrefFactor = ai->targetPref[i]->factor;
				}
			}
		}

		status->entity = proxEnt;
		status->db_id = proxEnt->db_id;

		if(aiProx->proxEntStatusList){
			aiProx->proxEntStatusList->entityList.prev = status;
			status->entityList.next = aiProx->proxEntStatusList;
			status->entityList.prev = NULL;
		}else{
			status->entityList.next = status->entityList.prev = NULL;
		}

		aiProx->proxEntStatusList = status;
	}

	return status;
}

void aiRemoveProxEntStatus(AIProxEntStatusTable* table, Entity* proxEnt, int db_id, int fullRemove){
	AIProxEntStatus* status;
	
	if(!table)
		return;

	// If a proxEnt was passed in, then use the db_id from it.
	
	if(proxEnt && proxEnt->db_id > 0)
		db_id = proxEnt->db_id;

	if(db_id > 0){
		if(fullRemove){
			// Totally remove the status.
		
			if ( !stashIntRemovePointer(table->dbIdToStatusTable, db_id, &status) )
				status = NULL;
		}else{
			// Remove the entity pointer, but keep the db_id around, in case the player comes back later.
		
			
			if(stashIntFindPointer(table->dbIdToStatusTable, db_id, &status )){
				
				if(status && status->entity){
					removeAIProxEntStatusBackRef(ENTAI(status->entity), status);
					
					aiProximityRemoveFromAvoidList(status);
					
					status->entity = NULL;
				}
			}
			
			status = NULL;
		}
	}
	else if(proxEnt){
		if (!stashAddressRemovePointer(table->entToStatusTable, proxEnt, &status))
			status = NULL;
	}
	else{
		status = NULL;
	}
	
	if(status){
		removeFromProxEntStatusList(table, status);

		destroyAIProxEntStatus(status);
	}
}

void aiProximityRemoveEntitysStatusEntries(Entity* e, AIVars* ai, int fullRemove)
{
	while(ai->proxEntStatusList){
		AIProxEntStatus* status = ai->proxEntStatusList;

		if(status->table->isTeamTable){
			AITeam* team = status->table->owner.team;

			assert(team && status->entity == e);

			aiTeamEntityRemoved(team, e);
		}else{
			Entity* owner = status->table->owner.entity;
			AIVars* aiOwner = ENTAI(owner);
			EntType entTypeOwner = ENTTYPE(owner);

			assert(owner && aiOwner && status->entity == e);

			aiRemoveProxEntStatus(aiOwner->proxEntStatusTable, e, 0, fullRemove);

			if(aiOwner->helpTarget == e){
				aiOwner->helpTarget = NULL;
			}

			if (aiOwner->brain.type == AI_BRAIN_COT_BEHEMOTH) {
				if (aiOwner->brain.cotbehemoth.statusPrimary &&
					aiOwner->brain.cotbehemoth.statusPrimary->entity == e)
				{
					aiOwner->brain.cotbehemoth.statusPrimary = NULL;
				}
			}

			if(	aiOwner->followTarget.type == AI_FOLLOWTARGET_ENTITY &&
				aiOwner->followTarget.entity == e)
			{
				aiDiscardFollowTarget(owner, "Target was destroyed.", false);
			}

			switch(entTypeOwner){
				case ENTTYPE_CRITTER:
					aiCritterEntityWasDestroyed(owner, aiOwner, e);
					break;
				case ENTTYPE_NPC:
					aiNPCEntityWasDestroyed(owner, aiOwner, e);
					break;
			}
		}
	}
}

void aiProximityAddToAvoidList(AIProxEntStatus* status){
	if(!status->inAvoidList){
		AIProxEntStatusTable* table = status->table;

		status->avoidList.next = table->avoid.head;
		status->avoidList.prev = NULL;

		if(status->avoidList.next){
			status->avoidList.next->avoidList.prev = status;
		}

		table->avoid.head = status;

		status->inAvoidList = 1;
	}
}

void aiProximityRemoveFromAvoidList(AIProxEntStatus* status){
	if(status->inAvoidList){
		AIProxEntStatusTable* table = status->table;

		if(status->avoidList.prev){
			status->avoidList.prev->avoidList.next = status->avoidList.next;
		}else{
			table->avoid.head = status->avoidList.next;
		}
		
		if(status->avoidList.next){
			status->avoidList.next->avoidList.prev = status->avoidList.prev;
		}
		
		status->inAvoidList = 0;
	}
}



